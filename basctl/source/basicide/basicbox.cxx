/*************************************************************************
 *
 *  $RCSfile: basicbox.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mh $ $Date: 2000-09-29 11:02:35 $
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

#include <ide_pch.hxx>

#pragma hdrstop

#include <basidesh.hrc>
#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>
#include <basobj.hxx>

#include <basicbox.hxx>
#include <iderid.hxx>
#include <bastypes.hxx>

SFX_IMPL_TOOLBOX_CONTROL( LibBoxControl, SfxStringItem );

LibBoxControl::LibBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind )
    : SfxToolBoxControl( nId, rTbx, rBind )
{
}



LibBoxControl::~LibBoxControl()
{
}



void LibBoxControl::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    BasicLibBox* pBox = (BasicLibBox*) GetToolBox().GetItemWindow( GetId() );

    DBG_ASSERT( pBox, "Box not found" );
    if ( !pBox )
        return;

    if ( eState != SFX_ITEM_AVAILABLE )
        pBox->Disable();
    else
    {
        pBox->Enable();

        if ( pState->ISA(SfxStringItem) )
            pBox->Update( (const SfxStringItem*)pState );
        else
            pBox->Update( NULL );
    }
}



Window* LibBoxControl::CreateItemWindow( Window *pParent )
{
    return new BasicLibBox( pParent );
}





BasicLibBox::BasicLibBox( Window* pParent ) :
    ListBox( pParent, WinBits( WB_BORDER | WB_DROPDOWN ) )
{
    FillBox();
    bIgnoreSelect = TRUE;   // Select von 0 noch nicht weiterleiten
    bFillBox = TRUE;
    SelectEntryPos( 0 );
    aCurText = GetEntry( 0 );
    SetSizePixel( Size( 165, 200 ) );
    bIgnoreSelect = FALSE;
    StartListening( *SFX_APP(), TRUE /* Nur einmal anmelden */ );
}



__EXPORT BasicLibBox::~BasicLibBox()
{
}

void __EXPORT BasicLibBox::Update( const SfxStringItem* pItem )
{
    // Immer auf dem laufenden sein...
//  if ( !pItem  || !pItem->GetValue().Len() )
        FillBox();

    if ( pItem )
    {
        aCurText = pItem->GetValue();
        if ( aCurText.Len() == 0 )
            aCurText = String( IDEResId( RID_STR_ALL ) );
    }

    if ( GetSelectEntry() != aCurText )
        SelectEntry( aCurText );
}

void __EXPORT BasicLibBox::ReleaseFocus()
{
    SfxViewShell* pCurSh = SfxViewShell::Current();
    DBG_ASSERT( pCurSh, "Current ViewShell not found!" );

    if ( pCurSh )
    {
        Window* pShellWin = pCurSh->GetWindow();
        if ( !pShellWin )       // sonst werde ich ihn nicht los
            pShellWin = Application::GetDefModalDialogParent();

        pShellWin->GrabFocus();
    }
}



void __EXPORT BasicLibBox::SFX_NOTIFY(  SfxBroadcaster& rBC, const TypeId&,
                                        const SfxHint& rHint, const TypeId& )
{
    if ( rHint.IsA( TYPE( SfxEventHint ) ) )
    {
        if ( ( ((SfxEventHint&)rHint).GetEventId() == SFX_EVENT_CREATEDOC ) ||
             ( ((SfxEventHint&)rHint).GetEventId() == SFX_EVENT_OPENDOC ) )
        {
            FillBox();  // IDE reagiert selbst, wenn == aktuelle Lib
        }
        else if ( ((SfxEventHint&)rHint).GetEventId() == SFX_EVENT_SAVEASDOC )
        {
            // Wenn das aktuell angezeigte Doc umbenannt wird...
            // Funktioniert nicht, da ich den alten Namen nicht bekomme
//          String aCurLib( GetLibFromMgrAndLib( aCurText ) );
//          String aCurMgr( GetMgrFromMgrAndLib( aCurText ) );
//          String aCurDoc = SfxObjectShell::Current()->GetTitle();
//          if ( aCurMgr == aCurDoc )
//          {
//              FillBox( FALSE );
//              aCurText = CreateMgrAndLibStr( aCurDoc, aCurLib );
//              bIgnoreSelect = TRUE;
//              SelectEntry( aCurText );
//              InfoBox( 0, aCurText ).Execute();
//              bIgnoreSelect = FALSE;
//          }
//          else
                FillBox( TRUE );

        }
        else if ( ((SfxEventHint&)rHint).GetEventId() == SFX_EVENT_CLOSEDOC )
        {
            if ( SFX_APP()->IsInBasicCall() )   // Nicht wenn Office beendet
                FillBox();
        }
    }
}



void BasicLibBox::FillBox( BOOL bSelect )
{
    SetUpdateMode( FALSE );
    bIgnoreSelect = TRUE;

    aCurText = GetSelectEntry();
    USHORT nCurPos = GetSelectEntryPos();

    SelectEntryPos( 0 );
    Clear();

    SvStrings* pStrings = BasicIDE::CreateBasicLibBoxEntries();
    for ( USHORT n = 0; n < pStrings->Count(); n++ )
    {
        String* pStr = pStrings->GetObject( n );
        InsertEntry( *pStr, LISTBOX_APPEND );
    }
    pStrings->DeleteAndDestroy( 0, pStrings->Count() );
    delete pStrings;

    SetUpdateMode( TRUE );

    if ( bSelect )
    {
        SelectEntry( aCurText );
        if ( !GetSelectEntryCount() )
        {
            SelectEntryPos( GetEntryCount() );  // gibst es nicht => leer?
            aCurText = GetSelectEntry();
        }
    }
    bIgnoreSelect = FALSE;
}

long BasicLibBox::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        KeyEvent aKeyEvt = *rNEvt.GetKeyEvent();
        USHORT nKeyCode = aKeyEvt.GetKeyCode().GetCode();
        switch( nKeyCode )
        {
            case KEY_RETURN:
            {
                NotifyIDE();
                nDone = 1;
            }
            break;

            case KEY_ESCAPE:
            {
                SelectEntry( aCurText );
                ReleaseFocus();
                nDone = 1;
            }
            break;
        }
    }
    else if( rNEvt.GetType() == EVENT_GETFOCUS )
    {
        if ( bFillBox )
        {
            FillBox();
            bFillBox = FALSE;
        }
    }
    else if( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( !HasChildPathFocus( TRUE ) )
        {
            bIgnoreSelect = TRUE;
            bFillBox = TRUE;
        }
    }

    return nDone ? nDone : ListBox::PreNotify( rNEvt );
}

void __EXPORT BasicLibBox::Select()
{
    if ( !IsTravelSelect() )
    {
        if ( !bIgnoreSelect )
            NotifyIDE();
        else
            SelectEntry( aCurText );    // Seit 306... (Select nach Escape)
    }
}



void BasicLibBox::NotifyIDE()
{
    String aLib;
    if ( GetSelectEntryPos() )
        aLib = GetSelectEntry();

    SfxStringItem aLibName( SID_BASICIDE_ARG_LIBNAME, aLib );
    SfxViewFrame* pCurFrame = SfxViewFrame::Current();
    DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
    SfxDispatcher* pDispatcher = pCurFrame ? pCurFrame->GetDispatcher() : NULL;
    if( pDispatcher )
    {
        pDispatcher->Execute( SID_BASICIDE_LIBSELECTED,
                              SFX_CALLMODE_SYNCHRON, &aLibName, 0L );
    }
    ReleaseFocus();
}



