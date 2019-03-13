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

#include <sal/config.h>

#include <o3tl/safeint.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <drawdoc.hxx>
#include <fmtpdsc.hxx>
#include <swtable.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <dflyobj.hxx>
#include <frmtool.hxx>
#include "virtoutp.hxx"
#include <blink.hxx>
#include <sectfrm.hxx>
#include <notxtfrm.hxx>
#include <pagedesc.hxx>
#include <viewimp.hxx>
#include <hints.hxx>
#include <viewopt.hxx>
#include <set>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <DocumentLayoutManager.hxx>
#include <DocumentRedlineManager.hxx>
#include <ndindex.hxx>

SwLayVout     *SwRootFrame::s_pVout = nullptr;
bool           SwRootFrame::s_isInPaint = false;
bool           SwRootFrame::s_isNoVirDev = false;

SwCache *SwFrame::mpCache = nullptr;

static long FirstMinusSecond( long nFirst, long nSecond )
    { return nFirst - nSecond; }
static long SecondMinusFirst( long nFirst, long nSecond )
    { return nSecond - nFirst; }
static long SwIncrement( long nA, long nAdd )
    { return nA + nAdd; }
static long SwDecrement( long nA, long nSub )
    { return nA - nSub; }

static SwRectFnCollection aHorizontal = {
    /*.fnGetTop =*/&SwRect::Top_,
    /*.fnGetBottom =*/&SwRect::Bottom_,
    /*.fnGetLeft =*/&SwRect::Left_,
    /*.fnGetRight =*/&SwRect::Right_,
    /*.fnGetWidth =*/&SwRect::Width_,
    /*.fnGetHeight =*/&SwRect::Height_,
    /*.fnGetPos =*/&SwRect::TopLeft,
    /*.fnGetSize =*/&SwRect::Size_,

    /*.fnSetTop =*/&SwRect::Top_,
    /*.fnSetBottom =*/&SwRect::Bottom_,
    /*.fnSetLeft =*/&SwRect::Left_,
    /*.fnSetRight =*/&SwRect::Right_,
    /*.fnSetWidth =*/&SwRect::Width_,
    /*.fnSetHeight =*/&SwRect::Height_,

    /*.fnSubTop =*/&SwRect::SubTop,
    /*.fnAddBottom =*/&SwRect::AddBottom,
    /*.fnSubLeft =*/&SwRect::SubLeft,
    /*.fnAddRight =*/&SwRect::AddRight,
    /*.fnAddWidth =*/&SwRect::AddWidth,
    /*.fnAddHeight =*/&SwRect::AddHeight,

    /*.fnSetPosX =*/&SwRect::SetPosX,
    /*.fnSetPosY =*/&SwRect::SetPosY,

    /*.fnGetTopMargin =*/&SwFrame::GetTopMargin,
    /*.fnGetBottomMargin =*/&SwFrame::GetBottomMargin,
    /*.fnGetLeftMargin =*/&SwFrame::GetLeftMargin,
    /*.fnGetRightMargin =*/&SwFrame::GetRightMargin,
    /*.fnSetXMargins =*/&SwFrame::SetLeftRightMargins,
    /*.fnSetYMargins =*/&SwFrame::SetTopBottomMargins,
    /*.fnGetPrtTop =*/&SwFrame::GetPrtTop,
    /*.fnGetPrtBottom =*/&SwFrame::GetPrtBottom,
    /*.fnGetPrtLeft =*/&SwFrame::GetPrtLeft,
    /*.fnGetPrtRight =*/&SwFrame::GetPrtRight,
    /*.fnTopDist =*/&SwRect::GetTopDistance,
    /*.fnBottomDist =*/&SwRect::GetBottomDistance,
    /*.fnLeftDist =*/&SwRect::GetLeftDistance,
    /*.fnRightDist =*/&SwRect::GetRightDistance,
    /*.fnSetLimit =*/&SwFrame::SetMaxBottom,
    /*.fnOverStep =*/&SwRect::OverStepBottom,

    /*.fnSetPos =*/&SwRect::SetUpperLeftCorner,
    /*.fnMakePos =*/&SwFrame::MakeBelowPos,
    /*.fnXDiff =*/&FirstMinusSecond,
    /*.fnYDiff =*/&FirstMinusSecond,
    /*.fnXInc =*/&SwIncrement,
    /*.fnYInc =*/&o3tl::saturating_add<long>,

    /*.fnSetLeftAndWidth =*/&SwRect::SetLeftAndWidth,
    /*.fnSetTopAndHeight =*/&SwRect::SetTopAndHeight
};

static SwRectFnCollection aVertical = {
    /*.fnGetTop =*/&SwRect::Right_,
    /*.fnGetBottom =*/&SwRect::Left_,
    /*.fnGetLeft =*/&SwRect::Top_,
    /*.fnGetRight =*/&SwRect::Bottom_,
    /*.fnGetWidth =*/&SwRect::Height_,
    /*.fnGetHeight =*/&SwRect::Width_,
    /*.fnGetPos =*/&SwRect::TopRight,
    /*.fnGetSize =*/&SwRect::SwappedSize,

    /*.fnSetTop =*/&SwRect::Right_,
    /*.fnSetBottom =*/&SwRect::Left_,
    /*.fnSetLeft =*/&SwRect::Top_,
    /*.fnSetRight =*/&SwRect::Bottom_,
    /*.fnSetWidth =*/&SwRect::Height_,
    /*.fnSetHeight =*/&SwRect::Width_,

    /*.fnSubTop =*/&SwRect::AddRight,
    /*.fnAddBottom =*/&SwRect::SubLeft,
    /*.fnSubLeft =*/&SwRect::SubTop,
    /*.fnAddRight =*/&SwRect::AddBottom,
    /*.fnAddWidth =*/&SwRect::AddHeight,
    /*.fnAddHeight =*/&SwRect::AddWidth,

    /*.fnSetPosX =*/&SwRect::SetPosY,
    /*.fnSetPosY =*/&SwRect::SetPosX,

    /*.fnGetTopMargin =*/&SwFrame::GetRightMargin,
    /*.fnGetBottomMargin =*/&SwFrame::GetLeftMargin,
    /*.fnGetLeftMargin =*/&SwFrame::GetTopMargin,
    /*.fnGetRightMargin =*/&SwFrame::GetBottomMargin,
    /*.fnSetXMargins =*/&SwFrame::SetTopBottomMargins,
    /*.fnSetYMargins =*/&SwFrame::SetRightLeftMargins,
    /*.fnGetPrtTop =*/&SwFrame::GetPrtRight,
    /*.fnGetPrtBottom =*/&SwFrame::GetPrtLeft,
    /*.fnGetPrtLeft =*/&SwFrame::GetPrtTop,
    /*.fnGetPrtRight =*/&SwFrame::GetPrtBottom,
    /*.fnTopDist =*/&SwRect::GetRightDistance,
    /*.fnBottomDist =*/&SwRect::GetLeftDistance,
    /*.fnLeftDist =*/&SwRect::GetTopDistance,
    /*.fnRightDist =*/&SwRect::GetBottomDistance,
    /*.fnSetLimit =*/&SwFrame::SetMinLeft,
    /*.fnOverStep =*/&SwRect::OverStepLeft,

    /*.fnSetPos =*/&SwRect::SetUpperRightCorner,
    /*.fnMakePos =*/&SwFrame::MakeLeftPos,
    /*.fnXDiff =*/&FirstMinusSecond,
    /*.fnYDiff =*/&SecondMinusFirst,
    /*.fnXInc =*/&SwIncrement,
    /*.fnYInc =*/&SwDecrement,

    /*.fnSetLeftAndWidth =*/&SwRect::SetTopAndHeight,
    /*.fnSetTopAndHeight =*/&SwRect::SetRightAndWidth
};

static SwRectFnCollection aVerticalLeftToRight = {
    /*.fnGetTop =*/&SwRect::Left_,
    /*.fnGetBottom =*/&SwRect::Right_,
    /*.fnGetLeft =*/&SwRect::Top_,
    /*.fnGetRight =*/&SwRect::Bottom_,
    /*.fnGetWidth =*/&SwRect::Height_,
    /*.fnGetHeight =*/&SwRect::Width_,
    /*.fnGetPos =*/&SwRect::TopLeft,
    /*.fnGetSize =*/&SwRect::SwappedSize,

    /*.fnSetTop =*/&SwRect::Left_,
    /*.fnSetBottom =*/&SwRect::Right_,
    /*.fnSetLeft =*/&SwRect::Top_,
    /*.fnSetRight =*/&SwRect::Bottom_,
    /*.fnSetWidth =*/&SwRect::Height_,
    /*.fnSetHeight =*/&SwRect::Width_,

    /*.fnSubTop =*/&SwRect::SubLeft,
    /*.fnAddBottom =*/&SwRect::AddRight,
    /*.fnSubLeft =*/&SwRect::SubTop,
    /*.fnAddRight =*/&SwRect::AddBottom,
    /*.fnAddWidth =*/&SwRect::AddHeight,
    /*.fnAddHeight =*/&SwRect::AddWidth,

    /*.fnSetPosX =*/&SwRect::SetPosY,
    /*.fnSetPosY =*/&SwRect::SetPosX,

    /*.fnGetTopMargin =*/&SwFrame::GetLeftMargin,
    /*.fnGetBottomMargin =*/&SwFrame::GetRightMargin,
    /*.fnGetLeftMargin =*/&SwFrame::GetTopMargin,
    /*.fnGetRightMargin =*/&SwFrame::GetBottomMargin,
    /*.fnSetXMargins =*/&SwFrame::SetTopBottomMargins,
    /*.fnSetYMargins =*/&SwFrame::SetLeftRightMargins,
    /*.fnGetPrtTop =*/&SwFrame::GetPrtLeft,
    /*.fnGetPrtBottom =*/&SwFrame::GetPrtRight,
    /*.fnGetPrtLeft =*/&SwFrame::GetPrtTop,
    /*.fnGetPrtRight =*/&SwFrame::GetPrtBottom,
    /*.fnTopDist =*/&SwRect::GetLeftDistance,
    /*.fnBottomDist =*/&SwRect::GetRightDistance,
    /*.fnLeftDist =*/&SwRect::GetTopDistance,
    /*.fnRightDist =*/&SwRect::GetBottomDistance,
    /*.fnSetLimit =*/&SwFrame::SetMaxRight,
    /*.fnOverStep =*/&SwRect::OverStepRight,

    /*.fnSetPos =*/&SwRect::SetUpperLeftCorner,
    /*.fnMakePos =*/&SwFrame::MakeRightPos,
    /*.fnXDiff =*/&FirstMinusSecond,
    /*.fnYDiff =*/&FirstMinusSecond,
    /*.fnXInc =*/&SwIncrement,
    /*.fnYInc =*/&SwIncrement,

    /*.fnSetLeftAndWidth =*/&SwRect::SetTopAndHeight,
    /*.fnSetTopAndHeight =*/&SwRect::SetLeftAndWidth
};

/**
 * This is the same as horizontal, but rotated counter-clockwise by 90 degrees.
 * This means logical top is physical left, bottom is right, left is bottom,
 * finally right is top. Values map from logical to physical.
 */
static SwRectFnCollection aVerticalLeftToRightBottomToTop = {
    /*.fnGetTop =*/&SwRect::Left_,
    /*.fnGetBottom =*/&SwRect::Right_,
    /*.fnGetLeft =*/&SwRect::Bottom_,
    /*.fnGetRight =*/&SwRect::Top_,
    /*.fnGetWidth =*/&SwRect::Height_,
    /*.fnGetHeight =*/&SwRect::Width_,
    /*.fnGetPos =*/&SwRect::BottomLeft,
    /*.fnGetSize =*/&SwRect::SwappedSize,

    /*.fnSetTop =*/&SwRect::Left_,
    /*.fnSetBottom =*/&SwRect::Right_,
    /*.fnSetLeft =*/&SwRect::Bottom_,
    /*.fnSetRight =*/&SwRect::Top_,
    /*.fnSetWidth =*/&SwRect::Height_,
    /*.fnSetHeight =*/&SwRect::Width_,

    /*.fnSubTop =*/&SwRect::SubLeft,
    /*.fnAddBottom =*/&SwRect::AddRight,
    /*.fnSubLeft =*/&SwRect::AddBottom,
    /*.fnAddRight =*/&SwRect::SubTop,
    /*.fnAddWidth =*/&SwRect::AddHeight,
    /*.fnAddHeight =*/&SwRect::AddWidth,

    /*.fnSetPosX =*/&SwRect::SetPosY,
    /*.fnSetPosY =*/&SwRect::SetPosX,

    /*.fnGetTopMargin =*/&SwFrame::GetLeftMargin,
    /*.fnGetBottomMargin =*/&SwFrame::GetRightMargin,
    /*.fnGetLeftMargin =*/&SwFrame::GetBottomMargin,
    /*.fnGetRightMargin =*/&SwFrame::GetTopMargin,
    /*.fnSetXMargins =*/&SwFrame::SetTopBottomMargins,
    /*.fnSetYMargins =*/&SwFrame::SetLeftRightMargins,
    /*.fnGetPrtTop =*/&SwFrame::GetPrtLeft,
    /*.fnGetPrtBottom =*/&SwFrame::GetPrtRight,
    /*.fnGetPrtLeft =*/&SwFrame::GetPrtBottom,
    /*.fnGetPrtRight =*/&SwFrame::GetPrtTop,
    /*.fnTopDist =*/&SwRect::GetLeftDistance,
    /*.fnBottomDist =*/&SwRect::GetRightDistance,
    /*.fnLeftDist =*/&SwRect::GetBottomDistance,
    /*.fnRightDist =*/&SwRect::GetTopDistance,
    /*.fnSetLimit =*/&SwFrame::SetMaxRight,
    /*.fnOverStep =*/&SwRect::OverStepRight,

    /*.fnSetPos =*/&SwRect::SetLowerLeftCorner,
    /*.fnMakePos =*/&SwFrame::MakeRightPos,
    /*.fnXDiff =*/&SecondMinusFirst,
    /*.fnYDiff =*/&FirstMinusSecond,
    /*.fnXInc =*/&SwDecrement,
    /*.fnYInc =*/&SwIncrement,

    /*.fnSetLeftAndWidth =*/&SwRect::SetBottomAndHeight,
    /*.fnSetTopAndHeight =*/&SwRect::SetLeftAndWidth
};

SwRectFn fnRectHori = &aHorizontal;
SwRectFn fnRectVert = &aVertical;
SwRectFn fnRectVertL2R = &aVerticalLeftToRight;
SwRectFn fnRectVertL2RB2T = &aVerticalLeftToRightBottomToTop;

// #i65250#
sal_uInt32 SwFrameAreaDefinition::mnLastFrameId=0;


void FrameInit()
{
    SwRootFrame::s_pVout = new SwLayVout();
    SwCache *pNew = new SwCache( 100
#ifdef DBG_UTIL
    , "static SwBorderAttrs::pCache"
#endif
    );
    SwFrame::SetCache( pNew );
}

void FrameFinit()
{
#if OSL_DEBUG_LEVEL > 0
    // The cache may only contain null pointers at this time.
    for( size_t n = SwFrame::GetCachePtr()->size(); n; )
        if( (*SwFrame::GetCachePtr())[ --n ] )
        {
            SwCacheObj* pObj = (*SwFrame::GetCachePtr())[ n ];
            OSL_ENSURE( !pObj, "Who didn't deregister?");
        }
#endif
    delete SwRootFrame::s_pVout;
    delete SwFrame::GetCachePtr();
}

// RootFrame::Everything that belongs to CurrShell

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
        pPrev = nullptr;
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
            pRoot->mpWaitingCurrShell = nullptr;
        }
    }
}

void SetShell( SwViewShell *pSh )
{
    SwRootFrame *pRoot = pSh->GetLayout();
    if ( pRoot->mpCurrShells->empty() )
        pRoot->mpCurrShell = pSh;
    else
        pRoot->mpWaitingCurrShell = pSh;
}

void SwRootFrame::DeRegisterShell( SwViewShell *pSh )
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
        mpWaitingCurrShell = nullptr;

    // Remove references
    for ( CurrShell *pC : *mpCurrShells )
    {
        if (pC->pPrev == pSh)
            pC->pPrev = nullptr;
    }
}

void InitCurrShells( SwRootFrame *pRoot )
{
    pRoot->mpCurrShells.reset( new SwCurrShells );
}

/*
|*      The RootFrame requests an own FrameFormat from the document, which it is
|*      going to delete again in the dtor. The own FrameFormat is derived from
|*      the passed FrameFormat.
|*/
SwRootFrame::SwRootFrame( SwFrameFormat *pFormat, SwViewShell * pSh ) :
    SwLayoutFrame( pFormat->GetDoc()->MakeFrameFormat(
        "Root", pFormat ), nullptr ),
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
    mbHideRedlines(pFormat->GetDoc()->GetDocumentRedlineManager().IsHideRedlines()),
    mnBrowseWidth(MIN_BROWSE_WIDTH),
    mpTurbo( nullptr ),
    mpLastPage( nullptr ),
    mpCurrShell( pSh ),
    mpWaitingCurrShell( nullptr ),
    mpDrawPage( nullptr ),
    mnPhyPageNums( 0 ),
    mnAccessibleShells( 0 )
{
    mnFrameType = SwFrameType::Root;
    setRootFrame( this );
}

void SwRootFrame::Init( SwFrameFormat* pFormat )
{
    InitCurrShells( this );

    IDocumentTimerAccess& rTimerAccess = pFormat->getIDocumentTimerAccess();
    IDocumentLayoutAccess& rLayoutAccess = pFormat->getIDocumentLayoutAccess();
    IDocumentFieldsAccess& rFieldsAccess = pFormat->getIDocumentFieldsAccess();
    const IDocumentSettingAccess& rSettingAccess = pFormat->getIDocumentSettingAccess();
    rTimerAccess.StopIdling();
    // For creating the Flys by MakeFrames()
    rLayoutAccess.SetCurrentViewShell( GetCurrShell() );
    mbCallbackActionEnabled = false; // needs to be set to true before leaving!

    SwDrawModel* pMd = pFormat->getIDocumentDrawModelAccess().GetDrawModel();
    if ( pMd )
    {
        // Disable "multiple layout"
        mpDrawPage = pMd->GetPage(0);

        mpDrawPage->SetSize( getFrameArea().SSize() );
    }

    // Initialize the layout: create pages, link content with Content etc.
    // First, initialize some stuff, then get hold of the first
    // node (which will be needed for the PageDesc).

    SwDoc* pDoc = pFormat->GetDoc();
    SwNodeIndex aIndex( *pDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
    SwContentNode *pNode = pDoc->GetNodes().GoNextSection( &aIndex, true, false );
    // #123067# pNode = 0 can really happen
    SwTableNode *pTableNd= pNode ? pNode->FindTableNode() : nullptr;

    // Get hold of PageDesc (either via FrameFormat of the first node or the initial one).
    SwPageDesc *pDesc = nullptr;
    ::boost::optional<sal_uInt16> oPgNum;

    if ( pTableNd )
    {
        const SwFormatPageDesc &rDesc = pTableNd->GetTable().GetFrameFormat()->GetPageDesc();
        pDesc = const_cast<SwPageDesc*>(rDesc.GetPageDesc());
        //#19104# respect the page number offset!!
        oPgNum = rDesc.GetNumOffset();
        if (oPgNum)
            mbIsVirtPageNum = true;
    }
    else if ( pNode )
    {
        const SwFormatPageDesc &rDesc = pNode->GetSwAttrSet().GetPageDesc();
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
    const bool bFirst = true;
    // Even page numbers are supposed to be printed as left pages.  So if a
    // page number has been explicitly set for this first page, then we must
    // insert a blank page before it to make it a left page.
    const bool bInsertEmpty = !bOdd;

    // Create a page and put it in the layout
    SwPageFrame *pPage = ::InsertNewPage( *pDesc, this, bOdd, bFirst, bInsertEmpty, false, nullptr );

    // Find the first page in the Bodytext section.
    SwLayoutFrame *pLay = pPage->FindBodyCont();
    while( pLay->Lower() )
        pLay = static_cast<SwLayoutFrame*>(pLay->Lower());

    SwNodeIndex aTmp( *pDoc->GetNodes().GetEndOfContent().StartOfSectionNode(), 1 );
    ::InsertCnt_( pLay, pDoc, aTmp.GetIndex(), true );
    //Remove masters that haven't been replaced yet from the list.
    RemoveMasterObjs( mpDrawPage );
    if( rSettingAccess.get(DocumentSettingId::GLOBAL_DOCUMENT) )
        rFieldsAccess.UpdateRefFields();
    //b6433357: Update page fields after loading
    if ( !mpCurrShell || !mpCurrShell->Imp()->IsUpdateExpFields() )
    {
        SwDocPosUpdate aMsgHint( pPage->getFrameArea().Top() );
        rFieldsAccess.UpdatePageFields( &aMsgHint );
    }

    rTimerAccess.StartIdling();
    mbCallbackActionEnabled = true;

    SwViewShell *pViewSh  = GetCurrShell();
    if (pViewSh)
        mbNeedGrammarCheck = pViewSh->GetViewOptions()->IsOnlineSpell();
}

void SwRootFrame::DestroyImpl()
{
    mbTurboAllowed = false;
    mpTurbo = nullptr;

    if(pBlink)
        pBlink->FrameDelete( this );
    SwFrameFormat *pRegisteredInNonConst = static_cast<SwFrameFormat*>(GetDep());
    if ( pRegisteredInNonConst )
    {
        SwDoc *pDoc = pRegisteredInNonConst->GetDoc();
        pDoc->DelFrameFormat( pRegisteredInNonConst );
        // do this before calling RemoveFootnotes() because footnotes
        // can contain anchored objects
        pDoc->GetDocumentLayoutManager().ClearSwLayouterEntries();
    }

    mpDestroy.reset();

    // Remove references
    for ( auto& rpCurrShell : *mpCurrShells )
        rpCurrShell->pRoot = nullptr;

    mpCurrShells.reset();

    // Some accessible shells are left => problems on second SwFrame::Destroy call
    assert(0 == mnAccessibleShells);

    // fdo#39510 crash on document close with footnotes
    // Object ownership in writer and esp. in layout are a mess: Before the
    // document/layout split SwDoc and SwRootFrame were essentially one object
    // and magically/uncleanly worked around their common destruction by call
    // to SwDoc::IsInDtor() -- even from the layout. As of now destruction of
    // the layout proceeds forward through the frames. Since SwTextFootnote::DelFrames
    // also searches backwards to find the master of footnotes, they must be
    // considered to be owned by the SwRootFrame and also be destroyed here,
    // before tearing down the (now footnote free) rest of the layout.
    RemoveFootnotes(nullptr, false, true);

    SwLayoutFrame::DestroyImpl();
}

SwRootFrame::~SwRootFrame()
{
}

void SwRootFrame::RemoveMasterObjs( SdrPage *pPg )
{
    // Remove all master objects from the Page. But don't delete!
    for( size_t i = pPg ? pPg->GetObjCount() : 0; i; )
    {
        SdrObject* pObj = pPg->GetObj( --i );
        if( dynamic_cast< const SwFlyDrawObj *>( pObj ) !=  nullptr )
            pPg->RemoveObject( i );
    }
}

void SwRootFrame::AllCheckPageDescs() const
{
    if ( !IsLayoutFreezed() )
        CheckPageDescs( const_cast<SwPageFrame*>(static_cast<const SwPageFrame*>(Lower())) );
}

void SwRootFrame::AllInvalidateAutoCompleteWords() const
{
    SwPageFrame *pPage = const_cast<SwPageFrame*>(static_cast<const SwPageFrame*>(Lower()));
    while ( pPage )
    {
        pPage->InvalidateAutoCompleteWords();
        pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    }
}

void SwRootFrame::AllAddPaintRect() const
{
    GetCurrShell()->AddPaintRect( getFrameArea() );
}

void SwRootFrame::AllRemoveFootnotes()
{
    RemoveFootnotes();
}

void SwRootFrame::AllInvalidateSmartTagsOrSpelling(bool bSmartTags) const
{
    SwPageFrame *pPage = const_cast<SwPageFrame*>(static_cast<const SwPageFrame*>(Lower()));
    while ( pPage )
    {
        if ( bSmartTags )
            pPage->InvalidateSmartTags();

        pPage->InvalidateSpelling();
        pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
