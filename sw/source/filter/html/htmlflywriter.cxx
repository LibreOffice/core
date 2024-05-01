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

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <hintids.hxx>
#include <tools/fract.hxx>
#include <svl/urihelper.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/event.hxx>
#include <svtools/htmlkywd.hxx>
#include <svtools/htmlout.hxx>
#include <svtools/htmltokn.h>
#include <vcl/imap.hxx>
#include <vcl/imapobj.hxx>
#include <svtools/htmlcfg.hxx>
#include <svtools/HtmlWriter.hxx>
#include <svx/svdouno.hxx>
#include <svx/xoutbmp.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/brushitem.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <svx/svdograf.hxx>
#include <comphelper/xmlencode.hxx>

#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmturl.hxx>
#include <fmtfsize.hxx>
#include <fmtclds.hxx>
#include <fmtcntnt.hxx>
#include <fmtsrnd.hxx>
#include <fmtinfmt.hxx>
#include <txtinet.hxx>
#include <frmatr.hxx>
#include <grfatr.hxx>
#include <flypos.hxx>
#include <ndgrf.hxx>

#include <doc.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <swerror.h>
#include <frmfmt.hxx>
#include "wrthtml.hxx"
#include "htmlatr.hxx"
#include "htmlfly.hxx"
#include "htmlreqifreader.hxx"

using namespace css;

const HtmlFrmOpts HTML_FRMOPTS_IMG_ALL        =
    HtmlFrmOpts::Alt |
    HtmlFrmOpts::Size |
    HtmlFrmOpts::AnySize |
    HtmlFrmOpts::Border |
    HtmlFrmOpts::Name;
const HtmlFrmOpts HTML_FRMOPTS_IMG_CNTNR      =
    HTML_FRMOPTS_IMG_ALL |
    HtmlFrmOpts::AbsSize;
const HtmlFrmOpts HTML_FRMOPTS_IMG            =
    HTML_FRMOPTS_IMG_ALL |
    HtmlFrmOpts::Align |
    HtmlFrmOpts::Space |
    HtmlFrmOpts::BrClear;
const HtmlFrmOpts HTML_FRMOPTS_IMG_CSS1       =
    HtmlFrmOpts::SAlign |
    HtmlFrmOpts::SSpace;

const HtmlFrmOpts HTML_FRMOPTS_DIV            =
    HtmlFrmOpts::Id |
    HtmlFrmOpts::SAlign |
    HtmlFrmOpts::SSize |
    HtmlFrmOpts::AnySize |
    HtmlFrmOpts::AbsSize |
    HtmlFrmOpts::SSpace |
    HtmlFrmOpts::SBorder |
    HtmlFrmOpts::SBackground |
    HtmlFrmOpts::BrClear |
    HtmlFrmOpts::Dir;

const HtmlFrmOpts HTML_FRMOPTS_MULTICOL       =
    HtmlFrmOpts::Id |
    HtmlFrmOpts::Width |
    HtmlFrmOpts::AnySize |
    HtmlFrmOpts::AbsSize |
    HtmlFrmOpts::Dir;

const HtmlFrmOpts HTML_FRMOPTS_MULTICOL_CSS1  =
    HtmlFrmOpts::SAlign |
    HtmlFrmOpts::SSize |
    HtmlFrmOpts::SSpace |
    HtmlFrmOpts::SBorder|
    HtmlFrmOpts::SBackground;

const HtmlFrmOpts HTML_FRMOPTS_SPACER         =
    HtmlFrmOpts::Align |
    HtmlFrmOpts::Size |
    HtmlFrmOpts::AnySize |
    HtmlFrmOpts::BrClear |
    HtmlFrmOpts::MarginSize |
    HtmlFrmOpts::AbsSize;

const HtmlFrmOpts HTML_FRMOPTS_CNTNR          =
    HtmlFrmOpts::SAlign |
    HtmlFrmOpts::SSpace |
    HtmlFrmOpts::SWidth |
    HtmlFrmOpts::AnySize |
    HtmlFrmOpts::AbsSize |
    HtmlFrmOpts::SPixSize;

static Writer& OutHTML_FrameFormatTableNode( Writer& rWrt, const SwFrameFormat& rFrameFormat );
static Writer& OutHTML_FrameFormatAsMulticol( Writer& rWrt, const SwFrameFormat& rFormat,
                                         bool bInCntnr );
static Writer& OutHTML_FrameFormatAsSpacer( Writer& rWrt, const SwFrameFormat& rFormat );
static Writer& OutHTML_FrameFormatAsDivOrSpan( Writer& rWrt,
                                          const SwFrameFormat& rFrameFormat, bool bSpan );
static Writer& OutHTML_FrameFormatAsImage( Writer& rWrt, const SwFrameFormat& rFormat, bool bPNGFallback );

static Writer& OutHTML_FrameFormatGrfNode( Writer& rWrt, const SwFrameFormat& rFormat,
                                      bool bInCntnr, bool bPNGFallback );

static Writer& OutHTML_FrameFormatAsMarquee( Writer& rWrt, const SwFrameFormat& rFrameFormat,
                                        const SdrObject& rSdrObj    );

HTMLOutEvent const aImageEventTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_O_SDonload,       OOO_STRING_SVTOOLS_HTML_O_onload,   SvMacroItemId::OnImageLoadDone        },
    { OOO_STRING_SVTOOLS_HTML_O_SDonabort,      OOO_STRING_SVTOOLS_HTML_O_onabort,  SvMacroItemId::OnImageLoadCancel       },
    { OOO_STRING_SVTOOLS_HTML_O_SDonerror,      OOO_STRING_SVTOOLS_HTML_O_onerror,  SvMacroItemId::OnImageLoadError       },
    { nullptr, nullptr, SvMacroItemId::NONE }
};

HTMLOutEvent const aIMapEventTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_O_SDonmouseover,  OOO_STRING_SVTOOLS_HTML_O_onmouseover,  SvMacroItemId::OnMouseOver  },
    { OOO_STRING_SVTOOLS_HTML_O_SDonmouseout,   OOO_STRING_SVTOOLS_HTML_O_onmouseout,   SvMacroItemId::OnMouseOut   },
    { nullptr, nullptr, SvMacroItemId::NONE }
};

sal_uInt16 SwHTMLWriter::GuessFrameType( const SwFrameFormat& rFrameFormat,
                                   const SdrObject*& rpSdrObj )
{
    SwHTMLFrameType eType;

    if( RES_DRAWFRMFMT == rFrameFormat.Which() )
    {
        // use an arbitrary draw object as the default value
        eType = HTML_FRMTYPE_DRAW;

        const SdrObject *pObj =
            SwHTMLWriter::GetMarqueeTextObj( static_cast<const SwDrawFrameFormat &>(rFrameFormat) );
        if( pObj )
        {
            // scrolling text
            rpSdrObj = pObj;
            eType = HTML_FRMTYPE_MARQUEE;
        }
        else
        {
            pObj = GetHTMLControl( static_cast<const SwDrawFrameFormat &>(rFrameFormat) );

            if( pObj )
            {
                // Form control
                rpSdrObj = pObj;
                eType = HTML_FRMTYPE_CONTROL;
            }
        }
    }
    else
    {
        // use a text frame as the default value
        eType = HTML_FRMTYPE_TEXT;

        const SwFormatContent& rFlyContent = rFrameFormat.GetContent();
        SwNodeOffset nStt = rFlyContent.GetContentIdx()->GetIndex()+1;
        const SwNode* pNd = m_pDoc->GetNodes()[ nStt ];

        if( pNd->IsGrfNode() )
        {
            // graphic node
            eType = HTML_FRMTYPE_GRF;
        }
        else if( pNd->IsOLENode() )
        {
            // applet, plugin, floating frame
            eType = static_cast<SwHTMLFrameType>(GuessOLENodeFrameType( *pNd ));
        }
        else
        {
            SwNodeOffset nEnd = m_pDoc->GetNodes()[nStt-1]->EndOfSectionIndex();

            const SfxItemSet& rItemSet = rFrameFormat.GetAttrSet();
            const SwFormatCol* pFormatCol = rItemSet.GetItemIfSet( RES_COL );
            if( pFormatCol && pFormatCol->GetNumCols() > 1 )
            {
                // frame with columns
                eType = HTML_FRMTYPE_MULTICOL;
            }
            else if( pNd->IsTableNode() )
            {
                const SwTableNode *pTableNd = pNd->GetTableNode();
                SwNodeOffset nTableEnd = pTableNd->EndOfSectionIndex();

                if( nTableEnd+1 == nEnd )
                {
                    // table
                    eType = HTML_FRMTYPE_TABLE;
                }
                else if( nTableEnd+2 == nEnd )
                {
                    // table with caption
                    eType = HTML_FRMTYPE_TABLE_CAP;
                }
            }
            else if( pNd->IsTextNode() )
            {
                const SwTextNode *pTextNd = pNd->GetTextNode();

                bool bEmpty = false;
                if( nStt==nEnd-1 && !pTextNd->Len() )
                {
                    // empty frame? Only if no frame is
                    // anchored to the text or start node.
                    bEmpty = true;
                    for( auto & pHTMLPosFlyFrame : m_aHTMLPosFlyFrames )
                    {
                        SwNodeOffset nIdx = pHTMLPosFlyFrame->GetNdIndex().GetIndex();
                        bEmpty = (nIdx != nStt) && (nIdx != nStt-1);
                        if( !bEmpty || nIdx > nStt )
                            break;
                    }
                }
                if( bEmpty )
                {
                    std::unique_ptr<SvxBrushItem> aBrush = rFrameFormat.makeBackgroundBrushItem();
                    /// background is not empty, if it has a background graphic
                    /// or its background color is not "no fill"/"auto fill".
                    if( GPOS_NONE != aBrush->GetGraphicPos() ||
                        aBrush->GetColor() != COL_TRANSPARENT )
                    {
                        bEmpty = false;
                    }
                }
                if( bEmpty )
                {
                    // empty frame
                    eType = HTML_FRMTYPE_EMPTY;
                }
                else if( m_pDoc->GetNodes()[nStt+1]->IsTableNode() )
                {
                    const SwTableNode *pTableNd =
                        m_pDoc->GetNodes()[nStt+1]->GetTableNode();
                    if( pTableNd->EndOfSectionIndex()+1 == nEnd )
                    {
                        // table with heading
                        eType = HTML_FRMTYPE_TABLE_CAP;
                    }
                }
            }
        }
    }

    return static_cast< sal_uInt16 >(eType);
}

void SwHTMLWriter::CollectFlyFrames()
{
    OSL_ENSURE( HTML_CFG_MAX+1 == MAX_BROWSERS,
            "number of browser configurations has changed" );

    SwPosFlyFrames aFlyPos(
        m_pDoc->GetAllFlyFormats(m_bWriteAll ? nullptr : m_pCurrentPam.get(), true));

    for(const SwPosFlyFrame& rItem : aFlyPos)
    {
        const SwFrameFormat& rFrameFormat = rItem.GetFormat();
        const SdrObject *pSdrObj = nullptr;
        const SwNode *pAnchorNode;
        const SwContentNode *pACNd;
        SwHTMLFrameType eType = static_cast<SwHTMLFrameType>(GuessFrameType( rFrameFormat, pSdrObj ));

        AllHtmlFlags nMode;
        const SwFormatAnchor& rAnchor = rFrameFormat.GetAnchor();
        sal_Int16 eHoriRel = rFrameFormat.GetHoriOrient().GetRelationOrient();
        switch( rAnchor.GetAnchorId() )
        {
        case RndStdIds::FLY_AT_PAGE:
        case RndStdIds::FLY_AT_FLY:
            nMode = aHTMLOutFramePageFlyTable[eType][m_nExportMode];
            break;

        case RndStdIds::FLY_AT_PARA:
            // frames that are anchored to a paragraph are only placed
            // before the paragraph, if the paragraph has a
            // spacing.
            if( text::RelOrientation::FRAME == eHoriRel &&
                (pAnchorNode = rAnchor.GetAnchorNode()) != nullptr &&
                (pACNd = pAnchorNode->GetContentNode()) != nullptr )
            {
                const SvxLRSpaceItem& rLRItem =
                    pACNd->GetAttr(RES_LR_SPACE);
                if( rLRItem.GetTextLeft() || rLRItem.GetRight() )
                {
                    nMode = aHTMLOutFrameParaFrameTable[eType][m_nExportMode];
                    break;
                }
            }
            nMode = aHTMLOutFrameParaPrtAreaTable[eType][m_nExportMode];
            break;

        case RndStdIds::FLY_AT_CHAR:
            if( text::RelOrientation::FRAME == eHoriRel || text::RelOrientation::PRINT_AREA == eHoriRel )
                nMode = aHTMLOutFrameParaPrtAreaTable[eType][m_nExportMode];
            else
                nMode = aHTMLOutFrameParaOtherTable[eType][m_nExportMode];
            break;

        default:
            nMode = aHTMLOutFrameParaPrtAreaTable[eType][m_nExportMode];
            break;
        }

        m_aHTMLPosFlyFrames.insert( std::make_unique<SwHTMLPosFlyFrame>(rItem, pSdrObj, nMode) );
    }
}

bool SwHTMLWriter::OutFlyFrame( SwNodeOffset nNdIdx, sal_Int32 nContentIdx, HtmlPosition nPos )
{
    bool bFlysLeft = false; // Are there still Flys left at the current node position?

    // OutFlyFrame can be called recursively. Thus, sometimes it is
    // necessary to start over after a Fly was returned.
    bool bRestart = true;
    while( !m_aHTMLPosFlyFrames.empty() && bRestart )
    {
        bFlysLeft = bRestart = false;

        // search for the beginning of the FlyFrames
        size_t i {0};

        for( ; i < m_aHTMLPosFlyFrames.size() &&
            m_aHTMLPosFlyFrames[i]->GetNdIndex().GetIndex() < nNdIdx; i++ )
            ;
        for( ; !bRestart && i < m_aHTMLPosFlyFrames.size() &&
            m_aHTMLPosFlyFrames[i]->GetNdIndex().GetIndex() == nNdIdx; i++ )
        {
            SwHTMLPosFlyFrame *pPosFly = m_aHTMLPosFlyFrames[i].get();
            if( ( HtmlPosition::Any == nPos ||
                  pPosFly->GetOutPos() == nPos ) &&
                pPosFly->GetContentIndex() == nContentIdx )
            {
                // It is important to remove it first, because additional
                // elements or the whole array could be deleted on
                // deeper recursion levels.
                std::unique_ptr<SwHTMLPosFlyFrame> flyHolder = m_aHTMLPosFlyFrames.erase_extract(i);
                i--;
                if( m_aHTMLPosFlyFrames.empty() )
                {
                    bRestart = true;    // not really, only exit the loop
                }

                HTMLOutFuncs::FlushToAscii(Strm()); // it was one time only; do we still need it?

                OutFrameFormat( pPosFly->GetOutMode(), pPosFly->GetFormat(),
                                pPosFly->GetSdrObject() );
                switch( pPosFly->GetOutFn() )
                {
                case HtmlOut::Div:
                case HtmlOut::Span:
                case HtmlOut::MultiCol:
                case HtmlOut::TableNode:
                    bRestart = true; // It could become recursive here
                    break;
                default: break;
                }
            }
            else
            {
                bFlysLeft = true;
            }
        }
    }

    return bFlysLeft;
}

void SwHTMLWriter::OutFrameFormat( AllHtmlFlags nMode, const SwFrameFormat& rFrameFormat,
                              const SdrObject *pSdrObject )
{
    HtmlContainerFlags nCntnrMode = nMode.nContainer;
    HtmlOut nOutMode = nMode.nOut;
    OString aContainerStr;
    if( HtmlContainerFlags::NONE != nCntnrMode )
    {

        if (IsLFPossible() && HtmlContainerFlags::Div == nCntnrMode)
            OutNewLine();

        OStringBuffer sOut;
        aContainerStr = (HtmlContainerFlags::Div == nCntnrMode)
                            ? OOO_STRING_SVTOOLS_HTML_division
                            : OOO_STRING_SVTOOLS_HTML_span;
        sOut.append("<" + GetNamespace() + aContainerStr + " "
                OOO_STRING_SVTOOLS_HTML_O_class "=\""
                "sd-abs-pos\"");
        Strm().WriteOString( sOut );
        sOut.setLength(0);

        // Output a width for non-draw objects
        HtmlFrmOpts nFrameFlags = HTML_FRMOPTS_CNTNR;

        // For frames with columns we can also output the background
        if( HtmlOut::MultiCol == nOutMode )
            nFrameFlags |= HtmlFrmOpts::SBackground|HtmlFrmOpts::SBorder;

        if( IsHTMLMode( HTMLMODE_BORDER_NONE ) )
           nFrameFlags |= HtmlFrmOpts::SNoBorder;
        OutCSS1_FrameFormatOptions( rFrameFormat, nFrameFlags, pSdrObject );
        Strm().WriteChar( '>' );

        if( HtmlContainerFlags::Div == nCntnrMode )
        {
            IncIndentLevel();
            SetLFPossible(true);
        }
    }

    switch( nOutMode )
    {
    case HtmlOut::TableNode:      // OK
        OSL_ENSURE( aContainerStr.isEmpty(), "Table: Container is not supposed to be here" );
        OutHTML_FrameFormatTableNode( *this, rFrameFormat );
        break;
    case HtmlOut::GraphicNode:      // OK
        OutHTML_FrameFormatGrfNode( *this, rFrameFormat, !aContainerStr.isEmpty(), /*bPNGFallback=*/true );
        break;
    case HtmlOut::OleNode:      // OK
        OutHTML_FrameFormatOLENode( *this, rFrameFormat, !aContainerStr.isEmpty() );
        break;
    case HtmlOut::OleGraphic:       // OK
        OutHTML_FrameFormatOLENodeGrf( *this, rFrameFormat, !aContainerStr.isEmpty() );
        break;
    case HtmlOut::Div:
    case HtmlOut::Span:
        OSL_ENSURE( aContainerStr.isEmpty(), "Div: Container is not supposed to be here" );
        OutHTML_FrameFormatAsDivOrSpan( *this, rFrameFormat, HtmlOut::Span==nOutMode );
        break;
    case HtmlOut::MultiCol:     // OK
        OutHTML_FrameFormatAsMulticol( *this, rFrameFormat, !aContainerStr.isEmpty() );
        break;
    case HtmlOut::Spacer:       // OK
        OSL_ENSURE( aContainerStr.isEmpty(), "Spacer: Container is not supposed to be here" );
        OutHTML_FrameFormatAsSpacer( *this, rFrameFormat );
        break;
    case HtmlOut::Control:      // OK
        OutHTML_DrawFrameFormatAsControl( *this,
                                    static_cast<const SwDrawFrameFormat &>(rFrameFormat), dynamic_cast<const SdrUnoObj&>(*pSdrObject),
                                    !aContainerStr.isEmpty() );
        break;
    case HtmlOut::AMarquee:
        OutHTML_FrameFormatAsMarquee( *this, rFrameFormat, *pSdrObject );
        break;
    case HtmlOut::Marquee:
        OSL_ENSURE( aContainerStr.isEmpty(), "Marquee: Container is not supposed to be here" );
        OutHTML_DrawFrameFormatAsMarquee( *this,
                    static_cast<const SwDrawFrameFormat &>(rFrameFormat), *pSdrObject );
        break;
    case HtmlOut::GraphicFrame:
        OutHTML_FrameFormatAsImage( *this, rFrameFormat, /*bPNGFallback=*/true );
        break;
    }

    if( HtmlContainerFlags::Div == nCntnrMode )
    {
        DecIndentLevel();
        if (IsLFPossible())
            OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), Concat2View(GetNamespace() + OOO_STRING_SVTOOLS_HTML_division), false );
        SetLFPossible(true);
    }
    else if( HtmlContainerFlags::Span == nCntnrMode )
        HTMLOutFuncs::Out_AsciiTag( Strm(), Concat2View(GetNamespace() + OOO_STRING_SVTOOLS_HTML_span), false );
}

OString SwHTMLWriter::OutFrameFormatOptions( const SwFrameFormat &rFrameFormat,
                                     const OUString& rAlternateText,
                                     HtmlFrmOpts nFrameOpts )
{
    OString sRetEndTags;
    OStringBuffer sOut;
    const SfxItemSet& rItemSet = rFrameFormat.GetAttrSet();

    // Name
    if( (nFrameOpts & (HtmlFrmOpts::Id|HtmlFrmOpts::Name)) &&
        !rFrameFormat.GetName().isEmpty() )
    {
        const char *pStr =
            (nFrameOpts & HtmlFrmOpts::Id) ? OOO_STRING_SVTOOLS_HTML_O_id : OOO_STRING_SVTOOLS_HTML_O_name;
        sOut.append(OString::Concat(" ") + pStr + "=\"");
        Strm().WriteOString( sOut );
        sOut.setLength(0);
        HTMLOutFuncs::Out_String( Strm(), rFrameFormat.GetName() );
        sOut.append('\"');
    }

    // Name
    if( nFrameOpts & HtmlFrmOpts::Dir )
    {
        SvxFrameDirection nDir = GetHTMLDirection( rItemSet );
        Strm().WriteOString( sOut );
        sOut.setLength(0);
        OutDirection( nDir );
    }

    // ALT
    if( (nFrameOpts & HtmlFrmOpts::Alt) && !rAlternateText.isEmpty() )
    {
        sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_alt "=\"");
        Strm().WriteOString( sOut );
        sOut.setLength(0);
        HTMLOutFuncs::Out_String( Strm(), rAlternateText );
        sOut.append('\"');
    }

    // ALIGN
    const char *pStr = nullptr;
    RndStdIds eAnchorId = rFrameFormat.GetAnchor().GetAnchorId();
    if( (nFrameOpts & HtmlFrmOpts::Align) &&
        ((RndStdIds::FLY_AT_PARA == eAnchorId) || (RndStdIds::FLY_AT_CHAR == eAnchorId)) )
    {
        // MIB 12.3.98: Wouldn't it be more clever to left-align frames that
        // are anchored to a paragraph if necessary, instead of inserting them
        // as being anchored to characters?
        const SwFormatHoriOrient& rHoriOri = rFrameFormat.GetHoriOrient();
        if( !(nFrameOpts & HtmlFrmOpts::SAlign) ||
            text::RelOrientation::FRAME == rHoriOri.GetRelationOrient() ||
            text::RelOrientation::PRINT_AREA == rHoriOri.GetRelationOrient() )
        {
            pStr = text::HoriOrientation::RIGHT == rHoriOri.GetHoriOrient()
                        ? OOO_STRING_SVTOOLS_HTML_AL_right
                        : OOO_STRING_SVTOOLS_HTML_AL_left;
        }
    }
    const SwFormatVertOrient* pVertOrient;
    if( (nFrameOpts & HtmlFrmOpts::Align) && !pStr &&
        ( !(nFrameOpts & HtmlFrmOpts::SAlign) ||
          (RndStdIds::FLY_AS_CHAR == eAnchorId) ) &&
        (pVertOrient = rItemSet.GetItemIfSet( RES_VERT_ORIENT )) )
    {
        switch( pVertOrient->GetVertOrient() )
        {
        case text::VertOrientation::LINE_TOP:     pStr = OOO_STRING_SVTOOLS_HTML_VA_top;        break;
        case text::VertOrientation::CHAR_TOP:
        case text::VertOrientation::BOTTOM:       pStr = OOO_STRING_SVTOOLS_HTML_VA_texttop;    break;  // not possible
        case text::VertOrientation::LINE_CENTER:
        case text::VertOrientation::CHAR_CENTER:  pStr = OOO_STRING_SVTOOLS_HTML_VA_absmiddle;  break;  // not possible
        case text::VertOrientation::CENTER:       pStr = OOO_STRING_SVTOOLS_HTML_VA_middle;     break;
        case text::VertOrientation::LINE_BOTTOM:
        case text::VertOrientation::CHAR_BOTTOM:  pStr = OOO_STRING_SVTOOLS_HTML_VA_absbottom;  break;  // not possible
        case text::VertOrientation::TOP:          pStr = OOO_STRING_SVTOOLS_HTML_VA_bottom;     break;
        case text::VertOrientation::NONE:     break;
        }
    }
    if( pStr )
    {
        sOut.append(OString::Concat(" " OOO_STRING_SVTOOLS_HTML_O_align "=\"") +
                pStr + "\"");
    }

    // HSPACE and VSPACE
    Size aTwipSpc( 0, 0 );
    const SvxLRSpaceItem* pLRSpaceItem;
    if( (nFrameOpts & (HtmlFrmOpts::Space|HtmlFrmOpts::MarginSize)) &&
        (pLRSpaceItem = rItemSet.GetItemIfSet( RES_LR_SPACE )) )
    {
        aTwipSpc.setWidth(
            ( pLRSpaceItem->GetLeft() + pLRSpaceItem->GetRight() ) / 2 );
        m_nDfltLeftMargin = m_nDfltRightMargin = aTwipSpc.Width();
    }
    const SvxULSpaceItem* pULSpaceItem;
    if( (nFrameOpts & (HtmlFrmOpts::Space|HtmlFrmOpts::MarginSize)) &&
        (pULSpaceItem = rItemSet.GetItemIfSet( RES_UL_SPACE )) )
    {
        aTwipSpc.setHeight(
            ( pULSpaceItem->GetUpper() + pULSpaceItem->GetLower() ) / 2 );
        m_nDfltTopMargin = m_nDfltBottomMargin = o3tl::narrowing<sal_uInt16>(aTwipSpc.Height());
    }

    if( (nFrameOpts & HtmlFrmOpts::Space) &&
        (aTwipSpc.Width() || aTwipSpc.Height()) &&
        Application::GetDefaultDevice() &&
        !mbReqIF )
    {
        Size aPixelSpc =
            Application::GetDefaultDevice()->LogicToPixel( aTwipSpc,
                                                MapMode(MapUnit::MapTwip) );
        if( !aPixelSpc.Width() && aTwipSpc.Width() )
            aPixelSpc.setWidth( 1 );
        if( !aPixelSpc.Height() && aTwipSpc.Height() )
            aPixelSpc.setHeight( 1 );

        if( aPixelSpc.Width() )
        {
            sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_hspace
                    "=\"" + OString::number(aPixelSpc.Width()) + "\"");
        }

        if( aPixelSpc.Height() )
        {
            sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_vspace
                    "=\"" + OString::number(aPixelSpc.Height()) + "\"");
        }
    }

    // The spacing must be considered for the size, if the corresponding flag
    // is set.
    if( nFrameOpts & HtmlFrmOpts::MarginSize )
    {
        aTwipSpc.setWidth( aTwipSpc.Width() * -2 );
        aTwipSpc.setHeight( aTwipSpc.Height() * -2 );
    }
    else
    {
        aTwipSpc.setWidth( 0 );
        aTwipSpc.setHeight( 0 );
    }

    const SvxBoxItem* pBoxItem;
    if( !(nFrameOpts & HtmlFrmOpts::AbsSize) &&
        (pBoxItem = rItemSet.GetItemIfSet( RES_BOX )) )
    {
        aTwipSpc.AdjustWidth(pBoxItem->CalcLineSpace( SvxBoxItemLine::LEFT ) );
        aTwipSpc.AdjustWidth(pBoxItem->CalcLineSpace( SvxBoxItemLine::RIGHT ) );
        aTwipSpc.AdjustHeight(pBoxItem->CalcLineSpace( SvxBoxItemLine::TOP ) );
        aTwipSpc.AdjustHeight(pBoxItem->CalcLineSpace( SvxBoxItemLine::BOTTOM ) );
    }

    // WIDTH and/or HEIGHT
    // Output SwFrameSize::Variable/SwFrameSize::Minimum only, if ANYSIZE is set
    const SwFormatFrameSize *pFSItem;
    if( (nFrameOpts & HtmlFrmOpts::Size) &&
        (pFSItem = rItemSet.GetItemIfSet( RES_FRM_SIZE )) &&
        ( (nFrameOpts & HtmlFrmOpts::AnySize) ||
          SwFrameSize::Fixed == pFSItem->GetHeightSizeType()) )
    {
        sal_uInt8 nPercentWidth = pFSItem->GetWidthPercent();
        sal_uInt8 nPercentHeight = pFSItem->GetHeightPercent();

        // Size of the object in Twips without margins
        Size aTwipSz( (nPercentWidth ? 0
                                 : pFSItem->GetWidth()-aTwipSpc.Width()),
                      (nPercentHeight ? 0
                                  : pFSItem->GetHeight()-aTwipSpc.Height()) );

        OSL_ENSURE( aTwipSz.Width() >= 0 && aTwipSz.Height() >= 0, "Frame size minus spacing  < 0!!!???" );
        if( aTwipSz.Width() < 0 )
            aTwipSz.setWidth( 0 );
        if( aTwipSz.Height() < 0 )
            aTwipSz.setHeight( 0 );

        Size aPixelSz( 0, 0 );
        if( (aTwipSz.Width() || aTwipSz.Height()) &&
            Application::GetDefaultDevice() )
        {
            aPixelSz =
                Application::GetDefaultDevice()->LogicToPixel( aTwipSz,
                                                    MapMode(MapUnit::MapTwip) );
            if( !aPixelSz.Width() && aTwipSz.Width() )
                aPixelSz.setWidth( 1 );
            if( !aPixelSz.Height() && aTwipSz.Height() )
                aPixelSz.setHeight( 1 );
        }

        if( (nFrameOpts & HtmlFrmOpts::Width) &&
            ((nPercentWidth && nPercentWidth!=255) || aPixelSz.Width()) )
        {
            sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_width "=\"");
            if( nPercentWidth )
                sOut.append(static_cast<sal_Int32>(nPercentWidth)).append('%');
            else
                sOut.append(static_cast<sal_Int32>(aPixelSz.Width()));
            sOut.append("\"");
        }

        if( (nFrameOpts & HtmlFrmOpts::Height) &&
            ((nPercentHeight && nPercentHeight!=255) || aPixelSz.Height()) )
        {
            sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_height "=\"");
            if( nPercentHeight )
                sOut.append(static_cast<sal_Int32>(nPercentHeight)).append('%');
            else
                sOut.append(static_cast<sal_Int32>(aPixelSz.Height()));
            sOut.append("\"");
        }
    }

    if (!sOut.isEmpty())
    {
        Strm().WriteOString( sOut );
        sOut.setLength(0);
    }

    if (!mbReqIF)
    {
        // Insert wrap for graphics that are anchored to a paragraph as
        // <BR CLEAR=...> in the string
        const SwFormatSurround* pSurround;
        if( (nFrameOpts & HtmlFrmOpts::BrClear) &&
            ((RndStdIds::FLY_AT_PARA == rFrameFormat.GetAnchor().GetAnchorId()) ||
             (RndStdIds::FLY_AT_CHAR == rFrameFormat.GetAnchor().GetAnchorId())) &&
            (pSurround = rItemSet.GetItemIfSet( RES_SURROUND )) )
        {
            sal_Int16 eHoriOri =    rFrameFormat.GetHoriOrient().GetHoriOrient();
            pStr = nullptr;
            css::text::WrapTextMode eSurround = pSurround->GetSurround();
            bool bAnchorOnly = pSurround->IsAnchorOnly();
            switch( eHoriOri )
            {
            case text::HoriOrientation::RIGHT:
                {
                    switch( eSurround )
                    {
                    case css::text::WrapTextMode_NONE:
                    case css::text::WrapTextMode_RIGHT:
                        pStr = OOO_STRING_SVTOOLS_HTML_AL_right;
                        break;
                    case css::text::WrapTextMode_LEFT:
                    case css::text::WrapTextMode_PARALLEL:
                        if( bAnchorOnly )
                            m_bClearRight = true;
                        break;
                    default:
                        ;
                    }
                }
                break;

            default:
                // If a frame is centered, it gets left aligned. This
                // should be taken into account here, too.
                {
                    switch( eSurround )
                    {
                    case css::text::WrapTextMode_NONE:
                    case css::text::WrapTextMode_LEFT:
                        pStr = OOO_STRING_SVTOOLS_HTML_AL_left;
                        break;
                    case css::text::WrapTextMode_RIGHT:
                    case css::text::WrapTextMode_PARALLEL:
                        if( bAnchorOnly )
                            m_bClearLeft = true;
                        break;
                    default:
                        ;
                    }
                }
                break;

            }

            if( pStr )
            {
                sOut.append("<" OOO_STRING_SVTOOLS_HTML_linebreak
                        " " OOO_STRING_SVTOOLS_HTML_O_clear
                        "=\"" + OString::Concat(pStr) + "\">");
                sRetEndTags = sOut.makeStringAndClear();
            }
        }
    }
    return sRetEndTags;
}

void SwHTMLWriter::writeFrameFormatOptions(HtmlWriter& aHtml, const SwFrameFormat& rFrameFormat, std::u16string_view rAlternateText, HtmlFrmOpts nFrameOptions)
{
    bool bReplacement = (nFrameOptions & HtmlFrmOpts::Replacement) || mbReqIF;
    const SfxItemSet& rItemSet = rFrameFormat.GetAttrSet();

    // Name
    if( (nFrameOptions & (HtmlFrmOpts::Id|HtmlFrmOpts::Name)) &&
        !rFrameFormat.GetName().isEmpty() && !bReplacement)
    {
        const char* pAttributeName = (nFrameOptions & HtmlFrmOpts::Id) ? OOO_STRING_SVTOOLS_HTML_O_id : OOO_STRING_SVTOOLS_HTML_O_name;
        aHtml.attribute(pAttributeName, rFrameFormat.GetName());
    }

    // Name
    if (nFrameOptions & HtmlFrmOpts::Dir)
    {
        SvxFrameDirection nCurrentDirection = GetHTMLDirection(rItemSet);
        OString sDirection = convertDirection(nCurrentDirection);
        aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_dir, sDirection);
    }

    // alt
    if( (nFrameOptions & HtmlFrmOpts::Alt) && !rAlternateText.empty() && !bReplacement )
    {
        aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_alt, rAlternateText);
    }

    // align
    const char* pAlignString = nullptr;
    RndStdIds eAnchorId = rFrameFormat.GetAnchor().GetAnchorId();
    if( (nFrameOptions & HtmlFrmOpts::Align) &&
        ((RndStdIds::FLY_AT_PARA == eAnchorId) || (RndStdIds::FLY_AT_CHAR == eAnchorId)) && !bReplacement)
    {
        const SwFormatHoriOrient& rHoriOri = rFrameFormat.GetHoriOrient();
        if( !(nFrameOptions & HtmlFrmOpts::SAlign) ||
            text::RelOrientation::FRAME == rHoriOri.GetRelationOrient() ||
            text::RelOrientation::PRINT_AREA == rHoriOri.GetRelationOrient() )
        {
            pAlignString = text::HoriOrientation::RIGHT == rHoriOri.GetHoriOrient()
                        ? OOO_STRING_SVTOOLS_HTML_AL_right
                        : OOO_STRING_SVTOOLS_HTML_AL_left;
        }
    }
    const SwFormatVertOrient* pVertOrient;
    if( (nFrameOptions & HtmlFrmOpts::Align) && !pAlignString &&
        ( !(nFrameOptions & HtmlFrmOpts::SAlign) ||
          (RndStdIds::FLY_AS_CHAR == eAnchorId) ) &&
        (pVertOrient = rItemSet.GetItemIfSet( RES_VERT_ORIENT )) )
    {
        switch( pVertOrient->GetVertOrient() )
        {
        case text::VertOrientation::LINE_TOP:     pAlignString = OOO_STRING_SVTOOLS_HTML_VA_top;        break;
        case text::VertOrientation::CHAR_TOP:
        case text::VertOrientation::BOTTOM:       pAlignString = OOO_STRING_SVTOOLS_HTML_VA_texttop;    break;
        case text::VertOrientation::LINE_CENTER:
        case text::VertOrientation::CHAR_CENTER:  pAlignString = OOO_STRING_SVTOOLS_HTML_VA_absmiddle;  break;
        case text::VertOrientation::CENTER:       pAlignString = OOO_STRING_SVTOOLS_HTML_VA_middle;     break;
        case text::VertOrientation::LINE_BOTTOM:
        case text::VertOrientation::CHAR_BOTTOM:  pAlignString = OOO_STRING_SVTOOLS_HTML_VA_absbottom;  break;
        case text::VertOrientation::TOP:          pAlignString = OOO_STRING_SVTOOLS_HTML_VA_bottom;     break;
        case text::VertOrientation::NONE:     break;
        }
    }
    if (pAlignString && !bReplacement)
    {
        aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_align, pAlignString);
    }

    // hspace and vspace
    Size aTwipSpc( 0, 0 );
    const SvxLRSpaceItem* pLRSpaceItem;
    if( (nFrameOptions & (HtmlFrmOpts::Space | HtmlFrmOpts::MarginSize)) &&
        (pLRSpaceItem = rItemSet.GetItemIfSet( RES_LR_SPACE )) )
    {
        aTwipSpc.setWidth(
            ( pLRSpaceItem->GetLeft() + pLRSpaceItem->GetRight() ) / 2 );
        m_nDfltLeftMargin = m_nDfltRightMargin = aTwipSpc.Width();
    }
    const SvxULSpaceItem* pULSpaceItem;
    if( (nFrameOptions & (HtmlFrmOpts::Space|HtmlFrmOpts::MarginSize)) &&
        (pULSpaceItem = rItemSet.GetItemIfSet( RES_UL_SPACE )) )
    {
        aTwipSpc.setHeight(
            ( pULSpaceItem->GetUpper() + pULSpaceItem->GetLower() ) / 2 );
        m_nDfltTopMargin = m_nDfltBottomMargin = o3tl::narrowing<sal_uInt16>(aTwipSpc.Height());
    }

    if( (nFrameOptions & HtmlFrmOpts::Space) &&
        (aTwipSpc.Width() || aTwipSpc.Height()) &&
        Application::GetDefaultDevice() &&
        !mbReqIF )
    {
        Size aPixelSpc =
            Application::GetDefaultDevice()->LogicToPixel( aTwipSpc,
                                                MapMode(MapUnit::MapTwip) );
        if( !aPixelSpc.Width() && aTwipSpc.Width() )
            aPixelSpc.setWidth( 1 );
        if( !aPixelSpc.Height() && aTwipSpc.Height() )
            aPixelSpc.setHeight( 1 );

        if (aPixelSpc.Width())
        {
            aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_hspace, static_cast<sal_Int32>(aPixelSpc.Width()));
        }

        if (aPixelSpc.Height())
        {
            aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_vspace, static_cast<sal_Int32>(aPixelSpc.Height()));
        }
    }

    // The spacing must be considered for the size, if the corresponding flag
    // is set.
    if( nFrameOptions & HtmlFrmOpts::MarginSize )
    {
        aTwipSpc.setWidth( aTwipSpc.Width() * -2 );
        aTwipSpc.setHeight( aTwipSpc.Height() * -2 );
    }
    else
    {
        aTwipSpc.setWidth( 0 );
        aTwipSpc.setHeight( 0 );
    }

    const SvxBoxItem* pBoxItem;
    if( !(nFrameOptions & HtmlFrmOpts::AbsSize) &&
        (pBoxItem = rItemSet.GetItemIfSet( RES_BOX )) )
    {
        aTwipSpc.AdjustWidth(pBoxItem->CalcLineSpace( SvxBoxItemLine::LEFT ) );
        aTwipSpc.AdjustWidth(pBoxItem->CalcLineSpace( SvxBoxItemLine::RIGHT ) );
        aTwipSpc.AdjustHeight(pBoxItem->CalcLineSpace( SvxBoxItemLine::TOP ) );
        aTwipSpc.AdjustHeight(pBoxItem->CalcLineSpace( SvxBoxItemLine::BOTTOM ) );
    }

    // "width" and/or "height"
    // Only output SwFrameSize::Variable/SwFrameSize::Minimum if ANYSIZE is set
    std::optional<SwFormatFrameSize> aFrameSize;
    const SwFormatFrameSize* pFSItem = rItemSet.GetItemIfSet( RES_FRM_SIZE );
    const SdrObject* pObject;
    if (!pFSItem && (pObject = rFrameFormat.FindSdrObject()))
    {
        // Write size for Draw shapes as well.
        const tools::Rectangle& rSnapRect = pObject->GetSnapRect();
        aFrameSize.emplace();
        aFrameSize->SetWidthSizeType(SwFrameSize::Fixed);
        aFrameSize->SetWidth(rSnapRect.getOpenWidth());
        aFrameSize->SetHeightSizeType(SwFrameSize::Fixed);
        aFrameSize->SetHeight(rSnapRect.getOpenHeight());
        pFSItem = &*aFrameSize;
    }
    if( (nFrameOptions & HtmlFrmOpts::Size) &&
        pFSItem &&
        ( (nFrameOptions & HtmlFrmOpts::AnySize) ||
          SwFrameSize::Fixed == pFSItem->GetHeightSizeType()) )
    {
        sal_uInt8 nPercentWidth = pFSItem->GetWidthPercent();
        sal_uInt8 nPercentHeight = pFSItem->GetHeightPercent();

        // Size of the object in Twips without margins
        Size aTwipSz( (nPercentWidth && nPercentWidth != 255 ? 0
                                 : pFSItem->GetWidth()-aTwipSpc.Width()),
                      (nPercentHeight && nPercentHeight != 255 ? 0
                                  : pFSItem->GetHeight()-aTwipSpc.Height()) );

        OSL_ENSURE( aTwipSz.Width() >= 0 && aTwipSz.Height() >= 0, "Frame size minus spacing < 0!!!???" );
        if( aTwipSz.Width() < 0 )
            aTwipSz.setWidth( 0 );
        if( aTwipSz.Height() < 0 )
            aTwipSz.setHeight( 0 );

        Size aPixelSz( 0, 0 );
        if( (aTwipSz.Width() || aTwipSz.Height()) &&
            Application::GetDefaultDevice() )
        {
            aPixelSz =
                Application::GetDefaultDevice()->LogicToPixel( aTwipSz,
                                                    MapMode(MapUnit::MapTwip) );
            if( !aPixelSz.Width() && aTwipSz.Width() )
                aPixelSz.setWidth( 1 );
            if( !aPixelSz.Height() && aTwipSz.Height() )
                aPixelSz.setHeight( 1 );
        }

        if( (nFrameOptions & HtmlFrmOpts::Width) &&
            ((nPercentWidth && nPercentWidth!=255) || aPixelSz.Width()) )
        {
            OString sWidth;
            if (nPercentWidth)
            {
                if (nPercentWidth == 255)
                {
                    if (nPercentHeight)
                    {
                        sWidth = "auto";
                    }
                    else
                    {
                        sWidth = OString::number(static_cast<sal_Int32>(aPixelSz.Width()));
                    }
                }
                else
                {
                    sWidth = OString::number(static_cast<sal_Int32>(nPercentWidth)) + "%";
                }
            }
            else
                sWidth = OString::number(static_cast<sal_Int32>(aPixelSz.Width()));
            if (!mbXHTML || sWidth != "auto")
            {
                aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_width, sWidth);
            }
        }

        if( (nFrameOptions & HtmlFrmOpts::Height) &&
            ((nPercentHeight && nPercentHeight!=255) || aPixelSz.Height()) )
        {
            OString sHeight;
            if (nPercentHeight)
            {
                if (nPercentHeight == 255)
                {
                    if (nPercentWidth)
                    {
                        sHeight = "auto";
                    }
                    else
                    {
                        sHeight = OString::number(static_cast<sal_Int32>(aPixelSz.Height()));
                    }
                }
                else
                {
                    sHeight = OString::number(static_cast<sal_Int32>(nPercentHeight)) + "%";
                }
            }
            else
                sHeight = OString::number(static_cast<sal_Int32>(aPixelSz.Height()));
            if (!mbXHTML || sHeight != "auto")
            {
                aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_height, sHeight);
            }
        }
    }

    if (mbReqIF)
        return;

    // Insert wrap for graphics that are anchored to a paragraph as
    // <BR CLEAR=...> in the string

    if( !(nFrameOptions & HtmlFrmOpts::BrClear) )
        return;
    RndStdIds nAnchorId = rFrameFormat.GetAnchor().GetAnchorId();
    if (RndStdIds::FLY_AT_PARA != nAnchorId && RndStdIds::FLY_AT_CHAR != nAnchorId)
        return;
    const SwFormatSurround* pSurround = rItemSet.GetItemIfSet( RES_SURROUND );
    if (!pSurround)
        return;

    const char* pSurroundString = nullptr;

    sal_Int16 eHoriOri = rFrameFormat.GetHoriOrient().GetHoriOrient();
    css::text::WrapTextMode eSurround = pSurround->GetSurround();
    bool bAnchorOnly = pSurround->IsAnchorOnly();
    switch( eHoriOri )
    {
        case text::HoriOrientation::RIGHT:
        {
            switch( eSurround )
            {
            case css::text::WrapTextMode_NONE:
            case css::text::WrapTextMode_RIGHT:
                pSurroundString = OOO_STRING_SVTOOLS_HTML_AL_right;
                break;
            case css::text::WrapTextMode_LEFT:
            case css::text::WrapTextMode_PARALLEL:
                if( bAnchorOnly )
                    m_bClearRight = true;
                break;
            default:
                ;
            }
        }
        break;

        default:
        // If a frame is centered, it gets left aligned. This
        // should be taken into account here, too.
        {
            switch( eSurround )
            {
            case css::text::WrapTextMode_NONE:
            case css::text::WrapTextMode_LEFT:
                pSurroundString = OOO_STRING_SVTOOLS_HTML_AL_left;
                break;
            case css::text::WrapTextMode_RIGHT:
            case css::text::WrapTextMode_PARALLEL:
                if( bAnchorOnly )
                    m_bClearLeft = true;
                break;
            default:
                break;
            }
        }
        break;
    }

    if (pSurroundString)
    {
        aHtml.start(OOO_STRING_SVTOOLS_HTML_linebreak);
        aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_clear, pSurroundString);
        aHtml.end();
    }
}

namespace
{

OUString lclWriteOutImap(SwHTMLWriter& rHTMLWrt, const SfxItemSet& rItemSet, const SwFrameFormat& rFrameFormat,
                         const Size& rRealSize, const ImageMap* pAltImgMap, const SwFormatURL*& pURLItem)
{
    OUString aIMapName;

    // Only consider the URL attribute if no ImageMap was supplied

    // write ImageMap
    const ImageMap* pIMap = pAltImgMap;
    if( !pIMap  )
    {
        pURLItem = rItemSet.GetItemIfSet(RES_URL);
        if (pURLItem)
            pIMap = pURLItem->GetMap();
    }

    if (pIMap && !rHTMLWrt.mbReqIF)
    {
        // make the name unique
        aIMapName = pIMap->GetName();
        OUString aNameBase;
        if (!aIMapName.isEmpty())
            aNameBase = aIMapName;
        else
        {
            aNameBase = OOO_STRING_SVTOOLS_HTML_map;
            aIMapName = aNameBase + OUString::number(rHTMLWrt.m_nImgMapCnt);
        }

        bool bFound;
        do
        {
            bFound = false;
            for (const OUString & rImgMapName : rHTMLWrt.m_aImgMapNames)
            {
                // TODO: Unicode: Comparison is case insensitive for ASCII
                // characters only now!
                if (aIMapName.equalsIgnoreAsciiCase(rImgMapName))
                {
                    bFound = true;
                    break;
                }
            }
            if (bFound)
            {
                rHTMLWrt.m_nImgMapCnt++;
                aIMapName = aNameBase + OUString::number( rHTMLWrt.m_nImgMapCnt );
            }
        } while (bFound);

        bool bScale = false;
        Fraction aScaleX(1, 1);
        Fraction aScaleY(1, 1);

        const SwFormatFrameSize& rFrameSize = rFrameFormat.GetFrameSize();
        const SvxBoxItem& rBox = rFrameFormat.GetBox();

        if (!rFrameSize.GetWidthPercent() && rRealSize.Width())
        {
            SwTwips nWidth = rFrameSize.GetWidth();
            nWidth -= rBox.CalcLineSpace(SvxBoxItemLine::LEFT) + rBox.CalcLineSpace(SvxBoxItemLine::RIGHT);

            OSL_ENSURE( nWidth > 0, "Are there any graphics that are 0 twip wide!?" );
            if (nWidth <= 0) // should not happen
                nWidth = 1;

            if (rRealSize.Width() != nWidth)
            {
                aScaleX = Fraction(nWidth, rRealSize.Width());
                bScale = true;
            }
        }

        if (!rFrameSize.GetHeightPercent() && rRealSize.Height())
        {
            SwTwips nHeight = rFrameSize.GetHeight();

            nHeight -= rBox.CalcLineSpace(SvxBoxItemLine::TOP) + rBox.CalcLineSpace(SvxBoxItemLine::BOTTOM);

            OSL_ENSURE( nHeight > 0, "Are there any graphics that are 0 twip high!?" );
            if (nHeight <= 0)
                nHeight = 1;

            if (rRealSize.Height() != nHeight)
            {
                aScaleY = Fraction(nHeight, rRealSize.Height());
                bScale = true;
            }
        }

        rHTMLWrt.m_aImgMapNames.push_back(aIMapName);

        OString aIndMap, aIndArea;
        const char *pIndArea = nullptr, *pIndMap = nullptr;

        if (rHTMLWrt.IsLFPossible())
        {
            rHTMLWrt.OutNewLine( true );
            aIndMap = rHTMLWrt.GetIndentString();
            aIndArea = rHTMLWrt.GetIndentString(1);
            pIndArea = aIndArea.getStr();
            pIndMap = aIndMap.getStr();
        }

        if (bScale)
        {
            ImageMap aScaledIMap(*pIMap);
            aScaledIMap.Scale(aScaleX, aScaleY);
            HTMLOutFuncs::Out_ImageMap( rHTMLWrt.Strm(), rHTMLWrt.GetBaseURL(), aScaledIMap, aIMapName,
                                        aIMapEventTable,
                                        rHTMLWrt.m_bCfgStarBasic,
                                        SAL_NEWLINE_STRING, pIndArea, pIndMap );
        }
        else
        {
            HTMLOutFuncs::Out_ImageMap( rHTMLWrt.Strm(), rHTMLWrt.GetBaseURL(), *pIMap, aIMapName,
                                        aIMapEventTable,
                                        rHTMLWrt.m_bCfgStarBasic,
                                        SAL_NEWLINE_STRING, pIndArea, pIndMap );
        }
    }
    return aIMapName;
}

OUString getFrameFormatText(const SwFrameFormat& rFrameFormat)
{
    const SwFormatContent& rFlyContent = rFrameFormat.GetContent();
    const SwNodeIndex* pSttIx = rFlyContent.GetContentIdx();
    if (!pSttIx)
        return {};

    const SwNodeOffset nStt = pSttIx->GetIndex();
    const auto& nodes = rFrameFormat.GetDoc()->GetNodes();
    const SwNodeOffset nEnd = nodes[nStt]->EndOfSectionIndex();

    OUStringBuffer result;
    for (SwNodeOffset i = nStt + 1; i < nEnd; ++i)
    {
        if (const auto* pTextNd = nodes[i]->GetTextNode())
        {
            if (!result.isEmpty())
                result.append("\n");
            result.append(comphelper::string::encodeForXml(pTextNd->GetExpandText(
                nullptr, 0, -1, true, true, false,
                ExpandMode::ExpandFields | ExpandMode::HideInvisible | ExpandMode::HideDeletions
                    | ExpandMode::HideFieldmarkCommands)));
        }
    }

    return result.makeStringAndClear();
}

}

Writer& OutHTML_ImageStart( HtmlWriter& rHtml, Writer& rWrt, const SwFrameFormat &rFrameFormat,
                       const OUString& rGraphicURL,
                       Graphic const & rGraphic, const OUString& rAlternateText,
                       const Size &rRealSize, HtmlFrmOpts nFrameOpts,
                       const char *pMarkType,
                       const ImageMap *pAltImgMap,
                       const OUString& rMimeType )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    // <object data="..."> instead of <img src="...">
    bool bReplacement = (nFrameOpts & HtmlFrmOpts::Replacement) || rHTMLWrt.mbReqIF;

    if (rHTMLWrt.mbSkipImages)
        return rHTMLWrt;

    // if necessary, temporarily close an open attribute
    if( !rHTMLWrt.m_aINetFormats.empty() )
    {
        SwFormatINetFormat* pINetFormat = rHTMLWrt.m_aINetFormats.back();
        OutHTML_INetFormat( rWrt, *pINetFormat, false );
    }

    OUString aGraphicURL( rGraphicURL );
    if( !rHTMLWrt.mbEmbedImages && !HTMLOutFuncs::PrivateURLToInternalImg(aGraphicURL) && !rHTMLWrt.mpTempBaseURL )
        aGraphicURL = URIHelper::simpleNormalizedMakeRelative( rWrt.GetBaseURL(), aGraphicURL);

    const SfxItemSet& rItemSet = rFrameFormat.GetAttrSet();

    const SwFormatURL* pURLItem = nullptr;
    OUString aIMapName = lclWriteOutImap(rHTMLWrt, rItemSet, rFrameFormat, rRealSize, pAltImgMap, pURLItem);

    // put img into new line
    if (rHTMLWrt.IsLFPossible())
        rHTMLWrt.OutNewLine( true );

    // <a name=...></a>...<img ...>
    if( pMarkType && !rFrameFormat.GetName().isEmpty() )
    {
        rHTMLWrt.OutImplicitMark( rFrameFormat.GetName(), pMarkType );
    }

    // URL -> <a>...<img ... >...</a>
    const SvxMacroItem *pMacItem = rItemSet.GetItemIfSet(RES_FRMMACRO);

    if (pURLItem || pMacItem || (rHTMLWrt.mbReqIF && pAltImgMap))
    {
        OUString aMapURL;
        OUString aName;
        OUString aTarget;

        if(pURLItem)
        {
            aMapURL = pURLItem->GetURL();
            aName = pURLItem->GetName();
            aTarget = pURLItem->GetTargetFrameName();
        }
        else if (rHTMLWrt.mbReqIF && pAltImgMap)
        {
            // Get first non-empty map element
            for (size_t i = 0; i < pAltImgMap->GetIMapObjectCount(); ++i)
            {
                if (auto* pIMapObject = pAltImgMap->GetIMapObject(i))
                {
                    aMapURL = pIMapObject->GetURL();
                    aName = pIMapObject->GetName();
                    aTarget = pIMapObject->GetTarget();
                    if (!aMapURL.isEmpty() || !aName.isEmpty() || !aTarget.isEmpty())
                        break;
                }
            }
        }

        bool bEvents = pMacItem && !pMacItem->GetMacroTable().empty();

        if( !aMapURL.isEmpty() || !aName.isEmpty() || !aTarget.isEmpty() || bEvents )
        {
            rHtml.start(OOO_STRING_SVTOOLS_HTML_anchor);

            // Output "href" element if a link or macro exists
            if( !aMapURL.isEmpty() || bEvents )
            {
                rHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_href, OUStringToOString(rHTMLWrt.convertHyperlinkHRefValue(aMapURL), RTL_TEXTENCODING_UTF8));
            }

            if( !aName.isEmpty() )
            {
                rHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_name, OUStringToOString(aName, RTL_TEXTENCODING_UTF8));
            }

            if( !aTarget.isEmpty() )
            {
                rHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_target, OUStringToOString(aTarget, RTL_TEXTENCODING_UTF8));
            }

            if( pMacItem )
            {
                const SvxMacroTableDtor& rMacTable = pMacItem->GetMacroTable();
                if (!rMacTable.empty())
                {
                    HtmlWriterHelper::applyEvents(rHtml, rMacTable, aAnchorEventTable, rHTMLWrt.m_bCfgStarBasic);
                }
            }
        }
    }

    // <font color = ...>...<img ... >...</font>
    sal_uInt16 nBorderWidth = 0;
    const SvxBoxItem* pBoxItem;
    if( (nFrameOpts & HtmlFrmOpts::Border) &&
        (pBoxItem = rItemSet.GetItemIfSet( RES_BOX )) )
    {
        Size aTwipBorder( 0, 0 );
        const ::editeng::SvxBorderLine *pColBorderLine = nullptr;
        const ::editeng::SvxBorderLine *pBorderLine = pBoxItem->GetLeft();
        if( pBorderLine )
        {
            pColBorderLine = pBorderLine;
            aTwipBorder.AdjustWidth(pBorderLine->GetOutWidth() );
        }

        pBorderLine = pBoxItem->GetRight();
        if( pBorderLine )
        {
            pColBorderLine = pBorderLine;
            aTwipBorder.AdjustWidth(pBorderLine->GetOutWidth() );
        }

        pBorderLine = pBoxItem->GetTop();
        if( pBorderLine )
        {
            pColBorderLine = pBorderLine;
            aTwipBorder.AdjustHeight(pBorderLine->GetOutWidth() );
        }

        pBorderLine = pBoxItem->GetBottom();
        if( pBorderLine )
        {
            pColBorderLine = pBorderLine;
            aTwipBorder.AdjustHeight(pBorderLine->GetOutWidth() );
        }

        aTwipBorder.setWidth( aTwipBorder.Width() / 2 );
        aTwipBorder.setHeight( aTwipBorder.Height() / 2 );

        if( (aTwipBorder.Width() || aTwipBorder.Height()) &&
            Application::GetDefaultDevice() )
        {
            Size aPixelBorder =
                Application::GetDefaultDevice()->LogicToPixel( aTwipBorder,
                                                    MapMode(MapUnit::MapTwip) );
            if( !aPixelBorder.Width() && aTwipBorder.Width() )
                aPixelBorder.setWidth( 1 );
            if( !aPixelBorder.Height() && aTwipBorder.Height() )
                aPixelBorder.setHeight( 1 );

            if( aPixelBorder.Width() )
                aPixelBorder.setHeight( 0 );

            nBorderWidth =
                o3tl::narrowing<sal_uInt16>(aPixelBorder.Width() + aPixelBorder.Height());
        }

        if( pColBorderLine )
        {
            rHtml.start(OOO_STRING_SVTOOLS_HTML_font);
            HtmlWriterHelper::applyColor(rHtml, OOO_STRING_SVTOOLS_HTML_O_color, pColBorderLine->GetColor());
        }
    }

    OString aTag(OOO_STRING_SVTOOLS_HTML_image);
    if (bReplacement)
        // Write replacement graphic of OLE object as <object>.
        aTag = OOO_STRING_SVTOOLS_HTML_object;
    rHtml.start(aTag);

    OStringBuffer sBuffer;
    if(rHTMLWrt.mbEmbedImages)
    {
        OUString aGraphicInBase64;
        if (XOutBitmap::GraphicToBase64(rGraphic, aGraphicInBase64))
        {
            sBuffer.append(OOO_STRING_SVTOOLS_HTML_O_data);
            sBuffer.append(":");
            sBuffer.append(OUStringToOString(aGraphicInBase64, RTL_TEXTENCODING_UTF8));
            rHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_src, sBuffer.makeStringAndClear().getStr());
        }
        else
            rHTMLWrt.m_nWarn = WARN_SWG_POOR_LOAD;
    }
    else
    {
        sBuffer.append(OUStringToOString(aGraphicURL, RTL_TEXTENCODING_UTF8));
        OString aAttribute(OOO_STRING_SVTOOLS_HTML_O_src);
        if (bReplacement)
            aAttribute = OOO_STRING_SVTOOLS_HTML_O_data;
        rHtml.attribute(aAttribute, sBuffer.makeStringAndClear().getStr());
    }

    if (bReplacement)
    {
        // Handle XHTML type attribute for OLE replacement images.
        if (!rMimeType.isEmpty())
            rHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_type, rMimeType.toUtf8());
    }

    // Events
    if (const SvxMacroItem* pMacroItem = rItemSet.GetItemIfSet(RES_FRMMACRO))
    {
        const SvxMacroTableDtor& rMacTable = pMacroItem->GetMacroTable();
        if (!rMacTable.empty())
        {
            HtmlWriterHelper::applyEvents(rHtml, rMacTable, aImageEventTable, rHTMLWrt.m_bCfgStarBasic);
        }
    }

    // alt, align, width, height, hspace, vspace
    rHTMLWrt.writeFrameFormatOptions(rHtml, rFrameFormat, rAlternateText, nFrameOpts);
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) )
        rHTMLWrt.OutCSS1_FrameFormatOptions( rFrameFormat, nFrameOpts );

    if ((nFrameOpts & HtmlFrmOpts::Border) && !bReplacement)
    {
        rHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_border, nBorderWidth);
    }

    if( pURLItem && pURLItem->IsServerMap() )
    {
        rHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_ismap);
    }

    if( !aIMapName.isEmpty() )
    {
        rHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_usemap, Concat2View("#" + aIMapName));
    }

    if (bReplacement)
    {
        OUString aAltText = rAlternateText;
        // In ReqIF mode, output text from the frame instead
        if (rHTMLWrt.mbReqIF)
            if (OUString aFrameText = getFrameFormatText(rFrameFormat); !aFrameText.isEmpty())
                aAltText = aFrameText;

        // XHTML object replacement image's alternate text doesn't use the
        // "alt" attribute.
        if (aAltText.isEmpty())
            // Empty alternate text is not valid.
            rHtml.characters(" ");
        else
            rHtml.characters(aAltText.toUtf8());
    }

    return rHTMLWrt;
}

Writer& OutHTML_ImageEnd( HtmlWriter& rHtml, Writer& rWrt )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    rHtml.flushStack();

    if( !rHTMLWrt.m_aINetFormats.empty() )
    {
        // There is still an attribute on the stack that has to be reopened
        SwFormatINetFormat *pINetFormat = rHTMLWrt.m_aINetFormats.back();
        OutHTML_INetFormat( rWrt, *pINetFormat, true );
    }

    return rHTMLWrt;
}

Writer& OutHTML_BulletImage( Writer& rWrt,
                             const char *pTag,
                             const SvxBrushItem* pBrush,
                             const OUString &rGraphicURL)
{
    SwHTMLWriter & rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    OUString aGraphicInBase64;
    OUString aLink;
    if( pBrush )
    {
        aLink = pBrush->GetGraphicLink();
        if(rHTMLWrt.mbEmbedImages || aLink.isEmpty())
        {
            const Graphic* pGrf = pBrush->GetGraphic();
            if( pGrf )
            {
                if( !XOutBitmap::GraphicToBase64(*pGrf, aGraphicInBase64) )
                {
                    rHTMLWrt.m_nWarn = WARN_SWG_POOR_LOAD;
                }
            }
        }
        else if(!aLink.isEmpty())
        {
            if( rHTMLWrt.m_bCfgCpyLinkedGrfs )
            {
                rHTMLWrt.CopyLocalFileToINet( aLink );
            }

        }
    }
    else if(!rHTMLWrt.mbEmbedImages)
    {
        aLink = rGraphicURL;
    }
    if(!aLink.isEmpty())
    {
        if( !HTMLOutFuncs::PrivateURLToInternalImg(aLink) )
            aLink = URIHelper::simpleNormalizedMakeRelative( rWrt.GetBaseURL(), aLink);
    }

    OStringBuffer sOut;
    if( pTag )
        sOut.append('<').append(pTag);

    sOut.append(' ');
    sOut.append(OOO_STRING_SVTOOLS_HTML_O_style "=\"");
    if(!aLink.isEmpty())
    {
        sOut.append(OOO_STRING_SVTOOLS_HTML_O_src "=\"");
        rWrt.Strm().WriteOString( sOut );
        sOut.setLength(0);
        HTMLOutFuncs::Out_String( rWrt.Strm(), aLink );
    }
    else
    {
        sOut.append("list-style-image: url("
                OOO_STRING_SVTOOLS_HTML_O_data ":");
        rWrt.Strm().WriteOString( sOut );
        sOut.setLength(0);
        HTMLOutFuncs::Out_String( rWrt.Strm(), aGraphicInBase64 );
        sOut.append(");");
    }
    sOut.append('\"');

    if (pTag)
        sOut.append('>');
    rWrt.Strm().WriteOString( sOut );

    return rWrt;
}

static Writer& OutHTML_FrameFormatTableNode( Writer& rWrt, const SwFrameFormat& rFrameFormat )
{
    SwHTMLWriter & rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    const SwFormatContent& rFlyContent = rFrameFormat.GetContent();
    SwNodeOffset nStt = rFlyContent.GetContentIdx()->GetIndex()+1;
    SwNodeOffset nEnd = rHTMLWrt.m_pDoc->GetNodes()[nStt-1]->EndOfSectionIndex();

    OUString aCaption;
    bool bTopCaption = false;

    // Not const, because GetTable won't be const sometime later
    SwNode *pNd = rHTMLWrt.m_pDoc->GetNodes()[ nStt ];
    SwTableNode *pTableNd = pNd->GetTableNode();
    const SwTextNode *pTextNd = pNd->GetTextNode();
    if( !pTableNd && pTextNd )
    {
        // Table with heading
        bTopCaption = true;
        pTableNd = rHTMLWrt.m_pDoc->GetNodes()[nStt+1]->GetTableNode();
    }
    OSL_ENSURE( pTableNd, "Frame does not contain a table" );
    if( pTableNd )
    {
        SwNodeOffset nTableEnd = pTableNd->EndOfSectionIndex();
        OSL_ENSURE( nTableEnd == nEnd - 1 ||
                (nTableEnd == nEnd - 2 && !bTopCaption),
                "Invalid frame content for a table" );

        if( nTableEnd == nEnd - 2 )
            pTextNd = rHTMLWrt.m_pDoc->GetNodes()[nTableEnd+1]->GetTextNode();
    }
    if( pTextNd )
        aCaption = pTextNd->GetText();

    if( pTableNd )
    {
        HTMLSaveData aSaveData( rHTMLWrt, pTableNd->GetIndex()+1,
                                pTableNd->EndOfSectionIndex(),
                                   true, &rFrameFormat );
        rHTMLWrt.m_bOutFlyFrame = true;
        OutHTML_SwTableNode( rHTMLWrt, *pTableNd, &rFrameFormat, &aCaption,
                           bTopCaption );
    }

    return rWrt;
}

static Writer & OutHTML_FrameFormatAsMulticol( Writer& rWrt,
                                          const SwFrameFormat& rFrameFormat,
                                          bool bInCntnr )
{
    SwHTMLWriter & rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    rHTMLWrt.ChangeParaToken( HtmlTokenId::NONE );

    // Close the current <DL>!
    rHTMLWrt.OutAndSetDefList( 0 );

    // output as Multicol
    if (rHTMLWrt.IsLFPossible())
        rHTMLWrt.OutNewLine();

    OStringBuffer sOut;
    sOut.append('<').append(rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_multicol);

    const SwFormatCol& rFormatCol = rFrameFormat.GetCol();

    // output the number of columns as COLS
    sal_uInt16 nCols = rFormatCol.GetNumCols();
    if( nCols )
    {
        sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_cols
                "=\"" + OString::number(nCols) + "\"");
    }

    // the Gutter width (minimum value) as GUTTER
    sal_uInt16 nGutter = rFormatCol.GetGutterWidth( true );
    if( nGutter!=USHRT_MAX )
    {
        if( nGutter && Application::GetDefaultDevice() )
        {
            nGutter = o3tl::narrowing<sal_uInt16>(Application::GetDefaultDevice()
                            ->LogicToPixel( Size(nGutter,0),
                                            MapMode(MapUnit::MapTwip) ).Width());
        }
        sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_gutter
                "=\"" + OString::number(nGutter) + "\"");
    }

    rWrt.Strm().WriteOString( sOut );
    sOut.setLength(0);

    // WIDTH
    HtmlFrmOpts nFrameFlags = HTML_FRMOPTS_MULTICOL;
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) && !bInCntnr )
        nFrameFlags |= HTML_FRMOPTS_MULTICOL_CSS1;
    rHTMLWrt.OutFrameFormatOptions(rFrameFormat, OUString(), nFrameFlags);
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) && !bInCntnr )
        rHTMLWrt.OutCSS1_FrameFormatOptions( rFrameFormat, nFrameFlags );

    rWrt.Strm().WriteChar( '>' );

    rHTMLWrt.SetLFPossible(true);
    rHTMLWrt.IncIndentLevel();  // indent the content of Multicol

    const SwFormatContent& rFlyContent = rFrameFormat.GetContent();
    SwNodeOffset nStt = rFlyContent.GetContentIdx()->GetIndex();
    const SwStartNode* pSttNd = rWrt.m_pDoc->GetNodes()[nStt]->GetStartNode();
    OSL_ENSURE( pSttNd, "Where is the start node" );

    {
        // in a block, so that the old state can be restored in time
        // before the end
        HTMLSaveData aSaveData( rHTMLWrt, nStt+1,
                                pSttNd->EndOfSectionIndex(),
                                   true, &rFrameFormat );
        rHTMLWrt.m_bOutFlyFrame = true;
        rHTMLWrt.Out_SwDoc( rWrt.m_pCurrentPam.get() );
    }

    rHTMLWrt.DecIndentLevel();  // indent the content of Multicol;
    if (rHTMLWrt.IsLFPossible())
        rHTMLWrt.OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), Concat2View(rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_multicol), false );
    rHTMLWrt.SetLFPossible(true);

    return rWrt;
}

static Writer& OutHTML_FrameFormatAsSpacer( Writer& rWrt, const SwFrameFormat& rFrameFormat )
{
    SwHTMLWriter & rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    // if possible, output a line break before the graphic
    if (rHTMLWrt.IsLFPossible())
        rHTMLWrt.OutNewLine( true );

    OString sOut =
        "<" + rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_spacer " "
        OOO_STRING_SVTOOLS_HTML_O_type "=\""
        OOO_STRING_SVTOOLS_HTML_SPTYPE_block "\"";
    rWrt.Strm().WriteOString( sOut );

    // ALIGN, WIDTH, HEIGHT
    OString aEndTags = rHTMLWrt.OutFrameFormatOptions(rFrameFormat, OUString(), HTML_FRMOPTS_SPACER);

    rWrt.Strm().WriteChar( '>' );
    if( !aEndTags.isEmpty() )
        rWrt.Strm().WriteOString( aEndTags );

    return rWrt;
}

static Writer& OutHTML_FrameFormatAsDivOrSpan( Writer& rWrt,
                                          const SwFrameFormat& rFrameFormat, bool bSpan)
{
    SwHTMLWriter & rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    OString aTag;
    if( !bSpan )
    {
        rHTMLWrt.ChangeParaToken( HtmlTokenId::NONE );

        // Close the current <DL>!
        rHTMLWrt.OutAndSetDefList( 0 );
        aTag = OOO_STRING_SVTOOLS_HTML_division;
    }
    else
        aTag = OOO_STRING_SVTOOLS_HTML_span;

    // output as DIV
    if (rHTMLWrt.IsLFPossible())
        rHTMLWrt.OutNewLine();

    OStringBuffer sOut;
    sOut.append('<').append(rHTMLWrt.GetNamespace() + aTag);

    rWrt.Strm().WriteOString( sOut );
    sOut.setLength(0);
    HtmlFrmOpts nFrameFlags = HTML_FRMOPTS_DIV;
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_BORDER_NONE ) )
       nFrameFlags |= HtmlFrmOpts::SNoBorder;
    OString aEndTags = rHTMLWrt.OutFrameFormatOptions(rFrameFormat, OUString(), nFrameFlags);
    rHTMLWrt.OutCSS1_FrameFormatOptions( rFrameFormat, nFrameFlags );
    rWrt.Strm().WriteChar( '>' );

    rHTMLWrt.IncIndentLevel();  // indent the content
    rHTMLWrt.SetLFPossible(true);

    const SwFormatContent& rFlyContent = rFrameFormat.GetContent();
    SwNodeOffset nStt = rFlyContent.GetContentIdx()->GetIndex();

    // Output frame-anchored frames that are anchored to the start node
    rHTMLWrt.OutFlyFrame( nStt, 0, HtmlPosition::Any );

    const SwStartNode* pSttNd = rWrt.m_pDoc->GetNodes()[nStt]->GetStartNode();
    OSL_ENSURE( pSttNd, "Where is the start node" );

    {
        // in a block, so that the old state can be restored in time
        // before the end
        HTMLSaveData aSaveData( rHTMLWrt, nStt+1,
                                pSttNd->EndOfSectionIndex(),
                                   true, &rFrameFormat );
        rHTMLWrt.m_bOutFlyFrame = true;
        rHTMLWrt.Out_SwDoc( rWrt.m_pCurrentPam.get() );
    }

    rHTMLWrt.DecIndentLevel();  // indent the content of Multicol;
    if (rHTMLWrt.IsLFPossible())
        rHTMLWrt.OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), Concat2View(rHTMLWrt.GetNamespace() + aTag), false );

    if( !aEndTags.isEmpty() )
        rWrt.Strm().WriteOString( aEndTags );

    return rWrt;
}

/// Starts the OLE version of an image in the ReqIF + OLE case.
static void OutHTML_ImageOLEStart(SwHTMLWriter& rHTMLWrt, const Graphic& rGraphic,
                                  const SwFrameFormat& rFrameFormat)
{
    if (!rHTMLWrt.mbReqIF || !rHTMLWrt.m_bExportImagesAsOLE)
        return;

    // Write the original image as an RTF fragment.
    OUString aFileName;
    if (rHTMLWrt.GetOrigFileName())
        aFileName = *rHTMLWrt.GetOrigFileName();
    INetURLObject aURL(aFileName);
    OUString aName = aURL.getBase() + "_" + aURL.getExtension() + "_"
                     + OUString::number(rGraphic.GetChecksum(), 16);
    aURL.setBase(aName);
    aURL.setExtension(u"ole");
    aFileName = aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);

    SvFileStream aOutStream(aFileName, StreamMode::WRITE);
    if (!SwReqIfReader::WrapGraphicInRtf(rGraphic, rFrameFormat, aOutStream))
        SAL_WARN("sw.html", "SwReqIfReader::WrapGraphicInRtf() failed");

    // Refer to this data.
    aFileName = URIHelper::simpleNormalizedMakeRelative(rHTMLWrt.GetBaseURL(), aFileName);
    rHTMLWrt.Strm().WriteOString(
        Concat2View("<" + rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_object));
    rHTMLWrt.Strm().WriteOString(Concat2View(" data=\"" + aFileName.toUtf8() + "\""));
    rHTMLWrt.Strm().WriteOString(" type=\"text/rtf\"");
    rHTMLWrt.Strm().WriteOString(">");
    rHTMLWrt.OutNewLine();
}

/// Ends the OLE version of an image in the ReqIF + OLE case.
static void OutHTML_ImageOLEEnd(SwHTMLWriter& rHTMLWrt)
{
    if (rHTMLWrt.mbReqIF && rHTMLWrt.m_bExportImagesAsOLE)
    {
        rHTMLWrt.Strm().WriteOString(
            Concat2View("</" + rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_object ">"));
    }
}

static Writer & OutHTML_FrameFormatAsImage( Writer& rWrt, const SwFrameFormat& rFrameFormat, bool bPNGFallback)
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    bool bWritePNGFallback = rHTMLWrt.mbReqIF && !rHTMLWrt.m_bExportImagesAsOLE && bPNGFallback;

    if (rHTMLWrt.mbSkipImages)
        return rWrt;

    ImageMap aIMap;
    std::optional<Size> aDPI;
    if (rHTMLWrt.m_nShapeDPI.has_value())
    {
        aDPI.emplace(*rHTMLWrt.m_nShapeDPI, *rHTMLWrt.m_nShapeDPI);
    }
    Graphic aGraphic( const_cast<SwFrameFormat &>(rFrameFormat).MakeGraphic( &aIMap, /*nMaximumQuadraticPixels=*/2100000, aDPI ) );

    if (rHTMLWrt.mbReqIF)
    {
        // ImageMap doesn't seem to be allowed in reqif.
        if (auto pGrafObj = dynamic_cast<const SdrGrafObj*>(rFrameFormat.FindSdrObject()))
        {
            aGraphic = pGrafObj->GetGraphic();
        }
        else
        {
            // We only have a bitmap, write that as PNG without any fallback.
            bWritePNGFallback = false;
        }
    }

    Size aSz( 0, 0 );
    OUString GraphicURL;
    OUString aMimeType("image/jpeg");
    if(!rHTMLWrt.mbEmbedImages)
    {
        if( rHTMLWrt.GetOrigFileName() )
            GraphicURL = *rHTMLWrt.GetOrigFileName();

        OUString aFilterName("JPG");
        XOutFlags nFlags = XOutFlags::UseGifIfPossible | XOutFlags::UseNativeIfPossible;

        if (rHTMLWrt.mbReqIF && !bWritePNGFallback)
        {
            // Writing image without fallback PNG in ReqIF mode: force PNG output.
            aFilterName = "PNG";
            nFlags = XOutFlags::NONE;
            aMimeType = "image/png";
        }
        else if (rHTMLWrt.mbReqIF)
        {
            // Original format is wanted, don't force JPG.
            aFilterName.clear();
            aMimeType.clear();
        }

        if( aGraphic.GetType() == GraphicType::NONE ||
            XOutBitmap::WriteGraphic( aGraphic, GraphicURL,
                                      aFilterName,
                                      nFlags ) != ERRCODE_NONE )
        {
            // empty or incorrect, because there is nothing to output
            rHTMLWrt.m_nWarn = WARN_SWG_POOR_LOAD;
            return rWrt;
        }

        GraphicURL = URIHelper::SmartRel2Abs(
            INetURLObject(rWrt.GetBaseURL()), GraphicURL,
            URIHelper::GetMaybeFileHdl() );

    }
    uno::Reference<beans::XPropertySet> xGraphic(aGraphic.GetXGraphic(), uno::UNO_QUERY);
    if (xGraphic.is() && aMimeType.isEmpty())
        xGraphic->getPropertyValue("MimeType") >>= aMimeType;

    OutHTML_ImageOLEStart(rHTMLWrt, aGraphic, rFrameFormat);

    HtmlWriter aHtml(rWrt.Strm(), rHTMLWrt.maNamespace);
    OutHTML_ImageStart( aHtml, rWrt, rFrameFormat, GraphicURL, aGraphic, rFrameFormat.GetName(), aSz,
                    HtmlFrmOpts::GenImgMask, "frame",
                    aIMap.GetIMapObjectCount() ? &aIMap : nullptr, aMimeType );

    GfxLink aLink = aGraphic.GetGfxLink();
    if (bWritePNGFallback && aLink.GetType() != GfxLinkType::NativePng)
    {
        OutHTML_FrameFormatAsImage( rWrt, rFrameFormat, /*bPNGFallback=*/false);
    }

    OutHTML_ImageEnd(aHtml, rWrt);

    OutHTML_ImageOLEEnd(rHTMLWrt);

    return rWrt;
}

static Writer& OutHTML_FrameFormatGrfNode( Writer& rWrt, const SwFrameFormat& rFrameFormat,
                                      bool bInCntnr, bool bPNGFallback )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    bool bWritePNGFallback = rHTMLWrt.mbReqIF && !rHTMLWrt.m_bExportImagesAsOLE && bPNGFallback;

    if (rHTMLWrt.mbSkipImages)
        return rWrt;

    const SwFormatContent& rFlyContent = rFrameFormat.GetContent();
    SwNodeOffset nStt = rFlyContent.GetContentIdx()->GetIndex()+1;
    SwGrfNode *pGrfNd = rHTMLWrt.m_pDoc->GetNodes()[ nStt ]->GetGrfNode();
    OSL_ENSURE( pGrfNd, "Grf node expected" );
    if( !pGrfNd )
        return rWrt;

    HtmlFrmOpts nFrameFlags = bInCntnr ? HTML_FRMOPTS_IMG_CNTNR : HTML_FRMOPTS_IMG;
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) && !bInCntnr )
         nFrameFlags |= HTML_FRMOPTS_IMG_CSS1;

    Graphic aGraphic = pGrfNd->GetGraphic();

    if (aGraphic.GetType() == GraphicType::GdiMetafile)
    {
        // We only have a metafile, write that as PNG without any fallback.
        bWritePNGFallback = false;
    }

    OUString aGraphicURL;
    OUString aMimeType;
    if(!rHTMLWrt.mbEmbedImages)
    {
        const SwMirrorGrf& rMirror = pGrfNd->GetSwAttrSet().GetMirrorGrf();

        if( !pGrfNd->IsLinkedFile() || MirrorGraph::Dont != rMirror.GetValue() )
        {
            // create a (mirrored) jpeg file
            if( rHTMLWrt.GetOrigFileName() )
                aGraphicURL = *rHTMLWrt.GetOrigFileName();
            else
                aGraphicURL = rHTMLWrt.GetBaseURL();
            pGrfNd->GetGrf( true );

            XOutFlags nFlags = XOutFlags::UseGifIfSensible |
                           XOutFlags::UseNativeIfPossible;
            switch( rMirror.GetValue() )
            {
            case MirrorGraph::Vertical: nFlags = XOutFlags::MirrorHorz; break;
            case MirrorGraph::Horizontal:    nFlags = XOutFlags::MirrorVert; break;
            case MirrorGraph::Both:
                nFlags = XOutFlags::MirrorVert | XOutFlags::MirrorHorz;
                break;
            default: break;
            }

            const SwFormatFrameSize& rSize = rFrameFormat.GetFrameSize();
            Size aMM100Size = o3tl::convert( rSize.GetSize(),
                            o3tl::Length::twip, o3tl::Length::mm100 );

            OUString aFilterName;

            if (rHTMLWrt.mbReqIF)
            {
                // In ReqIF mode, do not try to write GIF for other image types
                nFlags &= ~XOutFlags::UseGifIfSensible;
                if (!bWritePNGFallback)
                {
                    // Writing image without fallback PNG in ReqIF mode: force PNG
                    // output.
                    // But don't force it when writing the original format and we'll write PNG inside
                    // that.
                    aFilterName = "PNG";
                    nFlags &= ~XOutFlags::UseNativeIfPossible;
                }
            }

            const Graphic& rGraphic = pGrfNd->GetGrf();

            // So that Graphic::IsTransparent() can report true.
            if (!rGraphic.isAvailable())
                const_cast<Graphic&>(rGraphic).makeAvailable();

            OUString aMimeTypeOverride;

            if (rHTMLWrt.mbReqIF && bWritePNGFallback)
            {
                // ReqIF: force native data if possible.
                const std::shared_ptr<VectorGraphicData>& pVectorGraphicData = rGraphic.getVectorGraphicData();
                if (pVectorGraphicData && pVectorGraphicData->getType() == VectorGraphicDataType::Svg)
                {
                    aFilterName = "svg";
                }
                else if (rGraphic.GetGfxLink().IsEMF())
                {
                    aFilterName = "emf";
                    aMimeTypeOverride = "image/x-emf"; // avoid image/x-wmf
                }
                else if (pVectorGraphicData && pVectorGraphicData->getType() == VectorGraphicDataType::Wmf)
                {
                    aFilterName = "wmf";
                }
                else if (rGraphic.GetGfxLink().GetType() == GfxLinkType::NativeTif)
                {
                    aFilterName = "tif";
                    aMimeTypeOverride = "image/tiff"; // avoid image/x-vclgraphic
                }
            }

            ErrCode nErr = XOutBitmap::WriteGraphic( rGraphic, aGraphicURL,
                    aFilterName, nFlags, &aMM100Size, nullptr, &aMimeType );
            if( nErr )
            {
                rHTMLWrt.m_nWarn = WARN_SWG_POOR_LOAD;
                return rWrt;
            }
            aGraphicURL = URIHelper::SmartRel2Abs(
                INetURLObject(rWrt.GetBaseURL()), aGraphicURL,
                URIHelper::GetMaybeFileHdl() );
            if (!aMimeTypeOverride.isEmpty())
                aMimeType = aMimeTypeOverride;
        }
        else
        {
            pGrfNd->GetFileFilterNms( &aGraphicURL, nullptr );
            if( rHTMLWrt.m_bCfgCpyLinkedGrfs )
                rWrt.CopyLocalFileToINet( aGraphicURL );
        }

    }
    uno::Reference<beans::XPropertySet> xGraphic(aGraphic.GetXGraphic(), uno::UNO_QUERY);
    if (xGraphic.is() && aMimeType.isEmpty())
        xGraphic->getPropertyValue("MimeType") >>= aMimeType;

    OutHTML_ImageOLEStart(rHTMLWrt, aGraphic, rFrameFormat);

    HtmlWriter aHtml(rWrt.Strm(), rHTMLWrt.maNamespace);
    OutHTML_ImageStart( aHtml, rWrt, rFrameFormat, aGraphicURL, aGraphic, pGrfNd->GetTitle(),
                  pGrfNd->GetTwipSize(), nFrameFlags, "graphic", nullptr, aMimeType );

    GfxLink aLink = aGraphic.GetGfxLink();
    if (bWritePNGFallback && aLink.GetType() != GfxLinkType::NativePng)
    {
        // Not OLE mode, outer format is not PNG: write inner PNG.
        OutHTML_FrameFormatGrfNode( rWrt, rFrameFormat,
                                      bInCntnr, /*bPNGFallback=*/false );
    }

    OutHTML_ImageEnd(aHtml, rWrt);

    OutHTML_ImageOLEEnd(rHTMLWrt);

    return rWrt;
}

static Writer& OutHTML_FrameFormatAsMarquee( Writer& rWrt, const SwFrameFormat& rFrameFormat,
                                  const SdrObject& rSdrObj  )
{
    SwHTMLWriter & rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    // get the edit engine attributes of the object as SW attributes and
    // sort them as Hints
    const SfxItemSet& rFormatItemSet = rFrameFormat.GetAttrSet();
    SfxItemSetFixed<RES_CHRATR_BEGIN, RES_CHRATR_END> aItemSet( *rFormatItemSet.GetPool() );
    SwHTMLWriter::GetEEAttrsFromDrwObj( aItemSet, &rSdrObj );
    bool bCfgOutStylesOld = rHTMLWrt.m_bCfgOutStyles;
    rHTMLWrt.m_bCfgOutStyles = false;
    rHTMLWrt.m_bTextAttr = true;
    rHTMLWrt.m_bTagOn = true;
    Out_SfxItemSet( aHTMLAttrFnTab, rWrt, aItemSet, false );
    rHTMLWrt.m_bTextAttr = false;

    OutHTML_DrawFrameFormatAsMarquee( rHTMLWrt,
                                 static_cast<const SwDrawFrameFormat &>(rFrameFormat),
                                 rSdrObj );
    rHTMLWrt.m_bTextAttr = true;
    rHTMLWrt.m_bTagOn = false;
    Out_SfxItemSet( aHTMLAttrFnTab, rWrt, aItemSet, false );
    rHTMLWrt.m_bTextAttr = false;
    rHTMLWrt.m_bCfgOutStyles = bCfgOutStylesOld;

    return rWrt;
}

Writer& OutHTML_HeaderFooter( Writer& rWrt, const SwFrameFormat& rFrameFormat,
                              bool bHeader )
{
    SwHTMLWriter & rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    // output as Multicol
    rHTMLWrt.OutNewLine();
    OStringBuffer sOut;
    sOut.append(OOO_STRING_SVTOOLS_HTML_division " "
            OOO_STRING_SVTOOLS_HTML_O_title "=\"")
        .append( bHeader ? "header" : "footer" ).append("\"");
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), Concat2View(rHTMLWrt.GetNamespace() + sOut) );

    rHTMLWrt.IncIndentLevel();  // indent the content of Multicol;

    // Piece a spacer for the spacing together. Because the
    // <DL> or </DL> always produces a space between paragraphs, it is
    // subtracted if necessary.
    const SvxULSpaceItem& rULSpace = rFrameFormat.GetULSpace();
    sal_uInt16 nSize = bHeader ? rULSpace.GetLower() : rULSpace.GetUpper();
    rHTMLWrt.m_nHeaderFooterSpace = nSize;

    OString aSpacer;
    if( rHTMLWrt.IsHTMLMode(HTMLMODE_VERT_SPACER) &&
        nSize > HTML_PARSPACE && Application::GetDefaultDevice() )
    {
        nSize -= HTML_PARSPACE;
        nSize = static_cast<sal_Int16>(Application::GetDefaultDevice()
            ->LogicToPixel( Size(nSize,0), MapMode(MapUnit::MapTwip) ).Width());

        aSpacer = OOO_STRING_SVTOOLS_HTML_spacer
                " " OOO_STRING_SVTOOLS_HTML_O_type
                "=\"" OOO_STRING_SVTOOLS_HTML_SPTYPE_vertical "\""
                " " OOO_STRING_SVTOOLS_HTML_O_size
                "=\"" + OString::number(nSize) + "\"";
    }

    const SwFormatContent& rFlyContent = rFrameFormat.GetContent();
    SwNodeOffset nStt = rFlyContent.GetContentIdx()->GetIndex();
    const SwStartNode* pSttNd = rWrt.m_pDoc->GetNodes()[nStt]->GetStartNode();
    OSL_ENSURE( pSttNd, "Where is the start node" );

    if( !bHeader && !aSpacer.isEmpty() )
    {
        rHTMLWrt.OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), Concat2View(rHTMLWrt.GetNamespace() + aSpacer) );
    }

    {
        // in a block, so that the old state can be restored in time
        // before the end. pFlyFormat doesn't need to be set here, because
        // PageDesc attributes cannot occur here
        HTMLSaveData aSaveData( rHTMLWrt, nStt+1,
                                pSttNd->EndOfSectionIndex() );

        if( bHeader )
            rHTMLWrt.m_bOutHeader = true;
        else
            rHTMLWrt.m_bOutFooter = true;

        rHTMLWrt.Out_SwDoc( rWrt.m_pCurrentPam.get() );
    }

    if( bHeader && !aSpacer.isEmpty() )
    {
        rHTMLWrt.OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), Concat2View(rHTMLWrt.GetNamespace() + aSpacer) );
    }

    rHTMLWrt.DecIndentLevel();  // indent the content of Multicol;
    rHTMLWrt.OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), Concat2View(rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_division), false );

    rHTMLWrt.m_nHeaderFooterSpace = 0;

    return rWrt;
}

void SwHTMLWriter::AddLinkTarget( std::u16string_view aURL )
{
    if( aURL.empty() || aURL[0] != '#' )
        return;

    // There might be a '|' as delimiter (if the link has been inserted
    // freshly) or a '%7c' or a '%7C' if the document has been saved and
    // loaded already.
    sal_Int32 nPos = aURL.size();
    bool bFound = false, bEncoded = false;
    while( !bFound && nPos > 0 )
    {
        sal_Unicode c = aURL[ --nPos ];
        switch( c )
        {
        case cMarkSeparator:
            bFound = true;
            break;
        case '%':
            bFound = (aURL.size() - nPos) >=3 && aURL[ nPos+1 ] == '7';
            if(bFound)
            {
                c = aURL[ nPos+2 ];
                bFound = (c == 'C' || c == 'c');
            }
            if( bFound )
                bEncoded = true;
        }
    }
    if( !bFound || nPos < 2 ) // at least "#a|..."
        return;

    aURL = aURL.substr( 1 );

    // nPos-1+1/3 (-1 because of Erase)
    OUString sCmp = OUString(aURL.substr(bEncoded ? nPos+2 : nPos)).replaceAll(" ","");
    if( sCmp.isEmpty() )
        return;

    sCmp = sCmp.toAsciiLowerCase();

    if( sCmp == "region" ||
        sCmp == "frame" ||
        sCmp == "graphic" ||
        sCmp == "ole" ||
        sCmp == "table" )
    {
        // Just remember it in a sorted array
        OUString aURL2(aURL);
        if( bEncoded )
        {
            aURL2 = aURL2.replaceAt( nPos - 1, 3, rtl::OUStringChar(cMarkSeparator)  );
        }
        m_aImplicitMarks.insert( aURL2 );
    }
    else if( sCmp == "outline" )
    {
        // Here, we need position and name. That's why we sort a
        // sal_uInt16 and a string array ourselves.
        OUString aOutline( aURL.substr( 0, nPos-1 ) );
        SwPosition aPos( *m_pCurrentPam->GetPoint() );
        if( m_pDoc->GotoOutline( aPos, aOutline ) )
        {
            SwNodeOffset nIdx = aPos.GetNodeIndex();

            decltype(m_aOutlineMarkPoss)::size_type nIns=0;
            while( nIns < m_aOutlineMarkPoss.size() &&
                   m_aOutlineMarkPoss[nIns] < nIdx )
                nIns++;

            m_aOutlineMarkPoss.insert( m_aOutlineMarkPoss.begin()+nIns, nIdx );
            OUString aURL2(aURL);
            if( bEncoded )
            {
                aURL2 = aURL2.replaceAt( nPos - 1, 3, rtl::OUStringChar(cMarkSeparator) );
            }
            m_aOutlineMarks.insert( m_aOutlineMarks.begin()+nIns, aURL2 );
        }
    }
}

void SwHTMLWriter::CollectLinkTargets()
{
    const SwTextINetFormat* pTextAttr;

    for (const SfxPoolItem* pItem : m_pDoc->GetAttrPool().GetItemSurrogates(RES_TXTATR_INETFMT))
    {
        auto pINetFormat = dynamic_cast<const SwFormatINetFormat*>(pItem);
        const SwTextNode* pTextNd;

        if( pINetFormat &&
            nullptr != ( pTextAttr = pINetFormat->GetTextINetFormat()) &&
            nullptr != ( pTextNd = pTextAttr->GetpTextNode() ) &&
            pTextNd->GetNodes().IsDocNodes() )
        {
            AddLinkTarget( pINetFormat->GetValue() );
        }
    }

    for (const SfxPoolItem* pItem : m_pDoc->GetAttrPool().GetItemSurrogates(RES_URL))
    {
        auto pURL = dynamic_cast<const SwFormatURL*>(pItem);
        if( pURL )
        {
            AddLinkTarget( pURL->GetURL() );
            const ImageMap *pIMap = pURL->GetMap();
            if( pIMap )
            {
                for( size_t i=0; i<pIMap->GetIMapObjectCount(); ++i )
                {
                    const IMapObject* pObj = pIMap->GetIMapObject( i );
                    if( pObj )
                    {
                        AddLinkTarget( pObj->GetURL() );
                    }
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
