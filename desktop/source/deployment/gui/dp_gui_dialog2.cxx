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

#include <config_extensions.h>

#include <strings.hrc>
#include <helpids.h>

#include "dp_gui.h"
#include "dp_gui_dialog2.hxx"
#include "dp_gui_extlistbox.hxx"
#include <dp_shared.hxx>
#include "dp_gui_theextmgr.hxx"
#include "dp_gui_extensioncmdqueue.hxx"
#include <dp_misc.h>
#include <dp_update.hxx>
#include <dp_identifier.hxx>

#include <fpicker/strings.hrc>

#include <utility>
#include <vcl/commandevent.hxx>
#include <vcl/svapp.hxx>

#include <sal/log.hxx>
#include <rtl/ustrbuf.hxx>

#include <svtools/restartdialog.hxx>

#include <sfx2/filedlghelper.hxx>
#include <sfx2/sfxdlg.hxx>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <unotools/configmgr.hxx>

#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>

#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>

#include <officecfg/Office/ExtensionManager.hxx>

#include <map>
#include <memory>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::system;


namespace dp_gui {

constexpr OUStringLiteral USER_PACKAGE_MANAGER = u"user";
constexpr OUString SHARED_PACKAGE_MANAGER = u"shared"_ustr;
constexpr OUStringLiteral BUNDLED_PACKAGE_MANAGER = u"bundled";

// ExtBoxWithBtns_Impl
class ExtBoxWithBtns_Impl : public ExtensionBox_Impl
{
    bool            m_bInterfaceLocked;

    ExtMgrDialog*   m_pParent;

    void            SetButtonStatus( const TEntry_Impl& rEntry );
    OUString        ShowPopupMenu( const Point &rPos, const tools::Long nPos );

public:
    explicit ExtBoxWithBtns_Impl(std::unique_ptr<weld::ScrolledWindow> xScroll);

    void InitFromDialog(ExtMgrDialog *pParentDialog);

    virtual bool    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual bool    Command( const CommandEvent& rCEvt ) override;

    virtual void    RecalcAll() override;
    virtual void    selectEntry( const tools::Long nPos ) override;

    void            enableButtons( bool bEnable );
};

ExtBoxWithBtns_Impl::ExtBoxWithBtns_Impl(std::unique_ptr<weld::ScrolledWindow> xScroll)
    : ExtensionBox_Impl(std::move(xScroll))
    , m_bInterfaceLocked(false)
    , m_pParent(nullptr)
{
}

void ExtBoxWithBtns_Impl::InitFromDialog(ExtMgrDialog *pParentDialog)
{
    setExtensionManager(pParentDialog->getExtensionManager());

    m_pParent = pParentDialog;
}

void ExtBoxWithBtns_Impl::RecalcAll()
{
    const sal_Int32 nActive = getSelIndex();

    if ( nActive != ExtensionBox_Impl::ENTRY_NOTFOUND )
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
void ExtBoxWithBtns_Impl::selectEntry( const tools::Long nPos )
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

bool ExtBoxWithBtns_Impl::Command(const CommandEvent& rCEvt)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return ExtensionBox_Impl::Command(rCEvt);

    const Point aMousePos(rCEvt.GetMousePosPixel());
    const auto nPos = PointToPos(aMousePos);
    OUString sCommand = ShowPopupMenu(aMousePos, nPos);

    if (sCommand == "CMD_ENABLE")
        m_pParent->enablePackage( GetEntryData( nPos )->m_xPackage, true );
    else if (sCommand == "CMD_DISABLE")
        m_pParent->enablePackage( GetEntryData( nPos )->m_xPackage, false );
    else if (sCommand == "CMD_UPDATE")
        m_pParent->updatePackage( GetEntryData( nPos )->m_xPackage );
    else if (sCommand == "CMD_REMOVE")
        m_pParent->removePackage( GetEntryData( nPos )->m_xPackage );
    else if (sCommand == "CMD_SHOW_LICENSE")
    {
        m_pParent->incBusy();
        ShowLicenseDialog aLicenseDlg(m_pParent->getDialog(), GetEntryData(nPos)->m_xPackage);
        aLicenseDlg.run();
        m_pParent->decBusy();
    }

    return true;
}

OUString ExtBoxWithBtns_Impl::ShowPopupMenu( const Point & rPos, const tools::Long nPos )
{
    if ( nPos >= static_cast<tools::Long>(getItemCount()) )
        return "CMD_NONE";

    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(nullptr, "desktop/ui/extensionmenu.ui"));
    std::unique_ptr<weld::Menu> xPopup(xBuilder->weld_menu("menu"));

#if ENABLE_EXTENSION_UPDATE
    xPopup->append("CMD_UPDATE", DpResId( RID_CTX_ITEM_CHECK_UPDATE ) );
#endif

    if ( ! GetEntryData( nPos )->m_bLocked )
    {
        if ( GetEntryData( nPos )->m_bUser )
        {
            if ( GetEntryData( nPos )->m_eState == REGISTERED )
                xPopup->append("CMD_DISABLE", DpResId(RID_CTX_ITEM_DISABLE));
            else if ( GetEntryData( nPos )->m_eState != NOT_AVAILABLE )
                xPopup->append("CMD_ENABLE", DpResId(RID_CTX_ITEM_ENABLE));
        }
        if (!officecfg::Office::ExtensionManager::ExtensionSecurity::DisableExtensionRemoval::get())
        {
            xPopup->append("CMD_REMOVE", DpResId(RID_CTX_ITEM_REMOVE));
        }
    }

    if ( !GetEntryData( nPos )->m_sLicenseText.isEmpty() )
        xPopup->append("CMD_SHOW_LICENSE", DpResId(RID_STR_SHOW_LICENSE_CMD));

    return xPopup->popup_at_rect(GetDrawingArea(), tools::Rectangle(rPos, Size(1, 1)));
}

bool ExtBoxWithBtns_Impl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (m_bInterfaceLocked)
        return false;
    return ExtensionBox_Impl::MouseButtonDown(rMEvt);
}

void ExtBoxWithBtns_Impl::enableButtons( bool bEnable )
{
    m_bInterfaceLocked = ! bEnable;

    if ( bEnable )
    {
        sal_Int32 nIndex = getSelIndex();
        if ( nIndex != ExtensionBox_Impl::ENTRY_NOTFOUND )
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
                           weld::Window* pWindow)
    : m_pWindow(pWindow)
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
                                              TranslateId pResID,
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

bool DialogHelper::installExtensionWarn(std::u16string_view rExtensionName)
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

    m_nEventID = Application::PostUserEvent(rLink, pCaller);
}

//                             ExtMgrDialog
ExtMgrDialog::ExtMgrDialog(weld::Window *pParent, TheExtensionManager *pManager)
    : GenericDialogController(pParent, "desktop/ui/extensionmanager.ui", "ExtensionManagerDialog")
    , DialogHelper(pManager->getContext(), m_xDialog.get())
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
    , m_aIdle( "ExtMgrDialog m_aIdle TimeOutHdl" )
    , m_pManager(pManager)
    , m_xExtensionBox(new ExtBoxWithBtns_Impl(m_xBuilder->weld_scrolled_window("scroll", true)))
    , m_xExtensionBoxWnd(new weld::CustomWeld(*m_xBuilder, "extensions", *m_xExtensionBox))
    , m_xOptionsBtn(m_xBuilder->weld_button("optionsbtn"))
    , m_xAddBtn(m_xBuilder->weld_button("addbtn"))
    , m_xRemoveBtn(m_xBuilder->weld_button("removebtn"))
    , m_xEnableBtn(m_xBuilder->weld_button("enablebtn"))
    , m_xUpdateBtn(m_xBuilder->weld_button("updatebtn"))
    , m_xCloseBtn(m_xBuilder->weld_button("close"))
    , m_xBundledCbx(m_xBuilder->weld_check_button("bundled"))
    , m_xSharedCbx(m_xBuilder->weld_check_button("shared"))
    , m_xUserCbx(m_xBuilder->weld_check_button("user"))
    , m_xGetExtensions(m_xBuilder->weld_link_button("getextensions"))
    , m_xProgressText(m_xBuilder->weld_label("progressft"))
    , m_xProgressBar(m_xBuilder->weld_progress_bar("progressbar"))
    , m_xCancelBtn(m_xBuilder->weld_button("cancel"))
    , m_xSearchEntry(m_xBuilder->weld_entry("search"))
{
    m_xExtensionBox->InitFromDialog(this);

    m_xEnableBtn->set_help_id(HID_EXTENSION_MANAGER_LISTBOX_ENABLE);

    m_xOptionsBtn->connect_clicked( LINK( this, ExtMgrDialog, HandleOptionsBtn ) );
    m_xAddBtn->connect_clicked( LINK( this, ExtMgrDialog, HandleAddBtn ) );
    m_xRemoveBtn->connect_clicked( LINK( this, ExtMgrDialog, HandleRemoveBtn ) );
    m_xEnableBtn->connect_clicked( LINK( this, ExtMgrDialog, HandleEnableBtn ) );
    m_xCloseBtn->connect_clicked( LINK( this, ExtMgrDialog, HandleCloseBtn ) );

    m_xCancelBtn->connect_clicked( LINK( this, ExtMgrDialog, HandleCancelBtn ) );

    m_xBundledCbx->connect_toggled( LINK( this, ExtMgrDialog, HandleExtTypeCbx ) );
    m_xSharedCbx->connect_toggled( LINK( this, ExtMgrDialog, HandleExtTypeCbx ) );
    m_xUserCbx->connect_toggled( LINK( this, ExtMgrDialog, HandleExtTypeCbx ) );

    m_xSearchEntry->connect_changed( LINK( this, ExtMgrDialog, HandleSearch ) );

    m_xBundledCbx->set_active(true);
    m_xSharedCbx->set_active(true);
    m_xUserCbx->set_active(true);

    m_xProgressBar->hide();

#if ENABLE_EXTENSION_UPDATE
    m_xUpdateBtn->connect_clicked( LINK( this, ExtMgrDialog, HandleUpdateBtn ) );
    m_xUpdateBtn->set_sensitive(false);
#else
    m_xUpdateBtn->hide();
#endif

    if (officecfg::Office::ExtensionManager::ExtensionSecurity::DisableExtensionInstallation::get())
    {
        m_xAddBtn->set_sensitive(false);
        m_xAddBtn->set_tooltip_text(DpResId(RID_STR_WARNING_INSTALL_EXTENSION_DISABLED));
    }
    if (officecfg::Office::ExtensionManager::ExtensionSecurity::DisableExtensionRemoval::get())
    {
        m_xRemoveBtn->set_sensitive(false);
        m_xRemoveBtn->set_tooltip_text(DpResId(RID_STR_WARNING_REMOVE_EXTENSION_DISABLED));
    }

    m_aIdle.SetPriority(TaskPriority::LOWEST);
    m_aIdle.SetInvokeHandler( LINK( this, ExtMgrDialog, TimeOutHdl ) );
}

ExtMgrDialog::~ExtMgrDialog()
{
    m_aIdle.Stop();
}

void ExtMgrDialog::setGetExtensionsURL( const OUString &rURL )
{
    m_xGetExtensions->set_uri( rURL );
}

void ExtMgrDialog::addPackageToList( const uno::Reference< deployment::XPackage > &xPackage,
                                     bool bLicenseMissing )
{
    const SolarMutexGuard aGuard;
    m_xUpdateBtn->set_sensitive(true);

    bool bSearchMatch = m_xSearchEntry->get_text().isEmpty();
    if (!m_xSearchEntry->get_text().isEmpty()
        && xPackage->getDisplayName().toAsciiLowerCase().indexOf(
               m_xSearchEntry->get_text().toAsciiLowerCase())
               >= 0)
    {
        bSearchMatch = true;
    }

    if (!bSearchMatch)
        return;

    if (m_xBundledCbx->get_active() && (xPackage->getRepositoryName() == BUNDLED_PACKAGE_MANAGER) )
    {
        m_xExtensionBox->addEntry( xPackage, bLicenseMissing );
    }
    else if (m_xSharedCbx->get_active() && (xPackage->getRepositoryName() == SHARED_PACKAGE_MANAGER) )
    {
        m_xExtensionBox->addEntry( xPackage, bLicenseMissing );
    }
    else if (m_xUserCbx->get_active() && (xPackage->getRepositoryName() == USER_PACKAGE_MANAGER ))
    {
        m_xExtensionBox->addEntry( xPackage, bLicenseMissing );
    }
}

void ExtMgrDialog::updateList()
{
    // re-creates the list of packages with addEntry selecting the packages
    prepareChecking();
    m_pManager->createPackageList();
    checkEntries();
}

void ExtMgrDialog::prepareChecking()
{
    m_xExtensionBox->prepareChecking();
}

void ExtMgrDialog::checkEntries()
{
    const SolarMutexGuard guard;
    m_xExtensionBox->checkEntries();
}

bool ExtMgrDialog::removeExtensionWarn(std::u16string_view rExtensionName)
{
    const SolarMutexGuard guard;
    incBusy();
    std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_xDialog.get(),
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
        if (!continueOnSharedExtension(xPackage, m_xDialog.get(), RID_STR_WARNING_ENABLE_SHARED_EXTENSION, m_bEnableWarning))
            return;
    }
    else
    {
        if (!continueOnSharedExtension(xPackage, m_xDialog.get(), RID_STR_WARNING_DISABLE_SHARED_EXTENSION, m_bDisableWarning))
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

    if (!continueOnSharedExtension(xPackage, m_xDialog.get(), RID_STR_WARNING_REMOVE_SHARED_EXTENSION, m_bDeleteWarning))
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
    std::vector< css::uno::Reference< css::deployment::XPackage > > vEntries { extension };

    m_pManager->getCmdQueue()->checkForUpdates( std::move(vEntries) );
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
    sfx2::FileDialogHelper aDlgHelper(ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, FileDialogFlags::NONE, m_xDialog.get());
    aDlgHelper.SetContext(sfx2::FileDialogHelper::ExtensionManager);
    const uno::Reference<ui::dialogs::XFilePicker3>& xFilePicker = aDlgHelper.GetFilePicker();
    xFilePicker->setTitle( m_sAddPackages );

    // collect and set filter list:
    typedef std::map< OUString, OUString > t_string2string;
    t_string2string title2filter;
    OUStringBuffer supportedFilters;

    const uno::Sequence< uno::Reference< deployment::XPackageTypeInfo > > packageTypes(
        m_pManager->getExtensionManager()->getSupportedPackageTypes() );

    for ( uno::Reference< deployment::XPackageTypeInfo > const & xPackageType : packageTypes )
    {
        const OUString filter( xPackageType->getFileFilter() );
        if (!filter.isEmpty())
        {
            const OUString title( xPackageType->getShortDescription() );
            const std::pair< t_string2string::iterator, bool > insertion(
                title2filter.emplace( title, filter ) );
            if (!supportedFilters.isEmpty())
                supportedFilters.append(';');
            supportedFilters.append(filter);
            if ( ! insertion.second )
            { // already existing, append extensions:
                insertion.first->second = insertion.first->second +
                    ";" + filter;
            }
        }
    }

    static const OUString StrAllFiles = []()
        {
            const SolarMutexGuard guard;
            std::locale loc = Translate::Create("fps");
            return Translate::get(STR_FILTERNAME_ALL, loc);
        }();

    // All files at top:
    xFilePicker->appendFilter( StrAllFiles, "*.*" );
    xFilePicker->appendFilter( DpResId(RID_STR_ALL_SUPPORTED), supportedFilters.makeStringAndClear() );
    // then supported ones:
    for (auto const& elem : title2filter)
    {
        try
        {
            xFilePicker->appendFilter( elem.first, elem.second );
        }
        catch (const lang::IllegalArgumentException &)
        {
            TOOLS_WARN_EXCEPTION( "desktop", "" );
        }
    }
    xFilePicker->setCurrentFilter( DpResId(RID_STR_ALL_SUPPORTED) );

    if ( xFilePicker->execute() != ui::dialogs::ExecutableDialogResults::OK )
        return uno::Sequence<OUString>(); // cancelled

    uno::Sequence< OUString > files( xFilePicker->getSelectedFiles() );
    OSL_ASSERT( files.hasElements() );
    return files;
}

void ExtMgrDialog::enableOptionsButton( bool bEnable )
{
    m_xOptionsBtn->set_sensitive( bEnable );
}

void ExtMgrDialog::enableRemoveButton( bool bEnable )
{
    m_xRemoveBtn->set_sensitive( bEnable && !officecfg::Office::ExtensionManager::ExtensionSecurity::DisableExtensionRemoval::get());

    if (officecfg::Office::ExtensionManager::ExtensionSecurity::DisableExtensionRemoval::get())
    {
        m_xRemoveBtn->set_tooltip_text(DpResId(RID_STR_WARNING_REMOVE_EXTENSION_DISABLED));
    }
    else
    {
        m_xRemoveBtn->set_tooltip_text("");
    }
}

void ExtMgrDialog::enableEnableButton( bool bEnable )
{
    m_xEnableBtn->set_sensitive( bEnable );
}

void ExtMgrDialog::enableButtontoEnable( bool bEnable )
{
    if (bEnable)
    {
        m_xEnableBtn->set_label( DpResId( RID_CTX_ITEM_ENABLE ) );
        m_xEnableBtn->set_help_id( HID_EXTENSION_MANAGER_LISTBOX_ENABLE );
    }
    else
    {
        m_xEnableBtn->set_label( DpResId( RID_CTX_ITEM_DISABLE ) );
        m_xEnableBtn->set_help_id( HID_EXTENSION_MANAGER_LISTBOX_DISABLE );
    }
}

IMPL_LINK_NOARG(ExtMgrDialog, HandleCancelBtn, weld::Button&, void)
{
    if ( m_xAbortChannel.is() )
    {
        try
        {
            m_xAbortChannel->sendAbort();
        }
        catch ( const uno::RuntimeException & )
        {
            TOOLS_WARN_EXCEPTION( "dbaccess", "" );
        }
    }
}

IMPL_LINK_NOARG(ExtMgrDialog, HandleCloseBtn, weld::Button&, void)
{
    bool bCallClose = true;

    //only suggest restart if modified and this is the first close attempt
    if (!m_bClosed && m_pManager->isModified())
    {
        m_pManager->clearModified();

        //only suggest restart if we're actually running, e.g. not from standalone unopkg gui
        if (dp_misc::office_is_running())
        {
            SolarMutexGuard aGuard;
            bCallClose = !::svtools::executeRestartDialog(comphelper::getProcessComponentContext(),
                                                          m_xDialog.get(),
                                                          svtools::RESTART_REASON_EXTENSION_INSTALL);
        }
    }

    if (bCallClose)
        m_xDialog->response(RET_CANCEL);
}

IMPL_LINK( ExtMgrDialog, startProgress, void*, _bLockInterface, void )
{
    std::unique_lock aGuard( m_aMutex );
    bool bLockInterface = static_cast<bool>(_bLockInterface);

    if ( m_bStartProgress && !m_bHasProgress )
        m_aIdle.Start();

    if ( m_bStopProgress )
    {
        if ( m_xProgressBar->get_visible() )
            m_xProgressBar->set_percentage( 100 );
        m_xAbortChannel.clear();

        SAL_INFO( "desktop.deployment", " startProgress handler: stop" );
    }
    else
    {
        SAL_INFO( "desktop.deployment", " startProgress handler: start" );
    }

    m_xCancelBtn->set_sensitive( bLockInterface );
    m_xAddBtn->set_sensitive( !bLockInterface && !officecfg::Office::ExtensionManager::ExtensionSecurity::DisableExtensionInstallation::get());
    if (officecfg::Office::ExtensionManager::ExtensionSecurity::DisableExtensionInstallation::get())
    {
        m_xAddBtn->set_tooltip_text(DpResId(RID_STR_WARNING_INSTALL_EXTENSION_DISABLED));
    }
    else
    {
        m_xAddBtn->set_tooltip_text("");
    }

    m_xUpdateBtn->set_sensitive( !bLockInterface && m_xExtensionBox->getItemCount() );
    m_xExtensionBox->enableButtons( !bLockInterface );

    clearEventID();
}


void ExtMgrDialog::showProgress( bool _bStart )
{
    std::unique_lock aGuard( m_aMutex );

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


void ExtMgrDialog::updateProgress( const tools::Long nProgress )
{
    if ( m_nProgress != nProgress )
    {
        std::unique_lock aGuard( m_aMutex );
        m_nProgress = nProgress;
        m_aIdle.Start();
    }
}


void ExtMgrDialog::updateProgress( const OUString &rText,
                                   const uno::Reference< task::XAbortChannel > &xAbortChannel)
{
    std::unique_lock aGuard( m_aMutex );

    m_xAbortChannel = xAbortChannel;
    m_sProgressText = rText;
    m_bProgressChanged = true;
    m_aIdle.Start();
}


void ExtMgrDialog::updatePackageInfo( const uno::Reference< deployment::XPackage > &xPackage )
{
    const SolarMutexGuard aGuard;
    m_xExtensionBox->updateEntry( xPackage );
}

IMPL_LINK_NOARG(ExtMgrDialog, HandleOptionsBtn, weld::Button&, void)
{
    const sal_Int32 nActive = m_xExtensionBox->getSelIndex();

    if ( nActive != ExtensionBox_Impl::ENTRY_NOTFOUND )
    {
        SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();

        OUString sExtensionId = m_xExtensionBox->GetEntryData( nActive )->m_xPackage->getIdentifier().Value;
        ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateOptionsDialog(m_xDialog.get(), sExtensionId));

        pDlg->Execute();
    }
}

IMPL_LINK_NOARG(ExtMgrDialog, HandleAddBtn, weld::Button&, void)
{
    incBusy();

    uno::Sequence< OUString > aFileList = raiseAddPicker();

    if ( aFileList.hasElements() )
    {
        m_pManager->installPackage( aFileList[0] );
    }

    decBusy();
}

IMPL_LINK_NOARG(ExtMgrDialog, HandleRemoveBtn, weld::Button&, void)
{
    const sal_Int32 nActive = m_xExtensionBox->getSelIndex();

    if ( nActive != ExtensionBox_Impl::ENTRY_NOTFOUND )
    {
        TEntry_Impl pEntry = m_xExtensionBox->GetEntryData( nActive );
        removePackage( pEntry->m_xPackage );
    }
}

IMPL_LINK_NOARG(ExtMgrDialog, HandleEnableBtn, weld::Button&, void)
{
    const sal_Int32 nActive = m_xExtensionBox->getSelIndex();

    if ( nActive != ExtensionBox_Impl::ENTRY_NOTFOUND )
    {
        TEntry_Impl pEntry = m_xExtensionBox->GetEntryData( nActive );

        if ( pEntry->m_bMissingLic )
            acceptLicense( pEntry->m_xPackage );
        else
        {
            const bool bEnable( pEntry->m_eState != REGISTERED );
            enablePackage( pEntry->m_xPackage, bEnable );
        }
    }
}

IMPL_LINK_NOARG(ExtMgrDialog, HandleExtTypeCbx, weld::Toggleable&, void)
{
    updateList();
}

IMPL_LINK_NOARG(ExtMgrDialog, HandleSearch, weld::Entry&, void)
{
    updateList();
}

IMPL_LINK_NOARG(ExtMgrDialog, HandleUpdateBtn, weld::Button&, void)
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
        m_xProgressText->hide();
        m_xProgressBar->hide();
        m_xCancelBtn->hide();
    }
    else
    {
        if ( m_bProgressChanged )
        {
            m_bProgressChanged = false;
            m_xProgressText->set_label(m_sProgressText);
        }

        if ( m_bStartProgress )
        {
            m_bStartProgress = false;
            m_bHasProgress = true;
            m_xProgressBar->show();
            m_xProgressText->show();
            m_xCancelBtn->set_sensitive(true);
            m_xCancelBtn->show();
        }

        if ( m_xProgressBar->get_visible() )
            m_xProgressBar->set_percentage( static_cast<sal_uInt16>(m_nProgress) );
    }
}

void ExtMgrDialog::Close()
{
    m_pManager->terminateDialog();
    m_bClosed = true;
}

//UpdateRequiredDialog
UpdateRequiredDialog::UpdateRequiredDialog(weld::Window *pParent, TheExtensionManager *pManager)
    : GenericDialogController(pParent, "desktop/ui/updaterequireddialog.ui", "UpdateRequiredDialog")
    , DialogHelper(pManager->getContext(), m_xDialog.get())
    , m_sCloseText(DpResId(RID_STR_CLOSE_BTN))
    , m_bHasProgress(false)
    , m_bProgressChanged(false)
    , m_bStartProgress(false)
    , m_bStopProgress(false)
    , m_bHasLockedEntries(false)
    , m_nProgress(0)
    , m_aIdle( "UpdateRequiredDialog m_aIdle TimeOutHdl" )
    , m_pManager(pManager)
    , m_xExtensionBox(new ExtensionBox_Impl(m_xBuilder->weld_scrolled_window("scroll", true)))
    , m_xExtensionBoxWnd(new weld::CustomWeld(*m_xBuilder, "extensions", *m_xExtensionBox))
    , m_xUpdateNeeded(m_xBuilder->weld_label("updatelabel"))
    , m_xUpdateBtn(m_xBuilder->weld_button("ok"))
    , m_xCloseBtn(m_xBuilder->weld_button("disable"))
    , m_xCancelBtn(m_xBuilder->weld_button("cancel"))
    , m_xProgressText(m_xBuilder->weld_label("progresslabel"))
    , m_xProgressBar(m_xBuilder->weld_progress_bar("progress"))
{
    m_xExtensionBox->setExtensionManager(pManager);

    m_xUpdateBtn->connect_clicked( LINK( this, UpdateRequiredDialog, HandleUpdateBtn ) );
    m_xCloseBtn->connect_clicked( LINK( this, UpdateRequiredDialog, HandleCloseBtn ) );
    m_xCancelBtn->connect_clicked( LINK( this, UpdateRequiredDialog, HandleCancelBtn ) );

    OUString aText = m_xUpdateNeeded->get_label();
    aText = aText.replaceAll(
        "%PRODUCTNAME", utl::ConfigManager::getProductName());
    m_xUpdateNeeded->set_label(aText);

    m_xProgressBar->hide();
    m_xUpdateBtn->set_sensitive( false );
    m_xCloseBtn->grab_focus();

    m_aIdle.SetPriority( TaskPriority::LOWEST );
    m_aIdle.SetInvokeHandler( LINK( this, UpdateRequiredDialog, TimeOutHdl ) );
}

UpdateRequiredDialog::~UpdateRequiredDialog()
{
    m_aIdle.Stop();
}

void UpdateRequiredDialog::addPackageToList( const uno::Reference< deployment::XPackage > &xPackage,
                                             bool bLicenseMissing )
{
    // We will only add entries to the list with unsatisfied dependencies
    if ( !bLicenseMissing && !checkDependencies( xPackage ) )
    {
        m_bHasLockedEntries |= m_pManager->isReadOnly( xPackage );
        const SolarMutexGuard aGuard;
        m_xUpdateBtn->set_sensitive(true);
        m_xExtensionBox->addEntry( xPackage );
    }
}


void UpdateRequiredDialog::prepareChecking()
{
    m_xExtensionBox->prepareChecking();
}


void UpdateRequiredDialog::checkEntries()
{
    const SolarMutexGuard guard;
    m_xExtensionBox->checkEntries();

    if ( ! hasActiveEntries() )
    {
        m_xCloseBtn->set_label( m_sCloseText );
        m_xCloseBtn->grab_focus();
    }
}


IMPL_LINK_NOARG(UpdateRequiredDialog, HandleCancelBtn, weld::Button&, void)
{
    if ( m_xAbortChannel.is() )
    {
        try
        {
            m_xAbortChannel->sendAbort();
        }
        catch ( const uno::RuntimeException & )
        {
            TOOLS_WARN_EXCEPTION( "desktop", "" );
        }
    }
}


IMPL_LINK( UpdateRequiredDialog, startProgress, void*, _bLockInterface, void )
{
    std::unique_lock aGuard( m_aMutex );
    bool bLockInterface = static_cast<bool>(_bLockInterface);

    if ( m_bStartProgress && !m_bHasProgress )
        m_aIdle.Start();

    if ( m_bStopProgress )
    {
        if ( m_xProgressBar->get_visible() )
            m_xProgressBar->set_percentage( 100 );
        m_xAbortChannel.clear();
        SAL_INFO( "desktop.deployment", " startProgress handler: stop" );
    }
    else
    {
        SAL_INFO( "desktop.deployment", " startProgress handler: start" );
    }

    m_xCancelBtn->set_sensitive( bLockInterface );
    m_xUpdateBtn->set_sensitive( false );
    clearEventID();
}


void UpdateRequiredDialog::showProgress( bool _bStart )
{
    std::unique_lock aGuard( m_aMutex );

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


void UpdateRequiredDialog::updateProgress( const tools::Long nProgress )
{
    if ( m_nProgress != nProgress )
    {
        std::unique_lock aGuard( m_aMutex );
        m_nProgress = nProgress;
        m_aIdle.Start();
    }
}


void UpdateRequiredDialog::updateProgress( const OUString &rText,
                                           const uno::Reference< task::XAbortChannel > &xAbortChannel)
{
    std::unique_lock aGuard( m_aMutex );

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
        m_xExtensionBox->removeEntry( xPackage );
    else
        m_xExtensionBox->updateEntry( xPackage );

    if ( ! hasActiveEntries() )
    {
        m_xCloseBtn->set_label( m_sCloseText );
        m_xCloseBtn->grab_focus();
    }
}


IMPL_LINK_NOARG(UpdateRequiredDialog, HandleUpdateBtn, weld::Button&, void)
{
    std::unique_lock aGuard( m_aMutex );

    std::vector< uno::Reference< deployment::XPackage > > vUpdateEntries;
    sal_Int32 nCount = m_xExtensionBox->GetEntryCount();

    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        TEntry_Impl pEntry = m_xExtensionBox->GetEntryData( i );
        vUpdateEntries.push_back( pEntry->m_xPackage );
    }

    aGuard.unlock();

    m_pManager->getCmdQueue()->checkForUpdates( std::move(vUpdateEntries) );
}


IMPL_LINK_NOARG(UpdateRequiredDialog, HandleCloseBtn, weld::Button&, void)
{
    std::unique_lock aGuard( m_aMutex );

    if ( !isBusy() )
    {
        if ( m_bHasLockedEntries )
            m_xDialog->response(-1);
        else if ( hasActiveEntries() )
            disableAllEntries();
        else
            m_xDialog->response(RET_CANCEL);
    }
}


IMPL_LINK_NOARG(UpdateRequiredDialog, TimeOutHdl, Timer *, void)
{
    if ( m_bStopProgress )
    {
        m_bHasProgress = false;
        m_bStopProgress = false;
        m_xProgressText->hide();
        m_xProgressBar->hide();
        m_xCancelBtn->hide();
    }
    else
    {
        if ( m_bProgressChanged )
        {
            m_bProgressChanged = false;
            m_xProgressText->set_label( m_sProgressText );
        }

        if ( m_bStartProgress )
        {
            m_bStartProgress = false;
            m_bHasProgress = true;
            m_xProgressBar->show();
            m_xProgressText->show();
            m_xCancelBtn->set_sensitive(true);
            m_xCancelBtn->show();
        }

        if (m_xProgressBar->get_visible())
            m_xProgressBar->set_percentage(m_nProgress);
    }
}

// VCL::Dialog
short UpdateRequiredDialog::run()
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
        m_xUpdateNeeded->set_label( DpResId( RID_STR_NO_ADMIN_PRIVILEGE ) );
        m_xCloseBtn->set_label( DpResId( RID_STR_EXIT_BTN ) );
        m_xUpdateBtn->set_sensitive( false );
        m_xExtensionBox->RemoveUnlocked();
    }

    return GenericDialogController::run();
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
        TOOLS_WARN_EXCEPTION( "desktop", "" );
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
    std::unique_lock aGuard( m_aMutex );

    bool bRet = false;
    tools::Long nCount = m_xExtensionBox->GetEntryCount();
    for ( tools::Long nIndex = 0; nIndex < nCount; nIndex++ )
    {
        TEntry_Impl pEntry = m_xExtensionBox->GetEntryData( nIndex );

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
    std::unique_lock aGuard( m_aMutex );

    incBusy();

    tools::Long nCount = m_xExtensionBox->GetEntryCount();
    for ( tools::Long nIndex = 0; nIndex < nCount; nIndex++ )
    {
        TEntry_Impl pEntry = m_xExtensionBox->GetEntryData( nIndex );
        m_pManager->getCmdQueue()->enableExtension( pEntry->m_xPackage, false );
    }

    decBusy();

    if ( ! hasActiveEntries() )
        m_xCloseBtn->set_label( m_sCloseText );
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
                                                          uno::Reference< uno::XComponentContext > xComponentContext )
    : m_xComponentContext(std::move( xComponentContext ))
{
}

// XServiceInfo
OUString UpdateRequiredDialogService::getImplementationName()
{
    return "com.sun.star.comp.deployment.ui.UpdateRequiredDialog";
}

sal_Bool UpdateRequiredDialogService::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > UpdateRequiredDialogService::getSupportedServiceNames()
{
    return { "com.sun.star.deployment.ui.UpdateRequiredDialog" };
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
