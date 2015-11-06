#include <iostream>
#include <memory>
#include <string>
#include <vector>



#include "src/MicroCore.h"
#include "src/tools.h"


using namespace std;

unsigned int epee::g_test_dbg_lock_sleep = 0;



int main() {

    // enable basic monero log output
    // uint32_t log_level = 0;
    // epee::log_space::get_set_log_detalisation_level(true, log_level);
    // epee::log_space::log_singletone::add_logger(LOGGER_CONSOLE, NULL, NULL); //LOGGER_NULL

    // location of the lmdb blockchain
    string blockchain_path {"/home/mwo/.bitmonero/lmdb"};

    // input data: public address, private view key and tx hash
    string address_str {"48daf1rG3hE1Txapcsxh6WXNe9MLNKtu7W7tKTivtSoVLHErYzvdcpea2nSTgGkz66RFP4GKVAsTV14v6G3oddBTHfxP6tU"};
    string viewkey_str {"1ddabaa51cea5f6d9068728dc08c7ffaefe39a7a4b5f39fa8a976ecbe2cb520a"};
    string tx_hash_str {"66040ad29f0d780b4d47641a67f410c28cce575b5324c43b784bb376f4e30577"};


    // our micro cryptonote core
    xmreg::MicroCore mcore;

    if (!mcore.init(blockchain_path))
    {
        cerr << "Error accessing blockchain." << endl;
        return 1;
    }

    // get the high level cryptonote::Blockchain object to interact
    // with the blockchain lmdb database
    cryptonote::Blockchain& core_storage = mcore.get_core();

    // get the current blockchain height. Just to check
    // if it reads ok.
    uint64_t height = core_storage.get_current_blockchain_height();

    cout << "Blockchain height:" << height << endl;



    // parse string representing of monero address
    cryptonote::account_public_address address;

    if (!xmreg::parse_str_address(address_str,  address))
    {
        cerr << "Cant parse string address: " << address_str << endl;
        return 1;
    }


    // parse string representing of our private viewkey
    crypto::secret_key prv_view_key;
    if (!xmreg::parse_str_secret_key(viewkey_str, prv_view_key))
    {
        cerr << "Cant parse view key: " << viewkey_str << endl;
        return 1;
    }


    // we also need tx public key, rather than tx hash.
    // to get it first, we obtained transaction object tx
    // and then we get its public key from tx's extras.
    cryptonote::transaction tx;

    if (!xmreg::get_tx_pub_key_from_str_hash(core_storage, tx_hash_str, tx))
    {
        cerr << "Cant find transaction with hash: " << tx_hash_str << endl;
        return 1;
    }


    crypto::public_key pub_tx_key = cryptonote::get_tx_pub_key_from_extra(tx);

    if (pub_tx_key == cryptonote::null_pkey)
    {
        cerr << "Cant get public key of tx with hash: " << tx_hash_str << endl;
        return 1;
    }


    // public transaction key is combined with our view key
    // to get so called, derived key.
    crypto::key_derivation derivation;

    if (!generate_key_derivation(pub_tx_key, prv_view_key, derivation))
    {
        cerr << "Cant get dervied key for: " << endl
             << "pub_tx_key: " << prv_view_key << " and "
             << "prv_view_key" << prv_view_key << endl;
        return 1;
    }




    // lets check our keys
    cout << endl;
    cout << "address    : <" << xmreg::print_address(address) << ">" << endl
         << "viewkey    : "  << prv_view_key << endl
         << "txkey      : "  << pub_tx_key << endl
         << "dervied key: "  << derivation << endl;
    cout << endl;


    // each tx that we (or the adddress we are checking) received
    // contains a number of outputs.
    // some of them are ours, some not. so we need to go through
    // all of them in a given tx block, to check with outputs are ours.

    // get the total number of outputs in a transaction.
    size_t output_no = tx.vout.size();

    // sum amount of xmr sent to us
    // in the given transaction
    uint64_t money_transfered {0};

    // loop through outputs in the given tx
    // to check which outputs our ours, we compare outputs
    // public keys, with the public key that would had been
    // generated for us.
    for (size_t i = 0; i < output_no; ++i)
    {
        // get the tx output public key
        // that normally would be generated for us,
        // if someone send us some xrm
        crypto::public_key pubkey;

        crypto::derive_public_key(derivation,
                                  i,
                                  address.m_spend_public_key,
                                  pubkey);


        // get tx output public key
        const cryptonote::txout_to_key tx_out_to_key
                = boost::get<cryptonote::txout_to_key>(tx.vout[i].target);


        cout << "Output no: " << i << "," << tx_out_to_key.key;

        // check if the output's public key is ours
        if (tx_out_to_key.key == pubkey)
        {
            // if so, that add the xmr amount to the money_transfered
            money_transfered += tx.vout[i].amount;
            cout << ", mine key: " << cryptonote::print_money(tx.vout[i].amount) << endl;
        }
        else
        {
            cout << ", not mine key " << endl;
        }

    }

    cout << "\nTotal xmr recivied: " << cryptonote::print_money(money_transfered) << endl;


    cout << endl << endl;



    // print all transactions
//    size_t idx {0};
//    std::vector<crypto::hash> txids;
//    core_storage.for_all_transactions([&txids, &idx](const crypto::hash& hash,
//                                                const cryptonote::transaction& tx)->bool
//                                       {
//                                           if (++idx % 500 == 0)
//                                             cout << "\r" << idx << ": " << hash << std::flush;
//                                           txids.push_back(hash);
//                                           return true;
//                                       });
//
//    for (size_t i=0; i < txids.size(); ++i)
//    {
//        cout << i << ": "  << txids[i] << endl;
//    }




    cout << "Hgello, World!" << endl;
    return 0;
}