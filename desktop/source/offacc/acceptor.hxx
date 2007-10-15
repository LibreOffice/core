/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: acceptor.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-15 13:00:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_CONNECTION_XACCEPTOR_HPP_
#include <com/sun/star/connection/XAcceptor.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_BRIDGE_XINSTANCEPROVIDER_HPP_
#include <com/sun/star/bridge/XInstanceProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_BRIDGE_XBRIDGEFACTORY_HPP_
#include <com/sun/star/bridge/XBridgeFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <comphelper/weakbag.hxx>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <osl/thread.hxx>


using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::connection;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::registry;

namespace desktop {

class  Acceptor
    : public ::cppu::WeakImplHelper2<XServiceInfo, XInitialization>
{
private:
    static const sal_Char *serviceName;
    static const sal_Char *implementationName;
    static const sal_Char *supportedServiceNames[];

    static Mutex m_aMutex;

    oslThread m_thread;
    comphelper::WeakBag< com::sun::star::bridge::XBridge > m_bridges;

    Condition m_cEnable;

    Reference< XMultiServiceFactory > m_rSMgr;
    Reference< XInterface >           m_rContext;
    Reference< XAcceptor >            m_rAcceptor;
    Reference< XBridgeFactory >       m_rBridgeFactory;

    OUString m_aAcceptString;
    OUString m_aConnectString;
    OUString m_aProtocol;

    sal_Bool m_bInit;

public:
    Acceptor( const Reference< XMultiServiceFactory >& aFactory );
    virtual ~Acceptor();

    void SAL_CALL run();

    // XService info
    static  OUString                    impl_getImplementationName();
    virtual OUString           SAL_CALL getImplementationName()
        throw (RuntimeException);
    static  Sequence<OUString>          impl_getSupportedServiceNames();
    virtual Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);
    virtual sal_Bool           SAL_CALL supportsService( const OUString& aName )
        throw (RuntimeException);

    // XInitialize
    virtual void SAL_CALL initialize( const Sequence<Any>& aArguments )
        throw ( Exception );

    static  Reference<XInterface> impl_getInstance( const Reference< XMultiServiceFactory >& aFactory );
};

class AccInstanceProvider : public ::cppu::WeakImplHelper1<XInstanceProvider>
{
private:
    Reference<XMultiServiceFactory> m_rSMgr;
    Reference<XConnection> m_rConnection;

public:
    AccInstanceProvider(const Reference< XMultiServiceFactory >& aFactory,
                        const Reference< XConnection >& rConnection);
    virtual ~AccInstanceProvider();

    // XInstanceProvider
    virtual Reference<XInterface> SAL_CALL getInstance (const OUString& aName )
        throw ( NoSuchElementException );
};


} //namespace desktop

