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

#include <strings.hrc>
#include <helpids.h>
#include <svtools/controldims.hxx>

#include "dp_gui.h"
#include "dp_gui_dialog2.hxx"
#include "dp_gui_extlistbox.hxx"
#include <dp_shared.hxx>
#include "dp_gui_theextmgr.hxx"
#include "dp_gui_extensioncmdqueue.hxx"
#include <dp_misc.h>
#include <dp_ucb.h>
#include <dp_update.hxx>
#include <dp_identifier.hxx>
#include <dp_descriptioninfoset.hxx>

#include <fpicker/strings.hrc>

#include <vcl/ctrl.hxx>
#include <vcl/menu.hxx>
#include <vcl/layout.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/svapp.hxx>
#include <vcl/builderfactory.hxx>

#include <osl/mutex.hxx>
#include <sal/log.hxx>

#include <svtools/extensionlistbox.hxx>
#include <svtools/restartdialog.hxx>

#include <sfx2/filedlghelper.hxx>
#include <sfx2/sfxdlg.hxx>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/bootstrap.hxx>

#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>
#include <ucbhelper/content.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/configmgr.hxx>

#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>

#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/FilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <officecfg/Office/ExtensionManager.hxx>

#include <map>
#include <memory>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::system;


namespace dp_gui {

#define USER_PACKAGE_MANAGER    "user"
#define SHARED_PACKAGE_MANAGER  "shared"
#define BUNDLED_PACKAGE_MANAGER "bundled"


struct StrAllFiles : public rtl::StaticWithInit< OUString, StrAllFiles >
{
    const OUString operator () () {
        const SolarMutexGuard guard;
        std::locale loc = Translate::Create("fps");
        return Translate::get(STR_FILTERNAME_ALL, loc);
    }
};

//                            ExtBoxWithBtns_Impl

enum MENU_COMMAND
{
    CMD_NONE    = 0,
    CMD_REMOVE  = 1,
    CMD_ENABLE,
    CMD_DISABLE,
    CMD_UPDATE,
    CMD_SHOW_LICENSE
};

class ExtBoxWithBtns_Impl : public ExtensionBox_Impl
{
    bool            m_bInterfaceLocked;

    VclPtr<ExtMgrDialog>   m_pParent;

    void            SetButtonStatus( const TEntry_Impl& rEntry );
    MENU_COMMAND    ShowPopupMenu( const Point &rPos, const long nPos );

public:
    explicit ExtBoxWithBtns_Impl(vcl::Window* pParent);
    virtual ~ExtBoxWithBtns_Impl() override;
    virtual void dispose() override;

    void InitFromDialog(ExtMgrDialog *pParentDialog);

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;

    virtual void    RecalcAll() override;
    virtual void    selectEntry( const long nPos ) override;

    void            enableButtons( bool bEnable );
};

ExtBoxWithBtns_Impl::ExtBoxWithBtns_Impl(vcl::Window* pParent)
    : ExtensionBox_Impl(pParent)
    , m_bInterfaceLocked(false)
    , m_pParent(nullptr)
{
}

void ExtBoxWithBtns_Impl::InitFromDialog(ExtMgrDialog *pParentDialog)
{
    setExtensionManager(pParentDialog->getExtensionManager());

    m_pParent = pParentDialog;
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeExtBoxWithBtns(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap &)
{
    rRet = VclPtr<ExtBoxWithBtns_Impl>::Create(pParent);
}

ExtBoxWithBtns_Impl::~ExtBoxWithBtns_Impl()
{
    disposeOnce();
}

void ExtBoxWithBtns_Impl::dispose()
{
    m_pParent.clear();
    ExtensionBox_Impl::dispose();
}


void ExtBoxWithBtns_Impl::RecalcAll()
{
    const sal_Int32 nActive = getSelIndex();

    if ( nActive != svt::IExtensionListBox::ENTRY_NOTFOUND )
    {
        SetButtonStatus( GetEntryData( nActive) );
    }
    else
    {
        m_pParent->enableOptionsButton( false );
        m_pParent->enableRemoveButton( false );
        m_pParent->enableEnableButton( false );
    }

    ExtensionBox_Impl::RecalcAll();
}


//This function may be called with nPos < 0
void ExtBoxWithBtns_Impl::selectEntry( const long nPos )
{
    if ( HasActive() && ( nPos == getSelIndex() ) )
        return;

    ExtensionBox_Impl::selectEntry( nPos );
}

void ExtBoxWithBtns_Impl::SetButtonStatus(const TEntry_Impl& rEntry)
{
    bool bShowOptionBtn = true;

    rEntry->m_bHasButtons = false;
    if ( ( rEntry->m_eState == REGISTERED ) || ( rEntry->m_eState == NOT_AVAILABLE ) )
    {
        m_pParent->enableButtontoEnable( false );
    }
    else
    {
        m_pParent->enableButtontoEnable( true );
        bShowOptionBtn = false;
    }

    if ( ( !rEntry->m_bUser || ( rEntry->m_eState == NOT_AVAILABLE ) || rEntry->m_bMissingDeps )
         && !rEntry->m_bMissingLic )
    {
        m_pParent->enableEnableButton( false );
    }
    else
    {
        m_pParent->enableEnableButton( !rEntry->m_bLocked );
        rEntry->m_bHasButtons = true;
    }

    if ( rEntry->m_bHasOptions && bShowOptionBtn )
    {
        m_pParent->enableOptionsButton( true );
        rEntry->m_bHasButtons = true;
    }
    else
    {
        m_pParent->enableOptionsButton( false );
    }

    if ( rEntry->m_bUser || rEntry->m_bShared )
    {
        m_pParent->enableRemoveButton( !rEntry->m_bLocked );
        rEntry->m_bHasButtons = true;
    }
    else
    {
        m_pParent->enableRemoveButton( false );
    }
}

MENU_COMMAND ExtBoxWithBtns_Impl::ShowPopupMenu( const Point & rPos, const long nPos )
{
    if ( nPos >= static_cast<long>(getItemCount()) )
        return CMD_NONE;

    ScopedVclPtrInstance<PopupMenu> aPopup;

#if ENABLE_EXTENSION_UPDATE
    aPopup->InsertItem( CMD_UPDATE, DpResId( RID_CTX_ITEM_CHECK_UPDATE ) );
#endif

    if ( ! GetEntryData( nPos )->m_bLocked )
    {
        if ( GetEntryData( nPos )->m_bUser )
        {
            if ( GetEntryData( nPos )->m_eState == REGISTERED )
                aPopup->InsertItem( CMD_DISABLE, DpResId( RID_CTX_ITEM_DISABLE ) );
            else if ( GetEntryData( nPos )->m_eState != NOT_AVAILABLE )
                aPopup->InsertItem( CMD_ENABLE, DpResId( RID_CTX_ITEM_ENABLE ) );
        }
        if (!officecfg::Office::ExtensionManager::ExtensionSecurity::DisableExtensionRemoval::get())
        {
            aPopup->InsertItem( CMD_REMOVE, DpResId( RID_CTX_ITEM_REMOVE ) );
        }
    }

    if ( !GetEntryData( nPos )->m_sLicenseText.isEmpty() )
        aPopup->InsertItem( CMD_SHOW_LICENSE, DpResId( RID_STR_SHOW_LICENSE_CMD ) );

    return static_cast<MENU_COMMAND>(aPopup->Execute( this, rPos ));
}


void ExtBoxWithBtns_Impl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( m_bInterfaceLocked )
        return;

    const Point aMousePos( rMEvt.GetPosPixel() );
    const long nPos = PointToPos( aMousePos );

    if ( rMEvt.IsRight() )
    {
        switch( ShowPopupMenu( aMousePos, nPos ) )
        {
            case CMD_NONE:      break;
            case CMD_ENABLE:    m_pParent->enablePackage( GetEntryData( nPos )->m_xPackage, true );
                                break;
            case CMD_DISABLE:   m_pParent->enablePackage( GetEntryData( nPos )->m_xPackage, false );
                                break;
            case CMD_UPDATE:    m_pParent->updatePackage( GetEntryData( nPos )->m_xPackage );
                                break;
            case CMD_REMOVE:    m_pParent->removePackage( GetEntryData( nPos )->m_xPackage );
                                break;
            case CMD_SHOW_LICENSE:
                {
                    ShowLicenseDialog aLicenseDlg(m_pParent->GetFrameWeld(), GetEntryData(nPos)->m_xPackage);
                    aLicenseDlg.run();
                    break;
                }
        }
    }
    else if ( rMEvt.IsLeft() )
    {
        const SolarMutexGuard aGuard;
        if ( rMEvt.IsMod1() && HasActive() )
            selectEntry( svt::IExtensionListBox::ENTRY_NOTFOUND );
                // selecting a not existing entry will deselect the current one
        else
            selectEntry( nPos );
    }
}

void ExtBoxWithBtns_Impl::enableButtons( bool bEnable )
{
    m_bInterfaceLocked = ! bEnable;

    if ( bEnable )
    {
        sal_Int32 nIndex = getSelIndex();
        if ( nIndex != svt::IExtensionListBox::ENTRY_NOTFOUND )
            SetButtonStatus( GetEntryData( nIndex ) );
    }
    else
    {
        m_pParent->enableEnableButton( false );
        m_pParent->enableOptionsButton( false );
        m_pParent->enableRemoveButton( false );
    }
}

//                             DialogHelper

DialogHelper::DialogHelper(const uno::Reference< uno::XComponentContext > &xContext,
                           Dialog *pWindow)
    : m_xVCLWindow(pWindow)
    , m_nEventID(nullptr)
{
    m_xContext = xContext;
}

DialogHelper::~DialogHelper()
{
    if ( m_nEventID )
        Application::RemoveUserEvent( m_nEventID );
}


bool DialogHelper::IsSharedPkgMgr( const uno::Reference< deployment::XPackage > &xPackage )
{
    return xPackage->getRepositoryName() == SHARED_PACKAGE_MANAGER;
}

bool DialogHelper::continueOnSharedExtension( const uno::Reference< deployment::XPackage > &xPackage,
                                              weld::Widget* pParent,
                                              const char* pResID,
                                              bool &bHadWarning )
{
    if ( !bHadWarning && IsSharedPkgMgr( xPackage ) )
    {
        const SolarMutexGuard guard;
        incBusy();
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pParent,
                                                  VclMessageType::Warning, VclButtonsType::OkCancel, DpResId(pResID)));
        bHadWarning = true;

        bool bRet = RET_OK == xBox->run();
        xBox.reset();
        decBusy();
        return bRet;
    }
    else
        return true;
}

void DialogHelper::openWebBrowser(const OUString& sURL, const OUString& sTitle)
{
    if ( sURL.isEmpty() ) // Nothing to do, when the URL is empty
        return;

    try
    {
        uno::Reference< XSystemShellExecute > xSystemShellExecute(
            SystemShellExecute::create(m_xContext));
        //throws css::lang::IllegalArgumentException, css::system::SystemShellExecuteException
        xSystemShellExecute->execute( sURL, OUString(),  SystemShellExecuteFlags::URIS_ONLY );
    }
    catch ( const uno::Exception& )
    {
        uno::Any exc( ::cppu::getCaughtException() );
        OUString msg( ::comphelper::anyToString( exc ) );
        const SolarMutexGuard guard;
        incBusy();
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(getFrameWeld(),
                                                       VclMessageType::Warning, VclButtonsType::Ok, msg));
        xErrorBox->set_title(sTitle);
        xErrorBox->run();
        xErrorBox.reset();
        decBusy();
    }
}

bool DialogHelper::installExtensionWarn(const OUString &rExtensionName)
{
    const SolarMutexGuard guard;

    // Check if extension installation is disabled in the expert configurations
    if (officecfg::Office::ExtensionManager::ExtensionSecurity::DisableExtensionInstallation::get())
    {
        incBusy();
        std::unique_ptr<weld::MessageDialog> xWarnBox(Application::CreateMessageDialog(getFrameWeld(),
                                                      VclMessageType::Warning, VclButtonsType::Ok,
                                                      DpResId(RID_STR_WARNING_INSTALL_EXTENSION_DISABLED)));
        xWarnBox->run();
        xWarnBox.reset();
        decBusy();

        return false;
    }

    incBusy();
    std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(getFrameWeld(),
                                                  VclMessageType::Warning, VclButtonsType::OkCancel,
                                                  DpResId(RID_STR_WARNING_INSTALL_EXTENSION)));
    OUString sText(xInfoBox->get_primary_text());
    sText = sText.replaceAll("%NAME", rExtensionName);
    xInfoBox->set_primary_text(sText);

    bool bRet = RET_OK == xInfoBox->run();
    xInfoBox.reset();
    decBusy();
    return bRet;
}

bool DialogHelper::installForAllUsers(bool &bInstallForAll)
{
    const SolarMutexGuard guard;
    incBusy();
    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(getFrameWeld(), "desktop/ui/installforalldialog.ui"));
    std::unique_ptr<weld::MessageDialog> xQuery(xBuilder->weld_message_dialog("InstallForAllDialog"));
    short nRet = xQuery->run();
    xQuery.reset();
    decBusy();
    if (nRet == RET_CANCEL)
        return false;

    bInstallForAll = ( nRet == RET_NO );
    return true;
}

void DialogHelper::PostUserEvent( const Link<void*,void>& rLink, void* pCaller )
{
    if ( m_nEventID )
        Application::RemoveUserEvent( m_nEventID );

    m_nEventID = Application::PostUserEvent( rLink, pCaller, true/*bReferenceLink*/ );
}

//                             ExtMgrDialog
ExtMgrDialog::ExtMgrDialog(vcl::Window *pParent, TheExtensionManager *pManager, Dialog::InitFlag eFlag)
    : ModelessDialog(pParent, "ExtensionManagerDialog", "desktop/ui/extensionmanager.ui", eFlag)
    , DialogHelper(pManager->getContext(), static_cast<Dialog*>(this))
    , m_xRestartParent(pParent ? pParent : Application::GetDefDialogParent())
    , m_sAddPackages(DpResId(RID_STR_ADD_PACKAGES))
    , m_bHasProgress(false)
    , m_bProgressChanged(false)
    , m_bStartProgress(false)
    , m_bStopProgress(false)
    , m_bEnableWarning(false)
    , m_bDisableWarning(false)
    , m_bDeleteWarning(false)
    , m_bClosed(false)
    , m_nProgress(0)
    , m_pManager(pManager)
{
    get(m_pExtensionBox, "extensions");
    get(m_pOptionsBtn, "optionsbtn");
    get(m_pAddBtn, "addbtn");
    get(m_pRemoveBtn, "removebtn");
    get(m_pEnableBtn, "enablebtn");
    get(m_pUpdateBtn, "updatebtn");
    get(m_pCloseBtn, "close");
    get(m_pBundledCbx, "bundled");
    get(m_pSharedCbx, "shared");
    get(m_pUserCbx, "user");
    get(m_pGetExtensions, "getextensions");
    get(m_pProgressText, "progressft");
    get(m_pProgressBar, "progressbar");
    get(m_pCancelBtn, "cancel");

    m_pExtensionBox->InitFromDialog(this);

    m_pEnableBtn->SetHelpId( HID_EXTENSION_MANAGER_LISTBOX_ENABLE );

    m_pOptionsBtn->SetClickHdl( LINK( this, ExtMgrDialog, HandleOptionsBtn ) );
    m_pAddBtn->SetClickHdl( LINK( this, ExtMgrDialog, HandleAddBtn ) );
    m_pRemoveBtn->SetClickHdl( LINK( this, ExtMgrDialog, HandleRemoveBtn ) );
    m_pEnableBtn->SetClickHdl( LINK( this, ExtMgrDialog, HandleEnableBtn ) );
    m_pCloseBtn->SetClickHdl( LINK( this, ExtMgrDialog, HandleCloseBtn ) );

    m_pCancelBtn->SetClickHdl( LINK( this, ExtMgrDialog, HandleCancelBtn ) );

    m_pBundledCbx->SetClickHdl( LINK( this, ExtMgrDialog, HandleExtTypeCbx ) );
    m_pSharedCbx->SetClickHdl( LINK( this, ExtMgrDialog, HandleExtTypeCbx ) );
    m_pUserCbx->SetClickHdl( LINK( this, ExtMgrDialog, HandleExtTypeCbx ) );

    m_pBundledCbx->Check();
    m_pSharedCbx->Check();
    m_pUserCbx->Check();

    m_pProgressBar->Hide();

#if ENABLE_EXTENSION_UPDATE
    m_pUpdateBtn->SetClickHdl( LINK( this, ExtMgrDialog, HandleUpdateBtn ) );
    m_pUpdateBtn->Enable(false);
#else
    m_pUpdateBtn->Hide();
#endif

    if (officecfg::Office::ExtensionManager::ExtensionSecurity::DisableExtensionInstallation::get())
    {
        m_pAddBtn->Disable();
        m_pAddBtn->SetQuickHelpText(DpResId(RID_STR_WARNING_INSTALL_EXTENSION_DISABLED));
    }
    if (officecfg::Office::ExtensionManager::ExtensionSecurity::DisableExtensionRemoval::get())
    {
        m_pRemoveBtn->Disable();
        m_pRemoveBtn->SetQuickHelpText(DpResId(RID_STR_WARNING_REMOVE_EXTENSION_DISABLED));
    }

    m_aIdle.SetPriority(TaskPriority::LOWEST);
    m_aIdle.SetDebugName( "ExtMgrDialog m_aIdle TimeOutHdl" );
    m_aIdle.SetInvokeHandler( LINK( this, ExtMgrDialog, TimeOutHdl ) );
}


ExtMgrDialog::~ExtMgrDialog()
{
    disposeOnce();
}

void ExtMgrDialog::dispose()
{
    m_aIdle.Stop();
    m_pExtensionBox.clear();
    m_pOptionsBtn.clear();
    m_pAddBtn.clear();
    m_pRemoveBtn.clear();
    m_pEnableBtn.clear();
    m_pUpdateBtn.clear();
    m_pCloseBtn.clear();
    m_pBundledCbx.clear();
    m_pSharedCbx.clear();
    m_pUserCbx.clear();
    m_pGetExtensions.clear();
    m_pProgressText.clear();
    m_pProgressBar.clear();
    m_pCancelBtn.clear();
    m_xRestartParent.clear();
    ModelessDialog::dispose();
}


void ExtMgrDialog::setGetExtensionsURL( const OUString &rURL )
{
    m_pGetExtensions->SetURL( rURL );
}

void ExtMgrDialog::addPackageToList( const uno::Reference< deployment::XPackage > &xPackage,
                                     bool bLicenseMissing )
{
    const SolarMutexGuard aGuard;
    m_pUpdateBtn->Enable();

    if (m_pBundledCbx->IsChecked() && (xPackage->getRepositoryName() == BUNDLED_PACKAGE_MANAGER) )
    {
        m_pExtensionBox->addEntry( xPackage, bLicenseMissing );
    }
    else if (m_pSharedCbx->IsChecked() && (xPackage->getRepositoryName() == SHARED_PACKAGE_MANAGER) )
    {
        m_pExtensionBox->addEntry( xPackage, bLicenseMissing );
    }
    else if (m_pUserCbx->IsChecked() && (xPackage->getRepositoryName() == USER_PACKAGE_MANAGER ))
    {
        m_pExtensionBox->addEntry( xPackage, bLicenseMissing );
    }
}

void ExtMgrDialog::prepareChecking()
{
    m_pExtensionBox->prepareChecking();
}

void ExtMgrDialog::checkEntries()
{
    const SolarMutexGuard guard;
    m_pExtensionBox->checkEntries();
}

bool ExtMgrDialog::removeExtensionWarn(const OUString &rExtensionName)
{
    const SolarMutexGuard guard;
    incBusy();
    std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                  VclMessageType::Warning, VclButtonsType::OkCancel,
                                                  DpResId(RID_STR_WARNING_REMOVE_EXTENSION)));

    OUString sText(xInfoBox->get_primary_text());
    sText = sText.replaceAll("%NAME", rExtensionName);
    xInfoBox->set_primary_text(sText);

    bool bRet = RET_OK == xInfoBox->run();
    xInfoBox.reset();
    decBusy();

    return bRet;
}

void ExtMgrDialog::enablePackage( const uno::Reference< deployment::XPackage > &xPackage,
                                  bool bEnable )
{
    if ( !xPackage.is() )
        return;

    if ( bEnable )
    {
        if (!continueOnSharedExtension(xPackage, GetFrameWeld(), RID_STR_WARNING_ENABLE_SHARED_EXTENSION, m_bEnableWarning))
            return;
    }
    else
    {
        if (!continueOnSharedExtension(xPackage, GetFrameWeld(), RID_STR_WARNING_DISABLE_SHARED_EXTENSION, m_bDisableWarning))
            return;
    }

    m_pManager->getCmdQueue()->enableExtension( xPackage, bEnable );
}


void ExtMgrDialog::removePackage( const uno::Reference< deployment::XPackage > &xPackage )
{
    if ( !xPackage.is() )
        return;

    if ( !IsSharedPkgMgr( xPackage ) || m_bDeleteWarning )
    {
        if ( ! removeExtensionWarn( xPackage->getDisplayName() ) )
            return;
    }

    if (!continueOnSharedExtension(xPackage, GetFrameWeld(), RID_STR_WARNING_REMOVE_SHARED_EXTENSION, m_bDeleteWarning))
        return;

    m_pManager->getCmdQueue()->removeExtension( xPackage );
}


void ExtMgrDialog::updatePackage( const uno::Reference< deployment::XPackage > &xPackage )
{
    if ( !xPackage.is() )
        return;

    // get the extension with highest version
    uno::Sequence<uno::Reference<deployment::XPackage> > seqExtensions =
    m_pManager->getExtensionManager()->getExtensionsWithSameIdentifier(
        dp_misc::getIdentifier(xPackage), xPackage->getName(), uno::Reference<ucb::XCommandEnvironment>());
    uno::Reference<deployment::XPackage> extension =
        dp_misc::getExtensionWithHighestVersion(seqExtensions);
    OSL_ASSERT(extension.is());
    std::vector< css::uno::Reference< css::deployment::XPackage > > vEntries;
    vEntries.push_back(extension);

    m_pManager->getCmdQueue()->checkForUpdates( vEntries );
}


bool ExtMgrDialog::acceptLicense( const uno::Reference< deployment::XPackage > &xPackage )
{
    if ( !xPackage.is() )
        return false;

    m_pManager->getCmdQueue()->acceptLicense( xPackage );

    return true;
}


uno::Sequence< OUString > ExtMgrDialog::raiseAddPicker()
{
    sfx2::FileDialogHelper aDlgHelper(ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, FileDialogFlags::NONE, GetFrameWeld());
    const uno::Reference<ui::dialogs::XFilePicker3>& xFilePicker = aDlgHelper.GetFilePicker();
    xFilePicker->setTitle( m_sAddPackages );

    if ( !m_sLastFolderURL.isEmpty() )
        xFilePicker->setDisplayDirectory( m_sLastFolderURL );

    // collect and set filter list:
    typedef std::map< OUString, OUString > t_string2string;
    t_string2string title2filter;
    OUString sDefaultFilter( StrAllFiles::get() );

    const uno::Sequence< uno::Reference< deployment::XPackageTypeInfo > > packageTypes(
        m_pManager->getExtensionManager()->getSupportedPackageTypes() );

    for ( sal_Int32 pos = 0; pos < packageTypes.getLength(); ++pos )
    {
        uno::Reference< deployment::XPackageTypeInfo > const & xPackageType = packageTypes[ pos ];
        const OUString filter( xPackageType->getFileFilter() );
        if (!filter.isEmpty())
        {
            const OUString title( xPackageType->getShortDescription() );
            const std::pair< t_string2string::iterator, bool > insertion(
                title2filter.emplace( title, filter ) );
            if ( ! insertion.second )
            { // already existing, append extensions:
                OUStringBuffer buf;
                buf.append( insertion.first->second );
                buf.append( ';' );
                buf.append( filter );
                insertion.first->second = buf.makeStringAndClear();
            }
            if ( xPackageType->getMediaType() == "application/vnd.sun.star.package-bundle" )
                sDefaultFilter = title;
        }
    }

    // All files at top:
    xFilePicker->appendFilter( StrAllFiles::get(), "*.*" );
    // then supported ones:
    for (auto const& elem : title2filter)
    {
        try
        {
            xFilePicker->appendFilter( elem.first, elem.second );
        }
        catch (const lang::IllegalArgumentException &)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "desktop", exceptionToString(ex) );
        }
    }
    xFilePicker->setCurrentFilter( sDefaultFilter );

    if ( xFilePicker->execute() != ui::dialogs::ExecutableDialogResults::OK )
        return uno::Sequence<OUString>(); // cancelled

    m_sLastFolderURL = xFilePicker->getDisplayDirectory();
    uno::Sequence< OUString > files( xFilePicker->getSelectedFiles() );
    OSL_ASSERT( files.getLength() > 0 );
    return files;
}

void ExtMgrDialog::enableOptionsButton( bool bEnable )
{
    m_pOptionsBtn->Enable( bEnable );
}

void ExtMgrDialog::enableRemoveButton( bool bEnable )
{
    m_pRemoveBtn->Enable( bEnable && !officecfg::Office::ExtensionManager::ExtensionSecurity::DisableExtensionRemoval::get());

    if (officecfg::Office::ExtensionManager::ExtensionSecurity::DisableExtensionRemoval::get())
    {
        m_pRemoveBtn->SetQuickHelpText(DpResId(RID_STR_WARNING_REMOVE_EXTENSION_DISABLED));
    }
    else
    {
        m_pRemoveBtn->SetQuickHelpText("");
    }
}

void ExtMgrDialog::enableEnableButton( bool bEnable )
{
    m_pEnableBtn->Enable( bEnable );
}

void ExtMgrDialog::enableButtontoEnable( bool bEnable )
{
    if (bEnable)
    {
        m_pEnableBtn->SetText( DpResId( RID_CTX_ITEM_ENABLE ) );
        m_pEnableBtn->SetHelpId( HID_EXTENSION_MANAGER_LISTBOX_ENABLE );
    }
    else
    {
        m_pEnableBtn->SetText( DpResId( RID_CTX_ITEM_DISABLE ) );
        m_pEnableBtn->SetHelpId( HID_EXTENSION_MANAGER_LISTBOX_DISABLE );
    }
}

IMPL_LINK_NOARG(ExtMgrDialog, HandleCancelBtn, Button*, void)
{
    if ( m_xAbortChannel.is() )
    {
        try
        {
            m_xAbortChannel->sendAbort();
        }
        catch ( const uno::RuntimeException & )
        {
            OSL_FAIL( "### unexpected RuntimeException!" );
        }
    }
}

IMPL_LINK_NOARG(ExtMgrDialog, HandleCloseBtn, Button*, void)
{
    Close();
}


IMPL_LINK( ExtMgrDialog, startProgress, void*, _bLockInterface, void )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    bool bLockInterface = static_cast<bool>(_bLockInterface);

    if ( m_bStartProgress && !m_bHasProgress )
        m_aIdle.Start();

    if ( m_bStopProgress )
    {
        if ( m_pProgressBar->IsVisible() )
            m_pProgressBar->SetValue( 100 );
        m_xAbortChannel.clear();

        SAL_INFO( "desktop.deployment", " startProgress handler: stop" );
    }
    else
    {
        SAL_INFO( "desktop.deployment", " startProgress handler: start" );
    }

    m_pCancelBtn->Enable( bLockInterface );
    m_pAddBtn->Enable( !bLockInterface && !officecfg::Office::ExtensionManager::ExtensionSecurity::DisableExtensionInstallation::get());
    if (officecfg::Office::ExtensionManager::ExtensionSecurity::DisableExtensionInstallation::get())
    {
        m_pAddBtn->SetQuickHelpText(DpResId(RID_STR_WARNING_INSTALL_EXTENSION_DISABLED));
    }
    else
    {
        m_pAddBtn->SetQuickHelpText("");
    }

    m_pUpdateBtn->Enable( !bLockInterface && m_pExtensionBox->getItemCount() );
    m_pExtensionBox->enableButtons( !bLockInterface );

    clearEventID();
}


void ExtMgrDialog::showProgress( bool _bStart )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    bool bStart = _bStart;

    if ( bStart )
    {
        m_nProgress = 0;
        m_bStartProgress = true;
        SAL_INFO( "desktop.deployment", "showProgress start" );
    }
    else
    {
        m_nProgress = 100;
        m_bStopProgress = true;
        SAL_INFO( "desktop.deployment", "showProgress stop!" );
    }

    DialogHelper::PostUserEvent( LINK( this, ExtMgrDialog, startProgress ), reinterpret_cast<void*>(bStart) );
    m_aIdle.Start();
}


void ExtMgrDialog::updateProgress( const long nProgress )
{
    if ( m_nProgress != nProgress )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_nProgress = nProgress;
        m_aIdle.Start();
    }
}


void ExtMgrDialog::updateProgress( const OUString &rText,
                                   const uno::Reference< task::XAbortChannel > &xAbortChannel)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    m_xAbortChannel = xAbortChannel;
    m_sProgressText = rText;
    m_bProgressChanged = true;
    m_aIdle.Start();
}


void ExtMgrDialog::updatePackageInfo( const uno::Reference< deployment::XPackage > &xPackage )
{
    const SolarMutexGuard aGuard;
    m_pExtensionBox->updateEntry( xPackage );
}

IMPL_LINK_NOARG(ExtMgrDialog, HandleOptionsBtn, Button*, void)
{
    const sal_Int32 nActive = m_pExtensionBox->getSelIndex();

    if ( nActive != svt::IExtensionListBox::ENTRY_NOTFOUND )
    {
        SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();

        OUString sExtensionId = m_pExtensionBox->GetEntryData( nActive )->m_xPackage->getIdentifier().Value;
        ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateOptionsDialog( this, sExtensionId ));

        pDlg->Execute();
    }
}

IMPL_LINK_NOARG(ExtMgrDialog, HandleAddBtn, Button*, void)
{
    incBusy();

    uno::Sequence< OUString > aFileList = raiseAddPicker();

    if ( aFileList.getLength() )
    {
        m_pManager->installPackage( aFileList[0] );
    }

    decBusy();
}

IMPL_LINK_NOARG(ExtMgrDialog, HandleRemoveBtn, Button*, void)
{
    const sal_Int32 nActive = m_pExtensionBox->getSelIndex();

    if ( nActive != svt::IExtensionListBox::ENTRY_NOTFOUND )
    {
        TEntry_Impl pEntry = m_pExtensionBox->GetEntryData( nActive );
        removePackage( pEntry->m_xPackage );
    }
}

IMPL_LINK_NOARG(ExtMgrDialog, HandleEnableBtn, Button*, void)
{
    const sal_Int32 nActive = m_pExtensionBox->getSelIndex();

    if ( nActive != svt::IExtensionListBox::ENTRY_NOTFOUND )
    {
        TEntry_Impl pEntry = m_pExtensionBox->GetEntryData( nActive );

        if ( pEntry->m_bMissingLic )
            acceptLicense( pEntry->m_xPackage );
        else
        {
            const bool bEnable( pEntry->m_eState != REGISTERED );
            enablePackage( pEntry->m_xPackage, bEnable );
        }
    }
}

IMPL_LINK_NOARG(ExtMgrDialog, HandleExtTypeCbx, Button*, void)
{
    // re-creates the list of packages with addEntry selecting the packages
    prepareChecking();
    m_pManager->createPackageList();
    checkEntries();
}

IMPL_LINK_NOARG(ExtMgrDialog, HandleUpdateBtn, Button*, void)
{
#if ENABLE_EXTENSION_UPDATE
    m_pManager->checkUpdates();
#else
    (void) this;
#endif
}

IMPL_LINK_NOARG(ExtMgrDialog, TimeOutHdl, Timer *, void)
{
    if ( m_bStopProgress )
    {
        m_bHasProgress = false;
        m_bStopProgress = false;
        m_pProgressText->Hide();
        m_pProgressBar->Hide();
        m_pCancelBtn->Hide();
    }
    else
    {
        if ( m_bProgressChanged )
        {
            m_bProgressChanged = false;
            m_pProgressText->SetText(m_sProgressText);
        }

        if ( m_bStartProgress )
        {
            m_bStartProgress = false;
            m_bHasProgress = true;
            m_pProgressBar->Show();
            m_pProgressText->Show();
            m_pCancelBtn->Enable();
            m_pCancelBtn->Show();
        }

        if ( m_pProgressBar->IsVisible() )
            m_pProgressBar->SetValue( static_cast<sal_uInt16>(m_nProgress) );
    }
}


// VCL::Window / Dialog

bool ExtMgrDialog::EventNotify( NotifyEvent& rNEvt )
{
    bool bHandled = false;

    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        vcl::KeyCode aKeyCode = pKEvt->GetKeyCode();

        if ( aKeyCode.GetGroup() == KEYGROUP_CURSOR )
            bHandled = m_pExtensionBox->EventNotify(rNEvt);
    }
// VclEventId::WindowClose
    if ( !bHandled )
        return ModelessDialog::EventNotify(rNEvt);
    else
        return true;
}

IMPL_STATIC_LINK(ExtMgrDialog, Restart, void*, pParent, void)
{
    SolarMutexGuard aGuard;
    ::svtools::executeRestartDialog(comphelper::getProcessComponentContext(),
                                    pParent ? static_cast<vcl::Window*>(pParent)->GetFrameWeld() : nullptr,
                                    svtools::RESTART_REASON_EXTENSION_INSTALL);
}

bool ExtMgrDialog::Close()
{
    bool bRet = ModelessDialog::Close();
    m_pManager->terminateDialog();
    //only suggest restart if modified and this is the first close attempt
    if (!m_bClosed && m_pManager->isModified())
    {
        m_pManager->clearModified();
        //only suggest restart if we're actually running, e.g. not from standalone unopkg gui
        if (dp_misc::office_is_running())
            Application::PostUserEvent(LINK(nullptr, ExtMgrDialog, Restart), m_xRestartParent);
    }
    m_bClosed = true;
    return bRet;
}

//UpdateRequiredDialog
UpdateRequiredDialog::UpdateRequiredDialog(vcl::Window *pParent, TheExtensionManager *pManager)
    : ModalDialog(pParent, "UpdateRequiredDialog", "desktop/ui/updaterequireddialog.ui")
    , DialogHelper(pManager->getContext(), static_cast<Dialog*>(this))
    , m_sCloseText(DpResId(RID_STR_CLOSE_BTN))
    , m_bHasProgress(false)
    , m_bProgressChanged(false)
    , m_bStartProgress(false)
    , m_bStopProgress(false)
    , m_bHasLockedEntries(false)
    , m_nProgress(0)
    , m_pManager(pManager)
{
    get(m_pExtensionBox, "extensions");
    m_pExtensionBox->setExtensionManager(pManager);
    get(m_pUpdateNeeded, "updatelabel");
    get(m_pUpdateBtn, "check");
    get(m_pCloseBtn, "disable");
    get(m_pCancelBtn, "cancel");
    get(m_pProgressText, "progresslabel");
    get(m_pProgressBar, "progress");

    m_pUpdateBtn->SetClickHdl( LINK( this, UpdateRequiredDialog, HandleUpdateBtn ) );
    m_pCloseBtn->SetClickHdl( LINK( this, UpdateRequiredDialog, HandleCloseBtn ) );
    m_pCancelBtn->SetClickHdl( LINK( this, UpdateRequiredDialog, HandleCancelBtn ) );

    OUString aText = m_pUpdateNeeded->GetText();
    aText = aText.replaceAll(
        "%PRODUCTNAME", utl::ConfigManager::getProductName());
    m_pUpdateNeeded->SetText(aText);

    m_pProgressBar->Hide();
    m_pUpdateBtn->Enable( false );
    m_pCloseBtn->GrabFocus();

    m_aIdle.SetPriority( TaskPriority::LOWEST );
    m_aIdle.SetDebugName( "UpdateRequiredDialog m_aIdle TimeOutHdl" );
    m_aIdle.SetInvokeHandler( LINK( this, UpdateRequiredDialog, TimeOutHdl ) );
}

UpdateRequiredDialog::~UpdateRequiredDialog()
{
    disposeOnce();
}

void UpdateRequiredDialog::dispose()
{
    m_aIdle.Stop();
    m_pExtensionBox.clear();
    m_pUpdateNeeded.clear();
    m_pUpdateBtn.clear();
    m_pCloseBtn.clear();
    m_pCancelBtn.clear();
    m_pProgressText.clear();
    m_pProgressBar.clear();
    ModalDialog::dispose();
}

void UpdateRequiredDialog::addPackageToList( const uno::Reference< deployment::XPackage > &xPackage,
                                             bool bLicenseMissing )
{
    // We will only add entries to the list with unsatisfied dependencies
    if ( !bLicenseMissing && !checkDependencies( xPackage ) )
    {
        m_bHasLockedEntries |= m_pManager->isReadOnly( xPackage );
        const SolarMutexGuard aGuard;
        m_pUpdateBtn->Enable();
        m_pExtensionBox->addEntry( xPackage );
    }
}


void UpdateRequiredDialog::prepareChecking()
{
    m_pExtensionBox->prepareChecking();
}


void UpdateRequiredDialog::checkEntries()
{
    const SolarMutexGuard guard;
    m_pExtensionBox->checkEntries();

    if ( ! hasActiveEntries() )
    {
        m_pCloseBtn->SetText( m_sCloseText );
        m_pCloseBtn->GrabFocus();
    }
}


IMPL_LINK_NOARG(UpdateRequiredDialog, HandleCancelBtn, Button*, void)
{
    if ( m_xAbortChannel.is() )
    {
        try
        {
            m_xAbortChannel->sendAbort();
        }
        catch ( const uno::RuntimeException & )
        {
            OSL_FAIL( "### unexpected RuntimeException!" );
        }
    }
}


IMPL_LINK( UpdateRequiredDialog, startProgress, void*, _bLockInterface, void )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    bool bLockInterface = static_cast<bool>(_bLockInterface);

    if ( m_bStartProgress && !m_bHasProgress )
        m_aIdle.Start();

    if ( m_bStopProgress )
    {
        if ( m_pProgressBar->IsVisible() )
            m_pProgressBar->SetValue( 100 );
        m_xAbortChannel.clear();
        SAL_INFO( "desktop.deployment", " startProgress handler: stop" );
    }
    else
    {
        SAL_INFO( "desktop.deployment", " startProgress handler: start" );
    }

    m_pCancelBtn->Enable( bLockInterface );
    m_pUpdateBtn->Enable( false );
    clearEventID();
}


void UpdateRequiredDialog::showProgress( bool _bStart )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    bool bStart = _bStart;

    if ( bStart )
    {
        m_nProgress = 0;
        m_bStartProgress = true;
        SAL_INFO( "desktop.deployment", "showProgress start" );
    }
    else
    {
        m_nProgress = 100;
        m_bStopProgress = true;
        SAL_INFO( "desktop.deployment", "showProgress stop!" );
    }

    DialogHelper::PostUserEvent( LINK( this, UpdateRequiredDialog, startProgress ), reinterpret_cast<void*>(bStart) );
    m_aIdle.Start();
}


void UpdateRequiredDialog::updateProgress( const long nProgress )
{
    if ( m_nProgress != nProgress )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_nProgress = nProgress;
        m_aIdle.Start();
    }
}


void UpdateRequiredDialog::updateProgress( const OUString &rText,
                                           const uno::Reference< task::XAbortChannel > &xAbortChannel)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    m_xAbortChannel = xAbortChannel;
    m_sProgressText = rText;
    m_bProgressChanged = true;
    m_aIdle.Start();
}


void UpdateRequiredDialog::updatePackageInfo( const uno::Reference< deployment::XPackage > &xPackage )
{
    // We will remove all updated packages with satisfied dependencies, but
    // we will show all disabled entries so the user sees the result
    // of the 'disable all' button
    const SolarMutexGuard aGuard;
    if ( isEnabled( xPackage ) && checkDependencies( xPackage ) )
        m_pExtensionBox->removeEntry( xPackage );
    else
        m_pExtensionBox->updateEntry( xPackage );

    if ( ! hasActiveEntries() )
    {
        m_pCloseBtn->SetText( m_sCloseText );
        m_pCloseBtn->GrabFocus();
    }
}


IMPL_LINK_NOARG(UpdateRequiredDialog, HandleUpdateBtn, Button*, void)
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );

    std::vector< uno::Reference< deployment::XPackage > > vUpdateEntries;
    sal_Int32 nCount = m_pExtensionBox->GetEntryCount();

    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        TEntry_Impl pEntry = m_pExtensionBox->GetEntryData( i );
        vUpdateEntries.push_back( pEntry->m_xPackage );
    }

    aGuard.clear();

    m_pManager->getCmdQueue()->checkForUpdates( vUpdateEntries );
}


IMPL_LINK_NOARG(UpdateRequiredDialog, HandleCloseBtn, Button*, void)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !isBusy() )
    {
        if ( m_bHasLockedEntries )
            EndDialog( -1 );
        else if ( hasActiveEntries() )
            disableAllEntries();
        else
            EndDialog();
    }
}


IMPL_LINK_NOARG(UpdateRequiredDialog, TimeOutHdl, Timer *, void)
{
    if ( m_bStopProgress )
    {
        m_bHasProgress = false;
        m_bStopProgress = false;
        m_pProgressText->Hide();
        m_pProgressBar->Hide();
        m_pCancelBtn->Hide();
    }
    else
    {
        if ( m_bProgressChanged )
        {
            m_bProgressChanged = false;
            m_pProgressText->SetText( m_sProgressText );
        }

        if ( m_bStartProgress )
        {
            m_bStartProgress = false;
            m_bHasProgress = true;
            m_pProgressBar->Show();
            m_pProgressText->Show();
            m_pCancelBtn->Enable();
            m_pCancelBtn->Show();
        }

        if ( m_pProgressBar->IsVisible() )
            m_pProgressBar->SetValue( static_cast<sal_uInt16>(m_nProgress) );
    }
}

// VCL::Dialog
short UpdateRequiredDialog::Execute()
{
    //ToDo
    //I believe m_bHasLockedEntries was used to prevent showing extensions which cannot
    //be disabled because they are in a read only repository. However, disabling extensions
    //is now always possible because the registration data of all repositories
    //are in the user installation.
    //Therefore all extensions could be displayed and all the handling around m_bHasLockedEntries
    //could be removed.
    if ( m_bHasLockedEntries )
    {
        // Set other text, disable update btn, remove not shared entries from list;
        m_pUpdateNeeded->SetText( DpResId( RID_STR_NO_ADMIN_PRIVILEGE ) );
        m_pCloseBtn->SetText( DpResId( RID_STR_EXIT_BTN ) );
        m_pUpdateBtn->Enable( false );
        m_pExtensionBox->RemoveUnlocked();
        Resize();
    }

    return Dialog::Execute();
}

// VCL::Dialog
bool UpdateRequiredDialog::Close()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !isBusy() )
    {
        if ( m_bHasLockedEntries )
            EndDialog( -1 );
        else if ( hasActiveEntries() )
            disableAllEntries();
        else
            EndDialog();
    }

    return false;
}


// Check dependencies of all packages

bool UpdateRequiredDialog::isEnabled( const uno::Reference< deployment::XPackage > &xPackage )
{
    bool bRegistered = false;
    try {
        beans::Optional< beans::Ambiguous< sal_Bool > > option( xPackage->isRegistered( uno::Reference< task::XAbortChannel >(),
                                                                                        uno::Reference< ucb::XCommandEnvironment >() ) );
        if ( option.IsPresent )
        {
            ::beans::Ambiguous< sal_Bool > const & reg = option.Value;
            if ( reg.IsAmbiguous )
                bRegistered = false;
            else
                bRegistered = reg.Value;
        }
        else
            bRegistered = false;
    }
    catch ( const uno::RuntimeException & ) { throw; }
    catch (const uno::Exception & ) {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN( "desktop", exceptionToString(ex) );
        bRegistered = false;
    }

    return bRegistered;
}

// Checks the dependencies no matter if the extension is enabled or disabled!
bool UpdateRequiredDialog::checkDependencies( const uno::Reference< deployment::XPackage > &xPackage )
{
    bool bDependenciesValid = false;
    try {
        bDependenciesValid = xPackage->checkDependencies( uno::Reference< ucb::XCommandEnvironment >() );
    }
    catch ( const deployment::DeploymentException & ) {}
    return bDependenciesValid;
}


bool UpdateRequiredDialog::hasActiveEntries()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    bool bRet = false;
    long nCount = m_pExtensionBox->GetEntryCount();
    for ( long nIndex = 0; nIndex < nCount; nIndex++ )
    {
        TEntry_Impl pEntry = m_pExtensionBox->GetEntryData( nIndex );

        if ( isEnabled(pEntry->m_xPackage) && !checkDependencies( pEntry->m_xPackage ) )
        {
            bRet = true;
            break;
        }
    }

    return bRet;
}


void UpdateRequiredDialog::disableAllEntries()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    incBusy();

    long nCount = m_pExtensionBox->GetEntryCount();
    for ( long nIndex = 0; nIndex < nCount; nIndex++ )
    {
        TEntry_Impl pEntry = m_pExtensionBox->GetEntryData( nIndex );
        m_pManager->getCmdQueue()->enableExtension( pEntry->m_xPackage, false );
    }

    decBusy();

    if ( ! hasActiveEntries() )
        m_pCloseBtn->SetText( m_sCloseText );
}

//                             ShowLicenseDialog
ShowLicenseDialog::ShowLicenseDialog(weld::Window* pParent,
                                     const uno::Reference< deployment::XPackage> &xPackage)
    : GenericDialogController(pParent, "desktop/ui/showlicensedialog.ui", "ShowLicenseDialog")
    , m_xLicenseText(m_xBuilder->weld_text_view("textview"))
{
    m_xLicenseText->set_size_request(m_xLicenseText->get_approximate_digit_width() * 72,
                                     m_xLicenseText->get_height_rows(21));
    m_xLicenseText->set_text(xPackage->getLicenseText());
}

ShowLicenseDialog::~ShowLicenseDialog()
{
}

// UpdateRequiredDialogService

UpdateRequiredDialogService::UpdateRequiredDialogService( SAL_UNUSED_PARAMETER uno::Sequence< uno::Any > const&,
                                                          uno::Reference< uno::XComponentContext > const& xComponentContext )
    : m_xComponentContext( xComponentContext )
{
}


// XExecutableDialog

void UpdateRequiredDialogService::setTitle( OUString const & )
{
}


sal_Int16 UpdateRequiredDialogService::execute()
{
    ::rtl::Reference< ::dp_gui::TheExtensionManager > xManager( TheExtensionManager::get(
                                                              m_xComponentContext) );
    xManager->createDialog( true );
    sal_Int16 nRet = xManager->execute();

    return nRet;
}


} //namespace dp_gui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
