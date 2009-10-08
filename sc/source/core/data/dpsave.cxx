/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dpsave.cxx,v $
 * $Revision: 1.13.32.3 $
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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include "dpsave.hxx"
#include "dpdimsave.hxx"
#include "miscuno.hxx"
#include "scerrors.hxx"
#include "unonames.hxx"
#include "global.hxx"

#include <tools/debug.hxx>

#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>
#include <com/sun/star/sheet/TableFilterField.hpp>
#include <com/sun/star/sheet/XHierarchiesSupplier.hpp>
#include <com/sun/star/sheet/XLevelsSupplier.hpp>
#include <com/sun/star/sheet/XMembersSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/util/XCloneable.hpp>

using namespace com::sun::star;

// -----------------------------------------------------------------------

#define SC_DPSAVEMODE_NO        0
#define SC_DPSAVEMODE_YES       1
#define SC_DPSAVEMODE_DONTKNOW  2

// -----------------------------------------------------------------------

//! move to a header file
//! use names from unonames.hxx?
#define DP_PROP_COLUMNGRAND         "ColumnGrand"
#define DP_PROP_FUNCTION            "Function"
#define DP_PROP_IGNOREEMPTY         "IgnoreEmptyRows"
#define DP_PROP_ISDATALAYOUT        "IsDataLayoutDimension"
#define DP_PROP_ISVISIBLE           "IsVisible"
#define DP_PROP_ORIENTATION         "Orientation"
#define DP_PROP_REPEATIFEMPTY       "RepeatIfEmpty"
#define DP_PROP_ROWGRAND            "RowGrand"
#define DP_PROP_SHOWDETAILS         "ShowDetails"
#define DP_PROP_SHOWEMPTY           "ShowEmpty"
#define DP_PROP_SUBTOTALS           "SubTotals"
#define DP_PROP_USEDHIERARCHY       "UsedHierarchy"
#define DP_PROP_FILTER              "Filter"
#define DP_PROP_POSITION            "Position"

// -----------------------------------------------------------------------

void lcl_SetBoolProperty( const uno::Reference<beans::XPropertySet>& xProp,
                            const rtl::OUString& rName, sal_Bool bValue )
{
    //! move to ScUnoHelpFunctions?

    xProp->setPropertyValue( rName, uno::Any( &bValue, getBooleanCppuType() ) );
}

// -----------------------------------------------------------------------

void lcl_SkipExtra( SvStream& rStream )
{
    USHORT nExtra;
    rStream >> nExtra;
    if ( nExtra )
    {
        rStream.SeekRel( nExtra );
        if ( rStream.GetError() == SVSTREAM_OK )
            rStream.SetError( SCWARN_IMPORT_INFOLOST );
    }
}

// -----------------------------------------------------------------------

ScDPSaveMember::ScDPSaveMember(const String& rName) :
    aName( rName ),
    nVisibleMode( SC_DPSAVEMODE_DONTKNOW ),
    nShowDetailsMode( SC_DPSAVEMODE_DONTKNOW )
{
}

ScDPSaveMember::ScDPSaveMember(const ScDPSaveMember& r) :
    aName( r.aName ),
    nVisibleMode( r.nVisibleMode ),
    nShowDetailsMode( r.nShowDetailsMode )
{
}

ScDPSaveMember::~ScDPSaveMember()
{
}

BOOL ScDPSaveMember::operator== ( const ScDPSaveMember& r ) const
{
    if ( aName            != r.aName            ||
         nVisibleMode     != r.nVisibleMode     ||
         nShowDetailsMode != r.nShowDetailsMode )
        return FALSE;

    return TRUE;
}

BOOL ScDPSaveMember::HasIsVisible() const
{
    return nVisibleMode != SC_DPSAVEMODE_DONTKNOW;
}

void ScDPSaveMember::SetIsVisible(BOOL bSet)
{
    nVisibleMode = bSet;
}

BOOL ScDPSaveMember::HasShowDetails() const
{
    return nShowDetailsMode != SC_DPSAVEMODE_DONTKNOW;
}

void ScDPSaveMember::SetShowDetails(BOOL bSet)
{
    nShowDetailsMode = bSet;
}

void ScDPSaveMember::SetName( const String& rNew )
{
    // Used only if the source member was renamed (groups).
    // For UI renaming of members, a layout name must be used.

    aName = rNew;
}

void ScDPSaveMember::WriteToSource( const uno::Reference<uno::XInterface>& xMember, sal_Int32 nPosition )
{
    //  nothing to do?
    if ( nVisibleMode == SC_DPSAVEMODE_DONTKNOW && nShowDetailsMode == SC_DPSAVEMODE_DONTKNOW && nPosition < 0 )
        return;

    uno::Reference<beans::XPropertySet> xMembProp( xMember, uno::UNO_QUERY );
    DBG_ASSERT( xMembProp.is(), "no properties at member" );
    if ( xMembProp.is() )
    {
        // exceptions are caught at ScDPSaveData::WriteToSource

        if ( nVisibleMode != SC_DPSAVEMODE_DONTKNOW )
            lcl_SetBoolProperty( xMembProp,
                    rtl::OUString::createFromAscii(DP_PROP_ISVISIBLE), (BOOL)nVisibleMode );

        if ( nShowDetailsMode != SC_DPSAVEMODE_DONTKNOW )
            lcl_SetBoolProperty( xMembProp,
                    rtl::OUString::createFromAscii(DP_PROP_SHOWDETAILS), (BOOL)nShowDetailsMode );

        if ( nPosition >= 0 )
        {
            try
            {
                xMembProp->setPropertyValue( rtl::OUString::createFromAscii(DP_PROP_POSITION), uno::Any(nPosition) );
            }
            catch ( uno::Exception& )
            {
                // position is optional - exception must be ignored
            }
        }
    }
}

// -----------------------------------------------------------------------

ScDPSaveDimension::ScDPSaveDimension(const String& rName, BOOL bDataLayout) :
    aName( rName ),
    pLayoutName( NULL ),
    pSelectedPage( NULL ),
    bIsDataLayout( bDataLayout ),
    bDupFlag( FALSE ),
    nOrientation( sheet::DataPilotFieldOrientation_HIDDEN ),
    nFunction( sheet::GeneralFunction_AUTO ),
    nUsedHierarchy( -1 ),
    nShowEmptyMode( SC_DPSAVEMODE_DONTKNOW ),
    bSubTotalDefault( TRUE ),
    nSubTotalCount( 0 ),
    pSubTotalFuncs( NULL ),
    pReferenceValue( NULL ),
    pSortInfo( NULL ),
    pAutoShowInfo( NULL ),
    pLayoutInfo( NULL )
{
}

ScDPSaveDimension::ScDPSaveDimension(const ScDPSaveDimension& r) :
    aName( r.aName ),
    bIsDataLayout( r.bIsDataLayout ),
    bDupFlag( r.bDupFlag ),
    nOrientation( r.nOrientation ),
    nFunction( r.nFunction ),
    nUsedHierarchy( r.nUsedHierarchy ),
    nShowEmptyMode( r.nShowEmptyMode ),
    bSubTotalDefault( r.bSubTotalDefault ),
    nSubTotalCount( r.nSubTotalCount ),
    pSubTotalFuncs( NULL )
{
    if ( nSubTotalCount && r.pSubTotalFuncs )
    {
        pSubTotalFuncs = new USHORT[nSubTotalCount];
        for (long nSub=0; nSub<nSubTotalCount; nSub++)
            pSubTotalFuncs[nSub] = r.pSubTotalFuncs[nSub];
    }

    for (MemberList::const_iterator i=r.maMemberList.begin(); i != r.maMemberList.end() ; i++)
    {
        const String& rName =  (*i)->GetName();
        ScDPSaveMember* pNew = new ScDPSaveMember( **i );
        maMemberHash[rName] = pNew;
        maMemberList.push_back( pNew );
    }
    if (r.pReferenceValue)
        pReferenceValue = new sheet::DataPilotFieldReference( *(r.pReferenceValue) );
    else
        pReferenceValue = NULL;
    if (r.pSortInfo)
        pSortInfo = new sheet::DataPilotFieldSortInfo( *(r.pSortInfo) );
    else
        pSortInfo = NULL;
    if (r.pAutoShowInfo)
        pAutoShowInfo = new sheet::DataPilotFieldAutoShowInfo( *(r.pAutoShowInfo) );
    else
        pAutoShowInfo = NULL;
    if (r.pLayoutInfo)
        pLayoutInfo = new sheet::DataPilotFieldLayoutInfo( *(r.pLayoutInfo) );
    else
        pLayoutInfo = NULL;
    if (r.pLayoutName)
        pLayoutName = new String( *(r.pLayoutName) );
    else
        pLayoutName = NULL;
    if (r.pSelectedPage)
        pSelectedPage = new String( *(r.pSelectedPage) );
    else
        pSelectedPage = NULL;
}

ScDPSaveDimension::~ScDPSaveDimension()
{
    for (MemberHash::const_iterator i=maMemberHash.begin(); i != maMemberHash.end() ; i++)
        delete i->second;
    delete pReferenceValue;
    delete pSortInfo;
    delete pAutoShowInfo;
    delete pLayoutInfo;
    delete pLayoutName;
    delete pSelectedPage;
    delete [] pSubTotalFuncs;
}

BOOL ScDPSaveDimension::operator== ( const ScDPSaveDimension& r ) const
{
    if ( aName            != r.aName            ||
         bIsDataLayout    != r.bIsDataLayout    ||
         bDupFlag         != r.bDupFlag         ||
         nOrientation     != r.nOrientation     ||
         nFunction        != r.nFunction        ||
         nUsedHierarchy   != r.nUsedHierarchy   ||
         nShowEmptyMode   != r.nShowEmptyMode   ||
         bSubTotalDefault != r.bSubTotalDefault ||
         nSubTotalCount   != r.nSubTotalCount )
        return FALSE;

    if ( nSubTotalCount && ( !pSubTotalFuncs || !r.pSubTotalFuncs ) )   // should not happen
        return FALSE;

    long i;
    for (i=0; i<nSubTotalCount; i++)
        if ( pSubTotalFuncs[i] != r.pSubTotalFuncs[i] )
            return FALSE;

    if (maMemberHash.size() != r.maMemberHash.size() )
        return FALSE;

    MemberList::const_iterator a=maMemberList.begin();
    MemberList::const_iterator b=r.maMemberList.begin();
    for (; a != maMemberList.end() ; ++a, ++b)
        if (!(**a == **b))
            return FALSE;

    return TRUE;
}

void ScDPSaveDimension::AddMember(ScDPSaveMember* pMember)
{
    const String & rName =  pMember->GetName();
    MemberHash::iterator aExisting = maMemberHash.find( rName );
    if ( aExisting == maMemberHash.end() )
    {
        std::pair< const String, ScDPSaveMember *> key( rName, pMember );
        maMemberHash.insert ( key );
    }
    else
    {
        maMemberList.remove( aExisting->second );
        delete aExisting->second;
        aExisting->second = pMember;
    }
    maMemberList.push_back( pMember );
}

void ScDPSaveDimension::SetName( const String& rNew )
{
    // Used only if the source dim was renamed (groups).
    // For UI renaming of dimensions, the layout name must be used.

    aName = rNew;
}

void ScDPSaveDimension::SetOrientation(USHORT nNew)
{
    nOrientation = nNew;
}

void ScDPSaveDimension::SetSubTotals(BOOL bSet)
{
    if (bSet)
    {
        USHORT nFunc = sheet::GeneralFunction_AUTO;
        SetSubTotals( 1, &nFunc );
    }
    else
        SetSubTotals( 0, NULL );
}

void ScDPSaveDimension::SetSubTotals(long nCount, const USHORT* pFuncs)
{
    if (pSubTotalFuncs)
        delete [] pSubTotalFuncs;
    nSubTotalCount = nCount;
    if ( nCount && pFuncs )
    {
        pSubTotalFuncs = new USHORT[nCount];
        for (long i=0; i<nCount; i++)
            pSubTotalFuncs[i] = pFuncs[i];
    }
    else
        pSubTotalFuncs = NULL;

    bSubTotalDefault = FALSE;
}

void ScDPSaveDimension::SetShowEmpty(BOOL bSet)
{
    nShowEmptyMode = bSet;
}

void ScDPSaveDimension::SetFunction(USHORT nNew)
{
    nFunction = nNew;
}

void ScDPSaveDimension::SetUsedHierarchy(long nNew)
{
    nUsedHierarchy = nNew;
}

BOOL ScDPSaveDimension::HasLayoutName() const
{
    return ( pLayoutName != NULL );
}

void ScDPSaveDimension::SetLayoutName(const String* pName)
{
    delete pLayoutName;
    if (pName)
        pLayoutName = new String( *pName );
    else
        pLayoutName = NULL;
}

const String& ScDPSaveDimension::GetLayoutName() const
{
    if (pLayoutName)
        return *pLayoutName;
    return aName;
}

void ScDPSaveDimension::SetReferenceValue(const sheet::DataPilotFieldReference* pNew)
{
    delete pReferenceValue;
    if (pNew)
        pReferenceValue = new sheet::DataPilotFieldReference(*pNew);
    else
        pReferenceValue = NULL;
}

void ScDPSaveDimension::SetSortInfo(const sheet::DataPilotFieldSortInfo* pNew)
{
    delete pSortInfo;
    if (pNew)
        pSortInfo = new sheet::DataPilotFieldSortInfo(*pNew);
    else
        pSortInfo = NULL;
}

void ScDPSaveDimension::SetAutoShowInfo(const sheet::DataPilotFieldAutoShowInfo* pNew)
{
    delete pAutoShowInfo;
    if (pNew)
        pAutoShowInfo = new sheet::DataPilotFieldAutoShowInfo(*pNew);
    else
        pAutoShowInfo = NULL;
}

void ScDPSaveDimension::SetLayoutInfo(const sheet::DataPilotFieldLayoutInfo* pNew)
{
    delete pLayoutInfo;
    if (pNew)
        pLayoutInfo = new sheet::DataPilotFieldLayoutInfo(*pNew);
    else
        pLayoutInfo = NULL;
}

void ScDPSaveDimension::SetCurrentPage( const String* pPage )
{
    delete pSelectedPage;
    if (pPage)
        pSelectedPage = new String( *pPage );
    else
        pSelectedPage = NULL;
}

BOOL ScDPSaveDimension::HasCurrentPage() const
{
    return ( pSelectedPage != NULL );
}

const String& ScDPSaveDimension::GetCurrentPage() const
{
    if (pSelectedPage)
        return *pSelectedPage;
    return EMPTY_STRING;
}

ScDPSaveMember* ScDPSaveDimension::GetExistingMemberByName(const String& rName)
{
    MemberHash::const_iterator res = maMemberHash.find (rName);
    if (res != maMemberHash.end())
        return res->second;
    return NULL;
}


ScDPSaveMember* ScDPSaveDimension::GetMemberByName(const String& rName)
{
    MemberHash::const_iterator res = maMemberHash.find (rName);
    if (res != maMemberHash.end())
        return res->second;

    ScDPSaveMember* pNew = new ScDPSaveMember( rName );
    maMemberHash[rName] = pNew;
    maMemberList.push_back( pNew );
    return pNew;
}

void ScDPSaveDimension::SetMemberPosition( const String& rName, sal_Int32 nNewPos )
{
    ScDPSaveMember* pMember = GetMemberByName( rName );     // make sure it exists and is in the hash

    maMemberList.remove( pMember );

    MemberList::iterator aIter = maMemberList.begin();
    for (sal_Int32 i=0; i<nNewPos && aIter != maMemberList.end(); i++)
        ++aIter;
    maMemberList.insert( aIter, pMember );
}

void ScDPSaveDimension::WriteToSource( const uno::Reference<uno::XInterface>& xDim )
{
    uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
    DBG_ASSERT( xDimProp.is(), "no properties at dimension" );
    if ( xDimProp.is() )
    {
        // exceptions are caught at ScDPSaveData::WriteToSource
        uno::Any aAny;

        sheet::DataPilotFieldOrientation eOrient = (sheet::DataPilotFieldOrientation)nOrientation;
        aAny <<= eOrient;
        xDimProp->setPropertyValue( rtl::OUString::createFromAscii(DP_PROP_ORIENTATION), aAny );

        sheet::GeneralFunction eFunc = (sheet::GeneralFunction)nFunction;
        aAny <<= eFunc;
        xDimProp->setPropertyValue( rtl::OUString::createFromAscii(DP_PROP_FUNCTION), aAny );

        if ( nUsedHierarchy >= 0 )
        {
            aAny <<= (INT32)nUsedHierarchy;
            xDimProp->setPropertyValue( rtl::OUString::createFromAscii(DP_PROP_USEDHIERARCHY), aAny );
        }

        if ( pReferenceValue )
        {
            aAny <<= *pReferenceValue;
            xDimProp->setPropertyValue( rtl::OUString::createFromAscii(SC_UNO_REFVALUE), aAny );
        }

        uno::Sequence<sheet::TableFilterField> aFilter;
        // set the selected page field only if the dimension is used as page dimension
        if ( pSelectedPage && nOrientation == sheet::DataPilotFieldOrientation_PAGE )
        {
            // single filter field: first field equal to selected string
            sheet::TableFilterField aField( sheet::FilterConnection_AND, 0,
                    sheet::FilterOperator_EQUAL, sal_False, 0.0, *pSelectedPage );
            aFilter = uno::Sequence<sheet::TableFilterField>( &aField, 1 );
        }
        // else keep empty sequence
        try
        {
            aAny <<= aFilter;
            xDimProp->setPropertyValue( rtl::OUString::createFromAscii(DP_PROP_FILTER), aAny );
        }
        catch ( beans::UnknownPropertyException& )
        {
            // recent addition - allow source to not handle it (no error)
        }
    }

    //  Level loop outside of maMemberList loop
    //  because SubTotals have to be set independently of known members

    long nCount = maMemberHash.size();

    long nHierCount = 0;
    uno::Reference<container::XIndexAccess> xHiers;
    uno::Reference<sheet::XHierarchiesSupplier> xHierSupp( xDim, uno::UNO_QUERY );
    if ( xHierSupp.is() )
    {
        uno::Reference<container::XNameAccess> xHiersName = xHierSupp->getHierarchies();
        xHiers = new ScNameToIndexAccess( xHiersName );
        nHierCount = xHiers->getCount();
    }

    for (long nHier=0; nHier<nHierCount; nHier++)
    {
        uno::Reference<uno::XInterface> xHierarchy = ScUnoHelpFunctions::AnyToInterface( xHiers->getByIndex(nHier) );

        long nLevCount = 0;
        uno::Reference<container::XIndexAccess> xLevels;
        uno::Reference<sheet::XLevelsSupplier> xLevSupp( xHierarchy, uno::UNO_QUERY );
        if ( xLevSupp.is() )
        {
            uno::Reference<container::XNameAccess> xLevelsName = xLevSupp->getLevels();
            xLevels = new ScNameToIndexAccess( xLevelsName );
            nLevCount = xLevels->getCount();
        }

        for (long nLev=0; nLev<nLevCount; nLev++)
        {
            uno::Reference<uno::XInterface> xLevel = ScUnoHelpFunctions::AnyToInterface( xLevels->getByIndex(nLev) );
            uno::Reference<beans::XPropertySet> xLevProp( xLevel, uno::UNO_QUERY );
            DBG_ASSERT( xLevProp.is(), "no properties at level" );
            if ( xLevProp.is() )
            {
                uno::Any aAny;
                if ( !bSubTotalDefault )
                {
                    if ( !pSubTotalFuncs )
                        nSubTotalCount = 0;

                    uno::Sequence<sheet::GeneralFunction> aSeq(nSubTotalCount);
                    sheet::GeneralFunction* pArray = aSeq.getArray();
                    for (long i=0; i<nSubTotalCount; i++)
                        pArray[i] = (sheet::GeneralFunction)pSubTotalFuncs[i];
                    aAny <<= aSeq;
                    xLevProp->setPropertyValue( rtl::OUString::createFromAscii(DP_PROP_SUBTOTALS), aAny );
                }
                if ( nShowEmptyMode != SC_DPSAVEMODE_DONTKNOW )
                    lcl_SetBoolProperty( xLevProp,
                        rtl::OUString::createFromAscii(DP_PROP_SHOWEMPTY), (BOOL)nShowEmptyMode );

                if ( pSortInfo )
                {
                    aAny <<= *pSortInfo;
                    try
                    {
                        xLevProp->setPropertyValue( rtl::OUString::createFromAscii(SC_UNO_SORTING), aAny );
                    }
                    catch ( beans::UnknownPropertyException& )
                    {
                        // recent addition - allow source to not handle it (no error)
                    }
                }
                if ( pAutoShowInfo )
                {
                    aAny <<= *pAutoShowInfo;
                    try
                    {
                        xLevProp->setPropertyValue( rtl::OUString::createFromAscii(SC_UNO_AUTOSHOW), aAny );
                    }
                    catch ( beans::UnknownPropertyException& )
                    {
                        // recent addition - allow source to not handle it (no error)
                    }
                }
                if ( pLayoutInfo )
                {
                    aAny <<= *pLayoutInfo;
                    try
                    {
                        xLevProp->setPropertyValue( rtl::OUString::createFromAscii(SC_UNO_LAYOUT), aAny );
                    }
                    catch ( beans::UnknownPropertyException& )
                    {
                        // recent addition - allow source to not handle it (no error)
                    }
                }

                // exceptions are caught at ScDPSaveData::WriteToSource
            }

            if ( nCount > 0 )
            {
                uno::Reference<sheet::XMembersSupplier> xMembSupp( xLevel, uno::UNO_QUERY );
                if ( xMembSupp.is() )
                {
                    uno::Reference<container::XNameAccess> xMembers = xMembSupp->getMembers();
                    if ( xMembers.is() )
                    {
                        sal_Int32 nPosition = -1;           // set position only in manual mode
                        if ( !pSortInfo || pSortInfo->Mode == sheet::DataPilotFieldSortMode::MANUAL )
                            nPosition = 0;

                        for (MemberList::const_iterator i=maMemberList.begin(); i != maMemberList.end() ; i++)
                        {
                            rtl::OUString aMemberName = (*i)->GetName();
                            if ( xMembers->hasByName( aMemberName ) )
                            {
                                uno::Reference<uno::XInterface> xMemberInt = ScUnoHelpFunctions::AnyToInterface(
                                    xMembers->getByName( aMemberName ) );
                                (*i)->WriteToSource( xMemberInt, nPosition );

                                if ( nPosition >= 0 )
                                    ++nPosition;            // increase if initialized
                            }
                            // missing member is no error
                        }
                    }
                }
            }
        }
    }
}

// -----------------------------------------------------------------------

ScDPSaveData::ScDPSaveData() :
    pDimensionData( NULL ),
    nColumnGrandMode( SC_DPSAVEMODE_DONTKNOW ),
    nRowGrandMode( SC_DPSAVEMODE_DONTKNOW ),
    nIgnoreEmptyMode( SC_DPSAVEMODE_DONTKNOW ),
    nRepeatEmptyMode( SC_DPSAVEMODE_DONTKNOW ),
    bFilterButton( TRUE ),
    bDrillDown( TRUE )
{
}

ScDPSaveData::ScDPSaveData(const ScDPSaveData& r) :
    nColumnGrandMode( r.nColumnGrandMode ),
    nRowGrandMode( r.nRowGrandMode ),
    nIgnoreEmptyMode( r.nIgnoreEmptyMode ),
    nRepeatEmptyMode( r.nRepeatEmptyMode ),
    bFilterButton( r.bFilterButton ),
    bDrillDown( r.bDrillDown )
{
    if ( r.pDimensionData )
        pDimensionData = new ScDPDimensionSaveData( *r.pDimensionData );
    else
        pDimensionData = NULL;

    long nCount = r.aDimList.Count();
    for (long i=0; i<nCount; i++)
    {
        ScDPSaveDimension* pNew = new ScDPSaveDimension( *(ScDPSaveDimension*)r.aDimList.GetObject(i) );
        aDimList.Insert( pNew, LIST_APPEND );
    }
}

ScDPSaveData& ScDPSaveData::operator= ( const ScDPSaveData& r )
{
    if ( &r != this )
    {
        delete pDimensionData;
        if ( r.pDimensionData )
            pDimensionData = new ScDPDimensionSaveData( *r.pDimensionData );
        else
            pDimensionData = NULL;

        nColumnGrandMode = r.nColumnGrandMode;
        nRowGrandMode    = r.nRowGrandMode;
        nIgnoreEmptyMode = r.nIgnoreEmptyMode;
        nRepeatEmptyMode = r.nRepeatEmptyMode;
        bFilterButton    = r.bFilterButton;
        bDrillDown       = r.bDrillDown;

        //  remove old dimensions

        long nCount = aDimList.Count();
        long i;
        for (i=0; i<nCount; i++)
            delete (ScDPSaveDimension*)aDimList.GetObject(i);
        aDimList.Clear();

        //  copy new dimensions

        nCount = r.aDimList.Count();
        for (i=0; i<nCount; i++)
        {
            ScDPSaveDimension* pNew =
                new ScDPSaveDimension( *(ScDPSaveDimension*)r.aDimList.GetObject(i) );
            aDimList.Insert( pNew, LIST_APPEND );
        }
    }
    return *this;
}

BOOL ScDPSaveData::operator== ( const ScDPSaveData& r ) const
{
    if ( nColumnGrandMode != r.nColumnGrandMode ||
         nRowGrandMode    != r.nRowGrandMode    ||
         nIgnoreEmptyMode != r.nIgnoreEmptyMode ||
         nRepeatEmptyMode != r.nRepeatEmptyMode ||
         bFilterButton    != r.bFilterButton    ||
         bDrillDown       != r.bDrillDown )
        return FALSE;

    if ( pDimensionData || r.pDimensionData )
        if ( !pDimensionData || !r.pDimensionData || !( *pDimensionData == *r.pDimensionData ) )
            return FALSE;

    ULONG nCount = aDimList.Count();
    if ( nCount != r.aDimList.Count() )
        return FALSE;

    for (ULONG i=0; i<nCount; i++)
        if ( !( *(ScDPSaveDimension*)aDimList.GetObject(i) ==
                *(ScDPSaveDimension*)r.aDimList.GetObject(i) ) )
            return FALSE;

    return TRUE;
}

ScDPSaveData::~ScDPSaveData()
{
    long nCount = aDimList.Count();
    for (long i=0; i<nCount; i++)
        delete (ScDPSaveDimension*)aDimList.GetObject(i);
    aDimList.Clear();

    delete pDimensionData;
}

ScDPSaveDimension* ScDPSaveData::GetDimensionByName(const String& rName)
{
    long nCount = aDimList.Count();
    for (long i=0; i<nCount; i++)
    {
        ScDPSaveDimension* pDim = (ScDPSaveDimension*)aDimList.GetObject(i);
        if ( pDim->GetName() == rName && !pDim->IsDataLayout() )
            return pDim;
    }
    ScDPSaveDimension* pNew = new ScDPSaveDimension( rName, FALSE );
    aDimList.Insert( pNew, LIST_APPEND );
    return pNew;
}

ScDPSaveDimension* ScDPSaveData::GetExistingDimensionByName(const String& rName)
{
    long nCount = aDimList.Count();
    for (long i=0; i<nCount; i++)
    {
        ScDPSaveDimension* pDim = (ScDPSaveDimension*)aDimList.GetObject(i);
        if ( pDim->GetName() == rName && !pDim->IsDataLayout() )
            return pDim;
    }
    return NULL;        // don't create new
}

ScDPSaveDimension* ScDPSaveData::GetNewDimensionByName(const String& rName)
{
    long nCount = aDimList.Count();
    for (long i=0; i<nCount; i++)
    {
        ScDPSaveDimension* pDim = (ScDPSaveDimension*)aDimList.GetObject(i);
        if ( pDim->GetName() == rName && !pDim->IsDataLayout() )
            return DuplicateDimension(rName);
    }
    ScDPSaveDimension* pNew = new ScDPSaveDimension( rName, FALSE );
    aDimList.Insert( pNew, LIST_APPEND );
    return pNew;
}

ScDPSaveDimension* ScDPSaveData::GetDataLayoutDimension()
{
    long nCount = aDimList.Count();
    for (long i=0; i<nCount; i++)
    {
        ScDPSaveDimension* pDim = (ScDPSaveDimension*)aDimList.GetObject(i);
        if ( pDim->IsDataLayout() )
            return pDim;
    }
    ScDPSaveDimension* pNew = new ScDPSaveDimension( String(), TRUE );
    aDimList.Insert( pNew, LIST_APPEND );
    return pNew;
}

ScDPSaveDimension* ScDPSaveData::DuplicateDimension(const String& rName)
{
    //  always insert new
    //! check if dimension is there?

    ScDPSaveDimension* pOld = GetDimensionByName( rName );
    ScDPSaveDimension* pNew = new ScDPSaveDimension( *pOld );
    pNew->SetDupFlag( TRUE );
    aDimList.Insert( pNew, LIST_APPEND );
    return pNew;
}

void ScDPSaveData::RemoveDimensionByName(const String& rName)
{
    long nCount = aDimList.Count();
    for (long i=0; i<nCount; i++)
    {
        ScDPSaveDimension* pDim = (ScDPSaveDimension*)aDimList.GetObject(i);
        if ( pDim->GetName() == rName && !pDim->IsDataLayout() )
        {
            delete pDim;
            aDimList.Remove(i);
            break;
        }
    }
}

ScDPSaveDimension& ScDPSaveData::DuplicateDimension( const ScDPSaveDimension& rDim )
{
    ScDPSaveDimension* pNew = new ScDPSaveDimension( rDim );
    pNew->SetDupFlag( TRUE );
    aDimList.Insert( pNew, LIST_APPEND );
    return *pNew;
}

ScDPSaveDimension* ScDPSaveData::GetInnermostDimension(USHORT nOrientation)
{
    //  return the innermost dimension for the given orientation,
    //  excluding data layout dimension

    ScDPSaveDimension* pInner = NULL;
    long nCount = aDimList.Count();
    for (long i=0; i<nCount; i++)
    {
        ScDPSaveDimension* pDim = static_cast<ScDPSaveDimension*>(aDimList.GetObject(i));
        if ( pDim->GetOrientation() == nOrientation && !pDim->IsDataLayout() )
            pInner = pDim;
    }
    return pInner;      // the last matching one
}

long ScDPSaveData::GetDataDimensionCount() const
{
    long nDataCount = 0;

    long nCount = aDimList.Count();
    for (long i=0; i<nCount; i++)
    {
        const ScDPSaveDimension* pDim = static_cast<const ScDPSaveDimension*>(aDimList.GetObject(i));
        if ( pDim->GetOrientation() == sheet::DataPilotFieldOrientation_DATA )
            ++nDataCount;
    }

    return nDataCount;
}

void ScDPSaveData::SetPosition( ScDPSaveDimension* pDim, long nNew )
{
    //  position (nNew) is counted within dimensions of the same orientation

    USHORT nOrient = pDim->GetOrientation();

    aDimList.Remove( pDim );
    ULONG nCount = aDimList.Count();        // after remove

    ULONG nInsPos = 0;
    while ( nNew > 0 && nInsPos < nCount )
    {
        if ( ((ScDPSaveDimension*)aDimList.GetObject(nInsPos))->GetOrientation() == nOrient )
            --nNew;
        ++nInsPos;
    }

    aDimList.Insert( pDim, nInsPos );
}

void ScDPSaveData::SetColumnGrand(BOOL bSet)
{
    nColumnGrandMode = bSet;
}

void ScDPSaveData::SetRowGrand(BOOL bSet)
{
    nRowGrandMode = bSet;
}

void ScDPSaveData::SetIgnoreEmptyRows(BOOL bSet)
{
    nIgnoreEmptyMode = bSet;
}

void ScDPSaveData::SetRepeatIfEmpty(BOOL bSet)
{
    nRepeatEmptyMode = bSet;
}

void ScDPSaveData::SetFilterButton(BOOL bSet)
{
    bFilterButton = bSet;
}

void ScDPSaveData::SetDrillDown(BOOL bSet)
{
    bDrillDown = bSet;
}

void lcl_ResetOrient( const uno::Reference<sheet::XDimensionsSupplier>& xSource )
{
    sheet::DataPilotFieldOrientation eOrient = sheet::DataPilotFieldOrientation_HIDDEN;

    uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
    uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
    long nIntCount = xIntDims->getCount();
    for (long nIntDim=0; nIntDim<nIntCount; nIntDim++)
    {
        uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface( xIntDims->getByIndex(nIntDim) );
        uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
        if (xDimProp.is())
        {
            uno::Any aAny;
            aAny <<= eOrient;
            xDimProp->setPropertyValue( rtl::OUString::createFromAscii(DP_PROP_ORIENTATION), aAny );
        }
    }
}

void ScDPSaveData::WriteToSource( const uno::Reference<sheet::XDimensionsSupplier>& xSource )
{
    if (!xSource.is())
        return;

    //  source options must be first!

    uno::Reference<beans::XPropertySet> xSourceProp( xSource, uno::UNO_QUERY );
    DBG_ASSERT( xSourceProp.is(), "no properties at source" );
    if ( xSourceProp.is() )
    {
        //  source options are not available for external sources
        //! use XPropertySetInfo to test for availability?

        try
        {
            if ( nIgnoreEmptyMode != SC_DPSAVEMODE_DONTKNOW )
                lcl_SetBoolProperty( xSourceProp,
                    rtl::OUString::createFromAscii(DP_PROP_IGNOREEMPTY), (BOOL)nIgnoreEmptyMode );
            if ( nRepeatEmptyMode != SC_DPSAVEMODE_DONTKNOW )
                lcl_SetBoolProperty( xSourceProp,
                    rtl::OUString::createFromAscii(DP_PROP_REPEATIFEMPTY), (BOOL)nRepeatEmptyMode );
        }
        catch(uno::Exception&)
        {
            // no error
        }
    }

    // exceptions in the other calls are errors
    try
    {
        //  reset all orientations
        //! "forgetSettings" or similar at source ?????
        //! reset all duplicated dimensions, or reuse them below !!!

        lcl_ResetOrient( xSource );

        long nCount = aDimList.Count();
        for (long i=0; i<nCount; i++)
        {
            ScDPSaveDimension* pDim = (ScDPSaveDimension*)aDimList.GetObject(i);
            rtl::OUString aName = pDim->GetName();
            BOOL bData = pDim->IsDataLayout();

            //! getByName for ScDPSource, including DataLayoutDimension !!!!!!!!

            uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
            uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
            long nIntCount = xIntDims->getCount();
            BOOL bFound = FALSE;
            for (long nIntDim=0; nIntDim<nIntCount && !bFound; nIntDim++)
            {
                uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface( xIntDims->getByIndex(nIntDim) );
                if ( bData )
                {
                    uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
                    if ( xDimProp.is() )
                    {
                        bFound = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                                    rtl::OUString::createFromAscii(DP_PROP_ISDATALAYOUT) );
                        //! error checking -- is "IsDataLayoutDimension" property required??
                    }
                }
                else
                {
                    uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
                    if ( xDimName.is() && xDimName->getName() == aName )
                        bFound = TRUE;
                }

                if ( bFound )
                {
                    if ( pDim->GetDupFlag() )
                    {
                        String aNewName = pDim->GetName();

                        // different name for each duplication of a (real) dimension...
                        for (long j=0; j<=i; j++)   //! Test !!!!!!
                            aNewName += '*';        //! modify name at creation of SaveDimension

                        uno::Reference<util::XCloneable> xCloneable( xIntDim, uno::UNO_QUERY );
                        DBG_ASSERT( xCloneable.is(), "cannot clone dimension" );
                        if (xCloneable.is())
                        {
                            uno::Reference<util::XCloneable> xNew = xCloneable->createClone();
                            uno::Reference<container::XNamed> xNewName( xNew, uno::UNO_QUERY );
                            if (xNewName.is())
                            {
                                xNewName->setName( aNewName );
                                pDim->WriteToSource( xNew );
                            }
                        }
                    }
                    else
                        pDim->WriteToSource( xIntDim );
                }
            }
            DBG_ASSERT(bFound, "WriteToSource: Dimension not found");
        }

        if ( xSourceProp.is() )
        {
            if ( nColumnGrandMode != SC_DPSAVEMODE_DONTKNOW )
                lcl_SetBoolProperty( xSourceProp,
                    rtl::OUString::createFromAscii(DP_PROP_COLUMNGRAND), (BOOL)nColumnGrandMode );
            if ( nRowGrandMode != SC_DPSAVEMODE_DONTKNOW )
                lcl_SetBoolProperty( xSourceProp,
                    rtl::OUString::createFromAscii(DP_PROP_ROWGRAND), (BOOL)nRowGrandMode );
        }
    }
    catch(uno::Exception&)
    {
        DBG_ERROR("exception in WriteToSource");
    }
}

BOOL ScDPSaveData::IsEmpty() const
{
    long nCount = aDimList.Count();
    for (long i=0; i<nCount; i++)
    {
        ScDPSaveDimension* pDim = (ScDPSaveDimension*)aDimList.GetObject(i);
        if ( pDim->GetOrientation() != sheet::DataPilotFieldOrientation_HIDDEN && !pDim->IsDataLayout() )
            return FALSE;
    }
    return TRUE;    // no entries that are not hidden
}

ScDPDimensionSaveData* ScDPSaveData::GetDimensionData()
{
    if (!pDimensionData)
        pDimensionData = new ScDPDimensionSaveData;
    return pDimensionData;
}

void ScDPSaveData::SetDimensionData( const ScDPDimensionSaveData* pNew )
{
    delete pDimensionData;
    if ( pNew )
        pDimensionData = new ScDPDimensionSaveData( *pNew );
    else
        pDimensionData = NULL;
}

