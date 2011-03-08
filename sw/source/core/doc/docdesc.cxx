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
#include <hintids.hxx>
#include <vcl/virdev.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/paperinf.hxx>
#include "editeng/frmdiritem.hxx"
#include <tools/urlobj.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <unotools/localedatawrapper.hxx>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <fmtfsize.hxx>
#include <fmthdft.hxx>
#include <fmtcntnt.hxx>
#include <fmtpdsc.hxx>
#include <ftninfo.hxx>
#include <fesh.hxx>
#include <ndole.hxx>
#include <mdiexp.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <pagefrm.hxx>  //Fuer DelPageDesc
#include <rootfrm.hxx>  //Fuer DelPageDesc
#include <ndtxt.hxx>
#include <frmtool.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include <docsh.hxx>
#include <ndindex.hxx>
#include <ftnidx.hxx>
#include <fmtftn.hxx>
#include <txtftn.hxx>
#include <fntcache.hxx>
#include <viewsh.hxx>
#include <viewopt.hxx>
#include <fldbas.hxx>
#include <swwait.hxx>
#include <GetMetricVal.hxx>
#include <unotools/syslocale.hxx>
#include <statstr.hrc>

#include <SwUndoPageDesc.hxx>

#include <tgrditem.hxx>

using namespace com::sun::star;

static void lcl_DefaultPageFmt( sal_uInt16 nPoolFmtId,
                                SwFrmFmt &rFmt1,
                                SwFrmFmt &rFmt2 )
{
    // --> FME 2005-01-21 #i41075# Printer on demand
    // This function does not require a printer anymore.
    // The default page size is obtained from the application
    //locale
    // <--

    SwFmtFrmSize aFrmSize( ATT_FIX_SIZE );
    const Size aPhysSize = SvxPaperInfo::GetDefaultPaperSize();
    aFrmSize.SetSize( aPhysSize );

    //Auf Default-Raender vorbereiten.
    //Raender haben eine defaultmaessige Mindestgroesse.
    //wenn der Drucker einen groesseren Rand vorgibt, so
    //ist mir dass auch recht.
    // The HTML page desc had A4 as page size always.
    // This has been changed to take the page size from the printer.
    // Unfortunately, the margins of the HTML page desc are smaller than
    // the margins used here in general, so one extra case is required.
    // In the long term, this needs to be changed to always keep the
    // margins from the page desc.
    sal_Int32 nMinTop, nMinBottom, nMinLeft, nMinRight;
    if( RES_POOLPAGE_HTML == nPoolFmtId )
    {
        nMinRight = nMinTop = nMinBottom = GetMetricVal( CM_1 );
        nMinLeft = nMinRight * 2;
    }
    else if( MEASURE_METRIC == SvtSysLocale().GetLocaleData().getMeasurementSystemEnum() )
    {
        nMinTop = nMinBottom = nMinLeft = nMinRight = 1134; //2 Zentimeter
    }
    else
    {
        nMinTop = nMinBottom = 1440;    //al la WW: 1Inch
        nMinLeft = nMinRight = 1800;    //          1,25 Inch
    }

    //Raender einstellen.
    SvxLRSpaceItem aLR( RES_LR_SPACE );
    SvxULSpaceItem aUL( RES_UL_SPACE );

    aUL.SetUpper( (USHORT)nMinTop );
    aUL.SetLower( (USHORT)nMinBottom );
    aLR.SetRight( nMinRight );
    aLR.SetLeft( nMinLeft );

    rFmt1.SetFmtAttr( aFrmSize );
    rFmt1.SetFmtAttr( aLR );
    rFmt1.SetFmtAttr( aUL );

    rFmt2.SetFmtAttr( aFrmSize );
    rFmt2.SetFmtAttr( aLR );
    rFmt2.SetFmtAttr( aUL );
}

/*************************************************************************
|*
|*  SwDoc::ChgPageDesc()
|*
|*************************************************************************/

void lcl_DescSetAttr( const SwFrmFmt &rSource, SwFrmFmt &rDest,
                         const BOOL bPage = TRUE )
{
/////////////// !!!!!!!!!!!!!!!!
//JP 03.03.99:
// eigentlich sollte hier das Intersect von ItemSet benutzt werden, aber das
// funktioniert nicht richtig, wenn man unterschiedliche WhichRanges hat.
/////////////// !!!!!!!!!!!!!!!!
    //Die interressanten Attribute uebernehmen.
    USHORT const aIdArr[] = { RES_FRM_SIZE, RES_UL_SPACE,
                                        RES_BACKGROUND, RES_SHADOW,
                                        RES_COL, RES_COL,
                                        RES_FRAMEDIR, RES_FRAMEDIR,
                                        RES_TEXTGRID, RES_TEXTGRID,
                                        // --> FME 2005-04-18 #i45539#
                                        RES_HEADER_FOOTER_EAT_SPACING,
                                        RES_HEADER_FOOTER_EAT_SPACING,
                                        // <--
                                        RES_UNKNOWNATR_CONTAINER,
                                        RES_UNKNOWNATR_CONTAINER,
                                        0 };

    const SfxPoolItem* pItem;
    for( USHORT n = 0; aIdArr[ n ]; n += 2 )
    {
        for( USHORT nId = aIdArr[ n ]; nId <= aIdArr[ n+1]; ++nId )
        {
            // --> FME 2005-04-18 #i45539#
            // bPage == true:
            // All in aIdArr except from RES_HEADER_FOOTER_EAT_SPACING
            // bPage == false:
            // All in aIdArr except from RES_COL and RES_PAPER_BIN:
            // <--
            if( (  bPage && RES_HEADER_FOOTER_EAT_SPACING != nId ) ||
                ( !bPage && RES_COL != nId && RES_PAPER_BIN != nId ))
            {
                if( SFX_ITEM_SET == rSource.GetItemState( nId, FALSE, &pItem ))
                    rDest.SetFmtAttr( *pItem );
                else
                    rDest.ResetFmtAttr( nId );
            }
        }
    }

    // auch Pool-, Hilfe-Id's uebertragen
    rDest.SetPoolFmtId( rSource.GetPoolFmtId() );
    rDest.SetPoolHelpId( rSource.GetPoolHelpId() );
    rDest.SetPoolHlpFileId( rSource.GetPoolHlpFileId() );
}


void SwDoc::ChgPageDesc( USHORT i, const SwPageDesc &rChged )
{
    OSL_ENSURE( i < aPageDescs.Count(), "PageDescs ueberindiziert." );

    SwPageDesc *pDesc = aPageDescs[i];

    BOOL bDoesUndo = DoesUndo();
    if (DoesUndo())
    {
        AppendUndo(new SwUndoPageDesc(*pDesc, rChged, this));
        DoUndo(FALSE);
    }

    //Als erstes wird ggf. gespiegelt.
    if ( rChged.GetUseOn() == nsUseOnPage::PD_MIRROR )
        ((SwPageDesc&)rChged).Mirror();
    else
        //sonst Werte aus Master nach Left uebertragen.
        ::lcl_DescSetAttr( ((SwPageDesc&)rChged).GetMaster(),
                       ((SwPageDesc&)rChged).GetLeft() );

    //NumType uebernehmen.
    if( rChged.GetNumType().GetNumberingType() != pDesc->GetNumType().GetNumberingType() )
    {
        pDesc->SetNumType( rChged.GetNumType() );
        // JP 30.03.99: Bug 64121 - den Seitennummernfeldern bescheid sagen,
        //      das sich das Num-Format geaendert hat
        GetSysFldType( RES_PAGENUMBERFLD )->UpdateFlds();
        GetSysFldType( RES_REFPAGEGETFLD )->UpdateFlds();

        // Wenn sich die Numerierungsart geaendert hat, koennte es QuoVadis/
        // ErgoSum-Texte geben, die sich auf eine geaenderte Seite beziehen,
        // deshalb werden die Fussnoten invalidiert
        SwFtnIdxs& rFtnIdxs = GetFtnIdxs();
        for( USHORT nPos = 0; nPos < rFtnIdxs.Count(); ++nPos )
        {
            SwTxtFtn *pTxtFtn = rFtnIdxs[ nPos ];
            const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
            pTxtFtn->SetNumber( rFtn.GetNumber(), &rFtn.GetNumStr());
        }
    }

    //Orientierung uebernehmen
    pDesc->SetLandscape( rChged.GetLandscape() );

    // #i46909# no undo if header or footer changed
    bool bHeaderFooterChanged = false;

    //Header abgleichen.
    const SwFmtHeader &rHead = rChged.GetMaster().GetHeader();
    if( bDoesUndo )
    {
        // #i46909# no undo if header or footer changed
        // hat sich an den Nodes etwas veraendert ?
        const SwFmtHeader &rOldHead = pDesc->GetMaster().GetHeader();
        bHeaderFooterChanged |=
            ( rHead.IsActive() != rOldHead.IsActive() ||
              rChged.IsHeaderShared() != pDesc->IsHeaderShared() );
    }
    pDesc->GetMaster().SetFmtAttr( rHead );
    if ( rChged.IsHeaderShared() || !rHead.IsActive() )
    {
        //Left teilt sich den Header mit dem Master.
        pDesc->GetLeft().SetFmtAttr( pDesc->GetMaster().GetHeader() );
    }
    else if ( rHead.IsActive() )
    {   //Left bekommt einen eigenen Header verpasst wenn das Format nicht
        //bereits einen hat.
        //Wenn er bereits einen hat und dieser auf die gleiche Section
        //wie der Rechte zeigt, so muss er einen eigenen bekommen. Der
        //Inhalt wird sinnigerweise kopiert.
        const SwFmtHeader &rLeftHead = pDesc->GetLeft().GetHeader();
        if ( !rLeftHead.IsActive() )
        {
            SwFmtHeader aHead( MakeLayoutFmt( RND_STD_HEADERL, 0 ) );
            pDesc->GetLeft().SetFmtAttr( aHead );
            //Weitere Attribute (Raender, Umrandung...) uebernehmen.
            ::lcl_DescSetAttr( *rHead.GetHeaderFmt(), *aHead.GetHeaderFmt(), FALSE);
        }
        else
        {
            const SwFrmFmt *pRight = rHead.GetHeaderFmt();
            const SwFmtCntnt &aRCnt = pRight->GetCntnt();
            const SwFmtCntnt &aLCnt = rLeftHead.GetHeaderFmt()->GetCntnt();
            if( !aLCnt.GetCntntIdx() )
                pDesc->GetLeft().SetFmtAttr( rChged.GetLeft().GetHeader() );
            else if( (*aRCnt.GetCntntIdx()) == (*aLCnt.GetCntntIdx()) )
            {
                SwFrmFmt *pFmt = new SwFrmFmt( GetAttrPool(), "Header",
                                                GetDfltFrmFmt() );
                ::lcl_DescSetAttr( *pRight, *pFmt, FALSE );
                //Der Bereich auf den das rechte Kopfattribut zeigt wird
                //kopiert und der Index auf den StartNode in das linke
                //Kopfattribut gehaengt.
                SwNodeIndex aTmp( GetNodes().GetEndOfAutotext() );
                SwStartNode* pSttNd = GetNodes().MakeEmptySection( aTmp, SwHeaderStartNode );
                SwNodeRange aRange( aRCnt.GetCntntIdx()->GetNode(), 0,
                            *aRCnt.GetCntntIdx()->GetNode().EndOfSectionNode() );
                aTmp = *pSttNd->EndOfSectionNode();
                GetNodes()._Copy( aRange, aTmp, FALSE );

                pFmt->SetFmtAttr( SwFmtCntnt( pSttNd ) );
                pDesc->GetLeft().SetFmtAttr( SwFmtHeader( pFmt ) );
            }
            else
                ::lcl_DescSetAttr( *pRight,
                               *(SwFrmFmt*)rLeftHead.GetHeaderFmt(), FALSE );

        }
    }
    pDesc->ChgHeaderShare( rChged.IsHeaderShared() );

    //Footer abgleichen.
    const SwFmtFooter &rFoot = rChged.GetMaster().GetFooter();
    if( bDoesUndo )
    {
        // #i46909# no undo if header or footer changed
        // hat sich an den Nodes etwas veraendert ?
        const SwFmtFooter &rOldFoot = pDesc->GetMaster().GetFooter();
        bHeaderFooterChanged |=
            ( rFoot.IsActive() != rOldFoot.IsActive() ||
              rChged.IsFooterShared() != pDesc->IsFooterShared() );
    }
    pDesc->GetMaster().SetFmtAttr( rFoot );
    if ( rChged.IsFooterShared() || !rFoot.IsActive() )
        //Left teilt sich den Header mit dem Master.
        pDesc->GetLeft().SetFmtAttr( pDesc->GetMaster().GetFooter() );
    else if ( rFoot.IsActive() )
    {   //Left bekommt einen eigenen Footer verpasst wenn das Format nicht
        //bereits einen hat.
        //Wenn er bereits einen hat und dieser auf die gleiche Section
        //wie der Rechte zeigt, so muss er einen eigenen bekommen. Der
        //Inhalt wird sinnigerweise kopiert.
        const SwFmtFooter &rLeftFoot = pDesc->GetLeft().GetFooter();
        if ( !rLeftFoot.IsActive() )
        {
            SwFmtFooter aFoot( MakeLayoutFmt( RND_STD_FOOTER, 0 ) );
            pDesc->GetLeft().SetFmtAttr( aFoot );
            //Weitere Attribute (Raender, Umrandung...) uebernehmen.
            ::lcl_DescSetAttr( *rFoot.GetFooterFmt(), *aFoot.GetFooterFmt(), FALSE);
        }
        else
        {
            const SwFrmFmt *pRight = rFoot.GetFooterFmt();
            const SwFmtCntnt &aRCnt = pRight->GetCntnt();
            const SwFmtCntnt &aLCnt = rLeftFoot.GetFooterFmt()->GetCntnt();
            if( !aLCnt.GetCntntIdx() )
                pDesc->GetLeft().SetFmtAttr( rChged.GetLeft().GetFooter() );
            else if( (*aRCnt.GetCntntIdx()) == (*aLCnt.GetCntntIdx()) )
            {
                SwFrmFmt *pFmt = new SwFrmFmt( GetAttrPool(), "Footer",
                                                GetDfltFrmFmt() );
                ::lcl_DescSetAttr( *pRight, *pFmt, FALSE );
                //Der Bereich auf den das rechte Kopfattribut zeigt wird
                //kopiert und der Index auf den StartNode in das linke
                //Kopfattribut gehaengt.
                SwNodeIndex aTmp( GetNodes().GetEndOfAutotext() );
                SwStartNode* pSttNd = GetNodes().MakeEmptySection( aTmp, SwFooterStartNode );
                SwNodeRange aRange( aRCnt.GetCntntIdx()->GetNode(), 0,
                            *aRCnt.GetCntntIdx()->GetNode().EndOfSectionNode() );
                aTmp = *pSttNd->EndOfSectionNode();
                GetNodes()._Copy( aRange, aTmp, FALSE );

                pFmt->SetFmtAttr( SwFmtCntnt( pSttNd ) );
                pDesc->GetLeft().SetFmtAttr( SwFmtFooter( pFmt ) );
            }
            else
                ::lcl_DescSetAttr( *pRight,
                               *(SwFrmFmt*)rLeftFoot.GetFooterFmt(), FALSE );
        }
    }
    pDesc->ChgFooterShare( rChged.IsFooterShared() );

    if ( pDesc->GetName() != rChged.GetName() )
        pDesc->SetName( rChged.GetName() );

    // Dadurch wird ein RegisterChange ausgeloest, wenn notwendig
    pDesc->SetRegisterFmtColl( rChged.GetRegisterFmtColl() );

    //Wenn sich das UseOn oder der Follow aendern muessen die
    //Absaetze das erfahren.
    BOOL bUseOn  = FALSE;
    BOOL bFollow = FALSE;
    if ( pDesc->GetUseOn() != rChged.GetUseOn() )
    {   pDesc->SetUseOn( rChged.GetUseOn() );
        bUseOn = TRUE;
    }
    if ( pDesc->GetFollow() != rChged.GetFollow() )
    {   if ( rChged.GetFollow() == &rChged )
        {   if ( pDesc->GetFollow() != pDesc )
            {   pDesc->SetFollow( pDesc );
                bFollow = TRUE;
            }
        }
        else
        {   pDesc->SetFollow( rChged.pFollow );
            bFollow = TRUE;
        }
    }

    if ( (bUseOn || bFollow) && GetRootFrm() )
        //Layot benachrichtigen!
        GetRootFrm()->CheckPageDescs( (SwPageFrm*)GetRootFrm()->Lower() );

    //Jetzt noch die Seiten-Attribute uebernehmen.
    ::lcl_DescSetAttr( rChged.GetMaster(), pDesc->GetMaster() );
    ::lcl_DescSetAttr( rChged.GetLeft(), pDesc->GetLeft() );

    //Wenn sich FussnotenInfo veraendert, so werden die Seiten
    //angetriggert.
    if( !(pDesc->GetFtnInfo() == rChged.GetFtnInfo()) )
    {
        pDesc->SetFtnInfo( rChged.GetFtnInfo() );
        SwMsgPoolItem  aInfo( RES_PAGEDESC_FTNINFO );
        {
            SwClientIter aIter( pDesc->GetMaster() );
            for( SwClient* pLast = aIter.First(TYPE(SwFrm)); pLast;
                    pLast = aIter.Next() )
                pLast->Modify( &aInfo, 0 );
        }
        {
            SwClientIter aIter( pDesc->GetLeft() );
            for( SwClient* pLast = aIter.First(TYPE(SwFrm)); pLast;
                    pLast = aIter.Next() )
                pLast->Modify( &aInfo, 0 );
        }
    }
    SetModified();

    DoUndo(bDoesUndo);

    // #i46909# no undo if header or footer changed
    if( bHeaderFooterChanged )
    {
        ClearRedo();
        DelAllUndoObj();
    }
}

/*************************************************************************
|*
|*  SwDoc::DelPageDesc()
|*
|*  Beschreibung        Alle Descriptoren, deren Follow auf den zu loeschenden
|*      zeigen muessen angepasst werden.
|*
|*************************************************************************/

void lcl_RemoveFrms( SwFrmFmt& rFmt, BOOL& rbFtnsRemoved )
{
    SwClientIter aIter( rFmt );
    SwFrm *pFrm;
    for( pFrm = (SwFrm*)aIter.First(TYPE(SwFrm)); pFrm;
            pFrm = (SwFrm*)aIter.Next() )
        if ( !rbFtnsRemoved && pFrm->IsPageFrm() &&
                ((SwPageFrm*)pFrm)->IsFtnPage() )
        {
            rFmt.getIDocumentLayoutAccess()->GetRootFrm()->RemoveFtns( 0, FALSE, TRUE );
            rbFtnsRemoved = TRUE;
        }
        else
        {
            pFrm->Cut();
            delete pFrm;
        }
}

// #i7983#
void SwDoc::PreDelPageDesc(SwPageDesc * pDel)
{
    if (0 == pDel)
        return;

    SwFmtPageDesc aDfltDesc( aPageDescs[0] );
    SwClientIter aIter( *pDel );
    SwClient* pLast;
    while( 0 != ( pLast = aIter.GoRoot() ))
    {
        if( pLast->ISA( SwFmtPageDesc ) )
        {
            const SwModify* pMod = ((SwFmtPageDesc*)pLast)->GetDefinedIn();
            if ( pMod )
            {
                if( pMod->ISA( SwCntntNode ) )
                    ((SwCntntNode*)pMod)->SetAttr( aDfltDesc );
                else if( pMod->ISA( SwFmt ))
                    ((SwFmt*)pMod)->SetFmtAttr( aDfltDesc );
                else
                {
                    OSL_ENSURE( !this, "was ist das fuer ein Mofify-Obj?" );
                    aPageDescs[0]->Add( pLast );
                }
            }
            else    //Es kann noch eine Undo-Kopie existieren
                aPageDescs[0]->Add( pLast );
        }

        BOOL bFtnInf = FALSE;
        if ( TRUE == (bFtnInf = pLast == pFtnInfo->GetPageDescDep()) ||
             pLast == pEndNoteInfo->GetPageDescDep() )
        {
            aPageDescs[0]->Add( pLast );
            if ( GetRootFrm() )
                GetRootFrm()->CheckFtnPageDescs( !bFtnInf );
        }
    }

    for ( USHORT j = 0; j < aPageDescs.Count(); ++j )
    {
        if ( aPageDescs[j]->GetFollow() == pDel )
        {
            aPageDescs[j]->SetFollow( 0 );
            //Clients des PageDesc sind die Attribute, denen sagen wir bescheid.
            //die Attribute wiederum reichen die Meldung an die Absaetze weiter.

            //Layot benachrichtigen!
            if( GetRootFrm() )  // ist nicht immer vorhanden!! (Orginizer)
                GetRootFrm()->CheckPageDescs( (SwPageFrm*)GetRootFrm()->Lower() );
        }
    }

    if( GetRootFrm() )        // ist nicht immer vorhanden!! (Orginizer)
    {
        //Wenn jetzt noch irgendwelche Seiten auf die FrmFmt'e (Master und Left)
        //Zeigen (z.B. irgendwelche Fussnotenseiten), so muessen die Seiten
        //vernichtet werden.

        // Wenn wir auf Endnotenseiten stossen, schmeissen wir alle Fussnoten weg,
        // anders kann die Reihenfolge der Seiten (FollowsPageDescs usw.)
        // nicht garantiert werden.
        BOOL bFtnsRemoved = FALSE;

        ::lcl_RemoveFrms( pDel->GetMaster(), bFtnsRemoved );
        ::lcl_RemoveFrms( pDel->GetLeft(), bFtnsRemoved );
    }
}

void SwDoc::BroadcastStyleOperation(String rName, SfxStyleFamily eFamily,
                                    USHORT nOp)
{
    if (pDocShell)
    {
        SfxStyleSheetBasePool * pPool = pDocShell->GetStyleSheetPool();

        if (pPool)
        {
            pPool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
            SfxStyleSheetBase * pBase = pPool->Find(rName);

            if (pBase != NULL)
                pPool->Broadcast(SfxStyleSheetHint( nOp, *pBase ));
        }
    }
}

void SwDoc::DelPageDesc( USHORT i, BOOL bBroadcast )
{
    OSL_ENSURE( i < aPageDescs.Count(), "PageDescs ueberindiziert." );
    OSL_ENSURE( i != 0, "Default Pagedesc loeschen is nicht." );
    if ( i == 0 )
        return;

    SwPageDesc *pDel = aPageDescs[i];

    if (bBroadcast)
        BroadcastStyleOperation(pDel->GetName(), SFX_STYLE_FAMILY_PAGE,
                                SFX_STYLESHEET_ERASED);

    if (DoesUndo())
    {
        AppendUndo(new SwUndoPageDescDelete(*pDel, this));
    }

    PreDelPageDesc(pDel); // #i7983#

    aPageDescs.Remove( i );
    delete pDel;
    SetModified();
}



/*************************************************************************
|*
|*  SwDoc::MakePageDesc()
|*
|*************************************************************************/

USHORT SwDoc::MakePageDesc( const String &rName, const SwPageDesc *pCpy,
                            BOOL bRegardLanguage, BOOL bBroadcast)
{
    SwPageDesc *pNew;
    if( pCpy )
    {
        pNew = new SwPageDesc( *pCpy );
        pNew->SetName( rName );
        if( rName != pCpy->GetName() )
        {
            pNew->SetPoolFmtId( USHRT_MAX );
            pNew->SetPoolHelpId( USHRT_MAX );
            pNew->SetPoolHlpFileId( UCHAR_MAX );
        }
    }
    else
    {
        pNew = new SwPageDesc( rName, GetDfltFrmFmt(), this );
        //Default-Seitenformat einstellen.
        lcl_DefaultPageFmt( USHRT_MAX, pNew->GetMaster(), pNew->GetLeft() );

        SvxFrameDirection aFrameDirection = bRegardLanguage ?
            GetDefaultFrameDirection(GetAppLanguage())
            : FRMDIR_HORI_LEFT_TOP;

        pNew->GetMaster().SetFmtAttr( SvxFrameDirectionItem(aFrameDirection, RES_FRAMEDIR) );
        pNew->GetLeft().SetFmtAttr( SvxFrameDirectionItem(aFrameDirection, RES_FRAMEDIR) );
    }
    aPageDescs.Insert( pNew, aPageDescs.Count() );

    if (bBroadcast)
        BroadcastStyleOperation(rName, SFX_STYLE_FAMILY_PAGE,
                                SFX_STYLESHEET_CREATED);

    if (DoesUndo())
        AppendUndo(new SwUndoPageDescCreate(pNew, this));

    SetModified();
    return (aPageDescs.Count()-1);
}

SwPageDesc* SwDoc::FindPageDescByName( const String& rName, USHORT* pPos ) const
{
    SwPageDesc* pRet = 0;
    if( pPos ) *pPos = USHRT_MAX;

    for( USHORT n = 0, nEnd = aPageDescs.Count(); n < nEnd; ++n )
        if( aPageDescs[ n ]->GetName() == rName )
        {
            pRet = aPageDescs[ n ];
            if( pPos )
                *pPos = n;
            break;
        }
    return pRet;
}

/******************************************************************************
 *  Methode     :   void SwDoc::PrtDataChanged()
 *  Beschreibung:
 ******************************************************************************/

void SwDoc::PrtDataChanged()
{
//!!!!!!!! Bei Aenderungen hier bitte ggf. InJobSetup im Sw3io mitpflegen

    // --> FME 2005-01-21 #i41075#
    OSL_ENSURE( get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) ||
            0 != getPrinter( sal_False ), "PrtDataChanged will be called recursive!" );
    // <--

    SwWait *pWait = 0;
    BOOL bEndAction = FALSE;

    if( GetDocShell() )
        GetDocShell()->UpdateFontList();

    BOOL bDraw = TRUE;
    if ( GetRootFrm() )
    {
        ViewShell *pSh = GetRootFrm()->GetCurrShell();
        if( !get(IDocumentSettingAccess::BROWSE_MODE) ||
            ( pSh && pSh->GetViewOptions()->IsPrtFormat() ) )
        {
            if ( GetDocShell() )
                pWait = new SwWait( *GetDocShell(), TRUE );

            GetRootFrm()->StartAllAction();
            bEndAction = TRUE;

            bDraw = FALSE;
            if( pDrawModel )
            {
                pDrawModel->SetAddExtLeading( get(IDocumentSettingAccess::ADD_EXT_LEADING) );
                pDrawModel->SetRefDevice( getReferenceDevice( false ) );
            }

            pFntCache->Flush();
            GetRootFrm()->InvalidateAllCntnt();

            if ( pSh )
            {
                do
                {
                    pSh->InitPrt( pPrt );
                    pSh = (ViewShell*)pSh->GetNext();
                }
                while ( pSh != GetRootFrm()->GetCurrShell() );
            }

        }
    }
    if ( bDraw && pDrawModel )
    {
        const sal_Bool bTmpAddExtLeading = get(IDocumentSettingAccess::ADD_EXT_LEADING);
        if ( bTmpAddExtLeading != pDrawModel->IsAddExtLeading() )
            pDrawModel->SetAddExtLeading( bTmpAddExtLeading );

        OutputDevice* pOutDev = getReferenceDevice( false );
        if ( pOutDev != pDrawModel->GetRefDevice() )
            pDrawModel->SetRefDevice( pOutDev );
    }

    PrtOLENotify( TRUE );

    if ( bEndAction )
        GetRootFrm()->EndAllAction();
    delete pWait;
}

//Zur Laufzeit sammeln wir die GlobalNames der Server, die keine
//Benachrichtigung zu Druckerwechseln wuenschen. Dadurch sparen wir
//das Laden vieler Objekte (gluecklicherweise werden obendrein alle
//Fremdobjekte unter einer ID abgebuildet). Init und DeInit vom Array
//ist in init.cxx zu finden.
extern SvPtrarr *pGlobalOLEExcludeList;

void SwDoc::PrtOLENotify( BOOL bAll )
{
    SwFEShell *pShell = 0;
    if ( GetRootFrm() && GetRootFrm()->GetCurrShell() )
    {
        ViewShell *pSh = GetRootFrm()->GetCurrShell();
        if ( !pSh->ISA(SwFEShell) )
            do
            {   pSh = (ViewShell*)pSh->GetNext();
            } while ( !pSh->ISA(SwFEShell) &&
                      pSh != GetRootFrm()->GetCurrShell() );

        if ( pSh->ISA(SwFEShell) )
            pShell = (SwFEShell*)pSh;
    }
    if ( !pShell )
    {
        //Das hat ohne Shell und damit ohne Client keinen Sinn, weil nur darueber
        //die Kommunikation bezueglich der Groessenaenderung implementiert ist.
        //Da wir keine Shell haben, merken wir uns diesen unguenstigen
        //Zustand am Dokument, dies wird dann beim Erzeugen der ersten Shell
        //nachgeholt.
        mbOLEPrtNotifyPending = TRUE;
        if ( bAll )
            mbAllOLENotify = TRUE;
    }
    else
    {
        if ( mbAllOLENotify )
            bAll = TRUE;

        mbOLEPrtNotifyPending = mbAllOLENotify = FALSE;


        SwOLENodes *pNodes = 0;
        SwClientIter aIter( *(SwModify*)GetDfltGrfFmtColl() );
        for( SwCntntNode* pNd = (SwCntntNode*)aIter.First( TYPE( SwCntntNode ) );
             pNd;
             pNd = (SwCntntNode*)aIter.Next() )
        {
            SwOLENode *pONd;
            if ( 0 != (pONd = pNd->GetOLENode()) &&
                 (bAll || pONd->IsOLESizeInvalid()) )
            {
                if ( !pNodes  )
                    pNodes = new SwOLENodes;
                pNodes->Insert( pONd, pNodes->Count() );
            }
        }

        if ( pNodes )
        {
            ::StartProgress( STR_STATSTR_SWGPRTOLENOTIFY,
                             0, pNodes->Count(), GetDocShell());
            GetRootFrm()->StartAllAction();

            for( USHORT i = 0; i < pNodes->Count(); ++i )
            {
                ::SetProgressState( i, GetDocShell() );

                SwOLENode* pOLENd = (*pNodes)[i];
                pOLENd->SetOLESizeInvalid( FALSE );

                //Ersteinmal die Infos laden und festellen ob das Teil nicht
                //schon in der Exclude-Liste steht
                SvGlobalName aName;

                svt::EmbeddedObjectRef& xObj = pOLENd->GetOLEObj().GetObject();
                if ( xObj.is() )
                    aName = SvGlobalName( xObj->getClassID() );
                else  //Noch nicht geladen
                {
                        // TODO/LATER: retrieve ClassID of an unloaded object
                        // aName = ????
                }

                BOOL bFound = FALSE;
                for ( USHORT j = 0;
                      j < pGlobalOLEExcludeList->Count() && !bFound;
                      ++j )
                {
                    bFound = *(SvGlobalName*)(*pGlobalOLEExcludeList)[j] ==
                                    aName;
                }
                if ( bFound )
                    continue;

                //Kennen wir nicht, also muss das Objekt geladen werden.
                //Wenn es keine Benachrichtigung wuenscht
                if ( xObj.is() )
                {
                        pGlobalOLEExcludeList->Insert(
                                new SvGlobalName( aName ),
                                pGlobalOLEExcludeList->Count() );
                }
            }
            delete pNodes;
            GetRootFrm()->EndAllAction();
            ::EndProgress( GetDocShell() );
        }
    }
}

IMPL_LINK( SwDoc, DoUpdateModifiedOLE, Timer *, )
{
    SwFEShell* pSh = (SwFEShell*)GetEditShell();
    if( pSh )
    {
        mbOLEPrtNotifyPending = mbAllOLENotify = FALSE;

        SwOLENodes aOLENodes;
        SwClientIter aIter( *(SwModify*)GetDfltGrfFmtColl() );
        for( SwCntntNode* pNd = (SwCntntNode*)aIter.First( TYPE( SwCntntNode ) );
             pNd;
             pNd = (SwCntntNode*)aIter.Next() )
        {
            SwOLENode *pONd = pNd->GetOLENode();
            if( pONd && pONd->IsOLESizeInvalid() )
            {
                aOLENodes.Insert( pONd, aOLENodes.Count() );
            }
        }

        if( aOLENodes.Count() )
        {
            ::StartProgress( STR_STATSTR_SWGPRTOLENOTIFY,
                             0, aOLENodes.Count(), GetDocShell());
            GetRootFrm()->StartAllAction();
            SwMsgPoolItem aMsgHint( RES_UPDATE_ATTR );

            for( USHORT i = 0; i < aOLENodes.Count(); ++i )
            {
                ::SetProgressState( i, GetDocShell() );

                SwOLENode* pOLENd = aOLENodes[i];
                pOLENd->SetOLESizeInvalid( FALSE );

                //Kennen wir nicht, also muss das Objekt geladen werden.
                //Wenn es keine Benachrichtigung wuenscht
                if( pOLENd->GetOLEObj().GetOleRef().is() ) //Kaputt?
                {
                    pOLENd->Modify( &aMsgHint, &aMsgHint );
                }
            }
            GetRootFrm()->EndAllAction();
            ::EndProgress( GetDocShell() );
        }
    }
    return 0;
}

BOOL SwDoc::FindPageDesc( const String & rName, sal_uInt16 * pFound)
{
    BOOL bResult = FALSE;
    sal_uInt16 nI;
    for (nI = 0; nI < aPageDescs.Count(); nI++)
    {
        if (aPageDescs[nI]->GetName() == rName)
        {
            *pFound = nI;
            bResult = TRUE;
            break;
        }
    }

    return bResult;
}

SwPageDesc * SwDoc::GetPageDesc( const String & rName )
{
    SwPageDesc * aResult = NULL;

    sal_uInt16 nI;

    if (FindPageDesc(rName, &nI))
        aResult = aPageDescs[nI];

    return aResult;
}

void SwDoc::DelPageDesc( const String & rName, BOOL bBroadcast )
{
    sal_uInt16 nI;

    if (FindPageDesc(rName, &nI))
        DelPageDesc(nI, bBroadcast);
}

void SwDoc::ChgPageDesc( const String & rName, const SwPageDesc & rDesc)
{
    sal_uInt16 nI;

    if (FindPageDesc(rName, &nI))
        ChgPageDesc(nI, rDesc);
}

/*
 * The HTML import cannot resist changing the page descriptions, I don't
 * know why. This function is meant to check the page descriptors for invalid
 * values.
 */
void SwDoc::CheckDefaultPageFmt()
{
    for ( USHORT i = 0; i < GetPageDescCnt(); ++i )
    {
        SwPageDesc& rDesc = _GetPageDesc( i );

        SwFrmFmt& rMaster = rDesc.GetMaster();
        SwFrmFmt& rLeft   = rDesc.GetLeft();

        const SwFmtFrmSize& rMasterSize  = rMaster.GetFrmSize();
        const SwFmtFrmSize& rLeftSize    = rLeft.GetFrmSize();

        const bool bSetSize = LONG_MAX == rMasterSize.GetWidth() ||
                              LONG_MAX == rMasterSize.GetHeight() ||
                              LONG_MAX == rLeftSize.GetWidth() ||
                              LONG_MAX == rLeftSize.GetHeight();

        if ( bSetSize )
            lcl_DefaultPageFmt( rDesc.GetPoolFmtId(), rDesc.GetMaster(), rDesc.GetLeft() );
    }
}

void SwDoc::SetDefaultPageMode(bool bSquaredPageMode)
{
    if( !bSquaredPageMode == !IsSquaredPageMode() )
        return;

    const SwTextGridItem& rGrid =
                    (const SwTextGridItem&)GetDefault( RES_TEXTGRID );
    SwTextGridItem aNewGrid = rGrid;
    aNewGrid.SetSquaredMode(bSquaredPageMode);
    aNewGrid.Init();
    SetDefault(aNewGrid);

    for ( USHORT i = 0; i < GetPageDescCnt(); ++i )
    {
        SwPageDesc& rDesc = _GetPageDesc( i );

        SwFrmFmt& rMaster = rDesc.GetMaster();
        SwFrmFmt& rLeft = rDesc.GetLeft();

        SwTextGridItem aGrid((SwTextGridItem&)rMaster.GetFmtAttr(RES_TEXTGRID));
        aGrid.SwitchPaperMode( bSquaredPageMode );
        rMaster.SetFmtAttr(aGrid);
        rLeft.SetFmtAttr(aGrid);
    }
}

sal_Bool SwDoc::IsSquaredPageMode() const
{
    const SwTextGridItem& rGrid =
                        (const SwTextGridItem&)GetDefault( RES_TEXTGRID );
    return rGrid.IsSquaredMode();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
