/*************************************************************************
 *
 *  $RCSfile: grfatr.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-10-24 10:00:48 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _COM_SUN_STAR_TEXT_RELORIENTATION_HPP_
#include <com/sun/star/text/RelOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATION_HPP_
#include <com/sun/star/text/VertOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIZONTALADJUST_HPP_
#include <com/sun/star/text/HorizontalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_DOCUMENTSTATISTIC_HPP_
#include <com/sun/star/text/DocumentStatistic.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HPP_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATIONFORMAT_HPP_
#include <com/sun/star/text/HoriOrientationFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_NOTEPRINTMODE_HPP_
#include <com/sun/star/text/NotePrintMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_SIZETYPE_HPP_
#include <com/sun/star/text/SizeType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATIONFORMAT_HPP_
#include <com/sun/star/text/VertOrientationFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRAPTEXTMODE_HPP_
#include <com/sun/star/text/WrapTextMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_GRAPHICCROP_HPP_
#include <com/sun/star/text/GraphicCrop.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTGRAPHICOBJECTSSUPPLIER_HPP_
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#endif

#ifndef _GRFMGR_HXX
#include <goodies/grfmgr.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _UNOMID_H
#include <unomid.h>
#endif

using namespace ::com::sun::star;

TYPEINIT1_AUTOFACTORY(SwCropGrf, SfxPoolItem)
TYPEINIT1_AUTOFACTORY(SwGammaGrf, SfxPoolItem)

/******************************************************************************
 *  Implementierung     class SwMirrorGrf
 ******************************************************************************/

SfxPoolItem* SwMirrorGrf::Clone( SfxItemPool* ) const
{
    return new SwMirrorGrf( *this );
}

sal_uInt16 SwMirrorGrf::GetValueCount() const
{
    return RES_GRFMIRROR_END - RES_GRFMIRROR_BEGIN;
}

int SwMirrorGrf::operator==( const SfxPoolItem& rItem) const
{
    return SfxEnumItem::operator==(rItem) &&
            ((SwMirrorGrf&)rItem).IsGrfToggle() == IsGrfToggle();
}

BOOL lcl_IsHoriOnEvenPages(int nEnum, BOOL bToggle)
{
    BOOL bEnum = nEnum == RES_MIRROR_GRF_VERT ||
                   nEnum == RES_MIRROR_GRF_BOTH;
            return bEnum != bToggle;
}
BOOL lcl_IsHoriOnOddPages(int nEnum)
{
    BOOL bEnum = nEnum == RES_MIRROR_GRF_VERT ||
                   nEnum == RES_MIRROR_GRF_BOTH;
            return bEnum;
}
BOOL SwMirrorGrf::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    sal_Bool bRet = sal_True,
         bVal;
    // Vertikal und Horizontal sind mal getauscht worden!
    switch ( nMemberId )
    {
        case MID_MIRROR_HORZ_EVEN_PAGES:
            bVal = lcl_IsHoriOnEvenPages(GetValue(), IsGrfToggle());
        break;
        case MID_MIRROR_HORZ_ODD_PAGES:
            bVal = lcl_IsHoriOnOddPages(GetValue());
        break;
        case MID_MIRROR_VERT:
            bVal = GetValue() == RES_MIRROR_GRF_HOR ||
                   GetValue() == RES_MIRROR_GRF_BOTH;
            break;
        default:
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    rVal.setValue( &bVal, ::getBooleanCppuType() );
    return bRet;
}

BOOL SwMirrorGrf::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bRet = sal_True;
    sal_Bool bVal = *(sal_Bool*)rVal.getValue();
    // Vertikal und Horizontal sind mal getauscht worden!
    switch ( nMemberId )
    {
        case MID_MIRROR_HORZ_EVEN_PAGES:
        case MID_MIRROR_HORZ_ODD_PAGES:
        {
            BOOL bIsVert = GetValue() == RES_MIRROR_GRF_HOR ||
                                   GetValue() == RES_MIRROR_GRF_BOTH;
            BOOL bOnOddPages = nMemberId == MID_MIRROR_HORZ_EVEN_PAGES ?
                                    lcl_IsHoriOnOddPages(GetValue()) : bVal;
            BOOL bOnEvenPages = nMemberId == MID_MIRROR_HORZ_ODD_PAGES ?
                                       lcl_IsHoriOnEvenPages(GetValue(), IsGrfToggle()) : bVal;
            GRFMIRROR nEnum = bOnOddPages ?
                    bIsVert ? RES_MIRROR_GRF_BOTH : RES_MIRROR_GRF_VERT :
                        bIsVert ? RES_MIRROR_GRF_HOR : RES_DONT_MIRROR_GRF;
            BOOL bToggle = bOnOddPages != bOnEvenPages;
            SetValue(nEnum);
            SetGrfToggle( bToggle );
        }
        break;
        case MID_MIRROR_VERT:
            if ( bVal )
            {
                if ( GetValue() == RES_MIRROR_GRF_VERT )
                    SetValue( RES_MIRROR_GRF_BOTH );
                else if ( GetValue() != RES_MIRROR_GRF_BOTH )
                    SetValue( RES_MIRROR_GRF_HOR );
            }
            else
            {
                if ( GetValue() == RES_MIRROR_GRF_BOTH )
                    SetValue( RES_MIRROR_GRF_VERT );
                else if ( GetValue() == RES_MIRROR_GRF_HOR )
                    SetValue( RES_DONT_MIRROR_GRF );
            }
            break;
        default:
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    return bRet;
}


/******************************************************************************
 *  Implementierung     class SwCropGrf
 ******************************************************************************/

SwCropGrf::SwCropGrf()
    : SvxGrfCrop( RES_GRFATR_CROPGRF )
{}

SwCropGrf::SwCropGrf(sal_Int32 nL, sal_Int32 nR, sal_Int32 nT, sal_Int32 nB )
    : SvxGrfCrop( nL, nR, nT, nB, RES_GRFATR_CROPGRF )
{}

SfxPoolItem* SwCropGrf::Clone( SfxItemPool* ) const
{
    return new SwCropGrf( *this );
}

// ------------------------------------------------------------------

SfxPoolItem* SwRotationGrf::Clone( SfxItemPool * ) const
{
    return new SwRotationGrf( GetValue(), aUnrotatedSize );
}


int SwRotationGrf::operator==( const SfxPoolItem& rCmp ) const
{
    return SfxUInt16Item::operator==( rCmp ) &&
        GetUnrotatedSize() == ((SwRotationGrf&)rCmp).GetUnrotatedSize();
}


BOOL SwRotationGrf::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
/*
!!JP 28.07.00: IMPL- fehlt
    text::GraphicCrop  aCrop;
    aCrop.Left   = TWIP_TO_MM100(nLeft);
    aCrop.Right  = TWIP_TO_MM100(nRight) ;
    aCrop.Top    = TWIP_TO_MM100(nTop)   ;
    aCrop.Bottom = TWIP_TO_MM100(nBottom);
    rVal.setValue( &aCrop, ::getCppuType((text::GraphicCrop*)0) );

    return   sal_True;
*/
    return   sal_False;
}

BOOL SwRotationGrf::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bRet = sal_False;
/*
!!JP 28.07.00: IMPL- fehlt
    if(rVal.getValueType() == ::getCppuType((const text::GraphicCrop*)0))
    {
        const text::GraphicCrop* pCrop = (const text::GraphicCrop*)rVal.getValue();
Size aUnrotatedSize;
        nLeft   = MM100_TO_TWIP(pCrop->Left  );
        nRight  = MM100_TO_TWIP(pCrop->Right );
        nTop    = MM100_TO_TWIP(pCrop->Top   );
        nBottom = MM100_TO_TWIP(pCrop->Bottom);
        bRet = sal_True;
    }
    else
        //exception(wrong_type)
        ;
*/
    return bRet;
}

// ------------------------------------------------------------------

SfxPoolItem* SwLuminanceGrf::Clone( SfxItemPool *pPool ) const
{
    return new SwLuminanceGrf( *this );
}

// ------------------------------------------------------------------

SfxPoolItem* SwContrastGrf::Clone( SfxItemPool *pPool ) const
{
    return new SwContrastGrf( *this );
}

// ------------------------------------------------------------------

SfxPoolItem* SwChannelRGrf::Clone( SfxItemPool *pPool ) const
{
    return new SwChannelRGrf( *this );
}

// ------------------------------------------------------------------

SfxPoolItem* SwChannelGGrf::Clone( SfxItemPool *pPool ) const
{
    return new SwChannelGGrf( *this );
}

// ------------------------------------------------------------------

SfxPoolItem* SwChannelBGrf::Clone( SfxItemPool *pPool ) const
{
    return new SwChannelBGrf( *this );
}

// ------------------------------------------------------------------

SfxPoolItem* SwGammaGrf::Clone( SfxItemPool *pPool ) const
{
    return new SwGammaGrf( *this );
}

int SwGammaGrf::operator==( const SfxPoolItem& rCmp ) const
{
    return SfxPoolItem::operator==( rCmp ) &&
        nValue == ((SwGammaGrf&)rCmp).GetValue();
}

BOOL SwGammaGrf::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
//!!JP 28.07.00: IMPL- fehlt
    return sal_False;
}

BOOL SwGammaGrf::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bRet = sal_False;
/*
!!JP 28.07.00: IMPL- fehlt
    double aVal;
    rVal >>= aVal;

    if(rVal.getValueType() == ::getCppuType((const text::GraphicCrop*)0))
    {
        const text::GraphicCrop* pCrop = (const text::GraphicCrop*)rVal.getValue();
Size aUnrotatedSize;
        nLeft   = MM100_TO_TWIP(pCrop->Left  );
        nRight  = MM100_TO_TWIP(pCrop->Right );
        nTop    = MM100_TO_TWIP(pCrop->Top   );
        nBottom = MM100_TO_TWIP(pCrop->Bottom);
        bRet = sal_True;
    }
    else
        //exception(wrong_type)
        ;
*/
    return bRet;


}

// ------------------------------------------------------------------

SfxPoolItem* SwInvertGrf::Clone( SfxItemPool *pPool ) const
{
    return new SwInvertGrf( *this );
}

// ------------------------------------------------------------------

SfxPoolItem* SwTransparencyGrf::Clone( SfxItemPool *pPool ) const
{
    return new SwTransparencyGrf( *this );
}

// ------------------------------------------------------------------

SfxPoolItem* SwDrawModeGrf::Clone( SfxItemPool *pPool ) const
{
    return new SwDrawModeGrf( *this );
}

USHORT SwDrawModeGrf::GetValueCount() const
{
    // GRAPHICDRAWMODE_STANDARD = 0,
    // GRAPHICDRAWMODE_GREYS = 1,
    // GRAPHICDRAWMODE_MONO = 2,
    // GRAPHICDRAWMODE_WATERMARK = 3
    return GRAPHICDRAWMODE_WATERMARK + 1;
}

BOOL SwDrawModeGrf::QueryValue( com::sun::star::uno::Any& rVal,
                                BYTE nMemberId ) const
{
    return FALSE;
}

BOOL SwDrawModeGrf::PutValue( const com::sun::star::uno::Any& rVal,
                                BYTE nMemberId  )
{
    return FALSE;
}



