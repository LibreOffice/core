/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_service.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 17:03:46 $
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

#include "dp_gui.h"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "unotools/configmgr.hxx"
#include "comphelper/servicedecl.hxx"
#include "comphelper/unwrapargs.hxx"
#include <i18npool/mslangid.hxx>
#include "vcl/svapp.hxx"
#include "vcl/msgbox.hxx"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/task/XJobExecutor.hpp"

#include "boost/bind.hpp"
#include "license_dialog.hxx"

using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace dp_gui {

//==============================================================================
class MyApp : public Application, private boost::noncopyable
{
public:
    MyApp();
    virtual ~MyApp();

    // Application
    virtual void Main();
};

//______________________________________________________________________________
MyApp::~MyApp()
{
}

//______________________________________________________________________________
MyApp::MyApp()
{
}

//______________________________________________________________________________
void MyApp::Main()
{
}

//##############################################################################

//==============================================================================
class ServiceImpl
    : public ::cppu::WeakImplHelper2<ui::dialogs::XExecutableDialog,
                                     task::XJobExecutor>
{
    Reference<XComponentContext> const m_xComponentContext;
    boost::optional< Reference<awt::XWindow> > /* const */ m_parent;
    boost::optional<OUString> /* const */ m_view;
    OUString m_initialTitle;

public:
    ServiceImpl( Sequence<Any> const & args,
                 Reference<XComponentContext> const & xComponentContext );

    // XExecutableDialog
    virtual void SAL_CALL setTitle( OUString const & title )
        throw (RuntimeException);
    virtual sal_Int16 SAL_CALL execute() throw (RuntimeException);

    // XJobExecutor
    virtual void SAL_CALL trigger( OUString const & event )
        throw (RuntimeException);
};

//______________________________________________________________________________
ServiceImpl::ServiceImpl( Sequence<Any> const& args,
                          Reference<XComponentContext> const& xComponentContext)
    : m_xComponentContext(xComponentContext)
{
    comphelper::unwrapArgs( args, m_parent, m_view );
}

// XExecutableDialog
//______________________________________________________________________________
void ServiceImpl::setTitle( OUString const & title ) throw (RuntimeException)
{
    if (::dp_gui::DialogImpl::s_dialog.is()) {
        const ::vos::OGuard guard( Application::GetSolarMutex() );
        ::dp_gui::DialogImpl::get(
            m_xComponentContext,
            m_parent ? *m_parent : Reference<awt::XWindow>(),
            m_view ? *m_view : OUString() )->SetText( title );
    }
    else
        m_initialTitle = title;
}

//______________________________________________________________________________
sal_Int16 ServiceImpl::execute() throw (RuntimeException)
{
    ::std::auto_ptr<Application> app;
    if (! dp_gui::DialogImpl::s_dialog.is())
    {
        const bool bAppUp = (GetpApp() != 0);
        bool bOfficePipePresent;
        try {
            bOfficePipePresent = dp_misc::office_is_running();
        }
        catch (Exception & exc) {
            if (bAppUp) {
                const vos::OGuard guard( Application::GetSolarMutex() );
                std::auto_ptr<ErrorBox> box(
                    new ErrorBox( Application::GetActiveTopWindow(),
                                  WB_OK, exc.Message ) );
                box->Execute();
            }
            throw;
        }

        if (! bOfficePipePresent) {
            OSL_ASSERT( ! bAppUp );
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
            as.SetUILanguage( MsLangId::convertIsoStringToLanguage( slang ) );
            app->SetSettings( as );
        }
    }

    {
        const ::vos::OGuard guard( Application::GetSolarMutex() );
        ::rtl::Reference< ::dp_gui::DialogImpl > dialog(
            ::dp_gui::DialogImpl::get(
                m_xComponentContext,
                m_parent ? *m_parent : Reference<awt::XWindow>(),
                m_view ? *m_view : OUString() ) );
        if (m_initialTitle.getLength() > 0) {
            dialog->SetText( m_initialTitle );
            m_initialTitle = OUString();
        }
        dialog->Show();
        dialog->ToTop( TOTOP_RESTOREWHENMIN );
    }

    if (app.get() != 0) {
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

namespace sdecl = comphelper::service_decl;
sdecl::class_<ServiceImpl, sdecl::with_args<true> > serviceSI;
sdecl::ServiceDecl const serviceDecl(
    serviceSI,
    "com.sun.star.comp.deployment.ui.PackageManagerDialog",
    "com.sun.star.deployment.ui.PackageManagerDialog" );

sdecl::class_<LicenseDialog, sdecl::with_args<true> > licenseSI;
sdecl::ServiceDecl const licenseDecl(
    licenseSI,
    "com.sun.star.comp.deployment.ui.LicenseDialog",
    "com.sun.star.deployment.ui.LicenseDialog" );
} // namespace dp_gui

extern "C" {

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo(
    lang::XMultiServiceFactory * pServiceManager,
    registry::XRegistryKey * pRegistryKey )
{
    return component_writeInfoHelper(
        pServiceManager, pRegistryKey, dp_gui::serviceDecl, dp_gui::licenseDecl );
}

void * SAL_CALL component_getFactory(
    sal_Char const * pImplName,
    lang::XMultiServiceFactory * pServiceManager,
    registry::XRegistryKey * pRegistryKey )
{
    return component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, dp_gui::serviceDecl, dp_gui::licenseDecl );
}

} // extern "C"
