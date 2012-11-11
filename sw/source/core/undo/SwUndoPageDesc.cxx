/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <tools/resid.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <swundo.hxx>
#include <pagedesc.hxx>
#include <SwUndoPageDesc.hxx>
#include <SwRewriter.hxx>
#include <undobj.hxx>
#include <comcore.hrc>
#include <fmtcntnt.hxx>
#include <fmthdft.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <ndindex.hxx>
#endif


#if OSL_DEBUG_LEVEL > 1
// Pure debug help function to have a quick look at the header/footer attributes.
void DebugHeaderFooterContent( const SwPageDesc& rPageDesc )
{
    sal_uLong nHeaderMaster = ULONG_MAX;
    sal_uLong nHeaderLeft = ULONG_MAX;
    sal_uLong nFooterMaster = ULONG_MAX;
    sal_uLong nFooterLeft = ULONG_MAX;

    SwFmtHeader& rHead = (SwFmtHeader&)rPageDesc.GetMaster().GetHeader();
    SwFmtFooter& rFoot = (SwFmtFooter&)rPageDesc.GetMaster().GetFooter();
    SwFmtHeader& rLeftHead = (SwFmtHeader&)rPageDesc.GetLeft().GetHeader();
    SwFmtFooter& rLeftFoot = (SwFmtFooter&)rPageDesc.GetLeft().GetFooter();
    if( rHead.IsActive() )
    {
        SwFrmFmt* pHeaderFmt = rHead.GetHeaderFmt();
        if( pHeaderFmt )
        {
            const SwFmtCntnt* pCntnt = &pHeaderFmt->GetCntnt();
            if( pCntnt->GetCntntIdx() )
                nHeaderMaster = pCntnt->GetCntntIdx()->GetIndex();
            else
                nHeaderMaster = 0;
        }
        SwFrmFmt* pLeftHeaderFmt = rLeftHead.GetHeaderFmt();
        if( pLeftHeaderFmt )
        {
            const SwFmtCntnt* pLeftCntnt = &pLeftHeaderFmt->GetCntnt();
            if( pLeftCntnt->GetCntntIdx() )
                nHeaderLeft = pLeftCntnt->GetCntntIdx()->GetIndex();
            else
                nHeaderLeft = 0;
        }
    }
    if( rFoot.IsActive() )
    {
        SwFrmFmt* pFooterFmt = rFoot.GetFooterFmt();
        if( pFooterFmt )
        {
            const SwFmtCntnt* pCntnt = &pFooterFmt->GetCntnt();
            if( pCntnt->GetCntntIdx() )
                nFooterMaster = pCntnt->GetCntntIdx()->GetIndex();
            else
                nFooterMaster = 0;
        }
        SwFrmFmt* pLeftFooterFmt = rLeftFoot.GetFooterFmt();
        if( pLeftFooterFmt )
        {
            const SwFmtCntnt* pLeftCntnt = &pLeftFooterFmt->GetCntnt();
            if( pLeftCntnt->GetCntntIdx() )
                nFooterLeft = pLeftCntnt->GetCntntIdx()->GetIndex();
            else
                nFooterLeft = 0;
        }
    }

    (void)nHeaderMaster;
    (void)nHeaderLeft;
    (void)nFooterMaster;
    (void)nFooterLeft;
}
#endif

SwUndoPageDesc::SwUndoPageDesc(const SwPageDesc & _aOld,
                               const SwPageDesc & _aNew,
                               SwDoc * _pDoc)
    : SwUndo( _aOld.GetName() != _aNew.GetName() ?
              UNDO_RENAME_PAGEDESC :
              UNDO_CHANGE_PAGEDESC ),
      aOld(_aOld, _pDoc), aNew(_aNew, _pDoc), pDoc(_pDoc), bExchange( false )
{
    OSL_ENSURE(0 != pDoc, "no document?");

#if OSL_DEBUG_LEVEL > 1
    DebugHeaderFooterContent( (SwPageDesc&)aOld );
    DebugHeaderFooterContent( (SwPageDesc&)aNew );
#endif

    /*
    The page description changes.
    If there are no header/footer content changes like header on/off or change from shared content
    to unshared etc., there is no reason to duplicate the content nodes (Crash i55547)
    But this happens, this Undo Ctor will destroy the unnecessary duplicate and manipulate the
    content pointer of the both page descriptions.
    */
    SwPageDesc &rOldDesc = (SwPageDesc&)aOld;
    SwPageDesc &rNewDesc = (SwPageDesc&)aNew;
    const SwFmtHeader& rOldHead = rOldDesc.GetMaster().GetHeader();
    const SwFmtHeader& rNewHead = rNewDesc.GetMaster().GetHeader();
    const SwFmtFooter& rOldFoot = rOldDesc.GetMaster().GetFooter();
    const SwFmtFooter& rNewFoot = rNewDesc.GetMaster().GetFooter();
    /* bExchange must not be set, if the old page descriptor will stay active.
    Two known situations:
    #i67735#: renaming a page descriptor
    #i67334#: changing the follow style
    If header/footer will be activated or deactivated, this undo will not work.
    */
    bExchange = ( aOld.GetName() == aNew.GetName() ) &&
        ( _aOld.GetFollow() == _aNew.GetFollow() ) &&
        ( rOldHead.IsActive() == rNewHead.IsActive() ) &&
        ( rOldFoot.IsActive() == rNewFoot.IsActive() );
    if( rOldHead.IsActive() && ( rOldDesc.IsHeaderShared() != rNewDesc.IsHeaderShared() ) )
        bExchange = false;
    if( rOldFoot.IsActive() && ( rOldDesc.IsFooterShared() != rNewDesc.IsFooterShared() ) )
        bExchange = false;
    if( ( rOldHead.IsActive() || rOldFoot.IsActive() ) && ( rOldDesc.IsFirstShared() != rNewDesc.IsFirstShared() ) )
        bExchange = false;
    if( bExchange )
    {
        if( rNewHead.IsActive() )
        {
            SwFrmFmt* pFormat = new SwFrmFmt( *rNewHead.GetHeaderFmt() );
            // The Ctor of this object will remove the duplicate!
            SwFmtHeader aFmtHeader( pFormat );
            if( !rNewDesc.IsHeaderShared() )
            {
                pFormat = new SwFrmFmt( *rNewDesc.GetLeft().GetHeader().GetHeaderFmt() );
                // The Ctor of this object will remove the duplicate!
                SwFmtHeader aFormatHeader( pFormat );
            }
            if( !rNewDesc.IsFirstShared() )
            {
                pFormat = new SwFrmFmt( *rNewDesc.GetFirst().GetHeader().GetHeaderFmt() );
                // The Ctor of this object will remove the duplicate!
                SwFmtHeader aFormatHeader( pFormat );
            }
        }
        // Same procedure for footers...
        if( rNewFoot.IsActive() )
        {
            SwFrmFmt* pFormat = new SwFrmFmt( *rNewFoot.GetFooterFmt() );
            // The Ctor of this object will remove the duplicate!
            SwFmtFooter aFmtFooter( pFormat );
            if( !rNewDesc.IsFooterShared() )
            {
                pFormat = new SwFrmFmt( *rNewDesc.GetLeft().GetFooter().GetFooterFmt() );
                // The Ctor of this object will remove the duplicate!
                SwFmtFooter aFormatFooter( pFormat );
            }
            if( !rNewDesc.IsFirstShared() )
            {
                pFormat = new SwFrmFmt( *rNewDesc.GetFirst().GetFooter().GetFooterFmt() );
                // The Ctor of this object will remove the duplicate!
                SwFmtFooter aFormatFooter( pFormat );
            }
        }

        // After this exchange method the old page description will point to zero,
        // the new one will point to the node position of the original content nodes.
        ExchangeContentNodes( (SwPageDesc&)aOld, (SwPageDesc&)aNew );
#if OSL_DEBUG_LEVEL > 1
        DebugHeaderFooterContent( (SwPageDesc&)aOld );
        DebugHeaderFooterContent( (SwPageDesc&)aNew );
#endif
    }
}

SwUndoPageDesc::~SwUndoPageDesc()
{
}


void SwUndoPageDesc::ExchangeContentNodes( SwPageDesc& rSource, SwPageDesc &rDest )
{
    OSL_ENSURE( bExchange, "You shouldn't do that." );
    const SwFmtHeader& rDestHead = rDest.GetMaster().GetHeader();
    const SwFmtHeader& rSourceHead = rSource.GetMaster().GetHeader();
    if( rDestHead.IsActive() )
    {
        // Let the destination page descrition point to the source node position,
        // from now on this descriptor is responsible for the content nodes!
        const SfxPoolItem* pItem;
        rDest.GetMaster().GetAttrSet().GetItemState( RES_HEADER, sal_False, &pItem );
        SfxPoolItem *pNewItem = pItem->Clone();
        SwFrmFmt* pNewFmt = ((SwFmtHeader*)pNewItem)->GetHeaderFmt();
#if OSL_DEBUG_LEVEL > 1
        const SwFmtCntnt& rSourceCntnt = rSourceHead.GetHeaderFmt()->GetCntnt();
        (void)rSourceCntnt;
        const SwFmtCntnt& rDestCntnt = rDestHead.GetHeaderFmt()->GetCntnt();
        (void)rDestCntnt;
#endif
        pNewFmt->SetFmtAttr( rSourceHead.GetHeaderFmt()->GetCntnt() );
        delete pNewItem;

        // Let the source page description point to zero node position,
        // it loses the responsible and can be destroyed without removing the content nodes.
        rSource.GetMaster().GetAttrSet().GetItemState( RES_HEADER, sal_False, &pItem );
        pNewItem = pItem->Clone();
        pNewFmt = ((SwFmtHeader*)pNewItem)->GetHeaderFmt();
        pNewFmt->SetFmtAttr( SwFmtCntnt() );
        delete pNewItem;

        if( !rDest.IsHeaderShared() )
        {
            // Same procedure for unshared header..
            const SwFmtHeader& rSourceLeftHead = rSource.GetLeft().GetHeader();
            rDest.GetLeft().GetAttrSet().GetItemState( RES_HEADER, sal_False, &pItem );
            pNewItem = pItem->Clone();
            pNewFmt = ((SwFmtHeader*)pNewItem)->GetHeaderFmt();
#if OSL_DEBUG_LEVEL > 1
            const SwFmtCntnt& rSourceCntnt1 = rSourceLeftHead.GetHeaderFmt()->GetCntnt();
            (void)rSourceCntnt1;
            const SwFmtCntnt& rDestCntnt1 = rDest.GetLeft().GetHeader().GetHeaderFmt()->GetCntnt();
            (void)rDestCntnt1;
#endif
            pNewFmt->SetFmtAttr( rSourceLeftHead.GetHeaderFmt()->GetCntnt() );
            delete pNewItem;
            rSource.GetLeft().GetAttrSet().GetItemState( RES_HEADER, sal_False, &pItem );
            pNewItem = pItem->Clone();
            pNewFmt = ((SwFmtHeader*)pNewItem)->GetHeaderFmt();
            pNewFmt->SetFmtAttr( SwFmtCntnt() );
            delete pNewItem;
        }
    }
    // Same procedure for footers...
    const SwFmtFooter& rDestFoot = rDest.GetMaster().GetFooter();
    const SwFmtFooter& rSourceFoot = rSource.GetMaster().GetFooter();
    if( rDestFoot.IsActive() )
    {
        const SfxPoolItem* pItem;
        rDest.GetMaster().GetAttrSet().GetItemState( RES_FOOTER, sal_False, &pItem );
        SfxPoolItem *pNewItem = pItem->Clone();
        SwFrmFmt *pNewFmt = ((SwFmtFooter*)pNewItem)->GetFooterFmt();
        pNewFmt->SetFmtAttr( rSourceFoot.GetFooterFmt()->GetCntnt() );
        delete pNewItem;

#if OSL_DEBUG_LEVEL > 1
        const SwFmtCntnt& rFooterSourceCntnt = rSourceFoot.GetFooterFmt()->GetCntnt();
        (void)rFooterSourceCntnt;
        const SwFmtCntnt& rFooterDestCntnt = rDestFoot.GetFooterFmt()->GetCntnt();
        (void)rFooterDestCntnt;
#endif
        rSource.GetMaster().GetAttrSet().GetItemState( RES_FOOTER, sal_False, &pItem );
        pNewItem = pItem->Clone();
        pNewFmt = ((SwFmtFooter*)pNewItem)->GetFooterFmt();
        pNewFmt->SetFmtAttr( SwFmtCntnt() );
        delete pNewItem;

        if( !rDest.IsFooterShared() )
        {
            const SwFmtFooter& rSourceLeftFoot = rSource.GetLeft().GetFooter();
#if OSL_DEBUG_LEVEL > 1
            const SwFmtCntnt& rFooterSourceCntnt2 = rSourceLeftFoot.GetFooterFmt()->GetCntnt();
            const SwFmtCntnt& rFooterDestCntnt2 =
                rDest.GetLeft().GetFooter().GetFooterFmt()->GetCntnt();
            (void)rFooterSourceCntnt2;
            (void)rFooterDestCntnt2;
#endif
            rDest.GetLeft().GetAttrSet().GetItemState( RES_FOOTER, sal_False, &pItem );
            pNewItem = pItem->Clone();
            pNewFmt = ((SwFmtFooter*)pNewItem)->GetFooterFmt();
            pNewFmt->SetFmtAttr( rSourceLeftFoot.GetFooterFmt()->GetCntnt() );
            delete pNewItem;
            rSource.GetLeft().GetAttrSet().GetItemState( RES_FOOTER, sal_False, &pItem );
            pNewItem = pItem->Clone();
            pNewFmt = ((SwFmtFooter*)pNewItem)->GetFooterFmt();
            pNewFmt->SetFmtAttr( SwFmtCntnt() );
            delete pNewItem;
        }
    }
}

void SwUndoPageDesc::UndoImpl(::sw::UndoRedoContext &)
{
    // Move (header/footer)content node responsibility from new page descriptor to old one again.
    if( bExchange )
        ExchangeContentNodes( (SwPageDesc&)aNew, (SwPageDesc&)aOld );
    pDoc->ChgPageDesc(aOld.GetName(), aOld);
}

void SwUndoPageDesc::RedoImpl(::sw::UndoRedoContext &)
{
    // Move (header/footer)content node responsibility from old page descriptor to new one again.
    if( bExchange )
        ExchangeContentNodes( (SwPageDesc&)aOld, (SwPageDesc&)aNew );
    pDoc->ChgPageDesc(aNew.GetName(), aNew);
}

SwRewriter SwUndoPageDesc::GetRewriter() const
{
    SwRewriter aResult;

    aResult.AddRule(UndoArg1, aOld.GetName());
    aResult.AddRule(UndoArg2, SW_RESSTR(STR_YIELDS));
    aResult.AddRule(UndoArg3, aNew.GetName());

    return aResult;
}

// #116530#
SwUndoPageDescCreate::SwUndoPageDescCreate(const SwPageDesc * pNew,
                                           SwDoc * _pDoc)
    : SwUndo(UNDO_CREATE_PAGEDESC), pDesc(pNew), aNew(*pNew, _pDoc),
      pDoc(_pDoc)
{
    OSL_ENSURE(0 != pDoc, "no document?");
}

SwUndoPageDescCreate::~SwUndoPageDescCreate()
{
}

void SwUndoPageDescCreate::UndoImpl(::sw::UndoRedoContext &)
{
    // -> #116530#
    if (pDesc)
    {
        aNew = *pDesc;
        pDesc = NULL;
    }
    // <- #116530#

    pDoc->DelPageDesc(aNew.GetName(), true);
}

void SwUndoPageDescCreate::DoImpl()
{
    SwPageDesc aPageDesc = aNew;
    pDoc->MakePageDesc(aNew.GetName(), &aPageDesc, false, true); // #116530#
}

void SwUndoPageDescCreate::RedoImpl(::sw::UndoRedoContext &)
{
    DoImpl();
}

void SwUndoPageDescCreate::RepeatImpl(::sw::RepeatContext &)
{
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());
    DoImpl();
}

SwRewriter SwUndoPageDescCreate::GetRewriter() const
{
    SwRewriter aResult;

    if (pDesc)
        aResult.AddRule(UndoArg1, pDesc->GetName());
    else
        aResult.AddRule(UndoArg1, aNew.GetName());


    return aResult;
}

SwUndoPageDescDelete::SwUndoPageDescDelete(const SwPageDesc & _aOld,
                                           SwDoc * _pDoc)
    : SwUndo(UNDO_DELETE_PAGEDESC), aOld(_aOld, _pDoc), pDoc(_pDoc)
{
    OSL_ENSURE(0 != pDoc, "no document?");
}

SwUndoPageDescDelete::~SwUndoPageDescDelete()
{
}

void SwUndoPageDescDelete::UndoImpl(::sw::UndoRedoContext &)
{
    SwPageDesc aPageDesc = aOld;
    pDoc->MakePageDesc(aOld.GetName(), &aPageDesc, false, true); // #116530#
}

void SwUndoPageDescDelete::DoImpl()
{
    pDoc->DelPageDesc(aOld.GetName(), true); // #116530#
}

void SwUndoPageDescDelete::RedoImpl(::sw::UndoRedoContext &)
{
    DoImpl();
}

void SwUndoPageDescDelete::RepeatImpl(::sw::RepeatContext &)
{
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());
    DoImpl();
}

SwRewriter SwUndoPageDescDelete::GetRewriter() const
{
    SwRewriter aResult;

    aResult.AddRule(UndoArg1, aOld.GetName());

    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
