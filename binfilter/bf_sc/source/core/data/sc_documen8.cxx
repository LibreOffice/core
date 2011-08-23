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

#include "scitems.hxx"
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <bf_svx/frmdiritem.hxx>
#include <bf_svx/langitem.hxx>
#include <bf_svx/linkmgr.hxx>
#include <bf_sfx2/printer.hxx>
#include <bf_svtools/flagitem.hxx>
#define _SVSTDARR_USHORTS
#include <bf_svtools/zformat.hxx>
#include <bf_sfx2/misccfg.hxx>
#include <bf_sfx2/app.hxx>


#include "poolhelp.hxx"
#include "docpool.hxx"
#include "stlpool.hxx"
#include "docoptio.hxx"
#include "viewopti.hxx"
#include "rechead.hxx"
#include "ddelink.hxx"
#include "scmatrix.hxx"
#include "arealink.hxx"
#include "patattr.hxx"
#include "editutil.hxx"
#include "document.hxx"
#include "refupdat.hxx"
#include "scmod.hxx"
#include "globstr.hrc"
#include "bf_sc.hrc"
namespace binfilter {

#define GET_SCALEVALUE(set,id) 	((const SfxUInt16Item&)(set.Get( id ))).GetValue()

//	states for online spelling in the visible range (0 is set initially)
#define VSPL_START	0
#define VSPL_DONE	1


// STATIC DATA -----------------------------------------------------------



//------------------------------------------------------------------------

/*N*/ void ScDocument::ImplLoadDocOptions( SvStream& rStream )
/*N*/ {
/*N*/ 	USHORT d,m,y;
/*N*/ 
/*N*/ 	DBG_ASSERT( pDocOptions, "No DocOptions to load! :-(" );
/*N*/ 
/*N*/ 	pDocOptions->Load( rStream );
/*N*/ 
/*N*/ 	if ( pDocOptions->GetStdPrecision() > 20 ) //!!! ist 20 als Maximum konstant ???
/*N*/ 	{
/*?*/ 		DBG_ERROR( "Document options corrupted. Setting to defaults." );
/*?*/ 		pDocOptions->ResetDocOptions();
/*N*/ 	}
/*N*/ 
/*N*/ 	pDocOptions->GetDate( d,m,y );
/*N*/ 	SvNumberFormatter* pFormatter = xPoolHelper->GetFormTable();
/*N*/ 	pFormatter->ChangeNullDate( d,m,y );
/*N*/ 	pFormatter->ChangeStandardPrec( pDocOptions->GetStdPrecision() );
/*N*/ 	pFormatter->SetYear2000( pDocOptions->GetYear2000() );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void ScDocument::ImplLoadViewOptions( SvStream& rStream )
/*N*/ {
/*N*/ 	DBG_ASSERT( pViewOptions, "No ViewOptions to load! :-(" );
/*N*/ 	rStream >> *pViewOptions;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void ScDocument::ImplSaveDocOptions( SvStream& rStream ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( pDocOptions, "No DocOptions to save! :-(" );
/*N*/ 	pDocOptions->Save( rStream );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void ScDocument::ImplSaveViewOptions( SvStream& rStream ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( pViewOptions, "No ViewOptions to save! :-(" );
/*N*/ 	rStream << *pViewOptions;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void ScDocument::ImplCreateOptions()
/*N*/ {
/*N*/ 	pDocOptions  = new ScDocOptions();
/*N*/ 	pViewOptions = new ScViewOptions();
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void ScDocument::ImplDeleteOptions()
/*N*/ {
/*N*/ 	delete pDocOptions;
/*N*/ 	delete pViewOptions;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ SfxPrinter* ScDocument::GetPrinter()
/*N*/ {
/*N*/ 	if ( !pPrinter )
/*N*/ 	{
/*N*/ 		SfxItemSet* pSet =
/*N*/ 			new SfxItemSet( *xPoolHelper->GetDocPool(),
/*N*/ 							SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
/*N*/ 							SID_PRINTER_CHANGESTODOC,  SID_PRINTER_CHANGESTODOC,
/*N*/ 							SID_SCPRINTOPTIONS,        SID_SCPRINTOPTIONS,
/*N*/ 							NULL );
/*N*/ 
/*N*/         SfxMiscCfg* pOffCfg = SFX_APP()->GetMiscConfig();
/*N*/ 		if ( pOffCfg )
/*N*/ 		{
/*N*/ 			USHORT nFlags = 0;
/*N*/             if ( pOffCfg->IsPaperOrientationWarning() )
/*N*/ 				nFlags |= SFX_PRINTER_CHG_ORIENTATION;
/*N*/ 			if ( pOffCfg->IsPaperSizeWarning() )
/*N*/ 				nFlags |= SFX_PRINTER_CHG_SIZE;
/*N*/ 			pSet->Put( SfxFlagItem( SID_PRINTER_CHANGESTODOC, nFlags ) );
/*N*/ 			pSet->Put( SfxBoolItem( SID_PRINTER_NOTFOUND_WARN, pOffCfg->IsNotFoundWarning() ) );
/*N*/ 		}
/*N*/ 
/*N*/ 		pPrinter = new SfxPrinter( pSet );
/*N*/ 		UpdateDrawPrinter();
/*N*/ 		pPrinter->SetDigitLanguage( SC_MOD()->GetOptDigitLanguage() );
/*N*/ 	}
/*N*/ 
/*N*/ 	return pPrinter;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void ScDocument::SetPrinter( SfxPrinter* pNewPrinter )
/*N*/ {
/*N*/ 	if ( pNewPrinter == pPrinter )
/*N*/ 	{
/*N*/ 		//	#i6706# SetPrinter is called with the same printer again if
/*N*/ 		//	the JobSetup has changed. In that case just call UpdateDrawPrinter
/*N*/ 		//	(SetRefDevice for drawing layer) because of changed text sizes.
/*N*/ 		UpdateDrawPrinter();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SfxPrinter* pOld = pPrinter;
/*N*/ 		pPrinter = pNewPrinter;
/*N*/ 		UpdateDrawPrinter();
/*N*/ 		pPrinter->SetDigitLanguage( SC_MOD()->GetOptDigitLanguage() );
/*N*/ 		delete pOld;
/*N*/ 	}
/*N*/ 	InvalidateTextWidth();		// in both cases
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void ScDocument::SetPrintOptions()
/*N*/ {
/*N*/ 	if ( !pPrinter ) GetPrinter(); // setzt pPrinter
/*N*/ 	DBG_ASSERT( pPrinter, "Error in printer creation :-/" );
/*N*/ 
/*N*/ 	if ( pPrinter )
/*N*/ 	{
/*N*/         SfxMiscCfg* pOffCfg = SFX_APP()->GetMiscConfig();
/*N*/ 		if ( pOffCfg )
/*N*/ 		{
/*N*/ 			SfxItemSet aOptSet( pPrinter->GetOptions() );
/*N*/ 
/*N*/ 			USHORT nFlags = 0;
/*N*/             if ( pOffCfg->IsPaperOrientationWarning() )
/*N*/ 				nFlags |= SFX_PRINTER_CHG_ORIENTATION;
/*N*/ 			if ( pOffCfg->IsPaperSizeWarning() )
/*N*/ 				nFlags |= SFX_PRINTER_CHG_SIZE;
/*N*/ 			aOptSet.Put( SfxFlagItem( SID_PRINTER_CHANGESTODOC, nFlags ) );
/*N*/ 			aOptSet.Put( SfxBoolItem( SID_PRINTER_NOTFOUND_WARN, pOffCfg->IsNotFoundWarning() ) );
/*N*/ 
/*N*/ 			pPrinter->SetOptions( aOptSet );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ BOOL ScDocument::RemovePageStyleInUse( const String& rStyle )
/*N*/ {
/*N*/ 	BOOL bWasInUse = FALSE;
/*N*/ 	const USHORT nCount = GetTableCount();
/*N*/ 
/*N*/ 	for ( USHORT i=0; i<nCount && pTab[i]; i++ )
/*N*/ 		if ( pTab[i]->GetPageStyle() == rStyle )
/*N*/ 		{
/*N*/ 			bWasInUse = TRUE;
/*N*/ 			pTab[i]->SetPageStyle( ScGlobal::GetRscString(STR_STYLENAME_STANDARD) );
/*N*/ 		}
/*N*/ 
/*N*/ 	return bWasInUse;
/*N*/ }


//------------------------------------------------------------------------

/*M*/ BYTE ScDocument::GetEditTextDirection(USHORT nTab) const
/*M*/ {
/*M*/ 	EEHorizontalTextDirection eRet = EE_HTEXTDIR_DEFAULT;
/*M*/ 
/*M*/ 	String aStyleName = GetPageStyle( nTab );
/*M*/ 	SfxStyleSheetBase* pStyle = xPoolHelper->GetStylePool()->Find( aStyleName, SFX_STYLE_FAMILY_PAGE );
/*M*/ 	if ( pStyle )
/*M*/ 	{
/*M*/ 		SfxItemSet& rStyleSet = pStyle->GetItemSet();
/*M*/ 		SvxFrameDirection eDirection = (SvxFrameDirection)
/*M*/ 			((const SvxFrameDirectionItem&)rStyleSet.Get( ATTR_WRITINGDIR )).GetValue();
/*M*/ 
/*M*/ 		if ( eDirection == FRMDIR_HORI_LEFT_TOP )
/*M*/ 			eRet = EE_HTEXTDIR_L2R;
/*M*/ 		else if ( eDirection == FRMDIR_HORI_RIGHT_TOP )
/*M*/ 			eRet = EE_HTEXTDIR_R2L;
/*M*/ 		// else (invalid for EditEngine): keep "default"
/*M*/ 	}
/*M*/ 
/*M*/ 	return eRet;
/*M*/ }

//------------------------------------------------------------------------

/*N*/ void ScDocument::InvalidateTextWidth( const ScAddress* pAdrFrom,
/*N*/ 									  const ScAddress* pAdrTo,
/*N*/ 									  BOOL bBroadcast )
/*N*/ {
/*N*/ 	bBroadcast = (bBroadcast && GetDocOptions().IsCalcAsShown() && !IsImportingXML());
/*N*/ 	if ( pAdrFrom && !pAdrTo )
/*N*/ 	{
/*?*/ 		const USHORT nTab = pAdrFrom->Tab();
/*?*/ 
/*?*/ 		if ( pTab[nTab] )
/*?*/ 			pTab[nTab]->InvalidateTextWidth( pAdrFrom, NULL, bBroadcast );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		const USHORT nTabStart = pAdrFrom ? pAdrFrom->Tab() : 0;
/*N*/ 		const USHORT nTabEnd   = pAdrTo   ? pAdrTo->Tab()   : MAXTAB;
/*N*/ 
/*N*/ 		for ( USHORT nTab=nTabStart; nTab<=nTabEnd; nTab++ )
/*N*/ 			if ( pTab[nTab] )
/*N*/ 				pTab[nTab]->InvalidateTextWidth( pAdrFrom, pAdrTo, bBroadcast );
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------

#define CALCMAX					1000	// Berechnungen
#define ABORT_EVENTS			(INPUT_ANY & ~INPUT_TIMER & ~INPUT_OTHER)

//------------------------------------------------------------------------


//	SPELL_MAXCELLS muss mindestens 256 sein, solange am Iterator keine
//	Start-Spalte gesetzt werden kann

//!	SPELL_MAXTEST fuer Timer und Idle unterschiedlich ???

//	SPELL_MAXTEST now divided between visible and rest of document

#define SPELL_MAXTEST_VIS	1
#define SPELL_MAXTEST_ALL	3
#define SPELL_MAXCELLS		256

//------------------------------------------------------------------------

/*N*/ void ScDocument::SaveDdeLinks(SvStream& rStream) const
/*N*/ {
/*N*/ 	//	bei 4.0-Export alle mit Modus != DEFAULT weglassen
/*N*/ 	BOOL bExport40 = ( rStream.GetVersion() <= SOFFICE_FILEFORMAT_40 );
/*N*/ 
/*N*/ 	const ::binfilter::SvBaseLinks& rLinks = pLinkManager->GetLinks();
/*N*/ 	USHORT nCount = rLinks.Count();
/*N*/ 
/*N*/ 	//	erstmal zaehlen...
/*N*/ 
/*N*/ 	USHORT nDdeCount = 0;
/*N*/ 	USHORT i;
/*N*/ 	for (i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		::binfilter::SvBaseLink* pBase = *rLinks[i];
/*N*/ 		if (pBase->ISA(ScDdeLink))
/*?*/ 			if ( !bExport40 || ((ScDdeLink*)pBase)->GetMode() == SC_DDE_DEFAULT )
/*?*/ 				++nDdeCount;
/*N*/ 	}
/*N*/ 
/*N*/ 	//	Header
/*N*/ 
/*N*/ 	ScMultipleWriteHeader aHdr( rStream );
/*N*/ 	rStream << nDdeCount;
/*N*/ 
/*N*/ 	//	Links speichern
/*N*/ 
/*N*/ 	for (i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		::binfilter::SvBaseLink* pBase = *rLinks[i];
/*N*/ 		if (pBase->ISA(ScDdeLink))
/*N*/ 		{
/*?*/ 			ScDdeLink* pLink = (ScDdeLink*)pBase;
/*?*/ 			if ( !bExport40 || pLink->GetMode() == SC_DDE_DEFAULT )
/*?*/ 				pLink->Store( rStream, aHdr );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocument::LoadDdeLinks(SvStream& rStream)
/*N*/ {
/*N*/ 	ScMultipleReadHeader aHdr( rStream );
/*N*/ 
/*N*/ 	USHORT nCount;
/*N*/ 	rStream >> nCount;
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 	{
/*?*/ 		ScDdeLink* pLink = new ScDdeLink( this, rStream, aHdr );
/*?*/ 		pLinkManager->InsertDDELink( pLink,
/*?*/ 							pLink->GetAppl(), pLink->GetTopic(), pLink->GetItem() );
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocument::SetInLinkUpdate(BOOL bSet)
/*N*/ {
/*N*/ 	//	called from TableLink and AreaLink
/*N*/ 
/*N*/ 	DBG_ASSERT( bInLinkUpdate != bSet, "SetInLinkUpdate twice" );
/*N*/ 	bInLinkUpdate = bSet;
/*N*/ }


/*N*/ BOOL ScDocument::UpdateDdeLink( const String& rAppl, const String& rTopic, const String& rItem )
/*N*/ {
/*N*/ 	//	fuer refresh() per StarOne Api
/*N*/ 	//	ResetValue() fuer einzelnen Link nicht noetig
/*N*/ 	//!	wenn's mal alles asynchron wird, aber auch hier
/*N*/ 
/*N*/ 	BOOL bFound = FALSE;
/*N*/     if (pLinkManager)
/*N*/     {
/*?*/         DBG_BF_ASSERT(0, "STRIP"); //STRIP001 const ::binfilter::SvBaseLinks& rLinks = pLinkManager->GetLinks();
/*N*/     }
/*N*/ 	return bFound;
/*N*/ }



/*N*/ USHORT ScDocument::GetDdeLinkCount() const
/*N*/ {
/*N*/ 	USHORT nDdeCount = 0;
/*N*/ 	if (pLinkManager)
/*N*/ 	{
/*N*/ 		const ::binfilter::SvBaseLinks& rLinks = pLinkManager->GetLinks();
/*N*/ 		USHORT nCount = rLinks.Count();
/*N*/ 		for (USHORT i=0; i<nCount; i++)
/*N*/ 			if ((*rLinks[i])->ISA(ScDdeLink))
/*N*/ 				++nDdeCount;
/*N*/ 	}
/*N*/ 	return nDdeCount;
/*N*/ }

/*N*/ BOOL ScDocument::GetDdeLinkData( USHORT nPos, String& rAppl, String& rTopic, String& rItem ) const
/*N*/ {
/*N*/ 	USHORT nDdeCount = 0;
/*N*/ 	if (pLinkManager)
/*N*/ 	{
/*N*/ 		const ::binfilter::SvBaseLinks& rLinks = pLinkManager->GetLinks();
/*N*/ 		USHORT nCount = rLinks.Count();
/*N*/ 		for (USHORT i=0; i<nCount; i++)
/*N*/ 		{
/*N*/ 			::binfilter::SvBaseLink* pBase = *rLinks[i];
/*N*/ 			if (pBase->ISA(ScDdeLink))
/*N*/ 			{
/*N*/ 				if ( nDdeCount == nPos )
/*N*/ 				{
/*N*/ 					ScDdeLink* pDde = (ScDdeLink*)pBase;
/*N*/ 					rAppl  = pDde->GetAppl();
/*N*/ 					rTopic = pDde->GetTopic();
/*N*/ 					rItem  = pDde->GetItem();
/*N*/ 					return TRUE;
/*N*/ 				}
/*N*/ 				++nDdeCount;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ BOOL ScDocument::GetDdeLinkMode(USHORT nPos, USHORT& nMode)
/*N*/ {
/*N*/ 	USHORT nDdeCount = 0;
/*N*/  	if (pLinkManager)
/*N*/  	{
/*N*/  		const ::binfilter::SvBaseLinks& rLinks = pLinkManager->GetLinks();
/*N*/  		USHORT nCount = rLinks.Count();
/*N*/  		for (USHORT i=0; i<nCount; i++)
/*N*/  		{
/*N*/  			::binfilter::SvBaseLink* pBase = *rLinks[i];
/*N*/  			if (pBase->ISA(ScDdeLink))
/*N*/  			{
/*N*/  				if ( nDdeCount == nPos )
/*N*/  				{
/*N*/  					ScDdeLink* pDde = (ScDdeLink*)pBase;
/*N*/  					nMode = pDde->GetMode();
/*N*/  					return TRUE;
/*N*/  				}
/*N*/  				++nDdeCount;
/*N*/  			}
/*N*/  		}
/*N*/  	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ BOOL ScDocument::GetDdeLinkResultDimension( USHORT nPos, USHORT& nCol, USHORT& nRow, ScMatrix*& pMatrix)
/*N*/ {
/*N*/ 	USHORT nDdeCount = 0;
/*N*/  	if (pLinkManager)
/*N*/  	{
/*N*/  		const ::binfilter::SvBaseLinks& rLinks = pLinkManager->GetLinks();
/*N*/  		USHORT nCount = rLinks.Count();
/*N*/  		for (USHORT i=0; i<nCount; i++)
/*N*/  		{
/*N*/  			::binfilter::SvBaseLink* pBase = *rLinks[i];
/*N*/  			if (pBase->ISA(ScDdeLink))
/*N*/  			{
/*N*/  				if ( nDdeCount == nPos )
/*N*/  				{
/*N*/  					ScDdeLink* pDde = (ScDdeLink*)pBase;
/*N*/  					pMatrix = pDde->GetResult();
/*N*/  					if (pMatrix)
/*N*/  					{
/*N*/  						pMatrix->GetDimensions(nCol, nRow);
/*N*/  						return TRUE;
/*N*/  					}
/*N*/  				}
/*N*/  				++nDdeCount;
/*N*/  			}
/*N*/  		}
/*N*/  	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ BOOL ScDocument::GetDdeLinkResult(const ScMatrix* pMatrix, USHORT nCol, USHORT nRow, String& rStrValue, double& rDoubValue, BOOL& bIsString)
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	if (pMatrix)
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ void ScDocument::CreateDdeLink(const String& rAppl, const String& rTopic, const String& rItem, const BYTE nMode )
/*N*/ {
    //	DDE-Link anlegen und nicht updaten (z.B. fuer Excel-Import,
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	//	damit nicht ohne Nachfrage Verbindungen aufgebaut werden)
/*N*/ }

/*N*/ BOOL ScDocument::FindDdeLink(const String& rAppl, const String& rTopic, const String& rItem, const BYTE nMode, USHORT& nPos )
/*N*/ {
/*?*/     DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if (pLinkManager)
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ BOOL ScDocument::CreateDdeLinkResultDimension(USHORT nPos, USHORT nCols, USHORT nRows, ScMatrix*& pMatrix)
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	USHORT nDdeCount = 0;
/*N*/ 	return FALSE;
/*N*/ }

void ScDocument::SetDdeLinkResult(ScMatrix* pMatrix, const USHORT nCol, const USHORT nRow, const String& rStrValue, const double& rDoubValue, BOOL bString, BOOL bEmpty)
{
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	DBG_ASSERT(pMatrix, "there is no matrix");
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void ScDocument::UpdateRefAreaLinks( UpdateRefMode eUpdateRefMode,
/*N*/ 							 const ScRange& rRange, short nDx, short nDy, short nDz )
/*N*/ {
/*N*/     if (pLinkManager)
/*N*/     {
/*N*/         const ::binfilter::SvBaseLinks& rLinks = pLinkManager->GetLinks();
/*N*/         USHORT nCount = rLinks.Count();
/*N*/         for (USHORT i=0; i<nCount; i++)
/*N*/         {
/*?*/             ::binfilter::SvBaseLink* pBase = *rLinks[i];
/*?*/             if (pBase->ISA(ScAreaLink))
/*?*/             {
/*?*/                 ScAreaLink* pLink = (ScAreaLink*) pBase;
/*?*/                 ScRange aOutRange = pLink->GetDestArea();
/*?*/ 
/*?*/                 USHORT nCol1 = aOutRange.aStart.Col();
/*?*/                 USHORT nRow1 = aOutRange.aStart.Row();
/*?*/                 USHORT nTab1 = aOutRange.aStart.Tab();
/*?*/                 USHORT nCol2 = aOutRange.aEnd.Col();
/*?*/                 USHORT nRow2 = aOutRange.aEnd.Row();
/*?*/                 USHORT nTab2 = aOutRange.aEnd.Tab();
/*?*/ 
/*?*/                 ScRefUpdateRes eRes =
/*?*/                     ScRefUpdate::Update( this, eUpdateRefMode,
/*?*/                         rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
/*?*/                         rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aEnd.Tab(), nDx, nDy, nDz,
/*?*/                         nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
/*?*/                 if ( eRes != UR_NOTHING )
/*?*/                     pLink->SetDestArea( ScRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 ) );
/*?*/             }
/*N*/         }
/*N*/     }
/*N*/ }

/*N*/ void ScDocument::SaveAreaLinks(SvStream& rStream) const
/*N*/ {
/*N*/ 	const ::binfilter::SvBaseLinks& rLinks = pLinkManager->GetLinks();
/*N*/ 	USHORT nCount = rLinks.Count();
/*N*/ 
/*N*/ 	//	erstmal zaehlen...
/*N*/ 
/*N*/ 	USHORT nAreaCount = 0;
/*N*/ 	USHORT i;
/*N*/ 	for (i=0; i<nCount; i++)
/*N*/ 		if ((*rLinks[i])->ISA(ScAreaLink))		// rLinks[i] = Pointer auf Ref
/*N*/ 			++nAreaCount;
/*N*/ 
/*N*/ 	//	Header
/*N*/ 
/*N*/ 	ScMultipleWriteHeader aHdr( rStream );
/*N*/ 	rStream << nAreaCount;
/*N*/ 
/*N*/ 	//	Links speichern
/*N*/ 
/*N*/ 	for (i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		::binfilter::SvBaseLink* pBase = *rLinks[i];
/*N*/ 		if (pBase->ISA(ScAreaLink))
/*N*/ 		{
/*?*/ 			ScAreaLink* pLink = (ScAreaLink*)pBase;
/*?*/ 
/*?*/ 			aHdr.StartEntry();
/*?*/ 
/*?*/ 			rStream.WriteByteString( pLink->GetFile(), rStream.GetStreamCharSet() );
/*?*/ 			rStream.WriteByteString( pLink->GetFilter(), rStream.GetStreamCharSet() );
/*?*/ 			rStream.WriteByteString( pLink->GetSource(), rStream.GetStreamCharSet() );
/*?*/ 			rStream << pLink->GetDestArea();				// ScRange
/*?*/ 			rStream.WriteByteString( pLink->GetOptions(), rStream.GetStreamCharSet() );
/*?*/ 			//	filter options starting from 336
/*?*/ 
/*?*/ 			aHdr.EndEntry();
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocument::LoadAreaLinks(SvStream& rStream)
/*N*/ {
/*N*/ 	ScMultipleReadHeader aHdr( rStream );
/*N*/ 
/*N*/ 	if (!pShell)
/*N*/ 	{
/*N*/ 		DBG_ERROR("AreaLinks koennen nicht ohne Shell geladen werden");
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	String aFile, aFilter, aOptions, aSource;
/*N*/ 	ScRange aDestArea;
/*N*/ 
/*N*/ 	USHORT nCount;
/*N*/ 	rStream >> nCount;
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 aHdr.StartEntry();
/*?*/ 
/*N*/ 	}
/*N*/ }


//------------------------------------------------------------------------

// TimerDelays etc.

//	----------------------------------------------------------------------------

/*N*/ BOOL ScDocument::CheckMacroWarn()
/*N*/ {
/*N*/ 	//	The check for macro configuration, macro warning and disabling is now handled
/*N*/ 	//	in SfxObjectShell::AdjustMacroMode, called by SfxObjectShell::CallBasic.
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }



//------------------------------------------------------------------------



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
