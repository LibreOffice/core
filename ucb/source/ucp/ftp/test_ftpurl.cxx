#include "ftpurl.hxx"
#include "ftploaderthread.hxx"
#include "ftphandleprovider.hxx"


class FTPHandleProviderI
    : public ftp::FTPHandleProvider {

public:

    FTPHandleProviderI()
        : p(new ftp::FTPLoaderThread) {
    }

    ~FTPHandleProviderI() {
        delete p;
    }

    virtual CURL* handle() {
        return p->handle();
    }

private:

    ftp::FTPLoaderThread *p;
};




int test_ftpurl(void) {
    int number_of_errors = 0;

    ftp::FTPURL url1(
        rtl::OUString::createFromAscii(
            "ftp://abi:psswd@host/eins/../drei"));
    if(!url1.username().equalsAscii("abi"))
        ++number_of_errors;
    if(!url1.password().equalsAscii("psswd"))
        ++number_of_errors;
    if(!url1.ident().equalsAscii("ftp://host:21/drei"))
        ++number_of_errors;

    ftp::FTPURL url2(
        rtl::OUString::createFromAscii(
            "ftp://:psswd@host:22/../../test"));
    if(!url2.username().equalsAscii("anonymous"))
        ++number_of_errors;
    if(!url2.password().equalsAscii("psswd"))
        ++number_of_errors;
    if(!url2.ident().equalsAscii("ftp://host:22/../../test"))
        ++number_of_errors;


    ftp::FTPURL url3(
        rtl::OUString::createFromAscii(
            "ftp://host/bla/../../test/"));
    if(!url3.username().equalsAscii("anonymous"))
        ++number_of_errors;
    if(!url3.password().equalsAscii(""))
        ++number_of_errors;
    if(!url3.ident().equalsAscii("ftp://host:21/../test"))
        ++number_of_errors;


    return number_of_errors;
}



int test_ftpproperties(void) {
    int number_of_errors = 0;

    FTPHandleProviderI provider;

    ftp::FTPURL url3(
        rtl::OUString::createFromAscii(
            "ftp://abi:psswd@abi-1/zips"),
        &provider);

    ftp::FTPDirentry aDE = url3.direntry();

    return number_of_errors;
}
