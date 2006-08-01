#ifndef _MyJob_HXX
#include "MyJob.h"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTBROADCASTER_HPP_
#include <com/sun/star/document/XEventBroadcaster.hpp>
#endif

using rtl::OUString;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::beans::NamedValue;
using com::sun::star::document::XEventBroadcaster;

Any SAL_CALL MyJob::execute( const Sequence< NamedValue >& aArguments )
    throw ( IllegalArgumentException, Exception, RuntimeException )
{
    Reference < XEventBroadcaster > xBrd( mxMSF->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.GlobalEventBroadcaster") ), UNO_QUERY );
    Reference < com::sun::star::document::XEventListener > xLstner( mxMSF->createInstance(
        ::rtl::OUString::createFromAscii("com.sun.star.comp.Office.MyListener" ) ), UNO_QUERY );
    if ( xBrd.is() )
        xBrd->addEventListener( xLstner );
    return Any();
}

OUString MyJob_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Office.MyJob" ) );
}

#define SERVICE_NAME "com.sun.star.task.Job"

sal_Bool SAL_CALL MyJob_supportsService( const OUString& ServiceName )
    throw (RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
}

Sequence< OUString > SAL_CALL MyJob_getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
    return aRet;
}

#undef SERVICE_NAME

Reference< XInterface > SAL_CALL MyJob_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new MyJob( rSMgr );
}

// XServiceInfo
OUString SAL_CALL MyJob::getImplementationName(  )
    throw (RuntimeException)
{
    return MyJob_getImplementationName();
}

sal_Bool SAL_CALL MyJob::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return MyJob_supportsService( rServiceName );
}

Sequence< OUString > SAL_CALL MyJob::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    return MyJob_getSupportedServiceNames();
}
