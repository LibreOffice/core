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

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <comphelper/processfactory.hxx>
#include <svl/itemset.hxx>
#include <vcl/svapp.hxx>

#include "sdattr.hxx"
#include "present.hxx"
#include "sdresid.hxx"
#include "cusshow.hxx"
#include "customshowlist.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

SdStartPresentationDlg::SdStartPresentationDlg( vcl::Window* pWindow,
                                  const SfxItemSet& rInAttrs,
                                  const std::vector<OUString> &rPageNames, SdCustomShowList* pCSList ) :
                ModalDialog     ( pWindow, "PresentationDialog", "modules/simpress/ui/presentationdialog.ui" ),
                pCustomShowList         ( pCSList ),
                rOutAttrs               ( rInAttrs ),
                mnMonitors              ( 0 )
{
    get( aRbtAll,               "allslides"             );
    get( aRbtAtDia,             "from"                  );
    get( aRbtCustomshow,        "customslideshow"       );
    get( aLbDias,               "from_cb"               );
    get( aLbCustomshow,         "customslideshow_cb"    );

    get( aRbtStandard,          "default"               );
    get( aRbtWindow,            "window"                );
    get( aRbtAuto,              "auto"                  );
    get( aTmfPause,             "pauseduration"         );
    get( aCbxAutoLogo,          "showlogo"              );

    get( aCbxManuel,            "manualslides"          );
    get( aCbxMousepointer,      "pointervisible"        );
    get( aCbxPen,               "pointeraspen"          );
    get( aCbxNavigator,         "navigatorvisible"      );
    get( aCbxAnimationAllowed,  "animationsallowed"     );
    get( aCbxChangePage,        "changeslidesbyclick"   );
    get( aCbxAlwaysOnTop,       "alwaysontop"           );

    get( maFtMonitor,           "presdisplay_label"     );
    get( maLBMonitor,           "presdisplay_cb"        );

    get( msMonitor,             "monitor_str"           );
    get( msMonitorExternal,     "externalmonitor_str"   );
    get( msExternal,            "external_str"          );
    get( msMonitor,             "monitor_str"           );
    get( msAllMonitors,         "allmonitors_str"       );

    Link<Button*,void> aLink( LINK( this, SdStartPresentationDlg, ChangeRangeHdl ) );

    aRbtAll->SetClickHdl( aLink );
    aRbtAtDia->SetClickHdl( aLink );
    aRbtCustomshow->SetClickHdl( aLink );

    aLink = LINK( this, SdStartPresentationDlg, ClickWindowPresentationHdl );
    aRbtStandard->SetClickHdl( aLink );
    aRbtWindow->SetClickHdl( aLink );
    aRbtAuto->SetClickHdl( aLink );

    aTmfPause->SetModifyHdl( LINK( this, SdStartPresentationDlg, ChangePauseHdl ) );
    aTmfPause->SetFormat( TimeFieldFormat::F_SEC );

    // fill Listbox with page names
    for (std::vector<OUString>::const_iterator pIter = rPageNames.begin(); pIter != rPageNames.end(); ++pIter)
        aLbDias->InsertEntry(*pIter);

    if( pCustomShowList )
    {
        sal_uInt16 nPosToSelect = (sal_uInt16) pCustomShowList->GetCurPos();
        SdCustomShow* pCustomShow;
        // fill Listbox with CustomShows
        for( pCustomShow = pCustomShowList->First();
             pCustomShow != NULL;
             pCustomShow = pCustomShowList->Next() )
        {
            aLbCustomshow->InsertEntry( pCustomShow->GetName() );
        }
        aLbCustomshow->SelectEntryPos( nPosToSelect );
        pCustomShowList->Seek( nPosToSelect );
    }
    else
        aRbtCustomshow->Disable();

    if( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_CUSTOMSHOW ) ).GetValue() && pCSList )
        aRbtCustomshow->Check();
    else if( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_ALL ) ).GetValue() )
        aRbtAll->Check();
    else
        aRbtAtDia->Check();

    aLbDias->SelectEntry( static_cast<const SfxStringItem&>( rOutAttrs.Get( ATTR_PRESENT_DIANAME ) ).GetValue() );
    aCbxManuel->Check( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_MANUEL ) ).GetValue() );
    aCbxMousepointer->Check( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_MOUSE ) ).GetValue() );
    aCbxPen->Check( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_PEN ) ).GetValue() );
    aCbxNavigator->Check( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_NAVIGATOR ) ).GetValue() );
    aCbxAnimationAllowed->Check( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_ANIMATION_ALLOWED ) ).GetValue() );
    aCbxChangePage->Check( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_CHANGE_PAGE ) ).GetValue() );
    aCbxAlwaysOnTop->Check( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_ALWAYS_ON_TOP ) ).GetValue() );

    const bool  bEndless = static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_ENDLESS ) ).GetValue();
    const bool  bWindow = !static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_FULLSCREEN ) ).GetValue();
    const long  nPause = static_cast<const SfxUInt32Item&>( rOutAttrs.Get( ATTR_PRESENT_PAUSE_TIMEOUT ) ).GetValue();

    aTmfPause->SetTime( tools::Time( 0, 0, nPause ) );
    // set cursor in timefield
    Edit *pEdit = aTmfPause->GetField();
    Selection aSel( pEdit->GetMaxTextLen(), pEdit->GetMaxTextLen() );
    pEdit->SetSelection( aSel );

    aCbxAutoLogo->Check( static_cast<const SfxBoolItem&>( rOutAttrs.Get( ATTR_PRESENT_SHOW_PAUSELOGO ) ).GetValue() );

    if( bWindow )
        aRbtWindow->Check();
    else if( bEndless )
        aRbtAuto->Check();
    else
        aRbtStandard->Check();

    InitMonitorSettings();

    ChangeRangeHdl( NULL );

    ClickWindowPresentationHdl( NULL );
    ChangePauseHdl( *aTmfPause );
}

SdStartPresentationDlg::~SdStartPresentationDlg()
{
    disposeOnce();
}

void SdStartPresentationDlg::dispose()
{
    aRbtAll.clear();
    aRbtAtDia.clear();
    aRbtCustomshow.clear();
    aLbDias.clear();
    aLbCustomshow.clear();
    aRbtStandard.clear();
    aRbtWindow.clear();
    aRbtAuto.clear();
    aTmfPause.clear();
    aCbxAutoLogo.clear();
    aCbxManuel.clear();
    aCbxMousepointer.clear();
    aCbxPen.clear();
    aCbxNavigator.clear();
    aCbxAnimationAllowed.clear();
    aCbxChangePage.clear();
    aCbxAlwaysOnTop.clear();
    maFtMonitor.clear();
    maLBMonitor.clear();
    msMonitor.clear();
    msAllMonitors.clear();
    msMonitorExternal.clear();
    msExternal.clear();
    ModalDialog::dispose();
}

OUString SdStartPresentationDlg::GetDisplayName( sal_Int32   nDisplay,
                                                 DisplayType eType )
{
    OUString aName;

    switch ( eType )
    {
    case EXTERNAL_IS_NUMBER:
        aName = msExternal->GetText();
        break;
    case MONITOR_IS_EXTERNAL:
        aName = msMonitorExternal->GetText();
        break;
    default:
    case MONITOR_NORMAL:
        aName = msMonitor->GetText();
        break;
    }
    aName = aName.replaceFirst( "%1", OUString::number( nDisplay ) );

    return aName;
}

/// Store display index together with name in user data
sal_Int32 SdStartPresentationDlg::InsertDisplayEntry(const rtl::OUString &aName,
                                                     sal_Int32            nDisplay)
{
    maLBMonitor->InsertEntry( aName );
    const sal_uInt32 nEntryIndex = maLBMonitor->GetEntryCount() - 1;
    maLBMonitor->SetEntryData( nEntryIndex, reinterpret_cast<void*>((sal_IntPtr)nDisplay) );

    return nEntryIndex;
}

void SdStartPresentationDlg::InitMonitorSettings()
{
    try
    {
        maFtMonitor->Show( true );
        maLBMonitor->Show( true );

        mnMonitors = Application::GetScreenCount();

        if( mnMonitors <= 1 )
        {
            maFtMonitor->Enable( false );
            maLBMonitor->Enable( false );
        }
        else
        {
            bool bUnifiedDisplay = Application::IsUnifiedDisplay();
            sal_Int32 nExternalIndex = Application::GetDisplayExternalScreen();

            sal_Int32 nSelectedIndex (-1);
            sal_Int32 nDefaultExternalIndex (-1);
            const sal_Int32 nDefaultSelectedDisplay (
                static_cast<const SfxInt32Item&>( rOutAttrs.Get( ATTR_PRESENT_DISPLAY ) ).GetValue());

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

            if( bUnifiedDisplay )
            {
                nInsertedEntry = InsertDisplayEntry( msAllMonitors->GetText(), -1 );
                if( nDefaultSelectedDisplay == -1 )
                    nSelectedIndex = nInsertedEntry;
            }

            if (nSelectedIndex < 0)
            {
                if (nExternalIndex < 0)
                    nSelectedIndex = 0;
                else
                    nSelectedIndex = nDefaultExternalIndex;
            }

            maLBMonitor->SelectEntryPos(nSelectedIndex);
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
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_ALL, aRbtAll->IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_CUSTOMSHOW, aRbtCustomshow->IsChecked() ) );
    rAttr.Put( SfxStringItem ( ATTR_PRESENT_DIANAME, aLbDias->GetSelectEntry() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_MANUEL, aCbxManuel->IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_MOUSE, aCbxMousepointer->IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_PEN, aCbxPen->IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_NAVIGATOR, aCbxNavigator->IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_ANIMATION_ALLOWED, aCbxAnimationAllowed->IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_CHANGE_PAGE, aCbxChangePage->IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_ALWAYS_ON_TOP, aCbxAlwaysOnTop->IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_FULLSCREEN, !aRbtWindow->IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_ENDLESS, aRbtAuto->IsChecked() ) );
    rAttr.Put( SfxUInt32Item ( ATTR_PRESENT_PAUSE_TIMEOUT, aTmfPause->GetTime().GetMSFromTime() / 1000 ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_SHOW_PAUSELOGO, aCbxAutoLogo->IsChecked() ) );

    sal_Int32 nPos = maLBMonitor->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        rAttr.Put( SfxInt32Item ( ATTR_PRESENT_DISPLAY, (sal_Int32)reinterpret_cast<sal_IntPtr>(maLBMonitor->GetEntryData(nPos))) );

    nPos = aLbCustomshow->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        pCustomShowList->Seek( nPos );
}

/**
 *      Handler: Enabled/Disabled Listbox "Dias"
 */
IMPL_LINK_NOARG_TYPED(SdStartPresentationDlg, ChangeRangeHdl, Button*, void)
{
    aLbDias->Enable( aRbtAtDia->IsChecked() );
    aLbCustomshow->Enable( aRbtCustomshow->IsChecked() );
}

/**
 *      Handler: Enabled/Disabled Checkbox "AlwaysOnTop"
 */
IMPL_LINK_NOARG_TYPED(SdStartPresentationDlg, ClickWindowPresentationHdl, Button*, void)
{
    const bool bAuto = aRbtAuto->IsChecked();
    const bool bWindow = aRbtWindow->IsChecked();

    // aFtPause.Enable( bAuto );
    aTmfPause->Enable( bAuto );
    aCbxAutoLogo->Enable( bAuto && ( aTmfPause->GetTime().GetMSFromTime() > 0 ) );

    const bool bDisplay = !bWindow && ( mnMonitors > 1 );
    maFtMonitor->Enable( bDisplay );
    maLBMonitor->Enable( bDisplay );

    if( bWindow )
    {
        aCbxAlwaysOnTop->Enable( false );
        aCbxAlwaysOnTop->Check( false );
    }
    else
        aCbxAlwaysOnTop->Enable();
}

/**
 *      Handler: Enabled/Disabled Checkbox "AlwaysOnTop"
 */
IMPL_LINK_NOARG_TYPED(SdStartPresentationDlg, ChangePauseHdl, Edit&, void)
{
    aCbxAutoLogo->Enable( aRbtAuto->IsChecked() && ( aTmfPause->GetTime().GetMSFromTime() > 0 ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
