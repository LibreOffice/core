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

#include <svl/smplhint.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/sheet/NamedRangeFlag.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;

#include "nameuno.hxx"
#include "miscuno.hxx"
#include "cellsuno.hxx"
#include "convuno.hxx"
#include "targuno.hxx"
#include "tokenuno.hxx"
#include "tokenarray.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "rangenam.hxx"
#include "unonames.hxx"

#include "scui_def.hxx"

static const SfxItemPropertyMapEntry* lcl_GetNamedRangeMap()
{
    static const SfxItemPropertyMapEntry aNamedRangeMap_Impl[] =
    {
        {OUString(SC_UNO_LINKDISPBIT),      0,  cppu::UnoType<awt::XBitmap>::get(), beans::PropertyAttribute::READONLY, 0 },
        {OUString(SC_UNO_LINKDISPNAME),     0,  cppu::UnoType<OUString>::get(),                beans::PropertyAttribute::READONLY, 0 },
        {OUString(SC_UNONAME_TOKENINDEX),   0,  cppu::UnoType<sal_Int32>::get(),                    beans::PropertyAttribute::READONLY, 0 },
        {OUString(SC_UNONAME_ISSHAREDFMLA), 0,  cppu::UnoType<bool>::get(),                          0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aNamedRangeMap_Impl;
}

static const SfxItemPropertyMapEntry* lcl_GetNamedRangesMap()
{
    static const SfxItemPropertyMapEntry aNamedRangesMap_Impl[] =
    {
        {OUString(SC_UNO_MODIFY_BROADCAST), 0,  cppu::UnoType<bool>::get(), 0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aNamedRangesMap_Impl;
}

#define SCNAMEDRANGEOBJ_SERVICE     "com.sun.star.sheet.NamedRange"

SC_SIMPLE_SERVICE_INFO( ScLabelRangeObj, "ScLabelRangeObj", "com.sun.star.sheet.LabelRange" )
SC_SIMPLE_SERVICE_INFO( ScLabelRangesObj, "ScLabelRangesObj", "com.sun.star.sheet.LabelRanges" )
SC_SIMPLE_SERVICE_INFO( ScNamedRangesObj, "ScNamedRangesObj", "com.sun.star.sheet.NamedRanges" )

static bool lcl_UserVisibleName(const ScRangeData& rData)
{
    //! as method to ScRangeData

    return !rData.HasType(RT_DATABASE);
}

ScNamedRangeObj::ScNamedRangeObj( rtl::Reference< ScNamedRangesObj > xParent, ScDocShell* pDocSh, const OUString& rNm, Reference<container::XNamed> xSheet):
    mxParent(xParent),
    pDocShell( pDocSh ),
    aName( rNm ),
    mxSheet( xSheet )
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScNamedRangeObj::~ScNamedRangeObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScNamedRangeObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    // reference update is of no interest

    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
        pDocShell = nullptr;       // became invalid
}

// Helper functions

ScRangeData* ScNamedRangeObj::GetRangeData_Impl()
{
    ScRangeData* pRet = nullptr;
    if (pDocShell)
    {
        ScRangeName* pNames;
        SCTAB nTab = GetTab_Impl();
        if (nTab >= 0)
            pNames = pDocShell->GetDocument().GetRangeName(nTab);
        else
            pNames = pDocShell->GetDocument().GetRangeName();
        if (pNames)
        {
            pRet = pNames->findByUpperName(ScGlobal::pCharClass->uppercase(aName));
            if (pRet)
                pRet->ValidateTabRefs();        // adjust relative tab refs to valid tables
        }
    }
    return pRet;
}

SCTAB ScNamedRangeObj::GetTab_Impl()
{
    if (mxSheet.is())
    {
        if (!pDocShell)
            return -2;
        ScDocument& rDoc = pDocShell->GetDocument();
        SCTAB nTab;
        OUString sName = mxSheet->getName();
        bool bFound = rDoc.GetTable(sName, nTab);
        assert(bFound); (void)bFound;   // fouled up?
        return nTab;
    }
    else
        return -1;//global range name
}

// sheet::XNamedRange

void ScNamedRangeObj::Modify_Impl( const OUString* pNewName, const ScTokenArray* pNewTokens, const OUString* pNewContent,
                                    const ScAddress* pNewPos, const sal_uInt16* pNewType,
                                    const formula::FormulaGrammar::Grammar eGrammar )
{
    if (!pDocShell)
        return;

    ScDocument& rDoc = pDocShell->GetDocument();
    ScRangeName* pNames;
    SCTAB nTab = GetTab_Impl();
    if (nTab >= 0)
        pNames = rDoc.GetRangeName(nTab);
    else
        pNames = rDoc.GetRangeName();
    if (!pNames)
        return;

    const ScRangeData* pOld = pNames->findByUpperName(ScGlobal::pCharClass->uppercase(aName));
    if (!pOld)
        return;

    ScRangeName* pNewRanges = new ScRangeName(*pNames);

    OUString aInsName = pOld->GetName();
    if (pNewName)
        aInsName = *pNewName;

    OUString aContent;                            // Content string based =>
    pOld->GetSymbol( aContent, eGrammar);   // no problems with changed positions and such.
    if (pNewContent)
        aContent = *pNewContent;

    ScAddress aPos = pOld->GetPos();
    if (pNewPos)
        aPos = *pNewPos;

    sal_uInt16 nType = pOld->GetType();
    if (pNewType)
        nType = *pNewType;

    ScRangeData* pNew = nullptr;
    if (pNewTokens)
        pNew = new ScRangeData( &rDoc, aInsName, *pNewTokens, aPos, nType );
    else
        pNew = new ScRangeData( &rDoc, aInsName, aContent, aPos, nType, eGrammar );

    pNew->SetIndex( pOld->GetIndex() );

    pNewRanges->erase(*pOld);
    if (pNewRanges->insert(pNew))
    {
        pDocShell->GetDocFunc().SetNewRangeNames(pNewRanges, mxParent->IsModifyAndBroadcast(), nTab);

        aName = aInsName;   //! broadcast?
    }
    else
    {
        pNew = nullptr;        //! uno::Exception/Error or something
        delete pNewRanges;
    }
}

OUString SAL_CALL ScNamedRangeObj::getName() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return aName;
}

void SAL_CALL ScNamedRangeObj::setName( const OUString& aNewName )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    //! adapt formulas ?????

    OUString aNewStr(aNewName);
    // GRAM_PODF_A1 for API compatibility.
    Modify_Impl( &aNewStr, nullptr, nullptr, nullptr, nullptr,formula::FormulaGrammar::GRAM_PODF_A1 );

    if ( aName != aNewStr )                 // some error occurred...
        throw uno::RuntimeException();      // no other exceptions specified
}

OUString SAL_CALL ScNamedRangeObj::getContent() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    OUString aContent;
    ScRangeData* pData = GetRangeData_Impl();
    if (pData)
        // GRAM_PODF_A1 for API compatibility.
        pData->GetSymbol( aContent,formula::FormulaGrammar::GRAM_PODF_A1);
    return aContent;
}

void SAL_CALL ScNamedRangeObj::setContent( const OUString& aContent )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    OUString aContStr(aContent);
    // GRAM_PODF_A1 for API compatibility.
    Modify_Impl( nullptr, nullptr, &aContStr, nullptr, nullptr,formula::FormulaGrammar::GRAM_PODF_A1 );
}

table::CellAddress SAL_CALL ScNamedRangeObj::getReferencePosition()
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScAddress aPos;
    ScRangeData* pData = GetRangeData_Impl();
    if (pData)
        aPos = pData->GetPos();
    table::CellAddress aAddress;
    aAddress.Column = aPos.Col();
    aAddress.Row    = aPos.Row();
    aAddress.Sheet  = aPos.Tab();
    if (pDocShell)
    {
        SCTAB nDocTabs = pDocShell->GetDocument().GetTableCount();
        if ( aAddress.Sheet >= nDocTabs && nDocTabs > 0 )
        {
            //  Even after ValidateTabRefs, the position can be invalid if
            //  the content points to preceding tables. The resulting string
            //  is invalid in any case, so the position is just shifted.
            aAddress.Sheet = nDocTabs - 1;
        }
    }
    return aAddress;
}

void SAL_CALL ScNamedRangeObj::setReferencePosition( const table::CellAddress& aReferencePosition )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScAddress aPos( (SCCOL)aReferencePosition.Column, (SCROW)aReferencePosition.Row, aReferencePosition.Sheet );
    // GRAM_PODF_A1 for API compatibility.
    Modify_Impl( nullptr, nullptr, nullptr, &aPos, nullptr,formula::FormulaGrammar::GRAM_PODF_A1 );
}

sal_Int32 SAL_CALL ScNamedRangeObj::getType() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    sal_Int32 nType=0;
    ScRangeData* pData = GetRangeData_Impl();
    if (pData)
    {
        // do not return internal RT_* flags
        // see property 'IsSharedFormula' for RT_SHARED
        if ( pData->HasType(RT_CRITERIA) )  nType |= sheet::NamedRangeFlag::FILTER_CRITERIA;
        if ( pData->HasType(RT_PRINTAREA) ) nType |= sheet::NamedRangeFlag::PRINT_AREA;
        if ( pData->HasType(RT_COLHEADER) ) nType |= sheet::NamedRangeFlag::COLUMN_HEADER;
        if ( pData->HasType(RT_ROWHEADER) ) nType |= sheet::NamedRangeFlag::ROW_HEADER;
    }
    return nType;
}

void SAL_CALL ScNamedRangeObj::setType( sal_Int32 nUnoType )
    throw (uno::RuntimeException, std::exception)
{
    // see property 'IsSharedFormula' for RT_SHARED
    SolarMutexGuard aGuard;
    sal_uInt16 nNewType = RT_NAME;
    if ( nUnoType & sheet::NamedRangeFlag::FILTER_CRITERIA )    nNewType |= RT_CRITERIA;
    if ( nUnoType & sheet::NamedRangeFlag::PRINT_AREA )         nNewType |= RT_PRINTAREA;
    if ( nUnoType & sheet::NamedRangeFlag::COLUMN_HEADER )      nNewType |= RT_COLHEADER;
    if ( nUnoType & sheet::NamedRangeFlag::ROW_HEADER )         nNewType |= RT_ROWHEADER;

    // GRAM_PODF_A1 for API compatibility.
    Modify_Impl( nullptr, nullptr, nullptr, nullptr, &nNewType,formula::FormulaGrammar::GRAM_PODF_A1 );
}

// XFormulaTokens

uno::Sequence<sheet::FormulaToken> SAL_CALL ScNamedRangeObj::getTokens()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Sequence<sheet::FormulaToken> aSequence;
    ScRangeData* pData = GetRangeData_Impl();
    if (pData && pDocShell)
    {
        ScTokenArray* pTokenArray = pData->GetCode();
        if ( pTokenArray )
            (void)ScTokenConversion::ConvertToTokenSequence( pDocShell->GetDocument(), aSequence, *pTokenArray );
    }
    return aSequence;
}

void SAL_CALL ScNamedRangeObj::setTokens( const uno::Sequence<sheet::FormulaToken>& rTokens )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if( pDocShell )
    {
        ScTokenArray aTokenArray;
        (void)ScTokenConversion::ConvertToTokenArray( pDocShell->GetDocument(), aTokenArray, rTokens );
        // GRAM_PODF_A1 for API compatibility.
        Modify_Impl( nullptr, &aTokenArray, nullptr, nullptr, nullptr, formula::FormulaGrammar::GRAM_PODF_A1 );
    }
}

// XCellRangeSource

uno::Reference<table::XCellRange> SAL_CALL ScNamedRangeObj::getReferredCells()
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScRange aRange;
    ScRangeData* pData = GetRangeData_Impl();
    if ( pData && pData->IsValidReference( aRange ) )
    {
        //! static function to create ScCellObj/ScCellRangeObj at ScCellRangeObj ???

        if ( aRange.aStart == aRange.aEnd )
            return new ScCellObj( pDocShell, aRange.aStart );
        else
            return new ScCellRangeObj( pDocShell, aRange );
    }
    return nullptr;
}

// beans::XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScNamedRangeObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    static uno::Reference< beans::XPropertySetInfo >  aRef(new SfxItemPropertySetInfo( lcl_GetNamedRangeMap() ));
    return aRef;
}

void SAL_CALL ScNamedRangeObj::setPropertyValue(
                        const OUString& rPropertyName, const uno::Any& /*aValue*/ )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( rPropertyName == SC_UNONAME_ISSHAREDFMLA )
    {
        // Ignore this.
    }
}

uno::Any SAL_CALL ScNamedRangeObj::getPropertyValue( const OUString& rPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if ( rPropertyName == SC_UNO_LINKDISPBIT )
    {
        //  no target bitmaps for individual entries (would be all equal)
        // ScLinkTargetTypeObj::SetLinkTargetBitmap( aRet, SC_LINKTARGETTYPE_RANGENAME );
    }
    else if ( rPropertyName == SC_UNO_LINKDISPNAME )
        aRet <<= OUString( aName );
    else if ( rPropertyName == SC_UNONAME_TOKENINDEX )
    {
        // get index for use in formula tokens (read-only)
        ScRangeData* pData = GetRangeData_Impl();
        if (pData)
            aRet <<= static_cast<sal_Int32>(pData->GetIndex());
    }
    else if ( rPropertyName == SC_UNONAME_ISSHAREDFMLA )
    {
        if (GetRangeData_Impl())
            aRet <<= false;
    }
    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScNamedRangeObj )

// lang::XServiceInfo

OUString SAL_CALL ScNamedRangeObj::getImplementationName() throw(uno::RuntimeException, std::exception)
{
    return OUString( "ScNamedRangeObj" );
}

sal_Bool SAL_CALL ScNamedRangeObj::supportsService( const OUString& rServiceName )
                                                    throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScNamedRangeObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence<OUString> aRet(2);
    aRet[0] = SCNAMEDRANGEOBJ_SERVICE;
    aRet[1] = SCLINKTARGET_SERVICE;
    return aRet;
}

// XUnoTunnel

sal_Int64 SAL_CALL ScNamedRangeObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException, std::exception)
{
    if ( rId.getLength() == 16 &&
          0 == memcmp( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    return 0;
}

namespace
{
    class theScNamedRangeObjUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScNamedRangeObjUnoTunnelId> {};
}

const uno::Sequence<sal_Int8>& ScNamedRangeObj::getUnoTunnelId()
{
    return theScNamedRangeObjUnoTunnelId::get().getSeq();
}

ScNamedRangesObj::ScNamedRangesObj(ScDocShell* pDocSh) :
    mbModifyAndBroadcast(true),
    pDocShell( pDocSh )
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScNamedRangesObj::~ScNamedRangesObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScNamedRangesObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    // reference update is of no interest

    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
    {
        pDocShell = nullptr;       // became invalid
    }
}

// sheet::XNamedRanges

void SAL_CALL ScNamedRangesObj::addNewByName( const OUString& aName,
    const OUString& aContent, const table::CellAddress& aPosition,
    sal_Int32 nUnoType )
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScAddress aPos( (SCCOL)aPosition.Column, (SCROW)aPosition.Row, aPosition.Sheet );

    sal_uInt16 nNewType = RT_NAME;
    if ( nUnoType & sheet::NamedRangeFlag::FILTER_CRITERIA )    nNewType |= RT_CRITERIA;
    if ( nUnoType & sheet::NamedRangeFlag::PRINT_AREA )         nNewType |= RT_PRINTAREA;
    if ( nUnoType & sheet::NamedRangeFlag::COLUMN_HEADER )      nNewType |= RT_COLHEADER;
    if ( nUnoType & sheet::NamedRangeFlag::ROW_HEADER )         nNewType |= RT_ROWHEADER;

    bool bDone = false;
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScRangeName* pNames = GetRangeName_Impl();
        if (pNames && !pNames->findByUpperName(ScGlobal::pCharClass->uppercase(aName)))
        {
            ScRangeName* pNewRanges = new ScRangeName( *pNames );
            // GRAM_PODF_A1 for API compatibility.
            ScRangeData* pNew = new ScRangeData( &rDoc, aName, aContent,
                                                aPos, nNewType,formula::FormulaGrammar::GRAM_PODF_A1 );
            if ( pNewRanges->insert(pNew) )
            {
                pDocShell->GetDocFunc().SetNewRangeNames(pNewRanges, mbModifyAndBroadcast, GetTab_Impl());
                bDone = true;
            }
            else
            {
                pNew = nullptr;
                delete pNewRanges;
            }
        }
    }

    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

void SAL_CALL ScNamedRangesObj::addNewFromTitles( const table::CellRangeAddress& aSource,
                                    sheet::Border aBorder ) throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    //! this cannot be an enum, because multiple bits can be set !!!

    bool bTop    = ( aBorder == sheet::Border_TOP );
    bool bLeft   = ( aBorder == sheet::Border_LEFT );
    bool bBottom = ( aBorder == sheet::Border_BOTTOM );
    bool bRight  = ( aBorder == sheet::Border_RIGHT );

    ScRange aRange;
    ScUnoConversion::FillScRange( aRange, aSource );

    sal_uInt16 nFlags = 0;
    if (bTop)    nFlags |= NAME_TOP;
    if (bLeft)   nFlags |= NAME_LEFT;
    if (bBottom) nFlags |= NAME_BOTTOM;
    if (bRight)  nFlags |= NAME_RIGHT;

    if (nFlags)
        pDocShell->GetDocFunc().CreateNames( aRange, nFlags, true, GetTab_Impl() );
}

void SAL_CALL ScNamedRangesObj::removeByName( const OUString& aName )
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    if (pDocShell)
    {
        ScRangeName* pNames = GetRangeName_Impl();
        if (pNames)
        {
            const ScRangeData* pData = pNames->findByUpperName(ScGlobal::pCharClass->uppercase(aName));
            if (pData && lcl_UserVisibleName(*pData))
            {
                ScRangeName* pNewRanges = new ScRangeName(*pNames);
                pNewRanges->erase(*pData);
                pDocShell->GetDocFunc().SetNewRangeNames( pNewRanges, mbModifyAndBroadcast, GetTab_Impl());
                bDone = true;
            }
        }
    }

    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

void SAL_CALL ScNamedRangesObj::outputList( const table::CellAddress& aOutputPosition )
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScAddress aPos( (SCCOL)aOutputPosition.Column, (SCROW)aOutputPosition.Row, aOutputPosition.Sheet );
    if (pDocShell)
        pDocShell->GetDocFunc().InsertNameList( aPos, true );
}

// container::XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScNamedRangesObj::createEnumeration()
                                                    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.sheet.NamedRangesEnumeration"));
}

// container::XIndexAccess

sal_Int32 SAL_CALL ScNamedRangesObj::getCount() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    long nRet = 0;
    if (pDocShell)
    {
        ScRangeName* pNames = GetRangeName_Impl();
        if (pNames)
        {
            ScRangeName::const_iterator itr = pNames->begin(), itrEnd = pNames->end();
            for (; itr != itrEnd; ++itr)
                if (lcl_UserVisibleName(*itr->second))
                    ++nRet;
        }
    }
    return nRet;
}

uno::Any SAL_CALL ScNamedRangesObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference< sheet::XNamedRange >  xRange(GetObjectByIndex_Impl((sal_uInt16)nIndex));
    if ( xRange.is() )
        return uno::makeAny(xRange);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScNamedRangesObj::getElementType() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<sheet::XNamedRange>::get();   // must be suitable for getByIndex
}

sal_Bool SAL_CALL ScNamedRangesObj::hasElements() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

Reference<beans::XPropertySetInfo> SAL_CALL ScNamedRangesObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException, std::exception)
{
    static Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo(lcl_GetNamedRangesMap()));
    return aRef;
}

void SAL_CALL ScNamedRangesObj::setPropertyValue(
                        const OUString& rPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    if ( rPropertyName == SC_UNO_MODIFY_BROADCAST )
    {
        aValue >>= mbModifyAndBroadcast;
    }
}

Any SAL_CALL ScNamedRangesObj::getPropertyValue( const OUString& rPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    Any aRet;
    if ( rPropertyName == SC_UNO_MODIFY_BROADCAST )
    {
        aRet <<= mbModifyAndBroadcast;
    }

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScNamedRangesObj )

uno::Any SAL_CALL ScNamedRangesObj::getByName( const OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference< sheet::XNamedRange >  xRange(GetObjectByName_Impl(aName));
    if ( xRange.is() )
        return uno::makeAny(xRange);
    else
        throw container::NoSuchElementException();
}

uno::Sequence<OUString> SAL_CALL ScNamedRangesObj::getElementNames()
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScRangeName* pNames = GetRangeName_Impl();
        if (pNames)
        {
            long nVisCount = getCount();            // names with lcl_UserVisibleName
            uno::Sequence<OUString> aSeq(nVisCount);
            OUString* pAry = aSeq.getArray();
            sal_uInt16 nVisPos = 0;
            ScRangeName::const_iterator itr = pNames->begin(), itrEnd = pNames->end();
            for (; itr != itrEnd; ++itr)
            {
                if (lcl_UserVisibleName(*itr->second))
                    pAry[nVisPos++] = itr->second->GetName();
            }
            return aSeq;
        }
    }
    return uno::Sequence<OUString>(0);
}

sal_Bool SAL_CALL ScNamedRangesObj::hasByName( const OUString& aName )
                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScRangeName* pNames = GetRangeName_Impl();
        if (pNames)
        {
            const ScRangeData* pData = pNames->findByUpperName(ScGlobal::pCharClass->uppercase(aName));
            if (pData && lcl_UserVisibleName(*pData))
                return sal_True;
        }
    }
    return false;
}

/** called from the XActionLockable interface methods on initial locking */
void ScNamedRangesObj::lock()
{
    pDocShell->GetDocument().PreprocessRangeNameUpdate();
}

/** called from the XActionLockable interface methods on final unlock */
void ScNamedRangesObj::unlock()
{
    pDocShell->GetDocument().CompileHybridFormula();
}

// document::XActionLockable

sal_Bool ScNamedRangesObj::isActionLocked() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return pDocShell->GetDocument().GetNamedRangesLockCount() != 0;
}

void ScNamedRangesObj::addActionLock() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScDocument& rDoc = pDocShell->GetDocument();
    sal_Int16 nLockCount = rDoc.GetNamedRangesLockCount();
    ++nLockCount;
    if ( nLockCount == 1 )
    {
        lock();
    }
    rDoc.SetNamedRangesLockCount( nLockCount );
}

void ScNamedRangesObj::removeActionLock() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScDocument& rDoc = pDocShell->GetDocument();
    sal_Int16 nLockCount = rDoc.GetNamedRangesLockCount();
    if ( nLockCount > 0 )
    {
        --nLockCount;
        if ( nLockCount == 0 )
        {
            unlock();
        }
        rDoc.SetNamedRangesLockCount( nLockCount );
    }
}

void ScNamedRangesObj::setActionLocks( sal_Int16 nLock ) throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( nLock >= 0 )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        sal_Int16 nLockCount = rDoc.GetNamedRangesLockCount();
        if ( nLock == 0 && nLockCount > 0 )
        {
            unlock();
        }
        if ( nLock > 0 && nLockCount == 0 )
        {
            lock();
        }
        rDoc.SetNamedRangesLockCount( nLock );
    }
}

sal_Int16 ScNamedRangesObj::resetActionLocks() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScDocument& rDoc = pDocShell->GetDocument();
    sal_Int16 nLockCount = rDoc.GetNamedRangesLockCount();
    if ( nLockCount > 0 )
    {
        unlock();
    }
    rDoc.SetNamedRangesLockCount( 0 );
    return nLockCount;
}

ScGlobalNamedRangesObj::ScGlobalNamedRangesObj(ScDocShell* pDocSh)
    : ScNamedRangesObj(pDocSh)
{

}

ScGlobalNamedRangesObj::~ScGlobalNamedRangesObj()
{

}

ScNamedRangeObj* ScGlobalNamedRangesObj::GetObjectByIndex_Impl(sal_uInt16 nIndex)
{
    if (!pDocShell)
        return nullptr;

    ScRangeName* pNames = pDocShell->GetDocument().GetRangeName();
    if (!pNames)
        return nullptr;

    ScRangeName::const_iterator itr = pNames->begin(), itrEnd = pNames->end();
    sal_uInt16 nPos = 0;
    for (; itr != itrEnd; ++itr)
    {
        if (lcl_UserVisibleName(*itr->second))
        {
            if (nPos == nIndex)
                return new ScNamedRangeObj(this, pDocShell, itr->second->GetName());
        }
        ++nPos;
    }
    return nullptr;
}

ScNamedRangeObj* ScGlobalNamedRangesObj::GetObjectByName_Impl(const OUString& aName)
{
    if ( pDocShell && hasByName(aName) )
        return new ScNamedRangeObj(this, pDocShell, aName);
    return nullptr;
}

ScRangeName* ScGlobalNamedRangesObj::GetRangeName_Impl()
{
    return pDocShell->GetDocument().GetRangeName();
}

SCTAB ScGlobalNamedRangesObj::GetTab_Impl()
{
    return -1;
}

ScLocalNamedRangesObj::ScLocalNamedRangesObj( ScDocShell* pDocSh, uno::Reference<container::XNamed> xSheet )
    : ScNamedRangesObj(pDocSh),
    mxSheet(xSheet)
{

}

ScLocalNamedRangesObj::~ScLocalNamedRangesObj()
{

}

ScNamedRangeObj* ScLocalNamedRangesObj::GetObjectByName_Impl(const OUString& aName)
{
    if ( pDocShell && hasByName( aName ) )
        return new ScNamedRangeObj( this, pDocShell, aName, mxSheet);
    return nullptr;

}

ScNamedRangeObj* ScLocalNamedRangesObj::GetObjectByIndex_Impl( sal_uInt16 nIndex )
{
    if (!pDocShell)
        return nullptr;

    OUString aName = mxSheet->getName();
    ScDocument& rDoc = pDocShell->GetDocument();
    SCTAB nTab;
    if (!rDoc.GetTable(aName, nTab))
        return nullptr;

    ScRangeName* pNames = rDoc.GetRangeName( nTab );
    if (!pNames)
        return nullptr;

    ScRangeName::const_iterator itr = pNames->begin(), itrEnd = pNames->end();
    sal_uInt16 nPos = 0;
    for (; itr != itrEnd; ++itr)
    {
        if (lcl_UserVisibleName(*itr->second))
        {
            if (nPos == nIndex)
                return new ScNamedRangeObj(this, pDocShell, itr->second->GetName(), mxSheet);
        }
        ++nPos;
    }
    return nullptr;
}

ScRangeName* ScLocalNamedRangesObj::GetRangeName_Impl()
{
    SCTAB nTab = GetTab_Impl();
    return pDocShell->GetDocument().GetRangeName( nTab );
}

SCTAB ScLocalNamedRangesObj::GetTab_Impl()
{
    SCTAB nTab;
    (void)pDocShell->GetDocument().GetTable(mxSheet->getName(), nTab);
    return nTab;
}

ScLabelRangeObj::ScLabelRangeObj(ScDocShell* pDocSh, bool bCol, const ScRange& rR) :
    pDocShell( pDocSh ),
    bColumn( bCol ),
    aRange( rR )
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScLabelRangeObj::~ScLabelRangeObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScLabelRangeObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //! Ref-Update !!!

    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
        pDocShell = nullptr;       // became invalid
}

// Helper functions

ScRangePair* ScLabelRangeObj::GetData_Impl()
{
    ScRangePair* pRet = nullptr;
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScRangePairList* pList = bColumn ? rDoc.GetColNameRanges() : rDoc.GetRowNameRanges();
        if (pList)
            pRet = pList->Find( aRange );
    }
    return pRet;
}

void ScLabelRangeObj::Modify_Impl( const ScRange* pLabel, const ScRange* pData )
{
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScRangePairList* pOldList = bColumn ? rDoc.GetColNameRanges() : rDoc.GetRowNameRanges();
        if (pOldList)
        {
            ScRangePairListRef xNewList(pOldList->Clone());
            ScRangePair* pEntry = xNewList->Find( aRange );
            if (pEntry)
            {
                xNewList->Remove( pEntry );     // only removed from list, not deleted

                if ( pLabel )
                    pEntry->GetRange(0) = *pLabel;
                if ( pData )
                    pEntry->GetRange(1) = *pData;

                xNewList->Join( *pEntry );
                delete pEntry;

                if (bColumn)
                    rDoc.GetColNameRangesRef() = xNewList;
                else
                    rDoc.GetRowNameRangesRef() = xNewList;

                rDoc.CompileColRowNameFormula();
                pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID );
                pDocShell->SetDocumentModified();

                //! Undo ?!?! (here and from dialog)

                if ( pLabel )
                    aRange = *pLabel;   // adapt object to find range again
            }
        }
    }
}

// sheet::XLabelRange

table::CellRangeAddress SAL_CALL ScLabelRangeObj::getLabelArea()
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    table::CellRangeAddress aRet;
    ScRangePair* pData = GetData_Impl();
    if (pData)
        ScUnoConversion::FillApiRange( aRet, pData->GetRange(0) );
    return aRet;
}

void SAL_CALL ScLabelRangeObj::setLabelArea( const table::CellRangeAddress& aLabelArea )
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScRange aLabelRange;
    ScUnoConversion::FillScRange( aLabelRange, aLabelArea );
    Modify_Impl( &aLabelRange, nullptr );
}

table::CellRangeAddress SAL_CALL ScLabelRangeObj::getDataArea()
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    table::CellRangeAddress aRet;
    ScRangePair* pData = GetData_Impl();
    if (pData)
        ScUnoConversion::FillApiRange( aRet, pData->GetRange(1) );
    return aRet;
}

void SAL_CALL ScLabelRangeObj::setDataArea( const table::CellRangeAddress& aDataArea )
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScRange aDataRange;
    ScUnoConversion::FillScRange( aDataRange, aDataArea );
    Modify_Impl( nullptr, &aDataRange );
}

ScLabelRangesObj::ScLabelRangesObj(ScDocShell* pDocSh, bool bCol) :
    pDocShell( pDocSh ),
    bColumn( bCol )
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScLabelRangesObj::~ScLabelRangesObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScLabelRangesObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //  reference update is of no interest

    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
    {
        pDocShell = nullptr;       // became invalid
    }
}

// sheet::XLabelRanges

ScLabelRangeObj* ScLabelRangesObj::GetObjectByIndex_Impl(size_t nIndex)
{
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScRangePairList* pList = bColumn ? rDoc.GetColNameRanges() : rDoc.GetRowNameRanges();
        if ( pList && nIndex < pList->size() )
        {
            ScRangePair* pData = (*pList)[nIndex];
            if (pData)
                return new ScLabelRangeObj( pDocShell, bColumn, pData->GetRange(0) );
        }
    }
    return nullptr;
}

void SAL_CALL ScLabelRangesObj::addNew( const table::CellRangeAddress& aLabelArea,
                                const table::CellRangeAddress& aDataArea )
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScRangePairList* pOldList = bColumn ? rDoc.GetColNameRanges() : rDoc.GetRowNameRanges();
        if (pOldList)
        {
            ScRangePairListRef xNewList(pOldList->Clone());

            ScRange aLabelRange;
            ScRange aDataRange;
            ScUnoConversion::FillScRange( aLabelRange, aLabelArea );
            ScUnoConversion::FillScRange( aDataRange,  aDataArea );
            xNewList->Join( ScRangePair( aLabelRange, aDataRange ) );

            if (bColumn)
                rDoc.GetColNameRangesRef() = xNewList;
            else
                rDoc.GetRowNameRangesRef() = xNewList;

            rDoc.CompileColRowNameFormula();
            pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID );
            pDocShell->SetDocumentModified();

            //! Undo ?!?! (here and from dialog)
        }
    }
}

void SAL_CALL ScLabelRangesObj::removeByIndex( sal_Int32 nIndex )
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScRangePairList* pOldList = bColumn ? rDoc.GetColNameRanges() : rDoc.GetRowNameRanges();

        if ( pOldList && nIndex >= 0 && nIndex < (sal_Int32)pOldList->size() )
        {
            ScRangePairListRef xNewList(pOldList->Clone());

            ScRangePair* pEntry = (*xNewList)[nIndex];
            if (pEntry)
            {
                xNewList->Remove( pEntry );
                delete pEntry;

                if (bColumn)
                    rDoc.GetColNameRangesRef() = xNewList;
                else
                    rDoc.GetRowNameRangesRef() = xNewList;

                rDoc.CompileColRowNameFormula();
                pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID );
                pDocShell->SetDocumentModified();
                bDone = true;

                //! Undo ?!?! (here and from dialog)
            }
        }
    }
    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

// container::XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScLabelRangesObj::createEnumeration()
                                                    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.sheet.LabelRangesEnumeration"));
}

// container::XIndexAccess

sal_Int32 SAL_CALL ScLabelRangesObj::getCount() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScRangePairList* pList = bColumn ? rDoc.GetColNameRanges() : rDoc.GetRowNameRanges();
        if (pList)
            return pList->size();
    }
    return 0;
}

uno::Any SAL_CALL ScLabelRangesObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference< sheet::XLabelRange >  xRange(GetObjectByIndex_Impl((sal_uInt16)nIndex));
    if ( xRange.is() )
        return uno::makeAny(xRange);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScLabelRangesObj::getElementType() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<sheet::XLabelRange>::get();   // must be suitable for getByIndex

}

sal_Bool SAL_CALL ScLabelRangesObj::hasElements() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
