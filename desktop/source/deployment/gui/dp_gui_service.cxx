/*************************************************************************
 *
 *  $RCSfile: dp_gui_service.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:04:51 $
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

#include "dp_gui.h"
#include "dp_servicefactory.h"
#include "cppuhelper/implbase3.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "unotools/configmgr.hxx"
#include "tools/isolang.hxx"
#include "vcl/svapp.hxx"
#include "vcl/help.hxx"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/task/XJobExecutor.hpp"


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace dp_gui
{

//==============================================================================
class MyApp : public Application, public Help
{
public:
    MyApp();
    virtual ~MyApp();

    // Application
    virtual void Main();

    // Help
    virtual BOOL Start( ULONG nHelpId, Window const * pWindow  );
    virtual BOOL Start( XubString const & rKeyWord, Window const * pWindow );
    virtual XubString GetHelpText( ULONG nHelpId, Window const * pWindow );
};

//______________________________________________________________________________
MyApp::~MyApp()
{
}

//______________________________________________________________________________
MyApp::MyApp()
{
    EnableAutoHelpId();
    SetHelp( this );
}

//______________________________________________________________________________
void MyApp::Main()
{
}

//______________________________________________________________________________
BOOL MyApp::Start( ULONG nHelpId, Window const * pWindow  )
{
    // xxx todo:
    return false;
}

//______________________________________________________________________________
BOOL MyApp::Start( XubString const & rKeyWord, Window const * pWindow )
{
    // xxx todo:
    return false;
}

//______________________________________________________________________________
XubString MyApp::GetHelpText( ULONG nHelpId, Window const * pWindow )
{
    // xxx todo:
    return XubString();
}

//##############################################################################

//==============================================================================
class ServiceImpl :
    public ::cppu::WeakImplHelper3< lang::XServiceInfo,
                                    ui::dialogs::XExecutableDialog,
                                    task::XJobExecutor >
{
    Reference<XComponentContext> m_xComponentContext;
    Reference<awt::XWindow> m_xParent;
    OUString m_view;
    OUString m_initialTitle;

public:
    ServiceImpl( Sequence<Any> const & args,
                 Reference<XComponentContext> const & xComponentContext );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
        throw (RuntimeException);
    virtual Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);

    // XExecutableDialog
    virtual void SAL_CALL setTitle( OUString const & title )
        throw (RuntimeException);
    virtual sal_Int16 SAL_CALL execute() throw (RuntimeException);

    // XJobExecutor
    virtual void SAL_CALL trigger( OUString const & event )
        throw (RuntimeException);
};

//______________________________________________________________________________
ServiceImpl::ServiceImpl(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext )
    : m_xComponentContext( xComponentContext )
{
    if (args.getLength() > 0)
    {
        extract_throw( &m_xParent, args[ 0 ] );
        if (args.getLength() > 1)
            extract_throw( &m_view, args[ 1 ] );
    }
}

//==============================================================================
static Reference<XInterface> SAL_CALL create(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext )
{
    return static_cast< ::cppu::OWeakObject * >(
        new ServiceImpl( args, xComponentContext ) );
}

//==============================================================================
static OUString SAL_CALL getImplementationName()
{
    return OUSTR("com.sun.star.comp.deployment.ui.PackageManagerDialog");
}

//==============================================================================
static Sequence<OUString> SAL_CALL getSupportedServiceNames()
{
    OUString strName = OUSTR("com.sun.star.deployment.ui.PackageManagerDialog");
    return Sequence<OUString>( &strName, 1 );
}

// XServiceInfo
//______________________________________________________________________________
OUString ServiceImpl::getImplementationName()
    throw (RuntimeException)
{
    return ::dp_gui::getImplementationName();
}

//______________________________________________________________________________
sal_Bool ServiceImpl::supportsService( OUString const & serviceName )
    throw (RuntimeException)
{
    return ::dp_gui::getSupportedServiceNames()[ 0 ].equals( serviceName );
}

//______________________________________________________________________________
Sequence<OUString> ServiceImpl::getSupportedServiceNames()
    throw (RuntimeException)
{
    return ::dp_gui::getSupportedServiceNames();
}

// XExecutableDialog
//______________________________________________________________________________
void ServiceImpl::setTitle( OUString const & title ) throw (RuntimeException)
{
    if (DialogImpl::s_dialog.is())
    {
        ::vos::OGuard guard( Application::GetSolarMutex() );
        DialogImpl::get(
            m_xComponentContext, m_xParent, m_view )->SetText( title );
    }
    else
        m_initialTitle = title;
}

//______________________________________________________________________________
sal_Int16 ServiceImpl::execute() throw (RuntimeException)
{
    ::std::auto_ptr<Application> app;
    if (!DialogImpl::s_dialog.is() && !::dp_misc::office_is_running())
    {
        app.reset( new MyApp );
        if (! InitVCL( Reference<lang::XMultiServiceFactory>(
                           m_xComponentContext->getServiceManager(),
                           UNO_QUERY_THROW ) ))
            throw RuntimeException( OUSTR("Cannot initialize VCL!"),
                                    static_cast<OWeakObject *>(this) );
        AllSettings as = app->GetSettings();
        OUString slang;
        if (! (::utl::ConfigManager::GetDirectConfigProperty(
                   ::utl::ConfigManager::LOCALE ) >>= slang))
            throw RuntimeException( OUSTR("Cannot determine language!"),
                                    static_cast<OWeakObject *>(this) );
        as.SetUILanguage( ConvertIsoStringToLanguage( slang ) );
        app->SetSettings( as );
    }

    {
        ::vos::OGuard guard( Application::GetSolarMutex() );
        ::rtl::Reference<DialogImpl> dialog(
            DialogImpl::get( m_xComponentContext, m_xParent, m_view ) );
        if (m_initialTitle.getLength() > 0)
        {
            dialog->SetText( m_initialTitle );
            m_initialTitle = OUString();
        }
        dialog->Show();
        dialog->ToTop( TOTOP_RESTOREWHENMIN );
    }

    if (app.get() != 0)
    {
        Application::Execute();
        DeInitVCL();
    }

    return 0;
}

// XJobExecutor
//______________________________________________________________________________
void ServiceImpl::trigger( OUString const & event ) throw (RuntimeException)
{
    execute();
}

static const ::cppu::ImplementationEntry s_entries [] =
{
    {
        (::cppu::ComponentFactoryFunc) create,
        getImplementationName,
        getSupportedServiceNames,
        createFactory,
        0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

}

extern "C"
{

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo(
    lang::XMultiServiceFactory * pServiceManager,
    registry::XRegistryKey * pRegistryKey )
{
    return ::cppu::component_writeInfoHelper(
        pServiceManager, pRegistryKey, ::dp_gui::s_entries );
}

void * SAL_CALL component_getFactory(
    sal_Char const * pImplName,
    lang::XMultiServiceFactory * pServiceManager,
    registry::XRegistryKey * pRegistryKey )
{
    return ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, ::dp_gui::s_entries );
}

}
