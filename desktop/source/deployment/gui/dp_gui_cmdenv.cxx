/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_cmdenv.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-27 10:21:09 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "dp_gui.hrc"
#include "dp_gui.h"
#include "dp_gui_cmdenv.h"
#include "dp_gui_shared.hxx"
#include "dp_gui_dependencydialog.hxx"
#include "dp_dependencies.hxx"
#include "dp_identifier.hxx"
#include "dp_version.hxx"
#include "comphelper/anytostring.hxx"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/task/XInteractionApprove.hpp"
#include "com/sun/star/deployment/DependencyException.hpp"
#include "com/sun/star/deployment/LicenseException.hpp"
#include "com/sun/star/deployment/LicenseIndividualAgreementException.hpp"
#include "com/sun/star/deployment/VersionException.hpp"
#include "com/sun/star/deployment/InstallException.hpp"
#include "com/sun/star/deployment/ui/LicenseDialog.hpp"
#include "com/sun/star/ui/dialogs/ExecutableDialogResults.hpp"
#include "tools/resid.hxx"
#include "tools/rcid.h"
#include "vcl/msgbox.hxx"
#include "vcl/threadex.hxx"
#include "toolkit/helper/vclunohelper.hxx"
#include "boost/bind.hpp"


namespace css = ::com::sun::star;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace {

rtl::OUString getVersion(
    css::uno::Reference< css::deployment::XPackage > const & package)
{
    rtl::OUString s(package->getVersion());
    return s.getLength() == 0
        ? rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0")) : s;
}

}

namespace dp_gui
{
    ProgressCommandEnv::UpdateParams::UpdateParams(
        Reference<XInterface> const & _xProgressCommandEnv, OUString const & _text):
        xProgressCommandEnv(_xProgressCommandEnv),
        text(_text)
{
}
//______________________________________________________________________________
void ProgressCommandEnv::ProgressDialog::CancelButtonImpl::Click()
{
    m_dialog->m_cmdEnv->m_aborted = true;
    if (m_dialog->m_cmdEnv->m_xAbortChannel.is())
    {
        try {
            m_dialog->m_cmdEnv->m_xAbortChannel->sendAbort();
        }
        catch (RuntimeException &) {
            OSL_ENSURE( 0, "### unexpected RuntimeException!" );
        }
    }
}

void ProgressCommandEnv::solarthread_dtor()
{
    if (m_progressDialog.get() != 0) {
        const ::vos::OGuard guard( Application::GetSolarMutex() );
        m_progressDialog->SetModalInputMode( FALSE );
        m_progressDialog.reset();
    }
}

Dialog * ProgressCommandEnv::activeDialog() {
    Dialog * p = m_progressDialog.get();
    return p == NULL ? m_dialog : p;
}

//______________________________________________________________________________
ProgressCommandEnv::~ProgressCommandEnv()
{
    vcl::solarthread::syncExecute(
        boost::bind( &ProgressCommandEnv::solarthread_dtor, this ) );
}

//______________________________________________________________________________
ProgressCommandEnv::ProgressDialog::ProgressDialog(
    ProgressCommandEnv * cmdEnv )
    : Dialog( cmdEnv->m_dialog, WB_STDMODAL ),
      m_cmdEnv( cmdEnv )
{
    SetModalInputMode( TRUE );
}

//______________________________________________________________________________
IMPL_LINK( ProgressCommandEnv, executeDialog, ::osl::Condition *, pCond )
{
    {
        DialogImpl *mainDialog = dynamic_cast<dp_gui::DialogImpl*>(m_dialog);
        if (mainDialog == NULL)
        {
            OSL_ASSERT(0);
            return 0;
        }
        ::std::auto_ptr<ProgressDialog> that( new ProgressDialog( this ) );
        that->SetHelpId( HID_PACKAGE_MANAGER_PROGRESS );

        that->SetStyle( that->GetStyle() & ~WB_CLOSEABLE ); // non-closeable
        that->SetText( m_title );

        long totalWidth = that->LogicToPixel(
            Size( 250, 0 ), MapMode( MAP_APPFONT ) ).getWidth();
        long barWidth = totalWidth -
            (2 * mainDialog->m_borderLeftTopSpace.getWidth());
        long posY = mainDialog->m_borderLeftTopSpace.getHeight();

        that->m_ftCurrentAction.reset( new FixedText( that.get() ) );
        that->m_ftCurrentAction->SetPosSizePixel(
            mainDialog->m_borderLeftTopSpace.getWidth(), posY,
            barWidth, mainDialog->m_ftFontHeight );
        posY += (mainDialog->m_ftFontHeight +
                 mainDialog->m_descriptionYSpace);

        that->m_statusBar.reset(
            new StatusBar( that.get(), WB_LEFT | WB_3DLOOK ) );
        that->m_statusBar->SetPosSizePixel(
            mainDialog->m_borderLeftTopSpace.getWidth(), posY,
            barWidth, mainDialog->m_ftFontHeight + 4 );
        posY += (mainDialog->m_ftFontHeight + 4 +
                 mainDialog->m_unrelatedSpace.getHeight());

        that->m_cancelButton.reset(
            new ProgressDialog::CancelButtonImpl( that.get() ) );
        that->m_cancelButton->SetHelpId( HID_PACKAGE_MANAGER_PROGRESS_CANCEL );
        that->m_cancelButton->SetPosSizePixel(
            (totalWidth - mainDialog->m_buttonSize.getWidth()) / 2,
            posY,
            mainDialog->m_buttonSize.getWidth(),
            mainDialog->m_buttonSize.getHeight() );
        posY += mainDialog->m_buttonSize.getHeight();

        that->SetSizePixel(
            Size( totalWidth,
                  posY + mainDialog->m_borderRightBottomSpace.getHeight() ) );

        that->m_ftCurrentAction->Show();
        that->m_statusBar->Show();
        that->m_cancelButton->Show();
        that->Show();
        m_progressDialog = that;
    }

    pCond->set();
    return 0;
}

//______________________________________________________________________________
void ProgressCommandEnv::showProgress( sal_Int32 progressSections )
{
    m_progressSections = progressSections;
    m_currentProgressSection = 0;
    m_currentInnerProgress = 0;
    if (m_progressDialog.get() == 0) {
        ::osl::Condition cond;
        Application::PostUserEvent(
            LINK( this, ProgressCommandEnv, executeDialog ), &cond );
        cond.wait();
    }
}

//______________________________________________________________________________
void ProgressCommandEnv::progressSection(
    String const & text, Reference<task::XAbortChannel> const & xAbortChannel )
{
    m_xAbortChannel = xAbortChannel;
    if (! m_aborted)
    {
        ++m_currentProgressSection;
        m_currentInnerProgress = 0;
        if (m_progressDialog.get() != 0) {
            const ::vos::OGuard guard( Application::GetSolarMutex() );
            if (m_progressDialog->m_statusBar->IsProgressMode())
                m_progressDialog->m_statusBar->EndProgressMode();
            m_progressDialog->m_statusBar->StartProgressMode( text );
            m_progressDialog->m_ftCurrentAction->SetText( String() );
            updateProgress();
        }
    }
}

//______________________________________________________________________________
void ProgressCommandEnv::updateProgress( OUString const & text )
{
    //We pass a reference to this to keep the ProgrssCommandEnv alive until
    //asyncUpdateProgress was called.
    Application::PostUserEvent(
        LINK(this, ProgressCommandEnv, asyncUpdateProgress),
        new UpdateParams(
        Reference<XInterface>(static_cast<OWeakObject*>(this), UNO_QUERY_THROW), text));
 }

// This function may not call in any package manager functions. These use a mutex and
//then we have both the Solar and the package manager mutex locked here, which may
//cause deadlocks.
IMPL_LINK(ProgressCommandEnv, asyncUpdateProgress, UpdateParams*, pUpdateParams)
{
    try
    {
        if (m_progressDialog.get() != 0)
        {
    //        const ::vos::OGuard guard( Application::GetSolarMutex() );
            if (pUpdateParams->text.getLength() > 0)
                m_progressDialog->m_ftCurrentAction->SetText(pUpdateParams->text);
            // xxx todo: how to do better?
            m_progressDialog->m_statusBar->SetProgressValue(
                static_cast<USHORT>(
                    (((m_currentProgressSection - 1) +
                      (m_currentInnerProgress >= 20
                       ? 0.99 : (double)m_currentInnerProgress / 20.0)) * 100)
                    / m_progressSections ) % 101 );
    //             static_cast<USHORT>(
    //                 ((m_currentProgressSection +
    //                   (1.0 - (m_currentInnerProgress == 0
    //                           ? 1.0 : 1.0 / m_currentInnerProgress))) * 100)
    //                 / m_progressSections ) % 101 );
        }
    }
    catch (...)
    {
        delete pUpdateParams;
    }

    //Make sure we release the reference to NodeImpl
    delete pUpdateParams;

    return 0;
}

// XCommandEnvironment
//______________________________________________________________________________
Reference<task::XInteractionHandler> ProgressCommandEnv::getInteractionHandler()
    throw (RuntimeException)
{
    return this;
}

//______________________________________________________________________________
Reference<XProgressHandler> ProgressCommandEnv::getProgressHandler()
    throw (RuntimeException)
{
    return this;
}

// XInteractionHandler
//______________________________________________________________________________
void ProgressCommandEnv::handle(
    Reference<task::XInteractionRequest> const & xRequest )
    throw (RuntimeException)
{
    Any request( xRequest->getRequest() );
    OSL_ASSERT( request.getValueTypeClass() == TypeClass_EXCEPTION );
#if OSL_DEBUG_LEVEL > 1
    OSL_TRACE( "[dp_gui_cmdenv.cxx] incoming request:\n%s\n",
               ::rtl::OUStringToOString( ::comphelper::anyToString(request),
                                         RTL_TEXTENCODING_UTF8 ).getStr() );
#endif

    lang::WrappedTargetException wtExc;
    deployment::DependencyException depExc;
    deployment::LicenseException licExc;
    deployment::LicenseIndividualAgreementException licAgreementExc;
    deployment::VersionException verExc;
    deployment::InstallException instExc;

    // selections:
    bool approve = false;
    bool abort = false;

    if (request >>= wtExc) {
        // handable deployment error signalled, e.g.
        // bundle item registration failed, notify cause only:
        Any cause;
        deployment::DeploymentException dpExc;
        if (wtExc.TargetException >>= dpExc)
            cause = dpExc.Cause;
        else {
            CommandFailedException cfExc;
            if (wtExc.TargetException >>= cfExc)
                cause = cfExc.Reason;
            else
                cause = wtExc.TargetException;
        }
        update_( cause );

        // ignore intermediate errors of legacy packages, i.e.
        // former pkgchk behaviour:
        const Reference<deployment::XPackage> xPackage(
            wtExc.Context, UNO_QUERY );
        OSL_ASSERT( xPackage.is() );
        if (xPackage.is()) {
            const Reference<deployment::XPackageTypeInfo> xPackageType(
                xPackage->getPackageType() );
            OSL_ASSERT( xPackageType.is() );
            if (xPackageType.is()) {
                approve = (xPackage->isBundle() &&
                           xPackageType->getMediaType().matchAsciiL(
                               RTL_CONSTASCII_STRINGPARAM(
                                   "application/"
                                   "vnd.sun.star.legacy-package-bundle") ));
            }
        }
        abort = !approve;
    }
    else if (request >>= depExc)
    {
        std::vector< rtl::OUString > deps;
        for (sal_Int32 i = 0; i < depExc.UnsatisfiedDependencies.getLength();
             ++i)
        {
            deps.push_back(
                dp_misc::Dependencies::name(depExc.UnsatisfiedDependencies[i]));
        }
        {
            vos::OGuard guard(Application::GetSolarMutex());
            short n = DependencyDialog(activeDialog(), deps).Execute();
            // Distinguish between closing the dialog and programatically
            // canceling the dialog (headless VCL):
            approve = n == RET_OK
                || n == RET_CANCEL && !Application::IsDialogCancelEnabled();
        }
    }
    else if (request >>= licAgreementExc)
    {
        vos::OGuard aSolarGuard( Application::GetSolarMutex() );
        ResId warnId(WARNINGBOX_NOSHAREDALLOWED, *DeploymentGuiResMgr::get());
        WarningBox warn(activeDialog(), warnId);
        String msgText = warn.GetMessText();
        msgText.SearchAndReplaceAllAscii( "%PRODUCTNAME", BrandName::get() );
        msgText.SearchAndReplaceAllAscii("%NAME", licAgreementExc.ExtensionName);
        warn.SetMessText(msgText);
        warn.Execute();
        abort = true;
    }
    else if (request >>= licExc)
    {
        Reference<ui::dialogs::XExecutableDialog> xDialog(
            css::deployment::ui::LicenseDialog::create(
            m_xContext, VCLUnoHelper::GetInterface(activeDialog()), licExc.Text ) );
        sal_Int16 res = xDialog->execute();
        if (res == css::ui::dialogs::ExecutableDialogResults::CANCEL)
            abort = true;
        else if (res == css::ui::dialogs::ExecutableDialogResults::OK)
            approve = true;
        else
        {
            OSL_ASSERT(0);
        }
    }
    else if (request >>= verExc)
    {
        sal_uInt32 id;
        switch (dp_misc::comparePackageVersions(
                    verExc.New, verExc.Deployed))
        {
        case dp_misc::LESS:
            id = RID_QUERYBOX_VERSION_LESS;
            break;
        case dp_misc::EQUAL:
            id = RID_QUERYBOX_VERSION_EQUAL;
            break;
        default: // dp_misc::GREATER
            id = RID_QUERYBOX_VERSION_GREATER;
            break;
        }
        {
            vos::OGuard guard(Application::GetSolarMutex());
            InfoBox box(activeDialog(), ResId(id, *DeploymentGuiResMgr::get()));
            String s(box.GetMessText());
            s.SearchAndReplaceAllAscii(
                "$NAME", dp_misc::getIdentifier(verExc.New));
            s.SearchAndReplaceAllAscii("$NEW", getVersion(verExc.New));
            s.SearchAndReplaceAllAscii(
                "$DEPLOYED", getVersion(verExc.Deployed));
            box.SetMessText(s);
            approve = box.Execute() == RET_OK;
            abort = !approve;
        }
    }
    else if (request >>= instExc)
    {
        //Only if the unopgk was started with gui + extension then we ask the user
        if (!m_bAskWhenInstalling)
        {
            approve = true;
        }
        else
        {
            vos::OGuard guard(Application::GetSolarMutex());
            InfoBox box(activeDialog(), ResId(RID_QUERYBOX_INSTALL_EXTENSION, *DeploymentGuiResMgr::get()));
            String s(box.GetMessText());
            s.SearchAndReplaceAllAscii("%NAME", instExc.New->getDisplayName());
            box.SetMessText(s);
            approve = box.Execute() == RET_OK;
            abort = !approve;
        }
    }


    if (approve == false && abort == false)
    {
        // forward to UUI handler:
        if (! m_xHandler.is()) {
            // late init:
            Sequence<Any> handlerArgs( 1 );
            handlerArgs[ 0 ] <<= beans::PropertyValue(

                OUSTR("Context"), -1, Any(m_title),
                beans::PropertyState_DIRECT_VALUE );
             m_xHandler.set( m_xContext->getServiceManager()
                            ->createInstanceWithArgumentsAndContext(
                                OUSTR("com.sun.star.uui.InteractionHandler"),
                                handlerArgs, m_xContext ), UNO_QUERY_THROW );
        }
        m_xHandler->handle( xRequest );
    }
    else
    {
         // select:
        Sequence< Reference<task::XInteractionContinuation> > conts(
            xRequest->getContinuations() );
        Reference<task::XInteractionContinuation> const * pConts =
            conts.getConstArray();
        sal_Int32 len = conts.getLength();
        for ( sal_Int32 pos = 0; pos < len; ++pos )
        {
            if (approve) {
                Reference<task::XInteractionApprove> xInteractionApprove(
                    pConts[ pos ], UNO_QUERY );
                if (xInteractionApprove.is()) {
                    xInteractionApprove->select();
                    // don't query again for ongoing continuations:
                    approve = false;
                }
            }
            else if (abort) {
                Reference<task::XInteractionAbort> xInteractionAbort(
                    pConts[ pos ], UNO_QUERY );
                if (xInteractionAbort.is()) {
                    xInteractionAbort->select();
                    // don't query again for ongoing continuations:
                    abort = false;
                }
            }
        }
    }
}

// XProgressHandler
//______________________________________________________________________________
void ProgressCommandEnv::push( Any const & Status )
    throw (RuntimeException)
{
    update_( Status );
}

//______________________________________________________________________________
void ProgressCommandEnv::update_( Any const & Status )
    throw (RuntimeException)
{
    DialogImpl *mainDialog = dynamic_cast<dp_gui::DialogImpl*>(m_dialog);
    if (mainDialog == NULL)
        return;
    OUString text;
    if (Status.hasValue() && !(Status >>= text)) {
        if (Status.getValueTypeClass() == TypeClass_EXCEPTION)
            text = static_cast<Exception const *>(Status.getValue())->Message;
        if (text.getLength() == 0)
            text = ::comphelper::anyToString(Status); // fallback
        mainDialog->errbox( text );
    }
    updateProgress( text );
    ++m_currentInnerProgress;
}

//______________________________________________________________________________
void ProgressCommandEnv::update( Any const & Status )
    throw (RuntimeException)
{
    update_( Status );
}

//______________________________________________________________________________
void ProgressCommandEnv::pop() throw (RuntimeException)
{
    update_( Any() ); // no message
}

}

