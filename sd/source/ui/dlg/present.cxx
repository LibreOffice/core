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

#include <officecfg/Office/Impress.hxx>
#include <officecfg/Office/Security.hxx>
#include <svl/itemset.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <vcl/svapp.hxx>

#include <sdattr.hrc>
#include <present.hxx>
#include <cusshow.hxx>
#include <customshowlist.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

namespace
{
enum PresenterConsoleMode
{
    FullScreen = 0,
    Windowed = 1,
    Disabled = 2
};
}

SdStartPresentationDlg::SdStartPresentationDlg(weld::Window* pWindow, const SfxItemSet& rInAttrs,
                                  const std::vector<OUString> &rPageNames, SdCustomShowList* pCSList)
    : GenericDialogController(pWindow, "modules/simpress/ui/presentationdialog.ui", "PresentationDialog")
    , pCustomShowList(pCSList)
    , rOutAttrs(rInAttrs)
    , mnMonitors(0)
    , m_xRbtAll(m_xBuilder->weld_radio_button("allslides"))
    , m_xRbtAtDia(m_xBuilder->weld_radio_button("from"))
    , m_xRbtCustomshow(m_xBuilder->weld_radio_button("customslideshow"))
    , m_xLbDias(m_xBuilder->weld_combo_box("from_cb"))
    , m_xLbCustomshow(m_xBuilder->weld_combo_box("customslideshow_cb"))
    , m_xRbtStandard(m_xBuilder->weld_radio_button("default"))
    , m_xRbtWindow(m_xBuilder->weld_radio_button("window"))
    , m_xRbtAuto(m_xBuilder->weld_radio_button("auto"))
    , m_xTmfPause(m_xBuilder->weld_formatted_spin_button("pauseduration"))
    , m_xFormatter(new weld::TimeFormatter(*m_xTmfPause))
    , m_xCbxAutoLogo(m_xBuilder->weld_check_button("showlogo"))
    , m_xCbxManuel(m_xBuilder->weld_check_button("manualslides"))
    , m_xCbxMousepointer(m_xBuilder->weld_check_button("pointervisible"))
    , m_xCbxPen(m_xBuilder->weld_check_button("pointeraspen"))
    , m_xCbxAnimationAllowed(m_xBuilder->weld_check_button("animationsallowed"))
    , m_xCbxChangePage(m_xBuilder->weld_check_button("changeslidesbyclick"))
    , m_xCbxAlwaysOnTop(m_xBuilder->weld_check_button("alwaysontop"))
    , m_xCbxShowNavigationButton(m_xBuilder->weld_check_button("shownavigationbutton"))
    , m_xLbNavigationButtonsSize(m_xBuilder->weld_combo_box("navigation_buttons_size_cb"))
    , m_xFtNavigationButtonsSize(m_xBuilder->weld_label("navbar_btn_size_label"))
    , m_xFrameEnableRemote(m_xBuilder->weld_frame("frameremote"))
    , m_xCbxEnableRemote(m_xBuilder->weld_check_button("enableremote"))
    , m_xCbxEnableRemoteInsecure(m_xBuilder->weld_check_button("enableremoteinsecure"))
    , m_xCbxInteractiveMode(m_xBuilder->weld_check_button("enableinteractivemode"))
    , m_xLbConsole(m_xBuilder->weld_combo_box("console_cb"))
    , m_xFtMonitor(m_xBuilder->weld_label("presdisplay_label"))
    , m_xLBMonitor(m_xBuilder->weld_combo_box("presdisplay_cb"))
    , m_xMonitor(m_xBuilder->weld_label("monitor_str"))
    , m_xAllMonitors(m_xBuilder->weld_label("allmonitors_str"))
    , m_xMonitorExternal(m_xBuilder->weld_label("externalmonitor_str"))
    , m_xExternal(m_xBuilder->weld_label("external_str"))
{
    m_xFormatter->SetExtFormat(ExtTimeFieldFormat::LongDuration);
    m_xFormatter->EnableEmptyField(false);

    Link<weld::Toggleable&,void> aLink( LINK( this, SdStartPresentationDlg, ChangeRangeHdl ) );

    m_xRbtAll->connect_toggled( aLink );
    m_xRbtAtDia->connect_toggled( aLink );
    m_xRbtCustomshow->connect_toggled( aLink );

    aLink = LINK( this, SdStartPresentationDlg, ClickWindowPresentationHdl );
    m_xRbtStandard->connect_toggled( aLink );
    m_xRbtWindow->connect_toggled( aLink );
    m_xRbtAuto->connect_toggled( aLink );
    m_xCbxShowNavigationButton->connect_toggled( aLink );

    m_xTmfPause->connect_value_changed( LINK( this, SdStartPresentationDlg, ChangePauseHdl ) );

    // fill Listbox with page names
    for (const auto& rPageName : rPageNames)
        m_xLbDias->append_text(rPageName);

    if( pCustomShowList )
    {
        sal_uInt16 nPosToSelect = pCustomShowList->GetCurPos();
        SdCustomShow* pCustomShow;
        // fill Listbox with CustomShows
        for( pCustomShow = pCustomShowList->First();
             pCustomShow != nullptr;
             pCustomShow = pCustomShowList->Next() )
        {
            m_xLbCustomshow->append_text( pCustomShow->GetName() );
        }
        m_xLbCustomshow->set_active( nPosToSelect );
        pCustomShowList->Seek( nPosToSelect );
    }
    else
        m_xRbtCustomshow->set_sensitive(false);

    if( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_CUSTOMSHOW ) ).GetValue() && pCSList )
        m_xRbtCustomshow->set_active(true);
    else if( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_ALL ) ).GetValue() )
        m_xRbtAll->set_active(true);
    else
        m_xRbtAtDia->set_active(true);

    m_xLbDias->set_active_text( rOutAttrs.Get( ATTR_PRESENT_DIANAME ).GetValue() );
    m_xCbxManuel->set_active( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_MANUEL ) ).GetValue() );
    m_xCbxMousepointer->set_active( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_MOUSE ) ).GetValue() );
    m_xCbxPen->set_active( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_PEN ) ).GetValue() );
    m_xCbxAnimationAllowed->set_active( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_ANIMATION_ALLOWED ) ).GetValue() );
    m_xCbxChangePage->set_active( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_CHANGE_PAGE ) ).GetValue() );
    m_xCbxAlwaysOnTop->set_active( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_ALWAYS_ON_TOP ) ).GetValue() );

    const sal_Int32 nActiveNavigationBtnScale = officecfg::Office::Impress::Layout::Display::NavigationBtnScale::get();
    const bool bShowNavbar = officecfg::Office::Impress::Misc::Start::ShowNavigationPanel::get();
    m_xCbxShowNavigationButton->set_active( bShowNavbar );
    if (nActiveNavigationBtnScale != -1)
    {
        m_xLbNavigationButtonsSize->set_active(nActiveNavigationBtnScale);
    }
    m_xLbNavigationButtonsSize->set_sensitive( bShowNavbar );
    m_xFtNavigationButtonsSize->set_sensitive( bShowNavbar );

    const bool  bEndless = static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_ENDLESS ) ).GetValue();
    const bool  bWindow = !static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_FULLSCREEN ) ).GetValue();
    const tools::Long  nPause = rOutAttrs.Get( ATTR_PRESENT_PAUSE_TIMEOUT ).GetValue();

    m_xFormatter->SetTime( tools::Time( 0, 0, nPause ) );
    // set cursor in timefield to end
    m_xTmfPause->set_position(-1);

    m_xCbxAutoLogo->set_active( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_SHOW_PAUSELOGO ) ).GetValue() );

    if( bWindow )
        m_xRbtWindow->set_active(true);
    else if( bEndless )
        m_xRbtAuto->set_active(true);
    else
        m_xRbtStandard->set_active(true);

    if (!officecfg::Office::Impress::Misc::Start::EnablePresenterScreen::get())
        m_xLbConsole->set_active(PresenterConsoleMode::Disabled);
    else if (officecfg::Office::Impress::Misc::Start::PresenterScreenFullScreen::get())
        m_xLbConsole->set_active(PresenterConsoleMode::FullScreen);
    else
        m_xLbConsole->set_active(PresenterConsoleMode::Windowed);

#ifdef ENABLE_SDREMOTE
    m_xCbxEnableRemote->connect_toggled( LINK(this, SdStartPresentationDlg, ChangeRemoteHdl) );
    m_xCbxEnableRemote->set_active(officecfg::Office::Impress::Misc::Start::EnableSdremote::get());
    ChangeRemoteHdl(*m_xCbxEnableRemote);
    m_xCbxEnableRemoteInsecure->set_active(m_xCbxEnableRemote->get_active()
        && officecfg::Office::Security::Net::AllowInsecureImpressRemoteWiFi::get());
#else
    m_xFrameEnableRemote->hide();
#endif

    m_xCbxInteractiveMode->set_active( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_INTERACTIVE ) ).GetValue() );

    InitMonitorSettings();

    ChangeRangeHdl(*m_xRbtCustomshow);

    ClickWindowPresentationHdl(*m_xRbtStandard);
    ChangePause();
}

SdStartPresentationDlg::~SdStartPresentationDlg()
{
}

short SdStartPresentationDlg::run()
{
    short nRet = GenericDialogController::run();
    if (nRet == RET_OK)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> batch(
            comphelper::ConfigurationChanges::create());
        auto nActive = m_xLbConsole->get_active();
        bool bEnabled = nActive != PresenterConsoleMode::Disabled;
        officecfg::Office::Impress::Misc::Start::EnablePresenterScreen::set(bEnabled, batch);
        if (bEnabled)
        {
            officecfg::Office::Impress::Misc::Start::PresenterScreenFullScreen::set(
                nActive == PresenterConsoleMode::FullScreen, batch);
        }
        officecfg::Office::Impress::Misc::Start::ShowNavigationPanel::set(
            m_xCbxShowNavigationButton->get_active(), batch);
        officecfg::Office::Impress::Layout::Display::NavigationBtnScale::set(
            m_xLbNavigationButtonsSize->get_active(), batch);

#ifdef ENABLE_SDREMOTE
        officecfg::Office::Impress::Misc::Start::EnableSdremote::set(m_xCbxEnableRemote->get_active(), batch);
        officecfg::Office::Security::Net::AllowInsecureImpressRemoteWiFi::set(m_xCbxEnableRemoteInsecure->get_active(), batch);
#endif
        batch->commit();
    }
    return nRet;
}

OUString SdStartPresentationDlg::GetDisplayName( sal_Int32   nDisplay,
                                                 DisplayType eType )
{
    OUString aName;

    switch ( eType )
    {
    case EXTERNAL_IS_NUMBER:
        aName = m_xExternal->get_label();
        break;
    case MONITOR_IS_EXTERNAL:
        aName = m_xMonitorExternal->get_label();
        break;
    default:
    case MONITOR_NORMAL:
        aName = m_xMonitor->get_label();
        break;
    }
    aName = aName.replaceFirst( "%1", OUString::number( nDisplay ) );

    return aName;
}

/// Store display index together with name in user data
sal_Int32 SdStartPresentationDlg::InsertDisplayEntry(const OUString &aName,
                                                     sal_Int32            nDisplay)
{
    m_xLBMonitor->append(OUString::number(nDisplay), aName);
    return m_xLBMonitor->get_count() - 1;
}

void SdStartPresentationDlg::InitMonitorSettings()
{
    try
    {
        m_xFtMonitor->show();
        m_xLBMonitor->show();

        mnMonitors = Application::GetScreenCount();

        if( mnMonitors <= 1 )
        {
            m_xFtMonitor->set_sensitive( false );
            m_xLBMonitor->set_sensitive( false );
        }
        else
        {
            sal_Int32 nExternalIndex = Application::GetDisplayExternalScreen();

            sal_Int32 nSelectedIndex (-1);
            sal_Int32 nDefaultExternalIndex (-1);
            const sal_Int32 nDefaultSelectedDisplay (
                rOutAttrs.Get( ATTR_PRESENT_DISPLAY ).GetValue());

            // Un-conditionally add a version for '0' the default external display
            sal_Int32 nInsertedEntry;

            // Initial entry - the auto-detected external monitor
            OUString aName = GetDisplayName( nExternalIndex + 1, EXTERNAL_IS_NUMBER);
            nInsertedEntry = InsertDisplayEntry( aName, 0 );
            if( nDefaultSelectedDisplay == 0)
                nSelectedIndex = nInsertedEntry;

            // The user data contains the real setting
            for( sal_Int32 nDisplay = 0; nDisplay < mnMonitors; nDisplay++ )
            {
                aName = GetDisplayName( nDisplay + 1,
                                        nDisplay == nExternalIndex ?
                                        MONITOR_IS_EXTERNAL : MONITOR_NORMAL );
                nInsertedEntry = InsertDisplayEntry( aName, nDisplay + 1 );

                // Remember the index of the default selection.
                if( nDisplay + 1 == nDefaultSelectedDisplay )
                    nSelectedIndex = nInsertedEntry;

                // Remember index of the default display.
                if( nDisplay == nExternalIndex )
                    nDefaultExternalIndex = nInsertedEntry;
            }

            nInsertedEntry = InsertDisplayEntry( m_xAllMonitors->get_label(), -1 );
            if( nDefaultSelectedDisplay == -1 )
                nSelectedIndex = nInsertedEntry;

            if (nSelectedIndex < 0)
            {
                if (nExternalIndex < 0)
                    nSelectedIndex = 0;
                else
                    nSelectedIndex = nDefaultExternalIndex;
            }

            m_xLBMonitor->set_active(nSelectedIndex);
        }
    }
    catch( Exception& )
    {
    }
}

/**
 * sets the selected attributes of the dialog
 */
void SdStartPresentationDlg::GetAttr( SfxItemSet& rAttr )
{
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_ALL, m_xRbtAll->get_active() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_CUSTOMSHOW, m_xRbtCustomshow->get_active() ) );
    rAttr.Put( SfxStringItem ( ATTR_PRESENT_DIANAME, m_xLbDias->get_active_text() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_MANUEL, m_xCbxManuel->get_active() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_MOUSE, m_xCbxMousepointer->get_active() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_PEN, m_xCbxPen->get_active() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_ANIMATION_ALLOWED, m_xCbxAnimationAllowed->get_active() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_CHANGE_PAGE, m_xCbxChangePage->get_active() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_ALWAYS_ON_TOP, m_xCbxAlwaysOnTop->get_active() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_FULLSCREEN, !m_xRbtWindow->get_active() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_ENDLESS, m_xRbtAuto->get_active() ) );
    rAttr.Put( SfxUInt32Item ( ATTR_PRESENT_PAUSE_TIMEOUT, m_xFormatter->GetTime().GetMSFromTime() / 1000 ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_SHOW_PAUSELOGO, m_xCbxAutoLogo->get_active() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_INTERACTIVE, m_xCbxInteractiveMode->get_active() ) );

    int nPos = m_xLBMonitor->get_active();
    if (nPos != -1)
        rAttr.Put(SfxInt32Item(ATTR_PRESENT_DISPLAY, m_xLBMonitor->get_id(nPos).toInt32()));

    nPos = m_xLbCustomshow->get_active();
    if (nPos != -1)
        pCustomShowList->Seek( nPos );
}

IMPL_LINK_NOARG(SdStartPresentationDlg, ChangeRemoteHdl, weld::Toggleable&, void)
{
    m_xCbxEnableRemoteInsecure->set_sensitive(m_xCbxEnableRemote->get_active());
}

/**
 *      Handler: Enabled/Disabled Listbox "Dias"
 */
IMPL_LINK_NOARG(SdStartPresentationDlg, ChangeRangeHdl, weld::Toggleable&, void)
{
    m_xLbDias->set_sensitive( m_xRbtAtDia->get_active() );
    m_xLbCustomshow->set_sensitive( m_xRbtCustomshow->get_active() );
}

/**
 *      Handler: Enabled/Disabled Checkbox "AlwaysOnTop"
 */
IMPL_LINK_NOARG(SdStartPresentationDlg, ClickWindowPresentationHdl, weld::Toggleable&, void)
{
    const bool bAuto = m_xRbtAuto->get_active();
    const bool bWindow = m_xRbtWindow->get_active();

    m_xTmfPause->set_sensitive( bAuto );
    m_xCbxAutoLogo->set_sensitive( bAuto && ( m_xFormatter->GetTime().GetMSFromTime() > 0 ) );

    const bool bDisplay = !bWindow && ( mnMonitors > 1 );
    m_xFtMonitor->set_sensitive( bDisplay );
    m_xLBMonitor->set_sensitive( bDisplay );

    const bool bShowNavbar = m_xCbxShowNavigationButton->get_active();
    m_xLbNavigationButtonsSize->set_sensitive( bShowNavbar );
    m_xFtNavigationButtonsSize->set_sensitive( bShowNavbar );

    if( bWindow )
    {
        m_xCbxAlwaysOnTop->set_sensitive(false);
        m_xCbxAlwaysOnTop->set_active(false);
    }
    else
        m_xCbxAlwaysOnTop->set_sensitive(true);
}

/**
 *      Handler: Enabled/Disabled Checkbox "AlwaysOnTop"
 */
IMPL_LINK_NOARG(SdStartPresentationDlg, ChangePauseHdl, weld::FormattedSpinButton&, void)
{
    ChangePause();
}

void SdStartPresentationDlg::ChangePause()
{
    m_xCbxAutoLogo->set_sensitive(m_xRbtAuto->get_active() && ( m_xFormatter->GetTime().GetMSFromTime() > 0 ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
