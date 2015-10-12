/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include "updatecheck.hxx"
#include "updatecheckconfig.hxx"
#include "updatehdl.hxx"
#include "updateprotocol.hxx"

#include <memory>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>

#include "com/sun/star/frame/Desktop.hpp"
#include "com/sun/star/frame/XTerminateListener.hpp"
#include <com/sun/star/task/XJob.hpp>

namespace beans = com::sun::star::beans ;
namespace frame = com::sun::star::frame ;
namespace lang = com::sun::star::lang ;
namespace task = com::sun::star::task ;
namespace uno = com::sun::star::uno ;

namespace
{

class InitUpdateCheckJobThread : public osl::Thread
{
public:
    InitUpdateCheckJobThread( const uno::Reference< uno::XComponentContext > &xContext,
                              const uno::Sequence< beans::NamedValue > &xParameters,
                              bool bShowDialog );

    virtual void SAL_CALL run() override;

    void    setTerminating();

private:
    osl::Condition m_aCondition;
    uno::Reference<uno::XComponentContext> m_xContext;
    uno::Sequence<beans::NamedValue> m_xParameters;
    bool m_bShowDialog;
    bool m_bTerminating;
};

class UpdateCheckJob :
    public ::cppu::WeakImplHelper< task::XJob, lang::XServiceInfo, frame::XTerminateListener >
{
    virtual ~UpdateCheckJob();

public:

    UpdateCheckJob(
        css::uno::Reference<css::uno::XComponentContext> const & context,
        css::uno::Reference<css::frame::XDesktop2> const & desktop):
        m_xContext(context), m_xDesktop(desktop)
    {}

    static uno::Sequence< OUString > getServiceNames();
    static OUString getImplName();

    // XJob
    virtual uno::Any SAL_CALL execute(const uno::Sequence<beans::NamedValue>&)
        throw (lang::IllegalArgumentException, uno::Exception, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(OUString const & serviceName)
        throw (uno::RuntimeException, std::exception) override;
    virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (uno::RuntimeException, std::exception) override;

    // XEventListener
    virtual void SAL_CALL disposing( css::lang::EventObject const & evt )
        throw (css::uno::RuntimeException, std::exception) override;

    // XTerminateListener
    virtual void SAL_CALL queryTermination( lang::EventObject const & evt )
        throw ( frame::TerminationVetoException, uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL notifyTermination( lang::EventObject const & evt )
        throw ( uno::RuntimeException, std::exception ) override;

private:
    uno::Reference<uno::XComponentContext>  m_xContext;
    uno::Reference< frame::XDesktop2 >      m_xDesktop;
    std::unique_ptr< InitUpdateCheckJobThread > m_pInitThread;

    void handleExtensionUpdates( const uno::Sequence< beans::NamedValue > &rListProp );
};




InitUpdateCheckJobThread::InitUpdateCheckJobThread(
            const uno::Reference< uno::XComponentContext > &xContext,
            const uno::Sequence< beans::NamedValue > &xParameters,
            bool bShowDialog ) :
    m_xContext( xContext ),
    m_xParameters( xParameters ),
    m_bShowDialog( bShowDialog ),
    m_bTerminating( false )
{
    create();
}


void SAL_CALL InitUpdateCheckJobThread::run()
{
    osl_setThreadName("InitUpdateCheckJobThread");

    if (!m_bShowDialog) {
        TimeValue tv = { 25, 0 };
        m_aCondition.wait( &tv );
        if ( m_bTerminating )
            return;
    }

    try {
        rtl::Reference< UpdateCheck > aController( UpdateCheck::get() );
        aController->initialize( m_xParameters, m_xContext );

        if ( m_bShowDialog )
            aController->showDialog( true );
    } catch (const uno::Exception &e) {
        // fdo#64962 - don't bring the app down on some unexpected exception.
        OSL_TRACE( "Caught init update exception: %s\n thread terminated.\n",
            OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr() );
    }
}

void InitUpdateCheckJobThread::setTerminating() {
    m_bTerminating = true;
    m_aCondition.set();
}

UpdateCheckJob::~UpdateCheckJob()
{
}



uno::Sequence< OUString >
UpdateCheckJob::getServiceNames()
{
    uno::Sequence< OUString > aServiceList(1);
    aServiceList[0] = "com.sun.star.setup.UpdateCheck";
    return aServiceList;
};



OUString
UpdateCheckJob::getImplName()
{
    return OUString("vnd.sun.UpdateCheck");
}




uno::Any
UpdateCheckJob::execute(const uno::Sequence<beans::NamedValue>& namedValues)
    throw (lang::IllegalArgumentException, uno::Exception, std::exception)
{
    for ( sal_Int32 n=namedValues.getLength(); n-- > 0; )
    {
        if ( namedValues[ n ].Name == "DynamicData" )
        {
            uno::Sequence<beans::NamedValue> aListProp;
            if ( namedValues[n].Value >>= aListProp )
            {
                for ( sal_Int32 i=aListProp.getLength(); i-- > 0; )
                {
                    if ( aListProp[ i ].Name == "updateList" )
                    {
                        handleExtensionUpdates( aListProp );
                        return uno::Any();
                    }
                }
            }
        }
    }

    uno::Sequence<beans::NamedValue> aConfig =
        getValue< uno::Sequence<beans::NamedValue> > (namedValues, "JobConfig");

    /* Determine the way we got invoked here -
     * see Developers Guide Chapter "4.7.2 Jobs" to understand the magic
     */

    uno::Sequence<beans::NamedValue> aEnvironment =
        getValue< uno::Sequence<beans::NamedValue> > (namedValues, "Environment");

    OUString aEventName = getValue< OUString > (aEnvironment, "EventName");

    m_pInitThread.reset(
        new InitUpdateCheckJobThread(
            m_xContext, aConfig,
            aEventName != "onFirstVisibleTask"));

    return uno::Any();
}


void UpdateCheckJob::handleExtensionUpdates( const uno::Sequence< beans::NamedValue > &rListProp )
{
    try {
        uno::Sequence< uno::Sequence< OUString > > aList =
            getValue< uno::Sequence< uno::Sequence< OUString > > > ( rListProp, "updateList" );
        bool bPrepareOnly = getValue< bool > ( rListProp, "prepareOnly" );

        // we will first store any new found updates and then check, if there are any
        // pending updates.
        storeExtensionUpdateInfos( m_xContext, aList );

        if ( bPrepareOnly )
            return;

        bool bHasUpdates = checkForPendingUpdates( m_xContext );

        rtl::Reference<UpdateCheck> aController( UpdateCheck::get() );
        if ( ! aController.is() )
            return;

        aController->setHasExtensionUpdates( bHasUpdates );

        if ( ! aController->hasOfficeUpdate() )
        {
            if ( bHasUpdates )
                aController->setUIState( UPDATESTATE_EXT_UPD_AVAIL, true );
            else
                aController->setUIState( UPDATESTATE_NO_UPDATE_AVAIL, true );
        }
    }
    catch( const uno::Exception& e )
    {
         OSL_TRACE( "Caught exception: %s\n thread terminated.\n",
            OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
    }
}



OUString SAL_CALL
UpdateCheckJob::getImplementationName() throw (uno::RuntimeException, std::exception)
{
    return getImplName();
}



uno::Sequence< OUString > SAL_CALL
UpdateCheckJob::getSupportedServiceNames() throw (uno::RuntimeException, std::exception)
{
    return getServiceNames();
}

sal_Bool SAL_CALL
UpdateCheckJob::supportsService( OUString const & serviceName ) throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, serviceName);
}


// XEventListener
void SAL_CALL UpdateCheckJob::disposing( lang::EventObject const & rEvt )
    throw ( uno::RuntimeException, std::exception )
{
    bool shutDown = ( rEvt.Source == m_xDesktop );

    if ( shutDown && m_xDesktop.is() )
    {
        m_xDesktop->removeTerminateListener( this );
        m_xDesktop.clear();
    }
}


// XTerminateListener
void SAL_CALL UpdateCheckJob::queryTermination( lang::EventObject const & )
    throw ( frame::TerminationVetoException, uno::RuntimeException, std::exception )
{
}


void SAL_CALL UpdateCheckJob::notifyTermination( lang::EventObject const & )
    throw ( uno::RuntimeException, std::exception )
{
    if ( m_pInitThread.get() != 0 )
    {
        m_pInitThread->setTerminating();
        m_pInitThread->join();
    }
}

} // anonymous namespace



static uno::Reference<uno::XInterface> SAL_CALL
createJobInstance(const uno::Reference<uno::XComponentContext>& xContext)
{
    css::uno::Reference<css::frame::XDesktop2> desktop(
        css::frame::Desktop::create(xContext));
    rtl::Reference<UpdateCheckJob> job(new UpdateCheckJob(xContext, desktop));
    desktop->addTerminateListener(job.get());
    return static_cast<cppu::OWeakObject *>(job.get());
}



static uno::Reference<uno::XInterface> SAL_CALL
createConfigInstance(const uno::Reference<uno::XComponentContext>& xContext)
{
    return *UpdateCheckConfig::get(xContext, *UpdateCheck::get());
}



static const cppu::ImplementationEntry kImplementations_entries[] =
{
    {
        createJobInstance,
        UpdateCheckJob::getImplName,
        UpdateCheckJob::getServiceNames,
        cppu::createSingleComponentFactory,
        NULL,
        0
    },
    {
        createConfigInstance,
        UpdateCheckConfig::getImplName,
        UpdateCheckConfig::getServiceNames,
        cppu::createSingleComponentFactory,
        NULL,
        0
    },
    { NULL, NULL, NULL, NULL, NULL, 0 }
} ;



extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL updchk_component_getFactory(const sal_Char *pszImplementationName, void *pServiceManager, void *pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pszImplementationName,
        pServiceManager,
        pRegistryKey,
        kImplementations_entries) ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
