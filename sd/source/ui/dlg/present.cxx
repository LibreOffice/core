/*************************************************************************
 *
 *  $RCSfile: present.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:33 $
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

#pragma hdrstop

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

#include "sdattr.hxx"
#include "present.hxx"
#include "present.hrc"
#include "sdresid.hxx"
#include "cusshow.hxx"

/*************************************************************************
|* Ctor
\************************************************************************/
SdStartPresentationDlg::SdStartPresentationDlg( Window* pWindow,
                                  const SfxItemSet& rInAttrs,
                                  List& rPageNames, List* pCSList ) :
                ModalDialog     ( pWindow, SdResId( DLG_START_PRESENTATION ) ),
                aRbtAll                 ( this, SdResId( RBT_ALL ) ),
                aRbtAtDia               ( this, SdResId( RBT_AT_DIA ) ),
                aLbDias                 ( this, SdResId( LB_DIAS ) ),
                aRbtCustomshow          ( this, SdResId( RBT_CUSTOMSHOW ) ),
                aLbCustomshow           ( this, SdResId( LB_CUSTOMSHOW ) ),
                aGrpRange               ( this, SdResId( GRP_RANGE ) ),
                aRbtStandard            ( this, SdResId( RBT_STANDARD ) ),
                aRbtWindow              ( this, SdResId( RBT_WINDOW ) ),
                aRbtAuto                ( this, SdResId( RBT_AUTO ) ),
//              aFtPause                ( this, SdResId( FT_PAUSE ) ),
                aTmfPause               ( this, SdResId( TMF_PAUSE ) ),
                aCbxAutoLogo            ( this, SdResId( CBX_AUTOLOGO ) ),
                aGrpKind                ( this, SdResId( GRP_KIND ) ),
                aCbxManuel              ( this, SdResId( CBX_MANUEL ) ),
                aCbxMousepointer        ( this, SdResId( CBX_MOUSEPOINTER ) ),
                aCbxPen                 ( this, SdResId( CBX_PEN ) ),
                aCbxNavigator           ( this, SdResId( CBX_NAVIGATOR ) ),
                aCbxAnimationAllowed    ( this, SdResId( CBX_ANIMATION_ALLOWED ) ),
                aCbxChangePage          ( this, SdResId( CBX_CHANGE_PAGE ) ),
                aCbxAlwaysOnTop         ( this, SdResId( CBX_ALWAYS_ON_TOP ) ),
                aGrpOptions             ( this, SdResId( GRP_OPTIONS ) ),
                aBtnOK                  ( this, SdResId( BTN_OK ) ),
                aBtnCancel              ( this, SdResId( BTN_CANCEL ) ),
                aBtnHelp                ( this, SdResId( BTN_HELP ) ),
                pCustomShowList         ( pCSList ),
                rOutAttrs               ( rInAttrs )
{
    FreeResource();

    Link aLink( LINK( this, SdStartPresentationDlg, ChangeRangeHdl ) );

    aRbtAll.SetClickHdl( aLink );
    aRbtAtDia.SetClickHdl( aLink );
    aRbtCustomshow.SetClickHdl( aLink );
    aCbxMousepointer.SetClickHdl( LINK( this, SdStartPresentationDlg, ChangeMousepointerHdl ) );

    aLink = LINK( this, SdStartPresentationDlg, ClickWindowPresentationHdl );
    aRbtStandard.SetClickHdl( aLink );
    aRbtWindow.SetClickHdl( aLink );
    aRbtAuto.SetClickHdl( aLink );

    aTmfPause.SetModifyHdl( LINK( this, SdStartPresentationDlg, ChangePauseHdl ) );
    aTmfPause.SetFormat( TIMEF_SEC );

    // Listbox mit Seitennamen fuellen
    rPageNames.First();
    for( UINT16 i = 0;
         i < rPageNames.Count();
         i++ )
    {
        aLbDias.InsertEntry( *( String* ) rPageNames.GetCurObject() );
        rPageNames.Next();
    }

    if( pCustomShowList )
    {
        USHORT nPosToSelect = pCustomShowList->GetCurPos();
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

    const BOOL  bEndless = ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_ENDLESS ) ).GetValue();
    const BOOL  bWindow = !( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_FULLSCREEN ) ).GetValue();
    const long  nPause = ( ( const SfxUInt32Item& ) rOutAttrs.Get( ATTR_PRESENT_PAUSE_TIMEOUT ) ).GetValue();

    aTmfPause.SetTime( Time( 0, 0, nPause ) );
    // set cursor in timefield
    Edit *pEdit = aTmfPause.GetField();
    Selection aSel( pEdit->GetMaxTextLen(), pEdit->GetMaxTextLen() );
    pEdit->SetSelection( aSel );

    aCbxAutoLogo.Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_SHOW_PAUSELOGO ) ).GetValue() );

    if( bWindow )
        aRbtWindow.Check( TRUE );
    else if( bEndless )
        aRbtAuto.Check( TRUE );
    else
        aRbtStandard.Check( TRUE );

    /// NEU
    BOOL bStartWithActualPage = ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_START_ACTUAL_PAGE ) ).GetValue();
    if( bStartWithActualPage )
    {
        aRbtAll.Enable( FALSE );
        aRbtAtDia.Check();
        aRbtAtDia.Enable( FALSE );
        aLbDias.Enable( FALSE );
        aLbCustomshow.Enable( FALSE );
        aGrpRange.Enable( FALSE );
    }
    else
        ChangeRangeHdl( this );

    ChangeMousepointerHdl( this );
    ClickWindowPresentationHdl( NULL );
    ChangePauseHdl( NULL );
}

/*************************************************************************
|* Setzt die ausgewaehlten Attribute des Dialogs
\************************************************************************/
void SdStartPresentationDlg::GetAttr( SfxItemSet& rOutAttrs )
{
    rOutAttrs.Put( SfxBoolItem ( ATTR_PRESENT_ALL, aRbtAll.IsChecked() ) );
    rOutAttrs.Put( SfxBoolItem ( ATTR_PRESENT_CUSTOMSHOW, aRbtCustomshow.IsChecked() ) );
    rOutAttrs.Put( SfxStringItem ( ATTR_PRESENT_DIANAME, aLbDias.GetSelectEntry() ) );
    rOutAttrs.Put( SfxBoolItem ( ATTR_PRESENT_MANUEL, aCbxManuel.IsChecked() ) );
    rOutAttrs.Put( SfxBoolItem ( ATTR_PRESENT_MOUSE, aCbxMousepointer.IsChecked() ) );
    rOutAttrs.Put( SfxBoolItem ( ATTR_PRESENT_PEN, aCbxPen.IsChecked() ) );
    rOutAttrs.Put( SfxBoolItem ( ATTR_PRESENT_NAVIGATOR, aCbxNavigator.IsChecked() ) );
    rOutAttrs.Put( SfxBoolItem ( ATTR_PRESENT_ANIMATION_ALLOWED, aCbxAnimationAllowed.IsChecked() ) );
    rOutAttrs.Put( SfxBoolItem ( ATTR_PRESENT_CHANGE_PAGE, aCbxChangePage.IsChecked() ) );
    rOutAttrs.Put( SfxBoolItem ( ATTR_PRESENT_ALWAYS_ON_TOP, aCbxAlwaysOnTop.IsChecked() ) );
    rOutAttrs.Put( SfxBoolItem ( ATTR_PRESENT_FULLSCREEN, !aRbtWindow.IsChecked() ) );
    rOutAttrs.Put( SfxBoolItem ( ATTR_PRESENT_ENDLESS, aRbtAuto.IsChecked() ) );
    rOutAttrs.Put( SfxUInt32Item ( ATTR_PRESENT_PAUSE_TIMEOUT, aTmfPause.GetTime().GetMSFromTime() / 1000 ) );
    rOutAttrs.Put( SfxBoolItem ( ATTR_PRESENT_SHOW_PAUSELOGO, aCbxAutoLogo.IsChecked() ) );

    USHORT nPos = aLbCustomshow.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        pCustomShowList->Seek( nPos );
}

/*************************************************************************
|*      Handler: Enabled/Disabled Listbox "Dias"
\************************************************************************/
IMPL_LINK( SdStartPresentationDlg, ChangeRangeHdl, void *, EMPTYARG )
{
    aLbDias.Enable( aRbtAtDia.IsChecked() );
    aLbCustomshow.Enable( aRbtCustomshow.IsChecked() );

    return( 0L );
}

/*************************************************************************
|*      Handler: Enabled/Disabled Checkbox "Pen"
\************************************************************************/
IMPL_LINK( SdStartPresentationDlg, ChangeMousepointerHdl, void *, EMPTYARG )
{
    if( aCbxMousepointer.IsChecked() )
        aCbxPen.Enable();
    else
    {
        aCbxPen.Enable( FALSE );
        aCbxPen.Check( FALSE );
    }

    return( 0L );
}

/*************************************************************************
|*      Handler: Enabled/Disabled Checkbox "AlwaysOnTop"
\************************************************************************/
IMPL_LINK( SdStartPresentationDlg, ClickWindowPresentationHdl, void *, EMPTYARG )
{
    const BOOL bAuto = aRbtAuto.IsChecked();

    // aFtPause.Enable( bAuto );
    aTmfPause.Enable( bAuto );
    aCbxAutoLogo.Enable( bAuto && ( aTmfPause.GetTime().GetMSFromTime() > 0UL ) );

    if( aRbtWindow.IsChecked() )
    {
        aCbxAlwaysOnTop.Enable( FALSE );
        aCbxAlwaysOnTop.Check( FALSE );
    }
    else
        aCbxAlwaysOnTop.Enable();

    return( 0L );
}

/*************************************************************************
|*      Handler: Enabled/Disabled Checkbox "AlwaysOnTop"
\************************************************************************/
IMPL_LINK( SdStartPresentationDlg, ChangePauseHdl, void *, EMPTYARG )
{
    aCbxAutoLogo.Enable( aTmfPause.GetTime().GetMSFromTime() > 0UL );
    return( 0L );
}


