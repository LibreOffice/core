/*************************************************************************
 *
 *  $RCSfile: doclay.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:15 $
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
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SFX_PROGRESS_HXX //autogen
#include <sfx2/progress.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <svx/opaqitem.hxx>
#endif
#ifndef _SVX_PRNTITEM_HXX //autogen
#include <svx/prntitem.hxx>
#endif
#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif
#ifndef _SVDOUNO_HXX //autogen
#include <svx/svdouno.hxx>
#endif
#ifndef _SVX_FMPAGE_HXX
#include <svx/fmpage.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FRMCNCT_HXX //autogen
#include <fmtcnct.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _DCONTACT_HXX //autogen
#include <dcontact.hxx>
#endif
#ifndef _TXTFLCNT_HXX //autogen
#include <txtflcnt.hxx>
#endif
#ifndef _DOCFLD_HXX
#include <docfld.hxx>   // fuer Expression-Felder
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _NDNOTXT_HXX
#include <ndnotxt.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _FESH_HXX
#include <fesh.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _FLYPOS_HXX
#include <flypos.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>       // InsertLabel
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>      // PoolVorlagen-Id's
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _FLDUPDE_HXX
#include <fldupde.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FTNIDX_HXX //autogen
#include <ftnidx.hxx>
#endif

#ifndef _COMCORE_HRC
#include <comcore.hrc>      // STR-ResId's
#endif

using namespace ::com::sun::star;
using namespace ::rtl;

#define DEF_FLY_WIDTH    2268   //Defaultbreite fuer FlyFrms    (2268 == 4cm)

/*************************************************************************
|*
|*  SwDoc::MakeLayoutFmt()
|*
|*  Beschreibung        Erzeugt ein neues Format das in seinen Einstellungen
|*      Defaultmaessig zu dem Request passt. Das Format wird in das
|*      entsprechende Formate-Array gestellt.
|*      Wenn bereits ein passendes Format existiert, so wird dies
|*      zurueckgeliefert.
|*  Ersterstellung      MA 22. Sep. 92
|*  Letzte Aenderung    JP 08.05.98
|*
|*************************************************************************/

SwFrmFmt *SwDoc::MakeLayoutFmt( RndStdIds eRequest, SwFrmFmt* pFrmFmt,
                                const SfxItemSet* pSet )
{
    SwFrmFmt *pFmt = 0;
    const sal_Bool bMod = IsModified();
    sal_Bool bHeader = sal_False;

    switch ( eRequest )
    {
    case RND_STD_HEADER:
    case RND_STD_HEADERL:
    case RND_STD_HEADERR:
        {
            bHeader = sal_True;
            // kein break, es geht unten weiter
        }
    case RND_STD_FOOTER:
    case RND_STD_FOOTERL:
    case RND_STD_FOOTERR:
        {

//JP erstmal ein Hack, solange keine Flys/Headers/Footers Undofaehig sind
if( DoesUndo() )
    DelAllUndoObj();

            pFmt = new SwFrmFmt( GetAttrPool(),
                                (bHeader ? "Header" : "Footer"),
                                GetDfltFrmFmt() );

            SwNodeIndex aTmpIdx( GetNodes().GetEndOfAutotext() );
            SwStartNode* pSttNd = GetNodes().MakeTextSection( aTmpIdx,
                                bHeader ? SwHeaderStartNode : SwFooterStartNode,
                                GetTxtCollFromPool(
                                bHeader
                                    ? ( eRequest == RND_STD_HEADERL
                                        ? RES_POOLCOLL_HEADERL
                                        : eRequest == RND_STD_HEADERR
                                            ? RES_POOLCOLL_HEADERR
                                            : RES_POOLCOLL_HEADER )
                                    : ( eRequest == RND_STD_FOOTERL
                                        ? RES_POOLCOLL_FOOTERL
                                        : eRequest == RND_STD_FOOTERR
                                            ? RES_POOLCOLL_FOOTERR
                                            : RES_POOLCOLL_FOOTER )
                                    ) );
            pFmt->SetAttr( SwFmtCntnt( pSttNd ));

            if( pSet )      // noch ein paar Attribute setzen ?
                pFmt->SetAttr( *pSet );

// JP: warum zuruecksetzen ???  Doc. ist doch veraendert ???
// bei den Fly auf jedenfall verkehrt !!
            if ( !bMod )
                ResetModified();
        }
        break;

    case RND_DRAW_OBJECT:
        {
            pFmt = MakeDrawFrmFmt( aEmptyStr, GetDfltFrmFmt() );
            if( pSet )      // noch ein paar Attribute setzen ?
                pFmt->SetAttr( *pSet );

            if( DoesUndo() )
            {
                ClearRedo();
                AppendUndo( new SwUndoInsLayFmt( pFmt ));
            }
        }
        break;

#ifndef PRODUCT
    case FLY_PAGE:
    case FLY_AUTO_CNTNT:
    case FLY_AT_FLY:
    case FLY_AT_CNTNT:
    case FLY_IN_CNTNT:
        ASSERT( !this,
                "neue Schnittstelle benutzen: SwDoc::MakeFlySection!" );
        break;
#endif

    default:
        ASSERT( !this,
                "Layoutformat mit ungueltigem Request angefordert." );

    }
    return pFmt;
}
/*************************************************************************
|*
|*  SwDoc::DelLayoutFmt()
|*
|*  Beschreibung        Loescht das angegebene Format, der Inhalt wird mit
|*      geloescht.
|*  Ersterstellung      MA 23. Sep. 92
|*  Letzte Aenderung    MA 05. Feb. 93
|*
|*************************************************************************/

void SwDoc::DelLayoutFmt( SwFrmFmt *pFmt )
{
    //Verkettung von Rahmen muss ggf. zusammengefuehrt werden.
    //Bevor die Frames vernichtet werden, damit die Inhalte der Rahmen
    //ggf. entsprechend gerichtet werden.
    const SwFmtChain &rChain = pFmt->GetChain();
    if ( rChain.GetPrev() )
    {
        SwFmtChain aChain( rChain.GetPrev()->GetChain() );
        aChain.SetNext( rChain.GetNext() );
        SetAttr( aChain, *rChain.GetPrev() );
    }
    if ( rChain.GetNext() )
    {
        SwFmtChain aChain( rChain.GetNext()->GetChain() );
        aChain.SetPrev( rChain.GetPrev() );
        SetAttr( aChain, *rChain.GetNext() );
    }

    const SwNodeIndex* pCntIdx = pFmt->GetCntnt().GetCntntIdx();
    if( pCntIdx )
    {
        //Verbindung abbauen, falls es sich um ein OLE-Objekt handelt.
        SwOLENode* pOLENd = GetNodes()[ pCntIdx->GetIndex()+1 ]->GetOLENode();
        if( pOLENd && pOLENd->GetOLEObj().IsOleRef() )
            pOLENd->GetOLEObj().GetOleRef()->DoClose();
    }

    //Frms vernichten.
    pFmt->DelFrms();

    // erstmal sind nur Fly's Undofaehig
    const sal_uInt16 nWh = pFmt->Which();
    if( DoesUndo() && (RES_FLYFRMFMT == nWh || RES_DRAWFRMFMT == nWh) )
    {
        // erstmal werden alle Undo - Objecte geloescht.
        ClearRedo();
        AppendUndo( new SwUndoDelLayFmt( pFmt ));
    }
    else
    {
        //Inhalt Loeschen.
        if( pCntIdx )
        {

//JP erstmal ein Hack, solange keine Headers/Footers Undofaehig sind
if( DoesUndo() )
    DelAllUndoObj();

            SwNode *pNode = &pCntIdx->GetNode();
            ((SwFmtCntnt&)pFmt->GetAttr( RES_CNTNT )).SetNewCntntIdx( 0 );
            DeleteSection( pNode );
        }

        // ggfs. bei Zeichengebundenen Flys das Zeichen loeschen
        const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
        if( FLY_IN_CNTNT == rAnchor.GetAnchorId() && rAnchor.GetCntntAnchor())
        {
            const SwPosition* pPos = rAnchor.GetCntntAnchor();
            SwTxtNode *pTxtNd = pPos->nNode.GetNode().GetTxtNode();
            SwTxtFlyCnt* pAttr;

            // Attribut steht noch im TextNode, loeschen
            if( pTxtNd && 0 != ( pAttr = ((SwTxtFlyCnt*)pTxtNd->GetTxtAttr(
                                            pPos->nContent.GetIndex() ))) &&
                pAttr->GetFlyCnt().GetFrmFmt() == pFmt )
            {
                // Pointer auf 0, nicht loeschen
                ((SwFmtFlyCnt&)pAttr->GetFlyCnt()).SetFlyFmt();
                SwIndex aIdx( pPos->nContent );
                pTxtNd->Erase( aIdx, 1 );
            }
        }

        DelFrmFmt( pFmt );
    }
    SetModified();
}

/*************************************************************************
|*
|*  SwDoc::CopyLayoutFmt()
|*
|*  Beschreibung        Kopiert das angegebene Format pSrc in pDest und
|*                      returnt pDest. Wenn es noch kein pDest gibt, wird
|*                      eins angelegt.
|*                      JP: steht das Source Format in einem anderen
|*                          Dokument, so kopiere auch dann noch richtig !!
|*                          Vom chaos::Anchor-Attribut wird die Position immer
|*                          auf 0 gesetzt !!!
|*
|*  Ersterstellung      BP 18.12.92
|*  Letzte Aenderung    MA 17. Jul. 96
|*
|*************************************************************************/

SwFrmFmt *SwDoc::CopyLayoutFmt( const SwFrmFmt& rSource,
                                const SwFmtAnchor& rNewAnchor,
                                sal_Bool bSetTxtFlyAtt, sal_Bool bMakeFrms )
{
    SwDoc* pSrcDoc = (SwDoc*)rSource.GetDoc();

    const sal_Bool bFly = RES_FLYFRMFMT == rSource.Which();

    //DrawObjecte duerfen niemals in Kopf-/Fusszeilen landen.
    // JP 31.05.96: die Zeichengebundenen muessen spaeter abgeprueft werden.
    //              Und zwar wenn sie in den ZielNode eingefuegt werden;
    //              erst dann ist der richtige Anker gueltig !!!!
    if( !bFly &&
        ( FLY_AT_CNTNT == rNewAnchor.GetAnchorId() ||
          FLY_AT_FLY == rNewAnchor.GetAnchorId() ||
          FLY_AUTO_CNTNT == rNewAnchor.GetAnchorId() ) &&
        rNewAnchor.GetCntntAnchor() &&
        IsInHeaderFooter( rNewAnchor.GetCntntAnchor()->nNode ) )
        return 0;

    SwFrmFmt* pDest = GetDfltFrmFmt();
    if( rSource.GetRegisteredIn() != pSrcDoc->GetDfltFrmFmt() )
        pDest = CopyFrmFmt( *(SwFrmFmt*)rSource.GetRegisteredIn() );
    if( bFly )
        pDest = MakeFlyFrmFmt( rSource.GetName(), pDest );
    else
        pDest = MakeDrawFrmFmt( aEmptyStr, pDest );

    // alle anderen/neue Attribute kopieren.
    pDest->CopyAttrs( rSource );

    //Chains werden nicht kopiert.
    pDest->ResetAttr( RES_CHAIN );

    if( bFly )
    {
        //Der Inhalt wird dupliziert.
        const SwNode& rCSttNd = rSource.GetCntnt().GetCntntIdx()->GetNode();
        SwNodeRange aRg( rCSttNd, 1, *rCSttNd.EndOfSectionNode() );

        SwNodeIndex aIdx( GetNodes().GetEndOfAutotext() );
        SwStartNode* pSttNd = GetNodes().MakeEmptySection( aIdx, SwFlyStartNode );

        // erst den chaos::Anchor/CntntIndex setzen, innerhalb des Kopierens
        // auf die Werte zugegriffen werden kann (DrawFmt in Kopf-/Fusszeilen)
        aIdx = *pSttNd;
        SwFmtCntnt aAttr( rSource.GetCntnt() );
        aAttr.SetNewCntntIdx( &aIdx );
        pDest->SetAttr( aAttr );
        pDest->SetAttr( rNewAnchor );

        if( !bCopyIsMove || this != pSrcDoc )
        {
            if( bInReading )
                pDest->SetName( aEmptyStr );
            else
            {
                // Teste erstmal ob der Name schon vergeben ist.
                // Wenn ja -> neuen generieren
                sal_Int8 nNdTyp = aRg.aStart.GetNode().GetNodeType();

                String sOld( pDest->GetName() );
                pDest->SetName( aEmptyStr );
                if( FindFlyByName( sOld, nNdTyp ) )     // einen gefunden
                    switch( nNdTyp )
                    {
                    case ND_GRFNODE:    sOld = GetUniqueGrfName();      break;
                    case ND_OLENODE:    sOld = GetUniqueOLEName();      break;
                    default:            sOld = GetUniqueFrameName();    break;
                    }

                pDest->SetName( sOld );
            }
        }

        if( DoesUndo() )
        {
            ClearRedo();
            AppendUndo( new SwUndoInsLayFmt( pDest ));
        }

        // sorge dafuer das auch Fly's in Fly's kopiert werden
        aIdx = *pSttNd->EndOfSectionNode();
        pSrcDoc->CopyWithFlyInFly( aRg, aIdx, sal_False, sal_True, sal_True );
    }
    else
    {
        ASSERT( RES_DRAWFRMFMT == rSource.Which(), "Weder Fly noch Draw." );
        SwDrawContact *pContact = (SwDrawContact *)rSource.FindContactObj();

        pContact = new SwDrawContact( (SwDrawFrmFmt*)pDest,
                                CloneSdrObj( *pContact->GetMaster(),
                                        bCopyIsMove && this == pSrcDoc ) );

        if( pDest->GetAnchor() == rNewAnchor )
            pContact->ConnectToLayout( &rNewAnchor );
        else
            pDest->SetAttr( rNewAnchor );

        if( DoesUndo() )
        {
            ClearRedo();
            AppendUndo( new SwUndoInsLayFmt( pDest ));
        }
    }

    if( bSetTxtFlyAtt && FLY_IN_CNTNT == rNewAnchor.GetAnchorId() )
    {
        SwPosition* pPos = (SwPosition*)rNewAnchor.GetCntntAnchor();
        pPos->nNode.GetNode().GetTxtNode()->Insert(SwFmtFlyCnt( pDest ),
                                            pPos->nContent.GetIndex(), 0 );
    }

    if( bMakeFrms )
        pDest->MakeFrms();

    return pDest;
}

SdrObject* SwDoc::CloneSdrObj( const SdrObject& rObj, sal_Bool bMoveWithinDoc,
                                sal_Bool bInsInPage )
{
    SdrPage *pPg = MakeDrawModel()->GetPage( 0 );
    if( !pPg )
    {
        pPg = GetDrawModel()->AllocPage( sal_False );
        GetDrawModel()->InsertPage( pPg );
    }

    SdrObject *pObj = rObj.Clone();
    if( bMoveWithinDoc && FmFormInventor == pObj->GetObjInventor() )
    {
        // bei Controls muss der Name erhalten bleiben
        uno::Reference< awt::XControlModel >  xModel = ((SdrUnoObj*)pObj)->GetUnoControlModel();
        sal_Bool bModel = xModel.is();
        uno::Any aVal;
        uno::Reference< beans::XPropertySet >  xSet(xModel, uno::UNO_QUERY);
        OUString sName( rtl::OUString::createFromAscii("Name") );
        if( xSet.is() )
            aVal = xSet->getPropertyValue( sName );
        if( bInsInPage )
            pPg->InsertObject( pObj );
        if( xSet.is() )
            xSet->setPropertyValue( sName, aVal );
    }
    else if( bInsInPage )
        pPg->InsertObject( pObj );

    pObj->SetLayer( rObj.GetLayer() );
    return pObj;
}

SwFlyFrmFmt* SwDoc::_MakeFlySection( const SwPosition& rAnchPos,
                                    const SwCntntNode& rNode,
                                    RndStdIds eRequestId,
                                    const SfxItemSet* pFlySet,
                                    SwFrmFmt* pFrmFmt )
{
    if( !pFrmFmt )
        pFrmFmt = GetFrmFmtFromPool( RES_POOLFRM_FRAME );

    String sName;
    if( !bInReading )
        switch( rNode.GetNodeType() )
        {
        case ND_GRFNODE:        sName = GetUniqueGrfName();     break;
        case ND_OLENODE:        sName = GetUniqueOLEName();     break;
        default:                sName = GetUniqueFrameName();       break;
        }
    SwFlyFrmFmt* pFmt = MakeFlyFrmFmt( sName, pFrmFmt );

    //Inhalt erzeugen und mit dem Format verbinden.
    //CntntNode erzeugen und in die Autotextsection stellen
    SwNodeRange aRange( GetNodes().GetEndOfAutotext(), -1,
                        GetNodes().GetEndOfAutotext() );
    GetNodes().SectionDown( &aRange, SwFlyStartNode );

    pFmt->SetAttr( SwFmtCntnt( rNode.StartOfSectionNode() ));


    const SwFmtAnchor* pAnchor = 0;
    if( pFlySet )
    {
        pFlySet->GetItemState( RES_ANCHOR, sal_False,
                                (const SfxPoolItem**)&pAnchor );
        if( SFX_ITEM_SET == pFlySet->GetItemState( RES_CNTNT, sal_False ))
        {
            SfxItemSet aTmpSet( *pFlySet );
            aTmpSet.ClearItem( RES_CNTNT );
            pFmt->SetAttr( aTmpSet );
        }
        else
            pFmt->SetAttr( *pFlySet );
    }

    // Anker noch nicht gesetzt ?
    RndStdIds eAnchorId = pAnchor ? pAnchor->GetAnchorId()
                                  : pFmt->GetAnchor().GetAnchorId();
    if( !pAnchor ||
        (FLY_PAGE != pAnchor->GetAnchorId() &&
          //Nur Page und nicht:
//        FLY_AT_CNTNT == pAnchor->GetAnchorId() ||
//        FLY_IN_CNTNT == pAnchor->GetAnchorId() ||
//        FLY_AT_FLY == pAnchor->GetAnchorId() ||
//        FLY_AUTO_CNTNT == pAnchor->GetAnchorId() ) &&
        !pAnchor->GetCntntAnchor() ))
    {
        // dann setze ihn, wird im Undo gebraucht
        SwFmtAnchor aAnch( pFmt->GetAnchor() );
        if( pAnchor && FLY_AT_FLY == pAnchor->GetAnchorId() )
        {
            SwPosition aPos( *rAnchPos.nNode.GetNode().FindFlyStartNode() );
            aAnch.SetAnchor( &aPos );
            eAnchorId = FLY_AT_FLY;
        }
        else
        {
            if( eRequestId != aAnch.GetAnchorId() &&
                SFX_ITEM_SET != pFmt->GetItemState( RES_ANCHOR, sal_True ) )
                aAnch.SetType( eRequestId );

            eAnchorId = aAnch.GetAnchorId();
            if ( FLY_PAGE != eAnchorId )
            //Nur Page und nicht:
//          if( FLY_AT_CNTNT == eAnchorId || FLY_IN_CNTNT == eAnchorId ||
//              FLY_AT_FLY == eAnchorId || FLY_AUTO_CNTNT == eAnchorId )
                aAnch.SetAnchor( &rAnchPos );
        }
        pFmt->SetAttr( aAnch );
    }
    else
        eAnchorId = pFmt->GetAnchor().GetAnchorId();

    if( FLY_IN_CNTNT == eAnchorId )
    {
        xub_StrLen nStt = rAnchPos.nContent.GetIndex();
        rAnchPos.nNode.GetNode().GetTxtNode()->Insert(
                                        SwFmtFlyCnt( pFmt ), nStt, nStt );
    }

    if( SFX_ITEM_SET != pFmt->GetAttrSet().GetItemState( RES_FRM_SIZE ))
    {
        SwFmtFrmSize aFmtSize( ATT_VAR_SIZE, 0, DEF_FLY_WIDTH );
        const SwNoTxtNode* pNoTxtNode = rNode.GetNoTxtNode();
        if( pNoTxtNode )
        {
            //Groesse einstellen.
             Size aSize( pNoTxtNode->GetTwipSize() );
            if( MINFLY > aSize.Width() )
                aSize.Width() = DEF_FLY_WIDTH;
            aFmtSize.SetWidth( aSize.Width() );
            if( aSize.Height() )
            {
                aFmtSize.SetHeight( aSize.Height() );
                aFmtSize.SetSizeType( ATT_FIX_SIZE );
            }
        }
        pFmt->SetAttr( aFmtSize );
    }

    // Frames anlegen
    if( GetRootFrm() )
        pFmt->MakeFrms();           // ???

    if( DoesUndo() )
    {
        ClearRedo();
        AppendUndo( new SwUndoInsLayFmt( pFmt ));
    }

    SetModified();
    return pFmt;
}

SwFlyFrmFmt* SwDoc::MakeFlySection( RndStdIds eAnchorType,
                                    const SwPosition* pAnchorPos,
                                    const SfxItemSet* pFlySet,
                                    SwFrmFmt* pFrmFmt )
{
    SwFlyFrmFmt* pFmt = 0;
    sal_Bool bCallMake = sal_True;
    if( !pAnchorPos && FLY_PAGE != eAnchorType )
    {
        const SwFmtAnchor* pAnch;
        if( (pFlySet && SFX_ITEM_SET == pFlySet->GetItemState(
                RES_ANCHOR, sal_False, (const SfxPoolItem**)&pAnch )) ||
            ( pFrmFmt && SFX_ITEM_SET == pFrmFmt->GetItemState(
                RES_ANCHOR, sal_True, (const SfxPoolItem**)&pAnch )) )
        {
            if( FLY_PAGE != pAnch->GetAnchorId() &&
                0 == ( pAnchorPos = pAnch->GetCntntAnchor() ) )
                bCallMake = sal_False;
        }
    }

    if( bCallMake )
    {
        if( !pFrmFmt )
            pFrmFmt = GetFrmFmtFromPool( RES_POOLFRM_FRAME );

        sal_uInt16 nCollId = IsHTMLMode() ? RES_POOLCOLL_TEXT : RES_POOLCOLL_FRAME;
        pFmt = _MakeFlySection( *pAnchorPos, *GetNodes().MakeTxtNode(
                                SwNodeIndex( GetNodes().GetEndOfAutotext() ),
                                GetTxtCollFromPool( nCollId ) ),
                                eAnchorType, pFlySet, pFrmFmt );
    }
    return pFmt;
}

SwFlyFrmFmt* SwDoc::MakeFlyAndMove( const SwPaM& rPam, const SfxItemSet& rSet,
                                    const SwSelBoxes* pSelBoxes,
                                    SwFrmFmt *pParent )
{
    SwFmtAnchor& rAnch = (SwFmtAnchor&)rSet.Get( RES_ANCHOR );

    StartUndo( UNDO_INSLAYFMT );

    SwFlyFrmFmt* pFmt = MakeFlySection( rAnch.GetAnchorId(), rPam.GetPoint(),
                                        &rSet, pParent );

    // Wenn Inhalt selektiert ist, so wird dieser jetzt zum Inhalt des
    // neuen Rahmen. Sprich er wird in die entspr. Sektion des NodesArr
    //gemoved.

    if( pFmt )
    {
        do {        // middle check loop
            const SwFmtCntnt &rCntnt = pFmt->GetCntnt();
            ASSERT( rCntnt.GetCntntIdx(), "Kein Inhalt vorbereitet." );
            SwNodeIndex aIndex( *(rCntnt.GetCntntIdx()), 1 );
            SwCntntNode *pNode = aIndex.GetNode().GetCntntNode();

            // ACHTUNG: nicht einen Index auf dem Stack erzeugen, sonst
            //          kann der CntntnNode am Ende nicht geloscht werden !!
            SwPosition aPos( aIndex );
            aPos.nContent.Assign( pNode, 0 );

            if( pSelBoxes && pSelBoxes->Count() )
            {
                // Tabellenselection
                // kopiere Teile aus einer Tabelle: lege eine Tabelle mit der
                // Breite der Originalen an und move (kopiere/loesche) die
                // selektierten Boxen. Die Groessen werden prozentual
                // korrigiert.

                SwTableNode* pTblNd = (SwTableNode*)(*pSelBoxes)[0]->
                                                GetSttNd()->FindTableNode();
                if( !pTblNd )
                    break;

                SwTable& rTbl = pTblNd->GetTable();

                // ist die gesamte Tabelle selektiert ?
                if( pSelBoxes->Count() == rTbl.GetTabSortBoxes().Count() )
                {
                    // verschiebe die gesamte Tabelle
                    SwNodeRange aRg( *pTblNd, 0, *pTblNd->EndOfSectionNode(), 1 );

                    // wird die gesamte Tabelle verschoben und steht diese
                    // in einem FlyFrame, dann erzeuge dahinter einen neuen
                    // TextNode. Dadurch bleibt dieser Fly erhalten !
                    if( aRg.aEnd.GetNode().IsEndNode() )
                        GetNodes().MakeTxtNode( aRg.aStart,
                                    (SwTxtFmtColl*)GetDfltTxtFmtColl() );

                    Move( aRg, aPos.nNode );
                }
                else
                {
                    rTbl.MakeCopy( this, aPos, *pSelBoxes );
                    rTbl.DeleteSel( this, *pSelBoxes );
                }

                // wenn Tabelle im Rahmen, dann ohne nachfolgenden TextNode
                aIndex = rCntnt.GetCntntIdx()->GetNode().EndOfSectionIndex() - 1;
                ASSERT( aIndex.GetNode().GetTxtNode(),
                        "hier sollte ein TextNode stehen" );
                aPos.nContent.Assign( 0, 0 );       // Index abmelden !!
                GetNodes().Delete( aIndex, 1 );

//JP erstmal ein Hack, solange keine Flys/Headers/Footers Undofaehig sind
if( DoesUndo() )    // werden erstmal alle Undo - Objecte geloescht.
    DelAllUndoObj();

            }
            else
            {
                // alle Pams verschieben
                SwPaM* pTmp = (SwPaM*)&rPam;
                do {
                    if( pTmp->HasMark() &&
                        *pTmp->GetPoint() != *pTmp->GetMark() )
                        MoveAndJoin( *pTmp, aPos );
                } while( &rPam != ( pTmp = (SwPaM*)pTmp->GetNext() ) );
            }
        } while( sal_False );
    }

    SetModified();

    EndUndo( UNDO_INSLAYFMT );

    return pFmt;
}


    //Einfuegen eines DrawObjectes. Das Object muss bereits im DrawModel
    // angemeldet sein.
SwDrawFrmFmt* SwDoc::Insert( const SwPaM &rRg,
                             SdrObject& rDrawObj,
                             const SfxItemSet* pFlyAttrSet,
                             SwFrmFmt* pDefFmt )
{
    SwDrawFrmFmt *pFmt = MakeDrawFrmFmt( aEmptyStr,
                                        pDefFmt ? pDefFmt : GetDfltFrmFmt() );

    const SwFmtAnchor* pAnchor = 0;
    if( pFlyAttrSet )
    {
        pFlyAttrSet->GetItemState( RES_ANCHOR, sal_False,
                                    (const SfxPoolItem**)&pAnchor );
        pFmt->SetAttr( *pFlyAttrSet );
    }

    RndStdIds eAnchorId = pAnchor ? pAnchor->GetAnchorId()
                                  : pFmt->GetAnchor().GetAnchorId();

    // Anker noch nicht gesetzt ?
    // DrawObjecte duerfen niemals in Kopf-/Fusszeilen landen.
    sal_Bool bIsAtCntnt = FLY_PAGE != eAnchorId;
//                    FLY_AT_CNTNT == eAnchorId || FLY_IN_CNTNT == eAnchorId ||
//                    FLY_AT_FLY == eAnchorId || FLY_AUTO_CNTNT == eAnchorId;

    const SwNodeIndex* pChkIdx = 0;
    if( !pAnchor )
        pChkIdx = &rRg.GetPoint()->nNode;
    else if( bIsAtCntnt )
        pChkIdx = pAnchor->GetCntntAnchor()
                    ? &pAnchor->GetCntntAnchor()->nNode
                    : &rRg.GetPoint()->nNode;

    if( pChkIdx && IsInHeaderFooter( *pChkIdx ) )
        pFmt->SetAttr( SwFmtAnchor( eAnchorId = FLY_PAGE ) );
    else if( !pAnchor || (bIsAtCntnt && !pAnchor->GetCntntAnchor() ))
    {
        // dann setze ihn, wird im Undo gebraucht
        SwFmtAnchor aAnch( pAnchor ? *pAnchor : pFmt->GetAnchor() );
        eAnchorId = aAnch.GetAnchorId();
        if( FLY_AT_FLY == eAnchorId )
        {
            SwPosition aPos( *rRg.GetNode()->FindFlyStartNode() );
            aAnch.SetAnchor( &aPos );
        }
        else
        {
            aAnch.SetAnchor( rRg.GetPoint() );
            if( FLY_PAGE == eAnchorId )
            {
                eAnchorId = rDrawObj.ISA( SdrUnoObj )
                                    ? FLY_IN_CNTNT : FLY_AT_CNTNT;
                aAnch.SetType( eAnchorId );
            }
        }
        pFmt->SetAttr( aAnch );
    }

    // bei als Zeichen gebundenen Draws das Attribut im Absatz setzen
    if( FLY_IN_CNTNT == eAnchorId )
    {
        xub_StrLen nStt = rRg.GetPoint()->nContent.GetIndex();
        rRg.GetPoint()->nNode.GetNode().GetTxtNode()->Insert(
                                        SwFmtFlyCnt( pFmt ), nStt, nStt );
    }

    new SwDrawContact( pFmt, &rDrawObj );

    // ggfs. Frames anlegen
    if( GetRootFrm() )
        pFmt->MakeFrms();

    if( DoesUndo() )
    {
        ClearRedo();
        AppendUndo( new SwUndoInsLayFmt( pFmt ));
    }

    SetModified();
    return pFmt;
}

/*************************************************************************
|*
|*  SwDoc::GetAllFlyFmts
|*
|*  Ersterstellung      MA 14. Jul. 93
|*  Letzte Aenderung    MD 23. Feb. 95
|*
|*************************************************************************/

/*sal_Bool TstFlyRange( const SwPaM* pPam, sal_uInt32 nFlyPos )
{
    sal_Bool bOk = sal_False;
    const SwPaM* pTmp = pPam;
    do {
        bOk = pTmp->Start()->nNode.GetIndex() < nFlyPos &&
                pTmp->End()->nNode.GetIndex() > nFlyPos;
    } while( !bOk && pPam != ( pTmp = (const SwPaM*)pTmp->GetNext() ));
    return bOk;
}
*/
/* -----------------------------04.04.00 10:55--------------------------------
    paragraph frames - o.k. if the PaM includes the paragraph from the beginning
                       to the beginning of the next paragraph at least
    frames at character - o.k. if the pam start at least at the same position
                         as the frame
 ---------------------------------------------------------------------------*/
sal_Bool TstFlyRange( const SwPaM* pPam, const SwPosition* pFlyPos,
                        RndStdIds nAnchorId )
{
    sal_Bool bOk = FALSE;
    const SwPaM* pTmp = pPam;
    do {
        const sal_uInt32 nFlyIndex = pFlyPos->nNode.GetIndex();
        const SwPosition* pPaMStart = pTmp->Start();
        const SwPosition* pPaMEnd = pTmp->End();
        const sal_uInt32 nPamStartIndex = pPaMStart->nNode.GetIndex();
        const sal_uInt32 nPamEndIndex = pPaMEnd->nNode.GetIndex();
        if(FLY_AT_CNTNT == nAnchorId)
            bOk = (nPamStartIndex < nFlyIndex && nPamEndIndex > nFlyIndex) ||
               (((nPamStartIndex == nFlyIndex) && (pPaMStart->nContent.GetIndex() == 0)) &&
               (nPamEndIndex > nFlyIndex));
        else
        {
            xub_StrLen nFlyContentIndex = pFlyPos->nContent.GetIndex();
            xub_StrLen nPamEndContentIndex = pPaMEnd->nContent.GetIndex();
            bOk = (nPamStartIndex < nFlyIndex &&
                (( nPamEndIndex > nFlyIndex )||
                 ((nPamEndIndex == nFlyIndex) &&
                  (nPamEndContentIndex = nFlyContentIndex))) )
                ||
                       (((nPamStartIndex == nFlyIndex) &&
                      (pPaMStart->nContent.GetIndex() <= nFlyContentIndex)) &&
                        (nPamEndIndex > nFlyIndex) ||
                     (nPamEndContentIndex > nFlyContentIndex ));
        }

    } while( !bOk && pPam != ( pTmp = (const SwPaM*)pTmp->GetNext() ));
    return bOk;
}


void SwDoc::GetAllFlyFmts( SwPosFlyFrms& rPosFlyFmts,
                           const SwPaM* pCmpRange, sal_Bool bDrawAlso ) const
{
    SwPosFlyFrm *pFPos = 0;
    const SwPosition* pAPos;
    SwFrmFmt *pFly;

    // erstmal alle Absatzgebundenen einsammeln
    for( sal_uInt16 n = 0; n < GetSpzFrmFmts()->Count(); ++n )
    {
        pFly = (*GetSpzFrmFmts())[ n ];
        sal_Bool bDrawFmt = bDrawAlso ? RES_DRAWFRMFMT == pFly->Which() : sal_False;
        sal_Bool bFlyFmt = RES_FLYFRMFMT == pFly->Which();
        if( bFlyFmt || bDrawFmt )
        {
            const SwFmtAnchor& rAnchor = pFly->GetAnchor();
            if( ( FLY_AT_CNTNT == rAnchor.GetAnchorId() ||
                FLY_AT_FLY == rAnchor.GetAnchorId() ||
                FLY_AUTO_CNTNT == rAnchor.GetAnchorId() ) &&
                0 != ( pAPos = rAnchor.GetCntntAnchor()) )
            {
                if( pCmpRange &&
                    !TstFlyRange( pCmpRange, pAPos, rAnchor.GetAnchorId() ))
                        continue;       // kein gueltiger FlyFrame
                pFPos = new SwPosFlyFrm( pAPos->nNode, pFly, rPosFlyFmts.Count() );
                rPosFlyFmts.Insert( pFPos );
            }
        }
    }

    // kein Layout oder nur ein Teil, dann wars das
    // Seitenbezogen Flys nur, wenn vollstaendig "gewuenscht" wird !
    if( !GetRootFrm() || pCmpRange )
        return;

    pFPos = 0;
    SwPageFrm *pPage = (SwPageFrm*)GetRootFrm()->GetLower();
    while( pPage )
    {
        if( pPage->GetSortedObjs() )
        {
            SwSortDrawObjs &rObjs = *pPage->GetSortedObjs();
            for( sal_uInt16 i = 0; i < rObjs.Count(); ++i)
            {
                SdrObject *pO = rObjs[i];
                SwVirtFlyDrawObj *pObj = pO->IsWriterFlyFrame() ?
                                                    (SwVirtFlyDrawObj*)pO : 0;
                if( pObj )
                    pFly = pObj->GetFlyFrm()->GetFmt();
                else if ( bDrawAlso )
                    pFly = ::FindFrmFmt( rObjs[i] );
                else
                    continue;

                const SwFmtAnchor& rAnchor = pFly->GetAnchor();
                if( FLY_AT_CNTNT != rAnchor.GetAnchorId() &&
                    FLY_AT_FLY != rAnchor.GetAnchorId() &&
                    FLY_AUTO_CNTNT != rAnchor.GetAnchorId() )
                {
                    const SwCntntFrm * pCntntFrm = pPage->FindFirstBodyCntnt();
                    if ( !pCntntFrm )
                    {
                        //Oops! Eine leere Seite. Damit der Rahmen nicht ganz
                        //verlorengeht (RTF) suchen wir schnell den letzen
                        //Cntnt der vor der Seite steht.
                        SwPageFrm *pPrv = (SwPageFrm*)pPage->GetPrev();
                        while ( !pCntntFrm && pPrv )
                        {
                            pCntntFrm = pPrv->FindFirstBodyCntnt();
                            pPrv = (SwPageFrm*)pPrv->GetPrev();
                        }
                    }
                    if ( pCntntFrm )
                    {
                        SwNodeIndex aIdx( *pCntntFrm->GetNode() );
                        pFPos = new SwPosFlyFrm( aIdx, pFly, rPosFlyFmts.Count() );
                    }
                }
                if ( pFPos )
                {
                    rPosFlyFmts.Insert( pFPos );
                    pFPos = 0;
                }
            }
        }
        pPage = (SwPageFrm*)pPage->GetNext();
    }
}

/*************************************************************************
|*
|*  SwDoc::InsertLabel()
|*
|*  Ersterstellung      MA 11. Feb. 94
|*  Letzte Aenderung    MA 12. Nov. 97
|*
|*************************************************************************/

void lcl_CpyAttr( SfxItemSet &rNewSet, const SfxItemSet &rOldSet, sal_uInt16 nWhich )
{
    const SfxPoolItem *pItem;
    if ( SFX_ITEM_SET == (rOldSet.GetItemState( nWhich, sal_False, &pItem)))
        rNewSet.Put( *pItem );
}


SwFlyFrmFmt* SwDoc::InsertLabel( const SwLabelType eType, const String &rTxt,
            const sal_Bool bBefore, const sal_uInt16 nId, const sal_uInt32 nNdIdx,
            const sal_Bool bCpyBrd )
{
    sal_Bool bWasUndo = DoesUndo();
    SwUndoInsertLabel* pUndo = 0;
    if( bWasUndo )
    {
        ClearRedo();
        pUndo = new SwUndoInsertLabel( eType, rTxt, bBefore, nId, bCpyBrd );
        DoUndo( sal_False );
    }

    sal_Bool bTable = sal_False;    //Um etwas Code zu sparen.

    //Erstmal das Feld bauen, weil ueber den Namen die TxtColl besorgt werden
    //muss
    ASSERT( nId < GetFldTypes()->Count(), "FldType ueberindiziert." );
    SwFieldType *pType = (*GetFldTypes())[nId];
    ASSERT( pType->Which() == RES_SETEXPFLD, "Falsche Id fuer Label" );
    SwSetExpField aFld( (SwSetExpFieldType*)pType, aEmptyStr, SVX_NUM_ARABIC);

    SwTxtFmtColl *pColl = 0;
    for( sal_uInt16 i = pTxtFmtCollTbl->Count(); i; )
    {
        if( (*pTxtFmtCollTbl)[ --i ]->GetName() == pType->GetName() )
        {
            pColl = (*pTxtFmtCollTbl)[i];
            break;
        }
    }
    if( !pColl )
    {
        ASSERT( !this, "TxtCollection fuer Label nicht gefunden." );
        pColl = GetTxtCollFromPool( RES_POOLCOLL_TEXT );
    }

    SwTxtNode *pNew = 0;
    SwFlyFrmFmt* pNewFmt = 0;

    switch ( eType )
    {
        case LTYPE_TABLE:
            bTable = sal_True;
            /* Kein Break hier */
        case LTYPE_FLY:
            //Am Anfang/Ende der Fly-Section den entsprechenden Node mit Feld
            //einfuegen (Frame wird automatisch erzeugt).
            {
                SwStartNode *pSttNd = GetNodes()[nNdIdx]->GetStartNode();
                ASSERT( pSttNd, "Kein StartNode in InsertLabel." );
                sal_uInt32 nNode;
                if( bBefore )
                {
                    nNode = pSttNd->GetIndex();
                    if( !bTable )
                        ++nNode;
                }
                else
                {
                    nNode = pSttNd->EndOfSectionIndex();
                    if( bTable )
                        ++nNode;
                }

                if( pUndo )
                    pUndo->SetNodePos( nNode );

                //Node fuer Beschriftungsabsatz erzeugen.
                SwNodeIndex aIdx( GetNodes(), nNode );
                pNew = GetNodes().MakeTxtNode( aIdx, pColl );
            }
            break;

        case LTYPE_OBJECT:
            {
                //Rahmen zerstoeren, neuen Rahmen einfuegen, entsprechenden
                // Node mit Feld in den neuen Rahmen, den alten Rahmen mit
                // dem Object (Grafik/Ole) absatzgebunden in den neuen Rahmen,
                // Frames erzeugen.

                //Erstmal das Format zum Fly besorgen und das Layout entkoppeln.
                SwFrmFmt *pOldFmt = GetNodes()[nNdIdx]->GetFlyFmt();
                ASSERT( pOldFmt, "Format des Fly nicht gefunden." );
                pOldFmt->DelFrms();

                SfxItemSet* pNewSet = pOldFmt->GetAttrSet().Clone( sal_False );

                pNewFmt = MakeFlyFrmFmt( GetUniqueFrameName(),
                                    GetFrmFmtFromPool( RES_POOLFRM_FRAME ));

                //Diejenigen Attribute uebertragen die auch gesetzt sind,
                //andere sollen weiterhin aus den Vorlagen gueltig werden.
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_PRINT );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_OPAQUE );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_PROTECT );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_SURROUND );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_VERT_ORIENT );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_HORI_ORIENT );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_LR_SPACE );
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_UL_SPACE );
                if( bCpyBrd )
                {
                    // JP 07.07.99: Bug 67029 - if at Grafik no BoxItem but
                    //              in the new Format is any, then set the
                    //              default item in the new Set. Because
                    //              the Size of the Grafik have never been
                    //              changed!
                    const SfxPoolItem *pItem;
                    if( SFX_ITEM_SET == pOldFmt->GetAttrSet().
                            GetItemState( RES_BOX, sal_True, &pItem ))
                        pNewSet->Put( *pItem );
                    else if( SFX_ITEM_SET == pNewFmt->GetAttrSet().
                            GetItemState( RES_BOX, sal_True ))
                        pNewSet->Put( *GetDfltAttr( RES_BOX ) );

                    if( SFX_ITEM_SET == pOldFmt->GetAttrSet().
                            GetItemState( RES_SHADOW, sal_True, &pItem ))
                        pNewSet->Put( *pItem );
                    else if( SFX_ITEM_SET == pNewFmt->GetAttrSet().
                            GetItemState( RES_SHADOW, sal_True ))
                        pNewSet->Put( *GetDfltAttr( RES_SHADOW ) );
                }
                else
                {
                    //Die Attribute hart setzen, weil sie sonst aus der
                    // Vorlage kommen koenten und dann passt die
                    // Grossenberechnung nicht mehr.
                    pNewSet->Put( SvxBoxItem() );
                    pNewSet->Put( SvxShadowItem() );
                }

                //Anker immer uebertragen, ist sowieso ein hartes Attribut.
                pNewSet->Put( pOldFmt->GetAnchor() );

                //In der Hoehe soll der neue Varabel sein!
                SwFmtFrmSize aFrmSize( pOldFmt->GetFrmSize() );
                aFrmSize.SetSizeType( ATT_MIN_SIZE );
                pNewSet->Put( aFrmSize );

                SwStartNode* pSttNd = GetNodes().MakeTextSection(
                            SwNodeIndex( GetNodes().GetEndOfAutotext() ),
                            SwFlyStartNode, pColl );
                pNewSet->Put( SwFmtCntnt( pSttNd ));

                pNewFmt->SetAttr( *pNewSet );

                //Bei InCntnt's wird es spannend: Das TxtAttribut muss
                //vernichtet werden. Leider reisst dies neben den Frms auch
                //noch das Format mit in sein Grab. Um dass zu unterbinden
                //loesen wir vorher die Verbindung zwischen Attribut und Format.

                const SwFmtAnchor& rAnchor = pNewFmt->GetAnchor();
                if( FLY_IN_CNTNT == rAnchor.GetAnchorId() )
                {
                    const SwPosition *pPos = rAnchor.GetCntntAnchor();
                    SwTxtNode *pTxtNode = pPos->nNode.GetNode().GetTxtNode();
                    ASSERT( pTxtNode->HasHints(), "Missing FlyInCnt-Hint." );
                    const xub_StrLen nIdx = pPos->nContent.GetIndex();
                    SwTxtAttr *pHnt = pTxtNode->GetTxtAttr( nIdx, RES_TXTATR_FLYCNT );

#ifndef PRODUCT
                    ASSERT( pHnt && pHnt->Which() == RES_TXTATR_FLYCNT,
                                "Missing FlyInCnt-Hint." );
                    ASSERT( pHnt && ((SwFmtFlyCnt&)pHnt->GetFlyCnt()).
                                GetFrmFmt() == pOldFmt,
                                "Wrong TxtFlyCnt-Hint." );
#endif
                    ((SwFmtFlyCnt&)pHnt->GetFlyCnt()).SetFlyFmt( pNewFmt );
                }


                //Der Alte soll keinen Umlauf haben, und er soll oben/mittig
                //ausgerichtet sein.
                //Ausserdem soll die Breite 100% betragen und bei Aenderungen
                //Die Hoehe mit anpassen.
                pNewSet->ClearItem();

                pNewSet->Put( SwFmtSurround( SURROUND_NONE ) );
                pNewSet->Put( SwFmtVertOrient( VERT_TOP ) );
                pNewSet->Put( SwFmtHoriOrient( HORI_CENTER ) );

                aFrmSize = pOldFmt->GetFrmSize();
                aFrmSize.SetWidthPercent( 100 );
                aFrmSize.SetHeightPercent( 255 );
                pNewSet->Put( aFrmSize );

                //Die Attribute setzen wir hart, weil sie sonst aus der Vorlage
                //kommen koenten und dann passt die Grossenberechnung nicht mehr.
                if( bCpyBrd )
                {
                    pNewSet->Put( SvxBoxItem() );
                    pNewSet->Put( SvxShadowItem() );
                }
                pNewSet->Put( SvxLRSpaceItem() );
                pNewSet->Put( SvxULSpaceItem() );

                //Der Alte ist absatzgebunden, und zwar am Absatz im neuen.
                SwFmtAnchor aAnch( FLY_AT_CNTNT );
                SwNodeIndex aAnchIdx( *pNewFmt->GetCntnt().GetCntntIdx(), 1 );
                pNew = aAnchIdx.GetNode().GetTxtNode();
                SwPosition aPos( aAnchIdx );
                aAnch.SetAnchor( &aPos );
                pNewSet->Put( aAnch );

                if( pUndo )
                    pUndo->SetFlys( *pOldFmt, *pNewSet, *pNewFmt );
                else
                    pOldFmt->SetAttr( *pNewSet );

                delete pNewSet;

                //Nun nur noch die Flys erzeugen lassen. Das ueberlassen
                //wir vorhanden Methoden (insb. fuer InCntFlys etwas aufwendig).
                pNewFmt->MakeFrms();
            }
            break;

        default:
            ASSERT( !this, "Neuer LabelType?." );
    }
    ASSERT( pNew, "No Label inserted" );

    if( pNew )
    {
        //String aufbereiten
        String aTxt( aFld.GetTyp()->GetName() );
        aTxt += ' ';
        xub_StrLen nIdx = aTxt.Len();
        aTxt += rTxt;

        //String einfuegen
        SwIndex aIdx( pNew, 0 );
        pNew->Insert( aTxt, aIdx );

        //Feld einfuegen
        pNew->Insert( SwFmtFld( aFld ), nIdx, nIdx );

        if ( bTable )
        {
            if ( bBefore )
            {
                if ( !pNew->GetSwAttrSet().GetKeep().GetValue()  )
                    pNew->SwCntntNode::SetAttr( SvxFmtKeepItem( sal_True ) );
            }
            else
            {
                SwTableNode *pNd = GetNodes()[nNdIdx]->GetStartNode()->GetTableNode();
                SwTable &rTbl = pNd->GetTable();
                if ( !rTbl.GetFrmFmt()->GetKeep().GetValue() )
                    rTbl.GetFrmFmt()->SetAttr( SvxFmtKeepItem( sal_True ) );
                if ( pUndo )
                    pUndo->SetUndoKeep();
            }
        }
    }

    if( pUndo )
        AppendUndo( pUndo );
    else
        DelAllUndoObj();
    DoUndo( bWasUndo );

    return pNewFmt;
}

/*************************************************************************
|*
|*  SwDoc::InsertDrawLabel()
|*
|*  Ersterstellung      MIB 7. Dez. 98
|*  Letzte Aenderung    MIB 7. Dez. 98
|*
|*************************************************************************/

SwFlyFrmFmt* SwDoc::InsertDrawLabel( const String &rTxt,
                                     const sal_uInt16 nId,
                                     SdrObject& rSdrObj )
{

    SwDrawContact *pContact = (SwDrawContact*)GetUserCall( &rSdrObj );
    ASSERT( RES_DRAWFRMFMT == pContact->GetFmt()->Which(),
            "Kein DrawFrmFmt" );
    if( !pContact )
        return 0;

    SwDrawFrmFmt *pOldFmt = (SwDrawFrmFmt *)pContact->GetFmt();
    if( !pOldFmt )
        return 0;

    sal_Bool bWasUndo = DoesUndo();
    sal_Bool bWasNoDrawUndo = IsNoDrawUndoObj();
    SwUndoInsertLabel* pUndo = 0;
    if( bWasUndo )
    {
        ClearRedo();
        pUndo = new SwUndoInsertLabel( LTYPE_DRAW, rTxt, sal_False, nId, sal_False );
        DoUndo( sal_False );
        SetNoDrawUndoObj( sal_True );
    }

    // Erstmal das Feld bauen, weil ueber den Namen die TxtColl besorgt
    // werden muss
    ASSERT( nId < GetFldTypes()->Count(), "FldType ueberindiziert." );
    SwFieldType *pType = (*GetFldTypes())[nId];
    ASSERT( pType->Which() == RES_SETEXPFLD, "Falsche Id fuer Label" );
    SwSetExpField aFld( (SwSetExpFieldType*)pType, aEmptyStr,
                        SVX_NUM_ARABIC);

    SwTxtFmtColl *pColl = FindTxtFmtCollByName( pType->GetName() );
    if( !pColl )
    {
        ASSERT( !this, "TxtCollection fuer Label nicht gefunden." );
        pColl = GetTxtCollFromPool( RES_POOLCOLL_TEXT );
    }

    SwTxtNode *pNew = 0;
    SwFlyFrmFmt *pNewFmt = 0;

    // Rahmen zerstoeren, neuen Rahmen einfuegen, entsprechenden
    // Node mit Feld in den neuen Rahmen, den alten Rahmen mit
    // dem Object (Grafik/Ole) absatzgebunden in den neuen Rahmen,
    // Frames erzeugen.

    pOldFmt->DelFrms();

    //Bei InCntnt's wird es spannend: Das TxtAttribut muss
    //vernichtet werden. Leider reisst dies neben den Frms auch
    //noch das Format mit in sein Grab. Um dass zu unterbinden
    //loesen wir vorher die Verbindung zwischen Attribut und Format.
    SfxItemSet* pNewSet = pOldFmt->GetAttrSet().Clone( sal_False );

    // Ggf. Groesse und Position des Rahmens schuetzen
    if ( rSdrObj.IsMoveProtect() || rSdrObj.IsResizeProtect() )
    {
        SvxProtectItem aProtect;
        aProtect.SetCntntProtect( sal_False );
        aProtect.SetPosProtect( rSdrObj.IsMoveProtect() );
        aProtect.SetSizeProtect( rSdrObj.IsResizeProtect() );
        pNewSet->Put( aProtect );
    }

    // Umlauf uebernehmen
    lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_SURROUND );

    // Den Rahmen ggf. in den Hintergrund schicken.
    sal_Int8 nLayerId = rSdrObj.GetLayer();
    if ( GetHellId() != nLayerId )
    {
        SvxOpaqueItem aOpaque;
        aOpaque.SetValue( sal_True );
        pNewSet->Put( aOpaque );
    }

    // Position uebernehmen
    Point aPoint( rSdrObj.GetRelativePos() );
    SwFmtVertOrient aVert( aPoint.B(), VERT_NONE, FRAME );
    SwFmtHoriOrient aHori( aPoint.A(), HORI_NONE, FRAME );
    pNewSet->Put( aVert );
    pNewSet->Put( aHori );

    pNewSet->Put( pOldFmt->GetAnchor() );

    //In der Hoehe soll der neue Varabel sein!
     Size aSz( rSdrObj.GetBoundRect().GetSize() );
    SwFmtFrmSize aFrmSize( ATT_MIN_SIZE, aSz.Width(), aSz.Height() );
    pNewSet->Put( aFrmSize );

    // Abstaende auf den neuen Rahmen uebertragen. Eine Umrandung
    // gibt es beu Zeichen-Objekten nicht, also muss sie geloescht
    // werden.
    // MA: Falsch sie wird nicht gesetzt, denn die aus der Vorlage
    // soll ruhig wirksam werden
    pNewSet->Put( pOldFmt->GetLRSpace() );
    pNewSet->Put( pOldFmt->GetULSpace() );

    SwStartNode* pSttNd = GetNodes().MakeTextSection(
                SwNodeIndex( GetNodes().GetEndOfAutotext() ),
                SwFlyStartNode, pColl );

    pNewFmt = MakeFlyFrmFmt( GetUniqueFrameName(),
                        GetFrmFmtFromPool( RES_POOLFRM_FRAME ));

    // JP 28.10.99: Bug 69487 - set border and shadow to default if the
    //              template contains any.
    if( SFX_ITEM_SET == pNewFmt->GetAttrSet().GetItemState( RES_BOX, sal_True ))
        pNewSet->Put( *GetDfltAttr( RES_BOX ) );

    if( SFX_ITEM_SET == pNewFmt->GetAttrSet().GetItemState(RES_SHADOW,sal_True))
        pNewSet->Put( *GetDfltAttr( RES_SHADOW ) );

    pNewFmt->SetAttr( SwFmtCntnt( pSttNd ));
    pNewFmt->SetAttr( *pNewSet );

    const SwFmtAnchor& rAnchor = pNewFmt->GetAnchor();
    if( FLY_IN_CNTNT == rAnchor.GetAnchorId() )
    {
        const SwPosition *pPos = rAnchor.GetCntntAnchor();
        SwTxtNode *pTxtNode = pPos->nNode.GetNode().GetTxtNode();
        ASSERT( pTxtNode->HasHints(), "Missing FlyInCnt-Hint." );
        const xub_StrLen nIdx = pPos->nContent.GetIndex();
        SwTxtAttr *pHnt = pTxtNode->GetTxtAttr( nIdx, RES_TXTATR_FLYCNT );

#ifndef PRODUCT
        ASSERT( pHnt && pHnt->Which() == RES_TXTATR_FLYCNT,
                    "Missing FlyInCnt-Hint." );
        ASSERT( pHnt && ((SwFmtFlyCnt&)pHnt->GetFlyCnt()).
                    GetFrmFmt() == (SwFrmFmt*)pOldFmt,
                    "Wrong TxtFlyCnt-Hint." );
#endif
        ((SwFmtFlyCnt&)pHnt->GetFlyCnt()).SetFlyFmt( pNewFmt );
    }


    //Der Alte soll keinen Umlauf haben, und er soll oben/mittig
    //ausgerichtet sein.
    pNewSet->ClearItem();

    pNewSet->Put( SwFmtSurround( SURROUND_NONE ) );
    if( nLayerId == GetHellId() )
        rSdrObj.SetLayer( GetHeavenId() );
    pNewSet->Put( SvxLRSpaceItem() );
    pNewSet->Put( SvxULSpaceItem() );

    rSdrObj.SetRelativePos( Point(0,0) );

    //Der Alte ist absatzgebunden, und zwar am Absatz im neuen.
    SwFmtAnchor aAnch( FLY_AT_CNTNT );
    SwNodeIndex aAnchIdx( *pNewFmt->GetCntnt().GetCntntIdx(), 1 );
    pNew = aAnchIdx.GetNode().GetTxtNode();
    SwPosition aPos( aAnchIdx );
    aAnch.SetAnchor( &aPos );
    pNewSet->Put( aAnch );

    if( pUndo )
    {
        pUndo->SetFlys( *pOldFmt, *pNewSet, *pNewFmt );
        pUndo->SetDrawObj( aPoint, nLayerId );
    }
    else
        pOldFmt->SetAttr( *pNewSet );

    delete pNewSet;

    //Nun nur noch die Flys erzeugen lassen. Das ueberlassen
    //wir vorhanden Methoden (insb. fuer InCntFlys etwas aufwendig).
    pNewFmt->MakeFrms();

    ASSERT( pNew, "No Label inserted" );

    if( pNew )
    {
        //String aufbereiten
        String aTxt( aFld.GetTyp()->GetName() );
        aTxt += ' ';
        xub_StrLen nIdx = aTxt.Len();
        aTxt += rTxt;

        //String einfuegen
        SwIndex aIdx( pNew, 0 );
        pNew->Insert( aTxt, aIdx );

        //Feld einfuegen
        pNew->Insert( SwFmtFld( aFld ), nIdx, nIdx );
    }

    if( pUndo )
    {
        AppendUndo( pUndo );
        SetNoDrawUndoObj( bWasNoDrawUndo );
    }
    else
        DelAllUndoObj();
    DoUndo( bWasUndo );

    return pNewFmt;
}

/*************************************************************************
|*
|*  SwDoc::DoIdleJobs()
|*
|*  Ersterstellung      OK 30.03.94
|*  Letzte Aenderung    MA 09. Jun. 95
|*
|*************************************************************************/

IMPL_LINK( SwDoc, DoIdleJobs, Timer *, pTimer )
{
    if( !SfxProgress::GetActiveProgress( pDocShell ) &&
        GetRootFrm() && GetRootFrm()->GetCurrShell() )
    {
        ViewShell *pSh, *pStartSh;
        pSh = pStartSh = GetRootFrm()->GetCurrShell();
        do {
            if( pSh->ActionPend() )
                return 0;
            pSh = (ViewShell*)pSh->GetNext();
        } while( pSh != pStartSh );

        sal_uInt16 nFldUpdFlag;
        if( GetRootFrm()->IsIdleFormat() )
            GetRootFrm()->GetCurrShell()->LayoutIdle();
        else if( ( AUTOUPD_FIELD_ONLY == ( nFldUpdFlag = GetFldUpdateFlags() )
                    || AUTOUPD_FIELD_AND_CHARTS == nFldUpdFlag ) &&
                GetUpdtFlds().IsFieldsDirty() &&
                !GetUpdtFlds().IsInUpdateFlds() &&
                !IsExpFldsLocked()
                // das umschalten der Feldname fuehrt zu keinem Update der
                // Felder, also der "Hintergrund-Update" immer erfolgen
                /* && !pStartSh->GetViewOptions()->IsFldName()*/ )
        {
            // chaos::Action-Klammerung!
            GetUpdtFlds().SetInUpdateFlds( sal_True );

            GetRootFrm()->StartAllAction();

            GetSysFldType( RES_CHAPTERFLD )->Modify( 0, 0 );    // KapitelFld
            UpdateExpFlds( 0, sal_False );      // Expression-Felder Updaten
            UpdateTblFlds();                // Tabellen
            UpdateRefFlds();                // Referenzen

            if( AUTOUPD_FIELD_AND_CHARTS == nFldUpdFlag )
                aChartTimer.Start();

            GetRootFrm()->EndAllAction();

            GetUpdtFlds().SetInUpdateFlds( sal_False );
            GetUpdtFlds().SetFieldsDirty( sal_False );
        }
    }
    return 0;
}

IMPL_STATIC_LINK( SwDoc, BackgroundDone, SvxBrushItem*, EMPTYARG )
{
    ViewShell *pSh, *pStartSh;
    pSh = pStartSh = pThis->GetRootFrm()->GetCurrShell();
    if( pStartSh )
        do {
            if( pSh->GetWin() )
            {
                //Fuer Repaint mir virtuellen Device sorgen.
                pSh->LockPaint();
                pSh->UnlockPaint( sal_True );
            }
            pSh = (ViewShell*)pSh->GetNext();
        } while( pSh != pStartSh );
    return 0;
}

String lcl_GetUniqueFlyName( const SwDoc* pDoc, sal_uInt16 nDefStrId )
{
    ResId aId( nDefStrId, pSwResMgr );
    String aName( aId );
    xub_StrLen nNmLen = aName.Len();

    const SwSpzFrmFmts& rFmts = *pDoc->GetSpzFrmFmts();

    sal_uInt16 nNum, nTmp, nFlagSize = ( rFmts.Count() / 8 ) +2;
    sal_uInt8* pSetFlags = new sal_uInt8[ nFlagSize ];
    memset( pSetFlags, 0, nFlagSize );

    for( sal_uInt16 n = 0; n < rFmts.Count(); ++n )
    {
        const SwFrmFmt* pFlyFmt = rFmts[ n ];
        if( RES_FLYFRMFMT == pFlyFmt->Which() &&
            pFlyFmt->GetName().Match( aName ) == nNmLen )
        {
            // Nummer bestimmen und das Flag setzen
            nNum = pFlyFmt->GetName().Copy( nNmLen ).ToInt32();
            if( nNum-- && nNum < rFmts.Count() )
                pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
        }
    }

    // alle Nummern entsprechend geflag, also bestimme die richtige Nummer
    nNum = rFmts.Count();
    for( n = 0; n < nFlagSize; ++n )
        if( 0xff != ( nTmp = pSetFlags[ n ] ))
        {
            // also die Nummer bestimmen
            nNum = n * 8;
            while( nTmp & 1 )
                ++nNum, nTmp >>= 1;
            break;
        }

    __DELETE( nFlagSize ) pSetFlags;
    return aName += String::CreateFromInt32( ++nNum );
}

String SwDoc::GetUniqueGrfName() const
{
    return ::lcl_GetUniqueFlyName( this, STR_GRAPHIC_DEFNAME );
}

String SwDoc::GetUniqueOLEName() const
{
    return ::lcl_GetUniqueFlyName( this, STR_OBJECT_DEFNAME );
}

String SwDoc::GetUniqueFrameName() const
{
    return ::lcl_GetUniqueFlyName( this, STR_FRAME_DEFNAME );
}

const SwFlyFrmFmt* SwDoc::FindFlyByName( const String& rName, sal_Int8 nNdTyp ) const
{
    const SwSpzFrmFmts& rFmts = *GetSpzFrmFmts();
    for( sal_uInt16 n = rFmts.Count(); n; )
    {
        const SwFrmFmt* pFlyFmt = rFmts[ --n ];
        const SwNodeIndex* pIdx;
        if( RES_FLYFRMFMT == pFlyFmt->Which() && pFlyFmt->GetName() == rName &&
            0 != ( pIdx = pFlyFmt->GetCntnt().GetCntntIdx() ) &&
            pIdx->GetNode().GetNodes().IsDocNodes() )
        {
            if( nNdTyp )
            {
                // dann noch auf den richtigen Node-Typ abfragen
                const SwNode* pNd = GetNodes()[ pIdx->GetIndex()+1 ];
                if( nNdTyp == ND_TEXTNODE
                        ? !pNd->IsNoTxtNode()
                        : nNdTyp == pNd->GetNodeType() )
                    return (SwFlyFrmFmt*)pFlyFmt;
            }
            else
                return (SwFlyFrmFmt*)pFlyFmt;
        }
    }
    return 0;
}

void SwDoc::SetFlyName( SwFlyFrmFmt& rFmt, const String& rName )
{
    String sName( rName );
    if( !rName.Len() || FindFlyByName( rName ) )
    {
        sal_uInt16 nTyp = STR_FRAME_DEFNAME;
        const SwNodeIndex* pIdx = rFmt.GetCntnt().GetCntntIdx();
        if( pIdx && pIdx->GetNode().GetNodes().IsDocNodes() )
            switch( GetNodes()[ pIdx->GetIndex() + 1 ]->GetNodeType() )
            {
            case ND_GRFNODE:    nTyp = STR_GRAPHIC_DEFNAME; break;
            case ND_OLENODE:    nTyp = STR_OBJECT_DEFNAME;  break;
            }
        sName = ::lcl_GetUniqueFlyName( this, nTyp );
    }
    rFmt.SetName( sName );
}

void SwDoc::SetAllUniqueFlyNames()
{
    sal_uInt16 n, nFlyNum = 0, nGrfNum = 0, nOLENum = 0;

    ResId nFrmId( STR_FRAME_DEFNAME, pSwResMgr ),
          nGrfId( STR_GRAPHIC_DEFNAME, pSwResMgr ),
          nOLEId( STR_OBJECT_DEFNAME, pSwResMgr );
    String sFlyNm( nFrmId );
    String sGrfNm( nGrfId );
    String sOLENm( nOLEId );

    if( 255 < ( n = GetSpzFrmFmts()->Count() ))
        n = 255;
    SwSpzFrmFmts aArr( (sal_Int8)n, 10 );
    SwFrmFmtPtr pFlyFmt;
    sal_Bool bLoadedFlag = sal_True;            // noch etwas fuers Layout

    for( n = GetSpzFrmFmts()->Count(); n; )
    {
        if( RES_FLYFRMFMT == (pFlyFmt = (*GetSpzFrmFmts())[ --n ])->Which() )
        {
            sal_uInt16 *pNum = 0;
            xub_StrLen nLen;
            const String& rNm = pFlyFmt->GetName();
            if( rNm.Len() )
            {
                if( rNm.Match( sGrfNm ) == ( nLen = sGrfNm.Len() ))
                    pNum = &nGrfNum;
                else if( rNm.Match( sFlyNm ) == ( nLen = sFlyNm.Len() ))
                    pNum = &nFlyNum;
                else if( rNm.Match( sOLENm ) == ( nLen = sOLENm.Len() ))
                    pNum = &nOLENum;

                if( pNum && *pNum < ( nLen = rNm.Copy( nLen ).ToInt32() ))
                    *pNum = nLen;
            }
            else
                // das wollen wir nachher setzen
                aArr.Insert( pFlyFmt, aArr.Count() );

        }
        if( bLoadedFlag )
        {
            const SwFmtAnchor& rAnchor = pFlyFmt->GetAnchor();
            if( ( FLY_PAGE == rAnchor.GetAnchorId() &&
                    rAnchor.GetCntntAnchor() ) ||
                // oder werden DrawObjecte rel. zu irgendetwas ausgerichtet?
                ( RES_DRAWFRMFMT == pFlyFmt->Which() && (
                    SFX_ITEM_SET == pFlyFmt->GetItemState(
                                        RES_VERT_ORIENT )||
                    SFX_ITEM_SET == pFlyFmt->GetItemState(
                                        RES_HORI_ORIENT ))) )
                bLoadedFlag = sal_False;
        }
    }

    const SwNodeIndex* pIdx;

    for( n = aArr.Count(); n; )
        if( 0 != ( pIdx = ( pFlyFmt = aArr[ --n ])->GetCntnt().GetCntntIdx() )
            && pIdx->GetNode().GetNodes().IsDocNodes() )
        {
            sal_uInt16 nNum;
            String sNm;
            switch( GetNodes()[ pIdx->GetIndex() + 1 ]->GetNodeType() )
            {
            case ND_GRFNODE:
                sNm = sGrfNm;
                nNum = ++nGrfNum;
                break;
            case ND_OLENODE:
                sNm = sOLENm;
                nNum = ++nOLENum;
                break;
            default:
                sNm = sFlyNm;
                nNum = ++nFlyNum;
                break;
            }
            pFlyFmt->SetName( sNm += String::CreateFromInt32( nNum ));
        }
    aArr.Remove( 0, aArr.Count() );

    if( GetFtnIdxs().Count() )
    {
        SwTxtFtn::SetUniqueSeqRefNo( *this );
        SwNodeIndex aTmp( GetNodes() );
        GetFtnIdxs().UpdateFtn( aTmp );
    }

    // neues Document und keine seitengebundenen Rahmen/DrawObjecte gefunden,
    // die an einem Node verankert sind.
    if( bLoadedFlag )
        SetLoaded( sal_True );
}

sal_Bool SwDoc::IsInHeaderFooter( const SwNodeIndex& rIdx ) const
{
    // gibt es ein Layout, dann ueber das laufen!!
    //  (Das kann dann auch Fly in Fly in Kopfzeile !)
    // MIB 9.2.98: Wird auch vom sw3io benutzt, um festzustellen, ob sich
    // ein Redline-Objekt in einer Kopf- oder Fusszeile befindet. Da
    // Redlines auch an Start- und Endnodes haengen, muss der Index nicht
    // unbedingt der eines Content-Nodes sein.
    SwNode* pNd = &rIdx.GetNode();
    if( pNd->IsCntntNode() && pLayout )
    {
        SwFrm *pFrm = pNd->GetCntntNode()->GetFrm();
        if( pFrm )
        {
            SwFrm *pUp = pFrm->GetUpper();
            while ( pUp && !pUp->IsHeaderFrm() && !pUp->IsFooterFrm() )
            {
                if ( pUp->IsFlyFrm() )
                    pUp = ((SwFlyFrm*)pUp)->GetAnchor();
                pUp = pUp->GetUpper();
            }
            if ( pUp )
                return sal_True;

            return sal_False;
        }
    }


    const SwNode* pFlyNd = pNd->FindFlyStartNode();
    while( pFlyNd )
    {
        // dann ueber den Anker nach oben "hangeln"
        for( sal_uInt16 n = 0; n < GetSpzFrmFmts()->Count(); ++n )
        {
            const SwFrmFmt* pFmt = (*GetSpzFrmFmts())[ n ];
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if( pIdx && pFlyNd == &pIdx->GetNode() )
            {
                const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
                if( FLY_PAGE == rAnchor.GetAnchorId() ||
                    !rAnchor.GetCntntAnchor() )
                    return sal_False;

                pNd = &rAnchor.GetCntntAnchor()->nNode.GetNode();
                pFlyNd = pNd->FindFlyStartNode();
                break;
            }
        }
        if( n >= GetSpzFrmFmts()->Count() )
        {
            ASSERT( bInReading, "Fly-Section aber kein Format gefunden" );
            return sal_False;
        }
    }

    return 0 != pNd->FindHeaderStartNode() ||
            0 != pNd->FindFooterStartNode();
}


