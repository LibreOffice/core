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

#include <cstddef>

#include <com/sun/star/beans/NamedValue.hpp>

#include <com/sun/star/deployment/DependencyException.hpp>
#include <com/sun/star/deployment/LicenseException.hpp>
#include <com/sun/star/deployment/VersionException.hpp>
#include <com/sun/star/deployment/InstallException.hpp>
#include <com/sun/star/deployment/PlatformException.hpp>

#include <com/sun/star/deployment/ui/LicenseDialog.hpp>
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/deployment/UpdateInformationProvider.hpp>
#include <com/sun/star/deployment/XPackage.hpp>

#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/XAbortChannel.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>

#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>

#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/TypeClass.hpp>
#include <o3tl/any.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <salhelper/thread.hxx>
#include <ucbhelper/content.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/anytostring.hxx>
#include <vcl/weld.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include "dp_gui.h"
#include "dp_gui_extensioncmdqueue.hxx"
#include "dp_gui_dependencydialog.hxx"
#include "dp_gui_dialog2.hxx"
#include <dp_shared.hxx>
#include <strings.hrc>
#include "dp_gui_theextmgr.hxx"
#include "dp_gui_updatedialog.hxx"
#include "dp_gui_updateinstalldialog.hxx"
#include <dp_dependencies.hxx>
#include <dp_identifier.hxx>
#include <dp_version.hxx>

#include <queue>
#include <memory>

#ifdef _WIN32
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <objbase.h>
#endif


using namespace ::com::sun::star;

namespace {

OUString getVersion( OUString const & sVersion )
{
    return ( sVersion.isEmpty() ) ? OUString( "0" ) : sVersion;
}

OUString getVersion( const uno::Reference< deployment::XPackage > &rPackage )
{
    return getVersion( rPackage->getVersion());
}
}


namespace dp_gui {


class ProgressCmdEnv
    : public ::cppu::WeakImplHelper< ucb::XCommandEnvironment,
                                      task::XInteractionHandler,
                                      ucb::XProgressHandler >
{
    uno::Reference< task::XInteractionHandler2> m_xHandler;
    uno::Reference< uno::XComponentContext > m_xContext;

    DialogHelper*   m_pDialogHelper;
    OUString        m_sTitle;
    bool            m_bWarnUser;
    sal_Int32       m_nCurrentProgress;

    void updateProgress();

    /// @throws uno::RuntimeException
    void update_( uno::Any const & Status );

public:
    /** When param bAskWhenInstalling = true, then the user is asked if he
    agrees to install this extension. In case this extension is already installed
    then the user is also notified and asked if he wants to replace that existing
    extension. In first case an interaction request with an InstallException
    will be handled and in the second case a VersionException will be handled.
    */

    ProgressCmdEnv( const uno::Reference< uno::XComponentContext >& rContext,
                    DialogHelper* pDialogHelper,
                    const OUString& rTitle )
        : m_xContext( rContext )
        , m_pDialogHelper( pDialogHelper )
        , m_sTitle( rTitle )
        , m_bWarnUser( false )
        , m_nCurrentProgress(0)
        {}

    weld::Window* activeDialog() { return m_pDialogHelper ? m_pDialogHelper->getFrameWeld() : nullptr; }

    void startProgress();
    void stopProgress();
    void progressSection( const OUString &rText,
                          const uno::Reference< task::XAbortChannel > &xAbortChannel );
    void setWarnUser( bool bNewVal ) { m_bWarnUser = bNewVal; }

    // XCommandEnvironment
    virtual uno::Reference< task::XInteractionHandler > SAL_CALL getInteractionHandler() override;
    virtual uno::Reference< ucb::XProgressHandler > SAL_CALL getProgressHandler() override;

    // XInteractionHandler
    virtual void SAL_CALL handle( uno::Reference< task::XInteractionRequest > const & xRequest ) override;

    // XProgressHandler
    virtual void SAL_CALL push( uno::Any const & Status ) override;
    virtual void SAL_CALL update( uno::Any const & Status ) override;
    virtual void SAL_CALL pop() override;
};


struct ExtensionCmd
{
    enum E_CMD_TYPE { ADD, ENABLE, DISABLE, REMOVE, CHECK_FOR_UPDATES, ACCEPT_LICENSE };

    E_CMD_TYPE  m_eCmdType;
    bool        m_bWarnUser;
    OUString    m_sExtensionURL;
    OUString    m_sRepository;
    uno::Reference< deployment::XPackage > m_xPackage;
    std::vector< uno::Reference< deployment::XPackage > >        m_vExtensionList;

    ExtensionCmd( const E_CMD_TYPE eCommand,
                  const OUString &rExtensionURL,
                  const OUString &rRepository,
                  const bool bWarnUser )
        : m_eCmdType( eCommand ),
          m_bWarnUser( bWarnUser ),
          m_sExtensionURL( rExtensionURL ),
          m_sRepository( rRepository ) {};
    ExtensionCmd( const E_CMD_TYPE eCommand,
                  const uno::Reference< deployment::XPackage > &rPackage )
        : m_eCmdType( eCommand ),
          m_bWarnUser( false ),
          m_xPackage( rPackage ) {};
    ExtensionCmd( const E_CMD_TYPE eCommand,
                  const std::vector<uno::Reference<deployment::XPackage > > &vExtensionList )
        : m_eCmdType( eCommand ),
          m_bWarnUser( false ),
          m_vExtensionList( vExtensionList ) {};
};

typedef std::shared_ptr< ExtensionCmd > TExtensionCmd;


class ExtensionCmdQueue::Thread: public salhelper::Thread
{
public:
    Thread( DialogHelper *pDialogHelper,
            TheExtensionManager *pManager,
            const uno::Reference< uno::XComponentContext > & rContext );

    void addExtension( const OUString &rExtensionURL,
                       const OUString &rRepository,
                       const bool bWarnUser );
    void removeExtension( const uno::Reference< deployment::XPackage > &rPackage );
    void enableExtension( const uno::Reference< deployment::XPackage > &rPackage,
                          const bool bEnable );
    void checkForUpdates( const std::vector<uno::Reference<deployment::XPackage > > &vExtensionList );
    void acceptLicense( const uno::Reference< deployment::XPackage > &rPackage );
    void stop();
    bool isBusy();

private:
    virtual ~Thread() override;

    virtual void execute() override;

    void _insert(const TExtensionCmd& rExtCmd);

    void _addExtension( ::rtl::Reference< ProgressCmdEnv > const &rCmdEnv,
                        const OUString &rPackageURL,
                        const OUString &rRepository,
                        const bool bWarnUser );
    void _removeExtension( ::rtl::Reference< ProgressCmdEnv > const &rCmdEnv,
                           const uno::Reference< deployment::XPackage > &xPackage );
    void _enableExtension( ::rtl::Reference< ProgressCmdEnv > const &rCmdEnv,
                           const uno::Reference< deployment::XPackage > &xPackage );
    void _disableExtension( ::rtl::Reference< ProgressCmdEnv > const &rCmdEnv,
                            const uno::Reference< deployment::XPackage > &xPackage );
    void _checkForUpdates( const std::vector<uno::Reference<deployment::XPackage > > &vExtensionList );
    void _acceptLicense( ::rtl::Reference< ProgressCmdEnv > const &rCmdEnv,
                           const uno::Reference< deployment::XPackage > &xPackage );

    enum Input { NONE, START, STOP };

    uno::Reference< uno::XComponentContext > m_xContext;
    std::queue< TExtensionCmd >              m_queue;

    DialogHelper *m_pDialogHelper;
    TheExtensionManager *m_pManager;

    const OUString   m_sEnablingPackages;
    const OUString   m_sDisablingPackages;
    const OUString   m_sAddingPackages;
    const OUString   m_sRemovingPackages;
    const OUString   m_sDefaultCmd;
    const OUString   m_sAcceptLicense;
    osl::Condition   m_wakeup;
    osl::Mutex       m_mutex;
    Input            m_eInput;
    bool             m_bStopped;
    bool             m_bWorking;
};


void ProgressCmdEnv::startProgress()
{
    m_nCurrentProgress = 0;

    if ( m_pDialogHelper )
        m_pDialogHelper->showProgress( true );
}


void ProgressCmdEnv::stopProgress()
{
    if ( m_pDialogHelper )
        m_pDialogHelper->showProgress( false );
}


void ProgressCmdEnv::progressSection( const OUString &rText,
                                      const uno::Reference< task::XAbortChannel > &xAbortChannel )
{
    m_nCurrentProgress = 0;
    if ( m_pDialogHelper )
    {
        m_pDialogHelper->updateProgress( rText, xAbortChannel );
        m_pDialogHelper->updateProgress( 5 );
    }
}


void ProgressCmdEnv::updateProgress()
{
    long nProgress = ((m_nCurrentProgress*5) % 100) + 5;
    if ( m_pDialogHelper )
        m_pDialogHelper->updateProgress( nProgress );
}

// XCommandEnvironment

uno::Reference< task::XInteractionHandler > ProgressCmdEnv::getInteractionHandler()
{
    return this;
}


uno::Reference< ucb::XProgressHandler > ProgressCmdEnv::getProgressHandler()
{
    return this;
}


// XInteractionHandler

void ProgressCmdEnv::handle( uno::Reference< task::XInteractionRequest > const & xRequest )
{
    uno::Any request( xRequest->getRequest() );
    OSL_ASSERT( request.getValueTypeClass() == uno::TypeClass_EXCEPTION );
    dp_misc::TRACE( "[dp_gui_cmdenv.cxx] incoming request:\n"
        + ::comphelper::anyToString(request) + "\n");

    lang::WrappedTargetException wtExc;
    deployment::DependencyException depExc;
    deployment::LicenseException licExc;
    deployment::VersionException verExc;
    deployment::InstallException instExc;
    deployment::PlatformException platExc;

    // selections:
    bool approve = false;
    bool abort = false;

    if (request >>= wtExc) {
        // handable deployment error signalled, e.g.
        // bundle item registration failed, notify cause only:
        uno::Any cause;
        deployment::DeploymentException dpExc;
        if (wtExc.TargetException >>= dpExc)
            cause = dpExc.Cause;
        else {
            ucb::CommandFailedException cfExc;
            if (wtExc.TargetException >>= cfExc)
                cause = cfExc.Reason;
            else
                cause = wtExc.TargetException;
        }
        update_( cause );

        // ignore intermediate errors of legacy packages, i.e.
        // former pkgchk behaviour:
        const uno::Reference< deployment::XPackage > xPackage( wtExc.Context, uno::UNO_QUERY );
        OSL_ASSERT( xPackage.is() );
        if ( xPackage.is() )
        {
            const uno::Reference< deployment::XPackageTypeInfo > xPackageType( xPackage->getPackageType() );
            OSL_ASSERT( xPackageType.is() );
            if (xPackageType.is())
            {
                approve = ( xPackage->isBundle() &&
                            xPackageType->getMediaType().match(
                               "application/vnd.sun.star.legacy-package-bundle" ));
            }
        }
        abort = !approve;
    }
    else if (request >>= depExc)
    {
        std::vector< OUString > deps;
        deps.reserve(depExc.UnsatisfiedDependencies.getLength());
        for (sal_Int32 i = 0; i < depExc.UnsatisfiedDependencies.getLength(); ++i)
        {
            deps.push_back(
                dp_misc::Dependencies::getErrorText( depExc.UnsatisfiedDependencies[i]) );
        }
        {
            SolarMutexGuard guard;
            DependencyDialog aDlg(m_pDialogHelper ? m_pDialogHelper->getFrameWeld() : nullptr, deps);
            short n = aDlg.run();
            // Distinguish between closing the dialog and programmatically
            // canceling the dialog (headless VCL):
            approve = n == RET_OK
                || (n == RET_CANCEL && !Application::IsDialogCancelEnabled());
        }
    }
    else if (request >>= licExc)
    {
        SolarMutexGuard guard;
        uno::Reference< ui::dialogs::XExecutableDialog > xDialog(
            deployment::ui::LicenseDialog::create(
            m_xContext, VCLUnoHelper::GetInterface( m_pDialogHelper? m_pDialogHelper->getWindow() : nullptr ),
            licExc.ExtensionName, licExc.Text ) );
        sal_Int16 res = xDialog->execute();
        if ( res == ui::dialogs::ExecutableDialogResults::CANCEL )
            abort = true;
        else if ( res == ui::dialogs::ExecutableDialogResults::OK )
            approve = true;
        else
        {
            OSL_ASSERT(false);
        }
    }
    else if (request >>= verExc)
    {
        const char* id;
        switch (dp_misc::compareVersions(
                    verExc.NewVersion, verExc.Deployed->getVersion() ))
        {
        case dp_misc::LESS:
            id = RID_STR_WARNING_VERSION_LESS;
            break;
        case dp_misc::EQUAL:
            id = RID_STR_WARNING_VERSION_EQUAL;
            break;
        default: // dp_misc::GREATER
            id = RID_STR_WARNING_VERSION_GREATER;
            break;
        }
        OSL_ASSERT( verExc.Deployed.is() );
        bool bEqualNames = verExc.NewDisplayName ==
            verExc.Deployed->getDisplayName();
        {
            SolarMutexGuard guard;
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_pDialogHelper ? m_pDialogHelper->getFrameWeld() : nullptr,
                                                      VclMessageType::Warning, VclButtonsType::OkCancel, DpResId(id)));
            OUString s;
            if (bEqualNames)
            {
                s = xBox->get_primary_text();
            }
            else if (!strcmp(id, RID_STR_WARNING_VERSION_EQUAL))
            {
                //hypothetical: requires two instances of an extension with the same
                //version to have different display names. Probably the developer forgot
                //to change the version.
                s = DpResId(RID_STR_WARNINGBOX_VERSION_EQUAL_DIFFERENT_NAMES);
            }
            else if (!strcmp(id, RID_STR_WARNING_VERSION_LESS))
            {
                s = DpResId(RID_STR_WARNINGBOX_VERSION_LESS_DIFFERENT_NAMES);
            }
            else if (!strcmp(id, RID_STR_WARNING_VERSION_GREATER))
            {
               s = DpResId(RID_STR_WARNINGBOX_VERSION_GREATER_DIFFERENT_NAMES);
            }
            s = s.replaceAll("$NAME", verExc.NewDisplayName);
            s = s.replaceAll("$OLDNAME", verExc.Deployed->getDisplayName());
            s = s.replaceAll("$NEW", getVersion(verExc.NewVersion));
            s = s.replaceAll("$DEPLOYED", getVersion(verExc.Deployed));
            xBox->set_primary_text(s);
            approve = xBox->run() == RET_OK;
            abort = !approve;
        }
    }
    else if (request >>= instExc)
    {
        if ( ! m_bWarnUser )
        {
            approve = true;
        }
        else
        {
            if ( m_pDialogHelper )
            {
                SolarMutexGuard guard;

                approve = m_pDialogHelper->installExtensionWarn( instExc.displayName );
            }
            else
                approve = false;
            abort = !approve;
        }
    }
    else if (request >>= platExc)
    {
        SolarMutexGuard guard;
        OUString sMsg(DpResId(RID_STR_UNSUPPORTED_PLATFORM));
        sMsg = sMsg.replaceAll("%Name", platExc.package->getDisplayName());
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_pDialogHelper ? m_pDialogHelper->getFrameWeld() : nullptr,
                                                  VclMessageType::Warning, VclButtonsType::Ok, sMsg));
        xBox->run();
        approve = true;
    }

    if (!approve && !abort)
    {
        // forward to UUI handler:
        if (! m_xHandler.is()) {
            // late init:
            m_xHandler = task::InteractionHandler::createWithParentAndContext(m_xContext, nullptr, m_sTitle);
        }
        m_xHandler->handle( xRequest );
    }
    else
    {
        // select:
        uno::Sequence< uno::Reference< task::XInteractionContinuation > > conts(
            xRequest->getContinuations() );
        uno::Reference< task::XInteractionContinuation > const * pConts = conts.getConstArray();
        sal_Int32 len = conts.getLength();
        for ( sal_Int32 pos = 0; pos < len; ++pos )
        {
            if (approve) {
                uno::Reference< task::XInteractionApprove > xInteractionApprove( pConts[ pos ], uno::UNO_QUERY );
                if (xInteractionApprove.is()) {
                    xInteractionApprove->select();
                    // don't query again for ongoing continuations:
                    approve = false;
                }
            }
            else if (abort) {
                uno::Reference< task::XInteractionAbort > xInteractionAbort( pConts[ pos ], uno::UNO_QUERY );
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

void ProgressCmdEnv::push( uno::Any const & rStatus )
{
    update_( rStatus );
}


void ProgressCmdEnv::update_( uno::Any const & rStatus )
{
    OUString text;
    if ( rStatus.hasValue() && !( rStatus >>= text) )
    {
        if ( auto e = o3tl::tryAccess<uno::Exception>(rStatus) )
            text = e->Message;
        if ( text.isEmpty() )
            text = ::comphelper::anyToString( rStatus ); // fallback

        const SolarMutexGuard aGuard;
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_pDialogHelper ? m_pDialogHelper->getFrameWeld() : nullptr,
                                                  VclMessageType::Warning, VclButtonsType::Ok, text));
        xBox->run();
    }
    ++m_nCurrentProgress;
    updateProgress();
}


void ProgressCmdEnv::update( uno::Any const & rStatus )
{
    update_( rStatus );
}


void ProgressCmdEnv::pop()
{
    update_( uno::Any() ); // no message
}


ExtensionCmdQueue::Thread::Thread( DialogHelper *pDialogHelper,
                                   TheExtensionManager *pManager,
                                   const uno::Reference< uno::XComponentContext > & rContext ) :
    salhelper::Thread( "dp_gui_extensioncmdqueue" ),
    m_xContext( rContext ),
    m_pDialogHelper( pDialogHelper ),
    m_pManager( pManager ),
    m_sEnablingPackages( DpResId( RID_STR_ENABLING_PACKAGES ) ),
    m_sDisablingPackages( DpResId( RID_STR_DISABLING_PACKAGES ) ),
    m_sAddingPackages( DpResId( RID_STR_ADDING_PACKAGES ) ),
    m_sRemovingPackages( DpResId( RID_STR_REMOVING_PACKAGES ) ),
    m_sDefaultCmd( DpResId( RID_STR_ADD_PACKAGES ) ),
    m_sAcceptLicense( DpResId( RID_STR_ACCEPT_LICENSE ) ),
    m_eInput( NONE ),
    m_bStopped( false ),
    m_bWorking( false )
{
    OSL_ASSERT( pDialogHelper );
}


void ExtensionCmdQueue::Thread::addExtension( const OUString &rExtensionURL,
                                              const OUString &rRepository,
                                              const bool bWarnUser )
{
    if ( !rExtensionURL.isEmpty() )
    {
        TExtensionCmd pEntry( new ExtensionCmd( ExtensionCmd::ADD, rExtensionURL, rRepository, bWarnUser ) );
        _insert( pEntry );
    }
}


void ExtensionCmdQueue::Thread::removeExtension( const uno::Reference< deployment::XPackage > &rPackage )
{
    if ( rPackage.is() )
    {
        TExtensionCmd pEntry( new ExtensionCmd( ExtensionCmd::REMOVE, rPackage ) );
        _insert( pEntry );
    }
}


void ExtensionCmdQueue::Thread::acceptLicense( const uno::Reference< deployment::XPackage > &rPackage )
{
    if ( rPackage.is() )
    {
        TExtensionCmd pEntry( new ExtensionCmd( ExtensionCmd::ACCEPT_LICENSE, rPackage ) );
        _insert( pEntry );
    }
}


void ExtensionCmdQueue::Thread::enableExtension( const uno::Reference< deployment::XPackage > &rPackage,
                                                 const bool bEnable )
{
    if ( rPackage.is() )
    {
        TExtensionCmd pEntry( new ExtensionCmd( bEnable ? ExtensionCmd::ENABLE :
                                                          ExtensionCmd::DISABLE,
                                                rPackage ) );
        _insert( pEntry );
    }
}


void ExtensionCmdQueue::Thread::checkForUpdates(
    const std::vector<uno::Reference<deployment::XPackage > > &vExtensionList )
{
    TExtensionCmd pEntry( new ExtensionCmd( ExtensionCmd::CHECK_FOR_UPDATES, vExtensionList ) );
    _insert( pEntry );
}


//Stopping this thread will not abort the installation of extensions.
void ExtensionCmdQueue::Thread::stop()
{
    osl::MutexGuard aGuard( m_mutex );
    m_bStopped = true;
    m_eInput = STOP;
    m_wakeup.set();
}


bool ExtensionCmdQueue::Thread::isBusy()
{
    osl::MutexGuard aGuard( m_mutex );
    return m_bWorking;
}


ExtensionCmdQueue::Thread::~Thread() {}


void ExtensionCmdQueue::Thread::execute()
{
#ifdef _WIN32
    //Needed for use of the service "com.sun.star.system.SystemShellExecute" in
    //DialogHelper::openWebBrowser
    CoUninitialize();
    (void) CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
#endif
    for (;;)
    {
        if ( m_wakeup.wait() != osl::Condition::result_ok )
        {
            dp_misc::TRACE( "dp_gui::ExtensionCmdQueue::Thread::run: ignored "
                       "osl::Condition::wait failure\n" );
        }
        m_wakeup.reset();

        int nSize;
        Input eInput;
        {
            osl::MutexGuard aGuard( m_mutex );
            eInput = m_eInput;
            m_eInput = NONE;
            nSize = m_queue.size();
            m_bWorking = false;
        }

        // If this thread has been woken up by anything else except start, stop
        // then input is NONE and we wait again.
        // We only install the extension which are currently in the queue.
        // The progressbar will be set to show the progress of the current number
        // of extensions. If we allowed to add extensions now then the progressbar may
        // have reached the end while we still install newly added extensions.
        if ( ( eInput == NONE ) || ( nSize == 0 ) )
            continue;
        if ( eInput == STOP )
            break;

        ::rtl::Reference< ProgressCmdEnv > currentCmdEnv( new ProgressCmdEnv( m_xContext, m_pDialogHelper, m_sDefaultCmd ) );

        // Do not lock the following part with addExtension. addExtension may be called in the main thread.
        // If the message box "Do you want to install the extension (or similar)" is shown and then
        // addExtension is called, which then blocks the main thread, then we deadlock.
        bool bStartProgress = true;

        while ( --nSize >= 0 )
        {
            {
                osl::MutexGuard aGuard( m_mutex );
                m_bWorking = true;
            }

            try
            {
                TExtensionCmd pEntry;
                {
                    ::osl::MutexGuard queueGuard( m_mutex );
                    pEntry = m_queue.front();
                    m_queue.pop();
                }

                if ( bStartProgress && ( pEntry->m_eCmdType != ExtensionCmd::CHECK_FOR_UPDATES ) )
                {
                    currentCmdEnv->startProgress();
                    bStartProgress = false;
                }

                switch ( pEntry->m_eCmdType ) {
                case ExtensionCmd::ADD :
                    _addExtension( currentCmdEnv, pEntry->m_sExtensionURL, pEntry->m_sRepository, pEntry->m_bWarnUser );
                    break;
                case ExtensionCmd::REMOVE :
                    _removeExtension( currentCmdEnv, pEntry->m_xPackage );
                    break;
                case ExtensionCmd::ENABLE :
                    _enableExtension( currentCmdEnv, pEntry->m_xPackage );
                    break;
                case ExtensionCmd::DISABLE :
                    _disableExtension( currentCmdEnv, pEntry->m_xPackage );
                    break;
                case ExtensionCmd::CHECK_FOR_UPDATES :
                    _checkForUpdates( pEntry->m_vExtensionList );
                    break;
                case ExtensionCmd::ACCEPT_LICENSE :
                    _acceptLicense( currentCmdEnv, pEntry->m_xPackage );
                    break;
                }
            }
            catch ( const ucb::CommandAbortedException & )
            {
                //This exception is thrown when the user clicks cancel on the progressbar.
                //Then we cancel the installation of all extensions and remove them from
                //the queue.
                {
                    ::osl::MutexGuard queueGuard2(m_mutex);
                    while ( --nSize >= 0 )
                        m_queue.pop();
                }
                break;
            }
            catch ( const ucb::CommandFailedException & )
            {
                //This exception is thrown when a user clicked cancel in the messagebox which was
                //started by the interaction handler. For example the user will be asked if he/she
                //really wants to install the extension.
                //These interaction are run for exactly one extension at a time. Therefore we continue
                //with installing the remaining extensions.
                continue;
            }
            catch ( const uno::Exception & )
            {
                //Todo display the user an error
                //see also DialogImpl::SyncPushButton::Click()
                uno::Any exc( ::cppu::getCaughtException() );
                OUString msg;
                deployment::DeploymentException dpExc;
                if (exc >>= dpExc)
                {
                    if (auto e = o3tl::tryAccess<uno::Exception>(dpExc.Cause))
                    {
                        // notify error cause only:
                        msg = e->Message;
                    }
                }
                if (msg.isEmpty()) // fallback for debugging purposes
                    msg = ::comphelper::anyToString(exc);

                const SolarMutexGuard guard;
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(currentCmdEnv->activeDialog(),
                                                          VclMessageType::Warning, VclButtonsType::Ok, msg));
                if (m_pDialogHelper)
                    xBox->set_title(m_pDialogHelper->getWindow()->GetText());
                xBox->run();
                    //Continue with installation of the remaining extensions
            }
            {
                osl::MutexGuard aGuard( m_mutex );
                m_bWorking = false;
            }
        }

        {
            // when leaving the while loop with break, we should set working to false, too
            osl::MutexGuard aGuard( m_mutex );
            m_bWorking = false;
        }

        if ( !bStartProgress )
            currentCmdEnv->stopProgress();
    }
    //end for
#ifdef _WIN32
    CoUninitialize();
#endif
}


void ExtensionCmdQueue::Thread::_addExtension( ::rtl::Reference< ProgressCmdEnv > const &rCmdEnv,
                                               const OUString &rPackageURL,
                                               const OUString &rRepository,
                                               const bool bWarnUser )
{
    //check if we have a string in anyTitle. For example "unopkg gui \" caused anyTitle to be void
    //and anyTitle.get<OUString> throws as RuntimeException.
    uno::Any anyTitle;
    try
    {
        anyTitle = ::ucbhelper::Content( rPackageURL, rCmdEnv.get(), m_xContext ).getPropertyValue( "Title" );
    }
    catch ( const uno::Exception & )
    {
        return;
    }

    OUString sName;
    if ( ! (anyTitle >>= sName) )
    {
        OSL_FAIL("Could not get file name for extension.");
        return;
    }

    rCmdEnv->setWarnUser( bWarnUser );
    uno::Reference< deployment::XExtensionManager > xExtMgr = m_pManager->getExtensionManager();
    uno::Reference< task::XAbortChannel > xAbortChannel( xExtMgr->createAbortChannel() );
    OUString sTitle(
        m_sAddingPackages.replaceAll("%EXTENSION_NAME", sName));
    rCmdEnv->progressSection( sTitle, xAbortChannel );

    try
    {
        xExtMgr->addExtension(rPackageURL, uno::Sequence<beans::NamedValue>(),
                              rRepository, xAbortChannel, rCmdEnv.get() );
    }
    catch ( const ucb::CommandFailedException & )
    {
        // When the extension is already installed we'll get a dialog asking if we want to overwrite. If we then press
        // cancel this exception is thrown.
    }
    catch ( const ucb::CommandAbortedException & )
    {
        // User clicked the cancel button
        // TODO: handle cancel
    }
    rCmdEnv->setWarnUser( false );
}


void ExtensionCmdQueue::Thread::_removeExtension( ::rtl::Reference< ProgressCmdEnv > const &rCmdEnv,
                                                  const uno::Reference< deployment::XPackage > &xPackage )
{
    uno::Reference< deployment::XExtensionManager > xExtMgr = m_pManager->getExtensionManager();
    uno::Reference< task::XAbortChannel > xAbortChannel( xExtMgr->createAbortChannel() );
    OUString sTitle(
        m_sRemovingPackages.replaceAll("%EXTENSION_NAME",
            xPackage->getDisplayName()));
    rCmdEnv->progressSection( sTitle, xAbortChannel );

    OUString id( dp_misc::getIdentifier( xPackage ) );
    try
    {
        xExtMgr->removeExtension( id, xPackage->getName(), xPackage->getRepositoryName(), xAbortChannel, rCmdEnv.get() );
    }
    catch ( const deployment::DeploymentException & )
    {}
    catch ( const ucb::CommandFailedException & )
    {}
    catch ( const ucb::CommandAbortedException & )
    {}

    // Check, if there are still updates to be notified via menu bar icon
    uno::Sequence< uno::Sequence< OUString > > aItemList;
    UpdateDialog::createNotifyJob( false, aItemList );
}


void ExtensionCmdQueue::Thread::_checkForUpdates(
    const std::vector<uno::Reference<deployment::XPackage > > &vExtensionList )
{
    const SolarMutexGuard guard;

    std::vector< UpdateData > vData;
    UpdateDialog aUpdateDialog(m_xContext, m_pDialogHelper? m_pDialogHelper->getFrameWeld() : nullptr, vExtensionList, &vData);

    aUpdateDialog.notifyMenubar( true, false ); // prepare the checking, if there updates to be notified via menu bar icon

    if (aUpdateDialog.run() == RET_OK && !vData.empty())
    {
        // If there is at least one directly downloadable extension then we
        // open the install dialog.
        std::vector< UpdateData > dataDownload;

        for (auto const& data : vData)
        {
            if ( data.sWebsiteURL.isEmpty() )
                dataDownload.push_back(data);
        }

        short nDialogResult = RET_OK;
        if ( !dataDownload.empty() )
        {
            UpdateInstallDialog aDlg(m_pDialogHelper? m_pDialogHelper->getFrameWeld() : nullptr, dataDownload, m_xContext);
            nDialogResult = aDlg.run();
            aUpdateDialog.notifyMenubar( false, true ); // Check, if there are still pending updates to be notified via menu bar icon
        }
        else
            aUpdateDialog.notifyMenubar( false, false ); // Check, if there are pending updates to be notified via menu bar icon

        //Now start the webbrowser and navigate to the websites where we get the updates
        if ( RET_OK == nDialogResult )
        {
            for (auto const& data : vData)
            {
                if ( m_pDialogHelper && ( !data.sWebsiteURL.isEmpty() ) )
                    m_pDialogHelper->openWebBrowser( data.sWebsiteURL, m_pDialogHelper->getWindow()->GetText() );
            }
        }
    }
    else
        aUpdateDialog.notifyMenubar( false, false ); // check if there updates to be notified via menu bar icon
}


void ExtensionCmdQueue::Thread::_enableExtension( ::rtl::Reference< ProgressCmdEnv > const &rCmdEnv,
                                                  const uno::Reference< deployment::XPackage > &xPackage )
{
    if ( !xPackage.is() )
        return;

    uno::Reference< deployment::XExtensionManager > xExtMgr = m_pManager->getExtensionManager();
    uno::Reference< task::XAbortChannel > xAbortChannel( xExtMgr->createAbortChannel() );
    OUString sTitle(
        m_sEnablingPackages.replaceAll("%EXTENSION_NAME",
            xPackage->getDisplayName()));
    rCmdEnv->progressSection( sTitle, xAbortChannel );

    try
    {
        xExtMgr->enableExtension( xPackage, xAbortChannel, rCmdEnv.get() );
        if ( m_pDialogHelper )
            m_pDialogHelper->updatePackageInfo( xPackage );
    }
    catch ( const ::ucb::CommandAbortedException & )
    {}
}


void ExtensionCmdQueue::Thread::_disableExtension( ::rtl::Reference< ProgressCmdEnv > const &rCmdEnv,
                                                   const uno::Reference< deployment::XPackage > &xPackage )
{
    if ( !xPackage.is() )
        return;

    uno::Reference< deployment::XExtensionManager > xExtMgr = m_pManager->getExtensionManager();
    uno::Reference< task::XAbortChannel > xAbortChannel( xExtMgr->createAbortChannel() );
    OUString sTitle(
        m_sDisablingPackages.replaceAll("%EXTENSION_NAME",
            xPackage->getDisplayName()));
    rCmdEnv->progressSection( sTitle, xAbortChannel );

    try
    {
        xExtMgr->disableExtension( xPackage, xAbortChannel, rCmdEnv.get() );
        if ( m_pDialogHelper )
            m_pDialogHelper->updatePackageInfo( xPackage );
    }
    catch ( const ::ucb::CommandAbortedException & )
    {}
}


void ExtensionCmdQueue::Thread::_acceptLicense( ::rtl::Reference< ProgressCmdEnv > const &rCmdEnv,
                                                const uno::Reference< deployment::XPackage > &xPackage )
{
    if ( !xPackage.is() )
        return;

    uno::Reference< deployment::XExtensionManager > xExtMgr = m_pManager->getExtensionManager();
    uno::Reference< task::XAbortChannel > xAbortChannel( xExtMgr->createAbortChannel() );
    OUString sTitle(
        m_sAcceptLicense.replaceAll("%EXTENSION_NAME",
            xPackage->getDisplayName()));
    rCmdEnv->progressSection( sTitle, xAbortChannel );

    try
    {
        xExtMgr->checkPrerequisitesAndEnable( xPackage, xAbortChannel, rCmdEnv.get() );
        if ( m_pDialogHelper )
            m_pDialogHelper->updatePackageInfo( xPackage );
    }
    catch ( const ::ucb::CommandAbortedException & )
    {}
}

void ExtensionCmdQueue::Thread::_insert(const TExtensionCmd& rExtCmd)
{
    ::osl::MutexGuard aGuard( m_mutex );

    // If someone called stop then we do not process the command -> game over!
    if ( m_bStopped )
        return;

    m_queue.push( rExtCmd );
    m_eInput = START;
    m_wakeup.set();
}


ExtensionCmdQueue::ExtensionCmdQueue( DialogHelper * pDialogHelper,
                                      TheExtensionManager *pManager,
                                      const uno::Reference< uno::XComponentContext > &rContext )
  : m_thread( new Thread( pDialogHelper, pManager, rContext ) )
{
    m_thread->launch();
}

ExtensionCmdQueue::~ExtensionCmdQueue() {
    stop();
}

void ExtensionCmdQueue::addExtension( const OUString & extensionURL,
                                      const OUString & repository,
                                      const bool bWarnUser )
{
    m_thread->addExtension( extensionURL, repository, bWarnUser );
}

void ExtensionCmdQueue::removeExtension( const uno::Reference< deployment::XPackage > &rPackage )
{
    m_thread->removeExtension( rPackage );
}

void ExtensionCmdQueue::enableExtension( const uno::Reference< deployment::XPackage > &rPackage,
                                         const bool bEnable )
{
    m_thread->enableExtension( rPackage, bEnable );
}

void ExtensionCmdQueue::checkForUpdates( const std::vector<uno::Reference<deployment::XPackage > > &vExtensionList )
{
    m_thread->checkForUpdates( vExtensionList );
}

void ExtensionCmdQueue::acceptLicense( const uno::Reference< deployment::XPackage > &rPackage )
{
    m_thread->acceptLicense( rPackage );
}

void ExtensionCmdQueue::syncRepositories( const uno::Reference< uno::XComponentContext > &xContext )
{
    dp_misc::syncRepositories( false, new ProgressCmdEnv( xContext, nullptr, "Extension Manager" ) );
}

void ExtensionCmdQueue::stop()
{
    m_thread->stop();
}

bool ExtensionCmdQueue::isBusy()
{
    return m_thread->isBusy();
}

void handleInteractionRequest( const uno::Reference< uno::XComponentContext > & xContext,
                               const uno::Reference< task::XInteractionRequest > & xRequest )
{
    ::rtl::Reference< ProgressCmdEnv > xCmdEnv( new ProgressCmdEnv( xContext, nullptr, "Extension Manager" ) );
    xCmdEnv->handle( xRequest );
}

} //namespace dp_gui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
