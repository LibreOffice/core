/*************************************************************************
 *
 *  $RCSfile: toolbox2.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-11 17:30:52 $
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

#define _SV_TOOLBOX_CXX

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_RC_H
#include <rc.h>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <help.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <bitmap.hxx>
#endif
#define private public
#ifndef _SV_TOOLBOX_HXX
#include <toolbox.hxx>
#endif
#undef private
#ifndef _SV_TOOLBOX_H
#include <toolbox.h>
#endif

using namespace vcl;

// =======================================================================

#define TB_SEP_SIZE             8

// -----------------------------------------------------------------------

ImplToolItem::ImplToolItem()
{
    mnId            = 0;
    mpWindow        = NULL;
    mpUserData      = NULL;
    mnHelpId        = 0;
    meType          = TOOLBOXITEM_BUTTON;
    mnBits          = 0;
    meState         = STATE_NOCHECK;
    mbEnabled       = TRUE;
    mbVisible       = TRUE;
    mbEmptyBtn      = TRUE;
    mbShowWindow    = FALSE;
    mbBreak         = FALSE;
    mnNonStdSize    = 0;
    mnSepSize       = TB_SEP_SIZE;
    mnImageAngle    = 0;
    mbMirrorMode    = false;
}

// -----------------------------------------------------------------------

ImplToolItem::ImplToolItem( USHORT nItemId, const Image& rImage,
                            ToolBoxItemBits nItemBits ) :
    maImage( rImage )
{
    mnId            = nItemId;
    mpWindow        = NULL;
    mpUserData      = NULL;
    mnHelpId        = 0;
    meType          = TOOLBOXITEM_BUTTON;
    mnBits          = nItemBits;
    meState         = STATE_NOCHECK;
    mbEnabled       = TRUE;
    mbVisible       = TRUE;
    mbEmptyBtn      = FALSE;
    mbShowWindow    = FALSE;
    mbBreak         = FALSE;
    mnNonStdSize    = 0;
    mnSepSize       = TB_SEP_SIZE;
    mnImageAngle    = 0;
    mbMirrorMode    = false;
}

// -----------------------------------------------------------------------

ImplToolItem::ImplToolItem( USHORT nItemId, const XubString& rText,
                            ToolBoxItemBits nItemBits ) :
    maText( rText )
{
    mnId            = nItemId;
    mpWindow        = NULL;
    mpUserData      = NULL;
    mnHelpId        = 0;
    meType          = TOOLBOXITEM_BUTTON;
    mnBits          = nItemBits;
    meState         = STATE_NOCHECK;
    mbEnabled       = TRUE;
    mbVisible       = TRUE;
    mbEmptyBtn      = FALSE;
    mbShowWindow    = FALSE;
    mbBreak         = FALSE;
    mnNonStdSize    = 0;
    mnSepSize       = TB_SEP_SIZE;
    mnImageAngle    = 0;
    mbMirrorMode    = false;
}

// -----------------------------------------------------------------------

ImplToolItem::ImplToolItem( USHORT nItemId, const Image& rImage,
                            const XubString& rText, ToolBoxItemBits nItemBits ) :
    maImage( rImage ),
    maText( rText )
{
    mnId            = nItemId;
    mpWindow        = NULL;
    mpUserData      = NULL;
    mnHelpId        = 0;
    meType          = TOOLBOXITEM_BUTTON;
    mnBits          = nItemBits;
    meState         = STATE_NOCHECK;
    mbEnabled       = TRUE;
    mbVisible       = TRUE;
    mbEmptyBtn      = FALSE;
    mbShowWindow    = FALSE;
    mbBreak         = FALSE;
    mnNonStdSize    = 0;
    mnSepSize       = TB_SEP_SIZE;
    mnImageAngle    = 0;
    mbMirrorMode    = false;
}

// -----------------------------------------------------------------------

ImplToolItem::ImplToolItem( const ImplToolItem& rItem ) :
        mpWindow                ( rItem.mpWindow ),
        mpUserData              ( rItem.mpUserData ),
        maImage                 ( rItem.maImage ),
        maHighImage             ( rItem.maHighImage ),
        mnImageAngle            ( rItem.mnImageAngle ),
        mbMirrorMode            ( rItem.mbMirrorMode ),
        maText                  ( rItem.maText ),
        maQuickHelpText         ( rItem.maQuickHelpText ),
        maHelpText              ( rItem.maHelpText ),
        maCommandStr            ( rItem.maCommandStr ),
        mnHelpId                ( rItem.mnHelpId ),
        maRect                  ( rItem.maRect ),
        maCalcRect              ( rItem.maCalcRect ),
        mnNonStdSize            ( rItem.mnNonStdSize ),
        mnSepSize               ( rItem.mnSepSize ),
        meType                  ( rItem.meType ),
        mnBits                  ( rItem.mnBits ),
        meState                 ( rItem.meState ),
        mnId                    ( rItem.mnId ),
        mbEnabled               ( rItem.mbEnabled ),
        mbVisible               ( rItem.mbVisible ),
        mbEmptyBtn              ( rItem.mbEmptyBtn ),
        mbShowWindow            ( rItem.mbShowWindow ),
        mbBreak                 ( rItem.mbBreak )
{
}

// -----------------------------------------------------------------------

ImplToolItem::~ImplToolItem()
{
}

// -----------------------------------------------------------------------

ImplToolItem& ImplToolItem::operator=( const ImplToolItem& rItem )
{
    mpWindow                = rItem.mpWindow;
    mpUserData              = rItem.mpUserData;
    maImage                 = rItem.maImage;
    maHighImage             = rItem.maHighImage;
    mnImageAngle            = rItem.mnImageAngle;
    mbMirrorMode            = rItem.mbMirrorMode;
    maText                  = rItem.maText;
    maQuickHelpText         = rItem.maQuickHelpText;
    maHelpText              = rItem.maHelpText;
    maCommandStr            = rItem.maCommandStr;
    mnHelpId                = rItem.mnHelpId;
    maRect                  = rItem.maRect;
    maCalcRect              = rItem.maCalcRect;
    mnNonStdSize            = rItem.mnNonStdSize;
    mnSepSize               = rItem.mnSepSize;
    meType                  = rItem.meType;
    mnBits                  = rItem.mnBits;
    meState                 = rItem.meState;
    mnId                    = rItem.mnId;
    mbEnabled               = rItem.mbEnabled;
    mbVisible               = rItem.mbVisible;
    mbEmptyBtn              = rItem.mbEmptyBtn;
    mbShowWindow            = rItem.mbShowWindow;
    mbBreak                 = rItem.mbBreak;
    return *this;
}

// -----------------------------------------------------------------------

const XubString& ToolBox::ImplConvertMenuString( const XubString& rStr )
{
    if ( mbMenuStrings )
    {
        maCvtStr = rStr;
        maCvtStr.EraseTrailingChars( '.' );
        maCvtStr.EraseAllChars( '~' );
        return maCvtStr;
    }
    else
        return rStr;
}

// -----------------------------------------------------------------------

void ToolBox::ImplInvalidate( BOOL bNewCalc, BOOL bFullPaint )
{
    ImplUpdateInputEnable();

    if ( bNewCalc )
        mbCalc = TRUE;

    if ( bFullPaint )
    {
        mbFormat = TRUE;

        // Muss ueberhaupt eine neue Ausgabe erfolgen
        if ( IsReallyVisible() && IsUpdateMode() )
        {
            Invalidate( Rectangle( mnLeftBorder, mnTopBorder,
                                   mnDX-mnRightBorder-1, mnDY-mnBottomBorder-1 ) );
            maTimer.Stop();
        }
    }
    else
    {
        if ( !mbFormat )
        {
            mbFormat = TRUE;

            // Muss ueberhaupt eine neue Ausgabe erfolgen
            if ( IsReallyVisible() && IsUpdateMode() )
                maTimer.Start();
        }
    }
}

// -----------------------------------------------------------------------

void ToolBox::ImplUpdateItem( USHORT nIndex )
{
    // Muss ueberhaupt eine neue Ausgabe erfolgen
    if ( IsReallyVisible() && IsUpdateMode() )
    {
        if ( nIndex == 0xFFFF )
        {
            // Nur direkt neu ausgeben, wenn nicht neu formatiert
            // werden muss
            if ( !mbFormat )
            {
                USHORT nItemCount = mpData->m_aItems.size();
                for ( USHORT i = 0; i < nItemCount; i++ )
                    ImplDrawItem( i, (i == mnCurPos) ? TRUE : FALSE );
            }
            else
            {
                Invalidate( Rectangle( mnLeftBorder, mnTopBorder,
                                       mnDX-mnRightBorder-1, mnDY-mnBottomBorder-1 ) );
            }
        }
        else
        {
            // Nur direkt neu ausgeben, wenn nicht neu formatiert
            // werden muss
            if ( !mbFormat )
            {
                ImplToolItem* pItem = &mpData->m_aItems[nIndex];
                //ImplDrawItem( nIndex, (nIndex == mnCurPos) ? TRUE : FALSE );
                ImplDrawItem( nIndex, (pItem->mnId == mnHighItemId) ? 2 : FALSE );
            }
            else
                maPaintRect.Union( mpData->m_aItems[nIndex].maRect );
        }
    }
}

// -----------------------------------------------------------------------

void ToolBox::Click()
{
    ImplCallEventListeners( VCLEVENT_TOOLBOX_CLICK );
    maClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void ToolBox::DoubleClick()
{
    ImplCallEventListeners( VCLEVENT_TOOLBOX_DOUBLECLICK );
    maDoubleClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void ToolBox::Activate()
{
    mnActivateCount++;
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ACTIVATE );
    maActivateHdl.Call( this );
}

// -----------------------------------------------------------------------

void ToolBox::Deactivate()
{
    mnActivateCount--;
    ImplCallEventListeners( VCLEVENT_TOOLBOX_DEACTIVATE );
    maDeactivateHdl.Call( this );

    if ( mbHideStatusText )
    {
        GetpApp()->HideHelpStatusText();
        mbHideStatusText = FALSE;
    }
}

// -----------------------------------------------------------------------

void ToolBox::Highlight()
{
    ImplCallEventListeners( VCLEVENT_TOOLBOX_HIGHLIGHT );
    maHighlightHdl.Call( this );

    XubString aStr = GetHelpText( mnCurItemId );
    if ( aStr.Len() || mbHideStatusText )
    {
        GetpApp()->ShowHelpStatusText( aStr );
        mbHideStatusText = TRUE;
    }
}

// -----------------------------------------------------------------------

void ToolBox::Select()
{
    ImplCallEventListeners( VCLEVENT_TOOLBOX_SELECT );
    maSelectHdl.Call( this );
}

// -----------------------------------------------------------------------

void ToolBox::NextToolBox()
{
    maNextToolBoxHdl.Call( this );
}

// -----------------------------------------------------------------------

void ToolBox::Customize( const ToolBoxCustomizeEvent& )
{
}

// -----------------------------------------------------------------------

void ToolBox::UserDraw( const UserDrawEvent& rUDEvt )
{
}

// -----------------------------------------------------------------------

void ToolBox::InsertItem( const ResId& rResId, USHORT nPos )
{
    USHORT                  nObjMask;
    BOOL                    bImage = FALSE;     // Wurde Image gesetzt

    // Item anlegen
    ImplToolItem aItem;

    GetRes( rResId.SetRT( RSC_TOOLBOXITEM ) );
    nObjMask            = ReadShortRes();

    if ( nObjMask & RSC_TOOLBOXITEM_ID )
        aItem.mnId = ReadShortRes();
    else
        aItem.mnId = 1;

    if ( nObjMask & RSC_TOOLBOXITEM_TYPE )
        aItem.meType = (ToolBoxItemType)ReadShortRes();

    if ( nObjMask & RSC_TOOLBOXITEM_STATUS )
        aItem.mnBits = (ToolBoxItemBits)ReadShortRes();

    if( nObjMask & RSC_TOOLBOXITEM_HELPID )
        aItem.mnHelpId = ReadLongRes();

    if ( nObjMask & RSC_TOOLBOXITEM_TEXT )
    {
        aItem.maText = ReadStringRes();
        aItem.maText = ImplConvertMenuString( aItem.maText );
    }
    if ( nObjMask & RSC_TOOLBOXITEM_HELPTEXT )
        aItem.maHelpText = ReadStringRes();

/*
#ifndef WIN
    static
#endif
        short nHelpMode = -1;
    if( nHelpMode == -1 ) {
        SvHelpSettings aHelpSettings;

        GetpApp()->Property( aHelpSettings );
        nHelpMode = aHelpSettings.nHelpMode;
    }

    if( (nHelpMode & HELPTEXTMODE_EXTERN) && aItem.aHelpText.Len() )
        aItem.aHelpText.Erase();
    if( (nHelpMode & HELPTEXTMODE_DEBUG) && !aItem.nHelpId )
        aItem.aHelpText = "??? !aItem.nHelpId MP/W.P. ???";
*/

    if ( nObjMask & RSC_TOOLBOXITEM_BITMAP )
    {
        Bitmap aBmp = Bitmap( ResId( (RSHEADER_TYPE*)GetClassRes() ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
        aItem.maImage = Image( aBmp, IMAGE_STDBTN_COLOR );
        bImage = TRUE;
    }
    if ( nObjMask & RSC_TOOLBOXITEM_IMAGE )
    {
        aItem.maImage = Image( ResId( (RSHEADER_TYPE*)GetClassRes() ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
        bImage = TRUE;
    }
    if ( nObjMask & RSC_TOOLBOXITEM_DISABLE )
        aItem.mbEnabled = !(BOOL)ReadShortRes();

    if ( nObjMask & RSC_TOOLBOXITEM_STATE )
        aItem.meState   = (TriState)ReadShortRes();

    if ( nObjMask & RSC_TOOLBOXITEM_HIDE )
        aItem.mbVisible = !((BOOL)ReadShortRes());

    if ( nObjMask & RSC_TOOLBOXITEM_COMMAND )
        aItem.maCommandStr = ReadStringRes();

    // Wenn kein Image geladen wurde, versuchen wir das Image aus der
    // Image-Liste zu holen
    if ( !bImage && aItem.mnId )
        aItem.maImage = maImageList.GetImage( aItem.mnId );

    // Wenn es sich um ein ButtonItem handelt, die ID ueberpruefen
    BOOL bNewCalc;
    if ( aItem.meType != TOOLBOXITEM_BUTTON )
    {
        bNewCalc = FALSE;
        aItem.mnId = 0;
    }
    else
    {
        bNewCalc = TRUE;

        DBG_ASSERT( aItem.mnId, "ToolBox::InsertItem(): ItemId == 0" );
        DBG_ASSERT( GetItemPos( aItem.mnId ) == TOOLBOX_ITEM_NOTFOUND,
                    "ToolBox::InsertItem(): ItemId already exists" );
    }

    // Item anlegen und in die Liste einfuegen
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), aItem );
    mpData->ImplClearLayoutData();

    // ToolBox neu brechnen und neu ausgeben
    ImplInvalidate( bNewCalc );

    // Notify
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, (void*) nPos );
}

// -----------------------------------------------------------------------

void ToolBox::InsertItem( USHORT nItemId, const Image& rImage,
                          ToolBoxItemBits nBits, USHORT nPos )
{
    DBG_ASSERT( nItemId, "ToolBox::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == TOOLBOX_ITEM_NOTFOUND,
                "ToolBox::InsertItem(): ItemId already exists" );

    // Item anlegen und in die Liste einfuegen
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), ImplToolItem( nItemId, rImage, nBits ) );
    mpData->ImplClearLayoutData();

    ImplInvalidate( TRUE );

    // Notify
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, (void*) nPos );
}

// -----------------------------------------------------------------------

void ToolBox::InsertItem( USHORT nItemId, const Image& rImage,
                          const XubString& rText,
                          ToolBoxItemBits nBits, USHORT nPos )
{
    DBG_ASSERT( nItemId, "ToolBox::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == TOOLBOX_ITEM_NOTFOUND,
                "ToolBox::InsertItem(): ItemId already exists" );

    // Item anlegen und in die Liste einfuegen
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), ImplToolItem( nItemId, rImage, ImplConvertMenuString( rText ), nBits ) );
    mpData->ImplClearLayoutData();

    ImplInvalidate( TRUE );

    // Notify
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, (void*) nPos );
}

// -----------------------------------------------------------------------

void ToolBox::InsertItem( USHORT nItemId, const XubString& rText,
                          ToolBoxItemBits nBits, USHORT nPos )
{
    DBG_ASSERT( nItemId, "ToolBox::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == TOOLBOX_ITEM_NOTFOUND,
                "ToolBox::InsertItem(): ItemId already exists" );

    // Item anlegen und in die Liste einfuegen
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), ImplToolItem( nItemId, ImplConvertMenuString( rText ), nBits ) );
    mpData->ImplClearLayoutData();

    ImplInvalidate( TRUE );

    // Notify
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, (void*) nPos );
}

// -----------------------------------------------------------------------

void ToolBox::InsertWindow( USHORT nItemId, Window* pWindow,
                            ToolBoxItemBits nBits, USHORT nPos )
{
    DBG_ASSERT( nItemId, "ToolBox::InsertWindow(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == TOOLBOX_ITEM_NOTFOUND,
                "ToolBox::InsertWindow(): ItemId already exists" );

    // Item anlegen und in die Liste einfuegen
    ImplToolItem aItem;
    aItem.mnId       = nItemId;
    aItem.meType     = TOOLBOXITEM_BUTTON;
    aItem.mnBits     = nBits;
    aItem.mpWindow   = pWindow;
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), aItem );
    mpData->ImplClearLayoutData();

    if ( pWindow )
        pWindow->Hide();

    ImplInvalidate( TRUE );

    // Notify
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, (void*) nPos );
}

// -----------------------------------------------------------------------

void ToolBox::InsertSpace( USHORT nPos )
{
    // Item anlegen und in die Liste einfuegen
    ImplToolItem aItem;
    aItem.meType     = TOOLBOXITEM_SPACE;
    aItem.mbEnabled  = FALSE;
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), aItem );
    mpData->ImplClearLayoutData();

    ImplInvalidate( FALSE );

    // Notify
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, (void*) nPos );
}

// -----------------------------------------------------------------------

void ToolBox::InsertSeparator( USHORT nPos, USHORT nPixSize )
{
    // Item anlegen und in die Liste einfuegen
    ImplToolItem aItem;
    aItem.meType     = TOOLBOXITEM_SEPARATOR;
    aItem.mbEnabled  = FALSE;
    if ( nPixSize )
        aItem.mnSepSize = nPixSize;
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), aItem );
    mpData->ImplClearLayoutData();

    ImplInvalidate( FALSE );

    // Notify
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, (void*) nPos );
}

// -----------------------------------------------------------------------

void ToolBox::InsertBreak( USHORT nPos )
{
    // Item anlegen und in die Liste einfuegen
    ImplToolItem aItem;
    aItem.meType     = TOOLBOXITEM_BREAK;
    aItem.mbEnabled  = FALSE;
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), aItem );
    mpData->ImplClearLayoutData();

    ImplInvalidate( FALSE );

    // Notify
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, (void*) nPos );
}

// -----------------------------------------------------------------------

void ToolBox::RemoveItem( USHORT nPos )
{
    if( nPos < mpData->m_aItems.size() )
    {
        BOOL bMustCalc;
        if ( mpData->m_aItems[nPos].meType == TOOLBOXITEM_BUTTON )
            bMustCalc = TRUE;
        else
            bMustCalc = FALSE;

        if ( mpData->m_aItems[nPos].mpWindow )
            mpData->m_aItems[nPos].mpWindow->Hide();

        // PaintRect um das removete Item erweitern
        maPaintRect.Union( mpData->m_aItems[nPos].maRect );

        // Absichern gegen das Loeschen im Select-Handler
        if ( mpData->m_aItems[nPos].mnId == mnCurItemId )
            mnCurItemId = 0;
        if ( mpData->m_aItems[nPos].mnId == mnHighItemId )
            mnHighItemId = 0;

        ImplInvalidate( bMustCalc );

        mpData->m_aItems.erase( mpData->m_aItems.begin()+nPos );
        mpData->ImplClearLayoutData();

        // Notify
        ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMREMOVED, (void*) nPos );
    }
}

// -----------------------------------------------------------------------

void ToolBox::MoveItem( USHORT nItemId, USHORT nNewPos )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos == nNewPos )
        return;

    if ( nPos < nNewPos )
        nNewPos--;

    // Existiert Item
    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        // ToolBox-Item in der Liste verschieben
        ImplToolItem aItem = mpData->m_aItems[nPos];
        mpData->m_aItems.erase( mpData->m_aItems.begin()+nPos );
        mpData->m_aItems.insert( (nNewPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nNewPos : mpData->m_aItems.end(), aItem );
        mpData->ImplClearLayoutData();

        // ToolBox neu ausgeben
        ImplInvalidate( FALSE );

        // Notify
        if( nPos < nNewPos )    // only send one event, all indices above this item are invalid anyway
            ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMREMOVED, (void*) nPos );
        else
            ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, (void*) nNewPos );
    }
}

// -----------------------------------------------------------------------

void ToolBox::CopyItem( const ToolBox& rToolBox, USHORT nItemId,
                        USHORT nNewPos )
{
    DBG_ASSERT( GetItemPos( nItemId ) == TOOLBOX_ITEM_NOTFOUND,
                "ToolBox::CopyItem(): ItemId already exists" );

    USHORT nPos = rToolBox.GetItemPos( nItemId );

    // Existiert Item
    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        // ToolBox-Item in der Liste verschieben
        ImplToolItem aNewItem = mpData->m_aItems[nPos];
        // Bestimme Daten zuruecksetzen
        aNewItem.mpWindow      = NULL;
        aNewItem.mbShowWindow = FALSE;

        mpData->m_aItems.insert( (nNewPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nNewPos : mpData->m_aItems.end(), aNewItem );
        mpData->ImplClearLayoutData();
        // ToolBox neu ausgeben
        ImplInvalidate( FALSE );

        // Notify
        ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, (void*) nPos );
    }
}

// -----------------------------------------------------------------------

void ToolBox::CopyItems( const ToolBox& rToolBox )
{
    mpData->ImplClearLayoutData();
    mpData->m_aItems = rToolBox.mpData->m_aItems;
    // Absichern gegen das Loeschen im Select-Handler
    mnCurItemId = 0;
    mnHighItemId = 0;

    for( std::vector< ImplToolItem >::iterator it = mpData->m_aItems.begin();
         it != mpData->m_aItems.end(); ++it )
    {
        it->mpWindow        = NULL;
        it->mbShowWindow    = FALSE;
    }

    ImplInvalidate( TRUE, TRUE );

    // Notify
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ALLITEMSCHANGED );
}

// -----------------------------------------------------------------------

void ToolBox::Clear()
{
    mpData->m_aItems.clear();
    mpData->ImplClearLayoutData();

    // Absichern gegen das Loeschen im Select-Handler
    mnCurItemId = 0;
    mnHighItemId = 0;

    ImplInvalidate( TRUE, TRUE );

    // Notify
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ALLITEMSCHANGED );
}

// -----------------------------------------------------------------------

void ToolBox::SetButtonType( ButtonType eNewType )
{
    if ( meButtonType != eNewType )
    {
        meButtonType = eNewType;

        // Hier besser alles neu ausgeben, da es ansonsten zu Problemen
        // mit den per CopyBits kopierten Bereichen geben kann
        ImplInvalidate( TRUE );
    }
}

// -----------------------------------------------------------------------

void ToolBox::SetAlign( WindowAlign eNewAlign )
{
    if ( meAlign != eNewAlign )
    {
        meAlign = eNewAlign;

        if ( !IsFloatingMode() )
        {
            // Setzen, ob Items horizontal oder vertikal angeordnet werden sollen
            if ( (eNewAlign == WINDOWALIGN_LEFT) || (eNewAlign == WINDOWALIGN_RIGHT) )
                mbHorz = FALSE;
            else
                mbHorz = TRUE;

            // Hier alles neu ausgeben, da sich Border auch aendert
            mbCalc = TRUE;
            mbFormat = TRUE;
            if ( IsReallyVisible() && IsUpdateMode() )
                Invalidate();
        }
    }
}

// -----------------------------------------------------------------------

void ToolBox::SetLineCount( USHORT nNewLines )
{
    if ( !nNewLines )
        nNewLines = 1;

    if ( mnLines != nNewLines )
    {
        mnLines = nNewLines;

        // Hier besser alles neu ausgeben, da es ansonsten zu Problemen
        // mit den per CopyBits kopierten Bereichen geben kann
        ImplInvalidate( FALSE );
    }
}

// -----------------------------------------------------------------------

void ToolBox::SetNextToolBox( const XubString& rStr )
{
    BOOL bCalcNew = (!maNextToolBoxStr.Len() != !rStr.Len());
    maNextToolBoxStr = rStr;
    if ( bCalcNew )
        ImplInvalidate( TRUE, FALSE );
}

// -----------------------------------------------------------------------

USHORT ToolBox::GetItemCount() const
{
    return (USHORT)mpData->m_aItems.size();
}

// -----------------------------------------------------------------------

ToolBoxItemType ToolBox::GetItemType( USHORT nPos ) const
{
    return (nPos < mpData->m_aItems.size()) ? mpData->m_aItems[nPos].meType : TOOLBOXITEM_DONTKNOW;
}

// -----------------------------------------------------------------------

USHORT ToolBox::GetItemPos( USHORT nItemId ) const
{
    int nCount = mpData->m_aItems.size();
    for( int nPos = 0; nPos < nCount; nPos++ )
        if( mpData->m_aItems[nPos].mnId == nItemId )
            return (USHORT)nPos;

    return TOOLBOX_ITEM_NOTFOUND;
}

// -----------------------------------------------------------------------

USHORT ToolBox::GetItemPos( const Point& rPos ) const
{
    // search the item position on the given point
    USHORT nRet = TOOLBOX_ITEM_NOTFOUND;
    USHORT nPos = 0;
    std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
    while( it != mpData->m_aItems.end() )
    {
        if ( it->maRect.IsInside( rPos ) )
        {
            // item found -> save position and break
            nRet = nPos;
            break;
        }

        ++it;
        ++nPos;
    }

    return nRet;
}

// -----------------------------------------------------------------------

USHORT ToolBox::GetItemId( USHORT nPos ) const
{
    return (nPos < mpData->m_aItems.size()) ? mpData->m_aItems[nPos].mnId : 0;
}

// -----------------------------------------------------------------------

USHORT ToolBox::GetItemId( const Point& rPos ) const
{
    // Item suchen, das geklickt wurde
    std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
    while( it != mpData->m_aItems.end() )
    {
        // Ist es dieses Item
        if ( it->maRect.IsInside( rPos ) )
        {
            if ( it->meType == TOOLBOXITEM_BUTTON )
                return it->mnId;
            else
                return 0;
        }

        ++it;
    }

    return 0;
}

// -----------------------------------------------------------------------

Rectangle ToolBox::GetItemRect( USHORT nItemId ) const
{
    if ( mbCalc || mbFormat )
        ((ToolBox*)this)->ImplFormat();

    USHORT nPos = GetItemPos( nItemId );
    return GetItemPosRect( nPos );
}

// -----------------------------------------------------------------------

Rectangle ToolBox::GetItemPosRect( USHORT nPos ) const
{
    if ( mbCalc || mbFormat )
        ((ToolBox*)this)->ImplFormat();

    if ( nPos < mpData->m_aItems.size() )
        return mpData->m_aItems[nPos].maRect;
    else
        return Rectangle();
}

// -----------------------------------------------------------------------

void ToolBox::SetItemBits( USHORT nItemId, ToolBoxItemBits nBits )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos < mpData->m_aItems.size() )
    {
        ToolBoxItemBits nOldBits = mpData->m_aItems[nPos].mnBits;
        mpData->m_aItems[nPos].mnBits = nBits;
        nBits &= TIB_LEFT | TIB_AUTOSIZE | TIB_DROPDOWN;
        nOldBits &= TIB_LEFT | TIB_AUTOSIZE | TIB_DROPDOWN;
        if ( nBits != nOldBits )
            ImplInvalidate( TRUE );
    }
}

// -----------------------------------------------------------------------

ToolBoxItemBits ToolBox::GetItemBits( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mnBits;
    else
        return 0;
}

// -----------------------------------------------------------------------

void ToolBox::SetItemData( USHORT nItemId, void* pNewData )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos < mpData->m_aItems.size() )
    {
        mpData->m_aItems[nPos].mpUserData = pNewData;
        ImplUpdateItem( nPos );
    }
}

// -----------------------------------------------------------------------

void* ToolBox::GetItemData( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mpUserData;
    else
        return NULL;
}

// -----------------------------------------------------------------------

void ToolBox::SetItemImage( USHORT nItemId, const Image& rImage )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        // Nur wenn alles berechnet ist, mehr Aufwand treiben
        if ( !mbCalc )
        {
            Size aOldSize = pItem->maImage.GetSizePixel();
            pItem->maImage = rImage;
            if ( aOldSize != pItem->maImage.GetSizePixel() )
                ImplInvalidate( TRUE );
            else
                ImplUpdateItem( nPos );
        }
        else
            pItem->maImage = rImage;
    }
}

// -----------------------------------------------------------------------

void ToolBox::SetImageList( const ImageList& rImageList )
{
    maImageList = rImageList;

    USHORT nCount = (USHORT)mpData->m_aItems.size();
    for( USHORT i = 0; i < nCount; i++ )
    {
        Image aImage;
        if ( mpData->m_aItems[i].mnId )
            aImage = maImageList.GetImage( mpData->m_aItems[i].mnId );
        if( !!aImage )
            SetItemImage( mpData->m_aItems[i].mnId, aImage );
    }
}

// -----------------------------------------------------------------------

static Image ImplRotImage( const Image& rImage, long nAngle10 )
{
    Image aRet;

    // rotate the image to the new angle
    Bitmap aRotBitmap = rImage.GetBitmap();
    if( rImage.HasMaskColor() )
    {
        aRotBitmap.Rotate( nAngle10, rImage.GetMaskColor() );
        aRet = Image( aRotBitmap, rImage.GetMaskColor() );
    }
    else if( rImage.HasMaskBitmap() )
    {
        aRotBitmap.Rotate( nAngle10, Color( COL_WHITE ) );
        Bitmap aRotMask = rImage.GetMaskBitmap();
        aRotMask.Rotate( nAngle10, Color( COL_WHITE ) );
        aRet = Image( aRotBitmap, aRotMask );
    }
    else
    {
        aRotBitmap.Rotate( nAngle10, Color( COL_WHITE ) );
        aRet = Image( aRotBitmap );
    }
    return aRet;
}

void ToolBox::SetItemImageAngle( USHORT nItemId, long nAngle10 )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        Size aOldSize = pItem->maImage.GetSizePixel();

        long nDeltaAngle = (nAngle10 - pItem->mnImageAngle) % 3600;
        while( nDeltaAngle < 0 )
            nDeltaAngle += 3600;

        pItem->mnImageAngle = nAngle10;
        if( nDeltaAngle && !!pItem->maImage )
        {
            pItem->maImage = ImplRotImage( pItem->maImage, nDeltaAngle );
            if( !!pItem->maHighImage )
                pItem->maHighImage = ImplRotImage( pItem->maHighImage, nDeltaAngle );
        }

        if ( !mbCalc )
        {
            if ( aOldSize != pItem->maImage.GetSizePixel() )
                ImplInvalidate( TRUE );
            else
                ImplUpdateItem( nPos );
        }
    }
}

// -----------------------------------------------------------------------

static Image ImplMirrorImage( const Image& rImage )
{
    Image aRet;

    // rotate the image to the new angle
    Bitmap aMirrorBitmap = rImage.GetBitmap();
    aMirrorBitmap.Mirror( BMP_MIRROR_HORZ );
    if( rImage.HasMaskColor() )
    {
        aRet = Image( aMirrorBitmap, rImage.GetMaskColor() );
    }
    else if( rImage.HasMaskBitmap() )
    {
        Bitmap aMirrorMask = rImage.GetMaskBitmap();
        aMirrorMask.Mirror( BMP_MIRROR_HORZ );
        aRet = Image( aMirrorBitmap, aMirrorMask );
    }
    else
    {
        aRet = Image( aMirrorBitmap );
    }
    return aRet;
}

void ToolBox::SetItemImageMirrorMode( USHORT nItemId, BOOL bMirror )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];

        if( ( pItem->mbMirrorMode && ! bMirror ) ||
            ( ! pItem->mbMirrorMode && bMirror )
            )
        {
            pItem->mbMirrorMode = bMirror ? true : false;
            if( !!pItem->maImage )
            {
                pItem->maImage = ImplMirrorImage( pItem->maImage );
                if( !!pItem->maHighImage )
                    pItem->maHighImage = ImplMirrorImage( pItem->maHighImage );
            }

            if ( !mbCalc )
                ImplUpdateItem( nPos );
        }
    }
}

// -----------------------------------------------------------------------

Image ToolBox::GetItemImage( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->maImage;
    else
        return Image();
}

// -----------------------------------------------------------------------

long ToolBox::GetItemImageAngle( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mnImageAngle;
    else
        return 0;
}

// -----------------------------------------------------------------------

BOOL ToolBox::GetItemImageMirrorMode( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mbMirrorMode;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void ToolBox::SetItemHighImage( USHORT nItemId, const Image& rImage )
{
    ImplToolItem* pItem = ImplGetItem( nItemId );
    if ( pItem )
    {
        DBG_ASSERT( (pItem->maImage.GetSizePixel() == rImage.GetSizePixel()) ||
                    ((!rImage) == TRUE), "ToolBox::SetItemHighImage() - ImageSize != HighImageSize" );
        pItem->maHighImage = rImage;
    }
}

// -----------------------------------------------------------------------

Image ToolBox::GetItemHighImage( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->maHighImage;
    else
        return Image();
}

// -----------------------------------------------------------------------

void ToolBox::SetItemText( USHORT nItemId, const XubString& rText )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        // Nur wenn alles berechnet ist, mehr Aufwand treiben
        if ( !mbCalc &&
             ((meButtonType != BUTTON_SYMBOL) || !pItem->maImage) )
        {
            long nOldWidth = GetCtrlTextWidth( pItem->maText );
            pItem->maText = ImplConvertMenuString( rText );
            mpData->ImplClearLayoutData();
            if ( nOldWidth != GetCtrlTextWidth( pItem->maText ) )
                ImplInvalidate( TRUE );
            else
                ImplUpdateItem( nPos );
        }
        else
            pItem->maText = ImplConvertMenuString( rText );

        // Notify
        ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMTEXTCHANGED, (void*) nPos );
    }
}

// -----------------------------------------------------------------------

const XubString& ToolBox::GetItemText( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->maText;
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void ToolBox::SetItemWindow( USHORT nItemId, Window* pNewWindow )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        pItem->mpWindow = pNewWindow;
        if ( pNewWindow )
            pNewWindow->Hide();
        ImplInvalidate( TRUE );
        ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMWINDOWCHANGED, (void*) nPos );
    }
}

// -----------------------------------------------------------------------

Window* ToolBox::GetItemWindow( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mpWindow;
    else
        return NULL;
}

// -----------------------------------------------------------------------

void ToolBox::StartSelection()
{
    if ( mbDrag )
        EndSelection();

    if ( !mbSelection )
    {
        mbSelection  = TRUE;
        mnCurPos     = TOOLBOX_ITEM_NOTFOUND;
        mnCurItemId  = 0;
        Activate();
    }
}

// -----------------------------------------------------------------------

void ToolBox::EndSelection()
{
    mbCommandDrag = FALSE;

    if ( mbDrag || mbSelection )
    {
        // Daten zuruecksetzen
        mbDrag = FALSE;
        mbSelection = FALSE;
        if ( mnCurPos != TOOLBOX_ITEM_NOTFOUND )
            ImplDrawItem( mnCurPos );
        EndTracking();
        ReleaseMouse();
        Deactivate();
    }

    mnCurPos        = TOOLBOX_ITEM_NOTFOUND;
    mnCurItemId     = 0;
    mnDownItemId    = 0;
    mnMouseClicks   = 0;
    mnMouseModifier = 0;
}

// -----------------------------------------------------------------------

void ToolBox::SetItemDown( USHORT nItemId, BOOL bDown, BOOL bRelease )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        if ( bDown )
        {
            if ( nPos != mnCurPos )
            {
                mnCurPos = nPos;
                ImplDrawItem( mnCurPos );
            }
        }
        else
        {
            if ( nPos == mnCurPos )
            {
                ImplDrawItem( mnCurPos );
                mnCurPos = TOOLBOX_ITEM_NOTFOUND;
            }
        }

        if ( bRelease )
        {
            if ( mbDrag || mbSelection )
            {
                mbDrag = FALSE;
                mbSelection = FALSE;
                EndTracking();
                ReleaseMouse();
                Deactivate();
            }

            mnCurItemId     = 0;
            mnDownItemId    = 0;
            mnMouseClicks   = 0;
            mnMouseModifier = 0;
        }
    }
}

// -----------------------------------------------------------------------

BOOL ToolBox::IsItemDown( USHORT nItemId ) const
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
        return (nPos == mnCurPos);
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void ToolBox::SetItemState( USHORT nItemId, TriState eState )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];

        // Hat sich der Status geaendert
        if ( pItem->meState != eState )
        {
            // Wenn RadioCheck, dann vorherigen unchecken
            if ( (eState == STATE_CHECK) && (pItem->mnBits & TIB_AUTOCHECK) &&
                 (pItem->mnBits & TIB_RADIOCHECK) )
            {
                ImplToolItem*    pGroupItem;
                USHORT          nGroupPos;
                USHORT          nItemCount = GetItemCount();

                nGroupPos = nPos;
                while ( nGroupPos )
                {
                    pGroupItem = &mpData->m_aItems[nGroupPos-1];
                    if ( pGroupItem->mnBits & TIB_RADIOCHECK )
                    {
                        if ( pGroupItem->meState != STATE_NOCHECK )
                            SetItemState( pGroupItem->mnId, STATE_NOCHECK );
                    }
                    else
                        break;
                    nGroupPos--;
                }

                nGroupPos = nPos+1;
                while ( nGroupPos < nItemCount )
                {
                    pGroupItem = &mpData->m_aItems[nGroupPos];
                    if ( pGroupItem->mnBits & TIB_RADIOCHECK )
                    {
                        if ( pGroupItem->meState != STATE_NOCHECK )
                            SetItemState( pGroupItem->mnId, STATE_NOCHECK );
                    }
                    else
                        break;
                    nGroupPos++;
                }
            }

            pItem->meState = eState;
            ImplUpdateItem( nPos );
            // Notify
            ImplCallEventListeners( VCLEVENT_TOOLBOX_CLICK, (void*) nPos );
        }
    }
}

// -----------------------------------------------------------------------

TriState ToolBox::GetItemState( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->meState;
    else
        return STATE_NOCHECK;
}

// -----------------------------------------------------------------------

void ToolBox::EnableItem( USHORT nItemId, BOOL bEnable )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        if ( bEnable )
            bEnable = TRUE;
        if ( pItem->mbEnabled != bEnable )
        {
            pItem->mbEnabled = bEnable;

            // Gegebenenfalls das Fenster mit updaten
            if ( pItem->mpWindow )
                pItem->mpWindow->Enable( pItem->mbEnabled );

            // Item updaten
            ImplUpdateItem( nPos );

            ImplUpdateInputEnable();

            ImplCallEventListeners( bEnable ? VCLEVENT_TOOLBOX_ITEMENABLED : VCLEVENT_TOOLBOX_ITEMDISABLED, (void*)nPos );
        }
    }
}

// -----------------------------------------------------------------------

BOOL ToolBox::IsItemEnabled( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mbEnabled;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void ToolBox::ShowItem( USHORT nItemId, BOOL bVisible )
{
    USHORT nPos = GetItemPos( nItemId );
    mpData->ImplClearLayoutData();

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        if ( pItem->mbVisible != bVisible )
        {
            pItem->mbVisible = bVisible;
            ImplInvalidate( FALSE );
        }
    }
}

// -----------------------------------------------------------------------

BOOL ToolBox::IsItemVisible( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mbVisible;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void ToolBox::SetItemCommand( USHORT nItemId, const XubString& rCommand )
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        pItem->maCommandStr = rCommand;
}

// -----------------------------------------------------------------------

const XubString& ToolBox::GetItemCommand( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->maCommandStr;
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void ToolBox::SetQuickHelpText( USHORT nItemId, const XubString& rText )
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        pItem->maQuickHelpText = rText;
}

// -----------------------------------------------------------------------

const XubString& ToolBox::GetQuickHelpText( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->maQuickHelpText;
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void ToolBox::SetHelpText( USHORT nItemId, const XubString& rText )
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        pItem->maHelpText = rText;
}

// -----------------------------------------------------------------------

const XubString& ToolBox::GetHelpText( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
    {
        if ( !pItem->maHelpText.Len() && pItem->mnHelpId )
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
                pItem->maHelpText = pHelp->GetHelpText( pItem->mnHelpId, this );
        }

        return pItem->maHelpText;
    }
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void ToolBox::SetHelpId( USHORT nItemId, ULONG nHelpId )
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        pItem->mnHelpId = nHelpId;
}

// -----------------------------------------------------------------------

ULONG ToolBox::GetHelpId( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mnHelpId;
    else
        return 0;
}

// -----------------------------------------------------------------------

void ToolBox::SetBorder( long nX, long nY )
{
    mnBorderX = nX;
    mnBorderY = nY;

    ImplInvalidate( TRUE, TRUE );
}

// -----------------------------------------------------------------------

void ToolBox::SetOutStyle( USHORT nNewStyle )
{
    if ( mnOutStyle != nNewStyle )
    {
        mnOutStyle = nNewStyle;

        // Damit das ButtonDevice neu angelegt wird
        if ( !(mnOutStyle & TOOLBOX_STYLE_FLAT) )
        {
            mnItemWidth  = 1;
            mnItemHeight = 1;
        }

        ImplInvalidate( TRUE, TRUE );
    }
}

// -----------------------------------------------------------------------

void ToolBox::RecalcItems()
{
    ImplInvalidate( TRUE );
}

// -----------------------------------------------------------------------

// disable key input if all items are disabled

void ToolBox::ImplUpdateInputEnable()
{
    for( std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
         it != mpData->m_aItems.end(); ++it )
    {
        if( it->mbEnabled )
        {
            // at least one useful entry
            mbInputDisabled = FALSE;
            return;
        }
    }
    mbInputDisabled = TRUE;
}

// -----------------------------------------------------------------------

void ToolBox::ImplFillLayoutData() const
{
    mpData->m_pLayoutData = new ToolBoxLayoutData();

    USHORT nCount = (USHORT)mpData->m_aItems.size();
    for( USHORT i = 0; i < nCount; i++ )
    {
        ImplToolItem* pItem = &mpData->m_aItems[i];

        // Nur malen, wenn Rechteck im PaintRectangle liegt
        if ( !pItem->maRect.IsEmpty() )
            const_cast<ToolBox*>(this)->ImplDrawItem( i, FALSE, FALSE, TRUE );
    }
}

// -----------------------------------------------------------------------

String ToolBox::GetDisplayText() const
{
    if( ! mpData->m_pLayoutData )
        ImplFillLayoutData();
    return mpData->m_pLayoutData ? mpData->m_pLayoutData->m_aDisplayText : String();
}

// -----------------------------------------------------------------------

Rectangle ToolBox::GetCharacterBounds( USHORT nItemID, long nIndex ) const
{
    long nItemIndex = -1;
    if( ! mpData->m_pLayoutData )
        ImplFillLayoutData();
    if( mpData->m_pLayoutData )
    {
        for( ULONG i = 0; i < mpData->m_pLayoutData->m_aLineItemIds.size(); i++ )
        {
            if( mpData->m_pLayoutData->m_aLineItemIds[i] == nItemID )
            {
                nItemIndex = mpData->m_pLayoutData->m_aLineIndices[i];
                break;
            }
        }
    }
    return (mpData->m_pLayoutData && nItemIndex != -1) ? mpData->m_pLayoutData->GetCharacterBounds( nItemIndex+nIndex ) : Rectangle();
}

// -----------------------------------------------------------------------

long ToolBox::GetIndexForPoint( const Point& rPoint, USHORT& rItemID ) const
{
    long nIndex = -1;
    rItemID = 0;
    if( ! mpData->m_pLayoutData )
        ImplFillLayoutData();
    if( mpData->m_pLayoutData )
    {
        nIndex = mpData->m_pLayoutData->GetIndexForPoint( rPoint );
        for( ULONG i = 0; i < mpData->m_pLayoutData->m_aLineIndices.size(); i++ )
        {
            if( mpData->m_pLayoutData->m_aLineIndices[i] <= nIndex &&
                (i == mpData->m_pLayoutData->m_aLineIndices.size()-1 || mpData->m_pLayoutData->m_aLineIndices[i+1] > nIndex) )
            {
                rItemID = mpData->m_pLayoutData->m_aLineItemIds[i];
                break;
            }
        }
    }
    return nIndex;
}

// -----------------------------------------------------------------------

long ToolBox::GetTextCount() const
{
    if( ! mpData->m_pLayoutData )
        ImplFillLayoutData();
    return mpData->m_pLayoutData ? mpData->m_pLayoutData->GetLineCount() : 0;
}

// -----------------------------------------------------------------------

Pair ToolBox::GetTextStartEnd( long nText ) const
{
    if( ! mpData->m_pLayoutData )
        ImplFillLayoutData();
    return mpData->m_pLayoutData ? mpData->m_pLayoutData->GetLineStartEnd( nText ) : Pair( -1, -1 );
}

// -----------------------------------------------------------------------

USHORT ToolBox::GetDisplayItemId( long nText ) const
{
    USHORT nItemId = 0;
    if( ! mpData->m_pLayoutData )
        ImplFillLayoutData();
    if( mpData->m_pLayoutData && nText >= 0 && (ULONG)nText < mpData->m_pLayoutData->m_aLineItemIds.size() )
        nItemId = mpData->m_pLayoutData->m_aLineItemIds[nText];
    return nItemId;
}

