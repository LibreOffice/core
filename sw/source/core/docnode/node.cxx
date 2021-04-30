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

#include <config_wasm_strip.h>

#include <hintids.hxx>
#include <editeng/protitem.hxx>
#include <osl/diagnose.h>
#include <tools/gen.hxx>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <txtftn.hxx>
#include <ftnfrm.hxx>
#include <doc.hxx>
#include <node.hxx>
#include <ndindex.hxx>
#include <numrule.hxx>
#include <swtable.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <swcache.hxx>
#include <section.hxx>
#include <cntfrm.hxx>
#include <flyfrm.hxx>
#include <txtfrm.hxx>
#include <tabfrm.hxx>
#include <viewsh.hxx>
#include <paratr.hxx>
#include <ftnidx.hxx>
#include <fmtftn.hxx>
#include <fmthdft.hxx>
#include <frmatr.hxx>
#include <fmtautofmt.hxx>
#include <frmtool.hxx>
#include <pagefrm.hxx>
#include <node2lay.hxx>
#include <pagedesc.hxx>
#include <fmtpdsc.hxx>
#include <breakit.hxx>
#include <SwStyleNameMapper.hxx>
#include <scriptinfo.hxx>
#include <rootfrm.hxx>
#include <istyleaccess.hxx>
#include <IDocumentListItems.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <calbck.hxx>
#include <ndole.hxx>
#include <memory>
#include <swcrsr.hxx>
#include <hints.hxx>
#include <frameformats.hxx>
#ifdef DBG_UTIL
#include <sal/backtrace.hxx>
#endif

using namespace ::com::sun::star::i18n;


/*
 * Some local helper functions for the attribute set handle of a content node.
 * Since the attribute set of a content node may not be modified directly,
 * we always have to create a new SwAttrSet, do the modifications, and get
 * a new handle from the style access
 */

namespace AttrSetHandleHelper
{

static void GetNewAutoStyle( std::shared_ptr<const SfxItemSet>& rpAttrSet,
                      const SwContentNode& rNode,
                      SwAttrSet const & rNewAttrSet )
{
    const SwAttrSet* pAttrSet = static_cast<const SwAttrSet*>(rpAttrSet.get());
    if( rNode.GetModifyAtAttr() )
        const_cast<SwAttrSet*>(pAttrSet)->SetModifyAtAttr( nullptr );
    IStyleAccess& rSA = pAttrSet->GetPool()->GetDoc()->GetIStyleAccess();
    rpAttrSet = rSA.getAutomaticStyle( rNewAttrSet, rNode.IsTextNode() ?
                                                     IStyleAccess::AUTO_STYLE_PARA :
                                                     IStyleAccess::AUTO_STYLE_NOTXT );
    const bool bSetModifyAtAttr = const_cast<SwAttrSet*>(static_cast<const SwAttrSet*>(rpAttrSet.get()))->SetModifyAtAttr( &rNode );
    rNode.SetModifyAtAttr( bSetModifyAtAttr );
}

static void SetParent( std::shared_ptr<const SfxItemSet>& rpAttrSet,
                const SwContentNode& rNode,
                const SwFormat* pParentFormat,
                const SwFormat* pConditionalFormat )
{
    const SwAttrSet* pAttrSet = static_cast<const SwAttrSet*>(rpAttrSet.get());
    OSL_ENSURE( pAttrSet, "no SwAttrSet" );
    OSL_ENSURE( pParentFormat || !pConditionalFormat, "ConditionalFormat without ParentFormat?" );

    const SwAttrSet* pParentSet = pParentFormat ? &pParentFormat->GetAttrSet() : nullptr;

    if ( pParentSet == pAttrSet->GetParent() )
        return;

    SwAttrSet aNewSet( *pAttrSet );
    aNewSet.SetParent( pParentSet );
    aNewSet.ClearItem( RES_FRMATR_STYLE_NAME );
    aNewSet.ClearItem( RES_FRMATR_CONDITIONAL_STYLE_NAME );

    if ( pParentFormat )
    {
        OUString sVal;
        SwStyleNameMapper::FillProgName( pParentFormat->GetName(), sVal, SwGetPoolIdFromName::TxtColl );
        const SfxStringItem aAnyFormatColl( RES_FRMATR_STYLE_NAME, sVal );
        aNewSet.Put( aAnyFormatColl );

        if ( pConditionalFormat != pParentFormat )
            SwStyleNameMapper::FillProgName( pConditionalFormat->GetName(), sVal, SwGetPoolIdFromName::TxtColl );

        const SfxStringItem aFormatColl( RES_FRMATR_CONDITIONAL_STYLE_NAME, sVal );
        aNewSet.Put( aFormatColl );
    }

    GetNewAutoStyle( rpAttrSet, rNode, aNewSet );
}

static const SfxPoolItem* Put( std::shared_ptr<const SfxItemSet>& rpAttrSet,
                        const SwContentNode& rNode,
                        const SfxPoolItem& rAttr )
{
    SwAttrSet aNewSet( static_cast<const SwAttrSet&>(*rpAttrSet) );
    const SfxPoolItem* pRet = aNewSet.Put( rAttr );
    if ( pRet )
        GetNewAutoStyle( rpAttrSet, rNode, aNewSet );
    return pRet;
}

static bool Put( std::shared_ptr<const SfxItemSet>& rpAttrSet, const SwContentNode& rNode,
         const SfxItemSet& rSet )
{
    SwAttrSet aNewSet( static_cast<const SwAttrSet&>(*rpAttrSet) );

    // #i76273# Robust
    std::optional<SfxItemSetFixed<RES_FRMATR_STYLE_NAME, RES_FRMATR_CONDITIONAL_STYLE_NAME>> pStyleNames;
    if ( SfxItemState::SET == rSet.GetItemState( RES_FRMATR_STYLE_NAME, false ) )
    {
        pStyleNames.emplace( *aNewSet.GetPool() );
        pStyleNames->Put( aNewSet );
    }

    const bool bRet = aNewSet.Put( rSet );

    // #i76273# Robust
    if ( pStyleNames )
    {
        aNewSet.Put( *pStyleNames );
    }

    if ( bRet )
        GetNewAutoStyle( rpAttrSet, rNode, aNewSet );

    return bRet;
}

static bool Put_BC( std::shared_ptr<const SfxItemSet>& rpAttrSet,
            const SwContentNode& rNode, const SfxPoolItem& rAttr,
            SwAttrSet* pOld, SwAttrSet* pNew )
{
    SwAttrSet aNewSet( static_cast<const SwAttrSet&>(*rpAttrSet) );

    // for a correct broadcast, we need to do a SetModifyAtAttr with the items
    // from aNewSet. The 'regular' SetModifyAtAttr is done in GetNewAutoStyle
    if( rNode.GetModifyAtAttr() )
        aNewSet.SetModifyAtAttr( &rNode );

    const bool bRet = aNewSet.Put_BC( rAttr, pOld, pNew );

    if ( bRet )
        GetNewAutoStyle( rpAttrSet, rNode, aNewSet );

    return bRet;
}

static bool Put_BC( std::shared_ptr<const SfxItemSet>& rpAttrSet,
            const SwContentNode& rNode, const SfxItemSet& rSet,
            SwAttrSet* pOld, SwAttrSet* pNew )
{
    SwAttrSet aNewSet( static_cast<const SwAttrSet&>(*rpAttrSet) );

    // #i76273# Robust
    std::optional<SfxItemSetFixed<RES_FRMATR_STYLE_NAME, RES_FRMATR_CONDITIONAL_STYLE_NAME>> pStyleNames;
    if ( SfxItemState::SET == rSet.GetItemState( RES_FRMATR_STYLE_NAME, false ) )
    {
        pStyleNames.emplace( *aNewSet.GetPool() );
        pStyleNames->Put( aNewSet );
    }

    // for a correct broadcast, we need to do a SetModifyAtAttr with the items
    // from aNewSet. The 'regular' SetModifyAtAttr is done in GetNewAutoStyle
    if( rNode.GetModifyAtAttr() )
        aNewSet.SetModifyAtAttr( &rNode );

    const bool bRet = aNewSet.Put_BC( rSet, pOld, pNew );

    // #i76273# Robust
    if ( pStyleNames )
    {
        aNewSet.Put( *pStyleNames );
    }

    if ( bRet )
        GetNewAutoStyle( rpAttrSet, rNode, aNewSet );

    return bRet;
}

static sal_uInt16 ClearItem_BC( std::shared_ptr<const SfxItemSet>& rpAttrSet,
                     const SwContentNode& rNode, sal_uInt16 nWhich,
                     SwAttrSet* pOld, SwAttrSet* pNew )
{
    SwAttrSet aNewSet( static_cast<const SwAttrSet&>(*rpAttrSet) );
    if( rNode.GetModifyAtAttr() )
        aNewSet.SetModifyAtAttr( &rNode );
    const sal_uInt16 nRet = aNewSet.ClearItem_BC( nWhich, pOld, pNew );
    if ( nRet )
        GetNewAutoStyle( rpAttrSet, rNode, aNewSet );
    return nRet;
}

static sal_uInt16 ClearItem_BC( std::shared_ptr<const SfxItemSet>& rpAttrSet,
                     const SwContentNode& rNode,
                     sal_uInt16 nWhich1, sal_uInt16 nWhich2,
                     SwAttrSet* pOld, SwAttrSet* pNew )
{
    SwAttrSet aNewSet( static_cast<const SwAttrSet&>(*rpAttrSet) );
    if( rNode.GetModifyAtAttr() )
        aNewSet.SetModifyAtAttr( &rNode );
    const sal_uInt16 nRet = aNewSet.ClearItem_BC( nWhich1, nWhich2, pOld, pNew );
    if ( nRet )
        GetNewAutoStyle( rpAttrSet, rNode, aNewSet );
    return nRet;
}

}

/** Returns the section level at the position given by aIndex.
 *
 * We use the following logic:
 * S = Start, E = End, C = ContentNode
 * Level   0 = E
 *         1 = S E
 *         2 = SC
 *
 * All EndNodes of the BaseSection have level 0
 * All StartNodes of the BaseSection have level 1
 */
sal_uInt16 SwNode::GetSectionLevel() const
{
    // EndNode of a BaseSection? They are always 0!
    if( IsEndNode() && SwNodeOffset(0) == m_pStartOfSection->StartOfSectionIndex() )
        return 0;

    sal_uInt16 nLevel;
    const SwNode* pNode = IsStartNode() ? this : m_pStartOfSection;
    for( nLevel = 1; SwNodeOffset(0) != pNode->StartOfSectionIndex(); ++nLevel )
        pNode = pNode->m_pStartOfSection;
    return IsEndNode() ? nLevel-1 : nLevel;
}

#ifdef DBG_UTIL
tools::Long SwNode::s_nSerial = 0;
#endif

SwNode::SwNode( const SwNodeIndex &rWhere, const SwNodeType nNdType )
    : m_nNodeType( nNdType )
    , m_nAFormatNumLvl( 0 )
    , m_bIgnoreDontExpand( false)
    , m_eMerge(Merge::None)
#ifdef DBG_UTIL
    , m_nSerial( s_nSerial++)
#endif
    , m_pStartOfSection( nullptr )
{
    if( !rWhere.GetIndex() )
        return;

    SwNodes& rNodes = const_cast<SwNodes&> (rWhere.GetNodes());
    SwNode* pNd = rNodes[ rWhere.GetIndex() -1 ];
    rNodes.InsertNode( this, rWhere );
    m_pStartOfSection = pNd->GetStartNode();
    if( nullptr == m_pStartOfSection )
    {
        m_pStartOfSection = pNd->m_pStartOfSection;
        if( pNd->GetEndNode() )     // Skip EndNode ? Section
        {
            pNd = m_pStartOfSection;
            m_pStartOfSection = pNd->m_pStartOfSection;
        }
    }
}

/** Inserts a node into the rNodes array at the rWhere position
 *
 * @param rNodes the variable array in that the node will be inserted
 * @param nPos position within the array where the node will be inserted
 * @param nNdType the type of node to insert
 */
SwNode::SwNode( SwNodes& rNodes, SwNodeOffset nPos, const SwNodeType nNdType )
    : m_nNodeType( nNdType )
    , m_nAFormatNumLvl( 0 )
    , m_bIgnoreDontExpand( false)
    , m_eMerge(Merge::None)
#ifdef DBG_UTIL
    , m_nSerial( s_nSerial++)
#endif
    , m_pStartOfSection( nullptr )
{
    if( !nPos )
        return;

    SwNode* pNd = rNodes[ nPos - 1 ];
    rNodes.InsertNode( this, nPos );
    m_pStartOfSection = pNd->GetStartNode();
    if( nullptr == m_pStartOfSection )
    {
        m_pStartOfSection = pNd->m_pStartOfSection;
        if( pNd->GetEndNode() )     // Skip EndNode ? Section!
        {
            pNd = m_pStartOfSection;
            m_pStartOfSection = pNd->m_pStartOfSection;
        }
    }
}

SwNode::~SwNode()
{
    assert(m_aAnchoredFlys.empty() || GetDoc().IsInDtor()); // must all be deleted
    InvalidateInSwCache(RES_OBJECTDYING);
    assert(!IsInCache());
}

/// Find the TableNode in which it is located.
/// If we're not in a table: return 0
SwTableNode* SwNode::FindTableNode()
{
    if( IsTableNode() )
        return GetTableNode();
    SwStartNode* pTmp = m_pStartOfSection;
    while( !pTmp->IsTableNode() && pTmp->GetIndex() )
        pTmp = pTmp->m_pStartOfSection;
    return pTmp->GetTableNode();
}

/// Is the node located in the visible area of the Shell?
bool SwNode::IsInVisibleArea( SwViewShell const * pSh ) const
{
    bool bRet = false;
    const SwContentNode* pNd;

    if( SwNodeType::Start & m_nNodeType )
    {
        SwNodeIndex aIdx( *this );
        pNd = GetNodes().GoNext( &aIdx );
    }
    else if( SwNodeType::End & m_nNodeType )
    {
        SwNodeIndex aIdx( *EndOfSectionNode() );
        pNd = SwNodes::GoPrevious( &aIdx );
    }
    else
        pNd = GetContentNode();

    if( !pSh )
        // Get the Shell from the Doc
        pSh = GetDoc().getIDocumentLayoutAccess().GetCurrentViewShell();

    if( pSh )
    {
        const SwFrame* pFrame;
        if (pNd && nullptr != (pFrame = pNd->getLayoutFrame(pSh->GetLayout(), nullptr, nullptr)))
        {

            if ( pFrame->IsInTab() )
                pFrame = pFrame->FindTabFrame();

            if( !pFrame->isFrameAreaDefinitionValid() )
            {
                do
                {
                    pFrame = pFrame->FindPrev();
                }
                while ( pFrame && !pFrame->isFrameAreaDefinitionValid() );
            }

            if( !pFrame || pSh->VisArea().Overlaps( pFrame->getFrameArea() ) )
                bRet = true;
        }
    }

    return bRet;
}

bool SwNode::IsInProtectSect() const
{
    const SwNode* pNd = SwNodeType::Section == m_nNodeType ? m_pStartOfSection : this;
    const SwSectionNode* pSectNd = pNd->FindSectionNode();
    return pSectNd && pSectNd->GetSection().IsProtectFlag();
}

/// Does the node contain anything protected?
/// I.e.: Area/Frame/Table rows/... including the Anchor for
/// Frames/Footnotes/...
bool SwNode::IsProtect() const
{
    const SwNode* pNd = SwNodeType::Section == m_nNodeType ? m_pStartOfSection : this;
    const SwStartNode* pSttNd = pNd->FindSectionNode();
    if( pSttNd && static_cast<const SwSectionNode*>(pSttNd)->GetSection().IsProtectFlag() )
        return true;

    pSttNd = FindTableBoxStartNode();
    if( nullptr != pSttNd )
    {
        SwContentFrame* pCFrame;
        if( IsContentNode() && nullptr != (pCFrame = static_cast<const SwContentNode*>(this)->getLayoutFrame( GetDoc().getIDocumentLayoutAccess().GetCurrentLayout() ) ))
            return pCFrame->IsProtected();

        const SwTableBox* pBox = pSttNd->FindTableNode()->GetTable().
                                        GetTableBox( pSttNd->GetIndex() );
        //Robust #149568
        if( pBox && pBox->GetFrameFormat()->GetProtect().IsContentProtected() )
            return true;
    }

    SwFrameFormat* pFlyFormat = GetFlyFormat();
    if( pFlyFormat )
    {
        if (pFlyFormat->GetProtect().IsContentProtected())
            return true;
        const SwFormatAnchor& rAnchor = pFlyFormat->GetAnchor();
        const SwPosition* pAnchorPos = rAnchor.GetContentAnchor();
        if (!pAnchorPos)
            return false;
        const SwNode& rAnchorNd = pAnchorPos->nNode.GetNode();
        return &rAnchorNd != this && rAnchorNd.IsProtect();
    }

    pSttNd = FindFootnoteStartNode();
    if( nullptr != pSttNd )
    {
        const SwTextFootnote* pTFootnote = GetDoc().GetFootnoteIdxs().SeekEntry(
                                SwNodeIndex( *pSttNd ) );
        if( pTFootnote )
            return pTFootnote->GetTextNode().IsProtect();
    }

    return false;
}

/// Find the PageDesc that is used to format this node. If the Layout is available,
/// we search through that. Else we can only do it the hard way by searching onwards through the nodes.
const SwPageDesc* SwNode::FindPageDesc( SwNodeOffset* pPgDescNdIdx ) const
{
    if ( !GetNodes().IsDocNodes() )
    {
        return nullptr;
    }

    const SwPageDesc* pPgDesc = nullptr;

    const SwContentNode* pNode;
    if( SwNodeType::Start & m_nNodeType )
    {
        SwNodeIndex aIdx( *this );
        pNode = GetNodes().GoNext( &aIdx );
    }
    else if( SwNodeType::End & m_nNodeType )
    {
        SwNodeIndex aIdx( *EndOfSectionNode() );
        pNode = SwNodes::GoPrevious( &aIdx );
    }
    else
    {
        pNode = GetContentNode();
        if( pNode )
            pPgDesc = static_cast<const SwFormatPageDesc&>(pNode->GetAttr( RES_PAGEDESC )).GetPageDesc();
    }

    // Are we going through the layout?
    if( !pPgDesc )
    {
        const SwFrame* pFrame;
        const SwPageFrame* pPage;
        if (pNode && nullptr != (pFrame = pNode->getLayoutFrame(pNode->GetDoc().getIDocumentLayoutAccess().GetCurrentLayout(), nullptr, nullptr)) &&
            nullptr != ( pPage = pFrame->FindPageFrame() ) )
        {
            pPgDesc = pPage->GetPageDesc();
            if ( pPgDescNdIdx )
            {
                *pPgDescNdIdx = pNode->GetIndex();
            }
        }
    }

    if( !pPgDesc )
    {
        // Thus via the nodes array
        const SwDoc& rDoc = GetDoc();
        const SwNode* pNd = this;
        const SwStartNode* pSttNd;
        if( pNd->GetIndex() < GetNodes().GetEndOfExtras().GetIndex() &&
            nullptr != ( pSttNd = pNd->FindFlyStartNode() ) )
        {
            // Find the right Anchor first
            const SwFrameFormat* pFormat = nullptr;
            const SwFrameFormats& rFormats = *rDoc.GetSpzFrameFormats();

            for( size_t n = 0; n < rFormats.size(); ++n )
            {
                const SwFrameFormat* pFrameFormat = rFormats[ n ];
                const SwFormatContent& rContent = pFrameFormat->GetContent();
                if( rContent.GetContentIdx() &&
                    &rContent.GetContentIdx()->GetNode() == static_cast<SwNode const *>(pSttNd) )
                {
                    pFormat = pFrameFormat;
                    break;
                }
            }

            if( pFormat )
            {
                const SwFormatAnchor* pAnchor = &pFormat->GetAnchor();
                if ((RndStdIds::FLY_AT_PAGE != pAnchor->GetAnchorId()) &&
                    pAnchor->GetContentAnchor() )
                {
                    pNd = &pAnchor->GetContentAnchor()->nNode.GetNode();
                    const SwNode* pFlyNd = pNd->FindFlyStartNode();
                    while( pFlyNd )
                    {
                        // Get up through the Anchor
                        size_t n;
                        for( n = 0; n < rFormats.size(); ++n )
                        {
                            const SwFrameFormat* pFrameFormat = rFormats[ n ];
                            const SwNodeIndex* pIdx = pFrameFormat->GetContent().
                                                        GetContentIdx();
                            if( pIdx && pFlyNd == &pIdx->GetNode() )
                            {
                                if( pFormat == pFrameFormat )
                                {
                                    pNd = pFlyNd;
                                    pFlyNd = nullptr;
                                    break;
                                }
                                pAnchor = &pFrameFormat->GetAnchor();
                                if ((RndStdIds::FLY_AT_PAGE == pAnchor->GetAnchorId()) ||
                                    !pAnchor->GetContentAnchor() )
                                {
                                    pFlyNd = nullptr;
                                    break;
                                }

                                pFlyNd = pAnchor->GetContentAnchor()->nNode.
                                        GetNode().FindFlyStartNode();
                                break;
                            }
                        }
                        if( n >= rFormats.size() )
                        {
                            OSL_ENSURE( false, "FlySection, but no Format found" );
                            return nullptr;
                        }
                    }
                }
            }
            // pNd should now contain the correct Anchor or it's still this
        }

        if( pNd->GetIndex() < GetNodes().GetEndOfExtras().GetIndex() )
        {
            if( pNd->GetIndex() > GetNodes().GetEndOfAutotext().GetIndex() )
            {
                pPgDesc = &rDoc.GetPageDesc( 0 );
                pNd = nullptr;
            }
            else
            {
                // Find the Body text node
                if( nullptr != ( pSttNd = pNd->FindHeaderStartNode() ) ||
                    nullptr != ( pSttNd = pNd->FindFooterStartNode() ))
                {
                    // Then find this StartNode in the PageDescs
                    sal_uInt16 nId;
                    UseOnPage eAskUse;
                    if( SwHeaderStartNode == pSttNd->GetStartNodeType())
                    {
                        nId = RES_HEADER;
                        eAskUse = UseOnPage::HeaderShare;
                    }
                    else
                    {
                        nId = RES_FOOTER;
                        eAskUse = UseOnPage::FooterShare;
                    }

                    for( size_t n = rDoc.GetPageDescCnt(); n && !pPgDesc; )
                    {
                        const SwPageDesc& rPgDsc = rDoc.GetPageDesc( --n );
                        const SwFrameFormat* pFormat = &rPgDsc.GetMaster();
                        int nStt = 0, nLast = 1;
                        if( !( eAskUse & rPgDsc.ReadUseOn() )) ++nLast;

                        for( ; nStt < nLast; ++nStt, pFormat = &rPgDsc.GetLeft() )
                        {
                            const SwFrameFormat * pHdFtFormat = nId == RES_HEADER
                                ? static_cast<SwFormatHeader const &>(
                                    pFormat->GetFormatAttr(nId)).GetHeaderFormat()
                                : static_cast<SwFormatFooter const &>(
                                    pFormat->GetFormatAttr(nId)).GetFooterFormat();
                            if( pHdFtFormat )
                            {
                                const SwFormatContent& rContent = pHdFtFormat->GetContent();
                                if( rContent.GetContentIdx() &&
                                    &rContent.GetContentIdx()->GetNode() ==
                                    static_cast<SwNode const *>(pSttNd) )
                                {
                                    pPgDesc = &rPgDsc;
                                    break;
                                }
                            }
                        }
                    }

                    if( !pPgDesc )
                        pPgDesc = &rDoc.GetPageDesc( 0 );
                    pNd = nullptr;
                }
                else if( nullptr != ( pSttNd = pNd->FindFootnoteStartNode() ))
                {
                    // the Anchor can only be in the Body text
                    const SwTextFootnote* pTextFootnote;
                    const SwFootnoteIdxs& rFootnoteArr = rDoc.GetFootnoteIdxs();
                    for( size_t n = 0; n < rFootnoteArr.size(); ++n )
                        if( nullptr != ( pTextFootnote = rFootnoteArr[ n ])->GetStartNode() &&
                            static_cast<SwNode const *>(pSttNd) ==
                            &pTextFootnote->GetStartNode()->GetNode() )
                        {
                            pNd = &pTextFootnote->GetTextNode();
                            break;
                        }
                }
                else
                {
                    // Can only be a page-bound Fly (or something newer).
                    // we can only return the standard here
                    OSL_ENSURE( pNd->FindFlyStartNode(),
                            "Where is this Node?" );

                    pPgDesc = &rDoc.GetPageDesc( 0 );
                    pNd = nullptr;
                }
            }
        }

        if( pNd )
        {
            SwFindNearestNode aInfo( *pNd );
            // Over all Nodes of all PageDescs
            for (const SfxPoolItem* pItem : rDoc.GetAttrPool().GetItemSurrogates(RES_PAGEDESC))
            {
                auto pPageDescItem = dynamic_cast<const SwFormatPageDesc*>(pItem);
                if( pPageDescItem && pPageDescItem->GetDefinedIn() )
                {
                    const sw::BroadcastingModify* pMod = pPageDescItem->GetDefinedIn();
                    if( auto pContentNode = dynamic_cast<const SwContentNode*>( pMod) )
                        aInfo.CheckNode( *pContentNode );
                    else if( auto pFormat = dynamic_cast<const SwFormat*>( pMod) )
                        pFormat->GetInfo( aInfo );
                }
            }

            pNd = aInfo.GetFoundNode();
            if( nullptr != pNd )
            {
                if( pNd->IsContentNode() )
                    pPgDesc = static_cast<const SwFormatPageDesc&>(pNd->GetContentNode()->
                                GetAttr( RES_PAGEDESC )).GetPageDesc();
                else if( pNd->IsTableNode() )
                    pPgDesc = pNd->GetTableNode()->GetTable().
                            GetFrameFormat()->GetPageDesc().GetPageDesc();
                else if( pNd->IsSectionNode() )
                    pPgDesc = pNd->GetSectionNode()->GetSection().
                            GetFormat()->GetPageDesc().GetPageDesc();
                if ( pPgDescNdIdx )
                {
                    *pPgDescNdIdx = pNd->GetIndex();
                }
            }
            if( !pPgDesc )
                pPgDesc = &rDoc.GetPageDesc( 0 );
        }
    }
    return pPgDesc;
}

/// If the node is located in a Fly, we return it formatted accordingly
SwFrameFormat* SwNode::GetFlyFormat() const
{
    SwFrameFormat* pRet = nullptr;
    const SwNode* pSttNd = FindFlyStartNode();
    if( pSttNd )
    {
        if( IsContentNode() )
        {
            SwContentFrame* pFrame = SwIterator<SwContentFrame, SwContentNode, sw::IteratorMode::UnwrapMulti>(*static_cast<const SwContentNode*>(this)).First();
            if( pFrame )
                pRet = pFrame->FindFlyFrame()->GetFormat();
        }
        if( !pRet )
        {
            // The hard way through the Doc is our last way out
            const SwFrameFormats& rFrameFormatTable = *GetDoc().GetSpzFrameFormats();
            for( size_t n = 0; n < rFrameFormatTable.size(); ++n )
            {
                SwFrameFormat* pFormat = rFrameFormatTable[n];
                // Only Writer fly frames can contain Writer nodes.
                if (pFormat->Which() != RES_FLYFRMFMT)
                    continue;
                const SwFormatContent& rContent = pFormat->GetContent();
                if( rContent.GetContentIdx() &&
                    &rContent.GetContentIdx()->GetNode() == pSttNd )
                {
                    pRet = pFormat;
                    break;
                }
            }
        }
    }
    return pRet;
}

SwTableBox* SwNode::GetTableBox() const
{
    SwTableBox* pBox = nullptr;
    const SwNode* pSttNd = FindTableBoxStartNode();
    if( pSttNd )
        pBox = const_cast<SwTableBox*>(pSttNd->FindTableNode()->GetTable().GetTableBox(
                                                    pSttNd->GetIndex() ));
    return pBox;
}

SwStartNode* SwNode::FindSttNodeByType( SwStartNodeType eTyp )
{
    SwStartNode* pTmp = IsStartNode() ? static_cast<SwStartNode*>(this) : m_pStartOfSection;

    while( eTyp != pTmp->GetStartNodeType() && pTmp->GetIndex() )
        pTmp = pTmp->m_pStartOfSection;
    return eTyp == pTmp->GetStartNodeType() ? pTmp : nullptr;
}

const SwTextNode* SwNode::FindOutlineNodeOfLevel(sal_uInt8 const nLvl,
        SwRootFrame const*const pLayout) const
{
    const SwTextNode* pRet = nullptr;
    const SwOutlineNodes& rONds = GetNodes().GetOutLineNds();
    if( MAXLEVEL > nLvl && !rONds.empty() )
    {
        SwOutlineNodes::size_type nPos;
        SwNode* pNd = const_cast<SwNode*>(this);
        bool bCheckFirst = false;
        if( !rONds.Seek_Entry( pNd, &nPos ))
        {
            if (nPos == 0)
                bCheckFirst = true;
        }
        else
        {
            ++nPos;
        }

        if( bCheckFirst )
        {
            // The first OutlineNode comes after the one asking.
            // Test if both are on the same page.
            // If not it's invalid.
            for (nPos = 0; nPos < rONds.size(); ++nPos)
            {
                pRet = rONds[nPos]->GetTextNode();
                if (!pLayout || sw::IsParaPropsNode(*pLayout, *pRet))
                {
                    break;
                }
            }
            if (nPos == rONds.size())
            {
                return nullptr;
            }

            const SwContentNode* pCNd = GetContentNode();

            Point aPt( 0, 0 );
            std::pair<Point, bool> const tmp(aPt, false);
            const SwFrame* pFrame = pRet->getLayoutFrame(pRet->GetDoc().getIDocumentLayoutAccess().GetCurrentLayout(), nullptr, &tmp),
                       * pMyFrame = pCNd ? pCNd->getLayoutFrame(pCNd->GetDoc().getIDocumentLayoutAccess().GetCurrentLayout(), nullptr, &tmp) : nullptr;
            const SwPageFrame* pPgFrame = pFrame ? pFrame->FindPageFrame() : nullptr;
            if( pPgFrame && pMyFrame &&
                pPgFrame->getFrameArea().Top() > pMyFrame->getFrameArea().Top() )
            {
                // The one asking precedes the Page, thus its invalid
                pRet = nullptr;
            }
        }
        else
        {
            for ( ; 0 < nPos; --nPos)
            {
                SwTextNode const*const pNode = rONds[nPos - 1]->GetTextNode();
                if ((nPos == 1 /*as before*/ || pNode->GetAttrOutlineLevel() - 1 <= nLvl)
                    && (!pLayout || sw::IsParaPropsNode(*pLayout, *pNode)))
                {
                    pRet = pNode;
                    break;
                }
            }
        }
    }
    return pRet;
}

static bool IsValidNextPrevNd( const SwNode& rNd )
{
    return SwNodeType::Table == rNd.GetNodeType() ||
           ( SwNodeType::ContentMask & rNd.GetNodeType() ) ||
            ( SwNodeType::End == rNd.GetNodeType() && rNd.StartOfSectionNode() &&
            SwNodeType::Table == rNd.StartOfSectionNode()->GetNodeType() );
}

sal_uInt8 SwNode::HasPrevNextLayNode() const
{
    // assumption: <this> node is a node inside the document nodes array section.

    sal_uInt8 nRet = 0;
    if( IsValidNextPrevNd( *this ))
    {
        SwNodeIndex aIdx( *this, -1 );
        // #i77805# - skip section start and end nodes
        while ( aIdx.GetNode().IsSectionNode() ||
                ( aIdx.GetNode().IsEndNode() &&
                  aIdx.GetNode().StartOfSectionNode()->IsSectionNode() ) )
        {
            --aIdx;
        }
        if( IsValidNextPrevNd( aIdx.GetNode() ))
            nRet |= ND_HAS_PREV_LAYNODE;
        // #i77805# - skip section start and end nodes
        aIdx.Assign(*this, +1);
        while ( aIdx.GetNode().IsSectionNode() ||
                ( aIdx.GetNode().IsEndNode() &&
                  aIdx.GetNode().StartOfSectionNode()->IsSectionNode() ) )
        {
            ++aIdx;
        }
        if( IsValidNextPrevNd( aIdx.GetNode() ))
            nRet |= ND_HAS_NEXT_LAYNODE;
    }
    return nRet;
}

void SwNode::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    const char* pName = "???";
    switch (GetNodeType())
    {
    case SwNodeType::End:
        pName = "end";
        break;
    case SwNodeType::Start:
    case SwNodeType::Text:
    case SwNodeType::Ole:
        abort(); // overridden
    case SwNodeType::Table:
        pName = "table";
        break;
    case SwNodeType::Grf:
        pName = "grf";
        break;
    default: break;
    }
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST(pName));

    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("type"), BAD_CAST(OString::number(static_cast<sal_uInt8>(GetNodeType())).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("index"), BAD_CAST(OString::number(sal_Int32(GetIndex())).getStr()));

    switch (GetNodeType())
    {
        case SwNodeType::Grf:
        {
            auto pNoTextNode = static_cast<const SwNoTextNode*>(this);
            const tools::PolyPolygon* pContour = pNoTextNode->HasContour();
            if (pContour)
            {
                (void)xmlTextWriterStartElement(pWriter, BAD_CAST("pContour"));
                for (sal_uInt16 i = 0; i < pContour->Count(); ++i)
                {
                    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("polygon"));
                    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("index"),
                                                BAD_CAST(OString::number(i).getStr()));
                    const tools::Polygon& rPolygon = pContour->GetObject(i);
                    for (sal_uInt16 j = 0; j < rPolygon.GetSize(); ++j)
                    {
                        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("point"));
                        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("index"),
                                                    BAD_CAST(OString::number(j).getStr()));
                        const Point& rPoint = rPolygon.GetPoint(j);
                        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("x"),
                                                    BAD_CAST(OString::number(rPoint.X()).getStr()));
                        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("y"),
                                                    BAD_CAST(OString::number(rPoint.Y()).getStr()));
                        (void)xmlTextWriterEndElement(pWriter);
                    }
                    (void)xmlTextWriterEndElement(pWriter);
                }
                (void)xmlTextWriterEndElement(pWriter);
            }
        }
        break;
        default:
            break;
    }

    (void)xmlTextWriterEndElement(pWriter);
    if (GetNodeType() == SwNodeType::End)
        (void)xmlTextWriterEndElement(pWriter); // end start node
}

SwStartNode::SwStartNode( const SwNodeIndex &rWhere, const SwNodeType nNdType,
                            SwStartNodeType eSttNd )
    : SwNode( rWhere, nNdType ), m_eStartNodeType( eSttNd )
{
    if( !rWhere.GetIndex() )
    {
        SwNodes& rNodes = const_cast<SwNodes&> (rWhere.GetNodes());
        rNodes.InsertNode( this, rWhere );
        m_pStartOfSection = this;
    }
    // Just do this temporarily until the EndNode is inserted
    m_pEndOfSection = reinterpret_cast<SwEndNode*>(this);
}

SwStartNode::SwStartNode( SwNodes& rNodes, SwNodeOffset nPos )
    : SwNode( rNodes, nPos, SwNodeType::Start ), m_eStartNodeType( SwNormalStartNode )
{
    if( !nPos )
    {
        rNodes.InsertNode( this, nPos );
        m_pStartOfSection = this;
    }
    // Just do this temporarily until the EndNode is inserted
    m_pEndOfSection = reinterpret_cast<SwEndNode*>(this);
}

void SwStartNode::CheckSectionCondColl() const
{
//FEATURE::CONDCOLL
    SwNodeIndex aIdx( *this );
    SwNodeOffset nEndIdx = EndOfSectionIndex();
    const SwNodes& rNds = GetNodes();
    SwContentNode* pCNd;
    while( nullptr != ( pCNd = rNds.GoNext( &aIdx )) && pCNd->GetIndex() < nEndIdx )
        pCNd->ChkCondColl();
//FEATURE::CONDCOLL
}

void SwStartNode::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    const char* pName = "???";
    switch (GetNodeType())
    {
    case SwNodeType::Table:
        pName = "table";
        break;
    default:
        switch(GetStartNodeType())
        {
        case SwNormalStartNode:
            pName = "start";
            break;
        case SwTableBoxStartNode:
            pName = "tablebox";
            break;
        case SwFlyStartNode:
            pName = "fly";
            break;
        case SwFootnoteStartNode:
            pName = "footnote";
            break;
        case SwHeaderStartNode:
            pName = "header";
            break;
        case SwFooterStartNode:
            pName = "footer";
            break;
        }
        break;
    }

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST(pName));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("type"), BAD_CAST(OString::number(static_cast<sal_uInt8>(GetNodeType())).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("index"), BAD_CAST(OString::number(sal_Int32(GetIndex())).getStr()));

    if (IsTableNode())
    {
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("attrset"));
        GetTableNode()->GetTable().GetFrameFormat()->GetAttrSet().dumpAsXml(pWriter);
        (void)xmlTextWriterEndElement(pWriter);
    }
    else if (GetStartNodeType() == SwTableBoxStartNode)
    {
        if (SwTableBox* pBox = GetTableBox())
            (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("rowspan"), BAD_CAST(OString::number(pBox->getRowSpan()).getStr()));
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("attrset"));
        if (SwTableBox* pBox = GetTableBox())
            pBox->GetFrameFormat()->GetAttrSet().dumpAsXml(pWriter);
        (void)xmlTextWriterEndElement(pWriter);
    }

    // (void)xmlTextWriterEndElement(pWriter); - it is a start node, so don't end, will make xml better nested
}


/** Insert a node into the array
 *
 * The StartOfSection pointer is set to the given node.
 *
 * The EndOfSection pointer of the corresponding start node is set to this node.
 *
 * @param rWhere position where the node shoul be inserted
 * @param rSttNd the start note of the section
 */

SwEndNode::SwEndNode( const SwNodeIndex &rWhere, SwStartNode& rSttNd )
    : SwNode( rWhere, SwNodeType::End )
{
    m_pStartOfSection = &rSttNd;
    m_pStartOfSection->m_pEndOfSection = this;
}

SwEndNode::SwEndNode( SwNodes& rNds, SwNodeOffset nPos, SwStartNode& rSttNd )
    : SwNode( rNds, nPos, SwNodeType::End )
{
    m_pStartOfSection = &rSttNd;
    m_pStartOfSection->m_pEndOfSection = this;
}

SwContentNode::SwContentNode( const SwNodeIndex &rWhere, const SwNodeType nNdType,
                            SwFormatColl *pColl )
    : SwNode( rWhere, nNdType )
    , m_aCondCollListener( *this )
    , m_pCondColl( nullptr )
    , mbSetModifyAtAttr( false )
{
    if(pColl)
        pColl->Add(this);
}

SwContentNode::~SwContentNode()
{
    // The base class SwClient of SwFrame excludes itself from the dependency list!
    // Thus, we need to delete all Frames in the dependency list.
    if (!IsTextNode()) // see ~SwTextNode
    {
        DelFrames(nullptr);
    }

    m_aCondCollListener.EndListeningAll();
    m_pCondColl = nullptr;

    if ( mpAttrSet && mbSetModifyAtAttr )
        const_cast<SwAttrSet*>(static_cast<const SwAttrSet*>(mpAttrSet.get()))->SetModifyAtAttr( nullptr );
    InvalidateInSwCache(RES_OBJECTDYING);
}
void SwContentNode::UpdateAttr(const SwUpdateAttr& rUpdate)
{
    if (GetNodes().IsDocNodes()
            && IsTextNode()
            && RES_ATTRSET_CHG == rUpdate.getWhichAttr())
        static_cast<SwTextNode*>(this)->SetCalcHiddenCharFlags();
    CallSwClientNotify(sw::LegacyModifyHint(&rUpdate, &rUpdate));
}

void SwContentNode::SwClientNotify( const SwModify&, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::SwLegacyModify)
    {
        auto pLegacyHint = static_cast<const sw::LegacyModifyHint*>(&rHint);
        const sal_uInt16 nWhich = pLegacyHint->GetWhich();
        InvalidateInSwCache(nWhich);

        bool bSetParent = false;
        bool bCalcHidden = false;
        SwFormatColl* pFormatColl = nullptr;
        switch(nWhich)
        {
            case RES_OBJECTDYING:
                {
                    SwFormat* pFormat = pLegacyHint->m_pNew
                            ? static_cast<SwFormat*>(static_cast<const SwPtrMsgPoolItem*>(pLegacyHint->m_pNew)->pObject)
                            : nullptr;
                    // Do not mangle pointers if it is the upper-most format!
                    if(pFormat && GetRegisteredIn() == pFormat)
                    {
                        if(pFormat->GetRegisteredIn())
                        {
                            // If Parent, register anew in the new Parent
                            pFormat->GetRegisteredIn()->Add(this);
                            pFormatColl = GetFormatColl();
                        }
                        else
                            EndListeningAll();
                        bSetParent = true;
                    }
                }
                break;

            case RES_FMT_CHG:
                // If the Format parent was switched, register the Attrset at the new one
                // Skip own Modify!
                if(GetpSwAttrSet()
                        && pLegacyHint->m_pNew
                        && static_cast<const SwFormatChg*>(pLegacyHint->m_pNew)->pChangedFormat == GetRegisteredIn())
                {
                    pFormatColl = GetFormatColl();
                    bSetParent = true;
                }
                break;

            case RES_ATTRSET_CHG:
                if (GetNodes().IsDocNodes()
                        && IsTextNode()
                        && pLegacyHint->m_pOld
                        && SfxItemState::SET == pLegacyHint->m_pOld->StaticWhichCast(RES_ATTRSET_CHG).GetChgSet()->GetItemState(RES_CHRATR_HIDDEN, false))
                    bCalcHidden = true;
                break;

            case RES_UPDATE_ATTR:
                // RES_UPDATE_ATTR _should_ always contain a SwUpdateAttr hint in old and new.
                // However, faking one with just a basic SfxPoolItem setting a WhichId has been observed.
                // This makes the crude "WhichId" type divert from the true type, which is bad.
                // Thus we are asserting here, but falling back to an proper
                // hint instead. so that we at least will not spread such poison further.
#ifdef DBG_UTIL
                if(pLegacyHint->m_pNew != pLegacyHint->m_pOld)
                {
                    auto pBT = sal::backtrace_get(20);
                    SAL_WARN("sw.core", "UpdateAttr not matching! " << sal::backtrace_to_string(pBT.get()));
                }
#endif
                assert(pLegacyHint->m_pNew == pLegacyHint->m_pOld);
                assert(dynamic_cast<const SwUpdateAttr*>(pLegacyHint->m_pNew));
                const SwUpdateAttr aFallbackHint(0,0,0);
                const SwUpdateAttr& rUpdateAttr = pLegacyHint->m_pNew ? *static_cast<const SwUpdateAttr*>(pLegacyHint->m_pNew) : aFallbackHint;
                UpdateAttr(rUpdateAttr);
                return;
        }
        if(bSetParent && GetpSwAttrSet())
            AttrSetHandleHelper::SetParent(mpAttrSet, *this, pFormatColl, pFormatColl);
        if(bCalcHidden)
            static_cast<SwTextNode*>(this)->SetCalcHiddenCharFlags();
        CallSwClientNotify(rHint);
    }
    else if (auto pModifyChangedHint = dynamic_cast<const sw::ModifyChangedHint*>(&rHint))
    {
        m_pCondColl = const_cast<SwFormatColl*>(static_cast<const SwFormatColl*>(pModifyChangedHint->m_pNew));
    }
    else if(auto pCondCollCondChgHint = dynamic_cast<const sw::CondCollCondChg*>(&rHint))
    {
        ChkCondColl(&pCondCollCondChgHint->m_rColl);
    }
}

bool SwContentNode::InvalidateNumRule()
{
    SwNumRule* pRule = nullptr;
    const SfxPoolItem* pItem;
    if( GetNodes().IsDocNodes() &&
        nullptr != ( pItem = GetNoCondAttr( RES_PARATR_NUMRULE, true )) &&
        !static_cast<const SwNumRuleItem*>(pItem)->GetValue().isEmpty() &&
        nullptr != (pRule = GetDoc().FindNumRulePtr(
                                static_cast<const SwNumRuleItem*>(pItem)->GetValue() ) ) )
    {
        pRule->SetInvalidRule( true );
    }
    return nullptr != pRule;
}

SwContentFrame *SwContentNode::getLayoutFrame( const SwRootFrame* _pRoot,
    const SwPosition *const pPos,
    std::pair<Point, bool> const*const pViewPosAndCalcFrame) const
{
    return static_cast<SwContentFrame*>( ::GetFrameOfModify( _pRoot, *this, FRM_CNTNT,
                                            pPos, pViewPosAndCalcFrame));
}

SwRect SwContentNode::FindLayoutRect( const bool bPrtArea, const Point* pPoint ) const
{
    SwRect aRet;
    std::pair<Point, bool> tmp;
    if (pPoint)
    {
        tmp.first = *pPoint;
        tmp.second = false;
    }
    SwContentFrame* pFrame = static_cast<SwContentFrame*>( ::GetFrameOfModify( nullptr, *this,
                                FRM_CNTNT, nullptr, pPoint ? &tmp : nullptr) );
    if( pFrame )
        aRet = bPrtArea ? pFrame->getFramePrintArea() : pFrame->getFrameArea();
    return aRet;
}

SwRect SwContentNode::FindPageFrameRect() const
{
    SwRect aRet;
    SwFrame* pFrame = ::GetFrameOfModify( nullptr, *this, FRM_CNTNT );
    if( pFrame && nullptr != ( pFrame = pFrame->FindPageFrame() ))
        aRet = pFrame->getFrameArea();
    return aRet;
}

sal_Int32 SwContentNode::Len() const { return 0; }

SwFormatColl *SwContentNode::ChgFormatColl( SwFormatColl *pNewColl )
{
    OSL_ENSURE( pNewColl, "Collectionpointer is 0." );
    SwFormatColl *pOldColl = GetFormatColl();

    if( pNewColl != pOldColl )
    {
        pNewColl->Add( this );

        // Set the Parent of out AutoAttributes to the new Collection
        if( GetpSwAttrSet() )
            AttrSetHandleHelper::SetParent( mpAttrSet, *this, pNewColl, pNewColl );

        SetCondFormatColl( nullptr );

        if( !IsModifyLocked() )
        {
            assert(dynamic_cast<SwTextFormatColl*>(pNewColl));
            ChkCondColl(static_cast<SwTextFormatColl*>(pNewColl));
            SwFormatChg aTmp1( pOldColl );
            SwFormatChg aTmp2( pNewColl );
            SwClientNotify( *this, sw::LegacyModifyHint(&aTmp1, &aTmp2) );
        }
    }
    InvalidateInSwCache(RES_ATTRSET_CHG);
    return pOldColl;
}

bool SwContentNode::GoNext(SwIndex * pIdx, sal_uInt16 nMode ) const
{
    bool bRet = true;
    if( pIdx->GetIndex() < Len() )
    {
        if( !IsTextNode() )
            ++(*pIdx);
        else
        {
            const SwTextNode& rTNd = *GetTextNode();
            sal_Int32 nPos = pIdx->GetIndex();
            assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());
            sal_Int32 nDone = 0;
            sal_uInt16 nItrMode = ( CRSR_SKIP_CELLS & nMode ) ?
                                    CharacterIteratorMode::SKIPCELL :
                                    CharacterIteratorMode::SKIPCONTROLCHARACTER;
            nPos = g_pBreakIt->GetBreakIter()->nextCharacters( rTNd.GetText(), nPos,
                               g_pBreakIt->GetLocale( rTNd.GetLang( nPos ) ),
                               nItrMode, 1, nDone );

            // Check if nPos is inside hidden text range:
            if ( CRSR_SKIP_HIDDEN & nMode )
            {
                sal_Int32 nHiddenStart;
                sal_Int32 nHiddenEnd;
                SwScriptInfo::GetBoundsOfHiddenRange( rTNd, nPos, nHiddenStart, nHiddenEnd );
                if ( nHiddenStart != COMPLETE_STRING && nHiddenStart != nPos )
                     nPos = nHiddenEnd;
            }

            if( 1 == nDone )
                *pIdx = nPos;
            else
                bRet = false;
        }
    }
    else
        bRet = false;
    return bRet;
}

bool SwContentNode::GoPrevious(SwIndex * pIdx, sal_uInt16 nMode ) const
{
    bool bRet = true;
    if( pIdx->GetIndex() > 0 )
    {
        if( !IsTextNode() )
            --(*pIdx);
        else
        {
            const SwTextNode& rTNd = *GetTextNode();
            sal_Int32 nPos = pIdx->GetIndex();
            assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());
            sal_Int32 nDone = 0;
            sal_uInt16 nItrMode = ( CRSR_SKIP_CELLS & nMode ) ?
                                    CharacterIteratorMode::SKIPCELL :
                                    CharacterIteratorMode::SKIPCONTROLCHARACTER;
            nPos = g_pBreakIt->GetBreakIter()->previousCharacters( rTNd.GetText(), nPos,
                               g_pBreakIt->GetLocale( rTNd.GetLang( nPos ) ),
                               nItrMode, 1, nDone );

            // Check if nPos is inside hidden text range:
            if ( CRSR_SKIP_HIDDEN & nMode )
            {
                sal_Int32 nHiddenStart;
                sal_Int32 nHiddenEnd;
                SwScriptInfo::GetBoundsOfHiddenRange( rTNd, nPos, nHiddenStart, nHiddenEnd );
                if ( nHiddenStart != COMPLETE_STRING )
                     nPos = nHiddenStart;
            }

            if( 1 == nDone )
                *pIdx = nPos;
            else
                bRet = false;
        }
    }
    else
        bRet = false;
    return bRet;
}

/**
 * Creates all Views for the Doc for this Node.
 * The created ContentFrames are attached to the corresponding Layout.
 */
void SwContentNode::MakeFramesForAdjacentContentNode(SwContentNode& rNode)
{
    OSL_ENSURE( &rNode != this,
            "No ContentNode or CopyNode and new Node identical." );

    if( !HasWriterListeners() || &rNode == this )   // Do we actually have Frames?
        return;

    SwFrame *pFrame;
    SwLayoutFrame *pUpper;
    // Create Frames for Nodes which come after the Table?
    OSL_ENSURE( FindTableNode() == rNode.FindTableNode(), "Table confusion" );

    SwNode2Layout aNode2Layout( *this, rNode.GetIndex() );

    while( nullptr != (pUpper = aNode2Layout.UpperFrame( pFrame, rNode )) )
    {
        if (pUpper->getRootFrame()->HasMergedParas()
            && !rNode.IsCreateFrameWhenHidingRedlines())
        {
            continue;
        }
        SwFrame *pNew = rNode.MakeFrame( pUpper );
        pNew->Paste( pUpper, pFrame );
        // #i27138#
        // notify accessibility paragraphs objects about changed
        // CONTENT_FLOWS_FROM/_TO relation.
        // Relation CONTENT_FLOWS_FROM for next paragraph will change
        // and relation CONTENT_FLOWS_TO for previous paragraph will change.
#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
        if ( pNew->IsTextFrame() )
        {
            SwViewShell* pViewShell( pNew->getRootFrame()->GetCurrShell() );
            if ( pViewShell && pViewShell->GetLayout() &&
                 pViewShell->GetLayout()->IsAnyShellAccessible() )
            {
                auto pNext = pNew->FindNextCnt( true );
                auto pPrev = pNew->FindPrevCnt();
                pViewShell->InvalidateAccessibleParaFlowRelation(
                            pNext ? pNext->DynCastTextFrame() : nullptr,
                            pPrev ? pPrev->DynCastTextFrame() : nullptr );
            }
        }
#endif
    }
}

/**
 * Deletes all Views from the Doc for this Node.
 * The ContentFrames are removed from the corresponding Layout.
 */
void SwContentNode::DelFrames(SwRootFrame const*const pLayout)
{
    if( !HasWriterListeners() )
        return;

    SwIterator<SwContentFrame, SwContentNode, sw::IteratorMode::UnwrapMulti> aIter(*this);
    for( SwContentFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next() )
    {
        if (pLayout && pLayout != pFrame->getRootFrame())
        {
            continue; // skip it
        }
        if (pFrame->IsTextFrame())
        {
            if (sw::MergedPara * pMerged =
                    static_cast<SwTextFrame *>(pFrame)->GetMergedPara())
            {
                if (this != pMerged->pFirstNode)
                {
                    // SwNodes::RemoveNode iterates *backwards* - so
                    // ensure there are no more extents pointing to this
                    // node as SwFrame::InvalidatePage() will access them.
                    // Note: cannot send via SwClientNotify from dtor
                    // because that would access deleted wrong-lists
                    sw::UpdateMergedParaForDelete(*pMerged, true,
                            *static_cast<SwTextNode*>(this), 0, Len());
                    if (this == pMerged->pParaPropsNode)
                    {
                        // otherwise pointer should have been updated to a different node
                        assert(this == pMerged->pLastNode);
                        assert(pMerged->extents.empty());
                        for (SwNodeOffset i = pMerged->pLastNode->GetIndex() - 1;;
                                --i)
                        {
                            assert(pMerged->pFirstNode->GetIndex() <= i);
                            SwNode *const pNode(GetNodes()[i]);
                            if (pNode->IsTextNode()
                                && pNode->GetRedlineMergeFlag() != Merge::Hidden)
                            {
                                pMerged->pParaPropsNode = pNode->GetTextNode();
                                break;
                            }
                        }
                        assert(pMerged->listener.IsListeningTo(pMerged->pParaPropsNode));
                    }
                    assert(GetIndex() <= pMerged->pLastNode->GetIndex());
                    if (this == pMerged->pLastNode)
                    {
                        // tdf#130680 find the previous node that is a
                        // listener of pMerged; see CheckParaRedlineMerge()
                        for (SwNodeOffset i = GetIndex() - 1;
                             this == pMerged->pLastNode; --i)
                        {
                            SwNode *const pNode = GetNodes()[i];
                            if (pNode->IsTextNode())
                            {
                                pMerged->pLastNode = pNode->GetTextNode();
                            }
                            else if (SwEndNode const*const pEnd = pNode->GetEndNode())
                            {
                                SwStartNode const*const pStart(pEnd->StartOfSectionNode());
                                i = pStart->GetIndex(); // skip table or section
                            }
                        }
                        assert(pMerged->pFirstNode->GetIndex() <= pMerged->pLastNode->GetIndex());
                        assert(pMerged->listener.IsListeningTo(pMerged->pLastNode));
                    }
                    // avoid re-parenting mess (ModifyChangedHint)
                    pMerged->listener.EndListening(this);
                    continue; // don't delete
                }
            }
        // #i27138#
        // notify accessibility paragraphs objects about changed
        // CONTENT_FLOWS_FROM/_TO relation.
        // Relation CONTENT_FLOWS_FROM for current next paragraph will change
        // and relation CONTENT_FLOWS_TO for current previous paragraph will change.
#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
            SwViewShell* pViewShell( pFrame->getRootFrame()->GetCurrShell() );
            if ( pViewShell && pViewShell->GetLayout() &&
                 pViewShell->GetLayout()->IsAnyShellAccessible() )
            {
                auto pNext = pFrame->FindNextCnt( true );
                auto pPrev = pFrame->FindPrevCnt();
                pViewShell->InvalidateAccessibleParaFlowRelation(
                            pNext ? pNext->DynCastTextFrame() : nullptr,
                            pPrev ? pPrev->DynCastTextFrame() : nullptr );
            }
#endif
        }

        if( pFrame->IsFollow() )
        {
            SwContentFrame* pMaster = pFrame->FindMaster();
            pMaster->SetFollow( pFrame->GetFollow() );
        }
        pFrame->SetFollow( nullptr );//So it doesn't get funny ideas.
                                //Otherwise it could be possible that a follow
                                //gets destroyed before its master. Following
                                //the now invalid pointer will then lead to an
                                //illegal memory access. The chain can be
                                //crushed here because we'll destroy all of it
                                //anyway.

        if( pFrame->GetUpper() && pFrame->IsInFootnote() && !pFrame->GetIndNext() &&
            !pFrame->GetIndPrev() )
        {
            SwFootnoteFrame *pFootnote = pFrame->FindFootnoteFrame();
            OSL_ENSURE( pFootnote, "You promised a FootnoteFrame?" );
            SwContentFrame* pCFrame;
            if( !pFootnote->GetFollow() && !pFootnote->GetMaster() &&
                nullptr != ( pCFrame = pFootnote->GetRefFromAttr()) && pCFrame->IsFollow() )
            {
                OSL_ENSURE( pCFrame->IsTextFrame(), "NoTextFrame has Footnote?" );
                pCFrame->FindMaster()->Prepare( PrepareHint::FootnoteInvalidationGone );
            }
        }
        pFrame->Cut();
        SwFrame::DestroyFrame(pFrame);
    }
}

SwContentNode *SwContentNode::JoinNext()
{
    return this;
}

/// Get info from Modify
bool SwContentNode::GetInfo( SfxPoolItem& rInfo ) const
{
    switch( rInfo.Which() )
    {
    case RES_AUTOFMT_DOCNODE:
        if( &GetNodes() == static_cast<SwAutoFormatGetDocNode&>(rInfo).pNodes )
        {
            return false;
        }
        break;

    case RES_FINDNEARESTNODE:
        if( static_cast<const SwFormatPageDesc&>(GetAttr( RES_PAGEDESC )).GetPageDesc() )
            static_cast<SwFindNearestNode&>(rInfo).CheckNode( *this );
        return true;

    case RES_CONTENT_VISIBLE:
        {
            static_cast<SwPtrMsgPoolItem&>(rInfo).pObject =
                SwIterator<SwFrame, SwContentNode, sw::IteratorMode::UnwrapMulti>(*this).First();
        }
        return false;
    }

    return sw::BroadcastingModify::GetInfo( rInfo );
}

/// @param rAttr the attribute to set
bool SwContentNode::SetAttr(const SfxPoolItem& rAttr )
{
    if( !GetpSwAttrSet() ) // Have the Nodes created by the corresponding AttrSets
        NewAttrSet( GetDoc().GetAttrPool() );

    OSL_ENSURE( GetpSwAttrSet(), "Why did't we create an AttrSet?");

    InvalidateInSwCache(RES_ATTRSET_CHG);

    bool bRet = false;
    // If Modify is locked, we do not send any Modifys
    if( IsModifyLocked() ||
        ( !HasWriterListeners() &&  RES_PARATR_NUMRULE != rAttr.Which() ))
    {
        bRet = nullptr != AttrSetHandleHelper::Put( mpAttrSet, *this, rAttr );
    }
    else
    {
        SwAttrSet aOld( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() ),
                  aNew( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() );
        bRet = AttrSetHandleHelper::Put_BC( mpAttrSet, *this, rAttr, &aOld, &aNew );
        if( bRet )
            sw::ClientNotifyAttrChg(*this, *GetpSwAttrSet(), aOld, aNew);
    }
    return bRet;
}

bool SwContentNode::SetAttr( const SfxItemSet& rSet )
{
    InvalidateInSwCache(RES_ATTRSET_CHG);

    const SfxPoolItem* pFnd = nullptr;
    if( SfxItemState::SET == rSet.GetItemState( RES_AUTO_STYLE, false, &pFnd ) )
    {
        OSL_ENSURE( rSet.Count() == 1, "SetAutoStyle mixed with other attributes?!" );
        const SwFormatAutoFormat* pTmp = static_cast<const SwFormatAutoFormat*>(pFnd);

        // If there already is an attribute set (usually containing a numbering
        // item), we have to merge the attribute of the new set into the old set:
        bool bSetParent = true;
        if ( GetpSwAttrSet() )
        {
            bSetParent = false;
            AttrSetHandleHelper::Put( mpAttrSet, *this, *pTmp->GetStyleHandle() );
        }
        else
        {
            mpAttrSet = pTmp->GetStyleHandle();
        }

        if ( bSetParent )
        {
            // If the content node has a conditional style, we have to set the
            // string item containing the correct conditional style name (the
            // style name property has already been set during the import!)
            // In case we do not have a conditional style, we make use of the
            // fact that nobody else uses the attribute set behind the handle.
            // FME 2007-07-10 #i78124# If autostyle does not have a parent,
            // the string is empty.
            const SfxPoolItem* pNameItem = nullptr;
            if ( nullptr != GetCondFormatColl() ||
                 SfxItemState::SET != mpAttrSet->GetItemState( RES_FRMATR_STYLE_NAME, false, &pNameItem ) ||
                 static_cast<const SfxStringItem*>(pNameItem)->GetValue().isEmpty() )
                AttrSetHandleHelper::SetParent( mpAttrSet, *this, &GetAnyFormatColl(), GetFormatColl() );
            else
                const_cast<SfxItemSet*>(mpAttrSet.get())->SetParent( &GetFormatColl()->GetAttrSet() );
        }

        return true;
    }

    if( !GetpSwAttrSet() ) // Have the AttrsSets created by the corresponding Nodes
        NewAttrSet( GetDoc().GetAttrPool() );

    bool bRet = false;
    // If Modify is locked, do not send any Modifys
    if ( IsModifyLocked() ||
         ( !HasWriterListeners() &&
           SfxItemState::SET != rSet.GetItemState( RES_PARATR_NUMRULE, false ) ) )
    {
        // Some special treatment for Attributes
        bRet = AttrSetHandleHelper::Put( mpAttrSet, *this, rSet );
    }
    else
    {
        SwAttrSet aOld( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() ),
                  aNew( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() );
        bRet = AttrSetHandleHelper::Put_BC( mpAttrSet, *this, rSet, &aOld, &aNew );
        if( bRet )
            sw::ClientNotifyAttrChg(*this, *GetpSwAttrSet(), aOld, aNew);
    }
    return bRet;
}

// With nWhich it takes the Hint from the Delta array
bool SwContentNode::ResetAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 )
{
    if( !GetpSwAttrSet() )
        return false;

    InvalidateInSwCache(RES_ATTRSET_CHG);

    // If Modify is locked, do not send out any Modifys
    if( IsModifyLocked() )
    {
        sal_uInt16 nDel = 0;
        if ( !nWhich2 || nWhich2 < nWhich1 )
        {
            nDel = ClearItemsFromAttrSet( { nWhich1 } );
        }
        else
            nDel = AttrSetHandleHelper::ClearItem_BC( mpAttrSet, *this, nWhich1, nWhich2, nullptr, nullptr );

        if( !GetpSwAttrSet()->Count() ) // Empty? Delete
            mpAttrSet.reset();
        return 0 != nDel;
    }

    // No valid area defined?
    if( !nWhich2 || nWhich2 < nWhich1 )
        nWhich2 = nWhich1; // Then set only this Item to 1st Id

    SwAttrSet aOld( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() ),
              aNew( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() );
    bool bRet = 0 != AttrSetHandleHelper::ClearItem_BC( mpAttrSet, *this, nWhich1, nWhich2, &aOld, &aNew );

    if( bRet )
    {
        sw::ClientNotifyAttrChg(*this, *GetpSwAttrSet(), aOld, aNew);

        if( !GetpSwAttrSet()->Count() ) // Empty?, delete it
            mpAttrSet.reset();
    }
    return bRet;
}

bool SwContentNode::ResetAttr( const std::vector<sal_uInt16>& rWhichArr )
{
    if( !GetpSwAttrSet() )
        return false;

    InvalidateInSwCache(RES_ATTRSET_CHG);
    // If Modify is locked, do not send out any Modifys
    sal_uInt16 nDel = 0;
    if( IsModifyLocked() )
    {
        nDel = ClearItemsFromAttrSet( rWhichArr );
    }
    else
    {
        SwAttrSet aOld( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() ),
                  aNew( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() );

        for ( const auto& rWhich : rWhichArr )
            if( AttrSetHandleHelper::ClearItem_BC( mpAttrSet, *this, rWhich, &aOld, &aNew ))
                ++nDel;

        if( nDel )
            sw::ClientNotifyAttrChg(*this, *GetpSwAttrSet(), aOld, aNew);
    }
    if( !GetpSwAttrSet()->Count() ) // Empty?, delete it
        mpAttrSet.reset();
    return 0 != nDel ;
}

sal_uInt16 SwContentNode::ResetAllAttr()
{
    if( !GetpSwAttrSet() )
        return 0;
    InvalidateInSwCache(RES_ATTRSET_CHG);

    // If Modify is locked, do not send out any Modifys
    if( IsModifyLocked() )
    {
        sal_uInt16 nDel = ClearItemsFromAttrSet( { 0 } );
        if( !GetpSwAttrSet()->Count() ) // Empty? Delete
            mpAttrSet.reset();
        return nDel;
    }

    SwAttrSet aOld( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() ),
              aNew( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() );
    bool bRet = 0 != AttrSetHandleHelper::ClearItem_BC( mpAttrSet, *this, 0, &aOld, &aNew );

    if( bRet )
    {
        sw::ClientNotifyAttrChg(*this, *GetpSwAttrSet(), aOld, aNew);
        if( !GetpSwAttrSet()->Count() ) // Empty? Delete
            mpAttrSet.reset();
    }
    return aNew.Count();
}

bool SwContentNode::GetAttr( SfxItemSet& rSet ) const
{
    if( rSet.Count() )
        rSet.ClearItem();

    const SwAttrSet& rAttrSet = GetSwAttrSet();
    return rSet.Set( rAttrSet );
}

sal_uInt16 SwContentNode::ClearItemsFromAttrSet( const std::vector<sal_uInt16>& rWhichIds )
{
    sal_uInt16 nRet = 0;
    if ( rWhichIds.empty() )
        return nRet;

    OSL_ENSURE( GetpSwAttrSet(), "no item set" );
    SwAttrSet aNewAttrSet( *GetpSwAttrSet() );
    for ( const auto& rWhichId : rWhichIds )
    {
        nRet = nRet + aNewAttrSet.ClearItem( rWhichId );
    }
    if ( nRet )
        AttrSetHandleHelper::GetNewAutoStyle( mpAttrSet, *this, aNewAttrSet );

    return nRet;
}

const SfxPoolItem* SwContentNode::GetNoCondAttr( sal_uInt16 nWhich,
                                               bool bInParents ) const
{
    const SfxPoolItem* pFnd = nullptr;
    if( m_pCondColl && m_pCondColl->GetRegisteredIn() )
    {
        if( !GetpSwAttrSet() || ( SfxItemState::SET != GetpSwAttrSet()->GetItemState(
                    nWhich, false, &pFnd ) && bInParents ))
        {
            (void)static_cast<const SwFormat*>(GetRegisteredIn())->GetItemState( nWhich, bInParents, &pFnd );
        }
    }
    // undo change of issue #i51029#
    // Note: <GetSwAttrSet()> returns <mpAttrSet>, if set, otherwise it returns
    //       the attribute set of the paragraph style, which is valid for the
    //       content node - see file <node.hxx>
    else
    {
        GetSwAttrSet().GetItemState( nWhich, bInParents, &pFnd );
    }
    return pFnd;
}

static bool lcl_CheckMaxLength(SwNode const& rPrev, SwNode const& rNext)
{
    if (rPrev.GetNodeType() != rNext.GetNodeType())
    {
        return false;
    }
    if (!rPrev.IsTextNode())
    {
        return true;
    }

    // Check if a node can contain the other (order is not significant)
    return rPrev.GetTextNode()->GetSpaceLeft() > rNext.GetTextNode()->Len();
}

/// Can we join two Nodes?
/// We can return the 2nd position in pIdx.
bool SwContentNode::CanJoinNext( SwNodeIndex* pIdx ) const
{
    const SwNodes& rNds = GetNodes();
    SwNodeIndex aIdx( *this, 1 );

    const SwNode* pNd = this;
    while( aIdx < rNds.Count()-1 &&
        (( pNd = &aIdx.GetNode())->IsSectionNode() ||
            ( pNd->IsEndNode() && pNd->StartOfSectionNode()->IsSectionNode() )))
        ++aIdx;

    if (rNds.Count()-1 == aIdx.GetIndex())
        return false;
    if (!lcl_CheckMaxLength(*this, *pNd))
    {
        return false;
    }
    if( pIdx )
        *pIdx = aIdx;
    return true;
}

/// Can we join two Nodes?
/// We can return the 2nd position in pIdx.
bool SwContentNode::CanJoinPrev( SwNodeIndex* pIdx ) const
{
    SwNodeIndex aIdx( *this, -1 );

    const SwNode* pNd = this;
    while( aIdx.GetIndex() &&
        (( pNd = &aIdx.GetNode())->IsSectionNode() ||
            ( pNd->IsEndNode() && pNd->StartOfSectionNode()->IsSectionNode() )))
        --aIdx;

    if (SwNodeOffset(0) == aIdx.GetIndex())
        return false;
    if (!lcl_CheckMaxLength(*pNd, *this))
    {
        return false;
    }
    if( pIdx )
        *pIdx = aIdx;
    return true;
}

void SwContentNode::SetCondFormatColl(SwFormatColl* pColl)
{
    if( !((!pColl && m_pCondColl) || ( pColl && !m_pCondColl ) ||
        ( pColl && pColl != m_pCondColl->GetRegisteredIn() )) )
        return;

    SwFormatColl* pOldColl = GetCondFormatColl();
    m_aCondCollListener.EndListeningAll();
    if(pColl)
        m_aCondCollListener.StartListening(pColl);
    m_pCondColl = pColl;
    if(GetpSwAttrSet())
        AttrSetHandleHelper::SetParent(mpAttrSet, *this, &GetAnyFormatColl(), GetFormatColl());

    if(!IsModifyLocked())
    {
        SwFormatChg aTmp1(pOldColl ? pOldColl : GetFormatColl());
        SwFormatChg aTmp2(pColl ? pColl : GetFormatColl());
        CallSwClientNotify(sw::LegacyModifyHint(&aTmp1, &aTmp2));
    }
    InvalidateInSwCache(RES_ATTRSET_CHG);
}

bool SwContentNode::IsAnyCondition( SwCollCondition& rTmp ) const
{
    const SwNodes& rNds = GetNodes();
    {
        Master_CollCondition nCond = Master_CollCondition::NONE;
        const SwStartNode* pSttNd = StartOfSectionNode();
        while( pSttNd )
        {
            switch( pSttNd->GetNodeType() )
            {
            case SwNodeType::Table:      nCond = Master_CollCondition::PARA_IN_TABLEBODY; break;
            case SwNodeType::Section:    nCond = Master_CollCondition::PARA_IN_SECTION; break;

            default:
                switch( pSttNd->GetStartNodeType() )
                {
                case SwTableBoxStartNode:
                    {
                        nCond = Master_CollCondition::PARA_IN_TABLEBODY;
                        const SwTableNode* pTableNd = pSttNd->FindTableNode();
                        const SwTableBox* pBox;
                        if( pTableNd && nullptr != ( pBox = pTableNd->GetTable().
                            GetTableBox(pSttNd->GetIndex()) ) &&
                            pBox->IsInHeadline( &pTableNd->GetTable() ) )
                            nCond = Master_CollCondition::PARA_IN_TABLEHEAD;
                    }
                    break;
                case SwFlyStartNode:        nCond = Master_CollCondition::PARA_IN_FRAME; break;
                case SwFootnoteStartNode:
                    {
                        nCond = Master_CollCondition::PARA_IN_FOOTNOTE;
                        const SwFootnoteIdxs& rFootnoteArr = rNds.GetDoc().GetFootnoteIdxs();
                        const SwTextFootnote* pTextFootnote;
                        const SwNode* pSrchNd = pSttNd;

                        for( size_t n = 0; n < rFootnoteArr.size(); ++n )
                            if( nullptr != ( pTextFootnote = rFootnoteArr[ n ])->GetStartNode() &&
                                pSrchNd == &pTextFootnote->GetStartNode()->GetNode() )
                            {
                                if( pTextFootnote->GetFootnote().IsEndNote() )
                                    nCond = Master_CollCondition::PARA_IN_ENDNOTE;
                                break;
                            }
                    }
                    break;
                case SwHeaderStartNode:     nCond = Master_CollCondition::PARA_IN_HEADER; break;
                case SwFooterStartNode:     nCond = Master_CollCondition::PARA_IN_FOOTER; break;
                case SwNormalStartNode:     break;
                }
            }

            if( nCond != Master_CollCondition::NONE )
            {
                rTmp.SetCondition( nCond, 0 );
                return true;
            }
            pSttNd = pSttNd->GetIndex()
                        ? pSttNd->StartOfSectionNode()
                        : nullptr;
        }
    }

    {
        SwOutlineNodes::size_type nPos;
        const SwOutlineNodes& rOutlNds = rNds.GetOutLineNds();
        if( !rOutlNds.empty() )
        {
            if( !rOutlNds.Seek_Entry( const_cast<SwContentNode*>(this), &nPos ) && nPos )
                --nPos;
            if( nPos < rOutlNds.size() &&
                rOutlNds[ nPos ]->GetIndex() < GetIndex() )
            {
                SwTextNode* pOutlNd = rOutlNds[ nPos ]->GetTextNode();

                if( pOutlNd->IsOutline())
                {
                    rTmp.SetCondition( Master_CollCondition::PARA_IN_OUTLINE, pOutlNd->GetAttrOutlineLevel() - 1 );
                    return true;
                }
            }
        }
    }

    return false;
}

void SwContentNode::ChkCondColl(const SwTextFormatColl* pColl)
{
    if(pColl != GetRegisteredIn())
    {
        SAL_WARN("sw.core", "Wrong cond collection, skipping check of Cond Colls.");
        return;
    }
    if(&GetNodes() != &GetDoc().GetNodes())
    {
        SAL_WARN("sw.core", "Nodes amiss, skipping check of Cond Colls.");
        return;
    }
    // Check, just to be sure
    if( RES_CONDTXTFMTCOLL != GetFormatColl()->Which() )
        return;

    SwCollCondition aTmp( nullptr, Master_CollCondition::NONE, 0 );
    const SwCollCondition* pCColl;

    bool bDone = false;

    if( IsAnyCondition( aTmp ))
    {
        pCColl = static_cast<SwConditionTextFormatColl*>(GetFormatColl())
            ->HasCondition( aTmp );

        if (pCColl)
        {
            SetCondFormatColl( pCColl->GetTextFormatColl() );
            bDone = true;
        }
    }

    if (bDone)
        return;

    if( IsTextNode() && static_cast<SwTextNode*>(this)->GetNumRule())
    {
        // Is at which Level in a list?
        aTmp.SetCondition( Master_CollCondition::PARA_IN_LIST,
                        static_cast<SwTextNode*>(this)->GetActualListLevel() );
        pCColl = static_cast<SwConditionTextFormatColl*>(GetFormatColl())->
                        HasCondition( aTmp );
    }
    else
        pCColl = nullptr;

    if( pCColl )
        SetCondFormatColl( pCColl->GetTextFormatColl() );
    else if( m_pCondColl )
        SetCondFormatColl( nullptr );
}

// #i42921#
SvxFrameDirection SwContentNode::GetTextDirection( const SwPosition& rPos,
                                     const Point* pPt ) const
{
    SvxFrameDirection nRet = SvxFrameDirection::Unknown;

    Point aPt;
    if( pPt )
        aPt = *pPt;

    // #i72024# - No format of the frame, because this can cause recursive layout actions
    std::pair<Point, bool> const tmp(aPt, false);
    SwFrame* pFrame = getLayoutFrame(GetDoc().getIDocumentLayoutAccess().GetCurrentLayout(), &rPos, &tmp);

    if ( pFrame )
    {
        if ( pFrame->IsVertical() )
        {
            if (pFrame->IsVertLRBT())
                nRet = SvxFrameDirection::Vertical_LR_BT;
            else if (pFrame->IsRightToLeft())
                nRet = SvxFrameDirection::Vertical_LR_TB;
            else
                nRet = SvxFrameDirection::Vertical_RL_TB;
        }
        else
        {
            if ( pFrame->IsRightToLeft() )
                nRet = SvxFrameDirection::Horizontal_RL_TB;
            else
                nRet = SvxFrameDirection::Horizontal_LR_TB;
        }
    }

    return nRet;
}

std::unique_ptr<SwOLENodes> SwContentNode::CreateOLENodesArray( const SwFormatColl& rColl, bool bOnlyWithInvalidSize )
{
    std::unique_ptr<SwOLENodes> pNodes;
    SwIterator<SwContentNode,SwFormatColl> aIter( rColl );
    for( SwContentNode* pNd = aIter.First(); pNd; pNd = aIter.Next() )
    {
        SwOLENode *pONd = pNd->GetOLENode();
        if ( pONd && (!bOnlyWithInvalidSize || pONd->IsOLESizeInvalid()) )
        {
            if ( !pNodes  )
                pNodes.reset(new SwOLENodes);
            pNodes->push_back( pONd );
        }
    }

    return pNodes;
}

drawinglayer::attribute::SdrAllFillAttributesHelperPtr SwContentNode::getSdrAllFillAttributesHelper() const
{
    return drawinglayer::attribute::SdrAllFillAttributesHelperPtr();
}

/*
 * Document Interface Access
 */
const IDocumentSettingAccess* SwNode::getIDocumentSettingAccess() const { return &GetDoc().GetDocumentSettingManager(); }
const IDocumentDeviceAccess& SwNode::getIDocumentDeviceAccess() const { return GetDoc().getIDocumentDeviceAccess(); }
const IDocumentRedlineAccess& SwNode::getIDocumentRedlineAccess() const { return GetDoc().getIDocumentRedlineAccess(); }
const IDocumentStylePoolAccess& SwNode::getIDocumentStylePoolAccess() const { return GetDoc().getIDocumentStylePoolAccess(); }
const IDocumentDrawModelAccess& SwNode::getIDocumentDrawModelAccess() const { return GetDoc().getIDocumentDrawModelAccess(); }
const IDocumentLayoutAccess& SwNode::getIDocumentLayoutAccess() const { return GetDoc().getIDocumentLayoutAccess(); }
IDocumentLayoutAccess& SwNode::getIDocumentLayoutAccess() { return GetDoc().getIDocumentLayoutAccess(); }
const IDocumentLinksAdministration& SwNode::getIDocumentLinksAdministration() const { return GetDoc().getIDocumentLinksAdministration(); }
IDocumentLinksAdministration& SwNode::getIDocumentLinksAdministration() { return GetDoc().getIDocumentLinksAdministration(); }
const IDocumentFieldsAccess& SwNode::getIDocumentFieldsAccess() const { return GetDoc().getIDocumentFieldsAccess(); }
IDocumentFieldsAccess& SwNode::getIDocumentFieldsAccess() { return GetDoc().getIDocumentFieldsAccess(); }
IDocumentContentOperations& SwNode::getIDocumentContentOperations() { return GetDoc().getIDocumentContentOperations(); }
IDocumentListItems& SwNode::getIDocumentListItems() { return GetDoc().getIDocumentListItems(); } // #i83479#

const IDocumentMarkAccess* SwNode::getIDocumentMarkAccess() const { return GetDoc().getIDocumentMarkAccess(); }
IStyleAccess& SwNode::getIDocumentStyleAccess() { return GetDoc().GetIStyleAccess(); }

bool SwNode::IsInRedlines() const
{
    const SwDoc& rDoc = GetDoc();

    return rDoc.getIDocumentRedlineAccess().IsInRedlines(*this);
}

void SwNode::AddAnchoredFly(SwFrameFormat *const pFlyFormat)
{
    assert(pFlyFormat);
    assert(&pFlyFormat->GetAnchor(false).GetContentAnchor()->nNode.GetNode() == this);
    // check node type, cf. SwFormatAnchor::SetAnchor()
    assert(IsTextNode() || IsStartNode() || IsTableNode());
    m_aAnchoredFlys.push_back(pFlyFormat);
}

void SwNode::RemoveAnchoredFly(SwFrameFormat *const pFlyFormat)
{
    assert(pFlyFormat);
    // cannot assert this in Remove because it is called when new anchor is already set
//    assert(&pFlyFormat->GetAnchor(false).GetContentAnchor()->nNode.GetNode() == this);
    assert(IsTextNode() || IsStartNode() || IsTableNode());
    auto it(std::find(m_aAnchoredFlys.begin(), m_aAnchoredFlys.end(), pFlyFormat));
    assert(it != m_aAnchoredFlys.end());
    m_aAnchoredFlys.erase(it);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
