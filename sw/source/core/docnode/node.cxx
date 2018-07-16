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
#include <editeng/frmdiritem.hxx>
#include <editeng/protitem.hxx>
#include <tools/gen.hxx>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <txtftn.hxx>
#include <ftnfrm.hxx>
#include <doc.hxx>
#include <docary.hxx>
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
#include <svl/itemiter.hxx>

using namespace ::com::sun::star::i18n;


/*
 * Some local helper functions for the attribute set handle of a content node.
 * Since the attribute set of a content node may not be modified directly,
 * we always have to create a new SwAttrSet, do the modifications, and get
 * a new handle from the style access
 */

namespace AttrSetHandleHelper
{

void GetNewAutoStyle( std::shared_ptr<const SfxItemSet>& rpAttrSet,
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

void SetParent( std::shared_ptr<const SfxItemSet>& rpAttrSet,
                const SwContentNode& rNode,
                const SwFormat* pParentFormat,
                const SwFormat* pConditionalFormat )
{
    const SwAttrSet* pAttrSet = static_cast<const SwAttrSet*>(rpAttrSet.get());
    OSL_ENSURE( pAttrSet, "no SwAttrSet" );
    OSL_ENSURE( pParentFormat || !pConditionalFormat, "ConditionalFormat without ParentFormat?" );

    const SwAttrSet* pParentSet = pParentFormat ? &pParentFormat->GetAttrSet() : nullptr;

    if ( pParentSet != pAttrSet->GetParent() )
    {
        SwAttrSet aNewSet( *pAttrSet );
        aNewSet.SetParent( pParentSet );
        aNewSet.ClearItem( RES_FRMATR_STYLE_NAME );
        aNewSet.ClearItem( RES_FRMATR_CONDITIONAL_STYLE_NAME );
        OUString sVal;

        if ( pParentFormat )
        {
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
}

const SfxPoolItem* Put( std::shared_ptr<const SfxItemSet>& rpAttrSet,
                        const SwContentNode& rNode,
                        const SfxPoolItem& rAttr )
{
    SwAttrSet aNewSet( static_cast<const SwAttrSet&>(*rpAttrSet) );
    const SfxPoolItem* pRet = aNewSet.Put( rAttr );
    if ( pRet )
        GetNewAutoStyle( rpAttrSet, rNode, aNewSet );
    return pRet;
}

bool Put( std::shared_ptr<const SfxItemSet>& rpAttrSet, const SwContentNode& rNode,
         const SfxItemSet& rSet )
{
    SwAttrSet aNewSet( static_cast<const SwAttrSet&>(*rpAttrSet) );

    // #i76273# Robust
    std::unique_ptr<SfxItemSet> pStyleNames;
    if ( SfxItemState::SET == rSet.GetItemState( RES_FRMATR_STYLE_NAME, false ) )
    {
        pStyleNames.reset(new SfxItemSet( *aNewSet.GetPool(), svl::Items<RES_FRMATR_STYLE_NAME, RES_FRMATR_CONDITIONAL_STYLE_NAME>{} ));
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

bool Put_BC( std::shared_ptr<const SfxItemSet>& rpAttrSet,
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

bool Put_BC( std::shared_ptr<const SfxItemSet>& rpAttrSet,
            const SwContentNode& rNode, const SfxItemSet& rSet,
            SwAttrSet* pOld, SwAttrSet* pNew )
{
    SwAttrSet aNewSet( static_cast<const SwAttrSet&>(*rpAttrSet) );

    // #i76273# Robust
    std::unique_ptr<SfxItemSet> pStyleNames;
    if ( SfxItemState::SET == rSet.GetItemState( RES_FRMATR_STYLE_NAME, false ) )
    {
        pStyleNames.reset(new SfxItemSet( *aNewSet.GetPool(), svl::Items<RES_FRMATR_STYLE_NAME, RES_FRMATR_CONDITIONAL_STYLE_NAME>{} ));
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

sal_uInt16 ClearItem_BC( std::shared_ptr<const SfxItemSet>& rpAttrSet,
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

sal_uInt16 ClearItem_BC( std::shared_ptr<const SfxItemSet>& rpAttrSet,
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
    if( IsEndNode() && 0 == m_pStartOfSection->StartOfSectionIndex() )
        return 0;

    sal_uInt16 nLevel;
    const SwNode* pNode = IsStartNode() ? this : m_pStartOfSection;
    for( nLevel = 1; 0 != pNode->StartOfSectionIndex(); ++nLevel )
        pNode = pNode->m_pStartOfSection;
    return IsEndNode() ? nLevel-1 : nLevel;
}

#ifdef DBG_UTIL
long SwNode::s_nSerial = 0;
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
    if( rWhere.GetIndex() )
    {
        SwNodes& rNodes = const_cast<SwNodes&> (rWhere.GetNodes());
        SwNode* pNd = rNodes[ rWhere.GetIndex() -1 ];
        rNodes.InsertNode( this, rWhere );
        if( nullptr == ( m_pStartOfSection = pNd->GetStartNode()) )
        {
            m_pStartOfSection = pNd->m_pStartOfSection;
            if( pNd->GetEndNode() )     // Skip EndNode ? Section
            {
                pNd = m_pStartOfSection;
                m_pStartOfSection = pNd->m_pStartOfSection;
            }
        }
    }
}

/** Inserts a node into the rNodes array at the rWhere position
 *
 * @param rNodes the variable array in that the node will be inserted
 * @param nPos position within the array where the node will be inserted
 * @param nNdType the type of node to insert
 */
SwNode::SwNode( SwNodes& rNodes, sal_uLong nPos, const SwNodeType nNdType )
    : m_nNodeType( nNdType )
    , m_nAFormatNumLvl( 0 )
    , m_bIgnoreDontExpand( false)
    , m_eMerge(Merge::None)
#ifdef DBG_UTIL
    , m_nSerial( s_nSerial++)
#endif
    , m_pStartOfSection( nullptr )
{
    if( nPos )
    {
        SwNode* pNd = rNodes[ nPos - 1 ];
        rNodes.InsertNode( this, nPos );
        if( nullptr == ( m_pStartOfSection = pNd->GetStartNode()) )
        {
            m_pStartOfSection = pNd->m_pStartOfSection;
            if( pNd->GetEndNode() )     // Skip EndNode ? Section!
            {
                pNd = m_pStartOfSection;
                m_pStartOfSection = pNd->m_pStartOfSection;
            }
        }
    }
}

SwNode::~SwNode()
{
    assert(!m_pAnchoredFlys || GetDoc()->IsInDtor()); // must all be deleted
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
        pSh = GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();

    if( pSh )
    {
        const SwFrame* pFrame;
        if( pNd && nullptr != ( pFrame = pNd->getLayoutFrame( pSh->GetLayout(), nullptr, nullptr, false ) ) )
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

            if( !pFrame || pSh->VisArea().IsOver( pFrame->getFrameArea() ) )
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

    if( nullptr != ( pSttNd = FindTableBoxStartNode() ) )
    {
        SwContentFrame* pCFrame;
        if( IsContentNode() && nullptr != (pCFrame = static_cast<const SwContentNode*>(this)->getLayoutFrame( GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() ) ))
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

    if( nullptr != ( pSttNd = FindFootnoteStartNode() ) )
    {
        const SwTextFootnote* pTFootnote = GetDoc()->GetFootnoteIdxs().SeekEntry(
                                SwNodeIndex( *pSttNd ) );
        if( pTFootnote )
            return pTFootnote->GetTextNode().IsProtect();
    }

    return false;
}

/// Find the PageDesc that is used to format this node. If the Layout is available,
/// we search through that. Else we can only do it the hard way by searching onwards through the nodes.
const SwPageDesc* SwNode::FindPageDesc( size_t* pPgDescNdIdx ) const
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
        if( pNode && nullptr != ( pFrame = pNode->getLayoutFrame( pNode->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), nullptr, nullptr, false/*bCalcLay*/ ) ) &&
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
        const SwDoc* pDoc = GetDoc();
        const SwNode* pNd = this;
        const SwStartNode* pSttNd;
        if( pNd->GetIndex() < GetNodes().GetEndOfExtras().GetIndex() &&
            nullptr != ( pSttNd = pNd->FindFlyStartNode() ) )
        {
            // Find the right Anchor first
            const SwFrameFormat* pFormat = nullptr;
            const SwFrameFormats& rFormats = *pDoc->GetSpzFrameFormats();

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
                pPgDesc = &pDoc->GetPageDesc( 0 );
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

                    for( size_t n = pDoc->GetPageDescCnt(); n && !pPgDesc; )
                    {
                        const SwPageDesc& rPgDsc = pDoc->GetPageDesc( --n );
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
                        pPgDesc = &pDoc->GetPageDesc( 0 );
                    pNd = nullptr;
                }
                else if( nullptr != ( pSttNd = pNd->FindFootnoteStartNode() ))
                {
                    // the Anchor can only be in the Body text
                    const SwTextFootnote* pTextFootnote;
                    const SwFootnoteIdxs& rFootnoteArr = pDoc->GetFootnoteIdxs();
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

                    pPgDesc = &pDoc->GetPageDesc( 0 );
                    pNd = nullptr;
                }
            }
        }

        if( pNd )
        {
            SwFindNearestNode aInfo( *pNd );
            // Over all Nodes of all PageDescs
            sal_uInt32 i, nMaxItems = pDoc->GetAttrPool().GetItemCount2( RES_PAGEDESC );
            for( i = 0; i < nMaxItems; ++i )
            {
                const SfxPoolItem* pItem;
                if( nullptr != (pItem = pDoc->GetAttrPool().GetItem2( RES_PAGEDESC, i ) ) &&
                    static_cast<const SwFormatPageDesc*>(pItem)->GetDefinedIn() )
                {
                    const SwModify* pMod = static_cast<const SwFormatPageDesc*>(pItem)->GetDefinedIn();
                    if( auto pContentNode = dynamic_cast<const SwContentNode*>( pMod) )
                        aInfo.CheckNode( *pContentNode );
                    else if( auto pFormat = dynamic_cast<const SwFormat*>( pMod) )
                        pFormat->GetInfo( aInfo );
                }
            }

            if( nullptr != ( pNd = aInfo.GetFoundNode() ))
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
                pPgDesc = &pDoc->GetPageDesc( 0 );
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
            const SwFrameFormats& rFrameFormatTable = *GetDoc()->GetSpzFrameFormats();
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

const SwTextNode* SwNode::FindOutlineNodeOfLevel( sal_uInt8 nLvl ) const
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
            if( nPos )
                nPos = nPos-1;
            else
                bCheckFirst = true;
        }

        if( bCheckFirst )
        {
            // The first OutlineNode comes after the one asking. Test if it points to the same node.
            // If not it's invalid.
            pRet = rONds[0]->GetTextNode();

            const SwContentNode* pCNd = GetContentNode();

            Point aPt( 0, 0 );
            const SwFrame* pFrame = pRet->getLayoutFrame( pRet->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, nullptr, false ),
                       * pMyFrame = pCNd ? pCNd->getLayoutFrame( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, nullptr, false ) : nullptr;
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
            // Or at the Field and get it from there!
            while( nPos &&
                   nLvl < ( pRet = rONds[nPos]->GetTextNode() )
                    ->GetAttrOutlineLevel() - 1 )
                --nPos;

            if( !nPos )     // Get separately when 0
                pRet = rONds[0]->GetTextNode();
        }
    }
    return pRet;
}

inline bool IsValidNextPrevNd( const SwNode& rNd )
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
        abort(); // overridden
    case SwNodeType::Table:
        pName = "table";
        break;
    case SwNodeType::Grf:
        pName = "grf";
        break;
    case SwNodeType::Ole:
        pName = "ole";
        break;
    default: break;
    }
    xmlTextWriterStartElement(pWriter, BAD_CAST(pName));

    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("type"), BAD_CAST(OString::number(static_cast<sal_uInt8>(GetNodeType())).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("index"), BAD_CAST(OString::number(GetIndex()).getStr()));

    xmlTextWriterEndElement(pWriter);
    if (GetNodeType() == SwNodeType::End)
        xmlTextWriterEndElement(pWriter); // end start node
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

SwStartNode::SwStartNode( SwNodes& rNodes, sal_uLong nPos )
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
    sal_uLong nEndIdx = EndOfSectionIndex();
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
    case SwNodeType::Section:
        pName = "section";
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

    xmlTextWriterStartElement(pWriter, BAD_CAST(pName));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("type"), BAD_CAST(OString::number(static_cast<sal_uInt8>(GetNodeType())).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("index"), BAD_CAST(OString::number(GetIndex()).getStr()));

    if (IsTableNode())
    {
        xmlTextWriterStartElement(pWriter, BAD_CAST("attrset"));
        GetTableNode()->GetTable().GetFrameFormat()->GetAttrSet().dumpAsXml(pWriter);
        xmlTextWriterEndElement(pWriter);
    }

    // xmlTextWriterEndElement(pWriter); - it is a start node, so don't end, will make xml better nested
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

SwEndNode::SwEndNode( SwNodes& rNds, sal_uLong nPos, SwStartNode& rSttNd )
    : SwNode( rNds, nPos, SwNodeType::End )
{
    m_pStartOfSection = &rSttNd;
    m_pStartOfSection->m_pEndOfSection = this;
}

SwContentNode::SwContentNode( const SwNodeIndex &rWhere, const SwNodeType nNdType,
                            SwFormatColl *pColl )
    : SwModify( pColl ),     // CursorsShell, FrameFormat,
    SwNode( rWhere, nNdType ),
    m_pCondColl( nullptr ),
    mbSetModifyAtAttr( false )
{
}

SwContentNode::~SwContentNode()
{
    // The base class SwClient of SwFrame excludes itself from the dependency list!
    // Thus, we need to delete all Frames in the dependency list.
    if (!IsTextNode()) // see ~SwTextNode
    {
        DelFrames(false);
    }

    m_pCondColl.reset();

    if ( mpAttrSet.get() && mbSetModifyAtAttr )
        const_cast<SwAttrSet*>(static_cast<const SwAttrSet*>(mpAttrSet.get()))->SetModifyAtAttr( nullptr );
}

void SwContentNode::Modify( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue )
{
    sal_uInt16 nWhich = pOldValue ? pOldValue->Which() :
                    pNewValue ? pNewValue->Which() : 0 ;

    switch( nWhich )
    {
    case RES_OBJECTDYING :
        if (pNewValue)
        {
            SwFormat * pFormat = static_cast<SwFormat *>( static_cast<const SwPtrMsgPoolItem *>(pNewValue)->pObject );

            // Do not mangle pointers if it is the upper-most format!
            if( GetRegisteredIn() == pFormat )
            {
                if( pFormat->GetRegisteredIn() )
                {
                    // If Parent, register anew in the new Parent
                    pFormat->GetRegisteredIn()->Add( this );
                    if ( GetpSwAttrSet() )
                        AttrSetHandleHelper::SetParent( mpAttrSet, *this, GetFormatColl(), GetFormatColl() );
                }
                else
                {
                    // Else register anyways when dying
                    EndListeningAll();
                    if ( GetpSwAttrSet() )
                        AttrSetHandleHelper::SetParent( mpAttrSet, *this, nullptr, nullptr );
                }
            }
        }
        break;

    case RES_FMT_CHG:
        // If the Format parent was switched, register the Attrset at the new one
        // Skip own Modify!
        if( GetpSwAttrSet() && pNewValue &&
            static_cast<const SwFormatChg*>(pNewValue)->pChangedFormat == GetRegisteredIn() )
        {
            // Attach Set to the new parent
            AttrSetHandleHelper::SetParent( mpAttrSet, *this, GetFormatColl(), GetFormatColl() );
        }
        break;

//FEATURE::CONDCOLL
    case RES_CONDCOLL_CONDCHG:
        if( pNewValue && static_cast<const SwCondCollCondChg*>(pNewValue)->pChangedFormat == GetRegisteredIn() &&
            &GetNodes() == &GetDoc()->GetNodes() )
        {
            ChkCondColl();
        }
        return ;    // Do not pass through to the base class/Frames
//FEATURE::CONDCOLL

    case RES_ATTRSET_CHG:
        if (GetNodes().IsDocNodes() && IsTextNode() && pOldValue)
        {
            if( SfxItemState::SET == static_cast<const SwAttrSetChg*>(pOldValue)->GetChgSet()->GetItemState(
                RES_CHRATR_HIDDEN, false ) )
            {
                static_cast<SwTextNode*>(this)->SetCalcHiddenCharFlags();
            }
        }
        break;

    case RES_UPDATE_ATTR:
        if (GetNodes().IsDocNodes() && IsTextNode() && pNewValue)
        {
            const sal_uInt16 nTmp = static_cast<const SwUpdateAttr*>(pNewValue)->getWhichAttr();
            if ( RES_ATTRSET_CHG == nTmp )
            {
                // TODO: anybody wants to do some optimization here?
                static_cast<SwTextNode*>(this)->SetCalcHiddenCharFlags();
            }
        }
        break;
    }

    NotifyClients( pOldValue, pNewValue );
}

bool SwContentNode::InvalidateNumRule()
{
    SwNumRule* pRule = nullptr;
    const SfxPoolItem* pItem;
    if( GetNodes().IsDocNodes() &&
        nullptr != ( pItem = GetNoCondAttr( RES_PARATR_NUMRULE, true )) &&
        !static_cast<const SwNumRuleItem*>(pItem)->GetValue().isEmpty() &&
        nullptr != (pRule = GetDoc()->FindNumRulePtr(
                                static_cast<const SwNumRuleItem*>(pItem)->GetValue() ) ) )
    {
        pRule->SetInvalidRule( true );
    }
    return nullptr != pRule;
}

SwContentFrame *SwContentNode::getLayoutFrame( const SwRootFrame* _pRoot,
    const Point* pPoint, const SwPosition *pPos, const bool bCalcFrame ) const
{
    return static_cast<SwContentFrame*>( ::GetFrameOfModify( _pRoot, *this, FRM_CNTNT,
                                            pPoint, pPos, bCalcFrame ));
}

SwRect SwContentNode::FindLayoutRect( const bool bPrtArea, const Point* pPoint ) const
{
    SwRect aRet;
    SwContentFrame* pFrame = static_cast<SwContentFrame*>( ::GetFrameOfModify( nullptr, *this,
                                            FRM_CNTNT, pPoint ) );
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

//FEATURE::CONDCOLL
        // TODO: HACK: We need to recheck this condition according to the new template!
        if( true /*pNewColl */ )
        {
            SetCondFormatColl( nullptr );
        }
//FEATURE::CONDCOLL

        if( !IsModifyLocked() )
        {
            SwFormatChg aTmp1( pOldColl );
            SwFormatChg aTmp2( pNewColl );
            SwContentNode::Modify( &aTmp1, &aTmp2 );
        }
    }
    if ( IsInCache() )
    {
        SwFrame::GetCache().Delete( this );
        SetInCache( false );
    }
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
void SwContentNode::MakeFrames( SwContentNode& rNode )
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
        SwFrame *pNew = rNode.MakeFrame( pUpper );
        pNew->Paste( pUpper, pFrame );
        // #i27138#
        // notify accessibility paragraphs objects about changed
        // CONTENT_FLOWS_FROM/_TO relation.
        // Relation CONTENT_FLOWS_FROM for next paragraph will change
        // and relation CONTENT_FLOWS_TO for previous paragraph will change.
        if ( pNew->IsTextFrame() )
        {
            SwViewShell* pViewShell( pNew->getRootFrame()->GetCurrShell() );
            if ( pViewShell && pViewShell->GetLayout() &&
                 pViewShell->GetLayout()->IsAnyShellAccessible() )
            {
                pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTextFrame*>(pNew->FindNextCnt( true )),
                            dynamic_cast<SwTextFrame*>(pNew->FindPrevCnt()) );
            }
        }
    }
}

/**
 * Deletes all Views from the Doc for this Node.
 * The ContentFrames are removed from the corresponding Layout.
 *
 * An input param to identify if the acc table should be disposed.
 */
void SwContentNode::DelFrames(bool /*removeme*/)
{
    if( !HasWriterListeners() )
        return;

    SwIterator<SwContentFrame, SwContentNode, sw::IteratorMode::UnwrapMulti> aIter(*this);
    for( SwContentFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next() )
    {
        // #i27138#
        // notify accessibility paragraphs objects about changed
        // CONTENT_FLOWS_FROM/_TO relation.
        // Relation CONTENT_FLOWS_FROM for current next paragraph will change
        // and relation CONTENT_FLOWS_TO for current previous paragraph will change.
        if ( pFrame->IsTextFrame() )
        {
            SwViewShell* pViewShell( pFrame->getRootFrame()->GetCurrShell() );
            if ( pViewShell && pViewShell->GetLayout() &&
                 pViewShell->GetLayout()->IsAnyShellAccessible() )
            {
                pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTextFrame*>(pFrame->FindNextCnt( true )),
                            dynamic_cast<SwTextFrame*>(pFrame->FindPrevCnt()) );
            }
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
                pCFrame->FindMaster()->Prepare( PREP_FTN_GONE );
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

    return SwModify::GetInfo( rInfo );
}

/// @param rAttr the attribute to set
bool SwContentNode::SetAttr(const SfxPoolItem& rAttr )
{
    if( !GetpSwAttrSet() ) // Have the Nodes created by the corresponding AttrSets
        NewAttrSet( GetDoc()->GetAttrPool() );

    OSL_ENSURE( GetpSwAttrSet(), "Why did't we create an AttrSet?");

    if ( IsInCache() )
    {
        SwFrame::GetCache().Delete( this );
        SetInCache( false );
    }

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
        {
            SwAttrSetChg aChgOld( *GetpSwAttrSet(), aOld );
            SwAttrSetChg aChgNew( *GetpSwAttrSet(), aNew );
            ModifyNotification( &aChgOld, &aChgNew ); // Send all changed ones
        }
    }
    return bRet;
}

bool SwContentNode::SetAttr( const SfxItemSet& rSet )
{
    if ( IsInCache() )
    {
        SwFrame::GetCache().Delete( this );
        SetInCache( false );
    }

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
        NewAttrSet( GetDoc()->GetAttrPool() );

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
        {
            // Some special treatment for Attributes
            SwAttrSetChg aChgOld( *GetpSwAttrSet(), aOld );
            SwAttrSetChg aChgNew( *GetpSwAttrSet(), aNew );
            ModifyNotification( &aChgOld, &aChgNew ); // Send out all changed ones
        }
    }
    return bRet;
}

// With nWhich it takes the Hint from the Delta array
bool SwContentNode::ResetAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 )
{
    if( !GetpSwAttrSet() )
        return false;

    if ( IsInCache() )
    {
        SwFrame::GetCache().Delete( this );
        SetInCache( false );
    }

    // If Modify is locked, do not send out any Modifys
    if( IsModifyLocked() )
    {
        sal_uInt16 nDel = 0;
        if ( !nWhich2 || nWhich2 < nWhich1 )
        {
            std::vector<sal_uInt16> aClearWhichIds;
            aClearWhichIds.push_back( nWhich1 );
            nDel = ClearItemsFromAttrSet( aClearWhichIds );
        }
        else
            nDel = AttrSetHandleHelper::ClearItem_BC( mpAttrSet, *this, nWhich1, nWhich2, nullptr, nullptr );

        if( !GetpSwAttrSet()->Count() ) // Empt? Delete
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
        SwAttrSetChg aChgOld( *GetpSwAttrSet(), aOld );
        SwAttrSetChg aChgNew( *GetpSwAttrSet(), aNew );
        ModifyNotification( &aChgOld, &aChgNew ); // All changed ones are sent

        if( !GetpSwAttrSet()->Count() ) // Empty?, delete it
            mpAttrSet.reset();
    }
    return bRet;
}

bool SwContentNode::ResetAttr( const std::vector<sal_uInt16>& rWhichArr )
{
    if( !GetpSwAttrSet() )
        return false;

    if ( IsInCache() )
    {
        SwFrame::GetCache().Delete( this );
        SetInCache( false );
    }

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

        std::vector<sal_uInt16>::const_iterator it;
        for ( it = rWhichArr.begin(); it != rWhichArr.end(); ++it )
            if( AttrSetHandleHelper::ClearItem_BC( mpAttrSet, *this, *it, &aOld, &aNew ))
                ++nDel;

        if( nDel )
        {
            SwAttrSetChg aChgOld( *GetpSwAttrSet(), aOld );
            SwAttrSetChg aChgNew( *GetpSwAttrSet(), aNew );
            ModifyNotification( &aChgOld, &aChgNew ); // All changed ones are sent
        }
    }
    if( !GetpSwAttrSet()->Count() ) // Empty?, delete it
        mpAttrSet.reset();
    return 0 != nDel ;
}

sal_uInt16 SwContentNode::ResetAllAttr()
{
    if( !GetpSwAttrSet() )
        return 0;

    if ( IsInCache() )
    {
        SwFrame::GetCache().Delete( this );
        SetInCache( false );
    }

    // If Modify is locked, do not send out any Modifys
    if( IsModifyLocked() )
    {
        std::vector<sal_uInt16> aClearWhichIds;
        aClearWhichIds.push_back(0);
        sal_uInt16 nDel = ClearItemsFromAttrSet( aClearWhichIds );
        if( !GetpSwAttrSet()->Count() ) // Empty? Delete
            mpAttrSet.reset();
        return nDel;
    }

    SwAttrSet aOld( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() ),
              aNew( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() );
    bool bRet = 0 != AttrSetHandleHelper::ClearItem_BC( mpAttrSet, *this, 0, &aOld, &aNew );

    if( bRet )
    {
        SwAttrSetChg aChgOld( *GetpSwAttrSet(), aOld );
        SwAttrSetChg aChgNew( *GetpSwAttrSet(), aNew );
        ModifyNotification( &aChgOld, &aChgNew ); // All changed ones are sent

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
    if ( 0 == rWhichIds.size() )
        return nRet;

    OSL_ENSURE( GetpSwAttrSet(), "no item set" );
    SwAttrSet aNewAttrSet( *GetpSwAttrSet() );
    for ( std::vector<sal_uInt16>::const_iterator aIter = rWhichIds.begin();
          aIter != rWhichIds.end();
          ++aIter )
    {
        nRet = nRet + aNewAttrSet.ClearItem( *aIter );
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

    if (0 == aIdx.GetIndex())
        return false;
    if (!lcl_CheckMaxLength(*pNd, *this))
    {
        return false;
    }
    if( pIdx )
        *pIdx = aIdx;
    return true;
}

//FEATURE::CONDCOLL
void SwContentNode::SetCondFormatColl( SwFormatColl* pColl )
{
    if( (!pColl && m_pCondColl) || ( pColl && !m_pCondColl ) ||
        ( pColl && pColl != m_pCondColl->GetRegisteredIn() ) )
    {
        SwFormatColl* pOldColl = GetCondFormatColl();
        m_pCondColl.reset();
        if( pColl )
            m_pCondColl.reset(new SwDepend( this, pColl ));

        if( GetpSwAttrSet() )
        {
            AttrSetHandleHelper::SetParent( mpAttrSet, *this, &GetAnyFormatColl(), GetFormatColl() );
        }

        if( !IsModifyLocked() )
        {
            SwFormatChg aTmp1( pOldColl ? pOldColl : GetFormatColl() );
            SwFormatChg aTmp2( pColl ? pColl : GetFormatColl() );
            NotifyClients( &aTmp1, &aTmp2 );
        }
        if( IsInCache() )
        {
            SwFrame::GetCache().Delete( this );
            SetInCache( false );
        }
    }
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
                            GetTableBox( pSttNd->GetIndex() ) ) && pBox &&
                            pBox->IsInHeadline( &pTableNd->GetTable() ) )
                            nCond = Master_CollCondition::PARA_IN_TABLEHEAD;
                    }
                    break;
                case SwFlyStartNode:        nCond = Master_CollCondition::PARA_IN_FRAME; break;
                case SwFootnoteStartNode:
                    {
                        nCond = Master_CollCondition::PARA_IN_FOOTNOTE;
                        const SwFootnoteIdxs& rFootnoteArr = rNds.GetDoc()->GetFootnoteIdxs();
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

void SwContentNode::ChkCondColl()
{
    // Check, just to be sure
    if( RES_CONDTXTFMTCOLL == GetFormatColl()->Which() )
    {
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

        if (!bDone)
        {
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
    }
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
    SwFrame* pFrame = getLayoutFrame( GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, &rPos, false );

    if ( pFrame )
    {
        if ( pFrame->IsVertical() )
        {
            if ( pFrame->IsRightToLeft() )
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

SwOLENodes* SwContentNode::CreateOLENodesArray( const SwFormatColl& rColl, bool bOnlyWithInvalidSize )
{
    SwOLENodes *pNodes = nullptr;
    SwIterator<SwContentNode,SwFormatColl> aIter( rColl );
    for( SwContentNode* pNd = aIter.First(); pNd; pNd = aIter.Next() )
    {
        SwOLENode *pONd = pNd->GetOLENode();
        if ( pONd && (!bOnlyWithInvalidSize || pONd->IsOLESizeInvalid()) )
        {
            if ( !pNodes  )
                pNodes = new SwOLENodes;
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
const IDocumentSettingAccess* SwNode::getIDocumentSettingAccess() const { return &GetDoc()->GetDocumentSettingManager(); }
const IDocumentDeviceAccess& SwNode::getIDocumentDeviceAccess() const { return GetDoc()->getIDocumentDeviceAccess(); }
const IDocumentRedlineAccess& SwNode::getIDocumentRedlineAccess() const { return GetDoc()->getIDocumentRedlineAccess(); }
const IDocumentStylePoolAccess& SwNode::getIDocumentStylePoolAccess() const { return GetDoc()->getIDocumentStylePoolAccess(); }
const IDocumentDrawModelAccess& SwNode::getIDocumentDrawModelAccess() const { return GetDoc()->getIDocumentDrawModelAccess(); }
const IDocumentLayoutAccess& SwNode::getIDocumentLayoutAccess() const { return GetDoc()->getIDocumentLayoutAccess(); }
IDocumentLayoutAccess& SwNode::getIDocumentLayoutAccess() { return GetDoc()->getIDocumentLayoutAccess(); }
const IDocumentLinksAdministration& SwNode::getIDocumentLinksAdministration() const { return GetDoc()->getIDocumentLinksAdministration(); }
IDocumentLinksAdministration& SwNode::getIDocumentLinksAdministration() { return GetDoc()->getIDocumentLinksAdministration(); }
const IDocumentFieldsAccess& SwNode::getIDocumentFieldsAccess() const { return GetDoc()->getIDocumentFieldsAccess(); }
IDocumentFieldsAccess& SwNode::getIDocumentFieldsAccess() { return GetDoc()->getIDocumentFieldsAccess(); }
IDocumentContentOperations& SwNode::getIDocumentContentOperations() { return GetDoc()->getIDocumentContentOperations(); }
IDocumentListItems& SwNode::getIDocumentListItems() { return GetDoc()->getIDocumentListItems(); } // #i83479#

const IDocumentMarkAccess* SwNode::getIDocumentMarkAccess() const { return GetDoc()->getIDocumentMarkAccess(); }
IStyleAccess& SwNode::getIDocumentStyleAccess() { return GetDoc()->GetIStyleAccess(); }

bool SwNode::IsInRedlines() const
{
    const SwDoc * pDoc = GetDoc();
    bool bResult = false;

    if (pDoc != nullptr)
        bResult = pDoc->getIDocumentRedlineAccess().IsInRedlines(*this);

    return bResult;
}

void SwNode::AddAnchoredFly(SwFrameFormat *const pFlyFormat)
{
    assert(pFlyFormat);
    assert(&pFlyFormat->GetAnchor(false).GetContentAnchor()->nNode.GetNode() == this);
    // check node type, cf. SwFormatAnchor::SetAnchor()
    assert(IsTextNode() || IsStartNode() || IsTableNode());
    if (!m_pAnchoredFlys)
    {
        m_pAnchoredFlys.reset(new std::vector<SwFrameFormat*>);
    }
    m_pAnchoredFlys->push_back(pFlyFormat);
}

void SwNode::RemoveAnchoredFly(SwFrameFormat *const pFlyFormat)
{
    assert(pFlyFormat);
    // cannot assert this in Remove because it is called when new anchor is already set
//    assert(&pFlyFormat->GetAnchor(false).GetContentAnchor()->nNode.GetNode() == this);
    assert(IsTextNode() || IsStartNode() || IsTableNode());
    assert(m_pAnchoredFlys);
    auto it(std::find(m_pAnchoredFlys->begin(), m_pAnchoredFlys->end(), pFlyFormat));
    assert(it != m_pAnchoredFlys->end());
    m_pAnchoredFlys->erase(it);
    if (m_pAnchoredFlys->empty())
    {
        m_pAnchoredFlys.reset();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
