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

#include <editeng/pbinitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/shaditem.hxx>
#include "editeng/frmdiritem.hxx"
#include <fmtclds.hxx>
#include <fmtfsize.hxx>
#include <pagefrm.hxx>
#include <pagedesc.hxx>
#include <swtable.hxx>
#include <frmtool.hxx>
#include <doc.hxx>          // for GetAttrPool
#include <poolfmt.hrc>
#include <poolfmt.hxx>
#include <switerator.hxx>


using namespace ::com::sun::star;

/*************************************************************************
|*
|*  SwPageDesc::SwPageDesc()
|*
|*************************************************************************/



SwPageDesc::SwPageDesc( const OUString& rName, SwFrmFmt *pFmt, SwDoc *pDc ) :
    SwModify( 0 ),
    aDescName( rName ),
    aMaster( pDc->GetAttrPool(), rName, pFmt ),
    aLeft( pDc->GetAttrPool(), rName, pFmt ),
    aFirst( pDc->GetAttrPool(), rName, pFmt ),
    aDepend( this, 0 ),
    pFollow( this ),
    nRegHeight( 0 ),
    nRegAscent( 0 ),
    eUse( (UseOnPage)(nsUseOnPage::PD_ALL | nsUseOnPage::PD_HEADERSHARE | nsUseOnPage::PD_FOOTERSHARE | nsUseOnPage::PD_FIRSTSHARE ) ),
    bLandscape( sal_False ),
    bHidden( sal_False ),
    aFtnInfo()
{
}

SwPageDesc::SwPageDesc( const SwPageDesc &rCpy ) :
    SwModify( 0 ),
    aDescName( rCpy.GetName() ),
    aNumType( rCpy.GetNumType() ),
    aMaster( rCpy.GetMaster() ),
    aLeft( rCpy.GetLeft() ),
    aFirst( rCpy.GetFirst() ),
    aDepend( this, (SwModify*)rCpy.aDepend.GetRegisteredIn() ),
    pFollow( rCpy.pFollow ),
    nRegHeight( rCpy.GetRegHeight() ),
    nRegAscent( rCpy.GetRegAscent() ),
    eUse( rCpy.ReadUseOn() ),
    bLandscape( rCpy.GetLandscape() ),
    bHidden( rCpy.IsHidden() ),
    aFtnInfo( rCpy.GetFtnInfo() )
{
}

SwPageDesc & SwPageDesc::operator = (const SwPageDesc & rSrc)
{
    aDescName = rSrc.aDescName;
    aNumType = rSrc.aNumType;
    aMaster = rSrc.aMaster;
    aLeft = rSrc.aLeft;
    aFirst = rSrc.aFirst;

    if (rSrc.pFollow == &rSrc)
        pFollow = this;
    else
        pFollow = rSrc.pFollow;

    nRegHeight = rSrc.nRegHeight;
    nRegAscent = rSrc.nRegAscent;
    eUse = rSrc.eUse;
    bLandscape = rSrc.bLandscape;
    return *this;
}

SwPageDesc::~SwPageDesc()
{
}

/*************************************************************************
|*
|*  SwPageDesc::Mirror()
|*
|*  Description         Only the margin is mirrored.
|*      Attributes like borders and so on are copied 1:1.
|*
|*************************************************************************/



void SwPageDesc::Mirror()
{
    //Only the margins are mirrored, all other values are just copied.
    SvxLRSpaceItem aLR( RES_LR_SPACE );
    const SvxLRSpaceItem &rLR = aMaster.GetLRSpace();
    aLR.SetLeft(  rLR.GetRight() );
    aLR.SetRight( rLR.GetLeft() );

    SfxItemSet aSet( *aMaster.GetAttrSet().GetPool(),
                     aMaster.GetAttrSet().GetRanges() );
    aSet.Put( aLR );
    aSet.Put( aMaster.GetFrmSize() );
    aSet.Put( aMaster.GetPaperBin() );
    aSet.Put( aMaster.GetULSpace() );
    aSet.Put( aMaster.GetBox() );
    aSet.Put( aMaster.GetBackground() );
    aSet.Put( aMaster.GetShadow() );
    aSet.Put( aMaster.GetCol() );
    aSet.Put( aMaster.GetFrmDir() );    // #112217#
    aLeft.SetFmtAttr( aSet );
}

void SwPageDesc::ResetAllAttr( sal_Bool bLeft )
{
    SwFrmFmt& rFmt = bLeft ? GetLeft() : GetMaster();

    // #i73790# - method renamed
    rFmt.ResetAllFmtAttr();
    rFmt.SetFmtAttr( SvxFrameDirectionItem(FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR) );
}

/*************************************************************************
|*
|*                SwPageDesc::GetInfo()
|*
|*    Description       retrieves information
|*
*************************************************************************/


    // gets information from Modify
bool SwPageDesc::GetInfo( SfxPoolItem & rInfo ) const
{
    if( !aMaster.GetInfo( rInfo ) )
            return false;       // found
    return aLeft.GetInfo( rInfo );
}

/*************************************************************************
|*
|*                SwPageDesc::SetRegisterFmtColl()
|*
|*    Description       sets the style for the grid alignment
|*
*************************************************************************/


void SwPageDesc::SetRegisterFmtColl( const SwTxtFmtColl* pFmt )
{
    if( pFmt != GetRegisterFmtColl() )
    {
        if( pFmt )
            ((SwTxtFmtColl*)pFmt)->Add( &aDepend );
        else
            ((SwTxtFmtColl*)GetRegisterFmtColl())->Remove( &aDepend );

        RegisterChange();
    }
}

/*************************************************************************
|*
|*                SwPageDesc::GetRegisterFmtColl()
|*
|*    Description       retrieves the style for the grid alignment
|*
*************************************************************************/


const SwTxtFmtColl* SwPageDesc::GetRegisterFmtColl() const
{
    const SwModify* pReg = aDepend.GetRegisteredIn();
    return (SwTxtFmtColl*)pReg;
}

/*************************************************************************
|*
|*                SwPageDesc::RegisterChange()
|*
|*    Description       notifies all affected page frames
|*
*************************************************************************/


void SwPageDesc::RegisterChange()
{
    // #117072# - During destruction of the document <SwDoc>
    // the page description is modified. Thus, do nothing, if the document
    // is in destruction respectively if no viewshell exists.
    SwDoc* pDoc = GetMaster().GetDoc();
    if ( !pDoc || pDoc->IsInDtor() )
    {
        return;
    }
    ViewShell* pSh = 0L;
    pDoc->GetEditShell( &pSh );
    if ( !pSh )
    {
        return;
    }

    nRegHeight = 0;
    {
        SwIterator<SwFrm,SwFmt> aIter( GetMaster() );
        for( SwFrm* pLast = aIter.First(); pLast; pLast = aIter.Next() )
        {
            if( pLast->IsPageFrm() )
                ((SwPageFrm*)pLast)->PrepareRegisterChg();
        }
    }
    {
        SwIterator<SwFrm,SwFmt> aIter( GetLeft() );
        for( SwFrm* pLast = aIter.First(); pLast; pLast = aIter.Next() )
        {
            if( pLast->IsPageFrm() )
                ((SwPageFrm*)pLast)->PrepareRegisterChg();
        }
    }
    {
        SwIterator<SwFrm,SwFmt> aIter( GetFirst() );
        for( SwFrm* pLast = aIter.First(); pLast; pLast = aIter.Next() )
        {
            if( pLast->IsPageFrm() )
                ((SwPageFrm*)pLast)->PrepareRegisterChg();
        }
    }
}

/*************************************************************************
|*
|*                SwPageDesc::Modify()
|*
|*    special handling if the style of the grid alignment changes
|*
*************************************************************************/


void SwPageDesc::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    NotifyClients( pOld, pNew );

    if ( (RES_ATTRSET_CHG == nWhich) || (RES_FMT_CHG == nWhich)
        || isCHRATR(nWhich) || (RES_PARATR_LINESPACING == nWhich) )
    {
        RegisterChange();
    }
}

static const SwFrm* lcl_GetFrmOfNode( const SwNode& rNd )
{
    SwModify* pMod;
    sal_uInt16 nFrmType = FRM_CNTNT;

    if( rNd.IsCntntNode() )
    {
        pMod = &(SwCntntNode&)rNd;
    }
    else if( rNd.IsTableNode() )
    {
        pMod = ((SwTableNode&)rNd).GetTable().GetFrmFmt();
        nFrmType = FRM_TAB;
    }
    else
        pMod = 0;

    Point aNullPt;
    return pMod ? ::GetFrmOfModify( 0, *pMod, nFrmType, &aNullPt, 0, sal_False )
                : 0;
}

const SwPageDesc* SwPageDesc::GetPageDescOfNode(const SwNode& rNd)
{
    const SwPageDesc* pRet = 0;
    const SwFrm* pChkFrm = lcl_GetFrmOfNode( rNd );
    if (pChkFrm && 0 != (pChkFrm = pChkFrm->FindPageFrm()))
        pRet = ((const SwPageFrm*)pChkFrm)->GetPageDesc();
    return pRet;
}

const SwFrmFmt* SwPageDesc::GetPageFmtOfNode( const SwNode& rNd,
                                              sal_Bool bCheckForThisPgDc ) const
{
    // which PageDescFormat is valid for this node?
    const SwFrmFmt* pRet;
    const SwFrm* pChkFrm = lcl_GetFrmOfNode( rNd );

    if( pChkFrm && 0 != ( pChkFrm = pChkFrm->FindPageFrm() ))
    {
        const SwPageDesc* pPd = bCheckForThisPgDc ? this :
                                ((SwPageFrm*)pChkFrm)->GetPageDesc();
        pRet = &pPd->GetMaster();
        OSL_ENSURE( ((SwPageFrm*)pChkFrm)->GetPageDesc() == pPd, "Wrong node for detection of page format!" );
        // this page is assigned to which format?
        if( !pChkFrm->KnowsFormat(*pRet) )
        {
            pRet = &pPd->GetLeft();
            OSL_ENSURE( pChkFrm->KnowsFormat(*pRet), "Wrong node for detection of page format!" );
        }
    }
    else
        pRet = &GetMaster();
    return pRet;
}

sal_Bool SwPageDesc::IsFollowNextPageOfNode( const SwNode& rNd ) const
{
    sal_Bool bRet = sal_False;
    if( GetFollow() && this != GetFollow() )
    {
        const SwFrm* pChkFrm = lcl_GetFrmOfNode( rNd );
        if( pChkFrm && 0 != ( pChkFrm = pChkFrm->FindPageFrm() ) &&
            pChkFrm->IsPageFrm() &&
            ( !pChkFrm->GetNext() || GetFollow() ==
                        ((SwPageFrm*)pChkFrm->GetNext())->GetPageDesc() ))
            // the page on which the follow points was found
            bRet = sal_True;
    }
    return bRet;
}

SwFrmFmt *SwPageDesc::GetLeftFmt(bool const bFirst)
{
    return (nsUseOnPage::PD_LEFT & eUse)
            ? ((bFirst) ? &aFirst : &aLeft)
            : 0;
}

SwFrmFmt *SwPageDesc::GetRightFmt(bool const bFirst)
{
    return (nsUseOnPage::PD_RIGHT & eUse)
            ? ((bFirst) ? &aFirst : &aMaster)
            : 0;
}

sal_Bool SwPageDesc::IsFirstShared() const
{
    return eUse & nsUseOnPage::PD_FIRSTSHARE ? sal_True : sal_False;
}

void SwPageDesc::ChgFirstShare( sal_Bool bNew )
{
    if ( bNew )
        eUse = (UseOnPage) (eUse | nsUseOnPage::PD_FIRSTSHARE);
    else
        eUse = (UseOnPage) (eUse & nsUseOnPage::PD_NOFIRSTSHARE);
}

SwPageDesc* SwPageDesc::GetByName(SwDoc& rDoc, const OUString& rName)
{
    const sal_uInt16 nDCount = rDoc.GetPageDescCnt();

    for( sal_uInt16 i = 0; i < nDCount; i++ )
    {
        SwPageDesc* pDsc = &rDoc.GetPageDesc( i );
        if(pDsc->GetName() == rName)
        {
            return pDsc;
        }
    }

    for( sal_Int32 i = RC_POOLPAGEDESC_BEGIN; i <= STR_POOLPAGE_LANDSCAPE; ++i)
    {
        if (rName==SW_RESSTR(i))
        {
            return rDoc.GetPageDescFromPool( static_cast< sal_uInt16 >(
                        i - RC_POOLPAGEDESC_BEGIN + RES_POOLPAGE_BEGIN) );
        }
    }

    return 0;
}


/*************************************************************************
|*
|*  SwPageFtnInfo::SwPageFtnInfo()
|*
|*************************************************************************/



SwPageFtnInfo::SwPageFtnInfo() :
    nMaxHeight( 0 ),
    nLineWidth(10),
    eLineStyle( table::BorderLineStyle::SOLID ),
    aWidth( 25, 100 ),
    nTopDist( 57 ),         //1mm
    nBottomDist( 57 )
{
    eAdj = FRMDIR_HORI_RIGHT_TOP == GetDefaultFrameDirection(GetAppLanguage()) ?
           FTNADJ_RIGHT :
           FTNADJ_LEFT;
}



SwPageFtnInfo::SwPageFtnInfo( const SwPageFtnInfo &rCpy ) :
    nMaxHeight( rCpy.GetHeight() ),
    nLineWidth(rCpy.nLineWidth),
    eLineStyle(rCpy.eLineStyle),
    aLineColor(rCpy.aLineColor),
    aWidth( rCpy.GetWidth() ),
    eAdj( rCpy.GetAdj() ),
    nTopDist( rCpy.GetTopDist() ),
    nBottomDist( rCpy.GetBottomDist() )
{
}

/*************************************************************************
|*
|*  SwPageFtnInfo::operator=
|*
|*************************************************************************/



SwPageFtnInfo &SwPageFtnInfo::operator=( const SwPageFtnInfo& rCpy )
{
    nMaxHeight  = rCpy.GetHeight();
    nLineWidth  = rCpy.nLineWidth;
    eLineStyle  = rCpy.eLineStyle;
    aLineColor  = rCpy.aLineColor;
    aWidth      = rCpy.GetWidth();
    eAdj        = rCpy.GetAdj();
    nTopDist    = rCpy.GetTopDist();
    nBottomDist = rCpy.GetBottomDist();
    return *this;
}
/*************************************************************************
|*
|*  SwPageFtnInfo::operator==
|*
|*************************************************************************/



sal_Bool SwPageFtnInfo::operator==( const SwPageFtnInfo& rCmp ) const
{
    return ( nMaxHeight == rCmp.GetHeight() &&
             nLineWidth == rCmp.nLineWidth &&
             eLineStyle == rCmp.eLineStyle &&
             aLineColor == rCmp.aLineColor &&
             aWidth     == rCmp.GetWidth() &&
             eAdj       == rCmp.GetAdj() &&
             nTopDist   == rCmp.GetTopDist() &&
             nBottomDist== rCmp.GetBottomDist() );
}

SwPageDescExt::SwPageDescExt(const SwPageDesc & rPageDesc, SwDoc * _pDoc)
    : aPageDesc(rPageDesc), pDoc(_pDoc)
{
    SetPageDesc(rPageDesc);
}

SwPageDescExt::SwPageDescExt(const SwPageDescExt & rSrc)
    : aPageDesc(rSrc.aPageDesc), pDoc(rSrc.pDoc)
{
    SetPageDesc(rSrc.aPageDesc);
}

SwPageDescExt::~SwPageDescExt()
{
}

OUString SwPageDescExt::GetName() const
{
    return aPageDesc.GetName();
}

void SwPageDescExt::SetPageDesc(const SwPageDesc & _aPageDesc)
{
    aPageDesc = _aPageDesc;

    if (aPageDesc.GetFollow())
        sFollow = aPageDesc.GetFollow()->GetName();
}

SwPageDescExt & SwPageDescExt::operator = (const SwPageDesc & rSrc)
{
    SetPageDesc(rSrc);

    return *this;
}

SwPageDescExt & SwPageDescExt::operator = (const SwPageDescExt & rSrc)
{
    SetPageDesc(rSrc.aPageDesc);

    return *this;
}

SwPageDescExt::operator SwPageDesc() const
{
    SwPageDesc aResult(aPageDesc);

    SwPageDesc * pPageDesc = pDoc->GetPageDesc(sFollow);

    if ( 0 != pPageDesc )
        aResult.SetFollow(pPageDesc);

    return aResult;
}

SwPageDescs::~SwPageDescs()
{
    for(const_iterator it = begin(); it != end(); ++it)
        delete *it;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
