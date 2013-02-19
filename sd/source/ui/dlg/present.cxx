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


#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <comphelper/processfactory.hxx>
#include <svl/itemset.hxx>
#include <vcl/svapp.hxx>

#include "sdattr.hxx"
#include "present.hxx"
#include "present.hrc"
#include "sdresid.hxx"
#include "cusshow.hxx"
#include "customshowlist.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

/*************************************************************************
|* Ctor
\************************************************************************/
SdStartPresentationDlg::SdStartPresentationDlg( Window* pWindow,
                                  const SfxItemSet& rInAttrs,
                                  const std::vector<String> &rPageNames, SdCustomShowList* pCSList ) :
                ModalDialog     ( pWindow, SdResId( DLG_START_PRESENTATION ) ),
                aGrpRange               ( this, SdResId( GRP_RANGE ) ),
                aRbtAll                 ( this, SdResId( RBT_ALL ) ),
                aRbtAtDia               ( this, SdResId( RBT_AT_DIA ) ),
                aRbtCustomshow          ( this, SdResId( RBT_CUSTOMSHOW ) ),
                aLbDias                 ( this, SdResId( LB_DIAS ) ),
                aLbCustomshow           ( this, SdResId( LB_CUSTOMSHOW ) ),

                aGrpKind                ( this, SdResId( GRP_KIND ) ),
                aRbtStandard            ( this, SdResId( RBT_STANDARD ) ),
                aRbtWindow              ( this, SdResId( RBT_WINDOW ) ),
                aRbtAuto                ( this, SdResId( RBT_AUTO ) ),
                aTmfPause               ( this, SdResId( TMF_PAUSE ) ),
                aCbxAutoLogo            ( this, SdResId( CBX_AUTOLOGO ) ),

                aGrpOptions             ( this, SdResId( GRP_OPTIONS ) ),
                aCbxManuel              ( this, SdResId( CBX_MANUEL ) ),
                aCbxMousepointer        ( this, SdResId( CBX_MOUSEPOINTER ) ),
                aCbxPen                 ( this, SdResId( CBX_PEN ) ),
                aCbxNavigator           ( this, SdResId( CBX_NAVIGATOR ) ),
                aCbxAnimationAllowed    ( this, SdResId( CBX_ANIMATION_ALLOWED ) ),
                aCbxChangePage          ( this, SdResId( CBX_CHANGE_PAGE ) ),
                aCbxAlwaysOnTop         ( this, SdResId( CBX_ALWAYS_ON_TOP ) ),

                maGrpMonitor            ( this, SdResId( GRP_MONITOR ) ),
                maFtMonitor             ( this, SdResId( FT_MONITOR ) ),
                maLBMonitor             ( this, SdResId( LB_MONITOR ) ),

                aBtnOK                  ( this, SdResId( BTN_OK ) ),
                aBtnCancel              ( this, SdResId( BTN_CANCEL ) ),
                aBtnHelp                ( this, SdResId( BTN_HELP ) ),

                pCustomShowList         ( pCSList ),
                rOutAttrs               ( rInAttrs ),
                mnMonitors              ( 0 ),

                msExternalMonitor( SdResId(STR_EXTERNAL_MONITOR ) ),
                msMonitor( SdResId( STR_MONITOR ) ),
                msAllMonitors( SdResId( STR_ALL_MONITORS ) )
{
    FreeResource();

    Link aLink( LINK( this, SdStartPresentationDlg, ChangeRangeHdl ) );

    aRbtAll.SetClickHdl( aLink );
    aRbtAtDia.SetClickHdl( aLink );
    aRbtCustomshow.SetClickHdl( aLink );

    aLink = LINK( this, SdStartPresentationDlg, ClickWindowPresentationHdl );
    aRbtStandard.SetClickHdl( aLink );
    aRbtWindow.SetClickHdl( aLink );
    aRbtAuto.SetClickHdl( aLink );

    aTmfPause.SetModifyHdl( LINK( this, SdStartPresentationDlg, ChangePauseHdl ) );
    aTmfPause.SetFormat( TIMEF_SEC );

    aLbDias.SetAccessibleRelationLabeledBy( &aRbtAtDia );
    aLbDias.SetAccessibleName(aRbtAtDia.GetText());
    aLbCustomshow.SetAccessibleRelationLabeledBy( &aRbtCustomshow );
    aTmfPause.SetAccessibleRelationLabeledBy( &aRbtAuto );
    aTmfPause.SetAccessibleName(aRbtAuto.GetText());

    // Listbox mit Seitennamen fuellen
    for (std::vector<String>::const_iterator pIter = rPageNames.begin(); pIter != rPageNames.end(); ++pIter)
        aLbDias.InsertEntry(*pIter);

    if( pCustomShowList )
    {
        sal_uInt16 nPosToSelect = (sal_uInt16) pCustomShowList->GetCurPos();
        SdCustomShow* pCustomShow;
        // Listbox mit CustomShows fuellen
        for( pCustomShow = (SdCustomShow*) pCustomShowList->First();
             pCustomShow != NULL;
             pCustomShow = (SdCustomShow*) pCustomShowList->Next() )
        {
            aLbCustomshow.InsertEntry( pCustomShow->GetName() );
        }
        aLbCustomshow.SelectEntryPos( nPosToSelect );
        pCustomShowList->Seek( nPosToSelect );
    }
    else
        aRbtCustomshow.Disable();

    if( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_CUSTOMSHOW ) ).GetValue() && pCSList )
        aRbtCustomshow.Check();
    else if( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_ALL ) ).GetValue() )
        aRbtAll.Check();
    else
        aRbtAtDia.Check();

    aLbDias.SelectEntry( ( ( const SfxStringItem& ) rOutAttrs.Get( ATTR_PRESENT_DIANAME ) ).GetValue() );
    aCbxManuel.Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_MANUEL ) ).GetValue() );
    aCbxMousepointer.Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_MOUSE ) ).GetValue() );
    aCbxPen.Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_PEN ) ).GetValue() );
    aCbxNavigator.Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_NAVIGATOR ) ).GetValue() );
    aCbxAnimationAllowed.Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_ANIMATION_ALLOWED ) ).GetValue() );
    aCbxChangePage.Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_CHANGE_PAGE ) ).GetValue() );
    aCbxAlwaysOnTop.Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_ALWAYS_ON_TOP ) ).GetValue() );

    const sal_Bool  bEndless = ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_ENDLESS ) ).GetValue();
    const sal_Bool  bWindow = !( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_FULLSCREEN ) ).GetValue();
    const long  nPause = ( ( const SfxUInt32Item& ) rOutAttrs.Get( ATTR_PRESENT_PAUSE_TIMEOUT ) ).GetValue();

    aTmfPause.SetTime( Time( 0, 0, nPause ) );
    // set cursor in timefield
    Edit *pEdit = aTmfPause.GetField();
    Selection aSel( pEdit->GetMaxTextLen(), pEdit->GetMaxTextLen() );
    pEdit->SetSelection( aSel );

    aCbxAutoLogo.Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_SHOW_PAUSELOGO ) ).GetValue() );

    if( bWindow )
        aRbtWindow.Check( sal_True );
    else if( bEndless )
        aRbtAuto.Check( sal_True );
    else
        aRbtStandard.Check( sal_True );

    InitMonitorSettings();

    ChangeRangeHdl( this );

    ClickWindowPresentationHdl( NULL );
    ChangePauseHdl( NULL );
}

void SdStartPresentationDlg::InitMonitorSettings()
{
    try
    {
        maGrpMonitor.Show( true );
        maFtMonitor.Show( true );
        maLBMonitor.Show( true );

        mnMonitors = Application::GetScreenCount();

        if( mnMonitors <= 1 )
        {
            maFtMonitor.Enable( false );
            maLBMonitor.Enable( false );
        }
        else
        {
            sal_Bool bUnifiedDisplay = Application::IsUnifiedDisplay();
            sal_Int32 nExternalIndex = Application::GetDisplayExternalScreen();

            sal_Int32 nSelectedIndex (-1);
            sal_Int32 nDefaultExternalIndex (-1);
            const sal_Int32 nDefaultSelectedDisplay (
                ( ( const SfxInt32Item& ) rOutAttrs.Get( ATTR_PRESENT_DISPLAY ) ).GetValue());
            const String sPlaceHolder( RTL_CONSTASCII_USTRINGPARAM( "%1" ) );
            for( sal_Int32 nDisplay = 0; nDisplay < mnMonitors; nDisplay++ )
            {
                String aName( nDisplay == nExternalIndex ? msExternalMonitor : msMonitor );
                const String aNumber( OUString::number( nDisplay + 1 ) );
                aName.SearchAndReplace( sPlaceHolder, aNumber );
                maLBMonitor.InsertEntry( aName );

                // Store display index together with name.
                const sal_uInt32 nEntryIndex (maLBMonitor.GetEntryCount()-1);
                maLBMonitor.SetEntryData(nEntryIndex, (void*)(sal_IntPtr)nDisplay);

                // Remember the index of the default selection.
                if (nDefaultSelectedDisplay == nDisplay)
                    nSelectedIndex = nEntryIndex;

                // Remember index of the default display.
                if (nDisplay == nExternalIndex)
                    nDefaultExternalIndex = nEntryIndex;
            }

            if( bUnifiedDisplay )
            {
                maLBMonitor.InsertEntry( msAllMonitors );
                const sal_uInt32 nEntryIndex (maLBMonitor.GetEntryCount()-1);
                maLBMonitor.SetEntryData(nEntryIndex, (void*)-1);
                if (nDefaultSelectedDisplay == -1)
                    nSelectedIndex = nEntryIndex;
            }

            if (nSelectedIndex < 0)
            {
                if (nExternalIndex < 0)
                    nSelectedIndex = 0;
                else
                    nSelectedIndex = nDefaultExternalIndex;
            }

            maLBMonitor.SelectEntryPos((sal_uInt16)nSelectedIndex);
        }
    }
    catch( Exception& )
    {
    }
}

/*************************************************************************
|* Setzt die ausgewaehlten Attribute des Dialogs
\************************************************************************/
void SdStartPresentationDlg::GetAttr( SfxItemSet& rAttr )
{
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_ALL, aRbtAll.IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_CUSTOMSHOW, aRbtCustomshow.IsChecked() ) );
    rAttr.Put( SfxStringItem ( ATTR_PRESENT_DIANAME, aLbDias.GetSelectEntry() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_MANUEL, aCbxManuel.IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_MOUSE, aCbxMousepointer.IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_PEN, aCbxPen.IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_NAVIGATOR, aCbxNavigator.IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_ANIMATION_ALLOWED, aCbxAnimationAllowed.IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_CHANGE_PAGE, aCbxChangePage.IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_ALWAYS_ON_TOP, aCbxAlwaysOnTop.IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_FULLSCREEN, !aRbtWindow.IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_ENDLESS, aRbtAuto.IsChecked() ) );
    rAttr.Put( SfxUInt32Item ( ATTR_PRESENT_PAUSE_TIMEOUT, aTmfPause.GetTime().GetMSFromTime() / 1000 ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_SHOW_PAUSELOGO, aCbxAutoLogo.IsChecked() ) );

    sal_uInt16 nPos = maLBMonitor.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        rAttr.Put( SfxInt32Item ( ATTR_PRESENT_DISPLAY, (sal_Int32)(sal_IntPtr)maLBMonitor.GetEntryData(nPos)) );

    nPos = aLbCustomshow.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        pCustomShowList->Seek( nPos );
}

/*************************************************************************
|*      Handler: Enabled/Disabled Listbox "Dias"
\************************************************************************/
IMPL_LINK_NOARG(SdStartPresentationDlg, ChangeRangeHdl)
{
    aLbDias.Enable( aRbtAtDia.IsChecked() );
    aLbCustomshow.Enable( aRbtCustomshow.IsChecked() );

    return( 0L );
}

/*************************************************************************
|*      Handler: Enabled/Disabled Checkbox "AlwaysOnTop"
\************************************************************************/
IMPL_LINK_NOARG(SdStartPresentationDlg, ClickWindowPresentationHdl)
{
    const bool bAuto = aRbtAuto.IsChecked();
    const bool bWindow = aRbtWindow.IsChecked();

    // aFtPause.Enable( bAuto );
    aTmfPause.Enable( bAuto );
    aCbxAutoLogo.Enable( bAuto && ( aTmfPause.GetTime().GetMSFromTime() > 0 ) );

    const bool bDisplay = !bWindow && ( mnMonitors > 1 );
    maFtMonitor.Enable( bDisplay );
    maLBMonitor.Enable( bDisplay );

    if( bWindow )
    {
        aCbxAlwaysOnTop.Enable( sal_False );
        aCbxAlwaysOnTop.Check( sal_False );
    }
    else
        aCbxAlwaysOnTop.Enable();

    return( 0L );
}

/*************************************************************************
|*      Handler: Enabled/Disabled Checkbox "AlwaysOnTop"
\************************************************************************/
IMPL_LINK_NOARG(SdStartPresentationDlg, ChangePauseHdl)
{
    aCbxAutoLogo.Enable( aRbtAuto.IsChecked() && ( aTmfPause.GetTime().GetMSFromTime() > 0 ) );
    return( 0L );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
