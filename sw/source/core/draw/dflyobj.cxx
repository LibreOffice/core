/*************************************************************************
 *
 *  $RCSfile: dflyobj.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2000-11-25 18:59:54 $
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

#include "hintids.hxx"

#ifndef _XPOLY_HXX //autogen
#include <svx/xpoly.hxx>
#endif
#ifndef _SVDTRANS_HXX
#include <svx/svdtrans.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <svx/opaqitem.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif


#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
#endif
#include "frmsh.hxx"
#include "viewsh.hxx"
#include "viewimp.hxx"
#include "cntfrm.hxx"
#include "frmatr.hxx"
#include "doc.hxx"
#include "dview.hxx"
#include "dflyobj.hxx"
#include "flyfrm.hxx"
#include "frmfmt.hxx"
#include "viewopt.hxx"
#include "frmtool.hxx"
#include "flyfrms.hxx"
#include "ndnotxt.hxx"
#include "grfatr.hxx"
#include "pagefrm.hxx"

static FASTBOOL bInResize = FALSE;

TYPEINIT1( SwFlyDrawObj, SdrObject )
TYPEINIT1( SwVirtFlyDrawObj, SdrVirtObj )

/*************************************************************************
|*
|*  SwFlyDrawObj::Ctor
|*
|*  Ersterstellung      MA 18. Apr. 95
|*  Letzte Aenderung    MA 28. May. 96
|*
*************************************************************************/
SwFlyDrawObj::SwFlyDrawObj()
{
    bNotPersistent = TRUE;
    mpLocalItemSet = NULL;
}

SwFlyDrawObj::~SwFlyDrawObj()
{
    if(mpLocalItemSet)
        delete mpLocalItemSet;
}

SfxItemSet* SwFlyDrawObj::CreateNewItemSet(SfxItemPool& rPool)
{
    return new SfxItemSet(rPool);
}

const SfxItemSet& SwFlyDrawObj::GetItemSet() const
{
    if(!mpLocalItemSet)
    {
        ((SwFlyDrawObj*)this)->mpLocalItemSet =
        ((SwFlyDrawObj*)this)->CreateNewItemSet((SfxItemPool&)(*GetItemPool()));
        DBG_ASSERT(mpLocalItemSet, "Could not create an SfxItemSet(!)");
    }

    return *mpLocalItemSet;
}

/*************************************************************************
|*
|*  SwFlyDrawObj::Paint()
|*
|*  Ersterstellung      MA 08. Dec. 94
|*  Letzte Aenderung    MA 20. May. 95
|*
*************************************************************************/

FASTBOOL __EXPORT SwFlyDrawObj::Paint(ExtOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec) const
{
    return TRUE;
}

/*************************************************************************
|*
|*  SwFlyDrawObj::Factory-Methoden
|*
|*  Ersterstellung      MA 23. Feb. 95
|*  Letzte Aenderung    MA 23. Feb. 95
|*
*************************************************************************/

UINT32 __EXPORT SwFlyDrawObj::GetObjInventor() const
{
    return SWGInventor;
}


UINT16 __EXPORT SwFlyDrawObj::GetObjIdentifier()    const
{
    return SwFlyDrawObjIdentifier;
}


UINT16 __EXPORT SwFlyDrawObj::GetObjVersion() const
{
    return SwDrawFirst;
}

/*************************************************************************
|*
|*  SwVirtFlyDrawObj::CToren, Dtor
|*
|*  Ersterstellung      MA 08. Dec. 94
|*  Letzte Aenderung    MA 28. May. 96
|*
*************************************************************************/

SwVirtFlyDrawObj::SwVirtFlyDrawObj(SdrObject& rNew, SwFlyFrm* pFly) :
    SdrVirtObj( rNew ),
    pFlyFrm( pFly )
{
    bNotPersistent = bNeedColorRestore = bWriterFlyFrame = TRUE;
    const SvxProtectItem &rP = pFlyFrm->GetFmt()->GetProtect();
    bMovProt = rP.IsPosProtected();
    bSizProt = rP.IsSizeProtected();
}


__EXPORT SwVirtFlyDrawObj::~SwVirtFlyDrawObj()
{
    if ( GetPage() )    //Der SdrPage die Verantwortung entziehen.
        GetPage()->RemoveObject( GetOrdNum() );
}

/*************************************************************************
|*
|*  SwVirtFlyDrawObj::GetFmt()
|*
|*  Ersterstellung      MA 08. Dec. 94
|*  Letzte Aenderung    MA 08. Dec. 94
|*
*************************************************************************/

const SwFrmFmt *SwVirtFlyDrawObj::GetFmt() const
{
    return GetFlyFrm()->GetFmt();
}


SwFrmFmt *SwVirtFlyDrawObj::GetFmt()
{
    return GetFlyFrm()->GetFmt();
}

/*************************************************************************
|*
|*  SwVirtFlyDrawObj::Paint()
|*
|*  Ersterstellung      MA 20. Dec. 94
|*  Letzte Aenderung    MA 18. Dec. 95
|*
*************************************************************************/

FASTBOOL __EXPORT SwVirtFlyDrawObj::Paint(ExtOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec) const
{
    if ( !pFlyFrm->IsFlyInCntFrm() ) //FlyInCnt werden von den TxtPortions gepaintet.
    {
        //Rect auf den Fly begrenzen.
        SwRect aRect( rInfoRec.aDirtyRect );
        if ( rInfoRec.aDirtyRect.IsEmpty() )
            aRect = GetFlyFrm()->Frm();
        pFlyFrm->Paint( aRect );
    }
    return TRUE;
}

/*************************************************************************
|*
|*  SwVirtFlyDrawObj::CheckHit()
|*  Beschreibung        Das Teil ist genau dann getroffen wenn
|*                      1. der Point im Rand des Frm liegt.
|*                      2. der Point im heissen Bereich liegt.
|*                      3. der Point in der Flaeche liegt und es sich um
|*                         einen Rahmen mit NoTxtFrm handelt und dieser
|*                         keine URL traegt.
|*                      3a nicht aber wenn ueber dem Fly noch ein Fly liegt,
|*                         und der Point in dessen Flaeche nicht steht.
|*                      4. der Point in der Flaeche liegt und der Rahmen
|*                         selektiert ist.
|*  Ersterstellung      MA 08. Dec. 94
|*  Letzte Aenderung    JP 25.03.96
|*
*************************************************************************/

SdrObject* __EXPORT SwVirtFlyDrawObj::CheckHit( const Point& rPnt, USHORT nTol,
                                    const SetOfByte* pVisiLayer) const
{
    Rectangle aHitRect( pFlyFrm->Frm().Pos(), pFlyFrm->Frm().SSize() );
    if ( nTol )
    {
        Rectangle aExclude( aHitRect );
        aHitRect.Top()    -= nTol;
        aHitRect.Bottom() += nTol;
        aHitRect.Left()   -= nTol;
        aHitRect.Right()  += nTol;
        if( aHitRect.IsInside( rPnt ) )
        {
//          const SwFmtURL &rURL = pFlyFrm->GetFmt()->GetURL();
            if( pFlyFrm->Lower() && pFlyFrm->Lower()->IsNoTxtFrm()
                /*
                JP 07.08.96: nach Umstellung von JOE zur 330 darf das nicht
                            mehr sein!
                && !rURL.GetURL().Len() && !rURL.GetMap()
                */ )
            {
                //Vor dem Return noch 3a (siehe oben) pruefen.
                SdrPage *pPg = GetPage();
                for ( UINT32 i = GetOrdNumDirect()+1; i < pPg->GetObjCount(); ++i )
                {
                    SdrObject *pObj = pPg->GetObj( i );
                    if ( pObj->IsWriterFlyFrame() &&
                         ((SwVirtFlyDrawObj*)pObj)->GetBoundRect().IsInside( rPnt ) )
                        return 0;
                }
                return (SdrObject*)this;
            }
            else
            {
                ViewShell *pShell = pFlyFrm->GetShell();

                //4. Getroffen wenn das Objekt selektiert ist.
                if ( pShell )
                {
                    const SdrMarkList &rMrkList = pShell->
                                            Imp()->GetDrawView()->GetMarkList();
                    for ( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
                        if ( long(this) == long(rMrkList.GetMark(i)->GetObj()) )
                            return (SdrObject*)this;
                }

                const Rectangle aPrtRect( pFlyFrm->Frm().Pos() + pFlyFrm->Prt().Pos(),
                                          pFlyFrm->Prt().SSize() );
                aExclude.Top()    += Max( long(nTol), aPrtRect.Top()   - aHitRect.Top() );
                aExclude.Bottom() -= Max( long(nTol), aHitRect.Bottom()- aPrtRect.Bottom());
                aExclude.Left()   += Max( long(nTol), aPrtRect.Left()  - aHitRect.Left() );
                aExclude.Right()  -= Max( long(nTol), aHitRect.Right() - aPrtRect.Right() );
                return aExclude.IsInside( rPnt ) ? 0 : (SdrObject*)this;
            }
        }
    }
    else
        return aHitRect.IsInside( rPnt ) ? (SdrObject*)this : 0;
    return 0;
}

/*************************************************************************
|*
|*  SwVirtFlyDrawObj::TakeObjInfo()
|*
|*  Ersterstellung      MA 03. May. 95
|*  Letzte Aenderung    MA 03. May. 95
|*
*************************************************************************/

void __EXPORT SwVirtFlyDrawObj::TakeObjInfo( SdrObjTransformInfoRec& rInfo ) const
{
    rInfo.bSelectAllowed     = rInfo.bMoveAllowed =
    rInfo.bResizeFreeAllowed = rInfo.bResizePropAllowed = TRUE;

    rInfo.bRotateFreeAllowed = rInfo.bRotate90Allowed =
    rInfo.bMirrorFreeAllowed = rInfo.bMirror45Allowed =
    rInfo.bMirror90Allowed   = rInfo.bShearAllowed    =
    rInfo.bCanConvToPath     = rInfo.bCanConvToPoly   =
    rInfo.bCanConvToPathLineToArea = rInfo.bCanConvToPolyLineToArea = FALSE;
}


/*************************************************************************
|*
|*  SwVirtFlyDrawObj::Groessenermittlung
|*
|*  Ersterstellung      MA 12. Jan. 95
|*  Letzte Aenderung    MA 10. Nov. 95
|*
*************************************************************************/

void SwVirtFlyDrawObj::SetRect() const
{
    if ( GetFlyFrm()->Frm().HasArea() )
        ((SwVirtFlyDrawObj*)this)->aOutRect = GetFlyFrm()->Frm().SVRect();
    else
        ((SwVirtFlyDrawObj*)this)->aOutRect = Rectangle();
}


const Rectangle& __EXPORT SwVirtFlyDrawObj::GetBoundRect() const
{
    SetRect();
    return aOutRect;
}


void __EXPORT SwVirtFlyDrawObj::RecalcBoundRect()
{
    SetRect();
}


void __EXPORT SwVirtFlyDrawObj::RecalcSnapRect()
{
    SetRect();
}


const Rectangle& __EXPORT SwVirtFlyDrawObj::GetSnapRect()  const
{
    SetRect();
    return aOutRect;
}


void __EXPORT SwVirtFlyDrawObj::SetSnapRect(const Rectangle& rRect)
{
    Rectangle aTmp( aOutRect );
    SetRect();
    SetChanged();
    SendRepaintBroadcast();
    if (pUserCall!=NULL)
        pUserCall->Changed(*this, SDRUSERCALL_RESIZE, aTmp);
}


void __EXPORT SwVirtFlyDrawObj::NbcSetSnapRect(const Rectangle& rRect)
{
    SetRect();
}


const Rectangle& __EXPORT SwVirtFlyDrawObj::GetLogicRect() const
{
    SetRect();
    return aOutRect;
}


void __EXPORT SwVirtFlyDrawObj::SetLogicRect(const Rectangle& rRect)
{
    Rectangle aTmp( aOutRect );
    SetRect();
    SetChanged();
    SendRepaintBroadcast();
    if (pUserCall!=NULL)
        pUserCall->Changed(*this, SDRUSERCALL_RESIZE, aTmp);
}


void __EXPORT SwVirtFlyDrawObj::NbcSetLogicRect(const Rectangle& rRect)
{
    SetRect();
}


void __EXPORT SwVirtFlyDrawObj::TakeXorPoly(XPolyPolygon& rPoly, FASTBOOL ) const
{
    rPoly = XPolyPolygon( XPolygon( GetFlyFrm()->Frm().SVRect() ) );
}

/*************************************************************************
|*
|*  SwVirtFlyDrawObj::Move() und Resize()
|*
|*  Ersterstellung      MA 12. Jan. 95
|*  Letzte Aenderung    MA 26. Jul. 96
|*
*************************************************************************/

void __EXPORT SwVirtFlyDrawObj::NbcMove(const Size& rSiz)
{
    MoveRect( aOutRect, rSiz );

    const Point aOldPos( GetFlyFrm()->Frm().Pos() );
    const Point aNewPos( aOutRect.TopLeft() );
    const SwRect aFlyRect( aOutRect );

    ASSERT( aOldPos != aNewPos, "PosUnchanged." );

    //Wenn der Fly eine automatische Ausrichtung hat (rechts oder oben),
    //so soll die Automatik erhalten bleiben
    SwFrmFmt *pFmt = GetFlyFrm()->GetFmt();
    const SwHoriOrient eHori = pFmt->GetHoriOrient().GetHoriOrient();
    const SwVertOrient eVert = pFmt->GetVertOrient().GetVertOrient();
    const SwRelationOrient eRelHori = pFmt->GetHoriOrient().GetRelationOrient();
    const SwRelationOrient eRelVert = pFmt->GetVertOrient().GetRelationOrient();
    //Bei Absatzgebundenen Flys muss ausgehend von der neuen Position ein
    //neuer Anker gesetzt werden. Anker und neue RelPos werden vom Fly selbst
    //berechnet und gesetzt.
    if( GetFlyFrm()->IsFlyAtCntFrm() )
        ((SwFlyAtCntFrm*)GetFlyFrm())->SetAbsPos( aNewPos );
    else
    {
        const SwFrmFmt *pFmt = GetFmt();
        const SwFmtVertOrient &rVert = pFmt->GetVertOrient();
        const SwFmtHoriOrient &rHori = pFmt->GetHoriOrient();
        long lXDiff = aNewPos.X() - aOldPos.X();
        if( rHori.IsPosToggle() && HORI_NONE == eHori &&
            !(GetFlyFrm()->FindPageFrm()->GetVirtPageNum() % 2) )
            lXDiff = -lXDiff;
        const long lYDiff = aNewPos.Y() - aOldPos.Y();
        const Point aTmp( rHori.GetPos() + lXDiff,
                          rVert.GetPos() + lYDiff );
        GetFlyFrm()->ChgRelPos( aTmp );
    }

    SwAttrSet aSet( pFmt->GetDoc()->GetAttrPool(),
                                            RES_VERT_ORIENT, RES_HORI_ORIENT );
    SwFmtHoriOrient aHori( pFmt->GetHoriOrient() );
    SwFmtVertOrient aVert( pFmt->GetVertOrient() );
    FASTBOOL bPut = FALSE;

    if( !GetFlyFrm()->IsFlyLayFrm() &&
        ::GetHtmlMode(pFmt->GetDoc()->GetDocShell()) )
    {
        //Im HTML-Modus sind nur automatische Ausrichtungen erlaubt.
        //Einzig einen Snap auf Links/Rechts bzw. Linker-/Rechter-Rand koennen
        //wir versuchen.
        SwFrm *pAnch = GetFlyFrm()->GetAnchor();
        BOOL bNextLine = FALSE;

        if( !GetFlyFrm()->IsAutoPos() || REL_PG_FRAME != aHori.GetRelationOrient() )
        {
            if( REL_CHAR == eRelHori )
            {
                aHori.SetHoriOrient( HORI_LEFT );
                aHori.SetRelationOrient( REL_CHAR );
            }
            else
            {
                bNextLine = TRUE;
                //Horizontale Ausrichtung:
                const FASTBOOL bLeftFrm =
                    aFlyRect.Left() < pAnch->Frm().Left() + pAnch->Prt().Left(),
                    bLeftPrt = aFlyRect.Left() + aFlyRect.Width() <
                               pAnch->Frm().Left() + pAnch->Prt().Width()/2;
                if ( bLeftFrm || bLeftPrt )
                {
                    aHori.SetHoriOrient( HORI_LEFT );
                    aHori.SetRelationOrient( bLeftFrm ? FRAME : PRTAREA );
                }
                else
                {
                    const FASTBOOL bRightFrm = aFlyRect.Left() >
                                       pAnch->Frm().Left() + pAnch->Prt().Width();
                    aHori.SetHoriOrient( HORI_RIGHT );
                    aHori.SetRelationOrient( bRightFrm ? FRAME : PRTAREA );
                }
            }
            aSet.Put( aHori );
        }
        //Vertikale Ausrichtung bleibt grundsaetzlich schlicht erhalten,
        //nur bei nicht automatischer Ausrichtung wird umgeschaltet.
        BOOL bRelChar = REL_CHAR == eRelVert;
        aVert.SetVertOrient( eVert != VERT_NONE ? eVert :
                GetFlyFrm()->IsFlyInCntFrm() ? VERT_CHAR_CENTER :
                bRelChar && bNextLine ? VERT_CHAR_TOP : VERT_TOP );
        if( bRelChar )
            aVert.SetRelationOrient( REL_CHAR );
        else
            aVert.SetRelationOrient( PRTAREA );
        aSet.Put( aVert );
        bPut = TRUE;
    }

    //Automatische Ausrichtungen wollen wir moeglichst nicht verlieren.
    if ( !bPut && bInResize )
    {
        if ( HORI_NONE != eHori )
        {
            aHori.SetHoriOrient( eHori );
            aHori.SetRelationOrient( eRelHori );
            aSet.Put( aHori );
            bPut = TRUE;
        }
        if ( VERT_NONE != eVert )
        {
            aVert.SetVertOrient( eVert );
            aVert.SetRelationOrient( eRelVert );
            aSet.Put( aVert );
            bPut = TRUE;
        }
    }
    if ( bPut )
        pFmt->SetAttr( aSet );
}


void __EXPORT SwVirtFlyDrawObj::NbcResize(const Point& rRef,
            const Fraction& xFact, const Fraction& yFact)
{
    ResizeRect( aOutRect, rRef, xFact, yFact );

    const Point aNewPos( aOutRect.TopLeft() );

    Size aSz( aOutRect.Right() - aOutRect.Left() + 1,
              aOutRect.Bottom()- aOutRect.Top()  + 1 );
    if( aSz != GetFlyFrm()->Frm().SSize() )
    {
        //Die Breite darf bei Spalten nicht zu schmal werden
        if ( GetFlyFrm()->Lower() && GetFlyFrm()->Lower()->IsColumnFrm() )
        {
            SwBorderAttrAccess aAccess( SwFrm::GetCache(), GetFlyFrm() );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            long nMin = rAttrs.CalcLeftLine()+rAttrs.CalcRightLine();
            const SwFmtCol& rCol = rAttrs.GetAttrSet().GetCol();
            if ( rCol.GetColumns().Count() > 1 )
            {
                for ( USHORT i = 0; i < rCol.GetColumns().Count(); ++i )
                {
                    nMin += rCol.GetColumns()[i]->GetLeft() +
                            rCol.GetColumns()[i]->GetRight() +
                            MINFLY;
                }
                nMin -= MINFLY;
            }
            aSz.Width() = Max( aSz.Width(), nMin );
        }

        SwFrmFmt *pFmt = GetFmt();
        const SwFmtFrmSize aOldFrmSz( pFmt->GetFrmSize() );
        GetFlyFrm()->ChgSize( aSz );
        SwFmtFrmSize aFrmSz( pFmt->GetFrmSize() );
        if ( aFrmSz.GetWidthPercent() || aFrmSz.GetHeightPercent() )
        {
            long nRelWidth, nRelHeight;
            const SwFrm *pRel = GetFlyFrm()->IsFlyLayFrm() ?
                                GetFlyFrm()->GetAnchor() :
                                GetFlyFrm()->GetAnchor()->GetUpper();
            const ViewShell *pSh = GetFlyFrm()->GetShell();
            if ( pSh && pRel->IsBodyFrm() && pFmt->GetDoc()->IsBrowseMode() &&
                 pSh->VisArea().HasArea() )
            {
                nRelWidth  = pSh->VisArea().Width();
                nRelHeight = pSh->VisArea().Height();
                const Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
                nRelWidth  -= 2*aBorder.Width();
                nRelHeight -= 2*aBorder.Height();
            }
            else
            {
                nRelWidth  = pRel->Prt().Width();
                nRelHeight = pRel->Prt().Height();
            }
            if ( aFrmSz.GetWidthPercent() && aFrmSz.GetWidthPercent() != 0xFF &&
                 aOldFrmSz.GetWidth() != aFrmSz.GetWidth() )
                aFrmSz.SetWidthPercent( BYTE(aSz.Width() * 100L / nRelWidth + 0.5) );
            if ( aFrmSz.GetHeightPercent() && aFrmSz.GetHeightPercent() != 0xFF &&
                 aOldFrmSz.GetHeight() != aFrmSz.GetHeight() )
                aFrmSz.SetHeightPercent( BYTE(aSz.Height() * 100L / nRelHeight + 0.5) );
            pFmt->GetDoc()->SetAttr( aFrmSz, *pFmt );
        }
    }

    //Position kann auch veraendert sein!
    const Point aOldPos( GetFlyFrm()->Frm().Pos() );
    if ( aNewPos != aOldPos )
    {
        //Kann sich durch das ChgSize veraendert haben!
        if ( aOutRect.TopLeft() != aNewPos )
            aOutRect.SetPos( aNewPos );
        bInResize = TRUE;
        NbcMove( Size( 0, 0 ) );
        bInResize = FALSE;
    }
}


void __EXPORT SwVirtFlyDrawObj::Move(const Size& rSiz)
{
    NbcMove( rSiz );
    SetChanged();
    GetFmt()->GetDoc()->SetNoDrawUndoObj( TRUE );
}


void __EXPORT SwVirtFlyDrawObj::Resize(const Point& rRef,
                    const Fraction& xFact, const Fraction& yFact)
{
    NbcResize( rRef, xFact, yFact );
    SetChanged();
    GetFmt()->GetDoc()->SetNoDrawUndoObj( TRUE );
}


Pointer  __EXPORT SwVirtFlyDrawObj::GetMacroPointer(
    const SdrObjMacroHitRec& rRec) const
{
    return Pointer( POINTER_REFHAND );
}


FASTBOOL __EXPORT SwVirtFlyDrawObj::HasMacro() const
{
    const SwFmtURL &rURL = pFlyFrm->GetFmt()->GetURL();
    return rURL.GetMap() || rURL.GetURL().Len();
}


SdrObject* SwVirtFlyDrawObj::CheckMacroHit( const SdrObjMacroHitRec& rRec ) const
{
    const SwFmtURL &rURL = pFlyFrm->GetFmt()->GetURL();
    if( rURL.GetMap() || rURL.GetURL().Len() )
    {
        SwRect aRect;
        if ( pFlyFrm->Lower() && pFlyFrm->Lower()->IsNoTxtFrm() )
        {
            aRect = pFlyFrm->Prt();
            aRect += pFlyFrm->Frm().Pos();
        }
        else
            aRect = pFlyFrm->Frm();

        if( aRect.IsInside( rRec.aPos ) )
        {
            SwRect aActRect( aRect );
            Size aActSz( aRect.SSize() );
            aRect.Pos().X() += rRec.nTol;
            aRect.Pos().Y() += rRec.nTol;
            aRect.SSize().Height()-= 2 * rRec.nTol;
            aRect.SSize().Width() -= 2 * rRec.nTol;

            if( aRect.IsInside( rRec.aPos ) )
            {
                if( !rURL.GetMap() ||
                    pFlyFrm->GetFmt()->GetIMapObject( rRec.aPos, pFlyFrm ))
                    return (SdrObject*)this;

                return 0;
            }
        }
    }
    return SdrObject::CheckMacroHit( rRec );
}


