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

#include "hintids.hxx"
#include <svl/itemiter.hxx>
#include <svtools/imap.hxx>
#include <vcl/graph.hxx>
#include <tools/poly.hxx>
#include <tools/helpers.hxx>
#include <svx/contdlg.hxx>
#include <editeng/protitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/keepitem.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtclds.hxx>
#include <fmtcntnt.hxx>
#include <fmturl.hxx>
#include <fmtsrnd.hxx>
#include <fmtornt.hxx>
#include <fmtpdsc.hxx>
#include <fmtcnct.hxx>
#include <layhelp.hxx>
#include <ndtxt.hxx>
#include <svx/svdogrp.hxx>
#include <ndgrf.hxx>
#include <tolayoutanchoredobjectposition.hxx>
#include <fmtfollowtextflow.hxx>
#include <sortedobjs.hxx>
#include <objectformatter.hxx>
#include <anchoredobject.hxx>
#include <ndole.hxx>
#include <swtable.hxx>
#include <svx/svdpage.hxx>
#include "doc.hxx"
#include "viewsh.hxx"
#include "layouter.hxx"
#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "pam.hxx"
#include "frmatr.hxx"
#include "viewimp.hxx"
#include "viewopt.hxx"
#include "dcontact.hxx"
#include "dflyobj.hxx"
#include "dview.hxx"
#include "flyfrm.hxx"
#include "frmtool.hxx"
#include "frmfmt.hxx"
#include "hints.hxx"
#include "swregion.hxx"
#include "tabfrm.hxx"
#include "txtfrm.hxx"
#include "ndnotxt.hxx"
#include "notxtfrm.hxx"   // GetGrfArea
#include "flyfrms.hxx"
#include "ndindex.hxx"   // GetGrfArea
#include "sectfrm.hxx"
#include <vcl/svapp.hxx>
#include "switerator.hxx"

using namespace ::com::sun::star;


TYPEINIT2(SwFlyFrm,SwLayoutFrm,SwAnchoredObject);

/*************************************************************************
|*
|*  SwFlyFrm::SwFlyFrm()
|*
|*************************************************************************/

SwFlyFrm::SwFlyFrm( SwFlyFrmFmt *pFmt, SwFrm* pSib, SwFrm *pAnch ) :
    SwLayoutFrm( pFmt, pSib ),
    SwAnchoredObject(), // #i26791#
    pPrevLink( 0 ),
    pNextLink( 0 ),
    bInCnt( sal_False ),
    bAtCnt( sal_False ),
    bLayout( sal_False ),
    bAutoPosition( sal_False ),
    bNoShrink( sal_False ),
    bLockDeleteContent( sal_False )
{
    nType = FRMC_FLY;

    bInvalid = bNotifyBack = sal_True;
    bLocked  = bMinHeight =
    bHeightClipped = bWidthClipped = bFormatHeightOnly = sal_False;

    //Grosseneinstellung, Fixe groesse ist immer die Breite
    const SwFmtFrmSize &rFrmSize = pFmt->GetFrmSize();
    sal_uInt16 nDir =
        ((SvxFrameDirectionItem&)pFmt->GetFmtAttr( RES_FRAMEDIR )).GetValue();
    if( FRMDIR_ENVIRONMENT == nDir )
    {
        bDerivedVert = 1;
        bDerivedR2L = 1;
    }
    else
    {
        bInvalidVert = 0;
        bDerivedVert = 0;
        bDerivedR2L = 0;
        if( FRMDIR_HORI_LEFT_TOP == nDir || FRMDIR_HORI_RIGHT_TOP == nDir )
        {
            //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
            bVertLR = 0;
            bVertical = 0;
        }
        else
        {
            const ViewShell *pSh = getRootFrm() ? getRootFrm()->GetCurrShell() : 0;
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
                if ( FRMDIR_VERT_TOP_LEFT == nDir )
                    bVertLR = 1;
                else
                    bVertLR = 0;
            }
        }

        bInvalidR2L = 0;
        if( FRMDIR_HORI_RIGHT_TOP == nDir )
            bRightToLeft = 1;
        else
            bRightToLeft = 0;
    }

    Frm().Width( rFrmSize.GetWidth() );
    Frm().Height( rFrmSize.GetHeightSizeType() == ATT_VAR_SIZE ? MINFLY : rFrmSize.GetHeight() );

    // Fixed or variable Height?
    if ( rFrmSize.GetHeightSizeType() == ATT_MIN_SIZE )
        bMinHeight = sal_True;
    else if ( rFrmSize.GetHeightSizeType() == ATT_FIX_SIZE )
        bFixSize = sal_True;

    // insert columns, if necessary
    InsertColumns();

    //Erst das Init, dann den Inhalt, denn zum Inhalt koennen  widerum
    //Objekte/Rahmen gehoeren die dann angemeldet werden.
    InitDrawObj( sal_False );

    Chain( pAnch );

    InsertCnt();

    //Und erstmal in den Wald stellen die Kiste, damit bei neuen Dokument nicht
    //unnoetig viel formatiert wird.
    Frm().Pos().X() = Frm().Pos().Y() = FAR_AWAY;
}

void SwFlyFrm::Chain( SwFrm* _pAnch )
{
    // Connect to chain neighboors.
    // No problem, if a neighboor doesn't exist - the construction of the
    // neighboor will make the connection
    const SwFmtChain& rChain = GetFmt()->GetChain();
    if ( rChain.GetPrev() || rChain.GetNext() )
    {
        if ( rChain.GetNext() )
        {
            SwFlyFrm* pFollow = FindChainNeighbour( *rChain.GetNext(), _pAnch );
            if ( pFollow )
            {
                OSL_ENSURE( !pFollow->GetPrevLink(), "wrong chain detected" );
                if ( !pFollow->GetPrevLink() )
                    SwFlyFrm::ChainFrames( this, pFollow );
            }
        }
        if ( rChain.GetPrev() )
        {
            SwFlyFrm *pMaster = FindChainNeighbour( *rChain.GetPrev(), _pAnch );
            if ( pMaster )
            {
                OSL_ENSURE( !pMaster->GetNextLink(), "wrong chain detected" );
                if ( !pMaster->GetNextLink() )
                    SwFlyFrm::ChainFrames( pMaster, this );
            }
        }
    }
}

void SwFlyFrm::InsertCnt()
{
    if ( !GetPrevLink() )
    {
        const SwFmtCntnt& rCntnt = GetFmt()->GetCntnt();
        OSL_ENSURE( rCntnt.GetCntntIdx(), ":-( no content prepared." );
        sal_uLong nIndex = rCntnt.GetCntntIdx()->GetIndex();
        // Lower() bedeutet SwColumnFrm, eingefuegt werden muss der Inhalt dann in den (Column)BodyFrm
        ::_InsertCnt( Lower() ? (SwLayoutFrm*)((SwLayoutFrm*)Lower())->Lower() : (SwLayoutFrm*)this,
                      GetFmt()->GetDoc(), nIndex );

        // NoTxt always have a fixed height.
        if ( Lower() && Lower()->IsNoTxtFrm() )
        {
            bFixSize = sal_True;
            bMinHeight = sal_False;
        }
    }
}

 void SwFlyFrm::InsertColumns()
 {
    // #i97379#
    // Check, if column are allowed.
    // Columns are not allowed for fly frames, which represent graphics or embedded objects.
    const SwFmtCntnt& rCntnt = GetFmt()->GetCntnt();
    OSL_ENSURE( rCntnt.GetCntntIdx(), "<SwFlyFrm::InsertColumns()> - no content prepared." );
    SwNodeIndex aFirstCntnt( *(rCntnt.GetCntntIdx()), 1 );
    if ( aFirstCntnt.GetNode().IsNoTxtNode() )
    {
        return;
    }

    const SwFmtCol &rCol = GetFmt()->GetCol();
    if ( rCol.GetNumCols() > 1 )
    {
        //PrtArea ersteinmal so gross wie der Frm, damit die Spalten
        //vernuenftig eingesetzt werden koennen; das schaukelt sich dann
        //schon zurecht.
        Prt().Width( Frm().Width() );
        Prt().Height( Frm().Height() );
        const SwFmtCol aOld; //ChgColumns() verlaesst sich darauf, dass auch ein
                             //Old-Wert hereingereicht wird.
        ChgColumns( aOld, rCol );
    }
 }

/*************************************************************************
|*
|*  SwFlyFrm::~SwFlyFrm()
|*
|*************************************************************************/

SwFlyFrm::~SwFlyFrm()
{
    // Accessible objects for fly frames will be destroyed in this destructor.
    // For frames bound as char or frames that don't have an anchor we have
    // to do that ourselves. For any other frame the call RemoveFly at the
    // anchor will do that.
    if( IsAccessibleFrm() && GetFmt() && (IsFlyInCntFrm() || !GetAnchorFrm()) )
    {
        SwRootFrm *pRootFrm = getRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() )
        {
            ViewShell *pVSh = pRootFrm->GetCurrShell();
            if( pVSh && pVSh->Imp() )
            {
                // Lowers aren't disposed already, so we have to do a recursive
                // dispose
                pVSh->Imp()->DisposeAccessibleFrm( this, sal_True );
            }
        }
    }

    if( GetFmt() && !GetFmt()->GetDoc()->IsInDtor() )
    {
        // OD 2004-01-19 #110582#
        Unchain();

        // OD 2004-01-19 #110582#
        DeleteCnt();

        //Tschuess sagen.
        if ( GetAnchorFrm() )
            AnchorFrm()->RemoveFly( this );
    }

    FinitDrawObj();
}

const IDocumentDrawModelAccess* SwFlyFrm::getIDocumentDrawModelAccess()
{
    return GetFmt()->getIDocumentDrawModelAccess();
}

// OD 2004-01-19 #110582#
void SwFlyFrm::Unchain()
{
    if ( GetPrevLink() )
        UnchainFrames( GetPrevLink(), this );
    if ( GetNextLink() )
        UnchainFrames( this, GetNextLink() );
}

// OD 2004-01-19 #110582#
void SwFlyFrm::DeleteCnt()
{
    // #110582#-2
    if ( IsLockDeleteContent() )
        return;

    SwFrm* pFrm = pLower;
    while ( pFrm )
    {
        while ( pFrm->GetDrawObjs() && pFrm->GetDrawObjs()->Count() )
        {
            SwAnchoredObject *pAnchoredObj = (*pFrm->GetDrawObjs())[0];
            if ( pAnchoredObj->ISA(SwFlyFrm) )
                delete pAnchoredObj;
            else if ( pAnchoredObj->ISA(SwAnchoredDrawObject) )
            {
                // OD 23.06.2003 #108784# - consider 'virtual' drawing objects
                SdrObject* pObj = pAnchoredObj->DrawObj();
                if ( pObj->ISA(SwDrawVirtObj) )
                {
                    SwDrawVirtObj* pDrawVirtObj = static_cast<SwDrawVirtObj*>(pObj);
                    pDrawVirtObj->RemoveFromWriterLayout();
                    pDrawVirtObj->RemoveFromDrawingPage();
                }
                else
                {
                    SwDrawContact* pContact =
                            static_cast<SwDrawContact*>(::GetUserCall( pObj ));
                    if ( pContact )
                    {
                        pContact->DisconnectFromLayout();
                    }
                }
            }
        }

        pFrm->Remove();
        delete pFrm;
        pFrm = pLower;
    }

    InvalidatePage();
}

/*************************************************************************
|*
|*  SwFlyFrm::InitDrawObj()
|*
|*************************************************************************/

sal_uInt32 SwFlyFrm::_GetOrdNumForNewRef( const SwFlyDrawContact* pContact )
{
    sal_uInt32 nOrdNum( 0L );

    // search for another Writer fly frame registered at same frame format
    SwIterator<SwFlyFrm,SwFmt> aIter( *pContact->GetFmt() );
    const SwFlyFrm* pFlyFrm( 0L );
    for ( pFlyFrm = aIter.First(); pFlyFrm; pFlyFrm = aIter.Next() )
    {
        if ( pFlyFrm != this )
        {
            break;
        }
    }

    if ( pFlyFrm )
    {
        // another Writer fly frame found. Take its order number
        nOrdNum = pFlyFrm->GetVirtDrawObj()->GetOrdNum();
    }
    else
    {
        // no other Writer fly frame found. Take order number of 'master' object
        // #i35748# - use method <GetOrdNumDirect()> instead
        // of method <GetOrdNum()> to avoid a recalculation of the order number,
        // which isn't intended.
        nOrdNum = pContact->GetMaster()->GetOrdNumDirect();
    }

    return nOrdNum;
}

SwVirtFlyDrawObj* SwFlyFrm::CreateNewRef( SwFlyDrawContact *pContact )
{
    SwVirtFlyDrawObj *pDrawObj = new SwVirtFlyDrawObj( *pContact->GetMaster(), this );
    pDrawObj->SetModel( pContact->GetMaster()->GetModel() );
    pDrawObj->SetUserCall( pContact );

    //Der Reader erzeugt die Master und setzt diese, um die Z-Order zu
    //transportieren, in die Page ein. Beim erzeugen der ersten Referenz werden
    //die Master aus der Liste entfernt und fuehren von da an ein
    //Schattendasein.
    SdrPage* pPg( 0L );
    if ( 0 != ( pPg = pContact->GetMaster()->GetPage() ) )
    {
        const sal_uInt32 nOrdNum = pContact->GetMaster()->GetOrdNum();
        pPg->ReplaceObject( pDrawObj, nOrdNum );
    }
    // #i27030# - insert new <SwVirtFlyDrawObj> instance
    // into drawing page with correct order number
    else
    {
        pContact->GetFmt()->getIDocumentDrawModelAccess()->GetDrawModel()->GetPage( 0 )->
                        InsertObject( pDrawObj, _GetOrdNumForNewRef( pContact ) );
    }
    // #i38889# - assure, that new <SwVirtFlyDrawObj> instance
    // is in a visible layer.
    pContact->MoveObjToVisibleLayer( pDrawObj );
    return pDrawObj;
}



void SwFlyFrm::InitDrawObj( sal_Bool bNotify )
{
    //ContactObject aus dem Format suchen. Wenn bereits eines existiert, so
    //braucht nur eine neue Ref erzeugt werden, anderfalls ist es jetzt an
    //der Zeit das Contact zu erzeugen.

    IDocumentDrawModelAccess* pIDDMA = GetFmt()->getIDocumentDrawModelAccess();
    SwFlyDrawContact *pContact = SwIterator<SwFlyDrawContact,SwFmt>::FirstElement( *GetFmt() );
    if ( !pContact )
    {
        // #i52858# - method name changed
        pContact = new SwFlyDrawContact( (SwFlyFrmFmt*)GetFmt(),
                                          pIDDMA->GetOrCreateDrawModel() );
    }
    OSL_ENSURE( pContact, "InitDrawObj failed" );
    // OD 2004-03-22 #i26791#
    SetDrawObj( *(CreateNewRef( pContact )) );

    //Den richtigen Layer setzen.
    // OD 2004-01-19 #110582#
    SdrLayerID nHeavenId = pIDDMA->GetHeavenId();
    SdrLayerID nHellId = pIDDMA->GetHellId();
    // OD 2004-03-22 #i26791#
    GetVirtDrawObj()->SetLayer( GetFmt()->GetOpaque().GetValue()
                                ? nHeavenId
                                : nHellId );
    if ( bNotify )
        NotifyDrawObj();
}

/*************************************************************************
|*
|*  SwFlyFrm::FinitDrawObj()
|*
|*************************************************************************/

void SwFlyFrm::FinitDrawObj()
{
    if ( !GetVirtDrawObj() )
        return;

    //Bei den SdrPageViews abmelden falls das Objekt dort noch selektiert ist.
    if ( !GetFmt()->GetDoc()->IsInDtor() )
    {
        ViewShell *p1St = getRootFrm()->GetCurrShell();
        if ( p1St )
        {
            ViewShell *pSh = p1St;
            do
            {   //z.Zt. kann das Drawing nur ein Unmark auf alles, weil das
                //Objekt bereits Removed wurde.
                if( pSh->HasDrawView() )
                    pSh->Imp()->GetDrawView()->UnmarkAll();
                pSh = (ViewShell*)pSh->GetNext();

            } while ( pSh != p1St );
        }
    }

    //VirtObject mit in das Grab nehmen. Wenn das letzte VirObject
    //zerstoert wird, mussen das DrawObject und DrawContact ebenfalls
    //zerstoert werden.
    SwFlyDrawContact *pMyContact = 0;
    if ( GetFmt() )
    {
        bool bContinue = true;
        SwIterator<SwFrm,SwFmt> aFrmIter( *GetFmt() );
        for ( SwFrm* pFrm = aFrmIter.First(); pFrm; pFrm = aFrmIter.Next() )
            if ( pFrm != this )
            {
                // don't delete Contact if there is still a Frm
                bContinue = false;
                break;
            }

        if ( bContinue )
            // no Frm left, find Contact object to destroy
            pMyContact = SwIterator<SwFlyDrawContact,SwFmt>::FirstElement( *GetFmt() );
    }

    // OD, OS 2004-03-31 #116203# - clear user call of Writer fly frame 'master'
    // <SdrObject> to assure, that a <SwXFrame::dispose()> doesn't delete the
    // Writer fly frame again.
    if ( pMyContact )
    {
        pMyContact->GetMaster()->SetUserCall( 0 );
    }
    GetVirtDrawObj()->SetUserCall( 0 ); //Ruft sonst Delete des ContactObj
    delete GetVirtDrawObj();            //Meldet sich selbst beim Master ab.
    delete pMyContact;      //zerstoert den Master selbst.
}

/*************************************************************************
|*
|*  SwFlyFrm::ChainFrames()
|*
|*************************************************************************/

void SwFlyFrm::ChainFrames( SwFlyFrm *pMaster, SwFlyFrm *pFollow )
{
    OSL_ENSURE( pMaster && pFollow, "uncomplete chain" );
    OSL_ENSURE( !pMaster->GetNextLink(), "link can not be changed" );
    OSL_ENSURE( !pFollow->GetPrevLink(), "link can not be changed" );

    pMaster->pNextLink = pFollow;
    pFollow->pPrevLink = pMaster;

    if ( pMaster->ContainsCntnt() )
    {
        //Damit ggf. ein Textfluss zustande kommt muss invalidiert werden.
        SwFrm *pInva = pMaster->FindLastLower();
        SWRECTFN( pMaster )
        const long nBottom = (pMaster->*fnRect->fnGetPrtBottom)();
        while ( pInva )
        {
            if( (pInva->Frm().*fnRect->fnBottomDist)( nBottom ) <= 0 )
            {
                pInva->InvalidateSize();
                pInva->Prepare( PREP_CLEAR );
                pInva = pInva->FindPrev();
            }
            else
                pInva = 0;
        }
    }

    if ( pFollow->ContainsCntnt() )
    {
        //Es gibt nur noch den Inhalt des Masters, der Inhalt vom Follow
        //hat keine Frames mehr (sollte immer nur genau ein leerer TxtNode sein).
        SwFrm *pFrm = pFollow->ContainsCntnt();
        OSL_ENSURE( !pFrm->IsTabFrm() && !pFrm->FindNext(), "follow in chain contains content" );
        pFrm->Cut();
        delete pFrm;
    }

    // invalidate accessible relation set (accessibility wrapper)
    ViewShell* pSh = pMaster->getRootFrm()->GetCurrShell();
    if( pSh )
    {
        SwRootFrm* pLayout = pMaster->getRootFrm();
        if( pLayout && pLayout->IsAnyShellAccessible() )
        pSh->Imp()->InvalidateAccessibleRelationSet( pMaster, pFollow );
    }
}

void SwFlyFrm::UnchainFrames( SwFlyFrm *pMaster, SwFlyFrm *pFollow )
{
    pMaster->pNextLink = 0;
    pFollow->pPrevLink = 0;

    if ( pFollow->ContainsCntnt() )
    {
        //Der Master saugt den Inhalt vom Follow auf
        SwLayoutFrm *pUpper = pMaster;
        if ( pUpper->Lower()->IsColumnFrm() )
        {
            pUpper = static_cast<SwLayoutFrm*>(pUpper->GetLastLower());
            pUpper = static_cast<SwLayoutFrm*>(pUpper->Lower()); // der (Column)BodyFrm
            OSL_ENSURE( pUpper && pUpper->IsColBodyFrm(), "Missing ColumnBody" );
        }
        SwFlyFrm *pFoll = pFollow;
        while ( pFoll )
        {
            SwFrm *pTmp = ::SaveCntnt( pFoll );
            if ( pTmp )
                ::RestoreCntnt( pTmp, pUpper, pMaster->FindLastLower(), true );
            pFoll->SetCompletePaint();
            pFoll->InvalidateSize();
            pFoll = pFoll->GetNextLink();
        }
    }

    //Der Follow muss mit seinem eigenen Inhalt versorgt werden.
    const SwFmtCntnt &rCntnt = pFollow->GetFmt()->GetCntnt();
    OSL_ENSURE( rCntnt.GetCntntIdx(), ":-( Kein Inhalt vorbereitet." );
    sal_uLong nIndex = rCntnt.GetCntntIdx()->GetIndex();
    // Lower() bedeutet SwColumnFrm, dieser beinhaltet wieder einen SwBodyFrm
    ::_InsertCnt( pFollow->Lower() ? (SwLayoutFrm*)((SwLayoutFrm*)pFollow->Lower())->Lower()
                                   : (SwLayoutFrm*)pFollow,
                  pFollow->GetFmt()->GetDoc(), ++nIndex );

    // invalidate accessible relation set (accessibility wrapper)
    ViewShell* pSh = pMaster->getRootFrm()->GetCurrShell();
    if( pSh )
    {
        SwRootFrm* pLayout = pMaster->getRootFrm();
        if( pLayout && pLayout->IsAnyShellAccessible() )
        pSh->Imp()->InvalidateAccessibleRelationSet( pMaster, pFollow );
}
}

/*************************************************************************
|*
|*  SwFlyFrm::FindChainNeighbour()
|*
|*************************************************************************/

SwFlyFrm *SwFlyFrm::FindChainNeighbour( SwFrmFmt &rChain, SwFrm *pAnch )
{
    //Wir suchen denjenigen Fly, der in dem selben Bereich steht.
    //Bereiche koennen zunaechst nur Kopf-/Fusszeilen oder Flys sein.

    if ( !pAnch )           //Wenn ein Anchor uebergeben Wurde zaehlt dieser: Ctor!
        pAnch = AnchorFrm();

    SwLayoutFrm *pLay;
    if ( pAnch->IsInFly() )
        pLay = pAnch->FindFlyFrm();
    else
    {
        //FindFooterOrHeader taugt hier nicht, weil evtl. noch keine Verbindung
        //zum Anker besteht.
        pLay = pAnch->GetUpper();
        while ( pLay && !(pLay->GetType() & (FRM_HEADER|FRM_FOOTER)) )
            pLay = pLay->GetUpper();
    }

    SwIterator<SwFlyFrm,SwFmt> aIter( rChain );
    SwFlyFrm *pFly = aIter.First();
    if ( pLay )
    {
        while ( pFly )
        {
            if ( pFly->GetAnchorFrm() )
            {
                if ( pFly->GetAnchorFrm()->IsInFly() )
                {
                    if ( pFly->AnchorFrm()->FindFlyFrm() == pLay )
                        break;
                }
                else if ( pLay == pFly->FindFooterOrHeader() )
                    break;
            }
            pFly = aIter.Next();
        }
    }
    else if ( pFly )
    {
        OSL_ENSURE( !aIter.Next(), "chain with more than one inkarnation" );
    }
    return pFly;
}


/*************************************************************************
|*
|*  SwFlyFrm::FindLastLower()
|*
|*************************************************************************/

SwFrm *SwFlyFrm::FindLastLower()
{
    SwFrm *pRet = ContainsAny();
    if ( pRet && pRet->IsInTab() )
        pRet = pRet->FindTabFrm();
    SwFrm *pNxt = pRet;
    while ( pNxt && IsAnLower( pNxt ) )
    {   pRet = pNxt;
        pNxt = pNxt->FindNext();
    }
    return pRet;
}


/*************************************************************************
|*
|*  SwFlyFrm::FrmSizeChg()
|*
|*************************************************************************/

sal_Bool SwFlyFrm::FrmSizeChg( const SwFmtFrmSize &rFrmSize )
{
    sal_Bool bRet = sal_False;
    SwTwips nDiffHeight = Frm().Height();
    if ( rFrmSize.GetHeightSizeType() == ATT_VAR_SIZE )
        bFixSize = bMinHeight = sal_False;
    else
    {
        if ( rFrmSize.GetHeightSizeType() == ATT_FIX_SIZE )
        {
            bFixSize = sal_True;
            bMinHeight = sal_False;
        }
        else if ( rFrmSize.GetHeightSizeType() == ATT_MIN_SIZE )
        {
            bFixSize = sal_False;
            bMinHeight = sal_True;
        }
        nDiffHeight -= rFrmSize.GetHeight();
    }
    //Wenn der Fly Spalten enthaehlt muessen der Fly und
    //die Spalten schon einmal auf die Wunschwerte gebracht
    //werden, sonst haben wir ein kleines Problem.
    if ( Lower() )
    {
        if ( Lower()->IsColumnFrm() )
        {
            const SwRect aOld( GetObjRectWithSpaces() );
            const Size   aOldSz( Prt().SSize() );
            const SwTwips nDiffWidth = Frm().Width() - rFrmSize.GetWidth();
            aFrm.Height( aFrm.Height() - nDiffHeight );
            aFrm.Width ( aFrm.Width()  - nDiffWidth  );
            // #i68520#
            InvalidateObjRectWithSpaces();
            aPrt.Height( aPrt.Height() - nDiffHeight );
            aPrt.Width ( aPrt.Width()  - nDiffWidth  );
            ChgLowersProp( aOldSz );
            ::Notify( this, FindPageFrm(), aOld );
            bValidPos = sal_False;
            bRet = sal_True;
        }
        else if ( Lower()->IsNoTxtFrm() )
        {
            bFixSize = sal_True;
            bMinHeight = sal_False;
        }
    }
    return bRet;
}

/*************************************************************************
|*
|*  SwFlyFrm::Modify()
|*
|*************************************************************************/

void SwFlyFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
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
        _Invalidate();
        if ( nInvFlags & 0x01 )
        {
            _InvalidatePos();
            // #i68520#
            InvalidateObjRectWithSpaces();
        }
        if ( nInvFlags & 0x02 )
        {
            _InvalidateSize();
            // #i68520#
            InvalidateObjRectWithSpaces();
        }
        if ( nInvFlags & 0x04 )
            _InvalidatePrt();
        if ( nInvFlags & 0x08 )
            SetNotifyBack();
        if ( nInvFlags & 0x10 )
            SetCompletePaint();
        if ( ( nInvFlags & 0x40 ) && Lower() && Lower()->IsNoTxtFrm() )
            ClrContourCache( GetVirtDrawObj() );
        SwRootFrm *pRoot;
        if ( nInvFlags & 0x20 && 0 != (pRoot = getRootFrm()) )
            pRoot->InvalidateBrowseWidth();
        // #i28701#
        if ( nInvFlags & 0x80 )
        {
            // update sorted object lists, the Writer fly frame is registered at.
            UpdateObjInSortedList();
        }

        // #i87645# - reset flags for the layout process (only if something has been invalidated)
        ResetLayoutProcessBools();
    }
}

void SwFlyFrm::_UpdateAttr( const SfxPoolItem *pOld, const SfxPoolItem *pNew,
                            sal_uInt8 &rInvFlags,
                            SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    sal_Bool bClear = sal_True;
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    ViewShell *pSh = getRootFrm()->GetCurrShell();
    switch( nWhich )
    {
        case RES_VERT_ORIENT:
        case RES_HORI_ORIENT:
        // OD 22.09.2003 #i18732# - consider new option 'follow text flow'
        case RES_FOLLOW_TEXT_FLOW:
        {
            //Achtung! _immer_ Aktion in ChgRePos() mitpflegen.
            rInvFlags |= 0x09;
        }
        break;
        // OD 2004-07-01 #i28701# - consider new option 'wrap influence on position'
        case RES_WRAP_INFLUENCE_ON_OBJPOS:
        {
            rInvFlags |= 0x89;
        }
        break;
        case RES_SURROUND:
        {
            // OD 2004-05-13 #i28701# - invalidate position on change of
            // wrapping style.
            //rInvFlags |= 0x40;
            rInvFlags |= 0x41;
            //Der Hintergrund muss benachrichtigt und Invalidiert werden.
            const SwRect aTmp( GetObjRectWithSpaces() );
            NotifyBackground( FindPageFrm(), aTmp, PREP_FLY_ATTR_CHG );

            // Durch eine Umlaufaenderung von rahmengebundenen Rahmen kann eine
            // vertikale Ausrichtung aktiviert/deaktiviert werden => MakeFlyPos
            if( FLY_AT_FLY == GetFmt()->GetAnchor().GetAnchorId() )
                rInvFlags |= 0x09;

            //Ggf. die Kontur am Node loeschen.
            if ( Lower() && Lower()->IsNoTxtFrm() &&
                 !GetFmt()->GetSurround().IsContour() )
            {
                SwNoTxtNode *pNd = (SwNoTxtNode*)((SwCntntFrm*)Lower())->GetNode();
                if ( pNd->HasContour() )
                    pNd->SetContour( 0 );
            }
            // #i28701# - perform reorder of object lists
            // at anchor frame and at page frame.
            rInvFlags |= 0x80;
        }
        break;

        case RES_PROTECT:
            {
            const SvxProtectItem *pP = (SvxProtectItem*)pNew;
            GetVirtDrawObj()->SetMoveProtect( pP->IsPosProtected()   );
            GetVirtDrawObj()->SetResizeProtect( pP->IsSizeProtected() );
            if( pSh )
            {
                SwRootFrm* pLayout = getRootFrm();
                if( pLayout && pLayout->IsAnyShellAccessible() )
                pSh->Imp()->InvalidateAccessibleEditableState( sal_True, this );
            }
            break;
            }

        case RES_COL:
            {
                ChgColumns( *(const SwFmtCol*)pOld, *(const SwFmtCol*)pNew );
                const SwFmtFrmSize &rNew = GetFmt()->GetFrmSize();
                if ( FrmSizeChg( rNew ) )
                    NotifyDrawObj();
                rInvFlags |= 0x1A;
                break;
            }

        case RES_FRM_SIZE:
        case RES_FMT_CHG:
        {
            const SwFmtFrmSize &rNew = GetFmt()->GetFrmSize();
            if ( FrmSizeChg( rNew ) )
                NotifyDrawObj();
            rInvFlags |= 0x7F;
            if ( RES_FMT_CHG == nWhich )
            {
                SwRect aNew( GetObjRectWithSpaces() );
                SwRect aOld( aFrm );
                const SvxULSpaceItem &rUL = ((SwFmtChg*)pOld)->pChangedFmt->GetULSpace();
                aOld.Top( Max( aOld.Top() - long(rUL.GetUpper()), 0L ) );
                aOld.SSize().Height()+= rUL.GetLower();
                const SvxLRSpaceItem &rLR = ((SwFmtChg*)pOld)->pChangedFmt->GetLRSpace();
                aOld.Left  ( Max( aOld.Left() - long(rLR.GetLeft()), 0L ) );
                aOld.SSize().Width() += rLR.GetRight();
                aNew.Union( aOld );
                NotifyBackground( FindPageFrm(), aNew, PREP_CLEAR );

                //Dummer Fall. Bei der Zusweisung einer Vorlage k?nnen wir uns
                //nicht auf das alte Spaltenattribut verlassen. Da diese
                //wenigstens anzahlgemass fuer ChgColumns vorliegen muessen,
                //bleibt uns nur einen temporaeres Attribut zu basteln.
                SwFmtCol aCol;
                if ( Lower() && Lower()->IsColumnFrm() )
                {
                    sal_uInt16 nCol = 0;
                    SwFrm *pTmp = Lower();
                    do
                    {   ++nCol;
                        pTmp = pTmp->GetNext();
                    } while ( pTmp );
                    aCol.Init( nCol, 0, 1000 );
                }
                ChgColumns( aCol, GetFmt()->GetCol() );
            }

            SwFmtURL aURL( GetFmt()->GetURL() );
            if ( aURL.GetMap() )
            {
                const SwFmtFrmSize &rOld = nWhich == RES_FRM_SIZE ?
                                *(SwFmtFrmSize*)pNew :
                                ((SwFmtChg*)pOld)->pChangedFmt->GetFrmSize();
                //#35091# Kann beim Laden von Vorlagen mal 0 sein
                if ( rOld.GetWidth() && rOld.GetHeight() )
                {

                    Fraction aScaleX( rOld.GetWidth(), rNew.GetWidth() );
                    Fraction aScaleY( rOld.GetHeight(), rOld.GetHeight() );
                    aURL.GetMap()->Scale( aScaleX, aScaleY );
                    SwFrmFmt *pFmt = GetFmt();
                    pFmt->LockModify();
                    pFmt->SetFmtAttr( aURL );
                    pFmt->UnlockModify();
                }
            }
            const SvxProtectItem &rP = GetFmt()->GetProtect();
            GetVirtDrawObj()->SetMoveProtect( rP.IsPosProtected()    );
            GetVirtDrawObj()->SetResizeProtect( rP.IsSizeProtected() );

            if ( pSh )
                pSh->InvalidateWindows( Frm() );
            const IDocumentDrawModelAccess* pIDDMA = GetFmt()->getIDocumentDrawModelAccess();
            const sal_uInt8 nId = GetFmt()->GetOpaque().GetValue() ?
                             pIDDMA->GetHeavenId() :
                             pIDDMA->GetHellId();
            GetVirtDrawObj()->SetLayer( nId );

            if ( Lower() )
            {
                //Ggf. die Kontur am Node loeschen.
                if( Lower()->IsNoTxtFrm() &&
                     !GetFmt()->GetSurround().IsContour() )
                {
                    SwNoTxtNode *pNd = (SwNoTxtNode*)((SwCntntFrm*)Lower())->GetNode();
                    if ( pNd->HasContour() )
                        pNd->SetContour( 0 );
                }
                else if( !Lower()->IsColumnFrm() )
                {
                    SwFrm* pFrm = GetLastLower();
                    if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsUndersized() )
                        pFrm->Prepare( PREP_ADJUST_FRM );
                }
            }

            // #i28701# - perform reorder of object lists
            // at anchor frame and at page frame.
            rInvFlags |= 0x80;

            break;
        }
        case RES_UL_SPACE:
        case RES_LR_SPACE:
        {
            rInvFlags |= 0x41;
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
                getRootFrm()->InvalidateBrowseWidth();
            SwRect aNew( GetObjRectWithSpaces() );
            SwRect aOld( aFrm );
            if ( RES_UL_SPACE == nWhich )
            {
                const SvxULSpaceItem &rUL = *(SvxULSpaceItem*)pNew;
                aOld.Top( Max( aOld.Top() - long(rUL.GetUpper()), 0L ) );
                aOld.SSize().Height()+= rUL.GetLower();
            }
            else
            {
                const SvxLRSpaceItem &rLR = *(SvxLRSpaceItem*)pNew;
                aOld.Left  ( Max( aOld.Left() - long(rLR.GetLeft()), 0L ) );
                aOld.SSize().Width() += rLR.GetRight();
            }
            aNew.Union( aOld );
            NotifyBackground( FindPageFrm(), aNew, PREP_CLEAR );
        }
        break;

        case RES_BOX:
        case RES_SHADOW:
            rInvFlags |= 0x17;
            break;

        case RES_FRAMEDIR :
            SetDerivedVert( sal_False );
            SetDerivedR2L( sal_False );
            CheckDirChange();
            break;

        case RES_OPAQUE:
            {
                if ( pSh )
                    pSh->InvalidateWindows( Frm() );

                const IDocumentDrawModelAccess* pIDDMA = GetFmt()->getIDocumentDrawModelAccess();
                const sal_uInt8 nId = ((SvxOpaqueItem*)pNew)->GetValue() ?
                                    pIDDMA->GetHeavenId() :
                                    pIDDMA->GetHellId();
                GetVirtDrawObj()->SetLayer( nId );
                if( pSh )
                {
                    SwRootFrm* pLayout = getRootFrm();
                    if( pLayout && pLayout->IsAnyShellAccessible() )
                {
                    pSh->Imp()->DisposeAccessibleFrm( this );
                    pSh->Imp()->AddAccessibleFrm( this );
                }
                }
                // #i28701# - perform reorder of object lists
                // at anchor frame and at page frame.
                rInvFlags |= 0x80;
            }
            break;

        case RES_URL:
            //Das Interface arbeitet bei Textrahmen auf der Rahmengroesse,
            //die Map muss sich aber auf die FrmSize beziehen
            if ( (!Lower() || !Lower()->IsNoTxtFrm()) &&
                 ((SwFmtURL*)pNew)->GetMap() && ((SwFmtURL*)pOld)->GetMap() )
            {
                const SwFmtFrmSize &rSz = GetFmt()->GetFrmSize();
                if ( rSz.GetHeight() != Frm().Height() ||
                     rSz.GetWidth()  != Frm().Width() )
                {
                    SwFmtURL aURL( GetFmt()->GetURL() );
                    Fraction aScaleX( Frm().Width(),  rSz.GetWidth() );
                    Fraction aScaleY( Frm().Height(), rSz.GetHeight() );
                    aURL.GetMap()->Scale( aScaleX, aScaleY );
                    SwFrmFmt *pFmt = GetFmt();
                    pFmt->LockModify();
                    pFmt->SetFmtAttr( aURL );
                    pFmt->UnlockModify();
                }
            }
            /* Keine Invalidierung notwendig */
            break;

        case RES_CHAIN:
            {
                SwFmtChain *pChain = (SwFmtChain*)pNew;
                if ( pChain->GetNext() )
                {
                    SwFlyFrm *pFollow = FindChainNeighbour( *pChain->GetNext() );
                    if ( GetNextLink() && pFollow != GetNextLink() )
                        SwFlyFrm::UnchainFrames( this, GetNextLink());
                    if ( pFollow )
                    {
                        if ( pFollow->GetPrevLink() &&
                             pFollow->GetPrevLink() != this )
                            SwFlyFrm::UnchainFrames( pFollow->GetPrevLink(),
                                                     pFollow );
                        if ( !GetNextLink() )
                            SwFlyFrm::ChainFrames( this, pFollow );
                    }
                }
                else if ( GetNextLink() )
                    SwFlyFrm::UnchainFrames( this, GetNextLink() );
                if ( pChain->GetPrev() )
                {
                    SwFlyFrm *pMaster = FindChainNeighbour( *pChain->GetPrev() );
                    if ( GetPrevLink() && pMaster != GetPrevLink() )
                        SwFlyFrm::UnchainFrames( GetPrevLink(), this );
                    if ( pMaster )
                    {
                        if ( pMaster->GetNextLink() &&
                             pMaster->GetNextLink() != this )
                            SwFlyFrm::UnchainFrames( pMaster,
                                                     pMaster->GetNextLink() );
                        if ( !GetPrevLink() )
                            SwFlyFrm::ChainFrames( pMaster, this );
                    }
                }
                else if ( GetPrevLink() )
                    SwFlyFrm::UnchainFrames( GetPrevLink(), this );
            }

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
|*                SwFlyFrm::GetInfo()
|*
|*    Beschreibung      erfragt Informationen
|*
*************************************************************************/

    // erfrage vom Modify Informationen
sal_Bool SwFlyFrm::GetInfo( SfxPoolItem & rInfo ) const
{
    if( RES_AUTOFMT_DOCNODE == rInfo.Which() )
        return sal_False;   // es gibt einen FlyFrm also wird er benutzt
    return sal_True;        // weiter suchen
}

/*************************************************************************
|*
|*  SwFlyFrm::_Invalidate()
|*
|*************************************************************************/

void SwFlyFrm::_Invalidate( SwPageFrm *pPage )
{
    InvalidatePage( pPage );
    bNotifyBack = bInvalid = sal_True;

    SwFlyFrm *pFrm;
    if ( GetAnchorFrm() && 0 != (pFrm = AnchorFrm()->FindFlyFrm()) )
    {
        //Gaanz dumm: Wenn der Fly innerhalb eines Fly gebunden ist, der
        //Spalten enthaehlt, sollte das Format von diesem ausgehen.
        if ( !pFrm->IsLocked() && !pFrm->IsColLocked() &&
             pFrm->Lower() && pFrm->Lower()->IsColumnFrm() )
            pFrm->InvalidateSize();
    }

    // #i85216#
    // if vertical position is oriented at a layout frame inside a ghost section,
    // assure that the position is invalidated and that the information about
    // the vertical position oriented frame is cleared
    if ( GetVertPosOrientFrm() && GetVertPosOrientFrm()->IsLayoutFrm() )
    {
        const SwSectionFrm* pSectFrm( GetVertPosOrientFrm()->FindSctFrm() );
        if ( pSectFrm && pSectFrm->GetSection() == 0 )
        {
            InvalidatePos();
            ClearVertPosOrientFrm();
        }
    }
}

/*************************************************************************
|*
|*  SwFlyFrm::ChgRelPos()
|*
|*  Beschreibung        Aenderung der relativen Position, die Position wird
|*      damit automatisch Fix, das Attribut wird entprechend angepasst.
|*
|*************************************************************************/

void SwFlyFrm::ChgRelPos( const Point &rNewPos )
{
    if ( GetCurrRelPos() != rNewPos )
    {
        SwFrmFmt *pFmt = GetFmt();
        const bool bVert = GetAnchorFrm()->IsVertical();
        const SwTwips nNewY = bVert ? rNewPos.X() : rNewPos.Y();
        SwTwips nTmpY = nNewY == LONG_MAX ? 0 : nNewY;
        if( bVert )
            nTmpY = -nTmpY;
        SfxItemSet aSet( pFmt->GetDoc()->GetAttrPool(),
                         RES_VERT_ORIENT, RES_HORI_ORIENT);

        SwFmtVertOrient aVert( pFmt->GetVertOrient() );
        SwTxtFrm *pAutoFrm = NULL;
        // #i34948# - handle also at-page and at-fly anchored
        // Writer fly frames
        const RndStdIds eAnchorType = GetFrmFmt().GetAnchor().GetAnchorId();
        if ( eAnchorType == FLY_AT_PAGE )
        {
            aVert.SetVertOrient( text::VertOrientation::NONE );
            aVert.SetRelationOrient( text::RelOrientation::PAGE_FRAME );
        }
        else if ( eAnchorType == FLY_AT_FLY )
        {
            aVert.SetVertOrient( text::VertOrientation::NONE );
            aVert.SetRelationOrient( text::RelOrientation::FRAME );
        }
        else if ( IsFlyAtCntFrm() || text::VertOrientation::NONE != aVert.GetVertOrient() )
        {
            if( text::RelOrientation::CHAR == aVert.GetRelationOrient() && IsAutoPos() )
            {
                if( LONG_MAX != nNewY )
                {
                    aVert.SetVertOrient( text::VertOrientation::NONE );
                    xub_StrLen nOfs =
                        pFmt->GetAnchor().GetCntntAnchor()->nContent.GetIndex();
                    OSL_ENSURE( GetAnchorFrm()->IsTxtFrm(), "TxtFrm expected" );
                    pAutoFrm = (SwTxtFrm*)GetAnchorFrm();
                    while( pAutoFrm->GetFollow() &&
                           pAutoFrm->GetFollow()->GetOfst() <= nOfs )
                    {
                        if( pAutoFrm == GetAnchorFrm() )
                            nTmpY += pAutoFrm->GetRelPos().Y();
                        nTmpY -= pAutoFrm->GetUpper()->Prt().Height();
                        pAutoFrm = pAutoFrm->GetFollow();
                    }
                    nTmpY = ((SwFlyAtCntFrm*)this)->GetRelCharY(pAutoFrm)-nTmpY;
                }
                else
                    aVert.SetVertOrient( text::VertOrientation::CHAR_BOTTOM );
            }
            else
            {
                aVert.SetVertOrient( text::VertOrientation::NONE );
                aVert.SetRelationOrient( text::RelOrientation::FRAME );
            }
        }
        aVert.SetPos( nTmpY );
        aSet.Put( aVert );

        //Fuer Flys im Cnt ist die horizontale Ausrichtung uninteressant,
        //den sie ist stets 0.
        if ( !IsFlyInCntFrm() )
        {
            const SwTwips nNewX = bVert ? rNewPos.Y() : rNewPos.X();
            SwTwips nTmpX = nNewX == LONG_MAX ? 0 : nNewX;
            SwFmtHoriOrient aHori( pFmt->GetHoriOrient() );
            // #i34948# - handle also at-page and at-fly anchored
            // Writer fly frames
            if ( eAnchorType == FLY_AT_PAGE )
            {
                aHori.SetHoriOrient( text::HoriOrientation::NONE );
                aHori.SetRelationOrient( text::RelOrientation::PAGE_FRAME );
                aHori.SetPosToggle( sal_False );
            }
            else if ( eAnchorType == FLY_AT_FLY )
            {
                aHori.SetHoriOrient( text::HoriOrientation::NONE );
                aHori.SetRelationOrient( text::RelOrientation::FRAME );
                aHori.SetPosToggle( sal_False );
            }
            else if ( IsFlyAtCntFrm() || text::HoriOrientation::NONE != aHori.GetHoriOrient() )
            {
                aHori.SetHoriOrient( text::HoriOrientation::NONE );
                if( text::RelOrientation::CHAR == aHori.GetRelationOrient() && IsAutoPos() )
                {
                    if( LONG_MAX != nNewX )
                    {
                        if( !pAutoFrm )
                        {
                            xub_StrLen nOfs = pFmt->GetAnchor().GetCntntAnchor()
                                          ->nContent.GetIndex();
                            OSL_ENSURE( GetAnchorFrm()->IsTxtFrm(), "TxtFrm expected");
                            pAutoFrm = (SwTxtFrm*)GetAnchorFrm();
                            while( pAutoFrm->GetFollow() &&
                                   pAutoFrm->GetFollow()->GetOfst() <= nOfs )
                                pAutoFrm = pAutoFrm->GetFollow();
                        }
                        nTmpX -= ((SwFlyAtCntFrm*)this)->GetRelCharX(pAutoFrm);
                    }
                }
                else
                    aHori.SetRelationOrient( text::RelOrientation::FRAME );
                aHori.SetPosToggle( sal_False );
            }
            aHori.SetPos( nTmpX );
            aSet.Put( aHori );
        }
        SetCurrRelPos( rNewPos );
        pFmt->GetDoc()->SetAttr( aSet, *pFmt );
    }
}
/*************************************************************************
|*
|*  SwFlyFrm::Format()
|*
|*  Beschreibung:       "Formatiert" den Frame; Frm und PrtArea.
|*                      Die Fixsize wird hier nicht eingestellt.
|*
|*************************************************************************/

void SwFlyFrm::Format( const SwBorderAttrs *pAttrs )
{
    OSL_ENSURE( pAttrs, "FlyFrm::Format, pAttrs ist 0." );

    ColLock();

    if ( !bValidSize )
    {
        if ( Frm().Top() == FAR_AWAY && Frm().Left() == FAR_AWAY )
        {
            //Sicherheitsschaltung wegnehmen (siehe SwFrm::CTor)
            Frm().Pos().X() = Frm().Pos().Y() = 0;
            // #i68520#
            InvalidateObjRectWithSpaces();
        }

        //Breite der Spalten pruefen und ggf. einstellen.
        if ( Lower() && Lower()->IsColumnFrm() )
            AdjustColumns( 0, sal_False );

        bValidSize = sal_True;

        const SwTwips nUL = pAttrs->CalcTopLine()  + pAttrs->CalcBottomLine();
        const SwTwips nLR = pAttrs->CalcLeftLine() + pAttrs->CalcRightLine();
        const SwFmtFrmSize &rFrmSz = GetFmt()->GetFrmSize();
              Size aRelSize( CalcRel( rFrmSz ) );

        OSL_ENSURE( pAttrs->GetSize().Height() != 0 || rFrmSz.GetHeightPercent(), "Hoehe des RahmenAttr ist 0." );
        OSL_ENSURE( pAttrs->GetSize().Width()  != 0 || rFrmSz.GetWidthPercent(), "Breite des RahmenAttr ist 0." );

        SWRECTFN( this )
        if( !HasFixSize() )
        {
            SwTwips nRemaining = 0;

            long nMinHeight = 0;
            if( IsMinHeight() )
                nMinHeight = bVert ? aRelSize.Width() : aRelSize.Height();

            if ( Lower() )
            {
                if ( Lower()->IsColumnFrm() )
                {
                    FormatWidthCols( *pAttrs, nUL, nMinHeight );
                    nRemaining = (Lower()->Frm().*fnRect->fnGetHeight)();
                }
                else
                {
                    SwFrm *pFrm = Lower();
                    while ( pFrm )
                    {
                        nRemaining += (pFrm->Frm().*fnRect->fnGetHeight)();
                        if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsUndersized() )
                            // Dieser TxtFrm waere gern ein bisschen groesser
                            nRemaining += ((SwTxtFrm*)pFrm)->GetParHeight()
                                    - (pFrm->Prt().*fnRect->fnGetHeight)();
                        else if( pFrm->IsSctFrm() && ((SwSectionFrm*)pFrm)->IsUndersized() )
                            nRemaining += ((SwSectionFrm*)pFrm)->Undersize();
                        pFrm = pFrm->GetNext();
                    }
                    // #130878#
                    // Do not keep old height, if content has no height.
                    // The old height could be wrong due to wrong layout cache
                    // and isn't corrected in the further formatting, because
                    // the fly frame doesn't become invalid anymore.
//                    if( !nRemaining )
//                        nRemaining = nOldHeight - nUL;
                }
                if ( GetDrawObjs() )
                {
                    sal_uInt32 nCnt = GetDrawObjs()->Count();
                    SwTwips nTop = (Frm().*fnRect->fnGetTop)();
                    SwTwips nBorder = (Frm().*fnRect->fnGetHeight)() -
                                      (Prt().*fnRect->fnGetHeight)();
                    for ( sal_uInt16 i = 0; i < nCnt; ++i )
                    {
                        SwAnchoredObject* pAnchoredObj = (*GetDrawObjs())[i];
                        if ( pAnchoredObj->ISA(SwFlyFrm) )
                        {
                            SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                            // OD 06.11.2003 #i22305# - consider
                            // only Writer fly frames, which follow the text flow.
                            if ( pFly->IsFlyLayFrm() &&
                                 pFly->Frm().Top() != FAR_AWAY &&
                                 pFly->GetFmt()->GetFollowTextFlow().GetValue() )
                            {
                                SwTwips nDist = -(pFly->Frm().*fnRect->
                                    fnBottomDist)( nTop );
                                if( nDist > nBorder + nRemaining )
                                    nRemaining = nDist - nBorder;
                            }
                        }
                    }
                }
            }

            if( IsMinHeight() && (nRemaining + nUL) < nMinHeight )
                nRemaining = nMinHeight - nUL;
            //Weil das Grow/Shrink der Flys die Groessen nicht direkt
            //einstellt, sondern indirekt per Invalidate ein Format
            //ausloesst, muessen die Groessen hier direkt eingestellt
            //werden. Benachrichtung laeuft bereits mit.
            //Weil bereits haeufiger 0en per Attribut hereinkamen wehre
            //ich mich ab sofort dagegen.
            if ( nRemaining < MINFLY )
                nRemaining = MINFLY;
            (Prt().*fnRect->fnSetHeight)( nRemaining );
            nRemaining -= (Frm().*fnRect->fnGetHeight)();
            (Frm().*fnRect->fnAddBottom)( nRemaining + nUL );
            // #i68520#
            if ( nRemaining + nUL != 0 )
            {
                InvalidateObjRectWithSpaces();
            }
            bValidSize = sal_True;
        }
        else
        {
            bValidSize = sal_True;  //Fixe Frms formatieren sich nicht.
                                //Flys stellen ihre Groesse anhand des Attr ein.
            SwTwips nNewSize = bVert ? aRelSize.Width() : aRelSize.Height();
            nNewSize -= nUL;
            if( nNewSize < MINFLY )
                nNewSize = MINFLY;
            (Prt().*fnRect->fnSetHeight)( nNewSize );
            nNewSize += nUL - (Frm().*fnRect->fnGetHeight)();
            (Frm().*fnRect->fnAddBottom)( nNewSize );
            // #i68520#
            if ( nNewSize != 0 )
            {
                InvalidateObjRectWithSpaces();
            }
        }

        if ( !bFormatHeightOnly )
        {
            OSL_ENSURE( aRelSize == CalcRel( rFrmSz ), "SwFlyFrm::Format CalcRel problem" );
            SwTwips nNewSize = bVert ? aRelSize.Height() : aRelSize.Width();

            if ( rFrmSz.GetWidthSizeType() != ATT_FIX_SIZE )
            {
                // #i9046# Autowidth for fly frames
                const SwTwips nAutoWidth = CalcAutoWidth();
                if ( nAutoWidth )
                {
                    if( ATT_MIN_SIZE == rFrmSz.GetWidthSizeType() )
                        nNewSize = Max( nNewSize - nLR, nAutoWidth );
                    else
                        nNewSize = nAutoWidth;
                }
            }
            else
                nNewSize -= nLR;

            if( nNewSize < MINFLY )
                nNewSize = MINFLY;
            (Prt().*fnRect->fnSetWidth)( nNewSize );
            nNewSize += nLR - (Frm().*fnRect->fnGetWidth)();
            (Frm().*fnRect->fnAddRight)( nNewSize );
            // #i68520#
            if ( nNewSize != 0 )
            {
                InvalidateObjRectWithSpaces();
            }
        }
    }
    ColUnlock();
}

// OD 14.03.2003 #i11760# - change parameter <bNoColl>: type <bool>;
//                          default value = false.
// OD 14.03.2003 #i11760# - add new parameter <bNoCalcFollow> with
//                          default value = false.
// OD 11.04.2003 #108824# - new parameter <bNoCalcFollow> was used by method
//                          <FormatWidthCols(..)> to avoid follow formatting
//                          for text frames. But, unformatted follows causes
//                          problems in method <SwCntntFrm::_WouldFit(..)>,
//                          which assumes that the follows are formatted.
//                          Thus, <bNoCalcFollow> no longer used by <FormatWidthCols(..)>.
//void CalcCntnt( SwLayoutFrm *pLay, sal_Bool bNoColl )
void CalcCntnt( SwLayoutFrm *pLay,
                bool bNoColl,
                bool bNoCalcFollow )
{
    SwSectionFrm* pSect;
    sal_Bool bCollect = sal_False;
    if( pLay->IsSctFrm() )
    {
        pSect = (SwSectionFrm*)pLay;
        if( pSect->IsEndnAtEnd() && !bNoColl )
        {
            bCollect = sal_True;
            SwLayouter::CollectEndnotes( pLay->GetFmt()->GetDoc(), pSect );
        }
        pSect->CalcFtnCntnt();
    }
    else
        pSect = NULL;
    SwFrm *pFrm = pLay->ContainsAny();
    if ( !pFrm )
    {
        if( pSect )
        {
            if( pSect->HasFollow() )
                pFrm = pSect->GetFollow()->ContainsAny();
            if( !pFrm )
            {
                if( pSect->IsEndnAtEnd() )
                {
                    if( bCollect )
                        pLay->GetFmt()->GetDoc()->GetLayouter()->
                            InsertEndnotes( pSect );
                    sal_Bool bLock = pSect->IsFtnLock();
                    pSect->SetFtnLock( sal_True );
                    pSect->CalcFtnCntnt();
                    pSect->CalcFtnCntnt();
                    pSect->SetFtnLock( bLock );
                }
                return;
            }
            pFrm->_InvalidatePos();
        }
        else
            return;
    }
    pFrm->InvalidatePage();

    do
    {
        // local variables to avoid loops caused by anchored object positioning
        SwAnchoredObject* pAgainObj1 = 0;
        SwAnchoredObject* pAgainObj2 = 0;

        // FME 2007-08-30 #i81146# new loop control
        sal_uInt16 nLoopControlRuns = 0;
        const sal_uInt16 nLoopControlMax = 20;
        const SwFrm* pLoopControlCond = 0;

        SwFrm* pLast;
        do
        {
            pLast = pFrm;
            if( pFrm->IsVertical() ?
                ( pFrm->GetUpper()->Prt().Height() != pFrm->Frm().Height() )
                : ( pFrm->GetUpper()->Prt().Width() != pFrm->Frm().Width() ) )
            {
                pFrm->Prepare( PREP_FIXSIZE_CHG );
                pFrm->_InvalidateSize();
            }

            if ( pFrm->IsTabFrm() )
            {
                ((SwTabFrm*)pFrm)->bCalcLowers = sal_True;
                // OD 26.08.2003 #i18103# - lock move backward of follow table,
                // if no section content is formatted or follow table belongs
                // to the section, which content is formatted.
                if ( ((SwTabFrm*)pFrm)->IsFollow() &&
                     ( !pSect || pSect == pFrm->FindSctFrm() ) )
                {
                    ((SwTabFrm*)pFrm)->bLockBackMove = sal_True;
                }
            }

            // OD 14.03.2003 #i11760# - forbid format of follow, if requested.
            if ( bNoCalcFollow && pFrm->IsTxtFrm() )
                static_cast<SwTxtFrm*>(pFrm)->ForbidFollowFormat();

            pFrm->Calc();

            // OD 14.03.2003 #i11760# - reset control flag for follow format.
            if ( pFrm->IsTxtFrm() )
            {
                static_cast<SwTxtFrm*>(pFrm)->AllowFollowFormat();
            }

            // #111937# The keep-attribute can cause the position
            // of the prev to be invalid:
            // OD 2004-03-15 #116560# - Do not consider invalid previous frame
            // due to its keep-attribute, if current frame is a follow or is locked.
            // #i44049# - do not consider invalid previous
            // frame due to its keep-attribute, if it can't move forward.
            // #i57765# - do not consider invalid previous
            // frame, if current frame has a column/page break before attribute.
            SwFrm* pTmpPrev = pFrm->FindPrev();
            SwFlowFrm* pTmpPrevFlowFrm = pTmpPrev && pTmpPrev->IsFlowFrm() ? SwFlowFrm::CastFlowFrm(pTmpPrev) : 0;
            SwFlowFrm* pTmpFlowFrm     = pFrm->IsFlowFrm() ? SwFlowFrm::CastFlowFrm(pFrm) : 0;

            bool bPrevInvalid = pTmpPrevFlowFrm && pTmpFlowFrm &&
                               !pTmpFlowFrm->IsFollow() &&
                               !StackHack::IsLocked() && // #i76382#
                               !pTmpFlowFrm->IsJoinLocked() &&
                               !pTmpPrev->GetValidPosFlag() &&
                                pLay->IsAnLower( pTmpPrev ) &&
                                pTmpPrevFlowFrm->IsKeep( *pTmpPrev->GetAttrSet() ) &&
                                pTmpPrevFlowFrm->IsKeepFwdMoveAllowed();

            // format floating screen objects anchored to the frame.
            bool bRestartLayoutProcess = false;
            if ( !bPrevInvalid && pFrm->GetDrawObjs() && pLay->IsAnLower( pFrm ) )
            {
                bool bAgain = false;
                SwPageFrm* pPageFrm = pFrm->FindPageFrm();
                sal_uInt32 nCnt = pFrm->GetDrawObjs()->Count();
                for ( sal_uInt16 i = 0; i < nCnt; ++i )
                {
                    // #i28701#
                    SwAnchoredObject* pAnchoredObj = (*pFrm->GetDrawObjs())[i];
                    // determine, if anchored object has to be formatted.
                    if ( pAnchoredObj->PositionLocked() )
                    {
                        continue;
                    }

                    // format anchored object
                    if ( pAnchoredObj->IsFormatPossible() )
                    {
                        // #i43737# - no invalidation of
                        // anchored object needed - causes loops for as-character
                        // anchored objects.
                        //pAnchoredObj->InvalidateObjPos();
                        SwRect aRect( pAnchoredObj->GetObjRect() );
                        if ( !SwObjectFormatter::FormatObj( *pAnchoredObj, pFrm, pPageFrm ) )
                        {
                            bRestartLayoutProcess = true;
                            break;
                        }
                        // #i3317# - restart layout process,
                        // if the position of the anchored object is locked now.
                        if ( pAnchoredObj->PositionLocked() )
                        {
                            bRestartLayoutProcess = true;
                            break;
                        }

                        if ( aRect != pAnchoredObj->GetObjRect() )
                        {
                            bAgain = true;
                            if ( pAgainObj2 == pAnchoredObj )
                            {
                                OSL_FAIL( "::CalcCntnt(..) - loop detected, perform attribute changes to avoid the loop" );
                                //Oszillation unterbinden.
                                SwFrmFmt& rFmt = pAnchoredObj->GetFrmFmt();
                                SwFmtSurround aAttr( rFmt.GetSurround() );
                                if( SURROUND_THROUGHT != aAttr.GetSurround() )
                                {
                                    // Bei autopositionierten hilft manchmal nur
                                    // noch, auf Durchlauf zu schalten
                                    if ((rFmt.GetAnchor().GetAnchorId() ==
                                            FLY_AT_CHAR) &&
                                        (SURROUND_PARALLEL ==
                                            aAttr.GetSurround()))
                                    {
                                        aAttr.SetSurround( SURROUND_THROUGHT );
                                    }
                                    else
                                    {
                                        aAttr.SetSurround( SURROUND_PARALLEL );
                                    }
                                    rFmt.LockModify();
                                    rFmt.SetFmtAttr( aAttr );
                                    rFmt.UnlockModify();
                                }
                            }
                            else
                            {
                                if ( pAgainObj1 == pAnchoredObj )
                                    pAgainObj2 = pAnchoredObj;
                                pAgainObj1 = pAnchoredObj;
                            }
                        }

                        if ( !pFrm->GetDrawObjs() )
                            break;
                        if ( pFrm->GetDrawObjs()->Count() < nCnt )
                        {
                            --i;
                            --nCnt;
                        }
                    }
                }

                // #i28701# - restart layout process, if
                // requested by floating screen object formatting
                if ( bRestartLayoutProcess )
                {
                    pFrm = pLay->ContainsAny();
                    pAgainObj1 = 0L;
                    pAgainObj2 = 0L;
                    continue;
                }

                // OD 2004-05-17 #i28701# - format anchor frame after its objects
                // are formatted, if the wrapping style influence has to be considered.
                if ( pLay->GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) )
                {
                    pFrm->Calc();
                }

                if ( bAgain )
                {
                    pFrm = pLay->ContainsCntnt();
                    if ( pFrm && pFrm->IsInTab() )
                        pFrm = pFrm->FindTabFrm();
                    if( pFrm && pFrm->IsInSct() )
                    {
                        SwSectionFrm* pTmp = pFrm->FindSctFrm();
                        if( pTmp != pLay && pLay->IsAnLower( pTmp ) )
                            pFrm = pTmp;
                    }

                    if ( pFrm == pLoopControlCond )
                        ++nLoopControlRuns;
                    else
                    {
                        nLoopControlRuns = 0;
                        pLoopControlCond = pFrm;
                    }

                    if ( nLoopControlRuns < nLoopControlMax )
                        continue;

                    OSL_FAIL( "LoopControl in CalcCntnt" );
                }
            }
            if ( pFrm->IsTabFrm() )
            {
                if ( ((SwTabFrm*)pFrm)->IsFollow() )
                    ((SwTabFrm*)pFrm)->bLockBackMove = sal_False;
            }

            pFrm = bPrevInvalid ? pTmpPrev : pFrm->FindNext();
            if( !bPrevInvalid && pFrm && pFrm->IsSctFrm() && pSect )
            {
                // Es koennen hier leere SectionFrms herumspuken
                while( pFrm && pFrm->IsSctFrm() && !((SwSectionFrm*)pFrm)->GetSection() )
                    pFrm = pFrm->FindNext();
                // Wenn FindNext den Follow des urspruenglichen Bereichs liefert,
                // wollen wir mit dessen Inhalt weitermachen, solange dieser
                // zurueckfliesst.
                if( pFrm && pFrm->IsSctFrm() && ( pFrm == pSect->GetFollow() ||
                    ((SwSectionFrm*)pFrm)->IsAnFollow( pSect ) ) )
                {
                    pFrm = ((SwSectionFrm*)pFrm)->ContainsAny();
                    if( pFrm )
                        pFrm->_InvalidatePos();
                }
            }
            // Im pLay bleiben, Ausnahme, bei SectionFrms mit Follow wird der erste
            // CntntFrm des Follows anformatiert, damit er die Chance erhaelt, in
            // pLay zu landen. Solange diese Frames in pLay landen, geht's weiter.
        } while ( pFrm &&
                  ( pLay->IsAnLower( pFrm ) ||
                    ( pSect &&
                      ( ( pSect->HasFollow() &&
                          ( pLay->IsAnLower( pLast ) ||
                            ( pLast->IsInSct() &&
                              pLast->FindSctFrm()->IsAnFollow(pSect) ) ) &&
                          pSect->GetFollow()->IsAnLower( pFrm )  ) ||
                        ( pFrm->IsInSct() &&
                          pFrm->FindSctFrm()->IsAnFollow( pSect ) ) ) ) ) );
        if( pSect )
        {
            if( bCollect )
            {
                pLay->GetFmt()->GetDoc()->GetLayouter()->InsertEndnotes(pSect);
                pSect->CalcFtnCntnt();
            }
            if( pSect->HasFollow() )
            {
                SwSectionFrm* pNxt = pSect->GetFollow();
                while( pNxt && !pNxt->ContainsCntnt() )
                    pNxt = pNxt->GetFollow();
                if( pNxt )
                    pNxt->CalcFtnCntnt();
            }
            if( bCollect )
            {
                pFrm = pLay->ContainsAny();
                bCollect = sal_False;
                if( pFrm )
                    continue;
            }
        }
        break;
    }
    while( sal_True );
}

/*************************************************************************
|*
|*  SwFlyFrm::MakeFlyPos()
|*
|*************************************************************************/
// OD 2004-03-23 #i26791#
//void SwFlyFrm::MakeFlyPos()
void SwFlyFrm::MakeObjPos()
{
    if ( !bValidPos )
    {
        bValidPos = sal_True;

        // OD 29.10.2003 #113049# - use new class to position object
        GetAnchorFrm()->Calc();
        objectpositioning::SwToLayoutAnchoredObjectPosition
                aObjPositioning( *GetVirtDrawObj() );
        aObjPositioning.CalcPosition();

        // #i58280#
        // update relative position
        SetCurrRelPos( aObjPositioning.GetRelPos() );

        SWRECTFN( GetAnchorFrm() );
        aFrm.Pos( aObjPositioning.GetRelPos() );
        aFrm.Pos() += (GetAnchorFrm()->Frm().*fnRect->fnGetPos)();
        // #i69335#
        InvalidateObjRectWithSpaces();
    }
}

/*************************************************************************
|*
|*  SwFlyFrm::MakePrtArea()
|*
|*************************************************************************/
void SwFlyFrm::MakePrtArea( const SwBorderAttrs &rAttrs )
{

    if ( !bValidPrtArea )
    {
        bValidPrtArea = sal_True;

        // OD 31.07.2003 #110978# - consider vertical layout
        SWRECTFN( this )
        (this->*fnRect->fnSetXMargins)( rAttrs.CalcLeftLine(),
                                        rAttrs.CalcRightLine() );
        (this->*fnRect->fnSetYMargins)( rAttrs.CalcTopLine(),
                                        rAttrs.CalcBottomLine() );
    }
}

/*************************************************************************
|*
|*  SwFlyFrm::_Grow(), _Shrink()
|*
|*************************************************************************/

SwTwips SwFlyFrm::_Grow( SwTwips nDist, sal_Bool bTst )
{
    SWRECTFN( this )
    if ( Lower() && !IsColLocked() && !HasFixSize() )
    {
        SwTwips nSize = (Frm().*fnRect->fnGetHeight)();
        if( nSize > 0 && nDist > ( LONG_MAX - nSize ) )
            nDist = LONG_MAX - nSize;

        if ( nDist <= 0L )
            return 0L;

        if ( Lower()->IsColumnFrm() )
        {   //Bei Spaltigkeit ubernimmt das Format die Kontrolle ueber
            //das Wachstum (wg. des Ausgleichs).
            if ( !bTst )
            {
                // #i28701# - unlock position of Writer fly frame
                UnlockPosition();
                _InvalidatePos();
                InvalidateSize();
            }
            return 0L;
        }

        if ( !bTst )
        {
            const SwRect aOld( GetObjRectWithSpaces() );
            _InvalidateSize();
            const sal_Bool bOldLock = bLocked;
            Unlock();
            if ( IsFlyFreeFrm() )
            {
                // #i37068# - no format of position here
                // and prevent move in method <CheckClip(..)>.
                // This is needed to prevent layout loop caused by nested
                // Writer fly frames - inner Writer fly frames format its
                // anchor, which grows/shrinks the outer Writer fly frame.
                // Note: position will be invalidated below.
                bValidPos = sal_True;
                // #i55416#
                // Suppress format of width for autowidth frame, because the
                // format of the width would call <SwTxtFrm::CalcFitToContent()>
                // for the lower frame, which initiated this grow.
                const sal_Bool bOldFormatHeightOnly = bFormatHeightOnly;
                const SwFmtFrmSize& rFrmSz = GetFmt()->GetFrmSize();
                if ( rFrmSz.GetWidthSizeType() != ATT_FIX_SIZE )
                {
                    bFormatHeightOnly = sal_True;
                }
                static_cast<SwFlyFreeFrm*>(this)->SetNoMoveOnCheckClip( true );
                ((SwFlyFreeFrm*)this)->SwFlyFreeFrm::MakeAll();
                static_cast<SwFlyFreeFrm*>(this)->SetNoMoveOnCheckClip( false );
                // #i55416#
                if ( rFrmSz.GetWidthSizeType() != ATT_FIX_SIZE )
                {
                    bFormatHeightOnly = bOldFormatHeightOnly;
                }
            }
            else
                MakeAll();
            _InvalidateSize();
            InvalidatePos();
            if ( bOldLock )
                Lock();
            const SwRect aNew( GetObjRectWithSpaces() );
            if ( aOld != aNew )
                ::Notify( this, FindPageFrm(), aOld );
            return (aNew.*fnRect->fnGetHeight)()-(aOld.*fnRect->fnGetHeight)();
        }
        return nDist;
    }
    return 0L;
}

SwTwips SwFlyFrm::_Shrink( SwTwips nDist, sal_Bool bTst )
{
    if( Lower() && !IsColLocked() && !HasFixSize() && !IsNoShrink() )
    {
        SWRECTFN( this )
        SwTwips nHeight = (Frm().*fnRect->fnGetHeight)();
        if ( nDist > nHeight )
            nDist = nHeight;

        SwTwips nVal = nDist;
        if ( IsMinHeight() )
        {
            const SwFmtFrmSize& rFmtSize = GetFmt()->GetFrmSize();
            SwTwips nFmtHeight = bVert ? rFmtSize.GetWidth() : rFmtSize.GetHeight();

            nVal = Min( nDist, nHeight - nFmtHeight );
        }

        if ( nVal <= 0L )
            return 0L;

        if ( Lower()->IsColumnFrm() )
        {   //Bei Spaltigkeit ubernimmt das Format die Kontrolle ueber
            //das Wachstum (wg. des Ausgleichs).
            if ( !bTst )
            {
                SwRect aOld( GetObjRectWithSpaces() );
                (Frm().*fnRect->fnSetHeight)( nHeight - nVal );
                // #i68520#
                if ( nHeight - nVal != 0 )
                {
                    InvalidateObjRectWithSpaces();
                }
                nHeight = (Prt().*fnRect->fnGetHeight)();
                (Prt().*fnRect->fnSetHeight)( nHeight - nVal );
                _InvalidatePos();
                InvalidateSize();
                ::Notify( this, FindPageFrm(), aOld );
                NotifyDrawObj();
                if ( GetAnchorFrm()->IsInFly() )
                    AnchorFrm()->FindFlyFrm()->Shrink( nDist, bTst );
            }
            return 0L;
        }

        if ( !bTst )
        {
            const SwRect aOld( GetObjRectWithSpaces() );
            _InvalidateSize();
            const sal_Bool bOldLocked = bLocked;
            Unlock();
            if ( IsFlyFreeFrm() )
            {
                // #i37068# - no format of position here
                // and prevent move in method <CheckClip(..)>.
                // This is needed to prevent layout loop caused by nested
                // Writer fly frames - inner Writer fly frames format its
                // anchor, which grows/shrinks the outer Writer fly frame.
                // Note: position will be invalidated below.
                bValidPos = sal_True;
                // #i55416#
                // Suppress format of width for autowidth frame, because the
                // format of the width would call <SwTxtFrm::CalcFitToContent()>
                // for the lower frame, which initiated this shrink.
                const sal_Bool bOldFormatHeightOnly = bFormatHeightOnly;
                const SwFmtFrmSize& rFrmSz = GetFmt()->GetFrmSize();
                if ( rFrmSz.GetWidthSizeType() != ATT_FIX_SIZE )
                {
                    bFormatHeightOnly = sal_True;
                }
                static_cast<SwFlyFreeFrm*>(this)->SetNoMoveOnCheckClip( true );
                ((SwFlyFreeFrm*)this)->SwFlyFreeFrm::MakeAll();
                static_cast<SwFlyFreeFrm*>(this)->SetNoMoveOnCheckClip( false );
                // #i55416#
                if ( rFrmSz.GetWidthSizeType() != ATT_FIX_SIZE )
                {
                    bFormatHeightOnly = bOldFormatHeightOnly;
                }
            }
            else
                MakeAll();
            _InvalidateSize();
            InvalidatePos();
            if ( bOldLocked )
                Lock();
            const SwRect aNew( GetObjRectWithSpaces() );
            if ( aOld != aNew )
            {
                ::Notify( this, FindPageFrm(), aOld );
                if ( GetAnchorFrm()->IsInFly() )
                    AnchorFrm()->FindFlyFrm()->Shrink( nDist, bTst );
            }
            return (aOld.*fnRect->fnGetHeight)() -
                   (aNew.*fnRect->fnGetHeight)();
        }
        return nVal;
    }
    return 0L;
}

/*************************************************************************
|*
|*  SwFlyFrm::ChgSize()
|*
|*************************************************************************/

Size SwFlyFrm::ChgSize( const Size& aNewSize )
{
    // #i53298#
    // If the fly frame anchored at-paragraph or at-character contains an OLE
    // object, assure that the new size fits into the current clipping area
    // of the fly frame
    Size aAdjustedNewSize( aNewSize );
    {
        if ( dynamic_cast<SwFlyAtCntFrm*>(this) &&
             Lower() && dynamic_cast<SwNoTxtFrm*>(Lower()) &&
             static_cast<SwNoTxtFrm*>(Lower())->GetNode()->GetOLENode() )
        {
            SwRect aClipRect;
            ::CalcClipRect( GetVirtDrawObj(), aClipRect, sal_False );
            if ( aAdjustedNewSize.Width() > aClipRect.Width() )
            {
                aAdjustedNewSize.setWidth( aClipRect.Width() );
            }
            if ( aAdjustedNewSize.Height() > aClipRect.Height() )
            {
                aAdjustedNewSize.setWidth( aClipRect.Height() );
            }
        }
    }
    if ( aAdjustedNewSize != Frm().SSize() )
    {
        SwFrmFmt *pFmt = GetFmt();
        SwFmtFrmSize aSz( pFmt->GetFrmSize() );
        aSz.SetWidth( aAdjustedNewSize.Width() );
        // #i53298# - no tolerance any more.
        // If it reveals that the tolerance is still needed, then suppress a
        // <SetAttr> call, if <aSz> equals the current <SwFmtFrmSize> attribute.
//        if ( Abs(aAdjustedNewSize.Height() - aSz.GetHeight()) > 1 )
        aSz.SetHeight( aAdjustedNewSize.Height() );
        // uebers Doc fuers Undo!
        pFmt->GetDoc()->SetAttr( aSz, *pFmt );
        return aSz.GetSize();
    }
    else
        return Frm().SSize();
}

/*************************************************************************
|*
|*  SwFlyFrm::IsLowerOf()
|*
|*************************************************************************/

sal_Bool SwFlyFrm::IsLowerOf( const SwLayoutFrm* pUpperFrm ) const
{
    OSL_ENSURE( GetAnchorFrm(), "8-( Fly is lost in Space." );
    const SwFrm* pFrm = GetAnchorFrm();
    do
    {
        if ( pFrm == pUpperFrm )
            return sal_True;
        pFrm = pFrm->IsFlyFrm()
               ? ((const SwFlyFrm*)pFrm)->GetAnchorFrm()
               : pFrm->GetUpper();
    } while ( pFrm );
    return sal_False;
}

/*************************************************************************
|*
|*  SwFlyFrm::Cut()
|*
|*************************************************************************/

void SwFlyFrm::Cut()
{
}

/*************************************************************************
|*
|*  SwFrm::AppendFly(), RemoveFly()
|*
|*************************************************************************/

void SwFrm::AppendFly( SwFlyFrm *pNew )
{
    if ( !pDrawObjs )
        pDrawObjs = new SwSortedObjs();
    pDrawObjs->Insert( *pNew );
    pNew->ChgAnchorFrm( this );

    //Bei der Seite anmelden; kann sein, dass noch keine da ist - die
    //Anmeldung wird dann in SwPageFrm::PreparePage durch gefuehrt.
    SwPageFrm *pPage = FindPageFrm();
    if ( pPage )
    {
        if ( pNew->IsFlyAtCntFrm() && pNew->Frm().Top() == FAR_AWAY )
        {
            //Versuch die Seitenformatierung von neuen Dokumenten etwas
            //guenstiger zu gestalten.
            //Wir haengen die Flys erstenmal nach hinten damit sie bei heftigem
            //Fluss der Anker nicht unoetig oft formatiert werden.
            //Damit man noch brauchbar an das Ende des Dokumentes springen
            //kann werden die Flys nicht ganz an das Ende gehaengt.
            SwRootFrm *pRoot = (SwRootFrm*)pPage->GetUpper();
            if( !SwLayHelper::CheckPageFlyCache( pPage, pNew ) )
            {
                SwPageFrm *pTmp = pRoot->GetLastPage();
                if ( pTmp->GetPhyPageNum() > 30 )
                {
                    for ( sal_uInt16 i = 0; i < 10; ++i )
                    {
                        pTmp = (SwPageFrm*)pTmp->GetPrev();
                        if( pTmp->GetPhyPageNum() <= pPage->GetPhyPageNum() )
                            break; // damit wir nicht vor unserem Anker landen
                    }
                    if ( pTmp->IsEmptyPage() )
                        pTmp = (SwPageFrm*)pTmp->GetPrev();
                    pPage = pTmp;
                }
            }
            pPage->AppendFlyToPage( pNew );
        }
        else
            pPage->AppendFlyToPage( pNew );
    }
}

void SwFrm::RemoveFly( SwFlyFrm *pToRemove )
{
    //Bei der Seite Abmelden - kann schon passiert sein weil die Seite
    //bereits destruiert wurde.
    SwPageFrm *pPage = pToRemove->FindPageFrm();
    if ( pPage && pPage->GetSortedObjs() )
    {
        pPage->RemoveFlyFromPage( pToRemove );
    }
    // #i73201#
    else
    {
        if ( pToRemove->IsAccessibleFrm() &&
             pToRemove->GetFmt() &&
             !pToRemove->IsFlyInCntFrm() )
        {
            SwRootFrm *pRootFrm = getRootFrm();
            if( pRootFrm && pRootFrm->IsAnyShellAccessible() )
            {
                ViewShell *pVSh = pRootFrm->GetCurrShell();
                if( pVSh && pVSh->Imp() )
                {
                    pVSh->Imp()->DisposeAccessibleFrm( pToRemove );
                }
            }
        }
    }

    pDrawObjs->Remove( *pToRemove );
    if ( !pDrawObjs->Count() )
        DELETEZ( pDrawObjs );

    pToRemove->ChgAnchorFrm( 0 );

    if ( !pToRemove->IsFlyInCntFrm() && GetUpper() && IsInTab() )//MA_FLY_HEIGHT
        GetUpper()->InvalidateSize();
}

/*************************************************************************
|*
|*  SwFrm::AppendDrawObj(), RemoveDrawObj()
|*
|*  --> OD 2004-07-06 #i28701# - new methods
|*
|*************************************************************************/
void SwFrm::AppendDrawObj( SwAnchoredObject& _rNewObj )
{
    if ( !_rNewObj.ISA(SwAnchoredDrawObject) )
    {
        OSL_FAIL( "SwFrm::AppendDrawObj(..) - anchored object of unexcepted type -> object not appended" );
        return;
    }

    if ( !_rNewObj.GetDrawObj()->ISA(SwDrawVirtObj) &&
         _rNewObj.GetAnchorFrm() && _rNewObj.GetAnchorFrm() != this )
    {
        // perform disconnect from layout, if 'master' drawing object is appended
        // to a new frame.
        static_cast<SwDrawContact*>(::GetUserCall( _rNewObj.GetDrawObj() ))->
                                                DisconnectFromLayout( false );
    }

    if ( _rNewObj.GetAnchorFrm() != this )
    {
        if ( !pDrawObjs )
            pDrawObjs = new SwSortedObjs();
        pDrawObjs->Insert( _rNewObj );
        _rNewObj.ChgAnchorFrm( this );
    }

    // #i113730#
    // Assure the control objects and group objects containing controls are on the control layer
    if ( ::CheckControlLayer( _rNewObj.DrawObj() ) )
    {
        const IDocumentDrawModelAccess* pIDDMA = getIDocumentDrawModelAccess();
        const SdrLayerID aCurrentLayer(_rNewObj.DrawObj()->GetLayer());
        const SdrLayerID aControlLayerID(pIDDMA->GetControlsId());
        const SdrLayerID aInvisibleControlLayerID(pIDDMA->GetInvisibleControlsId());

        if(aCurrentLayer != aControlLayerID && aCurrentLayer != aInvisibleControlLayerID)
        {
            if ( aCurrentLayer == pIDDMA->GetInvisibleHellId() ||
                 aCurrentLayer == pIDDMA->GetInvisibleHeavenId() )
            {
                _rNewObj.DrawObj()->SetLayer(aInvisibleControlLayerID);
            }
            else
            {
                _rNewObj.DrawObj()->SetLayer(aControlLayerID);
            }
        }
    }

    // no direct positioning needed, but invalidate the drawing object position
    _rNewObj.InvalidateObjPos();

    // register at page frame
    SwPageFrm* pPage = FindPageFrm();
    if ( pPage )
    {
        pPage->AppendDrawObjToPage( _rNewObj );
    }

    // Notify accessible layout.
    ViewShell* pSh = getRootFrm()->GetCurrShell();
    if( pSh )
    {
        SwRootFrm* pLayout = getRootFrm();
        if( pLayout && pLayout->IsAnyShellAccessible() )
        pSh->Imp()->AddAccessibleObj( _rNewObj.GetDrawObj() );
    }
}

void SwFrm::RemoveDrawObj( SwAnchoredObject& _rToRemoveObj )
{
    // Notify accessible layout.
    ViewShell* pSh = getRootFrm()->GetCurrShell();
    if( pSh )
    {
        SwRootFrm* pLayout = getRootFrm();
        if( pLayout && pLayout->IsAnyShellAccessible() )
        pSh->Imp()->DisposeAccessibleObj( _rToRemoveObj.GetDrawObj() );
    }

    // deregister from page frame
    SwPageFrm* pPage = _rToRemoveObj.GetPageFrm();
    if ( pPage && pPage->GetSortedObjs() )
        pPage->RemoveDrawObjFromPage( _rToRemoveObj );

    pDrawObjs->Remove( _rToRemoveObj );
    if ( !pDrawObjs->Count() )
        DELETEZ( pDrawObjs );

    _rToRemoveObj.ChgAnchorFrm( 0 );
}

/*************************************************************************
|*
|*  SwFrm::InvalidateObjs()
|*
|*************************************************************************/
// #i28701# - change purpose of method and adjust its name
void SwFrm::InvalidateObjs( const bool _bInvaPosOnly,
                            const bool _bNoInvaOfAsCharAnchoredObjs )
{
    if ( GetDrawObjs() )
    {
        // #i26945# - determine page the frame is on,
        // in order to check, if anchored object is registered at the same
        // page.
        const SwPageFrm* pPageFrm = FindPageFrm();
        // #i28701# - re-factoring
        sal_uInt32 i = 0;
        for ( ; i < GetDrawObjs()->Count(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*GetDrawObjs())[i];
            if ( _bNoInvaOfAsCharAnchoredObjs &&
                 (pAnchoredObj->GetFrmFmt().GetAnchor().GetAnchorId()
                    == FLY_AS_CHAR) )
            {
                continue;
            }
            // #i26945# - no invalidation, if anchored object
            // isn't registered at the same page and instead is registered at
            // the page, where its anchor character text frame is on.
            if ( pAnchoredObj->GetPageFrm() &&
                 pAnchoredObj->GetPageFrm() != pPageFrm )
            {
                SwTxtFrm* pAnchorCharFrm = pAnchoredObj->FindAnchorCharFrm();
                if ( pAnchorCharFrm &&
                     pAnchoredObj->GetPageFrm() == pAnchorCharFrm->FindPageFrm() )
                {
                    continue;
                }
                // #115759# - unlock its position, if anchored
                // object isn't registered at the page, where its anchor
                // character text frame is on, respectively if it has no
                // anchor character text frame.
                else
                {
                    pAnchoredObj->UnlockPosition();
                }
            }
            // #i51474# - reset flag, that anchored object
            // has cleared environment, and unlock its position, if the anchored
            // object is registered at the same page as the anchor frame is on.
            if ( pAnchoredObj->ClearedEnvironment() &&
                 pAnchoredObj->GetPageFrm() &&
                 pAnchoredObj->GetPageFrm() == pPageFrm )
            {
                pAnchoredObj->UnlockPosition();
                pAnchoredObj->SetClearedEnvironment( false );
            }
            // distinguish between writer fly frames and drawing objects
            if ( pAnchoredObj->ISA(SwFlyFrm) )
            {
                SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                pFly->_Invalidate();
                pFly->_InvalidatePos();
                if ( !_bInvaPosOnly )
                    pFly->_InvalidateSize();
            }
            else
            {
                pAnchoredObj->InvalidateObjPos();
            } // end of distinction between writer fly frames and drawing objects

        } // end of loop on objects, which are connected to the frame
    }
}

/*************************************************************************
|*
|*  SwLayoutFrm::NotifyLowerObjs()
|*
|*************************************************************************/
// #i28701# - change purpose of method and its name
// #i26945# - correct check, if anchored object is a lower
// of the layout frame. E.g., anchor character text frame can be a follow text
// frame.
// #i44016# - add parameter <_bUnlockPosOfObjs> to
// force an unlockposition call for the lower objects.
void SwLayoutFrm::NotifyLowerObjs( const bool _bUnlockPosOfObjs )
{
    // invalidate lower floating screen objects
    SwPageFrm* pPageFrm = FindPageFrm();
    if ( pPageFrm && pPageFrm->GetSortedObjs() )
    {
        SwSortedObjs& rObjs = *(pPageFrm->GetSortedObjs());
        for ( sal_uInt32 i = 0; i < rObjs.Count(); ++i )
        {
            SwAnchoredObject* pObj = rObjs[i];
            // #i26945# - check, if anchored object is a lower
            // of the layout frame is changed to check, if its anchor frame
            // is a lower of the layout frame.
            // determine the anchor frame - usually it's the anchor frame,
            // for at-character/as-character anchored objects the anchor character
            // text frame is taken.
            const SwFrm* pAnchorFrm = pObj->GetAnchorFrmContainingAnchPos();
            if ( pObj->ISA(SwFlyFrm) )
            {
                SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pObj);

                if ( pFly->Frm().Left() == FAR_AWAY )
                    continue;

                if ( pFly->IsAnLower( this ) )
                    continue;

                // #i26945# - use <pAnchorFrm> to check, if
                // fly frame is lower of layout frame resp. if fly frame is
                // at a different page registered as its anchor frame is on.
                const bool bLow = IsAnLower( pAnchorFrm );
                if ( bLow || pAnchorFrm->FindPageFrm() != pPageFrm )
                {
                    pFly->_Invalidate( pPageFrm );
                    if ( !bLow || pFly->IsFlyAtCntFrm() )
                    {
                        // #i44016#
                        if ( _bUnlockPosOfObjs )
                        {
                            pFly->UnlockPosition();
                        }
                        pFly->_InvalidatePos();
                    }
                    else
                        pFly->_InvalidatePrt();
                }
            }
            else
            {
                OSL_ENSURE( pObj->ISA(SwAnchoredDrawObject),
                        "<SwLayoutFrm::NotifyFlys() - anchored object of unexcepted type" );
                // #i26945# - use <pAnchorFrm> to check, if
                // fly frame is lower of layout frame resp. if fly frame is
                // at a different page registered as its anchor frame is on.
                if ( IsAnLower( pAnchorFrm ) ||
                     pAnchorFrm->FindPageFrm() != pPageFrm )
                {
                    // #i44016#
                    if ( _bUnlockPosOfObjs )
                    {
                        pObj->UnlockPosition();
                    }
                    pObj->InvalidateObjPos();
                }
            }
        }
    }
}

/*************************************************************************
|*
|*  SwFlyFrm::NotifyDrawObj()
|*
|*************************************************************************/

void SwFlyFrm::NotifyDrawObj()
{
    SwVirtFlyDrawObj* pObj = GetVirtDrawObj();
    pObj->SetRect();
    pObj->SetRectsDirty();
    pObj->SetChanged();
    pObj->BroadcastObjectChange();
    if ( GetFmt()->GetSurround().IsContour() )
        ClrContourCache( pObj );
}

/*************************************************************************
|*
|*  SwFlyFrm::CalcRel()
|*
|*************************************************************************/

Size SwFlyFrm::CalcRel( const SwFmtFrmSize &rSz ) const
{
    Size aRet( rSz.GetSize() );

    const SwFrm *pRel = IsFlyLayFrm() ? GetAnchorFrm() : GetAnchorFrm()->GetUpper();
    if( pRel ) // LAYER_IMPL
    {
        long nRelWidth = LONG_MAX, nRelHeight = LONG_MAX;
        const ViewShell *pSh = getRootFrm()->GetCurrShell();
        if ( ( pRel->IsBodyFrm() || pRel->IsPageFrm() ) &&
             pSh && pSh->GetViewOptions()->getBrowseMode() &&
             pSh->VisArea().HasArea() )
        {
            nRelWidth  = pSh->GetBrowseWidth();
            nRelHeight = pSh->VisArea().Height();
            Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
            long nDiff = nRelWidth - pRel->Prt().Width();
            if ( nDiff > 0 )
                nRelWidth -= nDiff;
            nRelHeight -= 2*aBorder.Height();
            nDiff = nRelHeight - pRel->Prt().Height();
            if ( nDiff > 0 )
                nRelHeight -= nDiff;
        }
        nRelWidth  = Min( nRelWidth,  pRel->Prt().Width() );
        nRelHeight = Min( nRelHeight, pRel->Prt().Height() );
        if( !pRel->IsPageFrm() )
        {
            const SwPageFrm* pPage = FindPageFrm();
            if( pPage )
            {
                nRelWidth  = Min( nRelWidth,  pPage->Prt().Width() );
                nRelHeight = Min( nRelHeight, pPage->Prt().Height() );
            }
        }

        if ( rSz.GetWidthPercent() && rSz.GetWidthPercent() != 0xFF )
            aRet.Width() = nRelWidth * rSz.GetWidthPercent() / 100;
        if ( rSz.GetHeightPercent() && rSz.GetHeightPercent() != 0xFF )
            aRet.Height() = nRelHeight * rSz.GetHeightPercent() / 100;

        if ( rSz.GetWidthPercent() == 0xFF )
        {
            aRet.Width() *= aRet.Height();
            aRet.Width() /= rSz.GetHeight();
        }
        else if ( rSz.GetHeightPercent() == 0xFF )
        {
            aRet.Height() *= aRet.Width();
            aRet.Height() /= rSz.GetWidth();
        }
    }
    return aRet;
}

/*************************************************************************
|*
|*  SwFlyFrm::CalcAutoWidth()
|*
|*************************************************************************/

static SwTwips lcl_CalcAutoWidth( const SwLayoutFrm& rFrm )
{
    SwTwips nRet = 0;
    SwTwips nMin = 0;
    const SwFrm* pFrm = rFrm.Lower();

    // No autowidth defined for columned frames
    if ( !pFrm || pFrm->IsColumnFrm() )
        return nRet;

    while ( pFrm )
    {
        if ( pFrm->IsSctFrm() )
        {
            nMin = lcl_CalcAutoWidth( *(SwSectionFrm*)pFrm );
        }
        if ( pFrm->IsTxtFrm() )
        {
            nMin = ((SwTxtFrm*)pFrm)->CalcFitToContent();
            const SvxLRSpaceItem &rSpace =
                ((SwTxtFrm*)pFrm)->GetTxtNode()->GetSwAttrSet().GetLRSpace();
            nMin += rSpace.GetRight() + rSpace.GetTxtLeft() + rSpace.GetTxtFirstLineOfst();
        }
        else if ( pFrm->IsTabFrm() )
        {
            const SwFmtFrmSize& rTblFmtSz = ((SwTabFrm*)pFrm)->GetTable()->GetFrmFmt()->GetFrmSize();
            if ( USHRT_MAX == rTblFmtSz.GetSize().Width() ||
                 text::HoriOrientation::NONE == ((SwTabFrm*)pFrm)->GetFmt()->GetHoriOrient().GetHoriOrient() )
            {
                const SwPageFrm* pPage = rFrm.FindPageFrm();
                // auto width table
                nMin = pFrm->GetUpper()->IsVertical() ?
                    pPage->Prt().Height() :
                    pPage->Prt().Width();
            }
            else
            {
                nMin = rTblFmtSz.GetSize().Width();
            }
        }

        if ( nMin > nRet )
            nRet = nMin;

        pFrm = pFrm->GetNext();
    }

    return nRet;
}

SwTwips SwFlyFrm::CalcAutoWidth() const
{
    return lcl_CalcAutoWidth( *this );
}

/*************************************************************************
|*
|*  SwFlyFrm::GetContour()
|*
|*************************************************************************/
/// OD 16.04.2003 #i13147# - If called for paint and the <SwNoTxtFrm> contains
/// a graphic, load of intrinsic graphic has to be avoided.
sal_Bool SwFlyFrm::GetContour( PolyPolygon&   rContour,
                           const sal_Bool _bForPaint ) const
{
    sal_Bool bRet = sal_False;
    if( GetFmt()->GetSurround().IsContour() && Lower() &&
        Lower()->IsNoTxtFrm() )
    {
        SwNoTxtNode *pNd = (SwNoTxtNode*)((SwCntntFrm*)Lower())->GetNode();
        // OD 16.04.2003 #i13147# - determine <GraphicObject> instead of <Graphic>
        // in order to avoid load of graphic, if <SwNoTxtNode> contains a graphic
        // node and method is called for paint.
        const GraphicObject* pGrfObj = NULL;
        sal_Bool bGrfObjCreated = sal_False;
        const SwGrfNode* pGrfNd = pNd->GetGrfNode();
        if ( pGrfNd && _bForPaint )
        {
            pGrfObj = &(pGrfNd->GetGrfObj());
        }
        else
        {
            pGrfObj = new GraphicObject( pNd->GetGraphic() );
            bGrfObjCreated = sal_True;
        }
        OSL_ENSURE( pGrfObj, "SwFlyFrm::GetContour() - No Graphic/GraphicObject found at <SwNoTxtNode>." );
        if ( pGrfObj && pGrfObj->GetType() != GRAPHIC_NONE )
        {
            if( !pNd->HasContour() )
            {
                // OD 16.04.2003 #i13147# - no <CreateContour> for a graphic
                // during paint. Thus, return (value of <bRet> should be <sal_False>).
                if ( pGrfNd && _bForPaint )
                {
                    OSL_FAIL( "SwFlyFrm::GetContour() - No Contour found at <SwNoTxtNode> during paint." );
                    return bRet;
                }
                pNd->CreateContour();
            }
            pNd->GetContour( rContour );
            //Der Node haelt das Polygon passend zur Originalgroesse der Grafik
            //hier muss die Skalierung einkalkuliert werden.
            SwRect aClip;
            SwRect aOrig;
            Lower()->Calc();
            ((SwNoTxtFrm*)Lower())->GetGrfArea( aClip, &aOrig, sal_False );
            // OD 16.04.2003 #i13147# - copy method code <SvxContourDlg::ScaleContour(..)>
            // in order to avoid that graphic has to be loaded for contour scale.
            //SvxContourDlg::ScaleContour( rContour, aGrf, MAP_TWIP, aOrig.SSize() );
            {
                OutputDevice*   pOutDev = Application::GetDefaultDevice();
                const MapMode   aDispMap( MAP_TWIP );
                const MapMode   aGrfMap( pGrfObj->GetPrefMapMode() );
                const Size      aGrfSize( pGrfObj->GetPrefSize() );
                Size            aOrgSize;
                Point           aNewPoint;
                sal_Bool            bPixelMap = aGrfMap.GetMapUnit() == MAP_PIXEL;

                if ( bPixelMap )
                    aOrgSize = pOutDev->PixelToLogic( aGrfSize, aDispMap );
                else
                    aOrgSize = pOutDev->LogicToLogic( aGrfSize, aGrfMap, aDispMap );

                if ( aOrgSize.Width() && aOrgSize.Height() )
                {
                    double fScaleX = (double) aOrig.Width() / aOrgSize.Width();
                    double fScaleY = (double) aOrig.Height() / aOrgSize.Height();

                    for ( sal_uInt16 j = 0, nPolyCount = rContour.Count(); j < nPolyCount; j++ )
                    {
                        Polygon& rPoly = rContour[ j ];

                        for ( sal_uInt16 i = 0, nCount = rPoly.GetSize(); i < nCount; i++ )
                        {
                            if ( bPixelMap )
                                aNewPoint = pOutDev->PixelToLogic( rPoly[ i ], aDispMap  );
                            else
                                aNewPoint = pOutDev->LogicToLogic( rPoly[ i ], aGrfMap, aDispMap  );

                            rPoly[ i ] = Point( FRound( aNewPoint.X() * fScaleX ), FRound( aNewPoint.Y() * fScaleY ) );
                        }
                    }
                }
            }
            // OD 17.04.2003 #i13147# - destroy created <GraphicObject>.
            if ( bGrfObjCreated )
            {
                delete pGrfObj;
            }
            rContour.Move( aOrig.Left(), aOrig.Top() );
            if( !aClip.Width() )
                aClip.Width( 1 );
            if( !aClip.Height() )
                aClip.Height( 1 );
            rContour.Clip( aClip.SVRect() );
            rContour.Optimize(POLY_OPTIMIZE_CLOSE);
            bRet = sal_True;
        }
    }
    return bRet;
}

// OD 2004-03-25 #i26791#
const SwVirtFlyDrawObj* SwFlyFrm::GetVirtDrawObj() const
{
    return static_cast<const SwVirtFlyDrawObj*>(GetDrawObj());
}
SwVirtFlyDrawObj* SwFlyFrm::GetVirtDrawObj()
{
    return static_cast<SwVirtFlyDrawObj*>(DrawObj());
}

// =============================================================================
// OD 2004-03-24 #i26791# - implementation of pure virtual method declared in
// base class <SwAnchoredObject>
// =============================================================================
void SwFlyFrm::InvalidateObjPos()
{
    InvalidatePos();
    // #i68520#
    InvalidateObjRectWithSpaces();
}

SwFrmFmt& SwFlyFrm::GetFrmFmt()
{
    OSL_ENSURE( GetFmt(),
            "<SwFlyFrm::GetFrmFmt()> - missing frame format -> crash." );
    return *GetFmt();
}
const SwFrmFmt& SwFlyFrm::GetFrmFmt() const
{
    OSL_ENSURE( GetFmt(),
            "<SwFlyFrm::GetFrmFmt()> - missing frame format -> crash." );
    return *GetFmt();
}

const SwRect SwFlyFrm::GetObjRect() const
{
    return Frm();
}

// #i70122#
// for Writer fly frames the bounding rectangle equals the object rectangles
const SwRect SwFlyFrm::GetObjBoundRect() const
{
    return GetObjRect();
}

// #i68520#
bool SwFlyFrm::_SetObjTop( const SwTwips _nTop )
{
    const bool bChanged( Frm().Pos().Y() != _nTop );

    Frm().Pos().Y() = _nTop;

    return bChanged;
}
bool SwFlyFrm::_SetObjLeft( const SwTwips _nLeft )
{
    const bool bChanged( Frm().Pos().X() != _nLeft );

    Frm().Pos().X() = _nLeft;

    return bChanged;
}

/** method to assure that anchored object is registered at the correct
    page frame

    OD 2004-07-02 #i28701#

    @author OD
*/
void SwFlyFrm::RegisterAtCorrectPage()
{
    // default behaviour is to do nothing.
}

/** method to determine, if a <MakeAll()> on the Writer fly frame is possible

    OD 2004-05-11 #i28701#

    @author OD
*/
bool SwFlyFrm::IsFormatPossible() const
{
    return SwAnchoredObject::IsFormatPossible() &&
           !IsLocked() && !IsColLocked();
}

void SwFlyFrm::GetAnchoredObjects( std::list<SwAnchoredObject*>& aList, const SwFmt& rFmt )
{
    SwIterator<SwFlyFrm,SwFmt> aIter( rFmt );
    for( SwFlyFrm* pFlyFrm = aIter.First(); pFlyFrm; pFlyFrm = aIter.Next() )
        aList.push_back( pFlyFrm );
}

const SwFlyFrmFmt * SwFlyFrm::GetFmt() const
{
    return static_cast< const SwFlyFrmFmt * >( GetDep() );
}

SwFlyFrmFmt * SwFlyFrm::GetFmt()
{
    return static_cast< SwFlyFrmFmt * >( GetDep() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
