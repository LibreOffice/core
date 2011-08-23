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

//------------------------------------------------------------------



#include "appoptio.hxx"
#include "rechead.hxx"
#include "userlist.hxx"
#include "bf_sc.hrc"
#include "compiler.hrc"
#include "miscuno.hxx"
namespace binfilter {

using namespace utl;
using namespace rtl;
using namespace ::com::sun::star::uno;

// STATIC DATA -----------------------------------------------------------

#define SC_VERSION ((USHORT)304)

//========================================================================
//      ScAppOptions - Applikations-Optionen
//========================================================================

/*N*/ ScAppOptions::ScAppOptions() : pLRUList( NULL )
/*N*/ {
/*N*/ 	SetDefaults();
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScAppOptions::ScAppOptions( const ScAppOptions& rCpy ) : pLRUList( NULL )
/*N*/ {
/*N*/ 	*this = rCpy;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScAppOptions::~ScAppOptions()
/*N*/ {
/*N*/ 	delete [] pLRUList;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void ScAppOptions::SetDefaults()
/*N*/ {
/*N*/ 	if ( ScOptionsUtil::IsMetricSystem() )
/*N*/ 		eMetric		= FUNIT_CM;				// default for countries with metric system
/*N*/ 	else
/*N*/ 		eMetric		= FUNIT_INCH;			// default for others
/*N*/ 
/*N*/ 	nZoom 			= 100;
/*N*/ 	eZoomType		= SVX_ZOOM_PERCENT;
/*N*/ 	nStatusFunc		= SUBTOTAL_FUNC_SUM;
/*N*/ 	bAutoComplete	= TRUE;
/*N*/ 	bDetectiveAuto	= TRUE;
/*N*/ 
/*N*/ 	delete [] pLRUList;
/*N*/ 	pLRUList = new USHORT[5];				// sinnvoll vorbelegen
/*N*/ 	pLRUList[0] = SC_OPCODE_SUM;
/*N*/ 	pLRUList[1] = SC_OPCODE_AVERAGE;
/*N*/ 	pLRUList[2] = SC_OPCODE_MIN;
/*N*/ 	pLRUList[3] = SC_OPCODE_MAX;
/*N*/ 	pLRUList[4] = SC_OPCODE_IF;
/*N*/ 	nLRUFuncCount = 5;
/*N*/ 
/*N*/ 	nTrackContentColor = COL_TRANSPARENT;
/*N*/ 	nTrackInsertColor  = COL_TRANSPARENT;
/*N*/ 	nTrackDeleteColor  = COL_TRANSPARENT;
/*N*/ 	nTrackMoveColor    = COL_TRANSPARENT;
/*N*/ 	eLinkMode		   = LM_ON_DEMAND;
/*N*/ 
/*N*/ 	nDefaultObjectSizeWidth = 8000;
/*N*/ 	nDefaultObjectSizeHeight = 5000;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ const ScAppOptions& ScAppOptions::operator=( const ScAppOptions& rCpy )
/*N*/ {
/*N*/ 	eMetric			= rCpy.eMetric;
/*N*/ 	eZoomType		= rCpy.eZoomType;
/*N*/ 	nZoom			= rCpy.nZoom;
/*N*/ 	SetLRUFuncList( rCpy.pLRUList, rCpy.nLRUFuncCount );
/*N*/ 	nStatusFunc		= rCpy.nStatusFunc;
/*N*/ 	bAutoComplete	= rCpy.bAutoComplete;
/*N*/ 	bDetectiveAuto	= rCpy.bDetectiveAuto;
/*N*/ 	nTrackContentColor = rCpy.nTrackContentColor;
/*N*/ 	nTrackInsertColor  = rCpy.nTrackInsertColor;
/*N*/ 	nTrackDeleteColor  = rCpy.nTrackDeleteColor;
/*N*/ 	nTrackMoveColor    = rCpy.nTrackMoveColor;
/*N*/ 	eLinkMode		= rCpy.eLinkMode;
/*N*/ 	nDefaultObjectSizeWidth = rCpy.nDefaultObjectSizeWidth;
/*N*/ 	nDefaultObjectSizeHeight = rCpy.nDefaultObjectSizeHeight;
/*N*/ 	return *this;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ SvStream& operator>>( SvStream& rStream, ScAppOptions& rOpt )
/*N*/ {
/*N*/ 	rOpt.SetDefaults();
/*N*/ 
/*N*/ 	ScReadHeader aHdr( rStream );
/*N*/ 
/*N*/ 	BYTE n;
/*N*/ 	rStream >> n; rOpt.eMetric = (FieldUnit)n;
/*N*/ 
/*N*/ 	if ( aHdr.BytesLeft() )
/*N*/ 	{
/*N*/ 		rStream >> n; rOpt.nLRUFuncCount = (USHORT)n;
/*N*/ 
/*N*/ 		delete [] rOpt.pLRUList;
/*N*/ 		rOpt.pLRUList = new USHORT[rOpt.nLRUFuncCount];
/*N*/ 
/*N*/ 		for ( USHORT i=0; i<rOpt.nLRUFuncCount; i++ )
/*N*/ 			rStream >> rOpt.pLRUList[i];
/*N*/ 	}
/*N*/ 
/*N*/ 	// ab 31.10.95: globale benutzerdefinierte Listen einlesen :-/
/*N*/ 	//              (kleiner Hack :-/)
/*N*/ 	if ( aHdr.BytesLeft() )
/*N*/ 	{
/*N*/ 		ScUserList* pUserList = ScGlobal::GetUserList();
/*N*/ 		pUserList->Load( rStream );
/*N*/ 	}
/*N*/ 
/*N*/ 	// ab 11.12.95 (304)
/*N*/ 	// Zoom-Faktor
/*N*/ 	if ( aHdr.BytesLeft() )
/*N*/ 	{
/*N*/ 		USHORT e;
/*N*/ 		rStream >> e; rOpt.eZoomType = (SvxZoomType)e;
/*N*/ 		rStream >> rOpt.nZoom;
/*N*/ 	}
/*N*/ 
/*N*/ 	// ab 23.5.96: Funktion fuer Statusbar-Controller, Flag fuer Auto-Eingabe
/*N*/ 	if ( aHdr.BytesLeft() )
/*N*/ 	{
/*N*/ 		rStream >> rOpt.nStatusFunc;
/*N*/ 		rStream >> rOpt.bAutoComplete;
/*N*/ 	}
/*N*/ 
/*N*/ 	// ab 15.3.98: Farben fuer Change-Tracking
/*N*/ 	if ( aHdr.BytesLeft() )
/*N*/ 	{
/*N*/ 		rStream >> rOpt.nTrackContentColor;
/*N*/ 		rStream >> rOpt.nTrackInsertColor;
/*N*/ 		rStream >> rOpt.nTrackDeleteColor;
/*N*/ 		rStream >> rOpt.nTrackMoveColor;
/*N*/ 	}
/*N*/ 
/*N*/ 	// ab 22.6.98: Automatisches Detektiv-Update
/*N*/ 	if ( aHdr.BytesLeft() )
/*N*/ 		rStream >> rOpt.bDetectiveAuto;
/*N*/ 
/*N*/ 	if ( aHdr.BytesLeft() )
/*N*/ 	{
/*N*/ 		BYTE nLinkMode;
/*N*/ 		rStream >> nLinkMode;
/*N*/ 		rOpt.eLinkMode=(ScLkUpdMode) nLinkMode;
/*N*/ 	}
/*N*/ 
/*N*/ 	return rStream;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ SvStream& operator<<( SvStream& rStream, const ScAppOptions& rOpt )
/*N*/ {
/*N*/ 	ScWriteHeader aHdr( rStream, 25 );
/*N*/ 
/*N*/ 	rStream << (BYTE)rOpt.eMetric;
/*N*/ 	rStream << (BYTE)rOpt.nLRUFuncCount;
/*N*/ 
/*N*/ 	if ( rOpt.nLRUFuncCount > 0 )
/*N*/ 	{
/*N*/ 		for ( USHORT i=0; i<rOpt.nLRUFuncCount; i++ )
/*N*/ 			rStream << rOpt.pLRUList[i];
/*N*/ 	}
/*N*/ 
/*N*/ 	// ab 31.10.95: globale benutzerdefinierte Listen speichern
/*N*/ 	//              (kleiner Hack :-/)
/*N*/ 	ScUserList* pUserList = ScGlobal::GetUserList();
/*N*/ 	pUserList->Store( rStream );
/*N*/ 
/*N*/ 	// ab 11.12.95 (304)
/*N*/ 	// Zoom-Faktor
/*N*/ 	rStream << (USHORT)rOpt.eZoomType;
/*N*/ 	rStream << rOpt.nZoom;
/*N*/ 
/*N*/ 	// ab 23.5.96: Funktion fuer Statusbar-Controller, Flag fuer Auto-Eingabe
/*N*/ 	rStream << rOpt.nStatusFunc;
/*N*/ 	rStream << rOpt.bAutoComplete;
/*N*/ 
/*N*/ 	// ab 15.3.98: Farben fuer Change-Tracking
/*N*/ 	rStream << rOpt.nTrackContentColor;
/*N*/ 	rStream << rOpt.nTrackInsertColor;
/*N*/ 	rStream << rOpt.nTrackDeleteColor;
/*N*/ 	rStream << rOpt.nTrackMoveColor;
/*N*/ 
/*N*/ 	// ab 22.6.98: Automatisches Detektiv-Update
/*N*/ 	rStream << rOpt.bDetectiveAuto;
/*N*/ 	rStream << (BYTE) rOpt.eLinkMode;
/*N*/ 
/*N*/ 	return rStream;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void ScAppOptions::SetLRUFuncList( const USHORT* pList,	const USHORT nCount )
/*N*/ {
/*N*/ 	delete [] pLRUList;
/*N*/ 
/*N*/ 	nLRUFuncCount = nCount;
/*N*/ 
/*N*/ 	if ( nLRUFuncCount > 0 )
/*N*/ 	{
/*N*/ 		pLRUList = new USHORT[nLRUFuncCount];
/*N*/ 
/*N*/ 		for ( USHORT i=0; i<nLRUFuncCount; i++ )
/*N*/ 			pLRUList[i] = pList[i];
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pLRUList = NULL;
/*N*/ }

//==================================================================
//	Config Item containing app options
//==================================================================

/*N*/ void lcl_SetLastFunctions( ScAppOptions& rOpt, const Any& rValue )
/*N*/ {
/*N*/ 	Sequence<sal_Int32> aSeq;
/*N*/ 	if ( rValue >>= aSeq )
/*N*/ 	{
/*N*/ 		long nCount = aSeq.getLength();
/*N*/ 		if ( nCount < USHRT_MAX )
/*N*/ 		{
/*N*/ 			const sal_Int32* pArray = aSeq.getConstArray();
/*N*/ 			USHORT* pUShorts = new USHORT[nCount];
/*N*/ 			for (long i=0; i<nCount; i++)
/*N*/ 				pUShorts[i] = (USHORT) pArray[i];
/*N*/ 
/*N*/ 			rOpt.SetLRUFuncList( pUShorts, nCount );
/*N*/ 
/*N*/ 			delete[] pUShorts;
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void lcl_SetSortList( const Any& rValue )
/*N*/ {
/*N*/ 	Sequence<OUString> aSeq;
/*N*/ 	if ( rValue >>= aSeq )
/*N*/ 	{
/*N*/ 		long nCount = aSeq.getLength();
/*N*/ 		const OUString* pArray = aSeq.getConstArray();
/*N*/ 		ScUserList aList;
/*N*/ 
/*N*/ 		//	if setting is "default", keep default values from ScUserList ctor
/*N*/ 		//!	mark "default" in a safe way
/*N*/ 		BOOL bDefault = ( nCount == 1 &&
/*N*/ 						pArray[0].equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "NULL" ) ) );
/*N*/ 
/*N*/ 		if (!bDefault)
/*N*/ 		{
/*?*/ 			aList.FreeAll();
/*?*/ 
/*?*/ 			for (long i=0; i<nCount; i++)
/*?*/ 			{
/*?*/ 				ScUserListData* pNew = new ScUserListData( pArray[i] );
/*?*/ 				if ( !aList.Insert(pNew) )
/*?*/ 					delete pNew;
/*?*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		ScGlobal::SetUserList( &aList );
/*N*/ 	}
/*N*/ }


//------------------------------------------------------------------

#define CFGPATH_LAYOUT		"Office.Calc/Layout"

#define SCLAYOUTOPT_MEASURE			0
#define SCLAYOUTOPT_STATUSBAR		1
#define SCLAYOUTOPT_ZOOMVAL			2
#define SCLAYOUTOPT_ZOOMTYPE		3
#define SCLAYOUTOPT_COUNT			4

#define CFGPATH_INPUT		"Office.Calc/Input"

#define SCINPUTOPT_LASTFUNCS		0
#define SCINPUTOPT_AUTOINPUT		1
#define SCINPUTOPT_DET_AUTO			2
#define SCINPUTOPT_COUNT			3

#define CFGPATH_REVISION	"Office.Calc/Revision/Color"

#define SCREVISOPT_CHANGE			0
#define SCREVISOPT_INSERTION		1
#define SCREVISOPT_DELETION			2
#define SCREVISOPT_MOVEDENTRY		3
#define SCREVISOPT_COUNT			4

#define CFGPATH_CONTENT		"Office.Calc/Content/Update"

#define SCCONTENTOPT_LINK			0
#define SCCONTENTOPT_COUNT			1

#define CFGPATH_SORTLIST	"Office.Calc/SortList"

#define SCSORTLISTOPT_LIST			0
#define SCSORTLISTOPT_COUNT			1

#define CFGPATH_MISC		"Office.Calc/Misc"

#define SCMISCOPT_DEFOBJWIDTH		0
#define SCMISCOPT_DEFOBJHEIGHT		1
#define SCMISCOPT_COUNT				2


/*N*/ Sequence<OUString> ScAppCfg::GetLayoutPropertyNames()
/*N*/ {
/*N*/ 	static const char* aPropNames[] =
/*N*/ 	{
/*N*/ 		"Other/MeasureUnit/NonMetric",	// SCLAYOUTOPT_MEASURE
/*N*/ 		"Other/StatusbarFunction",		// SCLAYOUTOPT_STATUSBAR
/*N*/ 		"Zoom/Value",					// SCLAYOUTOPT_ZOOMVAL
/*N*/ 		"Zoom/Type"						// SCLAYOUTOPT_ZOOMTYPE
/*N*/ 	};
/*N*/ 	Sequence<OUString> aNames(SCLAYOUTOPT_COUNT);
/*N*/ 	OUString* pNames = aNames.getArray();
/*N*/ 	for(int i = 0; i < SCLAYOUTOPT_COUNT; i++)
/*N*/ 		pNames[i] = OUString::createFromAscii(aPropNames[i]);
/*N*/ 
/*N*/ 	//	adjust for metric system
/*N*/ 	if (ScOptionsUtil::IsMetricSystem())
/*N*/ 		pNames[SCLAYOUTOPT_MEASURE] = OUString::createFromAscii( "Other/MeasureUnit/Metric" );
/*N*/ 
/*N*/ 	return aNames;
/*N*/ }

/*N*/ Sequence<OUString> ScAppCfg::GetInputPropertyNames()
/*N*/ {
/*N*/ 	static const char* aPropNames[] =
/*N*/ 	{
/*N*/ 		"LastFunctions",			// SCINPUTOPT_LASTFUNCS
/*N*/ 		"AutoInput",				// SCINPUTOPT_AUTOINPUT
/*N*/ 		"DetectiveAuto"				// SCINPUTOPT_DET_AUTO
/*N*/ 	};
/*N*/ 	Sequence<OUString> aNames(SCINPUTOPT_COUNT);
/*N*/ 	OUString* pNames = aNames.getArray();
/*N*/ 	for(int i = 0; i < SCINPUTOPT_COUNT; i++)
/*N*/ 		pNames[i] = OUString::createFromAscii(aPropNames[i]);
/*N*/ 
/*N*/ 	return aNames;
/*N*/ }

/*N*/ Sequence<OUString> ScAppCfg::GetRevisionPropertyNames()
/*N*/ {
/*N*/ 	static const char* aPropNames[] =
/*N*/ 	{
/*N*/ 		"Change",					// SCREVISOPT_CHANGE
/*N*/ 		"Insertion",				// SCREVISOPT_INSERTION
/*N*/ 		"Deletion",					// SCREVISOPT_DELETION
/*N*/ 		"MovedEntry"				// SCREVISOPT_MOVEDENTRY
/*N*/ 	};
/*N*/ 	Sequence<OUString> aNames(SCREVISOPT_COUNT);
/*N*/ 	OUString* pNames = aNames.getArray();
/*N*/ 	for(int i = 0; i < SCREVISOPT_COUNT; i++)
/*N*/ 		pNames[i] = OUString::createFromAscii(aPropNames[i]);
/*N*/ 
/*N*/ 	return aNames;
/*N*/ }

/*N*/ Sequence<OUString> ScAppCfg::GetContentPropertyNames()
/*N*/ {
/*N*/ 	static const char* aPropNames[] =
/*N*/ 	{
/*N*/ 		"Link"						// SCCONTENTOPT_LINK
/*N*/ 	};
/*N*/ 	Sequence<OUString> aNames(SCCONTENTOPT_COUNT);
/*N*/ 	OUString* pNames = aNames.getArray();
/*N*/ 	for(int i = 0; i < SCCONTENTOPT_COUNT; i++)
/*N*/ 		pNames[i] = OUString::createFromAscii(aPropNames[i]);
/*N*/ 
/*N*/ 	return aNames;
/*N*/ }

/*N*/ Sequence<OUString> ScAppCfg::GetSortListPropertyNames()
/*N*/ {
/*N*/ 	static const char* aPropNames[] =
/*N*/ 	{
/*N*/ 		"List"						// SCSORTLISTOPT_LIST
/*N*/ 	};
/*N*/ 	Sequence<OUString> aNames(SCSORTLISTOPT_COUNT);
/*N*/ 	OUString* pNames = aNames.getArray();
/*N*/ 	for(int i = 0; i < SCSORTLISTOPT_COUNT; i++)
/*N*/ 		pNames[i] = OUString::createFromAscii(aPropNames[i]);
/*N*/ 
/*N*/ 	return aNames;
/*N*/ }

/*N*/ Sequence<OUString> ScAppCfg::GetMiscPropertyNames()
/*N*/ {
/*N*/ 	static const char* aPropNames[] =
/*N*/ 	{
/*N*/ 		"DefaultObjectSize/Width",	// SCMISCOPT_DEFOBJWIDTH
/*N*/ 		"DefaultObjectSize/Height"	// SCMISCOPT_DEFOBJHEIGHT
/*N*/ 	};
/*N*/ 	Sequence<OUString> aNames(SCMISCOPT_COUNT);
/*N*/ 	OUString* pNames = aNames.getArray();
/*N*/ 	for(int i = 0; i < SCMISCOPT_COUNT; i++)
/*N*/ 		pNames[i] = OUString::createFromAscii(aPropNames[i]);
/*N*/ 
/*N*/ 	return aNames;
/*N*/ }


/*N*/ ScAppCfg::ScAppCfg() :
/*N*/ 	aLayoutItem( OUString::createFromAscii( CFGPATH_LAYOUT ) ),
/*N*/ 	aInputItem( OUString::createFromAscii( CFGPATH_INPUT ) ),
/*N*/ 	aRevisionItem( OUString::createFromAscii( CFGPATH_REVISION ) ),
/*N*/ 	aContentItem( OUString::createFromAscii( CFGPATH_CONTENT ) ),
/*N*/ 	aSortListItem( OUString::createFromAscii( CFGPATH_SORTLIST ) ),
/*N*/ 	aMiscItem( OUString::createFromAscii( CFGPATH_MISC ) )
/*N*/ {
/*N*/ 	sal_Int32 nIntVal;
/*N*/ 
/*N*/ 	Sequence<OUString> aNames;
/*N*/ 	Sequence<Any> aValues;
/*N*/ 	const Any* pValues = NULL;
/*N*/ 
/*N*/ 	aNames = GetLayoutPropertyNames();
/*N*/ 	aValues = aLayoutItem.GetProperties(aNames);
/*N*/ 	aLayoutItem.EnableNotification(aNames);
/*N*/ 	pValues = aValues.getConstArray();
/*N*/ 	DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
/*N*/ 	if(aValues.getLength() == aNames.getLength())
/*N*/ 	{
/*N*/ 		for(int nProp = 0; nProp < aNames.getLength(); nProp++)
/*N*/ 		{
/*N*/ 			DBG_ASSERT(pValues[nProp].hasValue(), "property value missing");
/*N*/ 			if(pValues[nProp].hasValue())
/*N*/ 			{
/*N*/ 				switch(nProp)
/*N*/ 				{
/*N*/ 					case SCLAYOUTOPT_MEASURE:
/*N*/ 						if (pValues[nProp] >>= nIntVal) SetAppMetric( (FieldUnit) nIntVal );
/*N*/ 						break;
/*N*/ 					case SCLAYOUTOPT_STATUSBAR:
/*N*/ 						if (pValues[nProp] >>= nIntVal) SetStatusFunc( (USHORT) nIntVal );
/*N*/ 						break;
/*N*/ 					case SCLAYOUTOPT_ZOOMVAL:
/*N*/ 						if (pValues[nProp] >>= nIntVal) SetZoom( (USHORT) nIntVal );
/*N*/ 						break;
/*N*/ 					case SCLAYOUTOPT_ZOOMTYPE:
/*N*/ 						if (pValues[nProp] >>= nIntVal) SetZoomType( (SvxZoomType) nIntVal );
/*N*/ 						break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	aLayoutItem.SetCommitLink( LINK( this, ScAppCfg, LayoutCommitHdl ) );
/*N*/ 
/*N*/ 	aNames = GetInputPropertyNames();
/*N*/ 	aValues = aInputItem.GetProperties(aNames);
/*N*/ 	aInputItem.EnableNotification(aNames);
/*N*/ 	pValues = aValues.getConstArray();
/*N*/ 	DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
/*N*/ 	if(aValues.getLength() == aNames.getLength())
/*N*/ 	{
/*N*/ 		for(int nProp = 0; nProp < aNames.getLength(); nProp++)
/*N*/ 		{
/*N*/ 			DBG_ASSERT(pValues[nProp].hasValue(), "property value missing");
/*N*/ 			if(pValues[nProp].hasValue())
/*N*/ 			{
/*N*/ 				switch(nProp)
/*N*/ 				{
/*N*/ 					case SCINPUTOPT_LASTFUNCS:
/*N*/ 						lcl_SetLastFunctions( *this, pValues[nProp] );
/*N*/ 						break;
/*N*/ 					case SCINPUTOPT_AUTOINPUT:
/*N*/ 						SetAutoComplete( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCINPUTOPT_DET_AUTO:
/*N*/ 						SetDetectiveAuto( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	aInputItem.SetCommitLink( LINK( this, ScAppCfg, InputCommitHdl ) );
/*N*/ 
/*N*/ 	aNames = GetRevisionPropertyNames();
/*N*/ 	aValues = aRevisionItem.GetProperties(aNames);
/*N*/ 	aRevisionItem.EnableNotification(aNames);
/*N*/ 	pValues = aValues.getConstArray();
/*N*/ 	DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
/*N*/ 	if(aValues.getLength() == aNames.getLength())
/*N*/ 	{
/*N*/ 		for(int nProp = 0; nProp < aNames.getLength(); nProp++)
/*N*/ 		{
/*N*/ 			DBG_ASSERT(pValues[nProp].hasValue(), "property value missing");
/*N*/ 			if(pValues[nProp].hasValue())
/*N*/ 			{
/*N*/ 				switch(nProp)
/*N*/ 				{
/*N*/ 					case SCREVISOPT_CHANGE:
/*N*/ 						if (pValues[nProp] >>= nIntVal) SetTrackContentColor( (ULONG) nIntVal );
/*N*/ 						break;
/*N*/ 					case SCREVISOPT_INSERTION:
/*N*/ 						if (pValues[nProp] >>= nIntVal) SetTrackInsertColor( (ULONG) nIntVal );
/*N*/ 						break;
/*N*/ 					case SCREVISOPT_DELETION:
/*N*/ 						if (pValues[nProp] >>= nIntVal) SetTrackDeleteColor( (ULONG) nIntVal );
/*N*/ 						break;
/*N*/ 					case SCREVISOPT_MOVEDENTRY:
/*N*/ 						if (pValues[nProp] >>= nIntVal) SetTrackMoveColor( (ULONG) nIntVal );
/*N*/ 						break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	aRevisionItem.SetCommitLink( LINK( this, ScAppCfg, RevisionCommitHdl ) );
/*N*/ 
/*N*/ 	aNames = GetContentPropertyNames();
/*N*/ 	aValues = aContentItem.GetProperties(aNames);
/*N*/ 	aContentItem.EnableNotification(aNames);
/*N*/ 	pValues = aValues.getConstArray();
/*N*/ 	DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
/*N*/ 	if(aValues.getLength() == aNames.getLength())
/*N*/ 	{
/*N*/ 		for(int nProp = 0; nProp < aNames.getLength(); nProp++)
/*N*/ 		{
/*N*/ 			DBG_ASSERT(pValues[nProp].hasValue(), "property value missing");
/*N*/ 			if(pValues[nProp].hasValue())
/*N*/ 			{
/*N*/ 				switch(nProp)
/*N*/ 				{
/*N*/ 					case SCCONTENTOPT_LINK:
/*N*/ 						if (pValues[nProp] >>= nIntVal) SetLinkMode( (ScLkUpdMode) nIntVal );
/*N*/ 						break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	aContentItem.SetCommitLink( LINK( this, ScAppCfg, ContentCommitHdl ) );
/*N*/ 
/*N*/ 	aNames = GetSortListPropertyNames();
/*N*/ 	aValues = aSortListItem.GetProperties(aNames);
/*N*/ 	aSortListItem.EnableNotification(aNames);
/*N*/ 	pValues = aValues.getConstArray();
/*N*/ 	DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
/*N*/ 	if(aValues.getLength() == aNames.getLength())
/*N*/ 	{
/*N*/ 		for(int nProp = 0; nProp < aNames.getLength(); nProp++)
/*N*/ 		{
/*N*/ 			DBG_ASSERT(pValues[nProp].hasValue(), "property value missing");
/*N*/ 			if(pValues[nProp].hasValue())
/*N*/ 			{
/*N*/ 				switch(nProp)
/*N*/ 				{
/*N*/ 					case SCSORTLISTOPT_LIST:
/*N*/ 						lcl_SetSortList( pValues[nProp] );
/*N*/ 						break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	aSortListItem.SetCommitLink( LINK( this, ScAppCfg, SortListCommitHdl ) );
/*N*/ 
/*N*/ 	aNames = GetMiscPropertyNames();
/*N*/ 	aValues = aMiscItem.GetProperties(aNames);
/*N*/ 	aMiscItem.EnableNotification(aNames);
/*N*/ 	pValues = aValues.getConstArray();
/*N*/ 	DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
/*N*/ 	if(aValues.getLength() == aNames.getLength())
/*N*/ 	{
/*N*/ 		for(int nProp = 0; nProp < aNames.getLength(); nProp++)
/*N*/ 		{
/*N*/ 			DBG_ASSERT(pValues[nProp].hasValue(), "property value missing");
/*N*/ 			if(pValues[nProp].hasValue())
/*N*/ 			{
/*N*/ 				switch(nProp)
/*N*/ 				{
/*N*/ 					case SCMISCOPT_DEFOBJWIDTH:
/*N*/ 						if (pValues[nProp] >>= nIntVal) SetDefaultObjectSizeWidth( nIntVal );
/*N*/ 						break;
/*N*/ 					case SCMISCOPT_DEFOBJHEIGHT:
/*N*/ 						if (pValues[nProp] >>= nIntVal) SetDefaultObjectSizeHeight( nIntVal );
/*N*/ 						break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	aMiscItem.SetCommitLink( LINK( this, ScAppCfg, MiscCommitHdl ) );
/*N*/ }

/*N*/ IMPL_LINK( ScAppCfg, LayoutCommitHdl, void *, EMPTYARG )
/*N*/ {
            DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	Sequence<OUString> aNames = GetLayoutPropertyNames();
/*N*/ 	return 0;
/*N*/ }

/*N*/ IMPL_LINK( ScAppCfg, InputCommitHdl, void *, EMPTYARG )
/*N*/ {
         DBG_BF_ASSERT(0, "STRIP"); //STRIP001 Sequence<OUString> aNames = GetInputPropertyNames();
/*N*/ 	return 0;
/*N*/ }

/*N*/ IMPL_LINK( ScAppCfg, RevisionCommitHdl, void *, EMPTYARG )
/*N*/ {
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 Sequence<OUString> aNames = GetRevisionPropertyNames();
/*N*/ 	return 0;
/*N*/ }

/*N*/ IMPL_LINK( ScAppCfg, ContentCommitHdl, void *, EMPTYARG )
/*N*/ {
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 Sequence<OUString> aNames = GetContentPropertyNames();
/*N*/ 	return 0;
/*N*/ }

/*N*/ IMPL_LINK( ScAppCfg, SortListCommitHdl, void *, EMPTYARG )
/*N*/ {
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 Sequence<OUString> aNames = GetSortListPropertyNames();
/*N*/ 	return 0;
/*N*/ }

/*N*/ IMPL_LINK( ScAppCfg, MiscCommitHdl, void *, EMPTYARG )
/*N*/ {
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 Sequence<OUString> aNames = GetMiscPropertyNames();
/*N*/ 	return 0;
/*N*/ }




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
