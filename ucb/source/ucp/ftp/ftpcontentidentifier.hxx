#ifndef _FTP_FTPCONTENTIDENTIFIER_HXX_
#define _FTP_FTPCONTENTIDENTIFIER_HXX_

#include <vector>
#include <curl/curl.h>
#include <curl/easy.h>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/io/XOutputStream.hpp>


#include "ftpdirp.hxx"
#include "ftpurl.hxx"

namespace ftp {


    class FTPContentProvider;


    class FTPContentIdentifier
        : public cppu::OWeakObject,
          public com::sun::star::ucb::XContentIdentifier
    {
    public:

        FTPContentIdentifier(const rtl::OUString& ident,
                             FTPContentProvider* pFCP = 0);

        FTPContentIdentifier(FTPURL *pURL);

        ~FTPContentIdentifier();

        // XInterface

        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface( const com::sun::star::uno::Type& rType )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL acquire( void ) throw();

        virtual void SAL_CALL release( void ) throw();


        // XContentIdentifier

        virtual ::rtl::OUString SAL_CALL
        getContentIdentifier(
        )
            throw (
                ::com::sun::star::uno::RuntimeException
            );

        virtual ::rtl::OUString SAL_CALL
        getContentProviderScheme(
        )
            throw (
                ::com::sun::star::uno::RuntimeException
            );


    private:

        FTPURL *m_pURL;
    };

}


#endif
