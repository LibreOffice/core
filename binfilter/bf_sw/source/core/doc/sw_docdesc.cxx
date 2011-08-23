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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <hintids.hxx>
#include <vcl/virdev.hxx>
#include <bf_sfx2/printer.hxx>
#include <bf_svx/svdmodel.hxx>
#include <bf_svx/ulspitem.hxx>
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/paperinf.hxx>
#include "bf_svx/frmdiritem.hxx"
#include <tools/urlobj.hxx>
#include <bf_sfx2/docfile.hxx>
#include <unotools/localedatawrapper.hxx>

#include <fmtfsize.hxx>
#include <fmthdft.hxx>
#include <fmtcntnt.hxx>
#include <fmtpdsc.hxx>
#include <ftninfo.hxx>

#include <horiornt.hxx>
#include <errhdl.hxx>
#include <viscrs.hxx>
#include <fesh.hxx>
#include <ndole.hxx>
#include <mdiexp.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <pagefrm.hxx>  //Fuer DelPageDesc
#include <rootfrm.hxx>	//Fuer DelPageDesc
#include <hints.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include <docsh.hxx>
#include <ndindex.hxx>
#include <ftnidx.hxx>
#include <fmtftn.hxx>
#include <txtftn.hxx>
#include <fntcache.hxx>
#include <viewopt.hxx>
#include <fldbas.hxx>
#include <GetMetricVal.hxx>

#include <statstr.hrc>
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

/*N*/ void lcl_DefaultPageFmt( sal_uInt16 nPoolFmtId, SwFrmFmt &rFmt1,
/*N*/ 						 SwFrmFmt &rFmt2, SfxPrinter *pPrt, BOOL bCheck )
/*N*/ {
/*N*/ 	//Einstellung von Seitengroesse und Seitenraendern. Dazu wird
/*N*/ 	//der Default-Printer benutzt.
/*N*/ 	//Die Physikalische Seitengroesse ist die des Printers
/*N*/ 	//oder DIN-A4 wenn der Printer eine Groesse von 0 liefert.
/*N*/ 	//Der Seitenrand ergibt sich aus der Seitengroesse, der Output-
/*N*/ 	//SSize des Printers und einem Offset der die linke obere Ecke
/*N*/ 	//der Output-SSize relativ zur Physikalischen Pagesize angibt.
/*N*/ 	//Wenn der Offset 0 ist, werden eingestellt.
/*N*/ 	//!!!Die Seitengroesse wird hier im Attribut eingestellt,
/*N*/ 	//dies wird im Ctor des SwPageFrm beachtet.
/*N*/
/*N*/ 	SvxLRSpaceItem aLR;
/*N*/ 	SvxULSpaceItem aUL;
/*N*/ 	SwFmtFrmSize aFrmSize( ATT_FIX_SIZE );
/*N*/ 	BOOL bSetFmt1 = TRUE,
/*N*/ 		 bSetFmt2 = TRUE;
/*N*/ 	if ( pPrt )
/*N*/ 	{
/*N*/ 		if ( bCheck )
/*N*/ 		{
/*N*/ 			const SwFmtFrmSize &rFrmSize = rFmt1.GetFrmSize();
/*N*/ 			const SwFmtFrmSize &rFrmSize2 = rFmt2.GetFrmSize();
/*N*/ 			bSetFmt1 = LONG_MAX == rFrmSize.GetWidth() ||
/*N*/ 					   LONG_MAX == rFrmSize.GetHeight();
/*N*/ 			bSetFmt2 = LONG_MAX == rFrmSize2.GetWidth() ||
/*N*/ 					   LONG_MAX == rFrmSize2.GetHeight();
/*N*/ 			if ( !bSetFmt1 && !bSetFmt2 )
/*N*/ 				return;
/*N*/ 		}
/*N*/
/*N*/ 		//Seitengrosse
/*N*/ 		//fuer das Erfragen von SV, ob ein Drucker angeschlossen ist,
/*N*/ 		//werden die SV'ler noch eine Methode anbieten.
/*N*/ 		const Size aPhysSize( SvxPaperInfo::GetPaperSize( (Printer*)pPrt ));
/*N*/
/*N*/ 		//if ( aPhysSize.Width() <= 0 )
/*N*/ 		//	aPhysSize.Width() = lA4Width;
/*N*/ 		//if ( aPhysSize.Height() <= 0 )
/*N*/ 		//	aPhysSize.Height() = lA4Height;
/*N*/ 		aFrmSize.SetSize( aPhysSize );
/*N*/
/*N*/ 		//Raender
/*N*/ 		Point	aOffst(	pPrt->GetPageOffset() );
/*N*/         aOffst += pPrt->GetMapMode().GetOrigin();
/*N*/
/*N*/ 		//Auf Default-Raender vorbereiten.
/*N*/ 		//Raender haben eine defaultmaessige Mindestgroesse.
/*N*/ 		//wenn der Drucker einen groesseren Rand vorgibt, so
/*N*/ 		//ist mir dass auch recht.
/*N*/ 		// MIB 06/25/2002, #99397#: The HTML page desc had A4 as page size
/*N*/ 		// always. This has been changed to take the page size from the printer.
/*N*/ 		// Unfortunately, the margins of the HTML page desc are smaller than
/*N*/ 		// the margins used here in general, so one extra case is required.
/*N*/ 		// In the long term, this needs to be changed to always keep the
/*N*/ 		// margins from the page desc.
/*N*/ 		sal_Int32 nMinTop, nMinBottom, nMinLeft, nMinRight;
/*N*/ 		if( RES_POOLPAGE_HTML == nPoolFmtId )
/*N*/ 		{
/*N*/ 			nMinRight = nMinTop = nMinBottom = GetMetricVal( CM_1 );
/*N*/ 			nMinLeft = nMinRight * 2;
/*N*/ 		}
/*N*/ 		else if( MEASURE_METRIC == GetAppLocaleData().getMeasurementSystemEnum() )
/*N*/ 		{
/*N*/ 			nMinTop = nMinBottom = nMinLeft = nMinRight = 1134;	//2 Zentimeter
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			nMinTop = nMinBottom = 1440;	//al la WW: 1Inch
/*N*/ 			nMinLeft = nMinRight = 1800;	//			1,25 Inch
/*N*/ 		}
/*N*/
/*N*/ 		//Raender einstellen.
/*N*/ 		aUL.SetUpper( static_cast< sal_uInt16 >(
/*N*/ 						nMinTop > aOffst.Y() ? nMinTop : aOffst.Y() ) );
/*N*/ 		aUL.SetLower( static_cast< sal_uInt16 >(
/*N*/ 						nMinBottom > aOffst.Y() ? nMinBottom : aOffst.Y() ));
/*N*/ 		aLR.SetRight( nMinRight > aOffst.X() ? nMinRight : aOffst.X() );
/*N*/ 		aLR.SetLeft(  nMinLeft > aOffst.X() ? nMinLeft : aOffst.X());
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aFrmSize.SetWidth( LONG_MAX );
/*N*/ 		aFrmSize.SetHeight( LONG_MAX );
/*N*/ 		aUL.SetUpper( 0 );
/*N*/ 		aUL.SetLower( 0 );
/*N*/ 		aLR.SetRight( 0 );
/*N*/ 		aLR.SetLeft(  0 );
/*N*/ 	}
/*N*/
/*N*/ 	if ( bSetFmt1 )
/*N*/ 	{
/*N*/ 		rFmt1.SetAttr( aFrmSize );
/*N*/ 		rFmt1.SetAttr( aLR );
/*N*/ 		rFmt1.SetAttr( aUL );
/*N*/ 	}
/*N*/ 	if ( bSetFmt2 )
/*N*/ 	{
/*N*/ 		rFmt2.SetAttr( aFrmSize );
/*N*/ 		rFmt2.SetAttr( aLR );
/*N*/ 		rFmt2.SetAttr( aUL );
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwDoc::ChgPageDesc()
|*
|*	Ersterstellung		MA 25. Jan. 93
|*	Letzte Aenderung	MA 01. Mar. 95
|*
|*************************************************************************/

/*N*/ void lcl_DescSetAttr( const SwFrmFmt &rSource, SwFrmFmt &rDest,
/*N*/ 						 const BOOL bPage = TRUE )
/*N*/ {
/*N*/ /////////////// !!!!!!!!!!!!!!!!
/*N*/ //JP 03.03.99:
/*N*/ // eigentlich sollte hier das Intersect von ItemSet benutzt werden, aber das
/*N*/ // funktioniert nicht richtig, wenn man unterschiedliche WhichRanges hat.
/*N*/ /////////////// !!!!!!!!!!!!!!!!
/*N*/ 	//Die interressanten Attribute uebernehmen.
/*N*/ 	USHORT __READONLY_DATA aIdArr[] = { RES_FRM_SIZE, RES_UL_SPACE,
/*N*/ 										RES_BACKGROUND, RES_SHADOW,
/*N*/ 										RES_COL, RES_COL,
/*N*/ 										RES_FRAMEDIR, RES_FRAMEDIR,
/*N*/                                         RES_TEXTGRID, RES_TEXTGRID,
/*N*/                                         RES_UNKNOWNATR_CONTAINER,
/*N*/ 												RES_UNKNOWNATR_CONTAINER,
/*N*/ 										0 };
/*N*/ 	const SfxPoolItem* pItem;
/*N*/ 	for( USHORT n = 0; aIdArr[ n ]; n += 2 )
/*N*/ 	{
/*N*/ 		for( USHORT nId = aIdArr[ n ]; nId <= aIdArr[ n+1]; ++nId )
/*N*/ 		{
/*N*/ 			if( bPage || ( RES_COL != nId && RES_PAPER_BIN != nId ))
/*N*/ 			{
/*N*/ 				if( SFX_ITEM_SET == rSource.GetItemState( nId, FALSE, &pItem ))
/*N*/ 					rDest.SetAttr( *pItem );
/*N*/ 				else
/*N*/ 					rDest.ResetAttr( nId );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	// auch Pool-, Hilfe-Id's uebertragen
/*N*/ 	rDest.SetPoolFmtId( rSource.GetPoolFmtId() );
/*N*/ 	rDest.SetPoolHelpId( rSource.GetPoolHelpId() );
/*N*/ 	rDest.SetPoolHlpFileId( rSource.GetPoolHlpFileId() );
/*N*/ }


/*N*/ void SwDoc::ChgPageDesc( USHORT i, const SwPageDesc &rChged )
/*N*/ {
/*N*/ 	ASSERT( i < aPageDescs.Count(), "PageDescs ueberindiziert." );
/*N*/
/*N*/ 	SwPageDesc *pDesc = aPageDescs[i];
/*N*/
/*N*/ 	//Als erstes wird ggf. gespiegelt.
/*N*/ 	if ( rChged.GetUseOn() == PD_MIRROR )
/*N*/ 		((SwPageDesc&)rChged).Mirror();
/*N*/ 	else
/*N*/ 		//sonst Werte aus Master nach Left uebertragen.
/*N*/ 		::binfilter::lcl_DescSetAttr( ((SwPageDesc&)rChged).GetMaster(),
/*N*/ 					   ((SwPageDesc&)rChged).GetLeft() );
/*N*/
/*N*/ 	//NumType uebernehmen.
/*N*/ 	if( rChged.GetNumType().GetNumberingType() != pDesc->GetNumType().GetNumberingType() )
/*N*/ 	{
/*?*/ 		pDesc->SetNumType( rChged.GetNumType() );
/*?*/ 		// JP 30.03.99: Bug 64121 - den Seitennummernfeldern bescheid sagen,
/*?*/ 		//		das sich das Num-Format geaendert hat
/*?*/ 		GetSysFldType( RES_PAGENUMBERFLD )->UpdateFlds();
/*?*/ 		GetSysFldType( RES_REFPAGEGETFLD )->UpdateFlds();
/*?*/
/*?*/ 		// Wenn sich die Numerierungsart geaendert hat, koennte es QuoVadis/
/*?*/ 		// ErgoSum-Texte geben, die sich auf eine geaenderte Seite beziehen,
/*?*/ 		// deshalb werden die Fussnoten invalidiert
/*?*/ 		SwFtnIdxs& rFtnIdxs = GetFtnIdxs();
/*?*/ 		for( USHORT nPos = 0; nPos < rFtnIdxs.Count(); ++nPos )
/*?*/ 		{
/*?*/ 			SwTxtFtn *pTxtFtn = rFtnIdxs[ nPos ];
/*?*/ 			const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
/*?*/ 			pTxtFtn->SetNumber( rFtn.GetNumber(), &rFtn.GetNumStr());
/*?*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	//Orientierung uebernehmen
/*N*/ 	pDesc->SetLandscape( rChged.GetLandscape() );
/*N*/
/*N*/ 	//Header abgleichen.
/*N*/ 	const SwFmtHeader &rHead = rChged.GetMaster().GetHeader();
/*N*/ 	pDesc->GetMaster().SetAttr( rHead );
/*N*/ 	if ( rChged.IsHeaderShared() || !rHead.IsActive() )
/*N*/ 	{
/*N*/ 		//Left teilt sich den Header mit dem Master.
/*N*/ 		pDesc->GetLeft().SetAttr( pDesc->GetMaster().GetHeader() );
/*N*/ 	}
/*N*/ 	else if ( rHead.IsActive() )
/*N*/ 	{	//Left bekommt einen eigenen Header verpasst wenn das Format nicht
/*N*/ 		//bereits einen hat.
/*N*/ 		//Wenn er bereits einen hat und dieser auf die gleiche Section
/*N*/ 		//wie der Rechte zeigt, so muss er einen eigenen bekommen. Der
/*N*/ 		//Inhalt wird sinnigerweise kopiert.
/*N*/ 		const SwFmtHeader &rLeftHead = pDesc->GetLeft().GetHeader();
/*N*/ 		if ( !rLeftHead.IsActive() )
/*N*/ 		{
/*?*/ 			SwFmtHeader aHead( MakeLayoutFmt( RND_STD_HEADERL ) );
/*?*/ 			pDesc->GetLeft().SetAttr( aHead );
/*?*/ 			//Weitere Attribute (Raender, Umrandung...) uebernehmen.
/*?*/ 			::binfilter::lcl_DescSetAttr( *rHead.GetHeaderFmt(), *aHead.GetHeaderFmt(), FALSE);
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			const SwFrmFmt *pRight = rHead.GetHeaderFmt();
/*N*/ 			const SwFmtCntnt &aRCnt = pRight->GetCntnt();
/*N*/ 			const SwFmtCntnt &aLCnt = rLeftHead.GetHeaderFmt()->GetCntnt();
/*N*/ 			if ( (*aRCnt.GetCntntIdx()) == (*aLCnt.GetCntntIdx()) )
/*N*/ 			{
/*N*/ 				SwFrmFmt *pFmt = new SwFrmFmt( GetAttrPool(), "Header",
/*N*/ 												GetDfltFrmFmt() );
/*N*/ 				::binfilter::lcl_DescSetAttr( *pRight, *pFmt, FALSE );
/*N*/ 				//Der Bereich auf den das rechte Kopfattribut zeigt wird
/*N*/ 				//kopiert und der Index auf den StartNode in das linke
/*N*/ 				//Kopfattribut gehaengt.
/*N*/ 				SwNodeIndex aTmp( GetNodes().GetEndOfAutotext() );
/*N*/ 				SwStartNode* pSttNd = GetNodes().MakeEmptySection( aTmp, SwHeaderStartNode );
/*N*/ 				SwNodeRange aRange( aRCnt.GetCntntIdx()->GetNode(), 0,
/*N*/ 							*aRCnt.GetCntntIdx()->GetNode().EndOfSectionNode() );
/*N*/ 				aTmp = *pSttNd->EndOfSectionNode();
/*N*/ 				GetNodes()._Copy( aRange, aTmp, FALSE );
/*N*/
/*N*/ 				pFmt->SetAttr( SwFmtCntnt( pSttNd ) );
/*N*/ 				pDesc->GetLeft().SetAttr( SwFmtHeader( pFmt ) );
/*N*/ 			}
/*N*/ 			else
/*N*/ 				::binfilter::lcl_DescSetAttr( *pRight,
/*N*/ 							   *(SwFrmFmt*)rLeftHead.GetHeaderFmt(), FALSE );
/*N*/
/*N*/ 		}
/*N*/ 	}
/*N*/ 	pDesc->ChgHeaderShare( rChged.IsHeaderShared() );
/*N*/
/*N*/ 	//Footer abgleichen.
/*N*/ 	const SwFmtFooter &rFoot = rChged.GetMaster().GetFooter();
/*N*/ 	pDesc->GetMaster().SetAttr( rFoot );
/*N*/ 	if ( rChged.IsFooterShared() || !rFoot.IsActive() )
/*N*/ 		//Left teilt sich den Header mit dem Master.
/*N*/ 		pDesc->GetLeft().SetAttr( pDesc->GetMaster().GetFooter() );
/*N*/ 	else if ( rFoot.IsActive() )
/*N*/ 	{	//Left bekommt einen eigenen Footer verpasst wenn das Format nicht
/*?*/ 		//bereits einen hat.
/*?*/ 		//Wenn er bereits einen hat und dieser auf die gleiche Section
/*?*/ 		//wie der Rechte zeigt, so muss er einen eigenen bekommen. Der
/*?*/ 		//Inhalt wird sinnigerweise kopiert.
/*?*/ 		const SwFmtFooter &rLeftFoot = pDesc->GetLeft().GetFooter();
/*?*/ 		if ( !rLeftFoot.IsActive() )
/*?*/ 		{
/*?*/ 			SwFmtFooter aFoot( MakeLayoutFmt( RND_STD_FOOTER ) );
/*?*/ 			pDesc->GetLeft().SetAttr( aFoot );
/*?*/ 			//Weitere Attribute (Raender, Umrandung...) uebernehmen.
/*?*/ 			::binfilter::lcl_DescSetAttr( *rFoot.GetFooterFmt(), *aFoot.GetFooterFmt(), FALSE);
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			const SwFrmFmt *pRight = rFoot.GetFooterFmt();
/*?*/ 			const SwFmtCntnt &aRCnt = pRight->GetCntnt();
/*?*/ 			const SwFmtCntnt &aLCnt = rLeftFoot.GetFooterFmt()->GetCntnt();
/*?*/ 			if ( (*aRCnt.GetCntntIdx()) == (*aLCnt.GetCntntIdx()) )
/*?*/ 			{
/*?*/ 				SwFrmFmt *pFmt = new SwFrmFmt( GetAttrPool(), "Footer",
/*?*/ 												GetDfltFrmFmt() );
/*?*/ 				::binfilter::lcl_DescSetAttr( *pRight, *pFmt, FALSE );
/*?*/ 				//Der Bereich auf den das rechte Kopfattribut zeigt wird
/*?*/ 				//kopiert und der Index auf den StartNode in das linke
/*?*/ 				//Kopfattribut gehaengt.
/*?*/ 				SwNodeIndex aTmp( GetNodes().GetEndOfAutotext() );
/*?*/ 				SwStartNode* pSttNd = GetNodes().MakeEmptySection( aTmp, SwFooterStartNode );
/*?*/ 				SwNodeRange aRange( aRCnt.GetCntntIdx()->GetNode(), 0,
/*?*/ 							*aRCnt.GetCntntIdx()->GetNode().EndOfSectionNode() );
/*?*/ 				aTmp = *pSttNd->EndOfSectionNode();
/*?*/ 				GetNodes()._Copy( aRange, aTmp, FALSE );
/*?*/
/*?*/ 				pFmt->SetAttr( SwFmtCntnt( pSttNd ) );
/*?*/ 				pDesc->GetLeft().SetAttr( SwFmtFooter( pFmt ) );
/*?*/ 			}
/*?*/ 			else
/*?*/ 				::binfilter::lcl_DescSetAttr( *pRight,
/*?*/ 							   *(SwFrmFmt*)rLeftFoot.GetFooterFmt(), FALSE );
/*?*/ 		}
/*N*/ 	}
/*N*/ 	pDesc->ChgFooterShare( rChged.IsFooterShared() );
/*N*/
/*N*/ 	if ( pDesc->GetName() != rChged.GetName() )
/*?*/ 		pDesc->SetName( rChged.GetName() );
/*N*/
/*N*/ 	// Dadurch wird ein RegisterChange ausgeloest, wenn notwendig
/*N*/ 	pDesc->SetRegisterFmtColl( rChged.GetRegisterFmtColl() );
/*N*/
/*N*/ 	//Wenn sich das UseOn oder der Follow aendern muessen die
/*N*/ 	//Absaetze das erfahren.
/*N*/ 	BOOL bUseOn  = FALSE;
/*N*/ 	BOOL bFollow = FALSE;
/*N*/ 	if ( pDesc->GetUseOn() != rChged.GetUseOn() )
/*N*/ 	{   pDesc->SetUseOn( rChged.GetUseOn() );
/*N*/ 		bUseOn = TRUE;
/*N*/ 	}
/*N*/ 	if ( pDesc->GetFollow() != rChged.GetFollow() )
/*N*/ 	{	if ( rChged.GetFollow() == &rChged )
/*N*/ 		{	if ( pDesc->GetFollow() != pDesc )
/*?*/ 			{	pDesc->SetFollow( pDesc );
/*?*/ 				bFollow = TRUE;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{	pDesc->SetFollow( rChged.pFollow );
/*N*/ 			bFollow = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	if ( (bUseOn || bFollow) && GetRootFrm() )
/*N*/ 		//Layot benachrichtigen!
/*N*/ 		GetRootFrm()->CheckPageDescs( (SwPageFrm*)GetRootFrm()->Lower() );
/*N*/
/*N*/ 	//Jetzt noch die Seiten-Attribute uebernehmen.
/*N*/ 	::binfilter::lcl_DescSetAttr( rChged.GetMaster(), pDesc->GetMaster() );
/*N*/ 	::binfilter::lcl_DescSetAttr( rChged.GetLeft(), pDesc->GetLeft() );
/*N*/
/*N*/ 	//Wenn sich FussnotenInfo veraendert, so werden die Seiten
/*N*/ 	//angetriggert.
/*N*/ 	if( !(pDesc->GetFtnInfo() == rChged.GetFtnInfo()) )
/*N*/ 	{
/*?*/ 		pDesc->SetFtnInfo( rChged.GetFtnInfo() );
/*?*/ 		SwMsgPoolItem  aInfo( RES_PAGEDESC_FTNINFO );
/*?*/ 		{
/*?*/ 			SwClientIter aIter( pDesc->GetMaster() );
/*?*/ 			for( SwClient* pLast = aIter.First(TYPE(SwFrm)); pLast;
/*?*/ 					pLast = aIter.Next() )
/*?*/ 				pLast->Modify( &aInfo, 0 );
/*?*/ 		}
/*?*/ 		{
/*?*/ 			SwClientIter aIter( pDesc->GetLeft() );
/*?*/ 			for( SwClient* pLast = aIter.First(TYPE(SwFrm)); pLast;
/*?*/ 					pLast = aIter.Next() )
/*?*/ 				pLast->Modify( &aInfo, 0 );
/*?*/ 		}
/*N*/ 	}
/*N*/ 	SetModified();
/*N*/ }

/*************************************************************************
|*
|*	SwDoc::DelPageDesc()
|*
|* 	Beschreibung		Alle Descriptoren, deren Follow auf den zu loeschenden
|*		zeigen muessen angepasst werden.
|*	Ersterstellung		MA 25. Jan. 93
|*	Letzte Aenderung	JP 04.09.95
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

        ::binfilter::lcl_RemoveFrms( pDel->GetMaster(), bFtnsRemoved );
        ::binfilter::lcl_RemoveFrms( pDel->GetLeft(), bFtnsRemoved );
    }

    aPageDescs.Remove( i );
    delete pDel;
    SetModified();
}



/*************************************************************************
|*
|*	SwDoc::MakePageDesc()
|*
|*	Ersterstellung		MA 25. Jan. 93
|*	Letzte Aenderung	MA 20. Aug. 93
|*
|*************************************************************************/

/*N*/ USHORT SwDoc::MakePageDesc( const String &rName, const SwPageDesc *pCpy,
/*N*/                             BOOL bRegardLanguage)
/*N*/ {
/*N*/ 	SwPageDesc *pNew;
/*N*/ 	if( pCpy )
/*N*/ 	{
/*?*/ 		pNew = new SwPageDesc( *pCpy );
/*?*/ 		pNew->SetName( rName );
/*?*/ 		if( rName != pCpy->GetName() )
/*?*/ 		{
/*?*/ 			pNew->SetPoolFmtId( USHRT_MAX );
/*?*/ 			pNew->SetPoolHelpId( USHRT_MAX );
/*?*/ 			pNew->SetPoolHlpFileId( UCHAR_MAX );
/*?*/ 		}
/*?*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pNew = new SwPageDesc( rName, GetDfltFrmFmt(), this );
/*N*/ 		//Default-Seitenformat einstellen.
/*N*/ 		::binfilter::lcl_DefaultPageFmt( USHRT_MAX, pNew->GetMaster(), pNew->GetLeft(),
/*N*/ 							  GetPrt(), FALSE );
/*N*/
/*N*/         SvxFrameDirection aFrameDirection = bRegardLanguage ?
/*N*/             GetDefaultFrameDirection(GetAppLanguage())
/*N*/             : FRMDIR_HORI_LEFT_TOP;
/*N*/
/*N*/ 		pNew->GetMaster().SetAttr( SvxFrameDirectionItem(aFrameDirection) );
/*N*/ 		pNew->GetLeft().SetAttr( SvxFrameDirectionItem(aFrameDirection) );
/*N*/
/*N*/     	if( GetPrt() )
/*N*/ 			pNew->SetLandscape( ORIENTATION_LANDSCAPE ==
/*N*/ 								GetPrt()->GetOrientation() );
/*N*/ 	}
/*N*/ 	aPageDescs.Insert( pNew, aPageDescs.Count() );
/*N*/ 	SetModified();
/*N*/ 	return (aPageDescs.Count()-1);
/*N*/ }

/*N*/ SwPageDesc* SwDoc::FindPageDescByName( const String& rName, USHORT* pPos ) const
/*N*/ {
/*N*/ 	SwPageDesc* pRet = 0;
/*N*/ 	if( pPos ) *pPos = USHRT_MAX;
/*N*/
/*N*/ 	for( USHORT n = 0, nEnd = aPageDescs.Count(); n < nEnd; ++n )
/*N*/ 		if( aPageDescs[ n ]->GetName() == rName )
/*N*/ 		{
/*N*/ 			pRet = aPageDescs[ n ];
/*N*/ 			if( pPos )
/*N*/ 				*pPos = n;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	return pRet;
/*N*/ }

/******************************************************************************
 *	Methode		:	void SwDoc::SetPrt( SfxPrinter *pP )
 *	Beschreibung:
 *	Erstellt	:	OK 27.10.94 10:20
 *	Aenderung	:	MA 26. Mar. 98
 ******************************************************************************/

/*N*/ void SwDoc::PrtDataChanged()
/*N*/ {
/*N*/ //!!!!!!!! Bei Aenderungen hier bitte ggf. InJobSetup im Sw3io mitpflegen
/*N*/
/*N*/ 	BOOL bEndAction = FALSE;
/*N*/
/*N*/ 	if( GetDocShell() )
/*N*/ 		GetDocShell()->UpdateFontList();
/*N*/
/*N*/ 	BOOL bDraw = TRUE;
/*N*/ 	if ( GetRootFrm() )
/*N*/ 	{
/*?*/ 		ViewShell *pSh = GetRootFrm()->GetCurrShell();
/*?*/ 		if( !IsBrowseMode() || ( pSh && pSh->GetViewOptions()->IsPrtFormat() ) )
/*?*/ 		{
/*?*/ 			GetRootFrm()->StartAllAction();
/*?*/ 			bEndAction = TRUE;
/*?*/
/*?*/ 			bDraw = FALSE;
/*?*/ 			if( pDrawModel )
/*?*/ 			    pDrawModel->SetRefDevice( _GetRefDev() );
/*?*/
/*?*/ 			pFntCache->Flush();
/*?*/ 			GetRootFrm()->InvalidateAllCntnt();
/*?*/
/*?*/ 			if ( pSh )
/*?*/ 			{
/*?*/ 				do
/*?*/ 			                {
/*?*/                     pSh->InitPrt( pPrt );
/*?*/ 					pSh = (ViewShell*)pSh->GetNext();
/*?*/                 }
/*?*/                 while ( pSh != GetRootFrm()->GetCurrShell() );
/*?*/ 			}
/*?*/
/*?*/ 		}
/*N*/ 	}
/*N*/     if ( bDraw && pDrawModel && _GetRefDev() != pDrawModel->GetRefDevice() )
/*N*/         pDrawModel->SetRefDevice( _GetRefDev() );
/*N*/
/*N*/ 	PrtOLENotify( TRUE );
/*N*/
/*N*/ 	if ( bEndAction )
/*?*/ 		GetRootFrm()->EndAllAction();
/*N*/ }

//Zur Laufzeit sammeln wir die GlobalNames der Server, die keine
//Benachrichtigung zu Druckerwechseln wuenschen. Dadurch sparen wir
//das Laden vieler Objekte (gluecklicherweise werden obendrein alle
//Fremdobjekte unter einer ID abgebuildet). Init und DeInit vom Array
//ist in init.cxx zu finden.
extern SvPtrarr *pGlobalOLEExcludeList;

/*N*/ void SwDoc::PrtOLENotify( BOOL bAll )
/*N*/ {
/*N*/ 	SwFEShell *pShell = 0;
/*N*/ 	if ( GetRootFrm() && GetRootFrm()->GetCurrShell() )
/*N*/ 	{
/*N*/ 		ViewShell *pSh = GetRootFrm()->GetCurrShell();
/*N*/ 		if ( !pSh->ISA(SwFEShell) )
/*N*/ 			do
/*?*/ 			{	pSh = (ViewShell*)pSh->GetNext();
/*?*/ 			} while ( !pSh->ISA(SwFEShell) &&
/*?*/ 					  pSh != GetRootFrm()->GetCurrShell() );
/*N*/
/*N*/ 		if ( pSh->ISA(SwFEShell) )
/*N*/ 			pShell = (SwFEShell*)pSh;
/*N*/ 	}
/*N*/ 	if ( !pShell )
/*N*/ 	{
/*N*/ 		//Das hat ohne Shell und damit ohne Client keinen Sinn, weil nur darueber
/*N*/ 		//die Kommunikation bezueglich der Groessenaenderung implementiert ist.
/*N*/ 		//Da wir keine Shell haben, merken wir uns diesen unguenstigen
/*N*/ 		//Zustand am Dokument, dies wird dann beim Erzeugen der ersten Shell
/*N*/ 		//nachgeholt.
/*N*/ 		bOLEPrtNotifyPending = TRUE;
/*N*/ 		if ( bAll )
/*N*/ 			bAllOLENotify = TRUE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if ( bAllOLENotify )
/*N*/ 			bAll = TRUE;
/*N*/
/*N*/ 		bOLEPrtNotifyPending = bAllOLENotify = FALSE;
/*N*/
/*N*/
/*N*/ 		SwOLENodes *pNodes = 0;
/*N*/ 		SwClientIter aIter( *(SwModify*)GetDfltGrfFmtColl() );
/*N*/ 		for( SwCntntNode* pNd = (SwCntntNode*)aIter.First( TYPE( SwCntntNode ) );
/*N*/ 			 pNd;
/*N*/ 			 pNd = (SwCntntNode*)aIter.Next() )
/*N*/ 		{
/*N*/ 			SwOLENode *pONd;
/*N*/ 			if ( 0 != (pONd = pNd->GetOLENode()) &&
/*N*/ 				 (bAll || pONd->IsOLESizeInvalid()) )
/*N*/ 			{
/*N*/ 				if ( !pNodes  )
/*N*/ 					pNodes = new SwOLENodes;
/*N*/ 				pNodes->Insert( pONd, pNodes->Count() );
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		if ( pNodes )
/*N*/ 		{
/*N*/ 			::binfilter::StartProgress( STR_STATSTR_SWGPRTOLENOTIFY,
/*N*/ 							 0, pNodes->Count(), GetDocShell());
/*N*/ 			GetRootFrm()->StartAllAction();
/*N*/
/*N*/ 			for( USHORT i = 0; i < pNodes->Count(); ++i )
/*N*/ 			{
/*N*/ 				::binfilter::SetProgressState( i, GetDocShell() );
/*N*/
/*N*/ 				SwOLENode* pOLENd = (*pNodes)[i];
/*N*/ 				pOLENd->SetOLESizeInvalid( FALSE );
/*N*/
/*N*/ 				//Ersteinmal die Infos laden und festellen ob das Teil nicht
/*N*/ 				//schon in der Exclude-Liste steht
/*N*/ 				SvGlobalName aName;
/*N*/
/*N*/ 				if ( !pOLENd->GetOLEObj().IsOleRef() )	//Noch nicht geladen
/*N*/ 				{
/*N*/ 					String sBaseURL( ::binfilter::StaticBaseUrl::GetBaseURL() );
/*N*/ 					const SfxMedium *pMedium;
/*N*/ 					if( 0 != (pMedium = GetDocShell()->GetMedium()) &&
/*N*/ 						pMedium->GetName() != sBaseURL )
/*N*/ 						::binfilter::StaticBaseUrl::SetBaseURL( pMedium->GetName() );
/*N*/ 					SvInfoObjectRef xInfo = GetPersist()->Find( pOLENd->GetOLEObj().GetName() );
/*N*/ 					if ( xInfo.Is() )	//Muss normalerweise gefunden werden
/*N*/ 						aName = xInfo->GetClassName();
/*N*/ 					::binfilter::StaticBaseUrl::SetBaseURL( sBaseURL );
/*N*/ 				}
/*N*/ 				else
/*?*/ 					aName = pOLENd->GetOLEObj().GetOleRef()->GetClassName();
/*N*/
/*N*/ 				BOOL bFound = FALSE;
/*N*/ 				for ( USHORT i = 0;
/*N*/ 					  i < pGlobalOLEExcludeList->Count() && !bFound;
/*N*/ 					  ++i )
/*N*/ 				{
/*N*/ 					bFound = *(SvGlobalName*)(*pGlobalOLEExcludeList)[i] ==
/*N*/ 									aName;
/*N*/ 				}
/*N*/ 				if ( bFound )
/*N*/ 					continue;
/*N*/
/*N*/ 				//Kennen wir nicht, also muss das Objekt geladen werden.
/*N*/ 				//Wenn es keine Benachrichtigung wuenscht
/*N*/ 				SvEmbeddedObjectRef xRef( (SvInPlaceObject*) pOLENd->GetOLEObj().GetOleRef() );
/*N*/ 				if ( xRef ) //Kaputt?
/*N*/ 				{
/*N*/ 					if ( SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE & xRef->GetMiscStatus())
/*N*/ 					{
/*N*/ 						if ( pOLENd->GetFrm() )
/*N*/ 						{
/*N*/ 							xRef->OnDocumentPrinterChanged( pPrt );
/*N*/ 							pShell->CalcAndSetScale( xRef );//Client erzeugen lassen.
/*N*/ 						}
/*N*/ 						else
/*N*/ 							pOLENd->SetOLESizeInvalid( TRUE );
/*N*/ 					}
/*N*/ 					else
/*N*/ 						pGlobalOLEExcludeList->Insert(
/*N*/ 								new SvGlobalName( xRef->GetClassName()),
/*N*/ 								pGlobalOLEExcludeList->Count() );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			delete pNodes;
/*N*/ 			GetRootFrm()->EndAllAction();
/*N*/ 			::binfilter::EndProgress( GetDocShell() );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SwDoc::SetVirDev( VirtualDevice* pVd, sal_Bool bCallVirDevDataChanged )
/*N*/ {
/*N*/     if ( (ULONG)pVirDev != (ULONG)pVd )
/*N*/     {
/*N*/         delete pVirDev;
/*N*/         pVirDev = pVd;
/*N*/     }
/*N*/ }

/*N*/ void SwDoc::SetPrt( SfxPrinter *pP, sal_Bool bCallPrtDataChanged )
/*N*/ {
/*N*/ 	ASSERT( pP, "Kein Drucker!" );
/*N*/
/*N*/ 	const BOOL bInitPageDesc = pPrt == 0;
/*N*/
/*N*/ 	if ( (ULONG) pP != (ULONG) pPrt)
/*N*/ 	{
/*N*/ 		delete pPrt;
/*N*/ 		pPrt = pP;
/*N*/ 	}
/*N*/     // OD 05.03.2003 #107870# - first adjust page description, before trigger formatting.
/*N*/     if( bInitPageDesc )
/*N*/     {
/*N*/         // JP 17.04.97: Bug 38924 - falls noch kein Drucker gesetzt war
/*N*/         //              und der PageDesc nicht eingelesen wurde
/*N*/         //      -> passe an den Drucker an
/*N*/         if( pPrt && LONG_MAX == _GetPageDesc( 0 ).GetMaster().GetFrmSize().GetWidth() )
/*N*/             _GetPageDesc( 0 ).SetLandscape( ORIENTATION_LANDSCAPE ==
/*N*/                                             pPrt->GetOrientation() );
/*N*/
/*N*/         //MA 11. Mar. 97: Das sollten wir fuer alle Formate tun, weil die
/*N*/         //Werte auf LONG_MAX initalisiert sind (spaetes anlegen des Druckers)
/*N*/         //und die Reader u.U. "unfertige" Formate stehenlassen.
/*N*/         for ( USHORT i = 0; i < GetPageDescCnt(); ++i )
/*N*/         {
/*N*/             SwPageDesc& rDesc = _GetPageDesc( i );
/*N*/             ::binfilter::lcl_DefaultPageFmt( rDesc.GetPoolFmtId(), rDesc.GetMaster(),
/*N*/                                   rDesc.GetLeft(), pPrt, TRUE );
/*N*/         }
/*N*/     }
/*N*/
/*N*/     if ( bCallPrtDataChanged )
/*N*/ 		PrtDataChanged();
/*N*/ }

/*N*/ void SwDoc::SetUseVirtualDevice( sal_Bool bFlag )
/*N*/ {
/*N*/     if ( !IsUseVirtualDevice() != !bFlag )
/*N*/     {
/*N*/         _SetUseVirtualDevice( bFlag );
/*N*/         PrtDataChanged();
/*N*/         SetModified();
/*N*/     }
/*N*/
/*N*/ }

/*
 *	Kleiner Hack;
 *
const SwPageDesc& SwDoc::GetPageDesc( USHORT i ) const
{
    if( !i && !aPageDescs.Count() )            // noch keiner vorhanden?
        ((SwDoc*)this)->InitPageDescs();		//Default PageDescriptor
    return *aPageDescs[i];
}

SwPageDesc& SwDoc::_GetPageDesc( USHORT i ) const
{
    if( !i && !aPageDescs.Count() )			// noch keiner vorhanden?
        ((SwDoc*)this)->InitPageDescs();		//Default PageDescriptor
    return *aPageDescs[i];
}
*/



/*N*/ IMPL_LINK( SwDoc, DoUpdateModifiedOLE, Timer *, pTimer )
/*N*/ {
/*N*/ 	SwFEShell* pSh = (SwFEShell*)GetEditShell();
/*N*/ 	if( pSh )
/*N*/ 	{
/*N*/ 		bOLEPrtNotifyPending = bAllOLENotify = FALSE;
/*N*/
/*N*/ 		SwOLENodes aOLENodes;
/*N*/ 		SwClientIter aIter( *(SwModify*)GetDfltGrfFmtColl() );
/*N*/ 		for( SwCntntNode* pNd = (SwCntntNode*)aIter.First( TYPE( SwCntntNode ) );
/*N*/ 			 pNd;
/*N*/ 			 pNd = (SwCntntNode*)aIter.Next() )
/*N*/ 		{
/*N*/ 			SwOLENode *pONd = pNd->GetOLENode();
/*N*/ 			if( pONd && pONd->IsOLESizeInvalid() )
/*N*/ 			{
/*N*/ 				aOLENodes.Insert( pONd, aOLENodes.Count() );
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		if( aOLENodes.Count() )
/*N*/ 		{
/*N*/ 			::binfilter::StartProgress( STR_STATSTR_SWGPRTOLENOTIFY,
/*N*/ 							 0, aOLENodes.Count(), GetDocShell());
/*N*/ 			GetRootFrm()->StartAllAction();
/*N*/ 			SwMsgPoolItem aMsgHint( RES_UPDATE_ATTR );
/*N*/
/*N*/ 			for( USHORT i = 0; i < aOLENodes.Count(); ++i )
/*N*/ 			{
/*N*/ 				::binfilter::SetProgressState( i, GetDocShell() );
/*N*/
/*N*/ 				SwOLENode* pOLENd = aOLENodes[i];
/*N*/ 				pOLENd->SetOLESizeInvalid( FALSE );
/*N*/
/*N*/ 				//Kennen wir nicht, also muss das Objekt geladen werden.
/*N*/ 				//Wenn es keine Benachrichtigung wuenscht
/*N*/ 				SvEmbeddedObjectRef xRef( (SvInPlaceObject*)
/*N*/ 										pOLENd->GetOLEObj().GetOleRef() );
/*N*/ 				if( xRef ) //Kaputt?
/*N*/ 				{
/*N*/ 					if( SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE &
/*N*/ 							xRef->GetMiscStatus() )
/*N*/ 					{
/*N*/ 						if( pOLENd->GetFrm() )
/*N*/ 						{
/*N*/ 							xRef->OnDocumentPrinterChanged( pPrt );
/*N*/ 							pSh->CalcAndSetScale( xRef );//Client erzeugen lassen.
/*N*/ 						}
/*N*/ 						else
/*?*/ 							pOLENd->SetOLESizeInvalid( TRUE );
/*N*/ 					}
/*N*/ 					// repaint it
/*N*/ 					pOLENd->Modify( &aMsgHint, &aMsgHint );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			GetRootFrm()->EndAllAction();
/*N*/ 			::binfilter::EndProgress( GetDocShell() );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
