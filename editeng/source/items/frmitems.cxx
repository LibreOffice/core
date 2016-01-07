/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
#include <com/sun/star/drawing/ShadingPattern.hpp>

#include <i18nutil/unicode.hxx>
#include <unotools/securityoptions.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <limits.h>
#include <comphelper/processfactory.hxx>
#include <svtools/grfmgr.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/types.hxx>
#include <svl/memberid.hrc>
#include <svl/cntwall.hxx>
#include <svtools/borderhelper.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/mapunit.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
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
#include <editeng/formatbreakitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/itemtype.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/memberids.hrc>
#include <editeng/editerr.hxx>
#include <libxml/xmlwriter.h>
#include <o3tl/enumrange.hxx>

using namespace ::editeng;
using namespace ::com::sun::star;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::table::BorderLineStyle;

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
        WriteColor( stream, l.GetColor() );
        stream.WriteUInt16( l.GetOutWidth() )
              .WriteUInt16( l.GetInWidth() )
              .WriteUInt16( l.GetDistance() );

        if (version >= BORDER_LINE_WITH_STYLE_VERSION)
               stream.WriteUInt16( l.GetBorderLineStyle() );

        return stream;
    }

    /// Creates a border line from a stream.
    SvxBorderLine CreateBorderLine(SvStream &stream, sal_uInt16 version)
    {
        sal_uInt16 nOutline, nInline, nDistance;
        sal_uInt16 nStyle = css::table::BorderLineStyle::NONE;
        Color aColor;
        ReadColor( stream, aColor ).ReadUInt16( nOutline ).ReadUInt16( nInline ).ReadUInt16( nDistance );

        if (version >= BORDER_LINE_WITH_STYLE_VERSION)
            stream.ReadUInt16( nStyle );

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


SfxPoolItem* SvxPaperBinItem::CreateDefault() { return new  SvxPaperBinItem(0);}
SfxPoolItem* SvxSizeItem::CreateDefault() { return new  SvxSizeItem(0);}
SfxPoolItem* SvxLRSpaceItem::CreateDefault() { return new  SvxLRSpaceItem(0);}
SfxPoolItem* SvxULSpaceItem::CreateDefault() { return new  SvxULSpaceItem(0);}
SfxPoolItem* SvxPrintItem::CreateDefault() { return new  SvxPrintItem(0);}
SfxPoolItem* SvxOpaqueItem::CreateDefault() { return new  SvxOpaqueItem(0);}
SfxPoolItem* SvxProtectItem::CreateDefault() { return new  SvxProtectItem(0);}
SfxPoolItem* SvxBrushItem::CreateDefault() { return new  SvxBrushItem(0);}
SfxPoolItem* SvxShadowItem::CreateDefault() { return new  SvxShadowItem(0);}
SfxPoolItem* SvxBoxItem::CreateDefault() { return new  SvxBoxItem(0);}
SfxPoolItem* SvxBoxInfoItem::CreateDefault() { return new  SvxBoxInfoItem(0);}
SfxPoolItem* SvxFormatBreakItem::CreateDefault() { return new  SvxFormatBreakItem(SVX_BREAK_NONE, 0);}
SfxPoolItem* SvxFormatKeepItem::CreateDefault() { return new  SvxFormatKeepItem(false, 0);}
SfxPoolItem* SvxLineItem::CreateDefault() { return new  SvxLineItem(0);}
SfxPoolItem* SvxFrameDirectionItem::CreateDefault() { return new  SvxFrameDirectionItem(FRMDIR_HORI_LEFT_TOP, 0);}


// class SvxPaperBinItem ------------------------------------------------

SfxPoolItem* SvxPaperBinItem::Clone( SfxItemPool* ) const
{
    return new SvxPaperBinItem( *this );
}



SvStream& SvxPaperBinItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteUChar( GetValue() );
    return rStrm;
}



SfxPoolItem* SvxPaperBinItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 nBin;
    rStrm.ReadSChar( nBin );
    return new SvxPaperBinItem( Which(), nBin );
}



bool SvxPaperBinItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = OUString::number( GetValue() );
            return true;

        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            sal_uInt8 nValue = GetValue();

            if ( PAPERBIN_PRINTER_SETTINGS == nValue )
                rText = EE_RESSTR(RID_SVXSTR_PAPERBIN_SETTINGS);
            else
            {
                rText = EE_RESSTR(RID_SVXSTR_PAPERBIN) + " " + OUString::number( nValue );
            }
            return true;
        }
        //no break necessary
        default: ;//prevent warning
    }

    return false;
}

// class SvxSizeItem -----------------------------------------------------

SvxSizeItem::SvxSizeItem( const sal_uInt16 nId, const Size& rSize ) :

    SfxPoolItem( nId ),

    aSize( rSize )
{
}


bool SvxSizeItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    awt::Size aTmp(aSize.Width(), aSize.Height());
    if( bConvert )
    {
        aTmp.Height = convertTwipToMm100(aTmp.Height);
        aTmp.Width = convertTwipToMm100(aTmp.Width);
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

bool SvxSizeItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
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
                    aTmp.Height = convertMm100ToTwip(aTmp.Height);
                    aTmp.Width = convertMm100ToTwip(aTmp.Width);
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

            aSize.Width() = bConvert ? convertMm100ToTwip(nVal) : nVal;
        }
        break;
        case MID_SIZE_HEIGHT:
        {
            sal_Int32 nVal = 0;
            if(!(rVal >>= nVal))
                return true;

            aSize.Height() = bConvert ? convertMm100ToTwip(nVal) : nVal;
        }
        break;
        default: OSL_FAIL("Wrong MemberId!");
            return false;
    }
    return true;
}



SvxSizeItem::SvxSizeItem( const sal_uInt16 nId ) :

    SfxPoolItem( nId )
{
}



bool SvxSizeItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return ( aSize == static_cast<const SvxSizeItem&>( rAttr ).GetSize() );
}



SfxPoolItem* SvxSizeItem::Clone( SfxItemPool* ) const
{
    return new SvxSizeItem( *this );
}



bool SvxSizeItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    OUString&           rText, const IntlWrapper *pIntl
)   const
{
    OUString cpDelimTmp(cpDelim);
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = GetMetricText( aSize.Width(), eCoreUnit, ePresUnit, pIntl ) +
                    cpDelimTmp +
                    GetMetricText( aSize.Height(), eCoreUnit, ePresUnit, pIntl );
            return true;

        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = EE_RESSTR(RID_SVXITEMS_SIZE_WIDTH) +
                    GetMetricText( aSize.Width(), eCoreUnit, ePresUnit, pIntl ) +
                    " " + EE_RESSTR(GetMetricId(ePresUnit)) +
                    cpDelimTmp +
                    EE_RESSTR(RID_SVXITEMS_SIZE_HEIGHT) +
                    GetMetricText( aSize.Height(), eCoreUnit, ePresUnit, pIntl ) +
                    " " + EE_RESSTR(GetMetricId(ePresUnit));
            return true;
        //no break necessary
        default: ;//prevent warning

    }
    return false;
}



SvStream& SvxSizeItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteInt32( aSize.Width() );
    rStrm.WriteInt32( aSize.Height() );
    return rStrm;
}



bool SvxSizeItem::ScaleMetrics( long nMult, long nDiv )
{
    aSize.Width() = Scale( aSize.Width(), nMult, nDiv );
    aSize.Height() = Scale( aSize.Height(), nMult, nDiv );
    return true;
}



bool SvxSizeItem::HasMetrics() const
{
    return true;
}



SfxPoolItem* SvxSizeItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int32 nWidth(0), nHeight(0);
    rStrm.ReadInt32( nWidth ).ReadInt32( nHeight );

    SvxSizeItem* pAttr = new SvxSizeItem( Which() );
    pAttr->SetSize(Size(nWidth, nHeight));

    return pAttr;
}

// class SvxLRSpaceItem --------------------------------------------------

SvxLRSpaceItem::SvxLRSpaceItem( const sal_uInt16 nId ) :

    SfxPoolItem( nId ),

    nTxtLeft        ( 0 ),
    nLeftMargin     ( 0 ),
    nRightMargin    ( 0 ),
    nPropFirstLineOfst( 100 ),
    nPropLeftMargin( 100 ),
    nPropRightMargin( 100 ),
    nFirstLineOfst  ( 0 ),
    bAutoFirst      ( false ),
    bExplicitZeroMarginValRight(false),
    bExplicitZeroMarginValLeft(false)
{
}



SvxLRSpaceItem::SvxLRSpaceItem( const long nLeft, const long nRight,
                                const long nTLeft, const short nOfset,
                                const sal_uInt16 nId ) :

    SfxPoolItem( nId ),

    nTxtLeft        ( nTLeft ),
    nLeftMargin     ( nLeft ),
    nRightMargin    ( nRight ),
    nPropFirstLineOfst( 100 ),
    nPropLeftMargin( 100 ),
    nPropRightMargin( 100 ),
    nFirstLineOfst  ( nOfset ),
    bAutoFirst      ( false ),
    bExplicitZeroMarginValRight(false),
    bExplicitZeroMarginValLeft(false)
{
}


bool SvxLRSpaceItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bRet = true;
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        // now all signed
        case MID_L_MARGIN:
            rVal <<= (sal_Int32)(bConvert ? convertTwipToMm100(nLeftMargin) : nLeftMargin);
            break;

        case MID_TXT_LMARGIN :
            rVal <<= (sal_Int32)(bConvert ? convertTwipToMm100(nTxtLeft) : nTxtLeft);
        break;
        case MID_R_MARGIN:
            rVal <<= (sal_Int32)(bConvert ? convertTwipToMm100(nRightMargin) : nRightMargin);
            break;
        case MID_L_REL_MARGIN:
            rVal <<= (sal_Int16)nPropLeftMargin;
        break;
        case MID_R_REL_MARGIN:
            rVal <<= (sal_Int16)nPropRightMargin;
        break;

        case MID_FIRST_LINE_INDENT:
            rVal <<= (sal_Int32)(bConvert ? convertTwipToMm100(nFirstLineOfst) : nFirstLineOfst);
            break;

        case MID_FIRST_LINE_REL_INDENT:
            rVal <<= (sal_Int16)(nPropFirstLineOfst);
            break;

        case MID_FIRST_AUTO:
            rVal = css::uno::makeAny<bool>(IsAutoFirst());
            break;

        default:
            bRet = false;
            OSL_FAIL("unknown MemberId");
    }
    return bRet;
}


bool SvxLRSpaceItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = 0 != (nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    sal_Int32 nVal = 0;
    if( nMemberId != MID_FIRST_AUTO &&
            nMemberId != MID_L_REL_MARGIN && nMemberId != MID_R_REL_MARGIN)
        if(!(rVal >>= nVal))
            return false;

    switch( nMemberId )
    {
        case MID_L_MARGIN:
            SetLeft( bConvert ? convertMm100ToTwip(nVal) : nVal );
            break;

        case MID_TXT_LMARGIN :
            SetTextLeft( bConvert ? convertMm100ToTwip(nVal) : nVal );
        break;

        case MID_R_MARGIN:
            SetRight(bConvert ? convertMm100ToTwip(nVal) : nVal);
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
            SetTextFirstLineOfst((short)(bConvert ?  convertMm100ToTwip(nVal) : nVal));
            break;

        case MID_FIRST_LINE_REL_INDENT:
            SetPropTextFirstLineOfst ( (sal_uInt16)nVal );
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



// Adapt nLeftMargin and nTxtLeft.

void SvxLRSpaceItem::AdjustLeft()
{
    if ( 0 > nFirstLineOfst )
        nLeftMargin = nTxtLeft + nFirstLineOfst;
    else
        nLeftMargin = nTxtLeft;
}



bool SvxLRSpaceItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxLRSpaceItem& rOther = static_cast<const SvxLRSpaceItem&>(rAttr);

    return (
        nFirstLineOfst == rOther.GetTextFirstLineOfst() &&
        nTxtLeft == rOther.GetTextLeft() &&
        nLeftMargin == rOther.GetLeft()  &&
        nRightMargin == rOther.GetRight() &&
        nPropFirstLineOfst == rOther.GetPropTextFirstLineOfst() &&
        nPropLeftMargin == rOther.GetPropLeft()  &&
        nPropRightMargin == rOther.GetPropRight() &&
        bAutoFirst == rOther.IsAutoFirst() &&
        bExplicitZeroMarginValRight == rOther.IsExplicitZeroMarginValRight() &&
        bExplicitZeroMarginValLeft == rOther.IsExplicitZeroMarginValLeft() );
}



SfxPoolItem* SvxLRSpaceItem::Clone( SfxItemPool* ) const
{
    return new SvxLRSpaceItem( *this );
}



bool SvxLRSpaceItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    OUString&           rText, const IntlWrapper* pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            if ( 100 != nPropLeftMargin )
            {
                rText = unicode::formatPercent(nPropLeftMargin,
                    Application::GetSettings().GetUILanguageTag());
            }
            else
                rText = GetMetricText( (long)nLeftMargin,
                                       eCoreUnit, ePresUnit, pIntl );
            rText += OUString(cpDelim);
            if ( 100 != nPropFirstLineOfst )
            {
                rText += unicode::formatPercent(nPropFirstLineOfst,
                    Application::GetSettings().GetUILanguageTag());
            }
            else
                rText += GetMetricText( (long)nFirstLineOfst,
                                        eCoreUnit, ePresUnit, pIntl );
            rText += OUString(cpDelim);
            if ( 100 != nRightMargin )
            {
                rText += unicode::formatPercent(nRightMargin,
                    Application::GetSettings().GetUILanguageTag());
            }
            else
                rText += GetMetricText( (long)nRightMargin,
                                        eCoreUnit, ePresUnit, pIntl );
            return true;
        }
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = EE_RESSTR(RID_SVXITEMS_LRSPACE_LEFT);
            if ( 100 != nPropLeftMargin )
                rText += unicode::formatPercent(nPropLeftMargin,
                    Application::GetSettings().GetUILanguageTag());
            else
            {
                rText = rText +
                        GetMetricText( (long)nLeftMargin, eCoreUnit, ePresUnit, pIntl ) +
                        " " + EE_RESSTR(GetMetricId(ePresUnit));
            }
            rText += OUString(cpDelim);
            if ( 100 != nPropFirstLineOfst || nFirstLineOfst )
            {
                rText += EE_RESSTR(RID_SVXITEMS_LRSPACE_FLINE);
                if ( 100 != nPropFirstLineOfst )
                    rText = rText + unicode::formatPercent(nPropFirstLineOfst,
                    Application::GetSettings().GetUILanguageTag());
                else
                {
                    rText = rText +
                            GetMetricText( (long)nFirstLineOfst,
                                            eCoreUnit, ePresUnit, pIntl ) +
                            " " + EE_RESSTR(GetMetricId(ePresUnit));
                }
                rText += OUString(cpDelim);
            }
            rText += EE_RESSTR(RID_SVXITEMS_LRSPACE_RIGHT);
            if ( 100 != nPropRightMargin )
                rText = rText + unicode::formatPercent(nPropRightMargin,
                    Application::GetSettings().GetUILanguageTag());
            else
            {
                rText = rText +
                        GetMetricText( (long)nRightMargin,
                                       eCoreUnit, ePresUnit, pIntl ) +
                        " " + EE_RESSTR(GetMetricId(ePresUnit));
            }
            return true;
        }
        default: ;//prevent warning
    }
    return false;
}



// BulletFI: Before 501 in the Outliner the bullet was not on the position of
// the FI, so in older documents one must set FI to 0.
#define BULLETLR_MARKER 0x599401FE

SvStream& SvxLRSpaceItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    short nSaveFI = nFirstLineOfst;
    const_cast<SvxLRSpaceItem*>(this)->SetTextFirstLineOfst( 0 );  // nLeftMargin is manipulated together with this, see Create()

    sal_uInt16 nMargin = 0;
    if( nLeftMargin > 0 )
        nMargin = sal_uInt16( nLeftMargin );
    rStrm.WriteUInt16( nMargin );
    rStrm.WriteUInt16( nPropLeftMargin );
    if( nRightMargin > 0 )
        nMargin = sal_uInt16( nRightMargin );
    else
        nMargin = 0;
    rStrm.WriteUInt16( nMargin );
    rStrm.WriteUInt16( nPropRightMargin );
    rStrm.WriteInt16( nFirstLineOfst );
    rStrm.WriteUInt16( nPropFirstLineOfst );
    if( nTxtLeft > 0 )
        nMargin = sal_uInt16( nTxtLeft );
    else
        nMargin = 0;
    rStrm.WriteUInt16( nMargin );
    if( nItemVersion >= LRSPACE_AUTOFIRST_VERSION )
    {
        sal_Int8 nAutoFirst = bAutoFirst ? 1 : 0;
        if( nItemVersion >= LRSPACE_NEGATIVE_VERSION &&
            ( nLeftMargin < 0 || nRightMargin < 0 || nTxtLeft < 0 ) )
            nAutoFirst |= 0x80;
        rStrm.WriteSChar( nAutoFirst );

        // From 6.0 onwards, do not write Magic numbers...
        DBG_ASSERT( rStrm.GetVersion() <= SOFFICE_FILEFORMAT_50, "Change File format SvxLRSpaceItem!" );
        rStrm.WriteUInt32( BULLETLR_MARKER );
        rStrm.WriteInt16( nSaveFI );

        if( 0x80 & nAutoFirst )
        {
            rStrm.WriteInt32( nLeftMargin );
            rStrm.WriteInt32( nRightMargin );
        }
    }

    const_cast<SvxLRSpaceItem*>(this)->SetTextFirstLineOfst( nSaveFI );

    return rStrm;
}



SfxPoolItem* SvxLRSpaceItem::Create( SvStream& rStrm, sal_uInt16 nVersion ) const
{
    sal_uInt16 left, prpleft, right, prpright, prpfirstline, txtleft;
    short firstline;
    sal_Int8 autofirst = 0;

    if ( nVersion >= LRSPACE_AUTOFIRST_VERSION )
    {
        rStrm.ReadUInt16( left ).ReadUInt16( prpleft ).ReadUInt16( right ).ReadUInt16( prpright ).ReadInt16( firstline ).                 ReadUInt16( prpfirstline ).ReadUInt16( txtleft ).ReadSChar( autofirst );

        sal_Size nPos = rStrm.Tell();
        sal_uInt32 nMarker;
        rStrm.ReadUInt32( nMarker );
        if ( nMarker == BULLETLR_MARKER )
        {
            rStrm.ReadInt16( firstline );
            if ( firstline < 0 )
                left = left + static_cast<sal_uInt16>(firstline);   // see below: txtleft = ...
        }
        else
            rStrm.Seek( nPos );
    }
    else if ( nVersion == LRSPACE_TXTLEFT_VERSION )
    {
        rStrm.ReadUInt16( left ).ReadUInt16( prpleft ).ReadUInt16( right ).ReadUInt16( prpright ).ReadInt16( firstline ).                 ReadUInt16( prpfirstline ).ReadUInt16( txtleft );
    }
    else if ( nVersion == LRSPACE_16_VERSION )
    {
        rStrm.ReadUInt16( left ).ReadUInt16( prpleft ).ReadUInt16( right ).ReadUInt16( prpright ).ReadInt16( firstline ).                 ReadUInt16( prpfirstline );
    }
    else
    {
        sal_Int8 nL, nR, nFL;
        rStrm.ReadUInt16( left ).ReadSChar( nL ).ReadUInt16( right ).ReadSChar( nR ).ReadInt16( firstline ).ReadSChar( nFL );
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
        rStrm.ReadInt32( nMargin );
        pAttr->nLeftMargin = nMargin;
        pAttr->nTxtLeft = firstline >= 0 ? nMargin : nMargin - firstline;
        rStrm.ReadInt32( nMargin );
        pAttr->nRightMargin = nMargin;
    }
    return pAttr;
}



sal_uInt16 SvxLRSpaceItem::GetVersion( sal_uInt16 nFileVersion ) const
{
    return (nFileVersion == SOFFICE_FILEFORMAT_31)
               ? LRSPACE_TXTLEFT_VERSION
               : LRSPACE_NEGATIVE_VERSION;
}



bool SvxLRSpaceItem::ScaleMetrics( long nMult, long nDiv )
{
    nFirstLineOfst = (short)Scale( nFirstLineOfst, nMult, nDiv );
    nTxtLeft = Scale( nTxtLeft, nMult, nDiv );
    nLeftMargin = Scale( nLeftMargin, nMult, nDiv );
    nRightMargin = Scale( nRightMargin, nMult, nDiv );
    return true;
}



bool SvxLRSpaceItem::HasMetrics() const
{
    return true;
}

void SvxLRSpaceItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("svxLRSpaceItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nFirstLineOfst"), BAD_CAST(OString::number(nFirstLineOfst).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nTxtLeft"), BAD_CAST(OString::number(nTxtLeft).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nLeftMargin"), BAD_CAST(OString::number(nLeftMargin).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nRightMargin"), BAD_CAST(OString::number(nRightMargin).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nPropFirstLineOfst"), BAD_CAST(OString::number(nPropFirstLineOfst).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nPropLeftMargin"), BAD_CAST(OString::number(nPropLeftMargin).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nPropRightMargin"), BAD_CAST(OString::number(nPropRightMargin).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("bAutoFirst"), BAD_CAST(OString::number(int(bAutoFirst)).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("bExplicitZeroMarginValRight"), BAD_CAST(OString::number(int(bExplicitZeroMarginValRight)).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("bExplicitZeroMarginValLeft"), BAD_CAST(OString::number(int(bExplicitZeroMarginValLeft)).getStr()));
    xmlTextWriterEndElement(pWriter);
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


bool    SvxULSpaceItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        // now all signed
        case 0:
        {
            css::frame::status::UpperLowerMarginScale aUpperLowerMarginScale;
            aUpperLowerMarginScale.Upper = (sal_Int32)(bConvert ? convertTwipToMm100(nUpper) : nUpper);
            aUpperLowerMarginScale.Lower = (sal_Int32)(bConvert ? convertTwipToMm100(nLower) : nPropUpper);
            aUpperLowerMarginScale.ScaleUpper = (sal_Int16)nPropUpper;
            aUpperLowerMarginScale.ScaleLower = (sal_Int16)nPropLower;
            rVal <<= aUpperLowerMarginScale;
            break;
        }
        case MID_UP_MARGIN: rVal <<= (sal_Int32)(bConvert ? convertTwipToMm100(nUpper) : nUpper); break;
        case MID_LO_MARGIN: rVal <<= (sal_Int32)(bConvert ? convertTwipToMm100(nLower) : nLower); break;
        case MID_CTX_MARGIN: rVal <<= bContext; break;
        case MID_UP_REL_MARGIN: rVal <<= (sal_Int16) nPropUpper; break;
        case MID_LO_REL_MARGIN: rVal <<= (sal_Int16) nPropLower; break;
    }
    return true;
}


bool SvxULSpaceItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    sal_Int32 nVal = 0;
    bool bVal = false;
    switch( nMemberId )
    {
        case 0:
        {
            css::frame::status::UpperLowerMarginScale aUpperLowerMarginScale;
            if ( !(rVal >>= aUpperLowerMarginScale ))
                return false;
            {
                SetUpper((sal_uInt16)(bConvert ? convertMm100ToTwip( aUpperLowerMarginScale.Upper ) : aUpperLowerMarginScale.Upper));
                SetLower((sal_uInt16)(bConvert ? convertMm100ToTwip( aUpperLowerMarginScale.Lower ) : aUpperLowerMarginScale.Lower));
                if( aUpperLowerMarginScale.ScaleUpper > 1 )
                    nPropUpper = aUpperLowerMarginScale.ScaleUpper;
                if( aUpperLowerMarginScale.ScaleLower > 1 )
                    nPropUpper = aUpperLowerMarginScale.ScaleLower;
            }
        }
        break;
        case MID_UP_MARGIN :
            if(!(rVal >>= nVal) || nVal < 0)
                return false;
            SetUpper((sal_uInt16)(bConvert ? convertMm100ToTwip(nVal) : nVal));
            break;
        case MID_LO_MARGIN :
            if(!(rVal >>= nVal) || nVal < 0)
                return false;
            SetLower((sal_uInt16)(bConvert ? convertMm100ToTwip(nVal) : nVal));
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



bool SvxULSpaceItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxULSpaceItem& rSpaceItem = static_cast<const SvxULSpaceItem&>( rAttr );
    return ( nUpper == rSpaceItem.nUpper &&
             nLower == rSpaceItem.nLower &&
             bContext == rSpaceItem.bContext &&
             nPropUpper == rSpaceItem.nPropUpper &&
             nPropLower == rSpaceItem.nPropLower );
}



SfxPoolItem* SvxULSpaceItem::Clone( SfxItemPool* ) const
{
    return new SvxULSpaceItem( *this );
}



bool SvxULSpaceItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    OUString&           rText,
    const IntlWrapper  *pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            if ( 100 != nPropUpper )
            {
                rText = unicode::formatPercent(nPropUpper,
                    Application::GetSettings().GetUILanguageTag());
            }
            else
                rText = GetMetricText( (long)nUpper, eCoreUnit, ePresUnit, pIntl );
            rText += OUString(cpDelim);
            if ( 100 != nPropLower )
            {
                rText += unicode::formatPercent(nPropLower,
                    Application::GetSettings().GetUILanguageTag());
            }
            else
                rText += GetMetricText( (long)nLower, eCoreUnit, ePresUnit, pIntl );
            return true;
        }
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = EE_RESSTR(RID_SVXITEMS_ULSPACE_UPPER);
            if ( 100 != nPropUpper )
            {
                rText += unicode::formatPercent(nPropUpper,
                    Application::GetSettings().GetUILanguageTag());
            }
            else
            {
                rText = rText +
                        GetMetricText( (long)nUpper, eCoreUnit, ePresUnit, pIntl ) +
                        " " + EE_RESSTR(GetMetricId(ePresUnit));
            }
            rText = rText + OUString(cpDelim) + EE_RESSTR(RID_SVXITEMS_ULSPACE_LOWER);
            if ( 100 != nPropLower )
            {
                rText += unicode::formatPercent(nPropLower,
                    Application::GetSettings().GetUILanguageTag());
            }
            else
            {
                rText = rText +
                        GetMetricText( (long)nLower, eCoreUnit, ePresUnit, pIntl ) +
                        " " + EE_RESSTR(GetMetricId(ePresUnit));
            }
            return true;
        }
        default: ;//prevent warning
    }
    return false;
}



SvStream& SvxULSpaceItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteUInt16( GetUpper() )
         .WriteUInt16( GetPropUpper() )
         .WriteUInt16( GetLower() )
         .WriteUInt16( GetPropLower() );
    return rStrm;
}



SfxPoolItem* SvxULSpaceItem::Create( SvStream& rStrm, sal_uInt16 nVersion ) const
{
    sal_uInt16 upper, lower, nPL = 0, nPU = 0;

    if ( nVersion == ULSPACE_16_VERSION )
        rStrm.ReadUInt16( upper ).ReadUInt16( nPU ).ReadUInt16( lower ).ReadUInt16( nPL );
    else
    {
        sal_Int8 nU, nL;
        rStrm.ReadUInt16( upper ).ReadSChar( nU ).ReadUInt16( lower ).ReadSChar( nL );
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



sal_uInt16 SvxULSpaceItem::GetVersion( sal_uInt16 /*nFileVersion*/ ) const
{
    return ULSPACE_16_VERSION;
}



bool SvxULSpaceItem::ScaleMetrics( long nMult, long nDiv )
{
    nUpper = (sal_uInt16)Scale( nUpper, nMult, nDiv );
    nLower = (sal_uInt16)Scale( nLower, nMult, nDiv );
    return true;
}



bool SvxULSpaceItem::HasMetrics() const
{
    return true;
}

void SvxULSpaceItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("svxULSpaceItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nUpper"), BAD_CAST(OString::number(nUpper).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nLower"), BAD_CAST(OString::number(nLower).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("bContext"), BAD_CAST(OString::boolean(bContext).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nPropUpper"), BAD_CAST(OString::number(nPropUpper).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nPropLower"), BAD_CAST(OString::number(nPropLower).getStr()));
    xmlTextWriterEndElement(pWriter);
}

// class SvxPrintItem ----------------------------------------------------

SfxPoolItem* SvxPrintItem::Clone( SfxItemPool* ) const
{
    return new SvxPrintItem( *this );
}



SvStream& SvxPrintItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteSChar( (sal_Int8)GetValue() );
    return rStrm;
}



SfxPoolItem* SvxPrintItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 bIsPrint;
    rStrm.ReadSChar( bIsPrint );
    return new SvxPrintItem( Which(), bIsPrint != 0 );
}



bool SvxPrintItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    sal_uInt16 nId = RID_SVXITEMS_PRINT_FALSE;

    if ( GetValue() )
        nId = RID_SVXITEMS_PRINT_TRUE;
    rText = EE_RESSTR(nId);
    return true;
}

// class SvxOpaqueItem ---------------------------------------------------

SfxPoolItem* SvxOpaqueItem::Clone( SfxItemPool* ) const
{
    return new SvxOpaqueItem( *this );
}



SvStream& SvxOpaqueItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteSChar( (sal_Int8)GetValue() );
    return rStrm;
}



SfxPoolItem* SvxOpaqueItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 bIsOpaque;
    rStrm.ReadSChar( bIsOpaque );
    return new SvxOpaqueItem( Which(), bIsOpaque != 0 );
}



bool SvxOpaqueItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    sal_uInt16 nId = RID_SVXITEMS_OPAQUE_FALSE;

    if ( GetValue() )
        nId = RID_SVXITEMS_OPAQUE_TRUE;
    rText = EE_RESSTR(nId);
    return true;
}

// class SvxProtectItem --------------------------------------------------

bool SvxProtectItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxProtectItem& rItem = static_cast<const SvxProtectItem&>(rAttr);
    return ( bCntnt == rItem.bCntnt &&
             bSize  == rItem.bSize  &&
             bPos   == rItem.bPos );
}

bool SvxProtectItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bValue;
    switch(nMemberId)
    {
        case MID_PROTECT_CONTENT :  bValue = bCntnt; break;
        case MID_PROTECT_SIZE    :  bValue = bSize; break;
        case MID_PROTECT_POSITION:  bValue = bPos; break;
        default:
            OSL_FAIL("Wrong MemberId");
            return false;
    }

    rVal = css::uno::makeAny<bool>( bValue );
    return true;
}

bool    SvxProtectItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bVal( Any2Bool(rVal) );
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



SfxPoolItem* SvxProtectItem::Clone( SfxItemPool* ) const
{
    return new SvxProtectItem( *this );
}



bool SvxProtectItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    sal_uInt16 nId = RID_SVXITEMS_PROT_CONTENT_FALSE;

    if ( bCntnt )
        nId = RID_SVXITEMS_PROT_CONTENT_TRUE;
    rText = EE_RESSTR(nId) + OUString(cpDelim);
    nId = RID_SVXITEMS_PROT_SIZE_FALSE;

    if ( bSize )
        nId = RID_SVXITEMS_PROT_SIZE_TRUE;
    rText = rText + EE_RESSTR(nId) + OUString(cpDelim);
    nId = RID_SVXITEMS_PROT_POS_FALSE;

    if ( bPos )
        nId = RID_SVXITEMS_PROT_POS_TRUE;
    rText += EE_RESSTR(nId);
    return true;
}



SvStream& SvxProtectItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    sal_Int8 cProt = 0;
    if( IsPosProtected() )   cProt |= 0x01;
    if( IsSizeProtected() )  cProt |= 0x02;
    if( IsContentProtected() ) cProt |= 0x04;
    rStrm.WriteSChar( cProt );
    return rStrm;
}



SfxPoolItem* SvxProtectItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 cFlags;
    rStrm.ReadSChar( cFlags );
    SvxProtectItem* pAttr = new SvxProtectItem( Which() );
    pAttr->SetPosProtect( ( cFlags & 0x01 ) != 0 );
    pAttr->SetSizeProtect( ( cFlags & 0x02 ) != 0 );
    pAttr->SetContentProtect( ( cFlags & 0x04 ) != 0 );
    return pAttr;
}

void SvxProtectItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("svxProtectItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("content"), BAD_CAST(OString::boolean(bCntnt).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("size"), BAD_CAST(OString::boolean(bSize).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("position"), BAD_CAST(OString::boolean(bPos).getStr()));
    xmlTextWriterEndElement(pWriter);
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


bool SvxShadowItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
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
    aShadow.ShadowWidth =   bConvert ? convertTwipToMm100(nWidth) : nWidth;
    aShadow.IsTransparent = aShadowColor.GetTransparency() > 0;
    aShadow.Color = aShadowColor.GetColor();

    sal_Int8 nTransparence = rtl::math::round(float(aShadowColor.GetTransparency() * 100) / 255);

    switch ( nMemberId )
    {
        case MID_LOCATION: rVal <<= aShadow.Location; break;
        case MID_WIDTH: rVal <<= aShadow.ShadowWidth; break;
        case MID_TRANSPARENT: rVal <<= aShadow.IsTransparent; break;
        case MID_BG_COLOR: rVal <<= aShadow.Color; break;
        case 0: rVal <<= aShadow; break;
        case MID_SHADOW_TRANSPARENCE: rVal <<= nTransparence; break;
        default: OSL_FAIL("Wrong MemberId!"); return false;
    }

    return true;
}

bool SvxShadowItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
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
        case MID_SHADOW_TRANSPARENCE:
        {
            sal_Int32 nTransparence = 0;
            if (rVal >>= nTransparence)
            {
                Color aColor(aShadow.Color);
                aColor.SetTransparency(rtl::math::round(float(nTransparence * 255) / 100));
                aShadow.Color = aColor.GetColor();
            }
            break;
        }
        default: OSL_FAIL("Wrong MemberId!"); return false;
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

        nWidth = bConvert ? convertMm100ToTwip(aShadow.ShadowWidth) : aShadow.ShadowWidth;
        Color aSet(aShadow.Color);
        aShadowColor = aSet;
    }

    return bRet;
}



bool SvxShadowItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxShadowItem& rItem = static_cast<const SvxShadowItem&>(rAttr);
    return ( ( aShadowColor == rItem.aShadowColor ) &&
             ( nWidth    == rItem.GetWidth() ) &&
             ( eLocation == rItem.GetLocation() ) );
}



SfxPoolItem* SvxShadowItem::Clone( SfxItemPool* ) const
{
    return new SvxShadowItem( *this );
}



sal_uInt16 SvxShadowItem::CalcShadowSpace( SvxShadowItemSide nShadow ) const
{
    sal_uInt16 nSpace = 0;

    switch ( nShadow )
    {
        case SvxShadowItemSide::TOP:
            if ( eLocation == SVX_SHADOW_TOPLEFT ||
                 eLocation == SVX_SHADOW_TOPRIGHT  )
                nSpace = nWidth;
            break;

        case SvxShadowItemSide::BOTTOM:
            if ( eLocation == SVX_SHADOW_BOTTOMLEFT ||
                 eLocation == SVX_SHADOW_BOTTOMRIGHT  )
                nSpace = nWidth;
            break;

        case SvxShadowItemSide::LEFT:
            if ( eLocation == SVX_SHADOW_TOPLEFT ||
                 eLocation == SVX_SHADOW_BOTTOMLEFT )
                nSpace = nWidth;
            break;

        case SvxShadowItemSide::RIGHT:
            if ( eLocation == SVX_SHADOW_TOPRIGHT ||
                 eLocation == SVX_SHADOW_BOTTOMRIGHT )
                nSpace = nWidth;
            break;

        default:
            OSL_FAIL( "wrong shadow" );
    }
    return nSpace;
}



bool SvxShadowItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    OUString&           rText, const IntlWrapper *pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            rText = ::GetColorString( aShadowColor ) + OUString(cpDelim);
            sal_uInt16 nId = RID_SVXITEMS_TRANSPARENT_FALSE;

            if ( aShadowColor.GetTransparency() )
                nId = RID_SVXITEMS_TRANSPARENT_TRUE;
            rText = rText +
                    EE_RESSTR(nId) +
                    OUString(cpDelim) +
                    GetMetricText( (long)nWidth, eCoreUnit, ePresUnit, pIntl ) +
                    OUString(cpDelim) +
                    EE_RESSTR(RID_SVXITEMS_SHADOW_BEGIN + eLocation);
            return true;
        }
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = EE_RESSTR(RID_SVXITEMS_SHADOW_COMPLETE) +
                    ::GetColorString( aShadowColor ) +
                    OUString(cpDelim);

            sal_uInt16 nId = RID_SVXITEMS_TRANSPARENT_FALSE;
            if ( aShadowColor.GetTransparency() )
                nId = RID_SVXITEMS_TRANSPARENT_TRUE;
            rText = rText +
                    EE_RESSTR(nId) +
                    OUString(cpDelim) +
                    GetMetricText( (long)nWidth, eCoreUnit, ePresUnit, pIntl ) +
                    " " + EE_RESSTR(GetMetricId(ePresUnit)) +
                    OUString(cpDelim) +
                    EE_RESSTR(RID_SVXITEMS_SHADOW_BEGIN + eLocation);
            return true;
        }
        default: ;//prevent warning
    }
    return false;
}



SvStream& SvxShadowItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteSChar( GetLocation() )
         .WriteUInt16( GetWidth() )
         .WriteBool( aShadowColor.GetTransparency() > 0 );
    WriteColor( rStrm, GetColor() );
    WriteColor( rStrm, GetColor() );
    rStrm.WriteSChar( aShadowColor.GetTransparency() > 0 ? 0 : 1 ); //BRUSH_NULL : BRUSH_SOLID
    return rStrm;
}



bool SvxShadowItem::ScaleMetrics( long nMult, long nDiv )
{
    nWidth = (sal_uInt16)Scale( nWidth, nMult, nDiv );
    return true;
}



bool SvxShadowItem::HasMetrics() const
{
    return true;
}



SfxPoolItem* SvxShadowItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 cLoc;
    sal_uInt16 _nWidth;
    bool bTrans;
    Color aColor;
    Color aFillColor;
    sal_Int8 nStyle;
    rStrm.ReadSChar( cLoc ).ReadUInt16( _nWidth )
         .ReadCharAsBool( bTrans );
    ReadColor( rStrm, aColor );
    ReadColor( rStrm, aFillColor ).ReadSChar( nStyle );
    aColor.SetTransparency(bTrans ? 0xff : 0);
    return new SvxShadowItem( Which(), &aColor, _nWidth, (SvxShadowLocation)cLoc );
}



sal_uInt16 SvxShadowItem::GetValueCount() const
{
    return SVX_SHADOW_END;  // SVX_SHADOW_BOTTOMRIGHT + 1
}



OUString SvxShadowItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos < SVX_SHADOW_END, "enum overflow!" );
    return EE_RESSTR(RID_SVXITEMS_SHADOW_BEGIN + nPos );
}



sal_uInt16 SvxShadowItem::GetEnumValue() const
{
    return (sal_uInt16)GetLocation();
}



void SvxShadowItem::SetEnumValue( sal_uInt16 nVal )
{
    SetLocation( (const SvxShadowLocation)nVal );
}

void SvxShadowItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("svxShadowItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("aShadowColor"), BAD_CAST(aShadowColor.AsRGBHexString().toUtf8().getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nWidth"), BAD_CAST(OString::number(nWidth).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eLocation"), BAD_CAST(OString::number(eLocation).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"), BAD_CAST(EE_RESSTR(RID_SVXITEMS_SHADOW_BEGIN + eLocation).toUtf8().getStr()));
    xmlTextWriterEndElement(pWriter);
}

// class SvxBoxItem ------------------------------------------------------

SvxBoxItem::SvxBoxItem( const SvxBoxItem& rCpy ) :

    SfxPoolItem ( rCpy ),
    nTopDist    ( rCpy.nTopDist ),
    nBottomDist ( rCpy.nBottomDist ),
    nLeftDist   ( rCpy.nLeftDist ),
    nRightDist  ( rCpy.nRightDist ),
    bRemoveAdjCellBorder ( rCpy.bRemoveAdjCellBorder )

{
    pTop    = rCpy.GetTop()     ? new SvxBorderLine( *rCpy.GetTop() )    : nullptr;
    pBottom = rCpy.GetBottom()  ? new SvxBorderLine( *rCpy.GetBottom() ) : nullptr;
    pLeft   = rCpy.GetLeft()    ? new SvxBorderLine( *rCpy.GetLeft() )   : nullptr;
    pRight  = rCpy.GetRight()   ? new SvxBorderLine( *rCpy.GetRight() )  : nullptr;
}



SvxBoxItem::SvxBoxItem( const sal_uInt16 nId ) :
    SfxPoolItem( nId ),

    pTop        ( nullptr ),
    pBottom     ( nullptr ),
    pLeft       ( nullptr ),
    pRight      ( nullptr ),
    nTopDist    ( 0 ),
    nBottomDist ( 0 ),
    nLeftDist   ( 0 ),
    nRightDist  ( 0 ),
    bRemoveAdjCellBorder ( false )
{
}



SvxBoxItem::~SvxBoxItem()
{
    delete pTop;
    delete pBottom;
    delete pLeft;
    delete pRight;
}



SvxBoxItem& SvxBoxItem::operator=( const SvxBoxItem& rBox )
{
    nTopDist = rBox.nTopDist;
    nBottomDist = rBox.nBottomDist;
    nLeftDist = rBox.nLeftDist;
    nRightDist = rBox.nRightDist;
    bRemoveAdjCellBorder = rBox.bRemoveAdjCellBorder;
    SetLine( rBox.GetTop(), SvxBoxItemLine::TOP );
    SetLine( rBox.GetBottom(), SvxBoxItemLine::BOTTOM );
    SetLine( rBox.GetLeft(), SvxBoxItemLine::LEFT );
    SetLine( rBox.GetRight(), SvxBoxItemLine::RIGHT );
    return *this;
}



inline bool CmpBrdLn( const SvxBorderLine* pBrd1, const SvxBorderLine* pBrd2 )
{
    if( pBrd1 == pBrd2 )
        return true;
    if( pBrd1 == nullptr || pBrd2 == nullptr)
        return false;
    return *pBrd1 == *pBrd2;
}



bool SvxBoxItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxBoxItem& rBoxItem = static_cast<const SvxBoxItem&>(rAttr);
    return (
        ( nTopDist == rBoxItem.nTopDist ) &&
        ( nBottomDist == rBoxItem.nBottomDist )   &&
        ( nLeftDist == rBoxItem.nLeftDist )   &&
        ( nRightDist == rBoxItem.nRightDist ) &&
        ( bRemoveAdjCellBorder == rBoxItem.bRemoveAdjCellBorder ) &&
        CmpBrdLn( pTop, rBoxItem.GetTop() )           &&
        CmpBrdLn( pBottom, rBoxItem.GetBottom() )     &&
        CmpBrdLn( pLeft, rBoxItem.GetLeft() )         &&
        CmpBrdLn( pRight, rBoxItem.GetRight() ) );
}


table::BorderLine2 SvxBoxItem::SvxLineToLine(const SvxBorderLine* pLine, bool bConvert)
{
    table::BorderLine2 aLine;
    if(pLine)
    {
        aLine.Color          = pLine->GetColor().GetColor() ;
        aLine.InnerLineWidth = sal_uInt16( bConvert ? convertTwipToMm100(pLine->GetInWidth() ): pLine->GetInWidth() );
        aLine.OuterLineWidth = sal_uInt16( bConvert ? convertTwipToMm100(pLine->GetOutWidth()): pLine->GetOutWidth() );
        aLine.LineDistance   = sal_uInt16( bConvert ? convertTwipToMm100(pLine->GetDistance()): pLine->GetDistance() );
        aLine.LineStyle      = pLine->GetBorderLineStyle();
        aLine.LineWidth      = sal_uInt32( bConvert ? convertTwipToMm100( pLine->GetWidth( ) ) : pLine->GetWidth( ) );
    }
    else
        aLine.Color          = aLine.InnerLineWidth = aLine.OuterLineWidth = aLine.LineDistance  = 0;
    return aLine;
}

bool SvxBoxItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    table::BorderLine2 aRetLine;
    sal_uInt16 nDist = 0;
    bool bDistMember = false;
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
            aSeq[4] <<= uno::makeAny( (sal_Int32)(bConvert ? convertTwipToMm100( GetDistance()) : GetDistance()));
            aSeq[5] <<= uno::makeAny( (sal_Int32)(bConvert ? convertTwipToMm100( nTopDist ) : nTopDist ));
            aSeq[6] <<= uno::makeAny( (sal_Int32)(bConvert ? convertTwipToMm100( nBottomDist ) : nBottomDist ));
            aSeq[7] <<= uno::makeAny( (sal_Int32)(bConvert ? convertTwipToMm100( nLeftDist ) : nLeftDist ));
            aSeq[8] <<= uno::makeAny( (sal_Int32)(bConvert ? convertTwipToMm100( nRightDist ) : nRightDist ));
            rVal = uno::makeAny( aSeq );
            return true;
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
            bDistMember = true;
            break;
        case TOP_BORDER_DISTANCE:
            nDist = nTopDist;
            bDistMember = true;
            break;
        case BOTTOM_BORDER_DISTANCE:
            nDist = nBottomDist;
            bDistMember = true;
            break;
        case LEFT_BORDER_DISTANCE:
            nDist = nLeftDist;
            bDistMember = true;
            break;
        case RIGHT_BORDER_DISTANCE:
            nDist = nRightDist;
            bDistMember = true;
            break;
        case LINE_STYLE:
        case LINE_WIDTH:
            // it doesn't make sense to return a value for these since it's
            // probably ambiguous
            return true;
            break;
    }

    if( bDistMember )
        rVal <<= (sal_Int32)(bConvert ? convertTwipToMm100(nDist) : nDist);
    else
        rVal <<= aRetLine;

    return true;
}

namespace
{

bool
lcl_lineToSvxLine(const table::BorderLine& rLine, SvxBorderLine& rSvxLine, bool bConvert, bool bGuessWidth)
{
    rSvxLine.SetColor( Color(rLine.Color));
    if ( bGuessWidth )
    {
        rSvxLine.GuessLinesWidths( rSvxLine.GetBorderLineStyle(),
                sal_uInt16( bConvert ? convertMm100ToTwip(rLine.OuterLineWidth) : rLine.OuterLineWidth  ),
                sal_uInt16( bConvert ? convertMm100ToTwip(rLine.InnerLineWidth) : rLine.InnerLineWidth  ),
                sal_uInt16( bConvert ? convertMm100ToTwip(rLine.LineDistance )  : rLine.LineDistance  ));
    }

    bool bRet = !rSvxLine.isEmpty();
    return bRet;
}

}


bool SvxBoxItem::LineToSvxLine(const css::table::BorderLine& rLine, SvxBorderLine& rSvxLine, bool bConvert)
{
    return lcl_lineToSvxLine(rLine, rSvxLine, bConvert, true);
}

bool
SvxBoxItem::LineToSvxLine(const css::table::BorderLine2& rLine, SvxBorderLine& rSvxLine, bool bConvert)
{
    SvxBorderStyle const nStyle =
        (rLine.LineStyle < 0 || BORDER_LINE_STYLE_MAX < rLine.LineStyle)
        ? SOLID     // default
        : rLine.LineStyle;

    rSvxLine.SetBorderLineStyle( nStyle );

    bool bGuessWidth = true;
    if ( rLine.LineWidth )
    {
        rSvxLine.SetWidth( bConvert? convertMm100ToTwip( rLine.LineWidth ) : rLine.LineWidth );
        // fdo#46112: double does not necessarily mean symmetric
        // for backwards compatibility
        bGuessWidth = ((DOUBLE == nStyle || DOUBLE_THIN == nStyle)) &&
            (rLine.InnerLineWidth > 0) && (rLine.OuterLineWidth > 0);
    }

    return lcl_lineToSvxLine(rLine, rSvxLine, bConvert, bGuessWidth);
}



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

template<typename Item, typename Line>
bool
lcl_setLine(const uno::Any& rAny, Item& rItem, Line nLine, const bool bConvert)
{
    bool bDone = false;
    table::BorderLine2 aBorderLine;
    if (lcl_extractBorderLine(rAny, aBorderLine))
    {
        SvxBorderLine aLine;
        bool bSet = SvxBoxItem::LineToSvxLine(aBorderLine, aLine, bConvert);
        rItem.SetLine( bSet ? &aLine : nullptr, nLine);
        bDone = true;
    }
    return bDone;
}

}

bool SvxBoxItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    SvxBoxItemLine nLine = SvxBoxItemLine::TOP;
    bool bDistMember = false;
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case 0:
        {
            uno::Sequence< uno::Any > aSeq;
            if (( rVal >>= aSeq ) && ( aSeq.getLength() == 9 ))
            {
                // 4 Borders and 5 distances
                const SvxBoxItemLine aBorders[] = { SvxBoxItemLine::LEFT, SvxBoxItemLine::RIGHT, SvxBoxItemLine::BOTTOM, SvxBoxItemLine::TOP };
                for (int n(0); n != SAL_N_ELEMENTS(aBorders); ++n)
                {
                    if (!lcl_setLine(aSeq[n], *this, aBorders[n], bConvert))
                        return false;
                }

                // WTH are the borders and the distances saved in different order?
                SvxBoxItemLine nLines[4] = { SvxBoxItemLine::TOP, SvxBoxItemLine::BOTTOM, SvxBoxItemLine::LEFT, SvxBoxItemLine::RIGHT };
                for ( sal_Int32 n = 4; n < 9; n++ )
                {
                    sal_Int32 nDist = 0;
                    if ( aSeq[n] >>= nDist )
                    {
                        if( bConvert )
                            nDist = convertMm100ToTwip(nDist);
                        if ( n == 4 )
                            SetDistance( sal_uInt16( nDist ));
                        else
                            SetDistance( sal_uInt16( nDist ), nLines[n-5] );
                    }
                    else
                        return false;
                }

                return true;
            }
            else
                return false;
        }
        case LEFT_BORDER_DISTANCE:
            bDistMember = true;
            //fall-through
        case LEFT_BORDER:
        case MID_LEFT_BORDER:
            nLine = SvxBoxItemLine::LEFT;
            break;
        case RIGHT_BORDER_DISTANCE:
            bDistMember = true;
            //fall-through
        case RIGHT_BORDER:
        case MID_RIGHT_BORDER:
            nLine = SvxBoxItemLine::RIGHT;
            break;
        case BOTTOM_BORDER_DISTANCE:
            bDistMember = true;
            //fall-through
        case BOTTOM_BORDER:
        case MID_BOTTOM_BORDER:
            nLine = SvxBoxItemLine::BOTTOM;
            break;
        case TOP_BORDER_DISTANCE:
            bDistMember = true;
            //fall-through
        case TOP_BORDER:
        case MID_TOP_BORDER:
            nLine = SvxBoxItemLine::TOP;
            break;
        case LINE_STYLE:
            {
                drawing::LineStyle eDrawingStyle;
                rVal >>= eDrawingStyle;
                editeng::SvxBorderStyle eBorderStyle = css::table::BorderLineStyle::NONE;
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
                for( SvxBoxItemLine n : o3tl::enumrange<SvxBoxItemLine>() )
                {
                    editeng::SvxBorderLine* pLine = const_cast< editeng::SvxBorderLine* >( GetLine( n ) );
                    if( pLine )
                        pLine->SetBorderLineStyle( eBorderStyle );
                }
                return true;
            }
            break;
        case LINE_WIDTH:
            {
                // Set the line width on all borders
                long nWidth(0);
                rVal >>= nWidth;
                if( bConvert )
                    nWidth = convertMm100ToTwip( nWidth );

                // Set the line Width on all borders
                for( SvxBoxItemLine n : o3tl::enumrange<SvxBoxItemLine>() )
                {
                    editeng::SvxBorderLine* pLine = const_cast< editeng::SvxBorderLine* >( GetLine( n ) );
                    if( pLine )
                        pLine->SetWidth( nWidth );
                }
            }
            return true;
    }

    if( bDistMember || nMemberId == BORDER_DISTANCE )
    {
        sal_Int32 nDist = 0;
        if(!(rVal >>= nDist))
            return false;

        if(nDist >= 0)
        {
            if( bConvert )
                nDist = convertMm100ToTwip(nDist);
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
            return false;

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
            try { aNew = xConverter->convertTo( rVal, cppu::UnoType<uno::Sequence < uno::Any >>::get() ); }
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
                return false;
        }
        else
            return false;

        bool bSet = SvxBoxItem::LineToSvxLine(aBorderLine, aLine, bConvert);
        SetLine(bSet ? &aLine : nullptr, nLine);
    }

    return true;
}



SfxPoolItem* SvxBoxItem::Clone( SfxItemPool* ) const
{
    return new SvxBoxItem( *this );
}



bool SvxBoxItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    OUString&           rText, const IntlWrapper *pIntl
)   const
{
    OUString cpDelimTmp = OUString(cpDelim);
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            rText.clear();

            if ( pTop )
            {
                rText = pTop->GetValueString( eCoreUnit, ePresUnit, pIntl ) + cpDelimTmp;
            }
            if( !(pTop && pBottom && pLeft && pRight &&
                  *pTop == *pBottom && *pTop == *pLeft && *pTop == *pRight) )
            {
                if ( pBottom )
                {
                    rText = rText + pBottom->GetValueString( eCoreUnit, ePresUnit, pIntl ) + cpDelimTmp;
                }
                if ( pLeft )
                {
                    rText = rText + pLeft->GetValueString( eCoreUnit, ePresUnit, pIntl ) + cpDelimTmp;
                }
                if ( pRight )
                {
                    rText = rText + pRight->GetValueString( eCoreUnit, ePresUnit, pIntl ) + cpDelimTmp;
                }
            }
            rText += GetMetricText( (long)nTopDist, eCoreUnit, ePresUnit, pIntl );
            if( nTopDist != nBottomDist || nTopDist != nLeftDist ||
                nTopDist != nRightDist )
            {
                rText = rText +
                        cpDelimTmp +
                        GetMetricText( (long)nBottomDist, eCoreUnit,
                                        ePresUnit, pIntl ) +
                        cpDelimTmp +
                        GetMetricText( (long)nLeftDist, eCoreUnit, ePresUnit, pIntl ) +
                        cpDelimTmp +
                        GetMetricText( (long)nRightDist, eCoreUnit,
                                        ePresUnit, pIntl );
            }
            return true;
        }
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if( !(pTop || pBottom || pLeft || pRight) )
            {
                rText = EE_RESSTR(RID_SVXITEMS_BORDER_NONE) + cpDelimTmp;
            }
            else
            {
                rText = EE_RESSTR(RID_SVXITEMS_BORDER_COMPLETE);
                if( pTop && pBottom && pLeft && pRight &&
                    *pTop == *pBottom && *pTop == *pLeft && *pTop == *pRight )
                {
                    rText += pTop->GetValueString( eCoreUnit, ePresUnit, pIntl, true ) + cpDelimTmp;
                }
                else
                {
                    if ( pTop )
                    {
                        rText = rText +
                                EE_RESSTR(RID_SVXITEMS_BORDER_TOP) +
                                pTop->GetValueString( eCoreUnit, ePresUnit, pIntl, true ) +
                                cpDelimTmp;
                    }
                    if ( pBottom )
                    {
                        rText = rText +
                                EE_RESSTR(RID_SVXITEMS_BORDER_BOTTOM) +
                                pBottom->GetValueString( eCoreUnit, ePresUnit, pIntl, true ) +
                                cpDelimTmp;
                    }
                    if ( pLeft )
                    {
                        rText = rText +
                                EE_RESSTR(RID_SVXITEMS_BORDER_LEFT) +
                                pLeft->GetValueString( eCoreUnit, ePresUnit, pIntl, true ) +
                                cpDelimTmp;
                    }
                    if ( pRight )
                    {
                        rText = rText +
                                EE_RESSTR(RID_SVXITEMS_BORDER_RIGHT) +
                                pRight->GetValueString( eCoreUnit, ePresUnit, pIntl, true ) +
                                cpDelimTmp;
                    }
                }
            }

            rText += EE_RESSTR(RID_SVXITEMS_BORDER_DISTANCE);
            if( nTopDist == nBottomDist && nTopDist == nLeftDist &&
                nTopDist == nRightDist )
            {
                rText = rText +
                        GetMetricText( (long)nTopDist, eCoreUnit,
                                            ePresUnit, pIntl ) +
                        " " + EE_RESSTR(GetMetricId(ePresUnit));
            }
            else
            {
                rText = rText +
                        EE_RESSTR(RID_SVXITEMS_BORDER_TOP) +
                        GetMetricText( (long)nTopDist, eCoreUnit,
                                        ePresUnit, pIntl ) +
                        " " + EE_RESSTR(GetMetricId(ePresUnit)) +
                        cpDelimTmp +
                        EE_RESSTR(RID_SVXITEMS_BORDER_BOTTOM) +
                        GetMetricText( (long)nBottomDist, eCoreUnit,
                                        ePresUnit, pIntl ) +
                        " " + EE_RESSTR(GetMetricId(ePresUnit)) +
                        cpDelimTmp +
                        EE_RESSTR(RID_SVXITEMS_BORDER_LEFT) +
                        GetMetricText( (long)nLeftDist, eCoreUnit,
                                        ePresUnit, pIntl ) +
                        " " + EE_RESSTR(GetMetricId(ePresUnit)) +
                        cpDelimTmp +
                        EE_RESSTR(RID_SVXITEMS_BORDER_RIGHT) +
                        GetMetricText( (long)nRightDist, eCoreUnit,
                                        ePresUnit, pIntl ) +
                        " " + EE_RESSTR(GetMetricId(ePresUnit));
            }
            return true;
        }
        default: ;//prevent warning
    }
    return false;
}



SvStream& SvxBoxItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    rStrm.WriteUInt16( GetDistance() );
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
            rStrm.WriteSChar(i);
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

    rStrm.WriteSChar( cLine );

    if( nItemVersion >= BOX_4DISTS_VERSION && (cLine & 0x10) != 0 )
    {
        rStrm.WriteUInt16( nTopDist )
             .WriteUInt16( nLeftDist )
             .WriteUInt16( nRightDist )
             .WriteUInt16( nBottomDist );
    }

    return rStrm;
}



sal_uInt16 SvxBoxItem::GetVersion( sal_uInt16 nFFVer ) const
{
    DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
            SOFFICE_FILEFORMAT_40==nFFVer ||
            SOFFICE_FILEFORMAT_50==nFFVer,
            "SvxBoxItem: Gibt es ein neues Fileformat?" );
    return SOFFICE_FILEFORMAT_31==nFFVer ||
           SOFFICE_FILEFORMAT_40==nFFVer ? 0 : BOX_BORDER_STYLE_VERSION;
}



bool SvxBoxItem::ScaleMetrics( long nMult, long nDiv )
{
    if ( pTop )     pTop->ScaleMetrics( nMult, nDiv );
    if ( pBottom )  pBottom->ScaleMetrics( nMult, nDiv );
    if ( pLeft )    pLeft->ScaleMetrics( nMult, nDiv );
    if ( pRight )   pRight->ScaleMetrics( nMult, nDiv );
    nTopDist = (sal_uInt16)Scale( nTopDist, nMult, nDiv );
    nBottomDist = (sal_uInt16)Scale( nBottomDist, nMult, nDiv );
    nLeftDist = (sal_uInt16)Scale( nLeftDist, nMult, nDiv );
    nRightDist = (sal_uInt16)Scale( nRightDist, nMult, nDiv );
    return true;
}



bool SvxBoxItem::HasMetrics() const
{
    return true;
}



SfxPoolItem* SvxBoxItem::Create( SvStream& rStrm, sal_uInt16 nIVersion ) const
{
    sal_uInt16 nDistance;
    rStrm.ReadUInt16( nDistance );
    SvxBoxItem* pAttr = new SvxBoxItem( Which() );

    SvxBoxItemLine aLineMap[4] = { SvxBoxItemLine::TOP, SvxBoxItemLine::LEFT,
                           SvxBoxItemLine::RIGHT, SvxBoxItemLine::BOTTOM };

    sal_Int8 cLine;
    while( true )
    {
        rStrm.ReadSChar( cLine );

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
            rStrm.ReadUInt16( nDist );
            pAttr->SetDistance( nDist, aLineMap[i] );
        }
    }
    else
    {
        pAttr->SetDistance( nDistance );
    }

    return pAttr;
}



const SvxBorderLine *SvxBoxItem::GetLine( SvxBoxItemLine nLine ) const
{
    const SvxBorderLine *pRet = nullptr;

    switch ( nLine )
    {
        case SvxBoxItemLine::TOP:
            pRet = pTop;
            break;
        case SvxBoxItemLine::BOTTOM:
            pRet = pBottom;
            break;
        case SvxBoxItemLine::LEFT:
            pRet = pLeft;
            break;
        case SvxBoxItemLine::RIGHT:
            pRet = pRight;
            break;
        default:
            OSL_FAIL( "wrong line" );
            break;
    }

    return pRet;
}



void SvxBoxItem::SetLine( const SvxBorderLine* pNew, SvxBoxItemLine nLine )
{
    SvxBorderLine* pTmp = pNew ? new SvxBorderLine( *pNew ) : nullptr;

    switch ( nLine )
    {
        case SvxBoxItemLine::TOP:
            delete pTop;
            pTop = pTmp;
            break;
        case SvxBoxItemLine::BOTTOM:
            delete pBottom;
            pBottom = pTmp;
            break;
        case SvxBoxItemLine::LEFT:
            delete pLeft;
            pLeft = pTmp;
            break;
        case SvxBoxItemLine::RIGHT:
            delete pRight;
            pRight = pTmp;
            break;
        default:
            delete pTmp;
            OSL_FAIL( "wrong line" );
    }
}



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



sal_uInt16 SvxBoxItem::GetDistance( SvxBoxItemLine nLine ) const
{
    sal_uInt16 nDist = 0;
    switch ( nLine )
    {
        case SvxBoxItemLine::TOP:
            nDist = nTopDist;
            break;
        case SvxBoxItemLine::BOTTOM:
            nDist = nBottomDist;
            break;
        case SvxBoxItemLine::LEFT:
            nDist = nLeftDist;
            break;
        case SvxBoxItemLine::RIGHT:
            nDist = nRightDist;
            break;
        default:
            OSL_FAIL( "wrong line" );
    }

    return nDist;
}



void SvxBoxItem::SetDistance( sal_uInt16 nNew, SvxBoxItemLine nLine )
{
    switch ( nLine )
    {
        case SvxBoxItemLine::TOP:
            nTopDist = nNew;
            break;
        case SvxBoxItemLine::BOTTOM:
            nBottomDist = nNew;
            break;
        case SvxBoxItemLine::LEFT:
            nLeftDist = nNew;
            break;
        case SvxBoxItemLine::RIGHT:
            nRightDist = nNew;
            break;
        default:
            OSL_FAIL( "wrong line" );
    }
}



sal_uInt16 SvxBoxItem::CalcLineSpace( SvxBoxItemLine nLine, bool bIgnoreLine ) const
{
    SvxBorderLine* pTmp = nullptr;
    sal_uInt16 nDist = 0;
    switch ( nLine )
    {
    case SvxBoxItemLine::TOP:
        pTmp = pTop;
        nDist = nTopDist;
        break;
    case SvxBoxItemLine::BOTTOM:
        pTmp = pBottom;
        nDist = nBottomDist;
        break;
    case SvxBoxItemLine::LEFT:
        pTmp = pLeft;
        nDist = nLeftDist;
        break;
    case SvxBoxItemLine::RIGHT:
        pTmp = pRight;
        nDist = nRightDist;
        break;
    default:
        OSL_FAIL( "wrong line" );
    }

    if( pTmp )
    {
        nDist = nDist + pTmp->GetScaledWidth();
    }
    else if( !bIgnoreLine )
        nDist = 0;
    return nDist;
}

// class SvxBoxInfoItem --------------------------------------------------

SvxBoxInfoItem::SvxBoxInfoItem( const sal_uInt16 nId ) :
    SfxPoolItem( nId ),
    pHori   ( nullptr ),
    pVert   ( nullptr ),
    mbEnableHor( false ),
    mbEnableVer( false ),
    nDefDist( 0 )
{
    bDist = bMinDist = false;
    ResetFlags();
}



SvxBoxInfoItem::SvxBoxInfoItem( const SvxBoxInfoItem& rCpy ) :
    SfxPoolItem( rCpy ),
    mbEnableHor( rCpy.mbEnableHor ),
    mbEnableVer( rCpy.mbEnableVer )
{
    pHori       = rCpy.GetHori() ? new SvxBorderLine( *rCpy.GetHori() ) : nullptr;
    pVert       = rCpy.GetVert() ? new SvxBorderLine( *rCpy.GetVert() ) : nullptr;
    bDist       = rCpy.IsDist();
    bMinDist    = rCpy.IsMinDist();
    nValidFlags = rCpy.nValidFlags;
    nDefDist    = rCpy.GetDefDist();
}



SvxBoxInfoItem::~SvxBoxInfoItem()
{
    delete pHori;
    delete pVert;
}



SvxBoxInfoItem &SvxBoxInfoItem::operator=( const SvxBoxInfoItem& rCpy )
{
    delete pHori;
    delete pVert;
    pHori       = rCpy.GetHori() ? new SvxBorderLine( *rCpy.GetHori() ) : nullptr;
    pVert       = rCpy.GetVert() ? new SvxBorderLine( *rCpy.GetVert() ) : nullptr;
    mbEnableHor = rCpy.mbEnableHor;
    mbEnableVer = rCpy.mbEnableVer;
    bDist       = rCpy.IsDist();
    bMinDist    = rCpy.IsMinDist();
    nValidFlags = rCpy.nValidFlags;
    nDefDist    = rCpy.GetDefDist();
    return *this;
}



bool SvxBoxInfoItem::operator==( const SfxPoolItem& rAttr ) const
{
    const SvxBoxInfoItem& rBoxInfo = static_cast<const SvxBoxInfoItem&>(rAttr);

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



void SvxBoxInfoItem::SetLine( const SvxBorderLine* pNew, SvxBoxInfoItemLine nLine )
{
    SvxBorderLine* pTmp = pNew ? new SvxBorderLine( *pNew ) : nullptr;

    if ( SvxBoxInfoItemLine::HORI == nLine )
    {
        delete pHori;
        pHori = pTmp;
    }
    else if ( SvxBoxInfoItemLine::VERT == nLine )
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



SfxPoolItem* SvxBoxInfoItem::Clone( SfxItemPool* ) const
{
    return new SvxBoxInfoItem( *this );
}



bool SvxBoxInfoItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText.clear();
    return false;
}



SvStream& SvxBoxInfoItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    sal_Int8 cFlags = 0;

    if ( IsTable() )
        cFlags |= 0x01;
    if ( IsDist() )
        cFlags |= 0x02;
    if ( IsMinDist() )
        cFlags |= 0x04;
    rStrm.WriteSChar( cFlags )
         .WriteUInt16( GetDefDist() );
    const SvxBorderLine* pLine[ 2 ];
    pLine[ 0 ] = GetHori();
    pLine[ 1 ] = GetVert();

    for( int i = 0; i < 2; i++ )
    {
        const SvxBorderLine* l = pLine[ i ];
        if( l )
        {
            rStrm.WriteChar( (char) i );
            WriteColor( rStrm, l->GetColor() );
            rStrm.WriteInt16( l->GetOutWidth() )
                 .WriteInt16( l->GetInWidth() )
                 .WriteInt16( l->GetDistance() );
        }
    }
    rStrm.WriteChar( (char) 2 );
    return rStrm;
}



bool SvxBoxInfoItem::ScaleMetrics( long nMult, long nDiv )
{
    if ( pHori ) pHori->ScaleMetrics( nMult, nDiv );
    if ( pVert ) pVert->ScaleMetrics( nMult, nDiv );
    nDefDist = (sal_uInt16)Scale( nDefDist, nMult, nDiv );
    return true;
}



bool SvxBoxInfoItem::HasMetrics() const
{
    return true;
}



SfxPoolItem* SvxBoxInfoItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 cFlags;
    sal_uInt16 _nDefDist;
    rStrm.ReadSChar( cFlags ).ReadUInt16( _nDefDist );

    SvxBoxInfoItem* pAttr = new SvxBoxInfoItem( Which() );

    pAttr->SetTable  ( ( cFlags & 0x01 ) != 0 );
    pAttr->SetDist   ( ( cFlags & 0x02 ) != 0 );
    pAttr->SetMinDist( ( cFlags & 0x04 ) != 0 );
    pAttr->SetDefDist( _nDefDist );

    while( true )
    {
        sal_Int8 cLine;
        rStrm.ReadSChar( cLine );

        if( cLine > 1 )
            break;
        short nOutline, nInline, nDistance;
        Color aColor;
        ReadColor( rStrm, aColor ).ReadInt16( nOutline ).ReadInt16( nInline ).ReadInt16( nDistance );
        SvxBorderLine aBorder( &aColor );
        aBorder.GuessLinesWidths(css::table::BorderLineStyle::NONE, nOutline, nInline, nDistance);

        switch( cLine )
        {
            case 0: pAttr->SetLine( &aBorder, SvxBoxInfoItemLine::HORI ); break;
            case 1: pAttr->SetLine( &aBorder, SvxBoxInfoItemLine::VERT ); break;
        }
    }
    return pAttr;
}



void SvxBoxInfoItem::ResetFlags()
{
    nValidFlags = static_cast<SvxBoxInfoItemValidFlags>(0x7F); // all valid except Disable
}

bool SvxBoxInfoItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bConvert = 0 != (nMemberId & CONVERT_TWIPS);
    table::BorderLine2 aRetLine;
    sal_Int16 nVal=0;
    bool bIntMember = false;
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case 0:
        {
            // 2 BorderLines, flags, valid flags and distance
            css::uno::Sequence< css::uno::Any > aSeq( 5 );
            aSeq[0] = css::uno::makeAny( SvxBoxItem::SvxLineToLine( pHori, bConvert) );
            aSeq[1] = css::uno::makeAny( SvxBoxItem::SvxLineToLine( pVert, bConvert) );
            if ( IsTable() )
                nVal |= 0x01;
            if ( IsDist() )
                nVal |= 0x02;
            if ( IsMinDist() )
                nVal |= 0x04;
            aSeq[2] = css::uno::makeAny( nVal );
            aSeq[3] = css::uno::makeAny( static_cast<sal_Int16>(nValidFlags) );
            aSeq[4] = css::uno::makeAny( (sal_Int32)(bConvert ? convertTwipToMm100(GetDefDist()) : GetDefDist()) );
            rVal = css::uno::makeAny( aSeq );
            return true;
        }

        case MID_HORIZONTAL:
            aRetLine = SvxBoxItem::SvxLineToLine( pHori, bConvert);
            break;
        case MID_VERTICAL:
            aRetLine = SvxBoxItem::SvxLineToLine( pVert, bConvert);
            break;
        case MID_FLAGS:
            bIntMember = true;
            if ( IsTable() )
                nVal |= 0x01;
            if ( IsDist() )
                nVal |= 0x02;
            if ( IsMinDist() )
                nVal |= 0x04;
            rVal <<= nVal;
            break;
        case MID_VALIDFLAGS:
            bIntMember = true;
            rVal <<= static_cast<sal_Int16>(nValidFlags);
            break;
        case MID_DISTANCE:
            bIntMember = true;
            rVal <<= (sal_Int32)(bConvert ? convertTwipToMm100(GetDefDist()) : GetDefDist());
            break;
        default: OSL_FAIL("Wrong MemberId!"); return false;
    }

    if( !bIntMember )
        rVal <<= aRetLine;

    return true;
}



bool SvxBoxInfoItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet;
    switch(nMemberId)
    {
        case 0:
        {
            css::uno::Sequence< css::uno::Any > aSeq;
            if (( rVal >>= aSeq ) && ( aSeq.getLength() == 5 ))
            {
                // 2 BorderLines, flags, valid flags and distance
                if (!lcl_setLine(aSeq[0], *this, SvxBoxInfoItemLine::HORI, bConvert))
                    return false;
                if (!lcl_setLine(aSeq[1], *this, SvxBoxInfoItemLine::VERT, bConvert))
                    return false;

                sal_Int16 nFlags( 0 );
                sal_Int32 nVal( 0 );
                if ( aSeq[2] >>= nFlags )
                {
                    SetTable  ( ( nFlags & 0x01 ) != 0 );
                    SetDist   ( ( nFlags & 0x02 ) != 0 );
                    SetMinDist( ( nFlags & 0x04 ) != 0 );
                }
                else
                    return false;
                if ( aSeq[3] >>= nFlags )
                    nValidFlags = static_cast<SvxBoxInfoItemValidFlags>(nFlags);
                else
                    return false;
                if (( aSeq[4] >>= nVal ) && ( nVal >= 0 ))
                {
                    if( bConvert )
                        nVal = convertMm100ToTwip(nVal);
                    SetDefDist( (sal_uInt16)nVal );
                }
            }
            return true;
        }

        case MID_HORIZONTAL:
        case MID_VERTICAL:
        {
            if( !rVal.hasValue() )
                return false;

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
                try { aNew = xConverter->convertTo( rVal, cppu::UnoType<uno::Sequence < uno::Any >>::get() ); }
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
                    return false;
            }
            else if (rVal.getValueType() == cppu::UnoType<css::uno::Sequence < sal_Int16 >>::get() )
            {
                // serialization for basic macro recording
                css::uno::Sequence < sal_Int16 > aSeq;
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
                    return false;
            }
            else
                return false;

            SvxBorderLine aLine;
            bool bSet = SvxBoxItem::LineToSvxLine(aBorderLine, aLine, bConvert);
            if ( bSet )
                SetLine( &aLine, nMemberId == MID_HORIZONTAL ? SvxBoxInfoItemLine::HORI : SvxBoxInfoItemLine::VERT );
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
                nValidFlags = static_cast<SvxBoxInfoItemValidFlags>(nFlags);
            break;
        }
        case MID_DISTANCE:
        {
            sal_Int32 nVal = 0;
            bRet = (rVal >>= nVal);
            if ( bRet && nVal>=0 )
            {
                if( bConvert )
                    nVal = convertMm100ToTwip(nVal);
                SetDefDist( (sal_uInt16)nVal );
            }
            break;
        }
        default: OSL_FAIL("Wrong MemberId!"); return false;
    }

    return true;
}

// class SvxFormatBreakItem -------------------------------------------------

bool SvxFormatBreakItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rAttr ), "unequal types" );

    return GetValue() == static_cast<const SvxFormatBreakItem&>( rAttr ).GetValue();
}



bool SvxFormatBreakItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText = GetValueTextByPos( GetValue() );
    return true;
}



OUString SvxFormatBreakItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos < SVX_BREAK_END, "enum overflow!" );
    return EE_RESSTR(RID_SVXITEMS_BREAK_BEGIN + nPos);
}


bool SvxFormatBreakItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
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

bool SvxFormatBreakItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    style::BreakType nBreak;

    if(!(rVal >>= nBreak))
    {
        sal_Int32 nValue = 0;
        if(!(rVal >>= nValue))
            return false;

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



SfxPoolItem* SvxFormatBreakItem::Clone( SfxItemPool* ) const
{
    return new SvxFormatBreakItem( *this );
}



SvStream& SvxFormatBreakItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    rStrm.WriteSChar( GetValue() );
    if( FMTBREAK_NOAUTO > nItemVersion )
        rStrm.WriteSChar( 0x01 );
    return rStrm;
}



sal_uInt16 SvxFormatBreakItem::GetVersion( sal_uInt16 nFFVer ) const
{
    DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
            SOFFICE_FILEFORMAT_40==nFFVer ||
            SOFFICE_FILEFORMAT_50==nFFVer,
            "SvxFormatBreakItem: Is there a new file format? ");
    return SOFFICE_FILEFORMAT_31==nFFVer ||
           SOFFICE_FILEFORMAT_40==nFFVer ? 0 : FMTBREAK_NOAUTO;
}



SfxPoolItem* SvxFormatBreakItem::Create( SvStream& rStrm, sal_uInt16 nVersion ) const
{
    sal_Int8 eBreak, bDummy;
    rStrm.ReadSChar( eBreak );
    if( FMTBREAK_NOAUTO > nVersion )
        rStrm.ReadSChar( bDummy );
    return new SvxFormatBreakItem( (const SvxBreak)eBreak, Which() );
}



sal_uInt16 SvxFormatBreakItem::GetValueCount() const
{
    return SVX_BREAK_END;   // SVX_BREAK_PAGE_BOTH + 1
}

// class SvxFormatKeepItem -------------------------------------------------

SfxPoolItem* SvxFormatKeepItem::Clone( SfxItemPool* ) const
{
    return new SvxFormatKeepItem( *this );
}



SvStream& SvxFormatKeepItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteSChar( (sal_Int8)GetValue() );
    return rStrm;
}



SfxPoolItem* SvxFormatKeepItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 bIsKeep;
    rStrm.ReadSChar( bIsKeep );
    return new SvxFormatKeepItem( bIsKeep != 0, Which() );
}



bool SvxFormatKeepItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
    ) const
{
    sal_uInt16 nId = RID_SVXITEMS_FMTKEEP_FALSE;

    if ( GetValue() )
        nId = RID_SVXITEMS_FMTKEEP_TRUE;
    rText = EE_RESSTR(nId);
    return true;
}

// class SvxLineItem ------------------------------------------------------

SvxLineItem::SvxLineItem( const sal_uInt16 nId ) :

    SfxPoolItem ( nId ),

    pLine( nullptr )
{
}



SvxLineItem::SvxLineItem( const SvxLineItem& rCpy ) :

    SfxPoolItem ( rCpy )
{
    pLine = rCpy.GetLine() ? new SvxBorderLine( *rCpy.GetLine() ) : nullptr;
}



SvxLineItem::~SvxLineItem()
{
    delete pLine;
}



SvxLineItem& SvxLineItem::operator=( const SvxLineItem& rLine )
{
    SetLine( rLine.GetLine() );

    return *this;
}



bool SvxLineItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return CmpBrdLn( pLine, static_cast<const SvxLineItem&>(rAttr).GetLine() );
}



SfxPoolItem* SvxLineItem::Clone( SfxItemPool* ) const
{
    return new SvxLineItem( *this );
}

bool SvxLineItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemId ) const
{
    bool bConvert = 0!=(nMemId&CONVERT_TWIPS);
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



bool SvxLineItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemId )
{
    bool bConvert = 0!=(nMemId&CONVERT_TWIPS);
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
                return false;
        }

        return true;
    }

    return false;
}



bool SvxLineItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    OUString&           rText, const IntlWrapper *pIntl
)   const
{
    rText.clear();

    if ( pLine )
        rText = pLine->GetValueString( eCoreUnit, ePresUnit, pIntl,
            (SFX_ITEM_PRESENTATION_COMPLETE == ePres) );
    return true;
}



SvStream& SvxLineItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    if( pLine )
    {
        WriteColor( rStrm, pLine->GetColor() );
        rStrm.WriteInt16( pLine->GetOutWidth() )
             .WriteInt16( pLine->GetInWidth() )
             .WriteInt16( pLine->GetDistance() );
    }
    else
    {
        WriteColor( rStrm, Color() );
        rStrm.WriteInt16( 0 ).WriteInt16( 0 ).WriteInt16( 0 );
    }
    return rStrm;
}



bool SvxLineItem::ScaleMetrics( long nMult, long nDiv )
{
    if ( pLine ) pLine->ScaleMetrics( nMult, nDiv );
    return true;
}



bool SvxLineItem::HasMetrics() const
{
    return true;
}



SfxPoolItem* SvxLineItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    SvxLineItem* _pLine = new SvxLineItem( Which() );
    short        nOutline, nInline, nDistance;
    Color        aColor;

    ReadColor( rStrm, aColor ).ReadInt16( nOutline ).ReadInt16( nInline ).ReadInt16( nDistance );
    if( nOutline )
    {
        SvxBorderLine aLine( &aColor );
        aLine.GuessLinesWidths(css::table::BorderLineStyle::NONE, nOutline, nInline, nDistance);
        _pLine->SetLine( &aLine );
    }
    return _pLine;
}



void SvxLineItem::SetLine( const SvxBorderLine* pNew )
{
    delete pLine;
    pLine = pNew ? new SvxBorderLine( *pNew ) : nullptr;
}

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
    SvStream*       pStream;

    explicit SvxBrushItem_Impl( GraphicObject* p ) : pGraphicObject( p ), nGraphicTransparency(0), pStream(nullptr) {}
};



SvxBrushItem::SvxBrushItem( sal_uInt16 _nWhich ) :

    SfxPoolItem( _nWhich ),

    aColor           ( COL_TRANSPARENT ),
    nShadingValue    ( ShadingPattern::CLEAR ),
    pImpl            ( new SvxBrushItem_Impl( nullptr ) ),
    maStrLink        (),
    maStrFilter      (),
    eGraphicPos      ( GPOS_NONE ),
    bLoadAgain       ( true )

{
}



SvxBrushItem::SvxBrushItem( const Color& rColor, sal_uInt16 _nWhich) :

    SfxPoolItem( _nWhich ),

    aColor            ( rColor ),
    nShadingValue     ( ShadingPattern::CLEAR ),
    pImpl             ( new SvxBrushItem_Impl( nullptr ) ),
    maStrLink         (),
    maStrFilter       (),
    eGraphicPos       ( GPOS_NONE ),
    bLoadAgain        ( true )

{
}



SvxBrushItem::SvxBrushItem( const Graphic& rGraphic, SvxGraphicPosition ePos,
                            sal_uInt16 _nWhich ) :

    SfxPoolItem( _nWhich ),

    aColor            ( COL_TRANSPARENT ),
    nShadingValue     ( ShadingPattern::CLEAR ),
    pImpl             ( new SvxBrushItem_Impl( new GraphicObject( rGraphic ) ) ),
    maStrLink         (),
    maStrFilter       (),
    eGraphicPos       ( ( GPOS_NONE != ePos ) ? ePos : GPOS_MM ),
    bLoadAgain        ( true )

{
    DBG_ASSERT( GPOS_NONE != ePos, "SvxBrushItem-Ctor with GPOS_NONE == ePos" );
}



SvxBrushItem::SvxBrushItem( const GraphicObject& rGraphicObj,
                            SvxGraphicPosition ePos, sal_uInt16 _nWhich ) :

    SfxPoolItem( _nWhich ),

    aColor            ( COL_TRANSPARENT ),
    nShadingValue     ( ShadingPattern::CLEAR ),
    pImpl             ( new SvxBrushItem_Impl( new GraphicObject( rGraphicObj ) ) ),
    maStrLink         (),
    maStrFilter       (),
    eGraphicPos       ( ( GPOS_NONE != ePos ) ? ePos : GPOS_MM ),
    bLoadAgain        ( true )

{
    DBG_ASSERT( GPOS_NONE != ePos, "SvxBrushItem-Ctor with GPOS_NONE == ePos" );
}



SvxBrushItem::SvxBrushItem(
    const OUString& rLink, const OUString& rFilter,
    SvxGraphicPosition ePos, sal_uInt16 _nWhich ) :

    SfxPoolItem( _nWhich ),

    aColor            ( COL_TRANSPARENT ),
    nShadingValue     ( ShadingPattern::CLEAR ),
    pImpl             ( new SvxBrushItem_Impl( nullptr ) ),
    maStrLink         ( rLink ),
    maStrFilter       ( rFilter ),
    eGraphicPos       ( ( GPOS_NONE != ePos ) ? ePos : GPOS_MM ),
    bLoadAgain        ( true )

{
    DBG_ASSERT( GPOS_NONE != ePos, "SvxBrushItem-Ctor with GPOS_NONE == ePos" );
}



SvxBrushItem::SvxBrushItem( SvStream& rStream, sal_uInt16 nVersion,
                            sal_uInt16 _nWhich )
    : SfxPoolItem( _nWhich )
    , aColor            ( COL_TRANSPARENT )
    , nShadingValue     ( ShadingPattern::CLEAR )
    , pImpl             ( new SvxBrushItem_Impl( nullptr ) )
    , maStrLink         ()
    , maStrFilter       ()
    , eGraphicPos       ( GPOS_NONE )
    , bLoadAgain (false)
{
    bool bTrans;
    Color aTempColor;
    Color aTempFillColor;
    sal_Int8 nStyle;

    rStream.ReadCharAsBool( bTrans );
    ReadColor( rStream, aTempColor );
    ReadColor( rStream, aTempFillColor );
    rStream.ReadSChar( nStyle );

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

        rStream.ReadUInt16( nDoLoad );

        if ( nDoLoad & LOAD_GRAPHIC )
        {
            Graphic aGraphic;

            ReadGraphic( rStream, aGraphic );
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
            OUString aRel = rStream.ReadUniOrByteString(rStream.GetStreamCharSet());

            // TODO/MBA: how can we get a BaseURL here?!
            OSL_FAIL("No BaseURL!");
            OUString aAbs = INetURLObject::GetAbsURL( "", aRel );
            DBG_ASSERT( !aAbs.isEmpty(), "Invalid URL!" );
            maStrLink = aAbs;
        }

        if ( nDoLoad & LOAD_FILTER )
        {
            // UNICODE: rStream >> maStrFilter;
            maStrFilter = rStream.ReadUniOrByteString(rStream.GetStreamCharSet());
        }

        rStream.ReadSChar( nPos );

        eGraphicPos = (SvxGraphicPosition)nPos;
    }
}



SvxBrushItem::SvxBrushItem( const SvxBrushItem& rItem ) :

    SfxPoolItem( rItem.Which() ),
    nShadingValue     ( ShadingPattern::CLEAR ),
    pImpl             ( new SvxBrushItem_Impl( nullptr ) ),
    maStrLink         (),
    maStrFilter       (),
    eGraphicPos       ( GPOS_NONE ),
    bLoadAgain        ( true )

{
    *this = rItem;
}



SvxBrushItem::~SvxBrushItem()
{
    delete pImpl->pGraphicObject;
}



sal_uInt16 SvxBrushItem::GetVersion( sal_uInt16 /*nFileVersion*/ ) const
{
    return BRUSH_GRAPHIC_VERSION;
}


static inline sal_Int8 lcl_PercentToTransparency(long nPercent)
{
    //0xff must not be returned!
    return sal_Int8(nPercent ? (50 + 0xfe * nPercent) / 100 : 0);
}
sal_Int8 SvxBrushItem::TransparencyToPercent(sal_Int32 nTrans)
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
            rVal <<= SvxBrushItem::TransparencyToPercent(aColor.GetTransparency());
        break;
        case MID_GRAPHIC_POSITION:
            rVal <<= (style::GraphicLocation)(sal_Int16)eGraphicPos;
        break;

        case MID_GRAPHIC:
            SAL_WARN( "editeng.items", "not implemented" );
        break;

        case MID_GRAPHIC_TRANSPARENT:
            rVal = css::uno::makeAny<bool>( aColor.GetTransparency() == 0xff );
        break;

        case MID_GRAPHIC_URL:
        {
            OUString sLink;
            if ( !maStrLink.isEmpty() )
                sLink = maStrLink;
            else if( pImpl->pGraphicObject )
            {
                OUString sPrefix(
                    UNO_NAME_GRAPHOBJ_URLPREFIX);
                OUString sId(OStringToOUString(
                    pImpl->pGraphicObject->GetUniqueID(),
                    RTL_TEXTENCODING_ASCII_US));
                sLink = sPrefix + sId;
            }
            rVal <<= sLink;
        }
        break;

        case MID_GRAPHIC_FILTER:
        {
            rVal <<= maStrFilter;
        }
        break;

        case MID_GRAPHIC_TRANSPARENCY:
            rVal <<= pImpl->nGraphicTransparency;
        break;

        case MID_SHADING_VALUE:
        {
            rVal <<= nShadingValue;
        }
        break;
    }

    return true;
}



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
                return false;
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
                return false;
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
                    return false;
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
            if ( rVal.getValueType() == ::cppu::UnoType<OUString>::get() )
            {
                OUString sLink;
                rVal >>= sLink;
                if( sLink.startsWith( UNO_NAME_GRAPHOBJ_URLPKGPREFIX ) )
                {
                    OSL_FAIL( "package urls aren't implemented" );
                }
                else if( sLink.startsWith( UNO_NAME_GRAPHOBJ_URLPREFIX ) )
                {
                    maStrLink.clear();
                    OString sId(OUStringToOString(sLink.copy( sizeof(UNO_NAME_GRAPHOBJ_URLPREFIX)-1 ),
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
            if( rVal.getValueType() == ::cppu::UnoType<OUString>::get() )
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

        case MID_SHADING_VALUE:
        {
            sal_Int32 nVal = 0;
            if (!(rVal >>= nVal))
                return false;

            SetShadingValue( nVal );
        }
        break;
    }

    return true;
}



bool SvxBrushItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
    ) const
{
    if ( GPOS_NONE  == eGraphicPos )
    {
        rText = ::GetColorString( aColor ) + OUString(cpDelim);
        sal_uInt16 nId = RID_SVXITEMS_TRANSPARENT_FALSE;

        if ( aColor.GetTransparency() )
            nId = RID_SVXITEMS_TRANSPARENT_TRUE;
        rText += EE_RESSTR(nId);
    }
    else
    {
        rText = EE_RESSTR(RID_SVXITEMS_GRAPHIC);
    }

    return true;
}



SvxBrushItem& SvxBrushItem::operator=( const SvxBrushItem& rItem )
{
    aColor = rItem.aColor;
    eGraphicPos = rItem.eGraphicPos;

    DELETEZ( pImpl->pGraphicObject );
    maStrLink.clear();
    maStrFilter.clear();

    if ( GPOS_NONE != eGraphicPos )
    {
        maStrLink = rItem.maStrLink;
        maStrFilter = rItem.maStrFilter;
        if ( rItem.pImpl->pGraphicObject )
        {
            pImpl->pGraphicObject = new GraphicObject( *rItem.pImpl->pGraphicObject );
        }
    }

    nShadingValue = rItem.nShadingValue;

    pImpl->nGraphicTransparency = rItem.pImpl->nGraphicTransparency;
    return *this;
}



bool SvxBrushItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxBrushItem& rCmp = static_cast<const SvxBrushItem&>(rAttr);
    bool bEqual = ( aColor == rCmp.aColor && eGraphicPos == rCmp.eGraphicPos &&
        pImpl->nGraphicTransparency == rCmp.pImpl->nGraphicTransparency);

    if ( bEqual )
    {
        if ( GPOS_NONE != eGraphicPos )
        {
            bEqual = maStrLink == rCmp.maStrLink;

            if ( bEqual )
            {
                bEqual = maStrFilter == rCmp.maStrFilter;
            }

            if ( bEqual )
            {
                if ( !rCmp.pImpl->pGraphicObject )
                    bEqual = !pImpl->pGraphicObject;
                else
                    bEqual = pImpl->pGraphicObject &&
                             ( *pImpl->pGraphicObject == *rCmp.pImpl->pGraphicObject );
            }
        }

        if (bEqual)
        {
            bEqual = nShadingValue == rCmp.nShadingValue;
        }
    }

    return bEqual;
}



SfxPoolItem* SvxBrushItem::Clone( SfxItemPool* ) const
{
    return new SvxBrushItem( *this );
}



SfxPoolItem* SvxBrushItem::Create( SvStream& rStream, sal_uInt16 nVersion ) const
{
    return new SvxBrushItem( rStream, nVersion, Which() );
}



SvStream& SvxBrushItem::Store( SvStream& rStream , sal_uInt16 /*nItemVersion*/ ) const
{
    rStream.WriteBool( false );
    WriteColor( rStream, aColor );
    WriteColor( rStream, aColor );
    rStream.WriteSChar( aColor.GetTransparency() > 0 ? 0 : 1 ); //BRUSH_NULL : BRUSH_SOLID

    sal_uInt16 nDoLoad = 0;

    if ( pImpl->pGraphicObject && maStrLink.isEmpty() )
        nDoLoad |= LOAD_GRAPHIC;
    if ( !maStrLink.isEmpty() )
        nDoLoad |= LOAD_LINK;
    if ( !maStrFilter.isEmpty() )
        nDoLoad |= LOAD_FILTER;
    rStream.WriteUInt16( nDoLoad );

    if ( pImpl->pGraphicObject && maStrLink.isEmpty() )
        WriteGraphic( rStream, pImpl->pGraphicObject->GetGraphic() );
    if ( !maStrLink.isEmpty() )
    {
        OSL_FAIL("No BaseURL!");
        // TODO/MBA: how to get a BaseURL?!
        OUString aRel = INetURLObject::GetRelURL( "", maStrLink );
        // UNICODE: rStream << aRel;
        rStream.WriteUniOrByteString(aRel, rStream.GetStreamCharSet());
    }
    if ( !maStrFilter.isEmpty() )
    {
        // UNICODE: rStream << maStrFilter;
        rStream.WriteUniOrByteString(maStrFilter, rStream.GetStreamCharSet());
    }
    rStream.WriteSChar( eGraphicPos );
    return rStream;
}



void SvxBrushItem::PurgeMedium() const
{
    DELETEZ( pImpl->pStream );
}

const GraphicObject* SvxBrushItem::GetGraphicObject(OUString const & referer) const
{
    if ( bLoadAgain && !maStrLink.isEmpty() && !pImpl->pGraphicObject )
    // when graphics already loaded, use as a cache
    {
        if (SvtSecurityOptions().isUntrustedReferer(referer)) {
            return nullptr;
        }

        // tdf#94088 prepare graphic and state
        Graphic aGraphic;
        bool bGraphicLoaded = false;

        // try to create stream directly from given URL
        pImpl->pStream = utl::UcbStreamHelper::CreateStream( maStrLink, STREAM_STD_READ );

        // tdf#94088 if we have a stream, try to load it directly as graphic
        if( pImpl->pStream && !pImpl->pStream->GetError() )
        {
            if (GRFILTER_OK == GraphicFilter::GetGraphicFilter().ImportGraphic( aGraphic, maStrLink, *pImpl->pStream,
                GRFILTER_FORMAT_DONTKNOW, nullptr, GraphicFilterImportFlags::DontSetLogsizeForJpeg ))
            {
                bGraphicLoaded = true;
            }
        }

        // tdf#94088 if no succeeded, try if the string (which is not empty) contains
        // a 'data:' scheme url and try to load that (embedded graphics)
        if(!bGraphicLoaded)
        {
            INetURLObject aGraphicURL( maStrLink );

            if( INetProtocol::Data == aGraphicURL.GetProtocol() )
            {
                std::unique_ptr<SvMemoryStream> const pStream(aGraphicURL.getData());
                if (pStream)
                {
                    if (GRFILTER_OK == GraphicFilter::GetGraphicFilter().ImportGraphic(aGraphic, "", *pStream))
                    {
                        bGraphicLoaded = true;

                        // tdf#94088 delete the no longer needed data scheme URL which
                        // is potentially pretty // large, containing a base64 encoded copy of the graphic
                        const_cast< SvxBrushItem* >(this)->maStrLink.clear();
                    }
                }
            }
        }

        // tdf#94088 when we got a graphic, set it
        if(bGraphicLoaded && GRAPHIC_NONE != aGraphic.GetType())
        {
            pImpl->pGraphicObject = new GraphicObject;
            pImpl->pGraphicObject->SetGraphic( aGraphic );
            const_cast < SvxBrushItem*> (this)->ApplyGraphicTransparency_Impl();
        }
        else
        {
            bLoadAgain = false;
        }
    }

    return pImpl->pGraphicObject;
}

//UUUU
sal_Int8 SvxBrushItem::getGraphicTransparency() const
{
    return pImpl->nGraphicTransparency;
}

// -----------------------------------------------------------------------
//UUUU
void SvxBrushItem::setGraphicTransparency(sal_Int8 nNew)
{
    if(nNew != pImpl->nGraphicTransparency)
    {
        pImpl->nGraphicTransparency = nNew;
        ApplyGraphicTransparency_Impl();
    }
}

// -----------------------------------------------------------------------


const Graphic* SvxBrushItem::GetGraphic(OUString const & referer) const
{
    const GraphicObject* pGrafObj = GetGraphicObject(referer);
    return( pGrafObj ? &( pGrafObj->GetGraphic() ) : nullptr );
}



void SvxBrushItem::SetGraphicPos( SvxGraphicPosition eNew )
{
    eGraphicPos = eNew;

    if ( GPOS_NONE == eGraphicPos )
    {
        DELETEZ( pImpl->pGraphicObject );
        maStrLink.clear();
        maStrFilter.clear();
    }
    else
    {
        if ( !pImpl->pGraphicObject && maStrLink.isEmpty() )
        {
            pImpl->pGraphicObject = new GraphicObject; // Creating a dummy
        }
    }
}



void SvxBrushItem::SetGraphic( const Graphic& rNew )
{
    if ( maStrLink.isEmpty() )
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



void SvxBrushItem::SetGraphicObject( const GraphicObject& rNewObj )
{
    if ( maStrLink.isEmpty() )
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



void SvxBrushItem::SetGraphicLink( const OUString& rNew )
{
    if ( rNew.isEmpty() )
        maStrLink.clear();
    else
    {
        maStrLink = rNew;
        DELETEZ( pImpl->pGraphicObject );
    }
}



void SvxBrushItem::SetGraphicFilter( const OUString& rNew )
{
    maStrFilter = rNew;
}

void SvxBrushItem::SetShadingValue( const sal_Int32 nNew )
{
    nShadingValue = nNew;
}

//static
SvxGraphicPosition SvxBrushItem::WallpaperStyle2GraphicPos( WallpaperStyle eStyle )
{
    SvxGraphicPosition eResult;
    // The switch is not the fastest, but the safest
    switch( eStyle )
    {
        case WallpaperStyle::NONE: eResult = GPOS_NONE; break;
        case WallpaperStyle::Tile: eResult = GPOS_TILED; break;
        case WallpaperStyle::Center: eResult = GPOS_MM; break;
        case WallpaperStyle::Scale: eResult = GPOS_AREA; break;
        case WallpaperStyle::TopLeft: eResult = GPOS_LT; break;
        case WallpaperStyle::Top: eResult = GPOS_MT; break;
        case WallpaperStyle::TopRight: eResult = GPOS_RT; break;
        case WallpaperStyle::Left: eResult = GPOS_LM; break;
        case WallpaperStyle::Right: eResult = GPOS_RM; break;
        case WallpaperStyle::BottomLeft: eResult = GPOS_LB; break;
        case WallpaperStyle::Bottom: eResult = GPOS_MB; break;
        case WallpaperStyle::BottomRight: eResult = GPOS_RB; break;
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
        case GPOS_NONE: eResult = WallpaperStyle::NONE; break;
        case GPOS_TILED: eResult = WallpaperStyle::Tile; break;
        case GPOS_MM: eResult = WallpaperStyle::Center; break;
        case GPOS_AREA: eResult = WallpaperStyle::Scale; break;
        case GPOS_LT: eResult = WallpaperStyle::TopLeft; break;
        case GPOS_MT: eResult = WallpaperStyle::Top; break;
        case GPOS_RT: eResult = WallpaperStyle::TopRight; break;
        case GPOS_LM: eResult = WallpaperStyle::Left; break;
        case GPOS_RM: eResult = WallpaperStyle::Right; break;
        case GPOS_LB: eResult = WallpaperStyle::BottomLeft; break;
        case GPOS_MB: eResult = WallpaperStyle::Bottom; break;
        case GPOS_RB: eResult = WallpaperStyle::BottomRight; break;
        default: eResult = WallpaperStyle::NONE;
    }
    return eResult;
}

SvxBrushItem::SvxBrushItem( const CntWallpaperItem& rItem, sal_uInt16 _nWhich ) :
    SfxPoolItem    ( _nWhich ),
    nShadingValue  ( ShadingPattern::CLEAR ),
    pImpl          ( new SvxBrushItem_Impl( nullptr ) ),
    maStrLink      (),
    maStrFilter    (),
    bLoadAgain     ( true )
{
    aColor = rItem.GetColor();

    if (!rItem.GetBitmapURL().isEmpty())
    {
        maStrLink    = rItem.GetBitmapURL();
        SetGraphicPos( WallpaperStyle2GraphicPos((WallpaperStyle)rItem.GetStyle() ) );
    }
    else
    {
        SetGraphicPos( GPOS_NONE );
    }
}

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

void SvxBrushItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("svxBrushItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("color"), BAD_CAST(aColor.AsRGBHexString().toUtf8().getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("shadingValue"), BAD_CAST(OString::number(nShadingValue).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("link"), BAD_CAST(maStrLink.toUtf8().getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("filter"), BAD_CAST(maStrFilter.toUtf8().getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("graphicPos"), BAD_CAST(OString::number(eGraphicPos).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("loadAgain"), BAD_CAST(OString::boolean(bLoadAgain).getStr()));
    xmlTextWriterEndElement(pWriter);
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

bool SvxFrameDirectionItem::operator==( const SfxPoolItem& rCmp ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rCmp), "unequal types" );

    return GetValue() == static_cast<const SvxFrameDirectionItem&>(rCmp).GetValue();
}

SfxPoolItem* SvxFrameDirectionItem::Clone( SfxItemPool * ) const
{
    return new SvxFrameDirectionItem( *this );
}

SfxPoolItem* SvxFrameDirectionItem::Create( SvStream & rStrm, sal_uInt16 /*nVer*/ ) const
{
    sal_uInt16 nValue;
    rStrm.ReadUInt16( nValue );
    return new SvxFrameDirectionItem( (SvxFrameDirection)nValue, Which() );
}

SvStream& SvxFrameDirectionItem::Store( SvStream & rStrm, sal_uInt16 /*nIVer*/ ) const
{
    sal_uInt16 nValue = GetValue();
    rStrm.WriteUInt16( nValue );
    return rStrm;
}

sal_uInt16 SvxFrameDirectionItem::GetVersion( sal_uInt16 nFVer ) const
{
    return SOFFICE_FILEFORMAT_50 > nFVer ? USHRT_MAX : 0;
}

bool SvxFrameDirectionItem::GetPresentation(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *) const
{
    rText = EE_RESSTR( RID_SVXITEMS_FRMDIR_BEGIN + GetValue() );
    return true;
}

bool SvxFrameDirectionItem::PutValue( const css::uno::Any& rVal,
                                             sal_uInt8 )
{
    sal_Int16 nVal = sal_Int16();
    bool bRet = ( rVal >>= nVal );
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

bool SvxFrameDirectionItem::QueryValue( css::uno::Any& rVal,
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
