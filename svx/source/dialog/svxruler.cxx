/*************************************************************************
 *
 *  $RCSfile: svxruler.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:12 $
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

// INCLUDE ---------------------------------------------------------------

#include <string.h>
#include <limits.h>

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXRECTITEM_HXX
#include <svtools/rectitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#pragma hdrstop

#define ITEMID_TABSTOP  0
#define ITEMID_LRSPACE  0
#define ITEMID_PROTECT  0

#include "dialogs.hrc"
#include "dialmgr.hxx"
#include "ruler.hxx"
#include "rlrcitem.hxx"
#include "rulritem.hxx"
#include "tstpitem.hxx"
#include "lrspitem.hxx"
#include "protitem.hxx"

// STATIC DATA -----------------------------------------------------------

#define CTRL_ITEM_COUNT 11
#define GAP 10
#define OBJECT_BORDER_COUNT 4
#define TAB_GAP 1
#define INDENT_GAP 2

#define PIXEL_H_ADJUST( l1, l2 ) (USHORT)PixelHAdjust(l1,l2)

#ifdef DEBUGLIN

inline long ToMM(Window *pWin, long lVal)
{
    return pWin->PixelToLogic(Size(lVal, 0), MapMode(MAP_MM)).Width();
}

void Debug_Impl(Window *pWin, SvxColumnItem& rColItem)
{
    String aTmp("Aktuell: ");
    aTmp += rColItem.GetActColumn();
    aTmp += " ColLeft: ";
    aTmp +=  String(ToMM(pWin, rColItem.GetLeft()));
    aTmp += "   ColRight: ";
    aTmp +=  String(ToMM(pWin, rColItem.GetRight()));
    for(USHORT i = 0; i < rColItem.Count(); ++i) {
        aTmp += " Start: ";
        aTmp += String(ToMM(pWin, rColItem[i].nStart));
        aTmp += " End: ";
        aTmp += String(ToMM(pWin, rColItem[i].nEnd));
    }

    InfoBox(0, aTmp).Execute();
}

void Debug_Impl(Window *pWin, const SvxLongLRSpaceItem& rLRSpace)
{
    String aTmp("Left: ");
    aTmp += pWin->PixelToLogic(Size(rLRSpace.GetLeft(), 0), MapMode(MAP_MM)).Width();
    aTmp += "   Right: ";
    aTmp +=pWin->PixelToLogic(Size(rLRSpace.GetRight(), 0), MapMode(MAP_MM)).Width();
    InfoBox(0, aTmp).Execute();
}

void Debug_Impl(Window *pWin, const SvxLongULSpaceItem& rULSpace)
{
    String aTmp("Upper: ");
    aTmp += pWin->PixelToLogic(Size(rULSpace.GetUpper(), 0), MapMode(MAP_MM)).Width();
    aTmp += "   Lower: ";
    aTmp += pWin->PixelToLogic(Size(rULSpace.GetLower(), 0), MapMode(MAP_MM)).Width();

    InfoBox(0, aTmp).Execute();
}

void DebugTabStops_Impl(const SvxTabStopItem& rTabs)
{
    String aTmp("Tabs: ");

    // Def Tabs loeschen
    for(USHORT i = 0; i < rTabs.Count(); ++i)
    {
        aTmp += String(rTabs[i].GetTabPos() / 56);
        aTmp += " : ";
    }
    InfoBox(0, aTmp).Execute();
}

void DebugParaMargin_Impl(const SvxLRSpaceItem& rLRSpace)
{
    String aTmp("ParaLeft: ");
    aTmp += rLRSpace.GetTxtLeft() / 56;
    aTmp += "   ParaRight: ";
    aTmp += rLRSpace.GetRight() / 56;
    aTmp += "   FLI: ";
    aTmp += rLRSpace.GetTxtFirstLineOfst() / 56;
    InfoBox(0, aTmp).Execute();
}

#endif // DEBUGLIN

struct SvxRuler_Impl  {
    USHORT *pPercBuf;
    USHORT *pBlockBuf;
    USHORT nPercSize;
    long   nTotalDist;
    long   lOldWinPos;
    long   lMaxLeftLogic;
    long   lMaxRightLogic;
    long   lLastLMargin;
    SvxProtectItem aProtectItem;
    USHORT nControlerItems;
    USHORT nIdx;
    USHORT nColLeftPix, nColRightPix; // Pixelwerte fuer linken / rechten Rand
                                      // bei Spalten; gepuffert, um Umrechenfehler
                                      // zu vermeiden.
                                      // Muesste vielleicht fuer weitere Werte
                                      // aufgebohrt werden
    SvxRuler_Impl() :
    pPercBuf(0), pBlockBuf(0), nPercSize(0), nTotalDist(0), nIdx(0),
    nColLeftPix(0), nColRightPix(0), lOldWinPos(0)
    {
    }
    ~SvxRuler_Impl()
    {
        nPercSize = 0; nTotalDist = 0;
        delete pPercBuf; delete pBlockBuf; pPercBuf = 0;
    }
    void SetPercSize(USHORT nSize);

};



void SvxRuler_Impl::SetPercSize(USHORT nSize)
{
    if(nSize > nPercSize)
    {
        delete pPercBuf;
        delete pBlockBuf;
        pPercBuf = new USHORT[nPercSize = nSize];
        pBlockBuf = new USHORT[nPercSize = nSize];
    }
    size_t nSize2 = sizeof(USHORT) * nPercSize;
    memset(pPercBuf, 0, nSize2);
    memset(pBlockBuf, 0, nSize2);
}


// Konstruktor des Lineals

// SID_ATTR_ULSPACE, SID_ATTR_LRSPACE
// erwartet als Parameter SvxULSpaceItem f"ur Seitenr"ander
// (entweder links/rechts oder oben/unten)
// Lineal: SetMargin1, SetMargin2

// SID_RULER_PAGE_POS
// erwartet als Parameter Anfangswert der Seite sowie Seitenbreite
// Lineal: SetPagePos

// SID_ATTR_TABSTOP
// erwartet: SvxTabStopItem
// Lineal: SetTabs

// SID_ATTR_PARA_LRSPACE
// linker, rechter Absatzrand bei H-Lineal
// Lineal: SetIndents

// SID_RULER_BORDERS
// Tabellenraender, Spalten
// erwartet: so etwas wie SwTabCols
// Lineal: SetBorders


SvxRuler::SvxRuler
(
 Window* pParent,                               // StarView Parent
 Window* pWin,                                  // Ausgabefenster; wird fuer Umrechnung logische
                // Einheiten <-> Pixel verwendet
 USHORT  flags,                                 // Anzeige Flags, siehe ruler.hxx
 SfxBindings &rBindings,                // zugeordnete Bindings
 WinBits nWinStyle                              // StarView WinBits
)
: Ruler(pParent, nWinStyle),
  pLRSpaceItem(0),
  pMinMaxItem(0),
  pULSpaceItem(0),
  pTabStopItem(0),
  pParaItem(0),
  pPagePosItem(0),
  pColumnItem(0),
  pObjectItem(0),
  pEditWin(pWin),
  pIndents(0),
  pBorders(new RulerBorder[1]), //wg 1 Spaltiger Tabellen
  pObjectBorders(0),
  nBorderCount(0),
  nDefTabType(RULER_TAB_LEFT),
  nTabCount(0),
  lDefTabDist(50),
  lTabPos(-1),
  nTabBufSize(0),
  pTabs(0),
  nFlags(flags),
  nDragOffset(0),
  nDragType(NONE),
  nMaxLeft(0),
  nMaxRight(0),
  bListening(FALSE),
  bValid(FALSE),
  bActive(TRUE),
  bAppSetNullOffset(FALSE),  //Wird der 0-Offset des Lineals
                             //durch die appl. gesetzt?
  pBindings(&rBindings),
  lLogicNullOffset(0),
  lAppNullOffset(LONG_MAX),
  lMinFrame(10),
  lInitialDragPos(0),
  pCtrlItem(new SvxRulerItem *[CTRL_ITEM_COUNT]),
  pRuler_Imp(new SvxRuler_Impl)
/*
   [Beschreibung]

   ctor;
   Datenpuffer initialisieren; ControllerItems werden erzeugt

*/
{
    memset(pCtrlItem, 0, sizeof(SvxRulerItem *) * CTRL_ITEM_COUNT);

    rBindings.EnterRegistrations();

    // Unterstuetzte Items anlegen
    USHORT i = 0;
    // Seitenraender

    pCtrlItem[i++] = new SvxRulerItem(SID_RULER_LR_MIN_MAX, *this, rBindings);
    if((nWinStyle & WB_VSCROLL) == WB_VSCROLL)
    {
        bHorz = FALSE;
        pCtrlItem[i++] = new SvxRulerItem(SID_ATTR_LONG_ULSPACE, *this, rBindings);
    }
    else
    {
        bHorz = TRUE;
        pCtrlItem[i++] = new SvxRulerItem(SID_ATTR_LONG_LRSPACE, *this, rBindings);
    }

    // Seitenposition
    pCtrlItem[i++] = new SvxRulerItem(SID_RULER_PAGE_POS, *this, rBindings);

    if((nFlags & SVXRULER_SUPPORT_TABS) == SVXRULER_SUPPORT_TABS)
    {
        DBG_ASSERT((nWinStyle & WB_HSCROLL) == WB_HSCROLL,
                   "kein Tabs im vertikalen Lineal");
        pCtrlItem[i++] = new SvxRulerItem(SID_ATTR_TABSTOP, *this, rBindings);
        SetExtraType(RULER_EXTRA_TAB, nDefTabType);
    }

    if((nFlags & SVXRULER_SUPPORT_PARAGRAPH_MARGINS) ==
       SVXRULER_SUPPORT_PARAGRAPH_MARGINS)
    {
        pCtrlItem[i++] = new SvxRulerItem(SID_ATTR_PARA_LRSPACE, *this, rBindings);
        pIndents = new RulerIndent[3+INDENT_GAP];
        memset(pIndents, 0, sizeof(RulerIndent)*(3+INDENT_GAP));
        pIndents[0].nStyle = RULER_STYLE_DONTKNOW;
        pIndents[1].nStyle = RULER_STYLE_DONTKNOW;
        pIndents[2].nStyle = RULER_INDENT_TOP;
        pIndents[3].nStyle = RULER_INDENT_BOTTOM;
        pIndents[4].nStyle = RULER_INDENT_BOTTOM;
        pIndents[0].nPos = 0;
        pIndents[1].nPos = 0;
        pIndents[2].nPos = 0;
        pIndents[3].nPos = 0;
        pIndents[4].nPos = 0;
    }

    if((nFlags & SVXRULER_SUPPORT_BORDERS) ==  SVXRULER_SUPPORT_BORDERS)
        pCtrlItem[i++] = new SvxRulerItem(SID_RULER_BORDERS, *this, rBindings);

    if((nFlags & SVXRULER_SUPPORT_OBJECT) == SVXRULER_SUPPORT_OBJECT)
    {
        pCtrlItem[i++] = new SvxRulerItem(SID_RULER_OBJECT, *this, rBindings );
        pObjectBorders = new RulerBorder[OBJECT_BORDER_COUNT];
        size_t nSize = sizeof( RulerBorder ) * OBJECT_BORDER_COUNT;
        memset(pObjectBorders, 0, nSize);
        for(USHORT i = 0; i < OBJECT_BORDER_COUNT; ++i)
        {
            pObjectBorders[i].nPos   = 0;
            pObjectBorders[i].nWidth = 0;
            pObjectBorders[i].nStyle = RULER_BORDER_MOVEABLE;
        }
    }

    pCtrlItem[i++] = new SvxRulerItem( SID_RULER_PROTECT, *this, rBindings );
    pRuler_Imp->nControlerItems=i;

    if((nFlags & SVXRULER_SUPPORT_SET_NULLOFFSET) ==
       SVXRULER_SUPPORT_SET_NULLOFFSET)
        SetExtraType(RULER_EXTRA_NULLOFFSET, 0);

    rBindings.LeaveRegistrations();
}


__EXPORT SvxRuler::~SvxRuler()
/*
   [Beschreibung]

   Destruktor Lineal
   Freigabe interner Puffer


*/
{
    if(bListening)
        EndListening(*pBindings);

    pBindings->EnterRegistrations();
    for(USHORT i = 0; i < CTRL_ITEM_COUNT  && pCtrlItem[i]; ++i)
        delete pCtrlItem[i];
    delete pCtrlItem;
    delete pLRSpaceItem;
    delete pMinMaxItem;
    delete pULSpaceItem;
    delete pTabStopItem;
    delete pParaItem;
    delete pPagePosItem;
    delete pColumnItem;
    delete pObjectItem;
    delete pIndents;
    delete pBorders;
    delete pObjectBorders;
    delete pTabs;
    delete pRuler_Imp;
    pBindings->LeaveRegistrations();
}

/*

   [Beschreibung]

   Interne Umrechenroutinen

*/

long SvxRuler::ConvertHPosPixel(long nVal) const
{
    return pEditWin->LogicToPixel(Size(nVal, 0)).Width();
}

long SvxRuler::ConvertVPosPixel(long nVal) const
{
    return pEditWin->LogicToPixel(Size(0, nVal)).Height();
}

long SvxRuler::ConvertHSizePixel(long nVal) const
{
    return pEditWin->LogicToPixel(Size(nVal, 0)).Width();
}

long SvxRuler::ConvertVSizePixel(long nVal) const
{
    return pEditWin->LogicToPixel(Size(0, nVal)).Height();
}

long SvxRuler::ConvertPosPixel(long nVal) const
{
    return bHorz ? ConvertHPosPixel(nVal): ConvertVPosPixel(nVal);
}

long SvxRuler::ConvertSizePixel(long nVal) const
{
    return bHorz? ConvertHSizePixel(nVal): ConvertVSizePixel(nVal);
}


inline long SvxRuler::ConvertHPosLogic(long nVal) const
{
    return pEditWin->PixelToLogic(Size(nVal, 0)).Width();
}

inline long SvxRuler::ConvertVPosLogic(long nVal) const
{
    return pEditWin->PixelToLogic(Size(0, nVal)).Height();
}

inline long SvxRuler::ConvertHSizeLogic(long nVal) const
{
    return pEditWin->PixelToLogic(Size(nVal, 0)).Width();
}

inline long SvxRuler::ConvertVSizeLogic(long nVal) const
{
    return pEditWin->PixelToLogic(Size(0, nVal)).Height();
}

inline long SvxRuler::ConvertPosLogic(long nVal) const
{
    return bHorz? ConvertHPosLogic(nVal): ConvertVPosLogic(nVal);
}

inline long SvxRuler::ConvertSizeLogic(long nVal) const
{
    return bHorz? ConvertHSizeLogic(nVal): ConvertVSizeLogic(nVal);
}

long SvxRuler::PixelHAdjust(long nVal, long nValOld) const
{
        if(ConvertHSizePixel(nVal)!=ConvertHSizePixel(nValOld))
                return  nVal;
        else
                return  nValOld;
}

long SvxRuler::PixelVAdjust(long nVal, long nValOld) const
{
        if(ConvertVSizePixel(nVal)!=ConvertVSizePixel(nValOld))
                return  nVal;
        else
                return  nValOld;
}

long SvxRuler::PixelAdjust(long nVal, long nValOld) const
{
        if(ConvertSizePixel(nVal)!=ConvertSizePixel(nValOld))
                return  nVal;
        else
                return  nValOld;
}


inline USHORT SvxRuler::GetObjectBordersOff(USHORT nIdx) const
{
    return bHorz? nIdx: nIdx + 2;
}



void SvxRuler::UpdateFrame()

/*
   [Beschreibung]

   Linken, oberen Rand aktualisieren
   Items werden in die Darstellung des Lineals uebersetzt.

*/

{
    const USHORT nMarginStyle =
        ( pRuler_Imp->aProtectItem.IsSizeProtected() ||
          pRuler_Imp->aProtectItem.IsPosProtected() ) ?
        0 : RULER_MARGIN_SIZEABLE;

    if(pLRSpaceItem && pPagePosItem)
    {
        // wenn keine Initialisierung durch App Defaultverhalten
        const long nOld = lLogicNullOffset;
        lLogicNullOffset = pColumnItem?
            pColumnItem->GetLeft(): pLRSpaceItem->GetLeft();
        if(bAppSetNullOffset)
            lAppNullOffset += lLogicNullOffset - nOld;
        if(!bAppSetNullOffset || lAppNullOffset == LONG_MAX)
        {
            Ruler::SetNullOffset(ConvertHPosPixel(lLogicNullOffset));
            SetMargin1( 0, nMarginStyle );
            lAppNullOffset = 0;
        }
        else
            SetMargin1( ConvertHPosPixel( lAppNullOffset ), nMarginStyle );
        long lRight = 0;
            // bei Tabelle rechten Rand der Tabelle auswerten
        if(pColumnItem && pColumnItem->IsTable())
            lRight = pColumnItem->GetRight();
        else
            lRight = pLRSpaceItem->GetRight();

        ULONG aWidth=
            ConvertHPosPixel(pPagePosItem->GetWidth() - lRight -
                                    lLogicNullOffset + lAppNullOffset);
        SetMargin2( aWidth, nMarginStyle );
    }
    else
        if(pULSpaceItem && pPagePosItem)
        {
            // Nullpunkt aus oberem Rand des umgebenden Rahmens
            const long nOld = lLogicNullOffset;
            lLogicNullOffset = pColumnItem?
                pColumnItem->GetLeft(): pULSpaceItem->GetUpper();
            if(bAppSetNullOffset)
                lAppNullOffset += lLogicNullOffset - nOld;
            if(!bAppSetNullOffset || lAppNullOffset == LONG_MAX) {
                Ruler::SetNullOffset(ConvertVPosPixel(lLogicNullOffset));
                lAppNullOffset = 0;
                SetMargin1( 0, nMarginStyle );
            }
            else
                SetMargin1( ConvertVPosPixel( lAppNullOffset ),nMarginStyle );

            long lLower = pColumnItem ?
                pColumnItem->GetRight() : pULSpaceItem->GetLower();

            SetMargin2(ConvertVPosPixel(pPagePosItem->GetHeight() - lLower -
                                        lLogicNullOffset + lAppNullOffset),
                                        nMarginStyle );
        }
    else
    {
        // schaltet die Anzeige aus
        SetMargin1();
        SetMargin2();
    }
    if(pColumnItem)
    {
        pRuler_Imp->nColLeftPix = (USHORT) ConvertSizePixel(pColumnItem->GetLeft());
        pRuler_Imp->nColRightPix = (USHORT) ConvertSizePixel(pColumnItem->GetRight());
    }

}

void SvxRuler::MouseMove( const MouseEvent& rMEvt )
{
    if( bActive )
    {
        pBindings->Update( SID_RULER_LR_MIN_MAX );
        pBindings->Update( SID_ATTR_LONG_ULSPACE );
        pBindings->Update( SID_ATTR_LONG_LRSPACE );
        pBindings->Update( SID_RULER_PAGE_POS );
        pBindings->Update( SID_ATTR_TABSTOP );
        pBindings->Update( SID_ATTR_PARA_LRSPACE );
        pBindings->Update( SID_RULER_BORDERS );
        pBindings->Update( SID_RULER_OBJECT );
        pBindings->Update( SID_RULER_PROTECT );
    }
    Ruler::MouseMove( rMEvt );
}


void SvxRuler::UpdateFrame
(
 const SvxLongLRSpaceItem *pItem    // neuer Wert LRSpace
)

/*
   [Beschreibung]

   Neuen Wert fuer LRSpace merken; alten gfs. loeschen

*/

{
  if(bActive && bHorz)
  {
    delete pLRSpaceItem; pLRSpaceItem = 0;
    if(pItem)
        pLRSpaceItem = new SvxLongLRSpaceItem(*pItem);
    if(!bListening)
    {
        bValid = FALSE;
        StartListening(*pBindings);
        bListening = TRUE;
    }
  }
}


void SvxRuler::UpdateFrameMinMax
(
 const SfxRectangleItem *pItem  // Werte fuer MinMax
)

/*
   [Beschreibung]

   Neuen Wert fuer MinMax setzen; alten gfs. loeschen

*/

{
    if(bActive)
    {
        delete pMinMaxItem; pMinMaxItem = 0;
        if(pItem)
            pMinMaxItem = new SfxRectangleItem(*pItem);
    }
}


void SvxRuler::UpdateFrame
(
 const SvxLongULSpaceItem *pItem    // neuer Wert
)

/*
   [Beschreibung]

   Rechten / unteren Rand aktualisieren

*/


{
  if(bActive && !bHorz)
  {
    delete pULSpaceItem; pULSpaceItem = 0;
    if(pItem)
        pULSpaceItem = new SvxLongULSpaceItem(*pItem);
    if(!bListening)
    {
        bValid = FALSE;
        StartListening(*pBindings);
        bListening = TRUE;
    }
  }
}

void SvxRuler::Update( const SvxProtectItem* pItem )
{
    if( pItem ) pRuler_Imp->aProtectItem = *pItem;
}

void SvxRuler::Update
(
 const SvxColumnItem *pItem             // neuer Wert
)

/*
   [Beschreibung]

   Neuen Wert fuer Spaltendarstellung setzen

*/

{
    if(bActive)
    {
        delete pColumnItem; pColumnItem = 0;
        if(pItem)
            pColumnItem = new SvxColumnItem(*pItem);
        if(!bListening)
        {
            bValid = FALSE;
            StartListening(*pBindings);
            bListening = TRUE;
        }
    }
}


void SvxRuler::UpdateColumns()
/*
   [Beschreibung]

   Anzeige der Spaltendarstellung aktualisieren

*/
{
    if(pColumnItem && pColumnItem->Count() > 1)
    {
        if( nBorderCount < pColumnItem->Count())
        {
            delete pBorders;
            nBorderCount = pColumnItem->Count();
            pBorders = new RulerBorder[nBorderCount];
            size_t nSize = sizeof( RulerBorder ) * nBorderCount;
        }
        USHORT nFlags = RULER_BORDER_VARIABLE;
        BOOL bProtectColumns =
            pRuler_Imp->aProtectItem.IsSizeProtected() ||
            pRuler_Imp->aProtectItem.IsPosProtected();
        if( !bProtectColumns )
            nFlags |= RULER_BORDER_MOVEABLE;
        if( pColumnItem->IsTable() )
            nFlags |= RULER_BORDER_TABLE;
        else
            if ( !bProtectColumns )
                nFlags |= RULER_BORDER_SIZEABLE;

        for(USHORT i = 0; i < pColumnItem->Count()-1;++i)
        {
            pBorders[i].nStyle = nFlags;
            if(!(*pColumnItem)[i].bVisible)
                pBorders[i].nStyle |= RULER_STYLE_INVISIBLE;
            pBorders[i].nPos =
                ConvertPosPixel((*pColumnItem)[i].nEnd + lAppNullOffset);
            pBorders[i].nWidth =
                ConvertSizePixel((*pColumnItem)[i+1].nStart -
                                 (*pColumnItem)[i].nEnd);
        }
        const size_t nSize = sizeof(RulerBorder) * (pColumnItem->Count()-1);
        SetBorders(pColumnItem->Count()-1, pBorders);
    }
    else
    {
        SetBorders();
    }
}


void SvxRuler::UpdateObject()

/*
   [Beschreibung]

   Anzeige der Objektdarstellung aktualisieren

*/

{
    if(pObjectItem)
    {
        DBG_ASSERT(pObjectBorders, "kein Buffer");
        // !! zum Seitenrand
        long nMargin = pLRSpaceItem? pLRSpaceItem->GetLeft(): 0;
        pObjectBorders[0].nPos =
            ConvertPosPixel(pObjectItem->GetStartX() -
                            nMargin + lAppNullOffset);
        pObjectBorders[1].nPos =
            ConvertPosPixel(pObjectItem->GetEndX() - nMargin + lAppNullOffset);
        nMargin = pULSpaceItem? pULSpaceItem->GetUpper(): 0;
        pObjectBorders[2].nPos =
            ConvertPosPixel(pObjectItem->GetStartY() -
                            nMargin + lAppNullOffset);
        pObjectBorders[3].nPos =
            ConvertPosPixel(pObjectItem->GetEndY() - nMargin + lAppNullOffset);

        const size_t nSize = sizeof(RulerBorder) * 2;
        const USHORT nOff = GetObjectBordersOff(0);
        SetBorders(2, pObjectBorders + nOff);
    }
    else
    {
        SetBorders();
    }
}


void SvxRuler::UpdatePara()

/*
   [Beschreibung]

   Anzeige der Absatzeinzuege aktualisieren:
   Linken Rand, Erstzeileneinzug, rechten Rand Absatz aktualisieren
   pIndents[0] = Buffer fuer alten Einzug
   pIndents[1] = Buffer fuer alten Einzug
   pIndents[2] = Erstzeileneinzug
   pIndents[3] = linker Rand
   pIndents[4] = rechter Rand

*/

{
    // Abhaengigkeit zu PagePosItem
    if(pParaItem && pPagePosItem && !pObjectItem)
    {
        // Erstzeileneinzug, ist negativ zum linken Absatzrand
        pIndents[INDENT_GAP].nPos =
            ConvertHPosPixel(
                GetLeftFrameMargin()+
                pParaItem->GetTxtLeft() +
                pParaItem->GetTxtFirstLineOfst() +
                lAppNullOffset);
        if( pParaItem->IsAutoFirst() )
            pIndents[INDENT_GAP].nStyle |= RULER_STYLE_INVISIBLE;
        else
            pIndents[INDENT_GAP].nStyle &= ~RULER_STYLE_INVISIBLE;

        // linker Rand
        pIndents[INDENT_GAP+1].nPos =
            ConvertHPosPixel(
                GetLeftFrameMargin() +
                pParaItem->GetTxtLeft() + lAppNullOffset);
        // rechter Rand, immer negativ zum rechten Rand des umgebenden Frames
        pIndents[INDENT_GAP+2].nPos =
            ConvertHPosPixel(
                GetRightFrameMargin() -
                pParaItem->GetRight() + lAppNullOffset);
        SetIndents(3, pIndents+INDENT_GAP);
    }
    else
    {
        if(pIndents)
        {
            pIndents[INDENT_GAP].nPos =
            pIndents[INDENT_GAP+1].nPos =
            pIndents[INDENT_GAP+2].nPos = 0;
        }
        SetIndents();        // ausschalten
    }
}


void SvxRuler::UpdatePara
(
 const SvxLRSpaceItem *pItem    // neuer Wert Absatzeinzuege
)

/*
   [Beschreibung]

   Neuen Wert Absatzeinzuege merken
*/

{
    if(bActive)
    {
        delete pParaItem; pParaItem = 0;
        if(pItem)
            pParaItem = new SvxLRSpaceItem(*pItem);
        if(!bListening)
        {
            bValid = FALSE;
            StartListening(*pBindings);
            bListening = TRUE;
        }
    }
}


void SvxRuler::UpdatePage()

/*
   [Beschreibung]

   Anzeige von Postion und Breite der Seite aktualisieren

*/

{
    if(pPagePosItem)
    {
        // alle Objekte werden automatisch angepasst
        if(bHorz)
            SetPagePos(
                pEditWin->LogicToPixel(pPagePosItem->GetPos()).X(),
                pEditWin->LogicToPixel(Size(pPagePosItem->GetWidth(),0)).
                Width());
        else
            SetPagePos(
                pEditWin->LogicToPixel(pPagePosItem->GetPos()).Y(),
                pEditWin->LogicToPixel(Size(0, pPagePosItem->GetHeight())).
                Height());
        if(bAppSetNullOffset)
            SetNullOffset(ConvertSizePixel(-lAppNullOffset + lLogicNullOffset));
    }
    else
        SetPagePos();

    Point aPos(pEditWin->GetPosPixel()-GetPosPixel());
    long lPos= bHorz ? aPos.X() : aPos.Y();

// Leider bekommen wir den Offset des Editfensters zum Lineal nie
// per Statusmeldung. Also setzen wir ihn selbst, wenn noetig.

    if(lPos!=pRuler_Imp->lOldWinPos)
    {
        pRuler_Imp->lOldWinPos=lPos;
        SetWinPos(lPos);
    }
}


void SvxRuler::Update
(
 const SvxPagePosSizeItem *pItem // neuer Wert Seitenattribute
)

/*
   [Beschreibung]

   Neuen Wert Seitenattribute merken

*/

{
    if(bActive)
    {
        delete pPagePosItem; pPagePosItem = 0;
        if(pItem)
            pPagePosItem = new SvxPagePosSizeItem(*pItem);
        if(!bListening)
        {
            bValid = FALSE;
            StartListening(*pBindings);
            bListening = TRUE;
        }
    }
}


//

void SvxRuler::SetDefTabDist
(
 long l                                                 // Neuer Abstand fuer DefaultTabs in App-Metrik
)

/*
   [Beschreibung]

   Neuer Abstand fuer DefaultTabs wird gesetzt

*/

{

    lDefTabDist = l;
    UpdateTabs();
}


long SvxRuler::GetDefTabDist() const

/*
   [Beschreibung]

   Wert fuer DefaultTabs erfragen (wird in App.-Methik geliefert)

*/

{
    return lDefTabDist;
}


USHORT ToSvTab_Impl(SvxTabAdjust eAdj)

/*
   [Beschreibung]

   Interne Konvertierungsroutinen zwischen SV-Tab.-Enum und Svx

*/

{
    switch(eAdj) {
    case SVX_TAB_ADJUST_LEFT:    return RULER_TAB_LEFT;
    case SVX_TAB_ADJUST_RIGHT:   return RULER_TAB_RIGHT;
    case SVX_TAB_ADJUST_DECIMAL: return RULER_TAB_DECIMAL;
    case SVX_TAB_ADJUST_CENTER:  return RULER_TAB_CENTER;
    case SVX_TAB_ADJUST_DEFAULT: return RULER_TAB_DEFAULT;
    }
    return 0;
}


SvxTabAdjust ToAttrTab_Impl(USHORT eAdj)
{
    switch(eAdj) {
    case RULER_TAB_LEFT:    return SVX_TAB_ADJUST_LEFT    ;
    case RULER_TAB_RIGHT:   return SVX_TAB_ADJUST_RIGHT   ;
    case RULER_TAB_DECIMAL: return SVX_TAB_ADJUST_DECIMAL ;
    case RULER_TAB_CENTER:  return SVX_TAB_ADJUST_CENTER  ;
    case RULER_TAB_DEFAULT: return SVX_TAB_ADJUST_DEFAULT ;
    }
    return SVX_TAB_ADJUST_LEFT;
}


void SvxRuler::UpdateTabs()

/*
   [Beschreibung]

   Anzeige der Tabulatoren

*/

{
    if(IsDrag())
        return;
    if(pPagePosItem && pParaItem && pTabStopItem && !pObjectItem)
    {
        // Puffer fuer DefaultTabStop
        // Abstand letzter Tab <-> Rechter Absatzrand / DefaultTabDist
        const long lParaIndent =
            GetLeftFrameMargin() + pParaItem->GetTxtLeft();
        const long lLastTab =
             pTabStopItem->Count()?
              ConvertHPosPixel((*pTabStopItem)[pTabStopItem->Count()-1].GetTabPos()): 0;
        const long lPosPixel =
            ConvertHPosPixel(lParaIndent + lLastTab);
        const long lRightIndent =
            ConvertHPosPixel(GetRightFrameMargin() - pParaItem->GetRight());
        const USHORT nDefTabBuf = lPosPixel > lRightIndent ||
            lLastTab > lRightIndent
                ? 0
                : (USHORT)( (lRightIndent - lPosPixel) / ConvertHPosPixel(lDefTabDist) );
        const long nOldTabCount = nTabCount;

        if(pTabStopItem->Count() + TAB_GAP + nDefTabBuf > nTabBufSize)
        {
            delete pTabs;
            // 10 (GAP) auf Vorrat
            nTabBufSize = pTabStopItem->Count() + TAB_GAP + nDefTabBuf + GAP;
            pTabs = new RulerTab[nTabBufSize];
        }

        nTabCount = 0;
        USHORT j;
        for(j = 0; j < pTabStopItem->Count(); ++j)
        {
            const SvxTabStop *pTab = &(*pTabStopItem)[j];
            pTabs[nTabCount+TAB_GAP].nPos =
                ConvertHPosPixel(
                    lParaIndent + pTab->GetTabPos() + lAppNullOffset);
            // nicht ueber den rechten Absatzrand
            if(pTabs[nTabCount+TAB_GAP].nPos > lRightIndent)
                break;
            pTabs[nTabCount+TAB_GAP].nStyle = ToSvTab_Impl(pTab->GetAdjustment());
            ++nTabCount;
        }
        const long lParaIndentPix = ConvertSizePixel(lParaIndent);
        if(!pTabStopItem->Count())
            pTabs[0].nPos = lParaIndentPix;

        // Rest mit Default-Tabs fuellen
        for(j = 0; j < nDefTabBuf; ++j)
        {
            pTabs[nTabCount + TAB_GAP].nPos =
                pTabs[nTabCount].nPos + ConvertHPosPixel(lDefTabDist);

            if(j == 0 )
                pTabs[nTabCount + TAB_GAP].nPos -=
                    ((pTabs[nTabCount + TAB_GAP].nPos - lParaIndentPix)
                     % ConvertHPosPixel(lDefTabDist));
            if(pTabs[nTabCount+TAB_GAP].nPos >= lRightIndent)
                break;
            pTabs[nTabCount + TAB_GAP].nStyle = RULER_TAB_DEFAULT;
            ++nTabCount;
        }
        const size_t nSize = sizeof(RulerTab) * nTabCount;
        SetTabs(nTabCount, pTabs+TAB_GAP);
        DBG_ASSERT(nTabCount + TAB_GAP <= nTabBufSize, "BufferSize zu klein");
    }
    else
    {
        SetTabs();
    }
}


void SvxRuler::Update
(
 const SvxTabStopItem *pItem    // Neuer Wert fuer Tabulatoren
)

/*
   [Beschreibung]

   Neuen Wert fuer Tabulatoren merken; alten gfs. loeschen

*/

{
    if(bActive)
    {
        delete pTabStopItem; pTabStopItem = 0;
        if(pItem)
            pTabStopItem = new SvxTabStopItem(*pItem);
        if(!bListening)
        {
            bValid = FALSE;
            StartListening(*pBindings);
            bListening = TRUE;
        }
    }
}


void SvxRuler::Update
(
 const SvxObjectItem *pItem             // Neuer Wert fuer Objekte
)

/*
   [Beschreibung]

   Neuen Wert fuer Objekte merken

*/

{
    if(bActive)
    {
        delete pObjectItem; pObjectItem = 0;
        if(pItem)
            pObjectItem = new SvxObjectItem(*pItem);
        if(!bListening)
        {
            bValid = FALSE;
            StartListening(*pBindings);
            bListening = TRUE;
        }
    }
}


void SvxRuler::SetNullOffsetLogic
(
 long lVal                                              // Setzen des logischen NullOffsets
)
{
    lAppNullOffset = lLogicNullOffset - lVal;
    bAppSetNullOffset = TRUE;
    Ruler::SetNullOffset(ConvertSizePixel(lVal));
    Update();
}


void SvxRuler::Update()

/*
   [Beschreibung]

   Aktualisierung der Anzeige anstossen

*/

{
    if(IsDrag())
        return;
    UpdatePage();
    UpdateFrame();
    if((nFlags & SVXRULER_SUPPORT_OBJECT) == SVXRULER_SUPPORT_OBJECT)
        UpdateObject();
    else
        UpdateColumns();
    if(bHorz) {
      UpdatePara();
      UpdateTabs();
    }
}


inline long SvxRuler::GetPageWidth() const
{
    return bHorz ? pPagePosItem->GetWidth() : pPagePosItem->GetHeight();

}


inline long SvxRuler::GetFrameLeft() const

/*
   [Beschreibung]

   Erfragen des linken Randes in Pixeln

*/


{
    return  bAppSetNullOffset?
            GetMargin1() + ConvertSizePixel(lLogicNullOffset):
            Ruler::GetNullOffset();
}

inline void SvxRuler::SetFrameLeft(long l)

/*
   [Beschreibung]

   Setzen des linken Randes in Pixeln

*/

{
    BOOL bProtectColumns =
        pRuler_Imp->aProtectItem.IsSizeProtected() ||
        pRuler_Imp->aProtectItem.IsPosProtected();
    if(bAppSetNullOffset)
        SetMargin1(l - ConvertSizePixel(lLogicNullOffset),
                   bProtectColumns ? 0 : RULER_MARGIN_SIZEABLE);
    else
        Ruler::SetNullOffset(l);
}


long SvxRuler::GetFirstLineIndent() const

/*
   [Beschreibung]

   Erstzeileneinzug in Pixels erfragen
*/

{
    return pParaItem? pIndents[INDENT_GAP].nPos: GetMargin1();
}


long SvxRuler::GetLeftIndent() const

/*
   [Beschreibung]

   Linken Absatzrand in Pixels erfragen
*/

{
    return pParaItem? pIndents[1+INDENT_GAP].nPos: GetMargin1();
}



long SvxRuler::GetRightIndent() const

/*
   [Beschreibung]

   Rechten Absatzrand in Pixels erfragen
*/

{
    return pParaItem? pIndents[2+INDENT_GAP].nPos: GetMargin2();
}


long SvxRuler::GetLogicRightIndent() const

/*
   [Beschreibung]

   Rechten Absatzrand in Logic erfragen
*/

{
    return pParaItem ? GetRightFrameMargin()-pParaItem->GetRight() : GetRightFrameMargin();
}

// linker Rand in App-Werten; ist entweder der Seitenrand (=0)
// oder der linke Rand der Spalte, die im Spaltenattribut als
// altuelle Spalte eingestellt ist.

inline long SvxRuler::GetLeftFrameMargin() const
{
    return pColumnItem && pColumnItem->Count()?
        (*pColumnItem)[pColumnItem->GetActColumn()].nStart: 0;
}

inline long SvxRuler::GetLeftMin() const
{
    DBG_ASSERT(pMinMaxItem, "kein MinMax-Wert gesetzt");
    return pMinMaxItem?
        bHorz?  pMinMaxItem->GetValue().Left(): pMinMaxItem->GetValue().Top()
                : 0;
}

inline long SvxRuler::GetRightMax() const
{
    DBG_ASSERT(pMinMaxItem, "kein MinMax-Wert gesetzt");
    return pMinMaxItem?
        bHorz? pMinMaxItem->GetValue().Right(): pMinMaxItem->GetValue().Bottom()
            : 0;
}


long SvxRuler::GetRightFrameMargin() const

/*
   [Beschreibung]

   Rechten umgebenden Rand erfragen (in logischen Einheiten)

*/

{
    if(pColumnItem)
    {
        if(!IsActLastColumn( TRUE ))
            return (*pColumnItem)[GetActRightColumn( TRUE )].nEnd;
    }

    long l = lLogicNullOffset;

    // gfs. rechten Tabelleneinzug abziehen
    if(pColumnItem && pColumnItem->IsTable())
        l += pColumnItem->GetRight();
    else if(pLRSpaceItem)
        l += pLRSpaceItem->GetRight();

    return pPagePosItem->GetWidth() - l;
}

#define NEG_FLAG ( (nFlags & SVXRULER_SUPPORT_NEGATIVE_MARGINS) == \
                   SVXRULER_SUPPORT_NEGATIVE_MARGINS )
#define TAB_FLAG ( pColumnItem && pColumnItem->IsTable() )

long SvxRuler::GetCorrectedDragPos()

/*
   [Beschreibung]

   Korrigiert die Position innerhalb der errechneten Grenzwerte.
   Die Grenzwerte sind in Pixel relativ zum Seitenrand.

*/

{
    const long lNullPix = Ruler::GetNullOffset();
    long lDragPos = GetDragPos() + lNullPix;
     if(lDragPos < nMaxLeft)
        lDragPos = nMaxLeft;
    else if(lDragPos > nMaxRight)
        lDragPos = nMaxRight;
    return lDragPos - lNullPix;
}



void ModifyTabs_Impl
(
 USHORT nCount,                                 // Anzahl Tabs
 RulerTab *pTabs,                               // Tab-Puffer
 long lDiff                                     // zu addierende Differenz
 )

/*
   [Beschreibung]

   Hilfsroutine; alle Tabs um einen festen Wert verschieben

*/
{
    if( pTabs )
        for(USHORT i = 0; i < nCount; ++i)  pTabs[i].nPos += lDiff;
}



void SvxRuler::DragMargin1()

/*
   [Beschreibung]

   Draggen des linken Frame-Randes

*/
{
    const long lDragPos = ( TAB_FLAG && NEG_FLAG ) ?
            GetDragPos() : GetCorrectedDragPos();
    DrawLine_Impl(lTabPos, ( TAB_FLAG && NEG_FLAG ) ? 1 : 3, bHorz);
    const long nOld = bAppSetNullOffset? GetMargin1(): GetNullOffset();
    if(pColumnItem&&
       (//nDragType & DRAG_OBJECT_SIZE_LINEAR ||
        nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL))
        DragBorders();

    BOOL bProtectColumns =
        pRuler_Imp->aProtectItem.IsSizeProtected() ||
        pRuler_Imp->aProtectItem.IsPosProtected();

    const USHORT nMarginStyle =
        bProtectColumns ? 0 : RULER_MARGIN_SIZEABLE;

    if(!bAppSetNullOffset)
    {
        long lDiff = lDragPos;
        SetNullOffset(nOld + lDiff);
        if(!pColumnItem||!(nDragType & DRAG_OBJECT_SIZE_LINEAR))
        {
            SetMargin2( GetMargin2() - lDiff, nMarginStyle );

            if(!pColumnItem && !pObjectItem && pParaItem)
            {
                // Rechten Einzug an alter Position
                pIndents[2+INDENT_GAP].nPos -= lDiff;
                SetIndents(3, pIndents+INDENT_GAP);
            }
            if(pObjectItem)
            {
                pObjectBorders[GetObjectBordersOff(0)].nPos -= lDiff;
                pObjectBorders[GetObjectBordersOff(1)].nPos -= lDiff;
                SetBorders(2, pObjectBorders + GetObjectBordersOff(0));
            }
            if(pColumnItem)
            {
                for(USHORT i = 0; i < pColumnItem->Count()-1; ++i)
                    pBorders[i].nPos -= lDiff;
                SetBorders(pColumnItem->Count()-1, pBorders);
                if(pColumnItem->IsFirstAct())
                {
                    // Rechten Einzug an alter Position
                    if(pParaItem)
                    {
                        pIndents[2+INDENT_GAP].nPos -= lDiff;
                        SetIndents(3, pIndents+INDENT_GAP);
                    }
                }
                else
                {
                    if(pParaItem)
                    {
                        pIndents[0+INDENT_GAP].nPos -= lDiff;
                        pIndents[1+INDENT_GAP].nPos -= lDiff;
                        pIndents[2+INDENT_GAP].nPos -= lDiff;
                        SetIndents(3, pIndents+INDENT_GAP);
                    }
                }
                if(pTabStopItem&& (nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL)
                   &&!IsActFirstColumn())
                {
                    ModifyTabs_Impl(nTabCount+TAB_GAP, pTabs, -lDiff);
                    SetTabs(nTabCount, pTabs+TAB_GAP);
                }
            }
        }
    }
    else
    {
        long lDiff = lDragPos - nOld;
        SetMargin1(nOld + lDiff, nMarginStyle );

        if(!pColumnItem||!(nDragType & (DRAG_OBJECT_SIZE_LINEAR |
                                        DRAG_OBJECT_SIZE_PROPORTIONAL)))
        {
            if(!pColumnItem && !pObjectItem && pParaItem)
            {
                // Linke Einzuege an alter Position
                pIndents[0+INDENT_GAP].nPos += lDiff;
                pIndents[1+INDENT_GAP].nPos += lDiff;
                SetIndents(3, pIndents+INDENT_GAP);
            }

            if(pColumnItem)
            {
                for(USHORT i = 0; i < pColumnItem->Count()-1; ++i)
                    pBorders[i].nPos += lDiff;
                SetBorders(pColumnItem->Count()-1, pBorders);
                if(pColumnItem->IsFirstAct())
                {
                    // Linke Einzuege an alter Position
                    if(pParaItem)
                    {
                        pIndents[0+INDENT_GAP].nPos += lDiff;
                        pIndents[1+INDENT_GAP].nPos += lDiff;
                        SetIndents(3, pIndents+INDENT_GAP);
                    }
                }
                else
                {
                    if(pParaItem)
                    {
                        pIndents[0+INDENT_GAP].nPos += lDiff;
                        pIndents[1+INDENT_GAP].nPos += lDiff;
                        pIndents[2+INDENT_GAP].nPos += lDiff;
                        SetIndents(3, pIndents+INDENT_GAP);
                    }
                }
            }
            if(pTabStopItem)
            {
                ModifyTabs_Impl(nTabCount+TAB_GAP, pTabs, lDiff);
                SetTabs(nTabCount, pTabs+TAB_GAP);
            }
        }
    }
}


void SvxRuler::DragMargin2()
/*
   [Beschreibung]

   Draggen des rechten Frame-Randes

*/
{
    const long lDragPos = ( TAB_FLAG && NEG_FLAG ) ?
            GetDragPos() : GetCorrectedDragPos();
    DrawLine_Impl(lTabPos, ( TAB_FLAG && NEG_FLAG ) ? 1 : 3, bHorz);
    long lDiff = lDragPos - GetMargin2();

    BOOL bProtectColumns =
        pRuler_Imp->aProtectItem.IsSizeProtected() ||
        pRuler_Imp->aProtectItem.IsPosProtected();
    const USHORT nMarginStyle =
        bProtectColumns ? 0 : RULER_MARGIN_SIZEABLE;
    SetMargin2( lDragPos, nMarginStyle );

    // Rechten Einzug an alter Position
    if((!pColumnItem || IsActLastColumn()) && pParaItem)
    {
        pIndents[2+INDENT_GAP].nPos += lDiff;
        SetIndents(3, pIndents+INDENT_GAP);
    }
}


void SvxRuler::DragIndents()
/*
   [Beschreibung]

   Draggen der Absatzeinzuege

*/
{
    const long lDragPos = NEG_FLAG ? GetDragPos() : GetCorrectedDragPos();
    const USHORT nIdx = GetDragAryPos()+INDENT_GAP;
    const long lDiff = pIndents[nIdx].nPos - lDragPos;

    if(nIdx < 4 &&
        (nDragType & DRAG_OBJECT_LEFT_INDENT_ONLY) !=
        DRAG_OBJECT_LEFT_INDENT_ONLY)
        pIndents[INDENT_GAP].nPos -= lDiff;

    pIndents[nIdx].nPos = lDragPos;

    // Erstzeileneinzug mit verschieben
    if(1 == GetDragAryPos() &&
       (nDragType & DRAG_OBJECT_LEFT_INDENT_ONLY) !=
       DRAG_OBJECT_LEFT_INDENT_ONLY)
        SetIndents(5, pIndents);
    else
        SetIndents(4, pIndents+1);
    DrawLine_Impl(lTabPos, 1, bHorz);
}


void SvxRuler::DrawLine_Impl(long &lTabPos, int nNew, BOOL Hori)
/*
   [Beschreibung]

   Ausgaberoutine fuer Hilfslinie beim Vereschieben von Tabs, Tabellen-
   und anderen Spalten

*/
{
    if(Hori)
    {
        const long nHeight = pEditWin->GetOutputSize().Height();
        Point aZero=pEditWin->GetMapMode().GetOrigin();
        if(lTabPos!=-1)
            pEditWin->InvertTracking(
                Rectangle( Point(lTabPos, -aZero.Y()),
                           Point(lTabPos, -aZero.Y()+nHeight)),
                SHOWTRACK_SPLIT | SHOWTRACK_CLIP );
        if( nNew & 1 )
        {

            lTabPos = ConvertHSizeLogic( ( (nNew & 2) ? GetCorrectedDragPos() :
                GetDragPos() ) + GetNullOffset());
            if(pPagePosItem)
                lTabPos += pPagePosItem->GetPos().X();
            pEditWin->InvertTracking(
                Rectangle(Point(lTabPos, -aZero.Y()),
                          Point(lTabPos, -aZero.Y()+nHeight)),
                SHOWTRACK_CLIP | SHOWTRACK_SPLIT );
        }
    }
    else
    {
        const long nWidth = pEditWin->GetOutputSize().Width();
        Point aZero=pEditWin->GetMapMode().GetOrigin();
        if(lTabPos != -1)
        {
            pEditWin->InvertTracking(
                Rectangle( Point(-aZero.X(), lTabPos),
                           Point(-aZero.X()+nWidth, lTabPos)),
                SHOWTRACK_SPLIT | SHOWTRACK_CLIP );
        }

        if(nNew & 1)
        {
            lTabPos = ConvertVSizeLogic(GetCorrectedDragPos()+GetNullOffset());
            if(pPagePosItem)
                lTabPos += pPagePosItem->GetPos().Y();
            pEditWin->InvertTracking(
                Rectangle( Point(-aZero.X(), lTabPos),
                           Point(-aZero.X()+nWidth, lTabPos)),
                SHOWTRACK_CLIP | SHOWTRACK_SPLIT );
        }
    }
}




void SvxRuler::DragTabs()

/*
   [Beschreibung]

   Draggen von Tabs

*/
{

    const long lDragPos = GetCorrectedDragPos();




    USHORT nIdx = GetDragAryPos()+TAB_GAP;
    DrawLine_Impl(lTabPos, 3, bHorz);

    long nDiff = lDragPos - pTabs[nIdx].nPos;

    if(nDragType & DRAG_OBJECT_SIZE_LINEAR)
    {

        for(USHORT i = nIdx; i < nTabCount; ++i)
        {
            pTabs[i].nPos += nDiff;
            // auf Maximum begrenzen
            if(pTabs[i].nPos > GetMargin2())
                pTabs[nIdx].nStyle |= RULER_STYLE_INVISIBLE;
            else
                pTabs[nIdx].nStyle &= ~RULER_STYLE_INVISIBLE;
        }
    }
    else if(nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL)
    {
        pRuler_Imp->nTotalDist -= nDiff;
        pTabs[nIdx].nPos = lDragPos;
        for(USHORT i = nIdx+1; i < nTabCount; ++i)
        {
            if(pTabs[i].nStyle & RULER_TAB_DEFAULT)
                // bei den DefaultTabs kann abgebrochen werden
                break;
            long nDelta = pRuler_Imp->nTotalDist * pRuler_Imp->pPercBuf[i];
            nDelta /= 1000;
            pTabs[i].nPos = pTabs[nIdx].nPos + nDelta;
            if(pTabs[i].nPos+GetNullOffset() > nMaxRight)
                pTabs[i].nStyle |= RULER_STYLE_INVISIBLE;
            else
                pTabs[i].nStyle &= ~RULER_STYLE_INVISIBLE;
        }
    }
    else
        pTabs[nIdx].nPos = lDragPos;

    if(IsDragDelete())
        pTabs[nIdx].nStyle |= RULER_STYLE_INVISIBLE;
    else
        pTabs[nIdx].nStyle &= ~RULER_STYLE_INVISIBLE;
    SetTabs(nTabCount, pTabs+TAB_GAP);
}



void SvxRuler::SetActive(BOOL bOn)
{
    if(bOn)
    {
        Activate();
/*      pBindings->Invalidate( SID_RULER_LR_MIN_MAX, TRUE, TRUE );
        pBindings->Update( SID_RULER_LR_MIN_MAX );
        pBindings->Invalidate( SID_ATTR_LONG_ULSPACE, TRUE, TRUE );
        pBindings->Update( SID_ATTR_LONG_ULSPACE );
        pBindings->Invalidate( SID_ATTR_LONG_LRSPACE, TRUE, TRUE );
        pBindings->Update( SID_ATTR_LONG_LRSPACE );
        pBindings->Invalidate( SID_RULER_PAGE_POS, TRUE, TRUE );
        pBindings->Update( SID_RULER_PAGE_POS );
        pBindings->Invalidate( SID_ATTR_TABSTOP, TRUE, TRUE );
        pBindings->Update( SID_ATTR_TABSTOP );
        pBindings->Invalidate( SID_ATTR_PARA_LRSPACE, TRUE, TRUE );
        pBindings->Update( SID_ATTR_PARA_LRSPACE );
        pBindings->Invalidate( SID_RULER_BORDERS, TRUE, TRUE );
        pBindings->Update( SID_RULER_BORDERS );
        pBindings->Invalidate( SID_RULER_OBJECT, TRUE, TRUE );
        pBindings->Update( SID_RULER_OBJECT );
        pBindings->Invalidate( SID_RULER_PROTECT, TRUE, TRUE );
        pBindings->Update( SID_RULER_PROTECT );*/
    }
    else
        Deactivate();
    if(bActive!=bOn)
    {
        pBindings->EnterRegistrations();
        if(bOn)
            for(USHORT i=0;i<pRuler_Imp->nControlerItems;i++)
                pCtrlItem[i]->ReBind();
        else
            for(USHORT j=0;j<pRuler_Imp->nControlerItems;j++)
                pCtrlItem[j]->UnBind();
        pBindings->LeaveRegistrations();
    }
    bActive = bOn;
}




void SvxRuler::UpdateParaContents_Impl
(
 long l,                                                // Differenz
 UpdateType eType                               // Art (alle, links oder rechts)
)

/*
   [Beschreibung]

   Hilfsroutine; Mitfuehren von Tabulatoren und Absatzraendern

*/
{
    switch(eType) {
    case MOVE_RIGHT:
        pIndents[2+INDENT_GAP].nPos += l;
        break;
    case MOVE_ALL:
        pIndents[2+INDENT_GAP].nPos += l;
        // no break
    case MOVE_LEFT:
        {
            pIndents[INDENT_GAP].nPos += l;
            pIndents[1+INDENT_GAP].nPos += l;
            if ( pTabs )
            {
                for(USHORT i = 0; i < nTabCount+TAB_GAP;++i)
                    pTabs[i].nPos += l;
                SetTabs(nTabCount, pTabs+TAB_GAP);
            }
            break;
        }
    }
    SetIndents(3, pIndents+INDENT_GAP);
}



void SvxRuler::DragBorders()

/*
   [Beschreibung]

   Draggen von Borders (Tabellen- und anderen Spalten)

*/
{
    BOOL bLeftIndentsCorrected = FALSE, bRightIndentsCorrected = FALSE;
    const long lPos = GetCorrectedDragPos();
    int nIdx;

    if(GetDragType()==RULER_TYPE_BORDER)
    {
        DrawLine_Impl(lTabPos, 3, bHorz);
        nIdx = GetDragAryPos();
    }
    else
        nIdx=0;

    USHORT nDragSize = GetDragSize();
    long lDiff = 0;

    USHORT nRightCol = GetActRightColumn( FALSE, nIdx );
    USHORT nLeftCol = GetActLeftColumn( FALSE, nIdx );

    switch(nDragSize)
    {
      case RULER_DRAGSIZE_MOVE:
        {
            lDiff=GetDragType()==RULER_TYPE_BORDER ?
                lPos-nDragOffset - pBorders[nIdx].nPos
                : lPos - pRuler_Imp->lLastLMargin;

//          pBorders[nIdx].nPos += lDiff;
//          lDiff = pBorders[nIdx].nPos - nOld;
            if(nDragType & DRAG_OBJECT_SIZE_LINEAR)
            {
                long nRight = GetMargin2()-lMinFrame; // rechter Begrenzer
                for(int i = nBorderCount-2; i >= nIdx; --i)
                {
                    long l = pBorders[i].nPos;
                    pBorders[i].nPos += lDiff;
                    pBorders[i].nPos = Min(pBorders[i].nPos, nRight - pBorders[i].nWidth);
                    nRight = pBorders[i].nPos - lMinFrame;
                    // RR der Spalte aktualisieren
                    if(i == GetActRightColumn())
                    {
                        UpdateParaContents_Impl(pBorders[i].nPos - l, MOVE_RIGHT);
                        bRightIndentsCorrected = TRUE;
                    }
                    // LAR, EZE der Spalte aktualisieren
                    else if(i == GetActLeftColumn())
                    {
                        UpdateParaContents_Impl(pBorders[i].nPos - l, MOVE_LEFT);
                        bLeftIndentsCorrected = TRUE;
                    }
                }
            }
            else if(nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL)
            {
                int nLimit;
                long lLeft;
                if(GetDragType()==RULER_TYPE_BORDER)
                {
                    nLimit=nIdx+1;
                    lLeft=(pBorders[nIdx].nPos+=lDiff);
                }
                else
                {
                    nLimit=0;
                    lLeft=pRuler_Imp->lLastLMargin+lDiff;
                }

                pRuler_Imp->nTotalDist-=lDiff;

                for(int i  = nBorderCount-2; i >= nLimit; --i)
                {

                    long l = pBorders[i].nPos;
                    pBorders[i].nPos=lLeft+
                        (pRuler_Imp->nTotalDist*pRuler_Imp->pPercBuf[i])/1000+
                            pRuler_Imp->pBlockBuf[i];

                    // RR der Spalte aktualisieren
                    if(i == GetActRightColumn())
                    {
                        UpdateParaContents_Impl(pBorders[i].nPos - l, MOVE_RIGHT);
                        bRightIndentsCorrected = TRUE;
                    }
                    // LAR, EZE der Spalte aktualisieren
                    else if(i == GetActLeftColumn())
                    {
                        UpdateParaContents_Impl(pBorders[i].nPos - l, MOVE_LEFT);
                        bLeftIndentsCorrected = TRUE;
                    }
                }
            }
            else
                pBorders[nIdx].nPos+=lDiff;
            break;
        }
      case RULER_DRAGSIZE_1:
        {
            lDiff = lPos - pBorders[nIdx].nPos;
            pBorders[nIdx].nWidth += pBorders[nIdx].nPos - lPos;
            pBorders[nIdx].nPos = lPos;
            break;
        }
      case RULER_DRAGSIZE_2:
        {
            const long nOld = pBorders[nIdx].nWidth;
            pBorders[nIdx].nWidth = lPos - pBorders[nIdx].nPos;
            lDiff = pBorders[nIdx].nWidth - nOld;
            break;
        }
    }
    if(!bRightIndentsCorrected &&
       GetActRightColumn() == nIdx &&
       nDragSize != RULER_DRAGSIZE_2 && pIndents)
    {
        UpdateParaContents_Impl(lDiff, MOVE_RIGHT);
    }
    else if(!bLeftIndentsCorrected &&
            GetActLeftColumn()==nIdx &&
            nDragSize != RULER_DRAGSIZE_1 && pIndents)
    {
        UpdateParaContents_Impl(lDiff, MOVE_LEFT);
    }
    SetBorders(pColumnItem->Count()-1, pBorders);
}


void SvxRuler::DragObjectBorder()

/*
   [Beschreibung]

   Draggen von Objektraendern

*/
{
    if(RULER_DRAGSIZE_MOVE == GetDragSize())
    {
        const long lPos = GetCorrectedDragPos();
        const USHORT nIdx = GetDragAryPos();
        pObjectBorders[GetObjectBordersOff(nIdx)].nPos = lPos;
        SetBorders(2, pObjectBorders + GetObjectBordersOff(0));
        DrawLine_Impl(lTabPos, 3, bHorz);

    }
}


void SvxRuler::ApplyMargins()
/*
   [Beschreibung]

   Anwenden von Randeinstellungen; durch Draggen veraendert.

*/
{
    const SfxPoolItem *pItem = 0;
    USHORT nId = SID_ATTR_LONG_LRSPACE;
    if(bHorz)
    {
        const long lOldNull = lLogicNullOffset;
        if(pRuler_Imp->lMaxLeftLogic!=-1&&nMaxLeft==GetMargin1()+Ruler::GetNullOffset())
            pLRSpaceItem->SetLeft(lLogicNullOffset=pRuler_Imp->lMaxLeftLogic);
        else
            pLRSpaceItem->SetLeft(PixelHAdjust(
                lLogicNullOffset =  ConvertHPosLogic(GetFrameLeft()) -
                lAppNullOffset, pLRSpaceItem->GetLeft()));

        if(bAppSetNullOffset)
            lAppNullOffset += lLogicNullOffset - lOldNull;

        if(pRuler_Imp->lMaxRightLogic!=-1
           &&nMaxRight==GetMargin2()+Ruler::GetNullOffset())
            pLRSpaceItem->SetRight(GetPageWidth()-pRuler_Imp->lMaxRightLogic);
        else
            pLRSpaceItem->SetRight(
                PixelHAdjust(
                    Max((long)0,pPagePosItem->GetWidth() -
                        pLRSpaceItem->GetLeft() -
                        (ConvertHPosLogic(GetMargin2()) -
                         lAppNullOffset)),pLRSpaceItem->GetRight()));
        pItem = pLRSpaceItem;
#ifdef DEBUGLIN
        Debug_Impl(pEditWin,*pLRSpaceItem);
#endif // DEBUGLIN
    }
    else {
        const long lOldNull = lLogicNullOffset;
        pULSpaceItem->SetUpper(
            PixelVAdjust(
                lLogicNullOffset =
                ConvertVPosLogic(GetFrameLeft()) -
                lAppNullOffset,pULSpaceItem->GetUpper()));
        if(bAppSetNullOffset)
            lAppNullOffset += lLogicNullOffset - lOldNull;
        pULSpaceItem->SetLower(
            PixelVAdjust(
                Max((long)0, pPagePosItem->GetHeight() -
                    pULSpaceItem->GetUpper() -
                    (ConvertVPosLogic(GetMargin2()) -
                     lAppNullOffset)),pULSpaceItem->GetLower()));
        pItem = pULSpaceItem;
        nId = SID_ATTR_LONG_ULSPACE;
#ifdef DEBUGLIN
        Debug_Impl(pEditWin,*pULSpaceItem);
#endif // DEBUGLIN
    }
    pBindings->GetDispatcher()->Execute( nId, SFX_CALLMODE_RECORD, pItem, 0L );
    if(pTabStopItem)
        UpdateTabs();
}


void SvxRuler::ApplyIndents()
/*
   [Beschreibung]

   Anwenden von Absatzeinstellungen; durch Draggen veraendert.

*/
{
    long nNewTxtLeft;
    if(pColumnItem&&!IsActFirstColumn( TRUE ))
    {
        long nLeftCol=GetActLeftColumn( TRUE );
        nNewTxtLeft =
            PixelHAdjust(
                ConvertHPosLogic(
                    pIndents[1+INDENT_GAP].nPos-
                    (pBorders[nLeftCol].nPos +
                     pBorders[nLeftCol].nWidth))-
                lAppNullOffset,pParaItem->GetTxtLeft());
    }
    else
        nNewTxtLeft =
            PixelHAdjust(
                ConvertHPosLogic(pIndents[1+INDENT_GAP].nPos),
                pParaItem->GetTxtLeft());

    long nNewFirstLineOffset =
        PixelHAdjust(
            ConvertHPosLogic(pIndents[INDENT_GAP].nPos -
                             pIndents[1+INDENT_GAP].nPos) -
            lAppNullOffset,
            pParaItem->GetTxtFirstLineOfst());

    // #62986# : Ist der neue TxtLeft kleiner als der alte FirstLineIndent,
    // dann geht die Differenz verloren und der Absatz wird insgesamt
    // zu weit eingerueckt, deswegen erst den FirstLineOffset setzen, dann den TxtLeft
    pParaItem->SetTxtFirstLineOfst(nNewFirstLineOffset);
    pParaItem->SetTxtLeft(nNewTxtLeft);

    if(pColumnItem && !IsActLastColumn( TRUE ))
        pParaItem->SetRight(
            PixelHAdjust(
                ConvertHPosLogic(
                    pBorders[GetActRightColumn( TRUE )].nPos -
                    pIndents[2+INDENT_GAP].nPos) -
                lAppNullOffset,
                pParaItem->GetRight()));

    else
        pParaItem->SetRight( PixelHAdjust(
            ConvertHPosLogic(GetMargin2() -
                             pIndents[2+INDENT_GAP].nPos) -
            lAppNullOffset, pParaItem->GetRight()));

    pBindings->GetDispatcher()->Execute( SID_ATTR_PARA_LRSPACE, SFX_CALLMODE_RECORD, pParaItem, 0L );
    UpdateTabs();
}


void SvxRuler::ApplyTabs()
/*
   [Beschreibung]

   Anwenden von Tabulatoreinstellungen; durch Draggen veraendert.

*/
{
    const USHORT nCoreIdx = GetDragAryPos();
    if(IsDragDelete())
    {
        pTabStopItem->Remove(nCoreIdx);
    }
    else if(DRAG_OBJECT_SIZE_LINEAR & nDragType ||
            DRAG_OBJECT_SIZE_PROPORTIONAL & nDragType)
    {
        SvxTabStopItem *pItem = new SvxTabStopItem(pTabStopItem->Which());
        USHORT i;
        for(i = 0; i < nCoreIdx; ++i)
        {
            pItem->Insert((*pTabStopItem)[i]);
        }
        for(; i < pTabStopItem->Count(); ++i)
        {
            SvxTabStop aTabStop = (*pTabStopItem)[i];
            aTabStop.GetTabPos() = PixelHAdjust(
                ConvertHPosLogic(pTabs[i+TAB_GAP].nPos -
                                 GetLeftIndent()) -
                lAppNullOffset,
                aTabStop.GetTabPos());
            pItem->Insert(aTabStop);
        }
        delete pTabStopItem;
        pTabStopItem = pItem;
    }
    else if( pTabStopItem->Count() == 0 )
        return;
    else
    {
        SvxTabStop aTabStop = (*pTabStopItem)[nCoreIdx];
        if(pRuler_Imp->lMaxRightLogic!=-1&&
           pTabs[nCoreIdx+TAB_GAP].nPos+Ruler::GetNullOffset()==nMaxRight)
            aTabStop.GetTabPos() = pRuler_Imp->lMaxRightLogic-lLogicNullOffset;
        else
            aTabStop.GetTabPos() = PixelHAdjust(ConvertHPosLogic(pTabs[nCoreIdx+TAB_GAP].nPos -
                                                                 GetLeftIndent()) -
                                                lAppNullOffset,
                                                aTabStop.GetTabPos());
        pTabStopItem->Remove(nCoreIdx);
        pTabStopItem->Insert(aTabStop);
    }
    pBindings->GetDispatcher()->Execute( SID_ATTR_TABSTOP, SFX_CALLMODE_RECORD, pTabStopItem, 0L );
    UpdateTabs();
}


void SvxRuler::ApplyBorders()
/*
   [Beschreibung]

   Anwenden von (Tabellen-)Spalteneinstellungen; durch Draggen veraendert.

*/
{
    if(pColumnItem->IsTable())
    {
        long l = GetFrameLeft();
        if(l != pRuler_Imp->nColLeftPix)
            pColumnItem->SetLeft( PixelHAdjust(
                ConvertHPosLogic(l) - lAppNullOffset, pColumnItem->GetLeft()));
        l = GetMargin2();
        if(l != pRuler_Imp->nColRightPix)
            pColumnItem->SetRight( PixelHAdjust( pPagePosItem->GetWidth() -
                    pColumnItem->GetLeft() - ConvertHPosLogic(l) -
                    lAppNullOffset, pColumnItem->GetRight() ) );
    }
    for(USHORT i = 0; i < pColumnItem->Count()-1; ++i)
    {
        USHORT& nEnd = (*pColumnItem)[i].nEnd;
        nEnd = PIXEL_H_ADJUST(
            ConvertPosLogic(pBorders[i].nPos),
            (*pColumnItem)[i].nEnd);
        USHORT& nStart = (*pColumnItem)[i+1].nStart;
        nStart = PIXEL_H_ADJUST(
            ConvertSizeLogic(pBorders[i].nPos +
                             pBorders[i].nWidth) -
            lAppNullOffset,
            (*pColumnItem)[i+1].nStart);
        // Es kann sein, dass aufgrund der PIXEL_H_ADJUST rejustierung auf
        // alte Werte die Breite < 0 wird. Das rerejustieren wir.
        if( nEnd > nStart ) nStart = nEnd;
    }
#ifdef DEBUGLIN
        Debug_Impl(pEditWin,*pColumnItem);
#endif // DEBUGLIN
    SfxBoolItem aFlag(SID_RULER_ACT_LINE_ONLY,
                      nDragType & DRAG_OBJECT_ACTLINE_ONLY? TRUE: FALSE);
    pBindings->GetDispatcher()->Execute( SID_RULER_BORDERS, SFX_CALLMODE_RECORD, pColumnItem, &aFlag, 0L );
}

void SvxRuler::ApplyObject()
/*
   [Beschreibung]

   Anwenden von Objekteinstellungen; durch Draggen veraendert.

*/
{
        // zum Seitenrand
    long nMargin = pLRSpaceItem? pLRSpaceItem->GetLeft(): 0;
    pObjectItem->SetStartX(
                           PixelAdjust(
                              ConvertPosLogic(pObjectBorders[0].nPos)
                              + nMargin - lAppNullOffset,pObjectItem->GetStartX()));
    pObjectItem->SetEndX(
                         PixelAdjust(
                             ConvertPosLogic(pObjectBorders[1].nPos)
                         + nMargin -  lAppNullOffset,pObjectItem->GetEndX()));
    nMargin = pULSpaceItem? pULSpaceItem->GetUpper(): 0;
    pObjectItem->SetStartY(
                         PixelAdjust(
                             ConvertPosLogic(pObjectBorders[2].nPos)
                           + nMargin - lAppNullOffset,pObjectItem->GetStartY()));
    pObjectItem->SetEndY(
                     PixelAdjust(
                         ConvertPosLogic(pObjectBorders[3].nPos)
                         + nMargin - lAppNullOffset,pObjectItem->GetEndY()));
    pBindings->GetDispatcher()->Execute( SID_RULER_OBJECT, SFX_CALLMODE_RECORD, pObjectItem, 0L );
}

void SvxRuler::PrepareProportional_Impl(RulerType eType)
/*
   [Beschreibung]

   Vorbereitung proportionales Draggen; es wird der proportionale
   Anteil bezogen auf die Gesamtbreite in Promille berechnet.

*/
{
    pRuler_Imp->nTotalDist = GetMargin2();
    switch((int)eType)
    {
      case RULER_TYPE_MARGIN1:
      case RULER_TYPE_BORDER:
        {
            DBG_ASSERT(pColumnItem, "kein ColumnItem");

            pRuler_Imp->SetPercSize(nBorderCount);

            long lPos;
            long lWidth=0;
            USHORT nStart;
            USHORT nIdx=GetDragAryPos();
            lWidth=0;
            long lActWidth=0;
            long lActBorderSum;
            long lOrigLPos;

            if(eType==RULER_TYPE_MARGIN1)
            {
                lOrigLPos=GetMargin1();
                nStart=0;
                lActBorderSum=0;
            }
            else
            {
                lOrigLPos=pBorders[nIdx].nPos+pBorders[nIdx].nWidth;
                nStart=1;
                lActBorderSum=pBorders[nIdx].nWidth;
            }

            lPos=lOrigLPos;
            for(USHORT ii = nStart; ii < nBorderCount-1; ++ii)
            {
                lWidth+=pBorders[ii].nPos-lPos;
                lPos=pBorders[ii].nPos+pBorders[ii].nWidth;
            }

            lWidth+=GetMargin2()-lPos;
            pRuler_Imp->nTotalDist=lWidth;
            lPos=lOrigLPos;

            for(USHORT i = nStart; i < nBorderCount-1; ++i)
            {
                lActWidth+=pBorders[i].nPos-lPos;
                lPos=pBorders[i].nPos+pBorders[i].nWidth;
                pRuler_Imp->pPercBuf[i] = (USHORT)((lActWidth * 1000)
                                                   / pRuler_Imp->nTotalDist);
                pRuler_Imp->pBlockBuf[i]=(USHORT)lActBorderSum;
                lActBorderSum+=pBorders[i].nWidth;
            }
            break;
        }
      case RULER_TYPE_TAB:
        {
            const USHORT nIdx = GetDragAryPos()+TAB_GAP;
            pRuler_Imp->nTotalDist -= pTabs[nIdx].nPos;
            pRuler_Imp->SetPercSize(nTabCount);
            for(USHORT n=0;n<=nIdx;pRuler_Imp->pPercBuf[n++]=0);
            for(USHORT i = nIdx+1; i < nTabCount; ++i)
            {
                const long nDelta = pTabs[i].nPos - pTabs[nIdx].nPos;
                pRuler_Imp->pPercBuf[i] = (USHORT)((nDelta * 1000) / pRuler_Imp->nTotalDist);
            }
            break;
        }
    }
}


void SvxRuler::EvalModifier()

/*
   [Beschreibung]

   Modifier Draggen auswerten

   Shift: Linear verschieben
   Control: Proportional verschieben
   Shift+Control: Tabelle: nur aktuelle Zeile
   alt: Bemassungspfeile (n.i.) //!!

*/

{
    switch(GetDragModifier()) {
     case KEY_SHIFT:
        nDragType = DRAG_OBJECT_SIZE_LINEAR;
        break;
     case KEY_MOD1:  {
         const RulerType eType = GetDragType();
         nDragType = DRAG_OBJECT_SIZE_PROPORTIONAL;
         if(RULER_TYPE_BORDER == eType || RULER_TYPE_TAB == eType||
            RULER_TYPE_MARGIN1&&pColumnItem)
             PrepareProportional_Impl(eType);
         break;
     }
     case KEY_MOD1 | KEY_SHIFT:
        if(GetDragType()!=RULER_TYPE_MARGIN1&&
           GetDragType()!=RULER_TYPE_MARGIN2)
            nDragType = DRAG_OBJECT_ACTLINE_ONLY;
        break;
        // alt: Bemassungspfeile
    }
}


void __EXPORT SvxRuler::Click()

/*
   [Beschreibung]

   Ueberladener Handler SV; setzt Tab per Dispatcheraufruf

*/

{
    Ruler::Click();
    if( bActive )
    {
        pBindings->Update( SID_RULER_LR_MIN_MAX );
        pBindings->Update( SID_ATTR_LONG_ULSPACE );
        pBindings->Update( SID_ATTR_LONG_LRSPACE );
        pBindings->Update( SID_RULER_PAGE_POS );
        pBindings->Update( SID_ATTR_TABSTOP );
        pBindings->Update( SID_ATTR_PARA_LRSPACE );
        pBindings->Update( SID_RULER_BORDERS );
        pBindings->Update( SID_RULER_OBJECT );
        pBindings->Update( SID_RULER_PROTECT );
    }
    if(pTabStopItem &&
       (nFlags & SVXRULER_SUPPORT_TABS) == SVXRULER_SUPPORT_TABS) {
        BOOL bContentProtected = pRuler_Imp->aProtectItem.IsCntntProtected();
        if( bContentProtected ) return;
        const long lPos = GetClickPos();
        if(lPos > Min(GetFirstLineIndent(), GetLeftIndent()) &&
           lPos < GetRightIndent()) {
            SvxTabStop aTabStop(ConvertHPosLogic(lPos - GetLeftIndent()),
                                ToAttrTab_Impl(nDefTabType));
            pTabStopItem->Insert(aTabStop);
            UpdateTabs();
        }
    }
}


BOOL SvxRuler::CalcLimits
(
 long &nMax1,                                   // zu setzenden Minimalwert
 long &nMax2,                                   // zu setzenden Maximalwert
 BOOL
) const
/*
   [Beschreibung]

   Defaultimplementierung der virtuellen Funktion; kann die Applikation
   ueberladen, um eine eigene Grenzwertbehandlung zu implementieren.
   Die Werte sind auf die Seite bezogen.
*/
{
    nMax1 = LONG_MIN;
    nMax2 = LONG_MAX;
    return FALSE;
}


void SvxRuler::CalcMinMax()

/*
   [Beschreibung]

   Berechnet die Grenzwerte fuers Draggen; diese sind in Pixeln
   relativ zum Seitenrand

*/

{
    const long lNullPix = ConvertPosPixel(lLogicNullOffset);
    pRuler_Imp->lMaxLeftLogic=pRuler_Imp->lMaxRightLogic=-1;
    switch(GetDragType())
    {
      case RULER_TYPE_MARGIN1:
        {        // linker Rand umgebender Frame
            // DragPos - NOf zwischen links - rechts
            /*
              if(!pBorders)
              links = 0;
              rechts = NOf + RAR - Max(EZ,LAR)) -lMinFrame
              else
              links = 0;
              rechts = NOf - lMinFrame
              if(act)
              rechts += RAR - Max(EZ,LAR)
              else
              rechts += pBorders[0].nPos
              */
//      nMaxLeft = pMinMaxItem?
//          ConvertSizePixel(GetLeftMin()): 0; //Koordinate des linken Papierrands
            pRuler_Imp->lMaxLeftLogic = GetLeftMin();
            nMaxLeft=ConvertSizePixel(pRuler_Imp->lMaxLeftLogic);

            if(!pColumnItem || pColumnItem->Count() == 1 )
            {
                nMaxRight = lNullPix + GetRightIndent() -
                    Max(GetFirstLineIndent(), GetLeftIndent()) -
                    lMinFrame;
            }
            else
            {
                if(nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL)
                   //nDragType & DRAG_OBJECT_SIZE_LINEAR)
                {
                    nMaxRight=lNullPix+CalcPropMaxRight();
                }
                else if(nDragType & DRAG_OBJECT_SIZE_LINEAR)
                {
                    nMaxRight = ConvertPosPixel(
                        GetPageWidth() - (
                            (pColumnItem->IsTable() && pLRSpaceItem)
                            ? pLRSpaceItem->GetRight() : 0))
                            - GetMargin2() + GetMargin1();
                }
                else
                {
                    nMaxRight = lNullPix - lMinFrame;
                    if(pColumnItem->IsFirstAct())
                        nMaxRight += Min(
                            pBorders[0].nPos, GetRightIndent() -
                            Max(GetFirstLineIndent(), GetLeftIndent()));
                    else if( pColumnItem->Count() > 1 )
                        nMaxRight += pBorders[0].nPos;
                    else
                        nMaxRight +=GetRightIndent() -
                            Max(GetFirstLineIndent(), GetLeftIndent());
                    // den linken Tabellen-Rand nicht ueber den Seitenrand ziehen
                    if(pLRSpaceItem&&pColumnItem->IsTable())
                    {
                        long nTmp=ConvertSizePixel(pLRSpaceItem->GetLeft());
                        if(nTmp>nMaxLeft)
                            nMaxLeft=nTmp;
                    }
                //                      nMaxLeft = pLRSpaceItem?
                //                                              ConvertSizePixel(pLRSpaceItem->GetLeft()): 0;
                }
            }
            break;
        }
      case RULER_TYPE_MARGIN2:
                             {        // rechter Rand umgebender Frame
/*                rechts = PageWidth
        if(!pBorders)
            links = m2 - RAR + Max(EZ. LAR) + lMinF + nOF
            ??PageWidth - ( NOf + RAR - Max(EZ,LAR) - lMinFrame
        else
            links = NOf + pBorders[letzte].nPos +
                            pBorders[letzte].nWidth
            links += lMinFrame
            if(act)
                links += M2 - RAR + Max(EZ,LAR)
*/
//      nMaxRight = pMinMaxItem ?
//          ConvertSizePixel(
//              (bHorz ? pPagePosItem->GetWidth() : pPagePosItem->GetHeight())
//                           - GetRightMax()):  GetPageWidth();
        pRuler_Imp->lMaxRightLogic =
            pMinMaxItem ?
                GetPageWidth() - GetRightMax() : GetPageWidth();
        nMaxRight = ConvertSizePixel(pRuler_Imp->lMaxRightLogic);


        if(!pColumnItem)
        {
            nMaxLeft =  GetMargin2() - GetRightIndent() +
                Max(GetFirstLineIndent(),GetLeftIndent())  - GetMargin1()+
                    lMinFrame + lNullPix;
//          nMaxLeft = GetMargin1() + lNullPix + lMinFrame;
//          nMaxLeft =
//              GetMargin2() - /Max(GetFirstLineIndent(), GetLeftIndent()) +
//                  lMinFrame + lNullPix;
        }
        else
        {
            nMaxLeft = lMinFrame + lNullPix;
            if(IsActLastColumn() || pColumnItem->Count() < 2 ) //Falls letzte Spalte aktiv
            {
                nMaxLeft = lMinFrame + lNullPix + GetMargin2() -
                    GetRightIndent() + Max(GetFirstLineIndent(),
                                           GetLeftIndent());
            }
            if( pColumnItem->Count() >= 2 )
            {
                long nNewMaxLeft =
                    lMinFrame + lNullPix +
                    pBorders[pColumnItem->Count()-2].nPos +
                    pBorders[pColumnItem->Count()-2].nWidth;
                nMaxLeft=Max(nMaxLeft,nNewMaxLeft);
            }

            // den rechten Tabellen-Rand nicht ueber den Seitenrand ziehen
/*          if(pLRSpaceItem && pColumnItem->IsTable())
            {

                long nTmp=GetMargin2();/*ConvertSizePixel(pPagePosItem->GetWidth() -
                                           pLRSpaceItem->GetRight() -
                                           lLogicNullOffset + lAppNullOffset);

//              long nTmp=ConvertSizePixel(pLRSpaceItem->GetRight());
                if(nTmp<nMaxRight)
                    nMaxRight=nTmp;
            } TLX: Erstmal weggelassen, da zu undurchschaubar*/
//                      nMaxRight -= pLRSpaceItem && pColumnItem->IsTable()?
//                                                ConvertSizePixel(pLRSpaceItem->GetRight()): 0;
        }
        break;
    }
    case RULER_TYPE_BORDER:
    {                // Tabelle, Spalten (Modifier)
        const USHORT nIdx = GetDragAryPos();
/*
        // linke Seite sizen
        rechts = pBorders[nAct].nPos + pBorders[Act].nWidth + NOf
        if(erste)
            links = NOf
        else
            links  = pBorders[nAct-1].nPos+ pBorders[Act-1].nWidth + NOf
        if(act)
            links += pBorders[nAct].nPos - RAR + Max(EZ,LAR)
        links += lMinFrame

        // Moven
        if(erste)
            links = NOf
        else
            links  = pBorders[nAct-1].nPos+ pBorders[Act-1].nWidth + NOf
        if(act)
            links += pBorders[nAct].nPos - RAR + Max(EZ,LAR)
        links += lMinFrame

        if(letzte)
            rechts = M2 + NOf
            if(act = BorderCount-1) // Spalte rechts vom diesem Trenner
                rechts -= M2 - RAR + Max(EZ,LAR)
        else
            rechts = NOf + pBorders[nAct+1].nPos
            if(act+1) // Spalte rechts vom diesem Trenner
                rechts -= pBorders[nAct].nPos - RAR + Max(EZ,LAR)
        rechts -= lMinFrame

        // rechte Seite sizen
        links = pBorders[nAct].nPos
        if(letzte)
            rechts = M2 + NOf
            if(act = BorderCount-1) // Spalte rechts vom diesem Trenner
                rechts -= M2 - RAR + Max(EZ,LAR)
        else
            rechts = NOf + pBorders[nAct+1].nPos
            if(act+1) // Spalte rechts vom diesem Trenner
                rechts -= pBorders[nAct].nPos - RAR + Max(EZ,LAR)
        rechts -= lMinFrame

*/
        switch(GetDragSize())
        {
          case RULER_DRAGSIZE_1 :
            {
                nMaxRight = pBorders[nIdx].nPos +
                    pBorders[nIdx].nWidth + lNullPix;

                if(0 == nIdx)
                    nMaxLeft = lNullPix;
                else
                    nMaxLeft = pBorders[nIdx-1].nPos +
                        pBorders[nIdx-1].nWidth + lNullPix;
                if(nIdx == pColumnItem->GetActColumn()) {
                    nMaxLeft += pBorders[nIdx].nPos -
                        GetRightIndent() + Max(GetFirstLineIndent(),
                                               GetLeftIndent());
                    if(0 != nIdx)
                        nMaxLeft -= pBorders[nIdx-1].nPos +
                            pBorders[nIdx-1].nWidth;
                }
                nMaxLeft += lMinFrame;
                nMaxLeft += nDragOffset;
                break;
            }
          case RULER_DRAGSIZE_MOVE:
            {
                if(pColumnItem)
                {
                    USHORT nLeftCol=GetActLeftColumn(FALSE, nIdx);
                    USHORT nRightCol=GetActRightColumn(FALSE, nIdx);
                    USHORT nActLeftCol=GetActLeftColumn();
                    USHORT nActRightCol=GetActRightColumn();
                    if(nLeftCol==USHRT_MAX)
                        nMaxLeft=lNullPix;
                    else
                        nMaxLeft = pBorders[nLeftCol].nPos +
                            pBorders[nLeftCol].nWidth + lNullPix;

                    if(nActRightCol == nIdx)
                    {
                        nMaxLeft += pBorders[nIdx].nPos -
                            GetRightIndent() + Max(GetFirstLineIndent(),
                                                   GetLeftIndent());
                        if(nActLeftCol!=USHRT_MAX)
                            nMaxLeft -= pBorders[nActLeftCol].nPos +
                                pBorders[nActLeftCol].nWidth;
                    }
                    nMaxLeft += lMinFrame;
                    nMaxLeft += nDragOffset;

                    // nMaxRight
                    // linear / proprotional verschieben
                    if(DRAG_OBJECT_SIZE_PROPORTIONAL & nDragType||
                       (DRAG_OBJECT_SIZE_LINEAR & nDragType) )
                    {
                        nMaxRight=lNullPix+CalcPropMaxRight(nIdx);
                    }
                    else if(DRAG_OBJECT_SIZE_LINEAR & nDragType)
                    {
                        nMaxRight=lNullPix+GetMargin2()-GetMargin1()+
                            (nBorderCount-nIdx-1)*lMinFrame;
                    }
                    else
                    {
                        if(nRightCol==USHRT_MAX)
                        { // letzte Spalte
                            nMaxRight = GetMargin2() + lNullPix;
                            if(IsActLastColumn())
                            {
                                nMaxRight -=
                                    GetMargin2() - GetRightIndent() +
                                        Max(GetFirstLineIndent(),
                                            GetLeftIndent());
                                nMaxRight += pBorders[nIdx].nPos +
                                    pBorders[nIdx].nWidth;
                            }
                        }
                        else
                        {
                            nMaxRight = lNullPix + pBorders[nRightCol].nPos;
                            USHORT nNotHiddenRightCol =
                                GetActRightColumn(TRUE, nIdx);

                            if( nActLeftCol == nIdx )
                            {
                                long nBorder = nNotHiddenRightCol ==
                                    USHRT_MAX ?
                                    GetMargin2() :
                                    pBorders[nNotHiddenRightCol].nPos;
                                nMaxRight -= nBorder - GetRightIndent() +
                                    Max(GetFirstLineIndent(),
                                        GetLeftIndent());
                                nMaxRight += pBorders[nIdx].nPos +
                                    pBorders[nIdx].nWidth;
                            }
                        }
                        nMaxRight -= lMinFrame;
                        nMaxRight -= pBorders[nIdx].nWidth;
                    }
                }
                // ObjectItem
                else
                {
                    if(pObjectItem->HasLimits())
                    {
                        if(CalcLimits(nMaxLeft, nMaxRight, nIdx & 1? FALSE: TRUE))
                        {
                            nMaxLeft = ConvertPosPixel(nMaxLeft);
                            nMaxRight = ConvertPosPixel(nMaxRight);
                        }
                    }
                    else
                    {
                        nMaxLeft = LONG_MIN;
                        nMaxRight = LONG_MAX;
                    }
                }
                break;
            }
          case RULER_DRAGSIZE_2:
            {
                nMaxLeft = lNullPix + pBorders[nIdx].nPos;
                if(nIdx == pColumnItem->Count()-2) { // letzte Spalte
                    nMaxRight = GetMargin2() + lNullPix;
                    if(pColumnItem->IsLastAct()) {
                        nMaxRight -=
                            GetMargin2() - GetRightIndent() +
                                Max(GetFirstLineIndent(),
                                    GetLeftIndent());
                        nMaxRight += pBorders[nIdx].nPos +
                            pBorders[nIdx].nWidth;
                    }
                }
                else {
                    nMaxRight = lNullPix + pBorders[nIdx+1].nPos;
                    if(pColumnItem->GetActColumn()-1 == nIdx) {
                        nMaxRight -= pBorders[nIdx+1].nPos  - GetRightIndent() +
                            Max(GetFirstLineIndent(),
                                GetLeftIndent());
                        nMaxRight += pBorders[nIdx].nPos +
                            pBorders[nIdx].nWidth;
                    }
            }
                nMaxRight -= lMinFrame;
                nMaxRight -= pBorders[nIdx].nWidth;
                break;
            }
        }
        nMaxRight += nDragOffset;
        break;
    }
      case RULER_TYPE_INDENT:
        {
        const USHORT nIdx = GetDragAryPos();
        switch(nIdx) {
            // EZ:
            /*
               links = NOf
               if(borders && act != erste)
               links += pBorders[-1].nPos + pBorders[-1].nStyle

               rechts = NOf
               rechts += RAR - lMinFrame

               LAR:
               links = NOf
               if(borders && act != erste)
               links += pBorders[-1].nPos + pBorders[-1].nStyle
               rechts = NOf
               rechts += RAR - lMinFrame

               if(zusammen)
               if(LAR > EZ)
               nLeft += LAR - EZ
               else
               rechts -= EZ - LAR
               */
        case 0:
        case 1:
            {
                nMaxLeft = lNullPix;

                if(pColumnItem && !pColumnItem->IsFirstAct())
                    nMaxLeft += pBorders[pColumnItem->GetActColumn()-1].nPos +
                        pBorders[pColumnItem->GetActColumn()-1].nWidth;
                nMaxRight = lNullPix + GetRightIndent() - lMinFrame;

                // zusammem draggen
                if(0 != nIdx &&
                   (nDragType & DRAG_OBJECT_LEFT_INDENT_ONLY) !=
                   DRAG_OBJECT_LEFT_INDENT_ONLY)
                {
                    if(GetLeftIndent() > GetFirstLineIndent())
                        nMaxLeft += GetLeftIndent() - GetFirstLineIndent();
                    else
                        nMaxRight -= GetFirstLineIndent() - GetLeftIndent();
                }
                break;
            }
                /*
                   RAR:
                   links = NOf + Max(EZ, LAR)
                   rechts = NOf
                   if(borders && !letzte)
                   rechts =  borders[+1].nPos
                   else
                   rechts += M2
                   */
          case 2:
            {
                nMaxLeft = lNullPix +
                    Max(GetFirstLineIndent(), GetLeftIndent());
                nMaxRight = lNullPix;
                if(pColumnItem)
                {
                    USHORT nRightCol=GetActRightColumn( TRUE );
                    if(!IsActLastColumn( TRUE ))
                        nMaxRight += pBorders[nRightCol].nPos;
                    else
                        nMaxRight += GetMargin2();
                }
                else
                    nMaxRight += GetMargin2();
                nMaxLeft += lMinFrame;

//                  if(pColumnItem && !pColumnItem->IsLastAct())
//                      nMaxRight += pBorders[pColumnItem->GetActColumn()].nPos;
//                  else
//                      nMaxRight += GetMargin2();
//                  nMaxLeft += lMinFrame;

            }
            break;
        }
        break;
    }
    case RULER_TYPE_TAB:                // Tabs (Modifier)
        /*
           links = NOf + Max(LAR, EZ)
           rechts = NOf + RAR
           */
        nMaxLeft = lNullPix + Min(GetFirstLineIndent(), GetLeftIndent());
        pRuler_Imp->lMaxRightLogic=GetLogicRightIndent()+lLogicNullOffset;
        nMaxRight = ConvertSizePixel(pRuler_Imp->lMaxRightLogic);
//      nMaxRight = lNullPix + GetRightIndent();
        break;
    }
#ifdef DEBUGLIN
    {
        String aStr("MinLeft: ");
        Size aSize(nMaxLeft + lNullPix, 0);
        Size aSize2(nMaxRight + lNullPix, 0);
        aSize = pEditWin->PixelToLogic(aSize, MapMode(MAP_MM));
        aSize2 = pEditWin->PixelToLogic(aSize2, MapMode(MAP_MM));
        aStr += String(aSize.Width());
        aStr += " MaxRight: ";
        aStr += String(aSize2.Width());
        InfoBox(0, aStr).Execute();
    }
#endif
}


long __EXPORT SvxRuler::StartDrag()

/*
   [Beschreibung]

   Beginn eines Drag-Vorgangs (SV-Handler); wertet Modifier aus
   und berechnet Grenzwerte

   [Querverweise]

   <SvxRuler::EvalModifier()>
   <SvxRuler::CalcMinMax()>
   <SvxRuler::EndDrag()>

*/

{
    BOOL bContentProtected = pRuler_Imp->aProtectItem.IsCntntProtected();
    if(!bValid)
        return FALSE;

    pRuler_Imp->lLastLMargin=GetMargin1();
    long bOk = 1;
    if(GetStartDragHdl().IsSet())
        bOk = Ruler::StartDrag();
    if(bOk) {
        lInitialDragPos = GetDragPos();
        switch(GetDragType()) {
        case RULER_TYPE_MARGIN1:        // linker Rand umgebender Frame
        case RULER_TYPE_MARGIN2:        // rechter Rand umgebender Frame
            if((bHorz && pLRSpaceItem) || (!bHorz && pULSpaceItem))
            {
                if(pColumnItem)
                    EvalModifier();
                else
                    nDragType = DRAG_OBJECT;
            }
            else
                bOk = FALSE;
            break;
        case RULER_TYPE_BORDER:                // Tabelle, Spalten (Modifier)
            if(pColumnItem)
            {
                nDragOffset = pColumnItem->IsTable()? 0 :
                GetDragPos() - pBorders[GetDragAryPos()].nPos;
                EvalModifier();

            }
            else
                nDragOffset = 0;
            break;
        case RULER_TYPE_INDENT: {                // Absatzeinzuege (Modifier)
            if( bContentProtected ) return FALSE;
            if(1 == GetDragAryPos()) {        // Linker Absatzeinzug
                pIndents[0] = pIndents[INDENT_GAP];
                pIndents[0].nStyle |= RULER_STYLE_DONTKNOW;
                EvalModifier();
            }
            else
                nDragType = DRAG_OBJECT;
            pIndents[1] = pIndents[GetDragAryPos()+INDENT_GAP];
            pIndents[1].nStyle |= RULER_STYLE_DONTKNOW;
            break;
        }
        case RULER_TYPE_TAB:                // Tabs (Modifier)
            if( bContentProtected ) return FALSE;
            EvalModifier();
            pTabs[0] = pTabs[GetDragAryPos()+1];
            pTabs[0].nStyle |= RULER_STYLE_DONTKNOW;
            break;
        default:
            nDragType = NONE;
        }
    }
    else
        nDragType = NONE;
    if(bOk)
        CalcMinMax();
    return bOk;
}


void  __EXPORT SvxRuler::Drag()
/*
   [Beschreibung]

   SV-Draghandler

*/
{
    if(IsDragCanceled())
    {
        Ruler::Drag();
        return;
    }
    switch(GetDragType()) {
    case RULER_TYPE_MARGIN1:        // linker Rand umgebender Frame
        DragMargin1();
        pRuler_Imp->lLastLMargin=GetMargin1();
        break;
    case RULER_TYPE_MARGIN2:        // rechter Rand umgebender Frame
        DragMargin2();
        break;
    case RULER_TYPE_INDENT:         // Absatzeinzuege
        DragIndents();
        break;
    case RULER_TYPE_BORDER:         // Tabelle, Spalten
        if(pColumnItem)
            DragBorders();
        else if(pObjectItem)
            DragObjectBorder();
        break;
    case RULER_TYPE_TAB:            // Tabs
        DragTabs();
        break;
    }
    Ruler::Drag();
}


void __EXPORT SvxRuler::EndDrag()
/*
   [Beschreibung]

   SV-Handler; wird beim Beenden des Draggens gerufen.
   Stoesst die Aktualisierung der Daten der Applikation an, indem
   durch Aufruf der jeweiligen Apply...()- Methoden die Daten an die
   Applikation geschickt werden.

*/
{
    const BOOL bUndo = IsDragCanceled();
    const long lPos = GetDragPos();
    DrawLine_Impl(lTabPos, 2, bHorz);
    lTabPos=-1;
    if(!bUndo)
        switch(GetDragType())
        {
          case RULER_TYPE_MARGIN1:   // linker, oberer Rand umgebender Frame
          case RULER_TYPE_MARGIN2:   // rechter, unterer Rand umgebender Frame
            {
                if(!pColumnItem || !pColumnItem->IsTable())
                    ApplyMargins();

                if(pColumnItem &&
                   (pColumnItem->IsTable() ||
                    (nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL)))
                    ApplyBorders();

            }
            break;
          case RULER_TYPE_BORDER:                // Tabelle, Spalten
            if(lInitialDragPos != lPos)
            {
                if(pColumnItem)
                {
                    ApplyBorders();
                    if(bHorz)
                        UpdateTabs();
                }
                else if(pObjectItem)
                    ApplyObject();
            }
            break;
          case RULER_TYPE_INDENT:                // Absatzeinzuege
            if(lInitialDragPos != lPos)
                ApplyIndents();
            SetIndents(3, pIndents+INDENT_GAP);
            break;
          case RULER_TYPE_TAB:                // Tabs
            {
                ApplyTabs();
                pTabs[GetDragAryPos()].nStyle &= ~RULER_STYLE_INVISIBLE;
                SetTabs(nTabCount, pTabs+TAB_GAP);
            }
            break;
        }
    nDragType = NONE;
    Ruler::EndDrag();
    if(bUndo)
        for(USHORT i=0;i<pRuler_Imp->nControlerItems;i++)
        {
            pCtrlItem[i]->ClearCache();
            pCtrlItem[i]->GetBindings().Invalidate(pCtrlItem[i]->GetId());
            //      pCtrlItem[i]->UnBind();
//          pCtrlItem[i]->ReBind();
        }
}


void __EXPORT SvxRuler::ExtraDown()

/*
   [Beschreibung]

   Ueberladene SV-Methode; setzt den neuen Typ fuer den Defaulttabulator.
*/

{
    // Tabulator Typ umschalten
    if(pTabStopItem &&
        (nFlags & SVXRULER_SUPPORT_TABS) ==        SVXRULER_SUPPORT_TABS) {
        ++nDefTabType;
        if(RULER_TAB_DEFAULT == nDefTabType)
            nDefTabType = RULER_TAB_LEFT;
        SetExtraType(RULER_EXTRA_TAB, nDefTabType);
    }
    Ruler::ExtraDown();
}


void __EXPORT SvxRuler::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
                               const SfxHint& rHint, const TypeId& rHintType)
/*

   [Beschreibung]

   Benachrichtigung durch die Bindings, dass die Statusaktualisierung
   beendet ist.
   Das Lineal aktualisiert seine Darstellung und meldet sich bei den
   Bindings wieder ab.

*/

{
    // Aktualisierung anstossen
    if(bActive &&
        rHint.Type() == TYPE(SfxSimpleHint) &&
     ((SfxSimpleHint&) rHint ).GetId() == SFX_HINT_UPDATEDONE ) {
        Update();
        EndListening(*pBindings);
        bValid = TRUE;
        bListening = FALSE;
    }
}


IMPL_LINK_INLINE_START( SvxRuler, MenuSelect, Menu *, pMenu )

/*
   [Beschreibung]

   Handler des Kontextmenues fuer das Umschalten der Masseinheit

*/

{
    SetUnit(FieldUnit(pMenu->GetCurItemId()));
    return 0;
}
IMPL_LINK_INLINE_END( SvxRuler, MenuSelect, Menu *, pMenu )


IMPL_LINK( SvxRuler, TabMenuSelect, Menu *, pMenu )

/*
   [Beschreibung]

   Handler des Tabulatormenues fuer das Setzen des Typs

*/

{
    SvxTabStop aTabStop = (*pTabStopItem)[pRuler_Imp->nIdx];
    aTabStop.GetAdjustment() = ToAttrTab_Impl(pMenu->GetCurItemId()-1);
    pTabStopItem->Remove(pRuler_Imp->nIdx);
    pTabStopItem->Insert(aTabStop);
    pBindings->GetDispatcher()->Execute( SID_ATTR_TABSTOP, SFX_CALLMODE_RECORD, pTabStopItem, 0L );
    UpdateTabs();
    pRuler_Imp->nIdx = 0;
    return 0;
}


void SvxRuler::Command( const CommandEvent& rCEvt )

/*
   [Beschreibung]

   Mauskontextmenue fuer das Umschalten der Masseinheit

*/

{
    if ( COMMAND_CONTEXTMENU == rCEvt.GetCommand() )
    {
        CancelDrag();
        if ( pTabs &&
             RULER_TYPE_TAB ==
             GetType( rCEvt.GetMousePosPixel(), &pRuler_Imp->nIdx ) &&
             pTabs[pRuler_Imp->nIdx+TAB_GAP].nStyle < RULER_TAB_DEFAULT )
        {
            PopupMenu aMenu;
            aMenu.SetSelectHdl(LINK(this, SvxRuler, TabMenuSelect));
            VirtualDevice aDev;
            const Size aSz(RULER_TAB_WIDTH+2, RULER_TAB_HEIGHT+2);
            aDev.SetOutputSize(aSz);
            aDev.SetBackground(Wallpaper(Color(COL_WHITE)));
            const Point aPt(aSz.Width() / 2, aSz.Height() / 2);

            for ( USHORT i = RULER_TAB_LEFT; i < RULER_TAB_DEFAULT; ++i )
            {
                DrawTab(&aDev, aPt, i);
                aMenu.InsertItem(i+1,
                                 String(ResId(RID_SVXSTR_RULER_START+i, DIALOG_MGR())),
                                 Image(aDev.GetBitmap(Point(), aSz), Color(COL_WHITE)));
                aMenu.CheckItem(i+1, i == pTabs[pRuler_Imp->nIdx+TAB_GAP].nStyle);
                aDev.SetOutputSize(aSz); // device loeschen
            }
            aMenu.Execute( this, OutputToScreenPixel(rCEvt.GetMousePosPixel()));
        }
        else
        {
            PopupMenu aMenu(ResId(RID_SVXMN_RULER, DIALOG_MGR()));
            aMenu.SetSelectHdl(LINK(this, SvxRuler, MenuSelect));
            FieldUnit eUnit = GetUnit();
            const USHORT nCount = aMenu.GetItemCount();

            for ( USHORT i = 0; i < nCount; ++i )
            {
                const USHORT nId = aMenu.GetItemId(i);
                aMenu.CheckItem(nId, nId == (USHORT)eUnit);
            }
            aMenu.Execute( this, OutputToScreenPixel(rCEvt.GetMousePosPixel()));
        }
    }
    else
        Ruler::Command( rCEvt );
}


USHORT SvxRuler::GetActRightColumn(
    BOOL bForceDontConsiderHidden, USHORT nAct ) const
{
    if( nAct == USHRT_MAX )
        nAct = pColumnItem->GetActColumn();
    else nAct++; //Damit man die ActDrag uebergeben kann

    BOOL bConsiderHidden = !bForceDontConsiderHidden &&
        !( nDragType & DRAG_OBJECT_ACTLINE_ONLY );

    while( nAct < pColumnItem->Count() - 1 )
    {
        if( (*pColumnItem)[nAct].bVisible || bConsiderHidden )
            return nAct;
        else
            nAct++;
    }
    return USHRT_MAX;
}



USHORT SvxRuler::GetActLeftColumn(
    BOOL bForceDontConsiderHidden, USHORT nAct ) const
{
    if(nAct==USHRT_MAX)
        nAct=pColumnItem->GetActColumn();

    USHORT nLOffs=1;

    BOOL bConsiderHidden = !bForceDontConsiderHidden &&
        !( nDragType & DRAG_OBJECT_ACTLINE_ONLY );

    while(nAct>=nLOffs)
    {
        if( (*pColumnItem)[ nAct - nLOffs ].bVisible || bConsiderHidden )
            return nAct-nLOffs;
        else
            nLOffs++;
    }
    return USHRT_MAX;
}


BOOL SvxRuler::IsActLastColumn(
    BOOL bForceDontConsiderHidden, USHORT nAct) const
{
    return GetActRightColumn(bForceDontConsiderHidden, nAct)==USHRT_MAX;
}

BOOL SvxRuler::IsActFirstColumn(
    BOOL bForceDontConsiderHidden, USHORT nAct) const
{
    return GetActLeftColumn(bForceDontConsiderHidden, nAct)==USHRT_MAX;
}

long SvxRuler::CalcPropMaxRight(USHORT nCol) const
{

    if(!(nDragType & DRAG_OBJECT_SIZE_LINEAR))
    {

        // ausgehend vom rechten Rand die Mindestbreiten
        // aller betroffenen Spalten abziehen
        long nMaxRight = GetMargin2()-GetMargin1();

        long lFences=0;
        long lMinSpace=USHRT_MAX;
        long lOldPos;
        long lColumns=0;
        USHORT nStart;
        if(!pColumnItem->IsTable())
        {
            if(nCol==USHRT_MAX)
            {
                lOldPos=GetMargin1();
                nStart=0;
            }
            else
            {
                lOldPos=pBorders[nCol].nPos+pBorders[nCol].nWidth;
                nStart=nCol+1;
                lFences=pBorders[nCol].nWidth;
            }

            for(USHORT i = nStart; i < nBorderCount-1; ++i)
            {
                long lWidth=pBorders[i].nPos-lOldPos;
                lColumns+=lWidth;
                if(lWidth<lMinSpace)
                    lMinSpace=lWidth;
                lOldPos=pBorders[i].nPos+pBorders[i].nWidth;
                lFences+=pBorders[i].nWidth;
            }
            long lWidth=GetMargin2()-lOldPos;
            lColumns+=lWidth;
            if(lWidth<lMinSpace)
                lMinSpace=lWidth;
        }
        else
        {
            USHORT nActCol;
            if(nCol==USHRT_MAX) //CalcMinMax fuer LeftMargin
            {
                lOldPos=GetMargin1();
            }
            else
            {
                lOldPos=pBorders[nCol].nPos;
            }
            lColumns=GetMargin2()-lOldPos;
            nActCol=nCol;
            lFences=0;
            while(nActCol<nBorderCount||nActCol==USHRT_MAX)
            {
                USHORT nRight;
                if(nActCol==USHRT_MAX)
                {
                    nRight=0;
                    while(!(*pColumnItem)[nRight].bVisible)
                        nRight++;
                }
                else
                    nRight=GetActRightColumn(FALSE, nActCol);
                long lWidth;
                if(nRight!=USHRT_MAX)
                {
                    lWidth=pBorders[nRight].nPos-lOldPos;
                    lOldPos=pBorders[nRight].nPos;
                }
                else
                    lWidth=GetMargin2()-lOldPos;
                nActCol=nRight;
                if(lWidth<lMinSpace)
                    lMinSpace=lWidth;
                if(nActCol==USHRT_MAX)
                    break;
            }
        }

        nMaxRight-=(long)(lFences+lMinFrame/(float)lMinSpace*lColumns);
        return nMaxRight;
    }
    else
    {
        if(pColumnItem->IsTable())
        {
            USHORT nVisCols=0;
            for(USHORT i=GetActRightColumn(FALSE, nCol);i<nBorderCount;)
            {
                if((*pColumnItem)[i].bVisible)
                    nVisCols++;
                i=GetActRightColumn(FALSE, i);
            }
            return GetMargin2()-GetMargin1()-(nVisCols+1)*lMinFrame;
        }
        else
        {
            long lWidth=0;
            for(USHORT i=nCol;i<nBorderCount-1;i++)
            {
                lWidth+=lMinFrame+pBorders[i].nWidth;
            }
            return GetMargin2()-GetMargin1()-lWidth;
        }
    }
}

