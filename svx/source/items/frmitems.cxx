/*************************************************************************
 *
 *  $RCSfile: frmitems.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: mib $ $Date: 2000-11-08 11:19:06 $
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

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#include <limits.h>

#pragma hdrstop

#define ITEMID_PAPERBIN 0
#define ITEMID_SIZE     0
#define ITEMID_LRSPACE  0
#define ITEMID_ULSPACE  0
#define ITEMID_PRINT    0
#define ITEMID_OPAQUE   0
#define ITEMID_PROTECT  0
#define ITEMID_SHADOW   0
#define ITEMID_BOX      0
#define ITEMID_BOXINFO  0
#define ITEMID_FMTBREAK 0
#define ITEMID_FMTKEEP  0
#define ITEMID_LINE     0
#define ITEMID_BRUSH    0


#ifndef _ARGS_HXX //autogen
#include <svtools/args.hxx>
#endif
#ifndef _GRFMGR_HXX //autogen
#include <goodies/grfmgr.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef SVX_LIGHT
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#endif // !SVX_LIGHT
#include <svtools/sbx.hxx>
#define GLOBALOVERFLOW3

#define _SVX_FRMITEMS_CXX

#include <svtools/memberid.hrc>

#ifndef _WALLITEM_HXX
#include <svtools/wallitem.hxx>
#endif
#ifndef _CNTWALL_HXX
#include <svtools/cntwall.hxx>
#endif

// xml stuff
#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_XMLITMAP_HXX
#include <xmloff/xmlitmap.hxx>
#endif
#ifndef _XMLOFF_XMLITEM_HXX
#include <xmloff/xmlitem.hxx>
#endif
#include <rtl/ustring>
#include <rtl/ustrbuf.hxx>

#include <impgrf.hxx>
#include "svxids.hrc"
#include "svxitems.hrc"
#include "dialogs.hrc"

#include "pbinitem.hxx"
#include "sizeitem.hxx"
#include "lrspitem.hxx"
#include "ulspitem.hxx"
#include "prntitem.hxx"
#include "opaqitem.hxx"
#include "protitem.hxx"
#include "shaditem.hxx"
#include "boxitem.hxx"
#include "brkitem.hxx"
#include "keepitem.hxx"
#include "bolnitem.hxx"
#include "brshitem.hxx"
#include "backgrnd.hxx"

#include "itemtype.hxx"
#include "dialmgr.hxx"
#include "svxerr.hxx"

#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLVERTJUSTIFY_HPP_
#include <com/sun/star/table/CellVertJustify.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_SHADOWLOCATION_HPP_
#include <com/sun/star/table/ShadowLocation.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_TABLEBORDER_HPP_
#include <com/sun/star/table/TableBorder.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_SHADOWFORMAT_HPP_
#include <com/sun/star/table/ShadowFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLRANGEADDRESS_HPP_
#include <com/sun/star/table/CellRangeAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLCONTENTTYPE_HPP_
#include <com/sun/star/table/CellContentType.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_TABLEORIENTATION_HPP_
#include <com/sun/star/table/TableOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLHORIJUSTIFY_HPP_
#include <com/sun/star/table/CellHoriJustify.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SORTFIELD_HPP_
#include <com/sun/star/util/SortField.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SORTFIELDTYPE_HPP_
#include <com/sun/star/util/SortFieldType.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLORIENTATION_HPP_
#include <com/sun/star/table/CellOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_PAGESTYLELAYOUT_HPP_
#include <com/sun/star/style/PageStyleLayout.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_BREAKTYPE_HPP_
#include <com/sun/star/style/BreakType.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_GRAPHICLOCATION_HPP_
#include <com/sun/star/style/GraphicLocation.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SELECTION_HPP_
#include <com/sun/star/awt/Selection.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

#ifndef _SVX_UNOMID_HXX
#include <unomid.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;


// Konvertierung fuer UNO
#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)    ((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))

// STATIC DATA -----------------------------------------------------------


inline void SetValueProp( XubString& rStr, const sal_uInt16 nValue,
                          const sal_uInt16 nProp )
{
    if( 100 == nProp )
        rStr += String::CreateFromInt32( nValue );
    else
        ( rStr += String::CreateFromInt32( nProp )) += sal_Unicode('%');
}

inline void SetValueProp( XubString& rStr, const short nValue,
                          const sal_uInt16 nProp )
{
    if( 100 == nProp )
        rStr += String::CreateFromInt32( nValue );
    else
        ( rStr += String::CreateFromInt32( nProp )) += sal_Unicode('%');
}

// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SvxPaperBinItem, SfxByteItem);
TYPEINIT1_AUTOFACTORY(SvxSizeItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxLRSpaceItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxULSpaceItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxPrintItem, SfxBoolItem);
TYPEINIT1_AUTOFACTORY(SvxOpaqueItem, SfxBoolItem);
TYPEINIT1_AUTOFACTORY(SvxProtectItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxBrushItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxShadowItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxBoxItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxBoxInfoItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxFmtBreakItem, SfxEnumItem);
TYPEINIT1_AUTOFACTORY(SvxFmtKeepItem, SfxBoolItem);
TYPEINIT1_AUTOFACTORY(SvxLineItem, SfxPoolItem);

// class SvxPaperBinItem ------------------------------------------------

SfxPoolItem* SvxPaperBinItem::Clone( SfxItemPool* ) const
{
    return new SvxPaperBinItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxPaperBinItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    rStrm << GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxPaperBinItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 nBin;
    rStrm >> nBin;
    return new SvxPaperBinItem( Which(), nBin );
}

// -----------------------------------------------------------------------

SfxItemPresentation SvxPaperBinItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;

        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = String::CreateFromInt32( GetValue() );
            return SFX_ITEM_PRESENTATION_NAMELESS;

        case SFX_ITEM_PRESENTATION_COMPLETE:
            sal_Int8 nValue = GetValue();

            if ( PAPERBIN_PRINTER_SETTINGS == nValue )
                rText = SVX_RESSTR(RID_SVXSTR_PAPERBIN_SETTINGS);
            else
            {
                rText = SVX_RESSTR(RID_SVXSTR_PAPERBIN);
                rText += sal_Unicode(' ');
                rText += String::CreateFromInt32( nValue );
            }
            return SFX_ITEM_PRESENTATION_COMPLETE;
    }
#endif // !SVX_LIGHT
    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxSizeItem -----------------------------------------------------

SvxSizeItem::SvxSizeItem( const sal_uInt16 nId, const Size& rSize ) :

    SfxPoolItem( nId ),

    aSize( rSize )
{
}

// -----------------------------------------------------------------------
sal_Bool SvxSizeItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    awt::Size aTmp(aSize.Width(), aSize.Height());
    if(nMemberId&CONVERT_TWIPS)
    {
        aTmp.Height = TWIP_TO_MM100(aTmp.Height);
        aTmp.Width = TWIP_TO_MM100(aTmp.Width);
    }
    switch(nMemberId&(~CONVERT_TWIPS))
    {
        case MID_SIZE_SIZE:  rVal <<= aTmp; break;
        case MID_SIZE_WIDTH: rVal <<= aTmp.Width; break;
        case MID_SIZE_HEIGHT: rVal <<= aTmp.Height;  break;
    }
    return sal_True;
}
// -----------------------------------------------------------------------
sal_Bool SvxSizeItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    switch(nMemberId&(~CONVERT_TWIPS))
    {
        case MID_SIZE_SIZE:
        {
            awt::Size aTmp;
            if( rVal >>= aTmp )
            {
                if(bConvert)
                {
                    aTmp.Height = MM100_TO_TWIP(aTmp.Height);
                    aTmp.Width = MM100_TO_TWIP(aTmp.Width);
                }
                aSize = Size( aTmp.Width, aTmp.Height );
            }
            else
            {
                return sal_False;
            }
        }
        break;
        case MID_SIZE_WIDTH:
        {
            sal_Int32 nVal;
            if(!(rVal >>= nVal ))
                return sal_False;

            aSize.Width() = bConvert ? MM100_TO_TWIP(nVal) : nVal;
        }
        break;
        case MID_SIZE_HEIGHT:
        {
            sal_Int32 nVal;
            if(!(rVal >>= nVal))
                return sal_True;

            aSize.Height() = bConvert ? MM100_TO_TWIP(nVal) : nVal;
        }
        break;
        default:
            return sal_False;
    }
    return sal_True;
}

// -----------------------------------------------------------------------

SvxSizeItem::SvxSizeItem( const sal_uInt16 nId ) :

    SfxPoolItem( nId )
{
}

// -----------------------------------------------------------------------

int SvxSizeItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return ( aSize == ( (SvxSizeItem&)rAttr ).GetSize() );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxSizeItem::Clone( SfxItemPool* ) const
{
    return new SvxSizeItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxSizeItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;

        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = GetMetricText( aSize.Width(), eCoreUnit, ePresUnit );
            rText += cpDelim;
            rText += GetMetricText( aSize.Height(), eCoreUnit, ePresUnit );
            return SFX_ITEM_PRESENTATION_NAMELESS;

        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = SVX_RESSTR(RID_SVXITEMS_SIZE_WIDTH);
            rText += GetMetricText( aSize.Width(), eCoreUnit, ePresUnit );
            rText += SVX_RESSTR(GetMetricId(ePresUnit));
            rText += cpDelim;
            rText += SVX_RESSTR(RID_SVXITEMS_SIZE_HEIGHT);
            rText += GetMetricText( aSize.Height(), eCoreUnit, ePresUnit );
            rText += SVX_RESSTR(GetMetricId(ePresUnit));
            return SFX_ITEM_PRESENTATION_COMPLETE;
    }
#endif // !SVX_LIGHT
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SvStream& SvxSizeItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    rStrm << aSize.Width();
    rStrm << aSize.Height();
    return rStrm;
}

// -----------------------------------------------------------------------

int SvxSizeItem::ScaleMetrics( long nMult, long nDiv )
{
    aSize.Width() = Scale( aSize.Width(), nMult, nDiv );
    aSize.Height() = Scale( aSize.Height(), nMult, nDiv );
    return 1;
}

// -----------------------------------------------------------------------

int SvxSizeItem::HasMetrics() const
{
    return 1;
}

// -----------------------------------------------------------------------


SfxPoolItem* SvxSizeItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    long nWidth, nHeight;
    rStrm >> nWidth >> nHeight;

    SvxSizeItem* pAttr = new SvxSizeItem( Which() );
    pAttr->SetSize(Size(nWidth, nHeight));

    return pAttr;
}

// class SvxLRSpaceItem --------------------------------------------------

SvxLRSpaceItem::SvxLRSpaceItem( const sal_uInt16 nId ) :

    SfxPoolItem( nId ),

    nFirstLineOfst  ( 0 ), nPropFirstLineOfst( 100 ),
    nTxtLeft        ( 0 ),
    nLeftMargin     ( 0 ), nPropLeftMargin( 100 ),
    nRightMargin    ( 0 ), nPropRightMargin( 100 ),
    bBulletFI       ( 0 ),
    bAutoFirst      ( 0 )
{
}

// -----------------------------------------------------------------------

SvxLRSpaceItem::SvxLRSpaceItem( const sal_Int32 nLeft, const sal_Int32 nRight,
                                const sal_Int32 nTLeft, const short nOfset,
                                const sal_uInt16 nId ) :

    SfxPoolItem( nId ),

    nFirstLineOfst  ( nOfset ), nPropFirstLineOfst( 100 ),
    nTxtLeft        ( nTLeft ),
    nLeftMargin     ( nLeft ), nPropLeftMargin( 100 ),
    nRightMargin    ( nRight ), nPropRightMargin( 100 ),
    bBulletFI       ( 0 ),
    bAutoFirst      ( 0 )
{
}

// -----------------------------------------------------------------------
sal_Bool SvxLRSpaceItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    sal_Bool bRet = sal_True;
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        //  jetzt alles signed
        case MID_L_MARGIN:
            rVal <<= (sal_Int32)(bConvert ? TWIP_TO_MM100(nLeftMargin) : nLeftMargin);
            break;

        case MID_TXT_LMARGIN :
            rVal <<= (sal_Int32)(bConvert ? TWIP_TO_MM100(nTxtLeft) : nTxtLeft);
        break;
        case MID_R_MARGIN:
            rVal <<= (sal_Int32)(bConvert ? TWIP_TO_MM100(nRightMargin) : nRightMargin);
            break;
        case MID_L_REL_MARGIN:
            rVal <<= (sal_Int16)nPropLeftMargin;
        break;
        case MID_R_REL_MARGIN:
            rVal <<= (sal_Int16)nPropRightMargin;
        break;

        case MID_FIRST_LINE_INDENT:
            rVal <<= (sal_Int32)(bConvert ? TWIP_TO_MM100(nFirstLineOfst) : nFirstLineOfst);
            break;

        case MID_FIRST_LINE_REL_INDENT:
            rVal <<= (sal_Int16)(nPropFirstLineOfst);
            break;

        case MID_FIRST_AUTO:
            rVal = Bool2Any(IsAutoFirst());
            break;

        default:
            bRet = sal_False;
            DBG_ERROR("unknown MemberId");
    }
    return bRet;
}

// -----------------------------------------------------------------------
sal_Bool SvxLRSpaceItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    sal_Int32 nMaxVal = bConvert ? TWIP_TO_MM100(USHRT_MAX) : USHRT_MAX;
    sal_Int32 nVal;
    if( nMemberId != MID_FIRST_AUTO &&
            nMemberId != MID_L_REL_MARGIN && nMemberId != MID_R_REL_MARGIN)
        if(!(rVal >>= nVal))
            return sal_False;

    switch( nMemberId )
    {
        case MID_L_MARGIN:
            SetLeft((sal_Int32)bConvert ? MM100_TO_TWIP(nVal) : nVal);
            break;

        case MID_TXT_LMARGIN :
            SetTxtLeft((sal_Int32)bConvert ? MM100_TO_TWIP(nVal) : nVal);
        break;

        case MID_R_MARGIN:
            SetRight((sal_Int32)    bConvert ? MM100_TO_TWIP(nVal) : nVal);
            break;
        case MID_L_REL_MARGIN:
        case MID_R_REL_MARGIN:
        {
            sal_Int32 nRel;
            if((rVal >>= nRel) && nRel > 1)
            {
                if(MID_L_REL_MARGIN== nMemberId)
                    nPropLeftMargin = nRel;
                else
                    nPropRightMargin = nRel;
            }
            else
                return FALSE;
        }
        break;
        case MID_FIRST_LINE_INDENT     :
            SetTxtFirstLineOfst(bConvert ?  MM100_TO_TWIP(nVal) : nVal);
            break;

        case MID_FIRST_LINE_REL_INDENT:
            SetPropTxtFirstLineOfst ( nVal );
            break;

        case MID_FIRST_AUTO:
            SetAutoFirst( Any2Bool(rVal) );
            break;

        default:
            DBG_ERROR("unknown MemberId");
            return sal_False;
    }
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxLRSpaceItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
    sal_Bool bOk = sal_False;
#ifndef SVX_LIGHT

    switch( nMemberId )
    {
    case MID_L_MARGIN:
    case MID_R_MARGIN:
        {
            sal_Int32 nProp = 100;
            sal_Int32 nAbs = 0;

            if( rValue.indexOf( sal_Unicode('%') ) != -1 )
                bOk = rUnitConverter.convertPercent( nProp, rValue );
            else
                bOk = rUnitConverter.convertMeasure( nAbs, rValue, 0x0000, 0xffff );

            if( bOk )
            {
                switch( nMemberId )
                {
                case MID_L_MARGIN:
                    SetTxtLeft( (sal_Int32)nAbs, (sal_uInt16)nProp );
                    break;
                case MID_R_MARGIN:
                    SetRight( (sal_Int32)nAbs, (sal_uInt16)nProp );
                    break;
                case MID_FIRST_LINE_INDENT:
                    break;
                }
            }
        }
        break;
     case MID_FIRST_LINE_INDENT:
        {
            sal_Int32 nProp = 100;
            sal_Int32 nAbs = 0;

            if( rValue.indexOf( sal_Unicode('%') ) != -1 )
                bOk = rUnitConverter.convertPercent( nProp, rValue );
            else
                bOk = rUnitConverter.convertMeasure( nAbs, rValue, -0x7fff, 0x7fff );

            SetTxtFirstLineOfst( (short)nAbs, (sal_uInt16)nProp );
        }

    case MID_FIRST_AUTO:
        {
            sal_Bool bAutoFirst;
            bOk = rUnitConverter.convertBool( bAutoFirst, rValue );
            if( bOk )
                SetAutoFirst( bAutoFirst );
        }
        break;

    default:
        DBG_ERROR( "unknown member id!");
    }

#endif
    return bOk;
}

// -----------------------------------------------------------------------


sal_Bool SvxLRSpaceItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    // HACK FINAL_ASSERT( !rLRSpace.IsBulletFI(), "svxxml_out_SvxLRSpaceItem_para: Remember MIB to ask MT if this is OK" );
    OUStringBuffer aOut;

    sal_Bool bRet = sal_True;
    switch( nMemberId )
    {
    case  MID_L_MARGIN:
        if(GetPropLeft() != 100)
            rUnitConverter.convertPercent( aOut, GetPropLeft() );
        else
            rUnitConverter.convertMeasure( aOut, GetTxtLeft() );
        break;

    case  MID_R_MARGIN:
        if(GetPropRight() != 100)
            rUnitConverter.convertPercent( aOut, GetPropRight() );
        else
            rUnitConverter.convertMeasure( aOut, GetRight() );
        break;

    case MID_FIRST_AUTO:
        if( IsAutoFirst() )
            rUnitConverter.convertBool( aOut, IsAutoFirst() );
        else
            bRet = sal_False;
        break;

     case  MID_FIRST_LINE_INDENT:
        if( !IsAutoFirst() )
        {
            if(GetPropTxtFirstLineOfst() != 100)
                rUnitConverter.convertPercent( aOut,
                                        GetPropTxtFirstLineOfst() );
            else
                rUnitConverter.convertMeasure( aOut,
                                        nFirstLineOfst );
        }
        else
            bRet = sal_False;
        break;

    default:
        DBG_ERROR( "unknown member id!");
        bRet = sal_False;
        break;
    }

    rValue = aOut.makeStringAndClear();

    return bRet;
#else
    return sal_False;
#endif
}


// -----------------------------------------------------------------------

// nLeftMargin und nTxtLeft anpassen.

void SvxLRSpaceItem::AdjustLeft()
{
    if ( 0 > nFirstLineOfst )
        nLeftMargin = nTxtLeft + nFirstLineOfst;
    else
        nLeftMargin = nTxtLeft;
}

// -----------------------------------------------------------------------

int SvxLRSpaceItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return (
        nLeftMargin == ((SvxLRSpaceItem&)rAttr).GetLeft()  &&
        nRightMargin == ((SvxLRSpaceItem&)rAttr).GetRight() &&
        nFirstLineOfst == ((SvxLRSpaceItem&)rAttr).GetTxtFirstLineOfst() &&
        nPropLeftMargin == ((SvxLRSpaceItem&)rAttr).GetPropLeft()  &&
        nPropRightMargin == ((SvxLRSpaceItem&)rAttr).GetPropRight() &&
        nPropFirstLineOfst == ((SvxLRSpaceItem&)rAttr).GetPropTxtFirstLineOfst() &&
        bBulletFI == ((SvxLRSpaceItem&)rAttr).IsBulletFI() &&
        bAutoFirst == ((SvxLRSpaceItem&)rAttr).IsAutoFirst() );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxLRSpaceItem::Clone( SfxItemPool* ) const
{
    return new SvxLRSpaceItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxLRSpaceItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            if ( 100 != nPropLeftMargin )
                ( rText = String::CreateFromInt32( nPropLeftMargin )) += sal_Unicode('%');
            else
                rText = GetMetricText( (long)nLeftMargin,
                                       eCoreUnit, ePresUnit );
            rText += cpDelim;
            if ( 100 != nPropFirstLineOfst )
                ( rText += String::CreateFromInt32( nPropFirstLineOfst )) += sal_Unicode('%');
            else
                rText += GetMetricText( (long)nFirstLineOfst,
                                        eCoreUnit, ePresUnit );
            rText += cpDelim;
            if ( 100 != nRightMargin )
                ( rText += String::CreateFromInt32( nRightMargin )) += sal_Unicode('%');
            else
                rText += GetMetricText( (long)nRightMargin,
                                        eCoreUnit, ePresUnit );
            return SFX_ITEM_PRESENTATION_NAMELESS;
        }
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = SVX_RESSTR(RID_SVXITEMS_LRSPACE_LEFT);
            if ( 100 != nPropLeftMargin )
                ( rText += String::CreateFromInt32( nPropLeftMargin )) += sal_Unicode('%');
            else
            {
                rText += GetMetricText( (long)nLeftMargin,
                                       eCoreUnit, ePresUnit );
                rText += SVX_RESSTR(GetMetricId(ePresUnit));
            }
            rText += cpDelim;
            if ( 100 != nPropFirstLineOfst || nFirstLineOfst )
            {
                rText += SVX_RESSTR(RID_SVXITEMS_LRSPACE_FLINE);
                if ( 100 != nPropFirstLineOfst )
                    ( rText += String::CreateFromInt32( nPropFirstLineOfst ))
                            += sal_Unicode('%');
                else
                {
                    rText += GetMetricText( (long)nFirstLineOfst,
                                            eCoreUnit, ePresUnit );
                    rText += SVX_RESSTR(GetMetricId(ePresUnit));
                }
                rText += cpDelim;
            }
            rText += SVX_RESSTR(RID_SVXITEMS_LRSPACE_RIGHT);
            if ( 100 != nPropRightMargin )
                ( rText += String::CreateFromInt32( nPropRightMargin )) += sal_Unicode('%');
            else
            {
                rText += GetMetricText( (long)nRightMargin,
                                        eCoreUnit, ePresUnit );
                rText += SVX_RESSTR(GetMetricId(ePresUnit));
            }
            return SFX_ITEM_PRESENTATION_COMPLETE;
        }
    }
#endif // !SVX_LIGHT
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

// MT: BulletFI: Vor 501 wurde im Outliner das Bullet nicht auf der Position des
// FI positioniert, deshalb muss in aelteren Dokumenten der FI auf 0 stehen.

#define BULLETLR_MARKER 0x599401FE

SvStream& SvxLRSpaceItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    short nSaveFI = nFirstLineOfst;
    if ( IsBulletFI() )
        ((SvxLRSpaceItem*)this)->SetTxtFirstLineOfst( 0 );  // nLeftMargin wird mitmanipuliert, siehe Create()

    sal_uInt16 nMargin = 0;
    if( nLeftMargin > 0 )
        nMargin = sal_uInt16( nLeftMargin );
    rStrm << nMargin;
    rStrm << nPropLeftMargin;
    if( nRightMargin > 0 )
        nMargin = sal_uInt16( nRightMargin );
    else
        nMargin = 0;
    rStrm << nMargin;
    rStrm << nPropRightMargin;
    rStrm << nFirstLineOfst;
    rStrm << nPropFirstLineOfst;
    if( nTxtLeft > 0 )
        nMargin = sal_uInt16( nTxtLeft );
    else
        nMargin = 0;
    rStrm << nMargin;
    if( nItemVersion >= LRSPACE_AUTOFIRST_VERSION )
    {
        sal_Int8 nAutoFirst = bAutoFirst ? 1 : 0;
        if( nItemVersion >= LRSPACE_NEGATIVE_VERSION &&
            ( nLeftMargin < 0 || nRightMargin < 0 || nTxtLeft < 0 ) )
            nAutoFirst |= 0x80;
        rStrm << nAutoFirst;

        if ( IsBulletFI() )
        {
            // Ab 6.0 keine Magicnumber schreiben...
            DBG_ASSERT( SOFFICE_FILEFORMAT_NOW <= SOFFICE_FILEFORMAT_50, "MT: Fileformat SvxLRSpaceItem aendern!" );
            rStrm << (sal_uInt32) BULLETLR_MARKER;
            rStrm << nSaveFI;
        }
        if( 0x80 & nAutoFirst )
        {
            rStrm << nLeftMargin;
            rStrm << nRightMargin;
        }
    }

    if ( IsBulletFI() )
        ((SvxLRSpaceItem*)this)->SetTxtFirstLineOfst( nSaveFI );

    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxLRSpaceItem::Create( SvStream& rStrm, sal_uInt16 nVersion ) const
{
    sal_uInt16 left, prpleft, right, prpright, prpfirstline, txtleft;
    short firstline;
    sal_Int8 autofirst = 0;

    if ( nVersion >= LRSPACE_AUTOFIRST_VERSION )
    {
        rStrm >> left >> prpleft >> right >> prpright >> firstline >>
                 prpfirstline >> txtleft >> autofirst;

        sal_uInt32 nPos = rStrm.Tell();
        sal_uInt32 nMarker;
        rStrm >> nMarker;
        if ( nMarker == BULLETLR_MARKER )
        {
            rStrm >> firstline;
            if ( firstline < 0 )
                left += firstline;  // s.u.: txtleft = ...
        }
        else
            rStrm.Seek( nPos );
    }
    else if ( nVersion == LRSPACE_TXTLEFT_VERSION )
    {
        rStrm >> left >> prpleft >> right >> prpright >> firstline >>
                 prpfirstline >> txtleft;
    }
    else if ( nVersion == LRSPACE_16_VERSION )
    {
        rStrm >> left >> prpleft >> right >> prpright >> firstline >>
                 prpfirstline;
    }
    else
    {
        sal_Int8 nL, nR, nFL;
        rStrm >> left >> nL >> right >> nR >> firstline >> nFL;
        prpleft = (sal_uInt16)nL;
        prpright = (sal_uInt16)nR;
        prpfirstline = (sal_uInt16)nFL;
    }

    txtleft = firstline >= 0 ? left : left - firstline;
    SvxLRSpaceItem* pAttr = new SvxLRSpaceItem( Which() );

    pAttr->nLeftMargin = left;
    pAttr->nPropLeftMargin = prpleft;
    pAttr->nRightMargin = right;
    pAttr->nPropRightMargin = prpright;
    pAttr->nFirstLineOfst = firstline;
    pAttr->nPropFirstLineOfst = prpfirstline;
    pAttr->nTxtLeft = txtleft;
    pAttr->bAutoFirst = autofirst & 0x01;
    if( nVersion >= LRSPACE_NEGATIVE_VERSION && ( autofirst & 0x80 ) )
    {
        sal_Int32 nMargin;
        rStrm >> nMargin;
        pAttr->nLeftMargin = nMargin;
        pAttr->nTxtLeft = firstline >= 0 ? nMargin : nMargin - firstline;
        rStrm >> nMargin;
        pAttr->nRightMargin = nMargin;
    }
    return pAttr;
}

// -----------------------------------------------------------------------

sal_uInt16 SvxLRSpaceItem::GetVersion( sal_uInt16 nFileVersion ) const
{
    return (nFileVersion == SOFFICE_FILEFORMAT_31)
               ? LRSPACE_TXTLEFT_VERSION
               : LRSPACE_NEGATIVE_VERSION;
}

// -----------------------------------------------------------------------

int SvxLRSpaceItem::ScaleMetrics( long nMult, long nDiv )
{
    nFirstLineOfst = (short)Scale( nFirstLineOfst, nMult, nDiv );
    nTxtLeft = Scale( nTxtLeft, nMult, nDiv );
    nLeftMargin = Scale( nLeftMargin, nMult, nDiv );
    nRightMargin = Scale( nRightMargin, nMult, nDiv );
    return 1;
}

// -----------------------------------------------------------------------

int SvxLRSpaceItem::HasMetrics() const
{
    return 1;
}

// class SvxULSpaceItem --------------------------------------------------

SvxULSpaceItem::SvxULSpaceItem( const sal_uInt16 nId ) :

    SfxPoolItem( nId ),

    nUpper( 0 ), nPropUpper( 100 ),
    nLower( 0 ), nPropLower( 100 )
{
}

// -----------------------------------------------------------------------

SvxULSpaceItem::SvxULSpaceItem( const sal_uInt16 nUp, const sal_uInt16 nLow,
                                const sal_uInt16 nId ) :

    SfxPoolItem( nId ),

    nUpper( nUp  ), nPropUpper( 100 ),
    nLower( nLow ), nPropLower( 100 )
{
}

// -----------------------------------------------------------------------
sal_Bool    SvxULSpaceItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        //  jetzt alles signed
        case  MID_UP_MARGIN:    rVal <<= (sal_Int32)(bConvert ? TWIP_TO_MM100(nUpper) : nUpper); break;
        case  MID_LO_MARGIN:    rVal <<= (sal_Int32)(bConvert ? TWIP_TO_MM100(nLower) : nLower); break;
        case MID_UP_REL_MARGIN: rVal <<= (sal_Int16) nPropUpper; break;
        case MID_LO_REL_MARGIN: rVal <<= (sal_Int16) nPropLower; break;
    }
    return sal_True;
}

// -----------------------------------------------------------------------
sal_Bool SvxULSpaceItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    sal_Int32 nVal;
    switch( nMemberId )
    {
        case MID_UP_MARGIN :
            if(!(rVal >>= nVal) || nVal < 0)
                return sal_False;
            SetUpper((sal_uInt16)bConvert ? MM100_TO_TWIP(nVal) : nVal);
            break;
        case MID_LO_MARGIN :
            if(!(rVal >>= nVal) || nVal < 0)
                return sal_False;
            SetLower((sal_uInt16)bConvert ? MM100_TO_TWIP(nVal) : nVal);
            break;
        case MID_UP_REL_MARGIN:
        case MID_LO_REL_MARGIN:
        {
            sal_Int32 nRel;
            if((rVal >>= nRel) && nRel > 1 )
            {
                if(MID_UP_REL_MARGIN == nMemberId)
                    nPropUpper = nRel;
                else
                    nPropLower = nRel;
            }
            else
                return FALSE;
        }
        break;


        default:
            DBG_ERROR("unknown MemberId");
            return sal_False;
    }
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxULSpaceItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
    sal_Bool bOk = sal_False;
#ifndef SVX_LIGHT
    sal_Int32 nProp = 100;
    sal_Int32 nAbs = 0;

    if( rValue.indexOf( sal_Unicode('%') ) != -1 )
        bOk = rUnitConverter.convertPercent( nProp, rValue );
    else
        bOk = rUnitConverter.convertMeasure( nAbs, rValue );

    switch( nMemberId )
    {
    case MID_UP_MARGIN:
        SetUpper( (sal_uInt16)nAbs, (sal_uInt16)nProp );
        break;
    case MID_LO_MARGIN:
        SetLower( (sal_uInt16)nAbs, (sal_uInt16)nProp );
        break;
    default:
        DBG_ERROR("unknown MemberId");
    };
#endif
    return bOk;
}

// -----------------------------------------------------------------------


sal_Bool SvxULSpaceItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT

    OUStringBuffer aOut;

    switch( nMemberId )
    {
    case MID_UP_MARGIN:
        if( GetPropUpper() != 100 )
            rUnitConverter.convertPercent( aOut, GetPropUpper() );
        else
            rUnitConverter.convertMeasure( aOut, GetUpper() );
        break;

    case MID_LO_MARGIN:
        if( GetPropLower() != 100 )
            rUnitConverter.convertPercent( aOut, GetPropLower() );
        else
            rUnitConverter.convertMeasure( aOut, GetLower() );
        break;

    default:
        DBG_ERROR("unknown MemberId");
    };

    rValue = aOut.makeStringAndClear();
    return sal_True;
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------

int SvxULSpaceItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return ( nUpper == ( (SvxULSpaceItem&)rAttr ).nUpper &&
             nLower == ( (SvxULSpaceItem&)rAttr ).nLower &&
             nPropUpper == ( (SvxULSpaceItem&)rAttr ).nPropUpper &&
             nPropLower == ( (SvxULSpaceItem&)rAttr ).nPropLower );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxULSpaceItem::Clone( SfxItemPool* ) const
{
    return new SvxULSpaceItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxULSpaceItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            if ( 100 != nPropUpper )
                ( rText = String::CreateFromInt32( nPropUpper )) += sal_Unicode('%');
            else
                rText = GetMetricText( (long)nUpper, eCoreUnit, ePresUnit );
            rText += cpDelim;
            if ( 100 != nPropLower )
                ( rText += String::CreateFromInt32( nPropLower )) += sal_Unicode('%');
            else
                rText += GetMetricText( (long)nLower, eCoreUnit, ePresUnit );
            return SFX_ITEM_PRESENTATION_NAMELESS;
        }
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = SVX_RESSTR(RID_SVXITEMS_ULSPACE_UPPER);
            if ( 100 != nPropUpper )
                ( rText += String::CreateFromInt32( nPropUpper )) += sal_Unicode('%');
            else
            {
                rText += GetMetricText( (long)nUpper, eCoreUnit, ePresUnit );
                rText += SVX_RESSTR(GetMetricId(ePresUnit));
            }
            rText += cpDelim;
            rText += SVX_RESSTR(RID_SVXITEMS_ULSPACE_LOWER);
            if ( 100 != nPropLower )
                ( rText += String::CreateFromInt32( nPropLower )) += sal_Unicode('%');
            else
            {
                rText += GetMetricText( (long)nLower, eCoreUnit, ePresUnit );
                rText += SVX_RESSTR(GetMetricId(ePresUnit));
            }
            return SFX_ITEM_PRESENTATION_COMPLETE;
        }
    }
#endif
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SvStream& SvxULSpaceItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    rStrm << GetUpper()
          << GetPropUpper()
          << GetLower()
          << GetPropLower();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxULSpaceItem::Create( SvStream& rStrm, sal_uInt16 nVersion ) const
{
    sal_uInt16 upper, lower, nPL = 0, nPU = 0;

    if ( nVersion == ULSPACE_16_VERSION )
        rStrm >> upper >> nPU >> lower >> nPL;
    else
    {
        sal_Int8 nU, nL;
        rStrm >> upper >> nU >> lower >> nL;
        nPL = (sal_uInt16)nL;
        nPU = (sal_uInt16)nU;
    }

    SvxULSpaceItem* pAttr = new SvxULSpaceItem( Which() );
    pAttr->SetUpperValue( upper );
    pAttr->SetLowerValue( lower );
    pAttr->SetPropUpper( nPU );
    pAttr->SetPropLower( nPL );
    return pAttr;
}

// -----------------------------------------------------------------------

sal_uInt16 SvxULSpaceItem::GetVersion( sal_uInt16 nFileVersion ) const
{
    return ULSPACE_16_VERSION;
}

// -----------------------------------------------------------------------

int SvxULSpaceItem::ScaleMetrics( long nMult, long nDiv )
{
    nUpper = (sal_uInt16)Scale( nUpper, nMult, nDiv );
    nLower = (sal_uInt16)Scale( nLower, nMult, nDiv );
    return 1;
}

// -----------------------------------------------------------------------

int SvxULSpaceItem::HasMetrics() const
{
    return 1;
}


// class SvxPrintItem ----------------------------------------------------

SfxPoolItem* SvxPrintItem::Clone( SfxItemPool* ) const
{
    return new SvxPrintItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxPrintItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    rStrm << (sal_Int8)GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxPrintItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 bIsPrint;
    rStrm >> bIsPrint;
    return new SvxPrintItem( Which(), sal_Bool( bIsPrint != 0 ) );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxPrintItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;

        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            sal_uInt16 nId = RID_SVXITEMS_PRINT_FALSE;

            if ( GetValue() )
                nId = RID_SVXITEMS_PRINT_TRUE;
            rText = SVX_RESSTR(nId);
            return ePres;
        }
    }
#endif // !SVX_LIGHT
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool SvxPrintItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    sal_Bool bValue;
    sal_Bool bOk = rUnitConverter.convertBool( bValue, rValue );
    SetValue( bValue );
    return bOk;
#else
    return sal_False;
#endif
}

sal_Bool SvxPrintItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    OUStringBuffer aOut;
    rUnitConverter.convertBool( aOut, GetValue() );
    rValue = aOut.makeStringAndClear();
    return sal_True;
#else
    return sal_False;
#endif
}

// class SvxOpaqueItem ---------------------------------------------------

SfxPoolItem* SvxOpaqueItem::Clone( SfxItemPool* ) const
{
    return new SvxOpaqueItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxOpaqueItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    rStrm << (sal_Int8)GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxOpaqueItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 bIsOpaque;
    rStrm >> bIsOpaque;
    return new SvxOpaqueItem( Which(), sal_Bool( bIsOpaque != 0 ) );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxOpaqueItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;

        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            sal_uInt16 nId = RID_SVXITEMS_OPAQUE_FALSE;

            if ( GetValue() )
                nId = RID_SVXITEMS_OPAQUE_TRUE;
            rText = SVX_RESSTR(nId);
            return ePres;
        }
    }
#endif // !SVX_LIGHT
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool SvxOpaqueItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    SetValue( rValue.compareToAscii(sXML_opaque_foreground ) );
    return GetValue() || rValue.compareToAscii( sXML_opaque_background );
#else
    return sal_False;
#endif
}

sal_Bool SvxOpaqueItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    if(GetValue())
    {
        rValue = OUString( RTL_CONSTASCII_USTRINGPARAM(sXML_opaque_foreground) );
    }
    else
    {
        rValue = OUString( RTL_CONSTASCII_USTRINGPARAM(sXML_opaque_background) );
    }
    return sal_True;
#else
    return sal_False;
#endif
}

// class SvxProtectItem --------------------------------------------------

int SvxProtectItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return ( bCntnt == ( (SvxProtectItem&)rAttr ).bCntnt &&
             bSize  == ( (SvxProtectItem&)rAttr ).bSize  &&
             bPos   == ( (SvxProtectItem&)rAttr ).bPos );
}

/*-----------------16.03.98 12:42-------------------
--------------------------------------------------*/
sal_Bool SvxProtectItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    sal_Bool bValue;
    switch(nMemberId)
    {
        case MID_PROTECT_CONTENT :  bValue = bCntnt; break;
        case MID_PROTECT_SIZE    :  bValue = bSize; break;
        case MID_PROTECT_POSITION:  bValue = bPos; break;
        default:
            DBG_ERROR("falsche MemberId");
            return sal_False;
    }

    rVal = Bool2Any( bValue );
    return sal_True;
}
/*-----------------16.03.98 12:42-------------------

--------------------------------------------------*/
sal_Bool    SvxProtectItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bVal( Any2Bool(rVal) );
    switch(nMemberId)
    {
        case MID_PROTECT_CONTENT :  bCntnt = bVal;  break;
        case MID_PROTECT_SIZE    :  bSize  = bVal;  break;
        case MID_PROTECT_POSITION:  bPos   = bVal;  break;
        default:
            DBG_ERROR("falsche MemberId");
            return sal_False;
    }
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxProtectItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    OUString sContent(sXML_protect_content, sizeof(sXML_protect_content), gsl_getSystemTextEncoding());
    OUString sSize(sXML_protect_size, sizeof(sXML_protect_size), gsl_getSystemTextEncoding());
    OUString sPosition(sXML_protect_position, sizeof(sXML_protect_position), gsl_getSystemTextEncoding());

    bCntnt = rValue.indexOf(sContent) != -1;
    bSize  = rValue.indexOf(sSize) != -1;
    bPos   = rValue.indexOf(sPosition) != -1;

    return sal_True;
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------

sal_Bool SvxProtectItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    OUStringBuffer aOut;
    sal_Bool bEmpty = sal_True;

    if( bCntnt )
    {
        aOut.appendAscii( sXML_protect_content );
        bEmpty = sal_False;
    }

    if( bSize )
    {
        if( bEmpty )
            bEmpty = sal_True;
        else
            aOut.appendAscii( " " );

        aOut.appendAscii( sXML_protect_size );
    }

    if( bPos )
    {
        if( !bEmpty )
            aOut.appendAscii( " " );

        aOut.appendAscii( sXML_protect_position );
    }

    rValue = aOut.makeStringAndClear();

    return rValue.getLength() != 0;
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxProtectItem::Clone( SfxItemPool* ) const
{
    return new SvxProtectItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxProtectItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;

        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            sal_uInt16 nId = RID_SVXITEMS_PROT_CONTENT_FALSE;

            if ( bCntnt )
                nId = RID_SVXITEMS_PROT_CONTENT_TRUE;
            rText = SVX_RESSTR(nId);
            rText += cpDelim;
            nId = RID_SVXITEMS_PROT_SIZE_FALSE;

            if ( bSize )
                nId = RID_SVXITEMS_PROT_SIZE_TRUE;
            rText += SVX_RESSTR(nId);
            rText += cpDelim;
            nId = RID_SVXITEMS_PROT_POS_FALSE;

            if ( bPos )
                nId = RID_SVXITEMS_PROT_POS_TRUE;
            rText += SVX_RESSTR(nId);
            return ePres;
        }
    }
#endif // !SVX_LIGHT
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SvStream& SvxProtectItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    sal_Int8 cProt = 0;
    if( IsPosProtected() )   cProt |= 0x01;
    if( IsSizeProtected() )  cProt |= 0x02;
    if( IsCntntProtected() ) cProt |= 0x04;
    rStrm << (sal_Int8) cProt;
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxProtectItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 cFlags;
    rStrm >> cFlags;
    SvxProtectItem* pAttr = new SvxProtectItem( Which() );
    pAttr->SetPosProtect( sal_Bool( ( cFlags & 0x01 ) != 0 ) );
    pAttr->SetSizeProtect( sal_Bool(  ( cFlags & 0x02 ) != 0 ) );
    pAttr->SetCntntProtect( sal_Bool(  ( cFlags & 0x04 ) != 0 ) );
    return pAttr;
}

// class SvxShadowItem ---------------------------------------------------

SvxShadowItem::SvxShadowItem( const USHORT nId,
                 const Color *pColor, const USHORT nW,
                 const SvxShadowLocation eLoc ) :
    SfxEnumItemInterface( nId ),
    aShadowColor(COL_GRAY),
    nWidth      ( nW ),
    eLocation   ( eLoc )
{
    if ( pColor )
        aShadowColor = *pColor;
}

// -----------------------------------------------------------------------
sal_Bool SvxShadowItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    table::ShadowFormat aShadow;
    table::ShadowLocation eSet = table::ShadowLocation_NONE;
    switch( eLocation )
    {
        case SVX_SHADOW_TOPLEFT    : eSet = table::ShadowLocation_TOP_LEFT    ; break;
        case SVX_SHADOW_TOPRIGHT   : eSet = table::ShadowLocation_TOP_RIGHT   ; break;
        case SVX_SHADOW_BOTTOMLEFT : eSet = table::ShadowLocation_BOTTOM_LEFT ; break;
        case SVX_SHADOW_BOTTOMRIGHT: eSet = table::ShadowLocation_BOTTOM_RIGHT; break;
    }
    aShadow.Location = eSet;
    aShadow.ShadowWidth =   nMemberId&CONVERT_TWIPS ? TWIP_TO_MM100(nWidth) : nWidth;
    aShadow.IsTransparent = aShadowColor.GetTransparency() > 0;
    aShadow.Color = aShadowColor.GetColor();
    rVal <<= aShadow;
    return sal_True;
}
// -----------------------------------------------------------------------
sal_Bool SvxShadowItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    table::ShadowFormat aShadow;

    if(rVal >>= aShadow)
    {
        SvxShadowLocation eSet = SVX_SHADOW_NONE;
        switch( aShadow.Location )
        {
            case table::ShadowLocation_TOP_LEFT    : eLocation = SVX_SHADOW_TOPLEFT; break;
            case table::ShadowLocation_TOP_RIGHT   : eLocation = SVX_SHADOW_TOPRIGHT; break;
            case table::ShadowLocation_BOTTOM_LEFT : eLocation = SVX_SHADOW_BOTTOMLEFT ; break;
            case table::ShadowLocation_BOTTOM_RIGHT: eLocation = SVX_SHADOW_BOTTOMRIGHT; break;
        }
        nWidth = nMemberId&CONVERT_TWIPS ? MM100_TO_TWIP(aShadow.ShadowWidth) : aShadow.ShadowWidth;
        Color aSet(aShadow.Color);
        aSet.SetTransparency(aShadow.IsTransparent ? 0xff : 0);
        aShadowColor = aSet;
        return sal_True;
    }
    return sal_False;
}

sal_Bool SvxShadowItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    sal_Bool bOk = sal_False;

    sal_Bool bColorFound = sal_False;
    sal_Bool bOffsetFound = sal_False;

    SvXMLTokenEnumerator aTokenEnum( rValue );

    Color aColor( 128,128, 128 );
    eLocation = SVX_SHADOW_BOTTOMRIGHT;

    OUString aToken;
    while( aTokenEnum.getNextToken( aToken ) )
    {
        if( aToken.compareToAscii( sXML_none ) == 0 )
        {
            eLocation = SVX_SHADOW_NONE;
            return sal_True;
        }
        else if( !bColorFound && aToken.compareToAscii( "#", 1 ) == 0 )
        {
            bOk = rUnitConverter.convertColor( aColor, aToken );
            if( !bOk )
                return sal_False;

            bColorFound = sal_True;
        }
        else if( !bOffsetFound )
        {
            sal_Int32 nX = 0, nY = 0;

            bOk = rUnitConverter.convertMeasure( nX, aToken );
            if( bOk && aTokenEnum.getNextToken( aToken ) )
                bOk = rUnitConverter.convertMeasure( nY, aToken );

            if( bOk )
            {
                if( nX < 0 )
                {
                    if( nY < 0 )
                    {
                        eLocation = SVX_SHADOW_TOPLEFT;
                    }
                    else
                    {
                        eLocation = SVX_SHADOW_TOPRIGHT;
                    }
                }
                else
                {
                    if( nY < 0 )
                    {
                        eLocation = SVX_SHADOW_BOTTOMLEFT;
                    }
                    else
                    {
                        eLocation = SVX_SHADOW_BOTTOMRIGHT;
                    }
                }

                if( nX < 0 ) nX *= -1;
                if( nY < 0 ) nY *= -1;

                nWidth = (nX + nY) >> 1;
            }
        }
    }

    if( bOk && ( bColorFound || bOffsetFound ) )
    {
        aShadowColor = aColor;
        return sal_True;
    }
    else
#endif
        return sal_False;
}

// -----------------------------------------------------------------------


sal_Bool SvxShadowItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    sal_Int32 nX = 1, nY = 1;
    switch( eLocation )
    {
    case SVX_SHADOW_TOPLEFT:
        nX = -1;
        nY = -1;
        break;
    case SVX_SHADOW_TOPRIGHT:
        nY = -1;
        break;
    case SVX_SHADOW_BOTTOMLEFT:
        nX = -1;
        break;
    case SVX_SHADOW_BOTTOMRIGHT:
        break;
    case SVX_SHADOW_NONE:
    default:
        rValue = OUString( RTL_CONSTASCII_USTRINGPARAM(sXML_none) );
        return sal_True;
    }

    nX *= nWidth;
    nY *= nWidth;

    OUStringBuffer aOut;

    rUnitConverter.convertColor( aOut, aShadowColor );
    aOut.append( sal_Unicode(' ') );
    rUnitConverter.convertMeasure( aOut, nX );
    aOut.append( sal_Unicode(' ') );
    rUnitConverter.convertMeasure( aOut, nY );

    rValue = aOut.makeStringAndClear();

    return sal_True;
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------

int SvxShadowItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return ( ( aShadowColor == ( (SvxShadowItem&)rAttr ).aShadowColor ) &&
             ( nWidth    == ( (SvxShadowItem&)rAttr ).GetWidth() ) &&
             ( eLocation == ( (SvxShadowItem&)rAttr ).GetLocation() ) );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxShadowItem::Clone( SfxItemPool* ) const
{
    return new SvxShadowItem( *this );
}

// -----------------------------------------------------------------------

sal_uInt16 SvxShadowItem::CalcShadowSpace( sal_uInt16 nShadow ) const
{
    sal_uInt16 nSpace = 0;

    switch ( nShadow )
    {
        case SHADOW_TOP:
            if ( eLocation == SVX_SHADOW_TOPLEFT ||
                 eLocation == SVX_SHADOW_TOPRIGHT  )
                nSpace = nWidth;
            break;

        case SHADOW_BOTTOM:
            if ( eLocation == SVX_SHADOW_BOTTOMLEFT ||
                 eLocation == SVX_SHADOW_BOTTOMRIGHT  )
                nSpace = nWidth;
            break;

        case SHADOW_LEFT:
            if ( eLocation == SVX_SHADOW_TOPLEFT ||
                 eLocation == SVX_SHADOW_BOTTOMLEFT )
                nSpace = nWidth;
            break;

        case SHADOW_RIGHT:
            if ( eLocation == SVX_SHADOW_TOPRIGHT ||
                 eLocation == SVX_SHADOW_BOTTOMRIGHT )
                nSpace = nWidth;
            break;

        default:
            DBG_ERROR( "wrong shadow" );
    }
    return nSpace;
}

//------------------------------------------------------------------------

SfxItemPresentation SvxShadowItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;

        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            rText = ::GetColorString( aShadowColor );
            rText += cpDelim;
            sal_uInt16 nId = RID_SVXITEMS_TRANSPARENT_FALSE;

            if ( aShadowColor.GetTransparency() )
                nId = RID_SVXITEMS_TRANSPARENT_TRUE;
            rText += SVX_RESSTR(nId);
            rText += cpDelim;
            rText += GetMetricText( (long)nWidth, eCoreUnit, ePresUnit );
            rText += cpDelim;
            rText += SVX_RESSTR(RID_SVXITEMS_SHADOW_BEGIN + eLocation);
            return ePres;
        }
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = SVX_RESSTR(RID_SVXITEMS_SHADOW_COMPLETE);
            rText += ::GetColorString( aShadowColor );
            rText += cpDelim;

            sal_uInt16 nId = RID_SVXITEMS_TRANSPARENT_FALSE;
            if ( aShadowColor.GetTransparency() )
                nId = RID_SVXITEMS_TRANSPARENT_TRUE;
            rText += SVX_RESSTR(nId);
            rText += cpDelim;
            rText += GetMetricText( (long)nWidth, eCoreUnit, ePresUnit );
            rText += SVX_RESSTR(GetMetricId(ePresUnit));
            rText += cpDelim;
            rText += SVX_RESSTR(RID_SVXITEMS_SHADOW_BEGIN + eLocation);
            return ePres;
        }
    }
#endif
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SvStream& SvxShadowItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    rStrm << (sal_Int8) GetLocation()
          << (sal_uInt16) GetWidth()
          << (sal_Bool)(aShadowColor.GetTransparency() > 0)
          << GetColor()
          << GetColor()
          << (sal_Int8)(aShadowColor.GetTransparency() > 0 ? 0 : 1); //BRUSH_NULL : BRUSH_SOLID
    return rStrm;
}

// -----------------------------------------------------------------------

int SvxShadowItem::ScaleMetrics( long nMult, long nDiv )
{
    nWidth = (sal_uInt16)Scale( nWidth, nMult, nDiv );
    return 1;
}

// -----------------------------------------------------------------------

int SvxShadowItem::HasMetrics() const
{
    return 1;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxShadowItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 cLoc;
    sal_uInt16 nWidth;
    sal_Bool bTrans;
    Color aColor;
    Color aFillColor;
    sal_Int8 nStyle;
    rStrm >> cLoc >> nWidth
          >> bTrans >> aColor >> aFillColor >> nStyle;
    aColor.SetTransparency(bTrans ? 0xff : 0);
    return new SvxShadowItem( Which(), &aColor, nWidth, (SvxShadowLocation)cLoc );
}

// -----------------------------------------------------------------------

sal_uInt16 SvxShadowItem::GetValueCount() const
{
    return SVX_SHADOW_END;  // SVX_SHADOW_BOTTOMRIGHT + 1
}

// -----------------------------------------------------------------------

XubString SvxShadowItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos < SVX_SHADOW_END, "enum overflow!" );
    return XubString( SVX_RES( RID_SVXITEMS_SHADOW_BEGIN + nPos ) );
}

// -----------------------------------------------------------------------

sal_uInt16 SvxShadowItem::GetEnumValue() const
{
    return GetLocation();
}

// -----------------------------------------------------------------------

void SvxShadowItem::SetEnumValue( sal_uInt16 nVal )
{
    SetLocation( (const SvxShadowLocation)nVal );
}

// class SvxBorderLine  --------------------------------------------------

SvxBorderLine::SvxBorderLine( const Color *pCol, sal_uInt16 nOut, sal_uInt16 nIn,
                              sal_uInt16 nDist ) :

    nOutWidth( nOut ),
    nInWidth ( nIn ),
    nDistance( nDist )

{
    if ( pCol )
        aColor = *pCol;
}

// -----------------------------------------------------------------------

void SvxBorderLine::ScaleMetrics( long nMult, long nDiv )
{
    nOutWidth = (sal_uInt16)Scale( nOutWidth, nMult, nDiv );
    nInWidth = (sal_uInt16)Scale( nInWidth, nMult, nDiv );
    nDistance = (sal_uInt16)Scale( nDistance, nMult, nDiv );
}

// -----------------------------------------------------------------------

sal_Bool SvxBorderLine::operator==( const SvxBorderLine& rCmp ) const
{
    return ( ( aColor    == rCmp.GetColor() )    &&
             ( nInWidth  == rCmp.GetInWidth() )  &&
             ( nOutWidth == rCmp.GetOutWidth() ) &&
             ( nDistance == rCmp.GetDistance() ) );
}

// -----------------------------------------------------------------------

XubString SvxBorderLine::GetValueString( SfxMapUnit eSrcUnit,
                                      SfxMapUnit eDestUnit,
                                      sal_Bool bMetricStr) const
{
#ifndef SVX_LIGHT
    sal_uInt16 nResId = 0;

    if ( 0 == nDistance )
    {
        // einfach Linie
        if ( DEF_LINE_WIDTH_0 == nOutWidth )
            nResId = RID_SINGLE_LINE0;
        else if ( DEF_LINE_WIDTH_1 == nOutWidth )
            nResId = RID_SINGLE_LINE1;
        else if ( DEF_LINE_WIDTH_2 == nOutWidth )
            nResId = RID_SINGLE_LINE2;
        else if ( DEF_LINE_WIDTH_3 == nOutWidth )
            nResId = RID_SINGLE_LINE3;
        else if ( DEF_LINE_WIDTH_4 == nOutWidth )
            nResId = RID_SINGLE_LINE4;
    }
    else if ( DEF_LINE_WIDTH_1 == nDistance )
    {
        // doppelte Linie, kleiner Abstand
        if ( DEF_LINE_WIDTH_0 == nOutWidth && DEF_LINE_WIDTH_0 == nInWidth )
            nResId = RID_DOUBLE_LINE0;
        else if ( DEF_LINE_WIDTH_1 == nOutWidth &&
                  DEF_LINE_WIDTH_1 == nInWidth )
            nResId = RID_DOUBLE_LINE2;
        else if ( DEF_LINE_WIDTH_1 == nOutWidth &&
                  DEF_LINE_WIDTH_2 == nInWidth )
            nResId = RID_DOUBLE_LINE8;
    }
    else if ( DEF_LINE_WIDTH_2 == nDistance )
    {
        // doppelte Linie, gro\ser Abstand
        if ( DEF_LINE_WIDTH_0 == nOutWidth && DEF_LINE_WIDTH_0 == nInWidth )
            nResId = RID_DOUBLE_LINE1;
        else if ( DEF_LINE_WIDTH_2 == nOutWidth &&
                  DEF_LINE_WIDTH_2 == nInWidth )
            nResId = RID_DOUBLE_LINE3;
        else if ( DEF_LINE_WIDTH_1 == nOutWidth &&
                  DEF_LINE_WIDTH_0 == nInWidth )
            nResId = RID_DOUBLE_LINE4;
        else if ( DEF_LINE_WIDTH_2 == nOutWidth &&
                  DEF_LINE_WIDTH_0 == nInWidth )
            nResId = RID_DOUBLE_LINE5;
        else if ( DEF_LINE_WIDTH_3 == nOutWidth &&
                  DEF_LINE_WIDTH_0 == nInWidth )
            nResId = RID_DOUBLE_LINE6;
        else if ( DEF_LINE_WIDTH_2 == nOutWidth &&
                  DEF_LINE_WIDTH_1 == nInWidth )
            nResId = RID_DOUBLE_LINE7;
        else if ( DEF_LINE_WIDTH_3 == nOutWidth &&
                  DEF_LINE_WIDTH_2 == nInWidth )
            nResId = RID_DOUBLE_LINE9;
        else if ( DEF_LINE_WIDTH_2 == nOutWidth &&
                  DEF_LINE_WIDTH_3 == nInWidth )
            nResId = RID_DOUBLE_LINE10;
    }
    String aStr;
    aStr += sal_Unicode('(');
    aStr += ::GetColorString( aColor );
    aStr += cpDelim;

    if ( nResId )
        aStr += SVX_RESSTR(nResId);
    else
    {
        String sMetric = SVX_RESSTR(GetMetricId( eDestUnit ));
        aStr += GetMetricText( (long)nInWidth, eSrcUnit, eDestUnit );
        if ( bMetricStr )
            aStr += sMetric;
        aStr += cpDelim;
        aStr += GetMetricText( (long)nOutWidth, eSrcUnit, eDestUnit );
        if ( bMetricStr )
            aStr += sMetric;
        aStr += cpDelim;
        aStr += GetMetricText( (long)nDistance, eSrcUnit, eDestUnit );
        if ( bMetricStr )
            aStr += sMetric;
    }
    aStr += sal_Unicode(')');
    return aStr;
#else
    return UniString();
#endif
}

// class SvxBoxItem ------------------------------------------------------

SvxBoxItem::SvxBoxItem( const SvxBoxItem& rCpy ) :

    SfxPoolItem ( rCpy ),
    nTopDist    ( rCpy.nTopDist ),
    nBottomDist ( rCpy.nBottomDist ),
    nLeftDist   ( rCpy.nLeftDist ),
    nRightDist  ( rCpy.nRightDist )

{
    pTop    = rCpy.GetTop()     ? new SvxBorderLine( *rCpy.GetTop() )    : 0;
    pBottom = rCpy.GetBottom()  ? new SvxBorderLine( *rCpy.GetBottom() ) : 0;
    pLeft   = rCpy.GetLeft()    ? new SvxBorderLine( *rCpy.GetLeft() )   : 0;
    pRight  = rCpy.GetRight()   ? new SvxBorderLine( *rCpy.GetRight() )  : 0;
}

// -----------------------------------------------------------------------

SvxBoxItem::SvxBoxItem( const sal_uInt16 nId ) :
    SfxPoolItem( nId ),

    pTop        ( 0 ),
    pBottom     ( 0 ),
    pLeft       ( 0 ),
    pRight      ( 0 ),
    nTopDist    ( 0 ),
    nBottomDist ( 0 ),
    nLeftDist   ( 0 ),
    nRightDist  ( 0 )

{
}

// -----------------------------------------------------------------------

SvxBoxItem::~SvxBoxItem()
{
    delete pTop;
    delete pBottom;
    delete pLeft;
    delete pRight;
}

// -----------------------------------------------------------------------

SvxBoxItem& SvxBoxItem::operator=( const SvxBoxItem& rBox )
{
    nTopDist = rBox.nTopDist;
    nBottomDist = rBox.nBottomDist;
    nLeftDist = rBox.nLeftDist;
    nRightDist = rBox.nRightDist;
    SetLine( rBox.GetTop(), BOX_LINE_TOP );
    SetLine( rBox.GetBottom(), BOX_LINE_BOTTOM );
    SetLine( rBox.GetLeft(), BOX_LINE_LEFT );
    SetLine( rBox.GetRight(), BOX_LINE_RIGHT );
    return *this;
}

// -----------------------------------------------------------------------

inline sal_Bool CmpBrdLn( const SvxBorderLine* pBrd1, const SvxBorderLine* pBrd2 )
{
    sal_Bool bRet;
    if( 0 != pBrd1 ?  0 == pBrd2 : 0 != pBrd2 )
        bRet = sal_False;
    else
        if( !pBrd1 )
            bRet = sal_True;
        else
            bRet = (*pBrd1 == *pBrd2);
    return bRet;
}

// -----------------------------------------------------------------------

int SvxBoxItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return (
        ( nTopDist == ( (SvxBoxItem&)rAttr ).nTopDist ) &&
        ( nBottomDist == ( (SvxBoxItem&)rAttr ).nBottomDist )   &&
        ( nLeftDist == ( (SvxBoxItem&)rAttr ).nLeftDist )   &&
        ( nRightDist == ( (SvxBoxItem&)rAttr ).nRightDist ) &&
        CmpBrdLn( pTop, ( (SvxBoxItem&)rAttr ).GetTop() )           &&
        CmpBrdLn( pBottom, ( (SvxBoxItem&)rAttr ).GetBottom() )     &&
        CmpBrdLn( pLeft, ( (SvxBoxItem&)rAttr ).GetLeft() )         &&
        CmpBrdLn( pRight, ( (SvxBoxItem&)rAttr ).GetRight() ) );
}

// -----------------------------------------------------------------------
table::BorderLine lcl_SvxLineToLine(const SvxBorderLine* pLine, sal_Bool bConvert)
{
    table::BorderLine aLine;
    if(pLine)
    {
        aLine.Color          = pLine->GetColor().GetColor() ;
        aLine.InnerLineWidth = bConvert ? TWIP_TO_MM100(pLine->GetInWidth() ): pLine->GetInWidth()  ;
        aLine.OuterLineWidth = bConvert ? TWIP_TO_MM100(pLine->GetOutWidth()): pLine->GetOutWidth() ;
        aLine.LineDistance   = bConvert ? TWIP_TO_MM100(pLine->GetDistance()): pLine->GetDistance() ;
    }
    else
        aLine.Color          = aLine.InnerLineWidth = aLine.OuterLineWidth = aLine.LineDistance  = 0;
    return aLine;
}
// -----------------------------------------------------------------------
sal_Bool SvxBoxItem::QueryValue( uno::Any& rVal, BYTE nMemberId  ) const
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    table::BorderLine aRetLine;
    sal_uInt16 nDist;
    sal_Bool bDistMember = sal_False;
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case LEFT_BORDER:
            aRetLine = lcl_SvxLineToLine(GetLeft(), bConvert);
            break;
        case RIGHT_BORDER:
            aRetLine = lcl_SvxLineToLine(GetRight(), bConvert);
            break;
        case BOTTOM_BORDER:
            aRetLine = lcl_SvxLineToLine(GetBottom(), bConvert);
            break;
        case TOP_BORDER:
            aRetLine = lcl_SvxLineToLine(GetTop(), bConvert);
            break;
        case BORDER_DISTANCE:
            nDist = GetDistance();
            bDistMember = sal_True;
            break;
        case TOP_BORDER_DISTANCE:
            nDist = nTopDist;
            bDistMember = sal_True;
            break;
        case BOTTOM_BORDER_DISTANCE:
            nDist = nBottomDist;
            bDistMember = sal_True;
            break;
        case LEFT_BORDER_DISTANCE:
            nDist = nLeftDist;
            bDistMember = sal_True;
            break;
        case RIGHT_BORDER_DISTANCE:
            nDist = nRightDist;
            bDistMember = sal_True;
            break;
    }

    if( bDistMember )
        rVal <<= (sal_Int32)(bConvert ? TWIP_TO_MM100(nDist) : nDist);
    else
        rVal <<= aRetLine;

    return sal_True;
}

// -----------------------------------------------------------------------
sal_Bool lcl_LineToSvxLine(const ::com::sun::star::table::BorderLine& rLine, SvxBorderLine& rSvxLine, sal_Bool bConvert)
{
    rSvxLine.SetColor(   Color(rLine.Color));
    rSvxLine.SetInWidth( bConvert ? MM100_TO_TWIP(rLine.InnerLineWidth) : rLine.InnerLineWidth  );
    rSvxLine.SetOutWidth(bConvert ? MM100_TO_TWIP(rLine.OuterLineWidth) : rLine.OuterLineWidth  );
    rSvxLine.SetDistance(bConvert ? MM100_TO_TWIP(rLine.LineDistance    )  : rLine.LineDistance  );
    sal_Bool bRet = rLine.InnerLineWidth > 0 || rLine.OuterLineWidth > 0;
    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool SvxBoxItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    sal_uInt16 nLine = BOX_LINE_TOP;
    sal_Bool bDistMember = sal_False;
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case LEFT_BORDER_DISTANCE:
            bDistMember = sal_True;
        case LEFT_BORDER:
            nLine = BOX_LINE_LEFT;
            break;
        case RIGHT_BORDER_DISTANCE:
            bDistMember = sal_True;
        case RIGHT_BORDER:
            nLine = BOX_LINE_RIGHT;
            break;
        case BOTTOM_BORDER_DISTANCE:
            bDistMember = sal_True;
        case BOTTOM_BORDER:
            nLine = BOX_LINE_BOTTOM;
            break;
        case TOP_BORDER_DISTANCE:
            bDistMember = sal_True;
        case TOP_BORDER:
            nLine = BOX_LINE_TOP;
            break;
    }

    if( bDistMember || nMemberId == BORDER_DISTANCE )
    {
        sal_Int32 nDist;
        if(!(rVal >>= nDist))
            return sal_False;

        if(nDist >= 0)
        {
            if( bConvert )
                nDist = MM100_TO_TWIP(nDist);
            if( nMemberId == BORDER_DISTANCE )
                SetDistance( nDist );
            else
                SetDistance( nDist, nLine );
        }
    }
    else
    {
        SvxBorderLine aLine;
        if( !rVal.hasValue() || rVal.getValueType() != ::getCppuType((const ::com::sun::star::table::BorderLine*)0) )
            return sal_False;

        table::BorderLine* pLine = (table::BorderLine*)rVal.getValue();
        sal_Bool bSet = lcl_LineToSvxLine(*pLine, aLine, bConvert);
        SetLine(bSet ? &aLine : 0, nLine);
    }

    return sal_True;
}

// -----------------------------------------------------------------------

#ifndef SVX_LIGHT

#define SVX_XML_BORDER_STYLE_NONE 0
#define SVX_XML_BORDER_STYLE_SOLID 1
#define SVX_XML_BORDER_STYLE_DOUBLE 2

#define SVX_XML_BORDER_WIDTH_THIN 0
#define SVX_XML_BORDER_WIDTH_MIDDLE 1
#define SVX_XML_BORDER_WIDTH_THICK 2


SvXMLEnumMapEntry psXML_BorderStyles[] =
{
    { sXML_none,                SVX_XML_BORDER_STYLE_NONE   },
    { sXML_hidden,              SVX_XML_BORDER_STYLE_NONE   },
    { sXML_solid,               SVX_XML_BORDER_STYLE_SOLID  },
    { sXML_double,              SVX_XML_BORDER_STYLE_DOUBLE },
    { sXML_dotted,              SVX_XML_BORDER_STYLE_SOLID  },
    { sXML_dashed,              SVX_XML_BORDER_STYLE_SOLID  },
    { sXML_groove,              SVX_XML_BORDER_STYLE_SOLID  },
    { sXML_ridge,               SVX_XML_BORDER_STYLE_SOLID  },
    { sXML_inset,               SVX_XML_BORDER_STYLE_SOLID  },
    { sXML_outset,              SVX_XML_BORDER_STYLE_SOLID  },
    { 0,        0 }
};

SvXMLEnumMapEntry psXML_NamedBorderWidths[] =
{
    { sXML_thin,    SVX_XML_BORDER_WIDTH_THIN   },
    { sXML_middle,  SVX_XML_BORDER_WIDTH_MIDDLE },
    { sXML_thick,   SVX_XML_BORDER_WIDTH_THICK  },
    { 0,        0 }
};
// mapping tables to map external xml input to intarnal box line widths

// Ein Eintrag besteht aus vier USHORTs. Der erste ist die Gesamtbreite,
// die anderen sind die 3 Einzelbreiten

#define SBORDER_ENTRY( n ) \
    DEF_LINE_WIDTH_##n, DEF_LINE_WIDTH_##n, 0, 0

#define DBORDER_ENTRY( n ) \
    DEF_DOUBLE_LINE##n##_OUT + DEF_DOUBLE_LINE##n##_IN + \
    DEF_DOUBLE_LINE##n##_DIST, \
    DEF_DOUBLE_LINE##n##_OUT, \
    DEF_DOUBLE_LINE##n##_IN, \
    DEF_DOUBLE_LINE##n##_DIST

#define TDBORDER_ENTRY( n ) \
    DEF_DOUBLE_LINE##n##_OUT, \
    DEF_DOUBLE_LINE##n##_OUT, \
    DEF_DOUBLE_LINE##n##_IN, \
    DEF_DOUBLE_LINE##n##_DIST


static sal_uInt16 __READONLY_DATA aSBorderWidths[] =
{
    SBORDER_ENTRY( 0 ), SBORDER_ENTRY( 1 ), SBORDER_ENTRY( 2 ),
    SBORDER_ENTRY( 3 ), SBORDER_ENTRY( 4 )
};

static sal_uInt16 __READONLY_DATA aDBorderWidths[] =
{
    DBORDER_ENTRY( 0 ),
    DBORDER_ENTRY( 7 ),
    DBORDER_ENTRY( 1 ),
    DBORDER_ENTRY( 8 ),
    DBORDER_ENTRY( 4 ),
    DBORDER_ENTRY( 9 ),
    DBORDER_ENTRY( 3 ),
    DBORDER_ENTRY( 10 ),
    DBORDER_ENTRY( 2 ),
    DBORDER_ENTRY( 6 ),
    DBORDER_ENTRY( 5 )
};

sal_Bool lcl_frmitems_parseXMLBorder( const OUString& rValue,
                                    const SvXMLUnitConverter& rUnitConverter,
                                  sal_Bool& rHasStyle, sal_uInt16& rStyle,
                                  sal_Bool& rHasWidth, sal_uInt16& rWidth,
                                                     sal_uInt16& rNamedWidth,
                                  sal_Bool& rHasColor, Color& rColor )
{
    OUString aToken;
    SvXMLTokenEnumerator aTokens( rValue );

    rHasStyle = sal_False;
    rHasWidth = sal_False;
    rHasColor = sal_False;

    rStyle = USHRT_MAX;
    rWidth = 0;
    rNamedWidth = USHRT_MAX;

    long nTemp;
    while( aTokens.getNextToken( aToken ) && aToken.getLength() != 0 )
    {
        if( !rHasWidth &&
            rUnitConverter.convertEnum( rNamedWidth, aToken,
                                        psXML_NamedBorderWidths ) )
        {
            rHasWidth = sal_True;
        }
        else if( !rHasStyle &&
                 rUnitConverter.convertEnum( rStyle, aToken,
                                              psXML_BorderStyles ) )
        {
            rHasStyle = sal_True;
        }
        else if( !rHasColor && rUnitConverter.convertColor( rColor, aToken ) )
        {
            rHasColor = sal_True;
        }
        else if( !rHasWidth &&
                 rUnitConverter.convertMeasure( nTemp, aToken, 0,
                                                 USHRT_MAX ) )
        {
            rWidth = (sal_uInt16)nTemp;
            rHasWidth = sal_True;
        }
        else
        {
            // missformed
            return sal_False;
        }
    }

    return rHasStyle || rHasWidth || rHasColor;
}

void lcl_frmitems_setXMLBorderWidth( SvxBorderLine& rLine,
                                     sal_uInt16 nOutWidth, sal_uInt16 nInWidth,
                                     sal_uInt16 nDistance )
{
    rLine.SetOutWidth( nOutWidth );
    rLine.SetInWidth( nInWidth );
    rLine.SetDistance( nDistance );
}

void lcl_frmitems_setXMLBorderWidth( SvxBorderLine& rLine,
                                     sal_uInt16 nWidth, sal_Bool bDouble )
{
    const sal_uInt16 *aWidths;
    sal_uInt16 nSize;
    if( !bDouble )
    {
        aWidths = aSBorderWidths;
        nSize = sizeof( aSBorderWidths );
    }
    else
    {
        aWidths = aDBorderWidths;
        nSize = sizeof( aDBorderWidths );
    }

    sal_uInt16 i = (nSize / sizeof(sal_uInt16)) - 4;
    while( i>0 &&
           nWidth <= ((aWidths[i] + aWidths[i-4]) / 2)  )
    {
        DBG_ASSERT( aWidths[i] >= aWidths[i-4],
                "line widths are unordered!" );
        i -= 4;
    }

    rLine.SetOutWidth( aWidths[i+1] );
    rLine.SetInWidth( aWidths[i+2] );
    rLine.SetDistance( aWidths[i+3] );
}

sal_Bool lcl_frmitems_setXMLBorder( SvxBorderLine*& rpLine,
                                sal_Bool bHasStyle, sal_uInt16 nStyle,
                                sal_Bool bHasWidth, sal_uInt16 nWidth,
                                                sal_uInt16 nNamedWidth,
                                sal_Bool bHasColor, const Color& rColor )
{
    // first of all, delete an empty line
    if( (bHasStyle && SVX_XML_BORDER_STYLE_NONE == nStyle) ||
        (bHasWidth && USHRT_MAX == nNamedWidth && 0 == nWidth) )
    {
        sal_Bool bRet = 0 != rpLine;
        if( rpLine )
        {
            delete rpLine;
            rpLine = 0;
        }

        return bRet;
    }

    // if there is no line and no style and no with, there will never be a line
    if( !rpLine && !(bHasStyle && bHasWidth) )
        return sal_False;

    // We now do know that there will be a line
    if( !rpLine )
        rpLine = new SvxBorderLine;

    if( ( bHasWidth &&
          (USHRT_MAX != nNamedWidth || (nWidth != rpLine->GetOutWidth() +
                                                     rpLine->GetInWidth() +
                                                  rpLine->GetDistance()) ) ) ||
        ( bHasStyle &&
          ((SVX_XML_BORDER_STYLE_SOLID == nStyle && rpLine->GetDistance()) ||
           (SVX_XML_BORDER_STYLE_DOUBLE == nStyle && !rpLine->GetDistance())) ))
    {
        sal_Bool bDouble = (bHasWidth && SVX_XML_BORDER_STYLE_DOUBLE == nStyle) ||
                       rpLine->GetDistance();

        // The width has to be changed
        if( bHasWidth && USHRT_MAX != nNamedWidth )
        {
            const sal_uInt16 *aWidths = bDouble ? aDBorderWidths
                                            : aSBorderWidths;
            sal_uInt16 nNWidth = nNamedWidth * 4;
            rpLine->SetOutWidth( aWidths[nNWidth+1] );
            rpLine->SetInWidth( aWidths[nNWidth+2] );
            rpLine->SetDistance( aWidths[nNWidth+3] );

        }
        else
        {
            if( !bHasWidth )
                nWidth = rpLine->GetInWidth() + rpLine->GetDistance() +
                         rpLine->GetOutWidth();

            lcl_frmitems_setXMLBorderWidth( *rpLine, nWidth, bDouble );
        }
    }

    // set color
    if( bHasColor )
        rpLine->SetColor( rColor );

    return sal_True;
}

void lcl_frmitems_setXMLBorder( SvxBorderLine*& rpLine,
                                sal_uInt16 nWidth, sal_uInt16 nOutWidth,
                                sal_uInt16 nInWidth, sal_uInt16 nDistance )
{
    if( !rpLine )
        rpLine = new SvxBorderLine;

    if( nWidth > 0 )
        lcl_frmitems_setXMLBorderWidth( *rpLine, nWidth, sal_True );
    else
        lcl_frmitems_setXMLBorderWidth( *rpLine, nOutWidth, nInWidth,
                                        nDistance );
}
#endif

sal_Bool SvxBoxItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT

    sal_Int32 nTemp;

    switch( nMemberId )
    {
    case ALL_BORDER_PADDING:
    case LEFT_BORDER_PADDING:
    case RIGHT_BORDER_PADDING:
    case TOP_BORDER_PADDING:
    case BOTTOM_BORDER_PADDING:
        if(!rUnitConverter.convertMeasure( nTemp, rValue, 0, 0xffff ))
            return sal_False;

        if( nMemberId == LEFT_BORDER_PADDING ||
            nMemberId == ALL_BORDER_PADDING )
            nLeftDist = (sal_uInt16)nTemp;
        if( nMemberId == RIGHT_BORDER_PADDING ||
            nMemberId == ALL_BORDER_PADDING )
            nRightDist = (sal_uInt16)nTemp;
        if( nMemberId == TOP_BORDER_PADDING ||
            nMemberId == ALL_BORDER_PADDING )
            nTopDist = (sal_uInt16)nTemp;
        if( nMemberId == BOTTOM_BORDER_PADDING ||
            nMemberId == ALL_BORDER_PADDING )
            nBottomDist = (sal_uInt16)nTemp;
        break;

    case ALL_BORDER:
    case LEFT_BORDER:
    case RIGHT_BORDER:
    case TOP_BORDER:
    case BOTTOM_BORDER:
        {
            sal_Bool bHasStyle = sal_False, bHasWidth = sal_False, bHasColor = sal_False;
            sal_uInt16 nStyle = USHRT_MAX, nWidth = 0, nNamedWidth = USHRT_MAX;
            Color aColor( COL_BLACK );

            if( !lcl_frmitems_parseXMLBorder( rValue, rUnitConverter,
                                              bHasStyle, nStyle,
                                              bHasWidth, nWidth, nNamedWidth,
                                              bHasColor, aColor ) )
                return sal_False;

            if( TOP_BORDER == nMemberId || ALL_BORDER == nMemberId )
                lcl_frmitems_setXMLBorder( pTop,
                                           bHasStyle, nStyle,
                                           bHasWidth, nWidth, nNamedWidth,
                                           bHasColor, aColor );

            if( BOTTOM_BORDER == nMemberId || ALL_BORDER == nMemberId )
                lcl_frmitems_setXMLBorder( pBottom,
                                           bHasStyle, nStyle,
                                           bHasWidth, nWidth, nNamedWidth,
                                           bHasColor, aColor );

            if( LEFT_BORDER == nMemberId || ALL_BORDER == nMemberId )
                lcl_frmitems_setXMLBorder( pLeft,
                                           bHasStyle, nStyle,
                                           bHasWidth, nWidth, nNamedWidth,
                                           bHasColor, aColor );

            if( RIGHT_BORDER == nMemberId || ALL_BORDER == nMemberId )
                lcl_frmitems_setXMLBorder( pRight,
                                           bHasStyle, nStyle,
                                           bHasWidth, nWidth, nNamedWidth,
                                           bHasColor, aColor );
        }
        break;
    case ALL_BORDER_LINE_WIDTH:
    case LEFT_BORDER_LINE_WIDTH:
    case RIGHT_BORDER_LINE_WIDTH:
    case TOP_BORDER_LINE_WIDTH:
    case BOTTOM_BORDER_LINE_WIDTH:
        {
            SvXMLTokenEnumerator aTokenEnum( rValue );

            sal_Int32 nInWidth, nDistance, nOutWidth;

            OUString aToken;
            if( !aTokenEnum.getNextToken( aToken ) )
                return sal_False;

            if( !rUnitConverter.convertMeasure( nInWidth, aToken,
                                                DEF_LINE_WIDTH_0,
                                                DEF_LINE_WIDTH_4 ) )
                return sal_False;

            if( !aTokenEnum.getNextToken( aToken ) )
                return sal_False;

            if( !rUnitConverter.convertMeasure( nDistance, aToken,
                                                DEF_LINE_WIDTH_0,
                                                DEF_LINE_WIDTH_4 ) )
                return sal_False;

            if( !aTokenEnum.getNextToken( aToken ) )
                return sal_False;

            if( !rUnitConverter.convertMeasure( nOutWidth, aToken,
                                                DEF_LINE_WIDTH_0,
                                                DEF_LINE_WIDTH_4 ) )
                return sal_False;

            sal_uInt16 nSize = sizeof( aDBorderWidths );
            for( sal_uInt16 i=0; i < nSize; i += 4 )
            {
                if( aDBorderWidths[i+1] == nOutWidth &&
                    aDBorderWidths[i+2] == nInWidth &&
                    aDBorderWidths[i+3] == nDistance )
                    break;
            }

            sal_uInt16 nWidth = i < nSize ? 0 : nOutWidth + nInWidth + nDistance;

            if( TOP_BORDER_LINE_WIDTH == nMemberId ||
                ALL_BORDER_LINE_WIDTH == nMemberId )
                lcl_frmitems_setXMLBorder( pTop, nWidth, nOutWidth,
                                           nInWidth, nDistance );

            if( BOTTOM_BORDER_LINE_WIDTH == nMemberId ||
                ALL_BORDER_LINE_WIDTH == nMemberId )
                lcl_frmitems_setXMLBorder( pBottom, nWidth, nOutWidth,
                                           nInWidth, nDistance );

            if( LEFT_BORDER_LINE_WIDTH == nMemberId ||
                ALL_BORDER_LINE_WIDTH == nMemberId )
                lcl_frmitems_setXMLBorder( pLeft, nWidth, nOutWidth,
                                           nInWidth, nDistance );

            if( RIGHT_BORDER_LINE_WIDTH == nMemberId ||
                ALL_BORDER_LINE_WIDTH == nMemberId )
                lcl_frmitems_setXMLBorder( pRight, nWidth, nOutWidth,
                                           nInWidth, nDistance );
        }
        break;
    }

    return sal_True;
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------

/**
    xml -> MemberId

    border-padding          ALL_BORDER_PADDING
    border-padding-before   LEFT_BORDER_PADDING
    border-padding-after    RIGHT_BORDER_PADDING
    border-padding-start    TOP_BORDER_PADDING
    border-padding-end      BOTTOM_BORDER_PADDING

    border                  ALL_BORDER
    border-before           LEFT_BORDER
    border-after            RIGHT_BORDER
    border-start            TOP_BORDER
    border-end              BOTTOM_BORDER

    border-line-width           ALL_BORDER_LINE_WIDTH
    border-line-width-before    LEFT_BORDER_LINE_WIDTH
    border-line-width-after     RIGHT_BORDER_LINE_WIDTH
    border-line-width-start     TOP_BORDER_LINE_WIDTH
    border-line-width-end       BOTTOM_BORDER_LINE_WIDTH
*/
sal_Bool SvxBoxItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT

    OUStringBuffer aOut;

    // check if we need to export it
    switch( nMemberId )
    {
    case ALL_BORDER_PADDING:
    case LEFT_BORDER_PADDING:
    case RIGHT_BORDER_PADDING:
    case TOP_BORDER_PADDING:
    case BOTTOM_BORDER_PADDING:
        {
            sal_Bool bEqual = nLeftDist == nRightDist &&
                          nLeftDist == nTopDist &&
                          nLeftDist == nBottomDist;
            // don't export individual paddings if all paddings are equal and
            // don't export all padding if some paddings are not equal
            if( (bEqual && ALL_BORDER_PADDING != nMemberId) ||
                (!bEqual && ALL_BORDER_PADDING == nMemberId) )
                return sal_False;
        }
        break;
    case ALL_BORDER:
    case LEFT_BORDER:
    case RIGHT_BORDER:
    case TOP_BORDER:
    case BOTTOM_BORDER:
        {
            sal_Bool bEqual = ( NULL == pTop && NULL == pBottom &&
                            NULL == pLeft && NULL == pRight ) ||
                          ( pTop && pBottom && pLeft && pRight &&
                            *pTop == *pBottom  && *pTop == *pLeft &&
                            *pTop == *pRight );

            // don't export individual borders if all are the same and
            // don't export all borders if some are not equal
            if( (bEqual && ALL_BORDER != nMemberId) ||
                (!bEqual && ALL_BORDER == nMemberId) )
                return sal_False;
        }
        break;
    case ALL_BORDER_LINE_WIDTH:
    case LEFT_BORDER_LINE_WIDTH:
    case RIGHT_BORDER_LINE_WIDTH:
    case TOP_BORDER_LINE_WIDTH:
    case BOTTOM_BORDER_LINE_WIDTH:
        {
            // if no line is set, there is nothing to export
            if( !pTop && !pBottom && !pLeft && !pRight )
                return sal_False;

            sal_Bool bEqual = NULL != pTop &&
                          NULL != pBottom &&
                          NULL != pLeft &&
                          NULL != pRight;

            if( bEqual )
            {
                const sal_uInt16 nDistance = pTop->GetDistance();
                const sal_uInt16 nInWidth  = pTop->GetInWidth();
                const sal_uInt16 nOutWidth = pTop->GetOutWidth();

                bEqual = nDistance == pLeft->GetDistance() &&
                         nInWidth  == pLeft->GetInWidth()  &&
                         nOutWidth == pLeft->GetOutWidth() &&
                         nDistance == pRight->GetDistance()  &&
                         nInWidth  == pRight->GetInWidth()   &&
                         nOutWidth == pRight->GetOutWidth()  &&
                         nDistance == pBottom->GetDistance()  &&
                         nInWidth  == pBottom->GetInWidth()   &&
                         nOutWidth == pBottom->GetOutWidth();
            }

            switch( nMemberId )
            {
            case ALL_BORDER_LINE_WIDTH:
                if( !bEqual || pTop->GetDistance() == 0 )
                    return sal_False;
                break;
            case LEFT_BORDER_LINE_WIDTH:
                if( bEqual || NULL == pLeft || 0 == pLeft->GetDistance() )
                    return sal_False;
                break;
            case RIGHT_BORDER_LINE_WIDTH:
                if( bEqual || NULL == pRight || 0 == pRight->GetDistance() )
                    return sal_False;
                break;
            case TOP_BORDER_LINE_WIDTH:
                if( bEqual || NULL == pTop || 0 == pTop->GetDistance() )
                    return sal_False;
                break;
            case BOTTOM_BORDER_LINE_WIDTH:
                if( bEqual || NULL == pBottom || 0 == pBottom->GetDistance() )
                    return sal_False;
                break;

            }
        }
        break;
    }

    // now export it export

    switch( nMemberId )
    {
    // padding
    case ALL_BORDER_PADDING:
    case LEFT_BORDER_PADDING:
        rUnitConverter.convertMeasure( aOut, nLeftDist );
        break;
    case RIGHT_BORDER_PADDING:
        rUnitConverter.convertMeasure( aOut, nRightDist );
        break;
    case TOP_BORDER_PADDING:
        rUnitConverter.convertMeasure( aOut, nTopDist );
        break;
    case BOTTOM_BORDER_PADDING:
        rUnitConverter.convertMeasure( aOut, nBottomDist );
        break;
    // border
    case ALL_BORDER:
    case LEFT_BORDER:
    case RIGHT_BORDER:
    case TOP_BORDER:
    case BOTTOM_BORDER:
        {
            SvxBorderLine* pLine;
            switch( nMemberId )
            {
            case ALL_BORDER:
            case LEFT_BORDER:
                pLine = pLeft;
                break;
            case RIGHT_BORDER:
                pLine = pRight;
                break;
            case TOP_BORDER:
                pLine = pTop;
                break;
            case BOTTOM_BORDER:
                pLine = pBottom;
                break;
            default:
                pLine = NULL;
                break;
            }

            if( NULL != pLine )
            {
                sal_Int32 nWidth = pLine->GetOutWidth();
                const sal_uInt16 nDistance = pLine->GetDistance();
                if( 0 != nDistance )
                {
                    nWidth += nDistance;
                    nWidth += pLine->GetInWidth();
                }

                char *pStyle = (0 == nDistance) ? sXML_solid : sXML_double;

                rUnitConverter.convertMeasure( aOut, nWidth );

                aOut.append( sal_Unicode( ' ' ) );

                aOut.appendAscii( pStyle );

                aOut.append( sal_Unicode( ' ' ) );

                rUnitConverter.convertColor( aOut, pLine->GetColor() );

            }
            else
            {
                aOut.appendAscii( sXML_none );
            }

        }
        break;
    // width
    case ALL_BORDER_LINE_WIDTH:
    case LEFT_BORDER_LINE_WIDTH:
    case RIGHT_BORDER_LINE_WIDTH:
    case TOP_BORDER_LINE_WIDTH:
    case BOTTOM_BORDER_LINE_WIDTH:
        SvxBorderLine* pLine;
        switch( nMemberId )
        {
        case ALL_BORDER_LINE_WIDTH:
        case LEFT_BORDER_LINE_WIDTH:
            pLine = pLeft;
            break;
        case RIGHT_BORDER_LINE_WIDTH:
            pLine = pRight;
            break;
        case TOP_BORDER_LINE_WIDTH:
            pLine = pTop;
            break;
        case BOTTOM_BORDER_LINE_WIDTH:
            pLine = pBottom;
            break;
        default:
            return sal_False;
        }
        rUnitConverter.convertMeasure( aOut, pLine->GetInWidth() );
        aOut.append( sal_Unicode( ' ' ) );
        rUnitConverter.convertMeasure( aOut, pLine->GetDistance() );
        aOut.append( sal_Unicode( ' ' ) );
        rUnitConverter.convertMeasure( aOut, pLine->GetOutWidth() );
        break;
    }

    rValue = aOut.makeStringAndClear();
    return sal_True;
#endif
    return sal_False;
}


// -----------------------------------------------------------------------

SfxPoolItem* SvxBoxItem::Clone( SfxItemPool* ) const
{
    return new SvxBoxItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxBoxItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;

        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            rText.Erase();

            if ( pTop )
            {
                rText = pTop->GetValueString( eCoreUnit, ePresUnit );
                rText += cpDelim;
            }
            if( !(pTop && pBottom && pLeft && pRight &&
                  *pTop == *pBottom && *pTop == *pLeft && *pTop == *pRight) )
            {
                if ( pBottom )
                {
                    rText += pBottom->GetValueString( eCoreUnit, ePresUnit );
                    rText += cpDelim;
                }
                if ( pLeft )
                {
                    rText += pLeft->GetValueString( eCoreUnit, ePresUnit );
                    rText += cpDelim;
                }
                if ( pRight )
                {
                    rText += pRight->GetValueString( eCoreUnit, ePresUnit );
                    rText += cpDelim;
                }
            }
            rText += GetMetricText( (long)nTopDist, eCoreUnit, ePresUnit );
            if( nTopDist != nBottomDist || nTopDist != nLeftDist ||
                nTopDist != nRightDist )
            {
                (((((rText += cpDelim)
                      += GetMetricText( (long)nBottomDist, eCoreUnit,
                                        ePresUnit ))
                      += cpDelim)
                      += GetMetricText( (long)nLeftDist, eCoreUnit, ePresUnit ))
                      += cpDelim)
                      += GetMetricText( (long)nRightDist, eCoreUnit,
                                        ePresUnit );
            }
            return SFX_ITEM_PRESENTATION_NAMELESS;
        }
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if( !(pTop || pBottom || pLeft || pRight) )
            {
                rText = SVX_RESSTR(RID_SVXITEMS_BORDER_NONE);
                rText += cpDelim;
            }
            else
            {
                rText = SVX_RESSTR(RID_SVXITEMS_BORDER_COMPLETE);
                if( pTop && pBottom && pLeft && pRight &&
                    *pTop == *pBottom && *pTop == *pLeft && *pTop == *pRight )
                {
                    rText += pTop->GetValueString( eCoreUnit, ePresUnit, sal_True );
                    rText += cpDelim;
                }
                else
                {
                    if ( pTop )
                    {
                        rText += SVX_RESSTR(RID_SVXITEMS_BORDER_TOP);
                        rText += pTop->GetValueString( eCoreUnit, ePresUnit, sal_True );
                        rText += cpDelim;
                    }
                    if ( pBottom )
                    {
                        rText += SVX_RESSTR(RID_SVXITEMS_BORDER_BOTTOM);
                        rText += pBottom->GetValueString( eCoreUnit, ePresUnit, sal_True );
                        rText += cpDelim;
                    }
                    if ( pLeft )
                    {
                        rText += SVX_RESSTR(RID_SVXITEMS_BORDER_LEFT);
                        rText += pLeft->GetValueString( eCoreUnit, ePresUnit, sal_True );
                        rText += cpDelim;
                    }
                    if ( pRight )
                    {
                        rText += SVX_RESSTR(RID_SVXITEMS_BORDER_RIGHT);
                        rText += pRight->GetValueString( eCoreUnit, ePresUnit, sal_True );
                        rText += cpDelim;
                    }
                }
            }

            rText += SVX_RESSTR(RID_SVXITEMS_BORDER_DISTANCE);
            if( nTopDist == nBottomDist && nTopDist == nLeftDist &&
                nTopDist == nRightDist )
            {
                rText += GetMetricText( (long)nTopDist, eCoreUnit,
                                            ePresUnit );
                rText += SVX_RESSTR(GetMetricId(ePresUnit));
            }
            else
            {
                (((rText += SVX_RESSTR(RID_SVXITEMS_BORDER_TOP))
                      += GetMetricText( (long)nTopDist, eCoreUnit,
                                        ePresUnit ))
                      += SVX_RESSTR(GetMetricId(ePresUnit)))
                      += cpDelim;
                (((rText += SVX_RESSTR(RID_SVXITEMS_BORDER_BOTTOM))
                      += GetMetricText( (long)nBottomDist, eCoreUnit,
                                        ePresUnit ))
                      += SVX_RESSTR(GetMetricId(ePresUnit)))
                      += cpDelim;
                (((rText += SVX_RESSTR(RID_SVXITEMS_BORDER_LEFT))
                      += GetMetricText( (long)nLeftDist, eCoreUnit,
                                        ePresUnit ))
                      += SVX_RESSTR(GetMetricId(ePresUnit)))
                      += cpDelim;
                ((rText += SVX_RESSTR(RID_SVXITEMS_BORDER_RIGHT))
                      += GetMetricText( (long)nRightDist, eCoreUnit,
                                        ePresUnit ))
                      += SVX_RESSTR(GetMetricId(ePresUnit));
            }
            return SFX_ITEM_PRESENTATION_COMPLETE;
        }
    }
#endif // !SVX_LIGHT
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SvStream& SvxBoxItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    rStrm << (sal_uInt16) GetDistance();
    const SvxBorderLine* pLine[ 4 ];    // top, left, right, bottom
    pLine[ 0 ] = GetTop();
    pLine[ 1 ] = GetLeft();
    pLine[ 2 ] = GetRight();
    pLine[ 3 ] = GetBottom();

    for( int i = 0; i < 4; i++ )
    {
        const SvxBorderLine* l = pLine[ i ];
        if( l )
        {
            rStrm << (sal_Int8) i
                  << l->GetColor()
                  << (sal_uInt16) l->GetOutWidth()
                  << (sal_uInt16) l->GetInWidth()
                  << (sal_uInt16) l->GetDistance();
        }
    }
    sal_Int8 cLine = 4;
    if( nItemVersion >= BOX_4DISTS_VERSION &&
        !(nTopDist == nLeftDist &&
          nTopDist == nRightDist &&
          nTopDist == nBottomDist) )
    {
        cLine |= 0x10;
    }

    rStrm << cLine;

    if( nItemVersion >= BOX_4DISTS_VERSION && (cLine & 0x10) != 0 )
    {
        rStrm << (sal_uInt16)nTopDist
              << (sal_uInt16)nLeftDist
              << (sal_uInt16)nRightDist
              << (sal_uInt16)nBottomDist;
    }

    return rStrm;
}

// -----------------------------------------------------------------------

sal_uInt16 SvxBoxItem::GetVersion( sal_uInt16 nFFVer ) const
{
    DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
            SOFFICE_FILEFORMAT_40==nFFVer ||
            SOFFICE_FILEFORMAT_NOW==nFFVer,
            "SvxBoxItem: Gibt es ein neues Fileformat?" );
    return SOFFICE_FILEFORMAT_31==nFFVer ||
           SOFFICE_FILEFORMAT_40==nFFVer ? 0 : BOX_4DISTS_VERSION;
}

// -----------------------------------------------------------------------

int SvxBoxItem::ScaleMetrics( long nMult, long nDiv )
{
    if ( pTop )     pTop->ScaleMetrics( nMult, nDiv );
    if ( pBottom )  pBottom->ScaleMetrics( nMult, nDiv );
    if ( pLeft )    pLeft->ScaleMetrics( nMult, nDiv );
    if ( pRight )   pBottom->ScaleMetrics( nMult, nDiv );
    nTopDist = (sal_uInt16)Scale( nTopDist, nMult, nDiv );
    nBottomDist = (sal_uInt16)Scale( nBottomDist, nMult, nDiv );
    nLeftDist = (sal_uInt16)Scale( nLeftDist, nMult, nDiv );
    nRightDist = (sal_uInt16)Scale( nRightDist, nMult, nDiv );
    return 1;
}

// -----------------------------------------------------------------------

int SvxBoxItem::HasMetrics() const
{
    return 1;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxBoxItem::Create( SvStream& rStrm, sal_uInt16 nIVersion ) const
{
    sal_uInt16 nDistance;
    rStrm >> nDistance;
    SvxBoxItem* pAttr = new SvxBoxItem( Which() );

    sal_uInt16 aLineMap[4] = { BOX_LINE_TOP, BOX_LINE_LEFT,
                           BOX_LINE_RIGHT, BOX_LINE_BOTTOM };

    sal_Int8 cLine;
    while( sal_True )
    {
        rStrm >> cLine;

        if( cLine > 3 )
            break;
        sal_uInt16 nOutline, nInline, nDistance;
        Color aColor;
        rStrm >> aColor >> nOutline >> nInline >> nDistance;
        SvxBorderLine aBorder( &aColor, nOutline, nInline, nDistance );

        pAttr->SetLine( &aBorder, aLineMap[cLine] );
    }

    if( nIVersion >= BOX_4DISTS_VERSION && (cLine&0x10) != 0 )
    {
        for( sal_uInt16 i=0; i < 4; i++ )
        {
            sal_uInt16 nDist;
            rStrm >> nDist;
            pAttr->SetDistance( nDist, aLineMap[i] );
        }
    }
    else
    {
        pAttr->SetDistance( nDistance );
    }

    return pAttr;
}

// -----------------------------------------------------------------------

const SvxBorderLine *SvxBoxItem::GetLine( sal_uInt16 nLine ) const
{
    const SvxBorderLine *pRet = 0;

    switch ( nLine )
    {
        case BOX_LINE_TOP:
            pRet = pTop;
            break;
        case BOX_LINE_BOTTOM:
            pRet = pBottom;
            break;
        case BOX_LINE_LEFT:
            pRet = pLeft;
            break;
        case BOX_LINE_RIGHT:
            pRet = pRight;
            break;
        default:
            DBG_ERROR( "wrong line" );
            break;
    }

    return pRet;
}

// -----------------------------------------------------------------------

void SvxBoxItem::SetLine( const SvxBorderLine* pNew, sal_uInt16 nLine )
{
    SvxBorderLine* pTmp = pNew ? new SvxBorderLine( *pNew ) : 0;

    switch ( nLine )
    {
        case BOX_LINE_TOP:
            delete pTop;
            pTop = pTmp;
            break;
        case BOX_LINE_BOTTOM:
            delete pBottom;
            pBottom = pTmp;
            break;
        case BOX_LINE_LEFT:
            delete pLeft;
            pLeft = pTmp;
            break;
        case BOX_LINE_RIGHT:
            delete pRight;
            pRight = pTmp;
            break;
        default:
            DBG_ERROR( "wrong line" );
    }
}

// -----------------------------------------------------------------------

sal_uInt16 SvxBoxItem::GetDistance() const
{
    // The smallest distance that is not 0 will be returned.
    sal_uInt16 nDist = nTopDist;
    if( nBottomDist && (!nDist || nBottomDist < nDist) )
        nDist = nBottomDist;
    if( nLeftDist && (!nDist || nLeftDist < nDist) )
        nDist = nLeftDist;
    if( nRightDist && (!nDist || nRightDist < nDist) )
        nDist = nRightDist;

    return nDist;
}

// -----------------------------------------------------------------------

sal_uInt16 SvxBoxItem::GetDistance( sal_uInt16 nLine ) const
{
    sal_uInt16 nDist = 0;
    switch ( nLine )
    {
        case BOX_LINE_TOP:
            nDist = nTopDist;
            break;
        case BOX_LINE_BOTTOM:
            nDist = nBottomDist;
            break;
        case BOX_LINE_LEFT:
            nDist = nLeftDist;
            break;
        case BOX_LINE_RIGHT:
            nDist = nRightDist;
            break;
        default:
            DBG_ERROR( "wrong line" );
    }

    return nDist;
}

// -----------------------------------------------------------------------

void SvxBoxItem::SetDistance( sal_uInt16 nNew, sal_uInt16 nLine )
{
    switch ( nLine )
    {
        case BOX_LINE_TOP:
            nTopDist = nNew;
            break;
        case BOX_LINE_BOTTOM:
            nBottomDist = nNew;
            break;
        case BOX_LINE_LEFT:
            nLeftDist = nNew;
            break;
        case BOX_LINE_RIGHT:
            nRightDist = nNew;
            break;
        default:
            DBG_ERROR( "wrong line" );
    }
}

// -----------------------------------------------------------------------

sal_uInt16 SvxBoxItem::CalcLineSpace( sal_uInt16 nLine, sal_Bool bIgnoreLine ) const
{
    SvxBorderLine* pTmp = 0;
    sal_uInt16 nDist = 0;
    switch ( nLine )
    {
    case BOX_LINE_TOP:
        pTmp = pTop;
        nDist = nTopDist;
        break;
    case BOX_LINE_BOTTOM:
        pTmp = pBottom;
        nDist = nBottomDist;
        break;
    case BOX_LINE_LEFT:
        pTmp = pLeft;
        nDist = nLeftDist;
        break;
    case BOX_LINE_RIGHT:
        pTmp = pRight;
        nDist = nRightDist;
        break;
    default:
        DBG_ERROR( "wrong line" );
    }

    if( pTmp )
    {
        nDist += pTmp->GetOutWidth();
        nDist += pTmp->GetInWidth();
        nDist += pTmp->GetDistance();
    }
    else if( !bIgnoreLine )
        nDist = 0;
    return nDist;
}

// class SvxBoxInfoItem --------------------------------------------------

SvxBoxInfoItem::SvxBoxInfoItem( const sal_uInt16 nId ) :

    SfxPoolItem( nId ),

    pHori   ( 0 ),
    pVert   ( 0 ),
    nDefDist( 0 )

{
    bTable = bDist = bMinDist = sal_False;
    ResetFlags();
}

// -----------------------------------------------------------------------

SvxBoxInfoItem::SvxBoxInfoItem( const SvxBoxInfoItem& rCpy ) :

    SfxPoolItem( rCpy )

{
    pHori       = rCpy.GetHori() ? new SvxBorderLine( *rCpy.GetHori() ) : 0;
    pVert       = rCpy.GetVert() ? new SvxBorderLine( *rCpy.GetVert() ) : 0;
    bTable      = rCpy.IsTable();
    bDist       = rCpy.IsDist();
    bMinDist    = rCpy.IsMinDist();
    nValidFlags = rCpy.nValidFlags;
    nDefDist    = rCpy.GetDefDist();
}

// -----------------------------------------------------------------------

SvxBoxInfoItem::~SvxBoxInfoItem()
{
    delete pHori;
    delete pVert;
}

// -----------------------------------------------------------------------

SvxBoxInfoItem &SvxBoxInfoItem::operator=( const SvxBoxInfoItem& rCpy )
{
    delete pHori;
    delete pVert;
    pHori       = rCpy.GetHori() ? new SvxBorderLine( *rCpy.GetHori() ) : 0;
    pVert       = rCpy.GetVert() ? new SvxBorderLine( *rCpy.GetVert() ) : 0;
    bTable      = rCpy.IsTable();
    bDist       = rCpy.IsDist();
    bMinDist    = rCpy.IsMinDist();
    nValidFlags = rCpy.nValidFlags;
    nDefDist    = rCpy.GetDefDist();
    return *this;
}

// -----------------------------------------------------------------------

int SvxBoxInfoItem::operator==( const SfxPoolItem& rAttr ) const
{
    SvxBoxInfoItem& rBoxInfo = (SvxBoxInfoItem&)rAttr;

    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return (   bTable                    == rBoxInfo.IsTable()
            && bDist                     == rBoxInfo.IsDist()
            && bMinDist                  == rBoxInfo.IsMinDist()
            && nValidFlags               == rBoxInfo.nValidFlags
            && nDefDist                  == rBoxInfo.GetDefDist()
            && CmpBrdLn( pHori, rBoxInfo.GetHori() )
            && CmpBrdLn( pVert, rBoxInfo.GetVert() )
           );
}

// -----------------------------------------------------------------------

void SvxBoxInfoItem::SetLine( const SvxBorderLine* pNew, sal_uInt16 nLine )
{
    SvxBorderLine* pTmp = pNew ? new SvxBorderLine( *pNew ) : 0;

    if ( BOXINFO_LINE_HORI == nLine )
    {
        delete pHori;
        pHori = pTmp;
    }
    else if ( BOXINFO_LINE_VERT == nLine )
    {
        delete pVert;
        pVert = pTmp;
    }
    else
        DBG_ERROR( "wrong line" );
}


// -----------------------------------------------------------------------

SfxPoolItem* SvxBoxInfoItem::Clone( SfxItemPool* ) const
{
    return new SvxBoxInfoItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxBoxInfoItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
/*!!!
    ResMgr* pMgr = DIALOG_MGR();
    if ( pHori )
    {
        rText += pHori->GetValueString();
        rText += cpDelim;
    }
    if ( pVert )
    {
        rText += pVert->GetValueString();
        rText += cpDelim;
    }
    if ( bTable )
        rText += String( ResId( RID_SVXITEMS_BOXINF_TABLE_TRUE, pMgr ) );
    else
        rText += String( ResId( RID_SVXITEMS_BOXINF_TABLE_FALSE, pMgr ) );
    rText += cpDelim;
    if ( bDist )
        rText += String( ResId( RID_SVXITEMS_BOXINF_DIST_TRUE, pMgr ) );
    else
        rText += String( ResId( RID_SVXITEMS_BOXINF_DIST_FALSE, pMgr ) );
    rText += cpDelim;
    if ( bMinDist )
        rText += String( ResId( RID_SVXITEMS_BOXINF_MDIST_TRUE, pMgr ) );
    else
        rText += String( ResId( RID_SVXITEMS_BOXINF_MDIST_FALSE, pMgr ) );
    rText += cpDelim;
    rText += nDefDist;
    return SFX_ITEM_PRESENTATION_NAMELESS;
*/
    rText.Erase();
#endif // !SVX_LIGHT
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SvStream& SvxBoxInfoItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    sal_Int8 cFlags = 0;

    if ( IsTable() )
        cFlags |= 0x01;
    if ( IsDist() )
        cFlags |= 0x02;
    if ( IsMinDist() )
        cFlags |= 0x04;
    rStrm << (sal_Int8)   cFlags
          << (sal_uInt16) GetDefDist();
    const SvxBorderLine* pLine[ 2 ];
    pLine[ 0 ] = GetHori();
    pLine[ 1 ] = GetVert();

    for( int i = 0; i < 2; i++ )
    {
        const SvxBorderLine* l = pLine[ i ];
        if( l )
        {
            rStrm << (char) i
                  << l->GetColor()
                  << (short) l->GetOutWidth()
                  << (short) l->GetInWidth()
                  << (short) l->GetDistance();
        }
    }
    rStrm << (char) 2;
    return rStrm;
}

// -----------------------------------------------------------------------

int SvxBoxInfoItem::ScaleMetrics( long nMult, long nDiv )
{
    if ( pHori ) pHori->ScaleMetrics( nMult, nDiv );
    if ( pVert ) pVert->ScaleMetrics( nMult, nDiv );
    nDefDist = (sal_uInt16)Scale( nDefDist, nMult, nDiv );
    return 1;
}

// -----------------------------------------------------------------------

int SvxBoxInfoItem::HasMetrics() const
{
    return 1;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxBoxInfoItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 cFlags;
    sal_uInt16 nDefDist;
    rStrm >> cFlags >> nDefDist;

    SvxBoxInfoItem* pAttr = new SvxBoxInfoItem( Which() );

    pAttr->SetTable  ( ( cFlags & 0x01 ) != 0 );
    pAttr->SetDist   ( ( cFlags & 0x02 ) != 0 );
    pAttr->SetMinDist( ( cFlags & 0x04 ) != 0 );
    pAttr->SetDefDist( nDefDist );

    while( sal_True )
    {
        sal_Int8 cLine;
        rStrm >> cLine;

        if( cLine > 1 )
            break;
        short nOutline, nInline, nDistance;
        Color aColor;
        rStrm >> aColor >> nOutline >> nInline >> nDistance;
        SvxBorderLine aBorder( &aColor, nOutline, nInline, nDistance );

        switch( cLine )
        {
            case 0: pAttr->SetLine( &aBorder, BOXINFO_LINE_HORI ); break;
            case 1: pAttr->SetLine( &aBorder, BOXINFO_LINE_VERT ); break;
        }
    }
    return pAttr;
}

// -----------------------------------------------------------------------

void SvxBoxInfoItem::ResetFlags()
{
    nValidFlags = 0x7F; // alles g"ultig au/ser Disable
}

// class SvxFmtBreakItem -------------------------------------------------

int SvxFmtBreakItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rAttr ), "unequal types" );

    return GetValue() == ( (SvxFmtBreakItem&)rAttr ).GetValue();
}

//------------------------------------------------------------------------

SfxItemPresentation SvxFmtBreakItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;

        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetValueTextByPos( GetValue() );
            return ePres;
    }
#endif // !SVX_LIGHT
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

XubString SvxFmtBreakItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos < SVX_BREAK_END, "enum overflow!" );
    XubString aStr( SVX_RES( RID_SVXITEMS_BREAK_BEGIN + nPos ) );
    return aStr;
}

// -----------------------------------------------------------------------
sal_Bool SvxFmtBreakItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    style::BreakType eBreak = style::BreakType_NONE;
    switch ( (SvxBreak)GetValue() )
    {
    case SVX_BREAK_COLUMN_BEFORE:   eBreak = style::BreakType_COLUMN_BEFORE; break;
    case SVX_BREAK_COLUMN_AFTER:    eBreak = style::BreakType_COLUMN_AFTER ; break;
    case SVX_BREAK_COLUMN_BOTH:     eBreak = style::BreakType_COLUMN_BOTH  ; break;
    case SVX_BREAK_PAGE_BEFORE:     eBreak = style::BreakType_PAGE_BEFORE  ; break;
    case SVX_BREAK_PAGE_AFTER:      eBreak = style::BreakType_PAGE_AFTER   ; break;
    case SVX_BREAK_PAGE_BOTH:       eBreak = style::BreakType_PAGE_BOTH    ; break;
    }
    rVal <<= eBreak;
    return sal_True;
}
// -----------------------------------------------------------------------
sal_Bool SvxFmtBreakItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    style::BreakType nBreak;

    if(!(rVal >>= nBreak))
    {
        sal_Int32 nValue;
        if(!(rVal >>= nValue))
            return sal_False;

        nBreak = (style::BreakType) nValue;
    }

    SvxBreak eBreak = SVX_BREAK_NONE;
    switch( nBreak )
    {
        case style::BreakType_COLUMN_BEFORE:    eBreak = SVX_BREAK_COLUMN_BEFORE; break;
        case style::BreakType_COLUMN_AFTER: eBreak = SVX_BREAK_COLUMN_AFTER;  break;
        case style::BreakType_COLUMN_BOTH:      eBreak = SVX_BREAK_COLUMN_BOTH;   break;
        case style::BreakType_PAGE_BEFORE:      eBreak = SVX_BREAK_PAGE_BEFORE;   break;
        case style::BreakType_PAGE_AFTER:       eBreak = SVX_BREAK_PAGE_AFTER;    break;
        case style::BreakType_PAGE_BOTH:        eBreak = SVX_BREAK_PAGE_BOTH;     break;
    }
    SetValue((sal_uInt16) eBreak);

    return sal_True;
}

#ifndef SVX_LIGHT
SvXMLEnumMapEntry psXML_BreakType[] =
{
    { sXML_auto,    0 },
    { sXML_column, 1 },
    { sXML_page,   2 },
    { sXML_even_page, 2 },
    { sXML_odd_page, 2 },
    { 0, 0}
};
#endif

sal_Bool SvxFmtBreakItem::importXML( const OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    sal_uInt16 eEnum;

    if( !rUnitConverter.convertEnum( eEnum, rValue, psXML_BreakType ) )
        return sal_False;

    if( eEnum == 0 )
    {
        SetValue( SVX_BREAK_NONE );
        return sal_True;
    }

    switch( nMemberId )
    {
    case MID_BREAK_BEFORE:
        SetValue( (eEnum == 1)?SVX_BREAK_COLUMN_BEFORE:SVX_BREAK_PAGE_BEFORE );
        break;
    case MID_BREAK_AFTER:
        SetValue( (eEnum == 1)?SVX_BREAK_COLUMN_AFTER:SVX_BREAK_PAGE_AFTER );
        break;
    }

    return sal_True;
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------

sal_Bool SvxFmtBreakItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    sal_uInt16 eEnum = 0;

    switch( nMemberId )
    {
    case MID_BREAK_BEFORE:
        switch( GetValue() )
        {
        case SVX_BREAK_COLUMN_BEFORE:
            eEnum = 1;
            break;
        case SVX_BREAK_PAGE_BEFORE:
            eEnum = 2;
            break;
        case SVX_BREAK_NONE:
            eEnum = 0;
            break;
        default:
            return sal_False;
        }
        break;
    case MID_BREAK_AFTER:
        switch( GetValue() )
        {
        case SVX_BREAK_COLUMN_AFTER:
            eEnum = 1;
            break;
        case SVX_BREAK_PAGE_AFTER:
            eEnum = 2;
            break;
        case SVX_BREAK_NONE:
            eEnum = 0;
            break;
        default:
            return sal_False;
        }
        break;
    }

    OUStringBuffer aOut;
    sal_Bool bOk = rUnitConverter.convertEnum( aOut, eEnum, psXML_BreakType );
    rValue = aOut.makeStringAndClear();
    return bOk;
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFmtBreakItem::Clone( SfxItemPool* ) const
{
    return new SvxFmtBreakItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxFmtBreakItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    rStrm << (sal_Int8)GetValue();
    if( FMTBREAK_NOAUTO > nItemVersion )
        rStrm << (sal_Int8)0x01;
    return rStrm;
}

// -----------------------------------------------------------------------

sal_uInt16 SvxFmtBreakItem::GetVersion( sal_uInt16 nFFVer ) const
{
    DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
            SOFFICE_FILEFORMAT_40==nFFVer ||
            SOFFICE_FILEFORMAT_NOW==nFFVer,
            "SvxFmtBreakItem: Gibt es ein neues Fileformat?" );
    return SOFFICE_FILEFORMAT_31==nFFVer ||
           SOFFICE_FILEFORMAT_40==nFFVer ? 0 : FMTBREAK_NOAUTO;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFmtBreakItem::Create( SvStream& rStrm, sal_uInt16 nVersion ) const
{
    sal_Int8 eBreak, bDummy;
    rStrm >> eBreak;
    if( FMTBREAK_NOAUTO > nVersion )
        rStrm >> bDummy;
    return new SvxFmtBreakItem( (const SvxBreak)eBreak, Which() );
}

// -----------------------------------------------------------------------

sal_uInt16 SvxFmtBreakItem::GetValueCount() const
{
    return SVX_BREAK_END;   // SVX_BREAK_PAGE_BOTH + 1
}

// class SvxFmtKeepItem -------------------------------------------------

SfxPoolItem* SvxFmtKeepItem::Clone( SfxItemPool* ) const
{
    return new SvxFmtKeepItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxFmtKeepItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    rStrm << (sal_Int8)GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFmtKeepItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 bIsKeep;
    rStrm >> bIsKeep;
    return new SvxFmtKeepItem( sal_Bool( bIsKeep != 0 ), Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxFmtKeepItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;

        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            sal_uInt16 nId = RID_SVXITEMS_FMTKEEP_FALSE;

            if ( GetValue() )
                nId = RID_SVXITEMS_FMTKEEP_TRUE;
            rText = SVX_RESSTR(nId);
            return ePres;
        }
    }
#endif
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool SvxFmtKeepItem::importXML( const OUString& rValue,
                              sal_uInt16 nMemberId,
                              const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT
    sal_Bool bValue;
    if(!rUnitConverter.convertBool( bValue, rValue ) )
        return sal_False;

    SetValue( bValue );

    return sal_True;
#else
    return sal_False;
#endif
}

sal_Bool SvxFmtKeepItem::exportXML( OUString& rValue,
                            sal_uInt16 nMemberId,
                            const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    OUStringBuffer aOut;
    rUnitConverter.convertBool( aOut, GetValue() );
    rValue = aOut.makeStringAndClear();

    return sal_True;
#else
    return sal_False;
#endif
}

// class SvxLineItem ------------------------------------------------------

SvxLineItem::SvxLineItem( const sal_uInt16 nId ) :

    SfxPoolItem ( nId ),

    pLine( NULL )
{
}

// -----------------------------------------------------------------------

SvxLineItem::SvxLineItem( const SvxLineItem& rCpy ) :

    SfxPoolItem ( rCpy )
{
    pLine = rCpy.GetLine() ? new SvxBorderLine( *rCpy.GetLine() ) : 0;
}


// -----------------------------------------------------------------------

SvxLineItem::~SvxLineItem()
{
    delete pLine;
}

// -----------------------------------------------------------------------

SvxLineItem& SvxLineItem::operator=( const SvxLineItem& rLine )
{
    SetLine( rLine.GetLine() );

    return *this;
}

// -----------------------------------------------------------------------

int SvxLineItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return CmpBrdLn( pLine, ((SvxLineItem&)rAttr).GetLine() );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxLineItem::Clone( SfxItemPool* ) const
{
    return new SvxLineItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxLineItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if ( pLine )
                rText = pLine->GetValueString( eCoreUnit, ePresUnit,
                    (SFX_ITEM_PRESENTATION_COMPLETE == ePres) );
            return ePres;
        }
    }
#endif
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SvStream& SvxLineItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    if( pLine )
    {
        rStrm << pLine->GetColor()
              << (short)pLine->GetOutWidth()
              << (short)pLine->GetInWidth()
              << (short)pLine->GetDistance();
    }
    return rStrm;
}

// -----------------------------------------------------------------------

int SvxLineItem::ScaleMetrics( long nMult, long nDiv )
{
    if ( pLine ) pLine->ScaleMetrics( nMult, nDiv );
    return 1;
}

// -----------------------------------------------------------------------

int SvxLineItem::HasMetrics() const
{
    return 1;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxLineItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    SvxLineItem* pLine = new SvxLineItem( Which() );
    short        nOutline, nInline, nDistance;
    Color        aColor;

    rStrm >> aColor >> nOutline >> nInline >> nDistance;
    SvxBorderLine aLine( &aColor, nOutline, nInline, nDistance );
    pLine->SetLine( &aLine );
    return pLine;
}

// -----------------------------------------------------------------------

void SvxLineItem::SetLine( const SvxBorderLine* pNew )
{
    delete pLine;
    pLine = pNew ? new SvxBorderLine( *pNew ) : 0;
}

#ifdef WNT
#pragma optimize ( "", off )
#endif

// class SvxBrushItem ----------------------------------------------------

#define LOAD_GRAPHIC    ((sal_uInt16)0x0001)
#define LOAD_LINK       ((sal_uInt16)0x0002)
#define LOAD_FILTER     ((sal_uInt16)0x0004)

// class SvxBrushItem_Impl -----------------------------------------------

class SvxBrushItem_Impl
{
public:
    GraphicObject*  pGraphicObject;
#ifndef SVX_LIGHT
    SfxMediumRef    xMedium;
#endif
    Link            aDoneLink;

    SvxBrushItem_Impl( GraphicObject* p ) : pGraphicObject( p ) {}
};

// class SvxBrushItemLink_Impl -------------------------------------------

class SvxBrushItemLink_Impl : public SfxBrushItemLink
{
    virtual Graphic                     GetGraphic( const String& rLink, const String& rFilter );
    virtual CreateSvxBrushTabPage       GetBackgroundTabpageCreateFunc();
    virtual GetSvxBrushTabPageRanges    GetBackgroundTabpageRanges();
};

// -----------------------------------------------------------------------

Graphic SvxBrushItemLink_Impl::GetGraphic( const String& rLink, const String& rFilter)
{
    Graphic aResult;
    SvxBrushItem aItem( rLink, rFilter, GPOS_TILED );
    const Graphic* pGraph = aItem.GetGraphic();
    if( pGraph )
        aResult = *pGraph;
    return aResult;
}

// -----------------------------------------------------------------------

CreateSvxBrushTabPage SvxBrushItemLink_Impl::GetBackgroundTabpageCreateFunc()
{
#ifndef SVX_LIGHT
    return (CreateSvxBrushTabPage)SvxBackgroundTabPage::Create;
#else
    return CreateSvxBrushTabPage();
#endif
}

// -----------------------------------------------------------------------

GetSvxBrushTabPageRanges SvxBrushItemLink_Impl::GetBackgroundTabpageRanges()
{
#ifndef SVX_LIGHT
    return (GetSvxBrushTabPageRanges)SvxBackgroundTabPage::GetRanges;
#else
    return GetSvxBrushTabPageRanges();
#endif
}

// -----------------------------------------------------------------------

void SvxBrushItem::InitSfxLink()
{
    SfxBrushItemLink::Set( new SvxBrushItemLink_Impl );
}

// -----------------------------------------------------------------------

void SvxBrushItem::SetDoneLink( const Link& rLink )
{
    pImpl->aDoneLink = rLink;
}

// -----------------------------------------------------------------------

SvxBrushItem::SvxBrushItem( sal_uInt16 nWhich ) :

    SfxPoolItem( nWhich ),

    aColor      ( COL_TRANSPARENT ),
    pImpl       ( new SvxBrushItem_Impl( 0 ) ),
    pStrLink    ( NULL ),
    pStrFilter  ( NULL ),
    eGraphicPos ( GPOS_NONE ),
    bLoadAgain  ( sal_True )

{
}

// -----------------------------------------------------------------------

SvxBrushItem::SvxBrushItem( const Color& rColor, sal_uInt16 nWhich) :

    SfxPoolItem( nWhich ),

    aColor      ( rColor ),
    pImpl       ( new SvxBrushItem_Impl( 0 ) ),
    pStrLink    ( NULL ),
    pStrFilter  ( NULL ),
    eGraphicPos ( GPOS_NONE ),
    bLoadAgain  ( sal_True )

{
}

// -----------------------------------------------------------------------

SvxBrushItem::SvxBrushItem( const Graphic& rGraphic, SvxGraphicPosition ePos,
                            sal_uInt16 nWhich ) :

    SfxPoolItem( nWhich ),

    aColor      ( COL_TRANSPARENT ),
    pImpl       ( new SvxBrushItem_Impl( new GraphicObject( rGraphic ) ) ),
    pStrLink    ( NULL ),
    pStrFilter  ( NULL ),
    eGraphicPos ( ( GPOS_NONE != ePos ) ? ePos : GPOS_MM ),
    bLoadAgain  ( sal_True )

{
    DBG_ASSERT( GPOS_NONE != ePos, "SvxBrushItem-Ctor with GPOS_NONE == ePos" );
}

// -----------------------------------------------------------------------

SvxBrushItem::SvxBrushItem( const GraphicObject& rGraphicObj,
                            SvxGraphicPosition ePos, sal_uInt16 nWhich ) :

    SfxPoolItem( nWhich ),

    aColor      ( COL_TRANSPARENT ),
    pImpl       ( new SvxBrushItem_Impl( new GraphicObject( rGraphicObj ) ) ),
    pStrLink    ( NULL ),
    pStrFilter  ( NULL ),
    eGraphicPos ( ( GPOS_NONE != ePos ) ? ePos : GPOS_MM ),
    bLoadAgain  ( sal_True )

{
    DBG_ASSERT( GPOS_NONE != ePos, "SvxBrushItem-Ctor with GPOS_NONE == ePos" );
}

// -----------------------------------------------------------------------

SvxBrushItem::SvxBrushItem(
    const String& rLink, const String& rFilter,
    SvxGraphicPosition ePos, sal_uInt16 nWhich ) :

    SfxPoolItem( nWhich ),

    aColor      ( COL_TRANSPARENT ),
    pImpl       ( new SvxBrushItem_Impl( NULL ) ),
    pStrLink    ( new String( rLink ) ),
    pStrFilter  ( new String( rFilter ) ),
    eGraphicPos ( ( GPOS_NONE != ePos ) ? ePos : GPOS_MM ),
    bLoadAgain  ( sal_True )

{
    DBG_ASSERT( GPOS_NONE != ePos, "SvxBrushItem-Ctor with GPOS_NONE == ePos" );
}

// -----------------------------------------------------------------------

SvxBrushItem::SvxBrushItem( SvStream& rStream, sal_uInt16 nVersion,
                            sal_uInt16 nWhich ) :

    SfxPoolItem( nWhich ),

    aColor      ( COL_TRANSPARENT ),
    pImpl       ( new SvxBrushItem_Impl( NULL ) ),
    pStrLink    ( NULL ),
    pStrFilter  ( NULL ),
    eGraphicPos ( GPOS_NONE )

{
    sal_Bool bTrans;
    Color aTempColor;
    Color aTempFillColor;
    sal_Int8 nStyle;

    rStream >> bTrans;
    rStream >> aTempColor;
    rStream >> aTempFillColor;
    rStream >> nStyle;

    switch ( nStyle )
    {
        case 8://BRUSH_25:
        {
            sal_uInt32  nRed    = aTempColor.GetRed();
            sal_uInt32  nGreen  = aTempColor.GetGreen();
            sal_uInt32  nBlue   = aTempColor.GetBlue();
            nRed   += (sal_uInt32)(aTempFillColor.GetRed())*2;
            nGreen += (sal_uInt32)(aTempFillColor.GetGreen())*2;
            nBlue  += (sal_uInt32)(aTempFillColor.GetBlue())*2;
            aColor = Color( (sal_Int8)(nRed/3), (sal_Int8)(nGreen/3), (sal_Int8)(nBlue/3) );
        }
        break;

        case 9://BRUSH_50:
        {
            sal_uInt32  nRed    = aTempColor.GetRed();
            sal_uInt32  nGreen  = aTempColor.GetGreen();
            sal_uInt32  nBlue   = aTempColor.GetBlue();
            nRed   += (sal_uInt32)(aTempFillColor.GetRed());
            nGreen += (sal_uInt32)(aTempFillColor.GetGreen());
            nBlue  += (sal_uInt32)(aTempFillColor.GetBlue());
            aColor = Color( (sal_Int8)(nRed/2), (sal_Int8)(nGreen/2), (sal_Int8)(nBlue/2) );
        }
        break;

        case 10://BRUSH_75:
        {
            sal_uInt32  nRed    = aTempColor.GetRed()*2;
            sal_uInt32  nGreen  = aTempColor.GetGreen()*2;
            sal_uInt32  nBlue   = aTempColor.GetBlue()*2;
            nRed   += (sal_uInt32)(aTempFillColor.GetRed());
            nGreen += (sal_uInt32)(aTempFillColor.GetGreen());
            nBlue  += (sal_uInt32)(aTempFillColor.GetBlue());
            aColor = Color( (sal_Int8)(nRed/3), (sal_Int8)(nGreen/3), (sal_Int8)(nBlue/3) );
        }
        break;

        case 0://BRUSH_NULL:
            aColor = Color( COL_TRANSPARENT );
        break;

        default:
            aColor = aTempColor;
    }

    if ( nVersion >= BRUSH_GRAPHIC_VERSION )
    {
        sal_uInt16 nDoLoad = 0;
        sal_Int8 nPos;

        rStream >> nDoLoad;

        if ( nDoLoad & LOAD_GRAPHIC )
        {
            Graphic aGraphic;

            rStream >> aGraphic;
            pImpl->pGraphicObject = new GraphicObject( aGraphic );

            if( SVSTREAM_FILEFORMAT_ERROR == rStream.GetError() )
            {
                rStream.ResetError();
                rStream.SetError( ERRCODE_SVX_GRAPHIC_WRONG_FILEFORMAT|
                                  ERRCODE_WARNING_MASK  );
            }
        }

        if ( nDoLoad & LOAD_LINK )
        {
            String aRel;
            // UNICODE: rStream >> aRel;
            rStream.ReadByteString(aRel);

            String aAbs = INetURLObject::RelToAbs( aRel );
            pStrLink = new String( aAbs );
        }

        if ( nDoLoad & LOAD_FILTER )
        {
            pStrFilter = new String;
            // UNICODE: rStream >> *pStrFilter;
            rStream.ReadByteString(*pStrFilter);
        }

        rStream >> nPos;

        eGraphicPos = (SvxGraphicPosition)nPos;
    }
}

// -----------------------------------------------------------------------

SvxBrushItem::SvxBrushItem( const SvxBrushItem& rItem ) :

    SfxPoolItem( rItem.Which() ),

    pImpl       ( new SvxBrushItem_Impl( NULL ) ),
    pStrLink    ( NULL ),
    pStrFilter  ( NULL ),
    eGraphicPos ( GPOS_NONE ),
    bLoadAgain  ( sal_True )

{
    *this = rItem;
}

// -----------------------------------------------------------------------

SvxBrushItem::~SvxBrushItem()
{
#ifndef SVX_LIGHT
    if( pImpl->xMedium.Is() )
        pImpl->xMedium->SetDoneLink( Link() );
#endif
    delete pImpl->pGraphicObject;
    delete pImpl;
    delete pStrLink;
    delete pStrFilter;
}

// -----------------------------------------------------------------------

sal_uInt16 SvxBrushItem::GetVersion( sal_uInt16 nFileVersion ) const
{
    return BRUSH_GRAPHIC_VERSION;
}

// -----------------------------------------------------------------------

sal_Bool SvxBrushItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    switch( nMemberId)
    {
        case MID_BACK_COLOR:
            rVal <<= (sal_Int32)( aColor.GetColor() );
        break;

        case MID_GRAPHIC_POSITION:
            rVal <<= (style::GraphicLocation)(sal_uInt16)eGraphicPos;
        break;

        case MID_GRAPHIC:
            DBG_ERRORFILE( "not implemented" );
        break;

        case MID_GRAPHIC_TRANSPARENT:
            rVal = Bool2Any( aColor.GetTransparency() );
        break;

        case MID_GRAPHIC_URL:
        {
            OUString sLink;
            if ( pStrLink )
                sLink = *pStrLink;
            rVal <<= sLink;
        }
        break;

        case MID_GRAPHIC_FILTER:
        {
            OUString sFilter;
            if ( pStrFilter )
                sFilter = *pStrFilter;
            rVal <<= sFilter;
        }
        break;
    }

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxBrushItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    switch( nMemberId)
    {
        case MID_BACK_COLOR:
        {
            sal_Int32 nCol;
            if ( !( rVal >>= nCol ) )
                return sal_False;
            aColor = Color( nCol );
        }
        break;

        case MID_GRAPHIC_POSITION:
        {
            style::GraphicLocation eLocation;
            if ( !( rVal>>=eLocation ) )
            {
                sal_Int32 nValue;
                if ( !( rVal >>= nValue ) )
                    return sal_False;
                eLocation = (style::GraphicLocation)nValue;
            }
            SetGraphicPos( (SvxGraphicPosition)(sal_uInt16)eLocation );
        }
        break;

        case MID_GRAPHIC:
            DBG_ERRORFILE( "not implemented" );
        break;

        case MID_GRAPHIC_TRANSPARENT:
            aColor.SetTransparency( Any2Bool( rVal ) ? 0xff : 0 );
        break;

        case MID_GRAPHIC_URL:
        {
            if ( rVal.getValueType() == ::getCppuType( (OUString*)0 ) )
            {
                OUString sLink;
                rVal >>= sLink;
                SetGraphicLink(sLink);
                if ( sLink.getLength() && eGraphicPos == GPOS_NONE )
                    eGraphicPos = GPOS_MM;
                else if( !sLink.getLength() )
                    eGraphicPos = GPOS_NONE;
            }
        }
        break;

        case MID_GRAPHIC_FILTER:
        {
            if( rVal.getValueType() == ::getCppuType( (OUString*)0 ) )
            {
                OUString sLink;
                rVal >>= sLink;
                SetGraphicFilter( sLink );
            }
        }
        break;
    }

    return sal_True;
}

// -----------------------------------------------------------------------

#ifndef SVX_LIGHT

SvXMLEnumMapEntry psXML_BrushRepeat[] =
{
    { sXML_background_repeat,       GPOS_TILED  },
    { sXML_background_no_repeat,    GPOS_MM     },
    { sXML_background_stretch,      GPOS_AREA   },
    { 0,                            0           }
};

SvXMLEnumMapEntry psXML_BrushHoriPos[] =
{
    { sXML_left,        GPOS_LM },
    { sXML_right,       GPOS_RM },
    { 0,                            0           }
};

SvXMLEnumMapEntry psXML_BrushVertPos[] =
{
    { sXML_top,         GPOS_MT },
    { sXML_bottom,      GPOS_MB },
    { 0,                            0           }
};

void lcl_frmitems_MergeXMLHoriPos( SvxGraphicPosition& ePos,
                                   SvxGraphicPosition eHori )
{
    DBG_ASSERT( GPOS_LM==eHori || GPOS_MM==eHori || GPOS_RM==eHori,
                "lcl_frmitems_MergeXMLHoriPos: vertical pos must be middle" );

    switch( ePos )
    {
    case GPOS_LT:
    case GPOS_MT:
    case GPOS_RT:
        ePos = GPOS_LM==eHori ? GPOS_LT : (GPOS_MM==eHori ? GPOS_MT : GPOS_RT);
        break;

    case GPOS_LM:
    case GPOS_MM:
    case GPOS_RM:
        ePos = eHori;
        break;

    case GPOS_LB:
    case GPOS_MB:
    case GPOS_RB:
        ePos = GPOS_LM==eHori ? GPOS_LB : (GPOS_MM==eHori ? GPOS_MB : GPOS_RB);
        break;
    }
}

void lcl_frmitems_MergeXMLVertPos( SvxGraphicPosition& ePos,
                                     SvxGraphicPosition eVert )
{
    DBG_ASSERT( GPOS_MT==eVert || GPOS_MM==eVert || GPOS_MB==eVert,
                "lcl_frmitems_MergeXMLVertPos: horizontal pos must be middle" );

    switch( ePos )
    {
    case GPOS_LT:
    case GPOS_LM:
    case GPOS_LB:
        ePos = GPOS_MT==eVert ? GPOS_LT : (GPOS_MM==eVert ? GPOS_LM : GPOS_LB);
        ePos = eVert;
        break;

    case GPOS_MT:
    case GPOS_MM:
    case GPOS_MB:
        ePos = eVert;
        break;

    case GPOS_RT:
    case GPOS_RM:
    case GPOS_RB:
        ePos = GPOS_MT==eVert ? GPOS_RT : (GPOS_MM==eVert ? GPOS_RM : GPOS_RB);
        break;
    }
}

#endif

// note: the graphic is only imported if nMemberId equals MID_GRAPHIC.
//       If not, only the color or transparency is imported
sal_Bool SvxBrushItem::importXML( const OUString& rValue,
                              sal_uInt16 nMemberId,
                              const SvXMLUnitConverter& rUnitConverter )
{
#ifndef SVX_LIGHT

    sal_Bool bRet = sal_False;
    Color aTempColor;
    switch( nMemberId )
    {
    case MID_BACK_COLOR:
        if( 0 == rValue.compareToAscii( sXML_transparent ) )
        {
            aColor.SetTransparency(0xff);
            bRet = sal_True;
        }
        else if( rUnitConverter.convertColor( aTempColor, rValue ) )
        {
            aColor = aTempColor;
            aColor.SetTransparency(0);
            bRet = sal_True;
        }
        break;

    case MID_GRAPHIC_LINK:
        pStrLink = new String( rValue.getStr() );
        if( GPOS_NONE == eGraphicPos )
            eGraphicPos = GPOS_TILED;
        bRet = sal_True ;
        break;

    case MID_GRAPHIC_REPEAT:
        {
            sal_uInt16 nPos = GPOS_NONE;
            if( rUnitConverter.convertEnum( nPos, rValue,
                                            psXML_BrushRepeat ) )
            {
                if( GPOS_MM != nPos || GPOS_NONE == eGraphicPos ||
                    GPOS_AREA == eGraphicPos || GPOS_TILED == eGraphicPos )
                    eGraphicPos = (SvxGraphicPosition)nPos;
                bRet = sal_True;
            }
        }
        break;

    case MID_GRAPHIC_POSITION:
        {
            SvxGraphicPosition ePos = GPOS_NONE, eTmp;
            sal_uInt16 nTmp;
            SvXMLTokenEnumerator aTokenEnum( rValue );
            OUString aToken;
            sal_Bool bHori = sal_False, bVert = sal_False;
            bRet = sal_True;
            while( bRet && aTokenEnum.getNextToken( aToken ) )
            {
                if( bHori && bVert )
                {
                    bRet = sal_False;
                }
                else if( -1 != aToken.indexOf( sal_Unicode('%') ) )
                {
                    long nPrc = 50;
                    if( rUnitConverter.convertPercent( nPrc, aToken ) )
                    {
                        if( !bHori )
                        {
                            ePos = nPrc < 25 ? GPOS_LT : (nPrc < 75 ? GPOS_MM
                                                                      : GPOS_RB);
                            bHori = sal_True;
                        }
                        else
                        {
                            eTmp = nPrc < 25 ? GPOS_LT: (nPrc < 75 ? GPOS_LM
                                                                   : GPOS_LB);
                            lcl_frmitems_MergeXMLVertPos( ePos, eTmp );
                            bVert = sal_True;
                        }
                    }
                    else
                    {
                        // wrong percentage
                        bRet = sal_False;
                    }
                }
                else if( aToken.equalsAsciiL( sXML_center,
                                              sizeof(sXML_center)-1 ) )
                {
                    if( bHori )
                        lcl_frmitems_MergeXMLVertPos( ePos, GPOS_MM );
                    else if ( bVert )
                        lcl_frmitems_MergeXMLHoriPos( ePos, GPOS_MM );
                    else
                        ePos = GPOS_MM;
                }
                else if( rUnitConverter.convertEnum( nTmp, aToken,
                                                     psXML_BrushHoriPos ) )
                {
                    if( bVert )
                        lcl_frmitems_MergeXMLHoriPos(
                                            ePos, (SvxGraphicPosition)nTmp );
                    else if( !bHori )
                        ePos = (SvxGraphicPosition)nTmp;
                    else
                        bRet = sal_False;
                    bHori = sal_True;
                }
                else if( rUnitConverter.convertEnum( nTmp, aToken,
                                                     psXML_BrushVertPos ) )
                {
                    if( bHori )
                        lcl_frmitems_MergeXMLVertPos(
                                            ePos, (SvxGraphicPosition)nTmp );
                    else if( !bVert )
                        ePos = (SvxGraphicPosition)nTmp;
                    else
                        bRet = sal_False;
                    bVert = sal_True;
                }
                else
                {
                    bRet = sal_False;
                }
            }

            bRet &= GPOS_NONE != ePos;
            if( bRet )
                eGraphicPos = ePos;
        }
        break;

    case MID_GRAPHIC_FILTER:
        pStrFilter = new String( rValue.getStr() );
        bRet = sal_True;
        break;
    }

    return bRet;
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------

// note: the graphic is only exported if nMemberId equals MID_GRAPHIC.
//       If not, only the color or transparency is exported
sal_Bool SvxBrushItem::exportXML( OUString& rValue, sal_uInt16 nMemberId, const SvXMLUnitConverter& rUnitConverter ) const
{
#ifndef SVX_LIGHT
    OUStringBuffer aOut;
    sal_Bool bRet = sal_False;

    switch( nMemberId )
    {
    case MID_BACK_COLOR:
        if ( aColor.GetTransparency() )
            aOut.appendAscii( sXML_transparent );
        else
            rUnitConverter.convertColor( aOut, aColor );
        bRet = sal_True;
        break;

    case MID_GRAPHIC_LINK:
        if( eGraphicPos != GPOS_NONE && pStrLink )
        {
            aOut.append( pStrLink->GetBuffer() );
            bRet = sal_True;
        }
        break;

    case MID_GRAPHIC_POSITION:
        switch( eGraphicPos )
        {
        case GPOS_LT:
        case GPOS_MT:
        case GPOS_RT:
            aOut.appendAscii( sXML_top );
            bRet = sal_True;
            break;
        case GPOS_LM:
        case GPOS_MM:
        case GPOS_RM:
            aOut.appendAscii( sXML_center );
            bRet = sal_True;
            break;
        case GPOS_LB:
        case GPOS_MB:
        case GPOS_RB:
            aOut.appendAscii( sXML_bottom );
            bRet = sal_True;
            break;
        }

        if( bRet )
        {
            aOut.append( sal_Unicode( ' ' ) );

            switch( eGraphicPos )
            {
            case GPOS_LT:
            case GPOS_LB:
            case GPOS_LM:
                aOut.appendAscii( sXML_left );
                break;
            case GPOS_MT:
            case GPOS_MM:
            case GPOS_MB:
                aOut.appendAscii( sXML_center );
                break;
            case GPOS_RM:
            case GPOS_RT:
            case GPOS_RB:
                aOut.appendAscii( sXML_right );
                break;
            }
        }
        break;

    case MID_GRAPHIC_REPEAT:
        if( GPOS_AREA == eGraphicPos )
        {
            aOut.appendAscii( sXML_background_stretch  );
            bRet = sal_True;
        }
        else if( GPOS_NONE != eGraphicPos && GPOS_TILED != eGraphicPos  )
        {
            aOut.appendAscii( sXML_background_no_repeat );
            bRet = sal_True;
        }
        break;

    case MID_GRAPHIC_FILTER:
        if( eGraphicPos != GPOS_NONE && pStrFilter )
        {
            aOut.append( pStrFilter->GetBuffer() );
            bRet = sal_True;
        }
        break;
    }

    rValue = aOut.makeStringAndClear();
    return bRet;
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------

SfxItemPresentation SvxBrushItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
#ifndef SVX_LIGHT
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;

        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if ( GPOS_NONE  == eGraphicPos )
            {
                rText = ::GetColorString( aColor );
                rText += cpDelim;
                sal_uInt16 nId = RID_SVXITEMS_TRANSPARENT_FALSE;

                if ( aColor.GetTransparency() )
                    nId = RID_SVXITEMS_TRANSPARENT_TRUE;
                rText += SVX_RESSTR(nId);
            }
            else
            {
                rText = SVX_RESSTR(RID_SVXITEMS_GRAPHIC);
            }

            return ePres;
        }
    }
#endif // !SVX_LIGHT
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SvxBrushItem& SvxBrushItem::operator=( const SvxBrushItem& rItem )
{
    aColor = rItem.aColor;
    eGraphicPos = rItem.eGraphicPos;

    DELETEZ( pImpl->pGraphicObject );
    DELETEZ( pStrLink );
    DELETEZ( pStrFilter );

    if ( GPOS_NONE != eGraphicPos )
    {
        if ( rItem.pStrLink )
            pStrLink = new String( *rItem.pStrLink );
        if ( rItem.pStrFilter )
            pStrFilter = new String( *rItem.pStrFilter );
        if ( rItem.pImpl->pGraphicObject )
            pImpl->pGraphicObject = new GraphicObject( *rItem.pImpl->pGraphicObject );
    }
    return *this;
}

// -----------------------------------------------------------------------

int SvxBrushItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    SvxBrushItem& rCmp = (SvxBrushItem&)rAttr;
    sal_Bool bEqual = ( aColor == rCmp.aColor && eGraphicPos == rCmp.eGraphicPos );

    if ( bEqual )
    {
        if ( GPOS_NONE != eGraphicPos )
        {
            if ( !rCmp.pStrLink )
                bEqual = !pStrLink;
            else
                bEqual = pStrLink && ( *pStrLink == *rCmp.pStrLink );

            if ( bEqual )
            {
                if ( !rCmp.pStrFilter )
                    bEqual = !pStrFilter;
                else
                    bEqual = pStrFilter && ( *pStrFilter == *rCmp.pStrFilter );
            }

            if ( bEqual && !rCmp.pStrLink )
            {
                if ( !rCmp.pImpl->pGraphicObject )
                    bEqual = !pImpl->pGraphicObject;
                else
                    bEqual = pImpl->pGraphicObject &&
                             ( *pImpl->pGraphicObject == *rCmp.pImpl->pGraphicObject );
            }
        }
    }

    return bEqual;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxBrushItem::Clone( SfxItemPool* ) const
{
    return new SvxBrushItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxBrushItem::Create( SvStream& rStream, sal_uInt16 nVersion ) const
{
    return new SvxBrushItem( rStream, nVersion, Which() );
}

// -----------------------------------------------------------------------

SvStream& SvxBrushItem::Store( SvStream& rStream , sal_uInt16 nItemVersion ) const
{
    rStream << (sal_Bool)sal_False;
    rStream << aColor;
    rStream << aColor;
    rStream << (sal_Int8)(aColor.GetTransparency() > 0 ? 0 : 1); //BRUSH_NULL : BRUSH_SOLID

    sal_uInt16 nDoLoad = 0;

    if ( pImpl->pGraphicObject && !pStrLink )
        nDoLoad |= LOAD_GRAPHIC;
    if ( pStrLink )
        nDoLoad |= LOAD_LINK;
    if ( pStrFilter )
        nDoLoad |= LOAD_FILTER;
    rStream << nDoLoad;

    if ( pImpl->pGraphicObject && !pStrLink )
        rStream << pImpl->pGraphicObject->GetGraphic();
    if ( pStrLink )
    {
        String aRel = INetURLObject::AbsToRel( *pStrLink );
        // UNICODE: rStream << aRel;
        rStream.WriteByteString(aRel);
    }
    if ( pStrFilter )
    {
        // UNICODE: rStream << *pStrFilter;
        rStream.WriteByteString(*pStrFilter);
    }
    rStream << (sal_Int8)eGraphicPos;
    return rStream;
}

// -----------------------------------------------------------------------
// const wegcasten, da const als logisches const zu verstehen ist
// wenn GetGraphic() gerufen wird, soll sich das Item darum kuemmern,
// eine gelinkte Grafik zu holen.


GraphicFilter* GetGrfFilter();

IMPL_STATIC_LINK( SvxBrushItem, DoneHdl_Impl, void*, EMPTYARG )
{
#ifndef SVX_LIGHT
    pThis->pImpl->pGraphicObject = new GraphicObject;
    SvStream* pStream = pThis->pImpl->xMedium->GetInStream();
    if( pStream && !pStream->GetError() )
    {
        Graphic aGraphic;
        int nRes;
        pStream->Seek( STREAM_SEEK_TO_BEGIN );
        nRes = GetGrfFilter()->
            ImportGraphic( aGraphic, *pThis->pStrLink, *pStream );
        if( nRes != GRFILTER_OK )
        {
            DELETEZ( pThis->pImpl->pGraphicObject );
            pThis->bLoadAgain = sal_False;
        }
        else
            pThis->pImpl->pGraphicObject->SetGraphic( aGraphic );
    }
    else
        pThis->bLoadAgain = sal_False;

    pThis->pImpl->xMedium.Clear();
    pThis->pImpl->aDoneLink.Call( pThis );
#endif
    return 0;
}

// -----------------------------------------------------------------------

void SvxBrushItem::PurgeGraphic() const
{
#ifndef SVX_LIGHT
    PurgeMedium();
    DELETEZ( pImpl->pGraphicObject );
    ((SvxBrushItem*)this)->bLoadAgain = sal_True;
#endif
}

// -----------------------------------------------------------------------

void SvxBrushItem::PurgeMedium() const
{
#ifndef SVX_LIGHT
    pImpl->xMedium.Clear();
#endif
}

// -----------------------------------------------------------------------

const GraphicObject* SvxBrushItem::GetGraphicObject( SfxObjectShell* pSh ) const
{
#ifndef SVX_LIGHT
    if ( bLoadAgain && pStrLink && !pImpl->pGraphicObject && !pImpl->xMedium.Is() )
    // wenn Grafik schon geladen, als Cache benutzen
    {
        if( pSh && pSh->IsAbortingImport() )
        {
            SvxBrushItem* pThis = (SvxBrushItem*)this;
            pThis->bLoadAgain = sal_False;
            return 0;
        }
        pImpl->xMedium = new SfxMedium(
            *pStrLink, STREAM_STD_READ, sal_False );

        pImpl->xMedium->SetTransferPriority( SFX_TFPRIO_VISIBLE_LOWRES_GRAPHIC );
        if( pImpl->xMedium->IsRemote() )
        {
            if( pSh )
                pSh->RegisterTransfer( *pImpl->xMedium );
            else
                DBG_WARNING( "SvxBrushItem::GetGraphic ohne DocShell" );
        }

        SfxMediumRef xRef( pImpl->xMedium );
        // Ref halten wg. synchr. DoneCallback
        if( pImpl->aDoneLink.IsSet() )
        {
            // Auf besonderen Wunsch des Writers wird der synchrone und der
            // asynchrone Fall was die Benachrichtigung angeht unterschiedlich
            // behandelt. Der Callback erfolgt nur bei asynchronem Eintreffen
            // der Daten

            Link aTmp = pImpl->aDoneLink;
            pImpl->aDoneLink = Link();
            pImpl->xMedium->DownLoad(
                STATIC_LINK( this, SvxBrushItem, DoneHdl_Impl ) );
            pImpl->aDoneLink = aTmp;
        }
        else
        {
            pImpl->xMedium->DownLoad( );
            DoneHdl_Impl( (SvxBrushItem*)this, 0 );
        }
    }
#endif
    return pImpl->pGraphicObject;
}

// -----------------------------------------------------------------------

const Graphic* SvxBrushItem::GetGraphic( SfxObjectShell* pSh ) const
{
    const GraphicObject* pGrafObj = GetGraphicObject( pSh );
    return( pGrafObj ? &( pGrafObj->GetGraphic() ) : NULL );
}

// -----------------------------------------------------------------------

void SvxBrushItem::SetGraphicPos( SvxGraphicPosition eNew )
{
    eGraphicPos = eNew;

    if ( GPOS_NONE == eGraphicPos )
    {
        DELETEZ( pImpl->pGraphicObject );
        DELETEZ( pStrLink );
        DELETEZ( pStrFilter );
    }
    else
    {
        if ( !pImpl->pGraphicObject && !pStrLink )
        {
            pImpl->pGraphicObject = new GraphicObject; // dummy anlegen
        }
    }
}

// -----------------------------------------------------------------------

void SvxBrushItem::SetGraphic( const Graphic& rNew )
{
    if ( !pStrLink )
    {
        if ( pImpl->pGraphicObject )
            pImpl->pGraphicObject->SetGraphic( rNew );
        else
            pImpl->pGraphicObject = new GraphicObject( rNew );

        if ( GPOS_NONE == eGraphicPos )
            eGraphicPos = GPOS_MM; // None waere Brush, also Default: Mitte
    }
    else
        DBG_ERROR( "SetGraphic() on linked graphic! :-/" );
}

// -----------------------------------------------------------------------

void SvxBrushItem::SetGraphicObject( const GraphicObject& rNewObj )
{
    if ( !pStrLink )
    {
        if ( pImpl->pGraphicObject )
            *pImpl->pGraphicObject = rNewObj;
        else
            pImpl->pGraphicObject = new GraphicObject( rNewObj );

        if ( GPOS_NONE == eGraphicPos )
            eGraphicPos = GPOS_MM; // None waere Brush, also Default: Mitte
    }
    else
        DBG_ERROR( "SetGraphic() on linked graphic! :-/" );
}

// -----------------------------------------------------------------------

void SvxBrushItem::SetGraphicLink( const String& rNew )
{
    if ( !rNew.Len() )
        DELETEZ( pStrLink );
    else
    {
        if ( pStrLink )
            *pStrLink = rNew;
        else
            pStrLink = new String( rNew );

        DELETEZ( pImpl->pGraphicObject );
    }
}

// -----------------------------------------------------------------------

void SvxBrushItem::SetGraphicFilter( const String& rNew )
{
    if ( !rNew.Len() )
        DELETEZ( pStrFilter );
    else
    {
        if ( pStrFilter )
            *pStrFilter = rNew;
        else
            pStrFilter = new String( rNew );
    }
}

//static
SvxGraphicPosition SvxBrushItem::WallpaperStyle2GraphicPos( WallpaperStyle eStyle )
{
    SvxGraphicPosition eResult;
    // der Switch ist nicht der schnellste, dafuer aber am sichersten
    switch( eStyle )
    {
        case WALLPAPER_NULL: eResult = GPOS_NONE; break;
        case WALLPAPER_TILE: eResult = GPOS_TILED; break;
        case WALLPAPER_CENTER: eResult = GPOS_MM; break;
        case WALLPAPER_SCALE: eResult = GPOS_AREA; break;
        case WALLPAPER_TOPLEFT: eResult = GPOS_LT; break;
        case WALLPAPER_TOP: eResult = GPOS_MT; break;
        case WALLPAPER_TOPRIGHT: eResult = GPOS_RT; break;
        case WALLPAPER_LEFT: eResult = GPOS_LM; break;
        case WALLPAPER_RIGHT: eResult = GPOS_RM; break;
        case WALLPAPER_BOTTOMLEFT: eResult = GPOS_LB; break;
        case WALLPAPER_BOTTOM: eResult = GPOS_MB; break;
        case WALLPAPER_BOTTOMRIGHT: eResult = GPOS_RB; break;
        default: eResult = GPOS_NONE;
    }
    return eResult;
};

//static
WallpaperStyle SvxBrushItem::GraphicPos2WallpaperStyle( SvxGraphicPosition ePos )
{
    WallpaperStyle eResult;
    switch( ePos )
    {
        case GPOS_NONE: eResult = WALLPAPER_NULL; break;
        case GPOS_TILED: eResult = WALLPAPER_TILE; break;
        case GPOS_MM: eResult = WALLPAPER_CENTER; break;
        case GPOS_AREA: eResult = WALLPAPER_SCALE; break;
        case GPOS_LT: eResult = WALLPAPER_TOPLEFT; break;
        case GPOS_MT: eResult = WALLPAPER_TOP; break;
        case GPOS_RT: eResult = WALLPAPER_TOPRIGHT; break;
        case GPOS_LM: eResult = WALLPAPER_LEFT; break;
        case GPOS_RM: eResult = WALLPAPER_RIGHT; break;
        case GPOS_LB: eResult = WALLPAPER_BOTTOMLEFT; break;
        case GPOS_MB: eResult = WALLPAPER_BOTTOM; break;
        case GPOS_RB: eResult = WALLPAPER_BOTTOMRIGHT; break;
        default: eResult = WALLPAPER_NULL;
    }
    return eResult;
}


SvxBrushItem::SvxBrushItem( const CntWallpaperItem& rItem, sal_uInt16 nWhich ) :
    SfxPoolItem( nWhich ),
    pImpl( new SvxBrushItem_Impl( 0 ) ),
    bLoadAgain( sal_True ),
    pStrLink(0),
    pStrFilter(0)
{
    aColor = rItem.GetColor();

    if( rItem.GetBitmapURL().Len() )
    {
        pStrLink    = new String( rItem.GetBitmapURL() );
        SetGraphicPos( WallpaperStyle2GraphicPos((WallpaperStyle)rItem.GetStyle() ) );
    }
}

CntWallpaperItem* SvxBrushItem::CreateCntWallpaperItem() const
{
    CntWallpaperItem* pItem = new CntWallpaperItem( 0 );
    pItem->SetColor( aColor.GetColor() );
    pItem->SetStyle( GraphicPos2WallpaperStyle( GetGraphicPos() ) );
    sal_Bool bLink = (pStrLink != 0);
    if( bLink )
    {
        String aURL = *pStrLink;
        pItem->SetBitmapURL( aURL );
    }
    if( pImpl->pGraphicObject )
        DBG_ERRORFILE( "Don't know what to do with a graphic" );
//      pItem->SetGraphic( *pImpl->pGraphic, bLink );

    return pItem;
}


