#ifndef _FTP_STRCONT_HXX_
#define _FTP_STRCONT_HXX_

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include "ftpcfunc.hxx"


namespace ftp {


    class FTPInputStream;


    class FTPOutputStreamContainer
        : public FTPStreamContainer
    {
    public:

        FTPOutputStreamContainer(const com::sun::star::uno::Reference<
                                 com::sun::star::io::XOutputStream>& out);

        virtual write(void *buffer,size_t size,size_t nmemb);


    private:

        com::sun::star::uno::Reference<
        com::sun::star::io::XOutputStream> m_out;
    };


    class FTPInputStreamContainer
        : public FTPStreamContainer
    {
    public:

        FTPInputStreamContainer(FTPInputStream* out);

        virtual write(void *buffer,size_t size,size_t nmemb);

        com::sun::star::uno::Reference<
        com::sun::star::io::XInputStream> operator()();

    private:

        FTPInputStream* m_out;
    };


}


#endif
