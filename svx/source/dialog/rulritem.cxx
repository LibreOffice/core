/*************************************************************************
 *
 *  $RCSfile: rulritem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:11 $
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

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#pragma hdrstop

#include "dialogs.hrc"
#include "rulritem.hxx"

//------------------------------------------------------------------------

TYPEINIT1(SvxPagePosSizeItem, SfxPoolItem);
TYPEINIT1(SvxLongLRSpaceItem, SfxPoolItem);
TYPEINIT1(SvxLongULSpaceItem, SfxPoolItem);
TYPEINIT1(SvxColumnItem, SfxPoolItem);
TYPEINIT1(SvxObjectItem, SfxPoolItem);

//------------------------------------------------------------------------

int SvxLongLRSpaceItem::operator==( const SfxPoolItem& rCmp) const
{
    return SfxPoolItem::operator==(rCmp) &&
        lLeft==((const SvxLongLRSpaceItem &)rCmp).lLeft &&
        lRight==((const SvxLongLRSpaceItem &)rCmp).lRight;
}


//------------------------------------------------------------------------

String SvxLongLRSpaceItem::GetValueText() const
{
    return String();
}

//------------------------------------------------------------------------

SfxItemPresentation SvxLongLRSpaceItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText, const International *
)   const
{
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

SfxPoolItem* SvxLongLRSpaceItem::Clone(SfxItemPool *pPool) const
{
    return new SvxLongLRSpaceItem(*this);
}

//------------------------------------------------------------------------

SvxLongLRSpaceItem::SvxLongLRSpaceItem(long lL, long lR, USHORT nId)
    : SfxPoolItem(nId),
    lLeft(lL),
    lRight(lR)
{}

//------------------------------------------------------------------------

SvxLongLRSpaceItem::SvxLongLRSpaceItem(const SvxLongLRSpaceItem &rCpy)
    : SfxPoolItem(rCpy),
    lLeft(rCpy.lLeft),
    lRight(rCpy.lRight)
        {}

//------------------------------------------------------------------------

int SvxLongULSpaceItem::operator==( const SfxPoolItem& rCmp) const
{
    return SfxPoolItem::operator==(rCmp) &&
        lLeft==((const SvxLongULSpaceItem &)rCmp).lLeft &&
        lRight==((const SvxLongULSpaceItem &)rCmp).lRight;
}


//------------------------------------------------------------------------

String SvxLongULSpaceItem::GetValueText() const
{
    return String();
}

//------------------------------------------------------------------------

SfxItemPresentation SvxLongULSpaceItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText, const International *
)   const
{
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

SfxPoolItem* SvxLongULSpaceItem::Clone(SfxItemPool *pPool) const
{
    return new SvxLongULSpaceItem(*this);
}

//------------------------------------------------------------------------

SvxLongULSpaceItem::SvxLongULSpaceItem(long lL, long lR, USHORT nId)
    : SfxPoolItem(nId),
    lLeft(lL),
    lRight(lR)
{}

//------------------------------------------------------------------------

SvxLongULSpaceItem::SvxLongULSpaceItem(const SvxLongULSpaceItem &rCpy)
    : SfxPoolItem(rCpy),
    lLeft(rCpy.lLeft),
    lRight(rCpy.lRight)
        {}

//------------------------------------------------------------------------

int SvxPagePosSizeItem::operator==( const SfxPoolItem& rCmp) const
{
    return SfxPoolItem::operator==(rCmp) &&
        aPos == ((const SvxPagePosSizeItem &)rCmp).aPos &&
            lWidth == ((const SvxPagePosSizeItem &)rCmp).lWidth  &&
            lHeight == ((const SvxPagePosSizeItem &)rCmp).lHeight;
}


//------------------------------------------------------------------------

String SvxPagePosSizeItem::GetValueText() const
{
    return String();
}

//------------------------------------------------------------------------

SfxItemPresentation SvxPagePosSizeItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText, const International *
)   const
{
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

SfxPoolItem* SvxPagePosSizeItem::Clone(SfxItemPool *pPool) const
{
    return new SvxPagePosSizeItem(*this);
}

//------------------------------------------------------------------------

SvxPagePosSizeItem::SvxPagePosSizeItem(const Point &rP, long lW, long lH)
    : SfxPoolItem(SID_RULER_PAGE_POS),
    aPos(rP),
    lWidth(lW),
    lHeight(lH)
{}

//------------------------------------------------------------------------

SvxPagePosSizeItem::SvxPagePosSizeItem(const SvxPagePosSizeItem &rCpy)
    : SfxPoolItem(rCpy),
    aPos(rCpy.aPos),
    lWidth(rCpy.lWidth),
    lHeight(rCpy.lHeight)
        {}


//------------------------------------------------------------------------

void SvxColumnItem::DeleteAndDestroyColumns()
{
    for( USHORT i = aColumns.Count(); i>0; )
    {
        SvxColumnDescription *pTmp = (SvxColumnDescription *)aColumns[--i];
        aColumns.Remove( i );
        delete pTmp;
    }
}

//------------------------------------------------------------------------

int SvxColumnItem::operator==(const SfxPoolItem& rCmp) const
{
    if(!SfxPoolItem::operator==(rCmp) ||
       nActColumn != ((const SvxColumnItem&)rCmp).nActColumn ||
       nLeft != ((const SvxColumnItem&)rCmp).nLeft ||
       nRight != ((const SvxColumnItem&)rCmp).nRight ||
       bTable != ((const SvxColumnItem&)rCmp).bTable ||
       Count() != ((const SvxColumnItem&)rCmp).Count())
        return FALSE;

    const USHORT nCount = ((const SvxColumnItem&)rCmp).Count();
    for(USHORT i = 0; i < nCount;++i) {
#ifdef DEBUG
        SvxColumnDescription *p1 = (SvxColumnDescription *)aColumns[i],
                             *p2 = (SvxColumnDescription *)
                                    ((const SvxColumnItem&)rCmp).aColumns[i];
#endif
        if( (*this)[i] != ((const SvxColumnItem&)rCmp)[i] )
            return FALSE;
    }
    return TRUE;
}

//------------------------------------------------------------------------

String SvxColumnItem::GetValueText() const
{
    return String();
}

//------------------------------------------------------------------------

SfxItemPresentation SvxColumnItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText, const International *
)   const
{
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

SfxPoolItem* SvxColumnItem::Clone( SfxItemPool *pPool ) const
{
    return new SvxColumnItem(*this);
}

//------------------------------------------------------------------------

SvxColumnItem::SvxColumnItem( USHORT nAct ) :

    SfxPoolItem( SID_RULER_BORDERS ),

    nActColumn  ( nAct ),
    nLeft       ( 0 ),
    nRight      ( 0 ),
    bTable      ( FALSE )

{
}

//------------------------------------------------------------------------

SvxColumnItem::SvxColumnItem( USHORT nActCol, USHORT left, USHORT right ) :

    SfxPoolItem( SID_RULER_BORDERS ),

    nActColumn  ( nActCol ),
    nLeft       ( left ),
    nRight      ( right ),
    bTable      ( TRUE )

{
}

//------------------------------------------------------------------------

SvxColumnItem::SvxColumnItem( const SvxColumnItem& rCopy ) :

    SfxPoolItem( rCopy ),

      nActColumn( rCopy.nActColumn ),
      nLeft     ( rCopy.nLeft ),
      nRight    ( rCopy.nRight ),
      bTable    ( rCopy.bTable ),
      aColumns  ( (BYTE)rCopy.Count() )

{
    const USHORT nCount = rCopy.Count();

    for ( USHORT i = 0; i < nCount; ++i )
        Append( rCopy[i] );
}

//------------------------------------------------------------------------

SvxColumnItem::~SvxColumnItem()
{
    DeleteAndDestroyColumns();
}

//------------------------------------------------------------------------

const SvxColumnItem &SvxColumnItem::operator=(const SvxColumnItem &rCopy)
{
    nLeft = rCopy.nLeft;
    nRight = rCopy.nRight;
    bTable = rCopy.bTable;
    nActColumn = rCopy.nActColumn;
    DeleteAndDestroyColumns();
    const USHORT nCount = rCopy.Count();
    for(USHORT i = 0; i < nCount;++i)
        Insert(rCopy[i], i);
    return *this;
}

//------------------------------------------------------------------------

BOOL SvxColumnItem::CalcOrtho() const
{
    const USHORT nCount = Count();
    DBG_ASSERT(nCount >= 2, "keine Spalten");
    if(nCount < 2)
        return FALSE;

    const USHORT nColWidth = (*this)[0].GetWidth();
    for(USHORT i = 1; i < nCount; ++i) {
        if( (*this)[i].GetWidth() != nColWidth)
            return FALSE;
    }
    //!! Breite Trenner
    return TRUE;
}

//------------------------------------------------------------------------

long SvxColumnItem::GetVisibleRight() const
{
    USHORT nIdx = 0;

    for ( USHORT i = 0; i < nActColumn; ++i )
    {
        if ( (*this)[i].bVisible )
            ++nIdx;
    }
    return (*this)[nIdx].nEnd;
}

//------------------------------------------------------------------------

int SvxObjectItem::operator==( const SfxPoolItem& rCmp ) const
{
    return SfxPoolItem::operator==(rCmp) &&
       nStartX == ((const SvxObjectItem&)rCmp).nStartX &&
       nEndX == ((const SvxObjectItem&)rCmp).nEndX &&
       nStartY == ((const SvxObjectItem&)rCmp).nStartY &&
       nEndY == ((const SvxObjectItem&)rCmp).nEndY &&
       bLimits == ((const SvxObjectItem&)rCmp).bLimits;
}

//------------------------------------------------------------------------

String SvxObjectItem::GetValueText() const
{
    return String();
}

//------------------------------------------------------------------------

SfxItemPresentation SvxObjectItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText, const International *
)   const
{
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

SfxPoolItem* SvxObjectItem::Clone(SfxItemPool *pPool) const
{
    return new SvxObjectItem(*this);
}

//------------------------------------------------------------------------

SvxObjectItem::SvxObjectItem( long nSX, long nEX,
                              long nSY, long nEY, BOOL limits ) :

    SfxPoolItem( SID_RULER_OBJECT ),

    nStartX ( nSX ),
    nEndX   ( nEX ),
    nStartY ( nSY ),
    nEndY   ( nEY ),
    bLimits ( limits )

{
}

//------------------------------------------------------------------------

SvxObjectItem::SvxObjectItem( const SvxObjectItem& rCopy ) :

    SfxPoolItem( rCopy ),

    nStartX ( rCopy.nStartX ),
    nEndX   ( rCopy.nEndX ),
    nStartY ( rCopy.nStartY ),
    nEndY   ( rCopy.nEndY ),
    bLimits ( rCopy.bLimits )

{
}


