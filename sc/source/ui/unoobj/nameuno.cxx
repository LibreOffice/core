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

//------------------------------------------------------------------------

static const SfxItemPropertyMapEntry* lcl_GetNamedRangeMap()
{
    static const SfxItemPropertyMapEntry aNamedRangeMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNO_LINKDISPBIT),      0,  &getCppuType((uno::Reference<awt::XBitmap>*)0), beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNO_LINKDISPNAME),     0,  &getCppuType((OUString*)0),                beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_TOKENINDEX),   0,  &getCppuType((sal_Int32*)0),                    beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ISSHAREDFMLA), 0,  &getBooleanCppuType(),                          0, 0 },
        {0,0,0,0,0,0}
    };
    return aNamedRangeMap_Impl;
}

static const SfxItemPropertyMapEntry* lcl_GetNamedRangesMap()
{
    static const SfxItemPropertyMapEntry aNamedRangesMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNO_MODIFY_BROADCAST), 0,  &getBooleanCppuType(), 0, 0 },
        {0,0,0,0,0,0}
    };
    return aNamedRangesMap_Impl;
}

//------------------------------------------------------------------------

#define SCNAMEDRANGEOBJ_SERVICE     "com.sun.star.sheet.NamedRange"

SC_SIMPLE_SERVICE_INFO( ScLabelRangeObj, "ScLabelRangeObj", "com.sun.star.sheet.LabelRange" )
SC_SIMPLE_SERVICE_INFO( ScLabelRangesObj, "ScLabelRangesObj", "com.sun.star.sheet.LabelRanges" )
SC_SIMPLE_SERVICE_INFO( ScNamedRangesObj, "ScNamedRangesObj", "com.sun.star.sheet.NamedRanges" )

static bool lcl_UserVisibleName(const ScRangeData& rData)
{
    //! als Methode an ScRangeData

    return !rData.HasType(RT_DATABASE);
}

ScNamedRangeObj::ScNamedRangeObj( rtl::Reference< ScNamedRangesObj > xParent, ScDocShell* pDocSh, const String& rNm, Reference<container::XNamed> xSheet):
    mxParent(xParent),
    pDocShell( pDocSh ),
    aName( rNm ),
    mxSheet( xSheet )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScNamedRangeObj::~ScNamedRangeObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScNamedRangeObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //  Ref-Update interessiert nicht

    if ( rHint.ISA( SfxSimpleHint ) && ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
        pDocShell = NULL;       // ungueltig geworden
}

// Hilfsfuntionen

ScRangeData* ScNamedRangeObj::GetRangeData_Impl()
{
    ScRangeData* pRet = NULL;
    if (pDocShell)
    {
        ScRangeName* pNames;
        SCTAB nTab = GetTab_Impl();
        if (nTab >= 0)
            pNames = pDocShell->GetDocument()->GetRangeName(nTab);
        else
            pNames = pDocShell->GetDocument()->GetRangeName();
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
        ScDocument* pDoc = pDocShell->GetDocument();
        SCTAB nTab;
        OUString sName = mxSheet->getName();
        pDoc->GetTable(sName, nTab);
        return nTab;
    }
    else
        return -1;//global range name
}

// sheet::XNamedRange

void ScNamedRangeObj::Modify_Impl( const String* pNewName, const ScTokenArray* pNewTokens, const String* pNewContent,
                                    const ScAddress* pNewPos, const sal_uInt16* pNewType,
                                    const formula::FormulaGrammar::Grammar eGrammar )
{
    if (!pDocShell)
        return;

    ScDocument* pDoc = pDocShell->GetDocument();
    ScRangeName* pNames;
    SCTAB nTab = GetTab_Impl();
    if (nTab >= 0)
        pNames = pDoc->GetRangeName(nTab);
    else
        pNames = pDoc->GetRangeName();
    if (!pNames)
        return;

    const ScRangeData* pOld = pNames->findByUpperName(ScGlobal::pCharClass->uppercase(aName));
    if (!pOld)
        return;

    ScRangeName* pNewRanges = new ScRangeName(*pNames);

    String aInsName = pOld->GetName();
    if (pNewName)
        aInsName = *pNewName;

    String aContent;                            // Content string based =>
    pOld->GetSymbol( aContent, eGrammar);   // no problems with changed positions and such.
    if (pNewContent)
        aContent = *pNewContent;

    ScAddress aPos = pOld->GetPos();
    if (pNewPos)
        aPos = *pNewPos;

    sal_uInt16 nType = pOld->GetType();
    if (pNewType)
        nType = *pNewType;

    ScRangeData* pNew = NULL;
    if (pNewTokens)
        pNew = new ScRangeData( pDoc, aInsName, *pNewTokens, aPos, nType );
    else
        pNew = new ScRangeData( pDoc, aInsName, aContent, aPos, nType, eGrammar );

    pNew->SetIndex( pOld->GetIndex() );

    pNewRanges->erase(*pOld);
    if (pNewRanges->insert(pNew))
    {
        pDocShell->GetDocFunc().SetNewRangeNames(pNewRanges, mxParent->IsModifyAndBroadcast(), nTab);

        aName = aInsName;   //! broadcast?
    }
    else
    {
        pNew = NULL;        //! uno::Exception/Fehler oder so
        delete pNewRanges;
    }
}


OUString SAL_CALL ScNamedRangeObj::getName() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return aName;
}

void SAL_CALL ScNamedRangeObj::setName( const OUString& aNewName )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    //! Formeln anpassen ?????

    String aNewStr(aNewName);
    // GRAM_PODF_A1 for API compatibility.
    Modify_Impl( &aNewStr, NULL, NULL, NULL, NULL,formula::FormulaGrammar::GRAM_PODF_A1 );

    if ( aName != aNewStr )                 // some error occurred...
        throw uno::RuntimeException();      // no other exceptions specified
}

OUString SAL_CALL ScNamedRangeObj::getContent() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
       String aContent;
    ScRangeData* pData = GetRangeData_Impl();
    if (pData)
        // GRAM_PODF_A1 for API compatibility.
        pData->GetSymbol( aContent,formula::FormulaGrammar::GRAM_PODF_A1);
    return aContent;
}

void SAL_CALL ScNamedRangeObj::setContent( const OUString& aContent )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aContStr(aContent);
    // GRAM_PODF_A1 for API compatibility.
    Modify_Impl( NULL, NULL, &aContStr, NULL, NULL,formula::FormulaGrammar::GRAM_PODF_A1 );
}

table::CellAddress SAL_CALL ScNamedRangeObj::getReferencePosition()
                                                throw(uno::RuntimeException)
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
        SCTAB nDocTabs = pDocShell->GetDocument()->GetTableCount();
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
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScAddress aPos( (SCCOL)aReferencePosition.Column, (SCROW)aReferencePosition.Row, aReferencePosition.Sheet );
    // GRAM_PODF_A1 for API compatibility.
    Modify_Impl( NULL, NULL, NULL, &aPos, NULL,formula::FormulaGrammar::GRAM_PODF_A1 );
}

sal_Int32 SAL_CALL ScNamedRangeObj::getType() throw(uno::RuntimeException)
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

void SAL_CALL ScNamedRangeObj::setType( sal_Int32 nUnoType ) throw(uno::RuntimeException)
{
    // see property 'IsSharedFormula' for RT_SHARED
    SolarMutexGuard aGuard;
    sal_uInt16 nNewType = RT_NAME;
    if ( nUnoType & sheet::NamedRangeFlag::FILTER_CRITERIA )    nNewType |= RT_CRITERIA;
    if ( nUnoType & sheet::NamedRangeFlag::PRINT_AREA )         nNewType |= RT_PRINTAREA;
    if ( nUnoType & sheet::NamedRangeFlag::COLUMN_HEADER )      nNewType |= RT_COLHEADER;
    if ( nUnoType & sheet::NamedRangeFlag::ROW_HEADER )         nNewType |= RT_ROWHEADER;

    // GRAM_PODF_A1 for API compatibility.
    Modify_Impl( NULL, NULL, NULL, NULL, &nNewType,formula::FormulaGrammar::GRAM_PODF_A1 );
}

// XFormulaTokens

uno::Sequence<sheet::FormulaToken> SAL_CALL ScNamedRangeObj::getTokens() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Sequence<sheet::FormulaToken> aSequence;
    ScRangeData* pData = GetRangeData_Impl();
    if (pData && pDocShell)
    {
        ScTokenArray* pTokenArray = pData->GetCode();
        if ( pTokenArray )
            (void)ScTokenConversion::ConvertToTokenSequence( *pDocShell->GetDocument(), aSequence, *pTokenArray );
    }
    return aSequence;
}

void SAL_CALL ScNamedRangeObj::setTokens( const uno::Sequence<sheet::FormulaToken>& rTokens ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if( pDocShell )
    {
        ScTokenArray aTokenArray;
        (void)ScTokenConversion::ConvertToTokenArray( *pDocShell->GetDocument(), aTokenArray, rTokens );
        // GRAM_PODF_A1 for API compatibility.
        Modify_Impl( NULL, &aTokenArray, NULL, NULL, NULL, formula::FormulaGrammar::GRAM_PODF_A1 );
    }
}


// XCellRangeSource

uno::Reference<table::XCellRange> SAL_CALL ScNamedRangeObj::getReferredCells()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScRange aRange;
    ScRangeData* pData = GetRangeData_Impl();
    if ( pData && pData->IsValidReference( aRange ) )
    {
        //! static Funktion um ScCellObj/ScCellRangeObj zu erzeugen am ScCellRangeObj ???

        if ( aRange.aStart == aRange.aEnd )
            return new ScCellObj( pDocShell, aRange.aStart );
        else
            return new ScCellRangeObj( pDocShell, aRange );
    }
    return NULL;
}

// beans::XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScNamedRangeObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    static uno::Reference< beans::XPropertySetInfo >  aRef(new SfxItemPropertySetInfo( lcl_GetNamedRangeMap() ));
    return aRef;
}

void SAL_CALL ScNamedRangeObj::setPropertyValue(
                        const OUString& rPropertyName, const uno::Any& /*aValue*/ )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( rPropertyName == SC_UNONAME_ISSHAREDFMLA )
    {
        // Ignore this.
    }
}

uno::Any SAL_CALL ScNamedRangeObj::getPropertyValue( const OUString& rPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
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

OUString SAL_CALL ScNamedRangeObj::getImplementationName() throw(uno::RuntimeException)
{
    return OUString( "ScNamedRangeObj" );
}

sal_Bool SAL_CALL ScNamedRangeObj::supportsService( const OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    return rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SCNAMEDRANGEOBJ_SERVICE ) ) ||
           rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SCLINKTARGET_SERVICE ) );
}

uno::Sequence<OUString> SAL_CALL ScNamedRangeObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence<OUString> aRet(2);
    aRet[0] = OUString( SCNAMEDRANGEOBJ_SERVICE );
    aRet[1] = OUString( SCLINKTARGET_SERVICE );
    return aRet;
}


// XUnoTunnel

sal_Int64 SAL_CALL ScNamedRangeObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
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

//------------------------------------------------------------------------

ScNamedRangesObj::ScNamedRangesObj(ScDocShell* pDocSh) :
    mbModifyAndBroadcast(true),
    pDocShell( pDocSh )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScNamedRangesObj::~ScNamedRangesObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScNamedRangesObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //  Referenz-Update interessiert hier nicht

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}

bool ScNamedRangesObj::IsModifyAndBroadcast() const
{
    return mbModifyAndBroadcast;
}

// sheet::XNamedRanges

void SAL_CALL ScNamedRangesObj::addNewByName( const OUString& aName,
        const OUString& aContent, const table::CellAddress& aPosition,
        sal_Int32 nUnoType ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScAddress aPos( (SCCOL)aPosition.Column, (SCROW)aPosition.Row, aPosition.Sheet );

    sal_uInt16 nNewType = RT_NAME;
    if ( nUnoType & sheet::NamedRangeFlag::FILTER_CRITERIA )    nNewType |= RT_CRITERIA;
    if ( nUnoType & sheet::NamedRangeFlag::PRINT_AREA )         nNewType |= RT_PRINTAREA;
    if ( nUnoType & sheet::NamedRangeFlag::COLUMN_HEADER )      nNewType |= RT_COLHEADER;
    if ( nUnoType & sheet::NamedRangeFlag::ROW_HEADER )         nNewType |= RT_ROWHEADER;

    sal_Bool bDone = false;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScRangeName* pNames = GetRangeName_Impl();
        if (pNames && !pNames->findByUpperName(ScGlobal::pCharClass->uppercase(aName)))
        {
            ScRangeName* pNewRanges = new ScRangeName( *pNames );
            // GRAM_PODF_A1 for API compatibility.
            ScRangeData* pNew = new ScRangeData( pDoc, aName, aContent,
                                                aPos, nNewType,formula::FormulaGrammar::GRAM_PODF_A1 );
            if ( pNewRanges->insert(pNew) )
            {
                pDocShell->GetDocFunc().SetNewRangeNames(pNewRanges, mbModifyAndBroadcast, GetTab_Impl());
                bDone = true;
            }
            else
            {
                pNew = NULL;
                delete pNewRanges;
            }
        }
    }

    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

void SAL_CALL ScNamedRangesObj::addNewFromTitles( const table::CellRangeAddress& aSource,
                                    sheet::Border aBorder ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    //! das darf kein enum sein, weil mehrere Bits gesetzt sein koennen !!!

    sal_Bool bTop    = ( aBorder == sheet::Border_TOP );
    sal_Bool bLeft   = ( aBorder == sheet::Border_LEFT );
    sal_Bool bBottom = ( aBorder == sheet::Border_BOTTOM );
    sal_Bool bRight  = ( aBorder == sheet::Border_RIGHT );

    ScRange aRange;
    ScUnoConversion::FillScRange( aRange, aSource );

    sal_uInt16 nFlags = 0;
    if (bTop)    nFlags |= NAME_TOP;
    if (bLeft)   nFlags |= NAME_LEFT;
    if (bBottom) nFlags |= NAME_BOTTOM;
    if (bRight)  nFlags |= NAME_RIGHT;

    if (nFlags)
        pDocShell->GetDocFunc().CreateNames( aRange, nFlags, sal_True, GetTab_Impl() );
}

void SAL_CALL ScNamedRangesObj::removeByName( const OUString& aName )
                                                throw(uno::RuntimeException)
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
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScAddress aPos( (SCCOL)aOutputPosition.Column, (SCROW)aOutputPosition.Row, aOutputPosition.Sheet );
    if (pDocShell)
        pDocShell->GetDocFunc().InsertNameList( aPos, sal_True );
}

// container::XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScNamedRangesObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.sheet.NamedRangesEnumeration"));
}

// container::XIndexAccess

sal_Int32 SAL_CALL ScNamedRangesObj::getCount() throw(uno::RuntimeException)
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
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference< sheet::XNamedRange >  xRange(GetObjectByIndex_Impl((sal_uInt16)nIndex));
    if ( xRange.is() )
        return uno::makeAny(xRange);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScNamedRangesObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ::getCppuType((const uno::Reference< sheet::XNamedRange >*)0);   // muss zu getByIndex passen
}

sal_Bool SAL_CALL ScNamedRangesObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

Reference<beans::XPropertySetInfo> SAL_CALL ScNamedRangesObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    static Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo(lcl_GetNamedRangesMap()));
    return aRef;
}

void SAL_CALL ScNamedRangesObj::setPropertyValue(
                        const OUString& rPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    if ( rPropertyName == SC_UNO_MODIFY_BROADCAST )
    {
        aValue >>= mbModifyAndBroadcast;
    }
}

Any SAL_CALL ScNamedRangesObj::getPropertyValue( const OUString& rPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
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
                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference< sheet::XNamedRange >  xRange(GetObjectByName_Impl(aName));
    if ( xRange.is() )
        return uno::makeAny(xRange);
    else
        throw container::NoSuchElementException();
}

uno::Sequence<OUString> SAL_CALL ScNamedRangesObj::getElementNames()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScRangeName* pNames = GetRangeName_Impl();
        if (pNames)
        {
            long nVisCount = getCount();            // Namen mit lcl_UserVisibleName
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
                                        throw(uno::RuntimeException)
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
    pDocShell->GetDocument()->CompileNameFormula( sal_True ); // CreateFormulaString
}

/** called from the XActionLockable interface methods on final unlock */
void ScNamedRangesObj::unlock()
{
    pDocShell->GetDocument()->CompileNameFormula( false ); // CompileFormulaString
}

// document::XActionLockable

sal_Bool ScNamedRangesObj::isActionLocked() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return pDocShell->GetDocument()->GetNamedRangesLockCount() != 0;
}

void ScNamedRangesObj::addActionLock() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScDocument* pDoc = pDocShell->GetDocument();
    sal_Int16 nLockCount = pDoc->GetNamedRangesLockCount();
    ++nLockCount;
    if ( nLockCount == 1 )
    {
        lock();
    }
    pDoc->SetNamedRangesLockCount( nLockCount );
}

void ScNamedRangesObj::removeActionLock() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScDocument* pDoc = pDocShell->GetDocument();
    sal_Int16 nLockCount = pDoc->GetNamedRangesLockCount();
    if ( nLockCount > 0 )
    {
        --nLockCount;
        if ( nLockCount == 0 )
        {
            unlock();
        }
        pDoc->SetNamedRangesLockCount( nLockCount );
    }
}

void ScNamedRangesObj::setActionLocks( sal_Int16 nLock ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( nLock >= 0 )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        sal_Int16 nLockCount = pDoc->GetNamedRangesLockCount();
        if ( nLock == 0 && nLockCount > 0 )
        {
            unlock();
        }
        if ( nLock > 0 && nLockCount == 0 )
        {
            lock();
        }
        pDoc->SetNamedRangesLockCount( nLock );
    }
}

sal_Int16 ScNamedRangesObj::resetActionLocks() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScDocument* pDoc = pDocShell->GetDocument();
    sal_Int16 nLockCount = pDoc->GetNamedRangesLockCount();
    if ( nLockCount > 0 )
    {
        unlock();
    }
    pDoc->SetNamedRangesLockCount( 0 );
    return nLockCount;
}

//------------------------------------------------------------------------

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
        return NULL;

    ScRangeName* pNames = pDocShell->GetDocument()->GetRangeName();
    if (!pNames)
        return NULL;

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
    return NULL;
}

ScNamedRangeObj* ScGlobalNamedRangesObj::GetObjectByName_Impl(const OUString& aName)
{
    if ( pDocShell && hasByName(aName) )
        return new ScNamedRangeObj(this, pDocShell, String(aName));
    return NULL;
}

ScRangeName* ScGlobalNamedRangesObj::GetRangeName_Impl()
{
    return pDocShell->GetDocument()->GetRangeName();
}

SCTAB ScGlobalNamedRangesObj::GetTab_Impl()
{
    return -1;
}

//------------------------------------------------------------------------

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
        return new ScNamedRangeObj( this, pDocShell, String(aName), mxSheet);
    return NULL;

}

ScNamedRangeObj* ScLocalNamedRangesObj::GetObjectByIndex_Impl( sal_uInt16 nIndex )
{
    if (!pDocShell)
        return NULL;

    OUString aName = mxSheet->getName();
    ScDocument* pDoc = pDocShell->GetDocument();
    SCTAB nTab;
    pDoc->GetTable( aName, nTab );

    ScRangeName* pNames = pDoc->GetRangeName( nTab );
    if (!pNames)
        return NULL;

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
    return NULL;
}

ScRangeName* ScLocalNamedRangesObj::GetRangeName_Impl()
{
    SCTAB nTab = GetTab_Impl();
    return pDocShell->GetDocument()->GetRangeName( nTab );
}

SCTAB ScLocalNamedRangesObj::GetTab_Impl()
{
    SCTAB nTab;
    pDocShell->GetDocument()->GetTable(mxSheet->getName(), nTab);
    return nTab;
}

//------------------------------------------------------------------------

ScLabelRangeObj::ScLabelRangeObj(ScDocShell* pDocSh, sal_Bool bCol, const ScRange& rR) :
    pDocShell( pDocSh ),
    bColumn( bCol ),
    aRange( rR )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScLabelRangeObj::~ScLabelRangeObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScLabelRangeObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //! Ref-Update !!!

    if ( rHint.ISA( SfxSimpleHint ) && ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
        pDocShell = NULL;       // ungueltig geworden
}

// Hilfsfuntionen

ScRangePair* ScLabelRangeObj::GetData_Impl()
{
    ScRangePair* pRet = NULL;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScRangePairList* pList = bColumn ? pDoc->GetColNameRanges() : pDoc->GetRowNameRanges();
        if (pList)
            pRet = pList->Find( aRange );
    }
    return pRet;
}

void ScLabelRangeObj::Modify_Impl( const ScRange* pLabel, const ScRange* pData )
{
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScRangePairList* pOldList = bColumn ? pDoc->GetColNameRanges() : pDoc->GetRowNameRanges();
        if (pOldList)
        {
            ScRangePairListRef xNewList(pOldList->Clone());
            ScRangePair* pEntry = xNewList->Find( aRange );
            if (pEntry)
            {
                xNewList->Remove( pEntry );     // nur aus der Liste entfernt, nicht geloescht

                if ( pLabel )
                    pEntry->GetRange(0) = *pLabel;
                if ( pData )
                    pEntry->GetRange(1) = *pData;

                xNewList->Join( *pEntry );
                delete pEntry;

                if (bColumn)
                    pDoc->GetColNameRangesRef() = xNewList;
                else
                    pDoc->GetRowNameRangesRef() = xNewList;

                pDoc->CompileColRowNameFormula();
                pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID );
                pDocShell->SetDocumentModified();

                //! Undo ?!?! (hier und aus Dialog)

                if ( pLabel )
                    aRange = *pLabel;   // Objekt anpassen, um Range wiederzufinden
            }
        }
    }
}

// sheet::XLabelRange

table::CellRangeAddress SAL_CALL ScLabelRangeObj::getLabelArea()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    table::CellRangeAddress aRet;
    ScRangePair* pData = GetData_Impl();
    if (pData)
        ScUnoConversion::FillApiRange( aRet, pData->GetRange(0) );
    return aRet;
}

void SAL_CALL ScLabelRangeObj::setLabelArea( const table::CellRangeAddress& aLabelArea )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScRange aLabelRange;
    ScUnoConversion::FillScRange( aLabelRange, aLabelArea );
    Modify_Impl( &aLabelRange, NULL );
}

table::CellRangeAddress SAL_CALL ScLabelRangeObj::getDataArea()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    table::CellRangeAddress aRet;
    ScRangePair* pData = GetData_Impl();
    if (pData)
        ScUnoConversion::FillApiRange( aRet, pData->GetRange(1) );
    return aRet;
}

void SAL_CALL ScLabelRangeObj::setDataArea( const table::CellRangeAddress& aDataArea )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScRange aDataRange;
    ScUnoConversion::FillScRange( aDataRange, aDataArea );
    Modify_Impl( NULL, &aDataRange );
}

//------------------------------------------------------------------------

ScLabelRangesObj::ScLabelRangesObj(ScDocShell* pDocSh, sal_Bool bCol) :
    pDocShell( pDocSh ),
    bColumn( bCol )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScLabelRangesObj::~ScLabelRangesObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScLabelRangesObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //  Referenz-Update interessiert hier nicht

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}

// sheet::XLabelRanges

ScLabelRangeObj* ScLabelRangesObj::GetObjectByIndex_Impl(size_t nIndex)
{
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScRangePairList* pList = bColumn ? pDoc->GetColNameRanges() : pDoc->GetRowNameRanges();
        if ( pList && nIndex < pList->size() )
        {
            ScRangePair* pData = (*pList)[nIndex];
            if (pData)
                return new ScLabelRangeObj( pDocShell, bColumn, pData->GetRange(0) );
        }
    }
    return NULL;
}

void SAL_CALL ScLabelRangesObj::addNew( const table::CellRangeAddress& aLabelArea,
                                const table::CellRangeAddress& aDataArea )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScRangePairList* pOldList = bColumn ? pDoc->GetColNameRanges() : pDoc->GetRowNameRanges();
        if (pOldList)
        {
            ScRangePairListRef xNewList(pOldList->Clone());

            ScRange aLabelRange;
            ScRange aDataRange;
            ScUnoConversion::FillScRange( aLabelRange, aLabelArea );
            ScUnoConversion::FillScRange( aDataRange,  aDataArea );
            xNewList->Join( ScRangePair( aLabelRange, aDataRange ) );

            if (bColumn)
                pDoc->GetColNameRangesRef() = xNewList;
            else
                pDoc->GetRowNameRangesRef() = xNewList;

            pDoc->CompileColRowNameFormula();
            pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID );
            pDocShell->SetDocumentModified();

            //! Undo ?!?! (hier und aus Dialog)
        }
    }
}

void SAL_CALL ScLabelRangesObj::removeByIndex( sal_Int32 nIndex )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Bool bDone = false;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScRangePairList* pOldList = bColumn ? pDoc->GetColNameRanges() : pDoc->GetRowNameRanges();

        if ( pOldList && nIndex >= 0 && nIndex < (sal_Int32)pOldList->size() )
        {
            ScRangePairListRef xNewList(pOldList->Clone());

            ScRangePair* pEntry = (*xNewList)[nIndex];
            if (pEntry)
            {
                xNewList->Remove( pEntry );
                delete pEntry;

                if (bColumn)
                    pDoc->GetColNameRangesRef() = xNewList;
                else
                    pDoc->GetRowNameRangesRef() = xNewList;

                pDoc->CompileColRowNameFormula();
                pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID );
                pDocShell->SetDocumentModified();
                bDone = sal_True;

                //! Undo ?!?! (hier und aus Dialog)
            }
        }
    }
    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

// container::XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScLabelRangesObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.sheet.LabelRangesEnumeration"));
}

// container::XIndexAccess

sal_Int32 SAL_CALL ScLabelRangesObj::getCount() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScRangePairList* pList = bColumn ? pDoc->GetColNameRanges() : pDoc->GetRowNameRanges();
        if (pList)
            return pList->size();
    }
    return 0;
}

uno::Any SAL_CALL ScLabelRangesObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference< sheet::XLabelRange >  xRange(GetObjectByIndex_Impl((sal_uInt16)nIndex));
    if ( xRange.is() )
        return uno::makeAny(xRange);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScLabelRangesObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ::getCppuType((const uno::Reference< sheet::XLabelRange >*)0);   // muss zu getByIndex passen

}

sal_Bool SAL_CALL ScLabelRangesObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

//------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
