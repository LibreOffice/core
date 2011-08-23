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

// INCLUDE ---------------------------------------------------------------

#include "dpsave.hxx"
#include "miscuno.hxx"
#include "scerrors.hxx"

#include <tools/debug.hxx>
#include <tools/stream.hxx>

#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/XHierarchiesSupplier.hpp>
#include <com/sun/star/sheet/XLevelsSupplier.hpp>
#include <com/sun/star/sheet/XMembersSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/util/XCloneable.hpp>
namespace binfilter {

using namespace ::com::sun::star;

// -----------------------------------------------------------------------

#define SC_DPSAVEMODE_NO		0
#define SC_DPSAVEMODE_YES		1
#define SC_DPSAVEMODE_DONTKNOW	2

// -----------------------------------------------------------------------

//!	move to a header file
#define DP_PROP_COLUMNGRAND			"ColumnGrand"
#define DP_PROP_FUNCTION			"Function"
#define DP_PROP_IGNOREEMPTY			"IgnoreEmptyRows"
#define DP_PROP_ISDATALAYOUT		"IsDataLayoutDimension"
#define DP_PROP_ISVISIBLE			"IsVisible"
#define DP_PROP_ORIENTATION			"Orientation"
#define DP_PROP_REPEATIFEMPTY		"RepeatIfEmpty"
#define DP_PROP_ROWGRAND			"RowGrand"
#define DP_PROP_SHOWDETAILS			"ShowDetails"
#define DP_PROP_SHOWEMPTY			"ShowEmpty"
#define DP_PROP_SUBTOTALS			"SubTotals"
#define DP_PROP_USEDHIERARCHY		"UsedHierarchy"

// -----------------------------------------------------------------------

/*N*/ void lcl_SetBoolProperty( const uno::Reference<beans::XPropertySet>& xProp,
/*N*/ 							const ::rtl::OUString& rName, sal_Bool bValue )
/*N*/ {
/*N*/ 	//!	move to ScUnoHelpFunctions?
/*N*/ 
/*N*/ 	xProp->setPropertyValue( rName, uno::Any( &bValue, getBooleanCppuType() ) );
/*N*/ }
/*N*/ 
/*N*/ // -----------------------------------------------------------------------
/*N*/ 
/*N*/ void lcl_SkipExtra( SvStream& rStream )
/*N*/ {
/*N*/ 	USHORT nExtra;
/*N*/ 	rStream >> nExtra;
/*N*/ 	if ( nExtra )
/*N*/ 	{
/*N*/ 		rStream.SeekRel( nExtra );
/*N*/ 		if ( rStream.GetError() == SVSTREAM_OK )
/*N*/ 			rStream.SetError( SCWARN_IMPORT_INFOLOST );
/*N*/ 	}
/*N*/ }
/*N*/ 
/*N*/ // -----------------------------------------------------------------------
/*N*/ 
/*N*/ ScDPSaveMember::ScDPSaveMember(const String& rName) :
/*N*/ 	aName( rName ),
/*N*/ 	nVisibleMode( SC_DPSAVEMODE_DONTKNOW ),
/*N*/ 	nShowDetailsMode( SC_DPSAVEMODE_DONTKNOW )
/*N*/ {
/*N*/ }
/*N*/ 
/*N*/ ScDPSaveMember::ScDPSaveMember(const ScDPSaveMember& r) :
/*N*/ 	aName( r.aName ),
/*N*/ 	nVisibleMode( r.nVisibleMode ),
/*N*/ 	nShowDetailsMode( r.nShowDetailsMode )
/*N*/ {
/*N*/ }
/*N*/ 
/*N*/ ScDPSaveMember::ScDPSaveMember(SvStream& rStream)
/*N*/ {
/*N*/ 	rStream.ReadByteString( aName, rStream.GetStreamCharSet() );
/*N*/ 	rStream >> nVisibleMode;
/*N*/ 	rStream >> nShowDetailsMode;
/*N*/ 
/*N*/ 	lcl_SkipExtra( rStream );		// reads at least 1 USHORT
/*N*/ }
/*N*/ 
/*N*/ void ScDPSaveMember::Store( SvStream& rStream ) const
/*N*/ {
/*N*/ 	rStream.WriteByteString( aName, rStream.GetStreamCharSet() );
/*N*/ 	rStream << nVisibleMode;
/*N*/ 	rStream << nShowDetailsMode;
/*N*/ 
/*N*/ 	rStream << (USHORT) 0;	// nExtra
/*N*/ }
/*N*/ 
/*N*/ ScDPSaveMember::~ScDPSaveMember()
/*N*/ {
/*N*/ }
/*N*/ 
/*N*/ BOOL ScDPSaveMember::operator== ( const ScDPSaveMember& r ) const
/*N*/ {
/*N*/ 	if ( aName			  != r.aName 			||
/*N*/ 		 nVisibleMode	  != r.nVisibleMode		||
/*N*/ 		 nShowDetailsMode != r.nShowDetailsMode )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }
/*N*/ 
/*N*/ void ScDPSaveMember::SetIsVisible(BOOL bSet)
/*N*/ {
/*N*/ 	nVisibleMode = bSet;
/*N*/ }
/*N*/ 
/*N*/ void ScDPSaveMember::SetShowDetails(BOOL bSet)
/*N*/ {
/*N*/ 	nShowDetailsMode = bSet;
/*N*/ }
/*N*/ 
/*N*/ void ScDPSaveMember::WriteToSource( const uno::Reference<uno::XInterface>& xMember )
/*N*/ {
/*N*/ 	//	nothing to do?
/*N*/ 	if ( nVisibleMode == SC_DPSAVEMODE_DONTKNOW && nShowDetailsMode == SC_DPSAVEMODE_DONTKNOW )
/*N*/ 		return;
/*N*/ 
/*N*/ 	uno::Reference<beans::XPropertySet> xMembProp( xMember, uno::UNO_QUERY );
/*N*/ 	DBG_ASSERT( xMembProp.is(), "no properties at member" );
/*N*/ 	if ( xMembProp.is() )
/*N*/ 	{
/*N*/ 		// exceptions are caught at ScDPSaveData::WriteToSource
/*N*/ 
/*N*/ 		if ( nVisibleMode != SC_DPSAVEMODE_DONTKNOW )
/*N*/ 			lcl_SetBoolProperty( xMembProp,
/*N*/ 					::rtl::OUString::createFromAscii(DP_PROP_ISVISIBLE), (BOOL)nVisibleMode );
/*N*/ 
/*N*/ 		if ( nShowDetailsMode != SC_DPSAVEMODE_DONTKNOW )
/*N*/ 			lcl_SetBoolProperty( xMembProp,
/*N*/ 					::rtl::OUString::createFromAscii(DP_PROP_SHOWDETAILS), (BOOL)nShowDetailsMode );
/*N*/ 	}
/*N*/ }
/*N*/ 
/*N*/ // -----------------------------------------------------------------------
/*N*/ 
/*N*/ ScDPSaveDimension::ScDPSaveDimension(const String& rName, BOOL bDataLayout) :
/*N*/ 	aName( rName ),
/*N*/ 	bIsDataLayout( bDataLayout ),
/*N*/ 	bDupFlag( FALSE ),
/*N*/ 	nOrientation( sheet::DataPilotFieldOrientation_HIDDEN ),
/*N*/ 	bSubTotalDefault( TRUE ),
/*N*/ 	nSubTotalCount( 0 ),
/*N*/ 	pSubTotalFuncs( NULL ),
/*N*/ 	nShowEmptyMode( SC_DPSAVEMODE_DONTKNOW ),
/*N*/ 	nFunction( sheet::GeneralFunction_AUTO ),
/*N*/ 	nUsedHierarchy( -1 ),
/*N*/ 	pLayoutName( NULL )
/*N*/ {
/*N*/ }
/*N*/ 
/*N*/ ScDPSaveDimension::ScDPSaveDimension(const ScDPSaveDimension& r) :
/*N*/ 	aName( r.aName ),
/*N*/ 	bIsDataLayout( r.bIsDataLayout ),
/*N*/ 	bDupFlag( r.bDupFlag ),
/*N*/ 	nOrientation( r.nOrientation ),
/*N*/ 	bSubTotalDefault( r.bSubTotalDefault ),
/*N*/ 	nSubTotalCount( r.nSubTotalCount ),
/*N*/ 	pSubTotalFuncs( NULL ),
/*N*/ 	nShowEmptyMode( r.nShowEmptyMode ),
/*N*/ 	nFunction( r.nFunction ),
/*N*/ 	nUsedHierarchy( r.nUsedHierarchy )
/*N*/ {
/*N*/ 	if ( nSubTotalCount && r.pSubTotalFuncs )
/*N*/ 	{
/*N*/ 		pSubTotalFuncs = new USHORT[nSubTotalCount];
/*N*/ 		for (long nSub=0; nSub<nSubTotalCount; nSub++)
/*N*/ 			pSubTotalFuncs[nSub] = r.pSubTotalFuncs[nSub];
/*N*/ 	}
/*N*/ 
/*N*/ 	long nCount = r.aMemberList.Count();
/*N*/ 	for (long i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		ScDPSaveMember* pNew = new ScDPSaveMember( *(ScDPSaveMember*)r.aMemberList.GetObject(i) );
/*N*/ 		aMemberList.Insert( pNew, LIST_APPEND );
/*N*/ 	}
/*N*/ 	if (r.pLayoutName)
/*N*/ 		pLayoutName = new String( *(r.pLayoutName) );
/*N*/ 	else
/*N*/ 		pLayoutName = NULL;
/*N*/ }
/*N*/ 
/*N*/ ScDPSaveDimension::ScDPSaveDimension(SvStream& rStream)
/*N*/ {
/*N*/ 	long i;
/*N*/ 
/*N*/ 	rStream.ReadByteString( aName, rStream.GetStreamCharSet() );
/*N*/ 	rStream >> bIsDataLayout;
/*N*/ 
/*N*/ 	rStream >> bDupFlag;
/*N*/ 
/*N*/ 	rStream >> nOrientation;
/*N*/ 	rStream >> nFunction;			// enum GeneralFunction
/*N*/ 	rStream >> nUsedHierarchy;
/*N*/ 
/*N*/ 	rStream >> nShowEmptyMode;		//!	at level
/*N*/ 
/*N*/ 	rStream >> bSubTotalDefault;	//!	at level
/*N*/ 	rStream >> nSubTotalCount;
/*N*/ 	if (nSubTotalCount)
/*N*/ 	{
/*N*/ 		pSubTotalFuncs = new USHORT[nSubTotalCount];
/*N*/ 		for (i=0; i<nSubTotalCount; i++)
/*N*/ 			rStream >> pSubTotalFuncs[i];
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pSubTotalFuncs = NULL;
/*N*/ 
/*N*/ 	lcl_SkipExtra( rStream );		// reads at least 1 USHORT
/*N*/ 
/*N*/ 	long nNewCount;
/*N*/ 	rStream >> nNewCount;
/*N*/ 	for (i=0; i<nNewCount; i++)
/*N*/ 	{
/*N*/ 		ScDPSaveMember* pNew = new ScDPSaveMember( rStream );
/*N*/ 		aMemberList.Insert( pNew, LIST_APPEND );
/*N*/ 	}
/*N*/ 	pLayoutName = NULL;
/*N*/ }
/*N*/ 
/*N*/ void ScDPSaveDimension::Store( SvStream& rStream ) const
/*N*/ {
/*N*/ 	long i;
/*N*/ 
/*N*/ 	rStream.WriteByteString( aName, rStream.GetStreamCharSet() );
/*N*/ 	rStream << bIsDataLayout;
/*N*/ 
/*N*/ 	rStream << bDupFlag;
/*N*/ 
/*N*/ 	rStream << nOrientation;
/*N*/ 	rStream << nFunction;			// enum GeneralFunction
/*N*/ 	rStream << nUsedHierarchy;
/*N*/ 
/*N*/ 	rStream << nShowEmptyMode;		//!	at level
/*N*/ 
/*N*/ 	//!	subtotals at level
/*N*/ 	rStream << bSubTotalDefault;
/*N*/ 	long nSubCnt = pSubTotalFuncs ? nSubTotalCount : 0;
/*N*/ 	rStream << nSubCnt;
/*N*/ 	for (i=0; i<nSubCnt; i++)
/*N*/ 		rStream << pSubTotalFuncs[i];
/*N*/ 
/*N*/ 	rStream << (USHORT) 0;	// nExtra
/*N*/ 
/*N*/ 	long nCount = aMemberList.Count();
/*N*/ 	rStream << nCount;
/*N*/ 	for (i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		const ScDPSaveMember* pMember = (const ScDPSaveMember*)aMemberList.GetObject(i);
/*N*/ 		pMember->Store( rStream );
/*N*/ 	}
/*N*/ }
/*N*/ 
/*N*/ ScDPSaveDimension::~ScDPSaveDimension()
/*N*/ {
/*N*/ 	long nCount = aMemberList.Count();
/*N*/ 	for (long i=0; i<nCount; i++)
/*N*/ 		delete (ScDPSaveMember*)aMemberList.GetObject(i);
/*N*/ 	aMemberList.Clear();
/*N*/ 	if (pLayoutName)
/*N*/ 		delete pLayoutName;
/*N*/   delete [] pSubTotalFuncs;
/*N*/ }
/*N*/ 
/*N*/ BOOL ScDPSaveDimension::operator== ( const ScDPSaveDimension& r ) const
/*N*/ {
/*N*/ 	if ( aName			  != r.aName			||
/*N*/ 		 bIsDataLayout    != r.bIsDataLayout	||
/*N*/ 		 bDupFlag		  != r.bDupFlag			||
/*N*/ 		 nOrientation	  != r.nOrientation		||
/*N*/ 		 nFunction		  != r.nFunction		||
/*N*/ 		 nUsedHierarchy   != r.nUsedHierarchy   ||
/*N*/ 		 nShowEmptyMode   != r.nShowEmptyMode   ||
/*N*/ 		 bSubTotalDefault != r.bSubTotalDefault ||
/*N*/ 		 nSubTotalCount   != r.nSubTotalCount )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	if ( nSubTotalCount && ( !pSubTotalFuncs || !r.pSubTotalFuncs ) )	// should not happen
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	long i;
/*N*/ 	for (i=0; i<nSubTotalCount; i++)
/*N*/ 		if ( pSubTotalFuncs[i] != r.pSubTotalFuncs[i] )
/*N*/ 			return FALSE;
/*N*/ 
/*N*/ 	long nCount = aMemberList.Count();
/*N*/ 	if ( nCount != r.aMemberList.Count() )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	for (i=0; i<nCount; i++)
/*N*/ 		if ( !( *(ScDPSaveMember*)aMemberList.GetObject(i) ==
/*N*/ 				*(ScDPSaveMember*)r.aMemberList.GetObject(i) ) )
/*N*/ 			return FALSE;
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }
/*N*/ 
/*N*/ 
/*N*/ void ScDPSaveDimension::SetOrientation(USHORT nNew)
/*N*/ {
/*N*/ 	nOrientation = nNew;
/*N*/ }
/*N*/ 
/*N*/ void ScDPSaveDimension::SetSubTotals(long nCount, const USHORT* pFuncs)
/*N*/ {
/*N*/ 	if (pSubTotalFuncs)
/*N*/ 		delete pSubTotalFuncs;
/*N*/ 	nSubTotalCount = nCount;
/*N*/ 	if ( nCount && pFuncs )
/*N*/ 	{
/*N*/ 		pSubTotalFuncs = new USHORT[nCount];
/*N*/ 		for (long i=0; i<nCount; i++)
/*N*/ 			pSubTotalFuncs[i] = pFuncs[i];
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pSubTotalFuncs = NULL;
/*N*/ 
/*N*/ 	bSubTotalDefault = FALSE;
/*N*/ }
/*N*/ 
/*N*/ void ScDPSaveDimension::SetShowEmpty(BOOL bSet)
/*N*/ {
/*N*/ 	nShowEmptyMode = bSet;
/*N*/ }
/*N*/ 
/*N*/ void ScDPSaveDimension::SetFunction(USHORT nNew)
/*N*/ {
/*N*/ 	nFunction = nNew;
/*N*/ }
/*N*/ 
/*N*/ void ScDPSaveDimension::SetUsedHierarchy(long nNew)
/*N*/ {
/*N*/ 	nUsedHierarchy = nNew;
/*N*/ }

/*N*/ BOOL ScDPSaveDimension::HasLayoutName() const
/*N*/ {
/*N*/ 	return ( pLayoutName != NULL );
/*N*/ }

/*N*/ void ScDPSaveDimension::ResetLayoutName()
/*N*/ {
/*N*/ 	delete pLayoutName;
/*N*/ 	pLayoutName = NULL;
/*N*/ }

/*N*/ void ScDPSaveDimension::SetLayoutName(const String* pName)
/*N*/ {
/*N*/ 	if (pName)
/*N*/ 	{
/*N*/ 		if (pLayoutName)
/*N*/ 			delete pLayoutName;
/*N*/ 		pLayoutName = new String( *pName );
/*N*/ 	}
/*N*/ }

/*N*/ const String& ScDPSaveDimension::GetLayoutName() const
/*N*/ {
/*N*/ 	if (pLayoutName)
/*N*/ 		return *pLayoutName;
/*N*/ 	return aName;
/*N*/ }

/*N*/ void ScDPSaveDimension::WriteToSource( const uno::Reference<uno::XInterface>& xDim )
/*N*/ {
/*N*/ 	uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
/*N*/ 	DBG_ASSERT( xDimProp.is(), "no properties at dimension" );
/*N*/ 	if ( xDimProp.is() )
/*N*/ 	{
/*N*/ 		// exceptions are caught at ScDPSaveData::WriteToSource
/*N*/ 		uno::Any aAny;
/*N*/ 
/*N*/ 		sheet::DataPilotFieldOrientation eOrient = (sheet::DataPilotFieldOrientation)nOrientation;
/*N*/ 		aAny <<= eOrient;
/*N*/ 		xDimProp->setPropertyValue( ::rtl::OUString::createFromAscii(DP_PROP_ORIENTATION), aAny );
/*N*/ 
/*N*/ 		sheet::GeneralFunction eFunc = (sheet::GeneralFunction)nFunction;
/*N*/ 		aAny <<= eFunc;
/*N*/ 		xDimProp->setPropertyValue( ::rtl::OUString::createFromAscii(DP_PROP_FUNCTION), aAny );
/*N*/ 
/*N*/ 		if ( nUsedHierarchy >= 0 )
/*N*/ 		{
/*N*/ 			aAny <<= (INT32)nUsedHierarchy;
/*N*/ 			xDimProp->setPropertyValue( ::rtl::OUString::createFromAscii(DP_PROP_USEDHIERARCHY), aAny );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//	Level loop outside of aMemberList loop
/*N*/ 	//	because SubTotals have to be set independently of known members
/*N*/ 
/*N*/ 	long nCount = aMemberList.Count();
/*N*/ 
/*N*/ 	long nHierCount = 0;
/*N*/ 	uno::Reference<container::XIndexAccess> xHiers;
/*N*/ 	uno::Reference<sheet::XHierarchiesSupplier> xHierSupp( xDim, uno::UNO_QUERY );
/*N*/ 	if ( xHierSupp.is() )
/*N*/ 	{
/*N*/ 		uno::Reference<container::XNameAccess> xHiersName = xHierSupp->getHierarchies();
/*N*/ 		xHiers = new ScNameToIndexAccess( xHiersName );
/*N*/ 		nHierCount = xHiers->getCount();
/*N*/ 	}
/*N*/ 
/*N*/ 	for (long nHier=0; nHier<nHierCount; nHier++)
/*N*/ 	{
/*N*/ 		uno::Reference<uno::XInterface> xHierarchy = ScUnoHelpFunctions::AnyToInterface( xHiers->getByIndex(nHier) );
/*N*/ 
/*N*/ 		long nLevCount = 0;
/*N*/ 		uno::Reference<container::XIndexAccess> xLevels;
/*N*/ 		uno::Reference<sheet::XLevelsSupplier> xLevSupp( xHierarchy, uno::UNO_QUERY );
/*N*/ 		if ( xLevSupp.is() )
/*N*/ 		{
/*N*/ 			uno::Reference<container::XNameAccess> xLevelsName = xLevSupp->getLevels();
/*N*/ 			xLevels = new ScNameToIndexAccess( xLevelsName );
/*N*/ 			nLevCount = xLevels->getCount();
/*N*/ 		}
/*N*/ 
/*N*/ 		for (long nLev=0; nLev<nLevCount; nLev++)
/*N*/ 		{
/*N*/ 			uno::Reference<uno::XInterface> xLevel = ScUnoHelpFunctions::AnyToInterface( xLevels->getByIndex(nLev) );
/*N*/ 			uno::Reference<beans::XPropertySet> xLevProp( xLevel, uno::UNO_QUERY );
/*N*/ 			DBG_ASSERT( xLevProp.is(), "no properties at level" );
/*N*/ 			if ( xLevProp.is() )
/*N*/ 			{
/*N*/ 				if ( !bSubTotalDefault )
/*N*/ 				{
/*N*/ 					if ( !pSubTotalFuncs )
/*N*/ 						nSubTotalCount = 0;
/*N*/ 
/*N*/ 					uno::Sequence<sheet::GeneralFunction> aSeq(nSubTotalCount);
/*N*/ 					sheet::GeneralFunction* pArray = aSeq.getArray();
/*N*/ 					for (long i=0; i<nSubTotalCount; i++)
/*N*/ 						pArray[i] = (sheet::GeneralFunction)pSubTotalFuncs[i];
/*N*/ 					uno::Any aAny;
/*N*/ 					aAny <<= aSeq;
/*N*/ 					xLevProp->setPropertyValue( ::rtl::OUString::createFromAscii(DP_PROP_SUBTOTALS), aAny );
/*N*/ 				}
/*N*/ 				if ( nShowEmptyMode != SC_DPSAVEMODE_DONTKNOW )
/*N*/ 					lcl_SetBoolProperty( xLevProp,
/*N*/ 						::rtl::OUString::createFromAscii(DP_PROP_SHOWEMPTY), (BOOL)nShowEmptyMode );
/*N*/ 
/*N*/ 				// exceptions are caught at ScDPSaveData::WriteToSource
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( nCount > 0 )
/*N*/ 			{
/*N*/ 				uno::Reference<sheet::XMembersSupplier> xMembSupp( xLevel, uno::UNO_QUERY );
/*N*/ 				if ( xMembSupp.is() )
/*N*/ 				{
/*N*/ 					uno::Reference<container::XNameAccess> xMembers = xMembSupp->getMembers();
/*N*/ 					if ( xMembers.is() )
/*N*/ 					{
/*N*/ 						for (long i=0; i<nCount; i++)
/*N*/ 						{
/*N*/ 							ScDPSaveMember* pMember = (ScDPSaveMember*)aMemberList.GetObject(i);
/*N*/ 							::rtl::OUString aName = pMember->GetName();
/*N*/ 							if ( xMembers->hasByName( aName ) )
/*N*/ 							{
/*N*/ 								uno::Reference<uno::XInterface> xMemberInt = ScUnoHelpFunctions::AnyToInterface(
/*N*/ 									xMembers->getByName( aName ) );
/*N*/ 								pMember->WriteToSource( xMemberInt );
/*N*/ 							}
/*N*/ 							// missing member is no error
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }
/*N*/ 
// -----------------------------------------------------------------------

/*N*/ ScDPSaveData::ScDPSaveData() :
/*N*/ 	nColumnGrandMode( SC_DPSAVEMODE_DONTKNOW ),
/*N*/ 	nRowGrandMode( SC_DPSAVEMODE_DONTKNOW ),
/*N*/ 	nIgnoreEmptyMode( SC_DPSAVEMODE_DONTKNOW ),
/*N*/ 	nRepeatEmptyMode( SC_DPSAVEMODE_DONTKNOW )
/*N*/ {
/*N*/ }

/*N*/ ScDPSaveData::ScDPSaveData(const ScDPSaveData& r) :
/*N*/ 	nColumnGrandMode( r.nColumnGrandMode ),
/*N*/ 	nRowGrandMode( r.nRowGrandMode ),
/*N*/ 	nIgnoreEmptyMode( r.nIgnoreEmptyMode ),
/*N*/ 	nRepeatEmptyMode( r.nRepeatEmptyMode )
/*N*/ {
/*N*/ 	long nCount = r.aDimList.Count();
/*N*/ 	for (long i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		ScDPSaveDimension* pNew = new ScDPSaveDimension( *(ScDPSaveDimension*)r.aDimList.GetObject(i) );
/*N*/ 		aDimList.Insert( pNew, LIST_APPEND );
/*N*/ 	}
/*N*/ }

/*N*/ ScDPSaveData& ScDPSaveData::operator= ( const ScDPSaveData& r )
/*N*/ {
/*N*/ 	if ( &r != this )
/*N*/ 	{
/*N*/ 		nColumnGrandMode = r.nColumnGrandMode;
/*N*/ 		nRowGrandMode    = r.nRowGrandMode;
/*N*/ 		nIgnoreEmptyMode = r.nIgnoreEmptyMode;
/*N*/ 		nRepeatEmptyMode = r.nRepeatEmptyMode;
/*N*/ 
/*N*/ 		//	remove old dimensions
/*N*/ 
/*N*/ 		long nCount = aDimList.Count();
/*N*/ 		long i;
/*N*/ 		for (i=0; i<nCount; i++)
/*N*/ 			delete (ScDPSaveDimension*)aDimList.GetObject(i);
/*N*/ 		aDimList.Clear();
/*N*/ 
/*N*/ 		//	copy new dimensions
/*N*/ 
/*N*/ 		nCount = r.aDimList.Count();
/*N*/ 		for (i=0; i<nCount; i++)
/*N*/ 		{
/*N*/ 			ScDPSaveDimension* pNew =
/*N*/ 				new ScDPSaveDimension( *(ScDPSaveDimension*)r.aDimList.GetObject(i) );
/*N*/ 			aDimList.Insert( pNew, LIST_APPEND );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return *this;
/*N*/ }
/*N*/ 
/*N*/ BOOL ScDPSaveData::operator== ( const ScDPSaveData& r ) const
/*N*/ {
/*N*/ 	if ( nColumnGrandMode != r.nColumnGrandMode ||
/*N*/ 		 nRowGrandMode    != r.nRowGrandMode	||
/*N*/ 		 nIgnoreEmptyMode != r.nIgnoreEmptyMode ||
/*N*/ 		 nRepeatEmptyMode != r.nRepeatEmptyMode )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	long nCount = aDimList.Count();
/*N*/ 	if ( nCount != r.aDimList.Count() )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	for (long i=0; i<nCount; i++)
/*N*/ 		if ( !( *(ScDPSaveDimension*)aDimList.GetObject(i) ==
/*N*/ 				*(ScDPSaveDimension*)r.aDimList.GetObject(i) ) )
/*N*/ 			return FALSE;
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ ScDPSaveData::~ScDPSaveData()
/*N*/ {
/*N*/ 	long nCount = aDimList.Count();
/*N*/ 	for (long i=0; i<nCount; i++)
/*N*/ 		delete (ScDPSaveDimension*)aDimList.GetObject(i);
/*N*/ 	aDimList.Clear();
/*N*/ }

/*N*/ ScDPSaveDimension* ScDPSaveData::GetDimensionByName(const String& rName)
/*N*/ {
/*N*/ 	long nCount = aDimList.Count();
/*N*/ 	for (long i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		ScDPSaveDimension* pDim = (ScDPSaveDimension*)aDimList.GetObject(i);
/*N*/ 		if ( pDim->GetName() == rName && !pDim->IsDataLayout() )
/*N*/ 			return pDim;
/*N*/ 	}
/*N*/ 	ScDPSaveDimension* pNew = new ScDPSaveDimension( rName, FALSE );
/*N*/ 	aDimList.Insert( pNew, LIST_APPEND );
/*N*/ 	return pNew;
/*N*/ }

/*N*/ ScDPSaveDimension* ScDPSaveData::GetExistingDimensionByName(const String& rName)
/*N*/ {
/*N*/ 	long nCount = aDimList.Count();
/*N*/ 	for (long i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		ScDPSaveDimension* pDim = (ScDPSaveDimension*)aDimList.GetObject(i);
/*N*/ 		if ( pDim->GetName() == rName && !pDim->IsDataLayout() )
/*N*/ 			return pDim;
/*N*/ 	}
/*N*/ 	return NULL;		// don't create new
/*N*/ }

/*N*/ ScDPSaveDimension* ScDPSaveData::GetDataLayoutDimension()
/*N*/ {
/*N*/ 	long nCount = aDimList.Count();
/*N*/ 	for (long i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		ScDPSaveDimension* pDim = (ScDPSaveDimension*)aDimList.GetObject(i);
/*N*/ 		if ( pDim->IsDataLayout() )
/*N*/ 			return pDim;
/*N*/ 	}
/*N*/ 	ScDPSaveDimension* pNew = new ScDPSaveDimension( String(), TRUE );
/*N*/ 	aDimList.Insert( pNew, LIST_APPEND );
/*N*/ 	return pNew;
/*N*/ }
/*N*/ 
/*N*/ ScDPSaveDimension* ScDPSaveData::DuplicateDimension(const String& rName)
/*N*/ {
/*N*/ 	//	always insert new
/*N*/ 	//!	check if dimension is there?
/*N*/ 
/*N*/ 	ScDPSaveDimension* pOld = GetDimensionByName( rName );
/*N*/ 	ScDPSaveDimension* pNew = new ScDPSaveDimension( *pOld );
/*N*/ 	pNew->SetDupFlag( TRUE );
/*N*/ 	aDimList.Insert( pNew, LIST_APPEND );
/*N*/ 	return pNew;
/*N*/ }
/*N*/ 
/*N*/ void ScDPSaveData::SetColumnGrand(BOOL bSet)
/*N*/ {
/*N*/ 	nColumnGrandMode = bSet;
/*N*/ }
/*N*/ 
/*N*/ void ScDPSaveData::SetRowGrand(BOOL bSet)
/*N*/ {
/*N*/ 	nRowGrandMode = bSet;
/*N*/ }
/*N*/ 
/*N*/ void ScDPSaveData::SetIgnoreEmptyRows(BOOL bSet)
/*N*/ {
/*N*/ 	nIgnoreEmptyMode = bSet;
/*N*/ }
/*N*/ 
/*N*/ void ScDPSaveData::SetRepeatIfEmpty(BOOL bSet)
/*N*/ {
/*N*/ 	nRepeatEmptyMode = bSet;
/*N*/ }
/*N*/ 
/*N*/ void lcl_ResetOrient( const uno::Reference<sheet::XDimensionsSupplier>& xSource )
/*N*/ {
/*N*/ 	sheet::DataPilotFieldOrientation eOrient = sheet::DataPilotFieldOrientation_HIDDEN;
/*N*/ 
/*N*/ 	uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
/*N*/ 	uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
/*N*/ 	long nIntCount = xIntDims->getCount();
/*N*/ 	for (long nIntDim=0; nIntDim<nIntCount; nIntDim++)
/*N*/ 	{
/*N*/ 		uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface( xIntDims->getByIndex(nIntDim) );
/*N*/ 		uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
/*N*/ 		if (xDimProp.is())
/*N*/ 		{
/*N*/ 			uno::Any aAny;
/*N*/ 			aAny <<= eOrient;
/*N*/ 			xDimProp->setPropertyValue( ::rtl::OUString::createFromAscii(DP_PROP_ORIENTATION), aAny );
/*N*/ 		}
/*N*/ 	}
/*N*/ }
/*N*/ 
/*N*/ void ScDPSaveData::WriteToSource( const uno::Reference<sheet::XDimensionsSupplier>& xSource )
/*N*/ {
/*N*/ 	if (!xSource.is())
/*N*/ 		return;
/*N*/ 
/*N*/ 	//	source options must be first!
/*N*/ 
/*N*/ 	uno::Reference<beans::XPropertySet> xSourceProp( xSource, uno::UNO_QUERY );
/*N*/ 	DBG_ASSERT( xSourceProp.is(), "no properties at source" );
/*N*/ 	if ( xSourceProp.is() )
/*N*/ 	{
/*N*/ 		//	source options are not available for external sources
/*N*/ 		//!	use XPropertySetInfo to test for availability?
/*N*/ 
/*N*/ 		try
/*N*/ 		{
/*N*/ 			if ( nIgnoreEmptyMode != SC_DPSAVEMODE_DONTKNOW )
/*N*/ 				lcl_SetBoolProperty( xSourceProp,
/*N*/ 					::rtl::OUString::createFromAscii(DP_PROP_IGNOREEMPTY), (BOOL)nIgnoreEmptyMode );
/*N*/ 			if ( nRepeatEmptyMode != SC_DPSAVEMODE_DONTKNOW )
/*N*/ 				lcl_SetBoolProperty( xSourceProp,
/*N*/ 					::rtl::OUString::createFromAscii(DP_PROP_REPEATIFEMPTY), (BOOL)nRepeatEmptyMode );
/*N*/ 		}
/*N*/ 		catch(uno::Exception&)
/*N*/ 		{
/*N*/ 			// no error
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// exceptions in the other calls are errors
/*N*/ 	try
/*N*/ 	{
/*N*/ 		//	reset all orientations
/*N*/ 		//!	"forgetSettings" or similar at source ?????
/*N*/ 		//!	reset all duplicated dimensions, or reuse them below !!!
/*N*/ 
/*N*/ 		lcl_ResetOrient( xSource );
/*N*/ 
/*N*/ 		long nCount = aDimList.Count();
/*N*/ 		for (long i=0; i<nCount; i++)
/*N*/ 		{
/*N*/ 			ScDPSaveDimension* pDim = (ScDPSaveDimension*)aDimList.GetObject(i);
/*N*/ 			::rtl::OUString aName = pDim->GetName();
/*N*/ 			BOOL bData = pDim->IsDataLayout();
/*N*/ 
/*N*/ 			//!	getByName for ScDPSource, including DataLayoutDimension !!!!!!!!
/*N*/ 
/*N*/ 			uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
/*N*/ 			uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
/*N*/ 			long nIntCount = xIntDims->getCount();
/*N*/ 			BOOL bFound = FALSE;
/*N*/ 			for (long nIntDim=0; nIntDim<nIntCount && !bFound; nIntDim++)
/*N*/ 			{
/*N*/ 				uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface( xIntDims->getByIndex(nIntDim) );
/*N*/ 				if ( bData )
/*N*/ 				{
/*N*/ 					uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
/*N*/ 					if ( xDimProp.is() )
/*N*/ 					{
/*N*/ 						bFound = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
/*N*/ 									::rtl::OUString::createFromAscii(DP_PROP_ISDATALAYOUT) );
/*N*/ 						//!	error checking -- is "IsDataLayoutDimension" property required??
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
/*N*/ 					if ( xDimName.is() && xDimName->getName() == aName )
/*N*/ 						bFound = TRUE;
/*N*/ 				}
/*N*/ 
/*N*/ 				if ( bFound )
/*N*/ 				{
/*N*/ 					if ( pDim->GetDupFlag() )
/*N*/ 					{
/*N*/ 						String aNewName = pDim->GetName();
/*N*/ 
/*N*/ 						// different name for each duplication of a (real) dimension...
/*N*/ 						for (long j=0; j<=i; j++)	//! Test !!!!!!
/*N*/ 							aNewName += '*';		//! modify name at creation of SaveDimension
/*N*/ 
/*N*/ 						uno::Reference<util::XCloneable> xCloneable( xIntDim, uno::UNO_QUERY );
/*N*/ 						DBG_ASSERT( xCloneable.is(), "cannot clone dimension" );
/*N*/ 						if (xCloneable.is())
/*N*/ 						{
/*N*/ 							uno::Reference<util::XCloneable> xNew = xCloneable->createClone();
/*N*/ 							uno::Reference<container::XNamed> xNewName( xNew, uno::UNO_QUERY );
/*N*/ 							if (xNewName.is())
/*N*/ 							{
/*N*/ 								xNewName->setName( aNewName );
/*N*/ 								pDim->WriteToSource( xNew );
/*N*/ 							}
/*N*/ 						}
/*N*/ 					}
/*N*/ 					else
/*N*/ 						pDim->WriteToSource( xIntDim );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			DBG_ASSERT(bFound, "WriteToSource: Dimension not found");
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( xSourceProp.is() )
/*N*/ 		{
/*N*/ 			if ( nColumnGrandMode != SC_DPSAVEMODE_DONTKNOW )
/*N*/ 				lcl_SetBoolProperty( xSourceProp,
/*N*/ 					::rtl::OUString::createFromAscii(DP_PROP_COLUMNGRAND), (BOOL)nColumnGrandMode );
/*N*/ 			if ( nRowGrandMode != SC_DPSAVEMODE_DONTKNOW )
/*N*/ 				lcl_SetBoolProperty( xSourceProp,
/*N*/ 					::rtl::OUString::createFromAscii(DP_PROP_ROWGRAND), (BOOL)nRowGrandMode );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	catch(uno::Exception&)
/*N*/ 	{
/*N*/ 		DBG_ERROR("exception in WriteToSource");
/*N*/ 	}
/*N*/ }
/*N*/ 
/*N*/ void ScDPSaveData::Store( SvStream& rStream ) const
/*N*/ {
/*N*/ 	//!	multi-header for individual entries
/*N*/ 
/*N*/ 	long nCount = aDimList.Count();
/*N*/ 	rStream << nCount;
/*N*/ 	for (long i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		const ScDPSaveDimension* pDim = (const ScDPSaveDimension*)aDimList.GetObject(i);
/*N*/ 		pDim->Store( rStream );
/*N*/ 	}
/*N*/ 
/*N*/ 	rStream << nColumnGrandMode;
/*N*/ 	rStream << nRowGrandMode;
/*N*/ 	rStream << nIgnoreEmptyMode;
/*N*/ 	rStream << nRepeatEmptyMode;
/*N*/ 
/*N*/ 	rStream << (USHORT) 0;	// nExtra
/*N*/ }
/*N*/ 
/*N*/ void ScDPSaveData::Load( SvStream& rStream )
/*N*/ {
/*N*/ 	//!	multi-header for individual entries
/*N*/ 
/*N*/ 	DBG_ASSERT( aDimList.Count()==0, "ScDPSaveData::Load not empty" );
/*N*/ 
/*N*/ 	long nNewCount;
/*N*/ 	rStream >> nNewCount;
/*N*/ 	for (long i=0; i<nNewCount; i++)
/*N*/ 	{
/*N*/ 		ScDPSaveDimension* pNew = new ScDPSaveDimension( rStream );
/*N*/ 		aDimList.Insert( pNew, LIST_APPEND );
/*N*/ 	}
/*N*/ 
/*N*/ 	rStream >> nColumnGrandMode;
/*N*/ 	rStream >> nRowGrandMode;
/*N*/ 	rStream >> nIgnoreEmptyMode;
/*N*/ 	rStream >> nRepeatEmptyMode;
/*N*/ 
/*N*/ 	lcl_SkipExtra( rStream );		// reads at least 1 USHORT
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
