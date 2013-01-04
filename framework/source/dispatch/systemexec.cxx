/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <dispatch/systemexec.hxx>
#include <threadhelp/readguard.hxx>
#include <general.h>
#include <services.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>

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

DEFINE_XSERVICEINFO_MULTISERVICE_2(SystemExec                   ,
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

SystemExec::SystemExec( const css::uno::Reference< css::uno::XComponentContext >& xContext )
        //  Init baseclasses first
        : ThreadHelpBase( &Application::GetSolarMutex() )
        , OWeakObject   (                               )
        // Init member
        , m_xContext    ( xContext                      )
{
}

//_________________________________________________________________________________________________________________

SystemExec::~SystemExec()
{
    m_xContext = NULL;
}

//_________________________________________________________________________________________________________________

css::uno::Reference< css::frame::XDispatch > SAL_CALL SystemExec::queryDispatch( const css::util::URL&  aURL    ,
                                                                                 const ::rtl::OUString&,
                                                                                       sal_Int32 ) throw( css::uno::RuntimeException )
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
                                                    const css::uno::Sequence< css::beans::PropertyValue >&,
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
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.unlock();
    // <- SAFE

    // TODO check security settings ...

    try
    {
        css::uno::Reference< css::util::XStringSubstitution > xPathSubst(
            xContext->getServiceManager()->createInstanceWithContext(
                SERVICENAME_SUBSTITUTEPATHVARIABLES, xContext),
            css::uno::UNO_QUERY_THROW);

        ::rtl::OUString sSystemURL = xPathSubst->substituteVariables(sSystemURLWithVariables, sal_True); // sal_True force an exception if unknown variables exists !

        css::uno::Reference< css::system::XSystemShellExecute > xShell(
            css::system::SystemShellExecute::create(xContext));

        xShell->execute(sSystemURL, ::rtl::OUString(), css::system::SystemShellExecuteFlags::DEFAULTS);
        impl_notifyResultListener(xListener, css::frame::DispatchResultState::SUCCESS);
    }
    catch(const css::uno::Exception&)
        {
            impl_notifyResultListener(xListener, css::frame::DispatchResultState::FAILURE);
        }
}

//_________________________________________________________________________________________________________________

void SAL_CALL SystemExec::addStatusListener( const css::uno::Reference< css::frame::XStatusListener >&,
                                             const css::util::URL& ) throw( css::uno::RuntimeException )
{
    // not suported yet
}

//_________________________________________________________________________________________________________________

void SAL_CALL SystemExec::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >&,
                                                const css::util::URL& ) throw( css::uno::RuntimeException )
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
