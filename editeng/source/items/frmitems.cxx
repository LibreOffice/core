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

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/table/ShadowLocation.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/table/TableOrientation.hpp>
#include <com/sun/star/util/SortField.hpp>
#include <com/sun/star/util/SortFieldType.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/table/CellOrientation.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Selection.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/frame/status/UpperLowerMarginScale.hpp>

#include <unotools/ucbstreamhelper.hxx>
#include <limits.h>
#include <comphelper/processfactory.hxx>
#include <svtools/grfmgr.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/types.hxx>
#include <svl/memberid.hrc>
#include <svtools/wallitem.hxx>
#include <svl/cntwall.hxx>
#include <svtools/borderhelper.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <svtools/filter.hxx>
#include <editeng/editids.hrc>
#include <editeng/editrids.hrc>
#include <editeng/pbinitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/prntitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brkitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/bolnitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/itemtype.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/memberids.hrc>
#include <editeng/editerr.hxx>

using namespace ::editeng;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::table::BorderLineStyle;


// Conversion for UNO
#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)    ((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))
#define TWIP_TO_MM100_UNSIGNED(TWIP)     ((((TWIP)*127L+36L)/72L))
#define MM100_TO_TWIP_UNSIGNED(MM100)    ((((MM100)*72L+63L)/127L))

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

/*
SvxBorderLine is not an SfxPoolItem, and has no Store/Create serialization/deserialization methods.
Since border line information needs to be serialized by the table autoformat code, these file-local
methods are defined to encapsulate the necessary serialization logic.
*/
namespace
{
    /// Item version for saved border lines. The old version saves the line without style information.
    const int BORDER_LINE_OLD_VERSION = 0;
    /// Item version for saved border lies. The new version includes line style.
    const int BORDER_LINE_WITH_STYLE_VERSION = 1;

    /// Store a border line to a stream.
    SvStream& StoreBorderLine(SvStream &stream, const SvxBorderLine &l, sal_uInt16 version)
    {
        stream << l.GetColor()
               << l.GetOutWidth()
               << l.GetInWidth()
               << l.GetDistance();

        if (version >= BORDER_LINE_WITH_STYLE_VERSION)
               stream << static_cast<sal_uInt16>(l.GetBorderLineStyle());

        return stream;
    }

    /// Creates a border line from a stream.
    SvxBorderLine CreateBorderLine(SvStream &stream, sal_uInt16 version)
    {
        sal_uInt16 nOutline, nInline, nDistance;
        sal_uInt16 nStyle = NONE;
        Color aColor;
        stream >> aColor >> nOutline >> nInline >> nDistance;

        if (version >= BORDER_LINE_WITH_STYLE_VERSION)
            stream >> nStyle;

        SvxBorderLine border(&aColor);
        border.GuessLinesWidths(nStyle, nOutline, nInline, nDistance);
        return border;
    }

    /// Retrieves a BORDER_LINE_* version from a BOX_BORDER_* version.
    sal_uInt16 BorderLineVersionFromBoxVersion(sal_uInt16 boxVersion)
    {
        return (boxVersion >= BOX_BORDER_STYLE_VERSION)? BORDER_LINE_WITH_STYLE_VERSION : BORDER_LINE_OLD_VERSION;
    }
}

TYPEINIT1_FACTORY(SvxPaperBinItem, SfxByteItem, new SvxPaperBinItem(0));
TYPEINIT1_FACTORY(SvxSizeItem, SfxPoolItem, new SvxSizeItem(0));
TYPEINIT1_FACTORY(SvxLRSpaceItem, SfxPoolItem, new SvxLRSpaceItem(0));
TYPEINIT1_FACTORY(SvxULSpaceItem, SfxPoolItem, new SvxULSpaceItem(0));
TYPEINIT1_FACTORY(SvxPrintItem, SfxBoolItem, new SvxPrintItem(0));
TYPEINIT1_FACTORY(SvxOpaqueItem, SfxBoolItem, new SvxOpaqueItem(0));
TYPEINIT1_FACTORY(SvxProtectItem, SfxPoolItem, new SvxProtectItem(0));
TYPEINIT1_FACTORY(SvxBrushItem, SfxPoolItem, new SvxBrushItem(0));
TYPEINIT1_FACTORY(SvxShadowItem, SfxPoolItem, new SvxShadowItem(0));
TYPEINIT1_FACTORY(SvxBoxItem, SfxPoolItem, new SvxBoxItem(0));
TYPEINIT1_FACTORY(SvxBoxInfoItem, SfxPoolItem, new SvxBoxInfoItem(0));
TYPEINIT1_FACTORY(SvxFmtBreakItem, SfxEnumItem, new SvxFmtBreakItem(SVX_BREAK_NONE, 0));
TYPEINIT1_FACTORY(SvxFmtKeepItem, SfxBoolItem, new SvxFmtKeepItem(sal_False, 0));
TYPEINIT1_FACTORY(SvxLineItem, SfxPoolItem, new SvxLineItem(0));
TYPEINIT1_FACTORY(SvxFrameDirectionItem, SfxUInt16Item, new SvxFrameDirectionItem(FRMDIR_HORI_LEFT_TOP, 0));

// class SvxPaperBinItem ------------------------------------------------

SfxPoolItem* SvxPaperBinItem::Clone( SfxItemPool* ) const
{
    return new SvxPaperBinItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxPaperBinItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
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
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;

        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = String::CreateFromInt32( GetValue() );
            return SFX_ITEM_PRESENTATION_NAMELESS;

        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            sal_uInt8 nValue = GetValue();

            if ( PAPERBIN_PRINTER_SETTINGS == nValue )
                rText = EE_RESSTR(RID_SVXSTR_PAPERBIN_SETTINGS);
            else
            {
                rText = EE_RESSTR(RID_SVXSTR_PAPERBIN);
                rText += sal_Unicode(' ');
                rText += String::CreateFromInt32( nValue );
            }
            return SFX_ITEM_PRESENTATION_COMPLETE;
        }
        //no break necessary
        default: ;//prevent warning
    }

    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxSizeItem -----------------------------------------------------

SvxSizeItem::SvxSizeItem( const sal_uInt16 nId, const Size& rSize ) :

    SfxPoolItem( nId ),

    aSize( rSize )
{
}

// -----------------------------------------------------------------------
bool SvxSizeItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    awt::Size aTmp(aSize.Width(), aSize.Height());
    if( bConvert )
    {
        aTmp.Height = TWIP_TO_MM100(aTmp.Height);
        aTmp.Width = TWIP_TO_MM100(aTmp.Width);
    }

    switch( nMemberId )
    {
        case MID_SIZE_SIZE:  rVal <<= aTmp; break;
        case MID_SIZE_WIDTH: rVal <<= aTmp.Width; break;
        case MID_SIZE_HEIGHT: rVal <<= aTmp.Height;  break;
        default: OSL_FAIL("Wrong MemberId!"); return false;
    }

    return true;
}
// -----------------------------------------------------------------------
bool SvxSizeItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    switch( nMemberId )
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
                return false;
            }
        }
        break;
        case MID_SIZE_WIDTH:
        {
            sal_Int32 nVal = 0;
            if(!(rVal >>= nVal ))
                return false;

            aSize.Width() = bConvert ? MM100_TO_TWIP(nVal) : nVal;
        }
        break;
        case MID_SIZE_HEIGHT:
        {
            sal_Int32 nVal = 0;
            if(!(rVal >>= nVal))
                return true;

            aSize.Height() = bConvert ? MM100_TO_TWIP(nVal) : nVal;
        }
        break;
        default: OSL_FAIL("Wrong MemberId!");
            return false;
    }
    return true;
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
    XubString&          rText, const IntlWrapper *pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;

        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = GetMetricText( aSize.Width(), eCoreUnit, ePresUnit, pIntl );
            rText += cpDelim;
            rText += GetMetricText( aSize.Height(), eCoreUnit, ePresUnit, pIntl );
            return SFX_ITEM_PRESENTATION_NAMELESS;

        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = EE_RESSTR(RID_SVXITEMS_SIZE_WIDTH);
            rText += GetMetricText( aSize.Width(), eCoreUnit, ePresUnit, pIntl );
            rText += EE_RESSTR(GetMetricId(ePresUnit));
            rText += cpDelim;
            rText += EE_RESSTR(RID_SVXITEMS_SIZE_HEIGHT);
            rText += GetMetricText( aSize.Height(), eCoreUnit, ePresUnit, pIntl );
            rText += EE_RESSTR(GetMetricId(ePresUnit));
            return SFX_ITEM_PRESENTATION_COMPLETE;
        //no break necessary
        default: ;//prevent warning

    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SvStream& SvxSizeItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    //#fdo39428 SvStream no longer supports operator<<(long)
    rStrm << sal::static_int_cast<sal_Int32>(aSize.Width());
    rStrm << sal::static_int_cast<sal_Int32>(aSize.Height());
    return rStrm;
}

// -----------------------------------------------------------------------

bool SvxSizeItem::ScaleMetrics( long nMult, long nDiv )
{
    aSize.Width() = Scale( aSize.Width(), nMult, nDiv );
    aSize.Height() = Scale( aSize.Height(), nMult, nDiv );
    return true;
}

// -----------------------------------------------------------------------

bool SvxSizeItem::HasMetrics() const
{
    return true;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxSizeItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    //#fdo39428 SvStream no longer supports operator>>(long&)
    sal_Int32 nWidth(0), nHeight(0);
    rStrm >> nWidth >> nHeight;

    SvxSizeItem* pAttr = new SvxSizeItem( Which() );
    pAttr->SetSize(Size(nWidth, nHeight));

    return pAttr;
}

// class SvxLRSpaceItem --------------------------------------------------

SvxLRSpaceItem::SvxLRSpaceItem( const sal_uInt16 nId ) :

    SfxPoolItem( nId ),

    nFirstLineOfst  ( 0 ),
    nTxtLeft        ( 0 ),
    nLeftMargin     ( 0 ),
    nRightMargin    ( 0 ),
    nPropFirstLineOfst( 100 ),
    nPropLeftMargin( 100 ),
    nPropRightMargin( 100 ),
    bAutoFirst      ( 0 )
{
}

// -----------------------------------------------------------------------

SvxLRSpaceItem::SvxLRSpaceItem( const long nLeft, const long nRight,
                                const long nTLeft, const short nOfset,
                                const sal_uInt16 nId ) :

    SfxPoolItem( nId ),

    nFirstLineOfst  ( nOfset ),
    nTxtLeft        ( nTLeft ),
    nLeftMargin     ( nLeft ),
    nRightMargin    ( nRight ),
    nPropFirstLineOfst( 100 ),
    nPropLeftMargin( 100 ),
    nPropRightMargin( 100 ),
    bAutoFirst      ( 0 )
{
}

// -----------------------------------------------------------------------
bool SvxLRSpaceItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bRet = true;
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        // now all signed
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
            bRet = false;
            OSL_FAIL("unknown MemberId");
    }
    return bRet;
}

// -----------------------------------------------------------------------
bool SvxLRSpaceItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    sal_Int32 nVal = 0;
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
            sal_Int32 nRel = 0;
            if((rVal >>= nRel) && nRel >= 0 && nRel < USHRT_MAX)
            {
                if(MID_L_REL_MARGIN== nMemberId)
                    nPropLeftMargin = (sal_uInt16)nRel;
                else
                    nPropRightMargin = (sal_uInt16)nRel;
            }
            else
                return false;
        }
        break;
        case MID_FIRST_LINE_INDENT     :
            SetTxtFirstLineOfst((short)(bConvert ?  MM100_TO_TWIP(nVal) : nVal));
            break;

        case MID_FIRST_LINE_REL_INDENT:
            SetPropTxtFirstLineOfst ( (sal_uInt16)nVal );
            break;

        case MID_FIRST_AUTO:
            SetAutoFirst( Any2Bool(rVal) );
            break;

        default:
            OSL_FAIL("unknown MemberId");
            return false;
    }
    return true;
}

// -----------------------------------------------------------------------

// Adapt nLeftMargin and nTxtLeft.

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
    XubString&          rText, const IntlWrapper* pIntl
)   const
{
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
                                       eCoreUnit, ePresUnit, pIntl );
            rText += cpDelim;
            if ( 100 != nPropFirstLineOfst )
                ( rText += String::CreateFromInt32( nPropFirstLineOfst )) += sal_Unicode('%');
            else
                rText += GetMetricText( (long)nFirstLineOfst,
                                        eCoreUnit, ePresUnit, pIntl );
            rText += cpDelim;
            if ( 100 != nRightMargin )
                ( rText += String::CreateFromInt32( nRightMargin )) += sal_Unicode('%');
            else
                rText += GetMetricText( (long)nRightMargin,
                                        eCoreUnit, ePresUnit, pIntl );
            return SFX_ITEM_PRESENTATION_NAMELESS;
        }
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = EE_RESSTR(RID_SVXITEMS_LRSPACE_LEFT);
            if ( 100 != nPropLeftMargin )
                ( rText += String::CreateFromInt32( nPropLeftMargin )) += sal_Unicode('%');
            else
            {
                rText += GetMetricText( (long)nLeftMargin,
                                       eCoreUnit, ePresUnit, pIntl );
                rText += EE_RESSTR(GetMetricId(ePresUnit));
            }
            rText += cpDelim;
            if ( 100 != nPropFirstLineOfst || nFirstLineOfst )
            {
                rText += EE_RESSTR(RID_SVXITEMS_LRSPACE_FLINE);
                if ( 100 != nPropFirstLineOfst )
                    ( rText += String::CreateFromInt32( nPropFirstLineOfst ))
                            += sal_Unicode('%');
                else
                {
                    rText += GetMetricText( (long)nFirstLineOfst,
                                            eCoreUnit, ePresUnit, pIntl );
                    rText += EE_RESSTR(GetMetricId(ePresUnit));
                }
                rText += cpDelim;
            }
            rText += EE_RESSTR(RID_SVXITEMS_LRSPACE_RIGHT);
            if ( 100 != nPropRightMargin )
                ( rText += String::CreateFromInt32( nPropRightMargin )) += sal_Unicode('%');
            else
            {
                rText += GetMetricText( (long)nRightMargin,
                                        eCoreUnit, ePresUnit, pIntl );
                rText += EE_RESSTR(GetMetricId(ePresUnit));
            }
            return SFX_ITEM_PRESENTATION_COMPLETE;
        }
        default: ;//prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

// BulletFI: Before 501 in the Outliner the bullet was not on the position of
// the FI, so in older documents one must set FI to 0.
#define BULLETLR_MARKER 0x599401FE

SvStream& SvxLRSpaceItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    short nSaveFI = nFirstLineOfst;
    ((SvxLRSpaceItem*)this)->SetTxtFirstLineOfst( 0 );  // nLeftMargin is manipulated together with this, see Create()

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

        // From 6.0 onwards, do not write Magic numbers...
        DBG_ASSERT( rStrm.GetVersion() <= SOFFICE_FILEFORMAT_50, "Change File format SvxLRSpaceItem!" );
        rStrm << (sal_uInt32) BULLETLR_MARKER;
        rStrm << nSaveFI;

        if( 0x80 & nAutoFirst )
        {
            rStrm << static_cast<sal_Int32>(nLeftMargin);
            rStrm << static_cast<sal_Int32>(nRightMargin);
        }
    }

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
                left = left + static_cast<sal_uInt16>(firstline);   // see below: txtleft = ...
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

bool SvxLRSpaceItem::ScaleMetrics( long nMult, long nDiv )
{
    nFirstLineOfst = (short)Scale( nFirstLineOfst, nMult, nDiv );
    nTxtLeft = Scale( nTxtLeft, nMult, nDiv );
    nLeftMargin = Scale( nLeftMargin, nMult, nDiv );
    nRightMargin = Scale( nRightMargin, nMult, nDiv );
    return true;
}

// -----------------------------------------------------------------------

bool SvxLRSpaceItem::HasMetrics() const
{
    return true;
}

// class SvxULSpaceItem --------------------------------------------------

SvxULSpaceItem::SvxULSpaceItem( const sal_uInt16 nId )
    : SfxPoolItem(nId)
    , nUpper(0)
    , nLower(0)
    , bContext(false)
    , nPropUpper(100)
    , nPropLower(100)
{
}

// -----------------------------------------------------------------------

SvxULSpaceItem::SvxULSpaceItem( const sal_uInt16 nUp, const sal_uInt16 nLow,
                                const sal_uInt16 nId )
    : SfxPoolItem(nId)
    , nUpper(nUp)
    , nLower(nLow)
    , bContext(false)
    , nPropUpper(100)
    , nPropLower(100)
{
}

// -----------------------------------------------------------------------
bool    SvxULSpaceItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        // now all signed
        case 0:
        {
            ::com::sun::star::frame::status::UpperLowerMarginScale aUpperLowerMarginScale;
            aUpperLowerMarginScale.Upper = (sal_Int32)(bConvert ? TWIP_TO_MM100_UNSIGNED(nUpper) : nUpper);
            aUpperLowerMarginScale.Lower = (sal_Int32)(bConvert ? TWIP_TO_MM100_UNSIGNED(nLower) : nPropUpper);
            aUpperLowerMarginScale.ScaleUpper = (sal_Int16)nPropUpper;
            aUpperLowerMarginScale.ScaleLower = (sal_Int16)nPropLower;
            rVal <<= aUpperLowerMarginScale;
            break;
        }
        case MID_UP_MARGIN: rVal <<= (sal_Int32)(bConvert ? TWIP_TO_MM100_UNSIGNED(nUpper) : nUpper); break;
        case MID_LO_MARGIN: rVal <<= (sal_Int32)(bConvert ? TWIP_TO_MM100_UNSIGNED(nLower) : nLower); break;
        case MID_CTX_MARGIN: rVal <<= bContext; break;
        case MID_UP_REL_MARGIN: rVal <<= (sal_Int16) nPropUpper; break;
        case MID_LO_REL_MARGIN: rVal <<= (sal_Int16) nPropLower; break;
    }
    return true;
}

// -----------------------------------------------------------------------
bool SvxULSpaceItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    sal_Int32 nVal = 0;
    sal_Bool bVal = 0;
    switch( nMemberId )
    {
        case 0:
        {
            ::com::sun::star::frame::status::UpperLowerMarginScale aUpperLowerMarginScale;
            if ( !(rVal >>= aUpperLowerMarginScale ))
                return false;
            {
                SetUpper((sal_uInt16)(bConvert ? MM100_TO_TWIP( aUpperLowerMarginScale.Upper ) : aUpperLowerMarginScale.Upper));
                SetLower((sal_uInt16)(bConvert ? MM100_TO_TWIP( aUpperLowerMarginScale.Lower ) : aUpperLowerMarginScale.Lower));
                if( aUpperLowerMarginScale.ScaleUpper > 1 )
                    nPropUpper = aUpperLowerMarginScale.ScaleUpper;
                if( aUpperLowerMarginScale.ScaleLower > 1 )
                    nPropUpper = aUpperLowerMarginScale.ScaleLower;
            }
        }

        case MID_UP_MARGIN :
            if(!(rVal >>= nVal) || nVal < 0)
                return false;
            SetUpper((sal_uInt16)(bConvert ? MM100_TO_TWIP(nVal) : nVal));
            break;
        case MID_LO_MARGIN :
            if(!(rVal >>= nVal) || nVal < 0)
                return false;
            SetLower((sal_uInt16)(bConvert ? MM100_TO_TWIP(nVal) : nVal));
            break;
        case MID_CTX_MARGIN :
            if (!(rVal >>= bVal))
                return false;
            SetContextValue(bVal);
            break;
        case MID_UP_REL_MARGIN:
        case MID_LO_REL_MARGIN:
        {
            sal_Int32 nRel = 0;
            if((rVal >>= nRel) && nRel > 1 )
            {
                if(MID_UP_REL_MARGIN == nMemberId)
                    nPropUpper = (sal_uInt16)nRel;
                else
                    nPropLower = (sal_uInt16)nRel;
            }
            else
                return false;
        }
        break;

        default:
            OSL_FAIL("unknown MemberId");
            return false;
    }
    return true;
}

// -----------------------------------------------------------------------

int SvxULSpaceItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return ( nUpper == ( (SvxULSpaceItem&)rAttr ).nUpper &&
             nLower == ( (SvxULSpaceItem&)rAttr ).nLower &&
             bContext == ( (SvxULSpaceItem&)rAttr ).bContext &&
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
    XubString&          rText, const IntlWrapper *pIntl
)   const
{
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
                rText = GetMetricText( (long)nUpper, eCoreUnit, ePresUnit, pIntl );
            rText += cpDelim;
            if ( 100 != nPropLower )
                ( rText += String::CreateFromInt32( nPropLower )) += sal_Unicode('%');
            else
                rText += GetMetricText( (long)nLower, eCoreUnit, ePresUnit, pIntl );
            return SFX_ITEM_PRESENTATION_NAMELESS;
        }
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = EE_RESSTR(RID_SVXITEMS_ULSPACE_UPPER);
            if ( 100 != nPropUpper )
                ( rText += String::CreateFromInt32( nPropUpper )) += sal_Unicode('%');
            else
            {
                rText += GetMetricText( (long)nUpper, eCoreUnit, ePresUnit, pIntl );
                rText += EE_RESSTR(GetMetricId(ePresUnit));
            }
            rText += cpDelim;
            rText += EE_RESSTR(RID_SVXITEMS_ULSPACE_LOWER);
            if ( 100 != nPropLower )
                ( rText += String::CreateFromInt32( nPropLower )) += sal_Unicode('%');
            else
            {
                rText += GetMetricText( (long)nLower, eCoreUnit, ePresUnit, pIntl );
                rText += EE_RESSTR(GetMetricId(ePresUnit));
            }
            return SFX_ITEM_PRESENTATION_COMPLETE;
        }
        default: ;//prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SvStream& SvxULSpaceItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
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

sal_uInt16 SvxULSpaceItem::GetVersion( sal_uInt16 /*nFileVersion*/ ) const
{
    return ULSPACE_16_VERSION;
}

// -----------------------------------------------------------------------

bool SvxULSpaceItem::ScaleMetrics( long nMult, long nDiv )
{
    nUpper = (sal_uInt16)Scale( nUpper, nMult, nDiv );
    nLower = (sal_uInt16)Scale( nLower, nMult, nDiv );
    return true;
}

// -----------------------------------------------------------------------

bool SvxULSpaceItem::HasMetrics() const
{
    return true;
}

// class SvxPrintItem ----------------------------------------------------

SfxPoolItem* SvxPrintItem::Clone( SfxItemPool* ) const
{
    return new SvxPrintItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxPrintItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
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
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
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
            rText = EE_RESSTR(nId);
            return ePres;
        }
        default: ;//prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxOpaqueItem ---------------------------------------------------

SfxPoolItem* SvxOpaqueItem::Clone( SfxItemPool* ) const
{
    return new SvxOpaqueItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxOpaqueItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
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
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
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
            rText = EE_RESSTR(nId);
            return ePres;
        }
        default: ;//prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxProtectItem --------------------------------------------------

int SvxProtectItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return ( bCntnt == ( (SvxProtectItem&)rAttr ).bCntnt &&
             bSize  == ( (SvxProtectItem&)rAttr ).bSize  &&
             bPos   == ( (SvxProtectItem&)rAttr ).bPos );
}

bool SvxProtectItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bValue;
    switch(nMemberId)
    {
        case MID_PROTECT_CONTENT :  bValue = bCntnt; break;
        case MID_PROTECT_SIZE    :  bValue = bSize; break;
        case MID_PROTECT_POSITION:  bValue = bPos; break;
        default:
            OSL_FAIL("Wrong MemberId");
            return false;
    }

    rVal = Bool2Any( bValue );
    return true;
}

bool    SvxProtectItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bVal( Any2Bool(rVal) );
    switch(nMemberId)
    {
        case MID_PROTECT_CONTENT :  bCntnt = bVal;  break;
        case MID_PROTECT_SIZE    :  bSize  = bVal;  break;
        case MID_PROTECT_POSITION:  bPos   = bVal;  break;
        default:
            OSL_FAIL("Wrong MemberId");
            return false;
    }
    return true;
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
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
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
            rText = EE_RESSTR(nId);
            rText += cpDelim;
            nId = RID_SVXITEMS_PROT_SIZE_FALSE;

            if ( bSize )
                nId = RID_SVXITEMS_PROT_SIZE_TRUE;
            rText += EE_RESSTR(nId);
            rText += cpDelim;
            nId = RID_SVXITEMS_PROT_POS_FALSE;

            if ( bPos )
                nId = RID_SVXITEMS_PROT_POS_TRUE;
            rText += EE_RESSTR(nId);
            return ePres;
        }
        default: ;//prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SvStream& SvxProtectItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
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

SvxShadowItem::SvxShadowItem( const sal_uInt16 nId,
                 const Color *pColor, const sal_uInt16 nW,
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
bool SvxShadowItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    table::ShadowFormat aShadow;
    table::ShadowLocation eSet = table::ShadowLocation_NONE;
    switch( eLocation )
    {
        case SVX_SHADOW_TOPLEFT    : eSet = table::ShadowLocation_TOP_LEFT    ; break;
        case SVX_SHADOW_TOPRIGHT   : eSet = table::ShadowLocation_TOP_RIGHT   ; break;
        case SVX_SHADOW_BOTTOMLEFT : eSet = table::ShadowLocation_BOTTOM_LEFT ; break;
        case SVX_SHADOW_BOTTOMRIGHT: eSet = table::ShadowLocation_BOTTOM_RIGHT; break;
        default: ;//prevent warning
    }
    aShadow.Location = eSet;
    aShadow.ShadowWidth =   bConvert ? TWIP_TO_MM100_UNSIGNED(nWidth) : nWidth;
    aShadow.IsTransparent = aShadowColor.GetTransparency() > 0;
    aShadow.Color = aShadowColor.GetRGBColor();

    switch ( nMemberId )
    {
        case MID_LOCATION: rVal <<= aShadow.Location; break;
        case MID_WIDTH: rVal <<= aShadow.ShadowWidth; break;
        case MID_TRANSPARENT: rVal <<= aShadow.IsTransparent; break;
        case MID_BG_COLOR: rVal <<= aShadow.Color; break;
        case 0: rVal <<= aShadow; break;
        default: OSL_FAIL("Wrong MemberId!"); return false;
    }

    return true;
}
// -----------------------------------------------------------------------
bool SvxShadowItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    table::ShadowFormat aShadow;
    uno::Any aAny;
    bool bRet = QueryValue( aAny, bConvert ? CONVERT_TWIPS : 0 ) && ( aAny >>= aShadow );
    switch ( nMemberId )
    {
        case MID_LOCATION:
        {
            bRet = (rVal >>= aShadow.Location);
            if ( !bRet )
            {
                sal_Int16 nVal = 0;
                bRet = (rVal >>= nVal);
                aShadow.Location = (table::ShadowLocation) nVal;
            }

            break;
        }

        case MID_WIDTH: rVal >>= aShadow.ShadowWidth; break;
        case MID_TRANSPARENT: rVal >>= aShadow.IsTransparent; break;
        case MID_BG_COLOR: rVal >>= aShadow.Color; break;
        case 0: rVal >>= aShadow; break;
        default: OSL_FAIL("Wrong MemberId!"); return sal_False;
    }

    if ( bRet )
    {
//      SvxShadowLocation eSet = SVX_SHADOW_NONE;
        switch( aShadow.Location )
        {
            case table::ShadowLocation_TOP_LEFT    : eLocation = SVX_SHADOW_TOPLEFT; break;
            case table::ShadowLocation_TOP_RIGHT   : eLocation = SVX_SHADOW_TOPRIGHT; break;
            case table::ShadowLocation_BOTTOM_LEFT : eLocation = SVX_SHADOW_BOTTOMLEFT ; break;
            case table::ShadowLocation_BOTTOM_RIGHT: eLocation = SVX_SHADOW_BOTTOMRIGHT; break;
            default: ;//prevent warning
        }

        nWidth = bConvert ? MM100_TO_TWIP(aShadow.ShadowWidth) : aShadow.ShadowWidth;
        Color aSet(aShadow.Color);
        aSet.SetTransparency(aShadow.IsTransparent ? 0xff : 0);
        aShadowColor = aSet;
    }

    return bRet;
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
            OSL_FAIL( "wrong shadow" );
    }
    return nSpace;
}

//------------------------------------------------------------------------

SfxItemPresentation SvxShadowItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const IntlWrapper *pIntl
)   const
{
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
            rText += EE_RESSTR(nId);
            rText += cpDelim;
            rText += GetMetricText( (long)nWidth, eCoreUnit, ePresUnit, pIntl );
            rText += cpDelim;
            rText += EE_RESSTR(RID_SVXITEMS_SHADOW_BEGIN + eLocation);
            return ePres;
        }
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = EE_RESSTR(RID_SVXITEMS_SHADOW_COMPLETE);
            rText += ::GetColorString( aShadowColor );
            rText += cpDelim;

            sal_uInt16 nId = RID_SVXITEMS_TRANSPARENT_FALSE;
            if ( aShadowColor.GetTransparency() )
                nId = RID_SVXITEMS_TRANSPARENT_TRUE;
            rText += EE_RESSTR(nId);
            rText += cpDelim;
            rText += GetMetricText( (long)nWidth, eCoreUnit, ePresUnit, pIntl );
            rText += EE_RESSTR(GetMetricId(ePresUnit));
            rText += cpDelim;
            rText += EE_RESSTR(RID_SVXITEMS_SHADOW_BEGIN + eLocation);
            return ePres;
        }
        default: ;//prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SvStream& SvxShadowItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
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

bool SvxShadowItem::ScaleMetrics( long nMult, long nDiv )
{
    nWidth = (sal_uInt16)Scale( nWidth, nMult, nDiv );
    return true;
}

// -----------------------------------------------------------------------

bool SvxShadowItem::HasMetrics() const
{
    return true;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxShadowItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 cLoc;
    sal_uInt16 _nWidth;
    sal_Bool bTrans;
    Color aColor;
    Color aFillColor;
    sal_Int8 nStyle;
    rStrm >> cLoc >> _nWidth
          >> bTrans >> aColor >> aFillColor >> nStyle;
    aColor.SetTransparency(bTrans ? 0xff : 0);
    return new SvxShadowItem( Which(), &aColor, _nWidth, (SvxShadowLocation)cLoc );
}

// -----------------------------------------------------------------------

sal_uInt16 SvxShadowItem::GetValueCount() const
{
    return SVX_SHADOW_END;  // SVX_SHADOW_BOTTOMRIGHT + 1
}

// -----------------------------------------------------------------------

rtl::OUString SvxShadowItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos < SVX_SHADOW_END, "enum overflow!" );
    return EE_RESSTR(RID_SVXITEMS_SHADOW_BEGIN + nPos );
}

// -----------------------------------------------------------------------

sal_uInt16 SvxShadowItem::GetEnumValue() const
{
    return (sal_uInt16)GetLocation();
}

// -----------------------------------------------------------------------

void SvxShadowItem::SetEnumValue( sal_uInt16 nVal )
{
    SetLocation( (const SvxShadowLocation)nVal );
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
table::BorderLine2 SvxBoxItem::SvxLineToLine(const SvxBorderLine* pLine, sal_Bool bConvert)
{
    table::BorderLine2 aLine;
    if(pLine)
    {
        aLine.Color          = pLine->GetColor().GetColor() ;
        aLine.InnerLineWidth = sal_uInt16( bConvert ? TWIP_TO_MM100_UNSIGNED(pLine->GetInWidth() ): pLine->GetInWidth() );
        aLine.OuterLineWidth = sal_uInt16( bConvert ? TWIP_TO_MM100_UNSIGNED(pLine->GetOutWidth()): pLine->GetOutWidth() );
        aLine.LineDistance   = sal_uInt16( bConvert ? TWIP_TO_MM100_UNSIGNED(pLine->GetDistance()): pLine->GetDistance() );
        aLine.LineStyle      = pLine->GetBorderLineStyle();
        aLine.LineWidth      = sal_uInt32( bConvert ? TWIP_TO_MM100( pLine->GetWidth( ) ) : pLine->GetWidth( ) );
    }
    else
        aLine.Color          = aLine.InnerLineWidth = aLine.OuterLineWidth = aLine.LineDistance  = 0;
    return aLine;
}
// -----------------------------------------------------------------------
bool SvxBoxItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId  ) const
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    table::BorderLine2 aRetLine;
    sal_uInt16 nDist = 0;
    sal_Bool bDistMember = sal_False;
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case 0:
        {
            // 4 Borders and 5 distances
            uno::Sequence< uno::Any > aSeq( 9 );
            aSeq[0] = uno::makeAny( SvxBoxItem::SvxLineToLine(GetLeft(), bConvert) );
            aSeq[1] = uno::makeAny( SvxBoxItem::SvxLineToLine(GetRight(), bConvert) );
            aSeq[2] = uno::makeAny( SvxBoxItem::SvxLineToLine(GetBottom(), bConvert) );
            aSeq[3] = uno::makeAny( SvxBoxItem::SvxLineToLine(GetTop(), bConvert) );
            aSeq[4] <<= uno::makeAny( (sal_Int32)(bConvert ? TWIP_TO_MM100_UNSIGNED( GetDistance()) : GetDistance()));
            aSeq[5] <<= uno::makeAny( (sal_Int32)(bConvert ? TWIP_TO_MM100_UNSIGNED( nTopDist ) : nTopDist ));
            aSeq[6] <<= uno::makeAny( (sal_Int32)(bConvert ? TWIP_TO_MM100_UNSIGNED( nBottomDist ) : nBottomDist ));
            aSeq[7] <<= uno::makeAny( (sal_Int32)(bConvert ? TWIP_TO_MM100_UNSIGNED( nLeftDist ) : nLeftDist ));
            aSeq[8] <<= uno::makeAny( (sal_Int32)(bConvert ? TWIP_TO_MM100_UNSIGNED( nRightDist ) : nRightDist ));
            rVal = uno::makeAny( aSeq );
            return sal_True;
        }
        case MID_LEFT_BORDER:
        case LEFT_BORDER:
            aRetLine = SvxBoxItem::SvxLineToLine(GetLeft(), bConvert);
            break;
        case MID_RIGHT_BORDER:
        case RIGHT_BORDER:
            aRetLine = SvxBoxItem::SvxLineToLine(GetRight(), bConvert);
            break;
        case MID_BOTTOM_BORDER:
        case BOTTOM_BORDER:
            aRetLine = SvxBoxItem::SvxLineToLine(GetBottom(), bConvert);
            break;
        case MID_TOP_BORDER:
        case TOP_BORDER:
            aRetLine = SvxBoxItem::SvxLineToLine(GetTop(), bConvert);
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
        rVal <<= (sal_Int32)(bConvert ? TWIP_TO_MM100_UNSIGNED(nDist) : nDist);
    else
        rVal <<= aRetLine;

    return true;
}

namespace
{

sal_Bool
lcl_lineToSvxLine(const table::BorderLine& rLine, SvxBorderLine& rSvxLine, sal_Bool bConvert, sal_Bool bGuessWidth)
{
    rSvxLine.SetColor(   Color(rLine.Color));
    if ( bGuessWidth )
    {
        rSvxLine.GuessLinesWidths( rSvxLine.GetBorderLineStyle(),
                sal_uInt16( bConvert ? MM100_TO_TWIP(rLine.OuterLineWidth) : rLine.OuterLineWidth  ),
                sal_uInt16( bConvert ? MM100_TO_TWIP(rLine.InnerLineWidth) : rLine.InnerLineWidth  ),
                sal_uInt16( bConvert ? MM100_TO_TWIP(rLine.LineDistance )  : rLine.LineDistance  ));
    }

    sal_Bool bRet = !rSvxLine.isEmpty();
    return bRet;
}

}

// -----------------------------------------------------------------------
sal_Bool SvxBoxItem::LineToSvxLine(const ::com::sun::star::table::BorderLine& rLine, SvxBorderLine& rSvxLine, sal_Bool bConvert)
{
    return lcl_lineToSvxLine(rLine, rSvxLine, bConvert, sal_True);
}

sal_Bool
SvxBoxItem::LineToSvxLine(const ::com::sun::star::table::BorderLine2& rLine, SvxBorderLine& rSvxLine, sal_Bool bConvert)
{
    SvxBorderStyle const nStyle =
        (rLine.LineStyle < 0 || INSET < rLine.LineStyle)
        ? SOLID     // default
        : rLine.LineStyle;

    rSvxLine.SetBorderLineStyle( nStyle );

    sal_Bool bGuessWidth = sal_True;
    if ( rLine.LineWidth )
    {
        rSvxLine.SetWidth( bConvert? MM100_TO_TWIP_UNSIGNED( rLine.LineWidth ) : rLine.LineWidth );
        // fdo#46112: double does not necessarily mean symmetric
        // for backwards compatibility
        bGuessWidth = (DOUBLE == nStyle) &&
            (rLine.InnerLineWidth > 0) && (rLine.OuterLineWidth > 0);
    }

    return lcl_lineToSvxLine(rLine, rSvxLine, bConvert, bGuessWidth);
}

// -----------------------------------------------------------------------

namespace
{

bool
lcl_extractBorderLine(const uno::Any& rAny, table::BorderLine2& rLine)
{
    if (rAny >>= rLine)
        return true;

    table::BorderLine aBorderLine;
    if (rAny >>= aBorderLine)
    {
        rLine.Color = aBorderLine.Color;
        rLine.InnerLineWidth = aBorderLine.InnerLineWidth;
        rLine.OuterLineWidth = aBorderLine.OuterLineWidth;
        rLine.LineDistance = aBorderLine.LineDistance;
        rLine.LineStyle = table::BorderLineStyle::SOLID;
        return true;
    }

    return false;
}

template<typename Item>
bool
lcl_setLine(const uno::Any& rAny, Item& rItem, sal_uInt16 nLine, const bool bConvert)
{
    bool bDone = false;
    table::BorderLine2 aBorderLine;
    if (lcl_extractBorderLine(rAny, aBorderLine))
    {
        SvxBorderLine aLine;
        bool bSet = SvxBoxItem::LineToSvxLine(aBorderLine, aLine, bConvert);
        rItem.SetLine( bSet ? &aLine : NULL, nLine);
        bDone = true;
    }
    return bDone;
}

}

bool SvxBoxItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    sal_uInt16 nLine = BOX_LINE_TOP;
    sal_Bool bDistMember = sal_False;
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case 0:
        {
            uno::Sequence< uno::Any > aSeq;
            if (( rVal >>= aSeq ) && ( aSeq.getLength() == 9 ))
            {
                // 4 Borders and 5 distances
                const sal_uInt16 aBorders[] = { BOX_LINE_LEFT, BOX_LINE_RIGHT, BOX_LINE_BOTTOM, BOX_LINE_TOP };
                for (int n(0); n != SAL_N_ELEMENTS(aBorders); ++n)
                {
                    if (!lcl_setLine(aSeq[n], *this, aBorders[n], bConvert))
                        return sal_False;
                }

                // WTH are the borders and the distances saved in different order?
                sal_uInt16 nLines[4] = { BOX_LINE_TOP, BOX_LINE_BOTTOM, BOX_LINE_LEFT, BOX_LINE_RIGHT };
                for ( sal_Int32 n = 4; n < 9; n++ )
                {
                    sal_Int32 nDist = 0;
                    if ( aSeq[n] >>= nDist )
                    {
                        if( bConvert )
                            nDist = MM100_TO_TWIP(nDist);
                        if ( n == 4 )
                            SetDistance( sal_uInt16( nDist ));
                        else
                            SetDistance( sal_uInt16( nDist ), nLines[n-5] );
                    }
                    else
                        return sal_False;
                }

                return sal_True;
            }
            else
                return sal_False;
        }
        case LEFT_BORDER_DISTANCE:
            bDistMember = sal_True;
        case LEFT_BORDER:
        case MID_LEFT_BORDER:
            nLine = BOX_LINE_LEFT;
            break;
        case RIGHT_BORDER_DISTANCE:
            bDistMember = sal_True;
        case RIGHT_BORDER:
        case MID_RIGHT_BORDER:
            nLine = BOX_LINE_RIGHT;
            break;
        case BOTTOM_BORDER_DISTANCE:
            bDistMember = sal_True;
        case BOTTOM_BORDER:
        case MID_BOTTOM_BORDER:
            nLine = BOX_LINE_BOTTOM;
            break;
        case TOP_BORDER_DISTANCE:
            bDistMember = sal_True;
        case TOP_BORDER:
        case MID_TOP_BORDER:
            nLine = BOX_LINE_TOP;
            break;
        case LINE_STYLE:
            {
                drawing::LineStyle eDrawingStyle;
                rVal >>= eDrawingStyle;
                editeng::SvxBorderStyle eBorderStyle = NONE;
                switch ( eDrawingStyle )
                {
                    default:
                    case drawing::LineStyle_NONE:
                        break;
                    case drawing::LineStyle_SOLID:
                        eBorderStyle = SOLID;
                        break;
                    case drawing::LineStyle_DASH:
                        eBorderStyle = DASHED;
                        break;
                }

                // Set the line style on all borders
                const sal_uInt16 aBorders[] = { BOX_LINE_LEFT, BOX_LINE_RIGHT, BOX_LINE_BOTTOM, BOX_LINE_TOP };
                for (int n(0); n != SAL_N_ELEMENTS(aBorders); ++n)
                {
                    editeng::SvxBorderLine* pLine = const_cast< editeng::SvxBorderLine* >( GetLine( aBorders[n] ) );
                    if( pLine )
                        pLine->SetBorderLineStyle( eBorderStyle );
                }
                return sal_True;
            }
            break;
        case LINE_WIDTH:
            {
                // Set the line width on all borders
                long nWidth(0);
                rVal >>= nWidth;
                if( bConvert )
                    nWidth = MM100_TO_TWIP( nWidth );

                // Set the line Width on all borders
                const sal_uInt16 aBorders[] = { BOX_LINE_LEFT, BOX_LINE_RIGHT, BOX_LINE_BOTTOM, BOX_LINE_TOP };
                for (int n(0); n != SAL_N_ELEMENTS(aBorders); ++n)
                {
                    editeng::SvxBorderLine* pLine = const_cast< editeng::SvxBorderLine* >( GetLine( aBorders[n] ) );
                    pLine->SetWidth( nWidth );
                }
            }
            return sal_True;
            break;
    }

    if( bDistMember || nMemberId == BORDER_DISTANCE )
    {
        sal_Int32 nDist = 0;
        if(!(rVal >>= nDist))
            return sal_False;

        if(nDist >= 0)
        {
            if( bConvert )
                nDist = MM100_TO_TWIP(nDist);
            if( nMemberId == BORDER_DISTANCE )
                SetDistance( sal_uInt16( nDist ));
            else
                SetDistance( sal_uInt16( nDist ), nLine );
        }
    }
    else
    {
        SvxBorderLine aLine;
        if( !rVal.hasValue() )
            return sal_False;

        table::BorderLine2 aBorderLine;
        if( lcl_extractBorderLine(rVal, aBorderLine) )
        {
            // usual struct
        }
        else if (rVal.getValueTypeClass() == uno::TypeClass_SEQUENCE )
        {
            // serialization for basic macro recording
            uno::Reference < script::XTypeConverter > xConverter
                    ( script::Converter::create(::comphelper::getProcessComponentContext()) );
            uno::Sequence < uno::Any > aSeq;
            uno::Any aNew;
            try { aNew = xConverter->convertTo( rVal, ::getCppuType((const uno::Sequence < uno::Any >*)0) ); }
            catch (const uno::Exception&) {}

            aNew >>= aSeq;
            if (aSeq.getLength() >= 4 && aSeq.getLength() <= 6)
            {
                sal_Int32 nVal = 0;
                if ( aSeq[0] >>= nVal )
                    aBorderLine.Color = nVal;
                if ( aSeq[1] >>= nVal )
                    aBorderLine.InnerLineWidth = (sal_Int16) nVal;
                if ( aSeq[2] >>= nVal )
                    aBorderLine.OuterLineWidth = (sal_Int16) nVal;
                if ( aSeq[3] >>= nVal )
                    aBorderLine.LineDistance = (sal_Int16) nVal;
                if (aSeq.getLength() >= 5) // fdo#40874 added fields
                {
                    if (aSeq[4] >>= nVal)
                    {
                        aBorderLine.LineStyle = nVal;
                    }
                    if (aSeq.getLength() >= 6)
                    {
                        if (aSeq[5] >>= nVal)
                        {
                            aBorderLine.LineWidth = nVal;
                        }
                    }
                }
            }
            else
                return sal_False;
        }
        else
            return sal_False;

        sal_Bool bSet = SvxBoxItem::LineToSvxLine(aBorderLine, aLine, bConvert);
        SetLine(bSet ? &aLine : 0, nLine);
    }

    return sal_True;
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
    XubString&          rText, const IntlWrapper *pIntl
)   const
{
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
                rText = pTop->GetValueString( eCoreUnit, ePresUnit, pIntl );
                rText += cpDelim;
            }
            if( !(pTop && pBottom && pLeft && pRight &&
                  *pTop == *pBottom && *pTop == *pLeft && *pTop == *pRight) )
            {
                if ( pBottom )
                {
                    rText += pBottom->GetValueString( eCoreUnit, ePresUnit, pIntl );
                    rText += cpDelim;
                }
                if ( pLeft )
                {
                    rText += pLeft->GetValueString( eCoreUnit, ePresUnit, pIntl );
                    rText += cpDelim;
                }
                if ( pRight )
                {
                    rText += pRight->GetValueString( eCoreUnit, ePresUnit, pIntl );
                    rText += cpDelim;
                }
            }
            rText += GetMetricText( (long)nTopDist, eCoreUnit, ePresUnit, pIntl );
            if( nTopDist != nBottomDist || nTopDist != nLeftDist ||
                nTopDist != nRightDist )
            {
                (((((rText += cpDelim)
                      += GetMetricText( (long)nBottomDist, eCoreUnit,
                                        ePresUnit, pIntl ))
                      += cpDelim)
                      += GetMetricText( (long)nLeftDist, eCoreUnit, ePresUnit, pIntl ))
                      += cpDelim)
                      += GetMetricText( (long)nRightDist, eCoreUnit,
                                        ePresUnit, pIntl );
            }
            return SFX_ITEM_PRESENTATION_NAMELESS;
        }
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if( !(pTop || pBottom || pLeft || pRight) )
            {
                rText = EE_RESSTR(RID_SVXITEMS_BORDER_NONE);
                rText += cpDelim;
            }
            else
            {
                rText = EE_RESSTR(RID_SVXITEMS_BORDER_COMPLETE);
                if( pTop && pBottom && pLeft && pRight &&
                    *pTop == *pBottom && *pTop == *pLeft && *pTop == *pRight )
                {
                    rText += pTop->GetValueString( eCoreUnit, ePresUnit, pIntl, sal_True );
                    rText += cpDelim;
                }
                else
                {
                    if ( pTop )
                    {
                        rText += EE_RESSTR(RID_SVXITEMS_BORDER_TOP);
                        rText += pTop->GetValueString( eCoreUnit, ePresUnit, pIntl, sal_True );
                        rText += cpDelim;
                    }
                    if ( pBottom )
                    {
                        rText += EE_RESSTR(RID_SVXITEMS_BORDER_BOTTOM);
                        rText += pBottom->GetValueString( eCoreUnit, ePresUnit, pIntl, sal_True );
                        rText += cpDelim;
                    }
                    if ( pLeft )
                    {
                        rText += EE_RESSTR(RID_SVXITEMS_BORDER_LEFT);
                        rText += pLeft->GetValueString( eCoreUnit, ePresUnit, pIntl, sal_True );
                        rText += cpDelim;
                    }
                    if ( pRight )
                    {
                        rText += EE_RESSTR(RID_SVXITEMS_BORDER_RIGHT);
                        rText += pRight->GetValueString( eCoreUnit, ePresUnit, pIntl, sal_True );
                        rText += cpDelim;
                    }
                }
            }

            rText += EE_RESSTR(RID_SVXITEMS_BORDER_DISTANCE);
            if( nTopDist == nBottomDist && nTopDist == nLeftDist &&
                nTopDist == nRightDist )
            {
                rText += GetMetricText( (long)nTopDist, eCoreUnit,
                                            ePresUnit, pIntl );
                rText += EE_RESSTR(GetMetricId(ePresUnit));
            }
            else
            {
                (((rText += EE_RESSTR(RID_SVXITEMS_BORDER_TOP))
                      += GetMetricText( (long)nTopDist, eCoreUnit,
                                        ePresUnit, pIntl ))
                      += EE_RESSTR(GetMetricId(ePresUnit)))
                      += cpDelim;
                (((rText += EE_RESSTR(RID_SVXITEMS_BORDER_BOTTOM))
                      += GetMetricText( (long)nBottomDist, eCoreUnit,
                                        ePresUnit, pIntl ))
                      += EE_RESSTR(GetMetricId(ePresUnit)))
                      += cpDelim;
                (((rText += EE_RESSTR(RID_SVXITEMS_BORDER_LEFT))
                      += GetMetricText( (long)nLeftDist, eCoreUnit,
                                        ePresUnit, pIntl ))
                      += EE_RESSTR(GetMetricId(ePresUnit)))
                      += cpDelim;
                ((rText += EE_RESSTR(RID_SVXITEMS_BORDER_RIGHT))
                      += GetMetricText( (long)nRightDist, eCoreUnit,
                                        ePresUnit, pIntl ))
                      += EE_RESSTR(GetMetricId(ePresUnit));
            }
            return SFX_ITEM_PRESENTATION_COMPLETE;
        }
        default: ;//prevent warning
    }
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
            rStrm << static_cast<sal_Int8>(i);
            StoreBorderLine(rStrm, *l, BorderLineVersionFromBoxVersion(nItemVersion));
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
            SOFFICE_FILEFORMAT_50==nFFVer,
            "SvxBoxItem: Gibt es ein neues Fileformat?" );
    return SOFFICE_FILEFORMAT_31==nFFVer ||
           SOFFICE_FILEFORMAT_40==nFFVer ? 0 : BOX_BORDER_STYLE_VERSION;
}

// -----------------------------------------------------------------------

bool SvxBoxItem::ScaleMetrics( long nMult, long nDiv )
{
    if ( pTop )     pTop->ScaleMetrics( nMult, nDiv );
    if ( pBottom )  pBottom->ScaleMetrics( nMult, nDiv );
    if ( pLeft )    pLeft->ScaleMetrics( nMult, nDiv );
    if ( pRight )   pBottom->ScaleMetrics( nMult, nDiv );
    nTopDist = (sal_uInt16)Scale( nTopDist, nMult, nDiv );
    nBottomDist = (sal_uInt16)Scale( nBottomDist, nMult, nDiv );
    nLeftDist = (sal_uInt16)Scale( nLeftDist, nMult, nDiv );
    nRightDist = (sal_uInt16)Scale( nRightDist, nMult, nDiv );
    return true;
}

// -----------------------------------------------------------------------

bool SvxBoxItem::HasMetrics() const
{
    return true;
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

        SvxBorderLine aBorder = CreateBorderLine(rStrm, BorderLineVersionFromBoxVersion(nIVersion));
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
            OSL_FAIL( "wrong line" );
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
            delete pTmp;
            OSL_FAIL( "wrong line" );
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
            OSL_FAIL( "wrong line" );
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
            OSL_FAIL( "wrong line" );
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
        OSL_FAIL( "wrong line" );
    }

    if( pTmp )
    {
        nDist = nDist + (sal_uInt16)(pTmp->GetOutWidth()) + (sal_uInt16)(pTmp->GetInWidth()) + (sal_uInt16)(pTmp->GetDistance());
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
    mbEnableHor( false ),
    mbEnableVer( false ),
    nDefDist( 0 )
{
    bDist = bMinDist = sal_False;
    ResetFlags();
}

// -----------------------------------------------------------------------

SvxBoxInfoItem::SvxBoxInfoItem( const SvxBoxInfoItem& rCpy ) :
    SfxPoolItem( rCpy ),
    mbEnableHor( rCpy.mbEnableHor ),
    mbEnableVer( rCpy.mbEnableVer )
{
    pHori       = rCpy.GetHori() ? new SvxBorderLine( *rCpy.GetHori() ) : 0;
    pVert       = rCpy.GetVert() ? new SvxBorderLine( *rCpy.GetVert() ) : 0;
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
    mbEnableHor = rCpy.mbEnableHor;
    mbEnableVer = rCpy.mbEnableVer;
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

    return (   mbEnableHor               == rBoxInfo.mbEnableHor
            && mbEnableVer               == rBoxInfo.mbEnableVer
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
    {
        delete pTmp;
        OSL_FAIL( "wrong line" );
    }
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxBoxInfoItem::Clone( SfxItemPool* ) const
{
    return new SvxBoxInfoItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxBoxInfoItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SvStream& SvxBoxInfoItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
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

bool SvxBoxInfoItem::ScaleMetrics( long nMult, long nDiv )
{
    if ( pHori ) pHori->ScaleMetrics( nMult, nDiv );
    if ( pVert ) pVert->ScaleMetrics( nMult, nDiv );
    nDefDist = (sal_uInt16)Scale( nDefDist, nMult, nDiv );
    return true;
}

// -----------------------------------------------------------------------

bool SvxBoxInfoItem::HasMetrics() const
{
    return true;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxBoxInfoItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 cFlags;
    sal_uInt16 _nDefDist;
    rStrm >> cFlags >> _nDefDist;

    SvxBoxInfoItem* pAttr = new SvxBoxInfoItem( Which() );

    pAttr->SetTable  ( ( cFlags & 0x01 ) != 0 );
    pAttr->SetDist   ( ( cFlags & 0x02 ) != 0 );
    pAttr->SetMinDist( ( cFlags & 0x04 ) != 0 );
    pAttr->SetDefDist( _nDefDist );

    while( sal_True )
    {
        sal_Int8 cLine;
        rStrm >> cLine;

        if( cLine > 1 )
            break;
        short nOutline, nInline, nDistance;
        Color aColor;
        rStrm >> aColor >> nOutline >> nInline >> nDistance;
        SvxBorderLine aBorder( &aColor );
        aBorder.GuessLinesWidths(NONE, nOutline, nInline, nDistance);

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
    nValidFlags = 0x7F; // all valid except Disable
}

bool SvxBoxInfoItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId  ) const
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    table::BorderLine2 aRetLine;
    sal_Int16 nVal=0;
    sal_Bool bIntMember = sal_False;
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case 0:
        {
            // 2 BorderLines, flags, valid flags and distance
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aSeq( 5 );
            aSeq[0] = ::com::sun::star::uno::makeAny( SvxBoxItem::SvxLineToLine( pHori, bConvert) );
            aSeq[1] = ::com::sun::star::uno::makeAny( SvxBoxItem::SvxLineToLine( pVert, bConvert) );
            if ( IsTable() )
                nVal |= 0x01;
            if ( IsDist() )
                nVal |= 0x02;
            if ( IsMinDist() )
                nVal |= 0x04;
            aSeq[2] = ::com::sun::star::uno::makeAny( nVal );
            nVal = nValidFlags;
            aSeq[3] = ::com::sun::star::uno::makeAny( nVal );
            aSeq[4] = ::com::sun::star::uno::makeAny( (sal_Int32)(bConvert ? TWIP_TO_MM100_UNSIGNED(GetDefDist()) : GetDefDist()) );
            rVal = ::com::sun::star::uno::makeAny( aSeq );
            return sal_True;
        }

        case MID_HORIZONTAL:
            aRetLine = SvxBoxItem::SvxLineToLine( pHori, bConvert);
            break;
        case MID_VERTICAL:
            aRetLine = SvxBoxItem::SvxLineToLine( pVert, bConvert);
            break;
        case MID_FLAGS:
            bIntMember = sal_True;
            if ( IsTable() )
                nVal |= 0x01;
            if ( IsDist() )
                nVal |= 0x02;
            if ( IsMinDist() )
                nVal |= 0x04;
            rVal <<= nVal;
            break;
        case MID_VALIDFLAGS:
            bIntMember = sal_True;
            nVal = nValidFlags;
            rVal <<= nVal;
            break;
        case MID_DISTANCE:
            bIntMember = sal_True;
            rVal <<= (sal_Int32)(bConvert ? TWIP_TO_MM100_UNSIGNED(GetDefDist()) : GetDefDist());
            break;
        default: OSL_FAIL("Wrong MemberId!"); return false;
    }

    if( !bIntMember )
        rVal <<= aRetLine;

    return true;
}

// -----------------------------------------------------------------------

bool SvxBoxInfoItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet;
    switch(nMemberId)
    {
        case 0:
        {
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aSeq;
            if (( rVal >>= aSeq ) && ( aSeq.getLength() == 5 ))
            {
                // 2 BorderLines, flags, valid flags and distance
                if (!lcl_setLine(aSeq[0], *this, BOXINFO_LINE_HORI, bConvert))
                    return sal_False;
                if (!lcl_setLine(aSeq[1], *this, BOXINFO_LINE_VERT, bConvert))
                    return sal_False;

                sal_Int16 nFlags( 0 );
                sal_Int32 nVal( 0 );
                if ( aSeq[2] >>= nFlags )
                {
                    SetTable  ( ( nFlags & 0x01 ) != 0 );
                    SetDist   ( ( nFlags & 0x02 ) != 0 );
                    SetMinDist( ( nFlags & 0x04 ) != 0 );
                }
                else
                    return sal_False;
                if ( aSeq[3] >>= nFlags )
                    nValidFlags = (sal_uInt8)nFlags;
                else
                    return sal_False;
                if (( aSeq[4] >>= nVal ) && ( nVal >= 0 ))
                {
                    if( bConvert )
                        nVal = MM100_TO_TWIP(nVal);
                    SetDefDist( (sal_uInt16)nVal );
                }
            }
            return sal_True;
        }

        case MID_HORIZONTAL:
        case MID_VERTICAL:
        {
            if( !rVal.hasValue() )
                return sal_False;

            table::BorderLine2 aBorderLine;
            if( lcl_extractBorderLine(rVal, aBorderLine) )
            {
                // usual struct
            }
            else if (rVal.getValueTypeClass() == uno::TypeClass_SEQUENCE )
            {
                // serialization for basic macro recording
                uno::Reference < script::XTypeConverter > xConverter( script::Converter::create(::comphelper::getProcessComponentContext()) );
                uno::Any aNew;
                uno::Sequence < uno::Any > aSeq;
                try { aNew = xConverter->convertTo( rVal, ::getCppuType((const uno::Sequence < uno::Any >*)0) ); }
                catch (const uno::Exception&) {}

                if ((aNew >>= aSeq) &&
                    aSeq.getLength() >= 4  && aSeq.getLength() <= 6)
                {
                    sal_Int32 nVal = 0;
                    if ( aSeq[0] >>= nVal )
                        aBorderLine.Color = nVal;
                    if ( aSeq[1] >>= nVal )
                        aBorderLine.InnerLineWidth = (sal_Int16) nVal;
                    if ( aSeq[2] >>= nVal )
                        aBorderLine.OuterLineWidth = (sal_Int16) nVal;
                    if ( aSeq[3] >>= nVal )
                        aBorderLine.LineDistance = (sal_Int16) nVal;
                    if (aSeq.getLength() >= 5) // fdo#40874 added fields
                    {
                        if (aSeq[4] >>= nVal)
                        {
                            aBorderLine.LineStyle = nVal;
                        }
                        if (aSeq.getLength() >= 6)
                        {
                            if (aSeq[5] >>= nVal)
                            {
                                aBorderLine.LineWidth = nVal;
                            }
                        }
                    }
                }
                else
                    return sal_False;
            }
            else if (rVal.getValueType() == ::getCppuType((const ::com::sun::star::uno::Sequence < sal_Int16 >*)0) )
            {
                // serialization for basic macro recording
                ::com::sun::star::uno::Sequence < sal_Int16 > aSeq;
                rVal >>= aSeq;
                if (aSeq.getLength() >= 4 && aSeq.getLength() <= 6)
                {
                    aBorderLine.Color = aSeq[0];
                    aBorderLine.InnerLineWidth = aSeq[1];
                    aBorderLine.OuterLineWidth = aSeq[2];
                    aBorderLine.LineDistance = aSeq[3];
                    if (aSeq.getLength() >= 5) // fdo#40874 added fields
                    {
                        aBorderLine.LineStyle = aSeq[4];
                        if (aSeq.getLength() >= 6)
                        {
                            aBorderLine.LineWidth = aSeq[5];
                        }
                    }
                }
                else
                    return sal_False;
            }
            else
                return sal_False;

            SvxBorderLine aLine;
            sal_Bool bSet = SvxBoxItem::LineToSvxLine(aBorderLine, aLine, bConvert);
            if ( bSet )
                SetLine( &aLine, nMemberId == MID_HORIZONTAL ? BOXINFO_LINE_HORI : BOXINFO_LINE_VERT );
            break;
        }
        case MID_FLAGS:
        {
            sal_Int16 nFlags = sal_Int16();
            bRet = (rVal >>= nFlags);
            if ( bRet )
            {
                SetTable  ( ( nFlags & 0x01 ) != 0 );
                SetDist   ( ( nFlags & 0x02 ) != 0 );
                SetMinDist( ( nFlags & 0x04 ) != 0 );
            }

            break;
        }
        case MID_VALIDFLAGS:
        {
            sal_Int16 nFlags = sal_Int16();
            bRet = (rVal >>= nFlags);
            if ( bRet )
                nValidFlags = (sal_uInt8)nFlags;
            break;
        }
        case MID_DISTANCE:
        {
            sal_Int32 nVal = 0;
            bRet = (rVal >>= nVal);
            if ( bRet && nVal>=0 )
            {
                if( bConvert )
                    nVal = MM100_TO_TWIP(nVal);
                SetDefDist( (sal_uInt16)nVal );
            }
            break;
        }
        default: OSL_FAIL("Wrong MemberId!"); return sal_False;
    }

    return sal_True;
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
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;

        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetValueTextByPos( GetValue() );
            return ePres;
        default: ;//prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

rtl::OUString SvxFmtBreakItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos < SVX_BREAK_END, "enum overflow!" );
    return EE_RESSTR(RID_SVXITEMS_BREAK_BEGIN + nPos);
}

// -----------------------------------------------------------------------
bool SvxFmtBreakItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
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
        default: ;//prevent warning
    }
    rVal <<= eBreak;
    return true;
}
// -----------------------------------------------------------------------
bool SvxFmtBreakItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    style::BreakType nBreak;

    if(!(rVal >>= nBreak))
    {
        sal_Int32 nValue = 0;
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
        default: ;//prevent warning
    }
    SetValue((sal_uInt16) eBreak);

    return true;
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
            SOFFICE_FILEFORMAT_50==nFFVer,
            "SvxFmtBreakItem: Is there a new file format? ");
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

SvStream& SvxFmtKeepItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
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
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
    ) const
{
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
            rText = EE_RESSTR(nId);
            return ePres;
        }
        default: ;//prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
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

bool SvxLineItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemId ) const
{
    sal_Bool bConvert = 0!=(nMemId&CONVERT_TWIPS);
    nMemId &= ~CONVERT_TWIPS;
    if ( nMemId == 0 )
    {
        rVal <<= uno::makeAny( SvxBoxItem::SvxLineToLine(pLine, bConvert) );
        return true;
    }
    else if ( pLine )
    {
        switch ( nMemId )
        {
            case MID_FG_COLOR:      rVal <<= sal_Int32(pLine->GetColor().GetColor()); break;
            case MID_OUTER_WIDTH:   rVal <<= sal_Int32(pLine->GetOutWidth());   break;
            case MID_INNER_WIDTH:   rVal <<= sal_Int32(pLine->GetInWidth( ));   break;
            case MID_DISTANCE:      rVal <<= sal_Int32(pLine->GetDistance());   break;
            default:
                OSL_FAIL( "Wrong MemberId" );
                return false;
        }
    }

    return true;
}

// -----------------------------------------------------------------------

bool SvxLineItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemId )
{
    sal_Bool bConvert = 0!=(nMemId&CONVERT_TWIPS);
    nMemId &= ~CONVERT_TWIPS;
    sal_Int32 nVal = 0;
    if ( nMemId == 0 )
    {
        table::BorderLine2 aLine;
        if ( lcl_extractBorderLine(rVal, aLine) )
        {
            if ( !pLine )
                pLine = new SvxBorderLine;
            if( !SvxBoxItem::LineToSvxLine(aLine, *pLine, bConvert) )
                DELETEZ( pLine );
            return true;
        }
        return false;
    }
    else if ( rVal >>= nVal )
    {
        if ( !pLine )
            pLine = new SvxBorderLine;

        switch ( nMemId )
        {
            case MID_FG_COLOR:      pLine->SetColor( Color(nVal) ); break;
            case MID_LINE_STYLE:
                pLine->SetBorderLineStyle(static_cast<SvxBorderStyle>(nVal));
            break;
            default:
                OSL_FAIL( "Wrong MemberId" );
                return sal_False;
        }

        return true;
    }

    return false;
}

//------------------------------------------------------------------------

SfxItemPresentation SvxLineItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const IntlWrapper *pIntl
)   const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if ( pLine )
                rText = pLine->GetValueString( eCoreUnit, ePresUnit, pIntl,
                    (SFX_ITEM_PRESENTATION_COMPLETE == ePres) );
            return ePres;
        }
        default: ;//prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SvStream& SvxLineItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    if( pLine )
    {
        rStrm << pLine->GetColor()
              << (short)pLine->GetOutWidth()
              << (short)pLine->GetInWidth()
              << (short)pLine->GetDistance();
    }
    else
        rStrm << Color() << (short)0 << (short)0 << (short)0;
    return rStrm;
}

// -----------------------------------------------------------------------

bool SvxLineItem::ScaleMetrics( long nMult, long nDiv )
{
    if ( pLine ) pLine->ScaleMetrics( nMult, nDiv );
    return true;
}

// -----------------------------------------------------------------------

bool SvxLineItem::HasMetrics() const
{
    return true;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxLineItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    SvxLineItem* _pLine = new SvxLineItem( Which() );
    short        nOutline, nInline, nDistance;
    Color        aColor;

    rStrm >> aColor >> nOutline >> nInline >> nDistance;
    if( nOutline )
    {
        SvxBorderLine aLine( &aColor );
        aLine.GuessLinesWidths(NONE, nOutline, nInline, nDistance);
        _pLine->SetLine( &aLine );
    }
    return _pLine;
}

// -----------------------------------------------------------------------

void SvxLineItem::SetLine( const SvxBorderLine* pNew )
{
    delete pLine;
    pLine = pNew ? new SvxBorderLine( *pNew ) : 0;
}

#ifdef _MSC_VER
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
    sal_Int8        nGraphicTransparency; //contains a percentage value which is
                                          //copied to the GraphicObject when necessary
    Link            aDoneLink;
    SvStream*       pStream;

    SvxBrushItem_Impl( GraphicObject* p ) : pGraphicObject( p ), nGraphicTransparency(0), pStream(0) {}
};

// -----------------------------------------------------------------------

void SvxBrushItem::SetDoneLink( const Link& rLink )
{
    pImpl->aDoneLink = rLink;
}

// -----------------------------------------------------------------------

SvxBrushItem::SvxBrushItem( sal_uInt16 _nWhich ) :

    SfxPoolItem( _nWhich ),

    aColor      ( COL_TRANSPARENT ),
    pImpl       ( new SvxBrushItem_Impl( 0 ) ),
    pStrLink    ( NULL ),
    pStrFilter  ( NULL ),
    eGraphicPos ( GPOS_NONE ),
    bLoadAgain  ( sal_True )

{
}

// -----------------------------------------------------------------------

SvxBrushItem::SvxBrushItem( const Color& rColor, sal_uInt16 _nWhich) :

    SfxPoolItem( _nWhich ),

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
                            sal_uInt16 _nWhich ) :

    SfxPoolItem( _nWhich ),

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
                            SvxGraphicPosition ePos, sal_uInt16 _nWhich ) :

    SfxPoolItem( _nWhich ),

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
    SvxGraphicPosition ePos, sal_uInt16 _nWhich ) :

    SfxPoolItem( _nWhich ),

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
                            sal_uInt16 _nWhich ) :

    SfxPoolItem( _nWhich ),

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
            // UNICODE: rStream >> aRel;
            String aRel = rStream.ReadUniOrByteString(rStream.GetStreamCharSet());

            // TODO/MBA: how can we get a BaseURL here?!
            OSL_FAIL("No BaseURL!");
            String aAbs = INetURLObject::GetAbsURL( String(), aRel );
            DBG_ASSERT( aAbs.Len(), "Invalid URL!" );
            pStrLink = new String( aAbs );
        }

        if ( nDoLoad & LOAD_FILTER )
        {
            pStrFilter = new String;
            // UNICODE: rStream >> *pStrFilter;
            *pStrFilter = rStream.ReadUniOrByteString(rStream.GetStreamCharSet());
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
    delete pImpl->pGraphicObject;
    delete pImpl;
    delete pStrLink;
    delete pStrFilter;
}

// -----------------------------------------------------------------------

sal_uInt16 SvxBrushItem::GetVersion( sal_uInt16 /*nFileVersion*/ ) const
{
    return BRUSH_GRAPHIC_VERSION;
}

// -----------------------------------------------------------------------
static inline sal_Int8 lcl_PercentToTransparency(long nPercent)
{
    //0xff must not be returned!
    return sal_Int8(nPercent ? (50 + 0xfe * nPercent) / 100 : 0);
}
static inline sal_Int8 lcl_TransparencyToPercent(sal_Int32 nTrans)
{
    return (sal_Int8)((nTrans * 100 + 127) / 254);
}

bool SvxBrushItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId)
    {
        case MID_BACK_COLOR:
            rVal <<= (sal_Int32)( aColor.GetColor() );
        break;
        case MID_BACK_COLOR_R_G_B:
            rVal <<= (sal_Int32)( aColor.GetRGBColor() );
        break;
        case MID_BACK_COLOR_TRANSPARENCY:
            rVal <<= lcl_TransparencyToPercent(aColor.GetTransparency());
        break;
        case MID_GRAPHIC_POSITION:
            rVal <<= (style::GraphicLocation)(sal_Int16)eGraphicPos;
        break;

        case MID_GRAPHIC:
            SAL_WARN( "editeng.items", "not implemented" );
        break;

        case MID_GRAPHIC_TRANSPARENT:
            rVal = Bool2Any( aColor.GetTransparency() == 0xff );
        break;

        case MID_GRAPHIC_URL:
        {
            OUString sLink;
            if ( pStrLink )
                sLink = *pStrLink;
            else if( pImpl->pGraphicObject )
            {
                OUString sPrefix(
                    UNO_NAME_GRAPHOBJ_URLPREFIX);
                OUString sId(rtl::OStringToOUString(
                    pImpl->pGraphicObject->GetUniqueID(),
                    RTL_TEXTENCODING_ASCII_US));
                sLink = sPrefix + sId;
            }
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
        case MID_GRAPHIC_TRANSPARENCY :
            rVal <<= pImpl->nGraphicTransparency;
        break;
    }

    return true;
}

// -----------------------------------------------------------------------

bool SvxBrushItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId)
    {
        case MID_BACK_COLOR:
        case MID_BACK_COLOR_R_G_B:
        {
            sal_Int32 nCol = 0;
            if ( !( rVal >>= nCol ) )
                return sal_False;
            if(MID_BACK_COLOR_R_G_B == nMemberId)
            {
                nCol = COLORDATA_RGB( nCol );
                nCol += aColor.GetColor() & 0xff000000;
            }
            aColor = Color( nCol );
        }
        break;
        case MID_BACK_COLOR_TRANSPARENCY:
        {
            sal_Int32 nTrans = 0;
            if ( !( rVal >>= nTrans ) || nTrans < 0 || nTrans > 100 )
                return sal_False;
            aColor.SetTransparency(lcl_PercentToTransparency(nTrans));
        }
        break;

        case MID_GRAPHIC_POSITION:
        {
            style::GraphicLocation eLocation;
            if ( !( rVal>>=eLocation ) )
            {
                sal_Int32 nValue = 0;
                if ( !( rVal >>= nValue ) )
                    return sal_False;
                eLocation = (style::GraphicLocation)nValue;
            }
            SetGraphicPos( (SvxGraphicPosition)(sal_uInt16)eLocation );
        }
        break;

        case MID_GRAPHIC:
            SAL_WARN( "editeng.items", "not implemented" );
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
                if( 0 == sLink.compareToAscii( UNO_NAME_GRAPHOBJ_URLPKGPREFIX,
                                  sizeof(UNO_NAME_GRAPHOBJ_URLPKGPREFIX)-1 ) )
                {
                    OSL_FAIL( "package urls aren't implemented" );
                }
                else if( 0 == sLink.compareToAscii( UNO_NAME_GRAPHOBJ_URLPREFIX,
                                   sizeof(UNO_NAME_GRAPHOBJ_URLPREFIX)-1 ) )
                {
                    DELETEZ( pStrLink );
                    String sTmp( sLink );
                    rtl::OString sId(rtl::OUStringToOString(sTmp.Copy(
                        sizeof(UNO_NAME_GRAPHOBJ_URLPREFIX)-1),
                        RTL_TEXTENCODING_ASCII_US));
                    GraphicObject *pOldGrfObj = pImpl->pGraphicObject;
                    pImpl->pGraphicObject = new GraphicObject( sId );
                    ApplyGraphicTransparency_Impl();
                    delete pOldGrfObj;
                }
                else
                {
                    SetGraphicLink(sLink);
                }
                if ( !sLink.isEmpty() && eGraphicPos == GPOS_NONE )
                    eGraphicPos = GPOS_MM;
                else if( sLink.isEmpty() )
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
        case MID_GRAPHIC_TRANSPARENCY :
        {
            sal_Int32 nTmp = 0;
            rVal >>= nTmp;
            if(nTmp >= 0 && nTmp <= 100)
            {
                pImpl->nGraphicTransparency = sal_Int8(nTmp);
                if(pImpl->pGraphicObject)
                    ApplyGraphicTransparency_Impl();
            }
        }
        break;
    }

    return true;
}

// -----------------------------------------------------------------------

SfxItemPresentation SvxBrushItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
    ) const
{
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
                rText += EE_RESSTR(nId);
            }
            else
            {
                rText = EE_RESSTR(RID_SVXITEMS_GRAPHIC);
            }

            return ePres;
        }
        default: ;//prevent warning
    }

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
        {
            pImpl->pGraphicObject = new GraphicObject( *rItem.pImpl->pGraphicObject );
        }
    }
    pImpl->nGraphicTransparency = rItem.pImpl->nGraphicTransparency;
    return *this;
}

// -----------------------------------------------------------------------

int SvxBrushItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    SvxBrushItem& rCmp = (SvxBrushItem&)rAttr;
    sal_Bool bEqual = ( aColor == rCmp.aColor && eGraphicPos == rCmp.eGraphicPos &&
        pImpl->nGraphicTransparency == rCmp.pImpl->nGraphicTransparency);

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

SvStream& SvxBrushItem::Store( SvStream& rStream , sal_uInt16 /*nItemVersion*/ ) const
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
        OSL_FAIL("No BaseURL!");
        // TODO/MBA: how to get a BaseURL?!
        String aRel = INetURLObject::GetRelURL( String(), *pStrLink );
        // UNICODE: rStream << aRel;
        rStream.WriteUniOrByteString(aRel, rStream.GetStreamCharSet());
    }
    if ( pStrFilter )
    {
        // UNICODE: rStream << *pStrFilter;
        rStream.WriteUniOrByteString(*pStrFilter, rStream.GetStreamCharSet());
    }
    rStream << (sal_Int8)eGraphicPos;
    return rStream;
}

// -----------------------------------------------------------------------

void SvxBrushItem::PurgeMedium() const
{
    DELETEZ( pImpl->pStream );
}

// -----------------------------------------------------------------------
const GraphicObject* SvxBrushItem::GetGraphicObject() const
{
    if ( bLoadAgain && pStrLink && !pImpl->pGraphicObject )
    // when graphics already loaded, use as a cache
    {
        // only with "valid" names - empty names now allowed
        if( pStrLink->Len() )
        {
            pImpl->pStream = utl::UcbStreamHelper::CreateStream( *pStrLink, STREAM_STD_READ );
            if( pImpl->pStream && !pImpl->pStream->GetError() )
            {
                Graphic aGraphic;
                int nRes;
                pImpl->pStream->Seek( STREAM_SEEK_TO_BEGIN );
                nRes = GraphicFilter::GetGraphicFilter().
                    ImportGraphic( aGraphic, *pStrLink, *pImpl->pStream,
                                   GRFILTER_FORMAT_DONTKNOW, NULL, GRFILTER_I_FLAGS_DONT_SET_LOGSIZE_FOR_JPEG );

                if( nRes != GRFILTER_OK )
                {
                    const_cast < SvxBrushItem*> (this)->bLoadAgain = sal_False;
                }
                else
                {
                    pImpl->pGraphicObject = new GraphicObject;
                    pImpl->pGraphicObject->SetGraphic( aGraphic );
                    const_cast < SvxBrushItem*> (this)->ApplyGraphicTransparency_Impl();
                }
            }
            else
            {
                const_cast < SvxBrushItem*> (this)->bLoadAgain = sal_False;
            }
        }
    }

    return pImpl->pGraphicObject;
}

// -----------------------------------------------------------------------

const Graphic* SvxBrushItem::GetGraphic() const
{
    const GraphicObject* pGrafObj = GetGraphicObject();
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
            pImpl->pGraphicObject = new GraphicObject; // Creating a dummy
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

        ApplyGraphicTransparency_Impl();

        if ( GPOS_NONE == eGraphicPos )
            eGraphicPos = GPOS_MM; // None would be brush, then Default: middle
    }
    else
    {
        OSL_FAIL( "SetGraphic() on linked graphic! :-/" );
    }
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

        ApplyGraphicTransparency_Impl();

        if ( GPOS_NONE == eGraphicPos )
            eGraphicPos = GPOS_MM; // None would be brush, then Default: middle
    }
    else
    {
        OSL_FAIL( "SetGraphic() on linked graphic! :-/" );
    }
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
    // The switch is not the fastest, but the safest
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

SvxBrushItem::SvxBrushItem( const CntWallpaperItem& rItem, sal_uInt16 _nWhich ) :
    SfxPoolItem( _nWhich ),
    pImpl( new SvxBrushItem_Impl( 0 ) ),
    pStrLink(0),
    pStrFilter(0),
    bLoadAgain( sal_True )
{
    aColor = rItem.GetColor();

    if (!rItem.GetBitmapURL().isEmpty())
    {
        pStrLink    = new String( rItem.GetBitmapURL() );
        SetGraphicPos( WallpaperStyle2GraphicPos((WallpaperStyle)rItem.GetStyle() ) );
    }
}

#ifdef _MSC_VER
#pragma optimize ( "", on )
#endif

void  SvxBrushItem::ApplyGraphicTransparency_Impl()
{
    DBG_ASSERT(pImpl->pGraphicObject, "no GraphicObject available" );
    if(pImpl->pGraphicObject)
    {
        GraphicAttr aAttr(pImpl->pGraphicObject->GetAttr());
        aAttr.SetTransparency(lcl_PercentToTransparency(
                            pImpl->nGraphicTransparency));
        pImpl->pGraphicObject->SetAttr(aAttr);
    }
}
// class SvxFrameDirectionItem ----------------------------------------------

SvxFrameDirectionItem::SvxFrameDirectionItem( SvxFrameDirection nValue ,
                                            sal_uInt16 _nWhich )
    : SfxUInt16Item( _nWhich, (sal_uInt16)nValue )
{
}

SvxFrameDirectionItem::~SvxFrameDirectionItem()
{
}

int SvxFrameDirectionItem::operator==( const SfxPoolItem& rCmp ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rCmp), "unequal types" );

    return GetValue() == ((SvxFrameDirectionItem&)rCmp).GetValue();
}

SfxPoolItem* SvxFrameDirectionItem::Clone( SfxItemPool * ) const
{
    return new SvxFrameDirectionItem( *this );
}

SfxPoolItem* SvxFrameDirectionItem::Create( SvStream & rStrm, sal_uInt16 /*nVer*/ ) const
{
    sal_uInt16 nValue;
    rStrm >> nValue;
    return new SvxFrameDirectionItem( (SvxFrameDirection)nValue, Which() );
}

SvStream& SvxFrameDirectionItem::Store( SvStream & rStrm, sal_uInt16 /*nIVer*/ ) const
{
    sal_uInt16 nValue = GetValue();
    rStrm << nValue;
    return rStrm;
}

sal_uInt16 SvxFrameDirectionItem::GetVersion( sal_uInt16 nFVer ) const
{
    return SOFFICE_FILEFORMAT_50 > nFVer ? USHRT_MAX : 0;
}

SfxItemPresentation SvxFrameDirectionItem::GetPresentation(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *) const
{
    SfxItemPresentation eRet = ePres;
    switch( ePres )
    {
    case SFX_ITEM_PRESENTATION_NONE:
        rText.Erase();
        break;

    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        rText = EE_RESSTR( RID_SVXITEMS_FRMDIR_BEGIN + GetValue() );
        break;

    default:
        eRet = SFX_ITEM_PRESENTATION_NONE;
    }
    return eRet;
}

bool SvxFrameDirectionItem::PutValue( const com::sun::star::uno::Any& rVal,
                                             sal_uInt8 )
{
    sal_Int16 nVal = sal_Int16();
    sal_Bool bRet = ( rVal >>= nVal );
    if( bRet )
    {
        // translate WritingDirection2 constants into SvxFrameDirection
        switch( nVal )
        {
            case text::WritingMode2::LR_TB:
                SetValue( FRMDIR_HORI_LEFT_TOP );
                break;
            case text::WritingMode2::RL_TB:
                SetValue( FRMDIR_HORI_RIGHT_TOP );
                break;
            case text::WritingMode2::TB_RL:
                SetValue( FRMDIR_VERT_TOP_RIGHT );
                break;
            case text::WritingMode2::TB_LR:
                SetValue( FRMDIR_VERT_TOP_LEFT );
                break;
            case text::WritingMode2::PAGE:
                SetValue( FRMDIR_ENVIRONMENT );
                break;
            default:
                bRet = false;
                break;
        }
    }

    return bRet;
}

bool SvxFrameDirectionItem::QueryValue( com::sun::star::uno::Any& rVal,
                                            sal_uInt8 ) const
{
    // translate SvxFrameDirection into WritingDirection2
    sal_Int16 nVal;
    bool bRet = true;
    switch( GetValue() )
    {
        case FRMDIR_HORI_LEFT_TOP:
            nVal = text::WritingMode2::LR_TB;
            break;
        case FRMDIR_HORI_RIGHT_TOP:
            nVal = text::WritingMode2::RL_TB;
            break;
        case FRMDIR_VERT_TOP_RIGHT:
            nVal = text::WritingMode2::TB_RL;
            break;
        case FRMDIR_VERT_TOP_LEFT:
            nVal = text::WritingMode2::TB_LR;
            break;
        case FRMDIR_ENVIRONMENT:
            nVal = text::WritingMode2::PAGE;
            break;
        default:
            OSL_FAIL("Unknown SvxFrameDirection value!");
            bRet = false;
            break;
    }

    // return value + error state
    if( bRet )
    {
        rVal <<= nVal;
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
