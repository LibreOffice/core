/*************************************************************************
 *
 *  $RCSfile: grfitem.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-10-19 13:18:08 $
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

#pragma hdrstop

#define ITEMID_GRF_CROP         0

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SVX_GRFCROP_HXX
#include <grfcrop.hxx>
#endif
#ifndef _SVX_ITEMTYPE_HXX //autogen
#include <itemtype.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_GRAPHICCROP_HPP_
#include <com/sun/star/text/GraphicCrop.hpp>
#endif

using namespace ::com::sun::star;

#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)    ((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))
//TYPEINIT1_AUTOFACTORY( SvxGrfCrop, SfxPoolItem )

/******************************************************************************
 *  Implementierung     class SwCropGrf
 ******************************************************************************/

SvxGrfCrop::SvxGrfCrop( USHORT nItemId )
    : SfxPoolItem( nItemId ),
    nLeft( 0 ), nRight( 0 ), nTop( 0 ), nBottom( 0 )
{}

SvxGrfCrop::SvxGrfCrop( sal_Int32 nL, sal_Int32 nR,
                        sal_Int32 nT, sal_Int32 nB, USHORT nItemId )
    : SfxPoolItem( nItemId ),
    nLeft( nL ), nRight( nR ), nTop( nT ), nBottom( nB )
{}

SvxGrfCrop::~SvxGrfCrop()
{
}

int SvxGrfCrop::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rAttr ), "not equal attributes" );
    return nLeft    == ((const SvxGrfCrop&)rAttr).GetLeft() &&
           nRight   == ((const SvxGrfCrop&)rAttr).GetRight() &&
           nTop     == ((const SvxGrfCrop&)rAttr).GetTop() &&
           nBottom  == ((const SvxGrfCrop&)rAttr).GetBottom();
}

/*
SfxPoolItem* SvxGrfCrop::Clone( SfxItemPool* ) const
{
    return new SvxGrfCrop( *this );
}
*/

/*
USHORT SvxGrfCrop::GetVersion( USHORT nFFVer ) const
{
    DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
                SOFFICE_FILEFORMAT_40==nFFVer ||
                SOFFICE_FILEFORMAT_NOW==nFFVer,
                "SvxGrfCrop: exist a new fileformat?" );
    return GRFCROP_VERSION_SWDEFAULT;
}
*/

SfxPoolItem* SvxGrfCrop::Create( SvStream& rStrm, USHORT nVersion ) const
{
    INT32 top, left, right, bottom;
    rStrm >> top >> left >> right >> bottom;

    if( GRFCROP_VERSION_SWDEFAULT == nVersion )
        top = -top, bottom = -bottom, left = -left, right = -right;

    SvxGrfCrop* pNew = (SvxGrfCrop*)Clone();
    pNew->SetLeft( left );
    pNew->SetRight( right );
    pNew->SetTop( top );
    pNew->SetBottom( bottom );
    return pNew;
}


SvStream& SvxGrfCrop::Store( SvStream& rStrm, USHORT nVersion ) const
{
    INT32 left = GetLeft(), right = GetRight(),
            top = GetTop(), bottom = GetBottom();
    if( GRFCROP_VERSION_SWDEFAULT == nVersion )
        top = -top, bottom = -bottom, left = -left, right = -right;

    rStrm << top << left << right << bottom;

    return rStrm;
}



BOOL SvxGrfCrop::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    text::GraphicCrop aRet;
    aRet.Left   = nLeft;
    aRet.Right  = nRight;
    aRet.Top    = nTop;
    aRet.Bottom = nBottom;

    if(nMemberId&CONVERT_TWIPS)
    {
       aRet.Right   = TWIP_TO_MM100(aRet.Right );
       aRet.Top     = TWIP_TO_MM100(aRet.Top );
       aRet.Left    = TWIP_TO_MM100(aRet.Left   );
       aRet.Bottom  = TWIP_TO_MM100(aRet.Bottom);
    }


    rVal <<= aRet;
    return   sal_True;
}

BOOL SvxGrfCrop::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    if(!rVal.hasValue() || rVal.getValueType() != ::getCppuType((text::GraphicCrop*)0))
        return sal_False;
    text::GraphicCrop aVal;
    rVal >>= aVal;
    if(nMemberId&CONVERT_TWIPS)
    {
       aVal.Right   = MM100_TO_TWIP(aVal.Right );
       aVal.Top     = MM100_TO_TWIP(aVal.Top );
       aVal.Left    = MM100_TO_TWIP(aVal.Left   );
       aVal.Bottom  = MM100_TO_TWIP(aVal.Bottom);
    }

    nLeft   = aVal.Left  ;
    nRight  = aVal.Right ;
    nTop    = aVal.Top   ;
    nBottom = aVal.Bottom;
    return  sal_True;
}

SfxItemPresentation SvxGrfCrop::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit eCoreUnit, SfxMapUnit ePresUnit,
    String &rText, const International* pIntl ) const
{
    rText.Erase();
    switch( ePres )
    {
    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        if( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
        {
            ( rText.AssignAscii( "L: " )) += ::GetMetricText( GetLeft(),
                                            eCoreUnit, SFX_MAPUNIT_MM );
            ( rText.AppendAscii( " R: " )) += ::GetMetricText( GetRight(),
                                            eCoreUnit, SFX_MAPUNIT_MM );
            ( rText.AppendAscii( " T: " )) += ::GetMetricText( GetTop(),
                                            eCoreUnit, SFX_MAPUNIT_MM );
            ( rText.AppendAscii( " B: " )) += ::GetMetricText( GetBottom(),
                                            eCoreUnit, SFX_MAPUNIT_MM );
        }
        break;

    default:
        ePres = SFX_ITEM_PRESENTATION_NONE;
        break;
    }
    return ePres;
}




