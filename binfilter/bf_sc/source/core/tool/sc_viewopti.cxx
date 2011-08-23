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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif



#include "globstr.hrc"
#include "viewopti.hxx"
#include "rechead.hxx"
#include "bf_sc.hrc"
#include "miscuno.hxx"
namespace binfilter {

using namespace utl;
using namespace rtl;
using namespace ::com::sun::star::uno;

//------------------------------------------------------------------


#define SC_VERSION ((USHORT)302)


//========================================================================
// class ScGridOptions
//========================================================================


/*N*/ void ScGridOptions::SetDefaults()
/*N*/ {
/*N*/ 	*this = ScGridOptions();
/*N*/ 
/*N*/ 	//	Raster-Defaults sind jetzt zwischen den Apps unterschiedlich
/*N*/ 	//	darum hier selber eintragen (alles in 1/100mm)
/*N*/ 
/*N*/ 	if ( ScOptionsUtil::IsMetricSystem() )
/*N*/ 	{
/*N*/ 		nFldDrawX = 1000;	// 1cm
/*N*/ 		nFldDrawY = 1000;
/*N*/ 		nFldSnapX = 1000;
/*N*/ 		nFldSnapY = 1000;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		nFldDrawX = 1270;	// 0,5"
/*N*/ 		nFldDrawY = 1270;
/*N*/ 		nFldSnapX = 1270;
/*N*/ 		nFldSnapY = 1270;
/*N*/ 	}
/*N*/ 	nFldDivisionX = 1;
/*N*/ 	nFldDivisionY = 1;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ const ScGridOptions& ScGridOptions::operator=( const ScGridOptions& rCpy )
/*N*/ {
/*N*/ 	nFldDrawX		= rCpy.nFldDrawX;		// UINT32
/*N*/ 	nFldDrawX		= rCpy.nFldDrawX;
/*N*/ 	nFldDivisionX	= rCpy.nFldDivisionX;
/*N*/ 	nFldDrawY		= rCpy.nFldDrawY;
/*N*/ 	nFldDivisionY	= rCpy.nFldDivisionY;
/*N*/ 	nFldSnapX		= rCpy.nFldSnapX;
/*N*/ 	nFldSnapY		= rCpy.nFldSnapY;
/*N*/ 	bUseGridsnap	= rCpy.bUseGridsnap;	// BitBool
/*N*/ 	bSynchronize	= rCpy.bSynchronize;
/*N*/ 	bGridVisible	= rCpy.bGridVisible;
/*N*/ 	bEqualGrid		= rCpy.bEqualGrid;
/*N*/ 
/*N*/ 	return *this;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ int ScGridOptions::operator==( const ScGridOptions& rCpy ) const
/*N*/ {
/*N*/ 	return (   nFldDrawX		== rCpy.nFldDrawX
/*N*/ 			&& nFldDrawX		== rCpy.nFldDrawX
/*N*/ 			&& nFldDivisionX	== rCpy.nFldDivisionX
/*N*/ 			&& nFldDrawY		== rCpy.nFldDrawY
/*N*/ 			&& nFldDivisionY	== rCpy.nFldDivisionY
/*N*/ 			&& nFldSnapX		== rCpy.nFldSnapX
/*N*/ 			&& nFldSnapY		== rCpy.nFldSnapY
/*N*/ 			&& bUseGridsnap		== rCpy.bUseGridsnap
/*N*/ 			&& bSynchronize		== rCpy.bSynchronize
/*N*/ 			&& bGridVisible		== rCpy.bGridVisible
/*N*/ 			&& bEqualGrid		== rCpy.bEqualGrid );
/*N*/ }


//------------------------------------------------------------------------

/*N*/ SvStream& operator>>( SvStream& rStream, ScGridOptions& rOpt )
/*N*/ {
/*N*/ 	BYTE nDummy;
/*N*/ 	rStream >> rOpt.nFldDrawX;
/*N*/ 	rStream >> rOpt.nFldDrawY;
/*N*/ 	rStream >> rOpt.nFldDivisionX;
/*N*/ 	rStream >> rOpt.nFldDivisionY;
/*N*/ 	rStream >> rOpt.nFldSnapX;
/*N*/ 	rStream >> rOpt.nFldSnapY;
/*N*/ 	rStream >> nDummy; rOpt.bUseGridsnap = (BOOL)nDummy;
/*N*/ 	rStream >> nDummy; rOpt.bSynchronize = (BOOL)nDummy;
/*N*/ 	rStream >> nDummy; rOpt.bGridVisible = (BOOL)nDummy;
/*N*/ 	rStream >> nDummy; rOpt.bEqualGrid	 = (BOOL)nDummy;
/*N*/ 
/*N*/ 	return rStream;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ SvStream& operator<<( SvStream& rStream, const ScGridOptions& rOpt )
/*N*/ {
/*N*/ 	rStream << rOpt.nFldDrawX;
/*N*/ 	rStream << rOpt.nFldDrawY;
/*N*/ 	rStream << rOpt.nFldDivisionX;
/*N*/ 	rStream << rOpt.nFldDivisionY;
/*N*/ 	rStream << rOpt.nFldSnapX;
/*N*/ 	rStream << rOpt.nFldSnapY;
/*N*/ 	rStream << (BOOL)rOpt.bUseGridsnap;
/*N*/ 	rStream << (BOOL)rOpt.bSynchronize;
/*N*/ 	rStream << (BOOL)rOpt.bGridVisible;
/*N*/ 	rStream << (BOOL)rOpt.bEqualGrid;
/*N*/ 
/*N*/ 	return rStream;
/*N*/ }

//========================================================================
// class ScViewOptions
//========================================================================

/*N*/ ScViewOptions::ScViewOptions()
/*N*/ {
/*N*/ 	SetDefaults();
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScViewOptions::ScViewOptions( const ScViewOptions& rCpy )
/*N*/ {
/*N*/ 	*this = rCpy;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ __EXPORT ScViewOptions::~ScViewOptions()
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void ScViewOptions::SetDefaults()
/*N*/ {
/*N*/ 	aOptArr[ VOPT_FORMULAS	  ] =
/*N*/ 	aOptArr[ VOPT_SYNTAX	  ] =
/*N*/ 	aOptArr[ VOPT_HELPLINES   ] =
/*N*/ 	aOptArr[ VOPT_BIGHANDLES  ] = FALSE;
/*N*/ 	aOptArr[ VOPT_NOTES		  ] =
/*N*/ 	aOptArr[ VOPT_NULLVALS	  ] =
/*N*/ 	aOptArr[ VOPT_VSCROLL	  ] =
/*N*/ 	aOptArr[ VOPT_HSCROLL	  ] =
/*N*/ 	aOptArr[ VOPT_TABCONTROLS ] =
/*N*/ 	aOptArr[ VOPT_OUTLINER	  ] =
/*N*/ 	aOptArr[ VOPT_HEADER	  ] =
/*N*/ 	aOptArr[ VOPT_GRID		  ] =
/*N*/ 	aOptArr[ VOPT_ANCHOR	  ] =
/*N*/ 	aOptArr[ VOPT_PAGEBREAKS  ] =
/*N*/ 	aOptArr[ VOPT_SOLIDHANDLES] =
/*N*/ 	aOptArr[ VOPT_CLIPMARKS	  ] = TRUE;
/*N*/ 
/*N*/ 	aModeArr[VOBJ_TYPE_OLE ]  =
/*N*/ 	aModeArr[VOBJ_TYPE_CHART] =
/*N*/ 	aModeArr[VOBJ_TYPE_DRAW ] = VOBJ_MODE_SHOW;
/*N*/ 
/*N*/ 	aGridCol     = Color( SC_STD_GRIDCOLOR );
/*N*/ 	aGridColName = ScGlobal::GetRscString( STR_GRIDCOLOR );
/*N*/ 
/*N*/ 	aGridOpt.SetDefaults();
/*N*/ 
/*N*/ 	bHideAutoSpell = FALSE;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ Color ScViewOptions::GetGridColor( String* pStrName ) const
/*N*/ {
/*N*/ 	if ( pStrName )
/*N*/ 		*pStrName = aGridColName;
/*N*/ 
/*N*/ 	return aGridCol;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ const ScViewOptions& ScViewOptions::operator=( const ScViewOptions& rCpy )
/*N*/ {
/*N*/ 	USHORT i;
/*N*/ 
/*N*/ 	for ( i=0; i<MAX_OPT; i++ )	 aOptArr [i] = rCpy.aOptArr[i];
/*N*/ 	for ( i=0; i<MAX_TYPE; i++ ) aModeArr[i] = rCpy.aModeArr[i];
/*N*/ 
/*N*/ 	aGridCol     	= rCpy.aGridCol;
/*N*/ 	aGridColName 	= rCpy.aGridColName;
/*N*/ 	aGridOpt		= rCpy.aGridOpt;
/*N*/ 	bHideAutoSpell	= rCpy.bHideAutoSpell;
/*N*/ 
/*N*/ 	return *this;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ int ScViewOptions::operator==( const ScViewOptions& rOpt ) const
/*N*/ {
/*N*/ 	BOOL	bEqual = TRUE;
/*N*/ 	USHORT	i;
/*N*/ 
/*N*/ 	for ( i=0; i<MAX_OPT && bEqual; i++ )  bEqual = (aOptArr [i] == rOpt.aOptArr[i]);
/*N*/ 	for ( i=0; i<MAX_TYPE && bEqual; i++ ) bEqual = (aModeArr[i] == rOpt.aModeArr[i]);
/*N*/ 
/*N*/ 	bEqual = bEqual && (aGridCol       == rOpt.aGridCol);
/*N*/ 	bEqual = bEqual && (aGridColName   == rOpt.aGridColName);
/*N*/ 	bEqual = bEqual && (aGridOpt 	   == rOpt.aGridOpt);
/*N*/ 	bEqual = bEqual && (bHideAutoSpell == rOpt.bHideAutoSpell);
/*N*/ 
/*N*/ 	return bEqual;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ SvStream& operator>>( SvStream& rStream, ScViewOptions& rOpt )
/*N*/ {
/*N*/ 	USHORT	i;
/*N*/ 	BYTE	n;
/*N*/ 
/*N*/ 	ScReadHeader aHdr( rStream );
/*N*/ 
/*N*/ 	for ( i=0; i<=VOPT_GRID; i++ )			// kompatibel bleiben -> nur bis VOPT_GRID
/*N*/ 		rStream >> rOpt.aOptArr[i];
/*N*/ 
/*N*/ 	for ( i=0; i<MAX_TYPE; i++ )
/*N*/ 		rStream >> n, rOpt.aModeArr[i] = (ScVObjMode)n;
/*N*/ 
/*N*/ 	rStream >> rOpt.aGridCol;
/*N*/ 	rStream.ReadByteString( rOpt.aGridColName, rStream.GetStreamCharSet() );
/*N*/ 
/*N*/ 	if( aHdr.BytesLeft() )
/*N*/ 		rStream >> rOpt.aOptArr[VOPT_HELPLINES];
/*N*/ 
/*N*/ 	if( aHdr.BytesLeft() )
/*N*/ 		rStream >> rOpt.aGridOpt;
/*N*/ 
/*N*/ 	if( aHdr.BytesLeft() )
/*N*/ 		rStream >> rOpt.bHideAutoSpell;
/*N*/ 
/*N*/ 	if( aHdr.BytesLeft() )
/*N*/ 		rStream >> rOpt.aOptArr[VOPT_ANCHOR];
/*N*/ 
/*N*/ 	if( aHdr.BytesLeft() )
/*N*/ 		rStream >> rOpt.aOptArr[VOPT_PAGEBREAKS];
/*N*/ 
/*N*/ 	if( aHdr.BytesLeft() )
/*N*/ 		rStream >> rOpt.aOptArr[VOPT_SOLIDHANDLES];
/*N*/ 
/*N*/ 	if( aHdr.BytesLeft() )
/*N*/ 		rStream >> rOpt.aOptArr[VOPT_CLIPMARKS];
/*N*/ 
/*N*/ 	if( aHdr.BytesLeft() )
/*N*/ 		rStream >> rOpt.aOptArr[VOPT_BIGHANDLES];
/*N*/ 
/*N*/ 	return rStream;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void ScViewOptions::Save(SvStream& rStream, BOOL bConfig) const
/*N*/ {
/*N*/ 	USHORT i;
/*N*/ 
/*N*/ 	ScWriteHeader aHdr( rStream, 68 );
/*N*/ 
/*N*/ 	for ( i=0; i<=VOPT_GRID; i++ )			// kompatibel bleiben -> nur bis VOPT_GRID
/*N*/ 		rStream << aOptArr[i];
/*N*/ 
/*N*/ 	for ( i=0; i<MAX_TYPE; i++ )
/*N*/ 		rStream << (BYTE)aModeArr[i];
/*N*/ 
/*N*/ 	rStream << aGridCol;
/*N*/ 	rStream.WriteByteString( aGridColName, rStream.GetStreamCharSet() );
/*N*/ 	rStream << aOptArr[VOPT_HELPLINES];
/*N*/ 	rStream << aGridOpt;
/*N*/ 	rStream << bHideAutoSpell;
/*N*/ 	rStream << aOptArr[VOPT_ANCHOR];
/*N*/ 	rStream << aOptArr[VOPT_PAGEBREAKS];
/*N*/ 	rStream << aOptArr[VOPT_SOLIDHANDLES];
/*N*/ 
/*N*/ 	if ( bConfig || rStream.GetVersion() > SOFFICE_FILEFORMAT_40 )		// nicht bei 4.0 Export
/*N*/ 	{
/*N*/ 		rStream << aOptArr[VOPT_CLIPMARKS];
/*N*/ 
/*N*/ 		//	big handles are not saved in 5.0-documents to avoid warning messages
/*N*/ 		//!	save to files after 5.0 !!!
/*N*/ 
/*N*/ 		if ( bConfig )
/*N*/ 			rStream << aOptArr[VOPT_BIGHANDLES];
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------


//========================================================================
//      ScTpViewItem - Daten fuer die ViewOptions-TabPage
//========================================================================


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//==================================================================
//	Config Item containing view options
//==================================================================

#define CFGPATH_LAYOUT		"Office.Calc/Layout"

#define SCLAYOUTOPT_GRIDLINES		0
#define SCLAYOUTOPT_GRIDCOLOR		1
#define SCLAYOUTOPT_PAGEBREAK		2
#define SCLAYOUTOPT_GUIDE			3
#define SCLAYOUTOPT_SIMPLECONT		4
#define SCLAYOUTOPT_LARGECONT		5
#define SCLAYOUTOPT_COLROWHDR		6
#define SCLAYOUTOPT_HORISCROLL		7
#define SCLAYOUTOPT_VERTSCROLL		8
#define SCLAYOUTOPT_SHEETTAB		9
#define SCLAYOUTOPT_OUTLINE			10
#define SCLAYOUTOPT_COUNT			11

#define CFGPATH_DISPLAY		"Office.Calc/Content/Display"

#define SCDISPLAYOPT_FORMULA		0
#define SCDISPLAYOPT_ZEROVALUE		1
#define SCDISPLAYOPT_NOTETAG		2
#define SCDISPLAYOPT_VALUEHI		3
#define SCDISPLAYOPT_ANCHOR			4
#define SCDISPLAYOPT_TEXTOVER		5
#define SCDISPLAYOPT_OBJECTGRA		6
#define SCDISPLAYOPT_CHART			7
#define SCDISPLAYOPT_DRAWING		8
#define SCDISPLAYOPT_COUNT			9

#define CFGPATH_GRID		"Office.Calc/Grid"

#define SCGRIDOPT_RESOLU_X			0
#define SCGRIDOPT_RESOLU_Y			1
#define SCGRIDOPT_SUBDIV_X			2
#define SCGRIDOPT_SUBDIV_Y			3
#define SCGRIDOPT_OPTION_X			4
#define SCGRIDOPT_OPTION_Y			5
#define SCGRIDOPT_SNAPTOGRID		6
#define SCGRIDOPT_SYNCHRON			7
#define SCGRIDOPT_VISIBLE			8
#define SCGRIDOPT_SIZETOGRID		9
#define SCGRIDOPT_COUNT				10


/*N*/ Sequence<OUString> ScViewCfg::GetLayoutPropertyNames()
/*N*/ {
/*N*/ 	static const char* aPropNames[] =
/*N*/ 	{
/*N*/ 		"Line/GridLine",			// SCLAYOUTOPT_GRIDLINES
/*N*/ 		"Line/GridLineColor",		// SCLAYOUTOPT_GRIDCOLOR
/*N*/ 		"Line/PageBreak",			// SCLAYOUTOPT_PAGEBREAK
/*N*/ 		"Line/Guide",				// SCLAYOUTOPT_GUIDE
/*N*/ 		"Line/SimpleControlPoint",	// SCLAYOUTOPT_SIMPLECONT
/*N*/ 		"Line/LargeControlPoint",	// SCLAYOUTOPT_LARGECONT
/*N*/ 		"Window/ColumnRowHeader",	// SCLAYOUTOPT_COLROWHDR
/*N*/ 		"Window/HorizontalScroll",	// SCLAYOUTOPT_HORISCROLL
/*N*/ 		"Window/VerticalScroll",	// SCLAYOUTOPT_VERTSCROLL
/*N*/ 		"Window/SheetTab",			// SCLAYOUTOPT_SHEETTAB
/*N*/ 		"Window/OutlineSymbol"		// SCLAYOUTOPT_OUTLINE
/*N*/ 	};
/*N*/ 	Sequence<OUString> aNames(SCLAYOUTOPT_COUNT);
/*N*/ 	OUString* pNames = aNames.getArray();
/*N*/ 	for(int i = 0; i < SCLAYOUTOPT_COUNT; i++)
/*N*/ 		pNames[i] = OUString::createFromAscii(aPropNames[i]);
/*N*/ 
/*N*/ 	return aNames;
/*N*/ }

/*N*/ Sequence<OUString> ScViewCfg::GetDisplayPropertyNames()
/*N*/ {
/*N*/ 	static const char* aPropNames[] =
/*N*/ 	{
/*N*/ 		"Formula",					// SCDISPLAYOPT_FORMULA
/*N*/ 		"ZeroValue",				// SCDISPLAYOPT_ZEROVALUE
/*N*/ 		"NoteTag",					// SCDISPLAYOPT_NOTETAG
/*N*/ 		"ValueHighlighting",		// SCDISPLAYOPT_VALUEHI
/*N*/ 		"Anchor",					// SCDISPLAYOPT_ANCHOR
/*N*/ 		"TextOverflow",				// SCDISPLAYOPT_TEXTOVER
/*N*/ 		"ObjectGraphic",			// SCDISPLAYOPT_OBJECTGRA
/*N*/ 		"Chart",					// SCDISPLAYOPT_CHART
/*N*/ 		"DrawingObject"				// SCDISPLAYOPT_DRAWING
/*N*/ 	};
/*N*/ 	Sequence<OUString> aNames(SCDISPLAYOPT_COUNT);
/*N*/ 	OUString* pNames = aNames.getArray();
/*N*/ 	for(int i = 0; i < SCDISPLAYOPT_COUNT; i++)
/*N*/ 		pNames[i] = OUString::createFromAscii(aPropNames[i]);
/*N*/ 
/*N*/ 	return aNames;
/*N*/ }

/*N*/ Sequence<OUString> ScViewCfg::GetGridPropertyNames()
/*N*/ {
/*N*/ 	static const char* aPropNames[] =
/*N*/ 	{
/*N*/ 		"Resolution/XAxis/NonMetric",	// SCGRIDOPT_RESOLU_X
/*N*/ 		"Resolution/YAxis/NonMetric",	// SCGRIDOPT_RESOLU_Y
/*N*/ 		"Subdivision/XAxis",			// SCGRIDOPT_SUBDIV_X
/*N*/ 		"Subdivision/YAxis",			// SCGRIDOPT_SUBDIV_Y
/*N*/ 		"Option/XAxis/NonMetric",		// SCGRIDOPT_OPTION_X
/*N*/ 		"Option/YAxis/NonMetric",		// SCGRIDOPT_OPTION_Y
/*N*/ 		"Option/SnapToGrid",			// SCGRIDOPT_SNAPTOGRID
/*N*/ 		"Option/Synchronize",			// SCGRIDOPT_SYNCHRON
/*N*/ 		"Option/VisibleGrid",			// SCGRIDOPT_VISIBLE
/*N*/ 		"Option/SizeToGrid"				// SCGRIDOPT_SIZETOGRID
/*N*/ 	};
/*N*/ 	Sequence<OUString> aNames(SCGRIDOPT_COUNT);
/*N*/ 	OUString* pNames = aNames.getArray();
/*N*/ 	for(int i = 0; i < SCGRIDOPT_COUNT; i++)
/*N*/ 		pNames[i] = OUString::createFromAscii(aPropNames[i]);
/*N*/ 
/*N*/ 	//	adjust for metric system
/*N*/ 	if (ScOptionsUtil::IsMetricSystem())
/*N*/ 	{
/*N*/ 		pNames[SCGRIDOPT_RESOLU_X] = OUString::createFromAscii( "Resolution/XAxis/Metric" );
/*N*/ 		pNames[SCGRIDOPT_RESOLU_Y] = OUString::createFromAscii( "Resolution/YAxis/Metric" );
/*N*/ 		pNames[SCGRIDOPT_OPTION_X] = OUString::createFromAscii( "Option/XAxis/Metric" );
/*N*/ 		pNames[SCGRIDOPT_OPTION_Y] = OUString::createFromAscii( "Option/YAxis/Metric" );
/*N*/ 	}
/*N*/ 
/*N*/ 	return aNames;
/*N*/ }


/*N*/ ScViewCfg::ScViewCfg() :
/*N*/ 	aLayoutItem( OUString::createFromAscii( CFGPATH_LAYOUT ) ),
/*N*/ 	aDisplayItem( OUString::createFromAscii( CFGPATH_DISPLAY ) ),
/*N*/ 	aGridItem( OUString::createFromAscii( CFGPATH_GRID ) )
/*N*/ {
/*N*/ 	sal_Int32 nIntVal;
/*N*/ 
/*N*/ 	Sequence<OUString> aNames = GetLayoutPropertyNames();
/*N*/ 	Sequence<Any> aValues = aLayoutItem.GetProperties(aNames);
/*N*/ 	aLayoutItem.EnableNotification(aNames);
/*N*/ 	const Any* pValues = aValues.getConstArray();
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
/*N*/ 					case SCLAYOUTOPT_GRIDCOLOR:
/*N*/ 						if ( pValues[nProp] >>= nIntVal )
/*N*/ 							SetGridColor( Color(nIntVal), EMPTY_STRING );
/*N*/ 						break;
/*N*/ 					case SCLAYOUTOPT_GRIDLINES:
/*N*/ 						SetOption( VOPT_GRID, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCLAYOUTOPT_PAGEBREAK:
/*N*/ 						SetOption( VOPT_PAGEBREAKS, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCLAYOUTOPT_GUIDE:
/*N*/ 						SetOption( VOPT_HELPLINES, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCLAYOUTOPT_SIMPLECONT:
/*N*/ 						// content is reversed
/*N*/ 						SetOption( VOPT_SOLIDHANDLES, !ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCLAYOUTOPT_LARGECONT:
/*N*/ 						SetOption( VOPT_BIGHANDLES, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCLAYOUTOPT_COLROWHDR:
/*N*/ 						SetOption( VOPT_HEADER, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCLAYOUTOPT_HORISCROLL:
/*N*/ 						SetOption( VOPT_HSCROLL, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCLAYOUTOPT_VERTSCROLL:
/*N*/ 						SetOption( VOPT_VSCROLL, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCLAYOUTOPT_SHEETTAB:
/*N*/ 						SetOption( VOPT_TABCONTROLS, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCLAYOUTOPT_OUTLINE:
/*N*/ 						SetOption( VOPT_OUTLINER, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	aLayoutItem.SetCommitLink( LINK( this, ScViewCfg, LayoutCommitHdl ) );
/*N*/ 
/*N*/ 	aNames = GetDisplayPropertyNames();
/*N*/ 	aValues = aDisplayItem.GetProperties(aNames);
/*N*/ 	aDisplayItem.EnableNotification(aNames);
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
/*N*/ 					case SCDISPLAYOPT_FORMULA:
/*N*/ 						SetOption( VOPT_FORMULAS, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCDISPLAYOPT_ZEROVALUE:
/*N*/ 						SetOption( VOPT_NULLVALS, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCDISPLAYOPT_NOTETAG:
/*N*/ 						SetOption( VOPT_NOTES, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCDISPLAYOPT_VALUEHI:
/*N*/ 						SetOption( VOPT_SYNTAX, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCDISPLAYOPT_ANCHOR:
/*N*/ 						SetOption( VOPT_ANCHOR, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCDISPLAYOPT_TEXTOVER:
/*N*/ 						SetOption( VOPT_CLIPMARKS, ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCDISPLAYOPT_OBJECTGRA:
/*N*/ 						if ( pValues[nProp] >>= nIntVal )
/*N*/ 							SetObjMode( VOBJ_TYPE_OLE, (ScVObjMode) nIntVal );
/*N*/ 						break;
/*N*/ 					case SCDISPLAYOPT_CHART:
/*N*/ 						if ( pValues[nProp] >>= nIntVal )
/*N*/ 							SetObjMode( VOBJ_TYPE_CHART, (ScVObjMode) nIntVal );
/*N*/ 						break;
/*N*/ 					case SCDISPLAYOPT_DRAWING:
/*N*/ 						if ( pValues[nProp] >>= nIntVal )
/*N*/ 							SetObjMode( VOBJ_TYPE_DRAW, (ScVObjMode) nIntVal );
/*N*/ 						break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	aDisplayItem.SetCommitLink( LINK( this, ScViewCfg, DisplayCommitHdl ) );
/*N*/ 
/*N*/ 	ScGridOptions aGrid = GetGridOptions();		//! initialization necessary?
/*N*/ 	aNames = GetGridPropertyNames();
/*N*/ 	aValues = aGridItem.GetProperties(aNames);
/*N*/ 	aGridItem.EnableNotification(aNames);
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
/*N*/ 					case SCGRIDOPT_RESOLU_X:
/*N*/ 						if (pValues[nProp] >>= nIntVal) aGrid.SetFldDrawX( nIntVal );
/*N*/ 						break;
/*N*/ 					case SCGRIDOPT_RESOLU_Y:
/*N*/ 						if (pValues[nProp] >>= nIntVal) aGrid.SetFldDrawY( nIntVal );
/*N*/ 						break;
/*N*/ 					case SCGRIDOPT_SUBDIV_X:
/*N*/ 						if (pValues[nProp] >>= nIntVal) aGrid.SetFldDivisionX( nIntVal );
/*N*/ 						break;
/*N*/ 					case SCGRIDOPT_SUBDIV_Y:
/*N*/ 						if (pValues[nProp] >>= nIntVal) aGrid.SetFldDivisionY( nIntVal );
/*N*/ 						break;
/*N*/ 					case SCGRIDOPT_OPTION_X:
/*N*/ 						if (pValues[nProp] >>= nIntVal) aGrid.SetFldSnapX( nIntVal );
/*N*/ 						break;
/*N*/ 					case SCGRIDOPT_OPTION_Y:
/*N*/ 						if (pValues[nProp] >>= nIntVal) aGrid.SetFldSnapY( nIntVal );
/*N*/ 						break;
/*N*/ 					case SCGRIDOPT_SNAPTOGRID:
/*N*/ 						aGrid.SetUseGridSnap( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCGRIDOPT_SYNCHRON:
/*N*/ 						aGrid.SetSynchronize( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCGRIDOPT_VISIBLE:
/*N*/ 						aGrid.SetGridVisible( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCGRIDOPT_SIZETOGRID:
/*N*/ 						aGrid.SetEqualGrid( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	SetGridOptions( aGrid );
/*N*/ 	aGridItem.SetCommitLink( LINK( this, ScViewCfg, GridCommitHdl ) );
/*N*/ }

/*N*/ IMPL_LINK( ScViewCfg, LayoutCommitHdl, void *, EMPTYARG )
/*N*/ {
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 Sequence<OUString> aNames = GetLayoutPropertyNames();
/*N*/ 	return 0;
/*N*/ }

/*N*/ IMPL_LINK( ScViewCfg, DisplayCommitHdl, void *, EMPTYARG )
/*N*/ {
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 Sequence<OUString> aNames = GetDisplayPropertyNames();
/*N*/ 	return 0;
/*N*/ }

/*N*/ IMPL_LINK( ScViewCfg, GridCommitHdl, void *, EMPTYARG )
/*N*/ {
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 const ScGridOptions& rGrid = GetGridOptions();
/*N*/ 	return 0;
/*N*/ }



}
