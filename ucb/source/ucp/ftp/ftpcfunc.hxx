#ifndef _FTP_FTPCFUNC_HXX_
#define _FTP_FTPCFUNC_HXX_

#include <rtl/ustring.hxx>


namespace ftp {

    class FTPStreamContainer
    {
    public:

        virtual write(void *buffer,size_t size,size_t nmemb) = 0;
    };

}


extern "C" {

    /** callback for curl_easy_perform(),
     *  forwarding the written content to the stream.
     *  stream has to be of type 'FTPStreamContainer'.
     */


    int ftp_write(void *buffer,size_t size,size_t nmemb,void *stream);
}




namespace ftp {

    /** Is the first argument to 'passwd' below.
     */

    class FTPClient
    {
    public:

        virtual rtl::OUString passwd() const = 0;
    };

}


extern "C" {

    int ftp_passwd(void *client,char*prompt,char*buffer,int bufferlength);

}

#endif
