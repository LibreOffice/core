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

#define _SCALC_EXE

#ifdef WIN
#include <svwin.h>
#endif

#include <bf_svtools/inettype.hxx>
#include <comphelper/classids.hxx>
#include <bf_sfx2/fcontnr.hxx>
#include <bf_sfx2/docfile.hxx>
#include <bf_sfx2/app.hxx>
#include <unotools/configitem.hxx>
#include <comphelper/types.hxx>
#include <sal/macros.h>

#include <sot/formats.hxx>
#define SOT_FORMATSTR_ID_STARCALC_30 SOT_FORMATSTR_ID_STARCALC

#include <signal.h>


#include "docsh.hxx"
#include "bf_sc.hrc"
#include "scdll0.hxx"
namespace binfilter {


//------------------------------------------------------------------

//	Filter-Namen (wie in docsh.cxx)

static const sal_Char __FAR_DATA pFilterSc50[]		= "StarCalc 5.0";
static const sal_Char __FAR_DATA pFilterSc50Temp[]	= "StarCalc 5.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterSc40[]		= "StarCalc 4.0";
static const sal_Char __FAR_DATA pFilterSc40Temp[]	= "StarCalc 4.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterSc30[]		= "StarCalc 3.0";
static const sal_Char __FAR_DATA pFilterSc30Temp[]	= "StarCalc 3.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterSc10[]		= "StarCalc 1.0";
static const sal_Char __FAR_DATA pFilterXML[]		= "StarOffice XML (Calc)";
static const sal_Char __FAR_DATA pFilterAscii[]		= "Text - txt - csv (StarCalc)";
static const sal_Char __FAR_DATA pFilterLotus[]		= "Lotus";
static const sal_Char __FAR_DATA pFilterExcel4[]	= "MS Excel 4.0";
static const sal_Char __FAR_DATA pFilterEx4Temp[]	= "MS Excel 4.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterExcel5[]	= "MS Excel 5.0/95";
static const sal_Char __FAR_DATA pFilterEx5Temp[]	= "MS Excel 5.0/95 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterExcel95[]	= "MS Excel 95";
static const sal_Char __FAR_DATA pFilterEx95Temp[]	= "MS Excel 95 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterExcel97[]	= "MS Excel 97";
static const sal_Char __FAR_DATA pFilterEx97Temp[]	= "MS Excel 97 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterDBase[]		= "dBase";
static const sal_Char __FAR_DATA pFilterDif[]		= "DIF";
static const sal_Char __FAR_DATA pFilterSylk[]		= "SYLK";
static const sal_Char __FAR_DATA pFilterHtml[]		= "HTML (StarCalc)";
static const sal_Char __FAR_DATA pFilterHtmlWeb[]	= "calc_HTML_WebQuery";
static const sal_Char __FAR_DATA pFilterRtf[]		= "Rich Text Format (StarCalc)";

//------------------------------------------------------------------

//	filter detection can't use ScFilterOptions (in sc-dll),
//	so access to wk3 flag must be implemented here again

/*N*/ class ScLibOptions : public ::utl::ConfigItem
/*N*/ {
/*N*/ 	BOOL	bWK3Flag;
/*N*/
/*N*/ public:
/*N*/ 			ScLibOptions();
/*N*/ 	BOOL	GetWK3Flag() const	{ return bWK3Flag; }

        virtual void    Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
        virtual void    Commit();

/*N*/ };

void ScLibOptions::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames ) {}
void ScLibOptions::Commit() {}

#define CFGPATH_LIBFILTER		"Office.Calc/Filter/Import/Lotus123"
#define ENTRYSTR_WK3			"WK3"

/*N*/ ScLibOptions::ScLibOptions() :
/*N*/ 	    ConfigItem( ::rtl::OUString::createFromAscii( CFGPATH_LIBFILTER ) ),
/*N*/ 	    bWK3Flag( FALSE )
/*N*/ {
/*N*/ 	::com::sun::star::uno::Sequence<rtl::OUString> aNames(1);
/*N*/ 	aNames[0] = ::rtl::OUString::createFromAscii( ENTRYSTR_WK3 );
/*N*/ 	::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> aValues = GetProperties(aNames);
/*N*/ 	if ( aValues.getLength() == 1 && aValues[0].hasValue() )
/*N*/ 		bWK3Flag = comphelper::getBOOL( aValues[0] );
/*N*/ }

//------------------------------------------------------------------

extern "C" { static void SAL_CALL thisModule() {} }

//	GlobalName der aktuellen Version:
/*N*/ SFX_IMPL_OBJECTFACTORY_LIB(ScDocShell, SFXOBJECTSHELL_STD_NORMAL, scalc,
/*N*/ 							SvGlobalName(BF_SO3_SC_CLASSID), Sc,
/*N*/ 							String( RTL_CONSTASCII_USTRINGPARAM( DLL_NAME ) ))
/*N*/ {
/*N*/ 	((SfxObjectFactory&)Factory()).
/*N*/ 			SetDocumentServiceName( ::rtl::OUString::createFromAscii(
/*N*/ 					"com.sun.star.sheet.SpreadsheetDocument" ) );
/*N*/
/*N*/ 	const String	aEmptyStr;
/*N*/ 									// Clipboard-IDs:
/*N*/ 	const ULONG		nSc50Format	 = SOT_FORMATSTR_ID_STARCALC_50;
/*N*/
/*N*/ 	String aVndCalc = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(CONTENT_TYPE_STR_APP_VND_CALC));
/*N*/
/*N*/ 	Factory().GetFilterContainer()->SetDetectFilter( ScDLL::DetectFilter );
/*N*/
/*N*/ 	//	5.0 muss mit vnd-Mime-Type registriert werden, aeltere mit dem alten x-starcalc
/*
    SFX_OWN_FILTER_REGISTRATION( ScDLL::DetectFilter,
                        String::CreateFromAscii(pFilterSc50),
                        String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("*.sdc")),
                        SFX_FILTER_OWN | SFX_FILTER_TEMPLATE |
                        SFX_FILTER_IMPORT | SFX_FILTER_EXPORT,
                        nSc50Format,
                        String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("SVsc0.sdc")),
                        String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("StarCalc 5.0")),
                        RID_SCICN_DOCUMENT,
                        aVndCalc, aEmptyStr );

    SFX_OWN_FILTER_REGISTRATION( ScDLL::DetectFilter,
                        String::CreateFromAscii(pFilterSc50Temp),
                        String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("*.vor")),
                        SFX_FILTER_OWN | SFX_FILTER_TEMPLATE | SFX_FILTER_TEMPLATEPATH |
                        SFX_FILTER_IMPORT | SFX_FILTER_EXPORT,
                        nSc50Format,
                        String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("SVsc1.vor")),
                        String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("StarCalc 5.0")),
                        RID_SCICN_TEMPLATE,
                        aVndCalc, aEmptyStr );
*/
/*N*/ 	//	alle Im-/Exportfilter werden nur noch per install.ini registriert,
/*N*/ 	//	damit sie bei der Installation weggelassen werden koennen.
/*N*/ }


// this is the right place for SFX_OBJECTFACTORY_LIB

//------------------------------------------------------------------
//
//	ScModuleDummy::Load() muss zur App gelinkt werden:
//
//------------------------------------------------------------------

SfxModule* ScModuleDummy::Load()
{
    if ( LoadLibSc() )
    {
        ScModuleDummy* pMod = SC_DLL();
        return pMod;
    }
    return NULL;
}

//------------------------------------------------------------------

/*N*/ ScDLL::ScDLL()
/*N*/ {
/*N*/ 	// the ctor is called at the beginning of SfxApplication-subclass::Main()
/*N*/ 	// do whatever you want, but no calls to Sxx-DLL-code!
/*N*/ }

/*N*/ ScDLL::~ScDLL()
/*N*/ {
/*N*/ 	// the dtor is called at the end of SfxApplication-subclass::Main()
/*N*/ 	// do whatever you want, but no calls to Sxx-DLL-code!
/*N*/ }

/*N*/ void ScDLL::LibInit()
/*N*/ {
/*N*/ 	// this method is called before Application::Execute()
/*N*/ 	// do whatever you want, but no calls to Sxx-DLL-code!
/*N*/
/*N*/ 	// RegisterFactory must now be before ScModuleDummy is created
/*N*/ 	ScDocShell::RegisterFactory( SDT_SC_DOCFACTPRIO );
/*N*/
/*N*/ 	// create a dummy-module for Object-Factory-Pointer
/*N*/ 	ScModuleDummy* pMod = new ScModuleDummy( NULL, TRUE, &ScDocShell::Factory() );
/*N*/ 	SC_DLL() = pMod;
/*N*/ }

/*N*/ void ScDLL::PreExit()
/*N*/ {
/*N*/ 	//	PreExit wird aus Exit() gerufen und loescht das Module.
/*N*/ 	//	Der Module-dtor muss alle Daten loeschen, die z.B. noch die Svx-DLL brauchen.
/*N*/
/*N*/ 	ScModuleDummy **ppShlPtr = (ScModuleDummy**) GetAppData(BF_SHL_CALC);
/*N*/ 	SvFactory *pFact = (SvFactory*)(*ppShlPtr)->pScDocShellFactory;
/*N*/ 	delete (*ppShlPtr);
/*N*/ 	(*ppShlPtr) = new ScModuleDummy( NULL, TRUE, NULL);
/*N*/ 	(*ppShlPtr)->pScDocShellFactory = pFact;
/*N*/
/*N*/ 	//	Der ModuleDummy mit der Factory wird z.B. noch bei SvFactory::DeInit() gebraucht.
/*N*/ }

/*N*/ void ScDLL::LibExit()
/*N*/ {
/*N*/ 	//	LibExit wird am Ende von Main() gerufen und gibt die DLL selber frei
/*N*/
/*N*/ 	FreeLibSc();	// DLL freigeben (ruft ScDLL::Exit)
/*N*/
/*N*/ 	//	das sollte jetzt nur noch ein Dummy sein:
/*N*/
/*N*/ 	ScModuleDummy **ppShlPtr = (ScModuleDummy**) GetAppData(BF_SHL_CALC);
/*N*/ 	delete (*ppShlPtr);
/*N*/ 	(*ppShlPtr) = NULL;
/*N*/ }

//------------------------------------------------------------------

/*N*/ BOOL lcl_MayBeAscii( SvStream& rStream )
/*N*/ {
/*N*/ 	//	ASCII is considered possible if there are no null bytes
/*N*/
/*N*/ 	rStream.Seek(STREAM_SEEK_TO_BEGIN);
/*N*/
/*N*/ 	BOOL bNullFound = FALSE;
/*N*/ 	BYTE aBuffer[ 4097 ];
/*N*/ 	const BYTE* p = aBuffer;
/*N*/ 	ULONG nBytesRead = rStream.Read( aBuffer, 4096 );
/*N*/
/*N*/ 	if ( nBytesRead >= 2 &&
/*N*/ 			( ( aBuffer[0] == 0xff && aBuffer[1] == 0xfe ) ||
/*N*/ 			  ( aBuffer[0] == 0xfe && aBuffer[1] == 0xff ) ) )
/*N*/ 	{
/*N*/ 		//	unicode file may contain null bytes
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/
/*N*/ 	while( nBytesRead-- )
/*N*/ 		if( !*p++ )
/*N*/ 		{
/*N*/ 			bNullFound = TRUE;
/*N*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 	return !bNullFound;
/*N*/ }

/*N*/ BOOL lcl_MayBeDBase( SvStream& rStream )
/*N*/ {
/*N*/ 	//	for dBase, look for the 0d character at the end of the header
/*N*/
/*N*/ 	rStream.Seek(STREAM_SEEK_TO_END);
/*N*/ 	ULONG nSize = rStream.Tell();
/*N*/
/*N*/ 	// length of header starts at 8
/*N*/
/*N*/ 	if ( nSize < 10 )
/*N*/ 		return FALSE;
/*N*/ 	rStream.Seek(8);
/*N*/ 	USHORT nHeaderLen;
/*N*/ 	rStream >> nHeaderLen;
/*N*/
/*N*/ 	if ( nHeaderLen < 32 || nSize < nHeaderLen )
/*N*/ 		return FALSE;
/*N*/
/*N*/ 	// last byte of header must be 0d
/*N*/
/*N*/ 	rStream.Seek( nHeaderLen - 1 );
/*N*/ 	BYTE nEndFlag;
/*N*/ 	rStream >> nEndFlag;
/*N*/
/*N*/ 	return ( nEndFlag == 0x0d );
/*N*/ }

BOOL lcl_IsAnyXMLFilter( const SfxFilter* pFilter )
{
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if ( !pFilter )
        return FALSE;
}

/*N*/ ULONG __EXPORT ScDLL::DetectFilter( SfxMedium& rMedium, const SfxFilter** ppFilter,
/*N*/ 									SfxFilterFlags nMust, SfxFilterFlags nDont )
/*N*/ {
/*N*/ 	//	#59915# laut MBA darf hier nur ERRCODE_NONE, ERRCODE_ABORT und ERRCODE_FORCEQUIET
/*N*/ 	//	zurueckgegeben werden...
/*N*/
/*N*/ 	if ( SVSTREAM_OK != rMedium.GetError() )
/*N*/ 		return ERRCODE_ABORT;	// ERRCODE_IO_GENERAL
/*N*/
/*N*/ 	//	Formate, die sicher erkannt werden:
/*N*/
/*N*/ 	SvStorage* pStorage = rMedium.GetStorage();
/*N*/ 	if ( pStorage )
/*N*/ 	{
/*N*/ 		String		aStreamName;
/*N*/
/*N*/ 		// Erkennung ueber contained streams
/*N*/ 		// Excel-5 / StarCalc 3.0
/*N*/
/*N*/ 		aStreamName = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Workbook"));
/*N*/ 		BOOL bExcel97Stream = ( pStorage->IsContained( aStreamName ) && pStorage->IsStream( aStreamName ) );
/*N*/
/*N*/ 		aStreamName = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Book"));
/*N*/ 		BOOL bExcel5Stream = ( pStorage->IsContained( aStreamName ) && pStorage->IsStream( aStreamName ) );
/*N*/
/*N*/ 		if ( bExcel97Stream )
/*N*/ 		{
/*?*/ 			String aOldName;
/*?*/ 			if ( *ppFilter ) aOldName = (*ppFilter)->GetFilterName();
/*?*/ 			if ( aOldName.EqualsAscii(pFilterEx97Temp) )
/*?*/ 			{
/*?*/ 				//	Excel 97 template selected -> keep selection
/*?*/ 			}
/*?*/ 			else if ( bExcel5Stream &&
/*?*/ 						( aOldName.EqualsAscii(pFilterExcel5) || aOldName.EqualsAscii(pFilterEx5Temp) ||
/*?*/ 						  aOldName.EqualsAscii(pFilterExcel95) || aOldName.EqualsAscii(pFilterEx95Temp) ) )
/*?*/ 			{
/*?*/ 				//	dual format file and Excel 5 selected -> keep selection
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				//	else use Excel 97 filter
/*?*/ 				*ppFilter = SFX_APP()->GetFilter( ScDocShell::Factory(),
/*?*/ 									  String::CreateFromAscii(pFilterExcel97) );
/*?*/ 			}
/*?*/ 			return ERRCODE_NONE;
/*N*/ 		}
/*N*/ 		if ( bExcel5Stream )
/*N*/ 		{
/*?*/ 			String aOldName;
/*?*/ 			if ( *ppFilter ) aOldName = (*ppFilter)->GetFilterName();
/*?*/ 			if ( aOldName.EqualsAscii(pFilterExcel95) || aOldName.EqualsAscii(pFilterEx95Temp) ||
/*?*/ 					aOldName.EqualsAscii(pFilterEx5Temp) )
/*?*/ 			{
/*?*/ 				//	Excel 95 oder Vorlage (5 oder 95) eingestellt -> auch gut
/*?*/ 			}
/*?*/             else if ( aOldName.EqualsAscii(pFilterEx97Temp) )
/*?*/             {
/*?*/                 // #101923# auto detection has found template -> return Excel5 template
/*?*/                 *ppFilter = SFX_APP()->GetFilter( ScDocShell::Factory(),
/*?*/                                         String::CreateFromAscii(pFilterEx5Temp) );
/*?*/             }
/*?*/ 			else
/*?*/ 			{
/*?*/ 				//	sonst wird als Excel 5-Datei erkannt
/*?*/ 				*ppFilter = SFX_APP()->GetFilter( ScDocShell::Factory(),
/*?*/ 										String::CreateFromAscii(pFilterExcel5) );
/*?*/ 			}
/*?*/ 			return ERRCODE_NONE;
/*N*/ 		}
/*N*/
/*N*/ 		aStreamName = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(STRING_SCSTREAM));
/*N*/ 		if ( pStorage->IsContained( aStreamName ) && pStorage->IsStream( aStreamName ) )
/*N*/ 		{
/*N*/ 			//	Unterscheidung 3.0 / 4.0 / 5.0 ueber Clipboard-Id
/*N*/ 			ULONG nStorFmt = pStorage->GetFormat();
/*N*/ 			if ( nStorFmt == SOT_FORMATSTR_ID_STARCALC_30 )
/*?*/ 				*ppFilter = SFX_APP()->GetFilter( ScDocShell::Factory(),
/*?*/ 											String::CreateFromAscii(pFilterSc30) );
/*N*/ 			else if ( nStorFmt == SOT_FORMATSTR_ID_STARCALC_40 )
/*?*/ 				*ppFilter = SFX_APP()->GetFilter( ScDocShell::Factory(),
/*?*/ 											String::CreateFromAscii(pFilterSc40) );
/*N*/ 			else
/*N*/ 				*ppFilter = SFX_APP()->GetFilter( ScDocShell::Factory(),
/*N*/ 											String::CreateFromAscii(pFilterSc50) );
/*N*/ 			return ERRCODE_NONE;
/*N*/ 		}
/*N*/
/*N*/ 		//	XML package file: Stream "Content.xml" or "content.xml"
/*?*/ 		aStreamName = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("content.xml"));
/*?*/ 		String aOldXML = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Content.xml"));
/*?*/ 		if ( ( pStorage->IsContained( aStreamName ) && pStorage->IsStream( aStreamName ) ) ||
/*?*/ 			 ( pStorage->IsContained( aOldXML ) && pStorage->IsStream( aOldXML ) ) )
/*?*/ 		{
/*?*/ 			//	#85794# don't accept other applications' xml formats,
/*?*/ 			//	recognized by clipboard id
/*?*/ 			ULONG nStorageFormat = pStorage->GetFormat();
/*?*/ 			if ( nStorageFormat == 0 || nStorageFormat == SOT_FORMATSTR_ID_STARCALC_60 )
/*?*/ 			{
/*?*/ 				//	if XML template is set, don't modify
/*?*/ 				if (!lcl_IsAnyXMLFilter(*ppFilter))
/*?*/ 					*ppFilter = SFX_APP()->GetFilter( ScDocShell::Factory(),
/*?*/ 													  String::CreateFromAscii(pFilterXML) );
/*?*/ 				return ERRCODE_NONE;
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else 	// no storage
/*N*/ 	{
/*N*/ 		SvStream &rStr = *rMedium.GetInStream();
/*N*/ 		if ( &rStr == NULL )
/*N*/ 			return ERRCODE_ABORT;	// ERRCODE_IO_GENERAL
/*N*/
/*N*/ 		// Tabelle mit Suchmustern
/*N*/ 		// Bedeutung der Sequenzen
/*N*/ 		// 0x00??: genau Byte 0x?? muss an dieser Stelle stehen
/*N*/ 		// 0x0100: ein Byte ueberlesen (don't care)
/*N*/ 		// 0x02nn: ein Byte aus 0xnn Alternativen folgt
/*N*/ 		// 0x8000: Erkennung abgeschlossen
/*N*/ 		//
/*N*/
/*N*/ #define M_DC		0x0100
/*N*/ #define M_ALT(ANZ)	0x0200+ANZ
/*N*/ #define M_ENDE		0x8000
/*N*/
/*N*/ 		const UINT16 pLotus[] = 		// Lotus 1/1A/2
/*N*/ 			{ 0x0000, 0x0000, 0x0002, 0x0000,
/*N*/ 			  M_ALT(2), 0x0004, 0x0006,
/*N*/ 			  0x0004, M_ENDE };
/*N*/
/*N*/ 		const UINT16 pExcel1[] =		// Excel Biff/3/4 Tabellen
/*N*/ 			{ 0x0009,
/*N*/ 			  M_ALT(2), 0x0002, 0x0004,
/*N*/ 			  0x0006, 0x0000, M_DC, M_DC, 0x0010, 0x0000,
/*N*/ 			  M_DC, M_DC, M_ENDE };
/*N*/
/*N*/ 		const UINT16 pExcel2[] =		// Excel Biff3/4 Workbooks
/*N*/ 			{ 0x0009,
/*N*/ 			  M_ALT(2), 0x0002, 0x0004,
/*N*/ 			  0x0006, 0x0000, M_DC, M_DC, 0x0000, 0x0001,
/*N*/ 			  M_DC, M_DC, M_ENDE };
/*N*/
/*N*/ 		const UINT16 pExcel3[] =		// Excel Biff2 Tabellen
/*N*/ 			{ 0x0009, 0x0000, 0x0004, 0x0000,
/*N*/ 			  M_DC, M_DC, 0x0010, 0x0000, M_ENDE };
/*N*/
/*N*/ 		const UINT16 pSc10[] =			// StarCalc 1.0 Dokumente
/*N*/ 			{ 'B', 'l', 'a', 'i', 's', 'e', '-', 'T', 'a', 'b', 'e', 'l', 'l',
/*N*/ 			  'e', 0x000A, 0x000D, 0x0000,    // Sc10CopyRight[16]
/*N*/ 			  M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC,
/*N*/ 			  M_DC, M_DC, 					// Sc10CopyRight[29]
/*N*/ 			  M_ALT(2), 0x0065, 0x0066,		// Versionsnummer 101 oder 102
/*N*/ 			  0x0000,
/*N*/ 			  M_ENDE };
/*N*/
/*N*/ 		const UINT16 pLotus2[] =		// Lotus >3
/*N*/ 			{ 0x0000, 0x0000, 0x001A, 0x0000,	// Rec# + Len (26)
/*N*/ 			  M_ALT(2), 0x0000, 0x0002,			// File Revision Code
/*N*/ 			  0x0010,
/*N*/ 			  0x0004, 0x0000,					// File Revision Subcode
/*N*/ 			  M_ENDE };
/*N*/
/*N*/ 		const UINT16 pDIF1[] =			// DIF mit CR-LF
/*N*/ 			{
/*N*/ 			'T', 'A', 'B', 'L', 'E',
/*N*/ 			M_DC, M_DC,
/*N*/ 			'0', ',', '1',
/*N*/ 			M_DC, M_DC,
/*N*/ 			'\"',
/*N*/ 			M_ENDE };
/*N*/
/*N*/ 		const UINT16 pDIF2[] =			// DIF mit CR oder LF
/*N*/ 			{
/*N*/ 			'T', 'A', 'B', 'L', 'E',
/*N*/ 			M_DC,
/*N*/ 			'0', ',', '1',
/*N*/ 			M_DC,
/*N*/ 			'\"',
/*N*/ 			M_ENDE };
/*N*/
/*N*/ 		const UINT16 pSylk[] =			// Sylk
/*N*/ 			{
/*N*/ 			'I', 'D', ';', 'P',
/*N*/ 			M_ENDE };
/*N*/
/*N*/ #ifdef SINIX
/*N*/ 		const UINT16 nAnzMuster = 9;	// sollte fuer indiz. Zugriff stimmen...
/*N*/ 		UINT16 *ppMuster[ nAnzMuster ];			// Arrays mit Suchmustern
/*N*/ 		ppMuster[ 0 ] = pLotus;
/*N*/ 		ppMuster[ 1 ] = pExcel1;
/*N*/ 		ppMuster[ 2 ] = pExcel2;
/*N*/ 		ppMuster[ 3 ] = pExcel3;
/*N*/ 		ppMuster[ 4 ] = pSc10;
/*N*/ 		ppMuster[ 5 ] = pDIF1;
/*N*/ 		ppMuster[ 6 ] = pDIF2;
/*N*/ 		ppMuster[ 7 ] = pSylk;
/*N*/ 		ppMuster[ 8 ] = pLotus2;				// Lotus immer ganz hinten wegen Ini-Eintrag
/*N*/ #else
/*N*/ 		const UINT16 *ppMuster[] =		// Arrays mit Suchmustern
/*N*/ 			{
/*N*/ 			pLotus,
/*N*/ 			pExcel1,
/*N*/ 			pExcel2,
/*N*/ 			pExcel3,
/*N*/ 			pSc10,
/*N*/ 			pDIF1,
/*N*/ 			pDIF2,
/*N*/ 			pSylk,
/*N*/ 			pLotus2
/*N*/ 			};
/*N*/ 		const UINT16 nAnzMuster = SAL_N_ELEMENTS(ppMuster);
/*N*/ #endif
/*N*/
/*N*/ 		const sal_Char* pFilterName[ nAnzMuster ] = 	// zugehoerige Filter
/*N*/ 			{
/*N*/ 			pFilterLotus,
/*N*/ 			pFilterExcel4,
/*N*/ 			pFilterExcel4,
/*N*/ 			pFilterExcel4,
/*N*/ 			pFilterSc10,
/*N*/ 			pFilterDif,
/*N*/ 			pFilterDif,
/*N*/ 			pFilterSylk,
/*N*/ 			pFilterLotus
/*N*/ 			};
/*N*/
/*N*/ 		// suchen Sie jetzt!
/*N*/ 		// ... realisiert ueber 'Mustererkennung'
/*N*/
/*N*/ 		BYTE			nAkt;
/*N*/ 		BOOL			bSync;			// Datei und Muster stimmen ueberein
/*N*/ 		USHORT			nFilter;		// Zaehler ueber alle Filter
/*N*/ 		const UINT16	*pSearch;		// aktuelles Musterwort
/*N*/ 		UINT16			nFilterLimit = nAnzMuster;
/*N*/
/*N*/ 		// nur solange, bis es etwas Globales gibt
/*N*/ 		// funzt nur, solange Eintraege fuer WK3 letzte Muster-Tabelle ist!
/*N*/ 		ScLibOptions aLibOpt;
/*N*/ 		if( !aLibOpt.GetWK3Flag() )
/*N*/ 			nFilterLimit--;
/*N*/
/*N*/ 		for ( nFilter = 0 ; nFilter < nFilterLimit ; nFilter++ )
/*N*/ 		{
/*N*/ 			rStr.Seek( 0 ); // am Anfang war alles Uebel...
/*N*/ 			rStr >> nAkt;
/*N*/ 			pSearch = ppMuster[ nFilter ];
/*N*/ 			bSync = TRUE;
/*N*/ 			while( !rStr.IsEof() && bSync )
/*N*/ 			{
/*N*/ 				register UINT16 nMuster = *pSearch;
/*N*/
/*N*/ 				if( nMuster < 0x0100 )
/*N*/ 				{ // 								direkter Byte-Vergleich
/*N*/ 					if( ( BYTE ) nMuster != nAkt )
/*N*/ 						bSync = FALSE;
/*N*/ 				}
/*N*/ 				else if( nMuster & M_DC )
/*N*/ 				{ // 											 don't care
/*N*/ 				}
/*N*/ 				else if( nMuster & M_ALT(0) )
/*N*/ 				{ // 									  alternative Bytes
/*N*/ 					BYTE nAnzAlt = ( BYTE ) nMuster;
/*N*/ 					bSync = FALSE;			// zunaechst unsynchron
/*N*/ 					while( nAnzAlt > 0 )
/*N*/ 					{
/*N*/ 						pSearch++;
/*N*/ 						if( ( BYTE ) *pSearch == nAkt )
/*N*/ 							bSync = TRUE;	// jetzt erst Synchronisierung
/*N*/ 						nAnzAlt--;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else if( nMuster & M_ENDE )
/*N*/ 				{ // 										Format detected
/*?*/ 					if ( pFilterName[nFilter] == pFilterExcel4 && *ppFilter &&
/*?*/ 							(*ppFilter)->GetFilterName().EqualsAscii(pFilterEx4Temp) )
/*?*/ 					{
/*?*/ 						//	Excel 4 erkannt, Excel 4 Vorlage eingestellt -> auch gut
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{	// gefundenen Filter einstellen
/*?*/ 						*ppFilter = SFX_APP()->GetFilter(
/*?*/ 							ScDocShell::Factory(),
/*?*/ 							String::CreateFromAscii(pFilterName[ nFilter ]) );
/*?*/ 					}
/*?*/
/*?*/ 					return ERRCODE_NONE;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{ // 										 Tabellenfehler
/*N*/ 					DBG_ERROR( "-ScApplication::DetectFilter(): Fehler in Mustertabelle");
/*N*/ 				}
/*N*/
/*N*/ 				pSearch++;
/*N*/ 				rStr >> nAkt;
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/         String aPresetFilterName;
/*N*/         if ( *ppFilter )
/*N*/             aPresetFilterName = (*ppFilter)->GetFilterName();
/*N*/
/*N*/         // ASCII cannot be recognized.
/*N*/         // #i3341# But if the Text/CSV filter was set (either by the user or
/*N*/         // file extension) it takes precedence over HTML and RTF and dBase
/*N*/         // detection. Otherwise something like, for example, "lala <SUP> gugu"
/*N*/         // would trigger HTML to be recognized.
/*N*/
/*N*/         if ( aPresetFilterName.EqualsAscii(pFilterAscii) && lcl_MayBeAscii( rStr ) )
/*N*/             return ERRCODE_NONE;
/*N*/
/*N*/         // get file header
/*N*/
/*N*/ 		rStr.Seek( 0 );
/*N*/ 		const int nTrySize = 80;
/*N*/ 		ByteString aHeader;
/*N*/ 		for ( int j = 0; j < nTrySize && !rStr.IsEof(); j++ )
/*N*/ 		{
/*N*/ 			sal_Char c;
/*N*/ 			rStr >> c;
/*N*/ 			aHeader += c;
/*N*/ 		}
/*N*/ 		aHeader += '\0';

        // test for HTML

        // #97832#; we don't have a flat xml filter
/*		if ( aHeader.CompareTo( "<?xml", 5 ) == COMPARE_EQUAL )
        {
            //	if XML template is set, don't modify
            if (!lcl_IsAnyXMLFilter(*ppFilter))
                *ppFilter = SFX_APP()->GetFilter( ScDocShell::Factory(),
                                                  String::CreateFromAscii(pFilterXML) );
            return ERRCODE_NONE;
        }*/

        // dBase cannot safely be recognized - only test if the filter was set
/*N*/         if ( aPresetFilterName.EqualsAscii(pFilterDBase) && lcl_MayBeDBase( rStr ) )
/*N*/             return ERRCODE_NONE;
/*N*/ 	}
/*N*/
/*N*/ 	return ERRCODE_ABORT;		// war nix
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
