/*************************************************************************
 *
 *  $RCSfile: fly.cxx,v $
 *
 *  $Revision: 1.63 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:08:50 $
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


#pragma hdrstop

#include "hintids.hxx"
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _IMAP_HXX //autogen
#include <svtools/imap.hxx>
#endif
#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif
#ifndef _CONTDLG_HXX_ //autogen
#include <svx/contdlg.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <svx/opaqitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTCNCT_HXX //autogen
#include <fmtcnct.hxx>
#endif
#ifndef _LAYHELP_HXX
#include <layhelp.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif

// OD 16.04.2003 #i13147# - for <SwFlyFrm::GetContour(..)>
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
// OD 29.10.2003 #113049#
#ifndef _TOLAYOUTANCHOREDOBJECTPOSITION_HXX
#include <tolayoutanchoredobjectposition.hxx>
#endif
// OD 06.11.2003 #i22305#
#ifndef _FMTFOLLOWTEXTFLOW_HXX
#include <fmtfollowtextflow.hxx>
#endif
// --> OD 2004-06-28 #i28701#
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif
#ifndef _OBJECTFORMATTER_HXX
#include <objectformatter.hxx>
#endif
// <--
// OD 2004-04-06 #i26791#
#ifndef _ANCHOREDOBJECT_HXX
#include <anchoredobject.hxx>
#endif

#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif

#include "doc.hxx"
#include "viewsh.hxx"
#include "layouter.hxx"
#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "pam.hxx"
#include "frmatr.hxx"
#include "viewimp.hxx"
#include "errhdl.hxx"
#include "dcontact.hxx"
#include "dflyobj.hxx"
#include "dview.hxx"
#include "flyfrm.hxx"
#include "frmtool.hxx"
#include "frmfmt.hxx"
#include "hints.hxx"
#include "swregion.hxx"
#include "frmsh.hxx"
#include "tabfrm.hxx"
#include "txtfrm.hxx"
#include "ndnotxt.hxx"
#include "notxtfrm.hxx"   // GetGrfArea
#include "flyfrms.hxx"
#include "ndindex.hxx"   // GetGrfArea
#include "sectfrm.hxx"

// OD 2004-03-23 #i26791
TYPEINIT2(SwFlyFrm,SwLayoutFrm,SwAnchoredObject);

/*************************************************************************
|*
|*  SwFlyFrm::SwFlyFrm()
|*
|*  Ersterstellung      MA 28. Sep. 92
|*  Letzte Aenderung    MA 09. Apr. 99
|*
|*************************************************************************/

SwFlyFrm::SwFlyFrm( SwFlyFrmFmt *pFmt, SwFrm *pAnch ) :
    SwLayoutFrm( pFmt ),
    // OD 2004-03-22 #i26791#
    SwAnchoredObject(),
    // OD 2004-05-27 #i26791# - moved to <SwAnchoredObject>
//    aRelPos(),
    pPrevLink( 0 ),
    pNextLink( 0 ),
    bInCnt( FALSE ),
    bAtCnt( FALSE ),
    bLayout( FALSE ),
    bAutoPosition( FALSE ),
    bNoShrink( FALSE ),
    bLockDeleteContent( FALSE )
{
    nType = FRMC_FLY;

    bInvalid = bNotifyBack = TRUE;
    bLocked  = bMinHeight =
    bHeightClipped = bWidthClipped = bFormatHeightOnly = FALSE;

    //Grosseneinstellung, Fixe groesse ist immer die Breite
    const SwFmtFrmSize &rFrmSize = pFmt->GetFrmSize();
    BOOL bVert = FALSE;
    UINT16 nDir =
        ((SvxFrameDirectionItem&)pFmt->GetAttr( RES_FRAMEDIR )).GetValue();
    if( FRMDIR_ENVIRONMENT == nDir )
    {
        bDerivedVert = 1;
        bDerivedR2L = 1;
        if( pAnch && pAnch->IsVertical() )
            bVert = TRUE;
    }
    else
    {
        bInvalidVert = 0;
        bDerivedVert = 0;
        bDerivedR2L = 0;
        if( FRMDIR_HORI_LEFT_TOP == nDir || FRMDIR_HORI_RIGHT_TOP == nDir
                                         || pFmt->GetDoc()->IsBrowseMode() )
            bVertical = 0;
        else
            bVertical = 1;
        bVert = bVertical;
        bInvalidR2L = 0;
        if( FRMDIR_HORI_RIGHT_TOP == nDir )
            bRightToLeft = 1;
        else
            bRightToLeft = 0;
    }

    Frm().Width( rFrmSize.GetWidth() );
    Frm().Height( rFrmSize.GetHeightSizeType() == ATT_VAR_SIZE ? MINFLY : rFrmSize.GetHeight() );

    //Hoehe Fix oder Variabel oder was?
    if ( rFrmSize.GetHeightSizeType() == ATT_MIN_SIZE )
        bMinHeight = TRUE;
    else if ( rFrmSize.GetHeightSizeType() == ATT_FIX_SIZE )
        bFixSize = TRUE;

    // OD 2004-02-12 #110582#-2 - insert columns, if necessary
    InsertColumns();

    //Erst das Init, dann den Inhalt, denn zum Inhalt koennen  widerum
    //Objekte/Rahmen gehoeren die dann angemeldet werden.
    InitDrawObj( FALSE );

    // OD 2004-01-19 #110582#
    Chain( pAnch );

    // OD 2004-01-19 #110582#
    InsertCnt();

    //Und erstmal in den Wald stellen die Kiste, damit bei neuen Dokument nicht
    //unnoetig viel formatiert wird.
    Frm().Pos().X() = Frm().Pos().Y() = WEIT_WECH;
}

// OD 2004-01-19 #110582#
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
                ASSERT( !pFollow->GetPrevLink(), "wrong chain detected" );
                if ( !pFollow->GetPrevLink() )
                    SwFlyFrm::ChainFrames( this, pFollow );
            }
        }
        if ( rChain.GetPrev() )
        {
            SwFlyFrm *pMaster = FindChainNeighbour( *rChain.GetPrev(), _pAnch );
            if ( pMaster )
            {
                ASSERT( !pMaster->GetNextLink(), "wrong chain detected" );
                if ( !pMaster->GetNextLink() )
                    SwFlyFrm::ChainFrames( pMaster, this );
            }
        }
    }
}

// OD 2004-01-19 #110582#
void SwFlyFrm::InsertCnt()
{
    if ( !GetPrevLink() )
    {
        const SwFmtCntnt& rCntnt = GetFmt()->GetCntnt();
        ASSERT( rCntnt.GetCntntIdx(), ":-( no content prepared." );
        ULONG nIndex = rCntnt.GetCntntIdx()->GetIndex();
        // Lower() bedeutet SwColumnFrm, eingefuegt werden muss der Inhalt dann in den (Column)BodyFrm
        ::_InsertCnt( Lower() ? (SwLayoutFrm*)((SwLayoutFrm*)Lower())->Lower() : (SwLayoutFrm*)this,
                      GetFmt()->GetDoc(), nIndex );

        //NoTxt haben immer eine FixHeight.
        if ( Lower() && Lower()->IsNoTxtFrm() )
        {
            bFixSize = TRUE;
            bMinHeight = FALSE;
        }
    }
}

 // OD 2004-02-12 #110582#-2
 void SwFlyFrm::InsertColumns()
 {
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
|*  Ersterstellung      MA 28. Sep. 92
|*  Letzte Aenderung    MA 07. Jul. 95
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
        SwRootFrm *pRootFrm = FindRootFrm();
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
|*  Ersterstellung      MA 02. Dec. 94
|*  Letzte Aenderung    MA 30. Nov. 95
|*
|*************************************************************************/
#pragma optimize("",off)

void SwFlyFrm::InitDrawObj( BOOL bNotify )
{
    //ContactObject aus dem Format suchen. Wenn bereits eines existiert, so
    //braucht nur eine neue Ref erzeugt werden, anderfalls ist es jetzt an
    //der Zeit das Contact zu erzeugen.
    SwClientIter aIter( *GetFmt() );
    SwFlyDrawContact *pContact = (SwFlyDrawContact*)
                                        aIter.First( TYPE(SwFlyDrawContact) );
    if ( !pContact )
        pContact = new SwFlyDrawContact( (SwFlyFrmFmt*)GetFmt(),
                            GetFmt()->GetDoc()->MakeDrawModel() );
    ASSERT( pContact, "InitDrawObj failed" );
    // OD 2004-03-22 #i26791#
    SetDrawObj( *(pContact->CreateNewRef( this )) );

    //Den richtigen Layer setzen.
    // OD 2004-01-19 #110582#
    SdrLayerID nHeavenId = GetFmt()->GetDoc()->GetHeavenId();
    SdrLayerID nHellId = GetFmt()->GetDoc()->GetHellId();
    // OD 2004-03-22 #i26791#
    GetVirtDrawObj()->SetLayer( GetFmt()->GetOpaque().GetValue()
                                ? nHeavenId
                                : nHellId );
    if ( bNotify )
        NotifyDrawObj();
}

#pragma optimize("",on)

/*************************************************************************
|*
|*  SwFlyFrm::FinitDrawObj()
|*
|*  Ersterstellung      MA 12. Dec. 94
|*  Letzte Aenderung    MA 15. May. 95
|*
|*************************************************************************/

void SwFlyFrm::FinitDrawObj()
{
    if ( !GetVirtDrawObj() )
        return;

    //Bei den SdrPageViews abmelden falls das Objekt dort noch selektiert ist.
    if ( !GetFmt()->GetDoc()->IsInDtor() )
    {
        ViewShell *p1St = GetShell();
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
        SwClientIter aIter( *GetFmt() );
        aIter.GoStart();
        do {
            if ( aIter()->ISA(SwFrm) && (SwFrm*)aIter() != this )
            {
                pMyContact = 0;
                break;
            }
            if( !pMyContact && aIter()->ISA(SwFlyDrawContact) )
                pMyContact = (SwFlyDrawContact*)aIter();
            aIter++;
        } while( aIter() );
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
    if ( pMyContact )
        delete pMyContact;      //zerstoert den Master selbst.
}

/*************************************************************************
|*
|*  SwFlyFrm::ChainFrames()
|*
|*  Ersterstellung      MA 29. Oct. 97
|*  Letzte Aenderung    MA 20. Jan. 98
|*
|*************************************************************************/

void SwFlyFrm::ChainFrames( SwFlyFrm *pMaster, SwFlyFrm *pFollow )
{
    ASSERT( pMaster && pFollow, "uncomplete chain" );
    ASSERT( !pMaster->GetNextLink(), "link can not be changed" );
    ASSERT( !pFollow->GetPrevLink(), "link can not be changed" );

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
        ASSERT( !pFrm->IsTabFrm() && !pFrm->FindNext(), "follow in chain contains content" );
        pFrm->Cut();
        delete pFrm;
    }

#ifdef ACCESSIBLE_LAYOUT
    // invalidate accessible relation set (accessibility wrapper)
    ViewShell* pSh = pMaster->GetShell();
    if( pSh && pSh->GetLayout()->IsAnyShellAccessible() )
        pSh->Imp()->InvalidateAccessibleRelationSet( pMaster, pFollow );
#endif

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
            pUpper = (SwLayoutFrm*)pUpper->Lower();
            while ( pUpper->GetNext() ) // sucht die letzte Spalte
                pUpper = (SwLayoutFrm*)pUpper->GetNext();
            pUpper = (SwLayoutFrm*)((SwLayoutFrm*)pUpper)->Lower(); // der (Column)BodyFrm
            ASSERT( pUpper && pUpper->IsColBodyFrm(), "Missing ColumnBody" );
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
    ASSERT( rCntnt.GetCntntIdx(), ":-( Kein Inhalt vorbereitet." );
    ULONG nIndex = rCntnt.GetCntntIdx()->GetIndex();
    // Lower() bedeutet SwColumnFrm, dieser beinhaltet wieder einen SwBodyFrm
    ::_InsertCnt( pFollow->Lower() ? (SwLayoutFrm*)((SwLayoutFrm*)pFollow->Lower())->Lower()
                                   : (SwLayoutFrm*)pFollow,
                  pFollow->GetFmt()->GetDoc(), ++nIndex );

#ifdef ACCESSIBLE_LAYOUT
    // invalidate accessible relation set (accessibility wrapper)
    ViewShell* pSh = pMaster->GetShell();
    if( pSh && pSh->GetLayout()->IsAnyShellAccessible() )
        pSh->Imp()->InvalidateAccessibleRelationSet( pMaster, pFollow );
#endif
}

/*************************************************************************
|*
|*  SwFlyFrm::FindChainNeighbour()
|*
|*  Ersterstellung      MA 11. Nov. 97
|*  Letzte Aenderung    MA 09. Apr. 99
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

    SwClientIter aIter( rChain );
    SwFlyFrm *pFly = (SwFlyFrm*)aIter.First( TYPE(SwFlyFrm ) );
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
            pFly = (SwFlyFrm*)aIter.Next();
        }
    }
    else if ( pFly )
    {
        ASSERT( !aIter.Next(), "chain with more than one inkarnation" );
    }
    return pFly;
}


/*************************************************************************
|*
|*  SwFlyFrm::FindLastLower()
|*
|*  Ersterstellung      MA 29. Oct. 97
|*  Letzte Aenderung    MA 29. Oct. 97
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
|*  Ersterstellung      MA 17. Dec. 92
|*  Letzte Aenderung    MA 24. Jul. 96
|*
|*************************************************************************/

BOOL SwFlyFrm::FrmSizeChg( const SwFmtFrmSize &rFrmSize )
{
    BOOL bRet = FALSE;
    SwTwips nDiffHeight = Frm().Height();
    if ( rFrmSize.GetHeightSizeType() == ATT_VAR_SIZE )
        BFIXHEIGHT = bMinHeight = FALSE;
    else
    {
        if ( rFrmSize.GetHeightSizeType() == ATT_FIX_SIZE )
        {   BFIXHEIGHT = TRUE;
            bMinHeight = FALSE;
        }
        else if ( rFrmSize.GetHeightSizeType() == ATT_MIN_SIZE )
        {   BFIXHEIGHT = FALSE;
            bMinHeight = TRUE;
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
            aPrt.Height( aPrt.Height() - nDiffHeight );
            aPrt.Width ( aPrt.Width()  - nDiffWidth  );
            ChgLowersProp( aOldSz );
            ::Notify( this, FindPageFrm(), aOld );
            bValidPos = FALSE;
            bRet = TRUE;
        }
        else if ( Lower()->IsNoTxtFrm() )
        {
            BFIXHEIGHT = TRUE;
            bMinHeight = FALSE;
        }
    }
    return bRet;
}

/*************************************************************************
|*
|*  SwFlyFrm::Modify()
|*
|*  Ersterstellung      MA 17. Dec. 92
|*  Letzte Aenderung    MA 17. Jan. 97
|*
|*************************************************************************/

void SwFlyFrm::Modify( SfxPoolItem * pOld, SfxPoolItem * pNew )
{
    BYTE nInvFlags = 0;

    if( pNew && RES_ATTRSET_CHG == pNew->Which() )
    {
        SfxItemIter aNIter( *((SwAttrSetChg*)pNew)->GetChgSet() );
        SfxItemIter aOIter( *((SwAttrSetChg*)pOld)->GetChgSet() );
        SwAttrSetChg aOldSet( *(SwAttrSetChg*)pOld );
        SwAttrSetChg aNewSet( *(SwAttrSetChg*)pNew );
        while( TRUE )
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
            _InvalidatePos();
        if ( nInvFlags & 0x02 )
            _InvalidateSize();
        if ( nInvFlags & 0x04 )
            _InvalidatePrt();
        if ( nInvFlags & 0x08 )
            SetNotifyBack();
        if ( nInvFlags & 0x10 )
            SetCompletePaint();
        if ( ( nInvFlags & 0x40 ) && Lower() && Lower()->IsNoTxtFrm() )
            ClrContourCache( GetVirtDrawObj() );
        SwRootFrm *pRoot;
        if ( nInvFlags & 0x20 && 0 != (pRoot = FindRootFrm()) )
            pRoot->InvalidateBrowseWidth();
        // --> OD 2004-06-28 #i28701#
        if ( nInvFlags & 0x80 )
        {
            // update sorted object lists, the Writer fly frame is registered at.
            UpdateObjInSortedList();
        }
        // <--
    }
}

void SwFlyFrm::_UpdateAttr( SfxPoolItem *pOld, SfxPoolItem *pNew,
                            BYTE &rInvFlags,
                            SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    BOOL bClear = TRUE;
    const USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    ViewShell *pSh = GetShell();
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
            // --> OD 2004-06-28 #i28701# - perform reorder of object lists
            // at anchor frame and at page frame.
            rInvFlags |= 0x80;
        }
        break;

        case RES_PROTECT:
            {
            const SvxProtectItem *pP = (SvxProtectItem*)pNew;
            GetVirtDrawObj()->SetMoveProtect( pP->IsPosProtected()   );
            GetVirtDrawObj()->SetResizeProtect( pP->IsSizeProtected() );
            if( pSh && pSh->GetLayout()->IsAnyShellAccessible() )
                pSh->Imp()->InvalidateAccessibleEditableState( sal_True, this );
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

                //Dummer Fall. Bei der Zusweisung einer Vorlage k”nnen wir uns
                //nicht auf das alte Spaltenattribut verlassen. Da diese
                //wenigstens anzahlgemass fuer ChgColumns vorliegen muessen,
                //bleibt uns nur einen temporaeres Attribut zu basteln.
                SwFmtCol aCol;
                if ( Lower() && Lower()->IsColumnFrm() )
                {
                    USHORT nCol = 0;
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
                    pFmt->SetAttr( aURL );
                    pFmt->UnlockModify();
                }
            }
            const SvxProtectItem &rP = GetFmt()->GetProtect();
            GetVirtDrawObj()->SetMoveProtect( rP.IsPosProtected()    );
            GetVirtDrawObj()->SetResizeProtect( rP.IsSizeProtected() );

            if ( pSh )
                pSh->InvalidateWindows( Frm() );
            const BYTE nId = GetFmt()->GetOpaque().GetValue() ?
                                GetFmt()->GetDoc()->GetHeavenId() :
                                GetFmt()->GetDoc()->GetHellId();
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
                    SwFrm* pFrm = Lower();
                    while( pFrm->GetNext() )
                        pFrm = pFrm->GetNext();
                    if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsUndersized() )
                        pFrm->Prepare( PREP_ADJUST_FRM );
                }
            }

            // --> OD 2004-06-28 #i28701# - perform reorder of object lists
            // at anchor frame and at page frame.
            rInvFlags |= 0x80;

            break;
        }
        case RES_UL_SPACE:
        case RES_LR_SPACE:
        {
            rInvFlags |= 0x41;
            if ( GetFmt()->GetDoc()->IsBrowseMode() )
                GetFmt()->GetDoc()->GetRootFrm()->InvalidateBrowseWidth();
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
            SetDerivedVert( FALSE );
            SetDerivedR2L( FALSE );
            CheckDirChange();
            break;

        case RES_OPAQUE:
            {
                if ( pSh )
                    pSh->InvalidateWindows( Frm() );
                const BYTE nId = ((SvxOpaqueItem*)pNew)->GetValue() ?
                                    GetFmt()->GetDoc()->GetHeavenId() :
                                    GetFmt()->GetDoc()->GetHellId();
                GetVirtDrawObj()->SetLayer( nId );
                if( pSh && pSh->GetLayout()->IsAnyShellAccessible() )
                {
                    pSh->Imp()->DisposeAccessibleFrm( this );
                    pSh->Imp()->AddAccessibleFrm( this );
                }
                // --> OD 2004-06-28 #i28701# - perform reorder of object lists
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
                    pFmt->SetAttr( aURL );
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
            bClear = FALSE;
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
|*    Ersterstellung    JP 31.03.94
|*    Letzte Aenderung  JP 31.03.94
|*
*************************************************************************/

    // erfrage vom Modify Informationen
BOOL SwFlyFrm::GetInfo( SfxPoolItem & rInfo ) const
{
    if( RES_AUTOFMT_DOCNODE == rInfo.Which() )
        return FALSE;   // es gibt einen FlyFrm also wird er benutzt
    return TRUE;        // weiter suchen
}

/*************************************************************************
|*
|*  SwFlyFrm::_Invalidate()
|*
|*  Ersterstellung      MA 15. Oct. 92
|*  Letzte Aenderung    MA 26. Jun. 96
|*
|*************************************************************************/

void SwFlyFrm::_Invalidate( SwPageFrm *pPage )
{
    InvalidatePage( pPage );
    bNotifyBack = bInvalid = TRUE;

    SwFlyFrm *pFrm;
    if ( GetAnchorFrm() && 0 != (pFrm = AnchorFrm()->FindFlyFrm()) )
    {
        //Gaanz dumm: Wenn der Fly innerhalb eines Fly gebunden ist, der
        //Spalten enthaehlt, sollte das Format von diesem ausgehen.
        if ( !pFrm->IsLocked() && !pFrm->IsColLocked() &&
             pFrm->Lower() && pFrm->Lower()->IsColumnFrm() )
            pFrm->InvalidateSize();
    }
}

/*************************************************************************
|*
|*  SwFlyFrm::ChgRelPos()
|*
|*  Beschreibung        Aenderung der relativen Position, die Position wird
|*      damit automatisch Fix, das Attribut wird entprechend angepasst.
|*  Ersterstellung      MA 25. Aug. 92
|*  Letzte Aenderung    MA 09. Aug. 95
|*
|*************************************************************************/

void SwFlyFrm::ChgRelPos( const Point &rNewPos )
{
    if ( GetCurrRelPos() != rNewPos )
    {
        SwFrmFmt *pFmt = GetFmt();
        SWRECTFN( GetAnchorFrm() )
        SwTwips nNewY = bVert ? rNewPos.X() : rNewPos.Y();
        SwTwips nTmpY = nNewY == LONG_MAX ? 0 : nNewY;
        if( bVert )
            nTmpY = -nTmpY;
        SfxItemSet aSet( pFmt->GetDoc()->GetAttrPool(),
                         RES_VERT_ORIENT, RES_HORI_ORIENT);

        SwFmtVertOrient aVert( pFmt->GetVertOrient() );
        SwTxtFrm *pAutoFrm = NULL;
        if( IsFlyAtCntFrm() || VERT_NONE != aVert.GetVertOrient() )
        {
            if( REL_CHAR == aVert.GetRelationOrient() && IsAutoPos() )
            {
                if( LONG_MAX != nNewY )
                {
                    aVert.SetVertOrient( VERT_NONE );
                    xub_StrLen nOfs =
                        pFmt->GetAnchor().GetCntntAnchor()->nContent.GetIndex();
                    ASSERT( GetAnchorFrm()->IsTxtFrm(), "TxtFrm expected" );
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
                    aVert.SetVertOrient( VERT_CHAR_BOTTOM );
            }
            else
            {
                aVert.SetVertOrient( VERT_NONE );
                aVert.SetRelationOrient( FRAME );
            }
        }
        aVert.SetPos( nTmpY );
        aSet.Put( aVert );

        //Fuer Flys im Cnt ist die horizontale Ausrichtung uninteressant,
        //den sie ist stets 0.
        if ( !IsFlyInCntFrm() )
        {
            SwTwips nNewX = bVert ? rNewPos.Y() : rNewPos.X();
            SwTwips nTmpX = nNewX == LONG_MAX ? 0 : nNewX;
            SwFmtHoriOrient aHori( pFmt->GetHoriOrient() );
            if( IsFlyAtCntFrm() || HORI_NONE != aHori.GetHoriOrient() )
            {
                aHori.SetHoriOrient( HORI_NONE );
                if( REL_CHAR == aHori.GetRelationOrient() && IsAutoPos() )
                {
                    if( LONG_MAX != nNewX )
                    {
                        if( !pAutoFrm )
                        {
                            xub_StrLen nOfs = pFmt->GetAnchor().GetCntntAnchor()
                                          ->nContent.GetIndex();
                            ASSERT( GetAnchorFrm()->IsTxtFrm(), "TxtFrm expected");
                            pAutoFrm = (SwTxtFrm*)GetAnchorFrm();
                            while( pAutoFrm->GetFollow() &&
                                   pAutoFrm->GetFollow()->GetOfst() <= nOfs )
                                pAutoFrm = pAutoFrm->GetFollow();
                        }
                        nTmpX -= ((SwFlyAtCntFrm*)this)->GetRelCharX(pAutoFrm);
                    }
                }
                else
                    aHori.SetRelationOrient( FRAME );
                aHori.SetPosToggle( FALSE );
            }
            aHori.SetPos( nTmpX );
            aSet.Put( aHori );
        }
        pFmt->GetDoc()->SetAttr( aSet, *pFmt );
    }
}
/*************************************************************************
|*
|*  SwFlyFrm::Format()
|*
|*  Beschreibung:       "Formatiert" den Frame; Frm und PrtArea.
|*                      Die Fixsize wird hier nicht eingestellt.
|*  Ersterstellung      MA 14. Jun. 93
|*  Letzte Aenderung    MA 13. Jun. 96
|*
|*************************************************************************/

void SwFlyFrm::Format( const SwBorderAttrs *pAttrs )
{
    ASSERT( pAttrs, "FlyFrm::Format, pAttrs ist 0." );

    ColLock();

    if ( !bValidSize )
    {
        if ( Frm().Top() == WEIT_WECH && Frm().Left() == WEIT_WECH )
            //Sicherheitsschaltung wegnehmen (siehe SwFrm::CTor)
            Frm().Pos().X() = Frm().Pos().Y() = 0;

        //Breite der Spalten pruefen und ggf. einstellen.
        if ( Lower() && Lower()->IsColumnFrm() )
            AdjustColumns( 0, FALSE );

        bValidSize = TRUE;

        const SwTwips nUL = pAttrs->CalcTopLine()  + pAttrs->CalcBottomLine();
        const SwTwips nLR = pAttrs->CalcLeftLine() + pAttrs->CalcRightLine();
        const Size   &rSz = pAttrs->GetSize();
        const SwFmtFrmSize &rFrmSz = GetFmt()->GetFrmSize();
              Size aRelSize( CalcRel( rFrmSz ) );

        ASSERT( rSz.Height() != 0 || rFrmSz.GetHeightPercent(), "Hoehe des RahmenAttr ist 0." );
        ASSERT( rSz.Width()  != 0 || rFrmSz.GetWidthPercent(), "Breite des RahmenAttr ist 0." );

        SWRECTFN( this )
        if( !HasFixSize() )
        {
            SwTwips nRemaining = 0;
            SwTwips nOldHeight = (Frm().*fnRect->fnGetHeight)();

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
                    if( !nRemaining )
                        nRemaining = nOldHeight - nUL;
                }
                if ( GetDrawObjs() )
                {
                    sal_uInt32 nCnt = GetDrawObjs()->Count();
                    SwTwips nTop = (Frm().*fnRect->fnGetTop)();
                    SwTwips nBorder = (Frm().*fnRect->fnGetHeight)() -
                                      (Prt().*fnRect->fnGetHeight)();
                    for ( USHORT i = 0; i < nCnt; ++i )
                    {
                        SwAnchoredObject* pAnchoredObj = (*GetDrawObjs())[i];
                        if ( pAnchoredObj->ISA(SwFlyFrm) )
                        {
                            SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                            // OD 06.11.2003 #i22305# - consider
                            // only Writer fly frames, which follow the text flow.
                            if ( pFly->IsFlyLayFrm() &&
                                 pFly->Frm().Top() != WEIT_WECH &&
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
            bValidSize = TRUE;
        }
        else
        {
            bValidSize = TRUE;  //Fixe Frms formatieren sich nicht.
                                //Flys stellen ihre Groesse anhand des Attr ein.
            SwTwips nNewSize = bVert ? aRelSize.Width() : aRelSize.Height();
            nNewSize -= nUL;
            if( nNewSize < MINFLY )
                nNewSize = MINFLY;
            (Prt().*fnRect->fnSetHeight)( nNewSize );
            nNewSize += nUL - (Frm().*fnRect->fnGetHeight)();
            (Frm().*fnRect->fnAddBottom)( nNewSize );
        }

        if ( !bFormatHeightOnly )
        {
            ASSERT( aRelSize == CalcRel( rFrmSz ), "SwFlyFrm::Format CalcRel problem" )
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
//void CalcCntnt( SwLayoutFrm *pLay, BOOL bNoColl )
void CalcCntnt( SwLayoutFrm *pLay,
                bool bNoColl,
                bool bNoCalcFollow )
{
    SwSectionFrm* pSect;
    BOOL bCollect = FALSE;
    if( pLay->IsSctFrm() )
    {
        pSect = (SwSectionFrm*)pLay;
        if( pSect->IsEndnAtEnd() && !bNoColl )
        {
            bCollect = TRUE;
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
                    BOOL bLock = pSect->IsFtnLock();
                    pSect->SetFtnLock( TRUE );
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
                ((SwTabFrm*)pFrm)->bCalcLowers = TRUE;
                // OD 26.08.2003 #i18103# - lock move backward of follow table,
                // if no section content is formatted or follow table belongs
                // to the section, which content is formatted.
                if ( ((SwTabFrm*)pFrm)->IsFollow() &&
                     ( !pSect || pSect == pFrm->FindSctFrm() ) )
                {
                    ((SwTabFrm*)pFrm)->bLockBackMove = TRUE;
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
            SwFrm* pTmpPrev = pFrm->FindPrev();
            bool bPrevInvalid = ( pFrm->IsFlowFrm()
                                  ? ( !SwFlowFrm::CastFlowFrm(pFrm)->IsFollow() &&
                                      !SwFlowFrm::CastFlowFrm(pFrm)->IsJoinLocked() )
                                  : true ) &&
                                pTmpPrev &&
                               !pTmpPrev->GetValidPosFlag() &&
                                pTmpPrev->GetAttrSet()->GetKeep().GetValue() &&
                                pLay->IsAnLower( pTmpPrev );

            // format floating screen objects anchored to the frame.
            bool bRestartLayoutProcess = false;
            if ( !bPrevInvalid && pFrm->GetDrawObjs() && pLay->IsAnLower( pFrm ) )
            {
                bool bAgain = false;
                SwPageFrm* pPageFrm = pFrm->FindPageFrm();
                sal_uInt32 nCnt = pFrm->GetDrawObjs()->Count();
                for ( USHORT i = 0; i < nCnt; ++i )
                {
                    // --> OD 2004-07-01 #i28701#
                    SwAnchoredObject* pAnchoredObj = (*pFrm->GetDrawObjs())[i];
                    // determine, if anchored object has to be formatted.
                    if ( pAnchoredObj->PositionLocked() )
                    {
                        continue;
                    }

                    // format anchored object
                    if ( pAnchoredObj->IsFormatPossible() )
                    {
                        pAnchoredObj->InvalidateObjPos();
                        SwRect aRect( pAnchoredObj->GetObjRect() );
                        if ( !SwObjectFormatter::FormatObj( *pAnchoredObj, pFrm, pPageFrm ) )
                        {
                            bRestartLayoutProcess = true;
                            break;
                        }

                        if ( aRect != pAnchoredObj->GetObjRect() )
                        {
                            bAgain = true;
                            if ( pAgainObj2 == pAnchoredObj )
                            {
                                ASSERT( false,
                                        "::CalcCntnt(..) - loop detected, perform attribute changes to avoid the loop" );
                                //Oszillation unterbinden.
                                SwFrmFmt& rFmt = pAnchoredObj->GetFrmFmt();
                                SwFmtSurround aAttr( rFmt.GetSurround() );
                                if( SURROUND_THROUGHT != aAttr.GetSurround() )
                                {
                                    // Bei autopositionierten hilft manchmal nur
                                    // noch, auf Durchlauf zu schalten
                                    if( rFmt.GetAnchor().GetAnchorId() == FLY_AUTO_CNTNT &&
                                        SURROUND_PARALLEL == aAttr.GetSurround() )
                                        aAttr.SetSurround( SURROUND_THROUGHT );
                                    else
                                        aAttr.SetSurround( SURROUND_PARALLEL );
                                    rFmt.LockModify();
                                    rFmt.SetAttr( aAttr );
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

                // --> OD 2004-06-11 #i28701# - restart layout process, if
                // requested by floating screen object formatting
                if ( bRestartLayoutProcess )
                {
                    pFrm = pLay->ContainsAny();
                    continue;
                }

                // OD 2004-05-17 #i28701# - format anchor frame after its objects
                // are formatted, if the wrapping style influence has to be considered.
                if ( pLay->GetFmt()->GetDoc()->ConsiderWrapOnObjPos() )
                {
                    pFrm->Calc();
                }
                // <--

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
                    continue;
                }
            }
            if ( pFrm->IsTabFrm() )
            {
                if ( ((SwTabFrm*)pFrm)->IsFollow() )
                    ((SwTabFrm*)pFrm)->bLockBackMove = FALSE;
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
                bCollect = FALSE;
                if( pFrm )
                    continue;
            }
        }
        break;
    }
    while( TRUE );
}

/*************************************************************************
|*
|*  SwFlyFrm::MakeFlyPos()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA 14. Nov. 96
|*
|*************************************************************************/
// OD 2004-03-23 #i26791#
//void SwFlyFrm::MakeFlyPos()
void SwFlyFrm::MakeObjPos()
{
    if ( !bValidPos )
    {
        bValidPos = TRUE;

        // OD 29.10.2003 #113049# - use new class to position object
        GetAnchorFrm()->Calc();
        objectpositioning::SwToLayoutAnchoredObjectPosition
                aObjPositioning( *GetVirtDrawObj() );
        aObjPositioning.CalcPosition();

        SWRECTFN( GetAnchorFrm() );
        aFrm.Pos( aObjPositioning.GetRelPos() );
        aFrm.Pos() += (GetAnchorFrm()->Frm().*fnRect->fnGetPos)();
    }
}

/*************************************************************************
|*
|*  SwFlyFrm::MakePrtArea()
|*
|*  Ersterstellung      MA 23. Jun. 93
|*  Letzte Aenderung    MA 23. Jun. 93
|*
|*************************************************************************/
void SwFlyFrm::MakePrtArea( const SwBorderAttrs &rAttrs )
{

    if ( !bValidPrtArea )
    {
        bValidPrtArea = TRUE;

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
|*  Ersterstellung      MA 05. Oct. 92
|*  Letzte Aenderung    MA 05. Sep. 96
|*
|*************************************************************************/

SwTwips SwFlyFrm::_Grow( SwTwips nDist, BOOL bTst )
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
                // --> OD 2004-06-09 #i28701# - unlock position of Writer fly frame
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
            const BOOL bOldLock = bLocked;
            Unlock();
            if ( IsFlyFreeFrm() )
                ((SwFlyFreeFrm*)this)->SwFlyFreeFrm::MakeAll();
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

SwTwips SwFlyFrm::_Shrink( SwTwips nDist, BOOL bTst )
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
            const BOOL bOldLocked = bLocked;
            Unlock();
            if ( IsFlyFreeFrm() )
                ((SwFlyFreeFrm*)this)->SwFlyFreeFrm::MakeAll();
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
|*  Ersterstellung      MA 05. Oct. 92
|*  Letzte Aenderung    MA 04. Sep. 96
|*
|*************************************************************************/

void SwFlyFrm::ChgSize( const Size& aNewSize )
{
    if ( aNewSize != Frm().SSize() )
    {
        SwFrmFmt *pFmt = GetFmt();
        SwFmtFrmSize aSz( pFmt->GetFrmSize() );
        aSz.SetWidth( aNewSize.Width() );
        if ( Abs(aNewSize.Height() - aSz.GetHeight()) > 1 )
            aSz.SetHeight( aNewSize.Height() );
        // uebers Doc fuers Undo!
        pFmt->GetDoc()->SetAttr( aSz, *pFmt );
    }
}

/*************************************************************************
|*
|*  SwFlyFrm::IsLowerOf()
|*
|*  Ersterstellung      MA 27. Dec. 93
|*  Letzte Aenderung    MA 27. Dec. 93
|*
|*************************************************************************/

BOOL SwFlyFrm::IsLowerOf( const SwLayoutFrm* pUpper ) const
{
    ASSERT( GetAnchorFrm(), "8-( Fly is lost in Space." );
    const SwFrm* pFrm = GetAnchorFrm();
    do
    {
        if ( pFrm == pUpper )
            return TRUE;
        pFrm = pFrm->IsFlyFrm()
               ? ((const SwFlyFrm*)pFrm)->GetAnchorFrm()
               : pFrm->GetUpper();
    } while ( pFrm );
    return FALSE;
}

/*************************************************************************
|*
|*  SwFlyFrm::Cut()
|*
|*  Ersterstellung      MA 23. Feb. 94
|*  Letzte Aenderung    MA 23. Feb. 94
|*
|*************************************************************************/

void SwFlyFrm::Cut()
{
}

/*************************************************************************
|*
|*  SwFrm::AppendFly(), RemoveFly()
|*
|*  Ersterstellung      MA 25. Aug. 92
|*  Letzte Aenderung    MA 09. Jun. 95
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
        if ( pNew->IsFlyAtCntFrm() && pNew->Frm().Top() == WEIT_WECH )
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
                    for ( USHORT i = 0; i < 10; ++i )
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
        pPage->RemoveFlyFromPage( pToRemove );

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
        ASSERT( false,
                "SwFrm::AppendDrawObj(..) - anchored object of unexcepted type -> object not appended" );
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

    // no direct positioning needed, but invalidate the drawing object position
    _rNewObj.InvalidateObjPos();

    // move 'master' drawing object to visible layer
    if ( !_rNewObj.GetDrawObj()->ISA(SwDrawVirtObj) )
    {
        if ( _rNewObj.GetFrmFmt().GetDoc() )
        {
            ::GetUserCall( _rNewObj.GetDrawObj() )->
                                    MoveObjToVisibleLayer( _rNewObj.DrawObj() );
        }
    }

    // register at page frame
    SwPageFrm* pPage = FindPageFrm();
    if ( pPage )
    {
        pPage->AppendDrawObjToPage( _rNewObj );
    }

    // Notify accessible layout.
    ViewShell* pSh = GetShell();
    if( pSh && pSh->GetLayout()->IsAnyShellAccessible() )
    {
        pSh->Imp()->AddAccessibleObj( _rNewObj.GetDrawObj() );
    }
}

void SwFrm::RemoveDrawObj( SwAnchoredObject& _rToRemoveObj )
{
    // Notify accessible layout.
    ViewShell* pSh = GetShell();
    if( pSh && pSh->GetLayout()->IsAnyShellAccessible() )
    {
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
|*  Ersterstellung      MA 29. Nov. 96
|*  Letzte Aenderung    MA 29. Nov. 96
|*
|*************************************************************************/
// --> OD 2004-07-01 #i28701# - change purpose of method and adjust its name
void SwFrm::InvalidateObjs( const bool _bInvaPosOnly,
                            const bool _bNoInvaOfAsCharAnchoredObjs )
{
    if ( GetDrawObjs() )
    {
        // --> OD 2004-07-01 #i28701# - re-factoring
        sal_uInt32 i = 0;
        for ( ; i < GetDrawObjs()->Count(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*GetDrawObjs())[i];
            if ( _bNoInvaOfAsCharAnchoredObjs &&
                 pAnchoredObj->GetFrmFmt().GetAnchor().GetAnchorId() == FLY_IN_CNTNT )
            {
                continue;
            }
            // distinguish between writer fly frames and drawing objects
            if ( pAnchoredObj->ISA(SwFlyFrm) )
            {
                SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                ASSERT( pFly->IsFlyFreeFrm(),
                        "<SwFrm::InvalidateObjs(..) - <pFly> isn't a <SwFlyFreeFrm>." );

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
// --> OD 2004-07-01 #i28701# - change purpose of method and its name
void SwLayoutFrm::NotifyLowerObjs()
{
    // invalidate lower floating screen objects
    SwPageFrm* pPageFrm = FindPageFrm();
    if ( pPageFrm && pPageFrm->GetSortedObjs() )
    {
        SwSortedObjs& rObjs = *(pPageFrm->GetSortedObjs());
        for ( sal_uInt32 i = 0; i < rObjs.Count(); ++i )
        {
            SwAnchoredObject* pObj = rObjs[i];
            if ( pObj->ISA(SwFlyFrm) )
            {
                SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pObj);

                if ( pFly->Frm().Left() == WEIT_WECH )
                    continue;

                if ( pFly->IsAnLower( this ) )
                    continue;

                const bool bLow = pFly->IsLowerOf( this );
                if ( bLow || pFly->GetAnchorFrm()->FindPageFrm() != pPageFrm )
                {
                    pFly->_Invalidate( pPageFrm );
                    if ( !bLow || pFly->IsFlyAtCntFrm() )
                    {
                        pFly->_InvalidatePos();
                    }
                    else
                        pFly->_InvalidatePrt();
                }
            }
            else
            {
                ASSERT( pObj->ISA(SwAnchoredDrawObject),
                        "<SwLayoutFrm::NotifyFlys() - anchored object of unexcepted type" );
                if ( IsAnLower( pObj->GetAnchorFrm() ) )
                {
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
|*  Ersterstellung      OK 22. Nov. 94
|*  Letzte Aenderung    MA 10. Jan. 97
|*
|*************************************************************************/

void SwFlyFrm::NotifyDrawObj()
{
    GetVirtDrawObj()->SetRect();
    GetVirtDrawObj()->_SetRectsDirty();
    GetVirtDrawObj()->SetChanged();
    GetVirtDrawObj()->BroadcastObjectChange();
    if ( GetFmt()->GetSurround().IsContour() )
        ClrContourCache( GetVirtDrawObj() );
}

/*************************************************************************
|*
|*  SwFlyFrm::CalcRel()
|*
|*  Ersterstellung      MA 13. Jun. 96
|*  Letzte Aenderung    MA 10. Oct. 96
|*
|*************************************************************************/

Size SwFlyFrm::CalcRel( const SwFmtFrmSize &rSz ) const
{
    Size aRet( rSz.GetSize() );

    const SwFrm *pRel = IsFlyLayFrm() ? GetAnchorFrm() : GetAnchorFrm()->GetUpper();
    if( pRel ) // LAYER_IMPL
    {
        long nRelWidth = LONG_MAX, nRelHeight = LONG_MAX;
        const ViewShell *pSh = GetShell();
        if ( ( pRel->IsBodyFrm() || pRel->IsPageFrm() ) &&
             GetFmt()->GetDoc()->IsBrowseMode() &&
             pSh && pSh->VisArea().HasArea() )
        {
            nRelWidth  = pSh->VisArea().Width();
            nRelHeight = pSh->VisArea().Height();
            const Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
            nRelWidth -= 2*aBorder.Width();
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

SwTwips lcl_CalcAutoWidth( const SwLayoutFrm& rFrm )
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
                 HORI_NONE == ((SwTabFrm*)pFrm)->GetFmt()->GetHoriOrient().GetHoriOrient() )
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
|*  SwFlyFrm::AddSpacesToFrm
|*
|*  Ersterstellung      MA 11. Nov. 96
|*  Letzte Aenderung    MA 10. Mar. 97
|*
|*************************************************************************/

//SwRect SwFlyFrm::AddSpacesToFrm() const
//{
//  SwRect aRect( Frm() );
//  const SvxULSpaceItem &rUL = GetFmt()->GetULSpace();
//  const SvxLRSpaceItem &rLR = GetFmt()->GetLRSpace();
//  aRect.Left( Max( aRect.Left() - long(rLR.GetLeft()), 0L ) );
//  aRect.SSize().Width() += rLR.GetRight();
//  aRect.Top( Max( aRect.Top() - long(rUL.GetUpper()), 0L ) );
//  aRect.SSize().Height()+= rUL.GetLower();
//  return aRect;
//}

/*************************************************************************
|*
|*  SwFlyFrm::GetContour()
|*
|*  Ersterstellung      MA 09. Jan. 97
|*  Letzte Aenderung    MA 10. Jan. 97
|*
|*************************************************************************/
/// OD 16.04.2003 #i13147# - If called for paint and the <SwNoTxtFrm> contains
/// a graphic, load of intrinsic graphic has to be avoided.
BOOL SwFlyFrm::GetContour( PolyPolygon&   rContour,
                           const sal_Bool _bForPaint ) const
{
    BOOL bRet = FALSE;
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
        ASSERT( pGrfObj, "SwFlyFrm::GetContour() - No Graphic/GraphicObject found at <SwNoTxtNode>." );
        if ( pGrfObj && pGrfObj->GetType() != GRAPHIC_NONE )
        {
            if( !pNd->HasContour() )
            {
                // OD 16.04.2003 #i13147# - no <CreateContour> for a graphic
                // during paint. Thus, return (value of <bRet> should be <FALSE>).
                if ( pGrfNd && _bForPaint )
                {
                    ASSERT( false, "SwFlyFrm::GetContour() - No Contour found at <SwNoTxtNode> during paint." );
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
            ((SwNoTxtFrm*)Lower())->GetGrfArea( aClip, &aOrig, FALSE );
            // OD 16.04.2003 #i13147# - copy method code <SvxContourDlg::ScaleContour(..)>
            // in order to avoid that graphic has to be loaded for contour scale.
            //SvxContourDlg::ScaleContour( rContour, aGrf, MAP_TWIP, aOrig.SSize() );
            {
                OutputDevice*   pOutDev = Application::GetDefaultDevice();
                const MapMode   aDispMap( MAP_TWIP );
                const MapMode   aGrfMap( pGrfObj->GetPrefMapMode() );
                const Size      aGrfSize( pGrfObj->GetPrefSize() );
                double          fScaleX;
                double          fScaleY;
                Size            aOrgSize;
                Point           aNewPoint;
                BOOL            bPixelMap = aGrfMap.GetMapUnit() == MAP_PIXEL;

                if ( bPixelMap )
                    aOrgSize = pOutDev->PixelToLogic( aGrfSize, aDispMap );
                else
                    aOrgSize = pOutDev->LogicToLogic( aGrfSize, aGrfMap, aDispMap );

                if ( aOrgSize.Width() && aOrgSize.Height() )
                {
                    fScaleX = (double) aOrig.Width() / aOrgSize.Width();
                    fScaleY = (double) aOrig.Height() / aOrgSize.Height();

                    for ( USHORT j = 0, nPolyCount = rContour.Count(); j < nPolyCount; j++ )
                    {
                        Polygon& rPoly = rContour[ j ];

                        for ( USHORT i = 0, nCount = rPoly.GetSize(); i < nCount; i++ )
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
            bRet = TRUE;
        }
    }
    return bRet;
}

BOOL SwFlyFrm::ConvertHoriTo40( SwHoriOrient &rHori, SwRelationOrient &rRel,
                                SwTwips &rPos ) const
{
    ASSERT( rHori > PRTAREA, "ConvertHoriTo40: Why?" );
    if( !GetAnchorFrm() )
        return FALSE;
    rHori = HORI_NONE;
    rRel = FRAME;
    rPos = Frm().Left() - GetAnchorFrm()->Frm().Left();
    return TRUE;
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
}

SwFrmFmt& SwFlyFrm::GetFrmFmt()
{
    ASSERT( GetFmt(),
            "<SwFlyFrm::GetFrmFmt()> - missing frame format -> crash." );
    return *GetFmt();
}
const SwFrmFmt& SwFlyFrm::GetFrmFmt() const
{
    ASSERT( GetFmt(),
            "<SwFlyFrm::GetFrmFmt()> - missing frame format -> crash." );
    return *GetFmt();
}

const SwRect SwFlyFrm::GetObjRect() const
{
    return Frm();
}
void SwFlyFrm::SetObjTop( const SwTwips _nTop )
{
    Frm().Pos().Y() = _nTop;
}
void SwFlyFrm::SetObjLeft( const SwTwips _nLeft )
{
    Frm().Pos().X() = _nLeft;
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
