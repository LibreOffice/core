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
sal_Bool           SwRootFrm::bInPaint = sal_False;
sal_Bool           SwRootFrm::bNoVirDev = sal_False;

SwCache *SwFrm::pCache = 0;

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
//Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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
//End of SCMS
SwRectFn fnRectHori = &aHorizontal;
SwRectFn fnRectVert = &aVertical;
//Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
SwRectFn fnRectVertL2R = &aVerticalLeftToRight;
//End of SCMS
SwRectFn fnRectB2T = &aBottomToTop;
SwRectFn fnRectVL2R = &aVerticalRightToLeft;

// #i65250#
sal_uInt32 SwFrm::mnLastFrmId=0;

TYPEINIT1(SwFrm,SwClient);      //rtti for SwFrm
TYPEINIT1(SwCntntFrm,SwFrm);    //rtti for SwCntntFrm


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
    // The cache may only contain null pointers at this time.
    for( sal_uInt16 n = SwFrm::GetCachePtr()->size(); n; )
        if( (*SwFrm::GetCachePtr())[ --n ] )
        {
            SwCacheObj* pObj = (*SwFrm::GetCachePtr())[ n ];
            OSL_ENSURE( !pObj, "Who didn't derregister?");
        }
#endif
    delete SwRootFrm::pVout;
    delete SwFrm::GetCachePtr();
}

/*************************************************************************
|*
|*  RootFrm::Everything that belongs to CurrShell
|*
|*************************************************************************/

class SwCurrShells : public std::set<CurrShell*> {};

CurrShell::CurrShell( ViewShell *pNew )
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

void SetShell( ViewShell *pSh )
{
    SwRootFrm *pRoot = pSh->GetLayout();
    if ( pRoot->pCurrShells->empty() )
        pRoot->pCurrShell = pSh;
    else
        pRoot->pWaitingCurrShell = pSh;
}

void SwRootFrm::DeRegisterShell( ViewShell *pSh )
{
    // Activate some shell if possible
    if ( pCurrShell == pSh )
        pCurrShell = pSh->GetNext() != pSh ? (ViewShell*)pSh->GetNext() : 0;

    // Doesn't matter anymore
    if ( pWaitingCurrShell == pSh )
        pWaitingCurrShell = 0;

    // Remove references
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


/*************************************************************************
|*
|*  SwRootFrm::SwRootFrm()
|*
|*  Description:
|*      The RootFrm requests an own FrmFmt from the document, which it is
|*      going to delete again in the dtor. The own FrmFmt is derived from
|*      the passed FrmFmt.
|*
|*************************************************************************/


SwRootFrm::SwRootFrm( SwFrmFmt *pFmt, ViewShell * pSh ) :
    SwLayoutFrm( pFmt->GetDoc()->MakeFrmFmt(
        rtl::OUString("Root"), pFmt ), 0 ),
    // --> PAGES01
    maPagesArea(),
    mnViewWidth( -1 ),
    mnColumns( 0 ),
    mbBookMode( false ),
    mbSidebarChanged( false ),
    mbNeedGrammarCheck( false ),
    nBrowseWidth( MM50*4 ), //2cm minimum
    pTurbo( 0 ),
    pLastPage( 0 ),
    pCurrShell( pSh ),
    pWaitingCurrShell( 0 ),
    pDrawPage( 0 ),
    pDestroy( 0 ),
    nPhyPageNums( 0 ),
    nAccessibleShells( 0 )
{
    nType = FRMC_ROOT;
    bIdleFormat = bTurboAllowed = bAssertFlyPages = bIsNewLayout = sal_True;
    bCheckSuperfluous = bBrowseWidthValid = sal_False;
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
    bCallbackActionEnabled = sal_False; // needs to be set to sal_True before leaving!

    SdrModel *pMd = pFmt->getIDocumentDrawModelAccess()->GetDrawModel();
    if ( pMd )
    {
        // Disable "multiple layout"
        pDrawPage = pMd->GetPage(0); //pMd->AllocPage( FALSE );
        //pMd->InsertPage( pDrawPage );
        // end of disabling

        pDrawPage->SetSize( Frm().SSize() );
    }

    // Initialize the layout: create pages, link content with Cntnt etc.
    // First, initialize some stuff, then get hold of the first
    // node (which will be needed for the PageDesc).

    SwDoc* pDoc = pFmt->GetDoc();
    SwNodeIndex aIndex( *pDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
    SwCntntNode *pNode = pDoc->GetNodes().GoNextSection( &aIndex, sal_True, sal_False );
    // #123067# pNode = 0 can really happen
    SwTableNode *pTblNd= pNode ? pNode->FindTableNode() : 0;

    // Get hold of PageDesc (either via FrmFmt of the first node or the initial one).
    SwPageDesc *pDesc = 0;
    sal_uInt16 nPgNum = 1;

    if ( pTblNd )
    {
        const SwFmtPageDesc &rDesc = pTblNd->GetTable().GetFrmFmt()->GetPageDesc();
        pDesc = (SwPageDesc*)rDesc.GetPageDesc();
        //#19104# respect the page number offset!!
        bIsVirtPageNum = 0 != ( nPgNum = rDesc.GetNumOffset() );
    }
    else if ( pNode )
    {
        const SwFmtPageDesc &rDesc = pNode->GetSwAttrSet().GetPageDesc();
        pDesc = (SwPageDesc*)rDesc.GetPageDesc();
        //#19104# respect the page number offset!!
        bIsVirtPageNum = 0 != ( nPgNum = rDesc.GetNumOffset() );
    }
    else
        bIsVirtPageNum = sal_False;
    if ( !pDesc )
        pDesc = &pDoc->GetPageDesc( 0 );
    const sal_Bool bOdd = !nPgNum || 0 != ( nPgNum % 2 );
    bool bFirst = !nPgNum || 1 == nPgNum;

    // Create a page and put it in the layout
    SwPageFrm *pPage = ::InsertNewPage( *pDesc, this, bOdd, bFirst, sal_False, sal_False, 0 );

    // Find the first page in the Bodytext section.
    SwLayoutFrm *pLay = pPage->FindBodyCont();
    while( pLay->Lower() )
        pLay = (SwLayoutFrm*)pLay->Lower();

    SwNodeIndex aTmp( *pDoc->GetNodes().GetEndOfContent().StartOfSectionNode(), 1 );
    ::_InsertCnt( pLay, pDoc, aTmp.GetIndex(), sal_True );
    //Remove masters that haven't been replaced yet from the list.
    RemoveMasterObjs( pDrawPage );
    if( pSettingAccess->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
        pFieldsAccess->UpdateRefFlds( NULL );
    //b6433357: Update page fields after loading
    // --->
    if ( !pCurrShell || !pCurrShell->Imp()->IsUpdateExpFlds() )
    {
        SwDocPosUpdate aMsgHnt( pPage->Frm().Top() );
        pFieldsAccess->UpdatePageFlds( &aMsgHnt );
    }

    pTimerAccess->StartIdling();
    bCallbackActionEnabled = sal_True;

    ViewShell *pViewSh  = GetCurrShell();
    if (pViewSh)
        mbNeedGrammarCheck = pViewSh->GetViewOptions()->IsOnlineSpell();
}

/*************************************************************************
|*
|*  SwRootFrm::~SwRootFrm()
|*
|*************************************************************************/



SwRootFrm::~SwRootFrm()
{
    bTurboAllowed = sal_False;
    pTurbo = 0;
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
        pDoc->ClearSwLayouterEntries();
    }
    delete pDestroy;
    pDestroy = 0;

    // Remove references
    for ( SwCurrShells::iterator it = pCurrShells->begin(); it != pCurrShells->end(); ++it )
        (*it)->pRoot = 0;

    delete pCurrShells;
    pCurrShells = 0;

    // Some accessible shells are left => problems on second SwFrm::Destroy call
    assert(0 == nAccessibleShells);

    // manually call base classes Destroy because it could call stuff
    // that accesses members of this
    SwLayoutFrm::Destroy();
    SwFrm::Destroy();
}

/*************************************************************************
|*
|*  SwRootFrm::RemoveMasterObjs()
|*
|*************************************************************************/


void SwRootFrm::RemoveMasterObjs( SdrPage *pPg )
{
    // Remove all master objects from the Page. But don't delete!
    for( sal_uLong i = pPg ? pPg->GetObjCount() : 0; i; )
    {
        SdrObject* pObj = pPg->GetObj( --i );
        if( pObj->ISA(SwFlyDrawObj ) )
            pPg->RemoveObject( i );
    }
}


void SwRootFrm::AllCheckPageDescs() const
{
    CheckPageDescs( (SwPageFrm*)this->Lower() );
}
//swmod 080226
void SwRootFrm::AllInvalidateAutoCompleteWords() const
{
    SwPageFrm *pPage = (SwPageFrm*)this->Lower();
    while ( pPage )
    {
        pPage->InvalidateAutoCompleteWords();
        pPage = (SwPageFrm*)pPage->GetNext();
    }
}//swmod 080305
void SwRootFrm::AllAddPaintRect() const
{
    GetCurrShell()->AddPaintRect( this->Frm() );
}//swmod 080305
void SwRootFrm::AllRemoveFtns()
{
    RemoveFtns();
}
void SwRootFrm::AllInvalidateSmartTagsOrSpelling(sal_Bool bSmartTags) const
{
    SwPageFrm *pPage = (SwPageFrm*)this->Lower();
    while ( pPage )
    {
        if ( bSmartTags )
            pPage->InvalidateSmartTags();

        pPage->InvalidateSpelling();
        pPage = (SwPageFrm*)pPage->GetNext();
    }   //swmod 080218
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
