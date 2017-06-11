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
#include <strings.hrc>
#include <fmtftntx.hxx>
#include <fmtfollowtextflow.hxx>
#include <libxml/xmlwriter.h>

using namespace com::sun::star;


// query the attribute descriptions
void SwAttrSet::GetPresentation(
        SfxItemPresentation ePres,
        MapUnit eCoreMetric,
        MapUnit ePresMetric,
        OUString &rText ) const
{
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
                rText += ", ";
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
    MapUnit             eCoreUnit,
    MapUnit             ePresUnit,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    const SwCharFormat *pCharFormat = GetCharFormat();
    if ( pCharFormat )
    {
        OUString aStr;
        rText = SwResId( STR_CHARFMT );
        pCharFormat->GetPresentation( ePres, eCoreUnit, ePresUnit, aStr );
        rText = rText + "(" + aStr + ")";
    }
    else
        rText = SwResId( STR_NO_CHARFMT );
    return true;
}

bool SwFormatAutoFormat::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
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
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    rText = GetValue();
    return true;
}

bool SwFormatRuby::GetPresentation( SfxItemPresentation /*ePres*/,
                                    MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
                                    OUString &rText, const IntlWrapper* /*pIntl*/ ) const
{
    rText.clear();
    return true;
}

bool SwFormatDrop::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
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
                SwResId( STR_DROP_OVER ) +
                " " +
                OUString::number( GetLines() ) +
                " " +
                SwResId( STR_DROP_LINES );
    }
    else
        rText = SwResId( STR_NO_DROP_LINES );
    return true;
}

bool SwRegisterItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    const char* pId = GetValue() ? STR_REGISTER_ON : STR_REGISTER_OFF;
    rText = SwResId(pId);
    return true;
}

bool SwNumRuleItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    if( !GetValue().isEmpty() )
        rText = SwResId( STR_NUMRULE_ON ) +
            "(" + GetValue() + ")";
    else
        rText = SwResId( STR_NUMRULE_OFF );
    return true;
}

bool SwParaConnectBorderItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    const char* pId = GetValue() ? STR_CONNECT_BORDER_ON : STR_CONNECT_BORDER_OFF;
    rText = SwResId(pId);
    return true;
}

// Frame attribute

bool SwFormatFrameSize::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             eCoreUnit,
    MapUnit             ePresUnit,
    OUString&           rText,
    const IntlWrapper*        pIntl
)   const
{
    rText = SwResId( STR_FRM_WIDTH ) + " ";
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
        const char* pId = ATT_FIX_SIZE == m_eFrameHeightType ?
                                STR_FRM_FIXEDHEIGHT : STR_FRM_MINHEIGHT;
        rText = rText + ", " + SwResId(pId) + " ";
        if ( GetHeightPercent() )
        {
            rText = rText + unicode::formatPercent(GetHeightPercent(),
                Application::GetSettings().GetUILanguageTag());
        }
        else
        {
            rText = ::GetMetricText( GetHeight(), eCoreUnit, ePresUnit, pIntl ) +
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
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    const char* pId = GetHeaderFormat() ? STR_HEADER : STR_NO_HEADER;
    rText = SwResId(pId);
    return true;
}

//Footer for page formats.
//Client of FrameFormat which describes the footer.

bool SwFormatFooter::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    const char* pId = GetFooterFormat() ? STR_FOOTER : STR_NO_FOOTER;
    rText = SwResId(pId);
    return true;
}

bool SwFormatSurround::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    const char* pId = nullptr;
    switch ( (css::text::WrapTextMode)GetValue() )
    {
        case css::text::WrapTextMode_NONE:
            pId = STR_SURROUND_NONE;
        break;
        case css::text::WrapTextMode_THROUGH:
            pId = STR_SURROUND_THROUGH;
        break;
        case css::text::WrapTextMode_PARALLEL:
            pId = STR_SURROUND_PARALLEL;
        break;
        case css::text::WrapTextMode_DYNAMIC:
            pId = STR_SURROUND_IDEAL;
        break;
        case css::text::WrapTextMode_LEFT:
            pId = STR_SURROUND_LEFT;
        break;
        case css::text::WrapTextMode_RIGHT:
            pId = STR_SURROUND_RIGHT;
        break;
        default:;//prevent warning
    }
    if (pId)
        rText = SwResId(pId);

    if ( IsAnchorOnly() )
    {
        rText = rText + " " + SwResId( STR_SURROUND_ANCHORONLY );
    }
    return true;
}

//VertOrientation, how and by what orientate the FlyFrame in the vertical?

bool SwFormatVertOrient::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             eCoreUnit,
    MapUnit             ePresUnit,
    OUString&           rText,
    const IntlWrapper*        pIntl
)   const
{
    const char* pId = nullptr;
    switch ( GetVertOrient() )
    {
        case text::VertOrientation::NONE:
        {
            rText = rText + SwResId( STR_POS_Y ) + " " +
                    ::GetMetricText( GetPos(), eCoreUnit, ePresUnit, pIntl ) +
                    " " + ::GetSvxString( ::GetMetricId( ePresUnit ) );
        }
        break;
        case text::VertOrientation::TOP:
            pId = STR_VERT_TOP;
            break;
        case text::VertOrientation::CENTER:
            pId = STR_VERT_CENTER;
            break;
        case text::VertOrientation::BOTTOM:
            pId = STR_VERT_BOTTOM;
            break;
        case text::VertOrientation::LINE_TOP:
            pId = STR_LINE_TOP;
            break;
        case text::VertOrientation::LINE_CENTER:
            pId = STR_LINE_CENTER;
            break;
        case text::VertOrientation::LINE_BOTTOM:
            pId = STR_LINE_BOTTOM;
            break;
        default:;//prevent warning
    }
    if (pId)
        rText += SwResId(pId);
    return true;
}

//HoriOrientation, how and by what orientate the FlyFrame in the horizontal?

bool SwFormatHoriOrient::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             eCoreUnit,
    MapUnit             ePresUnit,
    OUString&           rText,
    const IntlWrapper*        pIntl
)   const
{
    const char* pId = nullptr;
    switch ( GetHoriOrient() )
    {
        case text::HoriOrientation::NONE:
        {
            rText = rText + SwResId( STR_POS_X ) + " " +
                    ::GetMetricText( GetPos(), eCoreUnit, ePresUnit, pIntl ) +
                    " " + ::GetSvxString( ::GetMetricId( ePresUnit ) );
        }
        break;
        case text::HoriOrientation::RIGHT:
            pId = STR_HORI_RIGHT;
        break;
        case text::HoriOrientation::CENTER:
            pId = STR_HORI_CENTER;
        break;
        case text::HoriOrientation::LEFT:
            pId = STR_HORI_LEFT;
        break;
        case text::HoriOrientation::INSIDE:
            pId = STR_HORI_INSIDE;
        break;
        case text::HoriOrientation::OUTSIDE:
            pId = STR_HORI_OUTSIDE;
        break;
        case text::HoriOrientation::FULL:
            pId = STR_HORI_FULL;
        break;
        default:;//prevent warning
    }
    if (pId)
        rText += SwResId(pId);
    return true;
}

// FlyAnchor, Anchor of the free-flying frame

bool SwFormatAnchor::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    const char* pId = nullptr;
    switch ( GetAnchorId() )
    {
        case RndStdIds::FLY_AT_PARA:
            pId = STR_FLY_AT_PARA;
            break;
        case RndStdIds::FLY_AS_CHAR:
            pId = STR_FLY_AS_CHAR;
            break;
        case RndStdIds::FLY_AT_PAGE:
            pId = STR_FLY_AT_PAGE;
            break;
        default:;//prevent warning
    }
    if (pId)
        rText += SwResId(pId);
    return true;
}

bool SwFormatPageDesc::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    const SwPageDesc *pPageDesc = GetPageDesc();
    if ( pPageDesc )
        rText = pPageDesc->GetName();
    else
        rText = SwResId( STR_NO_PAGEDESC );
    return true;
}

//The ColumnDescriptor

bool SwFormatCol::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             eCoreUnit,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        pIntl
)   const
{
    sal_uInt16 nCnt = GetNumCols();
    if ( nCnt > 1 )
    {
        rText = OUString::number(nCnt) + " " + SwResId( STR_COLUMNS );
        if ( COLADJ_NONE != GetLineAdj() )
        {
            const long nWdth = static_cast<long>(GetLineWidth());
            rText = rText + " " + SwResId( STR_LINE_WIDTH ) + " " +
                    ::GetMetricText( nWdth, eCoreUnit,
                                      MapUnit::MapPoint, pIntl );
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
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
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
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    rText.clear();
    if ( GetValue() )
        rText = SwResId(STR_EDIT_IN_READONLY);
    return true;
}

void SwFormatEditInReadonly::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatEditInReadonly"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::boolean(GetValue()).getStr()));
    xmlTextWriterEndElement(pWriter);
}

bool SwFormatLayoutSplit::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    if ( GetValue() )
        rText = SwResId(STR_LAYOUT_SPLIT);
    return true;
}

bool SwFormatRowSplit::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           /*rText*/,
    const IntlWrapper*        /*pIntl*/
)   const
{
    return false;
}

bool SwFormatFootnoteEndAtTextEnd::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           /*rText*/,
    const IntlWrapper*        /*pIntl*/
)   const
{
    return true;
}

bool SwFormatChain::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*        /*pIntl*/
)   const
{
    if ( GetPrev() || GetNext() )
    {
        rText = SwResId(STR_CONNECT1);
        if ( GetPrev() )
        {
            rText += GetPrev()->GetName();
            if ( GetNext() )
                rText += SwResId(STR_CONNECT2);
        }
        if ( GetNext() )
            rText += GetNext()->GetName();
    }
    return true;
}

bool SwFormatLineNumber::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*    /*pIntl*/
)   const
{
    if ( IsCount() )
        rText += SwResId(STR_LINECOUNT);
    else
        rText += SwResId(STR_DONTLINECOUNT);
    if ( GetStartValue() )
    {
        rText = rText + " " + SwResId(STR_LINCOUNT_START) +
                OUString::number( GetStartValue() );
    }
    return true;
}

bool SwTextGridItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper*  /*pIntl*/
)   const
{
    const char* pId = nullptr;

    switch ( GetGridType() )
    {
    case GRID_NONE :
        pId = STR_GRID_NONE;
        break;
    case GRID_LINES_ONLY :
        pId = STR_GRID_LINES_ONLY;
        break;
    case GRID_LINES_CHARS :
        pId = STR_GRID_LINES_CHARS;
        break;
    }
    if (pId)
        rText += SwResId(pId);
    return true;
}

bool SwHeaderAndFooterEatSpacingItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           /*rText*/,
    const IntlWrapper*        /*pIntl*/
)   const
{
    return false;
}

// Graphic attributes

bool SwMirrorGrf::GetPresentation(
    SfxItemPresentation /*ePres*/, MapUnit /*eCoreUnit*/, MapUnit /*ePresUnit*/,
    OUString& rText, const IntlWrapper* /*pIntl*/ ) const
{
    const char* pId;
    switch( GetValue() )
    {
    case MirrorGraph::Dont:     pId = STR_NO_MIRROR;    break;
    case MirrorGraph::Vertical: pId = STR_VERT_MIRROR;  break;
    case MirrorGraph::Horizontal:  pId = STR_HORI_MIRROR;  break;
    case MirrorGraph::Both:     pId = STR_BOTH_MIRROR;  break;
    default:                    pId = nullptr;    break;
    }
    if (pId)
    {
        rText = SwResId(pId);
        if (bGrfToggle)
            rText += SwResId( STR_MIRROR_TOGGLE );
    }
    return true;
}

bool SwRotationGrf::GetPresentation(
    SfxItemPresentation ePres, MapUnit /*eCoreUnit*/, MapUnit /*ePresUnit*/,
    OUString &rText, const IntlWrapper* /*pIntl*/) const
{
    if( SfxItemPresentation::Complete == ePres )
        rText = SwResId( STR_ROTATION );
    else if( rText.getLength() )
        rText.clear();
    rText = rText + OUString::number( GetValue() ) + "\xB0";
    return true;
}

bool SwLuminanceGrf::GetPresentation(
    SfxItemPresentation ePres, MapUnit /*eCoreUnit*/, MapUnit /*ePresUnit*/,
    OUString &rText, const IntlWrapper* /*pIntl*/) const
{
    if( SfxItemPresentation::Complete == ePres )
        rText = SwResId( STR_LUMINANCE );
    else if( rText.getLength() )
        rText.clear();
    rText = rText + unicode::formatPercent(GetValue(),
        Application::GetSettings().GetUILanguageTag());
    return true;
}

bool SwContrastGrf::GetPresentation(
    SfxItemPresentation ePres, MapUnit /*eCoreUnit*/, MapUnit /*ePresUnit*/,
    OUString &rText, const IntlWrapper* /*pIntl*/) const
{
    if( SfxItemPresentation::Complete == ePres )
        rText = SwResId( STR_CONTRAST );
    else if( rText.getLength() )
        rText.clear();
    rText = rText + unicode::formatPercent(GetValue(),
        Application::GetSettings().GetUILanguageTag());
    return true;
}

bool SwChannelGrf::GetPresentation(
    SfxItemPresentation ePres, MapUnit /*eCoreUnit*/, MapUnit /*ePresUnit*/,
    OUString &rText, const IntlWrapper* /*pIntl*/) const
{
    if( SfxItemPresentation::Complete == ePres )
    {
        const char* pId;
        switch ( Which() )
        {
        case RES_GRFATR_CHANNELR:   pId = STR_CHANNELR; break;
        case RES_GRFATR_CHANNELG:   pId = STR_CHANNELG; break;
        case RES_GRFATR_CHANNELB:   pId = STR_CHANNELB; break;
        default:                    pId = nullptr; break;
        }
        if (pId)
            rText = SwResId(pId);
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
    SfxItemPresentation ePres, MapUnit /*eCoreUnit*/, MapUnit /*ePresUnit*/,
    OUString &rText, const IntlWrapper* /*pIntl*/) const
{
    OUStringBuffer aText;
    if( SfxItemPresentation::Complete == ePres )
        aText.append(SwResId(STR_GAMMA));
    aText.append(unicode::formatPercent(GetValue(),
        Application::GetSettings().GetUILanguageTag()));
    rText = aText.makeStringAndClear();
    return true;
}

bool SwInvertGrf::GetPresentation(
    SfxItemPresentation ePres, MapUnit /*eCoreUnit*/, MapUnit /*ePresUnit*/,
    OUString &rText, const IntlWrapper* /*pIntl*/) const
{
    rText.clear();
    if( SfxItemPresentation::Complete == ePres )
    {
        const char* pId = GetValue() ? STR_INVERT : STR_INVERT_NOT;
        rText = SwResId(pId);
    }
    return true;
}

bool SwTransparencyGrf::GetPresentation(
    SfxItemPresentation ePres, MapUnit /*eCoreUnit*/, MapUnit /*ePresUnit*/,
    OUString &rText, const IntlWrapper* /*pIntl*/) const
{
    if( SfxItemPresentation::Complete == ePres )
        rText = SwResId( STR_TRANSPARENCY );
    else if( rText.getLength() )
        rText.clear();
    rText = rText + unicode::formatPercent(GetValue(),
        Application::GetSettings().GetUILanguageTag());
    return true;
}

bool SwDrawModeGrf::GetPresentation(
    SfxItemPresentation ePres, MapUnit /*eCoreUnit*/, MapUnit /*ePresUnit*/,
    OUString &rText, const IntlWrapper* /*pIntl*/) const
{
    rText.clear();
    if( SfxItemPresentation::Complete == ePres )
    {
        const char* pId;
        switch ( GetValue() )
        {

        case GraphicDrawMode::Greys:     pId = STR_DRAWMODE_GREY; break;
        case GraphicDrawMode::Mono:      pId = STR_DRAWMODE_BLACKWHITE; break;
        case GraphicDrawMode::Watermark: pId = STR_DRAWMODE_WATERMARK; break;
        default:                         pId = STR_DRAWMODE_STD; break;
        }
        rText = SwResId( STR_DRAWMODE ) + SwResId(pId);
    }
    return true;
}

bool SwFormatFollowTextFlow::GetPresentation( SfxItemPresentation ePres,
                                              MapUnit /*eCoreMetric*/,
                                              MapUnit /*ePresMetric*/,
                                              OUString &rText,
                                              const IntlWrapper*    /*pIntl*/ ) const
{
    rText.clear();
    if( SfxItemPresentation::Complete == ePres )
    {
        const char* pId = GetValue() ? STR_FOLLOW_TEXT_FLOW : STR_DONT_FOLLOW_TEXT_FLOW;
        rText = SwResId(pId);
    }
    return true;
}

void SwFormatFollowTextFlow::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatFollowTextFlow"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::boolean(GetValue()).getStr()));
    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
