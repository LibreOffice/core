/*************************************************************************
 *
 *  $RCSfile: docdesc.cxx,v $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX //autogen
#include <svx/paperinf.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _GLOBNAME_HXX //autogen
#include <tools/globname.hxx>
#endif

#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FTNINFO_HXX //autogen
#include <ftninfo.hxx>
#endif
#ifndef _FESH_HXX //autogen
#include <fesh.hxx>
#endif
#ifndef _NDOLE_HXX //autogen
#include <ndole.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>  //Fuer DelPageDesc
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>  //Fuer DelPageDesc
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _FTNIDX_HXX
#include <ftnidx.hxx>
#endif
#ifndef _FMTFTN_HXX
#include <fmtftn.hxx>
#endif
#ifndef _TXTFTN_HXX
#include <txtftn.hxx>
#endif
#ifndef _FNTCACHE_HXX
#include <fntcache.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif

#ifndef _STATSTR_HRC
#include <statstr.hrc>
#endif

void lcl_DefaultPageFmt( SwFrmFmt &rFmt1, SwFrmFmt &rFmt2, SfxPrinter *pPrt,
                     BOOL bCheck )
{
    //Einstellung von Seitengroesse und Seitenraendern. Dazu wird
    //der Default-Printer benutzt.
    //Die Physikalische Seitengroesse ist die des Printers
    //oder DIN-A4 wenn der Printer eine Groesse von 0 liefert.
    //Der Seitenrand ergibt sich aus der Seitengroesse, der Output-
    //SSize des Printers und einem Offset der die linke obere Ecke
    //der Output-SSize relativ zur Physikalischen Pagesize angibt.
    //Wenn der Offset 0 ist, werden eingestellt.
    //!!!Die Seitengroesse wird hier im Attribut eingestellt,
    //dies wird im Ctor des SwPageFrm beachtet.

    SvxLRSpaceItem aLR;
    SvxULSpaceItem aUL;
    SwFmtFrmSize aFrmSize( ATT_FIX_SIZE );
    BOOL bSetFmt1 = TRUE,
         bSetFmt2 = TRUE;
    if ( pPrt )
    {
        if ( bCheck )
        {
            const SwFmtFrmSize &rFrmSize = rFmt1.GetFrmSize();
            const SwFmtFrmSize &rFrmSize2 = rFmt2.GetFrmSize();
            bSetFmt1 = LONG_MAX == rFrmSize.GetWidth() ||
                       LONG_MAX == rFrmSize.GetHeight();
            bSetFmt2 = LONG_MAX == rFrmSize2.GetWidth() ||
                       LONG_MAX == rFrmSize2.GetHeight();
            if ( !bSetFmt1 && !bSetFmt2 )
                return;
        }

        //Seitengrosse
        //fuer das Erfragen von SV, ob ein Drucker angeschlossen ist,
        //werden die SV'ler noch eine Methode anbieten.
        const Size aPhysSize( SvxPaperInfo::GetPaperSize( (Printer*)pPrt ));

        //if ( aPhysSize.Width() <= 0 )
        //  aPhysSize.Width() = lA4Width;
        //if ( aPhysSize.Height() <= 0 )
        //  aPhysSize.Height() = lA4Height;
        aFrmSize.SetSize( aPhysSize );

        //Raender
        Size    aOutSize;
        Point   aOffst( pPrt->GetPageOffset() );

        //Auf Default-Raender vorbereiten.
        //Raender haben eine defaultmaessige Mindestgroesse.
        //wenn der Drucker einen groesseren Rand vorgibt, so
        //ist mir dass auch recht.
        International aInternational;
        long nMinTopBottom, nMinLeftRight;
        if ( MEASURE_METRIC == aInternational.GetMeasurementSystem() )
            nMinTopBottom = nMinLeftRight = 1134;   //2 Zentimeter
        else
        {
            nMinTopBottom = 1440;   //al la WW: 1Inch
            nMinLeftRight = 1800;   //          1,25 Inch
        }

        if ( aOffst.X() < nMinLeftRight )
            aOffst.X() = nMinLeftRight;
        if ( aOffst.Y() < nMinTopBottom )
            aOffst.Y() = nMinTopBottom;
        aOutSize.Width()  = aPhysSize.Width() - ( 2 * aOffst.X() );
        aOutSize.Height() = aPhysSize.Height() - ( 2 * aOffst.Y() );

        //Raender einstellen.
        aUL.SetUpper( USHORT(aOffst.Y()) );
        aUL.SetLower( USHORT(aPhysSize.Height() - aOutSize.Height() - aOffst.Y()));
        aLR.SetRight( USHORT(aOffst.X()) );
        aLR.SetLeft(  USHORT(aPhysSize.Width() - aOutSize.Width() - aOffst.X()));
    }
    else
    {
        aFrmSize.SetWidth( LONG_MAX );
        aFrmSize.SetHeight( LONG_MAX );
        aUL.SetUpper( 0 );
        aUL.SetLower( 0 );
        aLR.SetRight( 0 );
        aLR.SetLeft(  0 );
    }
    if ( bSetFmt1 )
    {
        rFmt1.SetAttr( aFrmSize );
        rFmt1.SetAttr( aLR );
        rFmt1.SetAttr( aUL );
    }
    if ( bSetFmt2 )
    {
        rFmt2.SetAttr( aFrmSize );
        rFmt2.SetAttr( aLR );
        rFmt2.SetAttr( aUL );
    }
}

/*************************************************************************
|*
|*  SwDoc::ChgPageDesc()
|*
|*  Ersterstellung      MA 25. Jan. 93
|*  Letzte Aenderung    MA 01. Mar. 95
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
    USHORT __READONLY_DATA aIdArr[] = { RES_FRM_SIZE, RES_UL_SPACE,
                                        RES_BACKGROUND, RES_SHADOW,
                                        RES_COL, RES_COL, 0 };
    const SfxPoolItem* pItem;
    for( USHORT n = 0; aIdArr[ n ]; n += 2 )
    {
        for( USHORT nId = aIdArr[ n ]; nId <= aIdArr[ n+1]; ++nId )
        {
            if( bPage || ( RES_COL != nId && RES_PAPER_BIN != nId ))
            {
                if( SFX_ITEM_SET == rSource.GetItemState( nId, FALSE, &pItem ))
                    rDest.SetAttr( *pItem );
                else
                    rDest.ResetAttr( nId );
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
    ASSERT( i < aPageDescs.Count(), "PageDescs ueberindiziert." );

    SwPageDesc *pDesc = aPageDescs[i];

    //Als erstes wird ggf. gespiegelt.
    if ( rChged.GetUseOn() == PD_MIRROR )
        ((SwPageDesc&)rChged).Mirror();
    else
        //sonst Werte aus Master nach Left uebertragen.
        ::lcl_DescSetAttr( ((SwPageDesc&)rChged).GetMaster(),
                       ((SwPageDesc&)rChged).GetLeft() );

    //NumType uebernehmen.
    if( rChged.GetNumType().eType != pDesc->GetNumType().eType )
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

    //Header abgleichen.
    const SwFmtHeader &rHead = rChged.GetMaster().GetHeader();
    if( DoesUndo() )
    {
        // hat sich an den Nodes etwas veraendert ?
        //JP erstmal ein Hack, solange keine Headers/Footers Undofaehig sind
        const SwFmtHeader &rOldHead = pDesc->GetMaster().GetHeader();
        if( rHead.IsActive() != rOldHead.IsActive() ||
            rChged.IsHeaderShared() != pDesc->IsHeaderShared() )
        {
            // erstmal werden alle Undo - Objecte geloescht.
            ClearRedo();
            DelAllUndoObj();
        }
    }
    pDesc->GetMaster().SetAttr( rHead );
    if ( rChged.IsHeaderShared() || !rHead.IsActive() )
    {
        //Left teilt sich den Header mit dem Master.
        pDesc->GetLeft().SetAttr( pDesc->GetMaster().GetHeader() );
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
            SwFmtHeader aHead( MakeLayoutFmt( RND_STD_HEADERL ) );
            pDesc->GetLeft().SetAttr( aHead );
            //Weitere Attribute (Raender, Umrandung...) uebernehmen.
            ::lcl_DescSetAttr( *rHead.GetHeaderFmt(), *aHead.GetHeaderFmt(), FALSE);
        }
        else
        {
            const SwFrmFmt *pRight = rHead.GetHeaderFmt();
            const SwFmtCntnt &aRCnt = pRight->GetCntnt();
            const SwFmtCntnt &aLCnt = rLeftHead.GetHeaderFmt()->GetCntnt();
            if ( (*aRCnt.GetCntntIdx()) == (*aLCnt.GetCntntIdx()) )
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

                pFmt->SetAttr( SwFmtCntnt( pSttNd ) );
                pDesc->GetLeft().SetAttr( SwFmtHeader( pFmt ) );
            }
            else
                ::lcl_DescSetAttr( *pRight,
                               *(SwFrmFmt*)rLeftHead.GetHeaderFmt(), FALSE );

        }
    }
    pDesc->ChgHeaderShare( rChged.IsHeaderShared() );

    //Footer abgleichen.
    const SwFmtFooter &rFoot = rChged.GetMaster().GetFooter();
    if( DoesUndo() )
    {
        // hat sich an den Nodes etwas veraendert ?
        //JP erstmal ein Hack, solange keine Headers/Footers Undofaehig sind
        const SwFmtFooter &rOldFoot = pDesc->GetMaster().GetFooter();
        if( rFoot.IsActive() != rOldFoot.IsActive() ||
            rChged.IsFooterShared() != pDesc->IsFooterShared() )
        {
            // erstmal werden alle Undo - Objecte geloescht.
            ClearRedo();
            DelAllUndoObj();
        }
    }
    pDesc->GetMaster().SetAttr( rFoot );
    if ( rChged.IsFooterShared() || !rFoot.IsActive() )
        //Left teilt sich den Header mit dem Master.
        pDesc->GetLeft().SetAttr( pDesc->GetMaster().GetFooter() );
    else if ( rFoot.IsActive() )
    {   //Left bekommt einen eigenen Footer verpasst wenn das Format nicht
        //bereits einen hat.
        //Wenn er bereits einen hat und dieser auf die gleiche Section
        //wie der Rechte zeigt, so muss er einen eigenen bekommen. Der
        //Inhalt wird sinnigerweise kopiert.
        const SwFmtFooter &rLeftFoot = pDesc->GetLeft().GetFooter();
        if ( !rLeftFoot.IsActive() )
        {
            SwFmtFooter aFoot( MakeLayoutFmt( RND_STD_FOOTER ) );
            pDesc->GetLeft().SetAttr( aFoot );
            //Weitere Attribute (Raender, Umrandung...) uebernehmen.
            ::lcl_DescSetAttr( *rFoot.GetFooterFmt(), *aFoot.GetFooterFmt(), FALSE);
        }
        else
        {
            const SwFrmFmt *pRight = rFoot.GetFooterFmt();
            const SwFmtCntnt &aRCnt = pRight->GetCntnt();
            const SwFmtCntnt &aLCnt = rLeftFoot.GetFooterFmt()->GetCntnt();
            if ( (*aRCnt.GetCntntIdx()) == (*aLCnt.GetCntntIdx()) )
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

                pFmt->SetAttr( SwFmtCntnt( pSttNd ) );
                pDesc->GetLeft().SetAttr( SwFmtFooter( pFmt ) );
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
}

/*************************************************************************
|*
|*  SwDoc::DelPageDesc()
|*
|*  Beschreibung        Alle Descriptoren, deren Follow auf den zu loeschenden
|*      zeigen muessen angepasst werden.
|*  Ersterstellung      MA 25. Jan. 93
|*  Letzte Aenderung    JP 04.09.95
|*
|*************************************************************************/

void lcl_RemoveFrms( SwFrmFmt& rFmt, FASTBOOL& rbFtnsRemoved )
{
    SwClientIter aIter( rFmt );
    SwFrm *pFrm;
    for( pFrm = (SwFrm*)aIter.First(TYPE(SwFrm)); pFrm;
            pFrm = (SwFrm*)aIter.Next() )
        if ( !rbFtnsRemoved && pFrm->IsPageFrm() &&
                ((SwPageFrm*)pFrm)->IsFtnPage() )
        {
            rFmt.GetDoc()->GetRootFrm()->RemoveFtns( 0, FALSE, TRUE );
            rbFtnsRemoved = TRUE;
        }
        else
        {
            pFrm->Cut();
            delete pFrm;
        }
}


void SwDoc::DelPageDesc( USHORT i )
{
    ASSERT( i < aPageDescs.Count(), "PageDescs ueberindiziert." );
    ASSERT( i != 0, "Default Pagedesc loeschen is nicht." );
    if ( i == 0 )
        return;

    SwPageDesc *pDel = aPageDescs[i];

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
                    ((SwFmt*)pMod)->SetAttr( aDfltDesc );
                else
                {
                    ASSERT( !this, "was ist das fuer ein Mofify-Obj?" );
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
        FASTBOOL bFtnsRemoved = FALSE;

        ::lcl_RemoveFrms( pDel->GetMaster(), bFtnsRemoved );
        ::lcl_RemoveFrms( pDel->GetLeft(), bFtnsRemoved );
    }

    aPageDescs.Remove( i );
    delete pDel;
    SetModified();
}

/*************************************************************************
|*
|*  SwDoc::MakePageDesc()
|*
|*  Ersterstellung      MA 25. Jan. 93
|*  Letzte Aenderung    MA 20. Aug. 93
|*
|*************************************************************************/

USHORT SwDoc::MakePageDesc( const String &rName, const SwPageDesc *pCpy)
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
        ::lcl_DefaultPageFmt( pNew->GetMaster(), pNew->GetLeft(), GetPrt(), FALSE );
        if( GetPrt() )
            pNew->SetLandscape( ORIENTATION_LANDSCAPE ==
                                GetPrt()->GetOrientation() );
    }
    aPageDescs.Insert( pNew, aPageDescs.Count() );
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
 *  Methode     :   void SwDoc::SetPrt( SfxPrinter *pP )
 *  Beschreibung:
 *  Erstellt    :   OK 27.10.94 10:20
 *  Aenderung   :   MA 26. Mar. 98
 ******************************************************************************/

void SwDoc::PrtDataChanged()
{
//!!!!!!!! Bei Aenderungen hier bitte ggf. InJobSetup im Sw3io mitpflegen

    SwWait *pWait = 0;
    BOOL bEndAction = FALSE;

    if( GetDocShell() )
        GetDocShell()->UpdateFontList();

    BOOL bDraw = TRUE;
    if ( GetRootFrm() )
    {
        ViewShell *pSh = GetRootFrm()->GetCurrShell();
        if( !IsBrowseMode() || ( pSh && pSh->GetViewOptions()->IsPrtFormat() ) )
        {
            if ( GetDocShell() )
                pWait = new SwWait( *GetDocShell(), TRUE );

            GetRootFrm()->StartAllAction();
            bEndAction = TRUE;

            bDraw = FALSE;
            if( pDrawModel )
                pDrawModel->SetRefDevice( pPrt );

            pFntCache->Flush();
            GetRootFrm()->InvalidateAllCntnt();

            if ( pSh )
            {
                do
                {   pSh->InitPrt( pPrt );
                    pSh = (ViewShell*)pSh->GetNext();
                } while ( pSh != GetRootFrm()->GetCurrShell() );
            }

        }
    }
    if ( bDraw && pDrawModel && pPrt && pPrt != pDrawModel->GetRefDevice() )
        pDrawModel->SetRefDevice( pPrt );

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
        bOLEPrtNotifyPending = TRUE;
        if ( bAll )
            bAllOLENotify = TRUE;
    }
    else
    {
        if ( bAllOLENotify )
            bAll = TRUE;

        bOLEPrtNotifyPending = bAllOLENotify = FALSE;


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

                if ( !pOLENd->GetOLEObj().IsOleRef() )  //Noch nicht geladen
                {
                    String sBaseURL( INetURLObject::GetBaseURL() );
                    const SfxMedium *pMedium;
                    if( 0 != (pMedium = GetDocShell()->GetMedium()) &&
                        pMedium->GetName() != sBaseURL )
                        INetURLObject::SetBaseURL( pMedium->GetName() );
                    SvInfoObjectRef xInfo = GetPersist()->Find( pOLENd->GetOLEObj().GetName() );
                    if ( xInfo.Is() )   //Muss normalerweise gefunden werden
                        aName = xInfo->GetClassName();
                    INetURLObject::SetBaseURL( sBaseURL );
                }
                else
                    aName = pOLENd->GetOLEObj().GetOleRef()->GetClassName();

                BOOL bFound = FALSE;
                for ( USHORT i = 0;
                      i < pGlobalOLEExcludeList->Count() && !bFound;
                      ++i )
                {
                    bFound = *(SvGlobalName*)(*pGlobalOLEExcludeList)[i] ==
                                    aName;
                }
                if ( bFound )
                    continue;

                //Kennen wir nicht, also muss das Objekt geladen werden.
                //Wenn es keine Benachrichtigung wuenscht
                SvEmbeddedObjectRef xRef( (SvInPlaceObject*) pOLENd->GetOLEObj().GetOleRef() );
                if ( xRef ) //Kaputt?
                {
                    if ( SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE & xRef->GetMiscStatus())
                    {
                        if ( pOLENd->GetFrm() )
                        {
                            xRef->OnDocumentPrinterChanged( pPrt );
                            pShell->CalcAndSetScale( xRef );//Client erzeugen lassen.
                        }
                        else
                            pOLENd->SetOLESizeInvalid( TRUE );
                    }
                    else
                        pGlobalOLEExcludeList->Insert(
                                new SvGlobalName( xRef->GetClassName()),
                                pGlobalOLEExcludeList->Count() );
                }
            }
            delete pNodes;
            GetRootFrm()->EndAllAction();
            ::EndProgress( GetDocShell() );
        }
    }
}

void SwDoc::SetPrt( SfxPrinter *pP )
{
    ASSERT( pP, "Kein Drucker !" );

    const BOOL bInitPageDesc = pPrt == 0;

    if ((ULONG) pP != (ULONG) pPrt)
    {
        delete pPrt;
        pPrt = pP;
        PrtDataChanged();
    }

    if( bInitPageDesc )
    {
        // JP 17.04.97: Bug 38924 - falls noch kein Drucker gesetzt war
        //              und der PageDesc nicht eingelesen wurde
        //      -> passe an den Drucker an
        if( pPrt && LONG_MAX == _GetPageDesc( 0 ).GetMaster().GetFrmSize().GetWidth() )
            _GetPageDesc( 0 ).SetLandscape( ORIENTATION_LANDSCAPE ==
                                            pPrt->GetOrientation() );

        //Ggf. Standard Seitenformat anhand des Druckers einstellen.
        //lcl_DefaultPageFmt() merkt ob ein Reader das Fmt veraendert hat.

        //MA 11. Mar. 97: Das sollten wir fuer alle Formate tun, weil die
        //Werte auf LONG_MAX initalisiert sind (spaetes anlegen des Druckers)
        //und die Reader u.U. "unfertige" Formate stehenlassen.
        for ( USHORT i = 0; i < GetPageDescCnt(); ++i )
        {
            ::lcl_DefaultPageFmt( _GetPageDesc( i ).GetMaster(),
                              _GetPageDesc( i ).GetLeft(),
                              pPrt, TRUE );
        }
    }
}

/*
 *  Kleiner Hack;
 *
const SwPageDesc& SwDoc::GetPageDesc( USHORT i ) const
{
    if( !i && !aPageDescs.Count() )            // noch keiner vorhanden?
        ((SwDoc*)this)->InitPageDescs();        //Default PageDescriptor
    return *aPageDescs[i];
}

SwPageDesc& SwDoc::_GetPageDesc( USHORT i ) const
{
    if( !i && !aPageDescs.Count() )         // noch keiner vorhanden?
        ((SwDoc*)this)->InitPageDescs();        //Default PageDescriptor
    return *aPageDescs[i];
}
*/



IMPL_LINK( SwDoc, DoUpdateModifiedOLE, Timer *, pTimer )
{
    SwFEShell* pSh = (SwFEShell*)GetEditShell();
    if( pSh )
    {
        bOLEPrtNotifyPending = bAllOLENotify = FALSE;

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
                SvEmbeddedObjectRef xRef( (SvInPlaceObject*)
                                        pOLENd->GetOLEObj().GetOleRef() );
                if( xRef ) //Kaputt?
                {
                    if( SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE &
                            xRef->GetMiscStatus() )
                    {
                        if( pOLENd->GetFrm() )
                        {
                            xRef->OnDocumentPrinterChanged( pPrt );
                            pSh->CalcAndSetScale( xRef );//Client erzeugen lassen.
                        }
                        else
                            pOLENd->SetOLESizeInvalid( TRUE );
                    }
                    // repaint it
                    pOLENd->Modify( &aMsgHint, &aMsgHint );
                }
            }
            GetRootFrm()->EndAllAction();
            ::EndProgress( GetDocShell() );
        }
    }
    return 0;
}




