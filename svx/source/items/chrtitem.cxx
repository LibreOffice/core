/*************************************************************************
 *
 *  $RCSfile: chrtitem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:20 $
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

// include ---------------------------------------------------------------

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef __SBX_SBXVARIABLE_HXX
#include <svtools/sbxvar.hxx>
#endif
#if defined UNX && !defined LINUX
#include <wchar.h>
#endif

#ifdef MAC
#include <stdlib.h>
#endif

#include <stdio.h>
#include <float.h>

#define ITEMID_CHARTSTYLE       0
#define ITEMID_CHARTDATADESCR   0
#define ITEMID_CHARTLEGENDPOS   0
#define ITEMID_CHARTTEXTORDER   0
#define ITEMID_CHARTTEXTORIENT  0
#define ITEMID_CHARTKINDERROR   0
#define ITEMID_CHARTINDICATE    0
#define ITEMID_DOUBLE           0
#define ITEMID_CHARTREGRESS     0

#include "chrtitem.hxx"

#pragma hdrstop

using namespace ::rtl;
using namespace ::com::sun::star;
// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SvxChartStyleItem, SfxEnumItem);
TYPEINIT1(SvxChartDataDescrItem, SfxEnumItem);
TYPEINIT1(SvxChartLegendPosItem, SfxEnumItem);
TYPEINIT1(SvxChartTextOrderItem, SfxEnumItem);
TYPEINIT1(SvxChartTextOrientItem, SfxEnumItem);
TYPEINIT1(SvxChartIndicateItem, SfxEnumItem);
TYPEINIT1(SvxChartKindErrorItem, SfxEnumItem);
TYPEINIT1(SvxChartRegressItem, SfxEnumItem);
TYPEINIT1(SvxDoubleItem, SfxPoolItem);

/*************************************************************************
|*
|*  SvxChartStyleItem
|*
*************************************************************************/

SvxChartStyleItem::SvxChartStyleItem(SvxChartStyle eStyle, USHORT nId) :
    SfxEnumItem(nId, eStyle)
{
}

// -----------------------------------------------------------------------

SvxChartStyleItem::SvxChartStyleItem(SvStream& rIn, USHORT nId) :
    SfxEnumItem(nId, rIn)
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartStyleItem::Clone(SfxItemPool* pPool) const
{
    return new SvxChartStyleItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartStyleItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new SvxChartStyleItem(rIn, Which());
}

/*************************************************************************
|*
|*  SvxChartDataDescrItem
|*
*************************************************************************/

SvxChartDataDescrItem::SvxChartDataDescrItem(SvxChartDataDescr eDataDescr,
                                             USHORT nId) :
    SfxEnumItem(nId, eDataDescr)
{
}

// -----------------------------------------------------------------------

SvxChartDataDescrItem::SvxChartDataDescrItem(SvStream& rIn, USHORT nId) :
    SfxEnumItem(nId, rIn)
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartDataDescrItem::Clone(SfxItemPool* pPool) const
{
    return new SvxChartDataDescrItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartDataDescrItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new SvxChartDataDescrItem(rIn, Which());
}

/*************************************************************************
|*
|*  SvxChartLegendPosItem
|*
*************************************************************************/

SvxChartLegendPosItem::SvxChartLegendPosItem(SvxChartLegendPos eLegendPos,
                                             USHORT nId) :
    SfxEnumItem(nId, eLegendPos)
{
}

// -----------------------------------------------------------------------

SvxChartLegendPosItem::SvxChartLegendPosItem(SvStream& rIn, USHORT nId) :
    SfxEnumItem(nId, rIn)
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartLegendPosItem::Clone(SfxItemPool* pPool) const
{
    return new SvxChartLegendPosItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartLegendPosItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new SvxChartLegendPosItem(rIn, Which());
}

/*************************************************************************
|*
|*  SvxChartTextOrderItem
|*
*************************************************************************/

SvxChartTextOrderItem::SvxChartTextOrderItem(SvxChartTextOrder eOrder,
                                             USHORT nId) :
    SfxEnumItem(nId, eOrder)
{
}

// -----------------------------------------------------------------------

SvxChartTextOrderItem::SvxChartTextOrderItem(SvStream& rIn, USHORT nId) :
    SfxEnumItem(nId, rIn)
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartTextOrderItem::Clone(SfxItemPool* pPool) const
{
    return new SvxChartTextOrderItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartTextOrderItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new SvxChartTextOrderItem(rIn, Which());
}

/*************************************************************************
|*
|*  SvxChartTextOrientItem
|*
*************************************************************************/

SvxChartTextOrientItem::SvxChartTextOrientItem(SvxChartTextOrient eOrient,
                                               USHORT nId) :
    SfxEnumItem(nId, eOrient)
{
}

// -----------------------------------------------------------------------

SvxChartTextOrientItem::SvxChartTextOrientItem(SvStream& rIn, USHORT nId) :
    SfxEnumItem(nId, rIn)
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartTextOrientItem::Clone(SfxItemPool* pPool) const
{
    return new SvxChartTextOrientItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartTextOrientItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new SvxChartTextOrientItem(rIn, Which());
}

/*************************************************************************
|*
|*  SvxDoubleItem
|*
*************************************************************************/

SvxDoubleItem::SvxDoubleItem(double fValue, USHORT nId) :
    SfxPoolItem(nId),
    fVal(fValue)
{
}

// -----------------------------------------------------------------------

SvxDoubleItem::SvxDoubleItem(SvStream& rIn, USHORT nId) :
    SfxPoolItem(nId)
{
    rIn >> fVal;
}

// -----------------------------------------------------------------------

SvxDoubleItem::SvxDoubleItem(const SvxDoubleItem& rItem) :
    SfxPoolItem(rItem),
    fVal(rItem.fVal)
{
}

// -----------------------------------------------------------------------

XubString SvxDoubleItem::GetValueText() const
{
    char cBuff[80];
    sprintf(cBuff, "%e", fVal);

    return String(cBuff, sizeof(cBuff), gsl_getSystemTextEncoding());
}

// -----------------------------------------------------------------------

SfxItemPresentation SvxDoubleItem::GetPresentation
            ( SfxItemPresentation ePresentation, SfxMapUnit eCoreMetric,
              SfxMapUnit ePresentationMetric, XubString& rText, const International *) const
{
    char cBuff[80];
    sprintf(cBuff, "%e", fVal);

    rText.Erase();
    rText.AppendAscii(cBuff);

    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// -----------------------------------------------------------------------

int SvxDoubleItem::operator == (const SfxPoolItem& rItem) const
{
    return (((SvxDoubleItem&)rItem).fVal == fVal);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxDoubleItem::Clone(SfxItemPool* Pool) const
{
    return new SvxDoubleItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxDoubleItem::Create(SvStream& rIn, USHORT nVersion) const
{
    double fVal;
    rIn >> fVal;
    return new SvxDoubleItem(fVal, Which());
}

// -----------------------------------------------------------------------

SvStream& SvxDoubleItem::Store(SvStream& rOut, USHORT nItemVersion) const
{
    rOut << fVal;
    return rOut;
}

// -----------------------------------------------------------------------

double SvxDoubleItem::GetMin() const
{
    return DBL_MIN;
}

// -----------------------------------------------------------------------

double SvxDoubleItem::GetMax() const
{
    return DBL_MAX;
}

// -----------------------------------------------------------------------

SfxFieldUnit SvxDoubleItem::GetUnit() const
{
    return SFX_FUNIT_NONE;
}




// -----------------------------------------------------------------------
sal_Bool SvxDoubleItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (float)fVal;
    return sal_True;
}

// -----------------------------------------------------------------------
sal_Bool SvxDoubleItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    return rVal >>= fVal;
}



/*************************************************************************
|*
|*  SvxChartKindErrorItem
|*
*************************************************************************/

SvxChartKindErrorItem::SvxChartKindErrorItem(SvxChartKindError eOrient,
                                               USHORT nId) :
    SfxEnumItem(nId, eOrient)
{
}

// -----------------------------------------------------------------------

SvxChartKindErrorItem::SvxChartKindErrorItem(SvStream& rIn, USHORT nId) :
    SfxEnumItem(nId, rIn)
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartKindErrorItem::Clone(SfxItemPool* pPool) const
{
    return new SvxChartKindErrorItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartKindErrorItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new SvxChartKindErrorItem(rIn, Which());
}

// -----------------------------------------------------------------------

USHORT SvxChartKindErrorItem::GetVersion (USHORT nFileFormatVersion) const
{
    return (nFileFormatVersion == SOFFICE_FILEFORMAT_31)
               ? USHRT_MAX
               : 0;
}

/*************************************************************************
|*
|*  SvxChartIndicateItem
|*
*************************************************************************/

SvxChartIndicateItem::SvxChartIndicateItem(SvxChartIndicate eOrient,
                                               USHORT nId) :
    SfxEnumItem(nId, eOrient)
{
}

// -----------------------------------------------------------------------

SvxChartIndicateItem::SvxChartIndicateItem(SvStream& rIn, USHORT nId) :
    SfxEnumItem(nId, rIn)
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartIndicateItem::Clone(SfxItemPool* pPool) const
{
    return new SvxChartIndicateItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartIndicateItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new SvxChartIndicateItem(rIn, Which());
}

// -----------------------------------------------------------------------

USHORT SvxChartIndicateItem::GetVersion (USHORT nFileFormatVersion) const
{
    return (nFileFormatVersion == SOFFICE_FILEFORMAT_31)
               ? USHRT_MAX
               : 0;
}

/*************************************************************************
|*
|*  SvxChartRegressItem
|*
*************************************************************************/

SvxChartRegressItem::SvxChartRegressItem(SvxChartRegress eOrient,
                                               USHORT nId) :
    SfxEnumItem(nId, eOrient)
{
}

// -----------------------------------------------------------------------

SvxChartRegressItem::SvxChartRegressItem(SvStream& rIn, USHORT nId) :
    SfxEnumItem(nId, rIn)
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartRegressItem::Clone(SfxItemPool* pPool) const
{
    return new SvxChartRegressItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartRegressItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new SvxChartRegressItem(rIn, Which());
}

// -----------------------------------------------------------------------

USHORT SvxChartRegressItem::GetVersion (USHORT nFileFormatVersion) const
{
    return (nFileFormatVersion == SOFFICE_FILEFORMAT_31)
               ? USHRT_MAX
               : 0;
}


