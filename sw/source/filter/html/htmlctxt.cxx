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

#include <hintids.hxx>
#include <svl/itemiter.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svtools/htmltokn.h>
#include <editeng/boxitem.hxx>
#include <osl/diagnose.h>

#include <doc.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <shellio.hxx>
#include <paratr.hxx>
#include "htmlnum.hxx"
#include "css1kywd.hxx"
#include "swcss1.hxx"
#include "swhtml.hxx"

#include <memory>

using namespace ::com::sun::star;

class HTMLAttrContext_SaveDoc
{
    SwHTMLNumRuleInfo aNumRuleInfo; // Numbering for this environment
    std::unique_ptr<SwPosition>
                      pPos;         // Jump back to here when leaving context
    std::shared_ptr<HTMLAttrTable>
                      xAttrTab;     // Valid attributes for the environment,
                                    // if attributes shouldn't be preserved

    size_t nContextStMin;           // Stack lower bound for the environment
                                    // if stack needs to be protected
    size_t nContextStAttrMin;       // Stack lower bound for the environment
                                    // if the attributes shouldn't be preserved
    bool bStripTrailingPara : 1;
    bool bKeepNumRules : 1;
    bool bFixHeaderDist : 1;
    bool bFixFooterDist : 1;

public:

    HTMLAttrContext_SaveDoc() :
        nContextStMin( SIZE_MAX ), nContextStAttrMin( SIZE_MAX ),
        bStripTrailingPara( false ), bKeepNumRules( false ),
        bFixHeaderDist( false ), bFixFooterDist( false )
    {}

    // The position is ours, so we need to create and delete it
    void SetPos( const SwPosition& rPos ) { pPos.reset( new SwPosition(rPos) ); }
    const SwPosition *GetPos() const { return pPos.get(); }

    // The index isn't ours. So no creation or deletion
    void SetNumInfo( const SwHTMLNumRuleInfo& rInf ) { aNumRuleInfo.Set(rInf); }
    const SwHTMLNumRuleInfo& GetNumInfo() const { return aNumRuleInfo; }

    std::shared_ptr<HTMLAttrTable> const & GetAttrTab(bool bCreate = false);

    void SetContextStMin( size_t nMin ) { nContextStMin = nMin; }
    size_t GetContextStMin() const { return nContextStMin; }

    void SetContextStAttrMin( size_t nMin ) { nContextStAttrMin = nMin; }
    size_t GetContextStAttrMin() const { return nContextStAttrMin; }

    void SetStripTrailingPara( bool bSet ) { bStripTrailingPara = bSet; }
    bool GetStripTrailingPara() const { return bStripTrailingPara; }

    void SetKeepNumRules( bool bSet ) { bKeepNumRules = bSet; }
    bool GetKeepNumRules() const { return bKeepNumRules; }

    void SetFixHeaderDist( bool bSet ) { bFixHeaderDist = bSet; }
    bool GetFixHeaderDist() const { return bFixHeaderDist; }

    void SetFixFooterDist( bool bSet ) { bFixFooterDist = bSet; }
    bool GetFixFooterDist() const { return bFixFooterDist; }
};

std::shared_ptr<HTMLAttrTable> const & HTMLAttrContext_SaveDoc::GetAttrTab( bool bCreate )
{
    if (!xAttrTab && bCreate)
    {
        xAttrTab.reset(new HTMLAttrTable);
        memset(xAttrTab.get(), 0, sizeof(HTMLAttrTable));
    }
    return xAttrTab;
}

HTMLAttrContext_SaveDoc *HTMLAttrContext::GetSaveDocContext( bool bCreate )
{
    if( !m_pSaveDocContext && bCreate )
        m_pSaveDocContext.reset(new HTMLAttrContext_SaveDoc);

    return m_pSaveDocContext.get();
}

HTMLAttrContext::HTMLAttrContext( HtmlTokenId nTokn, sal_uInt16 nPoolId, const OUString& rClass,
                  bool bDfltColl ) :
    m_aClass( rClass ),
    m_nToken( nTokn ),
    m_nTextFormatColl( nPoolId ),
    m_nLeftMargin( 0 ),
    m_nRightMargin( 0 ),
    m_nFirstLineIndent( 0 ),
    m_nUpperSpace( 0 ),
    m_nLowerSpace( 0 ),
    m_eAppend( AM_NONE ),
    m_bLRSpaceChanged( false ),
    m_bULSpaceChanged( false ),
    m_bDefaultTextFormatColl( bDfltColl ),
    m_bSpansSection( false ),
    m_bPopStack( false ),
    m_bFinishPREListingXMP( false ),
    m_bRestartPRE( false ),
    m_bRestartXMP( false ),
    m_bRestartListing( false ),
    m_bHeaderOrFooter( false )
{}

HTMLAttrContext::HTMLAttrContext( HtmlTokenId nTokn ) :
    m_nToken( nTokn ),
    m_nTextFormatColl( 0 ),
    m_nLeftMargin( 0 ),
    m_nRightMargin( 0 ),
    m_nFirstLineIndent( 0 ),
    m_nUpperSpace( 0 ),
    m_nLowerSpace( 0 ),
    m_eAppend( AM_NONE ),
    m_bLRSpaceChanged( false ),
    m_bULSpaceChanged( false ),
    m_bDefaultTextFormatColl( false ),
    m_bSpansSection( false ),
    m_bPopStack( false ),
    m_bFinishPREListingXMP( false ),
    m_bRestartPRE( false ),
    m_bRestartXMP( false ),
    m_bRestartListing( false ),
    m_bHeaderOrFooter( false )
{}

HTMLAttrContext::~HTMLAttrContext()
{
    m_pSaveDocContext.reset();
}

void HTMLAttrContext::ClearSaveDocContext()
{
    m_pSaveDocContext.reset();
}

void SwHTMLParser::SplitAttrTab( const SwPosition& rNewPos )
{
    // preliminary paragraph attributes are not allowed here, they could
    // be set here and then the pointers become invalid!
    OSL_ENSURE(m_aParaAttrs.empty(),
        "Danger: there are non-final paragraph attributes");
    m_aParaAttrs.clear();

    const SwNodeIndex* pOldEndPara = &m_pPam->GetPoint()->nNode;
#ifndef NDEBUG
    auto const nOld(pOldEndPara->GetIndex());
#endif
    sal_Int32 nOldEndCnt = m_pPam->GetPoint()->nContent.GetIndex();

    const SwNodeIndex& rNewSttPara = rNewPos.nNode;
    sal_Int32 nNewSttCnt = rNewPos.nContent.GetIndex();

    bool bMoveBack = false;

    // close all open attributes and re-open them after the table
    HTMLAttr** pHTMLAttributes = reinterpret_cast<HTMLAttr**>(m_xAttrTab.get());
    for (auto nCnt = sizeof(HTMLAttrTable) / sizeof(HTMLAttr*); nCnt--; ++pHTMLAttributes)
    {
        HTMLAttr *pAttr = *pHTMLAttributes;
        while( pAttr )
        {
            HTMLAttr *pNext = pAttr->GetNext();
            HTMLAttr *pPrev = pAttr->GetPrev();

            sal_uInt16 nWhich = pAttr->m_pItem->Which();
            if( !nOldEndCnt && RES_PARATR_BEGIN <= nWhich &&
                pAttr->GetSttParaIdx() < pOldEndPara->GetIndex() )
            {
                // The attribute needs to be closed one content position beforehand
                if( !bMoveBack )
                {
                    bMoveBack = m_pPam->Move( fnMoveBackward );
                    nOldEndCnt = m_pPam->GetPoint()->nContent.GetIndex();
                }
            }
            else if( bMoveBack )
            {
                m_pPam->Move( fnMoveForward );
                nOldEndCnt = m_pPam->GetPoint()->nContent.GetIndex();
                bMoveBack = false;
            }

            if( (RES_PARATR_BEGIN <= nWhich && bMoveBack) ||
                pAttr->GetSttParaIdx() < pOldEndPara->GetIndex() ||
                (pAttr->GetSttPara() == *pOldEndPara &&
                 pAttr->GetSttCnt() != nOldEndCnt) )
            {
                // The attribute needs to be set. Because we still need the original, since
                // pointers to the attribute still exists in the contexts, we need to clone it.
                // The next-list gets lost but the previous-list is preserved
                HTMLAttr *pSetAttr = pAttr->Clone( *pOldEndPara, nOldEndCnt );

                if( pNext )
                    pNext->InsertPrev( pSetAttr );
                else
                {
                    if (pSetAttr->m_bInsAtStart)
                        m_aSetAttrTab.push_front( pSetAttr );
                    else
                        m_aSetAttrTab.push_back( pSetAttr );
                }
            }
            else if( pPrev )
            {
                // The previous attributes still need to be set, even if the current attribute
                // doesn't need to be set before the table
                if( pNext )
                    pNext->InsertPrev( pPrev );
                else
                {
                    if (pPrev->m_bInsAtStart)
                        m_aSetAttrTab.push_front( pPrev );
                    else
                        m_aSetAttrTab.push_back( pPrev );
                }
            }

            // Set the start of the attribute
            pAttr->m_nStartPara = rNewSttPara;
            pAttr->m_nEndPara = rNewSttPara;
            pAttr->m_nStartContent = nNewSttCnt;
            pAttr->m_nEndContent = nNewSttCnt;
            pAttr->m_pPrev = nullptr;

            pAttr = pNext;
        }
    }

    if( bMoveBack )
        m_pPam->Move( fnMoveForward );

    assert(m_pPam->GetPoint()->nNode.GetIndex() == nOld);
}

void SwHTMLParser::SaveDocContext( HTMLAttrContext *pCntxt,
                                   HtmlContextFlags nFlags,
                                   const SwPosition *pNewPos )
{
    HTMLAttrContext_SaveDoc *pSave = pCntxt->GetSaveDocContext( true );
    pSave->SetStripTrailingPara( bool(HtmlContextFlags::StripPara & nFlags) );
    pSave->SetKeepNumRules( bool(HtmlContextFlags::KeepNumrule & nFlags) );
    pSave->SetFixHeaderDist( bool(HtmlContextFlags::HeaderDist & nFlags) );
    pSave->SetFixFooterDist( bool(HtmlContextFlags::FooterDist & nFlags) );

    if( pNewPos )
    {
        // If the PaM needs to be set to a different position, we need to preserve numbering
        if( !pSave->GetKeepNumRules() )
        {
            // Numbering shall not be preserved. So we need to preserve the current state
            // and turn off numbering afterwards
            pSave->SetNumInfo( GetNumInfo() );
            GetNumInfo().Clear();
        }

        if( HtmlContextFlags::KeepAttrs & nFlags )
        {
            // Close attribute on current position and start on new one
            SplitAttrTab( *pNewPos );
        }
        else
        {
            std::shared_ptr<HTMLAttrTable> xSaveAttrTab = pSave->GetAttrTab(true);
            SaveAttrTab(xSaveAttrTab);
        }

        pSave->SetPos( *m_pPam->GetPoint() );
        *m_pPam->GetPoint() = *pNewPos;
    }

    // Settings nContextStMin automatically means, that no
    // currently open lists (DL/OL/UL) can be closed
    if( HtmlContextFlags::ProtectStack & nFlags  )
    {
        pSave->SetContextStMin( m_nContextStMin );
        m_nContextStMin = m_aContexts.size();

        if( HtmlContextFlags::KeepAttrs & nFlags )
        {
            pSave->SetContextStAttrMin( m_nContextStAttrMin );
            m_nContextStAttrMin = m_aContexts.size();
        }
    }
}

void SwHTMLParser::RestoreDocContext( HTMLAttrContext *pCntxt )
{
    HTMLAttrContext_SaveDoc *pSave = pCntxt->GetSaveDocContext();
    if( !pSave )
        return;

    if( pSave->GetStripTrailingPara() )
        StripTrailingPara();

    if( pSave->GetPos() )
    {
        if( pSave->GetFixHeaderDist() || pSave->GetFixFooterDist() )
            FixHeaderFooterDistance( pSave->GetFixHeaderDist(),
                                     pSave->GetPos() );

        std::shared_ptr<HTMLAttrTable> xSaveAttrTab = pSave->GetAttrTab();
        if (!xSaveAttrTab)
        {
            // Close attribute on current position and start on the old one
            SplitAttrTab( *pSave->GetPos() );
        }
        else
        {
            RestoreAttrTab(xSaveAttrTab);
        }

        *m_pPam->GetPoint() = *pSave->GetPos();

        // We can already set the attributes so far
        SetAttr();
    }

    if( SIZE_MAX != pSave->GetContextStMin() )
    {
        m_nContextStMin = pSave->GetContextStMin();
        if( SIZE_MAX != pSave->GetContextStAttrMin() )
            m_nContextStAttrMin = pSave->GetContextStAttrMin();
    }

    if( !pSave->GetKeepNumRules() )
    {
        // Set the preserved numbering back
        GetNumInfo().Set( pSave->GetNumInfo() );
    }

    pCntxt->ClearSaveDocContext();
}

void SwHTMLParser::EndContext( HTMLAttrContext *pContext )
{
    if( pContext->GetPopStack() )
    {
        // Close all still open contexts. Our own context needs to be deleted already!
        while( m_aContexts.size() > m_nContextStMin )
        {
            std::unique_ptr<HTMLAttrContext> xCntxt(PopContext());
            OSL_ENSURE(xCntxt.get() != pContext,
                    "Context still on the stack" );
            if (xCntxt.get() == pContext)
                break;

            EndContext(xCntxt.get());
        }
    }

    // Close all still open attributes
    if( pContext->HasAttrs() )
        EndContextAttrs( pContext );

    // If a section has been opened, end it. Since sections can be part of absolute-positioned
    // objects, this needs to be done before restoring document context
    if( pContext->GetSpansSection() )
        EndSection();

    // Leave borders and other special sections
    if( pContext->HasSaveDocContext() )
        RestoreDocContext( pContext );

    // Add a paragraph break if needed
    if( AM_NONE != pContext->GetAppendMode() &&
        m_pPam->GetPoint()->nContent.GetIndex() )
        AppendTextNode( pContext->GetAppendMode() );

    // Restart PRE, LISTING and XMP environments
    if( pContext->IsFinishPREListingXMP() )
        FinishPREListingXMP();

    if( pContext->IsRestartPRE() )
        StartPRE();

    if( pContext->IsRestartXMP() )
        StartXMP();

    if( pContext->IsRestartListing() )
        StartListing();
}

void SwHTMLParser::ClearContext( HTMLAttrContext *pContext )
{
    HTMLAttrs &rAttrs = pContext->GetAttrs();
    for( auto pAttr : rAttrs )
    {
        // Simple deletion doesn't to the job, since the attribute
        // needs to be deregistered with its list.
        // In theory, you could delete the list and its attributes separately
        // but if you get that wrong, quite a lot is messed up
        DeleteAttr( pAttr );
    }
    rAttrs.clear();

    OSL_ENSURE( !pContext->GetSpansSection(),
            "Area can no longer be exited" );

    OSL_ENSURE( !pContext->HasSaveDocContext(),
            "Frame can no longer be exited" );

    // like RestoreDocContext reset enough of this to not catastrophically
    // fail if we still have a SaveDocContext here
    if (HTMLAttrContext_SaveDoc *pSave = pContext->GetSaveDocContext())
    {
        if (SIZE_MAX != pSave->GetContextStMin())
        {
            m_nContextStMin = pSave->GetContextStMin();
            if (SIZE_MAX != pSave->GetContextStAttrMin())
                m_nContextStAttrMin = pSave->GetContextStAttrMin();
        }

        pContext->ClearSaveDocContext();
    }

    // Restart PRE/LISTING/XMP environments
    if( pContext->IsFinishPREListingXMP() )
        FinishPREListingXMP();

    if( pContext->IsRestartPRE() )
        StartPRE();

    if( pContext->IsRestartXMP() )
        StartXMP();

    if( pContext->IsRestartListing() )
        StartListing();
}

bool SwHTMLParser::DoPositioning( SfxItemSet &rItemSet,
                                  SvxCSS1PropertyInfo &rPropInfo,
                                  HTMLAttrContext *pContext )
{
    bool bRet = false;

    // A border is opened on the following conditions
    // - the tag is absolute-positioned AND left/top are both known AND don't contain a % property
    // OR
    // - the tag should be floating AND
    // - there's a given width
    if( SwCSS1Parser::MayBePositioned( rPropInfo ) )
    {
        SfxItemSet aFrameItemSet( m_xDoc->GetAttrPool(),
                                svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END-1>{} );
        if( !IsNewDoc() )
            Reader::ResetFrameFormatAttrs(aFrameItemSet );

        SetAnchorAndAdjustment( text::VertOrientation::NONE, text::HoriOrientation::NONE, rPropInfo,
                                aFrameItemSet );

        SetVarSize( rPropInfo, aFrameItemSet );

        SetSpace( Size(0,0), rItemSet, rPropInfo, aFrameItemSet );

        SetFrameFormatAttrs( rItemSet,
                        HtmlFrameFormatFlags::Box|HtmlFrameFormatFlags::Padding|HtmlFrameFormatFlags::Background|HtmlFrameFormatFlags::Direction,
                        aFrameItemSet );

        InsertFlyFrame(aFrameItemSet, pContext, rPropInfo.m_aId);
        pContext->SetPopStack( true );
        rPropInfo.m_aId.clear();
        bRet = true;
    }

    return bRet;
}

bool SwHTMLParser::CreateContainer( const OUString& rClass,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo &rPropInfo,
                                    HTMLAttrContext *pContext )
{
    bool bRet = false;
    if( rClass.equalsIgnoreAsciiCase( "sd-abs-pos" ) &&
        SwCSS1Parser::MayBePositioned( rPropInfo ) )
    {
        // Container class
        SfxItemSet *pFrameItemSet = pContext->GetFrameItemSet( m_xDoc.get() );
        if( !IsNewDoc() )
            Reader::ResetFrameFormatAttrs( *pFrameItemSet );

        SetAnchorAndAdjustment( text::VertOrientation::NONE, text::HoriOrientation::NONE,
                                rPropInfo, *pFrameItemSet );
        Size aDummy(0,0);
        SetFixSize( aDummy, aDummy, false, false, rPropInfo, *pFrameItemSet );
        SetSpace( aDummy, rItemSet, rPropInfo, *pFrameItemSet );
        SetFrameFormatAttrs( rItemSet, HtmlFrameFormatFlags::Box|HtmlFrameFormatFlags::Background|HtmlFrameFormatFlags::Direction,
                        *pFrameItemSet );

        bRet = true;
    }

    return bRet;
}

void SwHTMLParser::InsertAttrs( SfxItemSet &rItemSet,
                                SvxCSS1PropertyInfo const &rPropInfo,
                                HTMLAttrContext *pContext,
                                bool bCharLvl )
{
    // Put together a DropCap attribute, if a "float:left" is before the first character
    if( bCharLvl && !m_pPam->GetPoint()->nContent.GetIndex() &&
        SvxAdjust::Left == rPropInfo.m_eFloat )
    {
        SwFormatDrop aDrop;
        aDrop.GetChars() = 1;

        m_pCSS1Parser->FillDropCap( aDrop, rItemSet );

        // We only set the DropCap attribute if the initial spans multiple lines
        if( aDrop.GetLines() > 1 )
        {
            NewAttr(m_xAttrTab, &m_xAttrTab->pDropCap, aDrop);

            HTMLAttrs &rAttrs = pContext->GetAttrs();
            rAttrs.push_back( m_xAttrTab->pDropCap );

            return;
        }
    }

    if( !bCharLvl )
        m_pCSS1Parser->SetFormatBreak( rItemSet, rPropInfo );

    OSL_ENSURE(m_aContexts.size() <= m_nContextStAttrMin ||
            m_aContexts.back().get() != pContext,
            "SwHTMLParser::InsertAttrs: Context already on the Stack");

    SfxItemIter aIter( rItemSet );

    const SfxPoolItem *pItem = aIter.FirstItem();
    while( pItem )
    {
        HTMLAttr **ppAttr = nullptr;

        switch( pItem->Which() )
        {
        case RES_LR_SPACE:
            {
                // Paragraph indents need to be added and are generated for each paragraphs
                // (here for the first paragraph only, all the following in SetTextCollAttrs)
                const SvxLRSpaceItem *pLRItem =
                    static_cast<const SvxLRSpaceItem *>(pItem);

                // Get old paragraph indents without the top context (that's the one we're editing)
                sal_uInt16 nOldLeft = 0, nOldRight = 0;
                short nOldIndent = 0;
                bool bIgnoreTop = m_aContexts.size() > m_nContextStMin &&
                                  m_aContexts.back().get() == pContext;
                GetMarginsFromContext( nOldLeft, nOldRight, nOldIndent,
                                       bIgnoreTop  );

                // ... and the currently valid ones
                sal_uInt16 nLeft = nOldLeft, nRight = nOldRight;
                short nIndent = nOldIndent;
                pContext->GetMargins( nLeft, nRight, nIndent );

                // ... and add the new indents to the old ones
                // Here, we don't get the ones from the item but the separately remembered ones,
                // since they could be negative. Accessing those via the item still works, since
                // the item (with value 0) will be added
                if( rPropInfo.m_bLeftMargin )
                {
                    OSL_ENSURE( rPropInfo.m_nLeftMargin < 0 ||
                            rPropInfo.m_nLeftMargin == pLRItem->GetTextLeft(),
                            "left margin does not match with item" );
                    if( rPropInfo.m_nLeftMargin < 0 &&
                        -rPropInfo.m_nLeftMargin > nOldLeft )
                        nLeft = 0;
                    else
                        nLeft = nOldLeft + static_cast< sal_uInt16 >(rPropInfo.m_nLeftMargin);
                }
                if( rPropInfo.m_bRightMargin )
                {
                    OSL_ENSURE( rPropInfo.m_nRightMargin < 0 ||
                            rPropInfo.m_nRightMargin == pLRItem->GetRight(),
                            "right margin does not match with item" );
                    if( rPropInfo.m_nRightMargin < 0 &&
                        -rPropInfo.m_nRightMargin > nOldRight )
                        nRight = 0;
                    else
                        nRight = nOldRight + static_cast< sal_uInt16 >(rPropInfo.m_nRightMargin);
                }
                if( rPropInfo.m_bTextIndent )
                    nIndent = pLRItem->GetTextFirstLineOfst();

                // Remember the value for the following paragraphs
                pContext->SetMargins( nLeft, nRight, nIndent );

                // Set the attribute on the current paragraph
                SvxLRSpaceItem aLRItem( *pLRItem );
                aLRItem.SetTextFirstLineOfst( nIndent );
                aLRItem.SetTextLeft( nLeft );
                aLRItem.SetRight( nRight );
                NewAttr(m_xAttrTab, &m_xAttrTab->pLRSpace, aLRItem);
                EndAttr( m_xAttrTab->pLRSpace, false );
            }
            break;

        case RES_UL_SPACE:
            if( !rPropInfo.m_bTopMargin || !rPropInfo.m_bBottomMargin )
            {
                sal_uInt16 nUpper = 0, nLower = 0;
                GetULSpaceFromContext( nUpper, nLower );
                SvxULSpaceItem aULSpace( *static_cast<const SvxULSpaceItem *>(pItem) );
                if( !rPropInfo.m_bTopMargin )
                    aULSpace.SetUpper( nUpper );
                if( !rPropInfo.m_bBottomMargin )
                    aULSpace.SetLower( nLower );

                NewAttr(m_xAttrTab, &m_xAttrTab->pULSpace, aULSpace);

                // save context information
                HTMLAttrs &rAttrs = pContext->GetAttrs();
                rAttrs.push_back( m_xAttrTab->pULSpace );

                pContext->SetULSpace( aULSpace.GetUpper(), aULSpace.GetLower() );
            }
            else
            {
                ppAttr = &m_xAttrTab->pULSpace;
            }
            break;
        case RES_CHRATR_FONTSIZE:
            // don't set attributes with a % property
            if( static_cast<const SvxFontHeightItem *>(pItem)->GetProp() == 100 )
                ppAttr = &m_xAttrTab->pFontHeight;
            break;
        case RES_CHRATR_CJK_FONTSIZE:
            // don't set attributes with a % property
            if( static_cast<const SvxFontHeightItem *>(pItem)->GetProp() == 100 )
                ppAttr = &m_xAttrTab->pFontHeightCJK;
            break;
        case RES_CHRATR_CTL_FONTSIZE:
            // don't set attributes with a % property
            if( static_cast<const SvxFontHeightItem *>(pItem)->GetProp() == 100 )
                ppAttr = &m_xAttrTab->pFontHeightCTL;
            break;

        case RES_BACKGROUND:
            if( bCharLvl )
            {
                // Convert the Frame attribute to a Char attribute (if needed)
                SvxBrushItem aBrushItem( *static_cast<const SvxBrushItem *>(pItem) );
                aBrushItem.SetWhich( RES_CHRATR_BACKGROUND );

                // Set the attribute
                NewAttr(m_xAttrTab, &m_xAttrTab->pCharBrush, aBrushItem);

                // and save context information
                HTMLAttrs &rAttrs = pContext->GetAttrs();
                rAttrs.push_back( m_xAttrTab->pCharBrush );
            }
            else if( pContext->GetToken() != HtmlTokenId::TABLEHEADER_ON &&
                     pContext->GetToken() != HtmlTokenId::TABLEDATA_ON )
            {
                ppAttr = &m_xAttrTab->pBrush;
            }
            break;

        case RES_BOX:
            if( bCharLvl )
            {
                SvxBoxItem aBoxItem( *static_cast<const SvxBoxItem *>(pItem) );
                aBoxItem.SetWhich( RES_CHRATR_BOX );

                NewAttr(m_xAttrTab, &m_xAttrTab->pCharBox, aBoxItem);

                HTMLAttrs &rAttrs = pContext->GetAttrs();
                rAttrs.push_back( m_xAttrTab->pCharBox );
            }
            else
            {
                ppAttr = &m_xAttrTab->pBox;
            }
            break;

        default:
            ppAttr = GetAttrTabEntry( pItem->Which() );
            break;
        }

        if( ppAttr )
        {
            // Set the attribute
            NewAttr(m_xAttrTab, ppAttr, *pItem);

            // and save context information
            HTMLAttrs &rAttrs = pContext->GetAttrs();
            rAttrs.push_back( *ppAttr );
        }

        pItem = aIter.NextItem();
    }

    if( !rPropInfo.m_aId.isEmpty() )
        InsertBookmark( rPropInfo.m_aId );
}

void SwHTMLParser::InsertAttr( HTMLAttr **ppAttr, const SfxPoolItem & rItem,
                               HTMLAttrContext *pCntxt )
{
    if( !ppAttr )
    {
        ppAttr = GetAttrTabEntry( rItem.Which() );
        if( !ppAttr )
            return;
    }

    // Set the attribute
    NewAttr(m_xAttrTab, ppAttr, rItem);

    // save context information
    HTMLAttrs &rAttrs = pCntxt->GetAttrs();
    rAttrs.push_back( *ppAttr );
}

void SwHTMLParser::SplitPREListingXMP( HTMLAttrContext *pCntxt )
{
    // PRE/Listing/XMP need to be finished when finishing context
    pCntxt->SetFinishPREListingXMP( true );

    // And set all now valid flags
    if( IsReadPRE() )
        pCntxt->SetRestartPRE( true );
    if( IsReadXMP() )
        pCntxt->SetRestartXMP( true );
    if( IsReadListing() )
        pCntxt->SetRestartListing( true );

    FinishPREListingXMP();
}

SfxItemSet *HTMLAttrContext::GetFrameItemSet( SwDoc *pCreateDoc )
{
    if( !m_pFrameItemSet && pCreateDoc )
        m_pFrameItemSet = std::make_unique<SfxItemSet>( pCreateDoc->GetAttrPool(),
                        svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END-1>{} );
    return m_pFrameItemSet.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
