/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <memory>

#include "updatecheck.hxx"
#include "updatecheckconfig.hxx"
#include "updatehdl.hxx"
#include "updateprotocol.hxx"

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implementationentry.hxx>

#include "com/sun/star/frame/XDesktop.hpp"
#include "com/sun/star/frame/XTerminateListener.hpp"
#include <com/sun/star/task/XJob.hpp>

namespace beans = com::sun::star::beans ;
namespace frame = com::sun::star::frame ;
namespace lang = com::sun::star::lang ;
namespace task = com::sun::star::task ;
namespace uno = com::sun::star::uno ;

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

namespace
{

class InitUpdateCheckJobThread : public osl::Thread
{
public:
    InitUpdateCheckJobThread( const uno::Reference< uno::XComponentContext > &xContext,
                              const uno::Sequence< beans::NamedValue > &xParameters,
                              bool bShowDialog );

    virtual void SAL_CALL run();

    void    setTerminating();

private:
    osl::Condition m_aCondition;
    uno::Reference<uno::XComponentContext> m_xContext;
    uno::Sequence<beans::NamedValue> m_xParameters;
    bool m_bShowDialog;
    bool m_bTerminating;
};

class UpdateCheckJob :
    public ::cppu::WeakImplHelper3< task::XJob, lang::XServiceInfo, frame::XTerminateListener >
{
    virtual ~UpdateCheckJob();

public:

    UpdateCheckJob(const uno::Reference<uno::XComponentContext>& xContext);

    static uno::Sequence< rtl::OUString > getServiceNames();
    static rtl::OUString getImplName();

    // Allows runtime exceptions to be thrown by const methods
    inline SAL_CALL operator uno::Reference< uno::XInterface > () const
        { return const_cast< cppu::OWeakObject * > (static_cast< cppu::OWeakObject const * > (this)); };

    // XJob
    virtual uno::Any SAL_CALL execute(const uno::Sequence<beans::NamedValue>&)
        throw (lang::IllegalArgumentException, uno::Exception);

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & serviceName)
        throw (uno::RuntimeException);
    virtual uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw (uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( ::com::sun::star::lang::EventObject const & evt )
        throw (::com::sun::star::uno::RuntimeException);

    // XTerminateListener
    virtual void SAL_CALL queryTermination( lang::EventObject const & evt )
        throw ( frame::TerminationVetoException, uno::RuntimeException );
    virtual void SAL_CALL notifyTermination( lang::EventObject const & evt )
        throw ( uno::RuntimeException );

private:
    uno::Reference<uno::XComponentContext>  m_xContext;
    uno::Reference< frame::XDesktop >       m_xDesktop;
    std::auto_ptr< InitUpdateCheckJobThread > m_pInitThread;

    void handleExtensionUpdates( const uno::Sequence< beans::NamedValue > &rListProp );
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
void SAL_CALL InitUpdateCheckJobThread::run()
{
    if (!m_bShowDialog) {
        TimeValue tv = { 25, 0 };
        m_aCondition.wait( &tv );
        if ( m_bTerminating )
            return;
    }

    rtl::Reference< UpdateCheck > aController( UpdateCheck::get() );
    aController->initialize( m_xParameters, m_xContext );

    if ( m_bShowDialog )
        aController->showDialog( true );
}

void InitUpdateCheckJobThread::setTerminating() {
    m_bTerminating = true;
    m_aCondition.set();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

UpdateCheckJob::UpdateCheckJob( const uno::Reference<uno::XComponentContext>& xContext ) :
    m_xContext(xContext)
{
    m_xDesktop.set( xContext->getServiceManager()->createInstanceWithContext( UNISTRING("com.sun.star.frame.Desktop"), xContext ), uno::UNO_QUERY );
    if ( m_xDesktop.is() )
        m_xDesktop->addTerminateListener( this );
}

//------------------------------------------------------------------------------

UpdateCheckJob::~UpdateCheckJob()
{
}

//------------------------------------------------------------------------------

uno::Sequence< rtl::OUString >
UpdateCheckJob::getServiceNames()
{
    uno::Sequence< rtl::OUString > aServiceList(1);
    aServiceList[0] = UNISTRING( "com.sun.star.setup.UpdateCheck");
    return aServiceList;
};

//------------------------------------------------------------------------------

rtl::OUString
UpdateCheckJob::getImplName()
{
    return UNISTRING( "vnd.sun.UpdateCheck");
}


//------------------------------------------------------------------------------

uno::Any
UpdateCheckJob::execute(const uno::Sequence<beans::NamedValue>& namedValues)
    throw (lang::IllegalArgumentException, uno::Exception)
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

    rtl::OUString aEventName = getValue< rtl::OUString > (aEnvironment, "EventName");

    m_pInitThread.reset(
        new InitUpdateCheckJobThread(
            m_xContext, aConfig,
            !aEventName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("onFirstVisibleTask"))));

    return uno::Any();
}

//------------------------------------------------------------------------------
void UpdateCheckJob::handleExtensionUpdates( const uno::Sequence< beans::NamedValue > &rListProp )
{
    try {
        uno::Sequence< uno::Sequence< rtl::OUString > > aList =
            getValue< uno::Sequence< uno::Sequence< rtl::OUString > > > ( rListProp, "updateList" );
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
            rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
    }
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL
UpdateCheckJob::getImplementationName() throw (uno::RuntimeException)
{
    return getImplName();
}

//------------------------------------------------------------------------------

uno::Sequence< rtl::OUString > SAL_CALL
UpdateCheckJob::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return getServiceNames();
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL
UpdateCheckJob::supportsService( rtl::OUString const & serviceName ) throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > aServiceNameList = getServiceNames();

    for( sal_Int32 n=0; n < aServiceNameList.getLength(); n++ )
        if( aServiceNameList[n].equals(serviceName) )
            return sal_True;

    return sal_False;
}

//------------------------------------------------------------------------------
// XEventListener
void SAL_CALL UpdateCheckJob::disposing( lang::EventObject const & rEvt )
    throw ( uno::RuntimeException )
{
    bool shutDown = ( rEvt.Source == m_xDesktop );

    if ( shutDown && m_xDesktop.is() )
    {
        m_xDesktop->removeTerminateListener( this );
        m_xDesktop.clear();
    }
}

//------------------------------------------------------------------------------
// XTerminateListener
void SAL_CALL UpdateCheckJob::queryTermination( lang::EventObject const & )
    throw ( frame::TerminationVetoException, uno::RuntimeException )
{
}

//------------------------------------------------------------------------------
void SAL_CALL UpdateCheckJob::notifyTermination( lang::EventObject const & )
    throw ( uno::RuntimeException )
{
    if ( m_pInitThread.get() != 0 )
    {
        m_pInitThread->setTerminating();
        m_pInitThread->join();
    }
}

} // anonymous namespace

//------------------------------------------------------------------------------

static uno::Reference<uno::XInterface> SAL_CALL
createJobInstance(const uno::Reference<uno::XComponentContext>& xContext)
{
    return *new UpdateCheckJob(xContext);
}

//------------------------------------------------------------------------------

static uno::Reference<uno::XInterface> SAL_CALL
createConfigInstance(const uno::Reference<uno::XComponentContext>& xContext)
{
    return *UpdateCheckConfig::get(xContext, *UpdateCheck::get());
}

//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL updchk_component_getFactory(const sal_Char *pszImplementationName, void *pServiceManager, void *pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pszImplementationName,
        pServiceManager,
        pRegistryKey,
        kImplementations_entries) ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
