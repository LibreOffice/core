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
#include <EnhancedPDFExportHelper.hxx>
#include <hintids.hxx>

#include <vcl/outdev.hxx>
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
#include <crsskip.hxx>
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
#include <porfld.hxx>
#include <SwStyleNameMapper.hxx>
#include <itrpaint.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <IMark.hxx>
#include <printdata.hxx>
#include <SwNodeNum.hxx>
#include <calbck.hxx>
#include <stack>

#include <tools/globname.hxx>

using namespace ::com::sun::star;

// Some static data structures

TableColumnsMap SwEnhancedPDFExportHelper::aTableColumnsMap;
LinkIdMap SwEnhancedPDFExportHelper::aLinkIdMap;
NumListIdMap SwEnhancedPDFExportHelper::aNumListIdMap;
NumListBodyIdMap SwEnhancedPDFExportHelper::aNumListBodyIdMap;
FrmTagIdMap SwEnhancedPDFExportHelper::aFrmTagIdMap;

LanguageType SwEnhancedPDFExportHelper::eLanguageDefault = 0;

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
    std::vector< sal_uInt16 >::iterator aIter;
    for ( aIter = aStructStack.begin(); aIter != aStructStack.end(); ++aIter )
    {
        nElement = *aIter;
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
bool lcl_IsHeadlineCell( const SwCellFrm& rCellFrm )
{
    bool bRet = false;

    const SwContentFrm *pCnt = rCellFrm.ContainsContent();
    if ( pCnt && pCnt->IsTextFrm() )
    {
        const SwTextNode* pTextNode = static_cast<const SwTextFrm*>(pCnt)->GetTextNode();
        const SwFormat* pTextFormat = pTextNode->GetFormatColl();

        OUString sStyleName;
        SwStyleNameMapper::FillProgName( pTextFormat->GetName(), sStyleName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, true );
        bRet = sStyleName == aTableHeadingName;
    }

    return bRet;
}

// List all frames for which the NonStructElement tag is set:
bool lcl_IsInNonStructEnv( const SwFrm& rFrm )
{
    bool bRet = false;

    if ( 0 != rFrm.FindFooterOrHeader() &&
           !rFrm.IsHeaderFrm() && !rFrm.IsFooterFrm() )
    {
        bRet = true;
    }
    else if ( rFrm.IsInTab() && !rFrm.IsTabFrm() )
    {
        const SwTabFrm* pTabFrm = rFrm.FindTabFrm();
        if ( rFrm.GetUpper() != pTabFrm &&
             pTabFrm->IsFollow() && pTabFrm->IsInHeadline( rFrm ) )
             bRet = true;
    }

    return bRet;
}

// Generate key from frame for reopening tags:
void* lcl_GetKeyFromFrame( const SwFrm& rFrm )
{
    void* pKey = 0;

    if ( rFrm.IsPageFrm() )
        pKey = const_cast<void*>(static_cast<void const *>(&(static_cast<const SwPageFrm&>(rFrm).GetFormat()->getIDocumentSettingAccess())));
    else if ( rFrm.IsTextFrm() )
        pKey = const_cast<void*>(static_cast<void const *>(static_cast<const SwTextFrm&>(rFrm).GetTextNode()));
    else if ( rFrm.IsSctFrm() )
        pKey = const_cast<void*>(static_cast<void const *>(static_cast<const SwSectionFrm&>(rFrm).GetSection()));
    else if ( rFrm.IsTabFrm() )
        pKey = const_cast<void*>(static_cast<void const *>(static_cast<const SwTabFrm&>(rFrm).GetTable()));
    else if ( rFrm.IsRowFrm() )
        pKey = const_cast<void*>(static_cast<void const *>(static_cast<const SwRowFrm&>(rFrm).GetTabLine()));
    else if ( rFrm.IsCellFrm() )
    {
        const SwTabFrm* pTabFrm = rFrm.FindTabFrm();
        const SwTable* pTable = pTabFrm->GetTable();
        pKey = const_cast<void*>(static_cast<void const *>(& static_cast<const SwCellFrm&>(rFrm).GetTabBox()->FindStartOfRowSpan( *pTable )));
    }

    return pKey;
}

bool lcl_HasPreviousParaSameNumRule( const SwTextNode& rNode )
{
    bool bRet = false;
    SwNodeIndex aIdx( rNode );
    const SwDoc* pDoc = rNode.GetDoc();
    const SwNodes& rNodes = pDoc->GetNodes();
    const SwNode* pNode = &rNode;
    const SwNumRule* pNumRule = rNode.GetNumRule();

    while (! (pNode == rNodes.DocumentSectionStartNode(const_cast<SwNode*>(static_cast<SwNode const *>(&rNode))) ) )
    {
        --aIdx;

        if (aIdx.GetNode().IsTextNode())
        {
            const SwTextNode* pPrevTextNd = aIdx.GetNode().GetTextNode();
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
                                      const Frm_Info* pFrmInfo,
                                      const Por_Info* pPorInfo,
                                      OutputDevice& rOut )
  : nEndStructureElement( 0 ),
    nRestoreCurrentTag( -1 ),
    mpNumInfo( pNumInfo ),
    mpFrmInfo( pFrmInfo ),
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
        else if ( mpFrmInfo )
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

    if ( mpFrmInfo )
    {
        const SwFrm& rFrm = mpFrmInfo->mrFrm;
        const SwFrm* pKeyFrm = 0;

        // Reopen an existing structure element if
        // - rFrm is not the first page frame (reopen Document tag)
        // - rFrm is a follow frame (reopen Master tag)
        // - rFrm is a fly frame anchored at content (reopen Anchor paragraph tag)
        // - rFrm is a fly frame anchored at page (reopen Document tag)
        // - rFrm is a follow flow row (reopen TableRow tag)
        // - rFrm is a cell frame in a follow flow row (reopen TableData tag)
        if ( ( rFrm.IsPageFrm() && static_cast<const SwPageFrm&>(rFrm).GetPrev() ) ||
             ( rFrm.IsFlowFrm() && SwFlowFrm::CastFlowFrm(&rFrm)->IsFollow() ) ||
             ( rFrm.IsRowFrm() && rFrm.IsInFollowFlowRow() ) ||
             ( rFrm.IsCellFrm() && const_cast<SwFrm&>(rFrm).GetPrevCellLeaf( MAKEPAGE_NONE ) ) )
        {
            pKeyFrm = &rFrm;
        }
        else if ( rFrm.IsFlyFrm() )
        {
            const SwFormatAnchor& rAnchor =
                static_cast<const SwFlyFrm*>(&rFrm)->GetFormat()->GetAnchor();
            if ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
                (FLY_AT_CHAR == rAnchor.GetAnchorId()) ||
                (FLY_AT_PAGE == rAnchor.GetAnchorId()))
            {
                pKeyFrm = static_cast<const SwFlyFrm&>(rFrm).GetAnchorFrm();
                bContinue = true;
            }
        }

        if ( pKeyFrm )
        {
            void* pKey = lcl_GetKeyFromFrame( *pKeyFrm );

            if ( pKey )
            {
                FrmTagIdMap& rFrmTagIdMap = SwEnhancedPDFExportHelper::GetFrmTagIdMap();
                const FrmTagIdMap::const_iterator aIter =  rFrmTagIdMap.find( pKey );
                if ( aIter != rFrmTagIdMap.end() )
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

bool SwTaggedPDFHelper::CheckRestoreTag() const
{
    bool bRet = false;
    if ( nRestoreCurrentTag != -1 )
    {
        const bool bSuccess = mpPDFExtOutDevData->SetCurrentStructureElement( nRestoreCurrentTag );
        (void)bSuccess;
        OSL_ENSURE( bSuccess, "Failed to restore reopened tag" );

#if OSL_DEBUG_LEVEL > 1
        aStructStack.pop_back();
#endif

        bRet = true;
    }

    return bRet;
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
    // - rFrm is the first page frame
    // - rFrm is a master frame
    // - rFrm has objects anchored to it
    // - rFrm is a row frame or cell frame in a split table row

    if ( mpNumInfo )
    {
        const SwTextFrm& rTextFrm = static_cast<const SwTextFrm&>(mpNumInfo->mrFrm);
        const SwTextNode* pTextNd = rTextFrm.GetTextNode();
        const SwNodeNum* pNodeNum = pTextNd->GetNum();

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
    else if ( mpFrmInfo )
    {
        const SwFrm& rFrm = mpFrmInfo->mrFrm;

        if ( ( rFrm.IsPageFrm() && !static_cast<const SwPageFrm&>(rFrm).GetPrev() ) ||
             ( rFrm.IsFlowFrm() && !SwFlowFrm::CastFlowFrm(&rFrm)->IsFollow() && SwFlowFrm::CastFlowFrm(&rFrm)->HasFollow() ) ||
             ( rFrm.IsTextFrm() && rFrm.GetDrawObjs() ) ||
             ( rFrm.IsRowFrm() && rFrm.IsInSplitTableRow() ) ||
             ( rFrm.IsCellFrm() && const_cast<SwFrm&>(rFrm).GetNextCellLeaf( MAKEPAGE_NONE ) ) )
        {
            const void* pKey = lcl_GetKeyFromFrame( rFrm );

            if ( pKey )
            {
                FrmTagIdMap& rFrmTagIdMap = SwEnhancedPDFExportHelper::GetFrmTagIdMap();
                rFrmTagIdMap[ pKey ] = nId;
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
    if ( mpFrmInfo )
    {
        const SwFrm* pFrm = &mpFrmInfo->mrFrm;
        SWRECTFN( pFrm )

        bool bPlacement = false;
        bool bWritingMode = false;
        bool bSpaceBefore = false;
        bool bSpaceAfter = false;
        bool bStartIndent = false;
        bool bEndIndent = false;
        bool bTextIndent = false;
        bool bTextAlign = false;
        bool bAlternateText = false;
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
                bAlternateText =
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
            eVal =  pFrm->IsVertical() ?
                    vcl::PDFWriter::TbRl :
                    pFrm->IsRightToLeft() ?
                    vcl::PDFWriter::RlTb :
                    vcl::PDFWriter::LrTb;

            if ( vcl::PDFWriter::LrTb != eVal )
                mpPDFExtOutDevData->SetStructureAttribute( vcl::PDFWriter::WritingMode, eVal );
        }

        if ( bSpaceBefore )
        {
            nVal = (pFrm->*fnRect->fnGetTopMargin)();
            if ( 0 != nVal )
                mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::SpaceBefore, nVal );
        }

        if ( bSpaceAfter )
        {
            nVal = (pFrm->*fnRect->fnGetBottomMargin)();
            if ( 0 != nVal )
                mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::SpaceAfter, nVal );
        }

        if ( bStartIndent )
        {
            nVal = (pFrm->*fnRect->fnGetLeftMargin)();
            if ( 0 != nVal )
                mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::StartIndent, nVal );
        }

        if ( bEndIndent )
        {
            nVal = (pFrm->*fnRect->fnGetRightMargin)();
            if ( 0 != nVal )
                mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::EndIndent, nVal );
        }

        if ( bTextIndent )
        {
            OSL_ENSURE( pFrm->IsTextFrm(), "Frame type <-> tag attribute mismatch" );
            const SvxLRSpaceItem &rSpace =
                static_cast<const SwTextFrm*>(pFrm)->GetTextNode()->GetSwAttrSet().GetLRSpace();
            nVal =  rSpace.GetTextFirstLineOfst();
            if ( 0 != nVal )
                mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::TextIndent, nVal );
        }

        if ( bTextAlign )
        {
            OSL_ENSURE( pFrm->IsTextFrm(), "Frame type <-> tag attribute mismatch" );
            const SwAttrSet& aSet = static_cast<const SwTextFrm*>(pFrm)->GetTextNode()->GetSwAttrSet();
            const SvxAdjust nAdjust = aSet.GetAdjust().GetAdjust();
            if ( SVX_ADJUST_BLOCK == nAdjust || SVX_ADJUST_CENTER == nAdjust ||
                 (  (pFrm->IsRightToLeft() && SVX_ADJUST_LEFT == nAdjust) ||
                   (!pFrm->IsRightToLeft() && SVX_ADJUST_RIGHT == nAdjust) ) )
            {
                eVal = SVX_ADJUST_BLOCK == nAdjust ?
                       vcl::PDFWriter::Justify :
                       SVX_ADJUST_CENTER == nAdjust ?
                       vcl::PDFWriter::Center :
                       vcl::PDFWriter::End;

                mpPDFExtOutDevData->SetStructureAttribute( vcl::PDFWriter::TextAlign, eVal );
            }
        }

        if ( bAlternateText )
        {
            OSL_ENSURE( pFrm->IsFlyFrm(), "Frame type <-> tag attribute mismatch" );
            const SwFlyFrm* pFly = static_cast<const SwFlyFrm*>(pFrm);
            if ( pFly->Lower() && pFly->Lower()->IsNoTextFrm() )
            {
                const SwNoTextFrm* pNoTextFrm   = static_cast<const SwNoTextFrm*>(pFly->Lower());
                const SwNoTextNode* pNoTextNode = static_cast<const SwNoTextNode*>(pNoTextFrm->GetNode());

                const OUString aAlternateText( pNoTextNode->GetTitle() );
                mpPDFExtOutDevData->SetAlternateText( aAlternateText );
            }
        }

        if ( bWidth )
        {
            nVal = (pFrm->Frm().*fnRect->fnGetWidth)();
            mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::Width, nVal );
        }

        if ( bHeight )
        {
            nVal = (pFrm->Frm().*fnRect->fnGetHeight)();
            mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::Height, nVal );
        }

        if ( bBox )
        {
            // BBox only for non-split tables:
            if ( vcl::PDFWriter::Table != eType ||
                 ( pFrm->IsTabFrm() &&
                   !static_cast<const SwTabFrm*>(pFrm)->IsFollow() &&
                   !static_cast<const SwTabFrm*>(pFrm)->HasFollow() ) )
            {
                mpPDFExtOutDevData->SetStructureBoundingBox(pFrm->Frm().SVRect());
            }
        }

        if ( bRowSpan )
        {
            const SwCellFrm* pThisCell = dynamic_cast<const SwCellFrm*>(pFrm);
            if ( pThisCell )
            {
                nVal =  pThisCell->GetTabBox()->getRowSpan();
                if ( nVal > 1 )
                    mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::RowSpan, nVal );

                // calculate colspan:
                const SwTabFrm* pTabFrm = pThisCell->FindTabFrm();
                const SwTable* pTable = pTabFrm->GetTable();

                SWRECTFNX( pTabFrm )

                const TableColumnsMapEntry& rCols = SwEnhancedPDFExportHelper::GetTableColumnsMap()[ pTable ];

                const long nLeft  = (pThisCell->Frm().*fnRectX->fnGetLeft)();
                const long nRight = (pThisCell->Frm().*fnRectX->fnGetRight)();
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
                if( POR_HYPHSTR == pPor->GetWhichPor() || POR_SOFTHYPHSTR == pPor->GetWhichPor() ||
                    POR_HYPH == pPor->GetWhichPor() || POR_SOFTHYPH == pPor->GetWhichPor() )
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
            if (pPor->GetWhichPor() == POR_SOFTHYPH || pPor->GetWhichPor() == POR_HYPH)
                aActualText = OUString(sal_Unicode(0xad)); // soft hyphen
            else
                aActualText = rInf.GetText().copy(rInf.GetIdx(), pPor->GetLen());
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
            if ( UNDERLINE_NONE    != rInf.GetFont()->GetUnderline() )
                mpPDFExtOutDevData->SetStructureAttribute( vcl::PDFWriter::TextDecorationType, vcl::PDFWriter::Underline );
            if ( UNDERLINE_NONE    != rInf.GetFont()->GetOverline() )
                mpPDFExtOutDevData->SetStructureAttribute( vcl::PDFWriter::TextDecorationType, vcl::PDFWriter::Overline );
            if ( STRIKEOUT_NONE    != rInf.GetFont()->GetStrikeout() )
                mpPDFExtOutDevData->SetStructureAttribute( vcl::PDFWriter::TextDecorationType, vcl::PDFWriter::LineThrough );
            if ( EMPHASISMARK_NONE != rInf.GetFont()->GetEmphasisMark() )
                mpPDFExtOutDevData->SetStructureAttribute( vcl::PDFWriter::TextDecorationType, vcl::PDFWriter::Overline );
        }

        if ( bLanguage )
        {

            const LanguageType nCurrentLanguage = rInf.GetFont()->GetLanguage();
            const LanguageType nDefaultLang = SwEnhancedPDFExportHelper::GetDefaultLanguage();

            if ( nDefaultLang != nCurrentLanguage )
                mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::Language, nCurrentLanguage );
        }

        if ( bLinkAttribute )
        {
            const LinkIdMap& rLinkIdMap = SwEnhancedPDFExportHelper::GetLinkIdMap();
            SwRect aPorRect;
            rInf.CalcRect( *pPor, &aPorRect );
            const Point aPorCenter = aPorRect.Center();
            LinkIdMap::const_iterator aIter;
            for ( aIter = rLinkIdMap.begin(); aIter != rLinkIdMap.end(); ++aIter )
            {
                const SwRect& rLinkRect = (*aIter).first;
                if ( rLinkRect.IsInside( aPorCenter ) )
                {
                    sal_Int32 nLinkId = (*aIter).second;
                    mpPDFExtOutDevData->SetStructureAttributeNumerical( vcl::PDFWriter::LinkAnnotation, nLinkId );
                    break;
                }
            }
        }
    }
}

void SwTaggedPDFHelper::BeginNumberedListStructureElements()
{
    OSL_ENSURE( mpNumInfo, "List without mpNumInfo?" );
    if ( !mpNumInfo )
        return;

    const SwFrm& rFrm = mpNumInfo->mrFrm;
    OSL_ENSURE( rFrm.IsTextFrm(), "numbered only for text frames" );
    const SwTextFrm& rTextFrm = static_cast<const SwTextFrm&>(rFrm);

    // Lowers of NonStructureElements should not be considered:

    if ( lcl_IsInNonStructEnv( rTextFrm ) || rTextFrm.IsFollow() )
        return;

    const SwTextNode* pTextNd = rTextFrm.GetTextNode();
    const SwNumRule* pNumRule = pTextNd->GetNumRule();
    const SwNodeNum* pNodeNum = pTextNd->GetNum();

    const bool bNumbered = !pTextNd->IsOutline() && pNodeNum && pNodeNum->GetParent() && pNumRule;

    // Check, if we have to reopen a list or a list body:
    // First condition:
    // Paragraph is numbered/bulleted
    if ( !bNumbered )
        return;

    const SwNumberTreeNode* pParent = pNodeNum->GetParent();
    const bool bSameNumbering = lcl_HasPreviousParaSameNumRule(*pTextNd);

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
                while ( aIter == rNumListBodyIdMap.end() && 0 != ( pParent = pParent->GetParent() ) );

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
        BeginTag( vcl::PDFWriter::List, OUString(aListString) );

    if ( bNewItemTag )
    {
        BeginTag( vcl::PDFWriter::ListItem, OUString(aListItemString) );
        BeginTag( vcl::PDFWriter::LIBody, OUString(aListBodyString) );
    }
}

void SwTaggedPDFHelper::BeginBlockStructureElements()
{
    const SwFrm* pFrm = &mpFrmInfo->mrFrm;

    // Lowers of NonStructureElements should not be considered:

    if ( lcl_IsInNonStructEnv( *pFrm ) )
        return;

    // Check if we have to reopen an existing structure element.
    // This has to be done e.g., if pFrm is a follow frame.
    if ( CheckReopenTag() )
        return;

    sal_uInt16 nPDFType = USHRT_MAX;
    OUString aPDFType;

    switch ( pFrm->GetType() )
    {
        /*
         * GROUPING ELEMENTS
         */

        case FRM_PAGE :

            // Document: Document

            nPDFType = vcl::PDFWriter::Document;
            aPDFType = aDocumentString;
            break;

        case FRM_HEADER :
        case FRM_FOOTER :

            // Header, Footer: NonStructElement

            nPDFType = vcl::PDFWriter::NonStructElement;
            break;

        case FRM_FTNCONT :

            // Footnote container: Division

            nPDFType = vcl::PDFWriter::Division;
            aPDFType = aDivString;
            break;

        case FRM_FTN :

            // Footnote frame: Note

            // Note: vcl::PDFWriter::Note is actually a ILSE. Nevertheless
            // we treat it like a grouping element!
            nPDFType = vcl::PDFWriter::Note;
            aPDFType = aNoteString;
            break;

        case FRM_SECTION :

            // Section: TOX, Index, or Sect

            {
                const SwSection* pSection =
                        static_cast<const SwSectionFrm*>(pFrm)->GetSection();
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

        case FRM_TXT :
            {
                const SwTextNode* pTextNd =
                    static_cast<const SwTextFrm*>(pFrm)->GetTextNode();

                const SwFormat* pTextFormat = pTextNd->GetFormatColl();
                const SwFormat* pParentTextFormat = pTextFormat ? pTextFormat->DerivedFrom() : NULL;

                OUString sStyleName;
                OUString sParentStyleName;

                if ( pTextFormat)
                    SwStyleNameMapper::FillProgName( pTextFormat->GetName(), sStyleName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, true );
                if ( pParentTextFormat)
                    SwStyleNameMapper::FillProgName( pParentTextFormat->GetName(), sParentStyleName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, true );

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

                if ( pTextNd->IsOutline() )
                {
                    int nRealLevel = pTextNd->GetAttrOutlineLevel()-1;
                    nRealLevel = nRealLevel > 5 ? 5 : nRealLevel;

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

                else if ( pFrm->IsInSct() )
                {
                    const SwSectionFrm* pSctFrm = pFrm->FindSctFrm();
                    const SwSection* pSection =
                            static_cast<const SwSectionFrm*>(pSctFrm)->GetSection();

                    if ( TOX_CONTENT_SECTION == pSection->GetType() )
                    {
                        const SwTOXBase* pTOXBase = pSection->GetTOXBase();
                        if ( pTOXBase && TOX_INDEX != pTOXBase->GetType() )
                        {
                            // Special case: Open additional TOCI tag:
                            BeginTag( vcl::PDFWriter::TOCI, OUString(aTOCIString) );
                        }
                    }
                }
            }
            break;

        case FRM_TAB :

            // TabFrm: Table

            nPDFType = vcl::PDFWriter::Table;
            aPDFType = aTableString;

            {
                // set up table column data:
                const SwTabFrm* pTabFrm = static_cast<const SwTabFrm*>(pFrm);
                const SwTable* pTable = pTabFrm->GetTable();

                TableColumnsMap& rTableColumnsMap = SwEnhancedPDFExportHelper::GetTableColumnsMap();
                const TableColumnsMap::const_iterator aIter = rTableColumnsMap.find( pTable );

                if ( aIter == rTableColumnsMap.end() )
                {
                    SWRECTFN( pTabFrm )
                    TableColumnsMapEntry& rCols = rTableColumnsMap[ pTable ];

                    const SwTabFrm* pMasterFrm = pTabFrm->IsFollow() ? pTabFrm->FindMaster( true ) : pTabFrm;

                    while ( pMasterFrm )
                    {
                        const SwRowFrm* pRowFrm = static_cast<const SwRowFrm*>(pMasterFrm->GetLower());

                        while ( pRowFrm )
                        {
                            const SwFrm* pCellFrm = pRowFrm->GetLower();

                            const long nLeft  = (pCellFrm->Frm().*fnRect->fnGetLeft)();
                            rCols.insert( nLeft );

                            while ( pCellFrm )
                            {
                                const long nRight = (pCellFrm->Frm().*fnRect->fnGetRight)();
                                rCols.insert( nRight );
                                pCellFrm = pCellFrm->GetNext();
                            }
                            pRowFrm = static_cast<const SwRowFrm*>(pRowFrm->GetNext());
                        }
                        pMasterFrm = static_cast<const SwTabFrm*>(pMasterFrm->GetFollow());
                    }
                }
            }

            break;

        /*
         * TABLE ELEMENTS
         */

        case FRM_ROW :

            // RowFrm: TR

            if ( !static_cast<const SwRowFrm*>(pFrm)->IsRepeatedHeadline() )
            {
                nPDFType = vcl::PDFWriter::TableRow;
                aPDFType = aTRString;
            }
            else
            {
                nPDFType = vcl::PDFWriter::NonStructElement;
            }
            break;

        case FRM_CELL :

            // CellFrm: TH, TD

            {
                const SwTabFrm* pTable = static_cast<const SwCellFrm*>(pFrm)->FindTabFrm();
                if ( pTable->IsInHeadline( *pFrm ) || lcl_IsHeadlineCell( *static_cast<const SwCellFrm*>(pFrm) ) )
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

        case FRM_FLY :

            // FlyFrm: Figure, Formula, Control
            // fly in content or fly at page
            {
                const SwFlyFrm* pFly = static_cast<const SwFlyFrm*>(pFrm);
                if ( pFly->Lower() && pFly->Lower()->IsNoTextFrm() )
                {
                    bool bFormula = false;

                    const SwNoTextFrm* pNoTextFrm = static_cast<const SwNoTextFrm*>(pFly->Lower());
                    SwOLENode* pOLENd = const_cast<SwOLENode*>(pNoTextFrm->GetNode()->GetOLENode());
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
    const SwTextFrm* pFrm = rInf.GetTextFrm();

    // Lowers of NonStructureElements should not be considered:

    if ( lcl_IsInNonStructEnv( *pFrm ) )
        return;

    sal_uInt16 nPDFType = USHRT_MAX;
    OUString aPDFType;

    switch ( pPor->GetWhichPor() )
    {
        case POR_HYPH :
        case POR_SOFTHYPH :
        // Check for alternative spelling:
        case POR_HYPHSTR :
        case POR_SOFTHYPHSTR :
            nPDFType = vcl::PDFWriter::Span;
            aPDFType = aSpanString;
            break;

        case POR_LAY :
        case POR_TXT :
        case POR_PARA :
            {
                SwTextNode* pNd = const_cast<SwTextNode*>(pFrm->GetTextNode());
                SwTextAttr const*const pInetFormatAttr =
                    pNd->GetTextAttrAt(rInf.GetIdx(), RES_TXTATR_INETFMT);

                OUString sStyleName;
                if ( !pInetFormatAttr )
                {
                    ::std::vector<SwTextAttr *> const charAttrs(
                        pNd->GetTextAttrsAt(rInf.GetIdx(), RES_TXTATR_CHARFMT));
                    // TODO: handle more than 1 char style?
                    const SwCharFormat* pCharFormat = (charAttrs.size())
                        ? (*charAttrs.begin())->GetCharFormat().GetCharFormat() : 0;
                    if ( pCharFormat )
                        SwStyleNameMapper::FillProgName( pCharFormat->GetName(), sStyleName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, true );
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
                    const sal_uInt8 nFont = rInf.GetFont()->GetActual();
                    const LanguageType nDefaultLang = SwEnhancedPDFExportHelper::GetDefaultLanguage();

                    if ( UNDERLINE_NONE    != rInf.GetFont()->GetUnderline() ||
                         UNDERLINE_NONE    != rInf.GetFont()->GetOverline()  ||
                         STRIKEOUT_NONE    != rInf.GetFont()->GetStrikeout() ||
                         EMPHASISMARK_NONE != rInf.GetFont()->GetEmphasisMark() ||
                         0                 != rInf.GetFont()->GetEscapement() ||
                         SW_LATIN          != nFont ||
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

        case POR_FTN :
            nPDFType = vcl::PDFWriter::Link;
            aPDFType = aLinkString;
            break;

        case POR_FLD :
            {
                // check field type:
                const sal_Int32 nIdx = static_cast<const SwFieldPortion*>(pPor)->IsFollow() ?
                                        rInf.GetIdx() - 1 :
                                        rInf.GetIdx();
                const SwTextAttr* pHint = mpPorInfo->mrTextPainter.GetAttr( nIdx );
                if ( pHint && RES_TXTATR_FIELD == pHint->Which() )
                {
                    const SwField* pField = pHint->GetFormatField().GetField();
                    if ( RES_GETREFFLD == pField->Which() )
                    {
                        nPDFType = vcl::PDFWriter::Link;
                        aPDFType = aLinkString;
                    }
                    else if ( RES_AUTHORITY == pField->Which() )
                    {
                        nPDFType = vcl::PDFWriter::BibEntry;
                        aPDFType = aBibEntryString;
                    }
                }
            }
            break;

        case POR_TAB :
        case POR_TABRIGHT :
        case POR_TABCENTER :
        case POR_TABDECIMAL :
            nPDFType = vcl::PDFWriter::NonStructElement;
            break;
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
      mpRangeEnum( 0 ),
      mbSkipEmptyPages( bSkipEmptyPages ),
      mbEditEngineOnly( bEditEngineOnly ),
      mrPrintData( rPrintData )
{
    if ( !rPageRange.isEmpty() )
        mpRangeEnum = new StringRangeEnumerator( rPageRange, 0, mrSh.GetPageCount()-1 );

    if ( mbSkipEmptyPages )
    {
        maPageNumberMap.resize( mrSh.GetPageCount() );
        const SwPageFrm* pCurrPage =
            static_cast<const SwPageFrm*>( mrSh.GetLayout()->Lower() );
        sal_Int32 nPageNumber = 0;
        for ( size_t i = 0, n = maPageNumberMap.size(); i < n && pCurrPage; ++i )
        {
            if ( pCurrPage->IsEmptyPage() )
                maPageNumberMap[i] = -1;
            else
                maPageNumberMap[i] = nPageNumber++;

            pCurrPage = static_cast<const SwPageFrm*>( pCurrPage->GetNext() );
        }
    }

    aTableColumnsMap.clear();
    aLinkIdMap.clear();
    aNumListIdMap.clear();
    aNumListBodyIdMap.clear();
    aFrmTagIdMap.clear();

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
    delete mpRangeEnum;
}

Rectangle SwEnhancedPDFExportHelper::SwRectToPDFRect(const SwPageFrm* pCurrPage,
    const Rectangle& rRectangle) const
{
    SwPostItMode nPostItMode = mrPrintData.GetPrintPostIts();
    if (nPostItMode != SwPostItMode::InMargins)
        return rRectangle;
    //the page has been scaled by 75% and vertically centered, so adjust these
    //rectangles equivalently
    Rectangle aRect(rRectangle);
    Size aRectSize(aRect.GetSize());
    double fScale = 0.75;
    aRectSize.Width() = (aRectSize.Width() * fScale);
    aRectSize.Height() = (aRectSize.Height() * fScale);
    long nOrigHeight = pCurrPage->Frm().Height();
    long nNewHeight = nOrigHeight*fScale;
    long nShiftY = (nOrigHeight-nNewHeight)/2;
    aRect.Left() = (aRect.Left() * fScale);
    aRect.Top() = (aRect.Top() * fScale);
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

    com::sun::star::lang::Locale aDocLocale( LanguageTag( SwEnhancedPDFExportHelper::GetDefaultLanguage() ).getLocale() );
    pPDFExtOutDevData->SetDocumentLocale( aDocLocale );

    // Prepare the output device:

    mrOut.Push( PushFlags::MAPMODE );
    MapMode aMapMode( mrOut.GetMapMode() );
    aMapMode.SetMapUnit( MAP_TWIP );
    mrOut.SetMapMode( aMapMode );

    // Create new cursor and lock the view:

    SwDoc* pDoc = mrSh.GetDoc();
    mrSh.SwCrsrShell::Push();
    mrSh.SwCrsrShell::ClearMark();
    const bool bOldLockView = mrSh.IsViewLocked();
    mrSh.LockView( true );

    if ( !mbEditEngineOnly )
    {

        // POSTITS

        if ( pPDFExtOutDevData->GetIsExportNotes() )
        {
            SwFieldType* pType = mrSh.GetFieldType( RES_POSTITFLD, OUString() );
            SwIterator<SwFormatField,SwFieldType> aIter( *pType );
            for( SwFormatField* pFirst = aIter.First(); pFirst; )
            {
                if( pFirst->GetTextField() && pFirst->IsFieldInDoc() )
                {
                    const SwTextNode* pTNd = pFirst->GetTextField()->GetpTextNode();
                    OSL_ENSURE( 0 != pTNd, "Enhanced pdf export - text node is missing" );

                    // 1. Check if the whole paragraph is hidden
                    // 2. Move to the field
                    // 3. Check for hidden text attribute
                    if ( !pTNd->IsHidden() &&
                          mrSh.GotoFormatField( *pFirst ) &&
                         !mrSh.SelectHiddenRange() )
                    {
                        // Link Rectangle
                        const SwRect& rNoteRect = mrSh.GetCharRect();
                        const SwPageFrm* pCurrPage =
                            static_cast<const SwPageFrm*>( mrSh.GetLayout()->Lower() );

                        // Link PageNums
                        std::vector<sal_Int32> aNotePageNums = CalcOutputPageNums( rNoteRect );
                        for ( size_t nNumIdx = 0; nNumIdx < aNotePageNums.size(); ++nNumIdx )
                        {
                            // Link Note
                            vcl::PDFNote aNote;

                            // Use the NumberFormatter to get the date string:
                            const SwPostItField* pField = static_cast<SwPostItField*>(pFirst->GetField());
                            SvNumberFormatter* pNumFormatter = pDoc->GetNumberFormatter();
                            const Date aDateDiff( pField->GetDate() -
                                                 *pNumFormatter->GetNullDate() );
                            const sal_uLong nFormat =
                                pNumFormatter->GetStandardFormat( css::util::NumberFormat::DATE, pField->GetLanguage() );
                            OUString sDate;
                            Color* pColor;
                            pNumFormatter->GetOutputString( aDateDiff.GetDate(), nFormat, sDate, &pColor );

                            // The title should consist of the author and the date:
                            aNote.Title = pField->GetPar1() + ", " + sDate;
                            // Guess what the contents contains...
                            aNote.Contents = pField->GetText();

                            // Link Export
                            Rectangle aRect(SwRectToPDFRect(pCurrPage, rNoteRect.SVRect()));
                            pPDFExtOutDevData->CreateNote(aRect, aNote, aNotePageNums[nNumIdx]);
                        }
                    }
                }
                pFirst = aIter.Next();
                mrSh.SwCrsrShell::ClearMark();
            }
        }

        // HYPERLINKS

        SwGetINetAttrs aArr;
        mrSh.GetINetAttrs( aArr );
        for( auto &rAttr : aArr )
        {
            SwGetINetAttr* p = &rAttr;
            OSL_ENSURE( 0 != p, "Enhanced pdf export - SwGetINetAttr is missing" );

            const SwTextNode* pTNd = p->rINetAttr.GetpTextNode();
            OSL_ENSURE( 0 != pTNd, "Enhanced pdf export - text node is missing" );

            // 1. Check if the whole paragraph is hidden
            // 2. Move to the hyperlink
            // 3. Check for hidden text attribute
            if ( !pTNd->IsHidden() &&
                  mrSh.GotoINetAttr( p->rINetAttr ) &&
                 !mrSh.SelectHiddenRange() )
            {
                // Select the hyperlink:
                mrSh.SwCrsrShell::Right( 1, CRSR_SKIP_CHARS );
                if ( mrSh.SwCrsrShell::SelectTextAttr( RES_TXTATR_INETFMT, true ) )
                {
                    // First, we create the destination, because there may be more
                    // than one link to this destination:
                    OUString aURL( INetURLObject::decode(
                        p->rINetAttr.GetINetFormat().GetValue(),
                        INetURLObject::DECODE_UNAMBIGUOUS ) );

                    // We have to distinguish between intern and real URLs
                    const bool bIntern = '#' == aURL[0];

                    // _GetCrsr() is a SwShellCrsr, which is derived from
                    // SwSelPaintRects, therefore the rectangles of the current
                    // selection can be easily obtained:
                    // Note: We make a copy of the rectangles, because they may
                    // be deleted again in JumpToSwMark.
                    SwRects aTmp;
                    aTmp.insert( aTmp.begin(), mrSh.SwCrsrShell::_GetCrsr()->begin(), mrSh.SwCrsrShell::_GetCrsr()->end() );
                    OSL_ENSURE( !aTmp.empty(), "Enhanced pdf export - rectangles are missing" );

                    const SwPageFrm* pSelectionPage =
                        static_cast<const SwPageFrm*>( mrSh.GetLayout()->Lower() );

                    // Create the destination for internal links:
                    sal_Int32 nDestId = -1;
                    if ( bIntern )
                    {
                        aURL = aURL.copy( 1 );
                        mrSh.SwCrsrShell::ClearMark();
                        JumpToSwMark( &mrSh, aURL );

                        // Destination Rectangle
                        const SwRect& rDestRect = mrSh.GetCharRect();

                        const SwPageFrm* pCurrPage =
                            static_cast<const SwPageFrm*>( mrSh.GetLayout()->Lower() );

                        // Destination PageNum
                        const sal_Int32 nDestPageNum = CalcOutputPageNum( rDestRect );

                        // Destination Export
                        if ( -1 != nDestPageNum )
                        {
                            Rectangle aRect(SwRectToPDFRect(pCurrPage, rDestRect.SVRect()));
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

                            for ( size_t nNumIdx = 0; nNumIdx < aLinkPageNums.size(); ++nNumIdx )
                            {
                                // Link Export
                                Rectangle aRect(SwRectToPDFRect(pSelectionPage, rLinkRect.SVRect()));
                                const sal_Int32 nLinkId =
                                    pPDFExtOutDevData->CreateLink(aRect, aLinkPageNums[nNumIdx]);

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
            mrSh.SwCrsrShell::ClearMark();
        }

        // HYPERLINKS (Graphics, Frames, OLEs )

        const SwFrameFormats* pTable = pDoc->GetSpzFrameFormats();
        const size_t nSpzFrameFormatsCount = pTable->size();
        for( size_t n = 0; n < nSpzFrameFormatsCount; ++n )
        {
            const SwFrameFormat* pFrameFormat = (*pTable)[n];
            const SfxPoolItem* pItem;
            if ( RES_DRAWFRMFMT != pFrameFormat->Which() &&
                 SfxItemState::SET == pFrameFormat->GetAttrSet().GetItemState( RES_URL, true, &pItem ) )
            {
                const SwPageFrm* pCurrPage =
                    static_cast<const SwPageFrm*>( mrSh.GetLayout()->Lower() );

                OUString aURL( static_cast<const SwFormatURL*>(pItem)->GetURL() );
                const bool bIntern = '#' == aURL[0];

                // Create the destination for internal links:
                sal_Int32 nDestId = -1;
                if ( bIntern )
                {
                    aURL = aURL.copy( 1 );
                    mrSh.SwCrsrShell::ClearMark();
                    JumpToSwMark( &mrSh, aURL );

                    // Destination Rectangle
                    const SwRect& rDestRect = mrSh.GetCharRect();

                    pCurrPage = static_cast<const SwPageFrm*>( mrSh.GetLayout()->Lower() );

                    // Destination PageNum
                    const sal_Int32 nDestPageNum = CalcOutputPageNum( rDestRect );

                    // Destination Export
                    if ( -1 != nDestPageNum )
                    {
                        Rectangle aRect(SwRectToPDFRect(pCurrPage, rDestRect.SVRect()));
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
                    for ( size_t nNumIdx = 0; nNumIdx < aLinkPageNums.size(); ++nNumIdx )
                    {
                        Rectangle aRect(SwRectToPDFRect(pCurrPage, aLinkRect.SVRect()));
                        const sal_Int32 nLinkId =
                            pPDFExtOutDevData->CreateLink(aRect, aLinkPageNums[nNumIdx]);

                        // Connect Link and Destination:
                        if ( bIntern )
                            pPDFExtOutDevData->SetLinkDest( nLinkId, nDestId );
                        else
                            pPDFExtOutDevData->SetLinkURL( nLinkId, aURL );

                        // #i44368# Links in Header/Footer
                        const SwFormatAnchor &rAnch = pFrameFormat->GetAnchor();
                        if (FLY_AT_PAGE != rAnch.GetAnchorId())
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
            mrSh.SwCrsrShell::ClearMark();
        }

        // REFERENCES

        SwFieldType* pType = mrSh.GetFieldType( RES_GETREFFLD, OUString() );
        SwIterator<SwFormatField,SwFieldType> aIter( *pType );
        for( SwFormatField* pFirst = aIter.First(); pFirst; )
        {
            if( pFirst->GetTextField() && pFirst->IsFieldInDoc() )
            {
                const SwTextNode* pTNd = pFirst->GetTextField()->GetpTextNode();
               OSL_ENSURE( 0 != pTNd, "Enhanced pdf export - text node is missing" );

                // 1. Check if the whole paragraph is hidden
                // 2. Move to the field
                // 3. Check for hidden text attribute
                if ( !pTNd->IsHidden() &&
                      mrSh.GotoFormatField( *pFirst ) &&
                     !mrSh.SelectHiddenRange() )
                {
                    // Select the field:
                    mrSh.SwCrsrShell::SetMark();
                    mrSh.SwCrsrShell::Right( 1, CRSR_SKIP_CHARS );

                    // Link Rectangles
                    SwRects aTmp;
                    aTmp.insert( aTmp.begin(), mrSh.SwCrsrShell::_GetCrsr()->begin(), mrSh.SwCrsrShell::_GetCrsr()->end() );
                    OSL_ENSURE( !aTmp.empty(), "Enhanced pdf export - rectangles are missing" );

                    mrSh.SwCrsrShell::ClearMark();

                    // Destination Rectangle
                    const SwGetRefField* pField =
                        static_cast<SwGetRefField*>(pFirst->GetField());
                    const OUString& rRefName = pField->GetSetRefName();
                    mrSh.GotoRefMark( rRefName, pField->GetSubType(), pField->GetSeqNo() );
                    const SwRect& rDestRect = mrSh.GetCharRect();

                    const SwPageFrm* pCurrPage = static_cast<const SwPageFrm*>( mrSh.GetLayout()->Lower() );

                    // Destination PageNum
                    const sal_Int32 nDestPageNum = CalcOutputPageNum( rDestRect );

                    if ( -1 != nDestPageNum )
                    {
                        // Destination Export
                        Rectangle aRect(SwRectToPDFRect(pCurrPage, rDestRect.SVRect()));
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

                            for ( size_t nNumIdx = 0; nNumIdx < aLinkPageNums.size(); ++nNumIdx )
                            {
                                // Link Export
                                aRect = SwRectToPDFRect(pCurrPage, rLinkRect.SVRect());
                                const sal_Int32 nLinkId =
                                    pPDFExtOutDevData->CreateLink(aRect, aLinkPageNums[nNumIdx]);

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
            mrSh.SwCrsrShell::ClearMark();
        }

        // FOOTNOTES

        const size_t nFootnoteCount = pDoc->GetFootnoteIdxs().size();
        for ( size_t nIdx = 0; nIdx < nFootnoteCount; ++nIdx )
        {
            // Set cursor to text node that contains the footnote:
            const SwTextFootnote* pTextFootnote = pDoc->GetFootnoteIdxs()[ nIdx ];
            SwTextNode& rTNd = const_cast<SwTextNode&>(pTextFootnote->GetTextNode());

            mrSh._GetCrsr()->GetPoint()->nNode = rTNd;
            mrSh._GetCrsr()->GetPoint()->nContent.Assign( &rTNd, pTextFootnote->GetStart() );

            // 1. Check if the whole paragraph is hidden
            // 2. Check for hidden text attribute
            if ( rTNd.GetTextNode()->IsHidden() || mrSh.SelectHiddenRange() )
                continue;

            SwCrsrSaveState aSaveState( *mrSh._GetCrsr() );

            // Select the footnote:
            mrSh.SwCrsrShell::SetMark();
            mrSh.SwCrsrShell::Right( 1, CRSR_SKIP_CHARS );

            // Link Rectangle
            SwRects aTmp;
            aTmp.insert( aTmp.begin(), mrSh.SwCrsrShell::_GetCrsr()->begin(), mrSh.SwCrsrShell::_GetCrsr()->end() );
            OSL_ENSURE( !aTmp.empty(), "Enhanced pdf export - rectangles are missing" );

            mrSh._GetCrsr()->RestoreSavePos();
            mrSh.SwCrsrShell::ClearMark();

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

                const SwPageFrm* pCurrPage =
                    static_cast<const SwPageFrm*>( mrSh.GetLayout()->Lower() );

                // Destination PageNum
                const sal_Int32 nDestPageNum = CalcOutputPageNum( rDestRect );

                for ( size_t nNumIdx = 0; nNumIdx < aLinkPageNums.size(); ++nNumIdx )
                {
                    // Link Export
                    Rectangle aRect(SwRectToPDFRect(pCurrPage, aLinkRect.SVRect()));
                    const sal_Int32 nLinkId =
                        pPDFExtOutDevData->CreateLink(aRect, aLinkPageNums[nNumIdx]);

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

            const sal_uInt16 nOutlineCount =
                static_cast<sal_uInt16>(mrSh.getIDocumentOutlineNodesAccess()->getOutlineNodesCount());
            for ( sal_uInt16 i = 0; i < nOutlineCount; ++i )
            {
                // Check if outline is hidden
                const SwTextNode* pTNd = mrSh.GetNodes().GetOutLineNds()[ i ]->GetTextNode();
                OSL_ENSURE( 0 != pTNd, "Enhanced pdf export - text node is missing" );

                if ( pTNd->IsHidden() ||
                     // #i40292# Skip empty outlines:
                     pTNd->GetText().isEmpty())
                    continue;

                // Get parent id from stack:
                const sal_Int8 nLevel = (sal_Int8)mrSh.getIDocumentOutlineNodesAccess()->getOutlineLevel( i );
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

                const SwPageFrm* pCurrPage =
                    static_cast<const SwPageFrm*>( mrSh.GetLayout()->Lower() );

                // Destination PageNum
                const sal_Int32 nDestPageNum = CalcOutputPageNum( rDestRect );

                if ( -1 != nDestPageNum )
                {
                    // Destination Export
                    Rectangle aRect(SwRectToPDFRect(pCurrPage, rDestRect.SVRect()));
                    const sal_Int32 nDestId =
                        pPDFExtOutDevData->CreateDest(aRect, nDestPageNum);

                    // Outline entry text
                    const OUString& rEntry = mrSh.getIDocumentOutlineNodesAccess()->getOutlineText( i, true, false, false );

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
                mrSh.SwCrsrShell::ClearMark();
                OUString sBkName = pBkmk->GetName();

                //jump to it
                JumpToSwMark( &mrSh, sBkName );

                // Destination Rectangle
                const SwRect& rDestRect = mrSh.GetCharRect();

                const SwPageFrm* pCurrPage =
                    static_cast<const SwPageFrm*>( mrSh.GetLayout()->Lower() );

                // Destination PageNum
                const sal_Int32 nDestPageNum = CalcOutputPageNum( rDestRect );

                // Destination Export
                if ( -1 != nDestPageNum )
                {
                    Rectangle aRect(SwRectToPDFRect(pCurrPage, rDestRect.SVRect()));
                    pPDFExtOutDevData->CreateNamedDest(sBkName, aRect, nDestPageNum);
                }
            }
            mrSh.SwCrsrShell::ClearMark();
            //<--- i56629
        }
    }
    else
    {

        // LINKS FROM EDITENGINE

        std::vector< vcl::PDFExtOutDevBookmarkEntry >& rBookmarks = pPDFExtOutDevData->GetBookmarks();
        std::vector< vcl::PDFExtOutDevBookmarkEntry >::const_iterator aIBeg = rBookmarks.begin();
        const std::vector< vcl::PDFExtOutDevBookmarkEntry >::const_iterator aIEnd = rBookmarks.end();
        while ( aIBeg != aIEnd )
        {
            OUString aBookmarkName( aIBeg->aBookmark );
            const bool bIntern = '#' == aBookmarkName[0];
            if ( bIntern )
            {
                aBookmarkName = aBookmarkName.copy( 1 );
                JumpToSwMark( &mrSh, aBookmarkName );

                // Destination Rectangle
                const SwRect& rDestRect = mrSh.GetCharRect();

                const SwPageFrm* pCurrPage =
                    static_cast<const SwPageFrm*>( mrSh.GetLayout()->Lower() );

                // Destination PageNum
                const sal_Int32 nDestPageNum = CalcOutputPageNum( rDestRect );

                if ( -1 != nDestPageNum )
                {
                    Rectangle aRect(SwRectToPDFRect(pCurrPage, rDestRect.SVRect()));
                    if ( aIBeg->nLinkId != -1 )
                    {
                        // Destination Export
                        const sal_Int32 nDestId = pPDFExtOutDevData->CreateDest(aRect, nDestPageNum);

                        // Connect Link and Destination:
                        pPDFExtOutDevData->SetLinkDest( aIBeg->nLinkId, nDestId );
                    }
                    else
                    {
                        pPDFExtOutDevData->DescribeRegisteredDest(aIBeg->nDestId, aRect, nDestPageNum);
                    }
                }
            }
            else
                pPDFExtOutDevData->SetLinkURL( aIBeg->nLinkId, aBookmarkName );

            ++aIBeg;
        }
        rBookmarks.clear();
    }

    // Restore view, cursor, and outdev:
    mrSh.LockView( bOldLockView );
    mrSh.SwCrsrShell::Pop( false );
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

    SwIterator<SwTextFrm,SwTextNode> aIter( rTNd );
    for ( SwTextFrm* pTmpFrm = aIter.First(); pTmpFrm; pTmpFrm = aIter.Next() )
        {
            // Add offset to current page:
            const SwPageFrm* pPageFrm = pTmpFrm->FindPageFrm();
            SwRect aHFLinkRect( rLinkRect );
            aHFLinkRect.Pos() = pPageFrm->Frm().Pos() + aOffset;

            // #i97135# the gcc_x64 optimizer gets aHFLinkRect != rLinkRect wrong
            // fool it by comparing the position only (the width and height are the
            // same anyway)
            if ( aHFLinkRect.Pos() != rLinkRect.Pos() )
            {
                // Link PageNums
                std::vector<sal_Int32> aHFLinkPageNums = CalcOutputPageNums( aHFLinkRect );

                for ( size_t nNumIdx = 0; nNumIdx < aHFLinkPageNums.size(); ++nNumIdx )
                {
                    // Link Export
                    Rectangle aRect(SwRectToPDFRect(pPageFrm, aHFLinkRect.SVRect()));
                    const sal_Int32 nHFLinkId =
                        rPDFExtOutDevData.CreateLink(aRect, aHFLinkPageNums[nNumIdx]);

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
