/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <memory>
#include <dpsave.hxx>
#include <dpdimsave.hxx>
#include <miscuno.hxx>
#include <unonames.hxx>
#include <dputil.hxx>
#include <generalfunction.hxx>
#include <dptabdat.hxx>

#include <sal/types.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <comphelper/stl_types.hxx>

#include <com/sun/star/sheet/XDimensionsSupplier.hpp>
#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>
#include <com/sun/star/sheet/XHierarchiesSupplier.hpp>
#include <com/sun/star/sheet/XLevelsSupplier.hpp>
#include <com/sun/star/sheet/XMembersSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <tools/diagnose_ex.h>

#include <unordered_map>
#include <algorithm>

using namespace com::sun::star;
using namespace com::sun::star::sheet;
using ::std::unique_ptr;

#define SC_DPSAVEMODE_DONTKNOW 2

static void lcl_SetBoolProperty( const uno::Reference<beans::XPropertySet>& xProp,
                            const OUString& rName, bool bValue )
{
    //TODO: move to ScUnoHelpFunctions?

    xProp->setPropertyValue( rName, uno::Any( bValue ) );
}

ScDPSaveMember::ScDPSaveMember(const OUString& rName) :
    aName( rName ),
    nVisibleMode( SC_DPSAVEMODE_DONTKNOW ),
    nShowDetailsMode( SC_DPSAVEMODE_DONTKNOW )
{
}

ScDPSaveMember::ScDPSaveMember(const ScDPSaveMember& r) :
    aName( r.aName ),
    mpLayoutName( r.mpLayoutName ),
    nVisibleMode( r.nVisibleMode ),
    nShowDetailsMode( r.nShowDetailsMode )
{
}

ScDPSaveMember::~ScDPSaveMember()
{
}

bool ScDPSaveMember::operator== ( const ScDPSaveMember& r ) const
{
    return aName            == r.aName           &&
           nVisibleMode     == r.nVisibleMode    &&
           nShowDetailsMode == r.nShowDetailsMode;
}

bool ScDPSaveMember::HasIsVisible() const
{
    return nVisibleMode != SC_DPSAVEMODE_DONTKNOW;
}

void ScDPSaveMember::SetIsVisible(bool bSet)
{
    nVisibleMode = sal_uInt16(bSet);
}

bool ScDPSaveMember::HasShowDetails() const
{
    return nShowDetailsMode != SC_DPSAVEMODE_DONTKNOW;
}

void ScDPSaveMember::SetShowDetails(bool bSet)
{
    nShowDetailsMode = sal_uInt16(bSet);
}

void ScDPSaveMember::SetName( const OUString& rNew )
{
    // Used only if the source member was renamed (groups).
    // For UI renaming of members, a layout name must be used.

    aName = rNew;
}

void ScDPSaveMember::SetLayoutName( const OUString& rName )
{
    mpLayoutName = rName;
}

const boost::optional<OUString> & ScDPSaveMember::GetLayoutName() const
{
    return mpLayoutName;
}

void ScDPSaveMember::RemoveLayoutName()
{
    mpLayoutName.reset();
}

void ScDPSaveMember::WriteToSource( const uno::Reference<uno::XInterface>& xMember, sal_Int32 nPosition )
{
    uno::Reference<beans::XPropertySet> xMembProp( xMember, uno::UNO_QUERY );
    OSL_ENSURE( xMembProp.is(), "no properties at member" );
    if ( xMembProp.is() )
    {
        // exceptions are caught at ScDPSaveData::WriteToSource

        if ( nVisibleMode != SC_DPSAVEMODE_DONTKNOW )
            lcl_SetBoolProperty( xMembProp,
                    SC_UNO_DP_ISVISIBLE, static_cast<bool>(nVisibleMode) );

        if ( nShowDetailsMode != SC_DPSAVEMODE_DONTKNOW )
            lcl_SetBoolProperty( xMembProp,
                    SC_UNO_DP_SHOWDETAILS, static_cast<bool>(nShowDetailsMode) );

        if (mpLayoutName)
            ScUnoHelpFunctions::SetOptionalPropertyValue(xMembProp, SC_UNO_DP_LAYOUTNAME, *mpLayoutName);

        if ( nPosition >= 0 )
            ScUnoHelpFunctions::SetOptionalPropertyValue(xMembProp, SC_UNO_DP_POSITION, nPosition);
    }
}

#if DUMP_PIVOT_TABLE

void ScDPSaveMember::Dump(int nIndent) const
{
    std::string aIndent(nIndent*4, ' ');
    cout << aIndent << "* member name: '" << aName << "'" << endl;

    cout << aIndent << "    + layout name: ";
    if (mpLayoutName)
        cout << "'" << *mpLayoutName << "'";
    else
        cout << "(none)";
    cout << endl;

    cout << aIndent << "    + visibility: ";
    if (nVisibleMode == SC_DPSAVEMODE_DONTKNOW)
        cout << "(unknown)";
    else
        cout << (nVisibleMode ? "visible" : "hidden");
    cout << endl;
}

#endif

ScDPSaveDimension::ScDPSaveDimension(const OUString& rName, bool bDataLayout) :
    aName( rName ),
    bIsDataLayout( bDataLayout ),
    bDupFlag( false ),
    nOrientation( sheet::DataPilotFieldOrientation_HIDDEN ),
    nFunction( ScGeneralFunction::AUTO ),
    nUsedHierarchy( -1 ),
    nShowEmptyMode( SC_DPSAVEMODE_DONTKNOW ),
    bRepeatItemLabels( false ),
    bSubTotalDefault( true )
{
}

ScDPSaveDimension::ScDPSaveDimension(const ScDPSaveDimension& r) :
    aName( r.aName ),
    mpLayoutName( r.mpLayoutName ),
    mpSubtotalName( r.mpSubtotalName ),
    bIsDataLayout( r.bIsDataLayout ),
    bDupFlag( r.bDupFlag ),
    nOrientation( r.nOrientation ),
    nFunction( r.nFunction ),
    nUsedHierarchy( r.nUsedHierarchy ),
    nShowEmptyMode( r.nShowEmptyMode ),
    bRepeatItemLabels( r.bRepeatItemLabels ),
    bSubTotalDefault( r.bSubTotalDefault ),
    maSubTotalFuncs( r.maSubTotalFuncs )
{
    for (const ScDPSaveMember* pMem : r.maMemberList)
    {
        const OUString& rName = pMem->GetName();
        std::unique_ptr<ScDPSaveMember> pNew(new ScDPSaveMember( *pMem ));
        maMemberList.push_back( pNew.get() );
        maMemberHash[rName] = std::move(pNew);
    }
    if (r.pReferenceValue)
        pReferenceValue.reset( new sheet::DataPilotFieldReference( *(r.pReferenceValue) ) );
    if (r.pSortInfo)
        pSortInfo.reset( new sheet::DataPilotFieldSortInfo( *(r.pSortInfo) ) );
    if (r.pAutoShowInfo)
        pAutoShowInfo.reset( new sheet::DataPilotFieldAutoShowInfo( *(r.pAutoShowInfo) ) );
    if (r.pLayoutInfo)
        pLayoutInfo.reset(new sheet::DataPilotFieldLayoutInfo( *(r.pLayoutInfo) ));
}

ScDPSaveDimension::~ScDPSaveDimension()
{
    maMemberHash.clear();
    pReferenceValue.reset();
    pSortInfo.reset();
    pAutoShowInfo.reset();
    pLayoutInfo.reset();
}

bool ScDPSaveDimension::operator== ( const ScDPSaveDimension& r ) const
{
    if ( aName            != r.aName            ||
         bIsDataLayout    != r.bIsDataLayout    ||
         bDupFlag         != r.bDupFlag         ||
         nOrientation     != r.nOrientation     ||
         nFunction        != r.nFunction        ||
         nUsedHierarchy   != r.nUsedHierarchy   ||
         nShowEmptyMode   != r.nShowEmptyMode   ||
         bRepeatItemLabels!= r.bRepeatItemLabels||
         bSubTotalDefault != r.bSubTotalDefault ||
         maSubTotalFuncs  != r.maSubTotalFuncs   )
        return false;

    if (maMemberHash.size() != r.maMemberHash.size() )
        return false;

    if (!std::equal(maMemberList.begin(), maMemberList.end(), r.maMemberList.begin(), r.maMemberList.end(),
                    [](const ScDPSaveMember* a, const ScDPSaveMember* b) { return *a == *b; }))
        return false;

    if( pReferenceValue && r.pReferenceValue )
    {
        if ( *pReferenceValue != *r.pReferenceValue )
        {
            return false;
        }
    }
    else if ( pReferenceValue || r.pReferenceValue )
    {
        return false;
    }
    if( this->pSortInfo && r.pSortInfo )
    {
        if ( *this->pSortInfo != *r.pSortInfo )
        {
            return false;
        }
    }
    else if ( this->pSortInfo || r.pSortInfo )
    {
        return false;
    }
    if( this->pAutoShowInfo && r.pAutoShowInfo )
    {
        if ( *this->pAutoShowInfo != *r.pAutoShowInfo )
        {
            return false;
        }
    }
    else if ( this->pAutoShowInfo || r.pAutoShowInfo )
    {
        return false;
    }

    return true;
}

void ScDPSaveDimension::AddMember(std::unique_ptr<ScDPSaveMember> pMember)
{
    const OUString & rName = pMember->GetName();
    auto aExisting = maMemberHash.find( rName );
    auto tmp = pMember.get();
    if ( aExisting == maMemberHash.end() )
    {
        maMemberHash[rName] = std::move(pMember);
    }
    else
    {
        maMemberList.erase(std::remove(maMemberList.begin(), maMemberList.end(), aExisting->second.get()), maMemberList.end());
        aExisting->second = std::move(pMember);
    }
    maMemberList.push_back( tmp );
}

void ScDPSaveDimension::SetName( const OUString& rNew )
{
    // Used only if the source dim was renamed (groups).
    // For UI renaming of dimensions, the layout name must be used.

    aName = rNew;
}

void ScDPSaveDimension::SetOrientation(css::sheet::DataPilotFieldOrientation nNew)
{
    nOrientation = nNew;
}

void ScDPSaveDimension::SetSubTotals(std::vector<ScGeneralFunction> const & rFuncs)
{
    maSubTotalFuncs = rFuncs;
    bSubTotalDefault = false;
}

bool ScDPSaveDimension::HasShowEmpty() const
{
    return nShowEmptyMode != SC_DPSAVEMODE_DONTKNOW;
}

void ScDPSaveDimension::SetShowEmpty(bool bSet)
{
    nShowEmptyMode = sal_uInt16(bSet);
}

void ScDPSaveDimension::SetRepeatItemLabels(bool bSet)
{
    bRepeatItemLabels = bSet;
}

void ScDPSaveDimension::SetFunction(ScGeneralFunction nNew)
{
    nFunction = nNew;
}

void ScDPSaveDimension::SetUsedHierarchy(long nNew)
{
    nUsedHierarchy = nNew;
}

void ScDPSaveDimension::SetSubtotalName(const OUString& rName)
{
    mpSubtotalName = rName;
}

const boost::optional<OUString> & ScDPSaveDimension::GetSubtotalName() const
{
    return mpSubtotalName;
}

void ScDPSaveDimension::RemoveSubtotalName()
{
    mpSubtotalName.reset();
}

bool ScDPSaveDimension::IsMemberNameInUse(const OUString& rName) const
{
    return std::any_of(maMemberList.begin(), maMemberList.end(), [&rName](const ScDPSaveMember* pMem) {
        if (rName.equalsIgnoreAsciiCase(pMem->GetName()))
            return true;

        const boost::optional<OUString> & pLayoutName = pMem->GetLayoutName();
        return pLayoutName && rName.equalsIgnoreAsciiCase(*pLayoutName);
    });
}

void ScDPSaveDimension::SetLayoutName(const OUString& rName)
{
    mpLayoutName = rName;
}

const boost::optional<OUString> & ScDPSaveDimension::GetLayoutName() const
{
    return mpLayoutName;
}

void ScDPSaveDimension::RemoveLayoutName()
{
    mpLayoutName.reset();
}

void ScDPSaveDimension::SetReferenceValue(const sheet::DataPilotFieldReference* pNew)
{
    if (pNew)
        pReferenceValue.reset( new sheet::DataPilotFieldReference(*pNew) );
    else
        pReferenceValue.reset();
}

void ScDPSaveDimension::SetSortInfo(const sheet::DataPilotFieldSortInfo* pNew)
{
    if (pNew)
        pSortInfo.reset( new sheet::DataPilotFieldSortInfo(*pNew) );
    else
        pSortInfo.reset();
}

void ScDPSaveDimension::SetAutoShowInfo(const sheet::DataPilotFieldAutoShowInfo* pNew)
{
    if (pNew)
        pAutoShowInfo.reset( new sheet::DataPilotFieldAutoShowInfo(*pNew) );
    else
        pAutoShowInfo.reset();
}

void ScDPSaveDimension::SetLayoutInfo(const sheet::DataPilotFieldLayoutInfo* pNew)
{
    if (pNew)
        pLayoutInfo.reset( new sheet::DataPilotFieldLayoutInfo(*pNew) );
    else
        pLayoutInfo.reset();
}

void ScDPSaveDimension::SetCurrentPage( const OUString* pPage )
{
    // We use member's visibility attribute to filter by page dimension.

    // pPage == nullptr -> all members visible.
    for (ScDPSaveMember* pMem : maMemberList)
    {
        bool bVisible = !pPage || pMem->GetName() == *pPage;
        pMem->SetIsVisible(bVisible);
    }
}

OUString ScDPSaveDimension::GetCurrentPage() const
{
    MemberList::const_iterator it = std::find_if(maMemberList.begin(), maMemberList.end(),
        [](const ScDPSaveMember* pMem) { return pMem->GetIsVisible(); });
    if (it != maMemberList.end())
        return (*it)->GetName();

    return OUString();
}

ScDPSaveMember* ScDPSaveDimension::GetExistingMemberByName(const OUString& rName)
{
    auto res = maMemberHash.find (rName);
    if (res != maMemberHash.end())
        return res->second.get();
    return nullptr;
}

ScDPSaveMember* ScDPSaveDimension::GetMemberByName(const OUString& rName)
{
    auto res = maMemberHash.find (rName);
    if (res != maMemberHash.end())
        return res->second.get();

    ScDPSaveMember* pNew = new ScDPSaveMember( rName );
    maMemberHash[rName] = std::unique_ptr<ScDPSaveMember>(pNew);
    maMemberList.push_back( pNew );
    return pNew;
}

void ScDPSaveDimension::SetMemberPosition( const OUString& rName, sal_Int32 nNewPos )
{
    ScDPSaveMember* pMember = GetMemberByName( rName ); // make sure it exists and is in the hash

    maMemberList.erase(std::remove( maMemberList.begin(), maMemberList.end(), pMember), maMemberList.end() );

    maMemberList.insert( maMemberList.begin() + nNewPos, pMember );
}

void ScDPSaveDimension::WriteToSource( const uno::Reference<uno::XInterface>& xDim )
{
    uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
    OSL_ENSURE( xDimProp.is(), "no properties at dimension" );
    if ( xDimProp.is() )
    {
        // exceptions are caught at ScDPSaveData::WriteToSource

        sheet::DataPilotFieldOrientation eOrient = nOrientation;
        xDimProp->setPropertyValue( SC_UNO_DP_ORIENTATION, uno::Any(eOrient) );

        sal_Int16 eFunc = static_cast<sal_Int16>(nFunction);
        xDimProp->setPropertyValue( SC_UNO_DP_FUNCTION2, uno::Any(eFunc) );

        if ( nUsedHierarchy >= 0 )
        {
            xDimProp->setPropertyValue( SC_UNO_DP_USEDHIERARCHY, uno::Any(static_cast<sal_Int32>(nUsedHierarchy)) );
        }

        if ( pReferenceValue )
        {
            ;
            xDimProp->setPropertyValue( SC_UNO_DP_REFVALUE, uno::Any(*pReferenceValue) );
        }

        if (mpLayoutName)
            ScUnoHelpFunctions::SetOptionalPropertyValue(xDimProp, SC_UNO_DP_LAYOUTNAME, *mpLayoutName);

        const boost::optional<OUString> & pSubTotalName = GetSubtotalName();
        if (pSubTotalName)
            // Custom subtotal name, with '?' being replaced by the visible field name later.
            ScUnoHelpFunctions::SetOptionalPropertyValue(xDimProp, SC_UNO_DP_FIELD_SUBTOTALNAME, *pSubTotalName);
    }

    // Level loop outside of maMemberList loop
    // because SubTotals have to be set independently of known members

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

    bool bHasHiddenMember = false;

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
            OSL_ENSURE( xLevProp.is(), "no properties at level" );
            if ( xLevProp.is() )
            {
                if ( !bSubTotalDefault )
                {
                    uno::Sequence<sal_Int16> aSeq(maSubTotalFuncs.size());
                    for(size_t i = 0; i < maSubTotalFuncs.size(); ++i)
                        aSeq.getArray()[i] = static_cast<sal_Int16>(maSubTotalFuncs[i]);
                    xLevProp->setPropertyValue( SC_UNO_DP_SUBTOTAL2, uno::Any(aSeq) );
                }
                if ( nShowEmptyMode != SC_DPSAVEMODE_DONTKNOW )
                    lcl_SetBoolProperty( xLevProp,
                        SC_UNO_DP_SHOWEMPTY, static_cast<bool>(nShowEmptyMode) );

                lcl_SetBoolProperty( xLevProp,
                    SC_UNO_DP_REPEATITEMLABELS, bRepeatItemLabels );

                if ( pSortInfo )
                    ScUnoHelpFunctions::SetOptionalPropertyValue(xLevProp, SC_UNO_DP_SORTING, *pSortInfo);

                if ( pAutoShowInfo )
                    ScUnoHelpFunctions::SetOptionalPropertyValue(xLevProp, SC_UNO_DP_AUTOSHOW, *pAutoShowInfo);

                if ( pLayoutInfo )
                    ScUnoHelpFunctions::SetOptionalPropertyValue(xLevProp, SC_UNO_DP_LAYOUT, *pLayoutInfo);

                // exceptions are caught at ScDPSaveData::WriteToSource
            }

            if ( nCount > 0 )
            {
                uno::Reference<sheet::XMembersSupplier> xMembSupp( xLevel, uno::UNO_QUERY );
                if ( xMembSupp.is() )
                {
                    uno::Reference<sheet::XMembersAccess> xMembers = xMembSupp->getMembers();
                    if ( xMembers.is() )
                    {
                        sal_Int32 nPosition = -1; // set position only in manual mode
                        if ( !pSortInfo || pSortInfo->Mode == sheet::DataPilotFieldSortMode::MANUAL )
                            nPosition = 0;

                        for (ScDPSaveMember* pMember : maMemberList)
                        {
                            if (!pMember->GetIsVisible())
                                bHasHiddenMember = true;
                            OUString aMemberName = pMember->GetName();
                            if ( xMembers->hasByName( aMemberName ) )
                            {
                                uno::Reference<uno::XInterface> xMemberInt = ScUnoHelpFunctions::AnyToInterface(
                                    xMembers->getByName( aMemberName ) );
                                pMember->WriteToSource( xMemberInt, nPosition );

                                if ( nPosition >= 0 )
                                    ++nPosition; // increase if initialized
                            }
                            // missing member is no error
                        }
                    }
                }
            }
        }
    }

    if (xDimProp.is())
        ScUnoHelpFunctions::SetOptionalPropertyValue(xDimProp, SC_UNO_DP_HAS_HIDDEN_MEMBER, bHasHiddenMember);
}

void ScDPSaveDimension::UpdateMemberVisibility(const std::unordered_map<OUString, bool>& rData)
{
    typedef std::unordered_map<OUString, bool> DataMap;
    for (ScDPSaveMember* pMem : maMemberList)
    {
        const OUString& rMemName = pMem->GetName();
        DataMap::const_iterator itr = rData.find(rMemName);
        if (itr != rData.end())
            pMem->SetIsVisible(itr->second);
    }
}

bool ScDPSaveDimension::HasInvisibleMember() const
{
    return std::any_of(maMemberList.begin(), maMemberList.end(),
        [](const ScDPSaveMember* pMem) { return !pMem->GetIsVisible(); });
}

void ScDPSaveDimension::RemoveObsoleteMembers(const MemberSetType& rMembers)
{
    MemberList aNew;
    for (ScDPSaveMember* pMem : maMemberList)
    {
        if (rMembers.count(pMem->GetName()))
        {
            // This member still exists.
            aNew.push_back(pMem);
        }
        else
        {
            maMemberHash.erase(pMem->GetName());
        }
    }

    maMemberList.swap(aNew);
}

#if DUMP_PIVOT_TABLE

void ScDPSaveDimension::Dump(int nIndent) const
{
    static const char* pOrientNames[] = { "hidden", "column", "row", "page", "data" };
    std::string aIndent(nIndent*4, ' ');

    cout << aIndent << "* dimension name: '" << aName << "'" << endl;

    cout << aIndent << "    + orientation: ";
    if (nOrientation <= DataPilotFieldOrientation_DATA)
        cout << pOrientNames[static_cast<int>(nOrientation)];
    else
        cout << "(invalid)";
    cout << endl;

    cout << aIndent << "    + layout name: ";
    if (mpLayoutName)
        cout << "'" << *mpLayoutName << "'";
    else
        cout << "(none)";
    cout << endl;

    cout << aIndent << "    + subtotal name: ";
    if (mpSubtotalName)
        cout << "'" << *mpSubtotalName << "'";
    else
        cout << "(none)";
    cout << endl;

    cout << aIndent << "    + is data layout: " << (bIsDataLayout ? "yes" : "no") << endl;
    cout << aIndent << "    + is duplicate: " << (bDupFlag ? "yes" : "no") << endl;

    for (ScDPSaveMember* pMem : maMemberList)
    {
        pMem->Dump(nIndent+1);
    }

    cout << endl; // blank line
}

#endif

ScDPSaveData::ScDPSaveData() :
    nColumnGrandMode( SC_DPSAVEMODE_DONTKNOW ),
    nRowGrandMode( SC_DPSAVEMODE_DONTKNOW ),
    nIgnoreEmptyMode( SC_DPSAVEMODE_DONTKNOW ),
    nRepeatEmptyMode( SC_DPSAVEMODE_DONTKNOW ),
    bFilterButton( true ),
    bDrillDown( true ),
    mbDimensionMembersBuilt(false)
{
}

ScDPSaveData::ScDPSaveData(const ScDPSaveData& r) :
    nColumnGrandMode( r.nColumnGrandMode ),
    nRowGrandMode( r.nRowGrandMode ),
    nIgnoreEmptyMode( r.nIgnoreEmptyMode ),
    nRepeatEmptyMode( r.nRepeatEmptyMode ),
    bFilterButton( r.bFilterButton ),
    bDrillDown( r.bDrillDown ),
    mbDimensionMembersBuilt(r.mbDimensionMembersBuilt),
    mpGrandTotalName(r.mpGrandTotalName)
{
    if ( r.pDimensionData )
        pDimensionData.reset( new ScDPDimensionSaveData( *r.pDimensionData ) );

    for (auto const& it : r.m_DimList)
    {
        m_DimList.push_back(std::make_unique<ScDPSaveDimension>(*it));
    }
}

ScDPSaveData& ScDPSaveData::operator= ( const ScDPSaveData& r )
{
    if ( &r != this )
    {
        this->~ScDPSaveData();
        new( this ) ScDPSaveData ( r );
    }
    return *this;
}

bool ScDPSaveData::operator== ( const ScDPSaveData& r ) const
{
    if ( nColumnGrandMode != r.nColumnGrandMode ||
         nRowGrandMode    != r.nRowGrandMode    ||
         nIgnoreEmptyMode != r.nIgnoreEmptyMode ||
         nRepeatEmptyMode != r.nRepeatEmptyMode ||
         bFilterButton    != r.bFilterButton    ||
         bDrillDown       != r.bDrillDown       ||
         mbDimensionMembersBuilt != r.mbDimensionMembersBuilt)
        return false;

    if ( pDimensionData || r.pDimensionData )
        if ( !pDimensionData || !r.pDimensionData || !( *pDimensionData == *r.pDimensionData ) )
            return false;

    if (!(::comphelper::ContainerUniquePtrEquals(m_DimList, r.m_DimList)))
        return false;

    if (mpGrandTotalName)
    {
        if (!r.mpGrandTotalName)
            return false;
        if (*mpGrandTotalName != *r.mpGrandTotalName)
            return false;
    }
    else if (r.mpGrandTotalName)
        return false;

    return true;
}

ScDPSaveData::~ScDPSaveData()
{
}

void ScDPSaveData::SetGrandTotalName(const OUString& rName)
{
    mpGrandTotalName = rName;
}

const boost::optional<OUString> & ScDPSaveData::GetGrandTotalName() const
{
    return mpGrandTotalName;
}

namespace {

class DimOrderInserter
{
    ScDPSaveData::DimOrderType& mrNames;
public:
    explicit DimOrderInserter(ScDPSaveData::DimOrderType& rNames) : mrNames(rNames) {}

    void operator() (const ScDPSaveDimension* pDim)
    {
        size_t nRank = mrNames.size();
        mrNames.emplace(pDim->GetName(), nRank);
    }
};

}

const ScDPSaveData::DimOrderType& ScDPSaveData::GetDimensionSortOrder() const
{
    if (!mpDimOrder)
    {
        mpDimOrder.reset(new DimOrderType);
        std::vector<const ScDPSaveDimension*> aRowDims, aColDims;
        GetAllDimensionsByOrientation(sheet::DataPilotFieldOrientation_ROW, aRowDims);
        GetAllDimensionsByOrientation(sheet::DataPilotFieldOrientation_COLUMN, aColDims);

        std::for_each(aRowDims.begin(), aRowDims.end(), DimOrderInserter(*mpDimOrder));
        std::for_each(aColDims.begin(), aColDims.end(), DimOrderInserter(*mpDimOrder));
    }
    return *mpDimOrder;
}

void ScDPSaveData::GetAllDimensionsByOrientation(
    sheet::DataPilotFieldOrientation eOrientation, std::vector<const ScDPSaveDimension*>& rDims) const
{
    std::vector<const ScDPSaveDimension*> aDims;
    for (auto const& it : m_DimList)
    {
        const ScDPSaveDimension& rDim = *it;
        if (rDim.GetOrientation() != eOrientation)
            continue;

        aDims.push_back(&rDim);
    }

    rDims.swap(aDims);
}

void ScDPSaveData::AddDimension(ScDPSaveDimension* pDim)
{
    if (!pDim)
        return;

    CheckDuplicateName(*pDim);
    m_DimList.push_back(std::unique_ptr<ScDPSaveDimension>(pDim));

    DimensionsChanged();
}

ScDPSaveDimension* ScDPSaveData::GetDimensionByName(const OUString& rName)
{
    for (auto const& iter : m_DimList)
    {
        if (iter->GetName() == rName && !iter->IsDataLayout() )
            return &(*iter);
    }

    return AppendNewDimension(rName, false);
}

ScDPSaveDimension* ScDPSaveData::GetExistingDimensionByName(const OUString& rName) const
{
    for (auto const& iter : m_DimList)
    {
        if (iter->GetName() == rName && !iter->IsDataLayout() )
            return &(*iter);
    }
    return nullptr; // don't create new
}

ScDPSaveDimension* ScDPSaveData::GetNewDimensionByName(const OUString& rName)
{
    for (auto const& iter : m_DimList)
    {
        if (iter->GetName() == rName && !iter->IsDataLayout() )
            return DuplicateDimension(rName);
    }

    return AppendNewDimension(rName, false);
}

ScDPSaveDimension* ScDPSaveData::GetDataLayoutDimension()
{
    ScDPSaveDimension* pDim = GetExistingDataLayoutDimension();
    if (pDim)
        return pDim;

    return AppendNewDimension(OUString(), true);
}

ScDPSaveDimension* ScDPSaveData::GetExistingDataLayoutDimension() const
{
    for (auto const& iter : m_DimList)
    {
        if ( iter->IsDataLayout() )
            return &(*iter);
    }
    return nullptr;
}

ScDPSaveDimension* ScDPSaveData::DuplicateDimension(const OUString& rName)
{
    // always insert new

    ScDPSaveDimension* pOld = GetExistingDimensionByName(rName);
    if (!pOld)
        return nullptr;

    ScDPSaveDimension* pNew = new ScDPSaveDimension( *pOld );
    AddDimension(pNew);
    return pNew;
}

void ScDPSaveData::RemoveDimensionByName(const OUString& rName)
{
    auto iter = std::find_if(m_DimList.begin(), m_DimList.end(),
        [&rName](const std::unique_ptr<ScDPSaveDimension>& rxDim) {
            return rxDim->GetName() == rName && !rxDim->IsDataLayout(); });
    if (iter != m_DimList.end())
    {
        m_DimList.erase(iter);
        RemoveDuplicateNameCount(rName);
        DimensionsChanged();
    }
}

ScDPSaveDimension& ScDPSaveData::DuplicateDimension( const ScDPSaveDimension& rDim )
{
    ScDPSaveDimension* pNew = new ScDPSaveDimension( rDim );
    AddDimension(pNew);
    return *pNew;
}

ScDPSaveDimension* ScDPSaveData::GetInnermostDimension(DataPilotFieldOrientation nOrientation)
{
    // return the innermost dimension for the given orientation,
    // excluding data layout dimension

    auto iter = std::find_if(m_DimList.rbegin(), m_DimList.rend(),
        [&nOrientation](const std::unique_ptr<ScDPSaveDimension>& rxDim) {
            return rxDim->GetOrientation() == nOrientation && !rxDim->IsDataLayout(); });
    if (iter != m_DimList.rend())
        return iter->get();

    return nullptr;
}

ScDPSaveDimension* ScDPSaveData::GetFirstDimension(sheet::DataPilotFieldOrientation eOrientation)
{
    for (auto const& iter : m_DimList)
    {
        if (iter->GetOrientation() == eOrientation && !iter->IsDataLayout())
            return &(*iter);
    }
    return nullptr;
}

long ScDPSaveData::GetDataDimensionCount() const
{
    long nDataCount = 0;

    for (auto const& iter : m_DimList)
    {
        if (iter->GetOrientation() == sheet::DataPilotFieldOrientation_DATA)
            ++nDataCount;
    }

    return nDataCount;
}

void ScDPSaveData::SetPosition( ScDPSaveDimension* pDim, long nNew )
{
    // position (nNew) is counted within dimensions of the same orientation

    DataPilotFieldOrientation nOrient = pDim->GetOrientation();

    auto it = std::find_if(m_DimList.begin(), m_DimList.end(),
        [&pDim](const std::unique_ptr<ScDPSaveDimension>& rxDim) { return pDim == rxDim.get(); });
    if (it != m_DimList.end())
    {
        // Tell vector<unique_ptr> to give up ownership of this element.
        // Don't delete this instance as it is re-inserted into the
        // container later.
        it->release();
        m_DimList.erase(it);
    }

    auto iterInsert = std::find_if(m_DimList.begin(), m_DimList.end(),
        [&nOrient, &nNew](const std::unique_ptr<ScDPSaveDimension>& rxDim) {
            if (rxDim->GetOrientation() == nOrient )
                --nNew;
            return nNew <= 0;
        });

    m_DimList.insert(iterInsert, std::unique_ptr<ScDPSaveDimension>(pDim));
    DimensionsChanged();
}

void ScDPSaveData::SetColumnGrand(bool bSet)
{
    nColumnGrandMode = sal_uInt16(bSet);
}

void ScDPSaveData::SetRowGrand(bool bSet)
{
    nRowGrandMode = sal_uInt16(bSet);
}

void ScDPSaveData::SetIgnoreEmptyRows(bool bSet)
{
    nIgnoreEmptyMode = sal_uInt16(bSet);
}

void ScDPSaveData::SetRepeatIfEmpty(bool bSet)
{
    nRepeatEmptyMode = sal_uInt16(bSet);
}

void ScDPSaveData::SetFilterButton(bool bSet)
{
    bFilterButton = bSet;
}

void ScDPSaveData::SetDrillDown(bool bSet)
{
    bDrillDown = bSet;
}

static void lcl_ResetOrient( const uno::Reference<sheet::XDimensionsSupplier>& xSource )
{
    uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
    uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
    long nIntCount = xIntDims->getCount();
    for (long nIntDim=0; nIntDim<nIntCount; nIntDim++)
    {
        uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface( xIntDims->getByIndex(nIntDim) );
        uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
        if (xDimProp.is())
        {
            xDimProp->setPropertyValue( SC_UNO_DP_ORIENTATION, uno::Any(sheet::DataPilotFieldOrientation_HIDDEN) );
        }
    }
}

void ScDPSaveData::WriteToSource( const uno::Reference<sheet::XDimensionsSupplier>& xSource )
{
    if (!xSource.is())
        return;

    // source options must be first!

    uno::Reference<beans::XPropertySet> xSourceProp( xSource, uno::UNO_QUERY );
    SAL_WARN_IF( !xSourceProp.is(), "sc.core", "no properties at source" );
    if ( xSourceProp.is() )
    {
        // source options are not available for external sources
        //TODO: use XPropertySetInfo to test for availability?

        try
        {
            if ( nIgnoreEmptyMode != SC_DPSAVEMODE_DONTKNOW )
                lcl_SetBoolProperty( xSourceProp,
                    SC_UNO_DP_IGNOREEMPTY, static_cast<bool>(nIgnoreEmptyMode) );
            if ( nRepeatEmptyMode != SC_DPSAVEMODE_DONTKNOW )
                lcl_SetBoolProperty( xSourceProp,
                    SC_UNO_DP_REPEATEMPTY, static_cast<bool>(nRepeatEmptyMode) );
        }
        catch(uno::Exception&)
        {
            // no error
        }

        const boost::optional<OUString> & pGrandTotalName = GetGrandTotalName();
        if (pGrandTotalName)
            ScUnoHelpFunctions::SetOptionalPropertyValue(xSourceProp, SC_UNO_DP_GRANDTOTAL_NAME, *pGrandTotalName);
    }

    // exceptions in the other calls are errors
    try
    {
        // reset all orientations
        //TODO: "forgetSettings" or similar at source ?????
        //TODO: reset all duplicated dimensions, or reuse them below !!!
        SAL_INFO("sc.core", "ScDPSaveData::WriteToSource");

        lcl_ResetOrient( xSource );

        uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
        uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
        long nIntCount = xIntDims->getCount();

        for (const auto& rxDim : m_DimList)
        {
            OUString aName = rxDim->GetName();
            OUString aCoreName = ScDPUtil::getSourceDimensionName(aName);

            SAL_INFO("sc.core", aName);

            bool bData = rxDim->IsDataLayout();

            //TODO: getByName for ScDPSource, including DataLayoutDimension !!!!!!!!

            bool bFound = false;
            for (long nIntDim=0; nIntDim<nIntCount && !bFound; nIntDim++)
            {
                uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface( xIntDims->getByIndex(nIntDim) );
                if ( bData )
                {
                    uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
                    if ( xDimProp.is() )
                    {
                        bFound = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                                    SC_UNO_DP_ISDATALAYOUT );
                        //TODO: error checking -- is "IsDataLayoutDimension" property required??
                    }
                }
                else
                {
                    uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
                    if (xDimName.is() && xDimName->getName() == aCoreName)
                        bFound = true;
                }

                if (bFound)
                {
                    if (rxDim->GetDupFlag())
                    {
                        uno::Reference<util::XCloneable> xCloneable(xIntDim, uno::UNO_QUERY);
                        SAL_WARN_IF(!xCloneable.is(), "sc.core", "cannot clone dimension");
                        if (xCloneable.is())
                        {
                            uno::Reference<util::XCloneable> xNew = xCloneable->createClone();
                            uno::Reference<container::XNamed> xNewName(xNew, uno::UNO_QUERY);
                            if (xNewName.is())
                            {
                                xNewName->setName(aName);
                                rxDim->WriteToSource(xNew);
                            }
                        }
                    }
                    else
                        rxDim->WriteToSource( xIntDim );
                }
            }
            SAL_WARN_IF(!bFound, "sc.core", "WriteToSource: Dimension not found: " + aName + ".");
        }

        if ( xSourceProp.is() )
        {
            if ( nColumnGrandMode != SC_DPSAVEMODE_DONTKNOW )
                lcl_SetBoolProperty( xSourceProp,
                    SC_UNO_DP_COLGRAND, static_cast<bool>(nColumnGrandMode) );
            if ( nRowGrandMode != SC_DPSAVEMODE_DONTKNOW )
                lcl_SetBoolProperty( xSourceProp,
                    SC_UNO_DP_ROWGRAND, static_cast<bool>(nRowGrandMode) );
        }
    }
    catch(uno::Exception const &)
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("sc.core", "exception in WriteToSource " << exceptionToString(ex));
    }
}

bool ScDPSaveData::IsEmpty() const
{
    for (auto const& iter : m_DimList)
    {
        if (iter->GetOrientation() != sheet::DataPilotFieldOrientation_HIDDEN && !iter->IsDataLayout())
            return false;
    }
    return true; // no entries that are not hidden
}

void ScDPSaveData::RemoveAllGroupDimensions( const OUString& rSrcDimName, std::vector<OUString>* pDeletedNames )
{
    if (!pDimensionData)
        // No group dimensions exist. Nothing to do.
        return;

    // Remove numeric group dimension (exists once at most). No need to delete
    // anything in save data (grouping was done inplace in an existing base
    // dimension).
    pDimensionData->RemoveNumGroupDimension(rSrcDimName);

    // Remove named group dimension(s). Dimensions have to be removed from
    // dimension save data and from save data too.
    const ScDPSaveGroupDimension* pExistingGroup = pDimensionData->GetGroupDimForBase(rSrcDimName);
    while ( pExistingGroup )
    {
        OUString aGroupDimName = pExistingGroup->GetGroupDimName();
        pDimensionData->RemoveGroupDimension(aGroupDimName);     // pExistingGroup is deleted

        // also remove SaveData settings for the dimension that no longer exists
        RemoveDimensionByName(aGroupDimName);

        if (pDeletedNames)
            pDeletedNames->push_back(aGroupDimName);

        // see if there are more group dimensions
        pExistingGroup = pDimensionData->GetGroupDimForBase(rSrcDimName);

        if ( pExistingGroup && pExistingGroup->GetGroupDimName() == aGroupDimName )
        {
            // still get the same group dimension?
            OSL_FAIL("couldn't remove group dimension");
            pExistingGroup = nullptr;      // avoid endless loop
        }
    }
}

ScDPDimensionSaveData* ScDPSaveData::GetDimensionData()
{
    if (!pDimensionData)
        pDimensionData.reset( new ScDPDimensionSaveData );
    return pDimensionData.get();
}

void ScDPSaveData::SetDimensionData( const ScDPDimensionSaveData* pNew )
{
    if ( pNew )
        pDimensionData.reset( new ScDPDimensionSaveData( *pNew ) );
    else
        pDimensionData.reset();
}

void ScDPSaveData::BuildAllDimensionMembers(ScDPTableData* pData)
{
    if (mbDimensionMembersBuilt)
        return;

    // First, build a dimension name-to-index map.
    typedef std::unordered_map<OUString, long> NameIndexMap;
    NameIndexMap aMap;
    long nColCount = pData->GetColumnCount();
    for (long i = 0; i < nColCount; ++i)
        aMap.emplace(pData->getDimensionName(i), i);

    NameIndexMap::const_iterator itrEnd = aMap.end();

    for (auto const& iter : m_DimList)
    {
        const OUString& rDimName = iter->GetName();
        if (rDimName.isEmpty())
            // empty dimension name. It must be data layout.
            continue;

        NameIndexMap::const_iterator itr = aMap.find(rDimName);
        if (itr == itrEnd)
            // dimension name not in the data. This should never happen!
            continue;

        long nDimIndex = itr->second;
        const std::vector<SCROW>& rMembers = pData->GetColumnEntries(nDimIndex);
        size_t nMemberCount = rMembers.size();
        for (size_t j = 0; j < nMemberCount; ++j)
        {
            const ScDPItemData* pMemberData = pData->GetMemberById( nDimIndex, rMembers[j] );
            OUString aMemName = pData->GetFormattedString(nDimIndex, *pMemberData, false);
            if (iter->GetExistingMemberByName(aMemName))
                // this member instance already exists. nothing to do.
                continue;

            unique_ptr<ScDPSaveMember> pNewMember(new ScDPSaveMember(aMemName));
            pNewMember->SetIsVisible(true);
            iter->AddMember(std::move(pNewMember));
        }
    }

    mbDimensionMembersBuilt = true;
}

void ScDPSaveData::SyncAllDimensionMembers(ScDPTableData* pData)
{
    typedef std::unordered_map<OUString, long> NameIndexMap;

    // First, build a dimension name-to-index map.
    NameIndexMap aMap;
    long nColCount = pData->GetColumnCount();
    for (long i = 0; i < nColCount; ++i)
        aMap.emplace(pData->getDimensionName(i), i);

    NameIndexMap::const_iterator itMapEnd = aMap.end();

    for (auto const& it : m_DimList)
    {
        const OUString& rDimName = it->GetName();
        if (rDimName.isEmpty())
            // empty dimension name. It must be data layout.
            continue;

        NameIndexMap::const_iterator itMap = aMap.find(rDimName);
        if (itMap == itMapEnd)
            // dimension name not in the data. This should never happen!
            continue;

        ScDPSaveDimension::MemberSetType aMemNames;
        long nDimIndex = itMap->second;
        const std::vector<SCROW>& rMembers = pData->GetColumnEntries(nDimIndex);
        size_t nMemberCount = rMembers.size();
        for (size_t j = 0; j < nMemberCount; ++j)
        {
            const ScDPItemData* pMemberData = pData->GetMemberById(nDimIndex, rMembers[j]);
            OUString aMemName = pData->GetFormattedString(nDimIndex, *pMemberData, false);
            aMemNames.insert(aMemName);
        }

        it->RemoveObsoleteMembers(aMemNames);
    }
}

bool ScDPSaveData::HasInvisibleMember(const OUString& rDimName) const
{
    ScDPSaveDimension* pDim = GetExistingDimensionByName(rDimName);
    if (!pDim)
        return false;

    return pDim->HasInvisibleMember();
}

#if DUMP_PIVOT_TABLE

void ScDPSaveData::Dump() const
{
    for (auto const& itDim : m_DimList)
    {
        const ScDPSaveDimension& rDim = *itDim;
        rDim.Dump();
    }
}

#endif

void ScDPSaveData::CheckDuplicateName(ScDPSaveDimension& rDim)
{
    const OUString aName = ScDPUtil::getSourceDimensionName(rDim.GetName());
    DupNameCountType::iterator it = maDupNameCounts.find(aName);
    if (it != maDupNameCounts.end())
    {
        rDim.SetName(ScDPUtil::createDuplicateDimensionName(aName, ++it->second));
        rDim.SetDupFlag(true);
    }
    else
        // New name.
        maDupNameCounts.emplace(aName, 0);
}

void ScDPSaveData::RemoveDuplicateNameCount(const OUString& rName)
{
    OUString aCoreName = rName;
    if (ScDPUtil::isDuplicateDimension(rName))
        aCoreName = ScDPUtil::getSourceDimensionName(rName);

    DupNameCountType::iterator it = maDupNameCounts.find(aCoreName);
    if (it == maDupNameCounts.end())
        return;

    if (!it->second)
    {
        maDupNameCounts.erase(it);
        return;
    }

    --it->second;
}

ScDPSaveDimension* ScDPSaveData::AppendNewDimension(const OUString& rName, bool bDataLayout)
{
    if (ScDPUtil::isDuplicateDimension(rName))
        // This call is for original dimensions only.
        return nullptr;

    ScDPSaveDimension* pNew = new ScDPSaveDimension(rName, bDataLayout);
    m_DimList.push_back(std::unique_ptr<ScDPSaveDimension>(pNew));
    if (!maDupNameCounts.count(rName))
        maDupNameCounts.emplace(rName, 0);

    DimensionsChanged();
    return pNew;
}

void ScDPSaveData::DimensionsChanged()
{
    mpDimOrder.reset();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
