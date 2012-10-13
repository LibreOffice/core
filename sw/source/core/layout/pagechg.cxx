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


#include <com/sun/star/embed/EmbedStates.hpp>
#include <ndole.hxx>
#include <docary.hxx>
#include <svl/itemiter.hxx>
#include <fmtfsize.hxx>
#include <fmthdft.hxx>
#include <fmtclds.hxx>
#include <fmtanchr.hxx>
#include <fmtpdsc.hxx>
#include <fmtfordr.hxx>
#include <fmtfld.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <ftninfo.hxx>
#include <tgrditem.hxx>
#include <viewopt.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <edtwin.hxx>

#include "viewimp.hxx"
#include "viewopt.hxx"
#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "flyfrm.hxx"
#include "doc.hxx"
#include "fesh.hxx"
#include "dview.hxx"
#include "dflyobj.hxx"
#include "dcontact.hxx"
#include "frmtool.hxx"
#include "fldbas.hxx"
#include "hints.hxx"
#include "swtable.hxx"

#include "ftnidx.hxx"
#include "bodyfrm.hxx"
#include "ftnfrm.hxx"
#include "tabfrm.hxx"
#include "txtfrm.hxx"
#include "layact.hxx"
#include "flyfrms.hxx"
#include "htmltbl.hxx"
#include "pagedesc.hxx"
#include "poolfmt.hxx"
#include <editeng/frmdiritem.hxx>
#include <swfntcch.hxx> // SwFontAccess
#include <sortedobjs.hxx>
#include <switerator.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;


/*************************************************************************
|*
|*  SwBodyFrm::SwBodyFrm()
|*
|*************************************************************************/
SwBodyFrm::SwBodyFrm( SwFrmFmt *pFmt, SwFrm* pSib ):
    SwLayoutFrm( pFmt, pSib )
{
    nType = FRMC_BODY;
}

/*************************************************************************
|*
|*  SwBodyFrm::Format()
|*
|*************************************************************************/
void SwBodyFrm::Format( const SwBorderAttrs * )
{
    //Formatieren des Body ist zu einfach, deshalb bekommt er ein eigenes
    //Format; Umrandungen und dergl. sind hier nicht zu beruecksichtigen.
    //Breite ist die der PrtArea des Uppers, Hoehe ist die Hoehe der PrtArea
    //des Uppers abzueglich der Nachbarn (Wird eigentlich eingestellt aber
    //Vorsicht ist die Mutter der Robustheit).
    //Die PrtArea ist stets so gross wie der Frm itself.

    if ( !bValidSize )
    {
        SwTwips nHeight = GetUpper()->Prt().Height();
        SwTwips nWidth = GetUpper()->Prt().Width();
        const SwFrm *pFrm = GetUpper()->Lower();
        do
        {
            if ( pFrm != this )
            {
                if( pFrm->IsVertical() )
                    nWidth -= pFrm->Frm().Width();
                else
                    nHeight -= pFrm->Frm().Height();
            }
            pFrm = pFrm->GetNext();
        } while ( pFrm );
        if ( nHeight < 0 )
            nHeight = 0;
        Frm().Height( nHeight );
        //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
        if( IsVertical() && !IsVertLR() && !IsReverse() && nWidth != Frm().Width() )
            Frm().Pos().X() += Frm().Width() - nWidth;
        Frm().Width( nWidth );
    }

    sal_Bool bNoGrid = sal_True;
    if( GetUpper()->IsPageFrm() && ((SwPageFrm*)GetUpper())->HasGrid() )
    {
        GETGRID( ((SwPageFrm*)GetUpper()) )
        if( pGrid )
        {
            bNoGrid = sal_False;
            long nSum = pGrid->GetBaseHeight() + pGrid->GetRubyHeight();
            SWRECTFN( this )
            long nSize = (Frm().*fnRect->fnGetWidth)();
            long nBorder = 0;
            if( GRID_LINES_CHARS == pGrid->GetGridType() )
            {
                //for textgrid refactor
                SwDoc *pDoc = GetFmt()->GetDoc();
                nBorder = nSize % (GETGRIDWIDTH(pGrid, pDoc));
                nSize -= nBorder;
                nBorder /= 2;
            }
            (Prt().*fnRect->fnSetPosX)( nBorder );
            (Prt().*fnRect->fnSetWidth)( nSize );

            // Height of body frame:
            nBorder = (Frm().*fnRect->fnGetHeight)();

            // Number of possible lines in area of body frame:
            long nNumberOfLines = nBorder / nSum;
            if( nNumberOfLines > pGrid->GetLines() )
                nNumberOfLines = pGrid->GetLines();

            // Space required for nNumberOfLines lines:
            nSize = nNumberOfLines * nSum;
            nBorder -= nSize;
            nBorder /= 2;

            // #i21774# Footnotes and centering the grid does not work together:
            const bool bAdjust = ((SwPageFrm*)GetUpper())->GetFmt()->GetDoc()->
                                        GetFtnIdxs().empty();

            (Prt().*fnRect->fnSetPosY)( bAdjust ? nBorder : 0 );
            (Prt().*fnRect->fnSetHeight)( nSize );
        }
    }
    if( bNoGrid )
    {
        Prt().Pos().X() = Prt().Pos().Y() = 0;
        Prt().Height( Frm().Height() );
        Prt().Width( Frm().Width() );
    }
    bValidSize = bValidPrtArea = sal_True;
}

void SwBodyFrm::Paint( const SwRect& rRect, const SwPrintData* ) const
{
#if OSL_DEBUG_LAYOUT > 1
    // Paint a red border around the SwBodyFrm in debug mode
    ViewShell *pSh = GetShell();
    OutputDevice* pOut =  pSh->GetOut();
    pOut->Push();
    pOut->SetLineColor(Color(255, 0, 0));
    pOut->SetFillColor(COL_TRANSPARENT);
    SwRect aRect = Frm();
    pOut->DrawRect(aRect.SVRect());
    pOut->Pop();
#endif
    SwLayoutFrm::Paint(rRect);
}

/*************************************************************************
|*
|*  SwPageFrm::SwPageFrm(), ~SwPageFrm()
|*
|*************************************************************************/
SwPageFrm::SwPageFrm( SwFrmFmt *pFmt, SwFrm* pSib, SwPageDesc *pPgDsc ) :
    SwFtnBossFrm( pFmt, pSib ),
    pSortedObjs( 0 ),
    pDesc( pPgDsc ),
    nPhyPageNum( 0 ),
    // OD 2004-05-17 #i28701#
    mbLayoutInProgress( false )
{
    SetDerivedVert( sal_False );
    SetDerivedR2L( sal_False );
    if( pDesc )
    {
        bHasGrid = sal_True;
        GETGRID( this )
        if( !pGrid )
            bHasGrid = sal_False;
    }
    else
        bHasGrid = sal_False;
    SetMaxFtnHeight( pPgDsc->GetFtnInfo().GetHeight() ?
                     pPgDsc->GetFtnInfo().GetHeight() : LONG_MAX ),
    nType = FRMC_PAGE;
    bInvalidLayout = bInvalidCntnt = bInvalidSpelling = bInvalidSmartTags = bInvalidAutoCmplWrds = bInvalidWordCount = sal_True;
    bInvalidFlyLayout = bInvalidFlyCntnt = bInvalidFlyInCnt = bFtnPage = bEndNotePage = sal_False;

    ViewShell *pSh = getRootFrm()->GetCurrShell();
    const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
    if ( bBrowseMode )
    {
        Frm().Height( 0 );
        long nWidth = pSh->VisArea().Width();
        if ( !nWidth )
            nWidth = 5000L;     //aendert sich sowieso
        Frm().Width ( nWidth );
    }
    else
        Frm().SSize( pFmt->GetFrmSize().GetSize() );

    //Body-Bereich erzeugen und einsetzen, aber nur wenn ich nicht gerade
    //eine Leerseite bin.
    SwDoc *pDoc = pFmt->GetDoc();
    if ( sal_False == (bEmptyPage = (pFmt == pDoc->GetEmptyPageFmt())) )
    {
        bEmptyPage = sal_False;
        Calc();                             //Damit die PrtArea stimmt.
        SwBodyFrm *pBodyFrm = new SwBodyFrm( pDoc->GetDfltFrmFmt(), this );
        pBodyFrm->ChgSize( Prt().SSize() );
        pBodyFrm->Paste( this );
        pBodyFrm->Calc();                   //Damit die Spalten korrekt
                                            //eingesetzt werden koennen.
        pBodyFrm->InvalidatePos();

        if ( bBrowseMode )
            _InvalidateSize();      //Alles nur gelogen

        //Header/Footer einsetzen, nur rufen wenn aktiv.
        if ( pFmt->GetHeader().IsActive() )
            PrepareHeader();
        if ( pFmt->GetFooter().IsActive() )
            PrepareFooter();

        const SwFmtCol &rCol = pFmt->GetCol();
        if ( rCol.GetNumCols() > 1 )
        {
            const SwFmtCol aOld; //ChgColumns() verlaesst sich darauf, dass ein
                                 //Old-Wert hereingereicht wird.
            pBodyFrm->ChgColumns( aOld, rCol );
        }
    }
}

SwPageFrm::~SwPageFrm()
{
    // Cleanup the header-footer controls in the SwEditWin
    ViewShell* pSh = getRootFrm()->GetCurrShell();
    SwWrtShell* pWrtSh = dynamic_cast< SwWrtShell* >( pSh );
    if ( pWrtSh )
    {
        SwEditWin& rEditWin = pWrtSh->GetView().GetEditWin();
        rEditWin.GetFrameControlsManager( ).RemoveControls( this );
    }

    //FlyContainer entleeren, delete der Flys uebernimmt der Anchor
    //(Basisklasse SwFrm)
    if ( pSortedObjs )
    {
        //Objekte koennen (warum auch immer) auch an Seiten verankert sein,
        //die vor Ihren Ankern stehen. Dann wuerde auf bereits freigegebenen
        //Speicher zugegriffen.
        for ( sal_uInt16 i = 0; i < pSortedObjs->Count(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*pSortedObjs)[i];
            pAnchoredObj->SetPageFrm( 0L );
        }
        delete pSortedObjs;
        pSortedObjs = 0;        //Auf 0 setzen, sonst rauchts beim Abmdelden von Flys!
    }

    //Damit der Zugriff auf zerstoerte Seiten verhindert werden kann.
    if ( !IsEmptyPage() ) //#59184# sollte fuer Leerseiten unnoetig sein.
    {
        SwDoc *pDoc = GetFmt() ? GetFmt()->GetDoc() : NULL;
        if( pDoc && !pDoc->IsInDtor() )
        {
            if ( pSh )
            {
                SwViewImp *pImp = pSh->Imp();
                pImp->SetFirstVisPageInvalid();
                if ( pImp->IsAction() )
                    pImp->GetLayAction().SetAgain();
                // OD 12.02.2003 #i9719#, #105645# - retouche area of page
                // including border and shadow area.
                const bool bRightSidebar = (SidebarPosition() == sw::sidebarwindows::SIDEBAR_RIGHT);
                SwRect aRetoucheRect;
                SwPageFrm::GetBorderAndShadowBoundRect( Frm(), pSh, aRetoucheRect, IsLeftShadowNeeded(), IsRightShadowNeeded(), bRightSidebar );
                pSh->AddPaintRect( aRetoucheRect );
            }
        }
    }
}


void SwPageFrm::CheckGrid( sal_Bool bInvalidate )
{
    sal_Bool bOld = bHasGrid;
    bHasGrid = sal_True;
    GETGRID( this )
    bHasGrid = 0 != pGrid;
    if( bInvalidate || bOld != bHasGrid )
    {
        SwLayoutFrm* pBody = FindBodyCont();
        if( pBody )
        {
            pBody->InvalidatePrt();
            SwCntntFrm* pFrm = pBody->ContainsCntnt();
            while( pBody->IsAnLower( pFrm ) )
            {
                ((SwTxtFrm*)pFrm)->Prepare( PREP_CLEAR );
                pFrm = pFrm->GetNextCntntFrm();
            }
        }
        SetCompletePaint();
    }
}


void SwPageFrm::CheckDirection( sal_Bool bVert )
{
    sal_uInt16 nDir =
            ((SvxFrameDirectionItem&)GetFmt()->GetFmtAttr( RES_FRAMEDIR )).GetValue();
    if( bVert )
    {
        if( FRMDIR_HORI_LEFT_TOP == nDir || FRMDIR_HORI_RIGHT_TOP == nDir )
        {
            //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
            bVertLR = 0;
            bVertical = 0;
        }
        else
        {
            const ViewShell *pSh = getRootFrm()->GetCurrShell();
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
            {
                //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
                bVertLR = 0;
                bVertical = 0;
            }
            else
            {
                bVertical = 1;
                //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
                if(FRMDIR_VERT_TOP_RIGHT == nDir)
                    bVertLR = 0;
                    else if(FRMDIR_VERT_TOP_LEFT==nDir)
                       bVertLR = 1;
            }
        }

        bReverse = 0;
        bInvalidVert = 0;
    }
    else
    {
        if( FRMDIR_HORI_RIGHT_TOP == nDir )
            bRightToLeft = 1;
        else
            bRightToLeft = 0;
        bInvalidR2L = 0;
    }
}

/*************************************************************************
|*
|*  SwPageFrm::PreparePage()
|*
|*  Beschreibung        Erzeugt die Spezifischen Flys zur Seite und formatiert
|*      generischen Cntnt
|*
|*************************************************************************/
static void lcl_FormatLay( SwLayoutFrm *pLay )
{
    //Alle LayoutFrms - nicht aber Tables, Flys o.ae. - formatieren.

    SwFrm *pTmp = pLay->Lower();
    //Erst die untergeordneten
    while ( pTmp )
    {
        if ( pTmp->GetType() & 0x00FF )
            ::lcl_FormatLay( (SwLayoutFrm*)pTmp );
        pTmp = pTmp->GetNext();
    }
    pLay->Calc();
}

static void lcl_MakeObjs( const SwFrmFmts &rTbl, SwPageFrm *pPage )
{
    //Anlegen bzw. registrieren von Flys und Drawobjekten.
    //Die Formate stehen in der SpzTbl (vom Dokument).
    //Flys werden angelegt, DrawObjekte werden bei der Seite angemeldet.

    for ( sal_uInt16 i = 0; i < rTbl.size(); ++i )
    {
        SdrObject *pSdrObj;
        SwFrmFmt *pFmt = rTbl[i];
        const SwFmtAnchor &rAnch = pFmt->GetAnchor();
        if ( rAnch.GetPageNum() == pPage->GetPhyPageNum() )
        {
            if( rAnch.GetCntntAnchor() )
            {
                if (FLY_AT_PAGE == rAnch.GetAnchorId())
                {
                    SwFmtAnchor aAnch( rAnch );
                    aAnch.SetAnchor( 0 );
                    pFmt->SetFmtAttr( aAnch );
                }
                else
                    continue;
            }

            //Wird ein Rahmen oder ein SdrObject beschrieben?
            sal_Bool bSdrObj = RES_DRAWFRMFMT == pFmt->Which();
            pSdrObj = 0;
            if ( bSdrObj  && 0 == (pSdrObj = pFmt->FindSdrObject()) )
            {
                OSL_FAIL( "DrawObject not found." );
                pFmt->GetDoc()->DelFrmFmt( pFmt );
                --i;
                continue;
            }
            //Das Objekt kann noch an einer anderen Seite verankert sein.
            //Z.B. beim Einfuegen einer neuen Seite aufgrund eines
            //Pagedescriptor-Wechsels. Das Objekt muss dann umgehaengt
            //werden.
            //Fuer bestimmte Faelle ist das Objekt bereits an der richtigen
            //Seite verankert. Das wird hier automatisch erledigt und braucht
            //- wenngleich performater machbar - nicht extra codiert werden.
            SwPageFrm *pPg = pPage->IsEmptyPage() ? (SwPageFrm*)pPage->GetNext() : pPage;
            if ( bSdrObj )
            {
                // OD 23.06.2003 #108784# - consider 'virtual' drawing objects
                SwDrawContact *pContact =
                            static_cast<SwDrawContact*>(::GetUserCall(pSdrObj));
                if ( pSdrObj->ISA(SwDrawVirtObj) )
                {
                    SwDrawVirtObj* pDrawVirtObj = static_cast<SwDrawVirtObj*>(pSdrObj);
                    if ( pContact )
                    {
                        pDrawVirtObj->RemoveFromWriterLayout();
                        pDrawVirtObj->RemoveFromDrawingPage();
                        pPg->AppendDrawObj( *(pContact->GetAnchoredObj( pDrawVirtObj )) );
                    }
                }
                else
                {
                    if ( pContact->GetAnchorFrm() )
                        pContact->DisconnectFromLayout( false );
                    pPg->AppendDrawObj( *(pContact->GetAnchoredObj( pSdrObj )) );
                }
            }
            else
            {
                SwIterator<SwFlyFrm,SwFmt> aIter( *pFmt );
                SwFlyFrm *pFly = aIter.First();
                if ( pFly)
                {
                    if( pFly->GetAnchorFrm() )
                        pFly->AnchorFrm()->RemoveFly( pFly );
                }
                else
                    pFly = new SwFlyLayFrm( (SwFlyFrmFmt*)pFmt, pPg, pPg );
                pPg->AppendFly( pFly );
                ::RegistFlys( pPg, pFly );
            }
        }
    }
}

void SwPageFrm::PreparePage( sal_Bool bFtn )
{
    SetFtnPage( bFtn );

    // #i82258#
    // Due to made change on OOo 2.0 code line, method <::lcl_FormatLay(..)> has
    // the side effect, that the content of page header and footer are formatted.
    // For this formatting it is needed that the anchored objects are registered
    // at the <SwPageFrm> instance.
    // Thus, first calling <::RegistFlys(..)>, then call <::lcl_FormatLay(..)>
    ::RegistFlys( this, this );

        if ( Lower() )
    {
                ::lcl_FormatLay( this );
    }

    //Flys und DrawObjekte die noch am Dokument bereitstehen.
    //Fussnotenseiten tragen keine Seitengebundenen Flys!
    //Es kann Flys und Objekte geben, die auf Leerseiten (Seitennummernmaessig)
    //stehen wollen, diese werden jedoch von den Leerseiten ignoriert;
    //sie werden von den Folgeseiten aufgenommen.
    if ( !bFtn && !IsEmptyPage() )
    {
        SwDoc *pDoc = GetFmt()->GetDoc();

        if ( GetPrev() && ((SwPageFrm*)GetPrev())->IsEmptyPage() )
            lcl_MakeObjs( *pDoc->GetSpzFrmFmts(), (SwPageFrm*)GetPrev() );
        lcl_MakeObjs( *pDoc->GetSpzFrmFmts(), this );

        //Kopf-/Fusszeilen) formatieren.
        SwLayoutFrm *pLow = (SwLayoutFrm*)Lower();
        while ( pLow )
        {
            if ( pLow->GetType() & (FRMTYPE_HEADER|FRMTYPE_FOOTER) )
            {
                SwCntntFrm *pCntnt = pLow->ContainsCntnt();
                while ( pCntnt && pLow->IsAnLower( pCntnt ) )
                {
                    pCntnt->OptCalc();  //Nicht die Vorgaenger
                    pCntnt = pCntnt->GetNextCntntFrm();
                }
            }
            pLow = (SwLayoutFrm*)pLow->GetNext();
        }
    }
}

/*************************************************************************
|*
|*  SwPageFrm::Modify()
|*
|*************************************************************************/
void SwPageFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    ViewShell *pSh = getRootFrm()->GetCurrShell();
    if ( pSh )
        pSh->SetFirstVisPageInvalid();
    sal_uInt8 nInvFlags = 0;

    if( pNew && RES_ATTRSET_CHG == pNew->Which() )
    {
        SfxItemIter aNIter( *((SwAttrSetChg*)pNew)->GetChgSet() );
        SfxItemIter aOIter( *((SwAttrSetChg*)pOld)->GetChgSet() );
        SwAttrSetChg aOldSet( *(SwAttrSetChg*)pOld );
        SwAttrSetChg aNewSet( *(SwAttrSetChg*)pNew );
        while( sal_True )
        {
            _UpdateAttr( (SfxPoolItem*)aOIter.GetCurItem(),
                         (SfxPoolItem*)aNIter.GetCurItem(), nInvFlags,
                         &aOldSet, &aNewSet );
            if( aNIter.IsAtEnd() )
                break;
            aNIter.NextItem();
            aOIter.NextItem();
        }
        if ( aOldSet.Count() || aNewSet.Count() )
            SwLayoutFrm::Modify( &aOldSet, &aNewSet );
    }
    else
        _UpdateAttr( pOld, pNew, nInvFlags );

    if ( nInvFlags != 0 )
    {
        InvalidatePage( this );
        if ( nInvFlags & 0x01 )
            _InvalidatePrt();
        if ( nInvFlags & 0x02 )
            SetCompletePaint();
        if ( nInvFlags & 0x04 && GetNext() )
            GetNext()->InvalidatePos();
        if ( nInvFlags & 0x08 )
            PrepareHeader();
        if ( nInvFlags & 0x10 )
            PrepareFooter();
        if ( nInvFlags & 0x20 )
            CheckGrid( nInvFlags & 0x40 );
    }
}

void SwPageFrm::_UpdateAttr( const SfxPoolItem *pOld, const SfxPoolItem *pNew,
                             sal_uInt8 &rInvFlags,
                             SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    sal_Bool bClear = sal_True;
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
        case RES_FMT_CHG:
        {
            //Wenn sich das FrmFmt aendert kann hier einiges passieren.
            //Abgesehen von den Grossenverhaeltnissen sind noch andere
            //Dinge betroffen.
            //1. Spaltigkeit.
            OSL_ENSURE( pOld && pNew, "FMT_CHG Missing Format." );
            const SwFmt* pOldFmt = ((SwFmtChg*)pOld)->pChangedFmt;
            const SwFmt* pNewFmt = ((SwFmtChg*)pNew)->pChangedFmt;
            OSL_ENSURE( pOldFmt && pNewFmt, "FMT_CHG Missing Format." );

            const SwFmtCol &rOldCol = pOldFmt->GetCol();
            const SwFmtCol &rNewCol = pNewFmt->GetCol();
            if( rOldCol != rNewCol )
            {
                SwLayoutFrm *pB = FindBodyCont();
                OSL_ENSURE( pB, "Seite ohne Body." );
                pB->ChgColumns( rOldCol, rNewCol );
                rInvFlags |= 0x20;
            }

            //2. Kopf- und Fusszeilen.
            const SwFmtHeader &rOldH = pOldFmt->GetHeader();
            const SwFmtHeader &rNewH = pNewFmt->GetHeader();
            if( rOldH != rNewH )
                rInvFlags |= 0x08;

            const SwFmtFooter &rOldF = pOldFmt->GetFooter();
            const SwFmtFooter &rNewF = pNewFmt->GetFooter();
            if( rOldF != rNewF )
                rInvFlags |= 0x10;
            CheckDirChange();
        }
            /* kein break hier */
        case RES_FRM_SIZE:
        {
            const SwRect aOldPageFrmRect( Frm() );
            ViewShell *pSh = getRootFrm()->GetCurrShell();
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
            {
                bValidSize = sal_False;
                // OD 28.10.2002 #97265# - Don't call <SwPageFrm::MakeAll()>
                // Calculation of the page is not necessary, because its size is
                // is invalidated here and further invalidation is done in the
                // calling method <SwPageFrm::Modify(..)> and probably by calling
                // <SwLayoutFrm::Modify(..)> at the end.
                // It can also causes inconsistences, because the lowers are
                // adjusted, but not calculated, and a <SwPageFrm::MakeAll()> of
                // a next page is called. This is performed on the switch to the
                // online layout.
                //MakeAll();
            }
            else
            {
                const SwFmtFrmSize &rSz = nWhich == RES_FMT_CHG ?
                        ((SwFmtChg*)pNew)->pChangedFmt->GetFrmSize() :
                        (const SwFmtFrmSize&)*pNew;

                Frm().Height( Max( rSz.GetHeight(), long(MINLAY) ) );
                Frm().Width ( Max( rSz.GetWidth(),  long(MINLAY) ) );

                // PAGES01
                if ( GetUpper() )
                    static_cast<SwRootFrm*>(GetUpper())->CheckViewLayout( 0, 0 );
            }
            //Window aufraeumen.
            if( pSh && pSh->GetWin() && aOldPageFrmRect.HasArea() )
            {
                // OD 12.02.2003 #i9719#, #105645# - consider border and shadow of
                // page frame for determine 'old' rectangle - it's used for invalidating.
                const bool bRightSidebar = (SidebarPosition() == sw::sidebarwindows::SIDEBAR_RIGHT);
                SwRect aOldRectWithBorderAndShadow;
                SwPageFrm::GetBorderAndShadowBoundRect( aOldPageFrmRect, pSh, aOldRectWithBorderAndShadow,
                    IsLeftShadowNeeded(), IsRightShadowNeeded(), bRightSidebar );
                pSh->InvalidateWindows( aOldRectWithBorderAndShadow );
            }
            rInvFlags |= 0x03;
            if ( aOldPageFrmRect.Height() != Frm().Height() )
                rInvFlags |= 0x04;
        }
        break;

        case RES_COL:
        {
            SwLayoutFrm *pB = FindBodyCont();
            OSL_ENSURE( pB, "Seite ohne Body." );
            pB->ChgColumns( *(const SwFmtCol*)pOld, *(const SwFmtCol*)pNew );
            rInvFlags |= 0x22;
        }
        break;

        case RES_HEADER:
            rInvFlags |= 0x08;
            break;

        case RES_FOOTER:
            rInvFlags |= 0x10;
            break;
        case RES_TEXTGRID:
            rInvFlags |= 0x60;
            break;

        case RES_PAGEDESC_FTNINFO:
            //Die derzeit einzig sichere Methode:
            ((SwRootFrm*)GetUpper())->SetSuperfluous();
            SetMaxFtnHeight( pDesc->GetFtnInfo().GetHeight() );
            if ( !GetMaxFtnHeight() )
                SetMaxFtnHeight( LONG_MAX );
            SetColMaxFtnHeight();
            //Hier wird die Seite ggf. zerstoert!
            ((SwRootFrm*)GetUpper())->RemoveFtns( 0, sal_False, sal_True );
            break;
        case RES_FRAMEDIR :
            CheckDirChange();
            break;

        default:
            bClear = sal_False;
    }
    if ( bClear )
    {
        if ( pOldSet || pNewSet )
        {
            if ( pOldSet )
                pOldSet->ClearItem( nWhich );
            if ( pNewSet )
                pNewSet->ClearItem( nWhich );
        }
        else
            SwLayoutFrm::Modify( pOld, pNew );
    }
}

/*************************************************************************
|*
|*                SwPageFrm::GetInfo()
|*
|*    Beschreibung      erfragt Informationen
|*
*************************************************************************/
    // erfrage vom Modify Informationen
sal_Bool SwPageFrm::GetInfo( SfxPoolItem & rInfo ) const
{
    if( RES_AUTOFMT_DOCNODE == rInfo.Which() )
    {
        // es gibt einen PageFrm also wird er benutzt
        return sal_False;
    }
    return sal_True;        // weiter suchen
}

/*************************************************************************
|*
|*  SwPageFrm::SetPageDesc()
|*
|*************************************************************************/
void  SwPageFrm::SetPageDesc( SwPageDesc *pNew, SwFrmFmt *pFmt )
{
    pDesc = pNew;
    if ( pFmt )
        SetFrmFmt( pFmt );
}

/*************************************************************************
|*
|*  SwPageFrm::FindPageDesc()
|*
|*  Beschreibung        Der richtige PageDesc wird bestimmt:
|*      0.  Vom Dokument bei Fussnotenseiten und Endnotenseiten
|*      1.  vom ersten BodyCntnt unterhalb der Seite.
|*      2.  vom PageDesc der vorstehenden Seite.
|*      3.  bei Leerseiten vom PageDesc der vorigen Seite.
|*      3.1 vom PageDesc der folgenden Seite wenn es keinen Vorgaenger gibt.
|*      4.  es ist der Default-PageDesc sonst.
|*      5.  Im BrowseMode ist der Pagedesc immer der vom ersten Absatz im
|*          Dokument oder Standard (der 0-te) wenn der erste Absatz keinen
|*          wuenscht.
|*     (6.  Im HTML-Mode ist der Pagedesc immer die HTML-Seitenvorlage.)
|*
|*************************************************************************/
SwPageDesc *SwPageFrm::FindPageDesc()
{
    //0.
    if ( IsFtnPage() )
    {
        SwDoc *pDoc = GetFmt()->GetDoc();
        if ( IsEndNotePage() )
            return pDoc->GetEndNoteInfo().GetPageDesc( *pDoc );
        else
            return pDoc->GetFtnInfo().GetPageDesc( *pDoc );
    }

    //6.
    //if ( GetFmt()->GetDoc()->IsHTMLMode() )
    //  return GetFmt()->GetDoc()->GetPageDescFromPool( RES_POOLPAGE_HTML );

    SwPageDesc *pRet = 0;

    //5.
    const ViewShell *pSh = getRootFrm()->GetCurrShell();
    if( pSh && pSh->GetViewOptions()->getBrowseMode() )
    {
        SwCntntFrm *pFrm = GetUpper()->ContainsCntnt();
        while ( !pFrm->IsInDocBody() )
            pFrm = pFrm->GetNextCntntFrm();
        SwFrm *pFlow = pFrm;
        if ( pFlow->IsInTab() )
            pFlow = pFlow->FindTabFrm();
        pRet = (SwPageDesc*)pFlow->GetAttrSet()->GetPageDesc().GetPageDesc();
        if ( !pRet )
            pRet = &GetFmt()->GetDoc()->GetPageDesc( 0 );
        return pRet;
    }

    SwFrm *pFlow = FindFirstBodyCntnt();
    if ( pFlow && pFlow->IsInTab() )
        pFlow = pFlow->FindTabFrm();

    //1.
    if ( pFlow )
    {
        SwFlowFrm *pTmp = SwFlowFrm::CastFlowFrm( pFlow );
        if ( !pTmp->IsFollow() )
            pRet = (SwPageDesc*)pFlow->GetAttrSet()->GetPageDesc().GetPageDesc();
    }

    //3. und 3.1
    if ( !pRet && IsEmptyPage() )
            // FME 2008-03-03 #i81544# lijian/fme: an empty page should have
            // the same page description as its prev, just like after construction
            // of the empty page.
        pRet = GetPrev() ? ((SwPageFrm*)GetPrev())->GetPageDesc() :
               GetNext() ? ((SwPageFrm*)GetNext())->GetPageDesc() : 0;

    //2.
    if ( !pRet )
        pRet = GetPrev() ?
                    ((SwPageFrm*)GetPrev())->GetPageDesc()->GetFollow() : 0;

    //4.
    if ( !pRet )
        pRet = &GetFmt()->GetDoc()->GetPageDesc( 0 );


    OSL_ENSURE( pRet, "Kein Descriptor gefunden." );
    return pRet;
}

//Wenn der RootFrm seine Groesse aendert muss benachrichtigt werden.
void AdjustSizeChgNotify( SwRootFrm *pRoot )
{
    const sal_Bool bOld = pRoot->IsSuperfluous();
    pRoot->bCheckSuperfluous = sal_False;
    ViewShell *pSh = pRoot->GetCurrShell();
    if ( pSh )
    {
        do
        {
            if( pRoot == pSh->GetLayout() )
            {
                pSh->SizeChgNotify();
                if ( pSh->Imp() )
                    pSh->Imp()->NotifySizeChg( pRoot->Frm().SSize() );
            }
            pSh = (ViewShell*)pSh->GetNext();
        } while ( pSh != pRoot->GetCurrShell() );
    }
    pRoot->bCheckSuperfluous = bOld;
}


inline void SetLastPage( SwPageFrm *pPage )
{
    ((SwRootFrm*)pPage->GetUpper())->pLastPage = pPage;
}

/*************************************************************************
|*
|*  SwPageFrm::Cut()
|*
|*************************************************************************/
void SwPageFrm::Cut()
{
    // PAGES01
    //AdjustRootSize( CHG_CUTPAGE, 0 );

    ViewShell *pSh = getRootFrm()->GetCurrShell();
    if ( !IsEmptyPage() )
    {
        if ( GetNext() )
            GetNext()->InvalidatePos();

        //Flys deren Anker auf anderen Seiten stehen umhaengen.
        //DrawObjecte spielen hier keine Rolle.
        if ( GetSortedObjs() )
        {
            for ( int i = 0; GetSortedObjs() &&
                             (sal_uInt16)i < GetSortedObjs()->Count(); ++i )
            {
                // #i28701#
                SwAnchoredObject* pAnchoredObj = (*GetSortedObjs())[i];

                if ( pAnchoredObj->ISA(SwFlyAtCntFrm) )
                {
                    SwFlyFrm* pFly = static_cast<SwFlyAtCntFrm*>(pAnchoredObj);
                    SwPageFrm *pAnchPage = pFly->GetAnchorFrm() ?
                                pFly->AnchorFrm()->FindPageFrm() : 0;
                    if ( pAnchPage && (pAnchPage != this) )
                    {
                        MoveFly( pFly, pAnchPage );
                        --i;
                        pFly->InvalidateSize();
                        pFly->_InvalidatePos();
                    }
                }
            }
        }
        //Window aufraeumen
        if ( pSh && pSh->GetWin() )
            pSh->InvalidateWindows( Frm() );
    }

    // die Seitennummer der Root runterzaehlen.
    ((SwRootFrm*)GetUpper())->DecrPhyPageNums();
    SwPageFrm *pPg = (SwPageFrm*)GetNext();
    if ( pPg )
    {
        while ( pPg )
        {
            pPg->DecrPhyPageNum();  //inline --nPhyPageNum
            pPg = (SwPageFrm*)pPg->GetNext();
        }
    }
    else
        ::SetLastPage( (SwPageFrm*)GetPrev() );

    SwFrm* pRootFrm = GetUpper();

    // Alle Verbindungen kappen.
    Remove();

    // PAGES01
    if ( pRootFrm )
        static_cast<SwRootFrm*>(pRootFrm)->CheckViewLayout( 0, 0 );
}

/*************************************************************************
|*
|*  SwPageFrm::Paste()
|*
|*************************************************************************/
void SwPageFrm::Paste( SwFrm* pParent, SwFrm* pSibling )
{
    OSL_ENSURE( pParent->IsRootFrm(), "Parent ist keine Root." );
    OSL_ENSURE( pParent, "Kein Parent fuer Paste." );
    OSL_ENSURE( pParent != this, "Bin selbst der Parent." );
    OSL_ENSURE( pSibling != this, "Bin mein eigener Nachbar." );
    OSL_ENSURE( !GetPrev() && !GetNext() && !GetUpper(),
            "Bin noch irgendwo angemeldet." );

    //In den Baum einhaengen.
    InsertBefore( (SwLayoutFrm*)pParent, pSibling );

    // die Seitennummer am Root hochzaehlen.
    ((SwRootFrm*)GetUpper())->IncrPhyPageNums();
    if( GetPrev() )
        SetPhyPageNum( ((SwPageFrm*)GetPrev())->GetPhyPageNum() + 1 );
    else
        SetPhyPageNum( 1 );
    SwPageFrm *pPg = (SwPageFrm*)GetNext();
    if ( pPg )
    {
        while ( pPg )
        {
            pPg->IncrPhyPageNum();  //inline ++nPhyPageNum
            pPg->_InvalidatePos();
            pPg->InvalidateLayout();
            pPg = (SwPageFrm*)pPg->GetNext();
        }
    }
    else
        ::SetLastPage( this );

    if( Frm().Width() != pParent->Prt().Width() )
        _InvalidateSize();

    InvalidatePos();

    ViewShell *pSh = getRootFrm()->GetCurrShell();
    if ( pSh )
        pSh->SetFirstVisPageInvalid();
    // PAGES01
    getRootFrm()->CheckViewLayout( 0, 0 );
}

/*************************************************************************
|*
|*  SwPageFrm::PrepareRegisterChg()
|*
|*************************************************************************/
static void lcl_PrepFlyInCntRegister( SwCntntFrm *pFrm )
{
    pFrm->Prepare( PREP_REGISTER );
    if( pFrm->GetDrawObjs() )
    {
        for( sal_uInt16 i = 0; i < pFrm->GetDrawObjs()->Count(); ++i )
        {
            // #i28701#
            SwAnchoredObject* pAnchoredObj = (*pFrm->GetDrawObjs())[i];
            if ( pAnchoredObj->ISA(SwFlyInCntFrm) )
            {
                SwFlyFrm* pFly = static_cast<SwFlyInCntFrm*>(pAnchoredObj);
                SwCntntFrm *pCnt = pFly->ContainsCntnt();
                while ( pCnt )
                {
                    lcl_PrepFlyInCntRegister( pCnt );
                    pCnt = pCnt->GetNextCntntFrm();
                }
            }
        }
    }
}

void SwPageFrm::PrepareRegisterChg()
{
    SwCntntFrm *pFrm = FindFirstBodyCntnt();
    while( pFrm )
    {
        lcl_PrepFlyInCntRegister( pFrm );
        pFrm = pFrm->GetNextCntntFrm();
        if( !IsAnLower( pFrm ) )
            break;
    }
    if( GetSortedObjs() )
    {
        for( sal_uInt16 i = 0; i < GetSortedObjs()->Count(); ++i )
        {
            // #i28701#
            SwAnchoredObject* pAnchoredObj = (*GetSortedObjs())[i];
            if ( pAnchoredObj->ISA(SwFlyFrm) )
            {
                SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                pFrm = pFly->ContainsCntnt();
                while ( pFrm )
                {
                    ::lcl_PrepFlyInCntRegister( pFrm );
                    pFrm = pFrm->GetNextCntntFrm();
                }
            }
        }
    }
}

/*************************************************************************
|*
|*  SwFrm::CheckPageDescs()
|*
|*  Beschreibung        Prueft alle Seiten ab der uebergebenen, daraufhin,
|*      ob sie das richtige FrmFmt verwenden. Wenn 'falsche' Seiten
|*      aufgespuehrt werden, so wird versucht die Situation moeglichst
|*      einfache zu bereinigen.
|*
|*************************************************************************/
void SwFrm::CheckPageDescs( SwPageFrm *pStart, sal_Bool bNotifyFields )
{
    OSL_ENSURE( pStart, "Keine Startpage." );

    ViewShell *pSh   = pStart->getRootFrm()->GetCurrShell();
    SwViewImp *pImp  = pSh ? pSh->Imp() : 0;

    if ( pImp && pImp->IsAction() && !pImp->GetLayAction().IsCheckPages() )
    {
        pImp->GetLayAction().SetCheckPageNum( pStart->GetPhyPageNum() );
        return;
    }

    //Fuer das Aktualisieren der Seitennummern-Felder gibt nDocPos
    //die Seitenposition an, _ab_ der invalidiert werden soll.
    SwTwips nDocPos  = LONG_MAX;

    SwRootFrm *pRoot = (SwRootFrm*)pStart->GetUpper();
    SwDoc* pDoc      = pStart->GetFmt()->GetDoc();
    const bool bFtns = !pDoc->GetFtnIdxs().empty();

    SwPageFrm *pPage = pStart;
    if( pPage->GetPrev() && ((SwPageFrm*)pPage->GetPrev())->IsEmptyPage() )
        pPage = (SwPageFrm*)pPage->GetPrev();
    while ( pPage )
    {
        //gewuenschten PageDesc und FrmFmt festellen.
        SwPageDesc *pDesc = pPage->FindPageDesc();
        sal_Bool bCheckEmpty = pPage->IsEmptyPage();
        sal_Bool bActOdd = pPage->OnRightPage();
        sal_Bool bOdd = pPage->WannaRightPage();
        bool bFirst = pPage->OnFirstPage();
        SwFrmFmt *pFmtWish = 0;
        if (bFirst)
            pFmtWish = pDesc->GetFirstFmt();
        else
            pFmtWish = bOdd ? pDesc->GetRightFmt() : pDesc->GetLeftFmt();

        if ( bActOdd != bOdd ||
             pDesc != pPage->GetPageDesc() ||       //falscher Desc
             ( pFmtWish != pPage->GetFmt()  &&      //falsches Format und
               ( !pPage->IsEmptyPage() || pFmtWish ) //nicht Leerseite
             )
           )
        {
            //Wenn wir schon ein Seite veraendern muessen kann das eine
            //Weile dauern, deshalb hier den WaitCrsr pruefen.
            if( pImp )
                pImp->CheckWaitCrsr();

            //Ab hier muessen die Felder invalidiert werden!
            if ( nDocPos == LONG_MAX )
                nDocPos = pPage->GetPrev() ?
                            pPage->GetPrev()->Frm().Top() : pPage->Frm().Top();

            //Faelle:
            //1. Wir haben eine EmptyPage und wollen eine "Normalseite".
            //      ->EmptyPage wegwerfen und weiter mit der naechsten.
            //2. Wir haben eine EmptyPage und wollen eine EmptyPage mit
            //   anderem Descriptor.
            //      ->Descriptor austauschen.
            //3. Wir haben eine "Normalseite" und wollen eine EmptyPage.
            //      ->Emptypage einfuegen, nicht aber wenn die Vorseite
            //                             bereits eine EmptyPage ist -> 6.
            //4. Wir haben eine "Normalseite" und wollen eine "Normalseite"
            //   mit anderem Descriptor
            //      ->Descriptor und Format austauschen
            //5. Wir haben eine "Normalseite" und wollen eine "Normalseite"
            //   mit anderem Format
            //      ->Format austauschen.
            //6. Wir haben kein Wunschformat erhalten, also nehmen wir das
            //   'andere' Format (rechts/links) des PageDesc.

            if ( pPage->IsEmptyPage() && ( pFmtWish ||          //1.
                 ( !bOdd && !pPage->GetPrev() ) ) )
            {
                SwPageFrm *pTmp = (SwPageFrm*)pPage->GetNext();
                pPage->Cut();
                delete pPage;
                if ( pStart == pPage )
                    pStart = pTmp;
                pPage = pTmp;
                continue;
            }
            else if ( pPage->IsEmptyPage() && !pFmtWish &&  //2.
                      pDesc != pPage->GetPageDesc() )
            {
                pPage->SetPageDesc( pDesc, 0 );
            }
            else if ( !pPage->IsEmptyPage() &&      //3.
                      bActOdd != bOdd &&
                      ( ( !pPage->GetPrev() && !bOdd ) ||
                        ( pPage->GetPrev() &&
                          !((SwPageFrm*)pPage->GetPrev())->IsEmptyPage() )
                      )
                    )
            {
                if ( pPage->GetPrev() )
                    pDesc = ((SwPageFrm*)pPage->GetPrev())->GetPageDesc();
                SwPageFrm *pTmp = new SwPageFrm( pDoc->GetEmptyPageFmt(),pRoot,pDesc);
                pTmp->Paste( pRoot, pPage );
                pTmp->PreparePage( sal_False );
                pPage = pTmp;
            }
            else if ( pPage->GetPageDesc() != pDesc )           //4.
            {
                SwPageDesc *pOld = pPage->GetPageDesc();
                pPage->SetPageDesc( pDesc, pFmtWish );
                if ( bFtns )
                {
                    //Wenn sich bestimmte Werte der FtnInfo veraendert haben
                    //muss etwas passieren. Wir versuchen den Schaden zu
                    //begrenzen.
                    //Wenn die Seiten keinen FtnCont hat, ist zwar theoretisches
                    //ein Problem denkbar, aber das ignorieren wir mit aller Kraft.
                    //Bei Aenderungen hoffen wir mal, dass eine Invalidierung
                    //ausreicht, denn alles andere wuerde viel Kraft kosten.
                    SwFtnContFrm *pCont = pPage->FindFtnCont();
                    if ( pCont && !(pOld->GetFtnInfo() == pDesc->GetFtnInfo()) )
                        pCont->_InvalidateAll();
                }
            }
            else if ( pFmtWish && pPage->GetFmt() != pFmtWish )         //5.
            {
                pPage->SetFrmFmt( pFmtWish );
            }
            else if ( !pFmtWish )                                       //6.
            {
                //Format mit verdrehter Logic besorgen.
                pFmtWish = bOdd ? pDesc->GetLeftFmt() : pDesc->GetRightFmt();
                if ( pPage->GetFmt() != pFmtWish )
                    pPage->SetFrmFmt( pFmtWish );
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                OSL_FAIL( "CheckPageDescs, missing solution" );
            }
#endif
        }
        if ( bCheckEmpty )
        {
            //Es kann noch sein, dass die Leerseite schlicht  ueberflussig ist.
            //Obiger Algorithmus kann dies leider nicht feststellen.
            //Eigentlich muesste die Leerseite einfach praeventiv entfernt
            //werden; sie wuerde ja ggf. wieder eingefuegt.
            //Die EmptyPage ist genau dann ueberfluessig, wenn die Folgeseite
            //auch ohne sie auskommt. Dazu muessen wir uns die Verhaeltnisse
            //genauer ansehen. Wir bestimmen den PageDesc und die virtuelle
            //Seitennummer manuell.
            SwPageFrm *pPg = (SwPageFrm*)pPage->GetNext();
            if( !pPg || pPage->OnRightPage() == pPg->WannaRightPage() )
            {
                //Die Folgeseite hat kein Problem ein FrmFmt zu finden oder keinen
                //Nachfolger, also ist die Leerseite ueberfluessig.
                SwPageFrm *pTmp = (SwPageFrm*)pPage->GetNext();
                pPage->Cut();
                delete pPage;
                if ( pStart == pPage )
                    pStart = pTmp;
                pPage = pTmp;
                continue;
            }
        }
        pPage = (SwPageFrm*)pPage->GetNext();
    }

    pRoot->SetAssertFlyPages();
    pRoot->AssertPageFlys( pStart );

    if ( bNotifyFields && (!pImp || !pImp->IsUpdateExpFlds()) )
    {
        SwDocPosUpdate aMsgHnt( nDocPos );
        pDoc->UpdatePageFlds( &aMsgHnt );
    }

#if OSL_DEBUG_LEVEL > 0
    //Ein paar Pruefungen muessen schon erlaubt sein.

    //1. Keine zwei EmptyPages hintereinander.
    //2. Alle PageDescs richtig?
    sal_Bool bEmpty = sal_False;
    SwPageFrm *pPg = pStart;
    while ( pPg )
    {
        if ( pPg->IsEmptyPage() )
        {
            if ( bEmpty )
            {
                OSL_FAIL( "Doppelte Leerseiten." );
                break;  //Einmal reicht.
            }
            bEmpty = sal_True;
        }
        else
            bEmpty = sal_False;

//MA 21. Jun. 95: Kann zu testzwecken 'rein, ist aber bei zyklen durchaus
//moeglich: Ein paar Seiten, auf der ersten 'erste Seite' anwenden,
//rechte als folge der ersten, linke als folge der rechten, rechte als
//folge der linken.
//   OSL_ENSURE( pPg->GetPageDesc() == pPg->FindPageDesc(),
//              "Seite mit falschem Descriptor." );

        pPg = (SwPageFrm*)pPg->GetNext();
    }
#endif
}

/*************************************************************************
|*
|*  SwFrm::InsertPage()
|*
|*************************************************************************/
SwPageFrm *SwFrm::InsertPage( SwPageFrm *pPrevPage, sal_Bool bFtn )
{
    SwRootFrm *pRoot = (SwRootFrm*)pPrevPage->GetUpper();
    SwPageFrm *pSibling = (SwPageFrm*)pRoot->GetLower();
    SwPageDesc *pDesc = pSibling->GetPageDesc();

    pSibling = (SwPageFrm*)pPrevPage->GetNext();
        //Rechte (ungerade) oder linke (gerade) Seite einfuegen?
    sal_Bool bNextOdd = !pPrevPage->OnRightPage();
    sal_Bool bWishedOdd = bNextOdd;

    //Welcher PageDesc gilt?
    //Bei CntntFrm der aus dem Format wenn einer angegeben ist,
    //der Follow vom bereits in der PrevPage gueltigen sonst.
    pDesc = 0;
    if ( IsFlowFrm() && !SwFlowFrm::CastFlowFrm( this )->IsFollow() )
    {   SwFmtPageDesc &rDesc = (SwFmtPageDesc&)GetAttrSet()->GetPageDesc();
        pDesc = rDesc.GetPageDesc();
        if ( rDesc.GetNumOffset() )
        {
            bWishedOdd = rDesc.GetNumOffset() % 2 ? sal_True : sal_False;
            //Die Gelegenheit nutzen wir um das Flag an der Root zu pflegen.
            pRoot->SetVirtPageNum( sal_True );
        }
    }
    if ( !pDesc )
        pDesc = pPrevPage->GetPageDesc()->GetFollow();

    OSL_ENSURE( pDesc, "Missing PageDesc" );
    if( !(bWishedOdd ? pDesc->GetRightFmt() : pDesc->GetLeftFmt()) )
        bWishedOdd = !bWishedOdd;
    bool bWishedFirst = pDesc != pPrevPage->GetPageDesc();
    if (bWishedFirst && !pDesc->GetFirstFmt())
        bWishedFirst = false;

    SwDoc *pDoc = pPrevPage->GetFmt()->GetDoc();
    SwFrmFmt *pFmt;
    sal_Bool bCheckPages = sal_False;
    //Wenn ich kein FrmFmt fuer die Seite gefunden habe, muss ich eben eine
    //Leerseite einfuegen.
    if( bWishedOdd != bNextOdd )
    {   pFmt = pDoc->GetEmptyPageFmt();
        SwPageDesc *pTmpDesc = pPrevPage->GetPageDesc();
        SwPageFrm *pPage = new SwPageFrm( pFmt, pRoot, pTmpDesc );
        pPage->Paste( pRoot, pSibling );
        pPage->PreparePage( bFtn );
        //Wenn der Sibling keinen Bodytext enthaelt kann ich ihn vernichten
        //Es sei denn, es ist eine Fussnotenseite
        if ( pSibling && !pSibling->IsFtnPage() &&
             !pSibling->FindFirstBodyCntnt() )
        {
            SwPageFrm *pDel = pSibling;
            pSibling = (SwPageFrm*)pSibling->GetNext();
            if ( !pDoc->GetFtnIdxs().empty() )
                pRoot->RemoveFtns( pDel, sal_True );
            pDel->Cut();
            delete pDel;
        }
        else
            bCheckPages = sal_True;
    }
    if (bWishedFirst && !pDesc->IsFirstShared())
        pFmt = pDesc->GetFirstFmt();
    else
        pFmt = bWishedOdd ? pDesc->GetRightFmt() : pDesc->GetLeftFmt();
    OSL_ENSURE( pFmt, "Descriptor without format." );
    SwPageFrm *pPage = new SwPageFrm( pFmt, pRoot, pDesc );
    pPage->Paste( pRoot, pSibling );
    pPage->PreparePage( bFtn );
    //Wenn der Sibling keinen Bodytext enthaelt kann ich ihn vernichten
    //Es sei denn es ist eine Fussnotenseite.
    if ( pSibling && !pSibling->IsFtnPage() &&
         !pSibling->FindFirstBodyCntnt() )
    {
        SwPageFrm *pDel = pSibling;
        pSibling = (SwPageFrm*)pSibling->GetNext();
        if ( !pDoc->GetFtnIdxs().empty() )
            pRoot->RemoveFtns( pDel, sal_True );
        pDel->Cut();
        delete pDel;
    }
    else
        bCheckPages = sal_True;

    if ( pSibling )
    {
        if ( bCheckPages )
        {
            CheckPageDescs( pSibling, sal_False );
            ViewShell *pSh = getRootFrm()->GetCurrShell();
            SwViewImp *pImp = pSh ? pSh->Imp() : 0;
            if ( pImp && pImp->IsAction() && !pImp->GetLayAction().IsCheckPages() )
            {
                const sal_uInt16 nNum = pImp->GetLayAction().GetCheckPageNum();
                if ( nNum == pPrevPage->GetPhyPageNum() + 1 )
                    pImp->GetLayAction().SetCheckPageNumDirect(
                                                    pSibling->GetPhyPageNum() );
                return pPage;
            }
        }
        else
            pRoot->AssertPageFlys( pSibling );
    }

    //Fuer das Aktualisieren der Seitennummern-Felder gibt nDocPos
    //die Seitenposition an, _ab_ der invalidiert werden soll.
    ViewShell *pSh = getRootFrm()->GetCurrShell();
    if ( !pSh || !pSh->Imp()->IsUpdateExpFlds() )
    {
        SwDocPosUpdate aMsgHnt( pPrevPage->Frm().Top() );
        pDoc->UpdatePageFlds( &aMsgHnt );
    }
    return pPage;
}

sw::sidebarwindows::SidebarPosition SwPageFrm::SidebarPosition() const
{
    ViewShell *pSh = getRootFrm()->GetCurrShell();
    if( !pSh || pSh->GetViewOptions()->getBrowseMode() )
    {
        return sw::sidebarwindows::SIDEBAR_RIGHT;
    }
    else
    {
        const bool bLTR = getRootFrm()->IsLeftToRightViewLayout();
        const bool bBookMode = pSh->GetViewOptions()->IsViewLayoutBookMode();
        const bool bRightSidebar = bLTR ? (!bBookMode || OnRightPage()) : (bBookMode && !OnRightPage());

        return bRightSidebar
               ? sw::sidebarwindows::SIDEBAR_RIGHT
               : sw::sidebarwindows::SIDEBAR_LEFT;
    }
}

/*************************************************************************
|*
|*  SwRootFrm::GrowFrm()
|*
|*************************************************************************/

SwTwips SwRootFrm::GrowFrm( SwTwips nDist, sal_Bool bTst, sal_Bool )
{
    if ( !bTst )
        Frm().SSize().Height() += nDist;
    return nDist;
}
/*************************************************************************
|*
|*  SwRootFrm::ShrinkFrm()
|*
|*************************************************************************/
SwTwips SwRootFrm::ShrinkFrm( SwTwips nDist, sal_Bool bTst, sal_Bool )
{
    OSL_ENSURE( nDist >= 0, "nDist < 0." );
    OSL_ENSURE( nDist <= Frm().Height(), "nDist > als aktuelle Groesse." );

    if ( !bTst )
        Frm().SSize().Height() -= nDist;
    return nDist;
}

/*************************************************************************
|*
|*  SwRootFrm::RemoveSuperfluous()
|*
|*  Beschreibung:       Entfernung von ueberfluessigen Seiten.
|*          Arbeitet nur wenn das Flag bCheckSuperfluous gesetzt ist.
|*          Definition: Eine Seite ist genau dann leer, wenn der
|*          Body-Textbereich keinen CntntFrm enthaelt, aber nicht, wenn noch
|*          mindestens ein Fly an der Seite klebt.
|*          Die Seite ist auch dann nicht leer, wenn sie noch eine
|*          Fussnote enthaelt.
|*          Es muss zweimal angesetzt werden um leeren Seiten aufzuspueren:
|*              - einmal fuer die Endnotenseiten.
|*              - und einmal fuer die Seiten des Bodytextes.
|*
|*************************************************************************/
void SwRootFrm::RemoveSuperfluous()
{
    if ( !IsSuperfluous() )
        return;
    bCheckSuperfluous = sal_False;

    SwPageFrm *pPage = GetLastPage();
    long nDocPos = LONG_MAX;

    //Jetzt wird fuer die jeweils letzte Seite geprueft ob sie leer ist
    //bei der ersten nicht leeren Seite wird die Schleife beendet.
    do
    {
        bool bExistEssentialObjs = ( 0 != pPage->GetSortedObjs() );
        if ( bExistEssentialObjs )
        {
            //Nur weil die Seite Flys hat sind wir noch lange nicht fertig,
            //denn wenn alle Flys an generischem Inhalt haengen, so ist sie
            //trotzdem ueberfluessig (Ueberpruefung auf DocBody sollte reichen).
            // OD 19.06.2003 #108784# - consider that drawing objects in
            // header/footer are supported now.
            bool bOnlySuperfluosObjs = true;
            SwSortedObjs &rObjs = *pPage->GetSortedObjs();
            for ( sal_uInt16 i = 0; bOnlySuperfluosObjs && i < rObjs.Count(); ++i )
            {
                // #i28701#
                SwAnchoredObject* pAnchoredObj = rObjs[i];
                // OD 2004-01-19 #110582# - do not consider hidden objects
                if ( pPage->GetFmt()->GetDoc()->IsVisibleLayerId(
                                    pAnchoredObj->GetDrawObj()->GetLayer() ) &&
                     !pAnchoredObj->GetAnchorFrm()->FindFooterOrHeader() )
                {
                    bOnlySuperfluosObjs = false;
                }
            }
            bExistEssentialObjs = !bOnlySuperfluosObjs;
        }

        // OD 19.06.2003 #108784# - optimization: check first, if essential objects
        // exists.
        const SwLayoutFrm* pBody = 0;
        if ( bExistEssentialObjs ||
             pPage->FindFtnCont() ||
             ( 0 != ( pBody = pPage->FindBodyCont() ) &&
                ( pBody->ContainsCntnt() ||
                    // #i47580#
                    // Do not delete page if there's an empty tabframe
                    // left. I think it might be correct to use ContainsAny()
                    // instead of ContainsCntnt() to cover the empty-table-case,
                    // but I'm not fully sure, since ContainsAny() also returns
                    // SectionFrames. Therefore I prefer to do it the safe way:
                  ( pBody->Lower() && pBody->Lower()->IsTabFrm() ) ) ) )
        {
            if ( pPage->IsFtnPage() )
            {
                while ( pPage->IsFtnPage() )
                {
                    pPage = (SwPageFrm*)pPage->GetPrev();
                    OSL_ENSURE( pPage, "Nur noch Endnotenseiten uebrig." );
                }
                continue;
            }
            else
                pPage = 0;
        }

        if ( pPage )
        {
            SwPageFrm *pEmpty = pPage;
            pPage = (SwPageFrm*)pPage->GetPrev();
            if ( !GetFmt()->GetDoc()->GetFtnIdxs().empty() )
                RemoveFtns( pEmpty, sal_True );
            pEmpty->Cut();
            delete pEmpty;
            nDocPos = pPage ? pPage->Frm().Top() : 0;
        }
    } while ( pPage );

    ViewShell *pSh = getRootFrm()->GetCurrShell();
    if ( nDocPos != LONG_MAX &&
         (!pSh || !pSh->Imp()->IsUpdateExpFlds()) )
    {
        SwDocPosUpdate aMsgHnt( nDocPos );
        GetFmt()->GetDoc()->UpdatePageFlds( &aMsgHnt );
    }
}

/*************************************************************************
|*
|*  SwRootFrm::AssertFlyPages()
|*
|*  Beschreibung        Stellt sicher, dass genuegend Seiten vorhanden
|*      sind, damit alle Seitengebundenen Rahmen und DrawObject
|*      untergebracht sind.
|*
|*************************************************************************/
void SwRootFrm::AssertFlyPages()
{
    if ( !IsAssertFlyPages() )
        return;
    bAssertFlyPages = sal_False;

    SwDoc *pDoc = GetFmt()->GetDoc();
    const SwFrmFmts *pTbl = pDoc->GetSpzFrmFmts();

    //Auf welche Seite will der 'letzte' Fly?
    sal_uInt16 nMaxPg = 0;
    sal_uInt16 i;

    for ( i = 0; i < pTbl->size(); ++i )
    {
        const SwFmtAnchor &rAnch = (*pTbl)[i]->GetAnchor();
        if ( !rAnch.GetCntntAnchor() && nMaxPg < rAnch.GetPageNum() )
            nMaxPg = rAnch.GetPageNum();
    }
    //Wieviele Seiten haben wir derzeit?
    SwPageFrm *pPage = (SwPageFrm*)Lower();
    while ( pPage && pPage->GetNext() &&
            !((SwPageFrm*)pPage->GetNext())->IsFtnPage() )
    {
        pPage = (SwPageFrm*)pPage->GetNext();
    }

    if ( nMaxPg > pPage->GetPhyPageNum() )
    {
        //Die Seiten werden ausgehend von der letzten Seite konsequent
        //nach den Regeln der PageDescs weitergefuehrt.
        sal_Bool bOdd = pPage->GetPhyPageNum() % 2 ? sal_True : sal_False;
        SwPageDesc *pDesc = pPage->GetPageDesc();
        SwFrm *pSibling = pPage->GetNext();
        for ( i = pPage->GetPhyPageNum(); i < nMaxPg; ++i  )
        {
            if ( !(bOdd ? pDesc->GetRightFmt() : pDesc->GetLeftFmt()) )
            {
                //Leerseite einfuegen, die Flys werden aber erst von
                //der naechsten Seite aufgenommen!
                pPage = new SwPageFrm( pDoc->GetEmptyPageFmt(), this, pDesc );
                pPage->Paste( this, pSibling );
                pPage->PreparePage( sal_False );
                bOdd = bOdd ? sal_False : sal_True;
                ++i;
            }
            pPage = new
                    SwPageFrm( (bOdd ? pDesc->GetRightFmt() :
                                       pDesc->GetLeftFmt()), this, pDesc );
            pPage->Paste( this, pSibling );
            pPage->PreparePage( sal_False );
            bOdd = bOdd ? sal_False : sal_True;
            pDesc = pDesc->GetFollow();
        }
        //Jetzt koennen die Endnotenseiten natuerlich wieder krumm sein;
        //in diesem Fall werden sie vernichtet.
        if ( !pDoc->GetFtnIdxs().empty() )
        {
            pPage = (SwPageFrm*)Lower();
            while ( pPage && !pPage->IsFtnPage() )
                pPage = (SwPageFrm*)pPage->GetNext();

            if ( pPage )
            {
                SwPageDesc *pTmpDesc = pPage->FindPageDesc();
                bOdd = pPage->OnRightPage();
                if ( pPage->GetFmt() !=
                     (bOdd ? pTmpDesc->GetRightFmt() : pTmpDesc->GetLeftFmt()) )
                    RemoveFtns( pPage, sal_False, sal_True );
            }
        }
    }
}

/*************************************************************************
|*
|*  SwRootFrm::AssertPageFlys()
|*
|*  Beschreibung        Stellt sicher, dass ab der uebergebenen Seite
|*      auf allen Seiten die Seitengebunden Objecte auf der richtigen
|*      Seite (Seitennummer stehen).
|*
|*************************************************************************/
void SwRootFrm::AssertPageFlys( SwPageFrm *pPage )
{
    while ( pPage )
    {
        if ( pPage->GetSortedObjs() )
        {
            pPage->GetSortedObjs();
            for ( int i = 0;
                  pPage->GetSortedObjs() && sal_uInt16(i) < pPage->GetSortedObjs()->Count();
                  ++i)
            {
                // #i28701#
                SwFrmFmt& rFmt = (*pPage->GetSortedObjs())[i]->GetFrmFmt();
                const SwFmtAnchor &rAnch = rFmt.GetAnchor();
                const sal_uInt16 nPg = rAnch.GetPageNum();
                if ((rAnch.GetAnchorId() == FLY_AT_PAGE) &&
                     nPg != pPage->GetPhyPageNum() )
                {
                    //Das er auf der falschen Seite steht muss noch nichts
                    //heissen, wenn er eigentlich auf der Vorseite
                    //stehen will und diese eine EmptyPage ist.
                    if( nPg && !(pPage->GetPhyPageNum()-1 == nPg &&
                        ((SwPageFrm*)pPage->GetPrev())->IsEmptyPage()) )
                    {
                        //Umhaengen kann er sich selbst, indem wir ihm
                        //einfach ein Modify mit seinem AnkerAttr schicken.
#if OSL_DEBUG_LEVEL > 1
                        const sal_uInt32 nCnt = pPage->GetSortedObjs()->Count();
                        rFmt.NotifyClients( 0, (SwFmtAnchor*)&rAnch );
                        OSL_ENSURE( !pPage->GetSortedObjs() ||
                                nCnt != pPage->GetSortedObjs()->Count(),
                                "Object couldn't be reattached!" );
#else
                        rFmt.NotifyClients( 0, (SwFmtAnchor*)&rAnch );
#endif
                        --i;
                    }
                }
            }
        }
        pPage = (SwPageFrm*)pPage->GetNext();
    }
}

/*************************************************************************
|*
|*  SwRootFrm::ChgSize()
|*
|*************************************************************************/
Size SwRootFrm::ChgSize( const Size& aNewSize )
{
    Frm().SSize() = aNewSize;
    _InvalidatePrt();
    bFixSize = sal_False;
    return Frm().SSize();
}

/*************************************************************************
|*
|*  SwRootFrm::MakeAll()
|*
|*************************************************************************/
void SwRootFrm::MakeAll()
{
    if ( !bValidPos )
    {   bValidPos = sal_True;
        aFrm.Pos().X() = aFrm.Pos().Y() = DOCUMENTBORDER;
    }
    if ( !bValidPrtArea )
    {   bValidPrtArea = sal_True;
        aPrt.Pos().X() = aPrt.Pos().Y() = 0;
        aPrt.SSize( aFrm.SSize() );
    }
    if ( !bValidSize )
        //SSize wird von den Seiten (Cut/Paste) eingestellt.
        bValidSize = sal_True;
}

/*************************************************************************
|*
|*  SwRootFrm::ImplInvalidateBrowseWidth()
|*
|*************************************************************************/
void SwRootFrm::ImplInvalidateBrowseWidth()
{
    bBrowseWidthValid = sal_False;
    SwFrm *pPg = Lower();
    while ( pPg )
    {
        pPg->InvalidateSize();
        pPg = pPg->GetNext();
    }
}

/*************************************************************************
|*
|*  SwRootFrm::ImplCalcBrowseWidth()
|*
|*************************************************************************/
void SwRootFrm::ImplCalcBrowseWidth()
{
    OSL_ENSURE( GetCurrShell() && GetCurrShell()->GetViewOptions()->getBrowseMode(),
            "CalcBrowseWidth and not in BrowseView" );

    //Die (minimale) Breite wird von Rahmen, Tabellen und Zeichenobjekten
    //bestimmt. Die Breite wird nicht anhand ihrer aktuellen Groessen bestimmt,
    //sondern anhand der Attribute. Es interessiert also nicht wie breit sie
    //sind, sondern wie breit sie sein wollen.
    //Rahmen und Zeichenobjekte innerhalb ander Objekte (Rahmen, Tabellen)
    //Zaehlen nicht.
    //Seitenraender und Spalten werden hier nicht beruecksichtigt.

    SwFrm *pFrm = ContainsCntnt();
    while ( pFrm && !pFrm->IsInDocBody() )
        pFrm = ((SwCntntFrm*)pFrm)->GetNextCntntFrm();
    if ( !pFrm )
        return;

    bBrowseWidthValid = sal_True;
    ViewShell *pSh = getRootFrm()->GetCurrShell();
    nBrowseWidth = pSh
                    ? MINLAY + 2 * pSh->GetOut()->
                                PixelToLogic( pSh->GetBrowseBorder() ).Width()
                    : 5000;
    do
    {
        if ( pFrm->IsInTab() )
            pFrm = pFrm->FindTabFrm();

        if ( pFrm->IsTabFrm() &&
             !((SwLayoutFrm*)pFrm)->GetFmt()->GetFrmSize().GetWidthPercent() )
        {
            SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFrm );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            const SwFmtHoriOrient &rHori = rAttrs.GetAttrSet().GetHoriOrient();
            long nWidth = rAttrs.GetSize().Width();
            if ( nWidth < USHRT_MAX-2000 && //-2000, weil bei Randeinstellung per
                                            //Zuppeln das USHRT_MAX verlorengeht!
                 text::HoriOrientation::FULL != rHori.GetHoriOrient() )
            {
                const SwHTMLTableLayout *pLayoutInfo =
                    ((const SwTabFrm *)pFrm)->GetTable()
                                            ->GetHTMLTableLayout();
                if ( pLayoutInfo )
                    nWidth = Min( nWidth, pLayoutInfo->GetBrowseWidthMin() );

                switch ( rHori.GetHoriOrient() )
                {
                    case text::HoriOrientation::NONE:
                        // OD 23.01.2003 #106895# - add 1st param to <SwBorderAttrs::CalcRight(..)>
                        nWidth += rAttrs.CalcLeft( pFrm ) + rAttrs.CalcRight( pFrm );
                        break;
                    case text::HoriOrientation::LEFT_AND_WIDTH:
                        nWidth += rAttrs.CalcLeft( pFrm );
                        break;
                    default:
                        break;

                }
                nBrowseWidth = Max( nBrowseWidth, nWidth );
            }
        }
        else if ( pFrm->GetDrawObjs() )
        {
            for ( sal_uInt16 i = 0; i < pFrm->GetDrawObjs()->Count(); ++i )
            {
                // #i28701#
                SwAnchoredObject* pAnchoredObj = (*pFrm->GetDrawObjs())[i];
                const SwFrmFmt& rFmt = pAnchoredObj->GetFrmFmt();
                const sal_Bool bFly = pAnchoredObj->ISA(SwFlyFrm);
                if ((bFly && (FAR_AWAY == pAnchoredObj->GetObjRect().Width()))
                    || rFmt.GetFrmSize().GetWidthPercent())
                {
                    continue;
                }

                long nWidth = 0;
                switch ( rFmt.GetAnchor().GetAnchorId() )
                {
                    case FLY_AS_CHAR:
                        nWidth = bFly ? rFmt.GetFrmSize().GetWidth() :
                                        pAnchoredObj->GetObjRect().Width();
                        break;
                    case FLY_AT_PARA:
                        {
                            // #i33170#
                            // Reactivated old code because
                            // nWidth = pAnchoredObj->GetObjRect().Right()
                            // gives wrong results for objects that are still
                            // at position FAR_AWAY.
                            if ( bFly )
                            {
                                nWidth = rFmt.GetFrmSize().GetWidth();
                                const SwFmtHoriOrient &rHori = rFmt.GetHoriOrient();
                                switch ( rHori.GetHoriOrient() )
                                {
                                    case text::HoriOrientation::NONE:
                                        nWidth += rHori.GetPos();
                                        break;
                                    case text::HoriOrientation::INSIDE:
                                    case text::HoriOrientation::LEFT:
                                        if ( text::RelOrientation::PRINT_AREA == rHori.GetRelationOrient() )
                                            nWidth += pFrm->Prt().Left();
                                        break;
                                    default:
                                        break;
                                }
                            }
                            else
                                //Fuer Zeichenobjekte ist die Auswahl sehr klein,
                                //weil sie keine Attribute haben, also durch ihre
                                //aktuelle Groesse bestimmt werden.
                                nWidth = pAnchoredObj->GetObjRect().Right() -
                                         pAnchoredObj->GetDrawObj()->GetAnchorPos().X();
                        }
                        break;
                    default:    /* do nothing */;
                }
                nBrowseWidth = Max( nBrowseWidth, nWidth );
            }
        }
        pFrm = pFrm->FindNextCnt();
    } while ( pFrm );
}

/*************************************************************************
|*
|*  SwRootFrm::StartAllAction()
|*
|*************************************************************************/

void SwRootFrm::StartAllAction()
{
    ViewShell *pSh = GetCurrShell();
    if ( pSh )
        do
        {   if ( pSh->ISA( SwCrsrShell ) )
                ((SwCrsrShell*)pSh)->StartAction();
            else
                pSh->StartAction();
            pSh = (ViewShell*)pSh->GetNext();

        } while ( pSh != GetCurrShell() );
}

void SwRootFrm::EndAllAction( sal_Bool bVirDev )
{
    ViewShell *pSh = GetCurrShell();
    if ( pSh )
        do
        {
            const sal_Bool bOldEndActionByVirDev = pSh->IsEndActionByVirDev();
            pSh->SetEndActionByVirDev( bVirDev );
            if ( pSh->ISA( SwCrsrShell ) )
            {
                ((SwCrsrShell*)pSh)->EndAction();
                ((SwCrsrShell*)pSh)->CallChgLnk();
                if ( pSh->ISA( SwFEShell ) )
                    ((SwFEShell*)pSh)->SetChainMarker();
            }
            else
                pSh->EndAction();
            pSh->SetEndActionByVirDev( bOldEndActionByVirDev );
            pSh = (ViewShell*)pSh->GetNext();

        } while ( pSh != GetCurrShell() );
}

void SwRootFrm::UnoRemoveAllActions()
{
    ViewShell *pSh = GetCurrShell();
    if ( pSh )
        do
        {
            // #i84729#
            // No end action, if <ViewShell> instance is currently in its end action.
            // Recursives calls to <::EndAction()> are not allowed.
            if ( !pSh->IsInEndAction() )
            {
                OSL_ENSURE(!pSh->GetRestoreActions(), "Restore action count is already set!");
                sal_Bool bCrsr = pSh->ISA( SwCrsrShell );
                sal_Bool bFE = pSh->ISA( SwFEShell );
                sal_uInt16 nRestore = 0;
                while( pSh->ActionCount() )
                {
                    if( bCrsr )
                    {
                        ((SwCrsrShell*)pSh)->EndAction();
                        ((SwCrsrShell*)pSh)->CallChgLnk();
                        if ( bFE )
                            ((SwFEShell*)pSh)->SetChainMarker();
                    }
                    else
                        pSh->EndAction();
                    nRestore++;
                }
                pSh->SetRestoreActions(nRestore);
            }
            pSh->LockView(sal_True);
            pSh = (ViewShell*)pSh->GetNext();

        } while ( pSh != GetCurrShell() );
}

void SwRootFrm::UnoRestoreAllActions()
{
    ViewShell *pSh = GetCurrShell();
    if ( pSh )
        do
        {
            sal_uInt16 nActions = pSh->GetRestoreActions();
            while( nActions-- )
            {
                if ( pSh->ISA( SwCrsrShell ) )
                    ((SwCrsrShell*)pSh)->StartAction();
                else
                    pSh->StartAction();
            }
            pSh->SetRestoreActions(0);
            pSh->LockView(sal_False);
            pSh = (ViewShell*)pSh->GetNext();

        } while ( pSh != GetCurrShell() );
}

// PAGES01: Helper functions for SwRootFrm::CheckViewLayout
static void lcl_MoveAllLowers( SwFrm* pFrm, const Point& rOffset );

static void lcl_MoveAllLowerObjs( SwFrm* pFrm, const Point& rOffset )
{
    SwSortedObjs* pSortedObj = 0;
    const bool bPage = pFrm->IsPageFrm();

    if ( bPage )
        pSortedObj = static_cast<SwPageFrm*>(pFrm)->GetSortedObjs();
    else
        pSortedObj = pFrm->GetDrawObjs();

    for ( sal_uInt16 i = 0; pSortedObj && i < pSortedObj->Count(); ++i)
    {
        SwAnchoredObject* pAnchoredObj = (*pSortedObj)[i];

        const SwFrmFmt& rObjFmt = pAnchoredObj->GetFrmFmt();
        const SwFmtAnchor& rAnchor = rObjFmt.GetAnchor();

        // all except from the as character anchored objects are moved
        // when processing the page frame:
        const bool bAsChar = (rAnchor.GetAnchorId() == FLY_AS_CHAR);
        if ( !bPage && !bAsChar )
            continue;

        SwObjPositioningInProgress aPosInProgress( *pAnchoredObj );

        if ( pAnchoredObj->ISA(SwFlyFrm) )
        {
            SwFlyFrm* pFlyFrm( static_cast<SwFlyFrm*>(pAnchoredObj) );
            lcl_MoveAllLowers( pFlyFrm, rOffset );
            pFlyFrm->NotifyDrawObj();
            // --> let the active embedded object be moved
            if ( pFlyFrm->Lower() )
            {
                if ( pFlyFrm->Lower()->IsNoTxtFrm() )
                {
                    SwCntntFrm* pCntntFrm = static_cast<SwCntntFrm*>(pFlyFrm->Lower());
                    SwRootFrm* pRoot = pFlyFrm->Lower()->getRootFrm();
                    ViewShell *pSh = pRoot ? pRoot->GetCurrShell() : 0;
                    if ( pSh )
                    {
                        SwOLENode* pNode = pCntntFrm->GetNode()->GetOLENode();
                        if ( pNode )
                        {
                            svt::EmbeddedObjectRef& xObj = pNode->GetOLEObj().GetObject();
                            if ( xObj.is() )
                            {
                                ViewShell* pTmp = pSh;
                                do
                                {
                                    SwFEShell* pFEShell = dynamic_cast< SwFEShell* >( pTmp );
                                    if ( pFEShell )
                                        pFEShell->MoveObjectIfActive( xObj, rOffset );
                                    pTmp = static_cast<ViewShell*>( pTmp->GetNext() );
                                } while( pTmp != pSh );
                            }
                        }
                    }
                }
            }
        }
        else if ( pAnchoredObj->ISA(SwAnchoredDrawObject) )
        {
            SwAnchoredDrawObject* pAnchoredDrawObj( static_cast<SwAnchoredDrawObject*>(pAnchoredObj) );

            // don't touch objects that are not yet positioned:
            const bool bNotYetPositioned = pAnchoredDrawObj->NotYetPositioned();
            if ( bNotYetPositioned )
                continue;

            const Point aCurrAnchorPos = pAnchoredDrawObj->GetDrawObj()->GetAnchorPos();
            const Point aNewAnchorPos( ( aCurrAnchorPos + rOffset ) );
            pAnchoredDrawObj->DrawObj()->SetAnchorPos( aNewAnchorPos );
            pAnchoredDrawObj->SetLastObjRect( pAnchoredDrawObj->GetObjRect().SVRect() );

            // clear contour cache
            if ( pAnchoredDrawObj->GetFrmFmt().GetSurround().IsContour() )
                ClrContourCache( pAnchoredDrawObj->GetDrawObj() );
        }
        // #i92511#
        // cache for object rectangle inclusive spaces has to be invalidated.
        pAnchoredObj->InvalidateObjRectWithSpaces();
    }
}

static void lcl_MoveAllLowers( SwFrm* pFrm, const Point& rOffset )
{
    const SwRect aFrm( pFrm->Frm() );

    // first move the current frame
    pFrm->Frm().Pos() += rOffset;

    // Don't forget accessibility:
    if( pFrm->IsAccessibleFrm() )
    {
        SwRootFrm *pRootFrm = pFrm->getRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
            pRootFrm->GetCurrShell() )
        {
            pRootFrm->GetCurrShell()->Imp()->MoveAccessibleFrm( pFrm, aFrm );
        }
    }

    // the move any objects
    lcl_MoveAllLowerObjs( pFrm, rOffset );

    // finally, for layout frames we have to call this function recursively:
    if ( pFrm->ISA(SwLayoutFrm) )
    {
        SwFrm* pLowerFrm = pFrm->GetLower();
        while ( pLowerFrm )
        {
            lcl_MoveAllLowers( pLowerFrm, rOffset );
            pLowerFrm = pLowerFrm->GetNext();
        }
    }
}

// PAGES01: Calculate how the pages have to be positioned
void SwRootFrm::CheckViewLayout( const SwViewOption* pViewOpt, const SwRect* pVisArea )
{
    // #i91432#
    // No calculation of page positions, if only an empty page is present.
    // This situation occurs when <SwRootFrm> instance is in construction
    // and the document contains only left pages.
    if ( Lower()->GetNext() == 0 &&
         static_cast<SwPageFrm*>(Lower())->IsEmptyPage() )
    {
        return;
    }

    if ( !pVisArea )
    {
        // no early return for bNewPage
        if ( mnViewWidth < 0 )
            mnViewWidth = 0;
    }
    else
    {
        OSL_ENSURE( pViewOpt, "CheckViewLayout required ViewOptions" );

        const sal_uInt16 nColumns =  pViewOpt->GetViewLayoutColumns();
        const bool   bBookMode = pViewOpt->IsViewLayoutBookMode();

        if ( nColumns == mnColumns && bBookMode == mbBookMode && pVisArea->Width() == mnViewWidth && !mbSidebarChanged )
            return;

        mnColumns = nColumns;
        mbBookMode = bBookMode;
        mnViewWidth = pVisArea->Width();
        mbSidebarChanged = false;
    }

    if( GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE ) )
    {
        mnColumns = 1;
        mbBookMode = false;
    }

    Calc();

    const sal_Bool bOldCallbackActionEnabled = IsCallbackActionEnabled();
    SetCallbackActionEnabled( sal_False );

    maPageRects.clear();

    const long nBorder = Frm().Pos().X();
    const long nVisWidth = mnViewWidth - 2 * nBorder;
    const long nGapBetweenPages = GAPBETWEENPAGES;

    // check how many pages fit into the first page layout row:
    SwPageFrm* pPageFrm = static_cast<SwPageFrm*>(Lower());

    // will contain the number of pages per row. 0 means that
    // the page does not fit.
    long nWidthRemain = nVisWidth;

    // after one row has been processed, these variables contain
    // the width of the row and the maxium of the page heights
    long nCurrentRowHeight = 0;
    long nCurrentRowWidth = 0;

    // these variables are used to finally set the size of the
    // root frame
    long nSumRowHeight = 0;
    SwTwips nMinPageLeft = TWIPS_MAX;
    SwTwips nMaxPageRight = 0;
    SwPageFrm* pStartOfRow = pPageFrm;
    sal_uInt16 nNumberOfPagesInRow = mbBookMode ? 1 : 0; // in book view, start with right page
    bool bFirstRow = true;

    bool bPageChanged = false;
    const bool bRTL = !IsLeftToRightViewLayout();
    const SwTwips nSidebarWidth = SwPageFrm::GetSidebarBorderWidth( GetCurrShell() );

    while ( pPageFrm )
    {
        // we consider the current page to be "start of row" if
        // 1. it is the first page in the current row or
        // 2. it is the second page in the row and the first page is an empty page in non-book view:
        const bool bStartOfRow = pPageFrm == pStartOfRow ||
                                             ( pStartOfRow->IsEmptyPage() && pPageFrm == pStartOfRow->GetNext() && !mbBookMode );

        const bool bEmptyPage = pPageFrm->IsEmptyPage() && !mbBookMode;

        // no half doc border space for first page in each row and
        long nPageWidth = 0;
        long nPageHeight = 0;

        if ( mbBookMode )
        {
            const SwFrm& rFormatPage = pPageFrm->GetFormatPage();

            nPageWidth  = rFormatPage.Frm().Width()  + nSidebarWidth + ((bStartOfRow || 1 == (pPageFrm->GetPhyPageNum()%2)) ? 0 : nGapBetweenPages);
            nPageHeight = rFormatPage.Frm().Height() + nGapBetweenPages;
        }
        else
        {
            SwRect aPageFrm;
            if ( !pPageFrm->IsEmptyPage() )
            {
                nPageWidth  = pPageFrm->Frm().Width() + nSidebarWidth + (bStartOfRow ? 0 : nGapBetweenPages);
                nPageHeight = pPageFrm->Frm().Height() + nGapBetweenPages;
            }
        }

        if ( !bEmptyPage )
            ++nNumberOfPagesInRow;

        // finish current row if
        // 1. in dynamic mode the current page does not fit anymore or
        // 2. the current page exceeds the maximum number of columns
        bool bRowFinished = (0 == mnColumns && nWidthRemain < nPageWidth ) ||
                            (0 != mnColumns && mnColumns < nNumberOfPagesInRow);

        // make sure that at least one page goes to the current row:
        if ( !bRowFinished || bStartOfRow )
        {
            // current page is allowed to be in current row
            nWidthRemain = nWidthRemain - nPageWidth;

            nCurrentRowWidth = nCurrentRowWidth + nPageWidth;
            nCurrentRowHeight = Max( nCurrentRowHeight, nPageHeight );

            pPageFrm = static_cast<SwPageFrm*>(pPageFrm->GetNext());

            if ( !pPageFrm )
                bRowFinished = true;
        }

        if ( bRowFinished )
        {
            // pPageFrm now points to the first page in the new row or null
            // pStartOfRow points to the first page in the current row

            // special centering for last row. pretend to fill the last row with virtual copies of the last page before centering:
            if ( !pPageFrm && nWidthRemain > 0 )
            {
                // find last page in current row:
                const SwPageFrm* pLastPageInCurrentRow = pStartOfRow;
                while( pLastPageInCurrentRow->GetNext() )
                    pLastPageInCurrentRow = static_cast<const SwPageFrm*>(pLastPageInCurrentRow->GetNext());

                if ( pLastPageInCurrentRow->IsEmptyPage() )
                    pLastPageInCurrentRow = static_cast<const SwPageFrm*>(pLastPageInCurrentRow->GetPrev());

                // check how many times the last page would still fit into the remaining space:
                sal_uInt16 nNumberOfVirtualPages = 0;
                const sal_uInt16 nMaxNumberOfVirtualPages = mnColumns > 0 ? mnColumns - nNumberOfPagesInRow : USHRT_MAX;
                SwTwips nRemain = nWidthRemain;
                SwTwips nVirtualPagesWidth = 0;
                SwTwips nLastPageWidth = pLastPageInCurrentRow->Frm().Width() + nSidebarWidth;

                while ( ( mnColumns > 0 || nRemain > 0 ) && nNumberOfVirtualPages < nMaxNumberOfVirtualPages )
                {
                    SwTwips nLastPageWidthWithGap = nLastPageWidth;
                    if ( !mbBookMode || ( 0 == (nNumberOfVirtualPages + nNumberOfPagesInRow) %2) )
                        nLastPageWidthWithGap += nGapBetweenPages;

                    if ( mnColumns > 0 || nLastPageWidthWithGap < nRemain )
                    {
                        ++nNumberOfVirtualPages;
                        nVirtualPagesWidth += nLastPageWidthWithGap;
                    }
                    nRemain = nRemain - nLastPageWidthWithGap;
                }

                nCurrentRowWidth = nCurrentRowWidth + nVirtualPagesWidth;
            }

            // first page in book mode is always special:
            if ( bFirstRow && mbBookMode )
            {
                // #i88036#
                nCurrentRowWidth +=
                    pStartOfRow->GetFormatPage().Frm().Width() + nSidebarWidth;
            }

            // center page if possible
            const long nSizeDiff = nVisWidth > nCurrentRowWidth ?
                                   ( nVisWidth - nCurrentRowWidth ) / 2 :
                                   0;

            // adjust positions of pages in current row
            long nX = nSizeDiff;

            const long nRowStart = nBorder + nSizeDiff;
            const long nRowEnd   = nRowStart + nCurrentRowWidth;

            if ( bFirstRow && mbBookMode )
            {
                // #i88036#
                nX += pStartOfRow->GetFormatPage().Frm().Width() + nSidebarWidth;
            }

            SwPageFrm* pEndOfRow = pPageFrm;
            SwPageFrm* pPageToAdjust = pStartOfRow;

            do
            {
                const SwPageFrm* pFormatPage = pPageToAdjust;
                if ( mbBookMode )
                    pFormatPage = &pPageToAdjust->GetFormatPage();

                const SwTwips nCurrentPageWidth = pFormatPage->Frm().Width() + (pFormatPage->IsEmptyPage() ? 0 : nSidebarWidth);
                const Point aOldPagePos = pPageToAdjust->Frm().Pos();
                const bool bLeftSidebar = pPageToAdjust->SidebarPosition() == sw::sidebarwindows::SIDEBAR_LEFT;
                const SwTwips nLeftPageAddOffset = bLeftSidebar ?
                                                   nSidebarWidth :
                                                   0;

                Point aNewPagePos( nBorder + nX, nBorder + nSumRowHeight );
                Point aNewPagePosWithLeftOffset( nBorder + nX + nLeftPageAddOffset, nBorder + nSumRowHeight );

                // RTL view layout: Calculate mirrored page position
                if ( bRTL )
                {
                    const long nXOffsetInRow = aNewPagePos.X() - nRowStart;
                    aNewPagePos.X() = nRowEnd - nXOffsetInRow - nCurrentPageWidth;
                    aNewPagePosWithLeftOffset = aNewPagePos;
                    aNewPagePosWithLeftOffset.X() += nLeftPageAddOffset;
                }

                if ( aNewPagePosWithLeftOffset != aOldPagePos )
                {
                    lcl_MoveAllLowers( pPageToAdjust, aNewPagePosWithLeftOffset - aOldPagePos );
                    pPageToAdjust->SetCompletePaint();
                    bPageChanged = true;
                }

                // calculate area covered by the current page and store to
                // maPageRects. This is used e.g., for cursor setting
                const bool bFirstColumn = pPageToAdjust == pStartOfRow;
                const bool bLastColumn = pPageToAdjust->GetNext() == pEndOfRow;
                const bool bLastRow = !pEndOfRow;

                nMinPageLeft  = Min( nMinPageLeft, aNewPagePos.X() );
                nMaxPageRight = Max( nMaxPageRight, aNewPagePos.X() + nCurrentPageWidth);

                // border of nGapBetweenPages around the current page:
                SwRect aPageRectWithBorders( aNewPagePos.X() - nGapBetweenPages,
                                             aNewPagePos.Y(),
                                             pPageToAdjust->Frm().SSize().Width() + nGapBetweenPages + nSidebarWidth,
                                             nCurrentRowHeight );

                static const long nOuterClickDiff = 1000000;

                // adjust borders for these special cases:
                if ( (bFirstColumn && !bRTL) || (bLastColumn && bRTL) )
                    aPageRectWithBorders.SubLeft( nOuterClickDiff );
                if ( (bLastColumn && !bRTL) || (bFirstColumn && bRTL) )
                    aPageRectWithBorders.AddRight( nOuterClickDiff );
                if ( bFirstRow )
                    aPageRectWithBorders.SubTop( nOuterClickDiff );
                if ( bLastRow )
                    aPageRectWithBorders.AddBottom( nOuterClickDiff );

                maPageRects.push_back( aPageRectWithBorders );

                nX = nX + nCurrentPageWidth;
                pPageToAdjust = static_cast<SwPageFrm*>(pPageToAdjust->GetNext());

                // distance to next page
                if ( pPageToAdjust && pPageToAdjust != pEndOfRow )
                {
                    // in book view, we add the x gap before left (even) pages:
                    if ( mbBookMode )
                    {
                        if ( 0 == (pPageToAdjust->GetPhyPageNum()%2) )
                            nX = nX + nGapBetweenPages;
                    }
                    else
                    {
                        // in non-book view, dont add x gap before
                        // 1. the last empty page in a row
                        // 2. after an empty page
                        const bool bDontAddGap = ( pPageToAdjust->IsEmptyPage() && pPageToAdjust->GetNext() == pEndOfRow ) ||
                                                 ( static_cast<SwPageFrm*>(pPageToAdjust->GetPrev())->IsEmptyPage() );

                        if  ( !bDontAddGap )
                            nX = nX + nGapBetweenPages;
                    }
                }
            }
            while ( pPageToAdjust != pEndOfRow );

            // adjust values for root frame size
            nSumRowHeight = nSumRowHeight + nCurrentRowHeight;

            // start new row:
            nCurrentRowHeight = 0;
            nCurrentRowWidth = 0;
            pStartOfRow = pEndOfRow;
            nWidthRemain = nVisWidth;
            nNumberOfPagesInRow = 0;
            bFirstRow = false;
        } // end row finished
    } // end while

    // set size of root frame:
    const Size aOldSize( Frm().SSize() );
    const Size aNewSize( nMaxPageRight - nBorder, nSumRowHeight - nGapBetweenPages );

    if ( bPageChanged || aNewSize != aOldSize )
    {
        ChgSize( aNewSize );
        ::AdjustSizeChgNotify( this );
        Calc();

        ViewShell* pSh = GetCurrShell();

        if ( pSh && pSh->GetDoc()->GetDocShell() )
        {
            pSh->SetFirstVisPageInvalid();
            if (bOldCallbackActionEnabled)
            {
                pSh->InvalidateWindows( SwRect( 0, 0, LONG_MAX, LONG_MAX ) );
                pSh->GetDoc()->GetDocShell()->Broadcast(SfxSimpleHint(SFX_HINT_DOCCHANGED));
            }
        }
    }

    maPagesArea.Pos( Frm().Pos() );
    maPagesArea.SSize( aNewSize );
    if ( TWIPS_MAX != nMinPageLeft )
        maPagesArea._Left( nMinPageLeft );

    SetCallbackActionEnabled( bOldCallbackActionEnabled );
}

bool SwRootFrm::IsLeftToRightViewLayout() const
{
    // Layout direction determined by layout direction of the first page.
    // #i88036#
    // Only ask a non-empty page frame for its layout direction
//    const SwPageFrm* pPage = dynamic_cast<const SwPageFrm*>(Lower());
//    return !pPage->IsRightToLeft() && !pPage->IsVertical();
    const SwPageFrm& rPage =
                    dynamic_cast<const SwPageFrm*>(Lower())->GetFormatPage();
    return !rPage.IsRightToLeft() && !rPage.IsVertical();
}

const SwPageFrm& SwPageFrm::GetFormatPage() const
{
    const SwPageFrm* pRet = this;
    if ( IsEmptyPage() )
    {
        pRet = static_cast<const SwPageFrm*>( OnRightPage() ? GetNext() : GetPrev() );
        // #i88035#
        // Typically a right empty page frame has a next non-empty page frame and
        // a left empty page frame has a previous non-empty page frame.
        // But under certain cirsumstances this assumption is not true -
        // e.g. during insertion of a left page at the end of the document right
        // after a left page in an intermediate state a right empty page does not
        // have a next page frame.
        if ( pRet == 0 )
        {
            if ( OnRightPage() )
            {
                pRet = static_cast<const SwPageFrm*>( GetPrev() );
            }
            else
            {
                pRet = static_cast<const SwPageFrm*>( GetNext() );
            }
        }
        OSL_ENSURE( pRet,
                "<SwPageFrm::GetFormatPage()> - inconsistent layout: empty page without previous and next page frame --> crash." );
    }
    return *pRet;
}

bool SwPageFrm::IsOverHeaderFooterArea( const Point& rPt, FrameControlType &rControl ) const
{
    long nUpperLimit = 0;
    long nLowerLimit = 0;
    const SwFrm* pFrm = Lower();
    while ( pFrm )
    {
        if ( pFrm->IsBodyFrm() )
        {
            nUpperLimit = pFrm->Frm().Top();
            nLowerLimit = pFrm->Frm().Bottom();
        }
        else if ( pFrm->IsFtnContFrm() )
            nLowerLimit = pFrm->Frm().Bottom();

        pFrm = pFrm->GetNext();
    }

    SwRect aHeaderArea( Frm().TopLeft(),
           Size( Frm().Width(), nUpperLimit - Frm().Top() ) );

    if ( aHeaderArea.IsInside( rPt ) )
    {
        rControl = Header;
        return true;
    }
    else
    {
        SwRect aFooterArea( Point( Frm().Left(), nLowerLimit ),
                Size( Frm().Width(), Frm().Bottom() - nLowerLimit ) );

        if ( aFooterArea.IsInside( rPt ) )
        {
            rControl = Footer;
            return true;
        }
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
