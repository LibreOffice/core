/*************************************************************************
 *
 *  $RCSfile: ssfrm.cxx,v $
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

#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "doc.hxx"
#include "node.hxx"
#include "errhdl.hxx"

#include "dview.hxx"
#include "dcontact.hxx"
#include "dflyobj.hxx"
#include "flyfrm.hxx"
#include "txtfrm.hxx"       // ClearPara()

#include "frmtool.hxx"
#include "pagedesc.hxx"
#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include "hints.hxx"        //fuer SwFmtChg
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif

/*************************************************************************
|*
|*  SwFrm::~SwFrm()
|*
|*  Ersterstellung      MA 02. Mar. 94
|*  Letzte Aenderung    MA 25. Jun. 95
|*
|*************************************************************************/


SwFrm::~SwFrm()
{
    if( pDrawObjs )
    {
        for ( USHORT i = pDrawObjs->Count(); i; )
        {
            SdrObject *pObj = (*pDrawObjs)[--i];
            if ( pObj->IsWriterFlyFrame() )
                delete ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
            else if ( pObj->GetUserCall() )
                ((SwDrawContact*)pObj->GetUserCall())->DisconnectFromLayout();
        }
        if ( pDrawObjs )
            delete pDrawObjs;
    }
}

/*************************************************************************
|*
|*    SwLayoutFrm::SetFrmFmt()
|*    Ersterstellung    MA 22. Apr. 93
|*    Letzte Aenderung  MA 02. Nov. 94
|*
|*************************************************************************/


void SwLayoutFrm::SetFrmFmt( SwFrmFmt *pNew )
{
    if ( pNew != GetFmt() )
    {
        SwFmtChg aOldFmt( GetFmt() );
        pNew->Add( this );
        SwFmtChg aNewFmt( pNew );
        Modify( &aOldFmt, &aNewFmt );
    }
}

/*************************************************************************
|*
|*    SwCntntFrm::SwCntntFrm(), ~SwCntntFrm()
|*
|*    Ersterstellung    AK 15-Feb-1991
|*    Letzte Aenderung  MA 25. Apr. 95
|*
|*************************************************************************/


SwCntntFrm::SwCntntFrm( SwCntntNode * const pCntnt ) :
    SwFrm( pCntnt ),
    SwFlowFrm( (SwFrm&)*this )
{
    nType = FRM_CNTNT;
}



SwCntntFrm::~SwCntntFrm()
{
    SwCntntNode* pCNd;
    if( 0 != ( pCNd = PTR_CAST( SwCntntNode, pRegisteredIn )) &&
        !pCNd->GetDoc()->IsInDtor() )
    {
        //Bei der Root abmelden wenn ich dort noch im Turbo stehe.
        SwRootFrm *pRoot = FindRootFrm();
        if( pRoot && pRoot->GetTurbo() == this )
        {
            pRoot->DisallowTurbo();
            pRoot->ResetTurbo();
        }
    }
    if( IsTxtFrm() && ((SwTxtFrm*)this)->HasBlinkPor() )
        ((SwTxtFrm*)this)->ClearPara();
}

/*************************************************************************
|*
|*  SwLayoutFrm::~SwLayoutFrm
|*
|*  Ersterstellung      AK 28-Feb-1991
|*  Letzte Aenderung    MA 11. Jan. 95
|*
|*************************************************************************/


SwLayoutFrm::~SwLayoutFrm()
{
    SwFrm *pFrm = pLower;

    if( GetFmt() && !GetFmt()->GetDoc()->IsInDtor() )
    {
        while ( pFrm )
        {
            //Erst die Objs des Frm vernichten, denn diese koennen sich sonst nach
            //dem Remove nicht mehr bei der Seite abmelden.
            //Falls sich einer nicht abmeldet wollen wir nicht gleich
            //endlos schleifen.
            USHORT nCnt;
            while ( pFrm->GetDrawObjs() && pFrm->GetDrawObjs()->Count() )
            {
                nCnt = pFrm->GetDrawObjs()->Count();
                SdrObject *pObj = (*pFrm->GetDrawObjs())[0];
                if ( pObj->IsWriterFlyFrame() )
                    delete ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
                else if ( pObj->GetUserCall() )
                    ((SwDrawContact*)pObj->GetUserCall())->DisconnectFromLayout();

                if ( pFrm->GetDrawObjs() &&
                     nCnt == pFrm->GetDrawObjs()->Count() )
                    pFrm->GetDrawObjs()->Remove( 0 );
            }
            pFrm->Remove();
            delete pFrm;
            pFrm = pLower;
        }
        //Fly's vernichten. Der letzte loescht gleich das Array.
        USHORT nCnt;
        while ( GetDrawObjs() && GetDrawObjs()->Count() )
        {
            nCnt = GetDrawObjs()->Count();
            SdrObject *pObj = (*GetDrawObjs())[0];
            if ( pObj->IsWriterFlyFrame() )
                delete ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
            else if ( pObj->GetUserCall() )
                ((SwDrawContact*)pObj->GetUserCall())->DisconnectFromLayout();

            if ( GetDrawObjs() && nCnt == GetDrawObjs()->Count() )
                GetDrawObjs()->Remove( 0 );
        }
    }
    else
    {
        while( pFrm )
        {
            SwFrm *pNxt = pFrm->GetNext();
            delete pFrm;
            pFrm = pNxt;
        }
    }
}

/*************************************************************************
|*
|*  SwFrm::PaintArea()
|*
|*  Created     AMA 08/22/2000
|*  Last change AMA 08/23/2000
|*
|*  The paintarea is the area, in which the content of a frame is allowed
|*  to be displayed. This region could be larger than the printarea (Prt())
|*  of the upper, it includes e.g. often the margin of the page.
|*
|*************************************************************************/

const SwRect SwFrm::PaintArea() const
{
    SwRect aRect( Frm() );
    long nRight = aRect.Right();
    const SwFrm* pTmp = this;
    BOOL bLeft = TRUE;
    BOOL bRight = TRUE;
    while( pTmp )
    {
        long nTmpRight = pTmp->Frm().Right();
        ASSERT( pTmp, "PaintArea lost in time and space" );
        if( pTmp->IsPageFrm() || pTmp->IsFlyFrm() ||
            pTmp->IsCellFrm() || pTmp->IsRowFrm() || //nobody leaves a table!
            pTmp->IsRootFrm() )
        {
            if( bLeft || aRect.Left() < pTmp->Frm().Left() )
                aRect.Left( pTmp->Frm().Left() );
            if( bRight || nTmpRight < nRight )
                nRight = nTmpRight;
            if( pTmp->IsPageFrm() || pTmp->IsFlyFrm() || pTmp->IsRootFrm() )
                break;
            bLeft = FALSE;
            bRight = FALSE;
        }
        else if( pTmp->IsColumnFrm() )  // nobody enters neightbour columns
        {
            if( pTmp->GetPrev() )   // the first column has _no_
            {                       // influence to the left range
                if( bLeft || aRect.Left() < pTmp->Frm().Left() )
                    aRect.Left( pTmp->Frm().Left() );
                bLeft = FALSE;
            }
            if( pTmp->GetNext() )   // the last column has _no_
            {                       // influence to the right range
                if( bRight || nTmpRight < nRight )
                nRight = nTmpRight;
                bRight = FALSE;
            }
        }
        pTmp = pTmp->GetUpper();
    }
    aRect.Right( nRight );
    return aRect;
}

/*************************************************************************
|*
|*  SwFrm::UnionFrm()
|*
|*  Created     AMA 08/22/2000
|*  Last change AMA 08/23/2000
|*
|*  The unionframe is the framearea (Frm()) of a frame expanded by the
|*  printarea, if there's a negative margin at the left or right side.
|*
|*************************************************************************/

const SwRect SwFrm::UnionFrm( BOOL bBorder ) const
{
    SwRect aRet( Frm() );
    if( Prt().Left() < 0 )
        aRet.Left( aRet.Left() + Prt().Left() );
    if( Prt().Left() + Prt().Width() > Frm().Width() )
        aRet.Width(aRet.Width() + Prt().Left() + Prt().Width() - Frm().Width());
    if( bBorder )
    {
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        const SvxBoxItem &rBox = rAttrs.GetBox();
        if ( rBox.GetLeft() )
            aRet.Left( aRet.Left() - rBox.CalcLineSpace( BOX_LINE_LEFT ) );
        else if ( rAttrs.IsBorderDist() )
            aRet.Left( aRet.Left() - rBox.GetDistance( BOX_LINE_LEFT ) - 1 );

        if ( rBox.GetRight() )
            aRet.SSize().Width() += rBox.CalcLineSpace( BOX_LINE_RIGHT );
        else if ( rAttrs.IsBorderDist() )
            aRet.SSize().Width() += rBox.GetDistance( BOX_LINE_RIGHT )  + 1;
        if( rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE )
        {
            const SvxShadowItem &rShadow = rAttrs.GetShadow();
            aRet.Left( aRet.Left()- rShadow.CalcShadowSpace(SHADOW_LEFT) );
            aRet.SSize().Width()  += rShadow.CalcShadowSpace(SHADOW_RIGHT);
        }
    }
    return aRet;
}




