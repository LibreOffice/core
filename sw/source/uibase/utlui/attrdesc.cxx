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

#include <i18nutil/unicode.hxx>
#include <svl/itemiter.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <editeng/itemtype.hxx>
#include <svtools/grfmgr.hxx>
#include <unotools/intlwrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ustrbuf.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtinfmt.hxx>
#include <fchrfmt.hxx>
#include <fmtautofmt.hxx>
#include <fmtsrnd.hxx>
#include <fmtornt.hxx>
#include <fmtlsplt.hxx>
#include <fmtrowsplt.hxx>
#include <fmtpdsc.hxx>
#include <fmtclds.hxx>
#include <fmteiro.hxx>
#include <fmturl.hxx>
#include <fmthdft.hxx>
#include <fmtcnct.hxx>
#include <fmtline.hxx>
#include <tgrditem.hxx>
#include <hfspacingitem.hxx>
#include <fmtruby.hxx>
#include <paratr.hxx>
#include <grfatr.hxx>
#include <pagedesc.hxx>
#include <charfmt.hxx>
#include <fmtcol.hxx>
#include <tox.hxx>
#include <attrdesc.hrc>
#include <fmtftntx.hxx>
#include <fmtfollowtextflow.hxx>
#include <libxml/xmlwriter.h>

using namespace com::sun::star;


// query the attribute descriptions
void SwAttrSet::GetPresentation(
        SfxItemPresentation ePres,
        SfxMapUnit eCoreMetric,
        SfxMapUnit ePresMetric,
        OUString &rText ) const
{
    static sal_Char const sComma[] = ", ";

    rText.clear();
    OUString aStr;
    if( Count() )
    {
        SfxItemIter aIter( *this );
        const IntlWrapper rInt( ::comphelper::getProcessComponentContext(),
                                    GetAppLanguageTag() );
        while( true )
        {
            aIter.GetCurItem()->GetPresentation( ePres, eCoreMetric,
                                                 ePresMetric, aStr,
                                                 &rInt );
            if( rText.getLength() && aStr.getLength() )
                rText += sComma;
            rText += aStr;
            if( aIter.IsAtEnd() )
                break;
            aIter.NextItem();
        }
    }
}

bool SwFormatCharFormat::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    const SwCharFormat *pCharFormat = GetCharFormat();
    if ( pCharFormat )
    {
        OUString aStr;
        rText = OUString( SW_RESSTR( STR_CHARFMT ) );
        pCharFormat->GetPresentation( ePres, eCoreUnit, ePresUnit, aStr );
        rText = rText + "(" + aStr + ")";
    }
    else
        rText = OUString( SW_RESSTR( STR_NO_CHARFMT ) );
    return true;
}

bool SwFormatAutoFormat::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    rText.clear(); //TODO
    return true;
}

bool SwFormatINetFormat::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    rText = GetValue();
    return true;
}

bool SwFormatRuby::GetPresentation( SfxItemPresentation /*ePres*/,
                            SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/,
                            OUString &rText, const IntlWrapper* /*pIntl*/ ) const
{
    rText.clear();
    return true;
}

bool SwFormatDrop::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    rText.clear();
    if ( GetLines() > 1 )
    {
        if ( GetChars() > 1 )
        {
            rText = OUString::number( GetChars() ) + " ";
        }
        rText = rText +
                OUString( SW_RESSTR( STR_DROP_OVER ) ) +
                " " +
                OUString::number( GetLines() ) +
                " " +
                OUString( SW_RESSTR( STR_DROP_LINES ) );
    }
    else
        rText = SW_RESSTR( STR_NO_DROP_LINES );
    return true;
}

bool SwRegisterItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    const sal_uInt16 nId = GetValue() ? STR_REGISTER_ON : STR_REGISTER_OFF;
    rText = SW_RESSTR( nId );
    return true;
}

bool SwNumRuleItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    if( !GetValue().isEmpty() )
        rText = SW_RESSTR( STR_NUMRULE_ON ) +
            "(" + GetValue() + ")";
    else
        rText = SW_RESSTR( STR_NUMRULE_OFF );
    return true;
}

bool SwParaConnectBorderItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    const sal_uInt16 nId = GetValue() ? STR_CONNECT_BORDER_ON : STR_CONNECT_BORDER_OFF;
    rText = SW_RESSTR( nId );
    return true;
}

// Frame attribute

bool SwFormatFrmSize::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    OUString&           rText,
    const IntlWrapper*        pIntl
)   const
{
    rText = SW_RESSTR( STR_FRM_WIDTH ) + " ";
    if ( GetWidthPercent() )
    {
        rText = rText + unicode::formatPercent(GetWidthPercent(),
            Application::GetSettings().GetUILanguageTag());
    }
    else
    {
        rText = rText + ::GetMetricText( GetWidth(), eCoreUnit, ePresUnit, pIntl ) +
            " " + ::GetSvxString( ::GetMetricId( ePresUnit ) );
    }
    if ( ATT_VAR_SIZE != GetHeightSizeType() )
    {
        const sal_uInt16 nId = ATT_FIX_SIZE == m_eFrmHeightType ?
                                STR_FRM_FIXEDHEIGHT : STR_FRM_MINHEIGHT;
        rText = rText + ", " + SW_RESSTR( nId ) + " ";
        if ( GetHeightPercent() )
        {
            rText = rText + unicode::formatPercent(GetHeightPercent(),
                Application::GetSettings().GetUILanguageTag());
        }
        else
        {
            rText = OUString( ::GetMetricText( GetHeight(), eCoreUnit, ePresUnit, pIntl ) ) +
                    " " + ::GetSvxString( ::GetMetricId( ePresUnit ) );
        }
    }
    return true;
}

//Header for page formats.
//Client of FrameFormat which describes the header.

bool SwFormatHeader::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    const sal_uInt16 nId = GetHeaderFormat() ? STR_HEADER : STR_NO_HEADER;
    rText = SW_RESSTR( nId );
    return true;
}

//Footer for page formats.
//Client of FrameFormat which describes the footer.

bool SwFormatFooter::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    const sal_uInt16 nId = GetFooterFormat() ? STR_FOOTER : STR_NO_FOOTER;
    rText = SW_RESSTR( nId );
    return true;
}

bool SwFormatSurround::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    sal_uInt16 nId = 0;
    switch ( (SwSurround)GetValue() )
    {
        case SURROUND_NONE:
            nId = STR_SURROUND_NONE;
        break;
        case SURROUND_THROUGHT:
            nId = STR_SURROUND_THROUGHT;
        break;
        case SURROUND_PARALLEL:
            nId = STR_SURROUND_PARALLEL;
        break;
        case SURROUND_IDEAL:
            nId = STR_SURROUND_IDEAL;
        break;
        case SURROUND_LEFT:
            nId = STR_SURROUND_LEFT;
        break;
        case SURROUND_RIGHT:
            nId = STR_SURROUND_RIGHT;
        break;
        default:;//prevent warning
    }
    if ( nId )
        rText = SW_RESSTR( nId );

    if ( IsAnchorOnly() )
    {
        rText = rText + " " + SW_RESSTR( STR_SURROUND_ANCHORONLY );
    }
    return true;
}

//VertOrientation, how and by what orientate the FlyFrm in the vertical?

bool SwFormatVertOrient::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    OUString&           rText,
    const IntlWrapper*        pIntl
)   const
{
    sal_uInt16 nId = 0;
    switch ( GetVertOrient() )
    {
        case text::VertOrientation::NONE:
        {
            rText = rText + SW_RESSTR( STR_POS_Y ) + " " +
                    ::GetMetricText( GetPos(), eCoreUnit, ePresUnit, pIntl ) +
                    " " + ::GetSvxString( ::GetMetricId( ePresUnit ) );
        }
        break;
        case text::VertOrientation::TOP:
            nId = STR_VERT_TOP;
            break;
        case text::VertOrientation::CENTER:
            nId = STR_VERT_CENTER;
            break;
        case text::VertOrientation::BOTTOM:
            nId = STR_VERT_BOTTOM;
            break;
        case text::VertOrientation::LINE_TOP:
            nId = STR_LINE_TOP;
            break;
        case text::VertOrientation::LINE_CENTER:
            nId = STR_LINE_CENTER;
            break;
        case text::VertOrientation::LINE_BOTTOM:
            nId = STR_LINE_BOTTOM;
            break;
        default:;//prevent warning
    }
    if ( nId )
        rText += SW_RESSTR( nId );
    return true;
}

//HoriOrientation, how and by what orientate the FlyFrm in the horizontal?

bool SwFormatHoriOrient::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    OUString&           rText,
    const IntlWrapper*        pIntl
)   const
{
    sal_uInt16 nId = 0;
    switch ( GetHoriOrient() )
    {
        case text::HoriOrientation::NONE:
        {
            rText = rText + SW_RESSTR( STR_POS_X ) + " " +
                    ::GetMetricText( GetPos(), eCoreUnit, ePresUnit, pIntl ) +
                    " " + ::GetSvxString( ::GetMetricId( ePresUnit ) );
        }
        break;
        case text::HoriOrientation::RIGHT:
            nId = STR_HORI_RIGHT;
        break;
        case text::HoriOrientation::CENTER:
            nId = STR_HORI_CENTER;
        break;
        case text::HoriOrientation::LEFT:
            nId = STR_HORI_LEFT;
        break;
        case text::HoriOrientation::INSIDE:
            nId = STR_HORI_INSIDE;
        break;
        case text::HoriOrientation::OUTSIDE:
            nId = STR_HORI_OUTSIDE;
        break;
        case text::HoriOrientation::FULL:
            nId = STR_HORI_FULL;
        break;
        default:;//prevent warning
    }
    if ( nId )
        rText += SW_RESSTR( nId );
    return true;
}

// FlyAnchor, Anchor of the free-flying frame

bool SwFormatAnchor::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    sal_uInt16 nId = 0;
    switch ( GetAnchorId() )
    {
        case FLY_AT_PARA:
            nId = STR_FLY_AT_PARA;
            break;
        case FLY_AS_CHAR:
            nId = STR_FLY_AS_CHAR;
            break;
        case FLY_AT_PAGE:
            nId = STR_FLY_AT_PAGE;
            break;
        default:;//prevent warning
    }
    if ( nId )
        rText += SW_RESSTR( nId );
    return true;
}

bool SwFormatPageDesc::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    const SwPageDesc *pPageDesc = GetPageDesc();
    if ( pPageDesc )
        rText = pPageDesc->GetName();
    else
        rText = SW_RESSTR( STR_NO_PAGEDESC );
    return true;
}

//The ColumnDescriptor

bool SwFormatCol::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        pIntl
)   const
{
    sal_uInt16 nCnt = GetNumCols();
    if ( nCnt > 1 )
    {
        rText = OUString::number(nCnt) + " " + SW_RESSTR( STR_COLUMNS );
        if ( COLADJ_NONE != GetLineAdj() )
        {
            const long nWdth = static_cast<long>(GetLineWidth());
            rText = rText + " " + SW_RESSTR( STR_LINE_WIDTH ) + " " +
                    ::GetMetricText( nWdth, eCoreUnit,
                                      SFX_MAPUNIT_POINT, pIntl );
        }
    }
    else
        rText.clear();
    return true;
}

//URL's and maps

bool SwFormatURL::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    rText.clear();
    if ( pMap )
        rText += "Client-Map";
    if ( !sURL.isEmpty() )
    {
        if ( pMap )
            rText += " - ";
        rText = rText + "URL: " + sURL;
        if ( bIsServerMap )
            rText += " (Server-Map)";
    }
    if ( !sTargetFrameName.isEmpty() )
    {
        rText = rText + ", Target: " + sTargetFrameName;
    }
    return true;
}

bool SwFormatEditInReadonly::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    rText.clear();
    if ( GetValue() )
        rText = SW_RESSTR(STR_EDIT_IN_READONLY);
    return true;
}

void SwFormatEditInReadonly::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swFormatEditInReadonly"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::boolean(GetValue()).getStr()));
    xmlTextWriterEndElement(pWriter);
}

bool SwFormatLayoutSplit::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    if ( GetValue() )
        rText = SW_RESSTR(STR_LAYOUT_SPLIT);
    return true;
}

bool SwFormatRowSplit::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           /*rText*/,
    const IntlWrapper*        /*pIntl*/
)   const
{
    return false;
}

bool SwFormatFootnoteEndAtTextEnd::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           /*rText*/,
    const IntlWrapper*        /*pIntl*/
)   const
{
    return true;
}

bool SwFormatChain::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    if ( GetPrev() || GetNext() )
    {
        rText = SW_RESSTR(STR_CONNECT1);
        if ( GetPrev() )
        {
            rText += GetPrev()->GetName();
            if ( GetNext() )
                rText += SW_RESSTR(STR_CONNECT2);
        }
        if ( GetNext() )
            rText += GetNext()->GetName();
    }
    return true;
}

bool SwFormatLineNumber::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*    /*pIntl*/
)   const
{
    if ( IsCount() )
        rText += SW_RESSTR(STR_LINECOUNT);
    else
        rText += SW_RESSTR(STR_DONTLINECOUNT);
    if ( GetStartValue() )
    {
        rText = rText + " " + SW_RESSTR(STR_LINCOUNT_START) +
                OUString::number( GetStartValue() );
    }
    return true;
}

bool SwTextGridItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*  /*pIntl*/
)   const
{
    sal_uInt16 nId = 0;

    switch ( GetGridType() )
    {
    case GRID_NONE :
        nId = STR_GRID_NONE;
        break;
    case GRID_LINES_ONLY :
        nId = STR_GRID_LINES_ONLY;
        break;
    case GRID_LINES_CHARS :
        nId = STR_GRID_LINES_CHARS;
        break;
    }
    if ( nId )
        rText += SW_RESSTR( nId );
    return true;
}

bool SwHeaderAndFooterEatSpacingItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           /*rText*/,
    const IntlWrapper*        /*pIntl*/
)   const
{
    return false;
}

// Graphic attributes

bool SwMirrorGrf::GetPresentation(
    SfxItemPresentation /*ePres*/, SfxMapUnit /*eCoreUnit*/, SfxMapUnit /*ePresUnit*/,
    OUString& rText, const IntlWrapper* /*pIntl*/ ) const
{
    sal_uInt16 nId;
    switch( GetValue() )
    {
    case RES_MIRROR_GRAPH_DONT:     nId = STR_NO_MIRROR;    break;
    case RES_MIRROR_GRAPH_VERT: nId = STR_VERT_MIRROR;  break;
    case RES_MIRROR_GRAPH_HOR:  nId = STR_HORI_MIRROR;  break;
    case RES_MIRROR_GRAPH_BOTH: nId = STR_BOTH_MIRROR;  break;
    default:                    nId = 0;    break;
    }
    if ( nId )
    {
        rText = SW_RESSTR( nId );
        if (bGrfToggle)
            rText += SW_RESSTR( STR_MIRROR_TOGGLE );
    }
    return true;
}

bool SwRotationGrf::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit /*eCoreUnit*/, SfxMapUnit /*ePresUnit*/,
    OUString &rText, const IntlWrapper* /*pIntl*/) const
{
    if( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
        rText = SW_RESSTR( STR_ROTATION );
    else if( rText.getLength() )
        rText.clear();
    rText = rText + OUString::number( GetValue() ) + "\xB0";
    return true;
}

bool SwLuminanceGrf::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit /*eCoreUnit*/, SfxMapUnit /*ePresUnit*/,
    OUString &rText, const IntlWrapper* /*pIntl*/) const
{
    if( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
        rText = SW_RESSTR( STR_LUMINANCE );
    else if( rText.getLength() )
        rText.clear();
    rText = rText + unicode::formatPercent(GetValue(),
        Application::GetSettings().GetUILanguageTag());
    return true;
}

bool SwContrastGrf::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit /*eCoreUnit*/, SfxMapUnit /*ePresUnit*/,
    OUString &rText, const IntlWrapper* /*pIntl*/) const
{
    if( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
        rText = SW_RESSTR( STR_CONTRAST );
    else if( rText.getLength() )
        rText.clear();
    rText = rText + unicode::formatPercent(GetValue(),
        Application::GetSettings().GetUILanguageTag());
    return true;
}

bool SwChannelGrf::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit /*eCoreUnit*/, SfxMapUnit /*ePresUnit*/,
    OUString &rText, const IntlWrapper* /*pIntl*/) const
{
    if( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
    {
        sal_uInt16 nId;
        switch ( Which() )
        {
        case RES_GRFATR_CHANNELR:   nId = STR_CHANNELR; break;
        case RES_GRFATR_CHANNELG:   nId = STR_CHANNELG; break;
        case RES_GRFATR_CHANNELB:   nId = STR_CHANNELB; break;
        default:                    nId = 0; break;
        }
        if( nId )
            rText = SW_RESSTR( nId );
        else if( rText.getLength() )
            rText.clear();
    }
    else if( rText.getLength() )
        rText.clear();
    rText = rText + unicode::formatPercent(GetValue(),
        Application::GetSettings().GetUILanguageTag());
    return true;
}

bool SwGammaGrf::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit /*eCoreUnit*/, SfxMapUnit /*ePresUnit*/,
    OUString &rText, const IntlWrapper* /*pIntl*/) const
{
    OUStringBuffer aText;
    if( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
        aText.append(SW_RESSTR(STR_GAMMA));
    aText.append(unicode::formatPercent(GetValue(),
        Application::GetSettings().GetUILanguageTag()));
    rText = aText.makeStringAndClear();
    return true;
}

bool SwInvertGrf::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit /*eCoreUnit*/, SfxMapUnit /*ePresUnit*/,
    OUString &rText, const IntlWrapper* /*pIntl*/) const
{
    rText.clear();
    if( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
    {
        const sal_uInt16 nId = GetValue() ? STR_INVERT : STR_INVERT_NOT;
        rText = SW_RESSTR( nId );
    }
    return true;
}

bool SwTransparencyGrf::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit /*eCoreUnit*/, SfxMapUnit /*ePresUnit*/,
    OUString &rText, const IntlWrapper* /*pIntl*/) const
{
    if( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
        rText = SW_RESSTR( STR_TRANSPARENCY );
    else if( rText.getLength() )
        rText.clear();
    rText = rText + unicode::formatPercent(GetValue(),
        Application::GetSettings().GetUILanguageTag());
    return true;
}

bool SwDrawModeGrf::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit /*eCoreUnit*/, SfxMapUnit /*ePresUnit*/,
    OUString &rText, const IntlWrapper* /*pIntl*/) const
{
    rText.clear();
    if( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
    {
        sal_uInt16 nId;
        switch ( GetValue() )
        {

        case GRAPHICDRAWMODE_GREYS:     nId = STR_DRAWMODE_GREY; break;
        case GRAPHICDRAWMODE_MONO:      nId = STR_DRAWMODE_BLACKWHITE; break;
        case GRAPHICDRAWMODE_WATERMARK: nId = STR_DRAWMODE_WATERMARK; break;
        default:                        nId = STR_DRAWMODE_STD; break;
        }
        rText = SW_RESSTR( STR_DRAWMODE ) + SW_RESSTR( nId );
    }
    return true;
}

bool SwFormatFollowTextFlow::GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit /*eCoreMetric*/,
                                    SfxMapUnit /*ePresMetric*/,
                                    OUString &rText,
                                    const IntlWrapper*    /*pIntl*/ ) const
{
    rText.clear();
    if( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
    {
        const sal_uInt16 nId = GetValue() ? STR_FOLLOW_TEXT_FLOW : STR_DONT_FOLLOW_TEXT_FLOW;
        rText = SW_RESSTR( nId );
    }
    return true;
}

void SwFormatFollowTextFlow::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swFormatFollowTextFlow"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::boolean(GetValue()).getStr()));
    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
