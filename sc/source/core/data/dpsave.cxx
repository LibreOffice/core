/*************************************************************************
 *
 *  $RCSfile: dpsave.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-09 17:25:08 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

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

using namespace com::sun::star;

// -----------------------------------------------------------------------

#define SC_DPSAVEMODE_NO        0
#define SC_DPSAVEMODE_YES       1
#define SC_DPSAVEMODE_DONTKNOW  2

// -----------------------------------------------------------------------

//! move to a header file
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

ScDPSaveMember::ScDPSaveMember(SvStream& rStream)
{
    rStream.ReadByteString( aName, rStream.GetStreamCharSet() );
    rStream >> nVisibleMode;
    rStream >> nShowDetailsMode;

    lcl_SkipExtra( rStream );       // reads at least 1 USHORT
}

void ScDPSaveMember::Store( SvStream& rStream ) const
{
    rStream.WriteByteString( aName, rStream.GetStreamCharSet() );
    rStream << nVisibleMode;
    rStream << nShowDetailsMode;

    rStream << (USHORT) 0;  // nExtra
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

void ScDPSaveMember::SetIsVisible(BOOL bSet)
{
    nVisibleMode = bSet;
}

void ScDPSaveMember::SetShowDetails(BOOL bSet)
{
    nShowDetailsMode = bSet;
}

void ScDPSaveMember::WriteToSource( const uno::Reference<uno::XInterface>& xMember )
{
    //  nothing to do?
    if ( nVisibleMode == SC_DPSAVEMODE_DONTKNOW && nShowDetailsMode == SC_DPSAVEMODE_DONTKNOW )
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
    }
}

// -----------------------------------------------------------------------

ScDPSaveDimension::ScDPSaveDimension(const String& rName, BOOL bDataLayout) :
    aName( rName ),
    bIsDataLayout( bDataLayout ),
    bDupFlag( FALSE ),
    nOrientation( sheet::DataPilotFieldOrientation_HIDDEN ),
    bSubTotalDefault( TRUE ),
    nSubTotalCount( 0 ),
    pSubTotalFuncs( NULL ),
    nShowEmptyMode( SC_DPSAVEMODE_DONTKNOW ),
    nFunction( sheet::GeneralFunction_AUTO ),
    nUsedHierarchy( -1 ),
    pLayoutName( NULL )
{
}

ScDPSaveDimension::ScDPSaveDimension(const ScDPSaveDimension& r) :
    aName( r.aName ),
    bIsDataLayout( r.bIsDataLayout ),
    bDupFlag( r.bDupFlag ),
    nOrientation( r.nOrientation ),
    bSubTotalDefault( r.bSubTotalDefault ),
    nSubTotalCount( r.nSubTotalCount ),
    pSubTotalFuncs( NULL ),
    nShowEmptyMode( r.nShowEmptyMode ),
    nFunction( r.nFunction ),
    nUsedHierarchy( r.nUsedHierarchy )
{
    if ( nSubTotalCount && r.pSubTotalFuncs )
    {
        pSubTotalFuncs = new USHORT[nSubTotalCount];
        for (long nSub=0; nSub<nSubTotalCount; nSub++)
            pSubTotalFuncs[nSub] = r.pSubTotalFuncs[nSub];
    }

    long nCount = r.aMemberList.Count();
    for (long i=0; i<nCount; i++)
    {
        ScDPSaveMember* pNew = new ScDPSaveMember( *(ScDPSaveMember*)r.aMemberList.GetObject(i) );
        aMemberList.Insert( pNew, LIST_APPEND );
    }
    if (r.pLayoutName)
        pLayoutName = new String( *(r.pLayoutName) );
    else
        pLayoutName = NULL;
}

ScDPSaveDimension::ScDPSaveDimension(SvStream& rStream)
{
    long i;

    rStream.ReadByteString( aName, rStream.GetStreamCharSet() );
    rStream >> bIsDataLayout;

    rStream >> bDupFlag;

    rStream >> nOrientation;
    rStream >> nFunction;           // enum GeneralFunction
    rStream >> nUsedHierarchy;

    rStream >> nShowEmptyMode;      //! at level

    rStream >> bSubTotalDefault;    //! at level
    rStream >> nSubTotalCount;
    if (nSubTotalCount)
    {
        pSubTotalFuncs = new USHORT[nSubTotalCount];
        for (i=0; i<nSubTotalCount; i++)
            rStream >> pSubTotalFuncs[i];
    }
    else
        pSubTotalFuncs = NULL;

    lcl_SkipExtra( rStream );       // reads at least 1 USHORT

    long nNewCount;
    rStream >> nNewCount;
    for (i=0; i<nNewCount; i++)
    {
        ScDPSaveMember* pNew = new ScDPSaveMember( rStream );
        aMemberList.Insert( pNew, LIST_APPEND );
    }
    pLayoutName = NULL;
}

void ScDPSaveDimension::Store( SvStream& rStream ) const
{
    long i;

    rStream.WriteByteString( aName, rStream.GetStreamCharSet() );
    rStream << bIsDataLayout;

    rStream << bDupFlag;

    rStream << nOrientation;
    rStream << nFunction;           // enum GeneralFunction
    rStream << nUsedHierarchy;

    rStream << nShowEmptyMode;      //! at level

    //! subtotals at level
    rStream << bSubTotalDefault;
    long nSubCnt = pSubTotalFuncs ? nSubTotalCount : 0;
    rStream << nSubCnt;
    for (i=0; i<nSubCnt; i++)
        rStream << pSubTotalFuncs[i];

    rStream << (USHORT) 0;  // nExtra

    long nCount = aMemberList.Count();
    rStream << nCount;
    for (i=0; i<nCount; i++)
    {
        const ScDPSaveMember* pMember = (const ScDPSaveMember*)aMemberList.GetObject(i);
        pMember->Store( rStream );
    }
}

ScDPSaveDimension::~ScDPSaveDimension()
{
    long nCount = aMemberList.Count();
    for (long i=0; i<nCount; i++)
        delete (ScDPSaveMember*)aMemberList.GetObject(i);
    aMemberList.Clear();
    if (pLayoutName)
        delete pLayoutName;
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

    long nCount = aMemberList.Count();
    if ( nCount != r.aMemberList.Count() )
        return FALSE;

    for (i=0; i<nCount; i++)
        if ( !( *(ScDPSaveMember*)aMemberList.GetObject(i) ==
                *(ScDPSaveMember*)r.aMemberList.GetObject(i) ) )
            return FALSE;

    return TRUE;
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
        delete pSubTotalFuncs;
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

void ScDPSaveDimension::SetLayoutName(const String* pName)
{
    if (pName)
    {
        if (pLayoutName)
            delete pLayoutName;
        pLayoutName = new String( *pName );
    }
}

const String& ScDPSaveDimension::GetLayoutName() const
{
    if (pLayoutName)
        return *pLayoutName;
    return aName;
}

ScDPSaveMember* ScDPSaveDimension::GetMemberByName(const String& rName)
{
    long nCount = aMemberList.Count();
    for (long i=0; i<nCount; i++)
    {
        ScDPSaveMember* pMember = (ScDPSaveMember*)aMemberList.GetObject(i);
        if ( pMember->GetName() == rName )
            return pMember;
    }
    ScDPSaveMember* pNew = new ScDPSaveMember( rName );
    aMemberList.Insert( pNew, LIST_APPEND );
    return pNew;
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
    }

    //  Level loop outside of aMemberList loop
    //  because SubTotals have to be set independently of known members

    long nCount = aMemberList.Count();

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
                if ( !bSubTotalDefault )
                {
                    if ( !pSubTotalFuncs )
                        nSubTotalCount = 0;

                    uno::Sequence<sheet::GeneralFunction> aSeq(nSubTotalCount);
                    sheet::GeneralFunction* pArray = aSeq.getArray();
                    for (long i=0; i<nSubTotalCount; i++)
                        pArray[i] = (sheet::GeneralFunction)pSubTotalFuncs[i];
                    uno::Any aAny;
                    aAny <<= aSeq;
                    xLevProp->setPropertyValue( rtl::OUString::createFromAscii(DP_PROP_SUBTOTALS), aAny );
                }
                if ( nShowEmptyMode != SC_DPSAVEMODE_DONTKNOW )
                    lcl_SetBoolProperty( xLevProp,
                        rtl::OUString::createFromAscii(DP_PROP_SHOWEMPTY), (BOOL)nShowEmptyMode );

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
                        for (long i=0; i<nCount; i++)
                        {
                            ScDPSaveMember* pMember = (ScDPSaveMember*)aMemberList.GetObject(i);
                            rtl::OUString aName = pMember->GetName();
                            if ( xMembers->hasByName( aName ) )
                            {
                                uno::Reference<uno::XInterface> xMemberInt = ScUnoHelpFunctions::AnyToInterface(
                                    xMembers->getByName( aName ) );
                                pMember->WriteToSource( xMemberInt );
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
    nColumnGrandMode( SC_DPSAVEMODE_DONTKNOW ),
    nRowGrandMode( SC_DPSAVEMODE_DONTKNOW ),
    nIgnoreEmptyMode( SC_DPSAVEMODE_DONTKNOW ),
    nRepeatEmptyMode( SC_DPSAVEMODE_DONTKNOW )
{
}

ScDPSaveData::ScDPSaveData(const ScDPSaveData& r) :
    nColumnGrandMode( r.nColumnGrandMode ),
    nRowGrandMode( r.nRowGrandMode ),
    nIgnoreEmptyMode( r.nIgnoreEmptyMode ),
    nRepeatEmptyMode( r.nRepeatEmptyMode )
{
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
        nColumnGrandMode = r.nColumnGrandMode;
        nRowGrandMode    = r.nRowGrandMode;
        nIgnoreEmptyMode = r.nIgnoreEmptyMode;
        nRepeatEmptyMode = r.nRepeatEmptyMode;

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
         nRepeatEmptyMode != r.nRepeatEmptyMode )
        return FALSE;

    long nCount = aDimList.Count();
    if ( nCount != r.aDimList.Count() )
        return FALSE;

    for (long i=0; i<nCount; i++)
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

void ScDPSaveData::Store( SvStream& rStream ) const
{
    //! multi-header for individual entries

    long nCount = aDimList.Count();
    rStream << nCount;
    for (long i=0; i<nCount; i++)
    {
        const ScDPSaveDimension* pDim = (const ScDPSaveDimension*)aDimList.GetObject(i);
        pDim->Store( rStream );
    }

    rStream << nColumnGrandMode;
    rStream << nRowGrandMode;
    rStream << nIgnoreEmptyMode;
    rStream << nRepeatEmptyMode;

    rStream << (USHORT) 0;  // nExtra
}

void ScDPSaveData::Load( SvStream& rStream )
{
    //! multi-header for individual entries

    DBG_ASSERT( aDimList.Count()==0, "ScDPSaveData::Load not empty" );

    long nNewCount;
    rStream >> nNewCount;
    for (long i=0; i<nNewCount; i++)
    {
        ScDPSaveDimension* pNew = new ScDPSaveDimension( rStream );
        aDimList.Insert( pNew, LIST_APPEND );
    }

    rStream >> nColumnGrandMode;
    rStream >> nRowGrandMode;
    rStream >> nIgnoreEmptyMode;
    rStream >> nRepeatEmptyMode;

    lcl_SkipExtra( rStream );       // reads at least 1 USHORT
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



