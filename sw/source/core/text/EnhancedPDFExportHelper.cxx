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

#include <EnhancedPDFExportHelper.hxx>

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <hintids.hxx>

#include <sot/exchange.hxx>
#include <vcl/outdev.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <vcl/pdf/PDFNote.hxx>
#include <tools/multisel.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/langitem.hxx>
#include <tools/urlobj.hxx>
#include <svl/languageoptions.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <swatrset.hxx>
#include <frmatr.hxx>
#include <paratr.hxx>
#include <ndtxt.hxx>
#include <ndole.hxx>
#include <section.hxx>
#include <tox.hxx>
#include <fmtfld.hxx>
#include <txtinet.hxx>
#include <fmtinfmt.hxx>
#include <fchrfmt.hxx>
#include <charfmt.hxx>
#include <fmtanchr.hxx>
#include <fmturl.hxx>
#include <editsh.hxx>
#include <viscrs.hxx>
#include <txtfld.hxx>
#include <reffld.hxx>
#include <doc.hxx>
#include <IDocumentOutlineNodes.hxx>
#include <mdiexp.hxx>
#include <docufld.hxx>
#include <ftnidx.hxx>
#include <txtftn.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <txtfrm.hxx>
#include <tabfrm.hxx>
#include <rowfrm.hxx>
#include <cellfrm.hxx>
#include <sectfrm.hxx>
#include <ftnfrm.hxx>
#include <flyfrm.hxx>
#include <notxtfrm.hxx>
#include "porfld.hxx"
#include "pormulti.hxx"
#include <SwStyleNameMapper.hxx>
#include "itrpaint.hxx"
#include <i18nlangtag/languagetag.hxx>
#include <IMark.hxx>
#include <printdata.hxx>
#include <vprint.hxx>
#include <SwNodeNum.hxx>
#include <calbck.hxx>
#include <frmtool.hxx>
#include <strings.hrc>
#include <frameformats.hxx>
#include <tblafmt.hxx>
#include <authfld.hxx>
#include <dcontact.hxx>

#include <tools/globname.hxx>
#include <svx/svdobj.hxx>

#include <stack>
#include <map>
#include <set>
#include <optional>

using namespace ::com::sun::star;

#if OSL_DEBUG_LEVEL > 1

static std::vector< sal_uInt16 > aStructStack;

void lcl_DBGCheckStack()
{
    /* NonStructElement = 0     Document = 1        Part = 2
     * Article = 3              Section = 4         Division = 5
     * BlockQuote = 6           Caption = 7         TOC = 8
     * TOCI = 9                 Index = 10          Paragraph = 11
     * Heading = 12             H1-6 = 13 - 18      List = 19
     * ListItem = 20            LILabel = 21        LIBody = 22
     * Table = 23               TableRow = 24       TableHeader = 25
     * TableData = 26           Span = 27           Quote = 28
     * Note = 29                Reference = 30      BibEntry = 31
     * Code = 32                Link = 33           Figure = 34
     * Formula = 35             Form = 36           Continued frame = 99
     */

    sal_uInt16 nElement;
    for ( const auto& rItem : aStructStack )
    {
        nElement = rItem;
    }
    (void)nElement;
};

#endif

typedef std::set< tools::Long, lt_TableColumn > TableColumnsMapEntry;
typedef std::pair< SwRect, sal_Int32 > IdMapEntry;
typedef std::vector< IdMapEntry > LinkIdMap;
typedef std::map< const SwTable*, TableColumnsMapEntry > TableColumnsMap;
typedef std::map< const SwNumberTreeNode*, sal_Int32 > NumListIdMap;
typedef std::map< const SwNumberTreeNode*, sal_Int32 > NumListBodyIdMap;
typedef std::set<const void*> FrameTagSet;

struct SwEnhancedPDFState
{
    TableColumnsMap m_TableColumnsMap;
    LinkIdMap m_LinkIdMap;
    NumListIdMap m_NumListIdMap;
    NumListBodyIdMap m_NumListBodyIdMap;
    FrameTagSet m_FrameTagSet;

    LanguageType m_eLanguageDefault;

    struct Span
    {
        FontLineStyle eUnderline;
        FontLineStyle eOverline;
        FontStrikeout eStrikeout;
        FontEmphasisMark eFontEmphasis;
        short nEscapement;
        SwFontScript nScript;
        LanguageType nLang;
        OUString StyleName;
    };

    ::std::optional<Span> m_oCurrentSpan;
    ::std::optional<SwTextAttr const*> m_oCurrentLink;

    SwEnhancedPDFState(LanguageType const eLanguageDefault)
        : m_eLanguageDefault(eLanguageDefault)
    {
    }
};

namespace
{
// ODF Style Names:
const char aTableHeadingName[]  = "Table Heading";
const char aQuotations[]        = "Quotations";
const char aCaption[]           = "Caption";
const char aHeading[]           = "Heading";
const char aQuotation[]         = "Quotation";
const char aSourceText[]        = "Source Text";

// PDF Tag Names:
constexpr OUStringLiteral aDocumentString = u"Document";
constexpr OUString aDivString = u"Div"_ustr;
constexpr OUStringLiteral aSectString = u"Sect";
constexpr OUStringLiteral aHString = u"H";
constexpr OUStringLiteral aH1String = u"H1";
constexpr OUStringLiteral aH2String = u"H2";
constexpr OUStringLiteral aH3String = u"H3";
constexpr OUStringLiteral aH4String = u"H4";
constexpr OUStringLiteral aH5String = u"H5";
constexpr OUStringLiteral aH6String = u"H6";
constexpr OUStringLiteral aH7String = u"H7";
constexpr OUStringLiteral aH8String = u"H8";
constexpr OUStringLiteral aH9String = u"H9";
constexpr OUStringLiteral aH10String = u"H10";
constexpr OUStringLiteral aListString = u"L";
constexpr OUStringLiteral aListItemString = u"LI";
constexpr OUStringLiteral aListLabelString = u"Lbl";
constexpr OUString aListBodyString = u"LBody"_ustr;
constexpr OUStringLiteral aBlockQuoteString = u"BlockQuote";
constexpr OUString aCaptionString = u"Caption"_ustr;
constexpr OUStringLiteral aIndexString = u"Index";
constexpr OUStringLiteral aTOCString = u"TOC";
constexpr OUStringLiteral aTOCIString = u"TOCI";
constexpr OUStringLiteral aTableString = u"Table";
constexpr OUStringLiteral aTRString = u"TR";
constexpr OUStringLiteral aTDString = u"TD";
constexpr OUStringLiteral aTHString = u"TH";
constexpr OUStringLiteral aBibEntryString = u"BibEntry";
constexpr OUStringLiteral aQuoteString = u"Quote";
constexpr OUString aSpanString = u"Span"_ustr;
constexpr OUStringLiteral aCodeString = u"Code";
constexpr OUStringLiteral aFigureString = u"Figure";
constexpr OUStringLiteral aFormulaString = u"Formula";
constexpr OUString aLinkString = u"Link"_ustr;
constexpr OUStringLiteral aNoteString = u"Note";

// returns true if first paragraph in cell frame has 'table heading' style
bool lcl_IsHeadlineCell( const SwCellFrame& rCellFrame )
{
    bool bRet = false;

    const SwContentFrame *pCnt = rCellFrame.ContainsContent();
    if ( pCnt && pCnt->IsTextFrame() )
    {
        SwTextNode const*const pTextNode = static_cast<const SwTextFrame*>(pCnt)->GetTextNodeForParaProps();
        const SwFormat* pTextFormat = pTextNode->GetFormatColl();

        OUString sStyleName;
        SwStyleNameMapper::FillProgName( pTextFormat->GetName(), sStyleName, SwGetPoolIdFromName::TxtColl );
        bRet = sStyleName == aTableHeadingName;
    }

    // tdf#153935 wild guessing for 1st row based on table autoformat
    if (!bRet && !rCellFrame.GetUpper()->GetPrev())
    {
        SwTable const*const pTable(rCellFrame.FindTabFrame()->GetTable());
        assert(pTable);
        OUString const& rStyleName(pTable->GetTableStyleName());
        if (!rStyleName.isEmpty())
        {
            if (SwTableAutoFormat const*const pTableAF =
                pTable->GetFrameFormat()->GetDoc()->GetTableStyles().FindAutoFormat(rStyleName))
            {
                bRet |= pTableAF->HasHeaderRow();
            }
        }
    }

    return bRet;
}

// List all frames for which the NonStructElement tag is set:
bool lcl_IsInNonStructEnv( const SwFrame& rFrame )
{
    bool bRet = false;

    if ( nullptr != rFrame.FindFooterOrHeader() &&
           !rFrame.IsHeaderFrame() && !rFrame.IsFooterFrame() )
    {
        bRet = true;
    }
    else if ( rFrame.IsInTab() && !rFrame.IsTabFrame() )
    {
        const SwTabFrame* pTabFrame = rFrame.FindTabFrame();
        if ( rFrame.GetUpper() != pTabFrame &&
             pTabFrame->IsFollow() && pTabFrame->IsInHeadline( rFrame ) )
             bRet = true;
    }

    return bRet;
}

// Generate key from frame for reopening tags:
void const* lcl_GetKeyFromFrame( const SwFrame& rFrame )
{
    void const* pKey = nullptr;

    if ( rFrame.IsPageFrame() )
        pKey = static_cast<void const *>(&(static_cast<const SwPageFrame&>(rFrame).GetFormat()->getIDocumentSettingAccess()));
    else if ( rFrame.IsTextFrame() )
        pKey = static_cast<void const *>(static_cast<const SwTextFrame&>(rFrame).GetTextNodeFirst());
    else if ( rFrame.IsSctFrame() )
        pKey = static_cast<void const *>(static_cast<const SwSectionFrame&>(rFrame).GetSection());
    else if ( rFrame.IsTabFrame() )
        pKey = static_cast<void const *>(static_cast<const SwTabFrame&>(rFrame).GetTable());
    else if ( rFrame.IsRowFrame() )
        pKey = static_cast<void const *>(static_cast<const SwRowFrame&>(rFrame).GetTabLine());
    else if ( rFrame.IsCellFrame() )
    {
        const SwTabFrame* pTabFrame = rFrame.FindTabFrame();
        const SwTable* pTable = pTabFrame->GetTable();
        pKey = static_cast<void const *>(& static_cast<const SwCellFrame&>(rFrame).GetTabBox()->FindStartOfRowSpan(*pTable));
    }
    else if (rFrame.IsFootnoteFrame())
    {
        pKey = static_cast<void const*>(static_cast<SwFootnoteFrame const&>(rFrame).GetAttr());
    }

    return pKey;
}

bool lcl_HasPreviousParaSameNumRule(SwTextFrame const& rTextFrame, const SwTextNode& rNode)
{
    bool bRet = false;
    SwNodeIndex aIdx( rNode );
    const SwDoc& rDoc = rNode.GetDoc();
    const SwNodes& rNodes = rDoc.GetNodes();
    const SwNode* pNode = &rNode;
    const SwNumRule* pNumRule = rNode.GetNumRule();

    while (pNode != rNodes.DocumentSectionStartNode(const_cast<SwNode*>(static_cast<SwNode const *>(&rNode))) )
    {
        sw::GotoPrevLayoutTextFrame(aIdx, rTextFrame.getRootFrame());

        if (aIdx.GetNode().IsTextNode())
        {
            const SwTextNode *const pPrevTextNd = sw::GetParaPropsNode(
                    *rTextFrame.getRootFrame(), *aIdx.GetNode().GetTextNode());
            const SwNumRule * pPrevNumRule = pPrevTextNd->GetNumRule();

            // We find the previous text node. Now check, if the previous text node
            // has the same numrule like rNode:
            if ( (pPrevNumRule == pNumRule) &&
                 (!pPrevTextNd->IsOutline() == !rNode.IsOutline()))
                bRet = true;

            break;
        }

        pNode = &aIdx.GetNode();
    }
    return bRet;
}

bool lcl_TryMoveToNonHiddenField(SwEditShell& rShell, const SwTextNode& rNd, const SwFormatField& rField)
{
    // 1. Check if the whole paragraph is hidden
    // 2. Move to the field
    // 3. Check for hidden text attribute
    if(rNd.IsHidden())
        return false;
    if(!rShell.GotoFormatField(rField) || rShell.IsInHiddenRange(/*bSelect=*/false))
    {
        rShell.SwCursorShell::ClearMark();
        return false;
    }
    return true;
};

// tdf#157816: try to check if the rectangle contains actual text
::std::vector<SwRect> GetCursorRectsContainingText(SwCursorShell const& rShell)
{
    ::std::vector<SwRect> ret;
    SwRects rects;
    rShell.GetLayout()->CalcFrameRects(*rShell.GetCursor_(), rects, SwRootFrame::RectsMode::NoAnchoredFlys);
    for (SwRect const& rRect : rects)
    {
        Point center(rRect.Center());
        SwSpecialPos special;
        SwCursorMoveState cms(CursorMoveState::NONE);
        cms.m_pSpecialPos = &special;
        cms.m_bFieldInfo = true;
        SwPosition pos(rShell.GetDoc()->GetNodes());
        auto const [pStart, pEnd] = rShell.GetCursor_()->StartEnd();
        if (rShell.GetLayout()->GetModelPositionForViewPoint(&pos, center, &cms)
            && *pStart <= pos && pos <= *pEnd)
        {
            SwRect charRect;
            std::pair<Point, bool> const tmp(center, false);
            SwContentFrame const*const pFrame(
                pos.nNode.GetNode().GetTextNode()->getLayoutFrame(rShell.GetLayout(), &pos, &tmp));
            if (pFrame->GetCharRect(charRect, pos, &cms, false)
                && rRect.Overlaps(charRect))
            {
                ret.push_back(rRect);
            }
        }
        // reset stupid static var that may have gotten set now
        SwTextCursor::SetRightMargin(false); // WTF is this crap
    }
    return ret;
}

} // end namespace

SwTaggedPDFHelper::SwTaggedPDFHelper( const Num_Info* pNumInfo,
                                      const Frame_Info* pFrameInfo,
                                      const Por_Info* pPorInfo,
                                      OutputDevice const & rOut )
  : m_nEndStructureElement( 0 ),
    m_nRestoreCurrentTag( -1 ),
    mpNumInfo( pNumInfo ),
    mpFrameInfo( pFrameInfo ),
    mpPorInfo( pPorInfo )
{
    mpPDFExtOutDevData =
        dynamic_cast< vcl::PDFExtOutDevData*>( rOut.GetExtOutDevData() );

    if ( !(mpPDFExtOutDevData && mpPDFExtOutDevData->GetIsExportTaggedPDF()) )
        return;

#if OSL_DEBUG_LEVEL > 1
    sal_Int32 nCurrentStruct = mpPDFExtOutDevData->GetCurrentStructureElement();
    lcl_DBGCheckStack();
#endif
    if ( mpNumInfo )
        BeginNumberedListStructureElements();
    else if ( mpFrameInfo )
        BeginBlockStructureElements();
    else if ( mpPorInfo )
        BeginInlineStructureElements();
    else
        BeginTag( vcl::PDFWriter::NonStructElement, OUString() );

#if OSL_DEBUG_LEVEL > 1
    nCurrentStruct = mpPDFExtOutDevData->GetCurrentStructureElement();
    lcl_DBGCheckStack();
    (void)nCurrentStruct;
#endif
}

SwTaggedPDFHelper::~SwTaggedPDFHelper()
{
    if ( !(mpPDFExtOutDevData && mpPDFExtOutDevData->GetIsExportTaggedPDF()) )
        return;

#if OSL_DEBUG_LEVEL > 1
    sal_Int32 nCurrentStruct = mpPDFExtOutDevData->GetCurrentStructureElement();
    lcl_DBGCheckStack();
#endif
    EndStructureElements();

#if OSL_DEBUG_LEVEL > 1
    nCurrentStruct = mpPDFExtOutDevData->GetCurrentStructureElement();
    lcl_DBGCheckStack();
    (void)nCurrentStruct;
#endif
}

void const* SwDrawContact::GetPDFAnchorStructureElementKey(SdrObject const& rObj)
{
    SwFrame const*const pAnchorFrame(GetAnchoredObj(&rObj)->GetAnchorFrame());
    return pAnchorFrame ? lcl_GetKeyFromFrame(*pAnchorFrame) : nullptr;
}

bool SwTaggedPDFHelper::CheckReopenTag()
{
    bool bRet = false;
    void const* pReopenKey(nullptr);
    bool bContinue = false; // in some cases we just have to reopen a tag without early returning

    if ( mpFrameInfo )
    {
        const SwFrame& rFrame = mpFrameInfo->mrFrame;
        const SwFrame* pKeyFrame = nullptr;

        // Reopen an existing structure element if
        // - rFrame is not the first page frame (reopen Document tag)
        // - rFrame is a follow frame (reopen Master tag)
        // - rFrame is a fly frame anchored at content (reopen Anchor paragraph tag)
        // - rFrame is a fly frame anchored at page (reopen Document tag)
        // - rFrame is a follow flow row (reopen TableRow tag)
        // - rFrame is a cell frame in a follow flow row (reopen TableData tag)
        if ( ( rFrame.IsPageFrame() && static_cast<const SwPageFrame&>(rFrame).GetPrev() ) ||
             ( rFrame.IsFlowFrame() && SwFlowFrame::CastFlowFrame(&rFrame)->IsFollow() ) ||
             (rFrame.IsFootnoteFrame() && static_cast<SwFootnoteFrame const&>(rFrame).GetMaster()) ||
             ( rFrame.IsRowFrame() && rFrame.IsInFollowFlowRow() ) ||
             ( rFrame.IsCellFrame() && const_cast<SwFrame&>(rFrame).GetPrevCellLeaf() ) )
        {
            pKeyFrame = &rFrame;
        }
        else if (rFrame.IsFlyFrame() && !mpFrameInfo->m_isLink)
        {
            const SwFormatAnchor& rAnchor =
                static_cast<const SwFlyFrame*>(&rFrame)->GetFormat()->GetAnchor();
            if ((RndStdIds::FLY_AT_PARA == rAnchor.GetAnchorId()) ||
                (RndStdIds::FLY_AT_CHAR == rAnchor.GetAnchorId()) ||
                (RndStdIds::FLY_AT_PAGE == rAnchor.GetAnchorId()))
            {
                pKeyFrame = static_cast<const SwFlyFrame&>(rFrame).GetAnchorFrame();
                bContinue = true;
            }
        }

        if ( pKeyFrame )
        {
            void const*const pKey = lcl_GetKeyFromFrame(*pKeyFrame);
            FrameTagSet& rFrameTagSet(mpPDFExtOutDevData->GetSwPDFState()->m_FrameTagSet);
            if (rFrameTagSet.find(pKey) != rFrameTagSet.end()
                || rFrame.IsFlyFrame()) // for hell layer flys
            {
                pReopenKey = pKey;
            }
        }
    }

    if (pReopenKey)
    {
        // note: it would be possible to get rid of the SetCurrentStructureElement()
        // - which is quite ugly - for most cases by recreating the parents until the
        // current ancestor, but there are special cases cell frame rowspan > 1 follow
        // and footnote frame follow where the parent of the follow is different from
        // the parent of the first one, and so in PDFExtOutDevData the wrong parent
        // would be restored and used for next elements.
        m_nRestoreCurrentTag = mpPDFExtOutDevData->GetCurrentStructureElement();
        sal_Int32 const id = mpPDFExtOutDevData->EnsureStructureElement(pReopenKey);
        mpPDFExtOutDevData->SetCurrentStructureElement(id);

        bRet = true;
    }

    return bRet && !bContinue;
}

void SwTaggedPDFHelper::CheckRestoreTag() const
{
    if ( m_nRestoreCurrentTag != -1 )
    {
        mpPDFExtOutDevData->SetCurrentStructureElement( m_nRestoreCurrentTag );

#if OSL_DEBUG_LEVEL > 1
        aStructStack.pop_back();
#endif
    }
}

void SwTaggedPDFHelper::OpenTagImpl(void const*const pKey)
{
    sal_Int32 const id = mpPDFExtOutDevData->EnsureStructureElement(pKey);
    mpPDFExtOutDevData->BeginStructureElement(id);
    ++m_nEndStructureElement;

#if OSL_DEBUG_LEVEL > 1
    aStructStack.push_back( 99 );
#endif
}

sal_Int32 SwTaggedPDFHelper::BeginTagImpl(void const*const pKey,
    vcl::PDFWriter::StructElement const eType, const OUString& rString)
{
    // write new tag
    const sal_Int32 nId = mpPDFExtOutDevData->EnsureStructureElement(pKey);
    mpPDFExtOutDevData->InitStructureElement(nId, eType, rString);
    mpPDFExtOutDevData->BeginStructureElement(nId);
    ++m_nEndStructureElement;

#if OSL_DEBUG_LEVEL > 1
    aStructStack.push_back( o3tl::narrowing<sal_uInt16>(eType) );
#endif

    return nId;
}

void SwTaggedPDFHelper::BeginTag( vcl::PDFWriter::StructElement eType, const OUString& rString )
{
    void const* pKey(nullptr);

    if (mpFrameInfo)
    {
        const SwFrame& rFrame = mpFrameInfo->mrFrame;

        if ( ( rFrame.IsPageFrame() && !static_cast<const SwPageFrame&>(rFrame).GetPrev() ) ||
             ( rFrame.IsFlowFrame() && !SwFlowFrame::CastFlowFrame(&rFrame)->IsFollow() && SwFlowFrame::CastFlowFrame(&rFrame)->HasFollow() ) ||
             rFrame.IsSctFrame() || // all of them, so that opening parent sections works
             ( rFrame.IsTextFrame() && rFrame.GetDrawObjs() ) ||
             (rFrame.IsFootnoteFrame() && static_cast<SwFootnoteFrame const&>(rFrame).GetFollow()) ||
             ( rFrame.IsRowFrame() && rFrame.IsInSplitTableRow() ) ||
             ( rFrame.IsCellFrame() && const_cast<SwFrame&>(rFrame).GetNextCellLeaf() ) )
        {
            pKey = lcl_GetKeyFromFrame(rFrame);

            if (pKey)
            {
                FrameTagSet& rFrameTagSet(mpPDFExtOutDevData->GetSwPDFState()->m_FrameTagSet);
                assert(rFrameTagSet.find(pKey) == rFrameTagSet.end());
                rFrameTagSet.emplace(pKey);
            }
        }
    }

    sal_Int32 const nId = BeginTagImpl(pKey, eType, rString);

    // Store the id of the current structure element if
    // - it is a list structure element
    // - it is a list body element with children
    // - rFrame is the first page frame
    // - rFrame is a master frame
    // - rFrame has objects anchored to it
    // - rFrame is a row frame or cell frame in a split table row

    if ( mpNumInfo )
    {
        const SwTextFrame& rTextFrame = mpNumInfo->mrFrame;
        SwTextNode const*const pTextNd = rTextFrame.GetTextNodeForParaProps();
        const SwNodeNum* pNodeNum = pTextNd->GetNum(rTextFrame.getRootFrame());

        if ( vcl::PDFWriter::List == eType )
        {
            NumListIdMap& rNumListIdMap(mpPDFExtOutDevData->GetSwPDFState()->m_NumListIdMap);
            rNumListIdMap[ pNodeNum ] = nId;
        }
        else if ( vcl::PDFWriter::LIBody == eType )
        {
            NumListBodyIdMap& rNumListBodyIdMap(mpPDFExtOutDevData->GetSwPDFState()->m_NumListBodyIdMap);
            rNumListBodyIdMap[ pNodeNum ] = nId;
        }
    }

    SetAttributes( eType );
}

void SwTaggedPDFHelper::EndTag()
{
    mpPDFExtOutDevData->EndStructureElement();

#if OSL_DEBUG_LEVEL > 1
    aStructStack.pop_back();
#endif
}

namespace {

    // link the link annotation to the link structured element
    void LinkLinkLink(vcl::PDFExtOutDevData & rPDFExtOutDevData, SwRect const& rRect)
    {
        const LinkIdMap& rLinkIdMap(rPDFExtOutDevData.GetSwPDFState()->m_LinkIdMap);
        const Point aCenter = rRect.Center();
        auto aIter = std::find_if(rLinkIdMap.begin(), rLinkIdMap.end(),
            [&aCenter](const IdMapEntry& rEntry) { return rEntry.first.Contains(aCenter); });
        if (aIter != rLinkIdMap.end())
        {
            sal_Int32 nLinkId = (*aIter).second;
            rPDFExtOutDevData.SetStructureAttributeNumerical(vcl::PDFWriter::LinkAnnotation, nLinkId);
        }
    }
}

// Sets the attributes according to the structure type.
void SwTaggedPDFHelper::SetAttributes( vcl::PDFWriter::StructElement eType )
{
    sal_Int32 nVal;

    /*
     * ATTRIBUTES FOR BLSE
     */
    if ( mpFrameInfo )
    {
        vcl::PDFWriter::StructAttributeValue eVal;
        const SwFrame* pFrame = &mpFrameInfo->mrFrame;
        SwRectFnSet aRectFnSet(pFrame);

        bool bPlacement = false;
        bool bWritingMode = false;
        bool bSpaceBefore = false;
        bool bSpaceAfter = false;
        bool bStartIndent = false;
        bool bEndIndent = false;
        bool bTextIndent = false;
        bool bTextAlign = false;
        bool bWidth = false;
        bool bHeight = false;
        bool bBox = false;
        bool bRowSpan = false;
        bool bAltText = false;

        // Check which attributes to set:

        switch ( eType )
        {
            case vcl::PDFWriter::Document :
                bWritingMode = true;
                break;

            case vcl::PDFWriter::Note:
                bPlacement = true;
                break;

            case vcl::PDFWriter::Table :
                bPlacement =
                bWritingMode =
                bSpaceBefore =
                bSpaceAfter =
                bStartIndent =
                bEndIndent =
                bWidth =
                bHeight =
                bBox = true;
                break;

            case vcl::PDFWriter::TableRow :
                bPlacement =
                bWritingMode = true;
                break;

            case vcl::PDFWriter::TableHeader :
                mpPDFExtOutDevData->SetStructureAttribute(vcl::PDFWriter::Scope, vcl::PDFWriter::Column);
                [[fallthrough]];
            case vcl::PDFWriter::TableData :
                bPlacement =
                bWritingMode =
                bWidth =
                bHeight =
                bRowSpan = true;
                break;

            case vcl::PDFWriter::Caption:
                if (pFrame->IsSctFrame())
                {
                    break;
                }
                [[fallthrough]];
            case vcl::PDFWriter::H1 :
            case vcl::PDFWriter::H2 :
            case vcl::PDFWriter::H3 :
            case vcl::PDFWriter::H4 :
            case vcl::PDFWriter::H5 :
            case vcl::PDFWriter::H6 :
            case vcl::PDFWriter::Paragraph :
            case vcl::PDFWriter::Heading :
            case vcl::PDFWriter::BlockQuote :

                bPlacement =
                bWritingMode =
                bSpaceBefore =
                bSpaceAfter =
                bStartIndent =
                bEndIndent =
                bTextIndent =
                bTextAlign = true;
                break;

            case vcl::PDFWriter::Formula :
            case vcl::PDFWriter::Figure :
                bAltText =
                bPlacement =
                bWidth =
                bHeight =
                bBox = true;
                break;

            case vcl::PDFWriter::Division:
                if (pFrame->IsFlyFrame()) // this can be something else too
                {
                    bAltText = true;
                    bBox = true;
                }
                break;

            case vcl::PDFWriter::NonStructElement:
                if (pFrame->IsHeaderFrame() || pFrame->IsFooterFrame())
                {
                    // ISO 14289-1:2014, Clause: 7.8
                    mpPDFExtOutDevData->SetStructureAttribute(vcl::PDFWriter::Type, vcl::PDFWriter::Pagination);
                    mpPDFExtOutDevData->SetStructureAttribute(vcl::PDFWriter::Subtype,
                        pFrame->IsHeaderFrame()
                           ? vcl::PDFWriter::Header
                           : vcl::PDFWriter::Footer);
                }
            break;

            default :
                break;
        }

        // Set the attributes:

        if ( bPlacement )
        {
            eVal = vcl::PDFWriter::TableHeader == eType ||
                   vcl::PDFWriter::TableData   == eType ?
                   vcl::PDFWriter::Inline :
                   vcl::PDFWriter::Block;

            mpPDFExtOutDevData->SetStructureAttribute( vcl::PDFWriter::Placement, eVal );
        }

        if ( bWritingMode )
        {
            eVal =  pFrame->IsVertical() ?
                    vcl::PDFWriter::TbRl :
                    pFrame->IsRightToLeft() ?
                    vcl::PDFWriter::RlTb :
                    vcl::PDFWriter::LrTb;

            if ( vcl::PDFWriter::LrTb != eVal )
                mpPDFExtOutDevData->SetStructureAttribute( vcl::PDFWriter::WritingMode, eVal );
        }

        if ( bSpaceBefore )
        {
            nVal = aRectFnSet.GetTopMargin(*pFrame);
            if ( 0 != nVal )
                mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::SpaceBefore, nVal );
        }

        if ( bSpaceAfter )
        {
            nVal = aRectFnSet.GetBottomMargin(*pFrame);
            if ( 0 != nVal )
                mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::SpaceAfter, nVal );
        }

        if ( bStartIndent )
        {
            nVal = aRectFnSet.GetLeftMargin(*pFrame);
            if ( 0 != nVal )
                mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::StartIndent, nVal );
        }

        if ( bEndIndent )
        {
            nVal = aRectFnSet.GetRightMargin(*pFrame);
            if ( 0 != nVal )
                mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::EndIndent, nVal );
        }

        if ( bTextIndent )
        {
            OSL_ENSURE( pFrame->IsTextFrame(), "Frame type <-> tag attribute mismatch" );
            const SvxFirstLineIndentItem& rFirstLine(
                static_cast<const SwTextFrame*>(pFrame)->GetTextNodeForParaProps()->GetSwAttrSet().GetFirstLineIndent());
            nVal = rFirstLine.GetTextFirstLineOffset();
            if ( 0 != nVal )
                mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::TextIndent, nVal );
        }

        if ( bTextAlign )
        {
            OSL_ENSURE( pFrame->IsTextFrame(), "Frame type <-> tag attribute mismatch" );
            const SwAttrSet& aSet = static_cast<const SwTextFrame*>(pFrame)->GetTextNodeForParaProps()->GetSwAttrSet();
            const SvxAdjust nAdjust = aSet.GetAdjust().GetAdjust();
            if ( SvxAdjust::Block == nAdjust || SvxAdjust::Center == nAdjust ||
                 (  (pFrame->IsRightToLeft() && SvxAdjust::Left == nAdjust) ||
                   (!pFrame->IsRightToLeft() && SvxAdjust::Right == nAdjust) ) )
            {
                eVal = SvxAdjust::Block == nAdjust ?
                       vcl::PDFWriter::Justify :
                       SvxAdjust::Center == nAdjust ?
                       vcl::PDFWriter::Center :
                       vcl::PDFWriter::End;

                mpPDFExtOutDevData->SetStructureAttribute( vcl::PDFWriter::TextAlign, eVal );
            }
        }

        // ISO 14289-1:2014, Clause: 7.3
        // ISO 14289-1:2014, Clause: 7.7
        // For images (but not embedded objects), an ObjectInfoPrimitive2D is
        // created, but it's not evaluated by VclMetafileProcessor2D any more;
        // that would require producing StructureTagPrimitive2D here but that
        // looks impossible so instead duplicate the code that sets the Alt
        // text here again.
        if (bAltText)
        {
            SwFlyFrameFormat const& rFly(*static_cast<SwFlyFrame const*>(pFrame)->GetFormat());
            OUString const sep(
                (rFly.GetObjTitle().isEmpty() || rFly.GetObjDescription().isEmpty())
                ? OUString() : OUString(" - "));
            OUString const altText(rFly.GetObjTitle() + sep + rFly.GetObjDescription());
            if (!altText.isEmpty())
            {
                mpPDFExtOutDevData->SetAlternateText(altText);
            }
        }

        if ( bWidth )
        {
            nVal = aRectFnSet.GetWidth(pFrame->getFrameArea());
            mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::Width, nVal );
        }

        if ( bHeight )
        {
            nVal = aRectFnSet.GetHeight(pFrame->getFrameArea());
            mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::Height, nVal );
        }

        if ( bBox )
        {
            // BBox only for non-split tables:
            if ( vcl::PDFWriter::Table != eType ||
                 ( pFrame->IsTabFrame() &&
                   !static_cast<const SwTabFrame*>(pFrame)->IsFollow() &&
                   !static_cast<const SwTabFrame*>(pFrame)->HasFollow() ) )
            {
                mpPDFExtOutDevData->SetStructureBoundingBox(pFrame->getFrameArea().SVRect());
            }
        }

        if ( bRowSpan )
        {
            if ( pFrame->IsCellFrame() )
            {
                const SwCellFrame* pThisCell = static_cast<const SwCellFrame*>(pFrame);
                nVal =  pThisCell->GetTabBox()->getRowSpan();
                if ( nVal > 1 )
                    mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::RowSpan, nVal );

                // calculate colspan:
                const SwTabFrame* pTabFrame = pThisCell->FindTabFrame();
                const SwTable* pTable = pTabFrame->GetTable();

                SwRectFnSet fnRectX(pTabFrame);

                const TableColumnsMapEntry& rCols(mpPDFExtOutDevData->GetSwPDFState()->m_TableColumnsMap[pTable]);

                const tools::Long nLeft  = fnRectX.GetLeft(pThisCell->getFrameArea());
                const tools::Long nRight = fnRectX.GetRight(pThisCell->getFrameArea());
                const TableColumnsMapEntry::const_iterator aLeftIter =  rCols.find( nLeft );
                const TableColumnsMapEntry::const_iterator aRightIter = rCols.find( nRight );

                OSL_ENSURE( aLeftIter != rCols.end() && aRightIter != rCols.end(), "Colspan trouble" );
                if ( aLeftIter != rCols.end() && aRightIter != rCols.end() )
                {
                    nVal = std::distance( aLeftIter, aRightIter );
                    if ( nVal > 1 )
                        mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::ColSpan, nVal );
                }
            }
        }

        if (mpFrameInfo->m_isLink)
        {
            SwRect const aRect(mpFrameInfo->mrFrame.getFrameArea());
            LinkLinkLink(*mpPDFExtOutDevData, aRect);
        }
    }

    /*
     * ATTRIBUTES FOR ILSE
     */
    else if ( mpPorInfo )
    {
        const SwLinePortion* pPor = &mpPorInfo->mrPor;
        const SwTextPaintInfo& rInf = mpPorInfo->mrTextPainter.GetInfo();

        bool bActualText = false;
        bool bBaselineShift = false;
        bool bTextDecorationType = false;
        bool bLinkAttribute = false;
        bool bLanguage = false;

        // Check which attributes to set:

        switch ( eType )
        {
            case vcl::PDFWriter::Span :
            case vcl::PDFWriter::Quote :
            case vcl::PDFWriter::Code :
                if( PortionType::HyphenStr == pPor->GetWhichPor() || PortionType::SoftHyphenStr == pPor->GetWhichPor() ||
                    PortionType::Hyphen == pPor->GetWhichPor() || PortionType::SoftHyphen == pPor->GetWhichPor() )
                    bActualText = true;
                else
                {
                    bBaselineShift =
                    bTextDecorationType =
                    bLanguage = true;
                }
                break;

            case vcl::PDFWriter::Link :
                bTextDecorationType =
                bBaselineShift =
                bLinkAttribute =
                bLanguage = true;
                break;

            case vcl::PDFWriter::BibEntry :
                bTextDecorationType =
                bBaselineShift =
                bLinkAttribute =
                bLanguage = true;
                break;

            case vcl::PDFWriter::RT:
                {
                    SwRubyPortion const*const pRuby(static_cast<SwRubyPortion const*>(pPor));
                    vcl::PDFWriter::StructAttributeValue nAlign = {};
                    switch (pRuby->GetAdjustment())
                    {
                        case text::RubyAdjust_LEFT:
                            nAlign = vcl::PDFWriter::RStart;
                            break;
                        case text::RubyAdjust_CENTER:
                            nAlign = vcl::PDFWriter::RCenter;
                            break;
                        case text::RubyAdjust_RIGHT:
                            nAlign = vcl::PDFWriter::REnd;
                            break;
                        case text::RubyAdjust_BLOCK:
                            nAlign = vcl::PDFWriter::RJustify;
                            break;
                        case text::RubyAdjust_INDENT_BLOCK:
                            nAlign = vcl::PDFWriter::RDistribute;
                            break;
                        default:
                            assert(false);
                            break;
                    }
                    ::std::optional<vcl::PDFWriter::StructAttributeValue> oPos;
                    switch (pRuby->GetRubyPosition())
                    {
                        case RubyPosition::ABOVE:
                            oPos = vcl::PDFWriter::RBefore;
                            break;
                        case RubyPosition::BELOW:
                            oPos = vcl::PDFWriter::RAfter;
                            break;
                        case RubyPosition::RIGHT:
                            break; // no such thing???
                    }
                    mpPDFExtOutDevData->SetStructureAttribute(vcl::PDFWriter::RubyAlign, nAlign);
                    if (oPos)
                    {
                        mpPDFExtOutDevData->SetStructureAttribute(vcl::PDFWriter::RubyPosition, *oPos);
                    }
                }
                break;

            default:
                break;
        }

        if ( bActualText )
        {
            OUString aActualText;
            if (pPor->GetWhichPor() == PortionType::SoftHyphen || pPor->GetWhichPor() == PortionType::Hyphen)
                aActualText = OUString(u'\x00ad'); // soft hyphen
            else
                aActualText = rInf.GetText().copy(sal_Int32(rInf.GetIdx()), sal_Int32(pPor->GetLen()));
            mpPDFExtOutDevData->SetActualText( aActualText );
        }

        if ( bBaselineShift )
        {
            // TODO: Calculate correct values!
            nVal = rInf.GetFont()->GetEscapement();
            if ( nVal > 0 ) nVal = 33;
            else if ( nVal < 0 ) nVal = -33;

            if ( 0 != nVal )
            {
                nVal = nVal * pPor->Height() / 100;
                mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::BaselineShift, nVal );
            }
        }

        if ( bTextDecorationType )
        {
            if ( LINESTYLE_NONE    != rInf.GetFont()->GetUnderline() )
                mpPDFExtOutDevData->SetStructureAttribute( vcl::PDFWriter::TextDecorationType, vcl::PDFWriter::Underline );
            if ( LINESTYLE_NONE    != rInf.GetFont()->GetOverline() )
                mpPDFExtOutDevData->SetStructureAttribute( vcl::PDFWriter::TextDecorationType, vcl::PDFWriter::Overline );
            if ( STRIKEOUT_NONE    != rInf.GetFont()->GetStrikeout() )
                mpPDFExtOutDevData->SetStructureAttribute( vcl::PDFWriter::TextDecorationType, vcl::PDFWriter::LineThrough );
            if ( FontEmphasisMark::NONE != rInf.GetFont()->GetEmphasisMark() )
                mpPDFExtOutDevData->SetStructureAttribute( vcl::PDFWriter::TextDecorationType, vcl::PDFWriter::Overline );
        }

        if ( bLanguage )
        {

            const LanguageType nCurrentLanguage = rInf.GetFont()->GetLanguage();
            const LanguageType nDefaultLang(mpPDFExtOutDevData->GetSwPDFState()->m_eLanguageDefault);

            if ( nDefaultLang != nCurrentLanguage )
                mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::Language, static_cast<sal_uInt16>(nCurrentLanguage) );
        }

        if ( bLinkAttribute )
        {
            SwRect aPorRect;
            rInf.CalcRect( *pPor, &aPorRect );
            LinkLinkLink(*mpPDFExtOutDevData, aPorRect);
        }
    }
    else if (mpNumInfo && eType == vcl::PDFWriter::List)
    {
        SwTextFrame const& rFrame(mpNumInfo->mrFrame);
        SwTextNode const& rNode(*rFrame.GetTextNodeForParaProps());
        SwNumRule const*const pNumRule = rNode.GetNumRule();
        assert(pNumRule); // was required for List

        auto ToPDFListNumbering = [](SvxNumberFormat const& rFormat) {
            switch (rFormat.GetNumberingType())
            {
                case css::style::NumberingType::CHARS_UPPER_LETTER:
                    return vcl::PDFWriter::UpperAlpha;
                case css::style::NumberingType::CHARS_LOWER_LETTER:
                    return vcl::PDFWriter::LowerAlpha;
                case css::style::NumberingType::ROMAN_UPPER:
                    return vcl::PDFWriter::UpperRoman;
                case css::style::NumberingType::ROMAN_LOWER:
                    return vcl::PDFWriter::LowerRoman;
                case css::style::NumberingType::ARABIC:
                    return vcl::PDFWriter::Decimal;
                case css::style::NumberingType::CHAR_SPECIAL:
                    switch (rFormat.GetBulletChar())
                    {
                        case u'\u2022': case u'\uE12C': case u'\uE01E': case u'\uE437':
                            return vcl::PDFWriter::Disc;
                        case u'\u2218': case u'\u25CB': case u'\u25E6':
                            return vcl::PDFWriter::Circle;
                        case u'\u25A0': case u'\u25AA': case u'\uE00A':
                            return vcl::PDFWriter::Square;
                        default:
                            return vcl::PDFWriter::NONE;
                    }
                default: // the other 50 types
                    return vcl::PDFWriter::NONE;
            }
        };

        // Note: for every level, BeginNumberedListStructureElements() produces
        // a separate List element, so even though in PDF this is limited to
        // the whole List we can just export the current level here.
        vcl::PDFWriter::StructAttributeValue const value(
                ToPDFListNumbering(pNumRule->Get(rNode.GetActualListLevel())));
        // ISO 14289-1:2014, Clause: 7.6
        mpPDFExtOutDevData->SetStructureAttribute(vcl::PDFWriter::ListNumbering, value);
    }
}

void SwTaggedPDFHelper::BeginNumberedListStructureElements()
{
    OSL_ENSURE( mpNumInfo, "List without mpNumInfo?" );
    if ( !mpNumInfo )
        return;

    const SwFrame& rFrame = mpNumInfo->mrFrame;
    assert(rFrame.IsTextFrame());
    const SwTextFrame& rTextFrame = static_cast<const SwTextFrame&>(rFrame);

    // Lowers of NonStructureElements should not be considered:
    if (lcl_IsInNonStructEnv(rTextFrame))
        return;

    // do it for the first one in the follow chain that has content
    for (SwFlowFrame const* pPrecede = rTextFrame.GetPrecede(); pPrecede; pPrecede = pPrecede->GetPrecede())
    {
        SwTextFrame const*const pText(static_cast<SwTextFrame const*>(pPrecede));
        if (!pText->HasPara() || pText->GetPara()->HasContentPortions())
        {
            return;
        }
    }

    const SwTextNode *const pTextNd = rTextFrame.GetTextNodeForParaProps();
    const SwNumRule* pNumRule = pTextNd->GetNumRule();
    const SwNodeNum* pNodeNum = pTextNd->GetNum(rTextFrame.getRootFrame());

    const bool bNumbered = !pTextNd->IsOutline() && pNodeNum && pNodeNum->GetParent() && pNumRule;

    // Check, if we have to reopen a list or a list body:
    // First condition:
    // Paragraph is numbered/bulleted
    if ( !bNumbered )
        return;

    const SwNumberTreeNode* pParent = pNodeNum->GetParent();
    const bool bSameNumbering = lcl_HasPreviousParaSameNumRule(rTextFrame, *pTextNd);

    // Second condition: current numbering is not 'interrupted'
    if ( bSameNumbering )
    {
        sal_Int32 nReopenTag = -1;

        // Two cases:
        // 1. We have to reopen an existing list body tag:
        // - If the current node is either the first child of its parent
        //   and its level > 1 or
        // - Numbering should restart at the current node and its level > 1
        // - The current item has no label
        const bool bNewSubListStart = pParent->GetParent() && (pParent->IsFirst( pNodeNum ) || pTextNd->IsListRestart() );
        const bool bNoLabel = !pTextNd->IsCountedInList() && !pTextNd->IsListRestart();
        if ( bNewSubListStart || bNoLabel )
        {
            // Fine, we try to reopen the appropriate list body
            NumListBodyIdMap& rNumListBodyIdMap(mpPDFExtOutDevData->GetSwPDFState()->m_NumListBodyIdMap);

            if ( bNewSubListStart )
            {
                // The list body tag associated with the parent has to be reopened
                // to start a new list inside the list body
                NumListBodyIdMap::const_iterator aIter;

                do
                    aIter = rNumListBodyIdMap.find( pParent );
                while ( aIter == rNumListBodyIdMap.end() && nullptr != ( pParent = pParent->GetParent() ) );

                if ( aIter != rNumListBodyIdMap.end() )
                    nReopenTag = (*aIter).second;
            }
            else // if(bNoLabel)
            {
                // The list body tag of a 'counted' predecessor has to be reopened
                const SwNumberTreeNode* pPrevious = pNodeNum->GetPred(true);
                while ( pPrevious )
                {
                    if ( pPrevious->IsCounted())
                    {
                        // get id of list body tag
                        const NumListBodyIdMap::const_iterator aIter =  rNumListBodyIdMap.find( pPrevious );
                        if ( aIter != rNumListBodyIdMap.end() )
                        {
                            nReopenTag = (*aIter).second;
                            break;
                        }
                    }
                    pPrevious = pPrevious->GetPred(true);
                }
            }
        }
        // 2. We have to reopen an existing list tag:
        else if ( !pParent->IsFirst( pNodeNum ) && !pTextNd->IsListRestart() )
        {
            // any other than the first node in a list level has to reopen the current
            // list. The current list is associated in a map with the first child of the list:
            NumListIdMap& rNumListIdMap(mpPDFExtOutDevData->GetSwPDFState()->m_NumListIdMap);

            // Search backwards and check if any of the previous nodes has a list associated with it:
            const SwNumberTreeNode* pPrevious = pNodeNum->GetPred(true);
            while ( pPrevious )
            {
                // get id of list tag
                const NumListIdMap::const_iterator aIter =  rNumListIdMap.find( pPrevious );
                if ( aIter != rNumListIdMap.end() )
                {
                    nReopenTag = (*aIter).second;
                    break;
                }

                pPrevious = pPrevious->GetPred(true);
            }
        }

        if ( -1 != nReopenTag )
        {
            m_nRestoreCurrentTag = mpPDFExtOutDevData->GetCurrentStructureElement();
            mpPDFExtOutDevData->SetCurrentStructureElement( nReopenTag );

#if OSL_DEBUG_LEVEL > 1
            aStructStack.push_back( 99 );
#endif
        }
    }
    else
    {
        // clear list maps in case a list has been interrupted
        NumListIdMap& rNumListIdMap(mpPDFExtOutDevData->GetSwPDFState()->m_NumListIdMap);
        rNumListIdMap.clear();
        NumListBodyIdMap& rNumListBodyIdMap(mpPDFExtOutDevData->GetSwPDFState()->m_NumListBodyIdMap);
        rNumListBodyIdMap.clear();
    }

    // New tags:
    const bool bNewListTag = (pNodeNum->GetParent()->IsFirst( pNodeNum ) || pTextNd->IsListRestart() || !bSameNumbering);
    const bool bNewItemTag = bNewListTag || pTextNd->IsCountedInList(); // If the text node is not counted, we do not start a new list item:

    if ( bNewListTag )
        BeginTag( vcl::PDFWriter::List, aListString );

    if ( bNewItemTag )
    {
        BeginTag( vcl::PDFWriter::ListItem, aListItemString );
        assert(rTextFrame.GetPara());
        // check whether to open LBody now or delay until after Lbl
        if (!rTextFrame.GetPara()->HasNumberingPortion(SwParaPortion::OnlyNumbering))
        {
            BeginTag(vcl::PDFWriter::LIBody, aListBodyString);
        }
    }
}

void SwTaggedPDFHelper::BeginBlockStructureElements()
{
    const SwFrame* pFrame = &mpFrameInfo->mrFrame;

    // Lowers of NonStructureElements should not be considered:

    if (lcl_IsInNonStructEnv(*pFrame) && !pFrame->IsFlyFrame())
        return;

    // Check if we have to reopen an existing structure element.
    // This has to be done e.g., if pFrame is a follow frame.
    if ( CheckReopenTag() )
        return;

    sal_uInt16 nPDFType = USHRT_MAX;
    OUString aPDFType;

    switch ( pFrame->GetType() )
    {
        /*
         * GROUPING ELEMENTS
         */

        case SwFrameType::Page :

            // Document: Document

            nPDFType = vcl::PDFWriter::Document;
            aPDFType = aDocumentString;
            break;

        case SwFrameType::Header :
        case SwFrameType::Footer :

            // Header, Footer: NonStructElement

            nPDFType = vcl::PDFWriter::NonStructElement;
            break;

        case SwFrameType::FtnCont :

            // Footnote container: Division

            nPDFType = vcl::PDFWriter::Division;
            aPDFType = aDivString;
            break;

        case SwFrameType::Ftn :

            // Footnote frame: Note

            // Note: vcl::PDFWriter::Note is actually a ILSE. Nevertheless
            // we treat it like a grouping element!
            nPDFType = vcl::PDFWriter::Note;
            aPDFType = aNoteString;
            break;

        case SwFrameType::Section :

            // Section: TOX, Index, or Sect

            {
                const SwSection* pSection =
                        static_cast<const SwSectionFrame*>(pFrame)->GetSection();

                // open all parent sections, so that the SEs of sections
                // are nested in the same way as their SwSectionNodes
                std::vector<SwSection const*> parents;
                for (SwSection const* pParent = pSection->GetParent();
                     pParent != nullptr; pParent = pParent->GetParent())
                {
                    parents.push_back(pParent);
                }
                for (auto it = parents.rbegin(); it != parents.rend(); ++it)
                {
                    // key is the SwSection - see lcl_GetKeyFromFrame()
                    OpenTagImpl(*it);
                }

                FrameTagSet& rFrameTagSet(mpPDFExtOutDevData->GetSwPDFState()->m_FrameTagSet);
                if (rFrameTagSet.find(pSection) != rFrameTagSet.end())
                {
                    // special case: section may have *multiple* master frames,
                    // when it is interrupted by nested section - reopen!
                    OpenTagImpl(pSection);
                    break;
                }
                else if (SectionType::ToxHeader == pSection->GetType())
                {
                    nPDFType = vcl::PDFWriter::Caption;
                    aPDFType = aCaptionString;
                }
                else if (SectionType::ToxContent == pSection->GetType())
                {
                    const SwTOXBase* pTOXBase = pSection->GetTOXBase();
                    if ( pTOXBase )
                    {
                        if ( TOX_INDEX == pTOXBase->GetType() )
                        {
                            nPDFType = vcl::PDFWriter::Index;
                            aPDFType = aIndexString;
                        }
                        else
                        {
                            nPDFType = vcl::PDFWriter::TOC;
                            aPDFType = aTOCString;
                        }
                    }
                }
                else if ( SectionType::Content == pSection->GetType() )
                {
                    nPDFType = vcl::PDFWriter::Section;
                    aPDFType = aSectString;
                }
            }
            break;

        /*
         * BLOCK-LEVEL STRUCTURE ELEMENTS
         */

        case SwFrameType::Txt :
            {
                SwTextFrame const& rTextFrame(*static_cast<const SwTextFrame*>(pFrame));
                const SwTextNode *const pTextNd(rTextFrame.GetTextNodeForParaProps());

                // lazy open LBody after Lbl
                if (!pTextNd->IsOutline()
                    && rTextFrame.GetPara()->HasNumberingPortion(SwParaPortion::OnlyNumbering))
                {
                    sal_Int32 const nId = BeginTagImpl(nullptr, vcl::PDFWriter::LIBody, aListBodyString);
                    SwNodeNum const*const pNodeNum(pTextNd->GetNum(rTextFrame.getRootFrame()));
                    NumListBodyIdMap& rNumListBodyIdMap(mpPDFExtOutDevData->GetSwPDFState()->m_NumListBodyIdMap);
                    rNumListBodyIdMap[ pNodeNum ] = nId;
                }

                const SwFormat* pTextFormat = pTextNd->GetFormatColl();
                const SwFormat* pParentTextFormat = pTextFormat ? pTextFormat->DerivedFrom() : nullptr;

                OUString sStyleName;
                OUString sParentStyleName;

                if ( pTextFormat)
                    SwStyleNameMapper::FillProgName( pTextFormat->GetName(), sStyleName, SwGetPoolIdFromName::TxtColl );
                if ( pParentTextFormat)
                    SwStyleNameMapper::FillProgName( pParentTextFormat->GetName(), sParentStyleName, SwGetPoolIdFromName::TxtColl );

                // This is the default. If the paragraph could not be mapped to
                // any of the standard pdf tags, we write a user defined tag
                // <stylename> with role = P
                nPDFType = vcl::PDFWriter::Paragraph;
                aPDFType = sStyleName;

                // Quotations: BlockQuote

                if (sStyleName == aQuotations)
                {
                    nPDFType = vcl::PDFWriter::BlockQuote;
                    aPDFType = aBlockQuoteString;
                }

                // Caption: Caption

                else if (sStyleName == aCaption)
                {
                    nPDFType = vcl::PDFWriter::Caption;
                    aPDFType = aCaptionString;
                }

                // Caption: Caption

                else if (sParentStyleName == aCaption)
                {
                    nPDFType = vcl::PDFWriter::Caption;
                    aPDFType = sStyleName + aCaptionString;
                }

                // Heading: H

                else if (sStyleName == aHeading)
                {
                    nPDFType = vcl::PDFWriter::Heading;
                    aPDFType = aHString;
                }

                // Heading: H1 - H6

                if (int nRealLevel = pTextNd->GetAttrOutlineLevel() - 1;
                    nRealLevel >= 0
                    && !pTextNd->IsInRedlines()
                    && sw::IsParaPropsNode(*pFrame->getRootFrame(), *pTextNd))
                {
                    switch(nRealLevel)
                    {
                        case 0 :
                            aPDFType = aH1String;
                            break;
                        case 1 :
                            aPDFType = aH2String;
                            break;
                        case 2 :
                            aPDFType = aH3String;
                            break;
                        case 3 :
                            aPDFType = aH4String;
                            break;
                        case 4 :
                            aPDFType = aH5String;
                            break;
                        case 5:
                            aPDFType = aH6String;
                            break;
                        case 6:
                            aPDFType = aH7String;
                            break;
                        case 7:
                            aPDFType = aH8String;
                            break;
                        case 8:
                            aPDFType = aH9String;
                            break;
                        case 9:
                            aPDFType = aH10String;
                            break;
                        default:
                            assert(false);
                            break;
                    }

                    // PDF/UA allows unlimited headings, but PDF only up to H6
                    // ... and apparently the extra H7.. must be declared in
                    // RoleMap, or veraPDF complains.
                    nRealLevel = std::min(nRealLevel, 5);
                    nPDFType =  o3tl::narrowing<sal_uInt16>(vcl::PDFWriter::H1 + nRealLevel);
                }

                // Section: TOCI

                else if ( pFrame->IsInSct() )
                {
                    const SwSectionFrame* pSctFrame = pFrame->FindSctFrame();
                    const SwSection* pSection = pSctFrame->GetSection();

                    if ( SectionType::ToxContent == pSection->GetType() )
                    {
                        const SwTOXBase* pTOXBase = pSection->GetTOXBase();
                        if ( pTOXBase && TOX_INDEX != pTOXBase->GetType() )
                        {
                            // Special case: Open additional TOCI tag:
                            BeginTagImpl(nullptr, vcl::PDFWriter::TOCI, aTOCIString);
                        }
                    }
                }
            }
            break;

        case SwFrameType::Tab :

            // TabFrame: Table

            nPDFType = vcl::PDFWriter::Table;
            aPDFType = aTableString;

            {
                // set up table column data:
                const SwTabFrame* pTabFrame = static_cast<const SwTabFrame*>(pFrame);
                const SwTable* pTable = pTabFrame->GetTable();

                TableColumnsMap& rTableColumnsMap(mpPDFExtOutDevData->GetSwPDFState()->m_TableColumnsMap);
                const TableColumnsMap::const_iterator aIter = rTableColumnsMap.find( pTable );

                if ( aIter == rTableColumnsMap.end() )
                {
                    SwRectFnSet aRectFnSet(pTabFrame);
                    TableColumnsMapEntry& rCols = rTableColumnsMap[ pTable ];

                    const SwTabFrame* pMasterFrame = pTabFrame->IsFollow() ? pTabFrame->FindMaster( true ) : pTabFrame;

                    while ( pMasterFrame )
                    {
                        const SwRowFrame* pRowFrame = static_cast<const SwRowFrame*>(pMasterFrame->GetLower());

                        while ( pRowFrame )
                        {
                            const SwFrame* pCellFrame = pRowFrame->GetLower();

                            const tools::Long nLeft  = aRectFnSet.GetLeft(pCellFrame->getFrameArea());
                            rCols.insert( nLeft );

                            while ( pCellFrame )
                            {
                                const tools::Long nRight = aRectFnSet.GetRight(pCellFrame->getFrameArea());
                                rCols.insert( nRight );
                                pCellFrame = pCellFrame->GetNext();
                            }
                            pRowFrame = static_cast<const SwRowFrame*>(pRowFrame->GetNext());
                        }
                        pMasterFrame = pMasterFrame->GetFollow();
                    }
                }
            }

            break;

        /*
         * TABLE ELEMENTS
         */

        case SwFrameType::Row :

            // RowFrame: TR

            if ( !static_cast<const SwRowFrame*>(pFrame)->IsRepeatedHeadline() )
            {
                nPDFType = vcl::PDFWriter::TableRow;
                aPDFType = aTRString;
            }
            else
            {
                nPDFType = vcl::PDFWriter::NonStructElement;
            }
            break;

        case SwFrameType::Cell :

            // CellFrame: TH, TD

            {
                const SwTabFrame* pTable = static_cast<const SwCellFrame*>(pFrame)->FindTabFrame();
                if ( pTable->IsInHeadline( *pFrame ) || lcl_IsHeadlineCell( *static_cast<const SwCellFrame*>(pFrame) ) )
                {
                    nPDFType = vcl::PDFWriter::TableHeader;
                    aPDFType = aTHString;
                }
                else
                {
                    nPDFType = vcl::PDFWriter::TableData;
                    aPDFType = aTDString;
                }
            }
            break;

        /*
         * ILLUSTRATION
         */

        case SwFrameType::Fly :

            // FlyFrame: Figure, Formula, Control
            // fly in content or fly at page
            if (mpFrameInfo->m_isLink)
            {   // tdf#154939 additional inner link element for flys
                nPDFType = vcl::PDFWriter::Link;
                aPDFType = aLinkString;
            }
            else
            {
                const SwFlyFrame* pFly = static_cast<const SwFlyFrame*>(pFrame);
                if (pFly->GetAnchorFrame()->FindFooterOrHeader() != nullptr
                    || pFly->GetFrameFormat()->GetAttrSet().Get(RES_DECORATIVE).GetValue())
                {
                    nPDFType = vcl::PDFWriter::NonStructElement;
                }
                else if (pFly->Lower() && pFly->Lower()->IsNoTextFrame())
                {
                    bool bFormula = false;

                    const SwNoTextFrame* pNoTextFrame = static_cast<const SwNoTextFrame*>(pFly->Lower());
                    SwOLENode* pOLENd = const_cast<SwOLENode*>(pNoTextFrame->GetNode()->GetOLENode());
                    if ( pOLENd )
                    {
                        SwOLEObj& aOLEObj = pOLENd->GetOLEObj();
                        uno::Reference< embed::XEmbeddedObject > aRef = aOLEObj.GetOleRef();
                        if ( aRef.is() )
                        {
                            bFormula = 0 != SotExchange::IsMath( SvGlobalName( aRef->getClassID() ) );
                        }
                    }
                    if ( bFormula )
                    {
                        nPDFType = vcl::PDFWriter::Formula;
                        aPDFType = aFormulaString;
                    }
                    else
                    {
                        nPDFType = vcl::PDFWriter::Figure;
                        aPDFType = aFigureString;
                    }
                }
                else
                {
                    nPDFType = vcl::PDFWriter::Division;
                    aPDFType = aDivString;
                }
            }
            break;

        default: break;
    }

    if ( USHRT_MAX != nPDFType )
    {
        BeginTag( static_cast<vcl::PDFWriter::StructElement>(nPDFType), aPDFType );
    }
}

void SwTaggedPDFHelper::EndStructureElements()
{
    if (mpFrameInfo != nullptr)
    {
        if (mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentSpan)
        {   // close span at end of paragraph
            mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentSpan.reset();
            ++m_nEndStructureElement;
        }
        if (mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentLink)
        {   // close link at end of paragraph
            mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentLink.reset();
            ++m_nEndStructureElement;
        }
    }

    while ( m_nEndStructureElement > 0 )
    {
        EndTag();
        --m_nEndStructureElement;
    }

    CheckRestoreTag();
}

void SwTaggedPDFHelper::EndCurrentLink(OutputDevice const& rOut)
{
    vcl::PDFExtOutDevData *const pPDFExtOutDevData(
        dynamic_cast<vcl::PDFExtOutDevData *>(rOut.GetExtOutDevData()));
    if (pPDFExtOutDevData && pPDFExtOutDevData->GetSwPDFState()->m_oCurrentLink)
    {
        pPDFExtOutDevData->GetSwPDFState()->m_oCurrentLink.reset();
        pPDFExtOutDevData->EndStructureElement();
#if OSL_DEBUG_LEVEL > 1
    aStructStack.pop_back();
#endif
    }
}

void SwTaggedPDFHelper::EndCurrentAll()
{
    if (mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentSpan)
    {
        mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentSpan.reset();
    }
    if (mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentLink)
    {
        mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentLink.reset();
    }
}

void SwTaggedPDFHelper::EndCurrentSpan()
{
    mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentSpan.reset();
    EndTag(); // close span
}

void SwTaggedPDFHelper::CreateCurrentSpan(
        SwTextPaintInfo const& rInf, OUString const& rStyleName)
{
    assert(!mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentSpan);
    mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentSpan.emplace(
        SwEnhancedPDFState::Span{
            rInf.GetFont()->GetUnderline(),
            rInf.GetFont()->GetOverline(),
            rInf.GetFont()->GetStrikeout(),
            rInf.GetFont()->GetEmphasisMark(),
            rInf.GetFont()->GetEscapement(),
            rInf.GetFont()->GetActual(),
            rInf.GetFont()->GetLanguage(),
            rStyleName});
    // leave it open to let next portion decide to merge or close
    --m_nEndStructureElement;
}

bool SwTaggedPDFHelper::CheckContinueSpan(
        SwTextPaintInfo const& rInf, std::u16string_view const rStyleName,
        SwTextAttr const*const pInetFormatAttr)
{
    // for now, don't create span inside of link - this should be very rare
    // situation and it looks complicated to implement.
    assert(!mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentSpan
        || !mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentLink);
    if (mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentLink)
    {
        if (pInetFormatAttr && pInetFormatAttr == *mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentLink)
        {
            return true;
        }
        else
        {
            mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentLink.reset();
            EndTag();
            return false;
        }
    }
    if (mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentSpan && pInetFormatAttr)
    {
        EndCurrentSpan();
        return false;
    }

    if (!mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentSpan)
        return false;

    SwEnhancedPDFState::Span const& rCurrent(*mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentSpan);

    bool const ret(rCurrent.eUnderline == rInf.GetFont()->GetUnderline()
                && rCurrent.eOverline == rInf.GetFont()->GetOverline()
                && rCurrent.eStrikeout == rInf.GetFont()->GetStrikeout()
                && rCurrent.eFontEmphasis == rInf.GetFont()->GetEmphasisMark()
                && rCurrent.nEscapement == rInf.GetFont()->GetEscapement()
                && rCurrent.nScript == rInf.GetFont()->GetActual()
                && rCurrent.nLang == rInf.GetFont()->GetLanguage()
                && rCurrent.StyleName == rStyleName);
    if (!ret)
    {
        EndCurrentSpan();
    }
    return ret;
}

void SwTaggedPDFHelper::BeginInlineStructureElements()
{
    const SwLinePortion* pPor = &mpPorInfo->mrPor;
    const SwTextPaintInfo& rInf = mpPorInfo->mrTextPainter.GetInfo();
    const SwTextFrame* pFrame = rInf.GetTextFrame();

    // Lowers of NonStructureElements should not be considered:

    if ( lcl_IsInNonStructEnv( *pFrame ) )
        return;

    std::pair<SwTextNode const*, sal_Int32> const pos(
            pFrame->MapViewToModel(rInf.GetIdx()));
    SwTextAttr const*const pInetFormatAttr =
        pos.first->GetTextAttrAt(pos.second, RES_TXTATR_INETFMT);

    OUString sStyleName;
    if (!pInetFormatAttr)
    {
        std::vector<SwTextAttr *> const charAttrs(
            pos.first->GetTextAttrsAt(pos.second, RES_TXTATR_CHARFMT));
        // TODO: handle more than 1 char style?
        const SwCharFormat* pCharFormat = (charAttrs.size())
            ? (*charAttrs.begin())->GetCharFormat().GetCharFormat() : nullptr;
        if (pCharFormat)
            SwStyleNameMapper::FillProgName( pCharFormat->GetName(), sStyleName, SwGetPoolIdFromName::TxtColl );
    }

    // note: ILSE may be nested, so only end the span if needed to start new one
    bool const isContinueSpan(CheckContinueSpan(rInf, sStyleName, pInetFormatAttr));

    sal_uInt16 nPDFType = USHRT_MAX;
    OUString aPDFType;

    switch ( pPor->GetWhichPor() )
    {
        case PortionType::Hyphen :
        case PortionType::SoftHyphen :
        // Check for alternative spelling:
        case PortionType::HyphenStr :
        case PortionType::SoftHyphenStr :
            nPDFType = vcl::PDFWriter::Span;
            aPDFType = aSpanString;
            break;

        case PortionType::Fly:
            // if a link is split by a fly overlap, then there will be multiple
            // annotations for the link, and hence there must be multiple SEs,
            // so every annotation has its own SE.
            if (mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentLink)
            {
                mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentLink.reset();
                EndTag();
            }
            break;

        case PortionType::Lay :
        case PortionType::Text :
        case PortionType::Para :
            {
                // Check for Link:
                if( pInetFormatAttr )
                {
                    if (!isContinueSpan)
                    {
                        nPDFType = vcl::PDFWriter::Link;
                        aPDFType = aLinkString;
                        assert(!mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentLink);
                        mpPDFExtOutDevData->GetSwPDFState()->m_oCurrentLink.emplace(pInetFormatAttr);
                        // leave it open to let next portion decide to merge or close
                        --m_nEndStructureElement;
                    }
                }
                // Check for Quote/Code character style:
                else if (sStyleName == aQuotation)
                {
                    if (!isContinueSpan)
                    {
                        nPDFType = vcl::PDFWriter::Quote;
                        aPDFType = aQuoteString;
                        CreateCurrentSpan(rInf, sStyleName);
                    }
                }
                else if (sStyleName == aSourceText)
                {
                    if (!isContinueSpan)
                    {
                        nPDFType = vcl::PDFWriter::Code;
                        aPDFType = aCodeString;
                        CreateCurrentSpan(rInf, sStyleName);
                    }
                }
                else if (!isContinueSpan)
                {
                    const LanguageType nCurrentLanguage = rInf.GetFont()->GetLanguage();
                    const SwFontScript nFont = rInf.GetFont()->GetActual();
                    const LanguageType nDefaultLang(mpPDFExtOutDevData->GetSwPDFState()->m_eLanguageDefault);

                    if ( LINESTYLE_NONE    != rInf.GetFont()->GetUnderline() ||
                         LINESTYLE_NONE    != rInf.GetFont()->GetOverline()  ||
                         STRIKEOUT_NONE    != rInf.GetFont()->GetStrikeout() ||
                         FontEmphasisMark::NONE != rInf.GetFont()->GetEmphasisMark() ||
                         0                 != rInf.GetFont()->GetEscapement() ||
                         SwFontScript::Latin != nFont ||
                         nCurrentLanguage  != nDefaultLang ||
                         !sStyleName.isEmpty())
                    {
                        nPDFType = vcl::PDFWriter::Span;
                        if (!sStyleName.isEmpty())
                            aPDFType = sStyleName;
                        else
                            aPDFType = aSpanString;
                        CreateCurrentSpan(rInf, sStyleName);
                    }
                }
            }
            break;

        case PortionType::Footnote :
            nPDFType = vcl::PDFWriter::Link;
            aPDFType = aLinkString;
            break;

        case PortionType::Field :
            {
                // check field type:
                TextFrameIndex const nIdx = static_cast<const SwFieldPortion*>(pPor)->IsFollow()
                        ? rInf.GetIdx() - TextFrameIndex(1)
                        : rInf.GetIdx();
                const SwTextAttr* pHint = mpPorInfo->mrTextPainter.GetAttr( nIdx );
                if ( pHint && RES_TXTATR_FIELD == pHint->Which() )
                {
                    const SwField* pField = pHint->GetFormatField().GetField();
                    if ( SwFieldIds::GetRef == pField->Which() )
                    {
                        nPDFType = vcl::PDFWriter::Link;
                        aPDFType = aLinkString;
                    }
                    else if ( SwFieldIds::TableOfAuthorities == pField->Which() )
                    {
                        nPDFType = vcl::PDFWriter::BibEntry;
                        aPDFType = aBibEntryString;
                    }
                }
            }
            break;

        case PortionType::Multi:
            {
                SwMultiPortion const*const pMulti(static_cast<SwMultiPortion const*>(pPor));
                if (pMulti->IsRuby())
                {
                    EndCurrentAll();
                    switch (mpPorInfo->m_Mode)
                    {
                        case 0:
                            nPDFType = vcl::PDFWriter::Ruby;
                            aPDFType = "Ruby";
                        break;
                        case 1:
                            nPDFType = vcl::PDFWriter::RT;
                            aPDFType = "RT";
                        break;
                        case 2:
                            nPDFType = vcl::PDFWriter::RB;
                            aPDFType = "RB";
                        break;
                    }
                }
                else if (pMulti->IsDouble())
                {
                    EndCurrentAll();
                    switch (mpPorInfo->m_Mode)
                    {
                        case 0:
                            nPDFType = vcl::PDFWriter::Warichu;
                            aPDFType = "Warichu";
                        break;
                        case 1:
                            nPDFType = vcl::PDFWriter::WP;
                            aPDFType = "WP";
                        break;
                        case 2:
                            nPDFType = vcl::PDFWriter::WT;
                            aPDFType = "WT";
                        break;
                    }
                }
            }
            break;


        // for FootnoteNum, is called twice: outer generates Lbl, inner Link
        case PortionType::FootnoteNum:
            assert(!isContinueSpan); // is at start
            if (mpPorInfo->m_Mode == 0)
            {   // tdf#152218 link both directions
                nPDFType = vcl::PDFWriter::Link;
                aPDFType = aLinkString;
                break;
            }
            [[fallthrough]];
        case PortionType::Number:
        case PortionType::Bullet:
        case PortionType::GrfNum:
            assert(!isContinueSpan); // is at start
            if (mpPorInfo->m_Mode == 1)
            {   // only works for multiple lines via wrapper from PaintSwFrame
                nPDFType = vcl::PDFWriter::LILabel;
                aPDFType = aListLabelString;
            }
            break;

        case PortionType::Tab :
        case PortionType::TabRight :
        case PortionType::TabCenter :
        case PortionType::TabDecimal :
            nPDFType = vcl::PDFWriter::NonStructElement;
            break;
        default: break;
    }

    if ( USHRT_MAX != nPDFType )
    {
        BeginTag( static_cast<vcl::PDFWriter::StructElement>(nPDFType), aPDFType );
    }
}

bool SwTaggedPDFHelper::IsExportTaggedPDF( const OutputDevice& rOut )
{
    vcl::PDFExtOutDevData* pPDFExtOutDevData = dynamic_cast< vcl::PDFExtOutDevData*>( rOut.GetExtOutDevData() );
    return pPDFExtOutDevData && pPDFExtOutDevData->GetIsExportTaggedPDF();
}

SwEnhancedPDFExportHelper::SwEnhancedPDFExportHelper( SwEditShell& rSh,
                                                      OutputDevice& rOut,
                                                      const OUString& rPageRange,
                                                      bool bSkipEmptyPages,
                                                      bool bEditEngineOnly,
                                                      const SwPrintData& rPrintData )
    : mrSh( rSh ),
      mrOut( rOut ),
      mbSkipEmptyPages( bSkipEmptyPages ),
      mbEditEngineOnly( bEditEngineOnly ),
      mrPrintData( rPrintData )
{
    if ( !rPageRange.isEmpty() )
        mpRangeEnum.reset( new StringRangeEnumerator( rPageRange, 0, mrSh.GetPageCount()-1 ) );

    if ( mbSkipEmptyPages )
    {
        maPageNumberMap.resize( mrSh.GetPageCount() );
        const SwPageFrame* pCurrPage =
            static_cast<const SwPageFrame*>( mrSh.GetLayout()->Lower() );
        sal_Int32 nPageNumber = 0;
        for ( size_t i = 0, n = maPageNumberMap.size(); i < n && pCurrPage; ++i )
        {
            if ( pCurrPage->IsEmptyPage() )
                maPageNumberMap[i] = -1;
            else
                maPageNumberMap[i] = nPageNumber++;

            pCurrPage = static_cast<const SwPageFrame*>( pCurrPage->GetNext() );
        }
    }

#if OSL_DEBUG_LEVEL > 1
    aStructStack.clear();
#endif

    const sal_Int16 nScript = SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage() );
    TypedWhichId<SvxLanguageItem> nLangRes = RES_CHRATR_LANGUAGE;

    if ( i18n::ScriptType::ASIAN == nScript )
        nLangRes = RES_CHRATR_CJK_LANGUAGE;
    else if ( i18n::ScriptType::COMPLEX == nScript )
        nLangRes = RES_CHRATR_CTL_LANGUAGE;

    const SvxLanguageItem& rLangItem = mrSh.GetDoc()->GetDefault( nLangRes );
    auto const eLanguageDefault = rLangItem.GetLanguage();

    EnhancedPDFExport(eLanguageDefault);
}

SwEnhancedPDFExportHelper::~SwEnhancedPDFExportHelper()
{
}

tools::Rectangle SwEnhancedPDFExportHelper::SwRectToPDFRect(const SwPageFrame* pCurrPage,
    const tools::Rectangle& rRectangle) const
{
    if (!::sw::IsShrinkPageForPostIts(mrSh, mrPrintData)) // tdf#148729
    {
        return rRectangle;
    }
    //the page has been scaled by 75% and vertically centered, so adjust these
    //rectangles equivalently
    tools::Rectangle aRect(rRectangle);
    Size aRectSize(aRect.GetSize());
    double fScale = 0.75;
    aRectSize.setWidth( aRectSize.Width() * fScale );
    aRectSize.setHeight( aRectSize.Height() * fScale );
    tools::Long nOrigHeight = pCurrPage->getFrameArea().Height();
    tools::Long nNewHeight = nOrigHeight*fScale;
    tools::Long nShiftY = (nOrigHeight-nNewHeight)/2;
    aRect.SetLeft( aRect.Left() * fScale );
    aRect.SetTop( aRect.Top() * fScale );
    aRect.Move(0, nShiftY);
    aRect.SetSize(aRectSize);
    return aRect;
}

void SwEnhancedPDFExportHelper::EnhancedPDFExport(LanguageType const eLanguageDefault)
{
    vcl::PDFExtOutDevData* pPDFExtOutDevData =
        dynamic_cast< vcl::PDFExtOutDevData*>( mrOut.GetExtOutDevData() );

    if ( !pPDFExtOutDevData )
        return;

    // set the document locale

    lang::Locale const aDocLocale( LanguageTag(eLanguageDefault).getLocale() );
    pPDFExtOutDevData->SetDocumentLocale( aDocLocale );

    // Prepare the output device:

    mrOut.Push( vcl::PushFlags::MAPMODE );
    MapMode aMapMode( mrOut.GetMapMode() );
    aMapMode.SetMapUnit( MapUnit::MapTwip );
    mrOut.SetMapMode( aMapMode );

    // Create new cursor and lock the view:

    SwDoc* pDoc = mrSh.GetDoc();
    mrSh.SwCursorShell::Push();
    mrSh.SwCursorShell::ClearMark();
    const bool bOldLockView = mrSh.IsViewLocked();
    mrSh.LockView( true );

    if ( !mbEditEngineOnly )
    {
        assert(pPDFExtOutDevData->GetSwPDFState() == nullptr);
        pPDFExtOutDevData->SetSwPDFState(new SwEnhancedPDFState(eLanguageDefault));

        // POSTITS

        if ( pPDFExtOutDevData->GetIsExportNotes() )
        {
            std::vector<SwFormatField*> vpFields;
            mrSh.GetFieldType(SwFieldIds::Postit, OUString())->GatherFields(vpFields);
            for(auto pFormatField : vpFields)
            {
                const SwTextNode* pTNd = pFormatField->GetTextField()->GetpTextNode();
                OSL_ENSURE(nullptr != pTNd, "Enhanced pdf export - text node is missing");
                if(!lcl_TryMoveToNonHiddenField(mrSh, *pTNd, *pFormatField))
                    continue;
                // Link Rectangle
                const SwRect& rNoteRect = mrSh.GetCharRect();
                const SwPageFrame* pCurrPage = static_cast<const SwPageFrame*>(mrSh.GetLayout()->Lower());

                // Link PageNums
                std::vector<sal_Int32> aNotePageNums = CalcOutputPageNums(rNoteRect);
                for (sal_Int32 aNotePageNum : aNotePageNums)
                {

                    // Use the NumberFormatter to get the date string:
                    const SwPostItField* pField = static_cast<SwPostItField*>(pFormatField->GetField());
                    SvNumberFormatter* pNumFormatter = pDoc->GetNumberFormatter();
                    const Date aDateDiff(pField->GetDate() - pNumFormatter->GetNullDate());
                    const sal_uLong nFormat = pNumFormatter->GetStandardFormat(SvNumFormatType::DATE, pField->GetLanguage());
                    OUString sDate;
                    const Color* pColor;
                    pNumFormatter->GetOutputString(aDateDiff.GetDate(), nFormat, sDate, &pColor);

                    vcl::pdf::PDFNote aNote;
                    // The title should consist of the author and the date:
                    aNote.maTitle = pField->GetPar1() + ", " + sDate + ", " + (pField->GetResolved() ? SwResId(STR_RESOLVED) : "");
                    // Guess what the contents contains...
                    aNote.maContents = pField->GetText();

                    // Link Export
                    tools::Rectangle aRect(SwRectToPDFRect(pCurrPage, rNoteRect.SVRect()));
                    pPDFExtOutDevData->CreateNote(aRect, aNote, aNotePageNum);
                }
                mrSh.SwCursorShell::ClearMark();
            }
        }

        // HYPERLINKS

        SwGetINetAttrs aArr;
        mrSh.GetINetAttrs( aArr );
        for( auto &rAttr : aArr )
        {
            SwGetINetAttr* p = &rAttr;
            OSL_ENSURE( nullptr != p, "Enhanced pdf export - SwGetINetAttr is missing" );

            const SwTextNode* pTNd = p->rINetAttr.GetpTextNode();
            OSL_ENSURE( nullptr != pTNd, "Enhanced pdf export - text node is missing" );

            // 1. Check if the whole paragraph is hidden
            // 2. Move to the hyperlink
            // 3. Check for hidden text attribute
            if ( !pTNd->IsHidden() &&
                  mrSh.GotoINetAttr( p->rINetAttr ) &&
                 !mrSh.IsInHiddenRange(/*bSelect=*/false) )
            {
                // Select the hyperlink:
                mrSh.SwCursorShell::Right( 1, SwCursorSkipMode::Chars );
                if ( mrSh.SwCursorShell::SelectTextAttr( RES_TXTATR_INETFMT, true ) )
                {
                    // First, we create the destination, because there may be more
                    // than one link to this destination:
                    OUString aURL( INetURLObject::decode(
                        p->rINetAttr.GetINetFormat().GetValue(),
                        INetURLObject::DecodeMechanism::Unambiguous ) );

                    // We have to distinguish between internal and real URLs
                    const bool bInternal = '#' == aURL[0];

                    // GetCursor_() is a SwShellCursor, which is derived from
                    // SwSelPaintRects, therefore the rectangles of the current
                    // selection can be easily obtained:
                    // Note: We make a copy of the rectangles, because they may
                    // be deleted again in JumpToSwMark.
                    SwRects const aTmp(GetCursorRectsContainingText(mrSh));
                    OSL_ENSURE( !aTmp.empty(), "Enhanced pdf export - rectangles are missing" );
                    OUString const altText(mrSh.GetSelText());

                    const SwPageFrame* pSelectionPage =
                        static_cast<const SwPageFrame*>( mrSh.GetLayout()->Lower() );

                    // Create the destination for internal links:
                    sal_Int32 nDestId = -1;
                    if ( bInternal )
                    {
                        aURL = aURL.copy( 1 );
                        mrSh.SwCursorShell::ClearMark();
                        if (! JumpToSwMark( &mrSh, aURL ))
                        {
                            continue; // target deleted
                        }

                        // Destination Rectangle
                        const SwRect& rDestRect = mrSh.GetCharRect();

                        const SwPageFrame* pCurrPage =
                            static_cast<const SwPageFrame*>( mrSh.GetLayout()->Lower() );

                        // Destination PageNum
                        const sal_Int32 nDestPageNum = CalcOutputPageNum( rDestRect );

                        // Destination Export
                        if ( -1 != nDestPageNum )
                        {
                            tools::Rectangle aRect(SwRectToPDFRect(pCurrPage, rDestRect.SVRect()));
                            nDestId = pPDFExtOutDevData->CreateDest(aRect, nDestPageNum);
                        }
                    }

                    if ( !bInternal || -1 != nDestId )
                    {
                        // #i44368# Links in Header/Footer
                        const bool bHeaderFooter = pDoc->IsInHeaderFooter( *pTNd );

                        // Create links for all selected rectangles:
                        const size_t nNumOfRects = aTmp.size();
                        for ( size_t i = 0; i < nNumOfRects; ++i )
                        {
                            // Link Rectangle
                            const SwRect& rLinkRect( aTmp[ i ] );

                            // Link PageNums
                            std::vector<sal_Int32> aLinkPageNums = CalcOutputPageNums( rLinkRect );

                            for (sal_Int32 aLinkPageNum : aLinkPageNums)
                            {
                                // Link Export
                                tools::Rectangle aRect(SwRectToPDFRect(pSelectionPage, rLinkRect.SVRect()));
                                const sal_Int32 nLinkId =
                                    pPDFExtOutDevData->CreateLink(aRect, altText, aLinkPageNum);

                                // Store link info for tagged pdf output:
                                const IdMapEntry aLinkEntry( rLinkRect, nLinkId );
                                pPDFExtOutDevData->GetSwPDFState()->m_LinkIdMap.push_back(aLinkEntry);

                                // Connect Link and Destination:
                                if ( bInternal )
                                    pPDFExtOutDevData->SetLinkDest( nLinkId, nDestId );
                                else
                                    pPDFExtOutDevData->SetLinkURL( nLinkId, aURL );

                                // #i44368# Links in Header/Footer
                                if ( bHeaderFooter )
                                    MakeHeaderFooterLinks(*pPDFExtOutDevData, *pTNd, rLinkRect, nDestId, aURL, bInternal, altText);
                            }
                        }
                    }
                }
            }
            mrSh.SwCursorShell::ClearMark();
        }

        // HYPERLINKS (Graphics, Frames, OLEs )

        for(sw::SpzFrameFormat* pFrameFormat: *pDoc->GetSpzFrameFormats())
        {
            const SwFormatURL* pItem;
            if ( RES_DRAWFRMFMT != pFrameFormat->Which() &&
                GetFrameOfModify(mrSh.GetLayout(), *pFrameFormat, SwFrameType::Fly) &&
                 (pItem = pFrameFormat->GetAttrSet().GetItemIfSet( RES_URL )) )
            {
                const SwPageFrame* pCurrPage =
                    static_cast<const SwPageFrame*>( mrSh.GetLayout()->Lower() );

                OUString aURL( pItem->GetURL() );
                if (aURL.isEmpty())
                    continue;
                const bool bInternal = '#' == aURL[0];

                // Create the destination for internal links:
                sal_Int32 nDestId = -1;
                if ( bInternal )
                {
                    aURL = aURL.copy( 1 );
                    mrSh.SwCursorShell::ClearMark();
                    if (! JumpToSwMark( &mrSh, aURL ))
                    {
                        continue; // target deleted
                    }

                    // Destination Rectangle
                    const SwRect& rDestRect = mrSh.GetCharRect();

                    pCurrPage = static_cast<const SwPageFrame*>( mrSh.GetLayout()->Lower() );

                    // Destination PageNum
                    const sal_Int32 nDestPageNum = CalcOutputPageNum( rDestRect );

                    // Destination Export
                    if ( -1 != nDestPageNum )
                    {
                        tools::Rectangle aRect(SwRectToPDFRect(pCurrPage, rDestRect.SVRect()));
                        nDestId = pPDFExtOutDevData->CreateDest(aRect, nDestPageNum);
                    }
                }

                if ( !bInternal || -1 != nDestId )
                {
                    Point aNullPt;
                    const SwRect aLinkRect = pFrameFormat->FindLayoutRect( false, &aNullPt );
                    OUString const formatName(pFrameFormat->GetName());
                    // Link PageNums
                    std::vector<sal_Int32> aLinkPageNums = CalcOutputPageNums( aLinkRect );

                    // Link Export
                    for (sal_Int32 aLinkPageNum : aLinkPageNums)
                    {
                        tools::Rectangle aRect(SwRectToPDFRect(pCurrPage, aLinkRect.SVRect()));
                        const sal_Int32 nLinkId =
                            pPDFExtOutDevData->CreateLink(aRect, formatName, aLinkPageNum);

                        // Store link info for tagged pdf output:
                        const IdMapEntry aLinkEntry(aLinkRect, nLinkId);
                        pPDFExtOutDevData->GetSwPDFState()->m_LinkIdMap.push_back(aLinkEntry);

                        // Connect Link and Destination:
                        if ( bInternal )
                            pPDFExtOutDevData->SetLinkDest( nLinkId, nDestId );
                        else
                            pPDFExtOutDevData->SetLinkURL( nLinkId, aURL );

                        // #i44368# Links in Header/Footer
                        const SwFormatAnchor &rAnch = pFrameFormat->GetAnchor();
                        if (RndStdIds::FLY_AT_PAGE != rAnch.GetAnchorId())
                        {
                            const SwNode* pAnchorNode = rAnch.GetAnchorNode();
                            if ( pAnchorNode && pDoc->IsInHeaderFooter( *pAnchorNode ) )
                            {
                                const SwTextNode* pTNd = pAnchorNode->GetTextNode();
                                if ( pTNd )
                                    MakeHeaderFooterLinks(*pPDFExtOutDevData, *pTNd, aLinkRect, nDestId, aURL, bInternal, formatName);
                            }
                        }
                    }
                }
            }
            else if (pFrameFormat->Which() == RES_DRAWFRMFMT)
            {
                // Turn media shapes into Screen annotations.
                if (SdrObject* pObject = pFrameFormat->FindRealSdrObject())
                {
                    SwRect aSnapRect(pObject->GetSnapRect());
                    std::vector<sal_Int32> aScreenPageNums = CalcOutputPageNums(aSnapRect);
                    if (aScreenPageNums.empty())
                        continue;

                    uno::Reference<drawing::XShape> xShape(pObject->getUnoShape(), uno::UNO_QUERY);
                    if (xShape->getShapeType() == "com.sun.star.drawing.MediaShape")
                    {
                        uno::Reference<beans::XPropertySet> xShapePropSet(xShape, uno::UNO_QUERY);
                        OUString title;
                        xShapePropSet->getPropertyValue("Title") >>= title;
                        OUString description;
                        xShapePropSet->getPropertyValue("Description") >>= description;
                        OUString const altText(title.isEmpty()
                            ? description
                            : description.isEmpty()
                                ? title
                                : OUString::Concat(title) + OUString::Concat("\n") + OUString::Concat(description));

                        OUString aMediaURL;
                        xShapePropSet->getPropertyValue("MediaURL") >>= aMediaURL;
                        if (!aMediaURL.isEmpty())
                        {
                            OUString const mimeType(xShapePropSet->getPropertyValue("MediaMimeType").get<OUString>());
                            const SwPageFrame* pCurrPage = mrSh.GetLayout()->GetPageAtPos(aSnapRect.Center());
                            tools::Rectangle aPDFRect(SwRectToPDFRect(pCurrPage, aSnapRect.SVRect()));
                            for (sal_Int32 nScreenPageNum : aScreenPageNums)
                            {
                                sal_Int32 nScreenId = pPDFExtOutDevData->CreateScreen(aPDFRect, altText, mimeType, nScreenPageNum, pObject);
                                if (aMediaURL.startsWith("vnd.sun.star.Package:"))
                                {
                                    // Embedded media.
                                    OUString aTempFileURL;
                                    xShapePropSet->getPropertyValue("PrivateTempFileURL") >>= aTempFileURL;
                                    pPDFExtOutDevData->SetScreenStream(nScreenId, aTempFileURL);
                                }
                                else
                                    // Linked media.
                                    pPDFExtOutDevData->SetScreenURL(nScreenId, aMediaURL);
                            }
                        }
                    }
                }
            }
            mrSh.SwCursorShell::ClearMark();
        }

        // REFERENCES

        std::vector<SwFormatField*> vpFields;
        mrSh.GetFieldType( SwFieldIds::GetRef, OUString() )->GatherFields(vpFields);
        for(auto pFormatField : vpFields )
        {
            if( pFormatField->GetTextField() && pFormatField->IsFieldInDoc() )
            {
                const SwTextNode* pTNd = pFormatField->GetTextField()->GetpTextNode();
                OSL_ENSURE( nullptr != pTNd, "Enhanced pdf export - text node is missing" );
                if(!lcl_TryMoveToNonHiddenField(mrSh, *pTNd, *pFormatField))
                    continue;
                // Select the field:
                mrSh.SwCursorShell::SetMark();
                mrSh.SwCursorShell::Right( 1, SwCursorSkipMode::Chars );

                // Link Rectangles
                SwRects const aTmp(GetCursorRectsContainingText(mrSh));
                OSL_ENSURE( !aTmp.empty(), "Enhanced pdf export - rectangles are missing" );

                mrSh.SwCursorShell::ClearMark();

                // Destination Rectangle
                const SwGetRefField* pField = static_cast<SwGetRefField*>(pFormatField->GetField());
                const OUString& rRefName = pField->GetSetRefName();
                mrSh.GotoRefMark( rRefName, pField->GetSubType(), pField->GetSeqNo(), pField->GetFlags() );
                const SwRect& rDestRect = mrSh.GetCharRect();

                const SwPageFrame* pCurrPage = static_cast<const SwPageFrame*>( mrSh.GetLayout()->Lower() );

                // Destination PageNum
                const sal_Int32 nDestPageNum = CalcOutputPageNum( rDestRect );

                if ( -1 != nDestPageNum )
                {
                    // Destination Export
                    tools::Rectangle aRect(SwRectToPDFRect(pCurrPage, rDestRect.SVRect()));
                    const sal_Int32 nDestId = pPDFExtOutDevData->CreateDest(aRect, nDestPageNum);

                    // #i44368# Links in Header/Footer
                    const bool bHeaderFooter = pDoc->IsInHeaderFooter( *pTNd );
                    OUString const content(pField->ExpandField(true, mrSh.GetLayout()));

                    // Create links for all selected rectangles:
                    const size_t nNumOfRects = aTmp.size();
                    for ( size_t i = 0; i < nNumOfRects; ++i )
                    {
                        // Link rectangle
                        const SwRect& rLinkRect( aTmp[ i ] );

                        // Link PageNums
                        std::vector<sal_Int32> aLinkPageNums = CalcOutputPageNums( rLinkRect );

                        for (sal_Int32 aLinkPageNum : aLinkPageNums)
                        {
                            // Link Export
                            aRect = SwRectToPDFRect(pCurrPage, rLinkRect.SVRect());
                            const sal_Int32 nLinkId =
                                pPDFExtOutDevData->CreateLink(aRect, content, aLinkPageNum);

                            // Store link info for tagged pdf output:
                            const IdMapEntry aLinkEntry( rLinkRect, nLinkId );
                            pPDFExtOutDevData->GetSwPDFState()->m_LinkIdMap.push_back(aLinkEntry);

                            // Connect Link and Destination:
                            pPDFExtOutDevData->SetLinkDest( nLinkId, nDestId );

                            // #i44368# Links in Header/Footer
                            if ( bHeaderFooter )
                            {
                                MakeHeaderFooterLinks(*pPDFExtOutDevData, *pTNd, rLinkRect, nDestId, "", true, content);
                            }
                        }
                    }
                }
            }
            mrSh.SwCursorShell::ClearMark();
        }

        ExportAuthorityEntryLinks();

        // FOOTNOTES

        const size_t nFootnoteCount = pDoc->GetFootnoteIdxs().size();
        for ( size_t nIdx = 0; nIdx < nFootnoteCount; ++nIdx )
        {
            // Set cursor to text node that contains the footnote:
            const SwTextFootnote* pTextFootnote = pDoc->GetFootnoteIdxs()[ nIdx ];
            SwTextNode& rTNd = const_cast<SwTextNode&>(pTextFootnote->GetTextNode());

            mrSh.GetCursor_()->GetPoint()->Assign(rTNd, pTextFootnote->GetStart());

            // 1. Check if the whole paragraph is hidden
            // 2. Check for hidden text attribute
            if (rTNd.GetTextNode()->IsHidden() || mrSh.IsInHiddenRange(/*bSelect=*/false)
                || (mrSh.GetLayout()->IsHideRedlines()
                    && sw::IsFootnoteDeleted(pDoc->getIDocumentRedlineAccess(), *pTextFootnote)))
            {
                continue;
            }

            SwCursorSaveState aSaveState( *mrSh.GetCursor_() );

            // Select the footnote:
            mrSh.SwCursorShell::SetMark();
            mrSh.SwCursorShell::Right( 1, SwCursorSkipMode::Chars );

            // Link Rectangle
            SwRects aTmp;
            aTmp.insert( aTmp.begin(), mrSh.SwCursorShell::GetCursor_()->begin(), mrSh.SwCursorShell::GetCursor_()->end() );
            OSL_ENSURE( !aTmp.empty(), "Enhanced pdf export - rectangles are missing" );

            mrSh.GetCursor_()->RestoreSavePos();
            mrSh.SwCursorShell::ClearMark();

            if (aTmp.empty())
                continue;

            const SwRect aLinkRect( aTmp[ 0 ] );

            // Goto footnote text:
            if ( mrSh.GotoFootnoteText() )
            {
                // Destination Rectangle
                const SwRect& rDestRect = mrSh.GetCharRect();
                const sal_Int32 nDestPageNum = CalcOutputPageNum( rDestRect );
                if ( -1 != nDestPageNum )
                {
                    const SwPageFrame* pCurrPage = static_cast<const SwPageFrame*>( mrSh.GetLayout()->Lower() );
                    // Destination PageNum
                    tools::Rectangle aRect = SwRectToPDFRect(pCurrPage, rDestRect.SVRect());
                    // Back link rectangle calculation
                    const SwPageFrame* fnBodyPage = pCurrPage->getRootFrame()->GetPageByPageNum(nDestPageNum+1);
                    SwRect fnSymbolRect;
                    if (fnBodyPage->IsVertical()){
                        tools::Long fnSymbolTop = fnBodyPage->GetTopMargin() + fnBodyPage->getFrameArea().Top();
                        tools::Long symbolHeight = rDestRect.Top() - fnSymbolTop;
                        fnSymbolRect = SwRect(rDestRect.Pos().X(),fnSymbolTop,rDestRect.Width(),symbolHeight);
                    } else {
                       if (fnBodyPage->IsRightToLeft()){
                           tools::Long fnSymbolRight = fnBodyPage->getFrameArea().Right() - fnBodyPage->GetRightMargin();
                           tools::Long symbolWidth = fnSymbolRight - rDestRect.Right();
                           fnSymbolRect = SwRect(rDestRect.Pos().X(),rDestRect.Pos().Y(),symbolWidth,rDestRect.Height());
                       } else {
                           tools::Long fnSymbolLeft = fnBodyPage->GetLeftMargin() + fnBodyPage->getFrameArea().Left();
                           tools::Long symbolWidth = rDestRect.Left() - fnSymbolLeft;
                           fnSymbolRect = SwRect(fnSymbolLeft,rDestRect.Pos().Y(),symbolWidth,rDestRect.Height());
                       }
                    }
                    tools::Rectangle aFootnoteSymbolRect = SwRectToPDFRect(pCurrPage, fnSymbolRect.SVRect());

                    OUString const numStrSymbol(pTextFootnote->GetFootnote().GetViewNumStr(*pDoc, mrSh.GetLayout(), true));
                    OUString const numStrRef(pTextFootnote->GetFootnote().GetViewNumStr(*pDoc, mrSh.GetLayout(), false));

                    // Export back link
                    const sal_Int32 nBackLinkId = pPDFExtOutDevData->CreateLink(aFootnoteSymbolRect, numStrSymbol, nDestPageNum);
                    // Destination Export
                    const sal_Int32 nDestId = pPDFExtOutDevData->CreateDest(aRect, nDestPageNum);
                    mrSh.GotoFootnoteAnchor();
                    // Link PageNums
                    sal_Int32 aLinkPageNum = CalcOutputPageNum( aLinkRect );
                    pCurrPage = static_cast<const SwPageFrame*>( mrSh.GetLayout()->Lower() );
                    // Link Export
                    aRect = SwRectToPDFRect(pCurrPage, aLinkRect.SVRect());
                    const sal_Int32 nLinkId = pPDFExtOutDevData->CreateLink(aRect, numStrRef, aLinkPageNum);
                    // Back link destination Export
                    const sal_Int32 nBackDestId = pPDFExtOutDevData->CreateDest(aRect, aLinkPageNum);
                    // Store link info for tagged pdf output:
                    const IdMapEntry aLinkEntry( aLinkRect, nLinkId );
                    pPDFExtOutDevData->GetSwPDFState()->m_LinkIdMap.push_back(aLinkEntry);

                    // Store backlink info for tagged pdf output:
                    const IdMapEntry aBackLinkEntry( aFootnoteSymbolRect, nBackLinkId );
                    pPDFExtOutDevData->GetSwPDFState()->m_LinkIdMap.push_back(aBackLinkEntry);
                    // Connect Links and Destinations:
                    pPDFExtOutDevData->SetLinkDest( nLinkId, nDestId );
                    pPDFExtOutDevData->SetLinkDest( nBackLinkId, nBackDestId );
                }
            }
        }

        // OUTLINE

        if( pPDFExtOutDevData->GetIsExportBookmarks() )
        {
            typedef std::pair< sal_Int8, sal_Int32 > StackEntry;
            std::stack< StackEntry > aOutlineStack;
            aOutlineStack.push( StackEntry( -1, -1 ) ); // push default value

            const SwOutlineNodes::size_type nOutlineCount =
                mrSh.getIDocumentOutlineNodesAccess()->getOutlineNodesCount();
            for ( SwOutlineNodes::size_type i = 0; i < nOutlineCount; ++i )
            {
                // Check if outline is hidden
                const SwTextNode* pTNd = mrSh.GetNodes().GetOutLineNds()[ i ]->GetTextNode();
                OSL_ENSURE( nullptr != pTNd, "Enhanced pdf export - text node is missing" );

                if ( pTNd->IsHidden() ||
                     !sw::IsParaPropsNode(*mrSh.GetLayout(), *pTNd) ||
                     // #i40292# Skip empty outlines:
                     pTNd->GetText().isEmpty())
                    continue;

                // Get parent id from stack:
                const sal_Int8 nLevel = static_cast<sal_Int8>(mrSh.getIDocumentOutlineNodesAccess()->getOutlineLevel( i ));
                sal_Int8 nLevelOnTopOfStack = aOutlineStack.top().first;
                while ( nLevelOnTopOfStack >= nLevel &&
                        nLevelOnTopOfStack != -1 )
                {
                    aOutlineStack.pop();
                    nLevelOnTopOfStack = aOutlineStack.top().first;
                }
                const sal_Int32 nParent = aOutlineStack.top().second;

                // Destination rectangle
                mrSh.GotoOutline(i);
                const SwRect& rDestRect = mrSh.GetCharRect();

                const SwPageFrame* pCurrPage =
                    static_cast<const SwPageFrame*>( mrSh.GetLayout()->Lower() );

                // Destination PageNum
                const sal_Int32 nDestPageNum = CalcOutputPageNum( rDestRect );

                if ( -1 != nDestPageNum )
                {
                    // Destination Export
                    tools::Rectangle aRect(SwRectToPDFRect(pCurrPage, rDestRect.SVRect()));
                    const sal_Int32 nDestId =
                        pPDFExtOutDevData->CreateDest(aRect, nDestPageNum);

                    // Outline entry text
                    const OUString& rEntry = mrSh.getIDocumentOutlineNodesAccess()->getOutlineText(
                        i, mrSh.GetLayout(), true, false, false );

                    // Create a new outline item:
                    const sal_Int32 nOutlineId =
                        pPDFExtOutDevData->CreateOutlineItem( nParent, rEntry, nDestId );

                    // Push current level and nOutlineId on stack:
                    aOutlineStack.push( StackEntry( nLevel, nOutlineId ) );
                }
            }
        }

        if( pPDFExtOutDevData->GetIsExportNamedDestinations() )
        {
            // #i56629# the iteration to convert the OOo bookmark (#bookmark)
            // into PDF named destination, see section 8.2.1 in PDF 1.4 spec
            // We need:
            // 1. a name for the destination, formed from the standard OOo bookmark name
            // 2. the destination, obtained from where the bookmark destination lies
            IDocumentMarkAccess* const pMarkAccess = mrSh.GetDoc()->getIDocumentMarkAccess();
            for(IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getBookmarksBegin();
                ppMark != pMarkAccess->getBookmarksEnd();
                ++ppMark)
            {
                //get the name
                const ::sw::mark::IMark* pBkmk = *ppMark;
                mrSh.SwCursorShell::ClearMark();
                const OUString& sBkName = pBkmk->GetName();

                //jump to it
                if (! JumpToSwMark( &mrSh, sBkName ))
                {
                    continue;
                }

                // Destination Rectangle
                const SwRect& rDestRect = mrSh.GetCharRect();

                const SwPageFrame* pCurrPage =
                    static_cast<const SwPageFrame*>( mrSh.GetLayout()->Lower() );

                // Destination PageNum
                const sal_Int32 nDestPageNum = CalcOutputPageNum( rDestRect );

                // Destination Export
                if ( -1 != nDestPageNum )
                {
                    tools::Rectangle aRect(SwRectToPDFRect(pCurrPage, rDestRect.SVRect()));
                    pPDFExtOutDevData->CreateNamedDest(sBkName, aRect, nDestPageNum);
                }
            }
            mrSh.SwCursorShell::ClearMark();
            //<--- i56629
        }
    }
    else
    {

        // LINKS FROM EDITENGINE

        std::vector< vcl::PDFExtOutDevBookmarkEntry >& rBookmarks = pPDFExtOutDevData->GetBookmarks();
        for ( const auto& rBookmark : rBookmarks )
        {
            OUString aBookmarkName( rBookmark.aBookmark );
            const bool bInternal = '#' == aBookmarkName[0];
            if ( bInternal )
            {
                aBookmarkName = aBookmarkName.copy( 1 );
                JumpToSwMark( &mrSh, aBookmarkName );

                // Destination Rectangle
                const SwRect& rDestRect = mrSh.GetCharRect();

                const SwPageFrame* pCurrPage =
                    static_cast<const SwPageFrame*>( mrSh.GetLayout()->Lower() );

                // Destination PageNum
                const sal_Int32 nDestPageNum = CalcOutputPageNum( rDestRect );

                if ( -1 != nDestPageNum )
                {
                    tools::Rectangle aRect(SwRectToPDFRect(pCurrPage, rDestRect.SVRect()));
                    if ( rBookmark.nLinkId != -1 )
                    {
                        // Destination Export
                        const sal_Int32 nDestId = pPDFExtOutDevData->CreateDest(aRect, nDestPageNum);

                        // Connect Link and Destination:
                        pPDFExtOutDevData->SetLinkDest( rBookmark.nLinkId, nDestId );
                    }
                    else
                    {
                        pPDFExtOutDevData->DescribeRegisteredDest(rBookmark.nDestId, aRect, nDestPageNum);
                    }
                }
            }
            else
                pPDFExtOutDevData->SetLinkURL( rBookmark.nLinkId, aBookmarkName );
        }
        rBookmarks.clear();
        assert(pPDFExtOutDevData->GetSwPDFState());
        delete pPDFExtOutDevData->GetSwPDFState();
        pPDFExtOutDevData->SetSwPDFState(nullptr);
    }

    // Restore view, cursor, and outdev:
    mrSh.LockView( bOldLockView );
    mrSh.SwCursorShell::Pop(SwCursorShell::PopMode::DeleteCurrent);
    mrOut.Pop();
}

void SwEnhancedPDFExportHelper::ExportAuthorityEntryLinks()
{
    auto pPDFExtOutDevData = dynamic_cast<vcl::PDFExtOutDevData*>(mrOut.GetExtOutDevData());
    if (!pPDFExtOutDevData)
    {
        return;
    }

    // Create PDF destinations for bibliography table entries
    std::vector<std::tuple<const SwTOXBase*, const OUString*, sal_Int32>> vDestinations;
    //  string is the row node text, sal_Int32 is number of the destination
    // Note: This way of iterating doesn't seem to take into account TOXes
    //          that are in a frame, probably in some other cases too
    {
        mrSh.GotoPage(1);
        while (mrSh.GotoNextTOXBase())
        {
            const SwTOXBase* pIteratedTOX = nullptr;
            while ((pIteratedTOX = mrSh.GetCurTOX()) != nullptr
                   && pIteratedTOX->GetType() == TOX_AUTHORITIES)
            {
                if (const SwNode& rCurrentNode = mrSh.GetCursor()->GetPoint()->GetNode();
                    rCurrentNode.GetNodeType() == SwNodeType::Text)
                {
                    if (mrSh.GetCursor()->GetPoint()->GetNode().FindSectionNode()->GetSection().GetType()
                        == SectionType::ToxContent) // this checks it's not a heading
                    {
                        // Destination Rectangle
                        const SwRect& rDestRect = mrSh.GetCharRect();

                        const SwPageFrame* pCurrPage =
                            static_cast<const SwPageFrame*>( mrSh.GetLayout()->Lower() );

                        // Destination PageNum
                        const sal_Int32 nDestPageNum = CalcOutputPageNum( rDestRect );

                        // Destination Export
                        if ( -1 != nDestPageNum )
                        {
                            tools::Rectangle aRect(SwRectToPDFRect(pCurrPage, rDestRect.SVRect()));
                            const sal_Int32 nDestId = pPDFExtOutDevData->CreateDest(aRect, nDestPageNum);
                            const OUString* vNodeText = &static_cast<const SwTextNode*>(&rCurrentNode)->GetText();
                            vDestinations.emplace_back(pIteratedTOX, vNodeText, nDestId);
                        }
                    }
                }
                if (!mrSh.MovePara(GoNextPara, fnParaStart))
                { // Cursor is stuck in the TOX due to document ending immediately afterwards
                    break;
                }
            }
        }
    }

    // Generate links to matching entries in the bibliography tables
    std::vector<SwFormatField*> aFields;
    SwFieldType* pType = mrSh.GetFieldType(SwFieldIds::TableOfAuthorities, OUString());
    if (!pType)
    {
        return;
    }

    pType->GatherFields(aFields);
    const auto pPageFrame = static_cast<const SwPageFrame*>(mrSh.GetLayout()->Lower());
    for (const auto pFormatField : aFields)
    {
        if (!pFormatField->GetTextField() || !pFormatField->IsFieldInDoc())
        {
            continue;
        }

        const auto& rAuthorityField
            = *static_cast<const SwAuthorityField*>(pFormatField->GetField());

        if (auto targetType = rAuthorityField.GetTargetType();
            targetType == SwAuthorityField::TargetType::UseDisplayURL
            || targetType == SwAuthorityField::TargetType::UseTargetURL)
        {
            // Since the target type specifies to use an URL, link to it
            const OUString& rURL = rAuthorityField.GetAbsoluteURL();
            if (rURL.getLength() == 0)
            {
                continue;
            }

            const SwTextNode& rTextNode = pFormatField->GetTextField()->GetTextNode();
            if (!lcl_TryMoveToNonHiddenField(mrSh, rTextNode, *pFormatField))
            {
                continue;
            }

            OUString const content(rAuthorityField.ExpandField(true, mrSh.GetLayout()));

            // Select the field.
            mrSh.SwCursorShell::SetMark();
            mrSh.SwCursorShell::Right(1, SwCursorSkipMode::Chars);

            // Create the links.
            SwRects const rects(GetCursorRectsContainingText(mrSh));
            for (const auto& rLinkRect : rects)
            {
                for (const auto& rLinkPageNum : CalcOutputPageNums(rLinkRect))
                {
                    tools::Rectangle aRect(SwRectToPDFRect(pPageFrame, rLinkRect.SVRect()));
                    sal_Int32 nLinkId = pPDFExtOutDevData->CreateLink(aRect, content, rLinkPageNum);
                    IdMapEntry aLinkEntry(rLinkRect, nLinkId);
                    pPDFExtOutDevData->GetSwPDFState()->m_LinkIdMap.push_back(aLinkEntry);
                    pPDFExtOutDevData->SetLinkURL(nLinkId, rURL);
                }
            }
            mrSh.SwCursorShell::ClearMark();
        }
        else if (targetType == SwAuthorityField::TargetType::BibliographyTableRow)
        {
            // As the target type specifies, try linking to a bibliography table row
            sal_Int32 nDestId = -1;

            std::unordered_map<const SwTOXBase*, OUString> vFormattedFieldStrings;
            for (const auto& rDestinationTuple : vDestinations)
            {
                if (vFormattedFieldStrings.find(std::get<0>(rDestinationTuple))
                    == vFormattedFieldStrings.end())
                    vFormattedFieldStrings.emplace(std::get<0>(rDestinationTuple),
                                                  rAuthorityField.GetAuthority(mrSh.GetLayout(),
                                                                               &std::get<0>(rDestinationTuple)->GetTOXForm()));

                if (vFormattedFieldStrings.at(std::get<0>(rDestinationTuple)) == *std::get<1>(rDestinationTuple))
                {
                    nDestId = std::get<2>(rDestinationTuple);
                    break;
                }
            }

            if (nDestId == -1)
                continue;

            const SwTextNode& rTextNode = pFormatField->GetTextField()->GetTextNode();
            if (!lcl_TryMoveToNonHiddenField(mrSh, rTextNode, *pFormatField))
            {
                continue;
            }

            OUString const content(rAuthorityField.ExpandField(true, mrSh.GetLayout()));

            // Select the field.
            mrSh.SwCursorShell::SetMark();
            mrSh.SwCursorShell::Right(1, SwCursorSkipMode::Chars);

            // Create the links.
            SwRects const rects(GetCursorRectsContainingText(mrSh));
            for (const auto& rLinkRect : rects)
            {
                for (const auto& rLinkPageNum : CalcOutputPageNums(rLinkRect))
                {
                    tools::Rectangle aRect(SwRectToPDFRect(pPageFrame, rLinkRect.SVRect()));
                    sal_Int32 nLinkId = pPDFExtOutDevData->CreateLink(aRect, content, rLinkPageNum);
                    IdMapEntry aLinkEntry(rLinkRect, nLinkId);
                    pPDFExtOutDevData->GetSwPDFState()->m_LinkIdMap.push_back(aLinkEntry);
                    pPDFExtOutDevData->SetLinkDest(nLinkId, nDestId);
                }
            }
            mrSh.SwCursorShell::ClearMark();
        }
    }
}

// Returns the page number in the output pdf on which the given rect is located.
// If this page is duplicated, method will return first occurrence of it.
sal_Int32 SwEnhancedPDFExportHelper::CalcOutputPageNum( const SwRect& rRect ) const
{
    std::vector< sal_Int32 > aPageNums = CalcOutputPageNums( rRect );
    if ( !aPageNums.empty() )
        return aPageNums[0];
    return -1;
}

// Returns a vector of the page numbers in the output pdf on which the given
// rect is located. There can be many such pages since StringRangeEnumerator
// allows duplication of its entries.
std::vector< sal_Int32 > SwEnhancedPDFExportHelper::CalcOutputPageNums(
    const SwRect& rRect ) const
{
    std::vector< sal_Int32 > aPageNums;

    // Document page number.
    sal_Int32 nPageNumOfRect = mrSh.GetPageNumAndSetOffsetForPDF( mrOut, rRect );
    if ( nPageNumOfRect < 0 )
        return aPageNums;

    // What will be the page numbers of page nPageNumOfRect in the output pdf?
    if ( mpRangeEnum )
    {
        if ( mbSkipEmptyPages )
            // Map the page number to the range without empty pages.
            nPageNumOfRect = maPageNumberMap[ nPageNumOfRect ];

        if ( mpRangeEnum->hasValue( nPageNumOfRect ) )
        {
            sal_Int32 nOutputPageNum = 0;
            StringRangeEnumerator::Iterator aIter = mpRangeEnum->begin();
            StringRangeEnumerator::Iterator aEnd  = mpRangeEnum->end();
            for ( ; aIter != aEnd; ++aIter )
            {
                if ( *aIter == nPageNumOfRect )
                    aPageNums.push_back( nOutputPageNum );
                ++nOutputPageNum;
            }
        }
    }
    else
    {
        if ( mbSkipEmptyPages )
        {
            sal_Int32 nOutputPageNum = 0;
            for ( size_t i = 0; i < maPageNumberMap.size(); ++i )
            {
                if ( maPageNumberMap[i] >= 0 ) // is not empty?
                {
                    if ( i == static_cast<size_t>( nPageNumOfRect ) )
                    {
                        aPageNums.push_back( nOutputPageNum );
                        break;
                    }
                    ++nOutputPageNum;
                }
            }
        }
        else
            aPageNums.push_back( nPageNumOfRect );
    }

    return aPageNums;
}

void SwEnhancedPDFExportHelper::MakeHeaderFooterLinks( vcl::PDFExtOutDevData& rPDFExtOutDevData,
                                                       const SwTextNode& rTNd,
                                                       const SwRect& rLinkRect,
                                                       sal_Int32 nDestId,
                                                       const OUString& rURL,
                                                       bool bInternal,
                                                       OUString const& rContent) const
{
    // We assume, that the primary link has just been exported. Therefore
    // the offset of the link rectangle calculates as follows:
    const Point aOffset = rLinkRect.Pos() + mrOut.GetMapMode().GetOrigin();

    SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(rTNd);
    for ( SwTextFrame* pTmpFrame = aIter.First(); pTmpFrame; pTmpFrame = aIter.Next() )
    {
        // Add offset to current page:
        const SwPageFrame* pPageFrame = pTmpFrame->FindPageFrame();
        SwRect aHFLinkRect( rLinkRect );
        aHFLinkRect.Pos() = pPageFrame->getFrameArea().Pos() + aOffset;

        // #i97135# the gcc_x64 optimizer gets aHFLinkRect != rLinkRect wrong
        // fool it by comparing the position only (the width and height are the
        // same anyway)
        if ( aHFLinkRect.Pos() != rLinkRect.Pos() )
        {
            // Link PageNums
            std::vector<sal_Int32> aHFLinkPageNums = CalcOutputPageNums( aHFLinkRect );

            for (sal_Int32 aHFLinkPageNum : aHFLinkPageNums)
            {
                // Link Export
                tools::Rectangle aRect(SwRectToPDFRect(pPageFrame, aHFLinkRect.SVRect()));
                const sal_Int32 nHFLinkId =
                    rPDFExtOutDevData.CreateLink(aRect, rContent, aHFLinkPageNum);

                // Connect Link and Destination:
                if ( bInternal )
                    rPDFExtOutDevData.SetLinkDest( nHFLinkId, nDestId );
                else
                    rPDFExtOutDevData.SetLinkURL( nHFLinkId, rURL );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
