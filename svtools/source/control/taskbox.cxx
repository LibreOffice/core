/*************************************************************************
 *
 *  $RCSfile: taskbox.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:57 $
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

#define _TASKBAR_CXX

#ifndef _TOOLS_LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _SV_SYSTEM_HXX
#include <vcl/system.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <vcl/help.hxx>
#endif

#include <taskbar.hxx>

// =======================================================================

#define TASKBOX_TASKOFF             3

// =======================================================================

struct ImplTaskItem
{
    Image               maImage;
    XubString           maText;
};

DECLARE_LIST( ImplTaskItemList, ImplTaskItem* );

// =======================================================================

TaskToolBox::TaskToolBox( Window* pParent, WinBits nWinStyle ) :
    ToolBox( pParent, nWinStyle | WB_SCROLL | WB_3DLOOK )
{
    mpItemList      = new ImplTaskItemList;
    mnMaxTextWidth  = 0;
    mnActiveItemId  = 0;
    mnTaskItem      = 0;
    mnSmallItem     = TOOLBOX_ITEM_NOTFOUND;
    mbMinActivate   = FALSE;

    SetAlign( WINDOWALIGN_BOTTOM );
    SetButtonType( BUTTON_SYMBOLTEXT );
}

// -----------------------------------------------------------------------

TaskToolBox::~TaskToolBox()
{
    ImplTaskItem* pItem = mpItemList->First();
    while ( pItem )
    {
        delete pItem;
        pItem = mpItemList->Next();
    }

    delete mpItemList;
}

// -----------------------------------------------------------------------

void TaskToolBox::ActivateTaskItem( USHORT nItemId, BOOL bMinActivate )
{
    if ( nItemId )
    {
        if ( nItemId != mnActiveItemId )
        {
            if ( mnActiveItemId )
                CheckItem( mnActiveItemId, FALSE );
            CheckItem( nItemId );
            mnActiveItemId = nItemId;
        }
        else
        {
            if ( !bMinActivate )
                return;

            mbMinActivate = TRUE;
        }

        mnTaskItem = nItemId-1;
        ActivateTask();
        mnTaskItem = 0;
        mbMinActivate = FALSE;
    }
}

// -----------------------------------------------------------------------

void TaskToolBox::ActivateTask()
{
    maActivateTaskHdl.Call( this );
}

// -----------------------------------------------------------------------

void TaskToolBox::ContextMenu()
{
    maContextMenuHdl.Call( this );
}

// -----------------------------------------------------------------------

void TaskToolBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( !rMEvt.IsRight() )
        ToolBox::MouseButtonDown( rMEvt );
}

// -----------------------------------------------------------------------

void TaskToolBox::Resize()
{
    mnOldItemCount  = mpItemList->Count();
    mnUpdatePos     = (USHORT)mnOldItemCount;
    mnUpdateNewPos  = TOOLBOX_ITEM_NOTFOUND;
    ImplFormat();
    ToolBox::Resize();
}

// -----------------------------------------------------------------------

void TaskToolBox::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        USHORT nItemId = GetItemId( rCEvt.GetMousePosPixel() );
// Dies machen wir nicht mehr, da es von zu vielen als stoerend empfunden wurde
//        ActivateTaskItem( nItemId );
        mnTaskItem = nItemId-1;

        maContextMenuPos = OutputToScreenPixel( rCEvt.GetMousePosPixel() );
        ContextMenu();
        maContextMenuPos = Point();
        mnTaskItem = 0;
    }
    else
        ToolBox::Command( rCEvt );
}

// -----------------------------------------------------------------------

void TaskToolBox::RequestHelp( const HelpEvent& rHEvt )
{
    if ( rHEvt.GetMode() & (HELPMODE_BALLOON | HELPMODE_QUICK) )
    {
        USHORT nItemId = GetItemId( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );

        if ( nItemId )
        {
            ImplTaskItem* pItem = mpItemList->GetObject( nItemId-1 );
            if ( pItem )
            {
                if ( pItem->maText != GetItemText( nItemId ) )
                {
                    Rectangle aItemRect = GetItemRect( nItemId );
                    if ( rHEvt.GetMode() & HELPMODE_QUICK )
                        Help::ShowQuickHelp( this, aItemRect, pItem->maText );
                    else
                        Help::ShowBalloon( this, aItemRect.Center(), aItemRect, pItem->maText );
                }
                else
                    Help::ShowQuickHelp( this, Rectangle(), String() );
                return;
            }
        }
    }

    ToolBox::RequestHelp( rHEvt );
}

// -----------------------------------------------------------------------

BOOL TaskToolBox::QueryDrop( DropEvent& rDEvt )
{
    if ( !rDEvt.IsLeaveWindow() )
    {
        USHORT nItemId = GetItemId( rDEvt.GetPosPixel() );
        if ( nItemId )
            ActivateTaskItem( nItemId );
    }
    return ToolBox::QueryDrop( rDEvt );
}

// -----------------------------------------------------------------------

void TaskToolBox::Select()
{
    USHORT nItemId = GetCurItemId();
    ActivateTaskItem( nItemId, TRUE );
}

// -----------------------------------------------------------------------

void TaskToolBox::ImplFormat()
{
    if ( mnUpdateNewPos == TOOLBOX_ITEM_NOTFOUND )
    {
        // Eintraege aus der Liste entfernen
        while ( mpItemList->Count() > mnUpdatePos )
            delete mpItemList->Remove( (ULONG)mnUpdatePos );
        mnUpdateNewPos = mnUpdatePos;
    }

    // Maximale Itemgroesse berechnen
    long nOldMaxTextWidth = mnMaxTextWidth;
    mnMaxTextWidth = 70;
    if ( mpItemList->Count() )
    {
        long nWinSize = GetOutputSizePixel().Width()-8;
        long nItemSize = mpItemList->GetObject(0)->maImage.GetSizePixel().Width()+7+TASKBOX_TASKOFF+2;
        nWinSize -= mpItemList->Count()*nItemSize;
        if ( nWinSize > 0 )
            nWinSize /= mpItemList->Count();
        else
            nWinSize = 0;
        if ( nWinSize < mnMaxTextWidth )
            mnMaxTextWidth = nWinSize;
        if ( (mnMaxTextWidth < nOldMaxTextWidth) ||
             ((mnMaxTextWidth-nOldMaxTextWidth > 3) &&
              (mnSmallItem != TOOLBOX_ITEM_NOTFOUND)) )
        {
            mnSmallItem = TOOLBOX_ITEM_NOTFOUND;
            mnUpdateNewPos = 0;
        }
    }

    // Eintraege aus der ToolBox entfernen, die ersetzt werden
    USHORT nBtnPos = (mnUpdateNewPos*2);
    while ( nBtnPos < GetItemCount() )
        RemoveItem( nBtnPos );
    if ( mnUpdateNewPos <= (mnActiveItemId-1) )
        mnActiveItemId = 0;

    // Neue Eintrage einfuegen
    USHORT i = mnUpdateNewPos;
    while ( i < mpItemList->Count() )
    {
        ImplTaskItem* pItem = mpItemList->GetObject( i );

        // Textlaenge berechnen
        XubString aText = pItem->maText;
        if ( !aText.Len() )
            aText = ' ';
        long nTxtWidth = GetTextWidth( aText );
        if ( nTxtWidth > mnMaxTextWidth )
        {
            if ( mnSmallItem == TOOLBOX_ITEM_NOTFOUND )
                mnSmallItem = i;
            // 3 == Len of "..."
            aText.AppendAscii( "..." );
            do
            {
                aText.Erase( aText.Len()-3-1, 1 );
                nTxtWidth = GetTextWidth( aText );
            }
            while ( (nTxtWidth > mnMaxTextWidth) && (aText.Len() > 3) );
        }

        USHORT nItemId = i+1;
        if ( aText.EqualsAscii( "..." ) )
            InsertItem( nItemId, pItem->maImage, TIB_LEFT );
        else
            InsertItem( nItemId, pItem->maImage, aText, TIB_LEFT );
        InsertSeparator( TOOLBOX_APPEND, TASKBOX_TASKOFF );
        i++;
    }

    if ( mnUpdateNewPos != 0 )
        mnMaxTextWidth = nOldMaxTextWidth;

    if ( mnNewActivePos+1 != mnActiveItemId )
    {
        if ( mnActiveItemId )
            CheckItem( mnActiveItemId, FALSE );
        mnActiveItemId = mnNewActivePos+1;
        CheckItem( mnActiveItemId );
    }
}

// -----------------------------------------------------------------------

void TaskToolBox::StartUpdateTask()
{
    mnOldItemCount  = mpItemList->Count();
    mnUpdatePos     = 0;
    mnUpdateNewPos  = TOOLBOX_ITEM_NOTFOUND;
    mnNewActivePos  = 0xFFFE;
}

// -----------------------------------------------------------------------

void TaskToolBox::UpdateTask( const Image& rImage, const String& rText,
                              BOOL bActive )
{
    ImplTaskItem* pItem = mpItemList->GetObject( mnUpdatePos );
    if ( pItem )
    {
        if ( (pItem->maText != rText) || (pItem->maImage != rImage) )
        {
            // Eintraege aus der Liste entfernen
            while ( mpItemList->Count() > mnUpdatePos )
                delete mpItemList->Remove( (ULONG)mnUpdatePos );
            pItem = NULL;
        }
    }

    if ( !pItem )
    {
        if ( mnUpdatePos < mnUpdateNewPos )
            mnUpdateNewPos = mnUpdatePos;

        pItem           = new ImplTaskItem;
        pItem->maImage  = rImage;
        pItem->maText   = rText;
        mpItemList->Insert( pItem, LIST_APPEND );
    }

    if ( bActive )
        mnNewActivePos = mnUpdatePos;

    mnUpdatePos++;
}

// -----------------------------------------------------------------------

void TaskToolBox::EndUpdateTask()
{
    if ( mnUpdateNewPos == TOOLBOX_ITEM_NOTFOUND )
    {
        // Eintraege aus der Liste entfernen
        while ( mpItemList->Count() > mnUpdatePos )
            delete mpItemList->Remove( (ULONG)mnUpdatePos );
        mnUpdateNewPos = mnUpdatePos;
    }

    ImplFormat();
}

