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
// Wang Xu Ming -- 2009-8-17
// DataPilot Migration - Cache&&Performance
#include "dptabsrc.hxx"
#include "dpglobal.hxx"
using namespace ScDPGlobal;
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDREFERENCETYPE_HPP_
#include <com/sun/star/sheet/DataPilotFieldReferenceType.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDREFERENCEITEMTYPE_HPP_
#include <com/sun/star/sheet/DataPilotFieldReferenceItemType.hpp>
#endif
using namespace com::sun::star::sheet;
// End Comments

#include <hash_map>

using namespace com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;
using ::rtl::OUStringHash;
using ::std::hash_map;
using ::std::auto_ptr;

// -----------------------------------------------------------------------

#define SC_DPSAVEMODE_NO        0
#define SC_DPSAVEMODE_YES       1
#define SC_DPSAVEMODE_DONTKNOW  2

// -----------------------------------------------------------------------

void lcl_SetBoolProperty( const uno::Reference<beans::XPropertySet>& xProp,
                            const rtl::OUString& rName, sal_Bool bValue )
{
    //! move to ScUnoHelpFunctions?

    xProp->setPropertyValue( rName, uno::Any( &bValue, getBooleanCppuType() ) );
}

// -----------------------------------------------------------------------

ScDPSaveMember::ScDPSaveMember(const String& rName) :
    aName( rName ),
    mpLayoutName(NULL),
    nVisibleMode( SC_DPSAVEMODE_DONTKNOW ),
    nShowDetailsMode( SC_DPSAVEMODE_DONTKNOW )
{
}

ScDPSaveMember::ScDPSaveMember(const ScDPSaveMember& r) :
    aName( r.aName ),
    mpLayoutName(NULL),
    nVisibleMode( r.nVisibleMode ),
    nShowDetailsMode( r.nShowDetailsMode )
{
    if (r.mpLayoutName.get())
        mpLayoutName.reset(new OUString(*r.mpLayoutName));
}

ScDPSaveMember::~ScDPSaveMember()
{
}

sal_Bool ScDPSaveMember::operator== ( const ScDPSaveMember& r ) const
{
    if ( aName            != r.aName            ||
         nVisibleMode     != r.nVisibleMode     ||
         nShowDetailsMode != r.nShowDetailsMode )
        return sal_False;

    return sal_True;
}

sal_Bool ScDPSaveMember::HasIsVisible() const
{
    return nVisibleMode != SC_DPSAVEMODE_DONTKNOW;
}

void ScDPSaveMember::SetIsVisible(sal_Bool bSet)
{
    nVisibleMode = bSet;
}

sal_Bool ScDPSaveMember::HasShowDetails() const
{
    return nShowDetailsMode != SC_DPSAVEMODE_DONTKNOW;
}

void ScDPSaveMember::SetShowDetails(sal_Bool bSet)
{
    nShowDetailsMode = bSet;
}

void ScDPSaveMember::SetName( const String& rNew )
{
    // Used only if the source member was renamed (groups).
    // For UI renaming of members, a layout name must be used.

    aName = rNew;
}

void ScDPSaveMember::SetLayoutName( const OUString& rName )
{
    mpLayoutName.reset(new OUString(rName));
}

const OUString* ScDPSaveMember::GetLayoutName() const
{
    return mpLayoutName.get();
}

void ScDPSaveMember::RemoveLayoutName()
{
    mpLayoutName.reset(NULL);
}

void ScDPSaveMember::WriteToSource( const uno::Reference<uno::XInterface>& xMember, sal_Int32 nPosition )
{
    uno::Reference<beans::XPropertySet> xMembProp( xMember, uno::UNO_QUERY );
    DBG_ASSERT( xMembProp.is(), "no properties at member" );
    if ( xMembProp.is() )
    {
        // exceptions are caught at ScDPSaveData::WriteToSource

        if ( nVisibleMode != SC_DPSAVEMODE_DONTKNOW )
            lcl_SetBoolProperty( xMembProp,
                    rtl::OUString::createFromAscii(DP_PROP_ISVISIBLE), (sal_Bool)nVisibleMode );

        if ( nShowDetailsMode != SC_DPSAVEMODE_DONTKNOW )
            lcl_SetBoolProperty( xMembProp,
                    rtl::OUString::createFromAscii(DP_PROP_SHOWDETAILS), (sal_Bool)nShowDetailsMode );

        if (mpLayoutName.get())
            ScUnoHelpFunctions::SetOptionalPropertyValue(xMembProp, SC_UNO_LAYOUTNAME, *mpLayoutName);

        if ( nPosition >= 0 )
            ScUnoHelpFunctions::SetOptionalPropertyValue(xMembProp, DP_PROP_POSITION, nPosition);
    }
}

// -----------------------------------------------------------------------

ScDPSaveDimension::ScDPSaveDimension(const String& rName, sal_Bool bDataLayout) :
    aName( rName ),
    pSelectedPage( NULL ),
    mpLayoutName(NULL),
    mpSubtotalName(NULL),
    bIsDataLayout( bDataLayout ),
    bDupFlag( sal_False ),
    nOrientation( sheet::DataPilotFieldOrientation_HIDDEN ),
    nFunction( sheet::GeneralFunction_AUTO ),
    nUsedHierarchy( -1 ),
    nShowEmptyMode( SC_DPSAVEMODE_DONTKNOW ),
    bSubTotalDefault( sal_True ),
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
    mpLayoutName(NULL),
    mpSubtotalName(NULL),
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
        pSubTotalFuncs = new sal_uInt16[nSubTotalCount];
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
    if (r.pSelectedPage)
        pSelectedPage = new String( *(r.pSelectedPage) );
    else
        pSelectedPage = NULL;
    if (r.mpLayoutName.get())
        mpLayoutName.reset(new OUString(*r.mpLayoutName));
    if (r.mpSubtotalName.get())
        mpSubtotalName.reset(new OUString(*r.mpSubtotalName));
}

ScDPSaveDimension::~ScDPSaveDimension()
{
    for (MemberHash::const_iterator i=maMemberHash.begin(); i != maMemberHash.end() ; i++)
        delete i->second;
    delete pReferenceValue;
    delete pSortInfo;
    delete pAutoShowInfo;
    delete pLayoutInfo;
    delete pSelectedPage;
    delete [] pSubTotalFuncs;
}

sal_Bool ScDPSaveDimension::operator== ( const ScDPSaveDimension& r ) const
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
        return sal_False;

    if ( nSubTotalCount && ( !pSubTotalFuncs || !r.pSubTotalFuncs ) )   // should not happen
        return sal_False;

    long i;
    for (i=0; i<nSubTotalCount; i++)
        if ( pSubTotalFuncs[i] != r.pSubTotalFuncs[i] )
            return sal_False;

    if (maMemberHash.size() != r.maMemberHash.size() )
        return sal_False;

    MemberList::const_iterator a=maMemberList.begin();
    MemberList::const_iterator b=r.maMemberList.begin();
    for (; a != maMemberList.end() ; ++a, ++b)
        if (!(**a == **b))
            return sal_False;

    if ( this->HasCurrentPage() && r.HasCurrentPage() )
    {
        if ( this->GetCurrentPage() != r.GetCurrentPage() )
        {
            return sal_False;
        }
    }
    else if ( this->HasCurrentPage() || r.HasCurrentPage() )
    {
        return sal_False;
    }
    if( pReferenceValue && r.pReferenceValue )
    {
        if ( !(*pReferenceValue == *r.pReferenceValue) )
        {
            return sal_False;
        }
    }
    else if ( pReferenceValue || r.pReferenceValue )
    {
        return sal_False;
    }
    if( this->pSortInfo && r.pSortInfo )
    {
        if ( !(*this->pSortInfo == *r.pSortInfo) )
        {
            return sal_False;
        }
    }
    else if ( this->pSortInfo || r.pSortInfo )
    {
        return sal_False;
    }
    if( this->pAutoShowInfo && r.pAutoShowInfo )
    {
        if ( !(*this->pAutoShowInfo == *r.pAutoShowInfo) )
        {
            return sal_False;
        }
    }
    else if ( this->pAutoShowInfo || r.pAutoShowInfo )
    {
        return sal_False;
    }

    return sal_True;
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

void ScDPSaveDimension::SetOrientation(sal_uInt16 nNew)
{
    nOrientation = nNew;
}

void ScDPSaveDimension::SetSubTotals(long nCount, const sal_uInt16* pFuncs)
{
    if (pSubTotalFuncs)
        delete [] pSubTotalFuncs;
    nSubTotalCount = nCount;
    if ( nCount && pFuncs )
    {
        pSubTotalFuncs = new sal_uInt16[nCount];
        for (long i=0; i<nCount; i++)
            pSubTotalFuncs[i] = pFuncs[i];
    }
    else
        pSubTotalFuncs = NULL;

    bSubTotalDefault = sal_False;
}

bool ScDPSaveDimension::HasShowEmpty() const
{
    return nShowEmptyMode != SC_DPSAVEMODE_DONTKNOW;
}

void ScDPSaveDimension::SetShowEmpty(sal_Bool bSet)
{
    nShowEmptyMode = bSet;
}

void ScDPSaveDimension::SetFunction(sal_uInt16 nNew)
{
    nFunction = nNew;
}

void ScDPSaveDimension::SetUsedHierarchy(long nNew)
{
    nUsedHierarchy = nNew;
}

void ScDPSaveDimension::SetSubtotalName(const OUString& rName)
{
    mpSubtotalName.reset(new OUString(rName));
}

const OUString* ScDPSaveDimension::GetSubtotalName() const
{
    return mpSubtotalName.get();
}

bool ScDPSaveDimension::IsMemberNameInUse(const OUString& rName) const
{
    MemberList::const_iterator itr = maMemberList.begin(), itrEnd = maMemberList.end();
    for (; itr != itrEnd; ++itr)
    {
        const ScDPSaveMember* pMem = *itr;
        if (rName.equalsIgnoreAsciiCase(pMem->GetName()))
            return true;

        const OUString* pLayoutName = pMem->GetLayoutName();
        if (pLayoutName && rName.equalsIgnoreAsciiCase(*pLayoutName))
            return true;
    }
    return false;
}

void ScDPSaveDimension::SetLayoutName(const OUString& rName)
{
    mpLayoutName.reset(new OUString(rName));
}

const OUString* ScDPSaveDimension::GetLayoutName() const
{
    return mpLayoutName.get();
}

void ScDPSaveDimension::RemoveLayoutName()
{
    mpLayoutName.reset(NULL);
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

sal_Bool ScDPSaveDimension::HasCurrentPage() const
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
            aAny <<= (sal_Int32)nUsedHierarchy;
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

        ScUnoHelpFunctions::SetOptionalPropertyValue(xDimProp, DP_PROP_FILTER, aFilter);
        if (mpLayoutName.get())
            ScUnoHelpFunctions::SetOptionalPropertyValue(xDimProp, SC_UNO_LAYOUTNAME, *mpLayoutName);

        const OUString* pSubTotalName = GetSubtotalName();
        if (pSubTotalName)
            // Custom subtotal name, with '?' being replaced by the visible field name later.
            ScUnoHelpFunctions::SetOptionalPropertyValue(xDimProp, SC_UNO_FIELD_SUBTOTALNAME, *pSubTotalName);
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

    sal_Bool bHasHiddenMember = false;

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
                        rtl::OUString::createFromAscii(DP_PROP_SHOWEMPTY), (sal_Bool)nShowEmptyMode );

                if ( pSortInfo )
                    ScUnoHelpFunctions::SetOptionalPropertyValue(xLevProp, SC_UNO_SORTING, *pSortInfo);

                if ( pAutoShowInfo )
                    ScUnoHelpFunctions::SetOptionalPropertyValue(xLevProp, SC_UNO_AUTOSHOW, *pAutoShowInfo);

                if ( pLayoutInfo )
                    ScUnoHelpFunctions::SetOptionalPropertyValue(xLevProp, SC_UNO_LAYOUT, *pLayoutInfo);

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
                            ScDPSaveMember* pMember = *i;
                            if (!pMember->GetIsVisible())
                                bHasHiddenMember = true;
                            rtl::OUString aMemberName = pMember->GetName();
                            if ( xMembers->hasByName( aMemberName ) )
                            {
                                uno::Reference<uno::XInterface> xMemberInt = ScUnoHelpFunctions::AnyToInterface(
                                    xMembers->getByName( aMemberName ) );
                                pMember->WriteToSource( xMemberInt, nPosition );

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

    if (xDimProp.is())
        ScUnoHelpFunctions::SetOptionalPropertyValue(xDimProp, SC_UNO_HAS_HIDDEN_MEMBER, bHasHiddenMember);
}

void ScDPSaveDimension::UpdateMemberVisibility(const hash_map<OUString, bool, OUStringHash>& rData)
{
    typedef hash_map<OUString, bool, OUStringHash> DataMap;
    MemberList::iterator itrMem = maMemberList.begin(), itrMemEnd = maMemberList.end();
    for (; itrMem != itrMemEnd; ++itrMem)
    {
        ScDPSaveMember* pMem = *itrMem;
        const String& rMemName = pMem->GetName();
        DataMap::const_iterator itr = rData.find(rMemName);
        if (itr != rData.end())
            pMem->SetIsVisible(itr->second);
    }
}

bool ScDPSaveDimension::HasInvisibleMember() const
{
    MemberList::const_iterator itrMem = maMemberList.begin(), itrMemEnd = maMemberList.end();
    for (; itrMem != itrMemEnd; ++itrMem)
    {
        const ScDPSaveMember* pMem = *itrMem;
        if (!pMem->GetIsVisible())
            return true;
    }
    return false;
}

// -----------------------------------------------------------------------

ScDPSaveData::ScDPSaveData() :
    pDimensionData( NULL ),
    nColumnGrandMode( SC_DPSAVEMODE_DONTKNOW ),
    nRowGrandMode( SC_DPSAVEMODE_DONTKNOW ),
    nIgnoreEmptyMode( SC_DPSAVEMODE_DONTKNOW ),
    nRepeatEmptyMode( SC_DPSAVEMODE_DONTKNOW ),
    bFilterButton( sal_True ),
    bDrillDown( sal_True ),
    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    mnCacheId( -1),
    // End Comments
    mbDimensionMembersBuilt(false),
    mpGrandTotalName(NULL)
{
}

ScDPSaveData::ScDPSaveData(const ScDPSaveData& r) :
    nColumnGrandMode( r.nColumnGrandMode ),
    nRowGrandMode( r.nRowGrandMode ),
    nIgnoreEmptyMode( r.nIgnoreEmptyMode ),
    nRepeatEmptyMode( r.nRepeatEmptyMode ),
    bFilterButton( r.bFilterButton ),
    bDrillDown( r.bDrillDown ),
    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    mnCacheId(  r.mnCacheId ),
    // End Comments
    mbDimensionMembersBuilt(r.mbDimensionMembersBuilt),
    mpGrandTotalName(NULL)
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

    if (r.mpGrandTotalName.get())
        mpGrandTotalName.reset(new OUString(*r.mpGrandTotalName));
}

ScDPSaveData& ScDPSaveData::operator= ( const ScDPSaveData& r )
{
    if ( &r != this )
    {
        // Wang Xu Ming -- 2009-8-17
        // DataPilot Migration - Cache&&Performance
        this->~ScDPSaveData();
        new( this ) ScDPSaveData ( r );
        // End Comments
    }
    return *this;
}

sal_Bool ScDPSaveData::operator== ( const ScDPSaveData& r ) const
{
    if ( nColumnGrandMode != r.nColumnGrandMode ||
         nRowGrandMode    != r.nRowGrandMode    ||
         nIgnoreEmptyMode != r.nIgnoreEmptyMode ||
         nRepeatEmptyMode != r.nRepeatEmptyMode ||
         bFilterButton    != r.bFilterButton    ||
         mnCacheId        != r.mnCacheId ||/// Wang Xu Ming -- 2009-6-18 DataPilot Migration
         bDrillDown       != r.bDrillDown ||
         mbDimensionMembersBuilt != r.mbDimensionMembersBuilt)
        return sal_False;

    if ( pDimensionData || r.pDimensionData )
        if ( !pDimensionData || !r.pDimensionData || !( *pDimensionData == *r.pDimensionData ) )
            return sal_False;

    sal_uLong nCount = aDimList.Count();
    if ( nCount != r.aDimList.Count() )
        return sal_False;

    for (sal_uLong i=0; i<nCount; i++)
        if ( !( *(ScDPSaveDimension*)aDimList.GetObject(i) ==
                *(ScDPSaveDimension*)r.aDimList.GetObject(i) ) )
            return sal_False;

    if (mpGrandTotalName.get())
    {
        if (!r.mpGrandTotalName.get())
            return false;
        if (!mpGrandTotalName->equals(*r.mpGrandTotalName))
            return false;
    }
    else if (r.mpGrandTotalName.get())
        return false;

    return sal_True;
}

ScDPSaveData::~ScDPSaveData()
{
    long nCount = aDimList.Count();
    for (long i=0; i<nCount; i++)
        delete (ScDPSaveDimension*)aDimList.GetObject(i);
    aDimList.Clear();

    delete pDimensionData;
}

void ScDPSaveData::SetGrandTotalName(const OUString& rName)
{
    mpGrandTotalName.reset(new OUString(rName));
}

const OUString* ScDPSaveData::GetGrandTotalName() const
{
    return mpGrandTotalName.get();
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
    ScDPSaveDimension* pNew = new ScDPSaveDimension( rName, sal_False );
    aDimList.Insert( pNew, LIST_APPEND );
    return pNew;
}

ScDPSaveDimension* ScDPSaveData::GetExistingDimensionByName(const String& rName) const
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
    ScDPSaveDimension* pNew = new ScDPSaveDimension( rName, sal_False );
    aDimList.Insert( pNew, LIST_APPEND );
    return pNew;
}

ScDPSaveDimension* ScDPSaveData::GetDataLayoutDimension()
{
    ScDPSaveDimension* pDim = GetExistingDataLayoutDimension();
    if (pDim)
        return pDim;

    ScDPSaveDimension* pNew = new ScDPSaveDimension( String(), sal_True );
    aDimList.Insert( pNew, LIST_APPEND );
    return pNew;
}

ScDPSaveDimension* ScDPSaveData::GetExistingDataLayoutDimension() const
{
    long nCount = aDimList.Count();
    for (long i=0; i<nCount; i++)
    {
        ScDPSaveDimension* pDim = (ScDPSaveDimension*)aDimList.GetObject(i);
        if ( pDim->IsDataLayout() )
            return pDim;
    }
    return NULL;
}

ScDPSaveDimension* ScDPSaveData::DuplicateDimension(const String& rName)
{
    //  always insert new
    //! check if dimension is there?

    ScDPSaveDimension* pOld = GetDimensionByName( rName );
    ScDPSaveDimension* pNew = new ScDPSaveDimension( *pOld );
    pNew->SetDupFlag( sal_True );
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
    pNew->SetDupFlag( sal_True );
    aDimList.Insert( pNew, LIST_APPEND );
    return *pNew;
}

ScDPSaveDimension* ScDPSaveData::GetInnermostDimension(sal_uInt16 nOrientation)
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

ScDPSaveDimension* ScDPSaveData::GetFirstDimension(sheet::DataPilotFieldOrientation eOrientation)
{
    long nCount = aDimList.Count();
    for (long i = 0; i < nCount; ++i)
    {
        ScDPSaveDimension* pDim = static_cast<ScDPSaveDimension*>(aDimList.GetObject(i));
        if (pDim->GetOrientation() == eOrientation && !pDim->IsDataLayout())
            return pDim;
    }
    return NULL;
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

    sal_uInt16 nOrient = pDim->GetOrientation();

    aDimList.Remove( pDim );
    sal_uLong nCount = aDimList.Count();        // after remove

    sal_uLong nInsPos = 0;
    while ( nNew > 0 && nInsPos < nCount )
    {
        if ( ((ScDPSaveDimension*)aDimList.GetObject(nInsPos))->GetOrientation() == nOrient )
            --nNew;
        ++nInsPos;
    }

    aDimList.Insert( pDim, nInsPos );
}

void ScDPSaveData::SetColumnGrand(sal_Bool bSet)
{
    nColumnGrandMode = bSet;
}

void ScDPSaveData::SetRowGrand(sal_Bool bSet)
{
    nRowGrandMode = bSet;
}

void ScDPSaveData::SetIgnoreEmptyRows(sal_Bool bSet)
{
    nIgnoreEmptyMode = bSet;
}

void ScDPSaveData::SetRepeatIfEmpty(sal_Bool bSet)
{
    nRepeatEmptyMode = bSet;
}

void ScDPSaveData::SetFilterButton(sal_Bool bSet)
{
    bFilterButton = bSet;
}

void ScDPSaveData::SetDrillDown(sal_Bool bSet)
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
                    rtl::OUString::createFromAscii(DP_PROP_IGNOREEMPTY), (sal_Bool)nIgnoreEmptyMode );
            if ( nRepeatEmptyMode != SC_DPSAVEMODE_DONTKNOW )
                lcl_SetBoolProperty( xSourceProp,
                    rtl::OUString::createFromAscii(DP_PROP_REPEATIFEMPTY), (sal_Bool)nRepeatEmptyMode );
        }
        catch(uno::Exception&)
        {
            // no error
        }

        const OUString* pGrandTotalName = GetGrandTotalName();
        if (pGrandTotalName)
            ScUnoHelpFunctions::SetOptionalPropertyValue(xSourceProp, SC_UNO_GRANDTOTAL_NAME, *pGrandTotalName);
    }

    // exceptions in the other calls are errors
    try
    {
        //  reset all orientations
        //! "forgetSettings" or similar at source ?????
        //! reset all duplicated dimensions, or reuse them below !!!
        DBG_TRACE( "ScDPSaveData::WriteToSource" );

        lcl_ResetOrient( xSource );

        long nCount = aDimList.Count();
        for (long i=0; i<nCount; i++)
        {
            ScDPSaveDimension* pDim = (ScDPSaveDimension*)aDimList.GetObject(i);
            rtl::OUString aName = pDim->GetName();

            DBG_TRACESTR(pDim->GetName());

            sal_Bool bData = pDim->IsDataLayout();

            //! getByName for ScDPSource, including DataLayoutDimension !!!!!!!!

            uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
            uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
            long nIntCount = xIntDims->getCount();
            sal_Bool bFound = sal_False;
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
                        bFound = sal_True;
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
                    rtl::OUString::createFromAscii(DP_PROP_COLUMNGRAND), (sal_Bool)nColumnGrandMode );
            if ( nRowGrandMode != SC_DPSAVEMODE_DONTKNOW )
                lcl_SetBoolProperty( xSourceProp,
                    rtl::OUString::createFromAscii(DP_PROP_ROWGRAND), (sal_Bool)nRowGrandMode );
        }
    }
    catch(uno::Exception&)
    {
        DBG_ERROR("exception in WriteToSource");
    }
}

sal_Bool ScDPSaveData::IsEmpty() const
{
    long nCount = aDimList.Count();
    for (long i=0; i<nCount; i++)
    {
        ScDPSaveDimension* pDim = (ScDPSaveDimension*)aDimList.GetObject(i);
        if ( pDim->GetOrientation() != sheet::DataPilotFieldOrientation_HIDDEN && !pDim->IsDataLayout() )
            return sal_False;
    }
    return sal_True;    // no entries that are not hidden
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

void ScDPSaveData::BuildAllDimensionMembers(ScDPTableData* pData)
{
    if (mbDimensionMembersBuilt)
        return;

    // First, build a dimension name-to-index map.
    typedef hash_map<OUString, long, ::rtl::OUStringHash> NameIndexMap;
    NameIndexMap aMap;
    long nColCount = pData->GetColumnCount();
    for (long i = 0; i < nColCount; ++i)
        aMap.insert( NameIndexMap::value_type(pData->getDimensionName(i), i));

    NameIndexMap::const_iterator itrEnd = aMap.end();

    sal_uInt32 n = aDimList.Count();
    for (sal_uInt32 i = 0; i < n; ++i)
    {
        ScDPSaveDimension* pDim = static_cast<ScDPSaveDimension*>(aDimList.GetObject(i));
        const String& rDimName = pDim->GetName();
        if (!rDimName.Len())
            // empty dimension name.  It must be data layout.
            continue;

        NameIndexMap::const_iterator itr = aMap.find(rDimName);
        if (itr == itrEnd)
            // dimension name not in the data.  This should never happen!
            continue;

        long nDimIndex = itr->second;
        const std::vector<SCROW>& rMembers = pData->GetColumnEntries(nDimIndex);
        size_t mMemberCount = rMembers.size();
        for (size_t j = 0; j < mMemberCount; ++j)
        {
            const ScDPItemData* pMemberData = pData->GetMemberById( nDimIndex, rMembers[j] );
            String aMemName = pMemberData->GetString();
            if (pDim->GetExistingMemberByName(aMemName))
                // this member instance already exists.  nothing to do.
                continue;

            auto_ptr<ScDPSaveMember> pNewMember(new ScDPSaveMember(aMemName));
            pNewMember->SetIsVisible(true);
            pDim->AddMember(pNewMember.release());
        }
    }

    mbDimensionMembersBuilt = true;
}

bool ScDPSaveData::HasInvisibleMember(const OUString& rDimName) const
{
    ScDPSaveDimension* pDim = GetExistingDimensionByName(rDimName);
    if (!pDim)
        return false;

    return pDim->HasInvisibleMember();
}

void ScDPSaveData::Refresh( const uno::Reference<sheet::XDimensionsSupplier>& xSource )
{
    try
    {
        long nCount = aDimList.Count();
        std::list<String> deletedDims;
        for (long i=nCount-1; i >=0 ; i--)
        {
            ScDPSaveDimension* pDim = (ScDPSaveDimension*)aDimList.GetObject(i);

            rtl::OUString aName = pDim->GetName();
            if ( pDim->IsDataLayout() )
                continue;

            uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
            uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
            long nIntCount = xIntDims->getCount();
            sal_Bool bFound = sal_False;
            for (long nIntDim=0; nIntDim<nIntCount && !bFound; nIntDim++)
            {
                uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface( xIntDims->getByIndex(nIntDim) );
                uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
                if ( xDimName.is() && xDimName->getName() == aName )
                    bFound = sal_True;
            }
            if ( !bFound )
            {
                deletedDims.push_back( aName );
                aDimList.Remove(i);
                DBG_TRACE( "\n Remove dim: \t" );
                DBG_TRACESTR(  String( aName ) );
            }

        }

        nCount = aDimList.Count();
        for (long  i=nCount-1; i >=0 ; i--) //check every dimension ??
        {
            ScDPSaveDimension* pDim = (ScDPSaveDimension*)aDimList.GetObject(i);

            rtl::OUString aName = pDim->GetName();
            if ( pDim->IsDataLayout() )
                continue;
            pDim->Refresh( xSource, deletedDims );

        }

        mbDimensionMembersBuilt = false;    // there may be new members
    }
    catch(uno::Exception&)
    {
        DBG_ERROR("error in ScDPSaveData::Refresh");
    }

}
void ScDPSaveDimension::Refresh( const com::sun::star::uno::Reference<
                                com::sun::star::sheet::XDimensionsSupplier>& xSource ,
                                const std::list<String>& deletedDims)
{
    if ( xSource.is() )
    {
        ScDPSource* pTabSource = static_cast<ScDPSource*>( xSource.get() );
        ScDPTableDataCache*  pCache = pTabSource->GetCache();
        if ( pCache->GetId() == -1 )
            return;

        SCCOL nSrcDim = pCache->GetDimensionIndex( GetName() );

        if ( nSrcDim == -1 )
            return;
        if ( pSelectedPage )
        {//check pSelected page
            DBG_TRACESTR( (*pSelectedPage) );
            if ( pCache->GetIdByItemData( nSrcDim, *pSelectedPage ) == -1 )
            {
                delete pSelectedPage;
                pSelectedPage = NULL;
            }

        };

        if ( pReferenceValue && pReferenceValue->ReferenceItemType == DataPilotFieldReferenceItemType::NAMED )
        {//check pReferenceValue
#ifdef DEBUG
            switch( pReferenceValue->ReferenceType)
            {
            case sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE: //both
                DBG_TRACE( "\n sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE \n" );
                break;
            case sheet::DataPilotFieldReferenceType::ITEM_DIFFERENCE: //both
                DBG_TRACE( "\n sheet::DataPilotFieldReferenceType::ITEM_DIFFERENCE \n" );
                break;
            case sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE: //both
                DBG_TRACE( "\n sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE \n" );
                break;
            case sheet::DataPilotFieldReferenceType::RUNNING_TOTAL:
                DBG_TRACE( "\n sheet::DataPilotFieldReferenceType::RUNNING_TOTAL \n" ); //enable name
                break;
            }
#endif
            switch( pReferenceValue->ReferenceType)
            {
            case sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE:
            case sheet::DataPilotFieldReferenceType::ITEM_DIFFERENCE:
            case sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE:
            case sheet::DataPilotFieldReferenceType::RUNNING_TOTAL:
                {
                    if( pReferenceValue->ReferenceItemType == DataPilotFieldReferenceItemType::NAMED  )
                    {
                        const String& sReferenceFieldName = pReferenceValue->ReferenceField;
                        DBG_TRACESTR( sReferenceFieldName );
                        SCCOL nRefDim = pCache->GetDimensionIndex( sReferenceFieldName );
                        bool bValid = true;
                        if ( nRefDim == -1 )
                            bValid = false;
                        else if ( pReferenceValue->ReferenceType != sheet::DataPilotFieldReferenceType::RUNNING_TOTAL )
                        { //running total has not reference item
                            const String& sReferenceItemName = pReferenceValue->ReferenceItemName;
                            DBG_TRACESTR( sReferenceItemName );
                            if ( pCache->GetIdByItemData( nRefDim, sReferenceItemName ) == -1 )
                                bValid = false;
                        }
                        if ( !bValid )
                        {
                            delete pReferenceValue;
                            pReferenceValue = NULL;
                        }
                    }
                }
                break;
            }

        };

        if ( pSortInfo )
        { //check sortinfo
            if ( pSortInfo->Mode == DataPilotFieldSortMode::DATA )
            {
                DBG_TRACE( "\n DataPilotFieldSortMode::DATA \n" );
                const String& sFieldDimName = pSortInfo->Field;
                std::list<String>::const_iterator iter = std::find( deletedDims.begin(), deletedDims.end(), sFieldDimName );
                if ( iter != deletedDims.end() && pCache->GetDimensionIndex( sFieldDimName ) == -1  )
                {
                    pSortInfo->Mode = DataPilotFieldSortMode::MANUAL;
                    pSortInfo->Field = GetName();
                }
            }

        };

        if ( pAutoShowInfo )
        { //check autoshow
            const String& sFieldDimName = pAutoShowInfo->DataField;
            std::list<String>::const_iterator iter = std::find( deletedDims.begin(), deletedDims.end(), sFieldDimName );
            if ( iter != deletedDims.end() && pCache->GetDimensionIndex( sFieldDimName ) == -1  )
            {
                delete pAutoShowInfo;
                pAutoShowInfo = NULL;
            }

        };

        //remove unused members
        //SODC_19124
        for (MemberList::iterator i=maMemberList.begin(); i != maMemberList.end() ; )
        {
            rtl::OUString aMemberName = (*i)->GetName();
            if ( pCache->GetIdByItemData( nSrcDim, aMemberName ) == -1 )
                i = maMemberList.erase( i );
            else
                i++;
        }
    }
}
// End Comments
bool operator == (const ::com::sun::star::sheet::DataPilotFieldSortInfo &l, const ::com::sun::star::sheet::DataPilotFieldSortInfo &r )
{
    return l.Field == r.Field && l.IsAscending == r.IsAscending && l.Mode == r.Mode;
}
bool operator == (const ::com::sun::star::sheet::DataPilotFieldAutoShowInfo &l, const ::com::sun::star::sheet::DataPilotFieldAutoShowInfo &r )
{
    return l.IsEnabled == r.IsEnabled &&
        l.ShowItemsMode == r.ShowItemsMode &&
        l.ItemCount == r.ItemCount &&
        l.DataField == r.DataField;
}
bool operator == (const ::com::sun::star::sheet::DataPilotFieldReference &l, const ::com::sun::star::sheet::DataPilotFieldReference &r )
{
    return l.ReferenceType == r.ReferenceType &&
        l.ReferenceField == r.ReferenceField &&
        l.ReferenceItemType == r.ReferenceItemType &&
        l.ReferenceItemName == r.ReferenceItemName;
}

