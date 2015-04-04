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

#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <drawdoc.hxx>
#include <fmtpdsc.hxx>
#include <swtable.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <dflyobj.hxx>
#include <frmtool.hxx>
#include <virtoutp.hxx>
#include <blink.hxx>
#include <sectfrm.hxx>
#include <notxtfrm.hxx>
#include <pagedesc.hxx>
#include "viewimp.hxx"
#include <hints.hxx>
#include <viewopt.hxx>
#include <set>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <DocumentLayoutManager.hxx>

SwLayVout     *SwRootFrm::mpVout = 0;
bool           SwRootFrm::mbInPaint = false;
bool           SwRootFrm::mbNoVirDev = false;

SwCache *SwFrm::mpCache = 0;

long FirstMinusSecond( long nFirst, long nSecond )
    { return nFirst - nSecond; }
long SecondMinusFirst( long nFirst, long nSecond )
    { return nSecond - nFirst; }
long SwIncrement( long nA, long nAdd )
    { return nA + nAdd; }
long SwDecrement( long nA, long nSub )
    { return nA - nSub; }

static SwRectFnCollection aHorizontal = {
    /* fnRectGet      */
    &SwRect::_Top,
    &SwRect::_Bottom,
    &SwRect::_Left,
    &SwRect::_Right,
    &SwRect::_Width,
    &SwRect::_Height,
    &SwRect::TopLeft,
    &SwRect::_Size,
    /* fnRectSet      */
    &SwRect::_Top,
    &SwRect::_Bottom,
    &SwRect::_Left,
    &SwRect::_Right,
    &SwRect::_Width,
    &SwRect::_Height,

    &SwRect::SubTop,
    &SwRect::AddBottom,
    &SwRect::SubLeft,
    &SwRect::AddRight,
    &SwRect::AddWidth,
    &SwRect::AddHeight,

    &SwRect::SetPosX,
    &SwRect::SetPosY,

    &SwFrm::GetTopMargin,
    &SwFrm::GetBottomMargin,
    &SwFrm::GetLeftMargin,
    &SwFrm::GetRightMargin,
    &SwFrm::SetLeftRightMargins,
    &SwFrm::SetTopBottomMargins,
    &SwFrm::GetPrtTop,
    &SwFrm::GetPrtBottom,
    &SwFrm::GetPrtLeft,
    &SwFrm::GetPrtRight,
    &SwRect::GetTopDistance,
    &SwRect::GetBottomDistance,
    &SwRect::GetLeftDistance,
    &SwRect::GetRightDistance,
    &SwFrm::SetMaxBottom,
    &SwRect::OverStepBottom,

    &SwRect::SetUpperLeftCorner,
    &SwFrm::MakeBelowPos,
    &FirstMinusSecond,
    &FirstMinusSecond,
    &SwIncrement,
    &SwIncrement,
    &SwRect::SetLeftAndWidth,
    &SwRect::SetTopAndHeight
};

static SwRectFnCollection aVertical = {
    /* fnRectGet      */
    &SwRect::_Right,
    &SwRect::_Left,
    &SwRect::_Top,
    &SwRect::_Bottom,
    &SwRect::_Height,
    &SwRect::_Width,
    &SwRect::TopRight,
    &SwRect::SwappedSize,
    /* fnRectSet      */
    &SwRect::_Right,
    &SwRect::_Left,
    &SwRect::_Top,
    &SwRect::_Bottom,
    &SwRect::_Height,
    &SwRect::_Width,

    &SwRect::AddRight,
    &SwRect::SubLeft,
    &SwRect::SubTop,
    &SwRect::AddBottom,
    &SwRect::AddHeight,
    &SwRect::AddWidth,

    &SwRect::SetPosY,
    &SwRect::SetPosX,

    &SwFrm::GetRightMargin,
    &SwFrm::GetLeftMargin,
    &SwFrm::GetTopMargin,
    &SwFrm::GetBottomMargin,
    &SwFrm::SetTopBottomMargins,
    &SwFrm::SetRightLeftMargins,
    &SwFrm::GetPrtRight,
    &SwFrm::GetPrtLeft,
    &SwFrm::GetPrtTop,
    &SwFrm::GetPrtBottom,
    &SwRect::GetRightDistance,
    &SwRect::GetLeftDistance,
    &SwRect::GetTopDistance,
    &SwRect::GetBottomDistance,
    &SwFrm::SetMinLeft,
    &SwRect::OverStepLeft,

    &SwRect::SetUpperRightCorner,
    &SwFrm::MakeLeftPos,
    &FirstMinusSecond,
    &SecondMinusFirst,
    &SwIncrement,
    &SwDecrement,
    &SwRect::SetTopAndHeight,
    &SwRect::SetRightAndWidth
};

static SwRectFnCollection aBottomToTop = {
    /* fnRectGet      */
    &SwRect::_Bottom,
    &SwRect::_Top,
    &SwRect::_Left,
    &SwRect::_Right,
    &SwRect::_Width,
    &SwRect::_Height,
    &SwRect::BottomLeft,
    &SwRect::_Size,
    /* fnRectSet      */
    &SwRect::_Bottom,
    &SwRect::_Top,
    &SwRect::_Left,
    &SwRect::_Right,
    &SwRect::_Width,
    &SwRect::_Height,

    &SwRect::AddBottom,
    &SwRect::SubTop,
    &SwRect::SubLeft,
    &SwRect::AddRight,
    &SwRect::AddWidth,
    &SwRect::AddHeight,

    &SwRect::SetPosX,
    &SwRect::SetPosY,

    &SwFrm::GetBottomMargin,
    &SwFrm::GetTopMargin,
    &SwFrm::GetLeftMargin,
    &SwFrm::GetRightMargin,
    &SwFrm::SetLeftRightMargins,
    &SwFrm::SetBottomTopMargins,
    &SwFrm::GetPrtBottom,
    &SwFrm::GetPrtTop,
    &SwFrm::GetPrtLeft,
    &SwFrm::GetPrtRight,
    &SwRect::GetBottomDistance,
    &SwRect::GetTopDistance,
    &SwRect::GetLeftDistance,
    &SwRect::GetRightDistance,
    &SwFrm::SetMinTop,
    &SwRect::OverStepTop,

    &SwRect::SetLowerLeftCorner,
    &SwFrm::MakeUpperPos,
    &FirstMinusSecond,
    &SecondMinusFirst,
    &SwIncrement,
    &SwDecrement,
    &SwRect::SetLeftAndWidth,
    &SwRect::SetBottomAndHeight
};

static SwRectFnCollection aVerticalRightToLeft = {
    /* fnRectGet      */
    &SwRect::_Left,
    &SwRect::_Right,
    &SwRect::_Top,
    &SwRect::_Bottom,
    &SwRect::_Height,
    &SwRect::_Width,
    &SwRect::BottomRight,
    &SwRect::SwappedSize,
    /* fnRectSet      */
    &SwRect::_Left,
    &SwRect::_Right,
    &SwRect::_Top,
    &SwRect::_Bottom,
    &SwRect::_Height,
    &SwRect::_Width,

    &SwRect::SubLeft,
    &SwRect::AddRight,
    &SwRect::SubTop,
    &SwRect::AddBottom,
    &SwRect::AddHeight,
    &SwRect::AddWidth,

    &SwRect::SetPosY,
    &SwRect::SetPosX,

    &SwFrm::GetLeftMargin,
    &SwFrm::GetRightMargin,
    &SwFrm::GetTopMargin,
    &SwFrm::GetBottomMargin,
    &SwFrm::SetTopBottomMargins,
    &SwFrm::SetLeftRightMargins,
    &SwFrm::GetPrtLeft,
    &SwFrm::GetPrtRight,
    &SwFrm::GetPrtBottom,
    &SwFrm::GetPrtTop,
    &SwRect::GetLeftDistance,
    &SwRect::GetRightDistance,
    &SwRect::GetBottomDistance,
    &SwRect::GetTopDistance,
    &SwFrm::SetMaxRight,
    &SwRect::OverStepRight,

    &SwRect::SetLowerLeftCorner,
    &SwFrm::MakeRightPos,
    &FirstMinusSecond,
    &FirstMinusSecond,
    &SwDecrement,
    &SwIncrement,
    &SwRect::SetBottomAndHeight,
    &SwRect::SetLeftAndWidth
};

static SwRectFnCollection aVerticalLeftToRight = {
    /* fnRectGet      */
    &SwRect::_Left,
    &SwRect::_Right,
    &SwRect::_Top,
    &SwRect::_Bottom,
    &SwRect::_Height,
    &SwRect::_Width,
    &SwRect::TopLeft,
    &SwRect::SwappedSize,
    /* fnRectSet      */
    &SwRect::_Left,
    &SwRect::_Right,
    &SwRect::_Top,
    &SwRect::_Bottom,
    &SwRect::_Height,
    &SwRect::_Width,

    &SwRect::SubLeft,
    &SwRect::AddRight,
    &SwRect::SubTop,
    &SwRect::AddBottom,
    &SwRect::AddHeight,
    &SwRect::AddWidth,

    &SwRect::SetPosY,
    &SwRect::SetPosX,

    &SwFrm::GetLeftMargin,
    &SwFrm::GetRightMargin,
    &SwFrm::GetTopMargin,
    &SwFrm::GetBottomMargin,
    &SwFrm::SetTopBottomMargins,
    &SwFrm::SetLeftRightMargins,
    &SwFrm::GetPrtLeft,
    &SwFrm::GetPrtRight,
    &SwFrm::GetPrtTop,
    &SwFrm::GetPrtBottom,
    &SwRect::GetLeftDistance,
    &SwRect::GetRightDistance,
    &SwRect::GetTopDistance,
    &SwRect::GetBottomDistance,
    &SwFrm::SetMaxRight,
    &SwRect::OverStepRight,

    &SwRect::SetUpperLeftCorner,
    &SwFrm::MakeRightPos,
    &FirstMinusSecond,
    &FirstMinusSecond,
    &SwIncrement,
    &SwIncrement,
    &SwRect::SetTopAndHeight,
    &SwRect::SetLeftAndWidth
};

SwRectFn fnRectHori = &aHorizontal;
SwRectFn fnRectVert = &aVertical;

SwRectFn fnRectVertL2R = &aVerticalLeftToRight;

SwRectFn fnRectB2T = &aBottomToTop;
SwRectFn fnRectVL2R = &aVerticalRightToLeft;

// #i65250#
sal_uInt32 SwFrm::mnLastFrmId=0;

TYPEINIT1(SwFrm,SwClient);      //rtti for SwFrm
TYPEINIT1(SwCntntFrm,SwFrm);    //rtti for SwCntntFrm

void _FrmInit()
{
    SwRootFrm::mpVout = new SwLayVout();
    SwCache *pNew = new SwCache( 100
#ifdef DBG_UTIL
    , "static SwBorderAttrs::pCache"
#endif
    );
    SwFrm::SetCache( pNew );
}

void _FrmFinit()
{
#if OSL_DEBUG_LEVEL > 0
    // The cache may only contain null pointers at this time.
    for( size_t n = SwFrm::GetCachePtr()->size(); n; )
        if( (*SwFrm::GetCachePtr())[ --n ] )
        {
            SwCacheObj* pObj = (*SwFrm::GetCachePtr())[ n ];
            OSL_ENSURE( !pObj, "Who didn't deregister?");
        }
#endif
    delete SwRootFrm::mpVout;
    delete SwFrm::GetCachePtr();
}

// RootFrm::Everything that belongs to CurrShell

class SwCurrShells : public std::set<CurrShell*> {};

CurrShell::CurrShell( SwViewShell *pNew )
{
    OSL_ENSURE( pNew, "insert 0-Shell?" );
    pRoot = pNew->GetLayout();
    if ( pRoot )
    {
        pPrev = pRoot->mpCurrShell;
        pRoot->mpCurrShell = pNew;
        pRoot->mpCurrShells->insert( this );
    }
    else
        pPrev = 0;
}

CurrShell::~CurrShell()
{
    if ( pRoot )
    {
        pRoot->mpCurrShells->erase( this );
        if ( pPrev )
            pRoot->mpCurrShell = pPrev;
        if ( pRoot->mpCurrShells->empty() && pRoot->mpWaitingCurrShell )
        {
            pRoot->mpCurrShell = pRoot->mpWaitingCurrShell;
            pRoot->mpWaitingCurrShell = 0;
        }
    }
}

void SetShell( SwViewShell *pSh )
{
    SwRootFrm *pRoot = pSh->GetLayout();
    if ( pRoot->mpCurrShells->empty() )
        pRoot->mpCurrShell = pSh;
    else
        pRoot->mpWaitingCurrShell = pSh;
}

void SwRootFrm::DeRegisterShell( SwViewShell *pSh )
{
    // Activate some shell if possible
    if ( mpCurrShell == pSh )
    {
        mpCurrShell = nullptr;
        for(SwViewShell& rShell : pSh->GetRingContainer())
        {
            if(&rShell != pSh)
            {
                mpCurrShell = &rShell;
                break;
            }
        }
    }

    // Doesn't matter anymore
    if ( mpWaitingCurrShell == pSh )
        mpWaitingCurrShell = 0;

    // Remove references
    for ( SwCurrShells::iterator it = mpCurrShells->begin(); it != mpCurrShells->end(); ++it )
    {
        CurrShell *pC = *it;
        if (pC->pPrev == pSh)
            pC->pPrev = 0;
    }
}

void InitCurrShells( SwRootFrm *pRoot )
{
    pRoot->mpCurrShells = new SwCurrShells;
}

/*
|*      The RootFrm requests an own FrmFmt from the document, which it is
|*      going to delete again in the dtor. The own FrmFmt is derived from
|*      the passed FrmFmt.
|*/
SwRootFrm::SwRootFrm( SwFrmFmt *pFmt, SwViewShell * pSh ) :
    SwLayoutFrm( pFmt->GetDoc()->MakeFrmFmt(
        OUString("Root"), pFmt ), 0 ),
    maPagesArea(),
    mnViewWidth( -1 ),
    mnColumns( 0 ),
    mbBookMode( false ),
    mbSidebarChanged( false ),
    mbNeedGrammarCheck( false ),
    mbCheckSuperfluous( false ),
    mbIdleFormat( true ),
    mbBrowseWidthValid( false ),
    mbTurboAllowed( true ),
    mbAssertFlyPages( true ),
    mbIsVirtPageNum( false ),
    mbIsNewLayout( true ),
    mbCallbackActionEnabled ( false ),
    mbLayoutFreezed ( false ),
    mnBrowseWidth( MM50*4 ), //2cm minimum
    mpTurbo( 0 ),
    mpLastPage( 0 ),
    mpCurrShell( pSh ),
    mpWaitingCurrShell( 0 ),
    mpCurrShells(NULL),
    mpDrawPage( 0 ),
    mpDestroy( 0 ),
    mnPhyPageNums( 0 ),
    mnAccessibleShells( 0 )
{
    mnFrmType = FRM_ROOT;
    setRootFrm( this );
}

void SwRootFrm::Init( SwFrmFmt* pFmt )
{
    InitCurrShells( this );

    IDocumentTimerAccess *pTimerAccess = pFmt->getIDocumentTimerAccess();
    IDocumentLayoutAccess *pLayoutAccess = pFmt->getIDocumentLayoutAccess();
    IDocumentFieldsAccess *pFieldsAccess = pFmt->getIDocumentFieldsAccess();
    const IDocumentSettingAccess *pSettingAccess = pFmt->getIDocumentSettingAccess();
    pTimerAccess->StopIdling();
    // For creating the Flys by MakeFrms()
    pLayoutAccess->SetCurrentViewShell( this->GetCurrShell() );
    mbCallbackActionEnabled = false; // needs to be set to true before leaving!

    SwDrawModel* pMd = pFmt->getIDocumentDrawModelAccess()->GetDrawModel();
    if ( pMd )
    {
        // Disable "multiple layout"
        mpDrawPage = pMd->GetPage(0);

        mpDrawPage->SetSize( Frm().SSize() );
    }

    // Initialize the layout: create pages, link content with Cntnt etc.
    // First, initialize some stuff, then get hold of the first
    // node (which will be needed for the PageDesc).

    SwDoc* pDoc = pFmt->GetDoc();
    SwNodeIndex aIndex( *pDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
    SwCntntNode *pNode = pDoc->GetNodes().GoNextSection( &aIndex, true, false );
    // #123067# pNode = 0 can really happen
    SwTableNode *pTblNd= pNode ? pNode->FindTableNode() : 0;

    // Get hold of PageDesc (either via FrmFmt of the first node or the initial one).
    SwPageDesc *pDesc = 0;
    ::boost::optional<sal_uInt16> oPgNum;

    if ( pTblNd )
    {
        const SwFmtPageDesc &rDesc = pTblNd->GetTable().GetFrmFmt()->GetPageDesc();
        pDesc = const_cast<SwPageDesc*>(rDesc.GetPageDesc());
        //#19104# respect the page number offset!!
        oPgNum = rDesc.GetNumOffset();
        if (oPgNum)
            mbIsVirtPageNum = true;
    }
    else if ( pNode )
    {
        const SwFmtPageDesc &rDesc = pNode->GetSwAttrSet().GetPageDesc();
        pDesc = const_cast<SwPageDesc*>(rDesc.GetPageDesc());
        //#19104# respect the page number offset!!
        oPgNum = rDesc.GetNumOffset();
        if (oPgNum)
            mbIsVirtPageNum = true;
    }
    else
        mbIsVirtPageNum = false;
    if ( !pDesc )
        pDesc = &pDoc->GetPageDesc( 0 );
    const bool bOdd = !oPgNum || 0 != ( oPgNum.get() % 2 );
    bool bFirst = !oPgNum || 1 == oPgNum.get();

    // Create a page and put it in the layout
    SwPageFrm *pPage = ::InsertNewPage( *pDesc, this, bOdd, bFirst, false, false, 0 );

    // Find the first page in the Bodytext section.
    SwLayoutFrm *pLay = pPage->FindBodyCont();
    while( pLay->Lower() )
        pLay = static_cast<SwLayoutFrm*>(pLay->Lower());

    SwNodeIndex aTmp( *pDoc->GetNodes().GetEndOfContent().StartOfSectionNode(), 1 );
    ::_InsertCnt( pLay, pDoc, aTmp.GetIndex(), true );
    //Remove masters that haven't been replaced yet from the list.
    RemoveMasterObjs( mpDrawPage );
    if( pSettingAccess->get(DocumentSettingId::GLOBAL_DOCUMENT) )
        pFieldsAccess->UpdateRefFlds( NULL );
    //b6433357: Update page fields after loading
    if ( !mpCurrShell || !mpCurrShell->Imp()->IsUpdateExpFlds() )
    {
        SwDocPosUpdate aMsgHnt( pPage->Frm().Top() );
        pFieldsAccess->UpdatePageFlds( &aMsgHnt );
    }

    pTimerAccess->StartIdling();
    mbCallbackActionEnabled = true;

    SwViewShell *pViewSh  = GetCurrShell();
    if (pViewSh)
        mbNeedGrammarCheck = pViewSh->GetViewOptions()->IsOnlineSpell();
}

SwRootFrm::~SwRootFrm()
{
    mbTurboAllowed = false;
    mpTurbo = 0;
    // fdo#39510 crash on document close with footnotes
    // Object ownership in writer and esp. in layout are a mess: Before the
    // document/layout split SwDoc and SwRootFrm were essentially one object
    // and magically/uncleanly worked around their common destruction by call
    // to SwDoc::IsInDtor() -- even from the layout. As of now destuction of
    // the layout proceeds forward through the frames. Since SwTxtFtn::DelFrms
    // also searches backwards to find the master of footnotes, they must be
    // considered to be owned by the SwRootFrm and also be destroyed here,
    // before tearing down the (now footnote free) rest of the layout.
    RemoveFtns(0, false, true);

    if(pBlink)
        pBlink->FrmDelete( this );
    SwFrmFmt *pRegisteredInNonConst = static_cast<SwFrmFmt*>(GetRegisteredInNonConst());
    if ( pRegisteredInNonConst )
    {
        SwDoc *pDoc = pRegisteredInNonConst->GetDoc();
        pDoc->DelFrmFmt( pRegisteredInNonConst );
        pDoc->GetDocumentLayoutManager().ClearSwLayouterEntries();
    }
    delete mpDestroy;
    mpDestroy = 0;

    // Remove references
    for ( SwCurrShells::iterator it = mpCurrShells->begin(); it != mpCurrShells->end(); ++it )
        (*it)->pRoot = 0;

    delete mpCurrShells;
    mpCurrShells = 0;

    // Some accessible shells are left => problems on second SwFrm::Destroy call
    assert(0 == mnAccessibleShells);

    // manually call base classes Destroy because it could call stuff
    // that accesses members of this
    SwLayoutFrm::Destroy();
    SwFrm::Destroy();
}

void SwRootFrm::RemoveMasterObjs( SdrPage *pPg )
{
    // Remove all master objects from the Page. But don't delete!
    for( size_t i = pPg ? pPg->GetObjCount() : 0; i; )
    {
        SdrObject* pObj = pPg->GetObj( --i );
        if( pObj->ISA(SwFlyDrawObj ) )
            pPg->RemoveObject( i );
    }
}

void SwRootFrm::AllCheckPageDescs() const
{
    if ( !IsLayoutFreezed() )
        CheckPageDescs( const_cast<SwPageFrm*>(static_cast<const SwPageFrm*>(this->Lower())) );
}

void SwRootFrm::AllInvalidateAutoCompleteWords() const
{
    SwPageFrm *pPage = const_cast<SwPageFrm*>(static_cast<const SwPageFrm*>(this->Lower()));
    while ( pPage )
    {
        pPage->InvalidateAutoCompleteWords();
        pPage = static_cast<SwPageFrm*>(pPage->GetNext());
    }
}

void SwRootFrm::AllAddPaintRect() const
{
    GetCurrShell()->AddPaintRect( this->Frm() );
}

void SwRootFrm::AllRemoveFtns()
{
    RemoveFtns();
}

void SwRootFrm::AllInvalidateSmartTagsOrSpelling(bool bSmartTags) const
{
    SwPageFrm *pPage = const_cast<SwPageFrm*>(static_cast<const SwPageFrm*>(this->Lower()));
    while ( pPage )
    {
        if ( bSmartTags )
            pPage->InvalidateSmartTags();

        pPage->InvalidateSpelling();
        pPage = static_cast<SwPageFrm*>(pPage->GetNext());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
