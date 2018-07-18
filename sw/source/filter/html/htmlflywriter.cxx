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
#include <svx/svxids.hrc>
#include <hintids.hxx>
#include <tools/fract.hxx>
#include <svl/urihelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <sfx2/event.hxx>
#include <svtools/htmlkywd.hxx>
#include <svtools/htmlout.hxx>
#include <svtools/htmltokn.h>
#include <svtools/imap.hxx>
#include <svtools/imapobj.hxx>
#include <svtools/htmlcfg.hxx>
#include <svx/svdouno.hxx>
#include <svx/xoutbmp.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/brushitem.hxx>

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
#include <docary.hxx>
#include <ndgrf.hxx>

#include <doc.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <swerror.h>
#include <frmfmt.hxx>
#include "wrthtml.hxx"
#include "css1kywd.hxx"
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
const HtmlFrmOpts HTML_FRMOPTS_MULTICOL_CNTNR =
    HTML_FRMOPTS_MULTICOL;
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
static Writer& OutHTML_FrameFormatAsImage( Writer& rWrt, const SwFrameFormat& rFormat );

static Writer& OutHTML_FrameFormatGrfNode( Writer& rWrt, const SwFrameFormat& rFormat,
                                      bool bInCntnr );

static Writer& OutHTML_FrameFormatAsMarquee( Writer& rWrt, const SwFrameFormat& rFrameFormat,
                                        const SdrObject& rSdrObj    );

static HTMLOutEvent aImageEventTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_O_SDonload,       OOO_STRING_SVTOOLS_HTML_O_onload,   SvMacroItemId::OnImageLoadDone        },
    { OOO_STRING_SVTOOLS_HTML_O_SDonabort,      OOO_STRING_SVTOOLS_HTML_O_onabort,  SvMacroItemId::OnImageLoadCancel       },
    { OOO_STRING_SVTOOLS_HTML_O_SDonerror,      OOO_STRING_SVTOOLS_HTML_O_onerror,  SvMacroItemId::OnImageLoadError       },
    { nullptr, nullptr, SvMacroItemId::NONE }
};

static HTMLOutEvent aIMapEventTable[] =
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
        sal_uLong nStt = rFlyContent.GetContentIdx()->GetIndex()+1;
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
            sal_uLong nEnd = m_pDoc->GetNodes()[nStt-1]->EndOfSectionIndex();

            const SfxPoolItem* pItem;
            const SfxItemSet& rItemSet = rFrameFormat.GetAttrSet();
            if( SfxItemState::SET == rItemSet.GetItemState( RES_COL,
                                                       true, &pItem ) &&
                static_cast<const SwFormatCol *>(pItem)->GetNumCols() > 1 )
            {
                // frame with columns
                eType = HTML_FRMTYPE_MULTICOL;
            }
            else if( pNd->IsTableNode() )
            {
                const SwTableNode *pTableNd = pNd->GetTableNode();
                sal_uLong nTableEnd = pTableNd->EndOfSectionIndex();

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
                    if( m_pHTMLPosFlyFrames )
                    {
                        for( auto pHTMLPosFlyFrame : *m_pHTMLPosFlyFrames )
                        {
                            sal_uLong nIdx = pHTMLPosFlyFrame->GetNdIndex().GetIndex();
                            bEmpty = (nIdx != nStt) && (nIdx != nStt-1);
                            if( !bEmpty || nIdx > nStt )
                                break;
                        }
                    }
                }
                if( bEmpty )
                {
                    SvxBrushItem aBrush = rFrameFormat.makeBackgroundBrushItem();
                    /// background is not empty, if it has a background graphic
                    /// or its background color is not "no fill"/"auto fill".
                    if( GPOS_NONE != aBrush.GetGraphicPos() ||
                        aBrush.GetColor() != COL_TRANSPARENT )
                        bEmpty = false;
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

    SwPosFlyFrames aFlyPos(m_pDoc->GetAllFlyFormats(m_bWriteAll ? nullptr : m_pCurrentPam, true));

    for(SwPosFlyFrames::const_iterator aIter(aFlyPos.begin()); aIter != aFlyPos.end(); ++aIter)
    {
        const SwFrameFormat& rFrameFormat = (*aIter)->GetFormat();
        const SdrObject *pSdrObj = nullptr;
        const SwPosition *pAPos;
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
                (pAPos = rAnchor.GetContentAnchor()) != nullptr &&
                (pACNd = pAPos->nNode.GetNode().GetContentNode()) != nullptr )
            {
                const SvxLRSpaceItem& rLRItem =
                    static_cast<const SvxLRSpaceItem&>(pACNd->GetAttr(RES_LR_SPACE));
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

        if( !m_pHTMLPosFlyFrames )
            m_pHTMLPosFlyFrames = new SwHTMLPosFlyFrames;

        SwHTMLPosFlyFrame *pNew = new SwHTMLPosFlyFrame(**aIter, pSdrObj, nMode);
        m_pHTMLPosFlyFrames->insert( pNew );
    }
}

bool SwHTMLWriter::OutFlyFrame( sal_uLong nNdIdx, sal_Int32 nContentIdx, HtmlPosition nPos,
                              HTMLOutContext *pContext )
{
    bool bFlysLeft = false; // Are there still Flys left at the current node position?

    // OutFlyFrame can be called recursively. Thus, sometimes it is
    // necessary to start over after a Fly was returned.
    bool bRestart = true;
    while( m_pHTMLPosFlyFrames && bRestart )
    {
        bFlysLeft = bRestart = false;

        // search for the beginning of the FlyFrames
        size_t i {0};

        for( ; i < m_pHTMLPosFlyFrames->size() &&
            (*m_pHTMLPosFlyFrames)[i]->GetNdIndex().GetIndex() < nNdIdx; i++ )
            ;
        for( ; !bRestart && i < m_pHTMLPosFlyFrames->size() &&
            (*m_pHTMLPosFlyFrames)[i]->GetNdIndex().GetIndex() == nNdIdx; i++ )
        {
            SwHTMLPosFlyFrame *pPosFly = (*m_pHTMLPosFlyFrames)[i];
            if( ( HtmlPosition::Any == nPos ||
                  pPosFly->GetOutPos() == nPos ) &&
                pPosFly->GetContentIndex() == nContentIdx )
            {
                // It is important to remove it first, because additional
                // elements or the whole array could be deleted on
                // deeper recursion levels.
                m_pHTMLPosFlyFrames->erase(i);
                i--;
                if( m_pHTMLPosFlyFrames->empty() )
                {
                    delete m_pHTMLPosFlyFrames;
                    m_pHTMLPosFlyFrames = nullptr;
                    bRestart = true;    // not really, only exit the loop
                }

                if( pContext )
                {
                    HTMLOutFuncs::FlushToAscii(Strm(), *pContext );
                    pContext = nullptr; // one time only
                }

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
                delete pPosFly;
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

        if( m_bLFPossible && HtmlContainerFlags::Div == nCntnrMode )
            OutNewLine();

        OStringBuffer sOut;
        aContainerStr = (HtmlContainerFlags::Div == nCntnrMode)
                            ? OOO_STRING_SVTOOLS_HTML_division
                            : OOO_STRING_SVTOOLS_HTML_span;
        sOut.append('<').append(GetNamespace() + aContainerStr).append(' ')
            .append(OOO_STRING_SVTOOLS_HTML_O_class).append("=\"")
            .append("sd-abs-pos").append('\"');
        Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

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
            m_bLFPossible = true;
        }
    }

    switch( nOutMode )
    {
    case HtmlOut::TableNode:      // OK
        OSL_ENSURE( aContainerStr.isEmpty(), "Table: Container is not supposed to be here" );
        OutHTML_FrameFormatTableNode( *this, rFrameFormat );
        break;
    case HtmlOut::GraphicNode:      // OK
        OutHTML_FrameFormatGrfNode( *this, rFrameFormat, !aContainerStr.isEmpty() );
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
        OutHTML_FrameFormatAsImage( *this, rFrameFormat );
        break;
    }

    if( HtmlContainerFlags::Div == nCntnrMode )
    {
        DecIndentLevel();
        if( m_bLFPossible )
            OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), GetNamespace() + OOO_STRING_SVTOOLS_HTML_division, false );
        m_bLFPossible = true;
    }
    else if( HtmlContainerFlags::Span == nCntnrMode )
        HTMLOutFuncs::Out_AsciiTag( Strm(), GetNamespace() + OOO_STRING_SVTOOLS_HTML_span, false );
}

OString SwHTMLWriter::OutFrameFormatOptions( const SwFrameFormat &rFrameFormat,
                                     const OUString& rAlternateText,
                                     HtmlFrmOpts nFrameOpts )
{
    OString sRetEndTags;
    OStringBuffer sOut;
    const SfxPoolItem* pItem;
    const SfxItemSet& rItemSet = rFrameFormat.GetAttrSet();

    // Name
    if( (nFrameOpts & (HtmlFrmOpts::Id|HtmlFrmOpts::Name)) &&
        !rFrameFormat.GetName().isEmpty() )
    {
        const sal_Char *pStr =
            (nFrameOpts & HtmlFrmOpts::Id) ? OOO_STRING_SVTOOLS_HTML_O_id : OOO_STRING_SVTOOLS_HTML_O_name;
        sOut.append(' ').append(pStr).
            append("=\"");
        Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        HTMLOutFuncs::Out_String( Strm(), rFrameFormat.GetName(), m_eDestEnc, &m_aNonConvertableCharacters );
        sOut.append('\"');
    }

    // Name
    if( nFrameOpts & HtmlFrmOpts::Dir )
    {
        SvxFrameDirection nDir = GetHTMLDirection( rItemSet );
        Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        OutDirection( nDir );
    }

    // ALT
    if( (nFrameOpts & HtmlFrmOpts::Alt) && !rAlternateText.isEmpty() )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_alt).
            append("=\"");
        Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        HTMLOutFuncs::Out_String( Strm(), rAlternateText, m_eDestEnc, &m_aNonConvertableCharacters );
        sOut.append('\"');
    }

    // ALIGN
    const sal_Char *pStr = nullptr;
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
    if( (nFrameOpts & HtmlFrmOpts::Align) && !pStr &&
        ( !(nFrameOpts & HtmlFrmOpts::SAlign) ||
          (RndStdIds::FLY_AS_CHAR == eAnchorId) ) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_VERT_ORIENT, true, &pItem ))
    {
        switch( static_cast<const SwFormatVertOrient*>(pItem)->GetVertOrient() )
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
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_align).append("=\"").
            append(pStr).append("\"");
    }

    // HSPACE and VSPACE
    Size aTwipSpc( 0, 0 );
    if( (nFrameOpts & (HtmlFrmOpts::Space|HtmlFrmOpts::MarginSize)) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_LR_SPACE, true, &pItem ))
    {
        aTwipSpc.setWidth(
            ( static_cast<const SvxLRSpaceItem*>(pItem)->GetLeft() +
                static_cast<const SvxLRSpaceItem*>(pItem)->GetRight() ) / 2 );
        m_nDfltLeftMargin = m_nDfltRightMargin = aTwipSpc.Width();
    }
    if( (nFrameOpts & (HtmlFrmOpts::Space|HtmlFrmOpts::MarginSize)) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_UL_SPACE, true, &pItem ))
    {
        aTwipSpc.setHeight(
            ( static_cast<const SvxULSpaceItem*>(pItem)->GetUpper() +
                static_cast<const SvxULSpaceItem*>(pItem)->GetLower() ) / 2 );
        m_nDfltTopMargin = m_nDfltBottomMargin = static_cast<sal_uInt16>(aTwipSpc.Height());
    }

    if( (nFrameOpts & HtmlFrmOpts::Space) &&
        (aTwipSpc.Width() || aTwipSpc.Height()) &&
        Application::GetDefaultDevice() )
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
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_hspace).
                append("=\"").append(static_cast<sal_Int32>(aPixelSpc.Width())).append("\"");
        }

        if( aPixelSpc.Height() )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_vspace).
                append("=\"").append(static_cast<sal_Int32>(aPixelSpc.Height())).append("\"");
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

    if( !(nFrameOpts & HtmlFrmOpts::AbsSize) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_BOX, true, &pItem ))
    {
        const SvxBoxItem* pBoxItem = static_cast<const SvxBoxItem*>(pItem);

        aTwipSpc.AdjustWidth(pBoxItem->CalcLineSpace( SvxBoxItemLine::LEFT ) );
        aTwipSpc.AdjustWidth(pBoxItem->CalcLineSpace( SvxBoxItemLine::RIGHT ) );
        aTwipSpc.AdjustHeight(pBoxItem->CalcLineSpace( SvxBoxItemLine::TOP ) );
        aTwipSpc.AdjustHeight(pBoxItem->CalcLineSpace( SvxBoxItemLine::BOTTOM ) );
    }

    // WIDTH and/or HEIGHT
    // Output ATT_VAR_SIZE/ATT_MIN_SIZE only, if ANYSIZE is set
    if( (nFrameOpts & HtmlFrmOpts::Size) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_FRM_SIZE, true, &pItem ) &&
        ( (nFrameOpts & HtmlFrmOpts::AnySize) ||
          ATT_FIX_SIZE == static_cast<const SwFormatFrameSize *>(pItem)->GetHeightSizeType()) )
    {
        const SwFormatFrameSize *pFSItem = static_cast<const SwFormatFrameSize *>(pItem);
        sal_uInt8 nPrcWidth = pFSItem->GetWidthPercent();
        sal_uInt8 nPrcHeight = pFSItem->GetHeightPercent();

        // Size of the object in Twips without margins
        Size aTwipSz( (nPrcWidth ? 0
                                 : pFSItem->GetWidth()-aTwipSpc.Width()),
                      (nPrcHeight ? 0
                                  : pFSItem->GetHeight()-aTwipSpc.Height()) );

        OSL_ENSURE( aTwipSz.Width() >= 0 && aTwipSz.Height() >= 0,
                "Frame size minus spacing  < 0!!!???" );
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
            ((nPrcWidth && nPrcWidth!=255) || aPixelSz.Width()) )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_width).
                append("=\"");
            if( nPrcWidth )
                sOut.append(static_cast<sal_Int32>(nPrcWidth)).append('%');
            else
                sOut.append(static_cast<sal_Int32>(aPixelSz.Width()));
            sOut.append("\"");
        }

        if( (nFrameOpts & HtmlFrmOpts::Height) &&
            ((nPrcHeight && nPrcHeight!=255) || aPixelSz.Height()) )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_height).
                append("=\"");
            if( nPrcHeight )
                sOut.append(static_cast<sal_Int32>(nPrcHeight)).append('%');
            else
                sOut.append(static_cast<sal_Int32>(aPixelSz.Height()));
            sOut.append("\"");
        }
    }

    if (!sOut.isEmpty())
        Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

    // Insert wrap for graphics that are anchored to a paragraph as
    // <BR CLEAR=...> in the string
    if( (nFrameOpts & HtmlFrmOpts::BrClear) &&
        ((RndStdIds::FLY_AT_PARA == rFrameFormat.GetAnchor().GetAnchorId()) ||
         (RndStdIds::FLY_AT_CHAR == rFrameFormat.GetAnchor().GetAnchorId())) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_SURROUND, true, &pItem ))
    {
        const SwFormatSurround* pSurround = static_cast<const SwFormatSurround*>(pItem);
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
            sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_linebreak).
                append(' ').append(OOO_STRING_SVTOOLS_HTML_O_clear).
                append("=\"").append(pStr).append("\">");
            sRetEndTags = sOut.makeStringAndClear();
        }
    }
    return sRetEndTags;
}

void SwHTMLWriter::writeFrameFormatOptions(HtmlWriter& aHtml, const SwFrameFormat& rFrameFormat, const OUString& rAlternateText, HtmlFrmOpts nFrameOptions)
{
    bool bReplacement = (nFrameOptions & HtmlFrmOpts::Replacement) || mbReqIF;
    const SfxPoolItem* pItem;
    const SfxItemSet& rItemSet = rFrameFormat.GetAttrSet();

    // Name
    if( (nFrameOptions & (HtmlFrmOpts::Id|HtmlFrmOpts::Name)) &&
        !rFrameFormat.GetName().isEmpty() && !bReplacement)
    {
        const sal_Char* pAttributeName = (nFrameOptions & HtmlFrmOpts::Id) ? OOO_STRING_SVTOOLS_HTML_O_id : OOO_STRING_SVTOOLS_HTML_O_name;
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
    if( (nFrameOptions & HtmlFrmOpts::Alt) && !rAlternateText.isEmpty() && !bReplacement )
    {
        aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_alt, rAlternateText);
    }

    // align
    const sal_Char* pAlignString = nullptr;
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
    if( (nFrameOptions & HtmlFrmOpts::Align) && !pAlignString &&
        ( !(nFrameOptions & HtmlFrmOpts::SAlign) ||
          (RndStdIds::FLY_AS_CHAR == eAnchorId) ) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_VERT_ORIENT, true, &pItem ))
    {
        switch( static_cast<const SwFormatVertOrient*>(pItem)->GetVertOrient() )
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
    if( (nFrameOptions & (HtmlFrmOpts::Space | HtmlFrmOpts::MarginSize)) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_LR_SPACE, true, &pItem ))
    {
        aTwipSpc.setWidth(
            ( static_cast<const SvxLRSpaceItem*>(pItem)->GetLeft() +
                static_cast<const SvxLRSpaceItem*>(pItem)->GetRight() ) / 2 );
        m_nDfltLeftMargin = m_nDfltRightMargin = aTwipSpc.Width();
    }
    if( (nFrameOptions & (HtmlFrmOpts::Space|HtmlFrmOpts::MarginSize)) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_UL_SPACE, true, &pItem ))
    {
        aTwipSpc.setHeight(
            ( static_cast<const SvxULSpaceItem*>(pItem)->GetUpper() +
                static_cast<const SvxULSpaceItem*>(pItem)->GetLower() ) / 2 );
        m_nDfltTopMargin = m_nDfltBottomMargin = static_cast<sal_uInt16>(aTwipSpc.Height());
    }

    if( (nFrameOptions & HtmlFrmOpts::Space) &&
        (aTwipSpc.Width() || aTwipSpc.Height()) &&
        Application::GetDefaultDevice() )
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

    if( !(nFrameOptions & HtmlFrmOpts::AbsSize) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_BOX, true, &pItem ))
    {
        const SvxBoxItem* pBoxItem = static_cast<const SvxBoxItem*>(pItem);

        aTwipSpc.AdjustWidth(pBoxItem->CalcLineSpace( SvxBoxItemLine::LEFT ) );
        aTwipSpc.AdjustWidth(pBoxItem->CalcLineSpace( SvxBoxItemLine::RIGHT ) );
        aTwipSpc.AdjustHeight(pBoxItem->CalcLineSpace( SvxBoxItemLine::TOP ) );
        aTwipSpc.AdjustHeight(pBoxItem->CalcLineSpace( SvxBoxItemLine::BOTTOM ) );
    }

    // "width" and/or "height"
    // Only output ATT_VAR_SIZE/ATT_MIN_SIZE if ANYSIZE is set
    if( (nFrameOptions & HtmlFrmOpts::Size) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_FRM_SIZE, true, &pItem ) &&
        ( (nFrameOptions & HtmlFrmOpts::AnySize) ||
          ATT_FIX_SIZE == static_cast<const SwFormatFrameSize *>(pItem)->GetHeightSizeType()) )
    {
        const SwFormatFrameSize *pFSItem = static_cast<const SwFormatFrameSize *>(pItem);
        sal_uInt8 nPrcWidth = pFSItem->GetWidthPercent();
        sal_uInt8 nPrcHeight = pFSItem->GetHeightPercent();

        // Size of the object in Twips without margins
        Size aTwipSz( (nPrcWidth ? 0
                                 : pFSItem->GetWidth()-aTwipSpc.Width()),
                      (nPrcHeight ? 0
                                  : pFSItem->GetHeight()-aTwipSpc.Height()) );

        OSL_ENSURE( aTwipSz.Width() >= 0 && aTwipSz.Height() >= 0,
                "Frame size minus spacing < 0!!!???" );
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
            ((nPrcWidth && nPrcWidth!=255) || aPixelSz.Width()) )
        {
            OString sWidth;
            if (nPrcWidth)
                sWidth = OString::number(static_cast<sal_Int32>(nPrcWidth)) + "%";
            else
                sWidth = OString::number(static_cast<sal_Int32>(aPixelSz.Width()));
            aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_width, sWidth);
        }

        if( (nFrameOptions & HtmlFrmOpts::Height) &&
            ((nPrcHeight && nPrcHeight!=255) || aPixelSz.Height()) )
        {
            OString sHeight;
            if (nPrcHeight)
                sHeight = OString::number(static_cast<sal_Int32>(nPrcHeight)) + "%";
            else
                sHeight = OString::number(static_cast<sal_Int32>(aPixelSz.Height()));
            aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_height, sHeight);
        }
    }

    // Insert wrap for graphics that are anchored to a paragraph as
    // <BR CLEAR=...> in the string

    if( (nFrameOptions & HtmlFrmOpts::BrClear) &&
        ((RndStdIds::FLY_AT_PARA == rFrameFormat.GetAnchor().GetAnchorId()) ||
         (RndStdIds::FLY_AT_CHAR == rFrameFormat.GetAnchor().GetAnchorId())) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_SURROUND, true, &pItem ))
    {
        const sal_Char* pSurroundString = nullptr;

        const SwFormatSurround* pSurround = static_cast<const SwFormatSurround*>(pItem);
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
}

namespace
{

OUString lclWriteOutImap(SwHTMLWriter& rHTMLWrt, const SfxItemSet& rItemSet, const SwFrameFormat& rFrameFormat,
                         const Size& rRealSize, const ImageMap* pAltImgMap, const SwFormatURL*& pURLItem)
{
    OUString aIMapName;

    const SfxPoolItem* pItem;

    // Only consider the URL attribute if no ImageMap was supplied
    if (!pAltImgMap && SfxItemState::SET == rItemSet.GetItemState( RES_URL, true, &pItem))
    {
        pURLItem = static_cast<const SwFormatURL*>( pItem);
    }

    // write ImageMap
    const ImageMap* pIMap = pAltImgMap;
    if( !pIMap && pURLItem )
    {
        pIMap = pURLItem->GetMap();
    }

    if (pIMap)
    {
        // make the name unique
        aIMapName = pIMap->GetName();
        OUString aNameBase;
        if (!aIMapName.isEmpty())
            aNameBase = aIMapName;
        else
            aNameBase = OOO_STRING_SVTOOLS_HTML_map;

        if (aIMapName.isEmpty())
            aIMapName = aNameBase + OUString::number(rHTMLWrt.m_nImgMapCnt);

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
        const sal_Char *pIndArea = nullptr, *pIndMap = nullptr;

        if (rHTMLWrt.m_bLFPossible)
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
                                        SAL_NEWLINE_STRING, pIndArea, pIndMap,
                                        rHTMLWrt.m_eDestEnc,
                                        &rHTMLWrt.m_aNonConvertableCharacters );
        }
        else
        {
            HTMLOutFuncs::Out_ImageMap( rHTMLWrt.Strm(), rHTMLWrt.GetBaseURL(), *pIMap, aIMapName,
                                        aIMapEventTable,
                                        rHTMLWrt.m_bCfgStarBasic,
                                        SAL_NEWLINE_STRING, pIndArea, pIndMap,
                                         rHTMLWrt.m_eDestEnc,
                                        &rHTMLWrt.m_aNonConvertableCharacters );
        }
    }
    return aIMapName;
}

}

Writer& OutHTML_Image( Writer& rWrt, const SwFrameFormat &rFrameFormat,
                       const OUString& rGraphicURL,
                       Graphic const & rGraphic, const OUString& rAlternateText,
                       const Size &rRealSize, HtmlFrmOpts nFrameOpts,
                       const sal_Char *pMarkType,
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

    const SfxPoolItem* pItem;
    const SfxItemSet& rItemSet = rFrameFormat.GetAttrSet();

    const SwFormatURL* pURLItem = nullptr;
    OUString aIMapName = lclWriteOutImap(rHTMLWrt, rItemSet, rFrameFormat, rRealSize, pAltImgMap, pURLItem);

    // put img into new line
    if( rHTMLWrt.m_bLFPossible )
        rHTMLWrt.OutNewLine( true );

    HtmlWriter aHtml(rWrt.Strm(), rHTMLWrt.maNamespace);

    // <a name=...></a>...<img ...>
    if( pMarkType && !rFrameFormat.GetName().isEmpty() )
    {
        rHTMLWrt.OutImplicitMark( rFrameFormat.GetName(), pMarkType );
    }

    // URL -> <a>...<img ... >...</a>
    const SvxMacroItem *pMacItem = nullptr;
    if (SfxItemState::SET == rItemSet.GetItemState(RES_FRMMACRO, true, &pItem))
    {
        pMacItem = static_cast<const SvxMacroItem *>(pItem);
    }

    if (pURLItem || pMacItem)
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

        bool bEvents = pMacItem && !pMacItem->GetMacroTable().empty();

        if( !aMapURL.isEmpty() || !aName.isEmpty() || !aTarget.isEmpty() || bEvents )
        {
            aHtml.start(OOO_STRING_SVTOOLS_HTML_anchor);

            // Output "href" element if a link or macro exists
            if( !aMapURL.isEmpty() || bEvents )
            {
                aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_href, OUStringToOString(rHTMLWrt.convertHyperlinkHRefValue(aMapURL), RTL_TEXTENCODING_UTF8));
            }

            if( !aName.isEmpty() )
            {
                aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_name, OUStringToOString(aName, RTL_TEXTENCODING_UTF8));
            }

            if( !aTarget.isEmpty() )
            {
                aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_target, OUStringToOString(aTarget, RTL_TEXTENCODING_UTF8));
            }

            if( pMacItem )
            {
                const SvxMacroTableDtor& rMacTable = pMacItem->GetMacroTable();
                if (!rMacTable.empty())
                {
                    HtmlWriterHelper::applyEvents(aHtml, rMacTable, aAnchorEventTable, rHTMLWrt.m_bCfgStarBasic);
                }
            }
        }
    }

    // <font color = ...>...<img ... >...</font>
    sal_uInt16 nBorderWidth = 0;
    if( (nFrameOpts & HtmlFrmOpts::Border) &&
        SfxItemState::SET == rItemSet.GetItemState( RES_BOX, true, &pItem ))
    {
        Size aTwipBorder( 0, 0 );
        const SvxBoxItem* pBoxItem = static_cast<const SvxBoxItem*>(pItem);

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
                static_cast<sal_uInt16>(aPixelBorder.Width() + aPixelBorder.Height());
        }

        if( pColBorderLine )
        {
            aHtml.start(OOO_STRING_SVTOOLS_HTML_font);
            HtmlWriterHelper::applyColor(aHtml, OOO_STRING_SVTOOLS_HTML_O_color, pColBorderLine->GetColor());
        }
    }

    OString aTag(OOO_STRING_SVTOOLS_HTML_image);
    if (bReplacement)
        // Write replacement graphic of OLE object as <object>.
        aTag = OOO_STRING_SVTOOLS_HTML_object;
    aHtml.start(aTag);

    OStringBuffer sBuffer;
    if(rHTMLWrt.mbEmbedImages)
    {
        OUString aGraphicInBase64;
        if (XOutBitmap::GraphicToBase64(rGraphic, aGraphicInBase64))
        {
            sBuffer.append(OOO_STRING_SVTOOLS_HTML_O_data);
            sBuffer.append(":");
            sBuffer.append(OUStringToOString(aGraphicInBase64, RTL_TEXTENCODING_UTF8));
            aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_src, sBuffer.makeStringAndClear().getStr());
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
        aHtml.attribute(aAttribute, sBuffer.makeStringAndClear().getStr());
    }

    if (bReplacement)
    {
        // Handle XHTML type attribute for OLE replacement images.
        if (!rMimeType.isEmpty())
            aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_type, rMimeType.toUtf8());
    }

    // Events
    if (SfxItemState::SET == rItemSet.GetItemState(RES_FRMMACRO, true, &pItem))
    {
        const SvxMacroTableDtor& rMacTable = static_cast<const SvxMacroItem *>(pItem)->GetMacroTable();
        if (!rMacTable.empty())
        {
            HtmlWriterHelper::applyEvents(aHtml, rMacTable, aImageEventTable, rHTMLWrt.m_bCfgStarBasic);
        }
    }

    // alt, align, width, height, hspace, vspace
    rHTMLWrt.writeFrameFormatOptions(aHtml, rFrameFormat, rAlternateText, nFrameOpts);
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) )
        rHTMLWrt.OutCSS1_FrameFormatOptions( rFrameFormat, nFrameOpts );

    if ((nFrameOpts & HtmlFrmOpts::Border) && !bReplacement)
    {
        aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_border, nBorderWidth);
    }

    if( pURLItem && pURLItem->IsServerMap() )
    {
        aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_ismap);
    }

    if( !aIMapName.isEmpty() )
    {
        aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_usemap, "#" + aIMapName);
    }

    if (bReplacement)
    {
        // XHTML object replacement image's alternate text doesn't use the
        // "alt" attribute.
        if (rAlternateText.isEmpty())
            // Empty alternate text is not valid.
            aHtml.characters(" ");
        else
            aHtml.characters(rAlternateText.toUtf8());
    }

    aHtml.flushStack();

    if( !rHTMLWrt.m_aINetFormats.empty() )
    {
        // There is still an attribute on the stack that has to be reopened
        SwFormatINetFormat *pINetFormat = rHTMLWrt.m_aINetFormats.back();
        OutHTML_INetFormat( rWrt, *pINetFormat, true );
    }

    return rHTMLWrt;
}

Writer& OutHTML_BulletImage( Writer& rWrt,
                             const sal_Char *pTag,
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
    sOut.append(OOO_STRING_SVTOOLS_HTML_O_style).append("=\"");
    if(!aLink.isEmpty())
    {
        sOut.append(OOO_STRING_SVTOOLS_HTML_O_src).append("=\"");
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        HTMLOutFuncs::Out_String( rWrt.Strm(), aLink, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
    }
    else
    {
        sOut.append("list-style-image: ").append("url(").
        append(OOO_STRING_SVTOOLS_HTML_O_data).append(":");
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        HTMLOutFuncs::Out_String( rWrt.Strm(), aGraphicInBase64, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
        sOut.append(");");
    }
    sOut.append('\"');

    if (pTag)
        sOut.append('>');
    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

    return rWrt;
}

static Writer& OutHTML_FrameFormatTableNode( Writer& rWrt, const SwFrameFormat& rFrameFormat )
{
    SwHTMLWriter & rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    const SwFormatContent& rFlyContent = rFrameFormat.GetContent();
    sal_uLong nStt = rFlyContent.GetContentIdx()->GetIndex()+1;
    sal_uLong nEnd = rHTMLWrt.m_pDoc->GetNodes()[nStt-1]->EndOfSectionIndex();

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
        sal_uLong nTableEnd = pTableNd->EndOfSectionIndex();
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
    if( rHTMLWrt.m_bLFPossible )
        rHTMLWrt.OutNewLine();

    OStringBuffer sOut;
    sOut.append('<').append(rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_multicol);

    const SwFormatCol& rFormatCol = rFrameFormat.GetCol();

    // output the number of columns as COLS
    sal_uInt16 nCols = rFormatCol.GetNumCols();
    if( nCols )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_cols).
            append("=\"").append(static_cast<sal_Int32>(nCols)).append("\"");
    }

    // the Gutter width (minimum value) as GUTTER
    sal_uInt16 nGutter = rFormatCol.GetGutterWidth( true );
    if( nGutter!=USHRT_MAX )
    {
        if( nGutter && Application::GetDefaultDevice() )
        {
            nGutter = static_cast<sal_uInt16>(Application::GetDefaultDevice()
                            ->LogicToPixel( Size(nGutter,0),
                                            MapMode(MapUnit::MapTwip) ).Width());
        }
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_gutter).
            append("=\"").append(static_cast<sal_Int32>(nGutter)).append("\"");
    }

    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

    // WIDTH
    HtmlFrmOpts nFrameFlags = bInCntnr ? HTML_FRMOPTS_MULTICOL_CNTNR
                                : HTML_FRMOPTS_MULTICOL;
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) && !bInCntnr )
        nFrameFlags |= HTML_FRMOPTS_MULTICOL_CSS1;
    rHTMLWrt.OutFrameFormatOptions( rFrameFormat, aEmptyOUStr, nFrameFlags );
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) && !bInCntnr )
        rHTMLWrt.OutCSS1_FrameFormatOptions( rFrameFormat, nFrameFlags );

    rWrt.Strm().WriteChar( '>' );

    rHTMLWrt.m_bLFPossible = true;
    rHTMLWrt.IncIndentLevel();  // indent the content of Multicol

    const SwFormatContent& rFlyContent = rFrameFormat.GetContent();
    sal_uLong nStt = rFlyContent.GetContentIdx()->GetIndex();
    const SwStartNode* pSttNd = rWrt.m_pDoc->GetNodes()[nStt]->GetStartNode();
    OSL_ENSURE( pSttNd, "Where is the start node" );

    {
        // in a block, so that the old state can be restored in time
        // before the end
        HTMLSaveData aSaveData( rHTMLWrt, nStt+1,
                                pSttNd->EndOfSectionIndex(),
                                   true, &rFrameFormat );
        rHTMLWrt.m_bOutFlyFrame = true;
        rHTMLWrt.Out_SwDoc( rWrt.m_pCurrentPam );
    }

    rHTMLWrt.DecIndentLevel();  // indent the content of Multicol;
    if( rHTMLWrt.m_bLFPossible )
        rHTMLWrt.OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_multicol, false );
    rHTMLWrt.m_bLFPossible = true;

    return rWrt;
}

static Writer& OutHTML_FrameFormatAsSpacer( Writer& rWrt, const SwFrameFormat& rFrameFormat )
{
    SwHTMLWriter & rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    // if possible, output a line break before the graphic
    if( rHTMLWrt.m_bLFPossible )
        rHTMLWrt.OutNewLine( true );

    OStringBuffer sOut;
    sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_spacer).append(' ')
        .append(OOO_STRING_SVTOOLS_HTML_O_type).append("=\"")
        .append(OOO_STRING_SVTOOLS_HTML_SPTYPE_block).append("\"");
    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

    // ALIGN, WIDTH, HEIGHT
    OString aEndTags = rHTMLWrt.OutFrameFormatOptions( rFrameFormat, aEmptyOUStr, HTML_FRMOPTS_SPACER );

    rWrt.Strm().WriteChar( '>' );
    if( !aEndTags.isEmpty() )
        rWrt.Strm().WriteCharPtr( aEndTags.getStr() );

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
    if( rHTMLWrt.m_bLFPossible )
        rHTMLWrt.OutNewLine();

    OStringBuffer sOut;
    sOut.append('<').append(rHTMLWrt.GetNamespace() + aTag);

    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
    HtmlFrmOpts nFrameFlags = HTML_FRMOPTS_DIV;
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_BORDER_NONE ) )
       nFrameFlags |= HtmlFrmOpts::SNoBorder;
    OString aEndTags = rHTMLWrt.OutFrameFormatOptions( rFrameFormat, aEmptyOUStr, nFrameFlags );
    rHTMLWrt.OutCSS1_FrameFormatOptions( rFrameFormat, nFrameFlags );
    rWrt.Strm().WriteChar( '>' );

    rHTMLWrt.IncIndentLevel();  // indent the content
    rHTMLWrt.m_bLFPossible = true;

    const SwFormatContent& rFlyContent = rFrameFormat.GetContent();
    sal_uLong nStt = rFlyContent.GetContentIdx()->GetIndex();

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
        rHTMLWrt.Out_SwDoc( rWrt.m_pCurrentPam );
    }

    rHTMLWrt.DecIndentLevel();  // indent the content of Multicol;
    if( rHTMLWrt.m_bLFPossible )
        rHTMLWrt.OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rHTMLWrt.GetNamespace() + aTag, false );

    if( !aEndTags.isEmpty() )
        rWrt.Strm().WriteCharPtr( aEndTags.getStr() );

    return rWrt;
}

static Writer & OutHTML_FrameFormatAsImage( Writer& rWrt, const SwFrameFormat& rFrameFormat )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    if (rHTMLWrt.mbSkipImages)
        return rWrt;

    ImageMap aIMap;
    Graphic aGraphic( const_cast<SwFrameFormat &>(rFrameFormat).MakeGraphic( &aIMap ) );
    Size aSz( 0, 0 );
    OUString GraphicURL;
    if(!rHTMLWrt.mbEmbedImages)
    {
        if( rHTMLWrt.GetOrigFileName() )
            GraphicURL = *rHTMLWrt.GetOrigFileName();
        if( aGraphic.GetType() == GraphicType::NONE ||
            XOutBitmap::WriteGraphic( aGraphic, GraphicURL,
                                      "JPG",
                                      (XOutFlags::UseGifIfPossible|
                                       XOutFlags::UseNativeIfPossible) ) != ERRCODE_NONE )
        {
            // empty or incorrect, because there is nothing to output
            rHTMLWrt.m_nWarn = WARN_SWG_POOR_LOAD;
            return rWrt;
        }

        GraphicURL = URIHelper::SmartRel2Abs(
            INetURLObject(rWrt.GetBaseURL()), GraphicURL,
            URIHelper::GetMaybeFileHdl() );

    }
    OutHTML_Image( rWrt, rFrameFormat, GraphicURL, aGraphic, rFrameFormat.GetName(), aSz,
                    HtmlFrmOpts::GenImgMask, "frame",
                    aIMap.GetIMapObjectCount() ? &aIMap : nullptr );

    return rWrt;
}

static Writer& OutHTML_FrameFormatGrfNode( Writer& rWrt, const SwFrameFormat& rFrameFormat,
                                      bool bInCntnr )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    if (rHTMLWrt.mbSkipImages)
        return rWrt;

    const SwFormatContent& rFlyContent = rFrameFormat.GetContent();
    sal_uLong nStt = rFlyContent.GetContentIdx()->GetIndex()+1;
    SwGrfNode *pGrfNd = rHTMLWrt.m_pDoc->GetNodes()[ nStt ]->GetGrfNode();
    OSL_ENSURE( pGrfNd, "Grf node expected" );
    if( !pGrfNd )
        return rWrt;

    HtmlFrmOpts nFrameFlags = bInCntnr ? HTML_FRMOPTS_IMG_CNTNR : HTML_FRMOPTS_IMG;
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) && !bInCntnr )
         nFrameFlags |= HTML_FRMOPTS_IMG_CSS1;

    Graphic aGraphic = pGrfNd->GetGraphic();
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

            Size aMM100Size;
            const SwFormatFrameSize& rSize = rFrameFormat.GetFrameSize();
            aMM100Size = OutputDevice::LogicToLogic( rSize.GetSize(),
                            MapMode( MapUnit::MapTwip ), MapMode( MapUnit::Map100thMM ));

            OUString aFilterName("JPG");

            if (rHTMLWrt.mbReqIF)
            {
                // Writing image without fallback PNG in ReqIF mode: force PNG
                // output.
                aFilterName = "PNG";
                nFlags &= ~XOutFlags::UseNativeIfPossible;
                nFlags &= ~XOutFlags::UseGifIfSensible;
                aMimeType = "image/png";
            }

            const Graphic& rGraphic = pGrfNd->GetGrf();

            // So that Graphic::IsTransparent() can report true.
            if (!rGraphic.isAvailable())
                const_cast<Graphic&>(rGraphic).makeAvailable();

            ErrCode nErr = XOutBitmap::WriteGraphic( rGraphic, aGraphicURL,
                    aFilterName, nFlags, &aMM100Size );
            if( nErr )
            {
                rHTMLWrt.m_nWarn = WARN_SWG_POOR_LOAD;
                return rWrt;
            }
            aGraphicURL = URIHelper::SmartRel2Abs(
                INetURLObject(rWrt.GetBaseURL()), aGraphicURL,
                URIHelper::GetMaybeFileHdl() );
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

    if (rHTMLWrt.mbReqIF)
    {
        // Write the original image as an RTF fragment.
        OUString aFileName;
        if (rHTMLWrt.GetOrigFileName())
            aFileName = *rHTMLWrt.GetOrigFileName();
        INetURLObject aURL(aFileName);
        OUString aName(aURL.getBase());
        aName += "_";
        aName += aURL.getExtension();
        aName += "_";
        aName += OUString::number(aGraphic.GetChecksum(), 16);
        aURL.setBase(aName);
        aURL.setExtension("ole");
        aFileName = aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);

        SvFileStream aOutStream(aFileName, StreamMode::WRITE);
        if (!SwReqIfReader::WrapGraphicInRtf(aGraphic, pGrfNd->GetTwipSize(), aOutStream))
            SAL_WARN("sw.html", "SwReqIfReader::WrapGraphicInRtf() failed");

        // Refer to this data.
        aFileName = URIHelper::simpleNormalizedMakeRelative(rWrt.GetBaseURL(), aFileName);
        rWrt.Strm().WriteOString("<" + rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_object);
        rWrt.Strm().WriteOString(" data=\"" + aFileName.toUtf8() + "\"");
        rWrt.Strm().WriteOString(" type=\"text/rtf\"");
        rWrt.Strm().WriteOString(">");
        rHTMLWrt.OutNewLine();
    }

    OutHTML_Image( rWrt, rFrameFormat, aGraphicURL, aGraphic, pGrfNd->GetTitle(),
                  pGrfNd->GetTwipSize(), nFrameFlags, "graphic", nullptr, aMimeType );

    if (rHTMLWrt.mbReqIF)
        rWrt.Strm().WriteOString("</" + rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_object ">");

    return rWrt;
}

static Writer& OutHTML_FrameFormatAsMarquee( Writer& rWrt, const SwFrameFormat& rFrameFormat,
                                  const SdrObject& rSdrObj  )
{
    SwHTMLWriter & rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    // get the edit engine attributes of the object as SW attributes and
    // sort them as Hints
    const SfxItemSet& rFormatItemSet = rFrameFormat.GetAttrSet();
    SfxItemSet aItemSet( *rFormatItemSet.GetPool(), svl::Items<RES_CHRATR_BEGIN,
                                                 RES_CHRATR_END>{} );
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
    sOut.append(OOO_STRING_SVTOOLS_HTML_division).append(' ')
        .append(OOO_STRING_SVTOOLS_HTML_O_title).append("=\"")
        .append( bHeader ? "header" : "footer" ).append("\"");
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rHTMLWrt.GetNamespace() + sOut.makeStringAndClear().getStr() );

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

        aSpacer = OStringBuffer(OOO_STRING_SVTOOLS_HTML_spacer).
            append(' ').append(OOO_STRING_SVTOOLS_HTML_O_type).
            append("=\"").append(OOO_STRING_SVTOOLS_HTML_SPTYPE_vertical).append("\"").
            append(' ').append(OOO_STRING_SVTOOLS_HTML_O_size).
            append("=\"").append(static_cast<sal_Int32>(nSize)).append("\"").
            makeStringAndClear();
    }

    const SwFormatContent& rFlyContent = rFrameFormat.GetContent();
    sal_uLong nStt = rFlyContent.GetContentIdx()->GetIndex();
    const SwStartNode* pSttNd = rWrt.m_pDoc->GetNodes()[nStt]->GetStartNode();
    OSL_ENSURE( pSttNd, "Where is the start node" );

    if( !bHeader && !aSpacer.isEmpty() )
    {
        rHTMLWrt.OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rHTMLWrt.GetNamespace() + aSpacer.getStr() );
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

        rHTMLWrt.Out_SwDoc( rWrt.m_pCurrentPam );
    }

    if( bHeader && !aSpacer.isEmpty() )
    {
        rHTMLWrt.OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rHTMLWrt.GetNamespace() + aSpacer.getStr() );
    }

    rHTMLWrt.DecIndentLevel();  // indent the content of Multicol;
    rHTMLWrt.OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_division, false );

    rHTMLWrt.m_nHeaderFooterSpace = 0;

    return rWrt;
}

void SwHTMLWriter::AddLinkTarget( const OUString& rURL )
{
    if( rURL.isEmpty() || rURL[0] != '#' )
        return;

    // There might be a '|' as delimiter (if the link has been inserted
    // freshly) or a '%7c' or a '%7C' if the document has been saved and
    // loaded already.
    sal_Int32 nPos = rURL.getLength();
    bool bFound = false, bEncoded = false;
    while( !bFound && nPos > 0 )
    {
        sal_Unicode c = rURL[ --nPos ];
        switch( c )
        {
        case cMarkSeparator:
            bFound = true;
            break;
        case '%':
            bFound = (rURL.getLength() - nPos) >=3 && rURL[ nPos+1 ] == '7';
            if(bFound)
            {
                c = rURL[ nPos+2 ];
                bFound = (c == 'C' || c == 'c');
            }
            if( bFound )
                bEncoded = true;
        }
    }
    if( !bFound || nPos < 2 ) // at least "#a|..."
        return;

    OUString aURL( rURL.copy( 1 ) );

    // nPos-1+1/3 (-1 because of Erase)
    OUString sCmp = aURL.copy(bEncoded ? nPos+2 : nPos).replaceAll(" ","");
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
        if( bEncoded )
        {
            aURL = aURL.replaceAt( nPos - 1, 3, OUString(cMarkSeparator)  );
        }
        m_aImplicitMarks.insert( aURL );
    }
    else if( sCmp == "outline" )
    {
        // Here, we need position and name. That's why we sort a
        // sal_uInt16 and a string array ourselves.
        OUString aOutline( aURL.copy( 0, nPos-1 ) );
        SwPosition aPos( *m_pCurrentPam->GetPoint() );
        if( m_pDoc->GotoOutline( aPos, aOutline ) )
        {
            sal_uInt32 nIdx = aPos.nNode.GetIndex();

            decltype(m_aOutlineMarkPoss)::size_type nIns=0;
            while( nIns < m_aOutlineMarkPoss.size() &&
                   m_aOutlineMarkPoss[nIns] < nIdx )
                nIns++;

            m_aOutlineMarkPoss.insert( m_aOutlineMarkPoss.begin()+nIns, nIdx );
            if( bEncoded )
            {
                aURL = aURL.replaceAt( nPos - 1, 3, OUString(cMarkSeparator) );
            }
            m_aOutlineMarks.insert( m_aOutlineMarks.begin()+nIns, aURL );
        }
    }
}

void SwHTMLWriter::CollectLinkTargets()
{
    const SwFormatINetFormat* pINetFormat;
    const SwTextINetFormat* pTextAttr;

    sal_uInt32 n, nMaxItems = m_pDoc->GetAttrPool().GetItemCount2( RES_TXTATR_INETFMT );
    for( n = 0; n < nMaxItems; ++n )
    {
        const SwTextNode* pTextNd;

        if( nullptr != ( pINetFormat = m_pDoc->GetAttrPool().GetItem2( RES_TXTATR_INETFMT, n ) ) &&
            nullptr != ( pTextAttr = pINetFormat->GetTextINetFormat()) &&
            nullptr != ( pTextNd = pTextAttr->GetpTextNode() ) &&
            pTextNd->GetNodes().IsDocNodes() )
        {
            AddLinkTarget( pINetFormat->GetValue() );
        }
    }

    const SwFormatURL *pURL;
    nMaxItems = m_pDoc->GetAttrPool().GetItemCount2( RES_URL );
    for( n = 0; n < nMaxItems; ++n )
    {
        if( nullptr != (pURL = m_pDoc->GetAttrPool().GetItem2(
            RES_URL, n ) ) )
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
