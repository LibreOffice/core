#ifndef _LAYOUT_UNO_HXX
#define _LAYOUT_UNO_HXX

#include <stdio.h>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "root.hxx"
#include "factory.hxx"

#if LAYOUT_WEAK
#include <cppuhelper/implbase1.hxx>
class UnoBootstrapLayout : public ::cppu::WeakImplHelper1< com::sun::star::lang::XMultiServiceFactory >
#else /* !LAYOUT_WEAK */
class UnoBootstrapLayout : public com::sun::star::lang::XMultiServiceFactory
#endif /* LAYOUT_WEAK */
{
public:
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;
    UnoBootstrapLayout( com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > xMSF )
        : mxMSF( xMSF )
    {
        fprintf( stderr, "UnoBootstrap Layout\n" );
    }
    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
    createInstance( const rtl::OUString& aServiceSpecifier ) throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException)
    {
        if ( aServiceSpecifier.equalsAscii( "com.sun.star.awt.Layout" ) )
        {
            fprintf( stderr, "UnoBootstrapLayout: create service '%s'\n",
                     rtl::OUStringToOString (aServiceSpecifier, RTL_TEXTENCODING_UTF8 ).getStr() );
            return com::sun::star::uno::Reference< com::sun::star::lang::XSingleServiceFactory >( new ::LayoutFactory( this ) );
        }
        else
        {
            fprintf( stderr, "UnoBootstrapLayout: create service '%s'\n",
                     rtl::OUStringToOString (aServiceSpecifier, RTL_TEXTENCODING_UTF8 ).getStr() );
            try
            {
                return mxMSF->createInstance( aServiceSpecifier );
            }
            catch ( const com::sun::star::uno::Exception &rExc )
            {
                rtl::OString aStr( rtl::OUStringToOString( rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
                fprintf( stderr, "service construction exception '%s'\n", aStr.getStr());
                throw rExc;
            }
        }
    }
    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
    createInstanceWithArguments( const rtl::OUString& ServiceSpecifier, const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& Arguments ) throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException)
    {
        return mxMSF->createInstanceWithArguments( ServiceSpecifier, Arguments );
    }
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getAvailableServiceNames() throw (com::sun::star::uno::RuntimeException)
    {
        return mxMSF->getAvailableServiceNames();
    }

#if !LAYOUT_WEAK
    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
    {
        return mxMSF->queryInterface( rType );
    }
    virtual void SAL_CALL acquire() throw()
    {
        mxMSF->acquire();
    }
    virtual void SAL_CALL release() throw()
    {
        mxMSF->release();
    }
#endif /* !LAYOUT_WEAK */
};

#endif /* _LAYOUT_UNO_HXX */
