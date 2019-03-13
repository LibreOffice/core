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

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <EnhancedPDFExportHelper.hxx>
#include <hintids.hxx>

#include <vcl/outdev.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <tools/multisel.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <tools/urlobj.hxx>
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
#include <docary.hxx>
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
#include <flyfrm.hxx>
#include <notxtfrm.hxx>
#include "porfld.hxx"
#include <SwStyleNameMapper.hxx>
#include "itrpaint.hxx"
#include <i18nlangtag/languagetag.hxx>
#include <IMark.hxx>
#include <printdata.hxx>
#include <SwNodeNum.hxx>
#include <calbck.hxx>
#include <stack>
#include <frmtool.hxx>

#include <tools/globname.hxx>
#include <svx/svdobj.hxx>

using namespace ::com::sun::star;

// Some static data structures

TableColumnsMap SwEnhancedPDFExportHelper::aTableColumnsMap;
LinkIdMap SwEnhancedPDFExportHelper::aLinkIdMap;
NumListIdMap SwEnhancedPDFExportHelper::aNumListIdMap;
NumListBodyIdMap SwEnhancedPDFExportHelper::aNumListBodyIdMap;
FrameTagIdMap SwEnhancedPDFExportHelper::aFrameTagIdMap;

LanguageType SwEnhancedPDFExportHelper::eLanguageDefault = LANGUAGE_SYSTEM;

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
}

#endif

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
const char aDocumentString[] = "Document";
const char aDivString[] = "Div";
const char aSectString[] = "Sect";
const char aHString[] = "H";
const char aH1String[] = "H1";
const char aH2String[] = "H2";
const char aH3String[] = "H3";
const char aH4String[] = "H4";
const char aH5String[] = "H5";
const char aH6String[] = "H6";
const char aListString[] = "L";
const char aListItemString[] = "LI";
const char aListBodyString[] = "LBody";
const char aBlockQuoteString[] = "BlockQuote";
const char aCaptionString[] = "Caption";
const char aIndexString[] = "Index";
const char aTOCString[] = "TOC";
const char aTOCIString[] = "TOCI";
const char aTableString[] = "Table";
const char aTRString[] = "TR";
const char aTDString[] = "TD";
const char aTHString[] = "TH";
const char aBibEntryString[] = "BibEntry";
const char aQuoteString[] = "Quote";
const char aSpanString[] = "Span";
const char aCodeString[] = "Code";
const char aFigureString[] = "Figure";
const char aFormulaString[] = "Formula";
const char aLinkString[] = "Link";
const char aNoteString[] = "Note";

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
void* lcl_GetKeyFromFrame( const SwFrame& rFrame )
{
    void* pKey = nullptr;

    if ( rFrame.IsPageFrame() )
        pKey = const_cast<void*>(static_cast<void const *>(&(static_cast<const SwPageFrame&>(rFrame).GetFormat()->getIDocumentSettingAccess())));
    else if ( rFrame.IsTextFrame() )
        pKey = const_cast<void*>(static_cast<void const *>(static_cast<const SwTextFrame&>(rFrame).GetTextNodeFirst()));
    else if ( rFrame.IsSctFrame() )
        pKey = const_cast<void*>(static_cast<void const *>(static_cast<const SwSectionFrame&>(rFrame).GetSection()));
    else if ( rFrame.IsTabFrame() )
        pKey = const_cast<void*>(static_cast<void const *>(static_cast<const SwTabFrame&>(rFrame).GetTable()));
    else if ( rFrame.IsRowFrame() )
        pKey = const_cast<void*>(static_cast<void const *>(static_cast<const SwRowFrame&>(rFrame).GetTabLine()));
    else if ( rFrame.IsCellFrame() )
    {
        const SwTabFrame* pTabFrame = rFrame.FindTabFrame();
        const SwTable* pTable = pTabFrame->GetTable();
        pKey = const_cast<void*>(static_cast<void const *>(& static_cast<const SwCellFrame&>(rFrame).GetTabBox()->FindStartOfRowSpan( *pTable )));
    }

    return pKey;
}

bool lcl_HasPreviousParaSameNumRule(SwTextFrame const& rTextFrame, const SwTextNode& rNode)
{
    bool bRet = false;
    SwNodeIndex aIdx( rNode );
    const SwDoc* pDoc = rNode.GetDoc();
    const SwNodes& rNodes = pDoc->GetNodes();
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

} // end namespace

SwTaggedPDFHelper::SwTaggedPDFHelper( const Num_Info* pNumInfo,
                                      const Frame_Info* pFrameInfo,
                                      const Por_Info* pPorInfo,
                                      OutputDevice const & rOut )
  : nEndStructureElement( 0 ),
    nRestoreCurrentTag( -1 ),
    mpNumInfo( pNumInfo ),
    mpFrameInfo( pFrameInfo ),
    mpPorInfo( pPorInfo )
{
    mpPDFExtOutDevData =
        dynamic_cast< vcl::PDFExtOutDevData*>( rOut.GetExtOutDevData() );

    if ( mpPDFExtOutDevData && mpPDFExtOutDevData->GetIsExportTaggedPDF() )
    {
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
}

SwTaggedPDFHelper::~SwTaggedPDFHelper()
{
    if ( mpPDFExtOutDevData && mpPDFExtOutDevData->GetIsExportTaggedPDF() )
    {
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
}

bool SwTaggedPDFHelper::CheckReopenTag()
{
    bool bRet = false;
    sal_Int32 nReopenTag = -1;
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
             ( rFrame.IsRowFrame() && rFrame.IsInFollowFlowRow() ) ||
             ( rFrame.IsCellFrame() && const_cast<SwFrame&>(rFrame).GetPrevCellLeaf() ) )
        {
            pKeyFrame = &rFrame;
        }
        else if ( rFrame.IsFlyFrame() )
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
            void* pKey = lcl_GetKeyFromFrame( *pKeyFrame );

            if ( pKey )
            {
                FrameTagIdMap& rFrameTagIdMap = SwEnhancedPDFExportHelper::GetFrameTagIdMap();
                const FrameTagIdMap::const_iterator aIter =  rFrameTagIdMap.find( pKey );
                if ( aIter != rFrameTagIdMap.end() )
                    nReopenTag = (*aIter).second;
            }
        }
    }

    if ( -1 != nReopenTag )
    {
        nRestoreCurrentTag = mpPDFExtOutDevData->GetCurrentStructureElement();
        const bool bSuccess = mpPDFExtOutDevData->SetCurrentStructureElement( nReopenTag );
        OSL_ENSURE( bSuccess, "Failed to reopen tag" );

#if OSL_DEBUG_LEVEL > 1
        aStructStack.push_back( 99 );
#endif

        bRet = bSuccess;
    }

    return bRet && !bContinue;
}

void SwTaggedPDFHelper::CheckRestoreTag() const
{
    if ( nRestoreCurrentTag != -1 )
    {
        const bool bSuccess = mpPDFExtOutDevData->SetCurrentStructureElement( nRestoreCurrentTag );
        OSL_ENSURE( bSuccess, "Failed to restore reopened tag" );

#if OSL_DEBUG_LEVEL > 1
        aStructStack.pop_back();
#endif
    }
}

void SwTaggedPDFHelper::BeginTag( vcl::PDFWriter::StructElement eType, const OUString& rString )
{
    // write new tag
    const sal_Int32 nId = mpPDFExtOutDevData->BeginStructureElement( eType, rString );
    ++nEndStructureElement;

#if OSL_DEBUG_LEVEL > 1
    aStructStack.push_back( static_cast<sal_uInt16>(eType) );
#endif

    // Store the id of the current structure element if
    // - it is a list structure element
    // - it is a list body element with children
    // - rFrame is the first page frame
    // - rFrame is a master frame
    // - rFrame has objects anchored to it
    // - rFrame is a row frame or cell frame in a split table row

    if ( mpNumInfo )
    {
        const SwTextFrame& rTextFrame = static_cast<const SwTextFrame&>(mpNumInfo->mrFrame);
        SwTextNode const*const pTextNd = rTextFrame.GetTextNodeForParaProps();
        const SwNodeNum* pNodeNum = pTextNd->GetNum(rTextFrame.getRootFrame());

        if ( vcl::PDFWriter::List == eType )
        {
            NumListIdMap& rNumListIdMap = SwEnhancedPDFExportHelper::GetNumListIdMap();
            rNumListIdMap[ pNodeNum ] = nId;
        }
        else if ( vcl::PDFWriter::LIBody == eType )
        {
            NumListBodyIdMap& rNumListBodyIdMap = SwEnhancedPDFExportHelper::GetNumListBodyIdMap();
            rNumListBodyIdMap[ pNodeNum ] = nId;
        }
    }
    else if ( mpFrameInfo )
    {
        const SwFrame& rFrame = mpFrameInfo->mrFrame;

        if ( ( rFrame.IsPageFrame() && !static_cast<const SwPageFrame&>(rFrame).GetPrev() ) ||
             ( rFrame.IsFlowFrame() && !SwFlowFrame::CastFlowFrame(&rFrame)->IsFollow() && SwFlowFrame::CastFlowFrame(&rFrame)->HasFollow() ) ||
             ( rFrame.IsTextFrame() && rFrame.GetDrawObjs() ) ||
             ( rFrame.IsRowFrame() && rFrame.IsInSplitTableRow() ) ||
             ( rFrame.IsCellFrame() && const_cast<SwFrame&>(rFrame).GetNextCellLeaf() ) )
        {
            const void* pKey = lcl_GetKeyFromFrame( rFrame );

            if ( pKey )
            {
                FrameTagIdMap& rFrameTagIdMap = SwEnhancedPDFExportHelper::GetFrameTagIdMap();
                rFrameTagIdMap[ pKey ] = nId;
            }
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

// Sets the attributes according to the structure type.
void SwTaggedPDFHelper::SetAttributes( vcl::PDFWriter::StructElement eType )
{
    vcl::PDFWriter::StructAttributeValue eVal;
    sal_Int32 nVal;

    /*
     * ATTRIBUTES FOR BLSE
     */
    if ( mpFrameInfo )
    {
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

        // Check which attributes to set:

        switch ( eType )
        {
            case vcl::PDFWriter::Document :
                bWritingMode = true;
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
            case vcl::PDFWriter::TableData :
                bPlacement =
                bWritingMode =
                bWidth =
                bHeight =
                bRowSpan = true;
                break;

            case vcl::PDFWriter::H1 :
            case vcl::PDFWriter::H2 :
            case vcl::PDFWriter::H3 :
            case vcl::PDFWriter::H4 :
            case vcl::PDFWriter::H5 :
            case vcl::PDFWriter::H6 :
            case vcl::PDFWriter::Paragraph :
            case vcl::PDFWriter::Heading :
            case vcl::PDFWriter::Caption :
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
                bPlacement =
                bWidth =
                bHeight =
                bBox = true;
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
            const SvxLRSpaceItem &rSpace =
                static_cast<const SwTextFrame*>(pFrame)->GetTextNodeForParaProps()->GetSwAttrSet().GetLRSpace();
            nVal =  rSpace.GetTextFirstLineOfst();
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

        // Formally here bAlternateText was triggered for PDF export, but this
        // was moved for more general use to primitives and usage in
        // VclMetafileProcessor2D (see processGraphicPrimitive2D).

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
            const SwCellFrame* pThisCell = dynamic_cast<const SwCellFrame*>(pFrame);
            if ( pThisCell )
            {
                nVal =  pThisCell->GetTabBox()->getRowSpan();
                if ( nVal > 1 )
                    mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::RowSpan, nVal );

                // calculate colspan:
                const SwTabFrame* pTabFrame = pThisCell->FindTabFrame();
                const SwTable* pTable = pTabFrame->GetTable();

                SwRectFnSet fnRectX(pTabFrame);

                const TableColumnsMapEntry& rCols = SwEnhancedPDFExportHelper::GetTableColumnsMap()[ pTable ];

                const long nLeft  = fnRectX.GetLeft(pThisCell->getFrameArea());
                const long nRight = fnRectX.GetRight(pThisCell->getFrameArea());
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
            const LanguageType nDefaultLang = SwEnhancedPDFExportHelper::GetDefaultLanguage();

            if ( nDefaultLang != nCurrentLanguage )
                mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::Language, static_cast<sal_uInt16>(nCurrentLanguage) );
        }

        if ( bLinkAttribute )
        {
            const LinkIdMap& rLinkIdMap = SwEnhancedPDFExportHelper::GetLinkIdMap();
            SwRect aPorRect;
            rInf.CalcRect( *pPor, &aPorRect );
            const Point aPorCenter = aPorRect.Center();
            auto aIter = std::find_if(rLinkIdMap.begin(), rLinkIdMap.end(),
                [&aPorCenter](const IdMapEntry& rEntry) { return rEntry.first.IsInside(aPorCenter); });
            if (aIter != rLinkIdMap.end())
            {
                sal_Int32 nLinkId = (*aIter).second;
                mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::LinkAnnotation, nLinkId );
            }
        }
    }
}

void SwTaggedPDFHelper::BeginNumberedListStructureElements()
{
    OSL_ENSURE( mpNumInfo, "List without mpNumInfo?" );
    if ( !mpNumInfo )
        return;

    const SwFrame& rFrame = mpNumInfo->mrFrame;
    OSL_ENSURE( rFrame.IsTextFrame(), "numbered only for text frames" );
    const SwTextFrame& rTextFrame = static_cast<const SwTextFrame&>(rFrame);

    // Lowers of NonStructureElements should not be considered:

    if ( lcl_IsInNonStructEnv( rTextFrame ) || rTextFrame.IsFollow() )
        return;

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
            NumListBodyIdMap& rNumListBodyIdMap = SwEnhancedPDFExportHelper::GetNumListBodyIdMap();

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
            NumListIdMap& rNumListIdMap = SwEnhancedPDFExportHelper::GetNumListIdMap();

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
            nRestoreCurrentTag = mpPDFExtOutDevData->GetCurrentStructureElement();
            mpPDFExtOutDevData->SetCurrentStructureElement( nReopenTag );

#if OSL_DEBUG_LEVEL > 1
            aStructStack.push_back( 99 );
#endif
        }
    }
    else
    {
        // clear list maps in case a list has been interrupted
        NumListIdMap& rNumListIdMap = SwEnhancedPDFExportHelper::GetNumListIdMap();
        rNumListIdMap.clear();
        NumListBodyIdMap& rNumListBodyIdMap = SwEnhancedPDFExportHelper::GetNumListBodyIdMap();
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
        BeginTag( vcl::PDFWriter::LIBody, aListBodyString );
    }
}

void SwTaggedPDFHelper::BeginBlockStructureElements()
{
    const SwFrame* pFrame = &mpFrameInfo->mrFrame;

    // Lowers of NonStructureElements should not be considered:

    if ( lcl_IsInNonStructEnv( *pFrame ) )
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
                if ( TOX_CONTENT_SECTION == pSection->GetType() )
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
                else if ( CONTENT_SECTION == pSection->GetType() )
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
                const SwTextNode* pTextNd =
                    static_cast<const SwTextFrame*>(pFrame)->GetTextNodeForParaProps();

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

                if (pTextNd->IsOutline()
                    && sw::IsParaPropsNode(*pFrame->getRootFrame(), *pTextNd))
                {
                    int nRealLevel = pTextNd->GetAttrOutlineLevel()-1;
                    nRealLevel = std::min(nRealLevel, 5);

                    nPDFType =  static_cast<sal_uInt16>(vcl::PDFWriter::H1 + nRealLevel);
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
                        default:
                            aPDFType = aH6String;
                            break;
                    }
                }

                // Section: TOCI

                else if ( pFrame->IsInSct() )
                {
                    const SwSectionFrame* pSctFrame = pFrame->FindSctFrame();
                    const SwSection* pSection = pSctFrame->GetSection();

                    if ( TOX_CONTENT_SECTION == pSection->GetType() )
                    {
                        const SwTOXBase* pTOXBase = pSection->GetTOXBase();
                        if ( pTOXBase && TOX_INDEX != pTOXBase->GetType() )
                        {
                            // Special case: Open additional TOCI tag:
                            BeginTag( vcl::PDFWriter::TOCI, aTOCIString );
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

                TableColumnsMap& rTableColumnsMap = SwEnhancedPDFExportHelper::GetTableColumnsMap();
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

                            const long nLeft  = aRectFnSet.GetLeft(pCellFrame->getFrameArea());
                            rCols.insert( nLeft );

                            while ( pCellFrame )
                            {
                                const long nRight = aRectFnSet.GetRight(pCellFrame->getFrameArea());
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
            {
                const SwFlyFrame* pFly = static_cast<const SwFlyFrame*>(pFrame);
                if ( pFly->Lower() && pFly->Lower()->IsNoTextFrame() )
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
    while ( nEndStructureElement > 0 )
    {
        EndTag();
        --nEndStructureElement;
    }

    CheckRestoreTag();
}

void SwTaggedPDFHelper::BeginInlineStructureElements()
{
    const SwLinePortion* pPor = &mpPorInfo->mrPor;
    const SwTextPaintInfo& rInf = mpPorInfo->mrTextPainter.GetInfo();
    const SwTextFrame* pFrame = rInf.GetTextFrame();

    // Lowers of NonStructureElements should not be considered:

    if ( lcl_IsInNonStructEnv( *pFrame ) )
        return;

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

        case PortionType::Lay :
        case PortionType::Text :
        case PortionType::Para :
            {
                std::pair<SwTextNode const*, sal_Int32> const pos(
                        pFrame->MapViewToModel(rInf.GetIdx()));
                SwTextAttr const*const pInetFormatAttr =
                    pos.first->GetTextAttrAt(pos.second, RES_TXTATR_INETFMT);

                OUString sStyleName;
                if ( !pInetFormatAttr )
                {
                    std::vector<SwTextAttr *> const charAttrs(
                        pos.first->GetTextAttrsAt(pos.second, RES_TXTATR_CHARFMT));
                    // TODO: handle more than 1 char style?
                    const SwCharFormat* pCharFormat = (charAttrs.size())
                        ? (*charAttrs.begin())->GetCharFormat().GetCharFormat() : nullptr;
                    if ( pCharFormat )
                        SwStyleNameMapper::FillProgName( pCharFormat->GetName(), sStyleName, SwGetPoolIdFromName::TxtColl );
                }

                // Check for Link:
                if( pInetFormatAttr )
                {
                    nPDFType = vcl::PDFWriter::Link;
                    aPDFType = aLinkString;
                }
                // Check for Quote/Code character style:
                else if (sStyleName == aQuotation)
                {
                    nPDFType = vcl::PDFWriter::Quote;
                    aPDFType = aQuoteString;
                }
                else if (sStyleName == aSourceText)
                {
                    nPDFType = vcl::PDFWriter::Code;
                    aPDFType = aCodeString;
                }
                else
                {
                    const LanguageType nCurrentLanguage = rInf.GetFont()->GetLanguage();
                    const SwFontScript nFont = rInf.GetFont()->GetActual();
                    const LanguageType nDefaultLang = SwEnhancedPDFExportHelper::GetDefaultLanguage();

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

        case PortionType::Table :
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

    aTableColumnsMap.clear();
    aLinkIdMap.clear();
    aNumListIdMap.clear();
    aNumListBodyIdMap.clear();
    aFrameTagIdMap.clear();

#if OSL_DEBUG_LEVEL > 1
    aStructStack.clear();
#endif

    const sal_Int16 nScript = SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage() );
    sal_uInt16 nLangRes = RES_CHRATR_LANGUAGE;

    if ( i18n::ScriptType::ASIAN == nScript )
        nLangRes = RES_CHRATR_CJK_LANGUAGE;
    else if ( i18n::ScriptType::COMPLEX == nScript )
        nLangRes = RES_CHRATR_CTL_LANGUAGE;

    eLanguageDefault = static_cast<const SvxLanguageItem*>(&mrSh.GetDoc()->GetDefault( nLangRes ))->GetLanguage();

    EnhancedPDFExport();
}

SwEnhancedPDFExportHelper::~SwEnhancedPDFExportHelper()
{
}

tools::Rectangle SwEnhancedPDFExportHelper::SwRectToPDFRect(const SwPageFrame* pCurrPage,
    const tools::Rectangle& rRectangle) const
{
    SwPostItMode nPostItMode = mrPrintData.GetPrintPostIts();
    if (nPostItMode != SwPostItMode::InMargins)
        return rRectangle;
    //the page has been scaled by 75% and vertically centered, so adjust these
    //rectangles equivalently
    tools::Rectangle aRect(rRectangle);
    Size aRectSize(aRect.GetSize());
    double fScale = 0.75;
    aRectSize.setWidth( aRectSize.Width() * fScale );
    aRectSize.setHeight( aRectSize.Height() * fScale );
    long nOrigHeight = pCurrPage->getFrameArea().Height();
    long nNewHeight = nOrigHeight*fScale;
    long nShiftY = (nOrigHeight-nNewHeight)/2;
    aRect.SetLeft( aRect.Left() * fScale );
    aRect.SetTop( aRect.Top() * fScale );
    aRect.Move(0, nShiftY);
    aRect.SetSize(aRectSize);
    return aRect;
}

void SwEnhancedPDFExportHelper::EnhancedPDFExport()
{
    vcl::PDFExtOutDevData* pPDFExtOutDevData =
        dynamic_cast< vcl::PDFExtOutDevData*>( mrOut.GetExtOutDevData() );

    if ( !pPDFExtOutDevData )
        return;

    // set the document locale

    css::lang::Locale aDocLocale( LanguageTag( SwEnhancedPDFExportHelper::GetDefaultLanguage() ).getLocale() );
    pPDFExtOutDevData->SetDocumentLocale( aDocLocale );

    // Prepare the output device:

    mrOut.Push( PushFlags::MAPMODE );
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

        // POSTITS

        if ( pPDFExtOutDevData->GetIsExportNotes() )
        {
            SwFieldType* pType = mrSh.GetFieldType( SwFieldIds::Postit, OUString() );
            SwIterator<SwFormatField,SwFieldType> aIter( *pType );
            for( SwFormatField* pFirst = aIter.First(); pFirst; )
            {
                if( pFirst->GetTextField() && pFirst->IsFieldInDoc() )
                {
                    const SwTextNode* pTNd = pFirst->GetTextField()->GetpTextNode();
                    OSL_ENSURE( nullptr != pTNd, "Enhanced pdf export - text node is missing" );

                    // 1. Check if the whole paragraph is hidden
                    // 2. Move to the field
                    // 3. Check for hidden text attribute
                    if ( !pTNd->IsHidden() &&
                          mrSh.GotoFormatField( *pFirst ) &&
                         !mrSh.SelectHiddenRange() )
                    {
                        // Link Rectangle
                        const SwRect& rNoteRect = mrSh.GetCharRect();
                        const SwPageFrame* pCurrPage =
                            static_cast<const SwPageFrame*>( mrSh.GetLayout()->Lower() );

                        // Link PageNums
                        std::vector<sal_Int32> aNotePageNums = CalcOutputPageNums( rNoteRect );
                        for (sal_Int32 aNotePageNum : aNotePageNums)
                        {
                            // Link Note
                            vcl::PDFNote aNote;

                            // Use the NumberFormatter to get the date string:
                            const SwPostItField* pField = static_cast<SwPostItField*>(pFirst->GetField());
                            SvNumberFormatter* pNumFormatter = pDoc->GetNumberFormatter();
                            const Date aDateDiff( pField->GetDate() -
                                                 pNumFormatter->GetNullDate() );
                            const sal_uLong nFormat =
                                pNumFormatter->GetStandardFormat( SvNumFormatType::DATE, pField->GetLanguage() );
                            OUString sDate;
                            Color* pColor;
                            pNumFormatter->GetOutputString( aDateDiff.GetDate(), nFormat, sDate, &pColor );

                            // The title should consist of the author and the date:
                            aNote.Title = pField->GetPar1() + ", " + sDate;
                            // Guess what the contents contains...
                            aNote.Contents = pField->GetText();

                            // Link Export
                            tools::Rectangle aRect(SwRectToPDFRect(pCurrPage, rNoteRect.SVRect()));
                            pPDFExtOutDevData->CreateNote(aRect, aNote, aNotePageNum);
                        }
                    }
                }
                pFirst = aIter.Next();
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
                 !mrSh.SelectHiddenRange() )
            {
                // Select the hyperlink:
                mrSh.SwCursorShell::Right( 1, CRSR_SKIP_CHARS );
                if ( mrSh.SwCursorShell::SelectTextAttr( RES_TXTATR_INETFMT, true ) )
                {
                    // First, we create the destination, because there may be more
                    // than one link to this destination:
                    OUString aURL( INetURLObject::decode(
                        p->rINetAttr.GetINetFormat().GetValue(),
                        INetURLObject::DecodeMechanism::Unambiguous ) );

                    // We have to distinguish between intern and real URLs
                    const bool bIntern = '#' == aURL[0];

                    // GetCursor_() is a SwShellCursor, which is derived from
                    // SwSelPaintRects, therefore the rectangles of the current
                    // selection can be easily obtained:
                    // Note: We make a copy of the rectangles, because they may
                    // be deleted again in JumpToSwMark.
                    SwRects aTmp;
                    aTmp.insert( aTmp.begin(), mrSh.SwCursorShell::GetCursor_()->begin(), mrSh.SwCursorShell::GetCursor_()->end() );
                    OSL_ENSURE( !aTmp.empty(), "Enhanced pdf export - rectangles are missing" );

                    const SwPageFrame* pSelectionPage =
                        static_cast<const SwPageFrame*>( mrSh.GetLayout()->Lower() );

                    // Create the destination for internal links:
                    sal_Int32 nDestId = -1;
                    if ( bIntern )
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

                    if ( !bIntern || -1 != nDestId )
                    {
                        // #i44368# Links in Header/Footer
                        const SwPosition aPos( *pTNd );
                        const bool bHeaderFooter = pDoc->IsInHeaderFooter( aPos.nNode );

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
                                    pPDFExtOutDevData->CreateLink(aRect, aLinkPageNum);

                                // Store link info for tagged pdf output:
                                const IdMapEntry aLinkEntry( rLinkRect, nLinkId );
                                aLinkIdMap.push_back( aLinkEntry );

                                // Connect Link and Destination:
                                if ( bIntern )
                                    pPDFExtOutDevData->SetLinkDest( nLinkId, nDestId );
                                else
                                    pPDFExtOutDevData->SetLinkURL( nLinkId, aURL );

                                // #i44368# Links in Header/Footer
                                if ( bHeaderFooter )
                                    MakeHeaderFooterLinks( *pPDFExtOutDevData, *pTNd, rLinkRect, nDestId, aURL, bIntern );
                            }
                        }
                    }
                }
            }
            mrSh.SwCursorShell::ClearMark();
        }

        // HYPERLINKS (Graphics, Frames, OLEs )

        SwFrameFormats* pTable = pDoc->GetSpzFrameFormats();
        const size_t nSpzFrameFormatsCount = pTable->size();
        for( size_t n = 0; n < nSpzFrameFormatsCount; ++n )
        {
            SwFrameFormat* pFrameFormat = (*pTable)[n];
            const SfxPoolItem* pItem;
            if ( RES_DRAWFRMFMT != pFrameFormat->Which() &&
                GetFrameOfModify(mrSh.GetLayout(), *pFrameFormat, SwFrameType::Fly) &&
                 SfxItemState::SET == pFrameFormat->GetAttrSet().GetItemState( RES_URL, true, &pItem ) )
            {
                const SwPageFrame* pCurrPage =
                    static_cast<const SwPageFrame*>( mrSh.GetLayout()->Lower() );

                OUString aURL( static_cast<const SwFormatURL*>(pItem)->GetURL() );
                const bool bIntern = !aURL.isEmpty() && '#' == aURL[0];

                // Create the destination for internal links:
                sal_Int32 nDestId = -1;
                if ( bIntern )
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

                if ( !bIntern || -1 != nDestId )
                {
                    Point aNullPt;
                    const SwRect aLinkRect = pFrameFormat->FindLayoutRect( false, &aNullPt );

                    // Link PageNums
                    std::vector<sal_Int32> aLinkPageNums = CalcOutputPageNums( aLinkRect );

                    // Link Export
                    for (sal_Int32 aLinkPageNum : aLinkPageNums)
                    {
                        tools::Rectangle aRect(SwRectToPDFRect(pCurrPage, aLinkRect.SVRect()));
                        const sal_Int32 nLinkId =
                            pPDFExtOutDevData->CreateLink(aRect, aLinkPageNum);

                        // Connect Link and Destination:
                        if ( bIntern )
                            pPDFExtOutDevData->SetLinkDest( nLinkId, nDestId );
                        else
                            pPDFExtOutDevData->SetLinkURL( nLinkId, aURL );

                        // #i44368# Links in Header/Footer
                        const SwFormatAnchor &rAnch = pFrameFormat->GetAnchor();
                        if (RndStdIds::FLY_AT_PAGE != rAnch.GetAnchorId())
                        {
                            const SwPosition* pPosition = rAnch.GetContentAnchor();
                            if ( pPosition && pDoc->IsInHeaderFooter( pPosition->nNode ) )
                            {
                                const SwTextNode* pTNd = pPosition->nNode.GetNode().GetTextNode();
                                if ( pTNd )
                                    MakeHeaderFooterLinks( *pPDFExtOutDevData, *pTNd, aLinkRect, nDestId, aURL, bIntern );
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
                    SwRect aSnapRect = pObject->GetSnapRect();
                    std::vector<sal_Int32> aScreenPageNums = CalcOutputPageNums(aSnapRect);
                    if (aScreenPageNums.empty())
                        continue;

                    uno::Reference<drawing::XShape> xShape(pObject->getUnoShape(), uno::UNO_QUERY);
                    if (xShape->getShapeType() == "com.sun.star.drawing.MediaShape")
                    {
                        uno::Reference<beans::XPropertySet> xShapePropSet(xShape, uno::UNO_QUERY);
                        OUString aMediaURL;
                        xShapePropSet->getPropertyValue("MediaURL") >>= aMediaURL;
                        if (!aMediaURL.isEmpty())
                        {
                            const SwPageFrame* pCurrPage = mrSh.GetLayout()->GetPageAtPos(aSnapRect.Center());
                            tools::Rectangle aPDFRect(SwRectToPDFRect(pCurrPage, aSnapRect.SVRect()));
                            for (sal_Int32 nScreenPageNum : aScreenPageNums)
                            {
                                sal_Int32 nScreenId = pPDFExtOutDevData->CreateScreen(aPDFRect, nScreenPageNum);
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

        SwFieldType* pType = mrSh.GetFieldType( SwFieldIds::GetRef, OUString() );
        SwIterator<SwFormatField,SwFieldType> aIter( *pType );
        for( SwFormatField* pFirst = aIter.First(); pFirst; )
        {
            if( pFirst->GetTextField() && pFirst->IsFieldInDoc() )
            {
                const SwTextNode* pTNd = pFirst->GetTextField()->GetpTextNode();
                OSL_ENSURE( nullptr != pTNd, "Enhanced pdf export - text node is missing" );

                // 1. Check if the whole paragraph is hidden
                // 2. Move to the field
                // 3. Check for hidden text attribute
                if ( !pTNd->IsHidden() &&
                      mrSh.GotoFormatField( *pFirst ) &&
                     !mrSh.SelectHiddenRange() )
                {
                    // Select the field:
                    mrSh.SwCursorShell::SetMark();
                    mrSh.SwCursorShell::Right( 1, CRSR_SKIP_CHARS );

                    // Link Rectangles
                    SwRects aTmp;
                    aTmp.insert( aTmp.begin(), mrSh.SwCursorShell::GetCursor_()->begin(), mrSh.SwCursorShell::GetCursor_()->end() );
                    OSL_ENSURE( !aTmp.empty(), "Enhanced pdf export - rectangles are missing" );

                    mrSh.SwCursorShell::ClearMark();

                    // Destination Rectangle
                    const SwGetRefField* pField =
                        static_cast<SwGetRefField*>(pFirst->GetField());
                    const OUString& rRefName = pField->GetSetRefName();
                    mrSh.GotoRefMark( rRefName, pField->GetSubType(), pField->GetSeqNo() );
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
                        const SwPosition aPos( *pTNd );
                        const bool bHeaderFooter = pDoc->IsInHeaderFooter( aPos.nNode );

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
                                    pPDFExtOutDevData->CreateLink(aRect, aLinkPageNum);

                                // Store link info for tagged pdf output:
                                const IdMapEntry aLinkEntry( rLinkRect, nLinkId );
                                aLinkIdMap.push_back( aLinkEntry );

                                // Connect Link and Destination:
                                pPDFExtOutDevData->SetLinkDest( nLinkId, nDestId );

                                // #i44368# Links in Header/Footer
                                if ( bHeaderFooter )
                                {
                                    const OUString aDummy;
                                    MakeHeaderFooterLinks( *pPDFExtOutDevData, *pTNd, rLinkRect, nDestId, aDummy, true );
                                }
                            }
                        }
                    }
                }
            }
            pFirst = aIter.Next();
            mrSh.SwCursorShell::ClearMark();
        }

        // FOOTNOTES

        const size_t nFootnoteCount = pDoc->GetFootnoteIdxs().size();
        for ( size_t nIdx = 0; nIdx < nFootnoteCount; ++nIdx )
        {
            // Set cursor to text node that contains the footnote:
            const SwTextFootnote* pTextFootnote = pDoc->GetFootnoteIdxs()[ nIdx ];
            SwTextNode& rTNd = const_cast<SwTextNode&>(pTextFootnote->GetTextNode());

            mrSh.GetCursor_()->GetPoint()->nNode = rTNd;
            mrSh.GetCursor_()->GetPoint()->nContent.Assign( &rTNd, pTextFootnote->GetStart() );

            // 1. Check if the whole paragraph is hidden
            // 2. Check for hidden text attribute
            if (rTNd.GetTextNode()->IsHidden() || mrSh.SelectHiddenRange()
                || (mrSh.GetLayout()->IsHideRedlines()
                    && sw::IsFootnoteDeleted(pDoc->getIDocumentRedlineAccess(), *pTextFootnote)))
            {
                continue;
            }

            SwCursorSaveState aSaveState( *mrSh.GetCursor_() );

            // Select the footnote:
            mrSh.SwCursorShell::SetMark();
            mrSh.SwCursorShell::Right( 1, CRSR_SKIP_CHARS );

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
                // Link PageNums
                std::vector<sal_Int32> aLinkPageNums = CalcOutputPageNums( aLinkRect );

                // Destination Rectangle
                const SwRect& rDestRect = mrSh.GetCharRect();

                const SwPageFrame* pCurrPage =
                    static_cast<const SwPageFrame*>( mrSh.GetLayout()->Lower() );

                // Destination PageNum
                const sal_Int32 nDestPageNum = CalcOutputPageNum( rDestRect );

                for (sal_Int32 aLinkPageNum : aLinkPageNums)
                {
                    // Link Export
                    tools::Rectangle aRect(SwRectToPDFRect(pCurrPage, aLinkRect.SVRect()));
                    const sal_Int32 nLinkId =
                        pPDFExtOutDevData->CreateLink(aRect, aLinkPageNum);

                    // Store link info for tagged pdf output:
                    const IdMapEntry aLinkEntry( aLinkRect, nLinkId );
                    aLinkIdMap.push_back( aLinkEntry );

                    if ( -1 != nDestPageNum )
                    {
                        aRect = SwRectToPDFRect(pCurrPage, rDestRect.SVRect());
                        // Destination Export
                        const sal_Int32 nDestId = pPDFExtOutDevData->CreateDest(rDestRect.SVRect(), nDestPageNum);

                        // Connect Link and Destination:
                        pPDFExtOutDevData->SetLinkDest( nLinkId, nDestId );
                    }
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
                const ::sw::mark::IMark* pBkmk = ppMark->get();
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
            const bool bIntern = '#' == aBookmarkName[0];
            if ( bIntern )
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
    }

    // Restore view, cursor, and outdev:
    mrSh.LockView( bOldLockView );
    mrSh.SwCursorShell::Pop(SwCursorShell::PopMode::DeleteCurrent);
    mrOut.Pop();
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
                                                       bool bIntern ) const
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
                        rPDFExtOutDevData.CreateLink(aRect, aHFLinkPageNum);

                    // Connect Link and Destination:
                    if ( bIntern )
                        rPDFExtOutDevData.SetLinkDest( nHFLinkId, nDestId );
                    else
                        rPDFExtOutDevData.SetLinkURL( nHFLinkId, rURL );
                }
            }
        }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
