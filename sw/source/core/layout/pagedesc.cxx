/*************************************************************************
 *
 *  $RCSfile: pagedesc.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include <hintids.hxx>

#ifndef _SVX_PBINITEM_HXX //autogen
#include <svx/pbinitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _PAGEFRM_HXX //autogen
#include <pagefrm.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>   // SwTxtFmtColl
#endif
#ifndef _NODE_HXX //autogen
#include <node.hxx>
#endif
#ifndef _SWTABLE_HXX //autogen
#include <swtable.hxx>
#endif
#ifndef _FRMTOOL_HXX //autogen
#include <frmtool.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>          // fuer GetAttrPool
#endif

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
    aDepend( this, 0 ),
    nRegHeight( 0 ),
    nRegAscent( 0 ),
    bLandscape( FALSE ),
    eUse( (UseOnPage)(PD_ALL | PD_HEADERSHARE | PD_FOOTERSHARE) ),
    aMaster( pDc->GetAttrPool(), rName, pFmt ),
    aLeft( pDc->GetAttrPool(), rName, pFmt ),
    pFollow( this ),
    aFtnInfo()
{
}



SwPageDesc::SwPageDesc( const SwPageDesc &rCpy ) :
    SwModify( 0 ),
    aDepend( this, (SwModify*)rCpy.aDepend.GetRegisteredIn() ),
    nRegHeight( rCpy.GetRegHeight() ),
    nRegAscent( rCpy.GetRegAscent() ),
    aDescName( rCpy.GetName() ),
    bLandscape( rCpy.GetLandscape() ),
    aNumType( rCpy.GetNumType() ),
    eUse( rCpy.ReadUseOn() ),
    aMaster( rCpy.GetMaster() ),
    aLeft( rCpy.GetLeft() ),
    pFollow( rCpy.pFollow ),
    aFtnInfo( rCpy.GetFtnInfo() )
{
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
    SvxLRSpaceItem aLR;
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
    aLeft.SetAttr( aSet );
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
BOOL SwPageDesc::GetInfo( SfxPoolItem & rInfo ) const
{
//    if( RES_AUTOFMT_DOCNODE == rInfo.Which() )
//    {
        // dann weiter zum Format
        if( !aMaster.GetInfo( rInfo ) )
            return FALSE;       // gefunden
        return aLeft.GetInfo( rInfo );
//    }
//    return TRUE;        // weiter suchen
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
    nRegHeight = 0;
    {
        SwClientIter aIter( GetMaster() );
        for( SwClient* pLast = aIter.First(TYPE(SwFrm)); pLast;
                pLast = aIter.Next() )
        {
            if( ((SwFrm*)pLast)->IsPageFrm() )
                ((SwPageFrm*)pLast)->PrepareRegisterChg();
        }
    }
    {
        SwClientIter aIter( GetLeft() );
        for( SwClient* pLast = aIter.First(TYPE(SwFrm)); pLast;
                pLast = aIter.Next() )
        {
            if( ((SwFrm*)pLast)->IsPageFrm() )
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


void SwPageDesc::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
{
    const USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    SwModify::Modify( pOld, pNew );

    if( RES_ATTRSET_CHG == nWhich || RES_FMT_CHG == nWhich ||
        ( nWhich >= RES_CHRATR_BEGIN && nWhich < RES_CHRATR_END ) ||
         nWhich == RES_PARATR_LINESPACING )
        RegisterChange();
}

const SwFrm* lcl_GetFrmOfNode( const SwNode& rNd )
{
    SwModify* pMod;
    USHORT nFrmType;

    if( rNd.IsCntntNode() )
    {
        pMod = &(SwCntntNode&)rNd;
        nFrmType = FRM_CNTNT;
    }
    else if( rNd.IsTableNode() )
    {
        pMod = ((SwTableNode&)rNd).GetTable().GetFrmFmt();
        nFrmType = FRM_TAB;
    }
    else
        pMod = 0;

    Point aNullPt;
    return pMod ? ::GetFrmOfModify( *pMod, nFrmType, &aNullPt, 0, FALSE )
                : 0;
}

const SwFrmFmt* SwPageDesc::GetPageFmtOfNode( const SwNode& rNd,
                                              BOOL bCheckForThisPgDc ) const
{
    // welches PageDescFormat ist fuer diesen Node gueltig?
    const SwFrmFmt* pRet;
    const SwFrm* pChkFrm = ::lcl_GetFrmOfNode( rNd );

    if( pChkFrm && 0 != ( pChkFrm = pChkFrm->FindPageFrm() ))
    {
        const SwPageDesc* pPd = bCheckForThisPgDc ? this :
                                ((SwPageFrm*)pChkFrm)->GetPageDesc();
        pRet = &pPd->GetMaster();
        ASSERT( ((SwPageFrm*)pChkFrm)->GetPageDesc() == pPd,
                "Falcher Node fuers erkennen des Seitenformats" );
        // an welchem Format haengt diese Seite?
        if( pRet != pChkFrm->GetRegisteredIn() )
        {
            pRet = &pPd->GetLeft();
            ASSERT( pRet == pChkFrm->GetRegisteredIn(),
                    "Falcher Node fuers erkennen des Seitenformats" );
        }
    }
    else
        pRet = &GetMaster();
    return pRet;
}

BOOL SwPageDesc::IsFollowNextPageOfNode( const SwNode& rNd ) const
{
    BOOL bRet = FALSE;
    if( GetFollow() && this != GetFollow() )
    {
        const SwFrm* pChkFrm = ::lcl_GetFrmOfNode( rNd );
        if( pChkFrm && 0 != ( pChkFrm = pChkFrm->FindPageFrm() ) &&
            pChkFrm->IsPageFrm() &&
            ( !pChkFrm->GetNext() || GetFollow() ==
                        ((SwPageFrm*)pChkFrm->GetNext())->GetPageDesc() ))
            // die Seite gefunden, auf die der Follow verweist
            bRet = TRUE;
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
    eAdj( FTNADJ_LEFT ),
    nTopDist( 57 ),         //1mm
    nBottomDist( 57 )
{
//  aPen.SetWidth( 10 );
}



SwPageFtnInfo::SwPageFtnInfo( const SwPageFtnInfo &rCpy ) :
    nMaxHeight( rCpy.GetHeight() ),
//  aPen( rCpy.GetPen() ),
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
//  aPen        = rCpy.GetPen();
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



BOOL SwPageFtnInfo::operator==( const SwPageFtnInfo& rCmp ) const
{
//  const Pen aTmp( rCmp.GetPen() );
    return ( nMaxHeight == rCmp.GetHeight() &&
//           aPen       == aTmp &&
             nLineWidth == rCmp.nLineWidth &&
             aLineColor == rCmp.aLineColor &&
             aWidth     == rCmp.GetWidth() &&
             eAdj       == rCmp.GetAdj() &&
             nTopDist   == rCmp.GetTopDist() &&
             nBottomDist== rCmp.GetBottomDist() );
}




