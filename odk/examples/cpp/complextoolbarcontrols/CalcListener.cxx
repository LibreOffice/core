#ifndef _MyListener_HXX
#include "MyListener.h"
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

void SAL_CALL CalcListener::notifyEvent( const ::com::sun::star::document::EventObject& aEvent ) throw (com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL CalcListener::disposing( const com::sun::star::lang::EventObject& aSource ) throw( com::sun::star::uno::RuntimeException )
{
}

