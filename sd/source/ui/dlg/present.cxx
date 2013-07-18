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


SdStartPresentationDlg::SdStartPresentationDlg( Window* pWindow,
                                  const SfxItemSet& rInAttrs,
                                  const std::vector<String> &rPageNames, SdCustomShowList* pCSList ) :
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

    get( msExternalMonitor,     "externalmonitor_str"   );
    get( msMonitor,             "monitor_str"           );
    get( msAllMonitors,         "allmonitors_str"       );

    Link aLink( LINK( this, SdStartPresentationDlg, ChangeRangeHdl ) );

    aRbtAll->SetClickHdl( aLink );
    aRbtAtDia->SetClickHdl( aLink );
    aRbtCustomshow->SetClickHdl( aLink );

    aLink = LINK( this, SdStartPresentationDlg, ClickWindowPresentationHdl );
    aRbtStandard->SetClickHdl( aLink );
    aRbtWindow->SetClickHdl( aLink );
    aRbtAuto->SetClickHdl( aLink );

    aTmfPause->SetModifyHdl( LINK( this, SdStartPresentationDlg, ChangePauseHdl ) );
    aTmfPause->SetFormat( TIMEF_SEC );

    // fill Listbox with page names
    for (std::vector<String>::const_iterator pIter = rPageNames.begin(); pIter != rPageNames.end(); ++pIter)
        aLbDias->InsertEntry(*pIter);

    if( pCustomShowList )
    {
        sal_uInt16 nPosToSelect = (sal_uInt16) pCustomShowList->GetCurPos();
        SdCustomShow* pCustomShow;
        // fill Listbox with CustomShows
        for( pCustomShow = (SdCustomShow*) pCustomShowList->First();
             pCustomShow != NULL;
             pCustomShow = (SdCustomShow*) pCustomShowList->Next() )
        {
            aLbCustomshow->InsertEntry( pCustomShow->GetName() );
        }
        aLbCustomshow->SelectEntryPos( nPosToSelect );
        pCustomShowList->Seek( nPosToSelect );
    }
    else
        aRbtCustomshow->Disable();

    if( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_CUSTOMSHOW ) ).GetValue() && pCSList )
        aRbtCustomshow->Check();
    else if( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_ALL ) ).GetValue() )
        aRbtAll->Check();
    else
        aRbtAtDia->Check();

    aLbDias->SelectEntry( ( ( const SfxStringItem& ) rOutAttrs.Get( ATTR_PRESENT_DIANAME ) ).GetValue() );
    aCbxManuel->Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_MANUEL ) ).GetValue() );
    aCbxMousepointer->Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_MOUSE ) ).GetValue() );
    aCbxPen->Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_PEN ) ).GetValue() );
    aCbxNavigator->Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_NAVIGATOR ) ).GetValue() );
    aCbxAnimationAllowed->Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_ANIMATION_ALLOWED ) ).GetValue() );
    aCbxChangePage->Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_CHANGE_PAGE ) ).GetValue() );
    aCbxAlwaysOnTop->Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_ALWAYS_ON_TOP ) ).GetValue() );

    const sal_Bool  bEndless = ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_ENDLESS ) ).GetValue();
    const sal_Bool  bWindow = !( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_FULLSCREEN ) ).GetValue();
    const long  nPause = ( ( const SfxUInt32Item& ) rOutAttrs.Get( ATTR_PRESENT_PAUSE_TIMEOUT ) ).GetValue();

    aTmfPause->SetTime( Time( 0, 0, nPause ) );
    // set cursor in timefield
    Edit *pEdit = aTmfPause->GetField();
    Selection aSel( pEdit->GetMaxTextLen(), pEdit->GetMaxTextLen() );
    pEdit->SetSelection( aSel );

    aCbxAutoLogo->Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_SHOW_PAUSELOGO ) ).GetValue() );

    if( bWindow )
        aRbtWindow->Check( sal_True );
    else if( bEndless )
        aRbtAuto->Check( sal_True );
    else
        aRbtStandard->Check( sal_True );

    InitMonitorSettings();

    ChangeRangeHdl( this );

    ClickWindowPresentationHdl( NULL );
    ChangePauseHdl( NULL );
}

String SdStartPresentationDlg::GetDisplayName( sal_Int32 nDisplay, bool bExternal )
{
    String aName( bExternal ? msExternalMonitor->GetText() :
                  msMonitor->GetText() );
    const String aNumber( OUString::number( nDisplay ) );
    aName.SearchAndReplace( String("%1"), aNumber );
    return aName;
}

/// Store display index together with name in user data
sal_Int32 SdStartPresentationDlg::InsertDisplayEntry(const rtl::OUString &aName,
                                                     sal_Int32            nDisplay)
{
    maLBMonitor->InsertEntry( aName );
    const sal_uInt32 nEntryIndex = maLBMonitor->GetEntryCount() - 1;
    maLBMonitor->SetEntryData( nEntryIndex, (void*)(sal_IntPtr)nDisplay );

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
            sal_Bool bUnifiedDisplay = Application::IsUnifiedDisplay();
            sal_Int32 nExternalIndex = Application::GetDisplayExternalScreen();

            sal_Int32 nSelectedIndex (-1);
            sal_Int32 nDefaultExternalIndex (-1);
            const sal_Int32 nDefaultSelectedDisplay (
                ( ( const SfxInt32Item& ) rOutAttrs.Get( ATTR_PRESENT_DISPLAY ) ).GetValue());

            // Un-conditionally add a version for '0' the default external display
            sal_Int32 nInsertedEntry;

            // FIXME: string-freeze this should really be 'External (display %)'
            String aName = GetDisplayName( nExternalIndex + 1, true);
            nInsertedEntry = InsertDisplayEntry( aName, 0 );
            if( nDefaultSelectedDisplay == 0)
                nSelectedIndex = nInsertedEntry;

            // The user data contains the real setting
            for( sal_Int32 nDisplay = 0; nDisplay < mnMonitors; nDisplay++ )
            {
                bool bIsExternal = nDisplay == nExternalIndex;
                // FIXME: string-freeze, use true to denote external for now
                bIsExternal = false;
                aName = GetDisplayName( nDisplay + 1, bIsExternal );
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

            maLBMonitor->SelectEntryPos((sal_uInt16)nSelectedIndex);
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

    sal_uInt16 nPos = maLBMonitor->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        rAttr.Put( SfxInt32Item ( ATTR_PRESENT_DISPLAY, (sal_Int32)(sal_IntPtr)maLBMonitor->GetEntryData(nPos)) );

    nPos = aLbCustomshow->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        pCustomShowList->Seek( nPos );
}

/**
 *      Handler: Enabled/Disabled Listbox "Dias"
 */
IMPL_LINK_NOARG(SdStartPresentationDlg, ChangeRangeHdl)
{
    aLbDias->Enable( aRbtAtDia->IsChecked() );
    aLbCustomshow->Enable( aRbtCustomshow->IsChecked() );

    return( 0L );
}

/**
 *      Handler: Enabled/Disabled Checkbox "AlwaysOnTop"
 */
IMPL_LINK_NOARG(SdStartPresentationDlg, ClickWindowPresentationHdl)
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
        aCbxAlwaysOnTop->Enable( sal_False );
        aCbxAlwaysOnTop->Check( sal_False );
    }
    else
        aCbxAlwaysOnTop->Enable();

    return( 0L );
}

/**
 *      Handler: Enabled/Disabled Checkbox "AlwaysOnTop"
 */
IMPL_LINK_NOARG(SdStartPresentationDlg, ChangePauseHdl)
{
    aCbxAutoLogo->Enable( aRbtAuto->IsChecked() && ( aTmfPause->GetTime().GetMSFromTime() > 0 ) );
    return( 0L );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
