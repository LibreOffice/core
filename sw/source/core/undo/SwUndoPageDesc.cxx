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

#include <doc.hxx>
#include <editsh.hxx>
#include <IDocumentUndoRedo.hxx>
#include <swundo.hxx>
#include <pagedesc.hxx>
#include <SwUndoPageDesc.hxx>
#include <SwRewriter.hxx>
#include <UndoManager.hxx>
#include <undobj.hxx>
#include <strings.hrc>
#include <fmtcntnt.hxx>
#include <fmthdft.hxx>
#include <osl/diagnose.h>
#include <cntfrm.hxx>
#include <fmtpdsc.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>

SwUndoPageDesc::SwUndoPageDesc(const SwPageDesc & _aOld,
                               const SwPageDesc & _aNew,
                               SwDoc& rDoc)
    : SwUndo( _aOld.GetName() != _aNew.GetName() ?
              SwUndoId::RENAME_PAGEDESC :
              SwUndoId::CHANGE_PAGEDESC,
              rDoc ),
      m_aOld(_aOld, rDoc), m_aNew(_aNew, rDoc), m_rDoc(rDoc), m_bExchange( false )
{
    /*
    The page description changes.
    If there are no header/footer content changes like header on/off or change from shared content
    to unshared etc., there is no reason to duplicate the content nodes (Crash i55547)
    But this happens, this Undo Ctor will destroy the unnecessary duplicate and manipulate the
    content pointer of the both page descriptions.
    */
    SwPageDesc &rOldDesc = m_aOld.m_PageDesc;
    SwPageDesc &rNewDesc = m_aNew.m_PageDesc;
    const SwFormatHeader& rOldHead = rOldDesc.GetMaster().GetHeader();
    const SwFormatHeader& rNewHead = rNewDesc.GetMaster().GetHeader();
    const SwFormatFooter& rOldFoot = rOldDesc.GetMaster().GetFooter();
    const SwFormatFooter& rNewFoot = rNewDesc.GetMaster().GetFooter();
    /* bExchange must not be set, if the old page descriptor will stay active.
    Two known situations:
    #i67735#: renaming a page descriptor
    #i67334#: changing the follow style
    If header/footer will be activated or deactivated, this undo will not work.
    */
    m_bExchange = ( m_aOld.GetName() == m_aNew.GetName() ) &&
        ( _aOld.GetFollow() == _aNew.GetFollow() ) &&
        ( rOldHead.IsActive() == rNewHead.IsActive() ) &&
        ( rOldFoot.IsActive() == rNewFoot.IsActive() );
    if( rOldHead.IsActive() && ( rOldDesc.IsHeaderShared() != rNewDesc.IsHeaderShared() ) )
        m_bExchange = false;
    if( rOldFoot.IsActive() && ( rOldDesc.IsFooterShared() != rNewDesc.IsFooterShared() ) )
        m_bExchange = false;
    if( ( rOldHead.IsActive() || rOldFoot.IsActive() ) && ( rOldDesc.IsFirstShared() != rNewDesc.IsFirstShared() ) )
        m_bExchange = false;
    if( !m_bExchange )
        return;

    if( rNewHead.IsActive() )
    {
        SwFrameFormat* pFormat = new SwFrameFormat( *rNewHead.GetHeaderFormat() );
        // The Ctor of this object will remove the duplicate!
        SwFormatHeader aFormatHeader(pFormat);
        (void)aFormatHeader;
        if (!rNewDesc.IsHeaderShared())
        {
            pFormat = new SwFrameFormat( *rNewDesc.GetLeft().GetHeader().GetHeaderFormat() );
            // The Ctor of this object will remove the duplicate!
            SwFormatHeader aLeftHeader(pFormat);
            (void)aLeftHeader;
        }
        if (!rNewDesc.IsFirstShared())
        {
            pFormat = new SwFrameFormat( *rNewDesc.GetFirstMaster().GetHeader().GetHeaderFormat() );
            // The Ctor of this object will remove the duplicate!
            SwFormatHeader aFirstHeader(pFormat);
            (void)aFirstHeader;
        }
    }
    // Same procedure for footers...
    if( rNewFoot.IsActive() )
    {
        SwFrameFormat* pFormat = new SwFrameFormat( *rNewFoot.GetFooterFormat() );
        // The Ctor of this object will remove the duplicate!
        SwFormatFooter aFormatFooter(pFormat);
        (void)aFormatFooter;
        if (!rNewDesc.IsFooterShared())
        {
            pFormat = new SwFrameFormat( *rNewDesc.GetLeft().GetFooter().GetFooterFormat() );
            // The Ctor of this object will remove the duplicate!
            SwFormatFooter aLeftFooter(pFormat);
            (void)aLeftFooter;
        }
        if (!rNewDesc.IsFirstShared())
        {
            pFormat = new SwFrameFormat( *rNewDesc.GetFirstMaster().GetFooter().GetFooterFormat() );
            // The Ctor of this object will remove the duplicate!
            SwFormatFooter aFirstFooter(pFormat);
            (void)aFirstFooter;
        }
    }

    // After this exchange method the old page description will point to zero,
    // the new one will point to the node position of the original content nodes.
    ExchangeContentNodes( m_aOld.m_PageDesc, m_aNew.m_PageDesc );
}

SwUndoPageDesc::~SwUndoPageDesc()
{
}

void SwUndoPageDesc::ExchangeContentNodes( SwPageDesc& rSource, SwPageDesc &rDest )
{
    OSL_ENSURE( m_bExchange, "You shouldn't do that." );
    const SwFormatHeader& rDestHead = rDest.GetMaster().GetHeader();
    const SwFormatHeader& rSourceHead = rSource.GetMaster().GetHeader();
    if( rDestHead.IsActive() )
    {
        // Let the destination page description point to the source node position,
        // from now on this descriptor is responsible for the content nodes!
        const SwFormatHeader* pItem = rDest.GetMaster().GetAttrSet().GetItemIfSet( RES_HEADER, false );
        std::unique_ptr<SwFormatHeader> pNewItem(pItem->Clone());
        SwFrameFormat* pNewFormat = pNewItem->GetHeaderFormat();
        pNewFormat->SetFormatAttr( rSourceHead.GetHeaderFormat()->GetContent() );

        // Let the source page description point to zero node position,
        // it loses the responsible and can be destroyed without removing the content nodes.
        pItem = rSource.GetMaster().GetAttrSet().GetItemIfSet( RES_HEADER, false );
        pNewItem.reset(pItem->Clone());
        pNewFormat = pNewItem->GetHeaderFormat();
        pNewFormat->SetFormatAttr( SwFormatContent() );

        if( !rDest.IsHeaderShared() )
        {
            // Same procedure for unshared header...
            const SwFormatHeader& rSourceLeftHead = rSource.GetLeft().GetHeader();
            pItem = rDest.GetLeft().GetAttrSet().GetItemIfSet( RES_HEADER, false );
            pNewItem.reset(pItem->Clone());
            pNewFormat = pNewItem->GetHeaderFormat();
            pNewFormat->SetFormatAttr( rSourceLeftHead.GetHeaderFormat()->GetContent() );
            pItem = rSource.GetLeft().GetAttrSet().GetItemIfSet( RES_HEADER, false );
            pNewItem.reset(pItem->Clone());
            pNewFormat = pNewItem->GetHeaderFormat();
            pNewFormat->SetFormatAttr( SwFormatContent() );
        }
        if (!rDest.IsFirstShared())
        {
            // Same procedure for unshared header...
            const SwFormatHeader& rSourceFirstMasterHead = rSource.GetFirstMaster().GetHeader();
            pItem = rDest.GetFirstMaster().GetAttrSet().GetItemIfSet( RES_HEADER, false );
            pNewItem.reset(pItem->Clone());
            pNewFormat = pNewItem->GetHeaderFormat();
            pNewFormat->SetFormatAttr( rSourceFirstMasterHead.GetHeaderFormat()->GetContent() );
            pItem = rSource.GetFirstMaster().GetAttrSet().GetItemIfSet( RES_HEADER, false );
            pNewItem.reset(pItem->Clone());
            pNewFormat = pNewItem->GetHeaderFormat();
            pNewFormat->SetFormatAttr( SwFormatContent() );
        }
    }
    // Same procedure for footers...
    const SwFormatFooter& rDestFoot = rDest.GetMaster().GetFooter();
    const SwFormatFooter& rSourceFoot = rSource.GetMaster().GetFooter();
    if( !rDestFoot.IsActive() )
        return;

    const SwFormatFooter* pItem;
    pItem = rDest.GetMaster().GetAttrSet().GetItemIfSet( RES_FOOTER, false );
    std::unique_ptr<SwFormatFooter> pNewItem(pItem->Clone());
    SwFrameFormat *pNewFormat = pNewItem->GetFooterFormat();
    pNewFormat->SetFormatAttr( rSourceFoot.GetFooterFormat()->GetContent() );

    pItem = rSource.GetMaster().GetAttrSet().GetItemIfSet( RES_FOOTER, false );
    pNewItem.reset(pItem->Clone());
    pNewFormat = pNewItem->GetFooterFormat();
    pNewFormat->SetFormatAttr( SwFormatContent() );

    if( !rDest.IsFooterShared() )
    {
        const SwFormatFooter& rSourceLeftFoot = rSource.GetLeft().GetFooter();
        pItem = rDest.GetLeft().GetAttrSet().GetItemIfSet( RES_FOOTER, false );
        pNewItem.reset(pItem->Clone());
        pNewFormat = pNewItem->GetFooterFormat();
        pNewFormat->SetFormatAttr( rSourceLeftFoot.GetFooterFormat()->GetContent() );
        pItem = rSource.GetLeft().GetAttrSet().GetItemIfSet( RES_FOOTER, false );
        pNewItem.reset(pItem->Clone());
        pNewFormat = pNewItem->GetFooterFormat();
        pNewFormat->SetFormatAttr( SwFormatContent() );
    }
    if (rDest.IsFirstShared())
        return;

    const SwFormatFooter& rSourceFirstMasterFoot = rSource.GetFirstMaster().GetFooter();
    pItem = rDest.GetFirstMaster().GetAttrSet().GetItemIfSet( RES_FOOTER, false );
    pNewItem.reset(pItem->Clone());
    pNewFormat = pNewItem->GetFooterFormat();
    pNewFormat->SetFormatAttr( rSourceFirstMasterFoot.GetFooterFormat()->GetContent() );
    pItem = rSource.GetFirstMaster().GetAttrSet().GetItemIfSet( RES_FOOTER, false );
    pNewItem.reset(pItem->Clone());
    pNewFormat = pNewItem->GetFooterFormat();
    pNewFormat->SetFormatAttr( SwFormatContent() );
}

void SwUndoPageDesc::ExitHeaderFooterEdit()
{
    SwEditShell* pESh = m_rDoc.GetEditShell();
    if (!pESh)
        return;
    if (pESh->IsHeaderFooterEdit())
        pESh->ToggleHeaderFooterEdit();
}

void SwUndoPageDesc::UndoImpl(::sw::UndoRedoContext &)
{
    // Move (header/footer)content node responsibility from new page descriptor to old one again.
    if( m_bExchange )
        ExchangeContentNodes( m_aNew.m_PageDesc, m_aOld.m_PageDesc );
    m_rDoc.ChgPageDesc(m_aOld.GetName(), SwPageDesc(m_aOld));
    ExitHeaderFooterEdit();
}

void SwUndoPageDesc::RedoImpl(::sw::UndoRedoContext &)
{
    // Move (header/footer)content node responsibility from old page descriptor to new one again.
    if( m_bExchange )
        ExchangeContentNodes( m_aOld.m_PageDesc, m_aNew.m_PageDesc );
    m_rDoc.ChgPageDesc(m_aNew.GetName(), SwPageDesc(m_aNew));
    ExitHeaderFooterEdit();
}

SwRewriter SwUndoPageDesc::GetRewriter() const
{
    SwRewriter aResult;

    aResult.AddRule(UndoArg1, m_aOld.GetName());
    aResult.AddRule(UndoArg2, SwResId(STR_YIELDS));
    aResult.AddRule(UndoArg3, m_aNew.GetName());

    return aResult;
}

SwUndoPageDescCreate::SwUndoPageDescCreate(const SwPageDesc * pNew,
                                           SwDoc& rDoc)
    : SwUndo(SwUndoId::CREATE_PAGEDESC, rDoc), m_pDesc(pNew), m_aNew(*pNew, rDoc),
      m_rDoc(rDoc)
{
}

SwUndoPageDescCreate::~SwUndoPageDescCreate()
{
}

void SwUndoPageDescCreate::UndoImpl(::sw::UndoRedoContext &)
{
    if (m_pDesc)
    {
        SwNodes& rUndoNds = m_rDoc.GetUndoManager().GetUndoNodes();

        // Record which nodes in the undo nodes used this page description.
        rUndoNds.ForEach([](SwNode* pNode, void* pArgs) -> bool {
            SwUndoPageDescCreate* pThis = static_cast<SwUndoPageDescCreate*>(pArgs);

            const SwPageDesc *pPgDesc = nullptr;

            if (pNode->IsContentNode())
                pPgDesc = pNode->GetContentNode()->GetAttr(RES_PAGEDESC).GetPageDesc();
            else if (pNode->IsTableNode())
                pPgDesc = pNode->GetTableNode()->GetTable().
                        GetFrameFormat()->GetPageDesc().GetPageDesc();
            else if (pNode->IsSectionNode())
                pPgDesc = pNode->GetSectionNode()->GetSection().
                        GetFormat()->GetPageDesc().GetPageDesc();

            if (pPgDesc == pThis->m_pDesc)
                pThis->m_aAppliedNodes.push_back(pNode->GetIndex());

            return true;
        }, this);

        m_aNew = *m_pDesc;
        m_pDesc = nullptr;
    }

    m_rDoc.DelPageDesc(m_aNew.GetName(), true);
}

void SwUndoPageDescCreate::DoImpl()
{
    SwPageDesc aPageDesc(m_aNew);
    SwPageDesc* pNewPageDesc = m_rDoc.MakePageDesc(m_aNew.GetName(), &aPageDesc, false);
    if (m_aNew.GetName() == m_aNew.GetFollowName())
        pNewPageDesc->SetFollow(pNewPageDesc);

    SwNodes& rUndoNds = m_rDoc.GetUndoManager().GetUndoNodes();

    for (const SwNodeOffset& aOffset : m_aAppliedNodes)
    {
        SwNode* pNd = rUndoNds[aOffset];

        SwFormatPageDesc aNew(pNewPageDesc);
        if (pNd->IsContentNode())
            pNd->GetContentNode()->SetAttr(aNew);
        else if (pNd->IsTableNode())
            pNd->GetTableNode()->GetTable().GetFrameFormat()->SetFormatAttr(aNew);
        else if (pNd->IsSectionNode())
            pNd->GetSectionNode()->GetSection().GetFormat()->SetFormatAttr(aNew);
    }
}

void SwUndoPageDescCreate::RedoImpl(::sw::UndoRedoContext &)
{
    DoImpl();
}

void SwUndoPageDescCreate::RepeatImpl(::sw::RepeatContext &)
{
    ::sw::UndoGuard const undoGuard(m_rDoc.GetIDocumentUndoRedo());
    DoImpl();
}

SwRewriter SwUndoPageDescCreate::GetRewriter() const
{
    SwRewriter aResult;

    if (m_pDesc)
        aResult.AddRule(UndoArg1, m_pDesc->GetName());
    else
        aResult.AddRule(UndoArg1, m_aNew.GetName());

    return aResult;
}

SwUndoPageDescDelete::SwUndoPageDescDelete(const SwPageDesc & _aOld,
                                           SwDoc& rDoc)
    : SwUndo(SwUndoId::DELETE_PAGEDESC, rDoc), m_aOld(_aOld, rDoc), m_rDoc(rDoc)
{
    // Record which page descriptions in this document used this page desc as their follow
    for (size_t i = 0; i < m_rDoc.GetPageDescCnt(); ++i)
    {
        if (const auto rPageDesc = m_rDoc.GetPageDesc(i); rPageDesc.GetFollow() == &_aOld)
        {
            m_aPageDescsThisFollowed.emplace_back(rPageDesc.GetName());
        }
    }

    // Record which nodes in the document used this page description.
    for (SwRootFrame* pRootFrame : rDoc.GetAllLayouts())
    {
        const SwPageFrame* pPageFrameIter = pRootFrame->GetLastPage();
        while (pPageFrameIter)
        {
            const SwContentFrame* pContentFrame = pPageFrameIter->FindFirstBodyContent();
            if (pContentFrame)
            {
                const SwFormatPageDesc& rFormatPageDesc = pContentFrame->GetPageDescItem();
                if (const sw::BroadcastingModify* pMod = rFormatPageDesc.GetDefinedIn())
                {
                    const SwPageDesc *pPgDesc = nullptr;
                    if (auto pNd = dynamic_cast<const SwNode*>( pMod); pNd != nullptr)
                    {
                        pPgDesc = pNd->GetContentNode()->GetAttr( RES_PAGEDESC ).GetPageDesc();

                        if (pNd->IsContentNode())
                            pPgDesc = pNd->GetContentNode()->GetAttr( RES_PAGEDESC ).GetPageDesc();
                        else if (pNd->IsTableNode())
                            pPgDesc = pNd->GetTableNode()->GetTable().
                                    GetFrameFormat()->GetPageDesc().GetPageDesc();
                        else if (pNd->IsSectionNode())
                            pPgDesc = pNd->GetSectionNode()->GetSection().
                                    GetFormat()->GetPageDesc().GetPageDesc();

                        if (pPgDesc == &_aOld)
                            m_aAppliedNodes.push_back(pNd->GetIndex());
                    }

                }
            }
            pPageFrameIter = static_cast<const SwPageFrame*>(pPageFrameIter->GetPrev());
        }
    }
}

SwUndoPageDescDelete::~SwUndoPageDescDelete()
{
}

void SwUndoPageDescDelete::UndoImpl(::sw::UndoRedoContext &)
{
    SwPageDesc aPageDesc(m_aOld);
    SwPageDesc* pNewPageDesc = m_rDoc.MakePageDesc(m_aOld.GetName(), &aPageDesc, false);
    if (m_aOld.GetName() == m_aOld.GetFollowName())
        pNewPageDesc->SetFollow(pNewPageDesc);

    for (const UIName& aName : m_aPageDescsThisFollowed)
    {
        if (SwPageDesc* pPageDesc = m_rDoc.FindPageDesc(aName); pPageDesc != nullptr)
        {
            pPageDesc->SetFollow(pNewPageDesc);
        }
    }

    for (const SwNodeOffset& aOffset : m_aAppliedNodes)
    {
        SwNode* pNd = m_rDoc.GetNodes()[aOffset];

        SwFormatPageDesc aNew(pNewPageDesc);
        if (pNd->IsContentNode())
            pNd->GetContentNode()->SetAttr(aNew);
        else if (pNd->IsTableNode())
            pNd->GetTableNode()->GetTable().GetFrameFormat()->SetFormatAttr(aNew);
        else if (pNd->IsSectionNode())
            pNd->GetSectionNode()->GetSection().GetFormat()->SetFormatAttr(aNew);
    }
}

void SwUndoPageDescDelete::DoImpl()
{
    m_rDoc.DelPageDesc(m_aOld.GetName(), true);
}

void SwUndoPageDescDelete::RedoImpl(::sw::UndoRedoContext &)
{
    DoImpl();
}

void SwUndoPageDescDelete::RepeatImpl(::sw::RepeatContext &)
{
    ::sw::UndoGuard const undoGuard(m_rDoc.GetIDocumentUndoRedo());
    DoImpl();
}

SwRewriter SwUndoPageDescDelete::GetRewriter() const
{
    SwRewriter aResult;

    aResult.AddRule(UndoArg1, m_aOld.GetName());

    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
