/*************************************************************************
*
*  $RCSfile: scripthandler.hxx,v $
*
*  $Revision: 1.5 $
*
*  last change: $Author: toconnor $ $Date: 2003-10-29 15:26:02 $
*
*  The Contents of this file are made available subject to the terms of
*  either of the following licenses
*
*         - GNU Lesser General Public License Version 2.1
*         - Sun Industry Standards Source License Version 1.1
*
*  Sun Microsystems Inc., October, 2000
*
*  GNU Lesser General Public License Version 2.1
*  =============================================
*  Copyright 2000 by Sun Microsystems, Inc.
*  901 San Antonio Road, Palo Alto, CA 94303, USA
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License version 2.1, as published by the Free Software Foundation.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
*  MA  02111-1307  USA
*
*
*  Sun Industry Standards Source License Version 1.1
*  =================================================
*  The contents of this file are subject to the Sun Industry Standards
*  Source License Version 1.1 (the "License"); You may not use this file
*  except in compliance with the License. You may obtain a copy of the
*  License at http://www.openoffice.org/license.html.
*
*  Software provided under this License is provided on an "AS IS" basis,
*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
*  See the License for the specific provisions governing your rights and
*  obligations concerning the Software.
*
*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
*
*  Copyright: 2000 by Sun Microsystems, Inc.
*
*  All Rights Reserved.
*
*  Contributor(s): _______________________________________
*
*
************************************************************************/

#ifndef _FRAMEWORK_SCRIPT_SCRIPTHANDLER_HXX
#define _FRAMEWORK_SCRIPT_SCRIPTHANDLER_HXX

#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/implbase4.hxx>
#include <drafts/com/sun/star/script/provider/XScriptProvider.hpp>

namespace rtl
{
class OUString;
};

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{
class Any;
}
namespace lang
{
class XMultiServiceFactory;
class XSingleServiceFactory;
}
namespace frame
{
class XFrame;
class XModel;
class XDispatch;
class XNotifyingDispatch;
class XDispatchResultListener;
struct DispatchDescriptor;
}
namespace beans
{
struct PropertyValue;
}
namespace util
{
struct URL;
}
}
}
}

namespace scripting_protocolhandler
{

// for simplification
#define css ::com::sun::star
#define dcss ::drafts::com::sun::star

class ScriptProtocolHandler :
public ::cppu::WeakImplHelper4< css::frame::XDispatchProvider,
    css::frame::XNotifyingDispatch, css::lang::XServiceInfo, css::lang::XInitialization >
{
private:
    bool m_bInitialised;
    css::uno::Reference < css::lang::XMultiServiceFactory > m_xFactory;
    css::uno::Reference < css::frame::XFrame > m_xFrame;
    css::uno::Reference < dcss::script::provider::XScriptProvider >
    m_xScriptProvider;
    void createScriptProvider( const ::rtl::OUString& url ) throw( css::uno::RuntimeException );
public:
    ScriptProtocolHandler( const css::uno::Reference <
        css::lang::XMultiServiceFactory >& xFactory );
    virtual ~ScriptProtocolHandler();

    /* XServiceInfo */
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& sServiceName )
        throw( css::uno::RuntimeException );
    virtual css::uno::Sequence < ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException );

    /* Helper for XServiceInfo */
    static css::uno::Sequence < ::rtl::OUString > impl_getStaticSupportedServiceNames();
    static ::rtl::OUString impl_getStaticImplementationName();

    /* Helper for registry */
    static css::uno::Reference < css::uno::XInterface > SAL_CALL
    impl_createInstance(
        const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager )
    throw( css::uno::RuntimeException );
    static css::uno::Reference < css::lang::XSingleServiceFactory > impl_createFactory(
        const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager );

    /* Implementation for XDispatchProvider */
    virtual css::uno::Reference < css::frame::XDispatch > SAL_CALL
    queryDispatch( const css::util::URL& aURL, const ::rtl::OUString& sTargetFrameName,
                   sal_Int32 eSearchFlags ) throw( css::uno::RuntimeException ) ;
    virtual css::uno::Sequence< css::uno::Reference < css::frame::XDispatch > > SAL_CALL
    queryDispatches(
        const css::uno::Sequence < css::frame::DispatchDescriptor >& seqDescriptor )
    throw( css::uno::RuntimeException );

    /* Implementation for X(Notifying)Dispatch */
    virtual void SAL_CALL dispatchWithNotification(
    const css::util::URL& aURL,
    const css::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArgs,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchResultListener >& Listener )
    throw ( css::uno::RuntimeException );
    virtual void SAL_CALL dispatch(
        const css::util::URL& aURL,
        const css::uno::Sequence< css::beans::PropertyValue >& lArgs )
        throw ( css::uno::RuntimeException );
    virtual void SAL_CALL addStatusListener(
        const css::uno::Reference< css::frame::XStatusListener >& xControl,
        const css::util::URL& aURL )
        throw ( css::uno::RuntimeException );
    virtual void SAL_CALL removeStatusListener(
        const css::uno::Reference< css::frame::XStatusListener >& xControl,
        const css::util::URL& aURL )
        throw ( css::uno::RuntimeException );

    /* Implementation for XInitialization */
    virtual void SAL_CALL initialize(
        const css::uno::Sequence < css::uno::Any >& aArguments )
        throw ( css::uno::Exception );
};

}
#endif
