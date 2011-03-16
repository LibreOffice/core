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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include <hintids.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/lrspitem.hxx>
#include "editeng/frmdiritem.hxx"
#include <fmtclds.hxx>
#include <fmtfsize.hxx>
#include <frmatr.hxx>
#include <pagefrm.hxx>
#include <pagedesc.hxx>
#include <frmfmt.hxx>
#include <fmtcol.hxx>   // SwTxtFmtColl
#include <node.hxx>
#include <swtable.hxx>
#include <frmtool.hxx>
#include <doc.hxx>          // fuer GetAttrPool
#include <poolfmt.hxx>
#include <switerator.hxx>

/*************************************************************************
|*
|*  SwPageDesc::SwPageDesc()
|*
|*  Ersterstellung      MA 25. Jan. 93
|*  Letzte Aenderung    MA 16. Feb. 94
|*
|*************************************************************************/



SwPageDesc::SwPageDesc( const String& rName, SwFrmFmt *pFmt, SwDoc *pDc ) :
    SwModify( 0 ),
    aDescName( rName ),
    aMaster( pDc->GetAttrPool(), rName, pFmt ),
    aLeft( pDc->GetAttrPool(), rName, pFmt ),
    aDepend( this, 0 ),
    pFollow( this ),
    nRegHeight( 0 ),
    nRegAscent( 0 ),
    eUse( (UseOnPage)(nsUseOnPage::PD_ALL | nsUseOnPage::PD_HEADERSHARE | nsUseOnPage::PD_FOOTERSHARE) ),
    bLandscape( sal_False ),
    aFtnInfo()
{
}

SwPageDesc::SwPageDesc( const SwPageDesc &rCpy ) :
    SwModify( 0 ),
    aDescName( rCpy.GetName() ),
    aNumType( rCpy.GetNumType() ),
    aMaster( rCpy.GetMaster() ),
    aLeft( rCpy.GetLeft() ),
    aDepend( this, (SwModify*)rCpy.aDepend.GetRegisteredIn() ),
    pFollow( rCpy.pFollow ),
    nRegHeight( rCpy.GetRegHeight() ),
    nRegAscent( rCpy.GetRegAscent() ),
    eUse( rCpy.ReadUseOn() ),
    bLandscape( rCpy.GetLandscape() ),
    aFtnInfo( rCpy.GetFtnInfo() )
{
}

SwPageDesc & SwPageDesc::operator = (const SwPageDesc & rSrc)
{
    aDescName = rSrc.aDescName;
    aNumType = rSrc.aNumType;
    aMaster = rSrc.aMaster;
    aLeft = rSrc.aLeft;

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
|*  Beschreibung        Gespiegelt werden nur die Raender.
|*      Attribute wie Umrandung und dergleichen werden 1:1 kopiert.
|*  Ersterstellung      MA 25. Jan. 93
|*  Letzte Aenderung    01. Nov. 94
|*
|*************************************************************************/



void SwPageDesc::Mirror()
{
    //Das Spiegeln findet nur beim RandAttribut statt, alle anderen Werte
    //werden schlicht uebertragen.
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

    // --> OD 2007-01-25 #i73790# - method renamed
    rFmt.ResetAllFmtAttr();
    // <--
    rFmt.SetFmtAttr( SvxFrameDirectionItem(FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR) );
}

/*************************************************************************
|*
|*                SwPageDesc::GetInfo()
|*
|*    Beschreibung      erfragt Informationen
|*    Ersterstellung    JP 31.03.94
|*    Letzte Aenderung  JP 31.03.94
|*
*************************************************************************/


    // erfrage vom Modify Informationen
sal_Bool SwPageDesc::GetInfo( SfxPoolItem & rInfo ) const
{
//    if( RES_AUTOFMT_DOCNODE == rInfo.Which() )
//    {
        // dann weiter zum Format
        if( !aMaster.GetInfo( rInfo ) )
            return sal_False;       // gefunden
        return aLeft.GetInfo( rInfo );
//    }
//    return sal_True;        // weiter suchen
}

/*************************************************************************
|*
|*                SwPageDesc::SetRegisterFmtColl()
|*
|*    Beschreibung      setzt die Vorlage fuer die Registerhaltigkeit
|*    Ersterstellung    AMA 22.07.96
|*    Letzte Aenderung  AMA 22.07.96
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
|*    Beschreibung      holt die Vorlage fuer die Registerhaltigkeit
|*    Ersterstellung    AMA 22.07.96
|*    Letzte Aenderung  AMA 22.07.96
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
|*    Beschreibung      benachrichtigt alle betroffenen PageFrames
|*    Ersterstellung    AMA 22.07.96
|*    Letzte Aenderung  AMA 22.07.96
|*
*************************************************************************/


void SwPageDesc::RegisterChange()
{
    // --> OD 2004-06-15 #117072# - During destruction of the document <SwDoc>
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
}

/*************************************************************************
|*
|*                SwPageDesc::Modify()
|*
|*    Beschreibung      reagiert insbesondere auf Aenderungen
|*                      der Vorlage fuer die Registerhaltigkeit
|*    Ersterstellung    AMA 22.07.96
|*    Letzte Aenderung  AMA 22.07.96
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
    // welches PageDescFormat ist fuer diesen Node gueltig?
    const SwFrmFmt* pRet;
    const SwFrm* pChkFrm = lcl_GetFrmOfNode( rNd );

    if( pChkFrm && 0 != ( pChkFrm = pChkFrm->FindPageFrm() ))
    {
        const SwPageDesc* pPd = bCheckForThisPgDc ? this :
                                ((SwPageFrm*)pChkFrm)->GetPageDesc();
        pRet = &pPd->GetMaster();
        ASSERT( ((SwPageFrm*)pChkFrm)->GetPageDesc() == pPd, "Wrong node for detection of page format!" );
        // an welchem Format haengt diese Seite?
        if( !pChkFrm->KnowsFormat(*pRet) )
        {
            pRet = &pPd->GetLeft();
            ASSERT( pChkFrm->KnowsFormat(*pRet), "Wrong node for detection of page format!" );
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
            // die Seite gefunden, auf die der Follow verweist
            bRet = sal_True;
    }
    return bRet;
}

/*************************************************************************
|*
|*  SwPageFtnInfo::SwPageFtnInfo()
|*
|*  Ersterstellung      MA 24. Feb. 93
|*  Letzte Aenderung    MA 24. Feb. 93
|*
|*************************************************************************/



SwPageFtnInfo::SwPageFtnInfo() :
    nMaxHeight( 0 ),
//  aPen( PEN_SOLID ),
    nLineWidth(10),
    aWidth( 25, 100 ),
    nTopDist( 57 ),         //1mm
    nBottomDist( 57 )
{
    eAdj = FRMDIR_HORI_RIGHT_TOP == GetDefaultFrameDirection(GetAppLanguage()) ?
           FTNADJ_RIGHT :
           FTNADJ_LEFT;
//  aPen.SetWidth( 10 );
}



SwPageFtnInfo::SwPageFtnInfo( const SwPageFtnInfo &rCpy ) :
    nMaxHeight( rCpy.GetHeight() ),
    nLineWidth(rCpy.nLineWidth),
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
|*  Ersterstellung      MA 24. Feb. 93
|*  Letzte Aenderung    MA 24. Feb. 93
|*
|*************************************************************************/



SwPageFtnInfo &SwPageFtnInfo::operator=( const SwPageFtnInfo& rCpy )
{
    nMaxHeight  = rCpy.GetHeight();
    nLineWidth  = rCpy.nLineWidth;
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
|*  Ersterstellung      MA 01. Mar. 93
|*  Letzte Aenderung    MA 01. Mar. 93
|*
|*************************************************************************/



sal_Bool SwPageFtnInfo::operator==( const SwPageFtnInfo& rCmp ) const
{
    return ( nMaxHeight == rCmp.GetHeight() &&
             nLineWidth == rCmp.nLineWidth &&
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

const String & SwPageDescExt::GetName() const
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
