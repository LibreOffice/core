/*************************************************************************
 *
 *  $RCSfile: rulritem.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:29:04 $
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

#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)    ((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))

sal_Bool SvxLongLRSpaceItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal;
    switch( nMemberId )
    {
        case MID_LEFT: nVal = lLeft; break;
        case MID_RIGHT: nVal = lRight; break;
        default: DBG_ERROR("Wrong MemberId!"); return sal_False;
    }

    if ( bConvert )
        nVal = TWIP_TO_MM100( nVal );

    rVal <<= nVal;
    return TRUE;
}

// -----------------------------------------------------------------------
sal_Bool SvxLongLRSpaceItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal;
    if ( rVal >>= nVal )
    {
        if ( bConvert )
            nVal = MM100_TO_TWIP( nVal );

        switch( nMemberId )
        {
            case MID_LEFT: lLeft = nVal; break;
            case MID_RIGHT: lRight = nVal; break;
            default: DBG_ERROR("Wrong MemberId!"); return sal_False;
        }

        return sal_True;
    }

    return sal_False;
}

//------------------------------------------------------------------------

SfxItemPresentation SvxLongLRSpaceItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText, const IntlWrapper *
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

sal_Bool SvxLongULSpaceItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal;
    switch( nMemberId )
    {
        case MID_UPPER: nVal = lLeft; break;
        case MID_LOWER: nVal = lRight; break;
        default: DBG_ERROR("Wrong MemberId!"); return sal_False;
    }

    if ( bConvert )
        nVal = TWIP_TO_MM100( nVal );

    rVal <<= nVal;
    return TRUE;
}

// -----------------------------------------------------------------------
sal_Bool SvxLongULSpaceItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal;
    if ( rVal >>= nVal )
    {
        if ( bConvert )
            nVal = MM100_TO_TWIP( nVal );

        switch( nMemberId )
        {
            case MID_UPPER: lLeft = nVal; break;
            case MID_LOWER: lRight = nVal; break;
            default: DBG_ERROR("Wrong MemberId!"); return sal_False;
        }

        return sal_True;
    }

    return sal_False;
}

//------------------------------------------------------------------------

SfxItemPresentation SvxLongULSpaceItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText, const IntlWrapper *
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

sal_Bool SvxPagePosSizeItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal;
    switch ( nMemberId )
    {
        case MID_X: nVal = aPos.X(); break;
        case MID_Y: nVal = aPos.Y(); break;
        case MID_WIDTH: nVal = lWidth; break;
        case MID_HEIGHT: nVal = lHeight; break;
        default: DBG_ERROR("Wrong MemberId!"); return sal_False;
    }

    rVal <<= nVal;
    return TRUE;
}

sal_Bool SvxPagePosSizeItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal;
    if ( rVal >>= nVal )
    {
        switch ( nMemberId )
        {
            case MID_X: aPos.X() = nVal; break;
            case MID_Y: aPos.Y() = nVal; break;
            case MID_WIDTH: lWidth = nVal; break;
            case MID_HEIGHT: lHeight = nVal; break;
            default: DBG_ERROR("Wrong MemberId!"); return sal_False;
        }

        return sal_True;
    }

    return sal_False;
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
    String&             rText, const IntlWrapper *
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
#if OSL_DEBUG_LEVEL > 1
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
    String&             rText, const IntlWrapper *
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

sal_Bool SvxColumnItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_COLUMNARRAY:
        {
            return sal_False;
            break;
        }
        case MID_RIGHT: rVal <<= nRight; break;
        case MID_LEFT: rVal <<= nLeft; break;
        case MID_ORTHO: rVal <<= (sal_Bool) bOrtho; break;
        case MID_ACTUAL: rVal <<= (sal_Int32) nActColumn; break;
        case MID_TABLE: rVal <<= (sal_Bool) bTable; break;
        default: DBG_ERROR("Wrong MemberId!"); return sal_False;
    }

    return sal_True;
}

sal_Bool SvxColumnItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    sal_Int32 nVal;
    switch ( nMemberId )
    {
        case MID_COLUMNARRAY:
        {
            return sal_False;
            break;
        }
        case MID_RIGHT: rVal >>= nRight; break;
        case MID_LEFT: rVal >>= nLeft; break;
        case MID_ORTHO: rVal >>= nVal; bOrtho = (BOOL) nVal; break;
        case MID_ACTUAL: rVal >>= nVal; nActColumn = (USHORT) nVal; break;
        case MID_TABLE: rVal >>= nVal; bTable = (BOOL) nVal; break;
        default: DBG_ERROR("Wrong MemberId!"); return sal_False;
    }

    return sal_True;
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
    String&             rText, const IntlWrapper *
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

sal_Bool SvxObjectItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    sal_Int32 nVal = 0;
    switch ( nMemberId )
    {
        case MID_START_X : rVal <<= nStartX; break;
        case MID_START_Y : rVal <<= nStartY; break;
        case MID_END_X : rVal <<= nEndX; break;
        case MID_END_Y : rVal <<= nEndY; break;
        case MID_LIMIT : rVal <<= bLimits; break;
        default:
            DBG_ERROR( "Wrong MemberId" );
            return sal_False;
    }

    return TRUE;
}

sal_Bool SvxObjectItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    BOOL bRet=FALSE;
    switch ( nMemberId )
    {
        case MID_START_X : bRet = (rVal >>= nStartX); break;
        case MID_START_Y : bRet = (rVal >>= nStartY); break;
        case MID_END_X : bRet = (rVal >>= nEndX); break;
        case MID_END_Y : bRet = (rVal >>= nEndY); break;
        case MID_LIMIT : bRet = (rVal >>= bLimits); break;
        default: DBG_ERROR( "Wrong MemberId" );
    }

    return bRet;
}

