/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dflyobj.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-19 13:41:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include "hintids.hxx"

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


using namespace ::com::sun::star;


// --> OD 2004-11-22 #117958#
#ifndef _SDR_PROPERTIES_DEFAULTPROPERTIES_HXX
#include <svx/sdr/properties/defaultproperties.hxx>
#endif
// <--

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

using namespace ::com::sun::star;

static BOOL bInResize = FALSE;

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

////////////////////////////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* SwFlyDrawObj::CreateObjectSpecificProperties()
{
    // --> OD 2004-11-22 #117958# - create default properties
    return new sdr::properties::DefaultProperties(*this);
    // <--
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SwFlyDrawObj::SwFlyDrawObj()
{
}

SwFlyDrawObj::~SwFlyDrawObj()
{
}

/*************************************************************************
|*
|*  SwFlyDrawObj::Paint()
|*
|*  Ersterstellung      MA 08. Dec. 94
|*  Letzte Aenderung    MA 20. May. 95
|*
*************************************************************************/

sal_Bool SwFlyDrawObj::DoPaintObject(XOutputDevice& , const SdrPaintInfoRec& ) const
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
    //#110094#-1
    // bNotPersistent = bNeedColorRestore = bWriterFlyFrame = TRUE;
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

sal_Bool SwVirtFlyDrawObj::DoPaintObject(XOutputDevice& , const SdrPaintInfoRec& rInfoRec) const
{
    //#110094#-2
    // Moved here from SwViewImp::PaintDispatcher
    sal_Bool bDrawObject(sal_True);

    if(!SwFlyFrm::IsPaint((SdrObject*)this, pFlyFrm->GetShell()))
    {
        bDrawObject = sal_False;
    }

    if(bDrawObject)
    {
        if ( !pFlyFrm->IsFlyInCntFrm() ) //FlyInCnt werden von den TxtPortions gepaintet.
        {
            //Rect auf den Fly begrenzen.
            SwRect aRect( rInfoRec.aDirtyRect );
            if ( rInfoRec.aDirtyRect.IsEmpty() )
                aRect = GetFlyFrm()->Frm();
            pFlyFrm->Paint( aRect );
        }
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
                                    const SetOfByte* ) const
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
            if ( pFlyFrm->Lower() && pFlyFrm->Lower()->IsNoTxtFrm() )
            {
                // #107513#
                // This test needs to be done outside, since also drawing layer HitTest
                // methods are called. Not all drawing objects are derived and the
                // CheckHit() overloaded. That's an conceptual error here.
                return (SdrObject*)this;
            }
            else
            {
                ViewShell *pShell = pFlyFrm->GetShell();

                //4. Getroffen wenn das Objekt selektiert ist.
                if ( pShell )
                {
                    const SdrMarkList &rMrkList = pShell->
                                            Imp()->GetDrawView()->GetMarkedObjectList();
                    for ( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
                        if ( long(this) == long(rMrkList.GetMark(i)->GetMarkedSdrObj()) )
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


const Rectangle& __EXPORT SwVirtFlyDrawObj::GetCurrentBoundRect() const
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


void __EXPORT SwVirtFlyDrawObj::SetSnapRect(const Rectangle& )
{
    Rectangle aTmp( GetLastBoundRect() );
    SetRect();
    SetChanged();
    BroadcastObjectChange();
    if (pUserCall!=NULL)
        pUserCall->Changed(*this, SDRUSERCALL_RESIZE, aTmp);
}


void __EXPORT SwVirtFlyDrawObj::NbcSetSnapRect(const Rectangle& )
{
    SetRect();
}


const Rectangle& __EXPORT SwVirtFlyDrawObj::GetLogicRect() const
{
    SetRect();
    return aOutRect;
}


void __EXPORT SwVirtFlyDrawObj::SetLogicRect(const Rectangle& )
{
    Rectangle aTmp( GetLastBoundRect() );
    SetRect();
    SetChanged();
    BroadcastObjectChange();
    if (pUserCall!=NULL)
        pUserCall->Changed(*this, SDRUSERCALL_RESIZE, aTmp);
}


void __EXPORT SwVirtFlyDrawObj::NbcSetLogicRect(const Rectangle& )
{
    SetRect();
}


::basegfx::B2DPolyPolygon SwVirtFlyDrawObj::TakeXorPoly(sal_Bool ) const
{
    const Rectangle aSourceRectangle(GetFlyFrm()->Frm().SVRect());
    const ::basegfx::B2DRange aSourceRange(aSourceRectangle.Left(), aSourceRectangle.Top(), aSourceRectangle.Right(), aSourceRectangle.Bottom());
    ::basegfx::B2DPolyPolygon aRetval;

    aRetval.append(::basegfx::tools::createPolygonFromRect(aSourceRange));

    return aRetval;
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

    //Wenn der Fly eine automatische Ausrichtung hat (rechts oder oben),
    //so soll die Automatik erhalten bleiben
    SwFrmFmt *pFmt = GetFlyFrm()->GetFmt();
    const sal_Int16 eHori = pFmt->GetHoriOrient().GetHoriOrient();
    const sal_Int16 eVert = pFmt->GetVertOrient().GetVertOrient();
    const sal_Int16 eRelHori = pFmt->GetHoriOrient().GetRelationOrient();
    const sal_Int16 eRelVert = pFmt->GetVertOrient().GetRelationOrient();
    //Bei Absatzgebundenen Flys muss ausgehend von der neuen Position ein
    //neuer Anker gesetzt werden. Anker und neue RelPos werden vom Fly selbst
    //berechnet und gesetzt.
    if( GetFlyFrm()->IsFlyAtCntFrm() )
        ((SwFlyAtCntFrm*)GetFlyFrm())->SetAbsPos( aNewPos );
    else
    {
        const SwFrmFmt *pTmpFmt = GetFmt();
        const SwFmtVertOrient &rVert = pTmpFmt->GetVertOrient();
        const SwFmtHoriOrient &rHori = pTmpFmt->GetHoriOrient();
        long lXDiff = aNewPos.X() - aOldPos.X();
        if( rHori.IsPosToggle() && text::HoriOrientation::NONE == eHori &&
            !GetFlyFrm()->FindPageFrm()->OnRightPage() )
            lXDiff = -lXDiff;

        if( GetFlyFrm()->GetAnchorFrm()->IsRightToLeft() &&
            text::HoriOrientation::NONE == eHori )
            lXDiff = -lXDiff;

        long lYDiff = aNewPos.Y() - aOldPos.Y();
        if( GetFlyFrm()->GetAnchorFrm()->IsVertical() )
        {
            lXDiff -= rVert.GetPos();
            lYDiff += rHori.GetPos();
        }
        else
        {
            lXDiff += rHori.GetPos();
            lYDiff += rVert.GetPos();
        }

        if( GetFlyFrm()->GetAnchorFrm()->IsRightToLeft() &&
            text::HoriOrientation::NONE != eHori )
            lXDiff = GetFlyFrm()->GetAnchorFrm()->Frm().Width() -
                     aFlyRect.Width() - lXDiff;

        const Point aTmp( lXDiff, lYDiff );
        GetFlyFrm()->ChgRelPos( aTmp );
    }

    SwAttrSet aSet( pFmt->GetDoc()->GetAttrPool(),
                                            RES_VERT_ORIENT, RES_HORI_ORIENT );
    SwFmtHoriOrient aHori( pFmt->GetHoriOrient() );
    SwFmtVertOrient aVert( pFmt->GetVertOrient() );
    BOOL bPut = FALSE;

    if( !GetFlyFrm()->IsFlyLayFrm() &&
        ::GetHtmlMode(pFmt->GetDoc()->GetDocShell()) )
    {
        //Im HTML-Modus sind nur automatische Ausrichtungen erlaubt.
        //Einzig einen Snap auf Links/Rechts bzw. Linker-/Rechter-Rand koennen
        //wir versuchen.
        const SwFrm* pAnch = GetFlyFrm()->GetAnchorFrm();
        BOOL bNextLine = FALSE;

        if( !GetFlyFrm()->IsAutoPos() || text::RelOrientation::PAGE_FRAME != aHori.GetRelationOrient() )
        {
            if( text::RelOrientation::CHAR == eRelHori )
            {
                aHori.SetHoriOrient( text::HoriOrientation::LEFT );
                aHori.SetRelationOrient( text::RelOrientation::CHAR );
            }
            else
            {
                bNextLine = TRUE;
                //Horizontale Ausrichtung:
                const BOOL bLeftFrm =
                    aFlyRect.Left() < pAnch->Frm().Left() + pAnch->Prt().Left(),
                    bLeftPrt = aFlyRect.Left() + aFlyRect.Width() <
                               pAnch->Frm().Left() + pAnch->Prt().Width()/2;
                if ( bLeftFrm || bLeftPrt )
                {
                    aHori.SetHoriOrient( text::HoriOrientation::LEFT );
                    aHori.SetRelationOrient( bLeftFrm ? text::RelOrientation::FRAME : text::RelOrientation::PRINT_AREA );
                }
                else
                {
                    const BOOL bRightFrm = aFlyRect.Left() >
                                       pAnch->Frm().Left() + pAnch->Prt().Width();
                    aHori.SetHoriOrient( text::HoriOrientation::RIGHT );
                    aHori.SetRelationOrient( bRightFrm ? text::RelOrientation::FRAME : text::RelOrientation::PRINT_AREA );
                }
            }
            aSet.Put( aHori );
        }
        //Vertikale Ausrichtung bleibt grundsaetzlich schlicht erhalten,
        //nur bei nicht automatischer Ausrichtung wird umgeschaltet.
        BOOL bRelChar = text::RelOrientation::CHAR == eRelVert;
        aVert.SetVertOrient( eVert != text::VertOrientation::NONE ? eVert :
                GetFlyFrm()->IsFlyInCntFrm() ? text::VertOrientation::CHAR_CENTER :
                bRelChar && bNextLine ? text::VertOrientation::CHAR_TOP : text::VertOrientation::TOP );
        if( bRelChar )
            aVert.SetRelationOrient( text::RelOrientation::CHAR );
        else
            aVert.SetRelationOrient( text::RelOrientation::PRINT_AREA );
        aSet.Put( aVert );
        bPut = TRUE;
    }

    //Automatische Ausrichtungen wollen wir moeglichst nicht verlieren.
    if ( !bPut && bInResize )
    {
        if ( text::HoriOrientation::NONE != eHori )
        {
            aHori.SetHoriOrient( eHori );
            aHori.SetRelationOrient( eRelHori );
            aSet.Put( aHori );
            bPut = TRUE;
        }
        if ( text::VertOrientation::NONE != eVert )
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

    const SwFrm* pTmpFrm = GetFlyFrm()->GetAnchorFrm();
    if( !pTmpFrm )
        pTmpFrm = GetFlyFrm();
    const bool bVertX = pTmpFrm->IsVertical();

    const sal_Bool bRTL = pTmpFrm->IsRightToLeft();

    const Point aNewPos( bVertX || bRTL ?
                         aOutRect.Right() + 1 :
                         aOutRect.Left(),
                         aOutRect.Top() );

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
                                GetFlyFrm()->GetAnchorFrm() :
                                GetFlyFrm()->GetAnchorFrm()->GetUpper();
            const ViewShell *pSh = GetFlyFrm()->GetShell();
            if ( pSh && pRel->IsBodyFrm() &&
                 pFmt->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) &&
                 pSh->VisArea().HasArea() )
            {
                nRelWidth  = pSh->GetBrowseWidth();
                nRelHeight = pSh->VisArea().Height();
                const Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
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
    const Point aOldPos( bVertX || bRTL ?
                         GetFlyFrm()->Frm().TopRight() :
                         GetFlyFrm()->Frm().Pos() );

    if ( aNewPos != aOldPos )
    {
        //Kann sich durch das ChgSize veraendert haben!
        if( bVertX || bRTL )
        {
            if( aOutRect.TopRight() != aNewPos )
            {
                SwTwips nDeltaX = aNewPos.X() - aOutRect.Right();
                SwTwips nDeltaY = aNewPos.Y() - aOutRect.Top();
                MoveRect( aOutRect, Size( nDeltaX, nDeltaY ) );
            }
        }
        else if ( aOutRect.TopLeft() != aNewPos )
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
    const SdrObjMacroHitRec& ) const
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


