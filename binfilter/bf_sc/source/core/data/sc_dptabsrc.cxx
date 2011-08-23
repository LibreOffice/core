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

#include <tools/debug.hxx>
#include <rtl/math.hxx>
#include <bf_svtools/itemprop.hxx>

#include "dptabsrc.hxx"
#include "dptabdat.hxx"
#include "global.hxx"
#include "collect.hxx"
#include "datauno.hxx"		// ScDataUnoConversion
#include "unoguard.hxx"
#include "miscuno.hxx"
#include "unonames.hxx"
#include "dptabres.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>

#ifndef _UNOTOOLS_CALENDARWRAPPER_HXX
#include <unotools/calendarwrapper.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_CALENDARDISPLAYINDEX_HPP_
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>
#endif
namespace binfilter {

using namespace ::com::sun::star;

// -----------------------------------------------------------------------

#define SC_MINCOUNT_LIMIT	1000000

// -----------------------------------------------------------------------

/*N*/ SC_SIMPLE_SERVICE_INFO( ScDPSource,      "ScDPSource",      "com.sun.star.sheet.DataPilotSource" )
/*N*/ SC_SIMPLE_SERVICE_INFO( ScDPDimensions,  "ScDPDimensions",  "com.sun.star.sheet.DataPilotSourceDimensions" )
/*N*/ SC_SIMPLE_SERVICE_INFO( ScDPDimension,   "ScDPDimension",   "com.sun.star.sheet.DataPilotSourceDimension" )
/*N*/ SC_SIMPLE_SERVICE_INFO( ScDPHierarchies, "ScDPHierarchies", "com.sun.star.sheet.DataPilotSourceHierarcies" )
/*N*/ SC_SIMPLE_SERVICE_INFO( ScDPHierarchy,   "ScDPHierarchy",   "com.sun.star.sheet.DataPilotSourceHierarcy" )
/*N*/ SC_SIMPLE_SERVICE_INFO( ScDPLevels,      "ScDPLevels",      "com.sun.star.sheet.DataPilotSourceLevels" )
/*N*/ SC_SIMPLE_SERVICE_INFO( ScDPLevel,       "ScDPLevel",       "com.sun.star.sheet.DataPilotSourceLevel" )

// -----------------------------------------------------------------------

// property maps for PropertySetInfo
//	DataDescription / NumberFormat are internal

// -----------------------------------------------------------------------

//!	move to a header?
/*N*/ BOOL lcl_GetBoolFromAny( const uno::Any& aAny )
/*N*/ {
/*N*/ 	if ( aAny.getValueTypeClass() == uno::TypeClass_BOOLEAN )
/*N*/ 		return *(sal_Bool*)aAny.getValue();
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ void lcl_SetBoolInAny( uno::Any& rAny, BOOL bValue )
/*N*/ {
/*N*/ 	rAny.setValue( &bValue, getBooleanCppuType() );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ ScDPSource::ScDPSource( ScDPTableData* pD ) :
/*N*/ 	pData( pD ),
/*N*/ 	pDimensions( NULL ),
/*N*/ 	nColDimCount( 0 ),
/*N*/ 	nRowDimCount( 0 ),
/*N*/ 	nDataDimCount( 0 ),
/*N*/ 	nPageDimCount( 0 ),
/*N*/ 	nDupCount( 0 ),
/*N*/ 	bResultOverflow( FALSE ),
/*N*/ 	pResData( NULL ),
/*N*/ 	pColResRoot( NULL ),
/*N*/ 	pRowResRoot( NULL ),
/*N*/ 	pColResults( NULL ),
/*N*/ 	pRowResults( NULL ),
/*N*/ 	bColumnGrand( TRUE ),		// default is true
/*N*/ 	bRowGrand( TRUE ),
/*N*/ 	bIgnoreEmptyRows( FALSE ),
/*N*/ 	bRepeatIfEmpty( FALSE )
/*N*/ {
/*N*/ 	pData->SetEmptyFlags( bIgnoreEmptyRows, bRepeatIfEmpty );
/*N*/ }

/*N*/ ScDPSource::~ScDPSource()
/*N*/ {
/*N*/ 	delete pData;			// ScDPTableData is not ref-counted
/*N*/ 
/*N*/ 	if (pDimensions)
/*N*/ 		pDimensions->release();		// ref-counted
/*N*/ 
/*N*/ 	//!	free lists
/*N*/ 
/*N*/ 	delete[] pColResults;
/*N*/ 	delete[] pRowResults;
/*N*/ 
/*N*/ 	delete pColResRoot;
/*N*/ 	delete pRowResRoot;
/*N*/ 	delete pResData;
/*N*/ }

/*N*/ USHORT ScDPSource::GetOrientation(long nColumn)
/*N*/ {
/*N*/ 	long i;
/*N*/ 	for (i=0; i<nColDimCount; i++)
/*N*/ 		if (nColDims[i] == nColumn)
/*N*/ 			return sheet::DataPilotFieldOrientation_COLUMN;
/*N*/ 	for (i=0; i<nRowDimCount; i++)
/*N*/ 		if (nRowDims[i] == nColumn)
/*N*/ 			return sheet::DataPilotFieldOrientation_ROW;
/*N*/ 	for (i=0; i<nDataDimCount; i++)
/*N*/ 		if (nDataDims[i] == nColumn)
/*N*/ 			return sheet::DataPilotFieldOrientation_DATA;
/*N*/ 	for (i=0; i<nPageDimCount; i++)
/*?*/ 		if (nPageDims[i] == nColumn)
/*?*/ 			return sheet::DataPilotFieldOrientation_PAGE;
/*N*/ 	return sheet::DataPilotFieldOrientation_HIDDEN;
/*N*/ }



/*N*/ long ScDPSource::GetPosition(long nColumn)
/*N*/ {
/*N*/ 	long i;
/*N*/ 	for (i=0; i<nColDimCount; i++)
/*N*/ 		if (nColDims[i] == nColumn)
/*N*/ 			return i;
/*N*/ 	for (i=0; i<nRowDimCount; i++)
/*N*/ 		if (nRowDims[i] == nColumn)
/*N*/ 			return i;
/*N*/ 	for (i=0; i<nDataDimCount; i++)
/*N*/ 		if (nDataDims[i] == nColumn)
/*N*/ 			return i;
/*N*/ 	for (i=0; i<nPageDimCount; i++)
/*?*/ 		if (nPageDims[i] == nColumn)
/*?*/ 			return i;
/*N*/ 	return 0;
/*N*/ }

/*N*/ BOOL lcl_TestSubTotal( BOOL& rAllowed, long nColumn, long* pArray, long nCount, ScDPSource* pSource )
/*N*/ {
/*N*/ 	for (long i=0; i<nCount; i++)
/*N*/ 		if (pArray[i] == nColumn)
/*N*/ 		{
/*N*/ 			//	no subtotals for data layout dim, no matter where
/*N*/ 			if ( pSource->IsDataLayoutDimension(nColumn) )
/*N*/ 				rAllowed = FALSE;
/*N*/ 			else
/*N*/ 			{
/*N*/ 				//	no subtotals if no other dim but data layout follows
/*N*/ 				long nNextIndex = i+1;
/*N*/ 				if ( nNextIndex < nCount && pSource->IsDataLayoutDimension(pArray[nNextIndex]) )
/*N*/ 					++nNextIndex;
/*N*/ 				if ( nNextIndex >= nCount )
/*N*/ 					rAllowed = FALSE;
/*N*/ 			}
/*N*/ 
/*N*/ 			return TRUE;	// found
/*N*/ 		}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ BOOL ScDPSource::SubTotalAllowed(long nColumn)
/*N*/ {
/*N*/ 	//!	cache this at ScDPResultData
/*N*/ 	BOOL bAllowed = TRUE;
/*N*/ 	if ( lcl_TestSubTotal( bAllowed, nColumn, nColDims, nColDimCount, this ) )
/*N*/ 		return bAllowed;
/*N*/ 	if ( lcl_TestSubTotal( bAllowed, nColumn, nRowDims, nRowDimCount, this ) )
/*N*/ 		return bAllowed;
/*N*/ 	return bAllowed;
/*N*/ }

/*N*/ void lcl_RemoveDim( long nRemove, long* pDims, long& rCount )
/*N*/ {
/*N*/ 	for (long i=0; i<rCount; i++)
/*N*/ 		if ( pDims[i] == nRemove )
/*N*/ 		{
/*?*/ 			for (long j=i; j+1<rCount; j++)
/*?*/ 				pDims[j] = pDims[j+1];
/*?*/ 			--rCount;
/*?*/ 			return;
/*N*/ 		}
/*N*/ }

/*N*/ void ScDPSource::SetOrientation(long nColumn, USHORT nNew)
/*N*/ {
/*N*/ 	//!	change to no-op if new orientation is equal to old?
/*N*/ 
/*N*/ 	// remove from old list
/*N*/ 	lcl_RemoveDim( nColumn, nColDims, nColDimCount );
/*N*/ 	lcl_RemoveDim( nColumn, nRowDims, nRowDimCount );
/*N*/ 	lcl_RemoveDim( nColumn, nDataDims, nDataDimCount );
/*N*/ 	lcl_RemoveDim( nColumn, nPageDims, nPageDimCount );
/*N*/ 
/*N*/ 	// add to new list
/*N*/ 	switch (nNew)
/*N*/ 	{
/*N*/ 		case sheet::DataPilotFieldOrientation_COLUMN:
/*N*/ 			nColDims[nColDimCount++] = nColumn;
/*N*/ 			break;
/*N*/ 		case sheet::DataPilotFieldOrientation_ROW:
/*N*/ 			nRowDims[nRowDimCount++] = nColumn;
/*N*/ 			break;
/*N*/ 		case sheet::DataPilotFieldOrientation_DATA:
/*N*/ 			nDataDims[nDataDimCount++] = nColumn;
/*N*/ 			break;
/*N*/ 		case sheet::DataPilotFieldOrientation_PAGE:
/*?*/ 			nPageDims[nPageDimCount++] = nColumn;
/*N*/ 			break;
/*N*/ 	}
/*N*/ }

/*N*/ BOOL ScDPSource::IsDataLayoutDimension(long nDim)
/*N*/ {
/*N*/ 	return nDim == pData->GetColumnCount();
/*N*/ }


/*N*/ BOOL ScDPSource::IsDateDimension(long nDim)
/*N*/ {
/*N*/ 	return pData->IsDateDimension(nDim);
/*N*/ }

/*N*/ ScDPDimensions* ScDPSource::GetDimensionsObject()
/*N*/ {
/*N*/ 	if (!pDimensions)
/*N*/ 	{
/*N*/ 		pDimensions = new ScDPDimensions(this);
/*N*/ 		pDimensions->acquire();						// ref-counted
/*N*/ 	}
/*N*/ 	return pDimensions;
/*N*/ }

/*N*/ uno::Reference<container::XNameAccess> SAL_CALL ScDPSource::getDimensions() throw(uno::RuntimeException)
/*N*/ {
/*N*/ 	return GetDimensionsObject();
/*N*/ }



/*N*/ long ScDPSource::GetSourceDim(long nDim)
/*N*/ {
/*N*/ 	//	original source dimension or data layout dimension?
/*N*/ 	if ( nDim <= pData->GetColumnCount() )
/*N*/ 		return nDim;
/*N*/ 
/*?*/ 	if ( nDim < pDimensions->getCount() )
/*?*/ 	{
/*?*/ 		ScDPDimension* pDimObj = pDimensions->getByIndex( nDim );
/*?*/ 		if ( pDimObj )
/*?*/ 		{
/*?*/ 			long nSource = pDimObj->GetSourceDim();
/*?*/ 			if ( nSource >= 0 )
/*?*/ 				return nSource;
/*?*/ 		}
/*?*/ 	}
/*?*/ 
/*?*/ 	DBG_ERROR("GetSourceDim: wrong dim");
/*?*/ 	return nDim;
/*N*/ }

/*?*/ uno::Sequence< uno::Sequence<sheet::DataResult> > SAL_CALL ScDPSource::getResults()
/*?*/ 															throw(uno::RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return uno::Sequence< uno::Sequence<sheet::DataResult> >(0); //STRIP001 
/*?*/ }

/*?*/ void SAL_CALL ScDPSource::refresh() throw(uno::RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ }

/*?*/ void SAL_CALL ScDPSource::addRefreshListener( const uno::Reference<util::XRefreshListener >& l )
/*?*/ 												throw(uno::RuntimeException)
/*?*/ {
/*?*/ 	DBG_ERROR("not implemented");	//! exception?
/*?*/ }

/*?*/ void SAL_CALL ScDPSource::removeRefreshListener( const uno::Reference<util::XRefreshListener >& l )
/*?*/ 												throw(uno::RuntimeException)
/*?*/ {
/*?*/ 	DBG_ERROR("not implemented");	//! exception?
/*?*/ }


/*N*/ BOOL ScDPSource::getColumnGrand() const
/*N*/ {
/*N*/ 	return bColumnGrand;
/*N*/ }

/*N*/ void ScDPSource::setColumnGrand(BOOL bSet)
/*N*/ {
/*N*/ 	bColumnGrand = bSet;
/*N*/ }

/*N*/ BOOL ScDPSource::getRowGrand() const
/*N*/ {
/*N*/ 	return bRowGrand;
/*N*/ }

/*N*/ void ScDPSource::setRowGrand(BOOL bSet)
/*N*/ {
/*N*/ 	bRowGrand = bSet;
/*N*/ }

/*N*/ BOOL ScDPSource::getIgnoreEmptyRows() const
/*N*/ {
/*N*/ 	return bIgnoreEmptyRows;
/*N*/ }

/*N*/ void ScDPSource::setIgnoreEmptyRows(BOOL bSet)
/*N*/ {
/*N*/ 	bIgnoreEmptyRows = bSet;
/*N*/ 	pData->SetEmptyFlags( bIgnoreEmptyRows, bRepeatIfEmpty );
/*N*/ }

/*N*/ BOOL ScDPSource::getRepeatIfEmpty() const
/*N*/ {
/*N*/ 	return bRepeatIfEmpty;
/*N*/ }

/*N*/ void ScDPSource::setRepeatIfEmpty(BOOL bSet)
/*N*/ {
/*N*/ 	bRepeatIfEmpty = bSet;
/*N*/ 	pData->SetEmptyFlags( bIgnoreEmptyRows, bRepeatIfEmpty );
/*N*/ }








// XPropertySet

/*?*/ uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDPSource::getPropertySetInfo()
/*?*/ 														throw(uno::RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return uno::Reference<beans::XPropertySetInfo>(0); //STRIP001 
/*?*/ }

/*N*/ void SAL_CALL ScDPSource::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
/*N*/ 				throw(beans::UnknownPropertyException, beans::PropertyVetoException,
/*N*/ 						lang::IllegalArgumentException, lang::WrappedTargetException,
/*N*/ 						uno::RuntimeException)
/*N*/ {
/*N*/ 	String aNameStr = aPropertyName;
/*N*/ 	if ( aNameStr.EqualsAscii( SC_UNO_COLGRAND ) )
/*N*/ 		setColumnGrand( lcl_GetBoolFromAny( aValue ) );
/*N*/ 	else if ( aNameStr.EqualsAscii( SC_UNO_ROWGRAND ) )
/*N*/ 		setRowGrand( lcl_GetBoolFromAny( aValue ) );
/*N*/ 	else if ( aNameStr.EqualsAscii( SC_UNO_IGNOREEM ) )
/*N*/ 		setIgnoreEmptyRows( lcl_GetBoolFromAny( aValue ) );
/*N*/ 	else if ( aNameStr.EqualsAscii( SC_UNO_REPEATIF ) )
/*N*/ 		setRepeatIfEmpty( lcl_GetBoolFromAny( aValue ) );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		DBG_ERROR("unknown property");
/*N*/ 		//!	THROW( UnknownPropertyException() );
/*N*/ 	}
/*N*/ }

/*N*/ uno::Any SAL_CALL ScDPSource::getPropertyValue( const ::rtl::OUString& aPropertyName )
/*N*/ 				throw(beans::UnknownPropertyException, lang::WrappedTargetException,
/*N*/ 						uno::RuntimeException)
/*N*/ {
/*N*/ 	uno::Any aRet;
/*N*/ 	String aNameStr = aPropertyName;
/*N*/ 	if ( aNameStr.EqualsAscii( SC_UNO_COLGRAND ) )
/*N*/ 		lcl_SetBoolInAny( aRet, getColumnGrand() );
/*N*/ 	else if ( aNameStr.EqualsAscii( SC_UNO_ROWGRAND ) )
/*N*/ 		lcl_SetBoolInAny( aRet, getRowGrand() );
/*N*/ 	else if ( aNameStr.EqualsAscii( SC_UNO_IGNOREEM ) )
/*N*/ 		lcl_SetBoolInAny( aRet, getIgnoreEmptyRows() );
/*N*/ 	else if ( aNameStr.EqualsAscii( SC_UNO_REPEATIF ) )
/*N*/ 		lcl_SetBoolInAny( aRet, getRepeatIfEmpty() );
/*N*/ 	else if ( aNameStr.EqualsAscii( SC_UNO_DATADESC ) )				// read-only
/*?*/ 	{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	aRet <<= ::rtl::OUString( getDataDescription() );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		DBG_ERROR("unknown property");
/*N*/ 		//!	THROW( UnknownPropertyException() );
/*N*/ 	}
/*N*/ 	return aRet;
/*N*/ }

/*N*/ SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDPSource )

// -----------------------------------------------------------------------

/*N*/ ScDPDimensions::ScDPDimensions( ScDPSource* pSrc ) :
/*N*/ 	pSource( pSrc ),
/*N*/ 	ppDims( NULL )
/*N*/ {
/*N*/ 	//!	hold pSource
/*N*/ 
/*N*/ 	// include data layout dimension and duplicated dimensions
/*N*/ 	nDimCount = pSource->GetData()->GetColumnCount() + 1 + pSource->GetDupCount();
/*N*/ }

/*N*/ ScDPDimensions::~ScDPDimensions()
/*N*/ {
/*N*/ 	//!	release pSource
/*N*/ 
/*N*/ 	if (ppDims)
/*N*/ 	{
/*N*/ 		for (long i=0; i<nDimCount; i++)
/*N*/ 			if ( ppDims[i] )
/*N*/ 				ppDims[i]->release();			// ref-counted
/*N*/ 		delete[] ppDims;
/*N*/ 	}
/*N*/ }


// very simple XNameAccess implementation using getCount/getByIndex

/*N*/ uno::Any SAL_CALL ScDPDimensions::getByName( const ::rtl::OUString& aName )
/*N*/ 			throw(container::NoSuchElementException,
/*N*/ 					lang::WrappedTargetException, uno::RuntimeException)
/*N*/ {
/*N*/ 	long nCount = getCount();
/*N*/ 	for (long i=0; i<nCount; i++)
/*N*/ 		if ( getByIndex(i)->getName() == aName )
/*N*/ 		{
/*N*/ 			uno::Reference<container::XNamed> xNamed = getByIndex(i);
/*N*/ 			uno::Any aRet;
/*N*/ 			aRet <<= xNamed;
/*N*/ 			return aRet;
/*N*/ 		}
/*N*/ 
/*N*/ 	throw container::NoSuchElementException();
/*N*/ 	return uno::Any();
/*N*/ }

/*N*/ uno::Sequence<rtl::OUString> SAL_CALL ScDPDimensions::getElementNames() throw(uno::RuntimeException)
/*N*/ {
/*N*/ 	long nCount = getCount();
/*N*/ 	uno::Sequence<rtl::OUString> aSeq(nCount);
/*N*/ 	::rtl::OUString* pArr = aSeq.getArray();
/*N*/ 	for (long i=0; i<nCount; i++)
/*N*/ 		pArr[i] = getByIndex(i)->getName();
/*N*/ 	return aSeq;
/*N*/ }

/*?*/ sal_Bool SAL_CALL ScDPDimensions::hasByName( const ::rtl::OUString& aName ) throw(uno::RuntimeException)
/*?*/ {
/*?*/ 	long nCount = getCount();
/*?*/ 	for (long i=0; i<nCount; i++)
/*?*/ 		if ( getByIndex(i)->getName() == aName )
/*?*/ 			return TRUE;
/*?*/ 	return FALSE;
/*?*/ }

/*?*/ uno::Type SAL_CALL ScDPDimensions::getElementType() throw(uno::RuntimeException)
/*?*/ {
/*?*/ 	return getCppuType((uno::Reference<container::XNamed>*)0);
/*?*/ }

/*?*/ sal_Bool SAL_CALL ScDPDimensions::hasElements() throw(uno::RuntimeException)
/*?*/ {
/*?*/ 	return ( getCount() > 0 );
/*?*/ }

// end of XNameAccess implementation

/*N*/ long ScDPDimensions::getCount() const
/*N*/ {
/*N*/ 	//	in tabular data, every column of source data is a dimension
/*N*/ 
/*N*/ 	return nDimCount;
/*N*/ }

/*N*/ ScDPDimension* ScDPDimensions::getByIndex(long nIndex) const
/*N*/ {
/*N*/ 	if ( nIndex >= 0 && nIndex < nDimCount )
/*N*/ 	{
/*N*/ 		if ( !ppDims )
/*N*/ 		{
/*N*/ 			((ScDPDimensions*)this)->ppDims = new ScDPDimension*[nDimCount];
/*N*/ 			for (long i=0; i<nDimCount; i++)
/*N*/ 				ppDims[i] = NULL;
/*N*/ 		}
/*N*/ 		if ( !ppDims[nIndex] )
/*N*/ 		{
/*N*/ 			ppDims[nIndex] = new ScDPDimension( pSource, nIndex );
/*N*/ 			ppDims[nIndex]->acquire();		// ref-counted
/*N*/ 		}
/*N*/ 
/*N*/ 		return ppDims[nIndex];
/*N*/ 	}
/*N*/ 
/*N*/ 	return NULL;	//! exception?
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ ScDPDimension::ScDPDimension( ScDPSource* pSrc, long nD ) :
/*N*/ 	pSource( pSrc ),
/*N*/ 	nDim( nD ),
/*N*/ 	pHierarchies( NULL ),
/*N*/ 	nUsedHier( 0 ),
/*N*/ 	nFunction( SUBTOTAL_FUNC_SUM ),		// sum is default
/*N*/ 	nSourceDim( -1 )
/*N*/ {
/*N*/ 	//!	hold pSource
/*N*/ }

/*N*/ ScDPDimension::~ScDPDimension()
/*N*/ {
/*N*/ 	//!	release pSource
/*N*/ 
/*N*/ 	if ( pHierarchies )
/*N*/ 		pHierarchies->release();	// ref-counted
/*N*/ }

/*N*/ ScDPHierarchies* ScDPDimension::GetHierarchiesObject()
/*N*/ {
/*N*/ 	if (!pHierarchies)
/*N*/ 	{
/*N*/ 		pHierarchies = new ScDPHierarchies( pSource, nDim );
/*N*/ 		pHierarchies->acquire();		// ref-counted
/*N*/ 	}
/*N*/ 	return pHierarchies;
/*N*/ }

/*N*/ uno::Reference<container::XNameAccess> SAL_CALL ScDPDimension::getHierarchies()
/*N*/ 													throw(uno::RuntimeException)
/*N*/ {
/*N*/ 	return GetHierarchiesObject();
/*N*/ }

/*N*/ ::rtl::OUString SAL_CALL ScDPDimension::getName() throw(uno::RuntimeException)
/*N*/ {
/*N*/ 	if (aName.Len())
/*N*/ 		return aName;
/*N*/ 	else
/*N*/ 		return pSource->GetData()->getDimensionName( nDim );
/*N*/ }

/*?*/ void SAL_CALL ScDPDimension::setName( const ::rtl::OUString& rNewName ) throw(uno::RuntimeException)
/*?*/ {
/*?*/ 	//	used after cloning
/*?*/ 	aName = String( rNewName );
/*?*/ }

/*N*/ USHORT ScDPDimension::getOrientation() const
/*N*/ {
/*N*/ 	return pSource->GetOrientation( nDim );
/*N*/ }

/*N*/ void ScDPDimension::setOrientation(USHORT nNew)
/*N*/ {
/*N*/ 	pSource->SetOrientation( nDim, nNew );
/*N*/ }

/*N*/ long ScDPDimension::getPosition() const
/*N*/ {
/*N*/ 	return pSource->GetPosition( nDim );
/*N*/ }


/*N*/ BOOL ScDPDimension::getIsDataLayoutDimension() const
/*N*/ {
/*N*/ 	return pSource->GetData()->getIsDataLayoutDimension( nDim );
/*N*/ }

/*N*/ USHORT ScDPDimension::getFunction() const
/*N*/ {
/*N*/ 	return nFunction;
/*N*/ }

/*N*/ void ScDPDimension::setFunction(USHORT nNew)
/*N*/ {
/*N*/ 	nFunction = nNew;
/*N*/ }

/*N*/ long ScDPDimension::getUsedHierarchy() const
/*N*/ {
/*N*/ 	return nUsedHier;
/*N*/ }



/*?*/ uno::Reference<util::XCloneable> SAL_CALL ScDPDimension::createClone() throw(uno::RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return uno::Reference<util::XCloneable>(0); //STRIP001 
/*?*/ }


// XPropertySet

/*?*/ uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDPDimension::getPropertySetInfo()
/*?*/ 														throw(uno::RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return uno::Reference<beans::XPropertySetInfo>(0); //STRIP001 
/*?*/ }

/*N*/ void SAL_CALL ScDPDimension::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
/*N*/ 				throw(beans::UnknownPropertyException, beans::PropertyVetoException,
/*N*/ 						lang::IllegalArgumentException, lang::WrappedTargetException,
/*N*/ 						uno::RuntimeException)
/*N*/ {
/*N*/ 	String aNameStr = aPropertyName;
/*N*/ 	if ( aNameStr.EqualsAscii( SC_UNO_POSITION ) )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 INT32 nInt;
/*N*/ 	}
/*N*/ 	else if ( aNameStr.EqualsAscii( SC_UNO_USEDHIER ) )
/*N*/ 	{
/*?*/ 		INT32 nInt;
/*?*/ 		if (aValue >>= nInt)
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	setUsedHierarchy( nInt );
/*N*/ 	}
/*N*/ 	else if ( aNameStr.EqualsAscii( SC_UNO_ORIENTAT ) )
/*N*/ 	{
/*N*/ 		sheet::DataPilotFieldOrientation eEnum;
/*N*/ 		if (aValue >>= eEnum)
/*N*/ 			setOrientation( eEnum );
/*N*/ 	}
/*N*/ 	else if ( aNameStr.EqualsAscii( SC_UNO_FUNCTION ) )
/*N*/ 	{
/*N*/ 		sheet::GeneralFunction eEnum;
/*N*/ 		if (aValue >>= eEnum)
/*N*/ 			setFunction( eEnum );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		DBG_ERROR("unknown property");
/*N*/ 		//!	THROW( UnknownPropertyException() );
/*N*/ 	}
/*N*/ }

/*N*/ uno::Any SAL_CALL ScDPDimension::getPropertyValue( const ::rtl::OUString& aPropertyName )
/*N*/ 				throw(beans::UnknownPropertyException, lang::WrappedTargetException,
/*N*/ 						uno::RuntimeException)
/*N*/ {
/*N*/ 	uno::Any aRet;
/*N*/ 	String aNameStr = aPropertyName;
/*N*/ 	if ( aNameStr.EqualsAscii( SC_UNO_POSITION ) )
/*N*/ 		aRet <<= (sal_Int32) getPosition();
/*N*/ 	else if ( aNameStr.EqualsAscii( SC_UNO_USEDHIER ) )
/*N*/ 		aRet <<= (sal_Int32) getUsedHierarchy();
/*N*/ 	else if ( aNameStr.EqualsAscii( SC_UNO_ORIENTAT ) )
/*N*/ 	{
/*N*/ 		sheet::DataPilotFieldOrientation eVal = (sheet::DataPilotFieldOrientation)getOrientation();
/*N*/ 		aRet <<= eVal;
/*N*/ 	}
/*N*/ 	else if ( aNameStr.EqualsAscii( SC_UNO_FUNCTION ) )
/*N*/ 	{
/*N*/ 		sheet::GeneralFunction eVal = (sheet::GeneralFunction)getFunction();
/*N*/ 		aRet <<= eVal;
/*N*/ 	}
/*N*/ 	else if ( aNameStr.EqualsAscii( SC_UNO_ISDATALA ) )					// read-only properties
/*N*/ 		lcl_SetBoolInAny( aRet, getIsDataLayoutDimension() );
/*N*/ 	else if ( aNameStr.EqualsAscii( SC_UNO_NUMBERFO ) )
/*?*/ 	{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	aRet <<= (sal_Int32) pSource->GetData()->GetNumberFormat(
/*N*/ 	else if ( aNameStr.EqualsAscii( SC_UNO_ORIGINAL ) )
/*N*/ 	{
/*N*/ 		uno::Reference<container::XNamed> xOriginal;
/*N*/ 		if (nSourceDim >= 0)
/*?*/ 			xOriginal = pSource->GetDimensionsObject()->getByIndex(nSourceDim);
/*N*/ 		aRet <<= xOriginal;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		DBG_ERROR("unknown property");
/*N*/ 		//!	THROW( UnknownPropertyException() );
/*N*/ 	}
/*N*/ 	return aRet;
/*N*/ }

/*N*/ SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDPDimension )

// -----------------------------------------------------------------------

/*N*/ ScDPHierarchies::ScDPHierarchies( ScDPSource* pSrc, long nD ) :
/*N*/ 	pSource( pSrc ),
/*N*/ 	nDim( nD ),
/*N*/ 	ppHiers( NULL )
/*N*/ {
/*N*/ 	//!	hold pSource
/*N*/ 
/*N*/ 	//	date columns have 3 hierarchies (flat/quarter/week), other columns only one
/*N*/ 	long nSrcDim = pSource->GetSourceDim( nDim );
/*N*/ 	if ( pSource->IsDateDimension( nSrcDim ) )
/*?*/ 		nHierCount = SC_DAPI_DATE_HIERARCHIES;
/*N*/ 	else
/*N*/ 		nHierCount = 1;
/*N*/ }

/*N*/ ScDPHierarchies::~ScDPHierarchies()
/*N*/ {
/*N*/ 	//!	release pSource
/*N*/ 
/*N*/ 	if (ppHiers)
/*N*/ 	{
/*N*/ 		for (long i=0; i<nHierCount; i++)
/*N*/ 			if ( ppHiers[i] )
/*N*/ 				ppHiers[i]->release();		// ref-counted
/*N*/ 		delete[] ppHiers;
/*N*/ 	}
/*N*/ }

// very simple XNameAccess implementation using getCount/getByIndex

/*N*/ uno::Any SAL_CALL ScDPHierarchies::getByName( const ::rtl::OUString& aName )
/*N*/ 			throw(container::NoSuchElementException,
/*N*/ 					lang::WrappedTargetException, uno::RuntimeException)
/*N*/ {
/*N*/ 	long nCount = getCount();
/*N*/ 	for (long i=0; i<nCount; i++)
/*N*/ 		if ( getByIndex(i)->getName() == aName )
/*N*/ 		{
/*N*/ 			uno::Reference<container::XNamed> xNamed = getByIndex(i);
/*N*/ 			uno::Any aRet;
/*N*/ 			aRet <<= xNamed;
/*N*/ 			return aRet;
/*N*/ 		}
/*N*/ 
/*?*/ 	throw container::NoSuchElementException();
/*N*/ 	return uno::Any();
/*N*/ }

/*N*/ uno::Sequence<rtl::OUString> SAL_CALL ScDPHierarchies::getElementNames() throw(uno::RuntimeException)
/*N*/ {
/*N*/ 	long nCount = getCount();
/*N*/ 	uno::Sequence<rtl::OUString> aSeq(nCount);
/*N*/ 	::rtl::OUString* pArr = aSeq.getArray();
/*N*/ 	for (long i=0; i<nCount; i++)
/*N*/ 		pArr[i] = getByIndex(i)->getName();
/*N*/ 	return aSeq;
/*N*/ }

/*?*/ sal_Bool SAL_CALL ScDPHierarchies::hasByName( const ::rtl::OUString& aName ) throw(uno::RuntimeException)
/*?*/ {
/*?*/ 	long nCount = getCount();
/*?*/ 	for (long i=0; i<nCount; i++)
/*?*/ 		if ( getByIndex(i)->getName() == aName )
/*?*/ 			return TRUE;
/*?*/ 	return FALSE;
/*?*/ }

/*?*/ uno::Type SAL_CALL ScDPHierarchies::getElementType() throw(uno::RuntimeException)
/*?*/ {
/*?*/ 	return getCppuType((uno::Reference<container::XNamed>*)0);
/*?*/ }

/*?*/ sal_Bool SAL_CALL ScDPHierarchies::hasElements() throw(uno::RuntimeException)
/*?*/ {
/*?*/ 	return ( getCount() > 0 );
/*?*/ }

// end of XNameAccess implementation

/*N*/ long ScDPHierarchies::getCount() const
/*N*/ {
/*N*/ 	return nHierCount;
/*N*/ }

/*N*/ ScDPHierarchy* ScDPHierarchies::getByIndex(long nIndex) const
/*N*/ {
/*N*/ 	//	pass hierarchy index to new object in case the implementation
/*N*/ 	//	will be extended to more than one hierarchy
/*N*/ 
/*N*/ 	if ( nIndex >= 0 && nIndex < nHierCount )
/*N*/ 	{
/*N*/ 		if ( !ppHiers )
/*N*/ 		{
/*N*/ 			((ScDPHierarchies*)this)->ppHiers = new ScDPHierarchy*[nHierCount];
/*N*/ 			for (long i=0; i<nHierCount; i++)
/*N*/ 				ppHiers[i] = NULL;
/*N*/ 		}
/*N*/ 		if ( !ppHiers[nIndex] )
/*N*/ 		{
/*N*/ 			ppHiers[nIndex] = new ScDPHierarchy( pSource, nDim, nIndex );
/*N*/ 			ppHiers[nIndex]->acquire();			// ref-counted
/*N*/ 		}
/*N*/ 
/*N*/ 		return ppHiers[nIndex];
/*N*/ 	}
/*N*/ 
/*N*/ 	return NULL;	//! exception?
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ ScDPHierarchy::ScDPHierarchy( ScDPSource* pSrc, long nD, long nH ) :
/*N*/ 	pSource( pSrc ),
/*N*/ 	nDim( nD ),
/*N*/ 	nHier( nH ),
/*N*/ 	pLevels( NULL )
/*N*/ {
/*N*/ 	//!	hold pSource
/*N*/ }

/*N*/ ScDPHierarchy::~ScDPHierarchy()
/*N*/ {
/*N*/ 	//!	release pSource
/*N*/ 
/*N*/ 	if (pLevels)
/*N*/ 		pLevels->release();		// ref-counted
/*N*/ }

/*N*/ ScDPLevels* ScDPHierarchy::GetLevelsObject()
/*N*/ {
/*N*/ 	if (!pLevels)
/*N*/ 	{
/*N*/ 		pLevels = new ScDPLevels( pSource, nDim, nHier );
/*N*/ 		pLevels->acquire();		// ref-counted
/*N*/ 	}
/*N*/ 	return pLevels;
/*N*/ }

/*N*/ uno::Reference<container::XNameAccess> SAL_CALL ScDPHierarchy::getLevels()
/*N*/ 													throw(uno::RuntimeException)
/*N*/ {
/*N*/ 	return GetLevelsObject();
/*N*/ }

/*N*/ ::rtl::OUString SAL_CALL ScDPHierarchy::getName() throw(uno::RuntimeException)
/*N*/ {
/*N*/ 	String aRet;		//!	globstr-ID !!!!
/*N*/ 	switch (nHier)
/*N*/ 	{
/*N*/ 		case SC_DAPI_HIERARCHY_FLAT:
/*N*/ 			aRet = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("flat"));
/*N*/ 			break;	//! name ???????
/*N*/ 		case SC_DAPI_HIERARCHY_QUARTER:
/*?*/ 			aRet = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Quarter"));
/*?*/ 			break;	//! name ???????
/*N*/ 		case SC_DAPI_HIERARCHY_WEEK:
/*?*/ 			aRet = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Week"));
/*?*/ 			break;	//! name ???????
/*N*/ 	}
/*N*/ 	return aRet;
/*N*/ }

/*?*/ void SAL_CALL ScDPHierarchy::setName( const ::rtl::OUString& rNewName ) throw(uno::RuntimeException)
/*?*/ {
/*?*/ 	DBG_ERROR("not implemented");		//! exception?
/*?*/ }

// -----------------------------------------------------------------------

/*N*/ ScDPLevels::ScDPLevels( ScDPSource* pSrc, long nD, long nH ) :
/*N*/ 	pSource( pSrc ),
/*N*/ 	nDim( nD ),
/*N*/ 	nHier( nH ),
/*N*/ 	ppLevs( NULL )
/*N*/ {
/*N*/ 	//!	hold pSource
/*N*/ 
/*N*/ 	//	text columns have only one level
/*N*/ 
/*N*/ 	long nSrcDim = pSource->GetSourceDim( nDim );
/*N*/ 	if ( pSource->IsDateDimension( nSrcDim ) )
/*N*/ 	{
/*?*/ 		switch ( nHier )
/*?*/ 		{
/*?*/ 			case SC_DAPI_HIERARCHY_FLAT:	nLevCount = SC_DAPI_FLAT_LEVELS;	break;
/*?*/ 			case SC_DAPI_HIERARCHY_QUARTER:	nLevCount = SC_DAPI_QUARTER_LEVELS;	break;
/*?*/ 			case SC_DAPI_HIERARCHY_WEEK:	nLevCount = SC_DAPI_WEEK_LEVELS;	break;
/*?*/ 			default:
/*?*/ 				DBG_ERROR("wrong hierarchy");
/*?*/ 				nLevCount = 0;
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		nLevCount = 1;
/*N*/ }

/*N*/ ScDPLevels::~ScDPLevels()
/*N*/ {
/*N*/ 	//!	release pSource
/*N*/ 
/*N*/ 	if (ppLevs)
/*N*/ 	{
/*N*/ 		for (long i=0; i<nLevCount; i++)
/*N*/ 			if ( ppLevs[i] )
/*N*/ 				ppLevs[i]->release();	// ref-counted
/*N*/ 		delete[] ppLevs;
/*N*/ 	}
/*N*/ }

// very simple XNameAccess implementation using getCount/getByIndex

/*N*/ uno::Any SAL_CALL ScDPLevels::getByName( const ::rtl::OUString& aName )
/*N*/ 			throw(container::NoSuchElementException,
/*N*/ 					lang::WrappedTargetException, uno::RuntimeException)
/*N*/ {
/*N*/ 	long nCount = getCount();
/*N*/ 	for (long i=0; i<nCount; i++)
/*N*/ 		if ( getByIndex(i)->getName() == aName )
/*N*/ 		{
/*N*/ 			uno::Reference<container::XNamed> xNamed = getByIndex(i);
/*N*/ 			uno::Any aRet;
/*N*/ 			aRet <<= xNamed;
/*N*/ 			return aRet;
/*N*/ 		}
/*N*/ 
/*?*/ 	throw container::NoSuchElementException();
/*N*/ 	return uno::Any();
/*N*/ }

/*N*/ uno::Sequence<rtl::OUString> SAL_CALL ScDPLevels::getElementNames() throw(uno::RuntimeException)
/*N*/ {
/*N*/ 	long nCount = getCount();
/*N*/ 	uno::Sequence<rtl::OUString> aSeq(nCount);
/*N*/ 	::rtl::OUString* pArr = aSeq.getArray();
/*N*/ 	for (long i=0; i<nCount; i++)
/*N*/ 		pArr[i] = getByIndex(i)->getName();
/*N*/ 	return aSeq;
/*N*/ }

/*N*/ sal_Bool SAL_CALL ScDPLevels::hasByName( const ::rtl::OUString& aName ) throw(uno::RuntimeException)
/*N*/ {
/*N*/ 	long nCount = getCount();
/*N*/ 	for (long i=0; i<nCount; i++)
/*N*/ 		if ( getByIndex(i)->getName() == aName )
/*N*/ 			return TRUE;
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ uno::Type SAL_CALL ScDPLevels::getElementType() throw(uno::RuntimeException)
/*N*/ {
/*N*/ 	return getCppuType((uno::Reference<container::XNamed>*)0);
/*N*/ }

/*N*/ sal_Bool SAL_CALL ScDPLevels::hasElements() throw(uno::RuntimeException)
/*N*/ {
/*N*/ 	return ( getCount() > 0 );
/*N*/ }

// end of XNameAccess implementation

/*N*/ long ScDPLevels::getCount() const
/*N*/ {
/*N*/ 	return nLevCount;
/*N*/ }

/*N*/ ScDPLevel* ScDPLevels::getByIndex(long nIndex) const
/*N*/ {
/*N*/ 	if ( nIndex >= 0 && nIndex < nLevCount )
/*N*/ 	{
/*N*/ 		if ( !ppLevs )
/*N*/ 		{
/*N*/ 			((ScDPLevels*)this)->ppLevs = new ScDPLevel*[nLevCount];
/*N*/ 			for (long i=0; i<nLevCount; i++)
/*N*/ 				ppLevs[i] = NULL;
/*N*/ 		}
/*N*/ 		if ( !ppLevs[nIndex] )
/*N*/ 		{
/*N*/ 			ppLevs[nIndex] = new ScDPLevel( pSource, nDim, nHier, nIndex );
/*N*/ 			ppLevs[nIndex]->acquire();		// ref-counted
/*N*/ 		}
/*N*/ 
/*N*/ 		return ppLevs[nIndex];
/*N*/ 	}
/*N*/ 
/*N*/ 	return NULL;	//! exception?
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ ScDPLevel::ScDPLevel( ScDPSource* pSrc, long nD, long nH, long nL ) :
/*N*/ 	pSource( pSrc ),
/*N*/ 	nDim( nD ),
/*N*/ 	nHier( nH ),
/*N*/ 	nLev( nL ),
/*N*/ 	pMembers( NULL ),
/*N*/ 	bShowEmpty( FALSE )
/*N*/ {
/*N*/ 	//!	hold pSource
/*N*/ 	//	aSubTotals is empty
/*N*/ }

/*N*/ ScDPLevel::~ScDPLevel()
/*N*/ {
/*N*/ 	//!	release pSource
/*N*/ 
/*N*/ 	if ( pMembers )
/*?*/ 	{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	pMembers->release();	// ref-counted
/*N*/ }


/*?*/ uno::Reference<container::XNameAccess> SAL_CALL ScDPLevel::getMembers() throw(uno::RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return uno::Reference<container::XNameAccess>(0); //STRIP001 
/*?*/ }

/*?*/ uno::Sequence<sheet::MemberResult> SAL_CALL ScDPLevel::getResults() throw(uno::RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ 	return uno::Sequence<sheet::MemberResult>(0);		//! Error?
/*?*/ }

/*N*/ ::rtl::OUString SAL_CALL ScDPLevel::getName() throw(uno::RuntimeException)
/*N*/ {
/*N*/ 	long nSrcDim = pSource->GetSourceDim( nDim );
/*N*/ 	if ( pSource->IsDateDimension( nSrcDim ) )
/*?*/ 	{
/*?*/ 		String aRet;		//!	globstr-ID !!!!
/*?*/ 
/*?*/ 		if ( nHier == SC_DAPI_HIERARCHY_QUARTER )
/*?*/ 		{
/*?*/ 			switch ( nLev )
/*?*/ 			{
/*?*/ 				case SC_DAPI_LEVEL_YEAR:
/*?*/ 					aRet = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Year"));
/*?*/ 					break;
/*?*/ 				case SC_DAPI_LEVEL_QUARTER:
/*?*/ 					aRet = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Quarter"));
/*?*/ 					break;
/*?*/ 				case SC_DAPI_LEVEL_MONTH:
/*?*/ 					aRet = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Month"));
/*?*/ 					break;
/*?*/ 				case SC_DAPI_LEVEL_DAY:
/*?*/ 					aRet = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Day"));
/*?*/ 					break;
/*?*/ 			}
/*?*/ 		}
/*?*/ 		else if ( nHier == SC_DAPI_HIERARCHY_WEEK )
/*?*/ 		{
/*?*/ 			switch ( nLev )
/*?*/ 			{
/*?*/ 				case SC_DAPI_LEVEL_YEAR:
/*?*/ 					aRet = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Year"));
/*?*/ 					break;
/*?*/ 				case SC_DAPI_LEVEL_WEEK:
/*?*/ 					aRet = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Week"));
/*?*/ 					break;
/*?*/ 				case SC_DAPI_LEVEL_WEEKDAY:
/*?*/ 					aRet = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Weekday"));
/*?*/ 					break;
/*?*/ 			}
/*?*/ 		}
/*?*/ 		if (aRet.Len())
/*?*/ 			return aRet;
/*N*/ 	}
/*N*/ 
/*N*/ 	return pSource->GetData()->getDimensionName( nSrcDim );		// (original) dimension name
/*N*/ }

/*?*/ void SAL_CALL ScDPLevel::setName( const ::rtl::OUString& rNewName ) throw(uno::RuntimeException)
/*?*/ {
/*?*/ 	DBG_ERROR("not implemented");		//! exception?
/*?*/ }

/*N*/ uno::Sequence<sheet::GeneralFunction> ScDPLevel::getSubTotals() const
/*N*/ {
/*N*/ 	//!	separate functions for settings and evaluation?
/*N*/ 
/*N*/ 	long nSrcDim = pSource->GetSourceDim( nDim );
/*N*/ 	if ( !pSource->SubTotalAllowed( nSrcDim ) )
/*N*/ 		return uno::Sequence<sheet::GeneralFunction>(0);
/*N*/ 
/*N*/ 	return aSubTotals;
/*N*/ }



/*N*/ void ScDPLevel::setShowEmpty(BOOL bSet)
/*N*/ {
/*N*/ 	bShowEmpty = bSet;
/*N*/ }

// XPropertySet

/*N*/ uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDPLevel::getPropertySetInfo()
/*N*/ 														throw(uno::RuntimeException)
/*N*/ {
/*N*/ 	ScUnoGuard aGuard;
/*N*/ 
/*N*/ 	static SfxItemPropertyMap aDPLevelMap_Impl[] =
/*N*/ 	{
/*N*/ 		{MAP_CHAR_LEN(SC_UNO_SHOWEMPT),	0,	&getBooleanCppuType(),									 0, 0 },
/*N*/ 		{MAP_CHAR_LEN(SC_UNO_SUBTOTAL),	0,	&getCppuType((uno::Sequence<sheet::GeneralFunction>*)0), 0, 0 },
/*N*/ 		{0,0,0,0}
/*N*/ 	};
/*N*/ 	static uno::Reference<beans::XPropertySetInfo> aRef =
/*N*/ 		new SfxItemPropertySetInfo( aDPLevelMap_Impl );
/*N*/ 	return aRef;
/*N*/ }

/*N*/ void SAL_CALL ScDPLevel::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
/*N*/ 				throw(beans::UnknownPropertyException, beans::PropertyVetoException,
/*N*/ 						lang::IllegalArgumentException, lang::WrappedTargetException,
/*N*/ 						uno::RuntimeException)
/*N*/ {
/*N*/ 	String aNameStr = aPropertyName;
/*N*/ 	if ( aNameStr.EqualsAscii( SC_UNO_SHOWEMPT ) )
/*N*/ 		setShowEmpty( lcl_GetBoolFromAny( aValue ) );
/*N*/ 	else if ( aNameStr.EqualsAscii( SC_UNO_SUBTOTAL ) )
/*N*/ 	{
/*?*/ 		uno::Sequence<sheet::GeneralFunction> aSeq;
/*?*/ 		if ( aValue >>= aSeq )
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	setSubTotals( aSeq );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		DBG_ERROR("unknown property");
/*N*/ 		//!	THROW( UnknownPropertyException() );
/*N*/ 	}
/*N*/ }

/*N*/ uno::Any SAL_CALL ScDPLevel::getPropertyValue( const ::rtl::OUString& aPropertyName )
/*N*/ 				throw(beans::UnknownPropertyException, lang::WrappedTargetException,
/*N*/ 						uno::RuntimeException)
/*N*/ {
/*N*/ 	uno::Any aRet;
/*N*/ 	String aNameStr = aPropertyName;
/*N*/ 	if ( aNameStr.EqualsAscii( SC_UNO_SHOWEMPT ) )
/*?*/ 	{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	lcl_SetBoolInAny( aRet, getShowEmpty() );
/*N*/ 	else if ( aNameStr.EqualsAscii( SC_UNO_SUBTOTAL ) )
/*N*/ 	{
/*N*/ 		uno::Sequence<sheet::GeneralFunction> aSeq = getSubTotals();		//! avoid extra copy?
/*N*/ 		aRet <<= aSeq;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		DBG_ERROR("unknown property");
/*N*/ 		//!	THROW( UnknownPropertyException() );
/*N*/ 	}
/*N*/ 	return aRet;
/*N*/ }

/*N*/ SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDPLevel )

}
