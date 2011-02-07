/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#define _TASKBAR_CXX

#include <tools/list.hxx>
#include <tools/debug.hxx>
#include <vcl/image.hxx>
#include <vcl/help.hxx>

#include <taskbar.hxx>

// =======================================================================

#define TASKBOX_TASKOFF             3

// =======================================================================

struct ImplTaskItem
{
    Image               maImage;
    XubString           maText;
};

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
    for ( size_t i = 0, n = mpItemList->size(); i < n; ++i ) {
        delete (*mpItemList)[ i ];
    }
    mpItemList->clear();
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
    mnOldItemCount  = mpItemList->size();
    mnUpdatePos     = mnOldItemCount;
    mnUpdateNewPos  = TOOLBOX_ITEM_NOTFOUND;
    ImplFormatTaskToolBox();
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

        maContextMenuPos = rCEvt.GetMousePosPixel();
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
        size_t nItemId = GetItemId( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );

        if ( nItemId )
        {
            ImplTaskItem* pItem = ( nItemId-1 < mpItemList->size() ) ? (*mpItemList)[ nItemId-1 ] : NULL;
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

void TaskToolBox::Select()
{
    USHORT nItemId = GetCurItemId();
    ActivateTaskItem( nItemId, TRUE );
}

// -----------------------------------------------------------------------

void TaskToolBox::ImplFormatTaskToolBox()
{
    if ( mnUpdateNewPos == TOOLBOX_ITEM_NOTFOUND )
    {
        // Eintraege aus der Liste entfernen
        while ( mpItemList->size() > mnUpdatePos ) {
            delete mpItemList->back();
            mpItemList->pop_back();
        }
        mnUpdateNewPos = mnUpdatePos;
    }

    // Maximale Itemgroesse berechnen
    long nOldMaxTextWidth = mnMaxTextWidth;
    mnMaxTextWidth = 70;
    if ( !mpItemList->empty() )
    {
        long nWinSize = GetOutputSizePixel().Width()-8;
        long nItemSize = (*mpItemList)[ 0 ]->maImage.GetSizePixel().Width()+7+TASKBOX_TASKOFF+2;
        nWinSize -= mpItemList->size()*nItemSize;
        if ( nWinSize > 0 )
            nWinSize /= mpItemList->size();
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
    size_t i = mnUpdateNewPos;
    while ( i < mpItemList->size() )
    {
        ImplTaskItem* pItem = (*mpItemList)[ i ];

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
    mnOldItemCount  = mpItemList->size();
    mnUpdatePos     = 0;
    mnUpdateNewPos  = TOOLBOX_ITEM_NOTFOUND;
    mnNewActivePos  = 0xFFFE;
}

// -----------------------------------------------------------------------

void TaskToolBox::UpdateTask( const Image& rImage, const String& rText,
                              BOOL bActive )
{
    ImplTaskItem* pItem = ( mnUpdatePos < mpItemList->size() ) ? (*mpItemList)[ mnUpdatePos ] : NULL;
    if ( pItem )
    {
        if ( (pItem->maText != rText) || (pItem->maImage != rImage) )
        {
            // Eintraege aus der Liste entfernen
            while ( mpItemList->size() > mnUpdatePos ) {
                delete mpItemList->back();
                mpItemList->pop_back();
            }
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
        mpItemList->push_back( pItem );
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
        while ( mpItemList->size() > mnUpdatePos ) {
            delete mpItemList->back();
            mpItemList->pop_back();
        }
        mnUpdateNewPos = mnUpdatePos;
    }

    ImplFormatTaskToolBox();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
