#ifndef _MyListener_HXX
#include "MyListener.h"
#endif


#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

void SAL_CALL WriterListener::notifyEvent( const ::com::sun::star::document::EventObject& aEvent ) throw (com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL WriterListener::disposing( const com::sun::star::lang::EventObject& aSource ) throw( com::sun::star::uno::RuntimeException )
{
}

WriterListener::WriterListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > &rxMSF)
    : mxMSF( rxMSF )
{
}


