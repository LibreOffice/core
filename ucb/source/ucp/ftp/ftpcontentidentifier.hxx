#ifndef _FTP_FTPCONTENTIDENTIFIER_HXX_
#define _FTP_FTPCONTENTIDENTIFIER_HXX_

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIER_HPP_
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#endif


namespace ftp {

    class FtpContentIdentifier
        : public cppu::OWeakObject,
          public com::sun::star::ucb::XContentIdentifier
    {
    public:

        FtpContentIdentifier(const rtl::OUString& ident);

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

        rtl::OUString m_aIdent;
    };

}


#endif
