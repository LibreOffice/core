/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <fmtfordr.hxx>
#include <fmtpdsc.hxx>
#include <frmfmt.hxx>
#include <swtable.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <viewsh.hxx>
#include <doc.hxx>
#include <node.hxx>
#include <dflyobj.hxx>
#include <frmtool.hxx>
#include <virtoutp.hxx>
#include <blink.hxx>
#include <ndindex.hxx>
#include <sectfrm.hxx>
#include <notxtfrm.hxx>
#include <pagedesc.hxx>
#include "viewimp.hxx"
#include "IDocumentTimerAccess.hxx"
#include "IDocumentLayoutAccess.hxx"
#include "IDocumentFieldsAccess.hxx"
#include "IDocumentSettingAccess.hxx"
#include "IDocumentDrawModelAccess.hxx"
#include <hints.hxx>
#include <viewopt.hxx>
#include <set>

SwLayVout     *SwRootFrm::pVout = 0;
bool           SwRootFrm::bInPaint = false;
sal_Bool           SwRootFrm::bNoVirDev = sal_False;

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


sal_uInt32 SwFrm::mnLastFrmId=0;

TYPEINIT1(SwFrm,SwClient);      
TYPEINIT1(SwCntntFrm,SwFrm);    

void _FrmInit()
{
    SwRootFrm::pVout = new SwLayVout();
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
    
    for( sal_uInt16 n = SwFrm::GetCachePtr()->size(); n; )
        if( (*SwFrm::GetCachePtr())[ --n ] )
        {
            SwCacheObj* pObj = (*SwFrm::GetCachePtr())[ n ];
            OSL_ENSURE( !pObj, "Who didn't deregister?");
        }
#endif
    delete SwRootFrm::pVout;
    delete SwFrm::GetCachePtr();
}



class SwCurrShells : public std::set<CurrShell*> {};

CurrShell::CurrShell( SwViewShell *pNew )
{
    OSL_ENSURE( pNew, "insert 0-Shell?" );
    pRoot = pNew->GetLayout();
    if ( pRoot )
    {
        pPrev = pRoot->pCurrShell;
        pRoot->pCurrShell = pNew;
        pRoot->pCurrShells->insert( this );
    }
    else
        pPrev = 0;
}

CurrShell::~CurrShell()
{
    if ( pRoot )
    {
        pRoot->pCurrShells->erase( this );
        if ( pPrev )
            pRoot->pCurrShell = pPrev;
        if ( pRoot->pCurrShells->empty() && pRoot->pWaitingCurrShell )
        {
            pRoot->pCurrShell = pRoot->pWaitingCurrShell;
            pRoot->pWaitingCurrShell = 0;
        }
    }
}

void SetShell( SwViewShell *pSh )
{
    SwRootFrm *pRoot = pSh->GetLayout();
    if ( pRoot->pCurrShells->empty() )
        pRoot->pCurrShell = pSh;
    else
        pRoot->pWaitingCurrShell = pSh;
}

void SwRootFrm::DeRegisterShell( SwViewShell *pSh )
{
    
    if ( pCurrShell == pSh )
        pCurrShell = pSh->GetNext() != pSh ? (SwViewShell*)pSh->GetNext() : 0;

    
    if ( pWaitingCurrShell == pSh )
        pWaitingCurrShell = 0;

    
    for ( SwCurrShells::iterator it = pCurrShells->begin(); it != pCurrShells->end(); ++it )
    {
        CurrShell *pC = *it;
        if (pC->pPrev == pSh)
            pC->pPrev = 0;
    }
}

void InitCurrShells( SwRootFrm *pRoot )
{
    pRoot->pCurrShells = new SwCurrShells;
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
    bCheckSuperfluous( false ),
    bIdleFormat( true ),
    bBrowseWidthValid( false ),
    bTurboAllowed( true ),
    bAssertFlyPages( true ),
    bIsVirtPageNum( false ),
    bIsNewLayout( true ),
    bCallbackActionEnabled ( false ),
    bLayoutFreezed ( false ),
    nBrowseWidth( MM50*4 ), 
    pTurbo( 0 ),
    pLastPage( 0 ),
    pCurrShell( pSh ),
    pWaitingCurrShell( 0 ),
    pCurrShells(NULL),
    pDrawPage( 0 ),
    pDestroy( 0 ),
    nPhyPageNums( 0 ),
    nAccessibleShells( 0 )
{
    mnType = FRMC_ROOT;
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
    
    pLayoutAccess->SetCurrentViewShell( this->GetCurrShell() );
    bCallbackActionEnabled = sal_False; 

    SdrModel *pMd = pFmt->getIDocumentDrawModelAccess()->GetDrawModel();
    if ( pMd )
    {
        
        pDrawPage = pMd->GetPage(0);

        pDrawPage->SetSize( Frm().SSize() );
    }

    
    
    

    SwDoc* pDoc = pFmt->GetDoc();
    SwNodeIndex aIndex( *pDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
    SwCntntNode *pNode = pDoc->GetNodes().GoNextSection( &aIndex, true, false );
    
    SwTableNode *pTblNd= pNode ? pNode->FindTableNode() : 0;

    
    SwPageDesc *pDesc = 0;
    ::boost::optional<sal_uInt16> oPgNum;

    if ( pTblNd )
    {
        const SwFmtPageDesc &rDesc = pTblNd->GetTable().GetFrmFmt()->GetPageDesc();
        pDesc = (SwPageDesc*)rDesc.GetPageDesc();
        
        oPgNum = rDesc.GetNumOffset();
        if (oPgNum)
            bIsVirtPageNum = true;
    }
    else if ( pNode )
    {
        const SwFmtPageDesc &rDesc = pNode->GetSwAttrSet().GetPageDesc();
        pDesc = (SwPageDesc*)rDesc.GetPageDesc();
        
        oPgNum = rDesc.GetNumOffset();
        if (oPgNum)
            bIsVirtPageNum = true;
    }
    else
        bIsVirtPageNum = sal_False;
    if ( !pDesc )
        pDesc = &pDoc->GetPageDesc( 0 );
    const bool bOdd = !oPgNum || 0 != ( oPgNum.get() % 2 );
    bool bFirst = !oPgNum || 1 == oPgNum.get();

    
    SwPageFrm *pPage = ::InsertNewPage( *pDesc, this, bOdd, bFirst, false, sal_False, 0 );

    
    SwLayoutFrm *pLay = pPage->FindBodyCont();
    while( pLay->Lower() )
        pLay = (SwLayoutFrm*)pLay->Lower();

    SwNodeIndex aTmp( *pDoc->GetNodes().GetEndOfContent().StartOfSectionNode(), 1 );
    ::_InsertCnt( pLay, pDoc, aTmp.GetIndex(), sal_True );
    
    RemoveMasterObjs( pDrawPage );
    if( pSettingAccess->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
        pFieldsAccess->UpdateRefFlds( NULL );
    
    if ( !pCurrShell || !pCurrShell->Imp()->IsUpdateExpFlds() )
    {
        SwDocPosUpdate aMsgHnt( pPage->Frm().Top() );
        pFieldsAccess->UpdatePageFlds( &aMsgHnt );
    }

    pTimerAccess->StartIdling();
    bCallbackActionEnabled = sal_True;

    SwViewShell *pViewSh  = GetCurrShell();
    if (pViewSh)
        mbNeedGrammarCheck = pViewSh->GetViewOptions()->IsOnlineSpell();
}

SwRootFrm::~SwRootFrm()
{
    bTurboAllowed = sal_False;
    pTurbo = 0;
    
    
    
    
    
    
    
    
    
    RemoveFtns(0, false, true);

    if(pBlink)
        pBlink->FrmDelete( this );
    SwFrmFmt *pRegisteredInNonConst = static_cast<SwFrmFmt*>(GetRegisteredInNonConst());
    if ( pRegisteredInNonConst )
    {
        SwDoc *pDoc = pRegisteredInNonConst->GetDoc();
        pDoc->DelFrmFmt( pRegisteredInNonConst );
        pDoc->ClearSwLayouterEntries();
    }
    delete pDestroy;
    pDestroy = 0;

    
    for ( SwCurrShells::iterator it = pCurrShells->begin(); it != pCurrShells->end(); ++it )
        (*it)->pRoot = 0;

    delete pCurrShells;
    pCurrShells = 0;

    
    assert(0 == nAccessibleShells);

    
    
    SwLayoutFrm::Destroy();
    SwFrm::Destroy();
}

void SwRootFrm::RemoveMasterObjs( SdrPage *pPg )
{
    
    for( sal_uLong i = pPg ? pPg->GetObjCount() : 0; i; )
    {
        SdrObject* pObj = pPg->GetObj( --i );
        if( pObj->ISA(SwFlyDrawObj ) )
            pPg->RemoveObject( i );
    }
}

void SwRootFrm::AllCheckPageDescs() const
{
    if ( !IsLayoutFreezed() )
        CheckPageDescs( (SwPageFrm*)this->Lower() );
}

void SwRootFrm::AllInvalidateAutoCompleteWords() const
{
    SwPageFrm *pPage = (SwPageFrm*)this->Lower();
    while ( pPage )
    {
        pPage->InvalidateAutoCompleteWords();
        pPage = (SwPageFrm*)pPage->GetNext();
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
    SwPageFrm *pPage = (SwPageFrm*)this->Lower();
    while ( pPage )
    {
        if ( bSmartTags )
            pPage->InvalidateSmartTags();

        pPage->InvalidateSpelling();
        pPage = (SwPageFrm*)pPage->GetNext();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
