/*************************************************************************
 *
 *  $RCSfile: fly.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ama $ $Date: 2001-03-02 10:42:31 $
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
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _IMAP_HXX //autogen
#include <svtools/imap.hxx>
#endif
#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _POLY_HXX //autogen
#include <vcl/poly.hxx>
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

//Aus dem PageFrm:

SV_IMPL_PTRARR_SORT(SwSortDrawObjs,SdrObjectPtr)


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
    aRelPos(),
    pAnchor( 0 ),
    pPrevLink( 0 ),
    pNextLink( 0 ),
    bInCnt( FALSE ),
    bAtCnt( FALSE ),
    bLayout( FALSE ),
    bAutoPosition( FALSE )
{
    nType = FRM_FLY;

    bInvalid = bNotifyBack = TRUE;
    bLocked  = bMinHeight =
    bHeightClipped = bWidthClipped = bFormatHeightOnly = FALSE;

    //Grosseneinstellung, Fixe groesse ist immer die Breite
    const SwFmtFrmSize &rFrmSize = pFmt->GetFrmSize();
    bVarHeight = TRUE;
    bFixWidth = TRUE;
    Frm().Width( rFrmSize.GetWidth() );
    Frm().Height( rFrmSize.GetHeight() );

    //Hoehe Fix oder Variabel oder was?
    if ( rFrmSize.GetSizeType() == ATT_MIN_SIZE )
        bMinHeight = TRUE;
    else if ( rFrmSize.GetSizeType() == ATT_FIX_SIZE )
        bFixHeight = TRUE;

    //Spalten?
    const SwFmtCol &rCol = pFmt->GetCol();
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

    //Erst das Init, dann den Inhalt, denn zum Inhalt koennen  widerum
    //Objekte/Rahmen gehoeren die dann angemeldet werden.
    InitDrawObj( FALSE );

    //Fuer Verkettungen kann jetzt die Verbindung aufgenommen werden. Wenn
    //ein Nachbar nicht existiert, so macht das nichts, denn dieser wird ja
    //irgendwann Konsturiert und nimmt dann die Verbindung auf.
    const SwFmtChain &rChain = pFmt->GetChain();
    if ( rChain.GetPrev() || rChain.GetNext() )
    {
        if ( rChain.GetNext() )
        {
            SwFlyFrm *pFollow = FindChainNeighbour( *rChain.GetNext(), pAnch );
            if ( pFollow )
            {
                ASSERT( !pFollow->GetPrevLink(), "wrong chain detected" );
                if ( !pFollow->GetPrevLink() )
                    SwFlyFrm::ChainFrames( this, pFollow );
            }
        }
        if ( rChain.GetPrev() )
        {
            SwFlyFrm *pMaster = FindChainNeighbour( *rChain.GetPrev(), pAnch );
            if ( pMaster )
            {
                ASSERT( !pMaster->GetNextLink(), "wrong chain detected" );
                if ( !pMaster->GetNextLink() )
                    SwFlyFrm::ChainFrames( pMaster, this );
            }
        }
    }

    if ( !GetPrevLink() ) //Inhalt gehoert sonst immer dem Master und meiner Zaehlt nicht
    {
        const SwFmtCntnt &rCntnt = pFmt->GetCntnt();
        ASSERT( rCntnt.GetCntntIdx(), ":-( Kein Inhalt vorbereitet." );
        ULONG nIndex = rCntnt.GetCntntIdx()->GetIndex();
        // Lower() bedeutet SwColumnFrm, eingefuegt werden muss der Inhalt dann in den (Column)BodyFrm
        ::_InsertCnt( Lower() ? (SwLayoutFrm*)((SwLayoutFrm*)Lower())->Lower() : (SwLayoutFrm*)this,
                      pFmt->GetDoc(), nIndex );

        //NoTxt haben immer eine FixHeight.
        if ( Lower() && Lower()->IsNoTxtFrm() )
        {
            bFixHeight = TRUE;
            bMinHeight = FALSE;
        }
    }

    //Und erstmal in den Wald stellen die Kiste, damit bei neuen Dokument nicht
    //unnoetig viel formatiert wird.
    Frm().Pos().X() = Frm().Pos().Y() = WEIT_WECH;
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
    if( GetFmt() && !GetFmt()->GetDoc()->IsInDtor() )
    {
        //Aus der Verkettung loessen.
        if ( GetPrevLink() )
            UnchainFrames( GetPrevLink(), this );
        if ( GetNextLink() )
            UnchainFrames( this, GetNextLink() );

        //Unterstruktur zerstoeren, wenn dies erst im LayFrm DTor passiert ist's
        //zu spaet, denn dort ist die Seite nicht mehr erreichbar (muss sie aber
        //sein, damit sich ggf. weitere Flys abmelden koennen).
        SwFrm *pFrm = pLower;
        while ( pFrm )
        {
            //Erst die Flys des Frm vernichten, denn diese koennen sich sonst nach
            //dem Remove nicht mehr bei der Seite abmelden.
            while ( pFrm->GetDrawObjs() && pFrm->GetDrawObjs()->Count() )
            {   SdrObject *pObj = (*pFrm->GetDrawObjs())[0];
                SdrObjUserCall *pUserCall;
                if ( pObj->IsWriterFlyFrame() )
                    delete ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
                else if ( 0 != ( pUserCall = GetUserCall(pObj) ) )
                    ((SwDrawContact*)pUserCall)->DisconnectFromLayout();
            }
            pFrm->Remove();
            delete pFrm;
            pFrm = pLower;
        }

        //Damit kein zerstoerter Cntnt als Turbo bei der Root angemeldet bleiben
        //kann verhindere ich hier, dass dort ueberhaupt noch einer angemeldet
        //ist.
        InvalidatePage();

        //Tschuess sagen.
        if ( pAnchor )
            pAnchor->RemoveFly( this );
    }
    FinitDrawObj();
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
    pDrawObj = pContact->CreateNewRef( this );

    //Den richtigen Layer setzen.
    pDrawObj->SetLayer( GetFmt()->GetOpaque().GetValue() ?
                            GetFmt()->GetDoc()->GetHeavenId() :
                            GetFmt()->GetDoc()->GetHellId() );
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
    if ( !pDrawObj )
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

    pDrawObj->SetUserCall( 0 ); //Ruft sonst Delete des ContactObj
    delete pDrawObj;            //Meldet sich selbst beim Master ab.
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
        const long nBottom = pMaster->Prt().Bottom() + pMaster->Frm().Top();
        while ( pInva )
        {
            if ( pInva->Frm().Bottom() >= nBottom )
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
                ::RestoreCntnt( pTmp, pUpper, pMaster->FindLastLower() );
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
        pAnch = GetAnchor();

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
            if ( pFly->GetAnchor() )
            {
                if ( pFly->GetAnchor()->IsInFly() )
                {
                    if ( pFly->GetAnchor()->FindFlyFrm() == pLay )
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
    if ( rFrmSize.GetSizeType() == ATT_VAR_SIZE )
        bFixHeight = bMinHeight = FALSE;
    else
    {
        if ( rFrmSize.GetSizeType() == ATT_FIX_SIZE )
        {   bFixHeight = TRUE;
            bMinHeight = FALSE;
        }
        else if ( rFrmSize.GetSizeType() == ATT_MIN_SIZE )
        {   bFixHeight = FALSE;
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
            const SwRect aOld( AddSpacesToFrm() );
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
            bFixHeight = TRUE;
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
            //Achtung! _immer_ Aktion in ChgRePos() mitpflegen.
            rInvFlags |= 0x09;
            break;

        case RES_SURROUND:
            {
            rInvFlags |= 0x40;
            //Der Hintergrund muss benachrichtigt und Invalidiert werden.
            const SwRect aTmp( AddSpacesToFrm() );
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
            }
            break;

        case RES_PROTECT:
            {
            const SvxProtectItem *pP = (SvxProtectItem*)pNew;
            GetVirtDrawObj()->SetMoveProtect( pP->IsPosProtected()  );
            GetVirtDrawObj()->SetResizeProtect( pP->IsSizeProtected() );
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
                SwRect aNew( AddSpacesToFrm() );
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
            GetVirtDrawObj()->SetMoveProtect( rP.IsPosProtected()   );
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

            break;
        }
        case RES_UL_SPACE:
        case RES_LR_SPACE:
            {
            rInvFlags |= 0x41;
            if ( GetFmt()->GetDoc()->IsBrowseMode() )
                GetFmt()->GetDoc()->GetRootFrm()->InvalidateBrowseWidth();
            SwRect aNew( AddSpacesToFrm() );
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

        case RES_OPAQUE:
            {
            if ( pSh )
                pSh->InvalidateWindows( Frm() );
            const BYTE nId = ((SvxOpaqueItem*)pNew)->GetValue() ?
                                GetFmt()->GetDoc()->GetHeavenId() :
                                GetFmt()->GetDoc()->GetHellId();
            GetVirtDrawObj()->SetLayer( nId );
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
    if ( GetAnchor() && 0 != (pFrm = GetAnchor()->FindFlyFrm()) )
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
    if ( GetCurRelPos() != rNewPos )
    {
        SwFrmFmt *pFmt = GetFmt();
        SwTwips nTmpY = rNewPos.Y() == LONG_MAX ? 0 : rNewPos.Y();
        SfxItemSet aSet( pFmt->GetDoc()->GetAttrPool(),
                         RES_VERT_ORIENT, RES_HORI_ORIENT);

        SwFmtVertOrient aVert( pFmt->GetVertOrient() );
        SwTxtFrm *pAutoFrm = NULL;
        if( IsFlyAtCntFrm() || VERT_NONE != aVert.GetVertOrient() )
        {
            if( REL_CHAR == aVert.GetRelationOrient() && IsAutoPos() )
            {
                if( LONG_MAX != rNewPos.Y() )
                {
                    aVert.SetVertOrient( VERT_NONE );
                    xub_StrLen nOfs =
                        pFmt->GetAnchor().GetCntntAnchor()->nContent.GetIndex();
                    ASSERT( GetAnchor()->IsTxtFrm(), "TxtFrm expected" );
                    pAutoFrm = (SwTxtFrm*)GetAnchor();
                    while( pAutoFrm->GetFollow() &&
                           pAutoFrm->GetFollow()->GetOfst() <= nOfs )
                    {
                        if( pAutoFrm == GetAnchor() )
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

        //Fuer Flys im Cnt ist die horizontale Ausrichtung uninterressant,
        //den sie ist stets 0.
        if ( !IsFlyInCntFrm() )
        {
            SwTwips nTmpX = rNewPos.X() == LONG_MAX ? 0 : rNewPos.X();
            SwFmtHoriOrient aHori( pFmt->GetHoriOrient() );
            if( IsFlyAtCntFrm() || HORI_NONE != aHori.GetHoriOrient() )
            {
                aHori.SetHoriOrient( HORI_NONE );
                if( REL_CHAR == aHori.GetRelationOrient() && IsAutoPos() )
                {
                    if( LONG_MAX != rNewPos.X() )
                    {
                        if( !pAutoFrm )
                        {
                            xub_StrLen nOfs = pFmt->GetAnchor().GetCntntAnchor()
                                          ->nContent.GetIndex();
                            ASSERT( GetAnchor()->IsTxtFrm(), "TxtFrm expected");
                            pAutoFrm = (SwTxtFrm*)GetAnchor();
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

        const SwTwips nUL  = pAttrs->CalcTopLine() + pAttrs->CalcBottomLine();
        const SwTwips nLR  = pAttrs->CalcLeftLine()+ pAttrs->CalcRightLine();
        const Size    &rSz = pAttrs->GetSize();
        const SwFmtFrmSize &rFrmSz = GetFmt()->GetFrmSize();

        ASSERT( rSz.Height() != 0 || rFrmSz.GetHeightPercent(), "Hoehe des RahmenAttr ist 0." );
        ASSERT( rSz.Width()  != 0 || rFrmSz.GetWidthPercent(), "Breite des RahmenAttr ist 0." );

        if ( !HasFixSize( pHeight ) )
        {
            //Die Groesse in der VarSize wird durch den Inhalt plus den
            //Raendern bestimmt.
            SwTwips nRemaining = 0;
            const long nMinHeight = IsMinHeight() ?
                CalcRel( rFrmSz ).Height() : 0;
            if ( Lower() )
            {
                if ( Lower()->IsColumnFrm() )
                {
                    FormatWidthCols( *pAttrs, nUL, nMinHeight );
                    nRemaining = Lower()->Frm().Height();
                }
                else
                {
                    SwFrm *pFrm = Lower();
                    while ( pFrm )
                    {   nRemaining += pFrm->Frm().Height();
                        if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsUndersized() )
                            // Dieser TxtFrm waere gern ein bisschen groesser
                            nRemaining += ((SwTxtFrm*)pFrm)->GetParHeight()
                                    - pFrm->Prt().Height();
                        else if( pFrm->IsSctFrm() && ((SwSectionFrm*)pFrm)->IsUndersized() )
                            nRemaining += ((SwSectionFrm*)pFrm)->Undersize();
                        pFrm = pFrm->GetNext();
                    }
                }
            }
            //Die Minimalgroesse darf falls erwuenscht nicht unterschritten
            //werden.
#ifndef PRODUCT
            if ( IsMinHeight() )
            {
                const long nMinHeightII = CalcRel( rFrmSz ).Height();
                ASSERT( nMinHeight==nMinHeightII, "FlyFrm::Format: Changed MinHeight" );
            }
#endif
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
            Prt().Height( nRemaining );
            Frm().Height( nRemaining + nUL );
            bValidSize = TRUE;
        }
        else
        {
            bValidSize = TRUE;  //Fixe Frms formatieren sich nicht.
                                //Flys stellen ihre Groesse anhand des Attr ein.
            const SwTwips nMin = MINFLY + nUL;
            Frm().Height( Max( CalcRel( rFrmSz ).Height(), nMin ) );
            Prt().Height( Frm().Height() - nUL );
        }
        if ( !bFormatHeightOnly )
        {
            const SwTwips nMin = MINFLY + nLR;
            Frm().Width( Max( CalcRel( rFrmSz ).Width(), nMin ) );
            Prt().Width( Frm().Width() - (nLR) );
        }
    }
    ColUnlock();
}

void CalcCntnt( SwLayoutFrm *pLay, BOOL bNoColl )
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
        SwFlyFrm *pAgainFly1 = 0,       //Oszillation abknipsen.
                 *pAgainFly2 = 0;
        SwFrm* pLast;
        do
        {
            pLast = pFrm;
            if ( pFrm->GetUpper()->Prt().Width() != pFrm->Frm().Width() )
            {
                pFrm->Prepare( PREP_FIXSIZE_CHG );
                pFrm->_InvalidateSize();
            }

            if ( pFrm->IsTabFrm() )
            {
                ((SwTabFrm*)pFrm)->bCalcLowers = TRUE;
                if ( ((SwTabFrm*)pFrm)->IsFollow() )
                    ((SwTabFrm*)pFrm)->bLockBackMove = TRUE;
            }

            pFrm->Calc();

            //Dumm aber wahr, die Flys muessen mitkalkuliert werden.
            BOOL bAgain = FALSE;
            if ( pFrm->GetDrawObjs() )
            {
                USHORT nCnt = pFrm->GetDrawObjs()->Count();
                for ( USHORT i = 0; i < nCnt; ++i )
                {
                    SdrObject *pO = (*pFrm->GetDrawObjs())[i];
                    if ( pO->IsWriterFlyFrame() )
                    {
                        SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                        pFly->InvalidatePos();
                        SwRect aRect( pFly->Frm() );
                        pFly->Calc();
                        if ( aRect != pFly->Frm() )
                        {
                            bAgain = TRUE;
                            if ( pAgainFly2 == pFly )
                            {
                                //Oszillation unterbinden.
                                SwFrmFmt *pFmt = pFly->GetFmt();
                                SwFmtSurround aAttr( pFmt->GetSurround() );
                                if( SURROUND_THROUGHT != aAttr.GetSurround() )
                                {
                                    // Bei autopositionierten hilft manchmal nur
                                    // noch, auf Durchlauf zu schalten
                                    if( pFly->IsAutoPos() &&
                                        SURROUND_PARALLEL == aAttr.GetSurround() )
                                        aAttr.SetSurround( SURROUND_THROUGHT );
                                    else
                                        aAttr.SetSurround( SURROUND_PARALLEL );
                                    pFmt->LockModify();
                                    pFmt->SetAttr( aAttr );
                                    pFmt->UnlockModify();
                                }
                            }
                            else
                            {
                                if ( pAgainFly1 == pFly )
                                    pAgainFly2 = pFly;
                                pAgainFly1 = pFly;
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
            pFrm = pFrm->FindNext();
            if( pFrm && pFrm->IsSctFrm() && pSect )
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
        } while ( pFrm && ( pLay->IsAnLower( pFrm ) ||
                ( pSect && ( ( pSect->HasFollow() && ( pLay->IsAnLower( pLast )
                ||(pLast->IsInSct() && pLast->FindSctFrm()->IsAnFollow(pSect)) )
                  && pSect->GetFollow()->IsAnLower( pFrm ) ) || ( pFrm->IsInSct()
                  && pFrm->FindSctFrm()->IsAnFollow( pSect ) ) ) ) ) );
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

void SwFlyFrm::MakeFlyPos()
{
    if ( !bValidPos )
    {   bValidPos = TRUE;
        GetAnchor()->Calc();

            //Die Werte in den Attributen muessen ggf. upgedated werden,
            //deshalb werden hier Attributinstanzen und Flags benoetigt.
        SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
        BOOL bFlyAtFly = FLY_AT_FLY == pFmt->GetAnchor().GetAnchorId();
        SwFmtVertOrient aVert( pFmt->GetVertOrient() );
        SwFmtHoriOrient aHori( pFmt->GetHoriOrient() );
        const SvxLRSpaceItem &rLR = pFmt->GetLRSpace();
        const SvxULSpaceItem &rUL = pFmt->GetULSpace();
        FASTBOOL bVertChgd = FALSE,
                 bHoriChgd = FALSE;

        //Horizontale und vertikale Positionen werden getrennt berechnet.
        //Sie koennen jeweils Fix oder Variabel (automatisch) sein.

        //Erst die vertikale Position
        BOOL bVertPrt = aVert.GetRelationOrient() == PRTAREA ||
                        aVert.GetRelationOrient() == REL_PG_PRTAREA;
        if ( aVert.GetVertOrient() == VERT_NONE )
        {
            SwTwips nYPos = aVert.GetPos();
            if ( bVertPrt )
            {
                nYPos += GetAnchor()->Prt().Top();
                if( GetAnchor()->IsPageFrm() )
                {
                    SwFrm* pPrtFrm = ((SwPageFrm*)GetAnchor())->Lower();
                    if( pPrtFrm && pPrtFrm->IsHeaderFrm() )
                        nYPos += pPrtFrm->Frm().Height();
                }
            }
            if( nYPos < 0 )
#ifdef AMA_OUT_OF_FLY
              if( !bFlyAtFly  )
#endif
                nYPos = 0;
            aRelPos.Y() = nYPos;
        }
        else
        {   //Zuerst den Bezugsrahmen festlegen (PrtArea oder Frame)
            SwTwips nRel, nAdd;
            if ( bVertPrt )
            {   nRel = GetAnchor()->Prt().Height();
                nAdd = GetAnchor()->Prt().Top();
                if( GetAnchor()->IsPageFrm() )
                {
                    // Wenn wir am SeitenTextBereich ausgerichtet sind,
                    // sollen Kopf- und Fusszeilen _nicht_ mit zaehlen.
                    SwFrm* pPrtFrm = ((SwPageFrm*)GetAnchor())->Lower();
                    while( pPrtFrm )
                    {
                        if( pPrtFrm->IsHeaderFrm() )
                        {
                            nRel -= pPrtFrm->Frm().Height();
                            nAdd += pPrtFrm->Frm().Height();
                        }
                        else if( pPrtFrm->IsFooterFrm() )
                            nRel -= pPrtFrm->Frm().Height();
                        pPrtFrm = pPrtFrm->GetNext();
                    }
                }
            }
            else
            {   nRel = GetAnchor()->Frm().Height();
                nAdd = 0;
            }
            // Bei rahmengebunden Rahmen wird nur vertikal unten oder zentriert
            // ausgerichtet, wenn der Text durchlaeuft oder der Anker eine feste
            // Hoehe besitzt.
            if( bFlyAtFly && VERT_TOP != aVert.GetVertOrient() &&
                SURROUND_THROUGHT != pFmt->GetSurround().GetSurround() &&
                !GetAnchor()->HasFixSize( pHeight ) )
                aRelPos.Y() = rUL.GetUpper();
            else if ( aVert.GetVertOrient() == VERT_CENTER )
                aRelPos.Y() = (nRel / 2) - (aFrm.Height() / 2);
            else if ( aVert.GetVertOrient() == VERT_BOTTOM )
                aRelPos.Y() = nRel - (aFrm.Height() + rUL.GetLower());
            else
                aRelPos.Y() = rUL.GetUpper();
            aRelPos.Y() += nAdd;

            if ( aVert.GetPos() != aRelPos.Y() )
            {   aVert.SetPos( aRelPos.Y() );
                bVertChgd = TRUE;
            }
        }

        //Fuer die Hoehe der Seiten im Browser muessen wir etwas tricksen. Das
        //Grow muessen wir auf den Body rufen; wegen ggf. eingeschalteter
        //Kopfzeilen und weil die Seite sowieso eine fix-Hoehe hat.
        if ( !bFlyAtFly && GetFmt()->GetDoc()->IsBrowseMode() &&
             GetAnchor()->IsPageFrm() ) //Was sonst?
        {
            const long nAnchorBottom = GetAnchor()->Frm().Bottom();
            const long nBottom = GetAnchor()->Frm().Top() + aRelPos.Y() + Frm().Height();
            if ( nAnchorBottom < nBottom )
            {
                ((SwPageFrm*)GetAnchor())->FindBodyCont()->
                                    Grow( nBottom - nAnchorBottom, pHeight );
            }
        }


        //Jetzt die Horizontale Position
        const BOOL bToggle = aHori.IsPosToggle()&&!FindPageFrm()->OnRightPage();
        BOOL bTmpToggle = bToggle;
        //und wieder erst der Bezugsrahmen
        SwTwips nRel, nAdd;
        SwHoriOrient eHOri = aHori.GetHoriOrient();
        if( bToggle )
        {
            if( HORI_RIGHT == eHOri )
                eHOri = HORI_LEFT;
            else if( HORI_LEFT == eHOri )
                eHOri = HORI_RIGHT;
        }
        switch ( aHori.GetRelationOrient() )
        {
            case PRTAREA:
            case REL_PG_PRTAREA:
            {
                nRel = GetAnchor()->Prt().Width();
                nAdd = GetAnchor()->Prt().Left();
                break;
            }
            case REL_PG_LEFT:
            case REL_FRM_LEFT:
                bTmpToggle = !bToggle;
                // kein break;
            case REL_PG_RIGHT:
            case REL_FRM_RIGHT:
            {
                if ( bTmpToggle )    // linker Seitenrand
                {
                    nRel = GetAnchor()->Prt().Left();
                    nAdd = 0;
                }
                else            // rechter Seitenrand
                {
                    nRel = GetAnchor()->Frm().Width();
                    nAdd = GetAnchor()->Prt().Right();
                    nRel -= nAdd;
                }
                break;
            }
            default:
            {
                nRel = GetAnchor()->Frm().Width();
                nAdd = 0;
                break;
            }
        }
        if ( aHori.GetHoriOrient() == HORI_NONE )
        {
            if( bToggle )
                aRelPos.X() = nRel - aFrm.Width() - aHori.GetPos();
            else
                aRelPos.X() = aHori.GetPos();
        }
        else if ( HORI_CENTER == eHOri )
            aRelPos.X() = (nRel / 2) - (aFrm.Width() / 2);
        else if ( HORI_RIGHT == eHOri )
            aRelPos.X() = nRel - (aFrm.Width() + rLR.GetRight());
        else
            aRelPos.X() = rLR.GetLeft();
        aRelPos.X() += nAdd;

        if( aRelPos.X() < 0 )
            aRelPos.X() = 0;
        if ( HORI_NONE != aHori.GetHoriOrient() &&
            aHori.GetPos() != aRelPos.X() )
        {   aHori.SetPos( aRelPos.X() );
            bHoriChgd = TRUE;
        }
        //Die Absolute Position ergibt sich aus der absoluten Position des
        //Ankers plus der relativen Position.
        aFrm.Pos( aRelPos );
        aFrm.Pos() += GetAnchor()->Frm().Pos();

        //Und ggf. noch die aktuellen Werte im Format updaten, dabei darf
        //zu diesem Zeitpunkt natuerlich kein Modify verschickt werden.
        pFmt->LockModify();
        if ( bVertChgd )
            pFmt->SetAttr( aVert );
        if ( bHoriChgd )
            pFmt->SetAttr( aHori );
        pFmt->UnlockModify();
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

        //Position einstellen.
        aPrt.Left( rAttrs.CalcLeftLine() );
        aPrt.Top ( rAttrs.CalcTopLine()  );

        //Sizes einstellen; die Groesse gibt der umgebende Frm vor, die
        //die Raender werden einfach abgezogen.
        aPrt.Width ( aFrm.Width() - (rAttrs.CalcRightLine() + aPrt.Left()) );
        aPrt.Height( aFrm.Height()- (aPrt.Top() + rAttrs.CalcBottomLine()));
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

SwTwips SwFlyFrm::_Grow( SwTwips nDist, const SzPtr pDirection, BOOL bTst )
{
    if ( Lower() && !IsColLocked() && !HasFixSize( pDirection ) )
    {
        if ( Frm().SSize().*pDirection > 0 &&
             nDist > (LONG_MAX - Frm().SSize().*pDirection) )
            nDist = LONG_MAX - Frm().SSize().*pDirection;

        if ( nDist <= 0L )
            return 0L;

        if ( Lower()->IsColumnFrm() )
        {   //Bei Spaltigkeit ubernimmt das Format die Kontrolle ueber
            //das Wachstum (wg. des Ausgleichs).
            if ( !bTst )
            {   _InvalidatePos();
                InvalidateSize();
            }
            return 0L;
        }

        if ( !bTst )
        {
            const SwRect aOld( AddSpacesToFrm() );
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
            const SwRect aNew( AddSpacesToFrm() );
            if ( aOld != aNew )
                ::Notify( this, FindPageFrm(), aOld );
            return aNew.SSize().*pDirection - aOld.SSize().*pDirection;
        }
        return nDist;
    }
    return 0L;
}

SwTwips SwFlyFrm::_Shrink( SwTwips nDist, const SzPtr pDirection, BOOL bTst )
{
    if ( Lower() && !IsColLocked() && !HasFixSize( pDirection ) )
    {
        if ( nDist > Frm().SSize().*pDirection )
            nDist = Frm().SSize().*pDirection;

        SwTwips nVal = IsMinHeight() && pDirection == pHeight ?
            Min( nDist, Frm().Height() - GetFmt()->GetFrmSize().GetHeight() ) :
            nDist;
        if ( nVal <= 0L )
            return 0L;

        if ( Lower()->IsColumnFrm() )
        {   //Bei Spaltigkeit ubernimmt das Format die Kontrolle ueber
            //das Wachstum (wg. des Ausgleichs).
            if ( !bTst )
            {
                SwRect aOld( AddSpacesToFrm() );
                Frm().SSize().*pDirection -= nVal;
                Prt().SSize().*pDirection -= nVal;
                _InvalidatePos();
                InvalidateSize();
                ::Notify( this, FindPageFrm(), aOld );
                NotifyDrawObj();
                if ( GetAnchor()->IsInFly() )
                    GetAnchor()->FindFlyFrm()->Shrink( nDist, pDirection, bTst );
            }
            return 0L;
        }

        if ( !bTst )
        {
            const SwRect aOld( AddSpacesToFrm() );
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
            const SwRect aNew( AddSpacesToFrm() );
            if ( aOld != aNew )
            {
                ::Notify( this, FindPageFrm(), aOld );
                if ( GetAnchor()->IsInFly() )
                    GetAnchor()->FindFlyFrm()->Shrink( nDist, pDirection, bTst );
            }
            return aOld.SSize().*pDirection - aNew.SSize().*pDirection;
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
        if ( Abs(aNewSize.Height() - Frm().Height()) > 1 )
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

BOOL SwFlyFrm::IsLowerOf( const SwLayoutFrm *pUpper ) const
{
    ASSERT( GetAnchor(), "8-( Fly is lost in Space." );
    const SwFrm *pFrm = GetAnchor();
    do
    {   if ( pFrm == pUpper )
            return TRUE;
        pFrm = pFrm->IsFlyFrm() ? ((const SwFlyFrm*)pFrm)->GetAnchor() :
                                  pFrm->GetUpper();
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
        pDrawObjs = new SwDrawObjs();
    SdrObject *pObj = pNew->GetVirtDrawObj();
    pDrawObjs->Insert( pObj, pDrawObjs->Count() );
    pNew->ChgAnchor( this );

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
            pPage->SwPageFrm::AppendFly( pNew );
        }
        else
            pPage->SwPageFrm::AppendFly( pNew );
    }
}

void SwFrm::RemoveFly( SwFlyFrm *pToRemove )
{
    //Bei der Seite Abmelden - kann schon passiert sein weil die Seite
    //bereits destruiert wurde.
    SwPageFrm *pPage = pToRemove->FindPageFrm();
    if ( pPage && pPage->GetSortedObjs() )
        pPage->SwPageFrm::RemoveFly( pToRemove );

    const SdrObjectPtr pObj = pToRemove->GetVirtDrawObj();
    pDrawObjs->Remove( pDrawObjs->GetPos( pObj ) );
    if ( !pDrawObjs->Count() )
        DELETEZ( pDrawObjs );

    pToRemove->ChgAnchor( 0 );

    if ( !pToRemove->IsFlyInCntFrm() && GetUpper() && IsInTab() )//MA_FLY_HEIGHT
        GetUpper()->InvalidateSize();
}

/*************************************************************************
|*
|*  SwFrm::AppendDrawObj(), RemoveDrawObj()
|*
|*  Ersterstellung      MA 09. Jan. 95
|*  Letzte Aenderung    MA 26. Jun. 95
|*
|*************************************************************************/

void SwFrm::AppendDrawObj( SwDrawContact *pNew )
{
    if ( pNew->GetAnchor() && pNew->GetAnchor() != this )
        pNew->DisconnectFromLayout( FALSE );

    SdrObject *pObj = pNew->GetMaster();
    if ( pNew->GetAnchor() != this )
    {
        if ( !pDrawObjs )
            pDrawObjs = new SwDrawObjs();
        pDrawObjs->Insert( pObj, pDrawObjs->Count() );
        pNew->ChgAnchor( this );
    }
    const SwFmtAnchor &rAnch = pNew->GetFmt()->GetAnchor();
    if( FLY_AUTO_CNTNT == rAnch.GetAnchorId() )
    {
        SwRect aTmpRect;
        SwPosition *pPos = (SwPosition*)rAnch.GetCntntAnchor();
        GetCharRect( aTmpRect, *pPos );
        pNew->GetMaster()->SetAnchorPos( aTmpRect.Pos() );
    }
    else if( FLY_IN_CNTNT != rAnch.GetAnchorId() )
        pNew->GetMaster()->SetAnchorPos( Frm().Pos() );
    //Bei der Seite anmelden; kann sein, dass noch keine da ist - die
    //Anmeldung wird dann in SwPageFrm::PreparePage durch gefuehrt.
    SwPageFrm *pPage = FindPageFrm();
    if ( pPage )
        pPage->SwPageFrm::AppendDrawObj( pNew );
}

void SwFrm::RemoveDrawObj( SwDrawContact *pToRemove )
{
    //Bei der Seite Abmelden - kann schon passiert sein weil die Seite
    //bereits destruiert wurde.
    SwPageFrm *pPage = pToRemove->GetPage();
    if ( pPage && pPage->GetSortedObjs() )
        pPage->SwPageFrm::RemoveDrawObj( pToRemove );

    SdrObject *pObj = pToRemove->GetMaster();
    pDrawObjs->Remove( pDrawObjs->GetPos( pObj ) );
    if ( !pDrawObjs->Count() )
        DELETEZ( pDrawObjs );

    pToRemove->ChgAnchor( 0 );
}

/*************************************************************************
|*
|*  SwFrm::CalcFlys()
|*
|*  Ersterstellung      MA 29. Nov. 96
|*  Letzte Aenderung    MA 29. Nov. 96
|*
|*************************************************************************/

void lcl_MakeFlyPosition( SwFlyFrm *pFly )
{
    if( pFly->IsFlyFreeFrm() )
    {
        ((SwFlyFreeFrm*)pFly)->SwFlyFreeFrm::MakeAll();
        return;
    }

    BOOL bOldLock = pFly->IsLocked();
    pFly->Lock();
    SwFlyNotify aNotify( pFly );
    pFly->MakeFlyPos();
    if( !bOldLock )
        pFly->Unlock();
}

void SwFrm::CalcFlys( BOOL bPosOnly )
{
    if ( GetDrawObjs() )
    {
        USHORT nCnt = GetDrawObjs()->Count();
        for ( USHORT i = 0; i < nCnt; ++i )
        {
            SdrObject *pO = (*GetDrawObjs())[i];
            if ( pO->IsWriterFlyFrame() )
            {
                SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                // Bei autopositionierten (am Zeichen geb.) Rahmen vertrauen wir
                // darauf, dass die Positionierung vom SwTxtFrm::Format vorgenommen
                // wird. Wenn wir sie dagegen hier kalkulieren wuerden, fuehrt es
                // zur Endlosschleife in Bug 50796.
                if ( pFly->IsFlyInCntFrm() )
                    continue;
                if( pFly->IsAutoPos() )
                {
                    if( bPosOnly )
                    {
                        pFly->_Invalidate();
                        pFly->_InvalidatePos();
                    }
                    continue;
                }
                pFly->_Invalidate();
                pFly->_InvalidatePos();

                if ( bPosOnly && pFly->GetValidSizeFlag() && pFly->GetValidPrtAreaFlag() )
                    ::lcl_MakeFlyPosition( pFly );
                else
                {
                    if ( !bPosOnly )
                        pFly->_InvalidateSize();
                    pFly->Calc();
                }
                if ( !GetDrawObjs() )
                    break;
                if ( GetDrawObjs()->Count() < nCnt )
                {
                    --i;
                    --nCnt;
                }
            }
            else
            {
                SwFrmFmt *pFrmFmt = ::FindFrmFmt( pO );
                if( !pFrmFmt ||
                    FLY_IN_CNTNT != pFrmFmt->GetAnchor().GetAnchorId() )
                {
                    pO->SetAnchorPos( Frm().Pos() );
                    ((SwDrawContact*)GetUserCall(pO))->ChkPage();
                }
            }
        }
    }
}


/*************************************************************************
|*
|*  SwLayoutFrm::NotifyFlys()
|*
|*  Ersterstellung      MA 18. Feb. 94
|*  Letzte Aenderung    MA 26. Jun. 96
|*
|*************************************************************************/

void SwLayoutFrm::NotifyFlys()
{
    //Sorgt dafuer, dass untergeordnete Flys pruefen, ob sich sich an
    //die Verhaeltnisse anpassen muessen.

    //Wenn mehr Platz da ist muessen die Positionen und Sizes der
    //Flys berechnet werden, denn es koennte sein, das sie kuenstlich
    //geschrumpft/vershoben wurden und jetzt wieder naeher an ihre
    //Sollwerte gehen duerfen.
    //Ist weniger Platz da, so reicht es wenn sie in das MakeAll laufen
    //der preiswerteste Weg dazu ist die Invalidierung der PrtArea.

    SwPageFrm *pPage = FindPageFrm();
    if ( pPage && pPage->GetSortedObjs() )
    {
        //Die Seite nur einmal antriggern.
        FASTBOOL bPageInva = TRUE;

        SwSortDrawObjs &rObjs = *pPage->GetSortedObjs();
        const BOOL bHeadFoot = IsHeaderFrm() || IsFooterFrm();
        for ( USHORT i = 0; i < rObjs.Count(); ++i )
        {
            SdrObject *pO = rObjs[i];
            if ( pO->IsWriterFlyFrame() )
            {
                SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();

                if ( pFly->Frm().Left() == WEIT_WECH )
                    continue;

                //Wenn der Fly nicht irgendwo ausserhalb liegt braucht er nur
                //nur benachrichtigt werden, wenn er geclipped ist.
                // Bei Header/Footer keine Abkuerzung, denn hier muesste die
                // die PrtArea geprueft werden, die zu diesem Zeitpunkt
                // (ShrinkFrm) noch nicht angepasst ist.
                if ( !bHeadFoot && Frm().IsInside( pFly->Frm() ) && !pFly->IsClipped() )
                    continue;

                const BOOL bLow = pFly->IsLowerOf( this );
                if ( bLow || pFly->GetAnchor()->FindPageFrm() != pPage )
                {
                    pFly->_Invalidate( pPage );
                    if ( !bLow || pFly->IsFlyAtCntFrm() )
                        pFly->_InvalidatePos();
                    else
                        pFly->_InvalidatePrt();
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
    pDrawObj->SetRect();
    pDrawObj->_SetRectsDirty();
    pDrawObj->SetChanged();
    pDrawObj->SendRepaintBroadcast( TRUE ); //Broadcast ohne Repaint!
    if ( GetFmt()->GetSurround().IsContour() )
        ClrContourCache( pDrawObj );
}

/*************************************************************************
|*
|*  SwLayoutFrm::CalcRel()
|*
|*  Ersterstellung      MA 13. Jun. 96
|*  Letzte Aenderung    MA 10. Oct. 96
|*
|*************************************************************************/

Size SwFlyFrm::CalcRel( const SwFmtFrmSize &rSz ) const
{
    Size aRet( rSz.GetSize() );

    const SwFrm *pRel = IsFlyLayFrm() ? GetAnchor() : GetAnchor()->GetUpper();
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
|*  SwFlyFrm::AddSpacesToFrm
|*
|*  Ersterstellung      MA 11. Nov. 96
|*  Letzte Aenderung    MA 10. Mar. 97
|*
|*************************************************************************/

SwRect SwFlyFrm::AddSpacesToFrm() const
{
    SwRect aRect( Frm() );
    const SvxULSpaceItem &rUL = GetFmt()->GetULSpace();
    const SvxLRSpaceItem &rLR = GetFmt()->GetLRSpace();
    aRect.Left( Max( aRect.Left() - long(rLR.GetLeft()), 0L ) );
    aRect.SSize().Width() += rLR.GetRight();
    aRect.Top( Max( aRect.Top() - long(rUL.GetUpper()), 0L ) );
    aRect.SSize().Height()+= rUL.GetLower();
    return aRect;
}

/*************************************************************************
|*
|*  SwFlyFrm::GetContour()
|*
|*  Ersterstellung      MA 09. Jan. 97
|*  Letzte Aenderung    MA 10. Jan. 97
|*
|*************************************************************************/

BOOL SwFlyFrm::GetContour( PolyPolygon & rPoly ) const
{
    BOOL bRet = FALSE;
    if( GetFmt()->GetSurround().IsContour() && Lower() &&
        Lower()->IsNoTxtFrm() )
    {
        SwNoTxtNode *pNd = (SwNoTxtNode*)((SwCntntFrm*)Lower())->GetNode();
        const Graphic aGrf( pNd->GetGraphic() );
        if( GRAPHIC_NONE != aGrf.GetType() )
        {
            if( !pNd->HasContour() )
                pNd->CreateContour();
            pNd->GetContour( rPoly );
            //Der Node haelt das Polygon passend zur Originalgroesse der Grafik
            //hier muss die Skalierung einkalkuliert werden.
            SwRect aClip;
            SwRect aOrig;
            Lower()->Calc();
            ((SwNoTxtFrm*)Lower())->GetGrfArea( aClip, &aOrig, FALSE );
            SvxContourDlg::ScaleContour( rPoly, aGrf, MAP_TWIP, aOrig.SSize() );
            rPoly.Move( aOrig.Left(), aOrig.Top() );
            rPoly.Clip( aClip.SVRect() );
            bRet = TRUE;
        }
    }
    return bRet;
}

BOOL SwFlyFrm::ConvertHoriTo40( SwHoriOrient &rHori, SwRelationOrient &rRel,
                                SwTwips &rPos ) const
{
    ASSERT( rHori > PRTAREA, "ConvertHoriTo40: Why?" );
    if( !GetAnchor() )
        return FALSE;
    rHori = HORI_NONE;
    rRel = FRAME;
    rPos = Frm().Left() - GetAnchor()->Frm().Left();
    return TRUE;
}


