/*************************************************************************
 *
 *  $RCSfile: scenwnd.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-22 18:55:47 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svtools/slstitm.hxx>
#include <svtools/stritem.hxx>
#include <vcl/msgbox.hxx>

#include "navipi.hxx"
#include "popmenu.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "globstr.hrc"

//------------------------------------------------------------------------


//========================================================================
// class ScScenarioWindow ------------------------------------------------
//========================================================================

// -----------------------------------------------------------------------

ScScenarioListBox::ScScenarioListBox( Window* pParent )
    :   ListBox ( pParent, WB_BORDER ),
        rParent ( (ScScenarioWindow&)*pParent ),
        pAccel  ( NULL )
{
    Font aFont( GetFont() );
    aFont.SetTransparent( TRUE );
    aFont.SetWeight( WEIGHT_LIGHT );
    SetFont( aFont );
}

// -----------------------------------------------------------------------

__EXPORT ScScenarioListBox::~ScScenarioListBox()
{
    ClearEntryList();
    delete pAccel;
}

// -----------------------------------------------------------------------

void ScScenarioListBox::UpdateEntries( List* pNewEntryList )
{
    ClearEntryList();
    Clear();

    if ( pNewEntryList )
    {
        if ( pNewEntryList->Count() > 1 )
        {
            CopyEntryList( *pNewEntryList );
            SetUpdateMode( FALSE );

            String* pEntry = (String*)aEntryList.First();

            while ( pEntry )
            {
                InsertEntry( *pEntry, LISTBOX_APPEND );
                aEntryList.Next(); // Kommentar ueberspringen
                pEntry = (String*)aEntryList.Next();
            }

            SetUpdateMode( TRUE );
            SetNoSelection();
            rParent.SetComment( EMPTY_STRING );
        }
        else if ( pNewEntryList->Count() == 1 )
            // Tabelle ist Scenario-Tabelle: nur Kommentar
            rParent.SetComment( *((String*)pNewEntryList->First()) );
        else
            rParent.SetComment( EMPTY_STRING );     // normale Tabelle ohne Szenarien
    }
}

// -----------------------------------------------------------------------

void ScScenarioListBox::ClearEntryList()
{
    String* pEntry = (String*)aEntryList.First();

    while ( pEntry )
    {
        delete pEntry;
        pEntry = (String*)aEntryList.Next();
    }
    aEntryList.Clear();
}

// -----------------------------------------------------------------------

void ScScenarioListBox::CopyEntryList( List& rNewList )
{
    if ( aEntryList.Count() > 0 )
        ClearEntryList();

    String* pEntry = (String*)rNewList.First();

    while ( pEntry )
    {
        aEntryList.Insert( new String( *pEntry ), LIST_APPEND );
        pEntry = (String*)rNewList.Next();
    }
}

// -----------------------------------------------------------------------

void __EXPORT ScScenarioListBox::Select()
{
    String* pEntry = (String*)aEntryList.GetObject( (GetSelectEntryPos()*2)+1 );

    if ( pEntry )
        rParent.SetComment( *pEntry );
}

// -----------------------------------------------------------------------

void __EXPORT ScScenarioListBox::DoubleClick()
{
    SfxStringItem aStringItem( SID_SELECT_SCENARIO, GetSelectEntry() );

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (pViewFrm)
        pViewFrm->GetDispatcher()->Execute( SID_SELECT_SCENARIO,
                              SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                              &aStringItem, 0L, 0L );
}

// -----------------------------------------------------------------------

void __EXPORT ScScenarioListBox::GetFocus()
{
    pAccel = new Accelerator;

    pAccel->InsertItem( 1, KeyCode( KEY_RETURN ) );
    pAccel->InsertItem( 2, KeyCode( KEY_ESCAPE ) );
    pAccel->SetSelectHdl( LINK( this, ScScenarioListBox, AccelSelectHdl ) );

    Application::InsertAccel( pAccel );
    aCurText = GetText();
}

// -----------------------------------------------------------------------

void __EXPORT ScScenarioListBox::LoseFocus()
{
    Application::RemoveAccel( pAccel );
    delete pAccel;
    pAccel = NULL;
}

// -----------------------------------------------------------------------

IMPL_LINK( ScScenarioListBox, AccelSelectHdl, Accelerator *, pAccel )
{
    if ( !pAccel ) return 0;

    switch ( pAccel->GetCurKeyCode().GetCode() )
    {
        case KEY_RETURN:
            Select();
            break;

        case KEY_ESCAPE:
            SelectEntry( aCurText );
            Select();
            break;

        default:
        break;
    }
    return 0;
}

long __EXPORT ScScenarioListBox::Notify( NotifyEvent& rNEvt )
{
    ListBox::Notify( rNEvt );
    long nHandled = 0;

    if ( rNEvt.GetType() == EVENT_COMMAND && GetSelectEntryCount() )
    {
        const CommandEvent* pCEvt = rNEvt.GetCommandEvent();
        if ( pCEvt && pCEvt->GetCommand() == COMMAND_CONTEXTMENU )
        {
            ScPopupMenu aPopup( ScResId( RID_POPUP_NAVIPI_SCENARIO ) );
            aPopup.Execute( this, pCEvt->GetMousePosPixel() );
            if (aPopup.WasHit())
            {
                String aName = GetSelectEntry();
                USHORT nId = aPopup.GetSelected();
                if ( nId == RID_NAVIPI_SCENARIO_DELETE )
                {
                    short nRes = QueryBox( NULL, WinBits( WB_YES_NO | WB_DEF_YES ),
                                    ScGlobal::GetRscString(STR_QUERY_DELSCENARIO) ).Execute();
                    if ( nRes == RET_YES )
                    {
                        SfxStringItem aStringItem( SID_DELETE_SCENARIO, aName );
                        SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                        if (pViewFrm)
                            pViewFrm->GetDispatcher()->Execute( SID_DELETE_SCENARIO,
                                                    SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                                                    &aStringItem, 0L, 0L );
                    }
                }
                else if ( nId == RID_NAVIPI_SCENARIO_EDIT )
                {
                    SfxStringItem aStringItem( SID_EDIT_SCENARIO, aName );
                    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                    if (pViewFrm)
                        pViewFrm->GetDispatcher()->Execute( SID_EDIT_SCENARIO,
                                                SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                                                &aStringItem, 0L, 0L );
                }
            }
            nHandled = 1;
        }
    }

    return nHandled;
}


//========================================================================
// class ScScenarioWindow ------------------------------------------------
//========================================================================

ScScenarioWindow::ScScenarioWindow( Window* pParent,const String& aQH_List,
                                    const String& aQH_Comment)
    :   Window      ( pParent ),
        aLbScenario ( this ),
        aEdComment  ( this,  WB_BORDER | WB_LEFT
                           | WB_READONLY | WB_VSCROLL )
{
    Font aFont( GetFont() );
    aFont.SetTransparent( TRUE );
    aFont.SetWeight( WEIGHT_LIGHT );
    aEdComment.SetFont( aFont );
    aEdComment.SetMaxTextLen( 512 );
    aLbScenario.SetPosPixel( Point(0,0) );
    aLbScenario.SetHelpId(HID_SC_SCENWIN_TOP);
    aEdComment.SetHelpId(HID_SC_SCENWIN_BOTTOM);
    aLbScenario.Show();
    aEdComment.Show();

    aLbScenario.SetQuickHelpText(aQH_List);
    aEdComment.SetQuickHelpText(aQH_Comment);
    aEdComment.SetBackground( Color( COL_LIGHTGRAY ) );

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (pViewFrm)
    {
        SfxBindings& rBindings = pViewFrm->GetBindings();
        rBindings.Invalidate( SID_SELECT_SCENARIO );
        rBindings.Update( SID_SELECT_SCENARIO );
    }
}

// -----------------------------------------------------------------------

__EXPORT ScScenarioWindow::~ScScenarioWindow()
{
}

void ScScenarioWindow::Paint( const Rectangle& rRec )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetFaceColor();

    SetBackground( aBgColor );

    Window::Paint( rRec );
}

// -----------------------------------------------------------------------

void ScScenarioWindow::NotifyState( const SfxPoolItem* pState )
{
    if( pState )
    {
        aLbScenario.Enable();

        if ( pState->ISA(SfxStringItem) )
        {
            String aNewEntry( ((const SfxStringItem*)pState)->GetValue() );

            if ( aNewEntry.Len() > 0 )
                aLbScenario.SelectEntry( aNewEntry );
            else
                aLbScenario.SetNoSelection();
        }
        else if ( pState->ISA(SfxStringListItem) )
        {
            aLbScenario.UpdateEntries( ((SfxStringListItem*)pState)->GetList() );
        }
    }
    else
    {
        aLbScenario.Disable();
        aLbScenario.SetNoSelection();
    }
}

// -----------------------------------------------------------------------

void ScScenarioWindow::SetSizePixel( const Size& rNewSize )
{
    Size aSize( rNewSize );
    long nHeight = aSize.Height() / 2;

    Window::SetSizePixel( aSize );

    aSize.Height() = nHeight;
    aLbScenario.SetSizePixel( aSize );

    aSize.Height() -= 4;
    aEdComment.SetPosSizePixel( Point( 0, nHeight+4 ), aSize );
}




