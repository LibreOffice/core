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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <unotools/linguprops.hxx>
#include <unotools/lingucfg.hxx>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <hintids.hxx>
#include <com/sun/star/util/XCloseable.hpp>
#include <sfx2/progress.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/prntitem.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdouno.hxx>
#include <svx/fmpage.hxx>
#include <editeng/frmdiritem.hxx>

#include <swmodule.hxx>
#include <modcfg.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <rtl/logfile.hxx>
#include <SwStyleNameMapper.hxx>
#include <fchrfmt.hxx>
#include <errhdl.hxx>
#include <frmatr.hxx>
#include <txatbase.hxx>
#include <fmtfld.hxx>
#include <fmtornt.hxx>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtsrnd.hxx>
#include <fmtflcnt.hxx>
#include <fmtcnct.hxx>
#include <frmfmt.hxx>
#include <txtflcnt.hxx>
#include <docfld.hxx>   // fuer Expression-Felder
#include <pam.hxx>
#include <ndtxt.hxx>
#include <ndnotxt.hxx>
#include <ndole.hxx>
#include <doc.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <flyfrm.hxx>
#include <fesh.hxx>
#include <docsh.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <swundo.hxx>
#include <flypos.hxx>
#include <undobj.hxx>
#include <expfld.hxx>       // InsertLabel
#include <poolfmt.hxx>      // PoolVorlagen-Id's
#include <docary.hxx>
#include <swtable.hxx>
#include <tblsel.hxx>
#include <viewopt.hxx>
#include <fldupde.hxx>
#include <txtftn.hxx>
#include <ftnidx.hxx>
#include <ftninfo.hxx>
#include <pagedesc.hxx>
#include <PostItMgr.hxx>
#include <comcore.hrc>      // STR-ResId's

#include <unoframe.hxx>

#include <sortedobjs.hxx>

#include <vector>

using namespace ::com::sun::star;
using ::rtl::OUString;

#define DEF_FLY_WIDTH    2268   //Defaultbreite fuer FlyFrms    (2268 == 4cm)


static bool lcl_IsItemSet(const SwCntntNode & rNode, USHORT which)
{
    bool bResult = false;

    if (SFX_ITEM_SET == rNode.GetSwAttrSet().GetItemState(which))
        bResult = true;

    return bResult;
}

/*************************************************************************
|*
|*  SwDoc::MakeLayoutFmt()
|*
|*  Beschreibung        Erzeugt ein neues Format das in seinen Einstellungen
|*      Defaultmaessig zu dem Request passt. Das Format wird in das
|*      entsprechende Formate-Array gestellt.
|*      Wenn bereits ein passendes Format existiert, so wird dies
|*      zurueckgeliefert.
|*************************************************************************/
SwFrmFmt *SwDoc::MakeLayoutFmt( RndStdIds eRequest, const SfxItemSet* pSet )
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
            pFmt = new SwFrmFmt( GetAttrPool(),
                                 (bHeader ? "Header" : "Footer"),
                                 GetDfltFrmFmt() );

            SwNodeIndex aTmpIdx( GetNodes().GetEndOfAutotext() );
            SwStartNode* pSttNd =
                GetNodes().MakeTextSection
                ( aTmpIdx,
                  bHeader ? SwHeaderStartNode : SwFooterStartNode,
                  GetTxtCollFromPool(static_cast<sal_uInt16>( bHeader
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
                                     ) ) );
            pFmt->SetFmtAttr( SwFmtCntnt( pSttNd ));

            if( pSet )      // noch ein paar Attribute setzen ?
                pFmt->SetFmtAttr( *pSet );

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
                pFmt->SetFmtAttr( *pSet );

            if( DoesUndo() )
            {
                ClearRedo();
                AppendUndo( new SwUndoInsLayFmt( pFmt,0,0 ));
            }
        }
        break;

#if OSL_DEBUG_LEVEL > 1
    case FLY_AT_PAGE:
    case FLY_AT_CHAR:
    case FLY_AT_FLY:
    case FLY_AT_PARA:
    case FLY_AS_CHAR:
        OSL_ENSURE( false, "use new interface instead: SwDoc::MakeFlySection!" );
        break;
#endif

    default:
        OSL_ENSURE( !this,
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
    if( pCntIdx && !DoesUndo() )
    {
        //Verbindung abbauen, falls es sich um ein OLE-Objekt handelt.
        SwOLENode* pOLENd = GetNodes()[ pCntIdx->GetIndex()+1 ]->GetOLENode();
        if( pOLENd && pOLENd->GetOLEObj().IsOleRef() )
        {

            // TODO/MBA: the old object closed the object, cleared all references to it, but didn't remove it from the container.
            // I have no idea, why, nobody could explain it - so I do my very best to mimic this behavior
            //uno::Reference < util::XCloseable > xClose( pOLENd->GetOLEObj().GetOleRef(), uno::UNO_QUERY );
            //if ( xClose.is() )
            {
                try
                {
                    pOLENd->GetOLEObj().GetOleRef()->changeState( embed::EmbedStates::LOADED );
                }
                catch ( uno::Exception& )
                {
                }
            }

        }
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
        // #i32089# - delete at-frame anchored objects
        if ( nWh == RES_FLYFRMFMT )
        {
            // determine frame formats of at-frame anchored objects
            const SwNodeIndex* pCntntIdx = pFmt->GetCntnt().GetCntntIdx();
            if ( pCntntIdx )
            {
                const SwSpzFrmFmts* pTbl = pFmt->GetDoc()->GetSpzFrmFmts();
                if ( pTbl )
                {
                    std::vector<SwFrmFmt*> aToDeleteFrmFmts;
                    const ULONG nNodeIdxOfFlyFmt( pCntntIdx->GetIndex() );

                    for ( USHORT i = 0; i < pTbl->Count(); ++i )
                    {
                        SwFrmFmt* pTmpFmt = (*pTbl)[i];
                        const SwFmtAnchor &rAnch = pTmpFmt->GetAnchor();
                        if ( rAnch.GetAnchorId() == FLY_AT_FLY &&
                             rAnch.GetCntntAnchor()->nNode.GetIndex() == nNodeIdxOfFlyFmt )
                        {
                            aToDeleteFrmFmts.push_back( pTmpFmt );
                        }
                    }

                    // delete found frame formats
                    while ( !aToDeleteFrmFmts.empty() )
                    {
                        SwFrmFmt* pTmpFmt = aToDeleteFrmFmts.back();
                        pFmt->GetDoc()->DelLayoutFmt( pTmpFmt );

                        aToDeleteFrmFmts.pop_back();
                    }
                }
            }
        }
        // <--

        //Inhalt Loeschen.
        if( pCntIdx )
        {
            SwNode *pNode = &pCntIdx->GetNode();
            ((SwFmtCntnt&)pFmt->GetFmtAttr( RES_CNTNT )).SetNewCntntIdx( 0 );
            DeleteSection( pNode );
        }

        // ggfs. bei Zeichengebundenen Flys das Zeichen loeschen
        const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
        if ((FLY_AS_CHAR == rAnchor.GetAnchorId()) && rAnchor.GetCntntAnchor())
        {
            const SwPosition* pPos = rAnchor.GetCntntAnchor();
            SwTxtNode *pTxtNd = pPos->nNode.GetNode().GetTxtNode();

            // attribute is still in text node, delete it
            if ( pTxtNd )
            {
                SwTxtFlyCnt* const pAttr = static_cast<SwTxtFlyCnt*>(
                    pTxtNd->GetTxtAttrForCharAt( pPos->nContent.GetIndex(),
                        RES_TXTATR_FLYCNT ));
                if ( pAttr && (pAttr->GetFlyCnt().GetFrmFmt() == pFmt) )
                {
                    // dont delete, set pointer to 0
                    const_cast<SwFmtFlyCnt&>(pAttr->GetFlyCnt()).SetFlyFmt();
                    SwIndex aIdx( pPos->nContent );
                    pTxtNd->EraseText( aIdx, 1 );
                }
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
|*************************************************************************/
SwFrmFmt *SwDoc::CopyLayoutFmt( const SwFrmFmt& rSource,
                                const SwFmtAnchor& rNewAnchor,
                                bool bSetTxtFlyAtt, bool bMakeFrms )
{
    const bool bFly = RES_FLYFRMFMT == rSource.Which();
    const bool bDraw = RES_DRAWFRMFMT == rSource.Which();
    OSL_ENSURE( bFly || bDraw, "this method only works for fly or draw" );

    SwDoc* pSrcDoc = (SwDoc*)rSource.GetDoc();

    // May we copy this object?
    // We may, unless it's 1) it's a control (and therfore a draw)
    //                     2) anchored in a header/footer
    //                     3) anchored (to paragraph?)
    bool bMayNotCopy = false;
    if( bDraw )
    {
        const SwDrawContact* pDrawContact =
            static_cast<const SwDrawContact*>( rSource.FindContactObj() );

        bMayNotCopy =
            ((FLY_AT_PARA == rNewAnchor.GetAnchorId()) ||
             (FLY_AT_FLY  == rNewAnchor.GetAnchorId()) ||
             (FLY_AT_CHAR == rNewAnchor.GetAnchorId())) &&
            rNewAnchor.GetCntntAnchor() &&
            IsInHeaderFooter( rNewAnchor.GetCntntAnchor()->nNode ) &&
            pDrawContact != NULL  &&
            pDrawContact->GetMaster() != NULL  &&
            CheckControlLayer( pDrawContact->GetMaster() );
    }

    // just return if we can't copy this
    if( bMayNotCopy )
        return NULL;

    SwFrmFmt* pDest = GetDfltFrmFmt();
    if( rSource.GetRegisteredIn() != pSrcDoc->GetDfltFrmFmt() )
        pDest = CopyFrmFmt( *(SwFrmFmt*)rSource.GetRegisteredIn() );
    if( bFly )
    {
        // #i11176#
        // To do a correct cloning concerning the ZOrder for all objects
        // it is necessary to actually create a draw object for fly frames, too.
        // These are then added to the DrawingLayer (which needs to exist).
        // Together with correct sorting of all drawinglayer based objects
        // before cloning ZOrder transfer works correctly then.
        SwFlyFrmFmt *pFormat = MakeFlyFrmFmt( rSource.GetName(), pDest );
        pDest = pFormat;

        SwXFrame::GetOrCreateSdrObject(pFormat);
    }
    else
        pDest = MakeDrawFrmFmt( aEmptyStr, pDest );

    // alle anderen/neue Attribute kopieren.
    pDest->CopyAttrs( rSource );

    //Chains werden nicht kopiert.
    pDest->ResetFmtAttr( RES_CHAIN );

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
        pDest->SetFmtAttr( aAttr );
        pDest->SetFmtAttr( rNewAnchor );

        if( !mbCopyIsMove || this != pSrcDoc )
        {
            if( mbInReading )
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
            AppendUndo( new SwUndoInsLayFmt( pDest,0,0 ));
        }

        // sorge dafuer das auch Fly's in Fly's kopiert werden
        aIdx = *pSttNd->EndOfSectionNode();
        pSrcDoc->CopyWithFlyInFly( aRg, 0, aIdx, sal_False, sal_True, sal_True );
    }
    else
    {
        OSL_ENSURE( RES_DRAWFRMFMT == rSource.Which(), "Weder Fly noch Draw." );
        // #i52780# - Note: moving object to visible layer not needed.
        SwDrawContact* pSourceContact = (SwDrawContact *)rSource.FindContactObj();

        SwDrawContact* pContact = new SwDrawContact( (SwDrawFrmFmt*)pDest,
                                CloneSdrObj( *pSourceContact->GetMaster(),
                                        mbCopyIsMove && this == pSrcDoc ) );
        // #i49730# - notify draw frame format that position attributes are
        // already set, if the position attributes are already set at the
        // source draw frame format.
        if ( pDest->ISA(SwDrawFrmFmt) &&
             rSource.ISA(SwDrawFrmFmt) &&
             static_cast<const SwDrawFrmFmt&>(rSource).IsPosAttrSet() )
        {
            static_cast<SwDrawFrmFmt*>(pDest)->PosAttrSet();
        }
        // <--

        if( pDest->GetAnchor() == rNewAnchor )
        {
            // Do *not* connect to layout, if a <MakeFrms> will not be called.
            if ( bMakeFrms )
            {
                pContact->ConnectToLayout( &rNewAnchor );
            }
        }
        else
            pDest->SetFmtAttr( rNewAnchor );

        if( DoesUndo() )
        {
            ClearRedo();
            AppendUndo( new SwUndoInsLayFmt( pDest,0,0 ));
        }
    }

    if (bSetTxtFlyAtt && (FLY_AS_CHAR == rNewAnchor.GetAnchorId()))
    {
        const SwPosition* pPos = rNewAnchor.GetCntntAnchor();
        SwFmtFlyCnt aFmt( pDest );
        pPos->nNode.GetNode().GetTxtNode()->InsertItem(
            aFmt, pPos->nContent.GetIndex(), 0 );
    }

    if( bMakeFrms )
        pDest->MakeFrms();

    return pDest;
}

SdrObject* SwDoc::CloneSdrObj( const SdrObject& rObj, sal_Bool bMoveWithinDoc,
                                sal_Bool bInsInPage )
{
    // #i52858# - method name changed
    SdrPage *pPg = GetOrCreateDrawModel()->GetPage( 0 );
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
        uno::Any aVal;
        uno::Reference< beans::XPropertySet >  xSet(xModel, uno::UNO_QUERY);
        OUString sName(RTL_CONSTASCII_USTRINGPARAM("Name"));
        if( xSet.is() )
            aVal = xSet->getPropertyValue( sName );
        if( bInsInPage )
            pPg->InsertObject( pObj );
        if( xSet.is() )
            xSet->setPropertyValue( sName, aVal );
    }
    else if( bInsInPage )
        pPg->InsertObject( pObj );

    // For drawing objects: set layer of cloned object to invisible layer
    SdrLayerID nLayerIdForClone = rObj.GetLayer();
    if ( !pObj->ISA(SwFlyDrawObj) &&
         !pObj->ISA(SwVirtFlyDrawObj) &&
         !IS_TYPE(SdrObject,pObj) )
    {
        if ( IsVisibleLayerId( nLayerIdForClone ) )
        {
            nLayerIdForClone = GetInvisibleLayerIdByVisibleOne( nLayerIdForClone );
        }
    }
    pObj->SetLayer( nLayerIdForClone );


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
    if( !mbInReading )
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

    pFmt->SetFmtAttr( SwFmtCntnt( rNode.StartOfSectionNode() ));


    const SwFmtAnchor* pAnchor = 0;
    if( pFlySet )
    {
        pFlySet->GetItemState( RES_ANCHOR, sal_False,
                                (const SfxPoolItem**)&pAnchor );
        if( SFX_ITEM_SET == pFlySet->GetItemState( RES_CNTNT, sal_False ))
        {
            SfxItemSet aTmpSet( *pFlySet );
            aTmpSet.ClearItem( RES_CNTNT );
            pFmt->SetFmtAttr( aTmpSet );
        }
        else
            pFmt->SetFmtAttr( *pFlySet );
    }

    // Anker noch nicht gesetzt ?
    RndStdIds eAnchorId = pAnchor ? pAnchor->GetAnchorId()
                                  : pFmt->GetAnchor().GetAnchorId();
    // #i107811# Assure that at-page anchored fly frames have a page num or a
    // content anchor set.
    if ( !pAnchor ||
         ( FLY_AT_PAGE != pAnchor->GetAnchorId() &&
           !pAnchor->GetCntntAnchor() ) ||
         ( FLY_AT_PAGE == pAnchor->GetAnchorId() &&
           !pAnchor->GetCntntAnchor() &&
           pAnchor->GetPageNum() == 0 ) )
    {
        // dann setze ihn, wird im Undo gebraucht
        SwFmtAnchor aAnch( pFmt->GetAnchor() );
        if (pAnchor && (FLY_AT_FLY == pAnchor->GetAnchorId()))
        {
            SwPosition aPos( *rAnchPos.nNode.GetNode().FindFlyStartNode() );
            aAnch.SetAnchor( &aPos );
            eAnchorId = FLY_AT_FLY;
        }
        else
        {
            if( eRequestId != aAnch.GetAnchorId() &&
                SFX_ITEM_SET != pFmt->GetItemState( RES_ANCHOR, sal_True ) )
            {
                aAnch.SetType( eRequestId );
            }

            eAnchorId = aAnch.GetAnchorId();
            if ( FLY_AT_PAGE != eAnchorId ||
                 ( FLY_AT_PAGE == eAnchorId &&
                   ( !pAnchor ||
                     aAnch.GetPageNum() == 0 ) ) )
            {
                aAnch.SetAnchor( &rAnchPos );
            }
        }
        // <--
        pFmt->SetFmtAttr( aAnch );
    }
    else
        eAnchorId = pFmt->GetAnchor().GetAnchorId();

    if ( FLY_AS_CHAR == eAnchorId )
    {
        xub_StrLen nStt = rAnchPos.nContent.GetIndex();
        SwTxtNode * pTxtNode = rAnchPos.nNode.GetNode().GetTxtNode();

        OSL_ENSURE(pTxtNode!= 0, "There should be a SwTxtNode!");

        if (pTxtNode != NULL)
        {
            SwFmtFlyCnt aFmt( pFmt );
            pTxtNode->InsertItem( aFmt, nStt, nStt );
        }
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
                aFmtSize.SetHeightSizeType( ATT_FIX_SIZE );
            }
        }
        pFmt->SetFmtAttr( aFmtSize );
    }

    // Frames anlegen
    if( GetRootFrm() )
        pFmt->MakeFrms();           // ???

    if( DoesUndo() )
    {
        ClearRedo();
        ULONG nNodeIdx = rAnchPos.nNode.GetIndex();
        xub_StrLen nCntIdx = rAnchPos.nContent.GetIndex();
        AppendUndo( new SwUndoInsLayFmt( pFmt, nNodeIdx, nCntIdx ));
    }

    SetModified();
    return pFmt;
}

SwFlyFrmFmt* SwDoc::MakeFlySection( RndStdIds eAnchorType,
                                    const SwPosition* pAnchorPos,
                                    const SfxItemSet* pFlySet,
                                    SwFrmFmt* pFrmFmt, BOOL bCalledFromShell )
{
    SwFlyFrmFmt* pFmt = 0;
    sal_Bool bCallMake = sal_True;
    if ( !pAnchorPos && (FLY_AT_PAGE != eAnchorType) )
    {
        const SwFmtAnchor* pAnch;
        if( (pFlySet && SFX_ITEM_SET == pFlySet->GetItemState(
                RES_ANCHOR, sal_False, (const SfxPoolItem**)&pAnch )) ||
            ( pFrmFmt && SFX_ITEM_SET == pFrmFmt->GetItemState(
                RES_ANCHOR, sal_True, (const SfxPoolItem**)&pAnch )) )
        {
            if ( (FLY_AT_PAGE != pAnch->GetAnchorId()) )
            {
                pAnchorPos = pAnch->GetCntntAnchor();
                if (pAnchorPos)
                {
                    bCallMake = sal_False;
                }
            }
        }
    }

    if( bCallMake )
    {
        if( !pFrmFmt )
            pFrmFmt = GetFrmFmtFromPool( RES_POOLFRM_FRAME );

        sal_uInt16 nCollId = static_cast<sal_uInt16>(
            get(IDocumentSettingAccess::HTML_MODE) ? RES_POOLCOLL_TEXT : RES_POOLCOLL_FRAME );

        /* If there exists no adjust item in the paragraph
            style for the content node of the new fly section
            propagate an existing adjust item at the anchor to the new
            content node. */
        SwCntntNode * pNewTxtNd = GetNodes().MakeTxtNode
            (SwNodeIndex( GetNodes().GetEndOfAutotext()),
             GetTxtCollFromPool( nCollId ));
        SwCntntNode * pAnchorNode = pAnchorPos->nNode.GetNode().GetCntntNode();

        const SfxPoolItem * pItem = NULL;

        if (bCalledFromShell && !lcl_IsItemSet(*pNewTxtNd, RES_PARATR_ADJUST) &&
            SFX_ITEM_SET == pAnchorNode->GetSwAttrSet().
            GetItemState(RES_PARATR_ADJUST, TRUE, &pItem))
            static_cast<SwCntntNode *>(pNewTxtNd)->SetAttr(*pItem);

         pFmt = _MakeFlySection( *pAnchorPos, *pNewTxtNd,
                                eAnchorType, pFlySet, pFrmFmt );
    }
    return pFmt;
}

SwFlyFrmFmt* SwDoc::MakeFlyAndMove( const SwPaM& rPam, const SfxItemSet& rSet,
                                    const SwSelBoxes* pSelBoxes,
                                    SwFrmFmt *pParent )
{
    SwFmtAnchor& rAnch = (SwFmtAnchor&)rSet.Get( RES_ANCHOR );

    StartUndo( UNDO_INSLAYFMT, NULL );

    SwFlyFrmFmt* pFmt = MakeFlySection( rAnch.GetAnchorId(), rPam.GetPoint(),
                                        &rSet, pParent );

    // Wenn Inhalt selektiert ist, so wird dieser jetzt zum Inhalt des
    // neuen Rahmen. Sprich er wird in die entspr. Sektion des NodesArr
    //gemoved.

    if( pFmt )
    {
        do {        // middle check loop
            const SwFmtCntnt &rCntnt = pFmt->GetCntnt();
            OSL_ENSURE( rCntnt.GetCntntIdx(), "Kein Inhalt vorbereitet." );
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

                    MoveNodeRange( aRg, aPos.nNode, DOC_MOVEDEFAULT );
                }
                else
                {
                    rTbl.MakeCopy( this, aPos, *pSelBoxes );
                    // Don't delete a part of a table with row span!!
                    // You could delete the content instead -> ToDo
                    //rTbl.DeleteSel( this, *pSelBoxes, 0, 0, TRUE, TRUE );
                }

                // wenn Tabelle im Rahmen, dann ohne nachfolgenden TextNode
                aIndex = rCntnt.GetCntntIdx()->GetNode().EndOfSectionIndex() - 1;
                OSL_ENSURE( aIndex.GetNode().GetTxtNode(),
                        "hier sollte ein TextNode stehen" );
                aPos.nContent.Assign( 0, 0 );       // Index abmelden !!
                GetNodes().Delete( aIndex, 1 );

//JP erstmal ein Hack, solange keine Flys/Headers/Footers Undofaehig sind
if( DoesUndo() )    // werden erstmal alle Undo - Objecte geloescht.
    DelAllUndoObj();

            }
            else
            {
                // copy all Pams and then delete all
                SwPaM* pTmp = (SwPaM*)&rPam;
                BOOL bOldFlag = mbCopyIsMove, bOldUndo = mbUndo;
                mbCopyIsMove = TRUE;
                mbUndo = FALSE;
                do {
                    if( pTmp->HasMark() &&
                        *pTmp->GetPoint() != *pTmp->GetMark() )
                    {
                        CopyRange( *pTmp, aPos, false );
                    }
                    pTmp = static_cast<SwPaM*>(pTmp->GetNext());
                } while ( &rPam != pTmp );
                mbCopyIsMove = bOldFlag;
                mbUndo = bOldUndo;

                pTmp = (SwPaM*)&rPam;
                do {
                    if( pTmp->HasMark() &&
                        *pTmp->GetPoint() != *pTmp->GetMark() )
                    {
                        DeleteAndJoin( *pTmp );
                    }
                    pTmp = static_cast<SwPaM*>(pTmp->GetNext());
                } while ( &rPam != pTmp );
            }
        } while( sal_False );
    }

    SetModified();

    EndUndo( UNDO_INSLAYFMT, NULL );

    return pFmt;
}

// Einfuegen eines DrawObjectes. Das Object muss bereits im DrawModel
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
        pFmt->SetFmtAttr( *pFlyAttrSet );
    }

    RndStdIds eAnchorId = pAnchor ? pAnchor->GetAnchorId()
                                  : pFmt->GetAnchor().GetAnchorId();

    // Anker noch nicht gesetzt ?
    // DrawObjecte duerfen niemals in Kopf-/Fusszeilen landen.
    const bool bIsAtCntnt = (FLY_AT_PAGE != eAnchorId);

    const SwNodeIndex* pChkIdx = 0;
    if( !pAnchor )
    {
        pChkIdx = &rRg.GetPoint()->nNode;
    }
    else if( bIsAtCntnt )
    {
        pChkIdx = pAnchor->GetCntntAnchor()
                    ? &pAnchor->GetCntntAnchor()->nNode
                    : &rRg.GetPoint()->nNode;
    }

    // Allow drawing objects in header/footer, but control objects aren't
    // allowed in header/footer.
    if( pChkIdx &&
        ::CheckControlLayer( &rDrawObj ) &&
        IsInHeaderFooter( *pChkIdx ) )
    {
       pFmt->SetFmtAttr( SwFmtAnchor( eAnchorId = FLY_AT_PAGE ) );
    }
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
            if ( FLY_AT_PAGE == eAnchorId )
            {
                eAnchorId = rDrawObj.ISA( SdrUnoObj )
                                    ? FLY_AS_CHAR : FLY_AT_PARA;
                aAnch.SetType( eAnchorId );
            }
        }
        pFmt->SetFmtAttr( aAnch );
    }

    // bei als Zeichen gebundenen Draws das Attribut im Absatz setzen
    if ( FLY_AS_CHAR == eAnchorId )
    {
        xub_StrLen nStt = rRg.GetPoint()->nContent.GetIndex();
        SwFmtFlyCnt aFmt( pFmt );
        rRg.GetPoint()->nNode.GetNode().GetTxtNode()->InsertItem(
                aFmt, nStt, nStt );
    }

    SwDrawContact* pContact = new SwDrawContact( pFmt, &rDrawObj );

    // ggfs. Frames anlegen
    if( GetRootFrm() )
    {
        pFmt->MakeFrms();
        // #i42319# - follow-up of #i35635#
        // move object to visible layer
        // #i79391#
        if ( pContact->GetAnchorFrm() )
        {
            pContact->MoveObjToVisibleLayer( &rDrawObj );
        }
    }

    if( DoesUndo() )
    {
        ClearRedo();
        AppendUndo( new SwUndoInsLayFmt( pFmt,0,0 ));
    }

    SetModified();
    return pFmt;
}

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
        if (FLY_AT_PARA == nAnchorId)
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
                  (nPamEndContentIndex > nFlyContentIndex))) )
                ||
                       (((nPamStartIndex == nFlyIndex) &&
                      (pPaMStart->nContent.GetIndex() <= nFlyContentIndex)) &&
                     ((nPamEndIndex > nFlyIndex) ||
                     (nPamEndContentIndex > nFlyContentIndex )));
        }

    } while( !bOk && pPam != ( pTmp = (const SwPaM*)pTmp->GetNext() ));
    return bOk;
}

void SwDoc::GetAllFlyFmts( SwPosFlyFrms& rPosFlyFmts,
                           const SwPaM* pCmpRange, sal_Bool bDrawAlso ) const
{
    SwPosFlyFrm *pFPos = 0;
    SwFrmFmt *pFly;

    // erstmal alle Absatzgebundenen einsammeln
    for( sal_uInt16 n = 0; n < GetSpzFrmFmts()->Count(); ++n )
    {
        pFly = (*GetSpzFrmFmts())[ n ];
        bool bDrawFmt = bDrawAlso ? RES_DRAWFRMFMT == pFly->Which() : false;
        bool bFlyFmt = RES_FLYFRMFMT == pFly->Which();
        if( bFlyFmt || bDrawFmt )
        {
            const SwFmtAnchor& rAnchor = pFly->GetAnchor();
            SwPosition const*const pAPos = rAnchor.GetCntntAnchor();
            if (pAPos &&
                ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
                 (FLY_AT_FLY  == rAnchor.GetAnchorId()) ||
                 (FLY_AT_CHAR == rAnchor.GetAnchorId())))
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
            SwSortedObjs &rObjs = *pPage->GetSortedObjs();
            for( sal_uInt16 i = 0; i < rObjs.Count(); ++i)
            {
                SwAnchoredObject* pAnchoredObj = rObjs[i];
                if ( pAnchoredObj->ISA(SwFlyFrm) )
                    pFly = &(pAnchoredObj->GetFrmFmt());
                else if ( bDrawAlso )
                    pFly = &(pAnchoredObj->GetFrmFmt());
                else
                    continue;

                const SwFmtAnchor& rAnchor = pFly->GetAnchor();
                if ((FLY_AT_PARA != rAnchor.GetAnchorId()) &&
                    (FLY_AT_FLY  != rAnchor.GetAnchorId()) &&
                    (FLY_AT_CHAR != rAnchor.GetAnchorId()))
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
|*************************************************************************/

/* #i6447# changed behaviour if lcl_CpyAttr:

   If the old item set contains the item to set (no inheritance) copy the item
   into the new set.

   If the old item set contains the item by inheritance and the new set
   contains the item, too:
      If the two items differ copy the item from the old set to the new set.

   Otherwise the new set will not be changed.
*/
void lcl_CpyAttr( SfxItemSet &rNewSet, const SfxItemSet &rOldSet, sal_uInt16 nWhich )
{
    const SfxPoolItem *pOldItem = NULL, *pNewItem = NULL;

    rOldSet.GetItemState( nWhich, sal_False, &pOldItem);
    if (pOldItem != NULL)
        rNewSet.Put( *pOldItem );
    else
    {
        pOldItem = rOldSet.GetItem( nWhich, sal_True);
        if (pOldItem != NULL)
        {
            pNewItem = rNewSet.GetItem( nWhich, sal_True);
            if (pNewItem != NULL)
            {
                if (*pOldItem != *pNewItem)
                    rNewSet.Put( *pOldItem );
            }
            else {
                OSL_ENSURE(0, "What am I doing here?");
            }
        }
        else {
            OSL_ENSURE(0, "What am I doing here?");
        }
    }

}

SwFlyFrmFmt* SwDoc::InsertLabel( const SwLabelType eType, const String &rTxt, const String& rSeparator,
            const String& rNumberingSeparator,
            const sal_Bool bBefore, const sal_uInt16 nId, const ULONG nNdIdx,
            const String& rCharacterStyle,
            const sal_Bool bCpyBrd )
{
    sal_Bool bWasUndo = DoesUndo();
    SwUndoInsertLabel* pUndo = 0;
    if( bWasUndo )
    {
        ClearRedo();
        pUndo = new SwUndoInsertLabel( eType, rTxt, rSeparator, rNumberingSeparator,
                                       bBefore, nId, rCharacterStyle, bCpyBrd );
        DoUndo( sal_False );
    }

    sal_Bool bTable = sal_False;    //Um etwas Code zu sparen.

    //Erstmal das Feld bauen, weil ueber den Namen die TxtColl besorgt werden
    //muss
    OSL_ENSURE( nId == USHRT_MAX  || nId < GetFldTypes()->Count(), "FldType ueberindiziert." );
    SwFieldType *pType = nId != USHRT_MAX ? (*GetFldTypes())[nId] : NULL;
    OSL_ENSURE( !pType || pType->Which() == RES_SETEXPFLD, "Falsche Id fuer Label" );

    SwTxtFmtColl *pColl = NULL;
    if( pType )
    {
        for( sal_uInt16 i = pTxtFmtCollTbl->Count(); i; )
        {
            if( (*pTxtFmtCollTbl)[ --i ]->GetName() == pType->GetName() )
            {
                pColl = (*pTxtFmtCollTbl)[i];
                break;
            }
        }
        DBG_ASSERT( pColl, "no text collection found" );
    }

    if( !pColl )
        pColl = GetTxtCollFromPool( RES_POOLCOLL_LABEL );

    SwTxtNode *pNew = NULL;
    SwFlyFrmFmt* pNewFmt = NULL;

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
                OSL_ENSURE( pSttNd, "Kein StartNode in InsertLabel." );
                ULONG nNode;
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
                OSL_ENSURE( pOldFmt, "Format des Fly nicht gefunden." );
                pOldFmt->DelFrms();

                pNewFmt = MakeFlyFrmFmt( GetUniqueFrameName(),
                                    GetFrmFmtFromPool( RES_POOLFRM_FRAME ));

                /* #i6447#: Only the selected items are copied from the old
                   format. */
                SfxItemSet* pNewSet = pNewFmt->GetAttrSet().Clone( sal_True );


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
                lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_BACKGROUND );
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
                    pNewSet->Put( SvxBoxItem(RES_BOX) );
                    pNewSet->Put( SvxShadowItem(RES_SHADOW) );

                }

                //Anker immer uebertragen, ist sowieso ein hartes Attribut.
                pNewSet->Put( pOldFmt->GetAnchor() );

                //In der Hoehe soll der neue Varabel sein!
                SwFmtFrmSize aFrmSize( pOldFmt->GetFrmSize() );
                aFrmSize.SetHeightSizeType( ATT_MIN_SIZE );
                pNewSet->Put( aFrmSize );

                SwStartNode* pSttNd = GetNodes().MakeTextSection(
                            SwNodeIndex( GetNodes().GetEndOfAutotext() ),
                            SwFlyStartNode, pColl );
                pNewSet->Put( SwFmtCntnt( pSttNd ));

                pNewFmt->SetFmtAttr( *pNewSet );

                //Bei InCntnt's wird es spannend: Das TxtAttribut muss
                //vernichtet werden. Leider reisst dies neben den Frms auch
                //noch das Format mit in sein Grab. Um dass zu unterbinden
                //loesen wir vorher die Verbindung zwischen Attribut und Format.

                const SwFmtAnchor& rAnchor = pNewFmt->GetAnchor();
                if ( FLY_AS_CHAR == rAnchor.GetAnchorId() )
                {
                    const SwPosition *pPos = rAnchor.GetCntntAnchor();
                    SwTxtNode *pTxtNode = pPos->nNode.GetNode().GetTxtNode();
                    OSL_ENSURE( pTxtNode->HasHints(), "Missing FlyInCnt-Hint." );
                    const xub_StrLen nIdx = pPos->nContent.GetIndex();
                    SwTxtAttr * const pHnt =
                        pTxtNode->GetTxtAttrForCharAt(nIdx, RES_TXTATR_FLYCNT);

                    OSL_ENSURE( pHnt && pHnt->Which() == RES_TXTATR_FLYCNT,
                                "Missing FlyInCnt-Hint." );
                    OSL_ENSURE( pHnt && pHnt->GetFlyCnt().GetFrmFmt() == pOldFmt,
                                "Wrong TxtFlyCnt-Hint." );

                    const_cast<SwFmtFlyCnt&>(pHnt->GetFlyCnt()).SetFlyFmt(
                            pNewFmt );
                }


                //Der Alte soll keinen Umlauf haben, und er soll oben/mittig
                //ausgerichtet sein.
                //Ausserdem soll die Breite 100% betragen und bei Aenderungen
                //Die Hoehe mit anpassen.
                pNewSet->ClearItem();

                pNewSet->Put( SwFmtSurround( SURROUND_NONE ) );
                pNewSet->Put( SvxOpaqueItem( RES_OPAQUE, sal_True ) );
                pNewSet->Put( SwFmtVertOrient( text::VertOrientation::TOP ) );
                pNewSet->Put( SwFmtHoriOrient( text::HoriOrientation::CENTER ) );

                aFrmSize = pOldFmt->GetFrmSize();
                aFrmSize.SetWidthPercent( 100 );
                aFrmSize.SetHeightPercent( 255 );
                pNewSet->Put( aFrmSize );

                //Die Attribute setzen wir hart, weil sie sonst aus der Vorlage
                //kommen koenten und dann passt die Grossenberechnung nicht mehr.
                if( bCpyBrd )
                {
                    pNewSet->Put( SvxBoxItem(RES_BOX) );
                    pNewSet->Put( SvxShadowItem(RES_SHADOW) );
                }
                pNewSet->Put( SvxLRSpaceItem(RES_LR_SPACE) );
                pNewSet->Put( SvxULSpaceItem(RES_UL_SPACE) );

                //Der Alte ist absatzgebunden, und zwar am Absatz im neuen.
                SwFmtAnchor aAnch( FLY_AT_PARA );
                SwNodeIndex aAnchIdx( *pNewFmt->GetCntnt().GetCntntIdx(), 1 );
                pNew = aAnchIdx.GetNode().GetTxtNode();
                SwPosition aPos( aAnchIdx );
                aAnch.SetAnchor( &aPos );
                pNewSet->Put( aAnch );

                if( pUndo )
                    pUndo->SetFlys( *pOldFmt, *pNewSet, *pNewFmt );
                else
                    pOldFmt->SetFmtAttr( *pNewSet );

                delete pNewSet;

                //Nun nur noch die Flys erzeugen lassen. Das ueberlassen
                //wir vorhanden Methoden (insb. fuer InCntFlys etwas aufwendig).
                pNewFmt->MakeFrms();
            }
            break;

        default:
            OSL_ENSURE( !this, "Neuer LabelType?." );
    }
    OSL_ENSURE( pNew, "No Label inserted" );
    if( pNew )
    {
        //#i61007# order of captions
        sal_Bool bOrderNumberingFirst = SW_MOD()->GetModuleConfig()->IsCaptionOrderNumberingFirst();
        //String aufbereiten
        String aTxt;
        if( bOrderNumberingFirst )
        {
            aTxt = rNumberingSeparator;
        }
        if( pType)
        {
            aTxt += pType->GetName();
            if( !bOrderNumberingFirst )
                aTxt += ' ';
        }
        xub_StrLen nIdx = aTxt.Len();
        if ( rTxt.Len() > 0 )
        {
            aTxt += rSeparator;
        }
        xub_StrLen nSepIdx = aTxt.Len();
        aTxt += rTxt;

        //String einfuegen
        SwIndex aIdx( pNew, 0 );
        pNew->InsertText( aTxt, aIdx );

        //
        //Feld einfuegen
        if(pType)
        {
            SwSetExpField aFld( (SwSetExpFieldType*)pType, aEmptyStr, SVX_NUM_ARABIC);
            if( bOrderNumberingFirst )
                nIdx = 0;
            SwFmtFld aFmt( aFld );
            pNew->InsertItem( aFmt, nIdx, nIdx );
            if(rCharacterStyle.Len())
            {
                SwCharFmt* pCharFmt = FindCharFmtByName( rCharacterStyle );
                if( !pCharFmt )
                {
                    const USHORT nMyId = SwStyleNameMapper::GetPoolIdFromUIName(rCharacterStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
                    pCharFmt = GetCharFmtFromPool( nMyId );
                }
                if (pCharFmt)
                {
                    SwFmtCharFmt aCharFmt( pCharFmt );
                    pNew->InsertItem( aCharFmt, 0,
                        nSepIdx + 1, nsSetAttrMode::SETATTR_DONTEXPAND );
                }
            }
        }

        if ( bTable )
        {
            if ( bBefore )
            {
                if ( !pNew->GetSwAttrSet().GetKeep().GetValue()  )
                    pNew->SetAttr( SvxFmtKeepItem( sal_True, RES_KEEP ) );
            }
            else
            {
                SwTableNode *pNd = GetNodes()[nNdIdx]->GetStartNode()->GetTableNode();
                SwTable &rTbl = pNd->GetTable();
                if ( !rTbl.GetFrmFmt()->GetKeep().GetValue() )
                    rTbl.GetFrmFmt()->SetFmtAttr( SvxFmtKeepItem( sal_True, RES_KEEP ) );
                if ( pUndo )
                    pUndo->SetUndoKeep();
            }
        }
        SetModified();
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
|*************************************************************************/
SwFlyFrmFmt* SwDoc::InsertDrawLabel( const String &rTxt,
                                     const String& rSeparator,
                                     const String& rNumberSeparator,
                                     const sal_uInt16 nId,
                                     const String& rCharacterStyle,
                                     SdrObject& rSdrObj )
{

    SwDrawContact* pContact = (SwDrawContact*)GetUserCall( &rSdrObj );
    if ( !pContact )
        return 0;
    OSL_ENSURE( RES_DRAWFRMFMT == pContact->GetFmt()->Which(),
            "Kein DrawFrmFmt" );

    SwDrawFrmFmt* pOldFmt = (SwDrawFrmFmt *)pContact->GetFmt();
    if( !pOldFmt )
        return 0;

    sal_Bool bWasUndo = DoesUndo();
    sal_Bool bWasNoDrawUndo = IsNoDrawUndoObj();
    SwUndoInsertLabel* pUndo = 0;
    if( bWasUndo )
    {
        ClearRedo();
        pUndo = new SwUndoInsertLabel(
            LTYPE_DRAW, rTxt, rSeparator, rNumberSeparator, sal_False, nId, rCharacterStyle, sal_False );
        DoUndo( sal_False );
        SetNoDrawUndoObj( sal_True );
    }

    // Erstmal das Feld bauen, weil ueber den Namen die TxtColl besorgt
    // werden muss
    OSL_ENSURE( nId == USHRT_MAX  || nId < GetFldTypes()->Count(), "FldType overflow" );
    SwFieldType *pType = nId != USHRT_MAX ? (*GetFldTypes())[nId] : 0;
    OSL_ENSURE( !pType || pType->Which() == RES_SETEXPFLD, "Wrong label id" );

    SwTxtFmtColl *pColl = NULL;
    if( pType )
    {
        for( sal_uInt16 i = pTxtFmtCollTbl->Count(); i; )
        {
            if( (*pTxtFmtCollTbl)[ --i ]->GetName() == pType->GetName() )
            {
                pColl = (*pTxtFmtCollTbl)[i];
                break;
            }
        }
        DBG_ASSERT( pColl, "no text collection found" );
    }

    if( !pColl )
        pColl = GetTxtCollFromPool( RES_POOLCOLL_LABEL );

    SwTxtNode* pNew = NULL;
    SwFlyFrmFmt* pNewFmt = NULL;

    // Rahmen zerstoeren, neuen Rahmen einfuegen, entsprechenden
    // Node mit Feld in den neuen Rahmen, den alten Rahmen mit
    // dem Object (Grafik/Ole) absatzgebunden in den neuen Rahmen,
    // Frames erzeugen.

    // Keep layer ID of drawing object before removing
    // its frames.
    // Note: The layer ID is passed to the undo and have to be the correct value.
    //       Removing the frames of the drawing object changes its layer.
    const SdrLayerID nLayerId = rSdrObj.GetLayer();

    pOldFmt->DelFrms();

    //Bei InCntnt's wird es spannend: Das TxtAttribut muss
    //vernichtet werden. Leider reisst dies neben den Frms auch
    //noch das Format mit in sein Grab. Um dass zu unterbinden
    //loesen wir vorher die Verbindung zwischen Attribut und Format.
    SfxItemSet* pNewSet = pOldFmt->GetAttrSet().Clone( sal_False );

    // Ggf. Groesse und Position des Rahmens schuetzen
    if ( rSdrObj.IsMoveProtect() || rSdrObj.IsResizeProtect() )
    {
        SvxProtectItem aProtect(RES_PROTECT);
        aProtect.SetCntntProtect( sal_False );
        aProtect.SetPosProtect( rSdrObj.IsMoveProtect() );
        aProtect.SetSizeProtect( rSdrObj.IsResizeProtect() );
        pNewSet->Put( aProtect );
    }

    // Umlauf uebernehmen
    lcl_CpyAttr( *pNewSet, pOldFmt->GetAttrSet(), RES_SURROUND );

    // Den Rahmen ggf. in den Hintergrund schicken.
    // Consider 'invisible' hell layer.
    if ( GetHellId() != nLayerId &&
         GetInvisibleHellId() != nLayerId )
    {
        SvxOpaqueItem aOpaque( RES_OPAQUE );
        aOpaque.SetValue( sal_True );
        pNewSet->Put( aOpaque );
    }

    // Position uebernehmen
    // #i26791# - use directly the positioning attributes of
    // the drawing object.
    pNewSet->Put( pOldFmt->GetHoriOrient() );
    pNewSet->Put( pOldFmt->GetVertOrient() );

    pNewSet->Put( pOldFmt->GetAnchor() );

    //In der Hoehe soll der neue Varabel sein!
     Size aSz( rSdrObj.GetCurrentBoundRect().GetSize() );
    SwFmtFrmSize aFrmSize( ATT_MIN_SIZE, aSz.Width(), aSz.Height() );
    pNewSet->Put( aFrmSize );

    // Abstaende auf den neuen Rahmen uebertragen. Eine Umrandung
    // gibt es beu Zeichen-Objekten nicht, also muss sie geloescht
    // werden.
    // MA: Falsch sie wird nicht gesetzt, denn die aus der Vorlage
    // soll ruhig wirksam werden
    pNewSet->Put( pOldFmt->GetLRSpace() );
    pNewSet->Put( pOldFmt->GetULSpace() );

    SwStartNode* pSttNd =
        GetNodes().MakeTextSection( SwNodeIndex( GetNodes().GetEndOfAutotext() ),
                                    SwFlyStartNode, pColl );

    pNewFmt = MakeFlyFrmFmt( GetUniqueFrameName(),
                             GetFrmFmtFromPool( RES_POOLFRM_FRAME ) );

    // JP 28.10.99: Bug 69487 - set border and shadow to default if the
    //              template contains any.
    if( SFX_ITEM_SET == pNewFmt->GetAttrSet().GetItemState( RES_BOX, sal_True ))
        pNewSet->Put( *GetDfltAttr( RES_BOX ) );

    if( SFX_ITEM_SET == pNewFmt->GetAttrSet().GetItemState(RES_SHADOW,sal_True))
        pNewSet->Put( *GetDfltAttr( RES_SHADOW ) );

    pNewFmt->SetFmtAttr( SwFmtCntnt( pSttNd ));
    pNewFmt->SetFmtAttr( *pNewSet );

    const SwFmtAnchor& rAnchor = pNewFmt->GetAnchor();
    if ( FLY_AS_CHAR == rAnchor.GetAnchorId() )
    {
        const SwPosition *pPos = rAnchor.GetCntntAnchor();
        SwTxtNode *pTxtNode = pPos->nNode.GetNode().GetTxtNode();
        OSL_ENSURE( pTxtNode->HasHints(), "Missing FlyInCnt-Hint." );
        const xub_StrLen nIdx = pPos->nContent.GetIndex();
        SwTxtAttr * const pHnt =
            pTxtNode->GetTxtAttrForCharAt( nIdx, RES_TXTATR_FLYCNT );

#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( pHnt && pHnt->Which() == RES_TXTATR_FLYCNT,
                    "Missing FlyInCnt-Hint." );
        OSL_ENSURE( pHnt && ((SwFmtFlyCnt&)pHnt->GetFlyCnt()).
                    GetFrmFmt() == (SwFrmFmt*)pOldFmt,
                    "Wrong TxtFlyCnt-Hint." );
#endif
        const_cast<SwFmtFlyCnt&>(pHnt->GetFlyCnt()).SetFlyFmt( pNewFmt );
    }


    //Der Alte soll keinen Umlauf haben, und er soll oben/mittig
    //ausgerichtet sein.
    pNewSet->ClearItem();

    pNewSet->Put( SwFmtSurround( SURROUND_NONE ) );
    if( nLayerId == GetHellId() )
        rSdrObj.SetLayer( GetHeavenId() );
    // Consider drawing objects in 'invisible' hell layer
    else if( nLayerId == GetInvisibleHellId() )
        rSdrObj.SetLayer( GetInvisibleHeavenId() );
    pNewSet->Put( SvxLRSpaceItem( RES_LR_SPACE ) );
    pNewSet->Put( SvxULSpaceItem( RES_UL_SPACE ) );

    // #i26791# - set position of the drawing object, which is labeled.
    pNewSet->Put( SwFmtVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::FRAME ) );
    pNewSet->Put( SwFmtHoriOrient( 0, text::HoriOrientation::CENTER, text::RelOrientation::FRAME ) );

    //Der Alte ist absatzgebunden, und zwar am Absatz im neuen.
    SwFmtAnchor aAnch( FLY_AT_PARA );
    SwNodeIndex aAnchIdx( *pNewFmt->GetCntnt().GetCntntIdx(), 1 );
    pNew = aAnchIdx.GetNode().GetTxtNode();
    SwPosition aPos( aAnchIdx );
    aAnch.SetAnchor( &aPos );
    pNewSet->Put( aAnch );

    if( pUndo )
    {
        pUndo->SetFlys( *pOldFmt, *pNewSet, *pNewFmt );
        // #i26791# - position no longer needed
        pUndo->SetDrawObj( nLayerId );
    }
    else
        pOldFmt->SetFmtAttr( *pNewSet );

    delete pNewSet;

    //Nun nur noch die Flys erzeugen lassen. Das ueberlassen
    //wir vorhanden Methoden (insb. fuer InCntFlys etwas aufwendig).
    pNewFmt->MakeFrms();

    OSL_ENSURE( pNew, "No Label inserted" );

    if( pNew )
    {
        //#i61007# order of captions
        sal_Bool bOrderNumberingFirst = SW_MOD()->GetModuleConfig()->IsCaptionOrderNumberingFirst();

        // prepare string
        String aTxt;
        if( bOrderNumberingFirst )
        {
            aTxt = rNumberSeparator;
        }
        if ( pType )
        {
            aTxt += pType->GetName();
            if( !bOrderNumberingFirst )
                aTxt += ' ';
        }
        xub_StrLen nIdx = aTxt.Len();
        aTxt += rSeparator;
        xub_StrLen nSepIdx = aTxt.Len();
        aTxt += rTxt;

        // insert text
        SwIndex aIdx( pNew, 0 );
        pNew->InsertText( aTxt, aIdx );

        // insert field
        if ( pType )
        {
            SwSetExpField aFld( (SwSetExpFieldType*)pType, aEmptyStr, SVX_NUM_ARABIC );
            if( bOrderNumberingFirst )
                nIdx = 0;
            SwFmtFld aFmt( aFld );
            pNew->InsertItem( aFmt, nIdx, nIdx );
            if ( rCharacterStyle.Len() )
            {
                SwCharFmt* pCharFmt = FindCharFmtByName( rCharacterStyle );
                if ( !pCharFmt )
                {
                    const USHORT nMyId = SwStyleNameMapper::GetPoolIdFromUIName( rCharacterStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
                    pCharFmt = GetCharFmtFromPool( nMyId );
                }
                if ( pCharFmt )
                {
                    SwFmtCharFmt aCharFmt( pCharFmt );
                    pNew->InsertItem( aCharFmt, 0, nSepIdx + 1,
                            nsSetAttrMode::SETATTR_DONTEXPAND );
                }
            }
        }
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
|*  IDocumentTimerAccess-methods
|*
|*************************************************************************/
void SwDoc::StartIdling()
{
    mbStartIdleTimer = sal_True;
    if( !mIdleBlockCount )
        aIdleTimer.Start();
}

void SwDoc::StopIdling()
{
    mbStartIdleTimer = sal_False;
    aIdleTimer.Stop();
}

void SwDoc::BlockIdling()
{
    aIdleTimer.Stop();
    ++mIdleBlockCount;
}

void SwDoc::UnblockIdling()
{
    --mIdleBlockCount;
    if( !mIdleBlockCount && mbStartIdleTimer && !aIdleTimer.IsActive() )
        aIdleTimer.Start();
}

/*************************************************************************
|*
|*  SwDoc::DoIdleJobs()
|*
|*************************************************************************/
IMPL_LINK( SwDoc, DoIdleJobs, Timer *, pTimer )
{
#ifdef TIMELOG
    static ::rtl::Logfile* pModLogFile = 0;
    if( !pModLogFile )
        pModLogFile = new ::rtl::Logfile( "First DoIdleJobs" );
#endif

    if( GetRootFrm() && GetRootFrm()->GetCurrShell() &&
        !SfxProgress::GetActiveProgress( pDocShell ) )
    {
        ViewShell *pSh, *pStartSh;
        pSh = pStartSh = GetRootFrm()->GetCurrShell();
        do {
            if( pSh->ActionPend() )
            {
                if( pTimer )
                    pTimer->Start();
                return 0;
            }
            pSh = (ViewShell*)pSh->GetNext();
        } while( pSh != pStartSh );

        if (GetRootFrm()->IsNeedGrammarCheck())
        {
            BOOL bIsOnlineSpell = pSh->GetViewOptions()->IsOnlineSpell();

            sal_Bool bIsAutoGrammar = sal_False;
            SvtLinguConfig().GetProperty( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        UPN_IS_GRAMMAR_AUTO )) ) >>= bIsAutoGrammar;

            if (bIsOnlineSpell && bIsAutoGrammar)
                StartGrammarChecking( *this );
        }

        sal_uInt16 nFldUpdFlag;
        if( GetRootFrm()->IsIdleFormat() )
            GetRootFrm()->GetCurrShell()->LayoutIdle();
        else if( ( AUTOUPD_FIELD_ONLY ==
                 ( nFldUpdFlag = static_cast<sal_uInt16>(getFieldUpdateFlags(true)) )
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

            // no jump on update of fields #i85168#
            const sal_Bool bOldLockView = pStartSh->IsViewLocked();
            pStartSh->LockView( sal_True );

            GetSysFldType( RES_CHAPTERFLD )->Modify( 0, 0 );    // KapitelFld
            UpdateExpFlds( 0, sal_False );      // Expression-Felder Updaten
            UpdateTblFlds(NULL);                // Tabellen
            UpdateRefFlds(NULL);                // Referenzen

            GetRootFrm()->EndAllAction();

            pStartSh->LockView( bOldLockView );

            GetUpdtFlds().SetInUpdateFlds( sal_False );
            GetUpdtFlds().SetFieldsDirty( sal_False );
        }
    }
#ifdef TIMELOG
    if( pModLogFile && 1 != (long)pModLogFile )
        delete pModLogFile, ((long&)pModLogFile) = 1;
#endif
    if( pTimer )
        pTimer->Start();
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

static String lcl_GetUniqueFlyName( const SwDoc* pDoc, sal_uInt16 nDefStrId )
{
    ResId aId( nDefStrId, *pSwResMgr );
    String aName( aId );
    xub_StrLen nNmLen = aName.Len();

    const SwSpzFrmFmts& rFmts = *pDoc->GetSpzFrmFmts();

    sal_uInt16 nNum, nTmp, nFlagSize = ( rFmts.Count() / 8 ) +2;
    sal_uInt8* pSetFlags = new sal_uInt8[ nFlagSize ];
    sal_uInt16 n;

    memset( pSetFlags, 0, nFlagSize );

    for( n = 0; n < rFmts.Count(); ++n )
    {
        const SwFrmFmt* pFlyFmt = rFmts[ n ];
        if( RES_FLYFRMFMT == pFlyFmt->Which() &&
            pFlyFmt->GetName().Match( aName ) == nNmLen )
        {
            // Nummer bestimmen und das Flag setzen
            nNum = static_cast< sal_uInt16 >( pFlyFmt->GetName().Copy( nNmLen ).ToInt32() );
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

    delete [] pSetFlags;
    return aName += String::CreateFromInt32( ++nNum );
}

String SwDoc::GetUniqueGrfName() const
{
    return lcl_GetUniqueFlyName( this, STR_GRAPHIC_DEFNAME );
}

String SwDoc::GetUniqueOLEName() const
{
    return lcl_GetUniqueFlyName( this, STR_OBJECT_DEFNAME );
}

String SwDoc::GetUniqueFrameName() const
{
    return lcl_GetUniqueFlyName( this, STR_FRAME_DEFNAME );
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
        sName = lcl_GetUniqueFlyName( this, nTyp );
    }
    rFmt.SetName( sName, sal_True );
    SetModified();
}

void SwDoc::SetAllUniqueFlyNames()
{
    sal_uInt16 n, nFlyNum = 0, nGrfNum = 0, nOLENum = 0;

    ResId nFrmId( STR_FRAME_DEFNAME, *pSwResMgr ),
          nGrfId( STR_GRAPHIC_DEFNAME, *pSwResMgr ),
          nOLEId( STR_OBJECT_DEFNAME, *pSwResMgr );
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

                if ( pNum && *pNum < ( nLen = static_cast< xub_StrLen >( rNm.Copy( nLen ).ToInt32() ) ) )
                    *pNum = nLen;
            }
            else
                // das wollen wir nachher setzen
                aArr.Insert( pFlyFmt, aArr.Count() );

        }
        if( bLoadedFlag )
        {
            const SwFmtAnchor& rAnchor = pFlyFmt->GetAnchor();
            if (((FLY_AT_PAGE == rAnchor.GetAnchorId()) &&
                 rAnchor.GetCntntAnchor()) ||
                // oder werden DrawObjecte rel. zu irgendetwas ausgerichtet?
                ( RES_DRAWFRMFMT == pFlyFmt->Which() && (
                    SFX_ITEM_SET == pFlyFmt->GetItemState(
                                        RES_VERT_ORIENT )||
                    SFX_ITEM_SET == pFlyFmt->GetItemState(
                                        RES_HORI_ORIENT ))) )
            {
                bLoadedFlag = sal_False;
            }
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
        // --> FME 2005-08-02 #i52775# Chapter footnotes did not
        // get updated correctly. Calling UpdateAllFtn() instead of
        // UpdateFtn() solves this problem, but I do not dare to
        // call UpdateAllFtn() in all cases: Safety first.
        if ( FTNNUM_CHAPTER == GetFtnInfo().eNum )
        {
            GetFtnIdxs().UpdateAllFtn();
        }
        // <--
        else
        {
            SwNodeIndex aTmp( GetNodes() );
            GetFtnIdxs().UpdateFtn( aTmp );
        }
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
        const SwFrm *pFrm = pNd->GetCntntNode()->GetFrm();
        if( pFrm )
        {
            const SwFrm *pUp = pFrm->GetUpper();
            while ( pUp && !pUp->IsHeaderFrm() && !pUp->IsFooterFrm() )
            {
                if ( pUp->IsFlyFrm() )
                    pUp = ((SwFlyFrm*)pUp)->GetAnchorFrm();
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
        sal_uInt16 n;
        for( n = 0; n < GetSpzFrmFmts()->Count(); ++n )
        {
            const SwFrmFmt* pFmt = (*GetSpzFrmFmts())[ n ];
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if( pIdx && pFlyNd == &pIdx->GetNode() )
            {
                const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
                if ((FLY_AT_PAGE == rAnchor.GetAnchorId()) ||
                    !rAnchor.GetCntntAnchor() )
                {
                    return sal_False;
                }

                pNd = &rAnchor.GetCntntAnchor()->nNode.GetNode();
                pFlyNd = pNd->FindFlyStartNode();
                break;
            }
        }
        if( n >= GetSpzFrmFmts()->Count() )
        {
            OSL_ENSURE( mbInReading, "Fly-Section aber kein Format gefunden" );
            return sal_False;
        }
    }

    return 0 != pNd->FindHeaderStartNode() ||
            0 != pNd->FindFooterStartNode();
}

short SwDoc::GetTextDirection( const SwPosition& rPos,
                               const Point* pPt ) const
{
    short nRet = -1;

    SwCntntNode *pNd = rPos.nNode.GetNode().GetCntntNode();

    // #i42921# - use new method <SwCntntNode::GetTextDirection(..)>
    if ( pNd )
    {
        nRet = pNd->GetTextDirection( rPos, pPt );
    }
    if ( nRet == -1 )
    {
        const SvxFrameDirectionItem* pItem = 0;
        if( pNd )
        {
            // in a flyframe? Then look at that for the correct attribute
            const SwFrmFmt* pFlyFmt = pNd->GetFlyFmt();
            while( pFlyFmt )
            {
                pItem = &pFlyFmt->GetFrmDir();
                if( FRMDIR_ENVIRONMENT == pItem->GetValue() )
                {
                    pItem = 0;
                    const SwFmtAnchor* pAnchor = &pFlyFmt->GetAnchor();
                    if ((FLY_AT_PAGE != pAnchor->GetAnchorId()) &&
                        pAnchor->GetCntntAnchor())
                    {
                        pFlyFmt = pAnchor->GetCntntAnchor()->nNode.
                                            GetNode().GetFlyFmt();
                    }
                    else
                        pFlyFmt = 0;
                }
                else
                    pFlyFmt = 0;
            }

            if( !pItem )
            {
                const SwPageDesc* pPgDsc = pNd->FindPageDesc( FALSE );
                if( pPgDsc )
                    pItem = &pPgDsc->GetMaster().GetFrmDir();
            }
        }
        if( !pItem )
            pItem = (SvxFrameDirectionItem*)&GetAttrPool().GetDefaultItem(
                                                            RES_FRAMEDIR );
        nRet = pItem->GetValue();
    }
    return nRet;
}

sal_Bool SwDoc::IsInVerticalText( const SwPosition& rPos, const Point* pPt ) const
{
    const short nDir = GetTextDirection( rPos, pPt );
    return FRMDIR_VERT_TOP_RIGHT == nDir || FRMDIR_VERT_TOP_LEFT == nDir;
}

const SwRootFrm* SwDoc::GetRootFrm() const { return pLayout; }
SwRootFrm* SwDoc::GetRootFrm() { return pLayout; }
void SwDoc::SetRootFrm( SwRootFrm* pNew ) { pLayout = pNew; }
SwLayouter* SwDoc::GetLayouter() { return pLayouter; }
const SwLayouter* SwDoc::GetLayouter() const { return pLayouter; }
void SwDoc::SetLayouter( SwLayouter* pNew ) { pLayouter = pNew; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
