/*************************************************************************
 *
 *  $RCSfile: toolbox2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:40 $
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
}

// -----------------------------------------------------------------------

ImplToolItem::~ImplToolItem()
{
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
                USHORT nItemCount = (USHORT)mpItemList->Count();
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
                ImplDrawItem( nIndex, (nIndex == mnCurPos) ? TRUE : FALSE );
            else
                maPaintRect.Union( mpItemList->GetObject( nIndex )->maRect );
        }
    }
}

// -----------------------------------------------------------------------

void ToolBox::Click()
{
    maClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void ToolBox::DoubleClick()
{
    maDoubleClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void ToolBox::Activate()
{
    maActivateHdl.Call( this );
}

// -----------------------------------------------------------------------

void ToolBox::Deactivate()
{
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
    ImplToolItem* pItem = new ImplToolItem;

    GetRes( rResId.SetRT( RSC_TOOLBOXITEM ) );
    nObjMask            = ReadShortRes();

    if ( nObjMask & RSC_TOOLBOXITEM_ID )
        pItem->mnId = ReadShortRes();
    else
        pItem->mnId = 1;

    if ( nObjMask & RSC_TOOLBOXITEM_TYPE )
        pItem->meType = (ToolBoxItemType)ReadShortRes();

    if ( nObjMask & RSC_TOOLBOXITEM_STATUS )
        pItem->mnBits = (ToolBoxItemBits)ReadShortRes();

    if( nObjMask & RSC_TOOLBOXITEM_HELPID )
        pItem->mnHelpId = ReadLongRes();

    if ( nObjMask & RSC_TOOLBOXITEM_TEXT )
    {
        pItem->maText = ReadStringRes();
        pItem->maText = ImplConvertMenuString( pItem->maText );
    }
    if ( nObjMask & RSC_TOOLBOXITEM_HELPTEXT )
        pItem->maHelpText = ReadStringRes();

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

    if( (nHelpMode & HELPTEXTMODE_EXTERN) && pItem->aHelpText.Len() )
        pItem->aHelpText.Erase();
    if( (nHelpMode & HELPTEXTMODE_DEBUG) && !pItem->nHelpId )
        pItem->aHelpText = "??? !pItem->nHelpId MP/W.P. ???";
*/

    if ( nObjMask & RSC_TOOLBOXITEM_BITMAP )
    {
        Bitmap aBmp = Bitmap( ResId( (RSHEADER_TYPE*)GetClassRes() ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
        pItem->maImage = Image( aBmp, IMAGE_STDBTN_COLOR );
        bImage = TRUE;
    }
    if ( nObjMask & RSC_TOOLBOXITEM_IMAGE )
    {
        pItem->maImage = Image( ResId( (RSHEADER_TYPE*)GetClassRes() ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
        bImage = TRUE;
    }
    if ( nObjMask & RSC_TOOLBOXITEM_DISABLE )
        pItem->mbEnabled = !(BOOL)ReadShortRes();

    if ( nObjMask & RSC_TOOLBOXITEM_STATE )
        pItem->meState  = (TriState)ReadShortRes();

    if ( nObjMask & RSC_TOOLBOXITEM_HIDE )
        pItem->mbVisible = !((BOOL)ReadShortRes());

    if ( nObjMask & RSC_TOOLBOXITEM_COMMAND )
        pItem->maCommandStr = ReadStringRes();

    // Wenn kein Image geladen wurde, versuchen wir das Image aus der
    // Image-Liste zu holen
    if ( !bImage && pItem->mnId )
        pItem->maImage = maImageList.GetImage( pItem->mnId );

    // Wenn es sich um ein ButtonItem handelt, die ID ueberpruefen
    BOOL bNewCalc;
    if ( pItem->meType != TOOLBOXITEM_BUTTON )
    {
        bNewCalc = FALSE;
        pItem->mnId = 0;
    }
    else
    {
        bNewCalc = TRUE;

        DBG_ASSERT( pItem->mnId, "ToolBox::InsertItem(): ItemId == 0" );
        DBG_ASSERT( GetItemPos( pItem->mnId ) == TOOLBOX_ITEM_NOTFOUND,
                    "ToolBox::InsertItem(): ItemId already exists" );
    }

    // Item anlegen und in die Liste einfuegen
    mpItemList->Insert( pItem, nPos );

    // ToolBox neu brechnen und neu ausgeben
    ImplInvalidate( bNewCalc );
}

// -----------------------------------------------------------------------

void ToolBox::InsertItem( USHORT nItemId, const Image& rImage,
                          ToolBoxItemBits nBits, USHORT nPos )
{
    DBG_ASSERT( nItemId, "ToolBox::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == TOOLBOX_ITEM_NOTFOUND,
                "ToolBox::InsertItem(): ItemId already exists" );

    // Item anlegen und in die Liste einfuegen
    mpItemList->Insert( new ImplToolItem( nItemId, rImage, nBits ), nPos );

    ImplInvalidate( TRUE );
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
    mpItemList->Insert( new ImplToolItem( nItemId, rImage, ImplConvertMenuString( rText ), nBits ), nPos );

    ImplInvalidate( TRUE );
}

// -----------------------------------------------------------------------

void ToolBox::InsertItem( USHORT nItemId, const XubString& rText,
                          ToolBoxItemBits nBits, USHORT nPos )
{
    DBG_ASSERT( nItemId, "ToolBox::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == TOOLBOX_ITEM_NOTFOUND,
                "ToolBox::InsertItem(): ItemId already exists" );

    // Item anlegen und in die Liste einfuegen
    mpItemList->Insert( new ImplToolItem( nItemId, ImplConvertMenuString( rText ), nBits ), nPos );

    ImplInvalidate( TRUE );
}

// -----------------------------------------------------------------------

void ToolBox::InsertWindow( USHORT nItemId, Window* pWindow,
                            ToolBoxItemBits nBits, USHORT nPos )
{
    DBG_ASSERT( nItemId, "ToolBox::InsertWindow(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == TOOLBOX_ITEM_NOTFOUND,
                "ToolBox::InsertWindow(): ItemId already exists" );

    // Item anlegen und in die Liste einfuegen
    ImplToolItem* pItem  = new ImplToolItem;
    pItem->mnId          = nItemId;
    pItem->meType        = TOOLBOXITEM_BUTTON;
    pItem->mnBits        = nBits;
    pItem->mpWindow      = pWindow;
    mpItemList->Insert( pItem, nPos );

    if ( pWindow )
        pWindow->Hide();

    ImplInvalidate( TRUE );
}

// -----------------------------------------------------------------------

void ToolBox::InsertSpace( USHORT nPos )
{
    // Item anlegen und in die Liste einfuegen
    ImplToolItem* pItem  = new ImplToolItem;
    pItem->meType        = TOOLBOXITEM_SPACE;
    pItem->mbEnabled     = FALSE;
    mpItemList->Insert( pItem, nPos );

    ImplInvalidate( FALSE );
}

// -----------------------------------------------------------------------

void ToolBox::InsertSeparator( USHORT nPos, USHORT nPixSize )
{
    // Item anlegen und in die Liste einfuegen
    ImplToolItem* pItem  = new ImplToolItem;
    pItem->meType        = TOOLBOXITEM_SEPARATOR;
    pItem->mbEnabled     = FALSE;
    if ( nPixSize )
        pItem->mnSepSize = nPixSize;
    mpItemList->Insert( pItem, nPos );

    ImplInvalidate( FALSE );
}

// -----------------------------------------------------------------------

void ToolBox::InsertBreak( USHORT nPos )
{
    // Item anlegen und in die Liste einfuegen
    ImplToolItem* pItem  = new ImplToolItem;
    pItem->meType        = TOOLBOXITEM_BREAK;
    pItem->mbEnabled     = FALSE;
    mpItemList->Insert( pItem, nPos );

    ImplInvalidate( FALSE );
}

// -----------------------------------------------------------------------

void ToolBox::RemoveItem( USHORT nPos )
{
    ImplToolItem* pItem = mpItemList->Remove( nPos );

    // Item entfernen
    if ( pItem )
    {
        BOOL bMustCalc;
        if ( pItem->meType == TOOLBOXITEM_BUTTON )
            bMustCalc = TRUE;
        else
            bMustCalc = FALSE;

        if ( pItem->mpWindow )
            pItem->mpWindow->Hide();

        // PaintRect um das removete Item erweitern
        maPaintRect.Union( pItem->maRect );

        // Absichern gegen das Loeschen im Select-Handler
        if ( pItem->mnId == mnCurItemId )
            mnCurItemId = 0;
        if ( pItem->mnId == mnHighItemId )
            mnHighItemId = 0;

        ImplInvalidate( bMustCalc );

        delete pItem;
    }
}

// -----------------------------------------------------------------------

void ToolBox::MoveItem( USHORT nItemId, USHORT nNewPos )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos < nNewPos )
        nNewPos--;

    if ( nPos == nNewPos )
        return;

    // Existiert Item
    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        // ToolBox-Item in der Liste verschieben
        ImplToolItem* pItem = mpItemList->Remove( nPos );
        mpItemList->Insert( pItem, nNewPos );

        // ToolBox neu ausgeben
        ImplInvalidate( FALSE );
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
        ImplToolItem* pItem = rToolBox.mpItemList->GetObject( nPos );
        ImplToolItem* pNewItem = new ImplToolItem( *pItem );
        mpItemList->Insert( pNewItem, nNewPos );
        // Bestimme Daten zuruecksetzen
        pNewItem->mpWindow     = NULL;
        pNewItem->mbShowWindow = FALSE;

        // ToolBox neu ausgeben
        ImplInvalidate( FALSE );
    }
}

// -----------------------------------------------------------------------

void ToolBox::CopyItems( const ToolBox& rToolBox )
{
    ImplToolItem* pItem;

    // Alle Items entfernen
    pItem = mpItemList->First();
    while ( pItem )
    {
        delete pItem;
        pItem = mpItemList->Next();
    }

    // Items aus der Liste loeschen
    mpItemList->Clear();

    // Absichern gegen das Loeschen im Select-Handler
    mnCurItemId = 0;
    mnHighItemId = 0;

    // Items kopieren
    ULONG i = 0;
    pItem = rToolBox.mpItemList->GetObject( i );
    while ( pItem )
    {
        ImplToolItem* pNewItem = new ImplToolItem( *pItem );
        mpItemList->Insert( pNewItem, LIST_APPEND );
        // Bestimme Daten zuruecksetzen
        pNewItem->mpWindow     = NULL;
        pNewItem->mbShowWindow = FALSE;
        i++;
        pItem = rToolBox.mpItemList->GetObject( i );
    }

    ImplInvalidate( TRUE, TRUE );
}

// -----------------------------------------------------------------------

void ToolBox::Clear()
{
    ImplToolItem* pItem;

    // Alle Item loeschen
    pItem = mpItemList->First();
    while ( pItem )
    {
        delete pItem;
        pItem = mpItemList->Next();
    }

    // Items aus der Liste loeschen
    mpItemList->Clear();

    // Absichern gegen das Loeschen im Select-Handler
    mnCurItemId = 0;
    mnHighItemId = 0;

    ImplInvalidate( TRUE, TRUE );
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
    return (USHORT)mpItemList->Count();
}

// -----------------------------------------------------------------------

ToolBoxItemType ToolBox::GetItemType( USHORT nPos ) const
{
    ImplToolItem* pItem = mpItemList->GetObject( nPos );
    if ( pItem )
        return pItem->meType;
    else
        return TOOLBOXITEM_DONTKNOW;
}

// -----------------------------------------------------------------------

USHORT ToolBox::GetItemPos( USHORT nItemId ) const
{
    ImplToolItem* pItem = mpItemList->First();
    while ( pItem )
    {
        if ( pItem->mnId == nItemId )
            return (USHORT)mpItemList->GetCurPos();

        pItem = mpItemList->Next();
    }

    return TOOLBOX_ITEM_NOTFOUND;
}

// -----------------------------------------------------------------------

USHORT ToolBox::GetItemId( USHORT nPos ) const
{
    ImplToolItem* pItem = mpItemList->GetObject( nPos );
    if ( pItem )
        return pItem->mnId;
    else
        return 0;
}

// -----------------------------------------------------------------------

USHORT ToolBox::GetItemId( const Point& rPos ) const
{
    // Item suchen, das geklickt wurde
    ImplToolItem*  pItem = mpItemList->First();
    while ( pItem )
    {
        // Ist es dieses Item
        if ( pItem->maRect.IsInside( rPos ) )
        {
            if ( pItem->meType == TOOLBOXITEM_BUTTON )
                return pItem->mnId;
            else
                return 0;
        }

        pItem = mpItemList->Next();
    }

    return 0;
}

// -----------------------------------------------------------------------

Rectangle ToolBox::GetItemRect( USHORT nItemId ) const
{
    if ( mbCalc || mbFormat )
        ((ToolBox*)this)->ImplFormat();

    USHORT nPos = GetItemPos( nItemId );
    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
        return mpItemList->GetObject( nPos )->maRect;
    else
        return Rectangle();
}

// -----------------------------------------------------------------------

void ToolBox::SetItemBits( USHORT nItemId, ToolBoxItemBits nBits )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = mpItemList->GetObject( nPos );
        ToolBoxItemBits nOldBits = pItem->mnBits;
        pItem->mnBits = nBits;
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

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = mpItemList->GetObject( nPos );
        pItem->mpUserData = pNewData;
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
        ImplToolItem* pItem = mpItemList->GetObject( nPos );
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

Image ToolBox::GetItemImage( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->maImage;
    else
        return Image();
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
        ImplToolItem* pItem = mpItemList->GetObject( nPos );
        // Nur wenn alles berechnet ist, mehr Aufwand treiben
        if ( !mbCalc &&
             ((meButtonType != BUTTON_SYMBOL) || !pItem->maImage) )
        {
            long nOldWidth = GetCtrlTextWidth( pItem->maText );
            pItem->maText = ImplConvertMenuString( rText );
            if ( nOldWidth != GetCtrlTextWidth( pItem->maText ) )
                ImplInvalidate( TRUE );
            else
                ImplUpdateItem( nPos );
        }
        else
            pItem->maText = ImplConvertMenuString( rText );
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
        ImplToolItem* pItem = mpItemList->GetObject( nPos );
        pItem->mpWindow = pNewWindow;
        if ( pNewWindow )
            pNewWindow->Hide();
        ImplInvalidate( TRUE );
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
        ImplToolItem* pItem = mpItemList->GetObject( nPos );

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
                    pGroupItem = mpItemList->GetObject( nGroupPos-1 );
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
                    pGroupItem = mpItemList->GetObject( nGroupPos );
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
        ImplToolItem* pItem = mpItemList->GetObject( nPos );
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

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = mpItemList->GetObject( nPos );
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
                pItem->maHelpText = pHelp->GetHelpText( pItem->mnHelpId );
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
