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

SwLayVout     *SwRootFrame::s_pVout = nullptr;
bool           SwRootFrame::s_isInPaint = false;
bool           SwRootFrame::s_isNoVirDev = false;

SwCache *SwFrame::mpCache = nullptr;

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

    &SwFrame::GetTopMargin,
    &SwFrame::GetBottomMargin,
    &SwFrame::GetLeftMargin,
    &SwFrame::GetRightMargin,
    &SwFrame::SetLeftRightMargins,
    &SwFrame::SetTopBottomMargins,
    &SwFrame::GetPrtTop,
    &SwFrame::GetPrtBottom,
    &SwFrame::GetPrtLeft,
    &SwFrame::GetPrtRight,
    &SwRect::GetTopDistance,
    &SwRect::GetBottomDistance,
    &SwFrame::SetMaxBottom,
    &SwRect::OverStepBottom,

    &SwRect::SetUpperLeftCorner,
    &SwFrame::MakeBelowPos,
    &FirstMinusSecond,
    &FirstMinusSecond,
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

    &SwFrame::GetRightMargin,
    &SwFrame::GetLeftMargin,
    &SwFrame::GetTopMargin,
    &SwFrame::GetBottomMargin,
    &SwFrame::SetTopBottomMargins,
    &SwFrame::SetRightLeftMargins,
    &SwFrame::GetPrtRight,
    &SwFrame::GetPrtLeft,
    &SwFrame::GetPrtTop,
    &SwFrame::GetPrtBottom,
    &SwRect::GetRightDistance,
    &SwRect::GetLeftDistance,
    &SwFrame::SetMinLeft,
    &SwRect::OverStepLeft,

    &SwRect::SetUpperRightCorner,
    &SwFrame::MakeLeftPos,
    &FirstMinusSecond,
    &SecondMinusFirst,
    &SwIncrement,
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

    &SwFrame::GetBottomMargin,
    &SwFrame::GetTopMargin,
    &SwFrame::GetLeftMargin,
    &SwFrame::GetRightMargin,
    &SwFrame::SetLeftRightMargins,
    &SwFrame::SetBottomTopMargins,
    &SwFrame::GetPrtBottom,
    &SwFrame::GetPrtTop,
    &SwFrame::GetPrtLeft,
    &SwFrame::GetPrtRight,
    &SwRect::GetBottomDistance,
    &SwRect::GetTopDistance,
    &SwFrame::SetMinTop,
    &SwRect::OverStepTop,

    &SwRect::SetLowerLeftCorner,
    &SwFrame::MakeUpperPos,
    &FirstMinusSecond,
    &SecondMinusFirst,
    &SwIncrement,
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

    &SwFrame::GetLeftMargin,
    &SwFrame::GetRightMargin,
    &SwFrame::GetTopMargin,
    &SwFrame::GetBottomMargin,
    &SwFrame::SetTopBottomMargins,
    &SwFrame::SetLeftRightMargins,
    &SwFrame::GetPrtLeft,
    &SwFrame::GetPrtRight,
    &SwFrame::GetPrtBottom,
    &SwFrame::GetPrtTop,
    &SwRect::GetLeftDistance,
    &SwRect::GetRightDistance,
    &SwFrame::SetMaxRight,
    &SwRect::OverStepRight,

    &SwRect::SetLowerLeftCorner,
    &SwFrame::MakeRightPos,
    &FirstMinusSecond,
    &FirstMinusSecond,
    &SwDecrement,
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

    &SwFrame::GetLeftMargin,
    &SwFrame::GetRightMargin,
    &SwFrame::GetTopMargin,
    &SwFrame::GetBottomMargin,
    &SwFrame::SetTopBottomMargins,
    &SwFrame::SetLeftRightMargins,
    &SwFrame::GetPrtLeft,
    &SwFrame::GetPrtRight,
    &SwFrame::GetPrtTop,
    &SwFrame::GetPrtBottom,
    &SwRect::GetLeftDistance,
    &SwRect::GetRightDistance,
    &SwFrame::SetMaxRight,
    &SwRect::OverStepRight,

    &SwRect::SetUpperLeftCorner,
    &SwFrame::MakeRightPos,
    &FirstMinusSecond,
    &FirstMinusSecond,
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
sal_uInt32 SwFrame::mnLastFrameId=0;


void _FrameInit()
{
    SwRootFrame::s_pVout = new SwLayVout();
    SwCache *pNew = new SwCache( 100
#ifdef DBG_UTIL
    , "static SwBorderAttrs::pCache"
#endif
    );
    SwFrame::SetCache( pNew );
}

void _FrameFinit()
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
    for ( SwCurrShells::iterator it = mpCurrShells->begin(); it != mpCurrShells->end(); ++it )
    {
        CurrShell *pC = *it;
        if (pC->pPrev == pSh)
            pC->pPrev = nullptr;
    }
}

void InitCurrShells( SwRootFrame *pRoot )
{
    pRoot->mpCurrShells = new SwCurrShells;
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
    mnBrowseWidth( MM50*4 ), //2cm minimum
    mpTurbo( nullptr ),
    mpLastPage( nullptr ),
    mpCurrShell( pSh ),
    mpWaitingCurrShell( nullptr ),
    mpCurrShells(nullptr),
    mpDrawPage( nullptr ),
    mpDestroy( nullptr ),
    mnPhyPageNums( 0 ),
    mnAccessibleShells( 0 )
{
    mnFrameType = FRM_ROOT;
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
    rLayoutAccess.SetCurrentViewShell( this->GetCurrShell() );
    mbCallbackActionEnabled = false; // needs to be set to true before leaving!

    SwDrawModel* pMd = pFormat->getIDocumentDrawModelAccess().GetDrawModel();
    if ( pMd )
    {
        // Disable "multiple layout"
        mpDrawPage = pMd->GetPage(0);

        mpDrawPage->SetSize( Frame().SSize() );
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
    bool bFirst = !oPgNum || 1 == oPgNum.get();

    // Create a page and put it in the layout
    SwPageFrame *pPage = ::InsertNewPage( *pDesc, this, bOdd, bFirst, false, false, nullptr );

    // Find the first page in the Bodytext section.
    SwLayoutFrame *pLay = pPage->FindBodyCont();
    while( pLay->Lower() )
        pLay = static_cast<SwLayoutFrame*>(pLay->Lower());

    SwNodeIndex aTmp( *pDoc->GetNodes().GetEndOfContent().StartOfSectionNode(), 1 );
    ::_InsertCnt( pLay, pDoc, aTmp.GetIndex(), true );
    //Remove masters that haven't been replaced yet from the list.
    RemoveMasterObjs( mpDrawPage );
    if( rSettingAccess.get(DocumentSettingId::GLOBAL_DOCUMENT) )
        rFieldsAccess.UpdateRefFields( nullptr );
    //b6433357: Update page fields after loading
    if ( !mpCurrShell || !mpCurrShell->Imp()->IsUpdateExpFields() )
    {
        SwDocPosUpdate aMsgHint( pPage->Frame().Top() );
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
    // fdo#39510 crash on document close with footnotes
    // Object ownership in writer and esp. in layout are a mess: Before the
    // document/layout split SwDoc and SwRootFrame were essentially one object
    // and magically/uncleanly worked around their common destruction by call
    // to SwDoc::IsInDtor() -- even from the layout. As of now destuction of
    // the layout proceeds forward through the frames. Since SwTextFootnote::DelFrames
    // also searches backwards to find the master of footnotes, they must be
    // considered to be owned by the SwRootFrame and also be destroyed here,
    // before tearing down the (now footnote free) rest of the layout.
    RemoveFootnotes(nullptr, false, true);

    if(pBlink)
        pBlink->FrameDelete( this );
    SwFrameFormat *pRegisteredInNonConst = static_cast<SwFrameFormat*>(GetRegisteredInNonConst());
    if ( pRegisteredInNonConst )
    {
        SwDoc *pDoc = pRegisteredInNonConst->GetDoc();
        pDoc->DelFrameFormat( pRegisteredInNonConst );
        pDoc->GetDocumentLayoutManager().ClearSwLayouterEntries();
    }
    delete mpDestroy;
    mpDestroy = nullptr;

    // Remove references
    for ( SwCurrShells::iterator it = mpCurrShells->begin(); it != mpCurrShells->end(); ++it )
        (*it)->pRoot = nullptr;

    delete mpCurrShells;
    mpCurrShells = nullptr;

    // Some accessible shells are left => problems on second SwFrame::Destroy call
    assert(0 == mnAccessibleShells);

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
        CheckPageDescs( const_cast<SwPageFrame*>(static_cast<const SwPageFrame*>(this->Lower())) );
}

void SwRootFrame::AllInvalidateAutoCompleteWords() const
{
    SwPageFrame *pPage = const_cast<SwPageFrame*>(static_cast<const SwPageFrame*>(this->Lower()));
    while ( pPage )
    {
        pPage->InvalidateAutoCompleteWords();
        pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    }
}

void SwRootFrame::AllAddPaintRect() const
{
    GetCurrShell()->AddPaintRect( this->Frame() );
}

void SwRootFrame::AllRemoveFootnotes()
{
    RemoveFootnotes();
}

void SwRootFrame::AllInvalidateSmartTagsOrSpelling(bool bSmartTags) const
{
    SwPageFrame *pPage = const_cast<SwPageFrame*>(static_cast<const SwPageFrame*>(this->Lower()));
    while ( pPage )
    {
        if ( bSmartTags )
            pPage->InvalidateSmartTags();

        pPage->InvalidateSpelling();
        pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
