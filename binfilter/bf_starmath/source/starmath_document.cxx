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


#include <sot/formats.hxx>

#include <bf_svtools/lingucfg.hxx>
#include <comphelper/classids.hxx>

#include <bf_svtools/itemset.hxx>
#include <bf_svtools/eitem.hxx>
#include <bf_svtools/whiter.hxx>
#include <bf_svtools/intitem.hxx>
#include <bf_svtools/stritem.hxx>

#include <bf_sfx2/app.hxx>
#include <bf_sfx2/docfile.hxx>
#include <bf_sfx2/request.hxx>
#include <bf_svtools/sfxecode.hxx>
#include <bf_sfx2/printer.hxx>

#include <bf_svtools/itempool.hxx>
#include <bf_svx/editeng.hxx>
#include <bf_svx/eeitem.hxx>
#include <bf_svx/editstat.hxx>
#include <bf_svx/eeitemid.hxx>
#include <bf_svx/fontitem.hxx>
#include <bf_svx/fhgtitem.hxx>
#include <bf_svtools/slstitm.hxx>

#include <com/sun/star/uno/Any.h>
#include <starmath.hrc>
#include <document.hxx>
#include <unomodel.hxx>
#include <config.hxx>
#include <mathml.hxx>

namespace binfilter {



using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

#define A2OU(x)        ::rtl::OUString::createFromAscii( x )

namespace binfilter
{
#ifndef SO2_DECL_SVSTORAGESTREAM_DEFINED
#define SO2_DECL_SVSTORAGESTREAM_DEFINED
SO2_DECL_REF(SvStorageStream)
#endif
#ifndef SO2_DECL_SVSTORAGE_DEFINED
#define SO2_DECL_SVSTORAGE_DEFINED
SO2_DECL_REF(SvStorage)
#endif
}

#define DOCUMENT_BUFFER_SIZE	(USHORT)32768

static const char __FAR_DATA pStarMathDoc[] = "StarMathDocument";


/**************************************************************************/
/*
**
**	CLASS IMPLEMENTATION
**
**/

/*N*/ TYPEINIT1( SmDocShell, SfxObjectShell );

/*N*/ SFX_IMPL_OBJECTFACTORY_DLL(SmDocShell, smath, SvGlobalName(BF_SO3_SM_CLASSID), Sm);

/*N*/ SmSymSetManager & SmDocShell::GetSymSetManager()
/*N*/ {
/*N*/ 	if (!pSymSetMgr)
/*N*/     {
/*N*/ 		pSymSetMgr = new SmSymSetManager;
/*N*/         pSymSetMgr->Load();
/*N*/     }
/*N*/ 	return *pSymSetMgr;
/*N*/ }

/*N*/ void SmDocShell::SetText(const String& rBuffer)
/*N*/ {
/*N*/ 	if (rBuffer != aText)
/*N*/ 	{
/*N*/ 		BOOL bIsEnabled = IsEnableSetModified();
/*N*/ 		if( bIsEnabled )
/*N*/ 			EnableSetModified( FALSE );
/*N*/
/*N*/ 		aText = rBuffer;
/*N*/ 		Parse();
/*N*/         //Resize();
/*N*/
/*N*/ 		if ( bIsEnabled )
/*N*/ 			EnableSetModified( bIsEnabled );
/*N*/ 		SetModified(TRUE);
/*N*/
        // launch accessible event if necessary
/*N*/ 	}
/*N*/ }



/*N*/ void SmDocShell::Parse()
/*N*/ {
/*N*/ 	if (pTree)
/*N*/ 		delete pTree;
/*N*/ 	pTree = aInterpreter.Parse(aText);
/*N*/ 	nModifyCount++;
/*N*/     SetFormulaArranged( FALSE );
/*N*/ }


/*N*/ void SmDocShell::ArrangeFormula()
/*N*/ {
/*N*/   //! Nur fuer die Dauer der Existenz dieses Objekts sind am Drucker die
/*N*/ 	//! richtigen Einstellungen garantiert.
/*N*/     SmPrinterAccess  aPrtAcc(*this);
/*N*/ //	OutputDevice	*pOutDev = aPrtAcc.GetPrinter();
/*N*/     OutputDevice* pOutDev = aPrtAcc.GetRefDev();
/*N*/
/*N*/ #ifdef DBG_UTIL
/*N*/ 	if (!pOutDev)
/*N*/ 		DBG_WARNING("Sm : kein Drucker vorhanden");
/*N*/ #endif
/*N*/
/*N*/   // falls noetig ein anderes OutputDevice holen fuer das formatiert wird
/*N*/ 	if (!pOutDev)
/*N*/ 	{
/*N*/       pOutDev = &SM_MOD1()->GetDefaultVirtualDev();
/*N*/ 		pOutDev->SetMapMode( MapMode(MAP_100TH_MM) );
/*N*/ 	}
/*N*/ 	DBG_ASSERT(pOutDev->GetMapMode().GetMapUnit() == MAP_100TH_MM,
/*N*/ 			   "Sm : falscher MapMode");
/*N*/
/*N*/ 	const SmFormat &rFormat = GetFormat();
/*N*/ 	pTree->Prepare(rFormat, *this);
/*N*/ 	pTree->Arrange(*pOutDev, rFormat);
/*N*/
/*N*/ 	SetFormulaArranged(TRUE);
/*N*/
/*N*/     // invalidate accessible text
/*N*/     aAccText = String();
/*N*/ }

/*N*/ Size SmDocShell::GetSize()
/*N*/ {
/*N*/ 	Size aRet;
/*N*/
/*N*/ 	if (!pTree)
/*N*/ 		Parse();
/*N*/
/*N*/ 	if (pTree)
/*N*/ 	{
/*N*/ 		if (!IsFormulaArranged())
/*N*/ 			ArrangeFormula();
/*N*/ 		aRet = pTree->GetSize();
/*N*/
/*N*/ 		if ( !aRet.Width() )
/*?*/ 			aRet.Width() = 2000;
/*N*/ 		else
/*N*/ 			aRet.Width()  += aFormat.GetDistance( DIS_LEFTSPACE ) +
/*N*/ 							 aFormat.GetDistance( DIS_RIGHTSPACE );
/*N*/ 		if ( !aRet.Height() )
/*?*/ 			aRet.Height() = 1000;
/*N*/ 		else
/*N*/ 			aRet.Height() += aFormat.GetDistance( DIS_TOPSPACE ) +
/*N*/ 							 aFormat.GetDistance( DIS_BOTTOMSPACE );
/*N*/ 	}
/*N*/
/*N*/ 	return aRet;
/*N*/ }

////////////////////////////////////////

/*N*/ SmPrinterAccess::SmPrinterAccess( SmDocShell &rDocShell )
/*N*/ {
/*N*/ 	if ( 0 != (pPrinter = rDocShell.GetPrt()) )
/*N*/ 	{
/*N*/ 		pPrinter->Push( PUSH_MAPMODE );
/*N*/ 		if ( rDocShell.GetProtocol().IsInPlaceActive() ||
/*N*/ 			 SFX_CREATE_MODE_EMBEDDED == rDocShell.GetCreateMode() )
/*N*/ 		{
/*N*/ 			// if it is an embedded object (without it's own printer)
/*N*/ 			// we change the MapMode temporarily.
/*N*/ 			//!If it is a document with it's own printer the MapMode should
/*N*/ 			//!be set correct (once) elsewhere(!), in order to avoid numerous
/*N*/ 			//!superfluous pushing and poping of the MapMode when using
/*N*/ 			//!this class.
/*N*/
/*N*/ 			const MapUnit eOld = pPrinter->GetMapMode().GetMapUnit();
/*?*/  			if ( MAP_100TH_MM != eOld )
/*?*/  			{
/*?*/  				MapMode aMap( pPrinter->GetMapMode() );
/*?*/  				aMap.SetMapUnit( MAP_100TH_MM );
/*?*/  				Point aTmp( aMap.GetOrigin() );
/*?*/  				aTmp.X() = OutputDevice::LogicToLogic( aTmp.X(), eOld, MAP_100TH_MM );
/*?*/  				aTmp.Y() = OutputDevice::LogicToLogic( aTmp.Y(), eOld, MAP_100TH_MM );
/*?*/  				aMap.SetOrigin( aTmp );
/*?*/  				pPrinter->SetMapMode( aMap );
/*?*/  			}
/*N*/ 		}
/*N*/ 	}
/*N*/     if ( 0 != (pRefDev = rDocShell.GetRefDev()) && pPrinter != pRefDev )
/*N*/     {
/*N*/         pRefDev->Push( PUSH_MAPMODE );
/*N*/         if ( rDocShell.GetProtocol().IsInPlaceActive() ||
/*N*/              SFX_CREATE_MODE_EMBEDDED == rDocShell.GetCreateMode() )
/*N*/         {
/*N*/             // if it is an embedded object (without it's own printer)
/*N*/             // we change the MapMode temporarily.
/*N*/             //!If it is a document with it's own printer the MapMode should
/*N*/             //!be set correct (once) elsewhere(!), in order to avoid numerous
/*N*/             //!superfluous pushing and poping of the MapMode when using
/*N*/             //!this class.
/*N*/
/*N*/             const MapUnit eOld = pRefDev->GetMapMode().GetMapUnit();
/*N*/             if ( MAP_100TH_MM != eOld )
/*N*/             {
/*N*/                 MapMode aMap( pRefDev->GetMapMode() );
/*N*/                 aMap.SetMapUnit( MAP_100TH_MM );
/*N*/                 Point aTmp( aMap.GetOrigin() );
/*N*/                 aTmp.X() = OutputDevice::LogicToLogic( aTmp.X(), eOld, MAP_100TH_MM );
/*N*/                 aTmp.Y() = OutputDevice::LogicToLogic( aTmp.Y(), eOld, MAP_100TH_MM );
/*N*/                 aMap.SetOrigin( aTmp );
/*N*/                 pRefDev->SetMapMode( aMap );
/*N*/             }
/*N*/         }
/*N*/     }
/*N*/ }

/*N*/ SmPrinterAccess::~SmPrinterAccess()
/*N*/ {
/*N*/ 	if ( pPrinter )
/*N*/ 		pPrinter->Pop();
/*N*/     if ( pRefDev && pRefDev != pPrinter )
/*N*/         pRefDev->Pop();
/*N*/ }

////////////////////////////////////////

/*N*/ Printer* SmDocShell::GetPrt()
/*N*/ {
/*N*/ 	if ( GetProtocol().IsInPlaceActive() ||
/*N*/  		 SFX_CREATE_MODE_EMBEDDED == GetCreateMode() )
/*N*/ 	{
/*N*/ 		//Normalerweise wird der Printer vom Server besorgt. Wenn dieser aber
/*N*/ 		//keinen liefert (weil etwa noch keine connection da ist), kann es
/*N*/ 		//dennoch sein, dass wir den Printer kennen, denn dieser wird in
/*N*/ 		//OnDocumentPrinterChanged vom Server durchgereicht und dann temporaer
/*N*/ 		//festgehalten.
/*N*/         Printer *pPrt = GetDocumentPrinter();
/*N*/         if ( !pPrt && pTmpPrinter )
/*N*/             pPrt = pTmpPrinter;
/*N*/         return pPrt;
/*N*/ 	}
/*N*/ 	else if ( !pPrinter )
/*N*/ 	{
/*N*/ 		SfxItemSet *pOptions =
/*N*/ 			new SfxItemSet(GetPool(),
/*N*/ 						   SID_PRINTSIZE,		SID_PRINTSIZE,
/*N*/ 						   SID_PRINTZOOM,		SID_PRINTZOOM,
/*N*/ 						   SID_PRINTTITLE,		SID_PRINTTITLE,
/*N*/ 						   SID_PRINTTEXT,		SID_PRINTTEXT,
/*N*/ 						   SID_PRINTFRAME,		SID_PRINTFRAME,
/*N*/ 						   SID_NO_RIGHT_SPACES, SID_NO_RIGHT_SPACES,
/*N*/ 						   0);
/*N*/
/*N*/ 		SmModule *pp = SM_MOD1();
/*N*/ 		pp->GetConfig()->ConfigToItemSet(*pOptions);
/*N*/ 		pPrinter = new SfxPrinter(pOptions);
/*N*/ 		pPrinter->SetMapMode( MapMode(MAP_100TH_MM) );
/*N*/ 	}
/*N*/ 	return pPrinter;
/*N*/ }

/*N*/ OutputDevice* SmDocShell::GetRefDev()
/*N*/ {
/*N*/     if ( GetProtocol().IsInPlaceActive() ||
/*N*/          SFX_CREATE_MODE_EMBEDDED == GetCreateMode() )
/*N*/     {
/*N*/         OutputDevice* pOutDev = GetDocumentRefDev();
/*N*/         if ( pOutDev )
/*N*/             return pOutDev;
/*N*/     }
/*N*/
/*N*/     return GetPrt();
/*N*/ }


/*N*/ void SmDocShell::SetPrinter( SfxPrinter *pNew )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ }

/*N*/ void SmDocShell::OnDocumentPrinterChanged( Printer *pPrt )
/*N*/ {
/*N*/ 	pTmpPrinter = pPrt;
/*N*/ 	SetFormulaArranged(FALSE);
/*N*/ 	SM_MOD1()->GetRectCache()->Reset();
/*N*/ 	Size aOldSize = GetVisArea().GetSize();
/*N*/ 	Resize();
/*N*/ 	if( aOldSize != GetVisArea().GetSize() && aText.Len() )
/*N*/ 		SetModified( TRUE );
/*N*/ 	pTmpPrinter = 0;
/*N*/ }

/*N*/ void SmDocShell::Resize()
/*N*/ {
/*N*/ 	Size aVisSize = GetSize();
/*N*/
/*N*/ 	BOOL bIsEnabled = IsEnableSetModified();
/*N*/ 	if ( bIsEnabled )
/*?*/ 		EnableSetModified( FALSE );
/*N*/
/*N*/ 	SetVisAreaSize( aVisSize );
/*N*/
/*N*/ 	if ( bIsEnabled )
/*?*/ 		EnableSetModified( bIsEnabled );
/*N*/ }


/*N*/ SmDocShell::SmDocShell(SfxObjectCreateMode eMode) :
/*N*/ 	SfxObjectShell(eMode),
/*N*/ 	pSymSetMgr			( 0 ),
/*N*/ 	pTree				( 0 ),
/*N*/ 	pPrinter			( 0 ),
/*N*/ 	pTmpPrinter			( 0 ),
/*N*/ 	pEditEngineItemPool	( 0 ),
/*N*/ 	pEditEngine			( 0 ),
/*N*/ 	nModifyCount		( 0 ),
/*N*/ 	bIsFormulaArranged	( FALSE )
/*N*/ {
/*N*/ 	SetPool(&SFX_APP()->GetPool());
/*N*/
/*N*/ 	SmModule *pp = SM_MOD1();
/*N*/     aFormat = pp->GetConfig()->GetStandardFormat();
/*N*/
/*N*/ 	StartListening(aFormat);
/*N*/ 	StartListening(*pp->GetConfig());
/*N*/
/*N*/ 	SetShell(this);
/*N*/ 	SetModel( new SmModel(this) );	//! das hier mit new erzeugte Model brauch
/*N*/                                   //! im Destruktor nicht explizit geloescht werden.
/*N*/ 									//! Dies erledigt das Sfx.
/*N*/ }



/*N*/ SmDocShell::~SmDocShell()
/*N*/ {
/*N*/ 	SmModule *pp = SM_MOD1();
/*N*/
/*N*/ 	EndListening(aFormat);
/*N*/ 	EndListening(*pp->GetConfig());
/*N*/
/*N*/ 	delete pEditEngine;
/*N*/ 	delete pEditEngineItemPool;
/*N*/ 	delete pTree;
/*N*/ 	delete pPrinter;
/*N*/ }



/*N*/ void SmDocShell::ConvertText( String &rText, SmConvert eConv )
/*N*/     // adapts the text 'rText' that suits one office version to be
/*N*/     // usable in another office version.
/*N*/     // Example: "2 over sin x" acts very different in 4.0 and 5.0,
/*N*/     // and from 5.2 to 6.0 many symbol names were renamed.
/*N*/ {
/*N*/ 	if (pTree)
/*N*/ 		delete pTree;
/*N*/
/*N*/     SmConvert  eTmpConv = aInterpreter.GetConversion();
/*N*/
/*N*/     // parse in old style and make changes for new style
/*N*/     aInterpreter.SetConversion(eConv);
/*N*/     pTree = aInterpreter.Parse(rText);
/*N*/     // get to new version converted text
/*N*/     rText = aInterpreter.GetText();
/*N*/
/*N*/     aInterpreter.SetConversion(eTmpConv);
/*N*/
/*N*/ 	// clean up tree parsed in old style
/*N*/ 	if (pTree)
/*N*/ 	{	delete pTree;
/*N*/ 		pTree = NULL;
/*N*/ 	}
/*N*/ }








/*N*/ BOOL SmDocShell::InitNew(SvStorage * pStor)
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	if (SfxInPlaceObject::InitNew(pStor))
/*N*/ 	{
/*N*/ 		bRet = TRUE;
/*N*/ 		SetVisArea(Rectangle(Point(0, 0), Size(2000, 1000)));
/*N*/ #if 0
/*N*/ 		if (pStor)
/*N*/ 		{
/*N*/ 			aDocStream = pStor->OpenStream(String::CreateFromAscii(pStarMathDoc));
/*N*/ 			aDocStream->SetVersion (pStor->GetVersion ());
/*N*/ 			GetPool().SetFileFormatVersion(USHORT(pStor->GetVersion()));
/*N*/
/*N*/ 			if (! aDocStream )
/*N*/ 				bRet = FALSE;
/*N*/ 		}
/*N*/ #endif
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }


/*N*/ BOOL SmDocShell::Load(SvStorage *pStor)
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	if( SfxInPlaceObject::Load( pStor ))
/*N*/ 	{
/*N*/ 		String aTmpStr( C2S( "Equation Native" ));
/*N*/ 		if( pStor->IsStream( aTmpStr ))
/*N*/ 		{
/*N*/ 			// is this a MathType Storage?
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/  			MathType aEquation(aText);
/*N*/ 		}
/*N*/ 		else if( pStor->IsStream(C2S("content.xml")) ||
/*N*/ 				 pStor->IsStream(C2S("Content.xml")) )
/*N*/ 		{
/*?*/  			// is this a fabulous math package ?
/*?*/              Reference< ::com::sun::star::frame::XModel> xModel(GetModel());
/*?*/  			SmXMLWrapper aEquation(xModel);
/*?*/  			SfxMedium aMedium(pStor);
/*?*/              ULONG nError = aEquation.Import(aMedium);
/*?*/              bRet = 0 == nError;
/*?*/              SetError( nError );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/             bRet = Try3x(pStor, STREAM_READWRITE);
/*N*/
/*N*/ 			if( !bRet )
/*N*/ 			{
/*?*/  				pStor->Remove(String::CreateFromAscii(pStarMathDoc));
/*?*/                   bRet = Try2x(pStor, STREAM_READWRITE);
/*?*/  				pStor->Remove(C2S("\1Ole10Native"));
/*N*/ 			}
/*N*/ 			else
/*N*/             {
/*N*/                 long nVersion = pStor->GetVersion();
/*N*/                 if ( nVersion <= SOFFICE_FILEFORMAT_40 )
/*N*/                     ConvertText( aText, CONVERT_40_TO_50 );
/*N*/                 if ( nVersion <= SOFFICE_FILEFORMAT_50 )
/*N*/                     ConvertText( aText, CONVERT_50_TO_60 );
/*N*/                 if (pTree)
/*?*/                 {   delete pTree;
/*?*/                     pTree = NULL;
/*N*/                 }
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	FinishedLoading( SFX_LOADED_ALL );
/*N*/ 	return bRet;
/*N*/ }




//------------------------------------------------------------------

/*N*/ void SmDocShell::ImplSave( SvStorageStreamRef xStrm )
/*N*/ {
/*N*/     String aTmp( aText );
/*N*/     if (SOFFICE_FILEFORMAT_50 >= xStrm->GetVersion())
/*N*/         ConvertText( aTmp, CONVERT_60_TO_50 );
/*N*/     ByteString exString( ExportString( aTmp ) );
/*N*/
/*N*/ 	*xStrm	<< SM304AIDENT << SM50VERSION
/*N*/ 			<< 'T';
/*N*/ 	xStrm->WriteByteString(exString);
/*N*/ 	*xStrm	<< 'F' << aFormat
/*N*/             << 'S';
/*N*/     xStrm->WriteByteString( ExportString(C2S("unknown")) );
/*N*/     *xStrm  << (USHORT) 0
/*N*/ 			<< '\0';
/*N*/ }

/*N*/ BOOL SmDocShell::Save()
/*N*/ {
/*N*/     //! apply latest changes if necessary
/*N*/     UpdateText();
/*N*/
/*N*/     if ( SfxInPlaceObject::Save() )
/*N*/ 	{
/*N*/ 		if( !pTree )
/*N*/ 			Parse();
/*N*/ 		if( pTree && !IsFormulaArranged() )
/*N*/ 			ArrangeFormula();
/*N*/
/*N*/ 		SvStorage *pStor = GetStorage();
/*N*/ 		if(pStor->GetVersion() >= SOFFICE_FILEFORMAT_60)
/*N*/ 		{
/*N*/ 			// a math package as a storage
/*N*/             Reference< ::com::sun::star::frame::XModel> xModel(GetModel());
/*N*/ 			SmXMLWrapper aEquation(xModel);
/*N*/ 			SfxMedium aMedium(pStor);
/*N*/ 			aEquation.SetFlat(sal_False);
/*N*/ 			return aEquation.Export(aMedium);
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			aDocStream = pStor->OpenStream(String::CreateFromAscii(pStarMathDoc));
/*?*/ 			aDocStream->SetVersion (pStor->GetVersion ());
/*?*/ 			GetPool().SetFileFormatVersion(USHORT(pStor->GetVersion()));
/*?*/
/*?*/ 			aDocStream->Seek(0);
/*?*/ 			ImplSave( aDocStream );
/*?*/
/*?*/ 			aDocStream.Clear();
/*?*/ 			return TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ void SmDocShell::UpdateText()
/*N*/ {
/*N*/     if (pEditEngine && pEditEngine->IsModified())
/*N*/     {
/*N*/         String aEngTxt( pEditEngine->GetText( LINEEND_LF ) );
/*N*/         if (GetText() != aEngTxt)
/*N*/             SetText( aEngTxt );
/*N*/     }
/*N*/ }


/*N*/ BOOL SmDocShell::SaveAs(SvStorage * pNewStor)
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/
/*N*/     //! apply latest changes if necessary
/*N*/     UpdateText();
/*N*/
/*N*/     if ( SfxInPlaceObject::SaveAs( pNewStor ) )
/*N*/ 	{
/*N*/ 		if( !pTree )
/*?*/ 			Parse();
/*N*/ 		if( pTree && !IsFormulaArranged() )
/*?*/ 			ArrangeFormula();
/*N*/
/*N*/ 		if (pNewStor->GetVersion() >= SOFFICE_FILEFORMAT_60)
/*N*/ 		{
/*N*/ 			// a math package as a storage
/*?*/              Reference< ::com::sun::star::frame::XModel> xModel(GetModel());
/*?*/  			SmXMLWrapper aEquation(xModel);
/*?*/  			SfxMedium aMedium(pNewStor);
/*?*/  			aEquation.SetFlat(sal_False);
/*?*/  			bRet = aEquation.Export(aMedium);
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			SvStorageStreamRef aStm = pNewStor->OpenStream(
/*N*/ 										String::CreateFromAscii(pStarMathDoc));
/*N*/ 			aStm->SetVersion( pNewStor->GetVersion() );
/*N*/ 			GetPool().SetFileFormatVersion( USHORT( pNewStor->GetVersion() ));
/*N*/ 			aStm->SetBufferSize(DOCUMENT_BUFFER_SIZE);
/*N*/ 			aStm->SetKey( pNewStor->GetKey() ); // Passwort setzen
/*N*/
/*N*/ 			if ( aStm.Is() )
/*N*/ 			{
/*N*/ 				ImplSave( aStm );
/*N*/ 				bRet = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }


/*N*/ BOOL SmDocShell::SaveCompleted(SvStorage * pStor)
/*N*/ {
/*N*/ 	if( SfxInPlaceObject::SaveCompleted( pStor ))
/*N*/ 	{
/*N*/ #if 0
/*N*/ 		if (! pStor)
/*N*/ 			return TRUE;
/*N*/
/*N*/ 		aDocStream = pStor->OpenStream(String::CreateFromAscii(pStarMathDoc));
/*N*/ 		aDocStream->SetVersion (pStor->GetVersion ());
/*N*/ 		GetPool().SetFileFormatVersion(USHORT(pStor->GetVersion()));
/*N*/ 		aDocStream->SetBufferSize(DOCUMENT_BUFFER_SIZE);
/*N*/ 		aDocStream->SetKey( pStor->GetKey() ); // Passwort setzen
/*N*/ 		return aDocStream.Is();
/*N*/ #endif
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }





/*N*/ void SmDocShell::HandsOff()
/*N*/ {
/*N*/ 	SfxInPlaceObject::HandsOff();
/*N*/ #if 0
/*N*/ 	aDocStream.Clear();
/*N*/ #endif
/*N*/ }


/*N*/ SfxItemPool& SmDocShell::GetPool()
/*N*/ {
/*N*/ 	return SFX_APP()->GetPool();
/*N*/ }

/*N*/ void SmDocShell::SetVisArea (const Rectangle & rVisArea)
/*N*/ {
/*N*/ 	Rectangle aNewRect (rVisArea);
/*N*/
/*N*/ 	aNewRect.SetPos(Point ());
/*N*/
/*N*/ 	if (! aNewRect.Right ()) aNewRect.Right () = 2000;
/*N*/ 	if (! aNewRect.Bottom ()) aNewRect.Bottom () = 1000;
/*N*/
/*N*/ 	BOOL bIsEnabled = IsEnableSetModified();
/*N*/ 	if ( bIsEnabled )
/*?*/ 		EnableSetModified( FALSE );
/*N*/
/*N*/ 	// If outplace editing, then dont resize the OutplaceWindow. But the
/*N*/ 	// ObjectShell has to resize. Bug 56470
/*N*/
/*N*/ 	SfxInPlaceObject::SetVisArea( aNewRect );
/*N*/
/*N*/
/*N*/ 	if ( bIsEnabled )
/*?*/ 		EnableSetModified( bIsEnabled );
/*N*/ }


/*N*/ BOOL SmDocShell::Try3x (SvStorage *pStor,
/*N*/ 						StreamMode eMode)
/*N*/
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/
/*N*/ 	SvStorageStreamRef aTempStream = pStor->OpenStream(
/*N*/ 								String::CreateFromAscii(pStarMathDoc), eMode);
/*N*/ 	aTempStream->SetVersion (pStor->GetVersion ());
/*N*/ 	GetPool().SetFileFormatVersion (USHORT(pStor->GetVersion()));
/*N*/ 	aTempStream->SetBufferSize(DOCUMENT_BUFFER_SIZE);
/*N*/ 	aTempStream->SetKey( pStor->GetKey() ); // Passwort setzen
/*N*/
/*N*/ 	if (aTempStream->GetError() == 0)
/*N*/ 	{
/*N*/ 		SvStream*	 pSvStream = aTempStream;
/*N*/ 		char		 cTag;
/*N*/ 		sal_uInt32	 lIdent, lVersion;
/*N*/ 		long		 lTime;
/*N*/ 		sal_uInt32	 lDate;
/*N*/ 		String		 aBuffer;
/*N*/         ByteString   aByteStr;
/*N*/
/*N*/ 		*pSvStream >> lIdent >> lVersion;
/*N*/
/*N*/ 		if ((lIdent == SM30IDENT) || (lIdent == SM30BIDENT) || (lIdent == SM304AIDENT))
/*N*/ 		{
/*N*/ 			DBG_ASSERT((lVersion == SM30VERSION) ||
/*N*/ 					   (lVersion == SM50VERSION), "Illegal file version");
/*N*/
/*N*/ 			*pSvStream >> cTag;
/*N*/             rtl_TextEncoding eEnc = RTL_TEXTENCODING_MS_1252;
/*N*/ 			while (cTag && !pSvStream->IsEof())
/*N*/ 			{
/*N*/ 				switch (cTag)
/*N*/ 				{
/*N*/ 					case 'T':
/*N*/                         pSvStream->ReadByteString( aByteStr );
/*N*/                         aText = ImportString( aByteStr );
/*N*/ 						Parse();
/*N*/ 						break;
/*N*/
/*N*/ 					case 'D':
/*?*/ 						pSvStream->ReadByteString(aBuffer, eEnc);
/*?*/ 						pSvStream->ReadByteString(aBuffer, eEnc);
/*?*/ 						*pSvStream >> lDate >> lTime;
/*?*/ 						pSvStream->ReadByteString(aBuffer, eEnc);
/*?*/ 						*pSvStream >> lDate >> lTime;
/*?*/ 						pSvStream->ReadByteString(aBuffer, eEnc);
/*?*/ 						break;
/*N*/
/*N*/ 					case 'F':
/*N*/ 						*pSvStream >> aFormat;
/*N*/ 						if (lIdent != SM304AIDENT)
/*?*/ 							aFormat.From300To304a ();
/*N*/ 						else if ( lVersion == SM30VERSION )
/*N*/ 						{
/*?*/ 							aFormat.SetDistance(DIS_LEFTSPACE, 100);
/*?*/ 							aFormat.SetDistance(DIS_RIGHTSPACE, 100);
/*?*/ 							aFormat.SetDistance(DIS_TOPSPACE, 100);
/*?*/ 							aFormat.SetDistance(DIS_BOTTOMSPACE, 100);
/*N*/ 						}
/*N*/ 						break;
/*N*/
/*N*/ 					case 'S':
/*N*/ 					{
/*N*/                         String      aTmp;
/*N*/                         USHORT      n;
/*N*/                         pSvStream->ReadByteString(aTmp, eEnc);
/*N*/                         *pSvStream >> n;
/*N*/ 						break;
/*N*/ 					}
/*N*/
/*N*/ 					default:
/*N*/ 						DBG_ASSERT((cTag != 0), "Illegal data tag");
/*N*/ 				}
/*N*/ 				*pSvStream >> cTag;
/*N*/ 			}
/*N*/
/*N*/ 			bRet = TRUE;
/*N*/ #if 0
/*N*/ 			aDocStream = aTempStream;
/*N*/ #endif
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	if (!bRet)
/*N*/ 	{
/*N*/ 		// kein Passwort gesetzt --> Datei marode
/*?*/ 		if (pStor->GetKey().Len() == 0)
/*?*/ 		{
/*?*/ 			SetError(ERRCODE_SFX_DOLOADFAILED);
/*?*/ 		}
/*?*/ 		// Passwort gesetzt --> war wohl falsch
/*?*/ 		else
/*?*/ 		{
/*?*/ 			SetError(ERRCODE_SFX_WRONGPASSWORD);
/*?*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	return bRet;
/*N*/ }



BOOL SmDocShell::Try2x (SvStorage *pStor,
                        StreamMode eMode)
{
    SvStorageStreamRef aTempStream = pStor->OpenStream(C2S("\1Ole10Native"), eMode);
    aTempStream->SetVersion (pStor->GetVersion ());
    GetPool().SetFileFormatVersion(USHORT(pStor->GetVersion ()));

    if (aTempStream->GetError() == SVSTREAM_OK)
    {
        void ReadSM20SymSet(SvStream*, SmSymSet*);

        SvStream*    pSvStream = aTempStream;
        char         cTag;
        sal_uInt32   lIdent, lVersion;
        long         lTime;
        sal_uInt32   lDate;
        UINT32       lDataSize;
        String       aBuffer;
        ByteString   aByteStr;
        SmSymSet    *pSymbolSet;

        *pSvStream >> lDataSize >> lIdent >> lVersion;

        if (lIdent == FRMIDENT)
        {
            DBG_ASSERT((lVersion == FRMVERSION), "Illegal file version");

            *pSvStream >> cTag;
            rtl_TextEncoding eEnc = RTL_TEXTENCODING_MS_1252;
            while (cTag && !pSvStream->IsEof())
            {
                switch (cTag)
                {
                    case 'T':
                        pSvStream->ReadByteString( aByteStr );
                        aText = ImportString( aByteStr );
                        Parse();
                        break;

                    case 'D':
                        {
                            pSvStream->ReadByteString(aBuffer, eEnc);
                            pSvStream->ReadByteString(aBuffer, eEnc);
                            *pSvStream >> lDate >> lTime;
                            pSvStream->ReadByteString(aBuffer, eEnc);
                            *pSvStream >> lDate >> lTime;
                            pSvStream->ReadByteString(aBuffer, eEnc);
                        }
                        break;

                    case 'F':
                        {
                            //SmFormat aFormat;
                            aFormat.ReadSM20Format(*pSvStream);
                            aFormat.From300To304a ();
                        }
                        break;

                    case 'S':
                    {
                        // not sure about this...
                        /* ??? pSymbolSet = new SmSymSet();
                        ReadSM20SymSet(pSvStream, pSymbolSet);
                        delete pSymbolSet; */
                        String      aTmp;
                        USHORT      n;
                        pSvStream->ReadByteString(aTmp, eEnc);
                        *pSvStream >> n;
                        break;
                    }

                    default:
                        DBG_ASSERT((cTag != 0), "Illegal data tag");
                }
                *pSvStream >> cTag;
            }

            return TRUE;
        }
    }

    return FALSE;
}




/*N*/ void SmDocShell::FillClass(SvGlobalName* pClassName,
/*N*/ 						   ULONG*  pFormat,
/*N*/ 						   String* pAppName,
/*N*/ 						   String* pFullTypeName,
/*N*/ 						   String* pShortTypeName,
/*N*/ 						   long    nFileFormat) const
/*N*/ {
/*N*/ 	SfxInPlaceObject::FillClass(pClassName, pFormat, pAppName, pFullTypeName,
/*N*/ 								pShortTypeName, nFileFormat);
/*N*/
/*N*/ 	if (nFileFormat == SOFFICE_FILEFORMAT_31)
/*N*/ 	{
/*N*/ 		*pClassName 	= SvGlobalName(BF_SO3_SM_CLASSID_30);
/*N*/ 		*pFormat		= SOT_FORMATSTR_ID_STARMATH;
/*N*/ 		pAppName->AssignAscii( RTL_CONSTASCII_STRINGPARAM("Smath 3.1"));
/*N*/ 		*pFullTypeName	= String(SmResId(STR_MATH_DOCUMENT_FULLTYPE_31));
/*N*/ 		*pShortTypeName = String(SmResId(RID_DOCUMENTSTR));
/*N*/ 	}
/*N*/ 	else if (nFileFormat == SOFFICE_FILEFORMAT_40)
/*N*/ 	{
/*N*/ 		*pClassName 	= SvGlobalName(BF_SO3_SM_CLASSID_40);
/*N*/ 		*pFormat		= SOT_FORMATSTR_ID_STARMATH_40;
/*N*/ 		*pFullTypeName	= String(SmResId(STR_MATH_DOCUMENT_FULLTYPE_40));
/*N*/ 		*pShortTypeName = String(SmResId(RID_DOCUMENTSTR));
/*N*/ 	}
/*N*/ 	else if (nFileFormat == SOFFICE_FILEFORMAT_50)
/*N*/ 	{
/*N*/ 		*pClassName 	= SvGlobalName(BF_SO3_SM_CLASSID_50);
/*N*/ 		*pFormat		= SOT_FORMATSTR_ID_STARMATH_50;
/*N*/ 		*pFullTypeName	= String(SmResId(STR_MATH_DOCUMENT_FULLTYPE_50));
/*N*/ 		*pShortTypeName = String(SmResId(RID_DOCUMENTSTR));
/*N*/ 	}
/*N*/ 	else if (nFileFormat == SOFFICE_FILEFORMAT_60 )
/*N*/ 	{
/*N*/ 		*pFullTypeName	= String(SmResId(STR_MATH_DOCUMENT_FULLTYPE_60));
/*N*/ 		*pShortTypeName = String(SmResId(RID_DOCUMENTSTR));

            // for binfilter, we need the FormatIDs to be set. Not setting them
            // has always been an error (!)
             *pClassName 	= SvGlobalName(BF_SO3_SM_CLASSID_60);
             *pFormat		= SOT_FORMATSTR_ID_STARMATH_60;

/*N*/ 	}
/*N*/ }


/*N*/ ULONG SmDocShell::GetMiscStatus() const
/*N*/ {
/*N*/ 	return SfxInPlaceObject::GetMiscStatus() | SVOBJ_MISCSTATUS_NOTRESIZEABLE
/*N*/ 											 | SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE;
/*N*/ }

/*N*/ void SmDocShell::SetModified(BOOL bModified)
/*N*/ {
/*N*/ 	if( IsEnableSetModified() )
/*N*/ 		SfxObjectShell::SetModified( bModified );
/*N*/ 	Broadcast(SfxSimpleHint(SFX_HINT_DOCCHANGED));
/*N*/ }




}
