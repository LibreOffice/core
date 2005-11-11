/*************************************************************************
 *
 *  $RCSfile: systemexec.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-11 14:13:34 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_DISPATCH_SYSTEMEXEC_HXX_
#include <dispatch/systemexec.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_SYSTEM_XSYSTEMSHELLEXECUTE_HPP_
#include <com/sun/star/system/XSystemShellExecute.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XSTRINGSUBSTITUTION_HPP_
#include <com/sun/star/util/XStringSubstitution.hpp>
#endif

#ifndef _COM_SUN_STAR_SYSTEM_SYSTEMSHELLEXECUTEFLAGS_HPP_
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTSTATE_HPP_
#include <com/sun/star/frame/DispatchResultState.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#include <vcl/svapp.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

#define PROTOCOL_VALUE      "systemexecute:"
#define PROTOCOL_LENGTH     14

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
// XInterface, XTypeProvider, XServiceInfo

DEFINE_XINTERFACE_5(SystemExec                                      ,
                    OWeakObject                                     ,
                    DIRECT_INTERFACE(css::lang::XTypeProvider      ),
                    DIRECT_INTERFACE(css::lang::XServiceInfo       ),
                    DIRECT_INTERFACE(css::frame::XDispatchProvider ),
                    DIRECT_INTERFACE(css::frame::XNotifyingDispatch),
                    DIRECT_INTERFACE(css::frame::XDispatch         ))

DEFINE_XTYPEPROVIDER_5(SystemExec                    ,
                       css::lang::XTypeProvider      ,
                       css::lang::XServiceInfo       ,
                       css::frame::XDispatchProvider ,
                       css::frame::XNotifyingDispatch,
                       css::frame::XDispatch         )

DEFINE_XSERVICEINFO_MULTISERVICE(SystemExec                   ,
                                 ::cppu::OWeakObject          ,
                                 SERVICENAME_PROTOCOLHANDLER  ,
                                 IMPLEMENTATIONNAME_SYSTEMEXEC)

DEFINE_INIT_SERVICE(SystemExec,
                    {
                        /*Attention
                            I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                            to create a new instance of this class by our own supported service factory.
                            see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                        */
                    }
                   )

//_________________________________________________________________________________________________________________

SystemExec::SystemExec( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory )
        //  Init baseclasses first
        : ThreadHelpBase( &Application::GetSolarMutex() )
        , OWeakObject   (                               )
        // Init member
        , m_xFactory    ( xFactory                      )
{
}

//_________________________________________________________________________________________________________________

SystemExec::~SystemExec()
{
    m_xFactory = NULL;
}

//_________________________________________________________________________________________________________________

css::uno::Reference< css::frame::XDispatch > SAL_CALL SystemExec::queryDispatch( const css::util::URL&  aURL    ,
                                                                                 const ::rtl::OUString& sTarget ,
                                                                                       sal_Int32        nFlags  ) throw( css::uno::RuntimeException )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;
    if (aURL.Complete.compareToAscii(PROTOCOL_VALUE,PROTOCOL_LENGTH)==0)
        xDispatcher = this;
    return xDispatcher;
}

//_________________________________________________________________________________________________________________

css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL SystemExec::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor ) throw( css::uno::RuntimeException )
{
    sal_Int32 nCount = lDescriptor.getLength();
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > lDispatcher( nCount );
    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcher[i] = this->queryDispatch(
                            lDescriptor[i].FeatureURL,
                            lDescriptor[i].FrameName,
                            lDescriptor[i].SearchFlags);
    }
    return lDispatcher;
}

//_________________________________________________________________________________________________________________

void SAL_CALL SystemExec::dispatch( const css::util::URL&                                  aURL       ,
                                    const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException )
{
    dispatchWithNotification(aURL, lArguments, css::uno::Reference< css::frame::XDispatchResultListener >());
}

//_________________________________________________________________________________________________________________

void SAL_CALL SystemExec::dispatchWithNotification( const css::util::URL&                                             aURL      ,
                                                    const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                    const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) throw( css::uno::RuntimeException )
{
    // convert "systemexec:file:///c:/temp/test.html" => "file:///c:/temp/test.html"
    sal_Int32 c = aURL.Complete.getLength()-PROTOCOL_LENGTH;
    if (c<1) // we dont check for valid URLs here! The system will show an error message ...
    {
        impl_notifyResultListener(xListener, css::frame::DispatchResultState::FAILURE);
        return;
    }
    ::rtl::OUString sSystemURLWithVariables = aURL.Complete.copy(PROTOCOL_LENGTH, c);

    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xFactory = m_xFactory;
    aReadLock.unlock();
    // <- SAFE

    // TODO check security settings ...

    try
    {
        css::uno::Reference< css::util::XStringSubstitution > xPathSubst(
            xFactory->createInstance(SERVICENAME_SUBSTITUTEPATHVARIABLES),
            css::uno::UNO_QUERY_THROW);

        ::rtl::OUString sSystemURL = xPathSubst->substituteVariables(sSystemURLWithVariables, sal_True); // sal_True force an exception if unknown variables exists !

        css::uno::Reference< css::system::XSystemShellExecute > xShell(
            xFactory->createInstance(SERVICENAME_SYSTEMSHELLEXECUTE),
            css::uno::UNO_QUERY_THROW);

        xShell->execute(sSystemURL, ::rtl::OUString(), css::system::SystemShellExecuteFlags::DEFAULTS);
        impl_notifyResultListener(xListener, css::frame::DispatchResultState::SUCCESS);
    }
    catch(const css::uno::Exception&)
        {
            impl_notifyResultListener(xListener, css::frame::DispatchResultState::FAILURE);
        }
}

//_________________________________________________________________________________________________________________

void SAL_CALL SystemExec::addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                             const css::util::URL&                                     aURL      ) throw( css::uno::RuntimeException )
{
    // not suported yet
}

//_________________________________________________________________________________________________________________

void SAL_CALL SystemExec::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                                const css::util::URL&                                     aURL      ) throw( css::uno::RuntimeException )
{
    // not suported yet
}

//_________________________________________________________________________________________________________________

void SystemExec::impl_notifyResultListener(const css::uno::Reference< css::frame::XDispatchResultListener >& xListener,
                                           const sal_Int16                                                   nState   )
{
    if (xListener.is())
    {
        css::frame::DispatchResultEvent aEvent;
        aEvent.State = nState;
        xListener->dispatchFinished(aEvent);
    }
}

}       //  namespace framework
