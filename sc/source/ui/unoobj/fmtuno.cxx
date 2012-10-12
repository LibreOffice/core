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


#include <boost/bind.hpp>

#include <vcl/svapp.hxx>
#include <comphelper/servicehelper.hxx>

#include <com/sun/star/sheet/ValidationAlertStyle.hpp>
#include <com/sun/star/sheet/ValidationType.hpp>
#include <com/sun/star/sheet/TableValidationVisibility.hpp>

#include "fmtuno.hxx"
#include "miscuno.hxx"
#include "validat.hxx"
#include "document.hxx"
#include "unonames.hxx"
#include "styleuno.hxx"     // ScStyleNameConversion
#include "tokenarray.hxx"
#include "tokenuno.hxx"

using namespace ::com::sun::star;
using namespace ::formula;

//------------------------------------------------------------------------

//  Map nur fuer PropertySetInfo

static const SfxItemPropertyMapEntry* lcl_GetValidatePropertyMap()
{
    static SfxItemPropertyMapEntry aValidatePropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_ERRALSTY), 0,  &getCppuType((sheet::ValidationAlertStyle*)0),  0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_ERRMESS),  0,  &getCppuType((rtl::OUString*)0),                0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_ERRTITLE), 0,  &getCppuType((rtl::OUString*)0),                0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_IGNOREBL), 0,  &getBooleanCppuType(),                          0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_INPMESS),  0,  &getCppuType((rtl::OUString*)0),                0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_INPTITLE), 0,  &getCppuType((rtl::OUString*)0),                0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_SHOWERR),  0,  &getBooleanCppuType(),                          0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_SHOWINP),  0,  &getBooleanCppuType(),                          0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_SHOWLIST), 0,  &getCppuType((sal_Int16*)0),                    0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_TYPE),     0,  &getCppuType((sheet::ValidationType*)0),        0, 0},
        {0,0,0,0,0,0}
    };
    return aValidatePropertyMap_Impl;
}

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScTableConditionalEntry, "ScTableConditionalEntry", "com.sun.star.sheet.TableConditionalEntry" )
SC_SIMPLE_SERVICE_INFO( ScTableConditionalFormat, "ScTableConditionalFormat", "com.sun.star.sheet.TableConditionalFormat" )
SC_SIMPLE_SERVICE_INFO( ScTableValidationObj, "ScTableValidationObj", "com.sun.star.sheet.TableValidation" )

//------------------------------------------------------------------------

static sal_Int32 lcl_ConditionModeToOperatorNew( ScConditionMode eMode )
{
    sal_Int32 eOper = sheet::ConditionOperator2::NONE;
    switch (eMode)
    {
        case SC_COND_EQUAL:         eOper = sheet::ConditionOperator2::EQUAL;           break;
        case SC_COND_LESS:          eOper = sheet::ConditionOperator2::LESS;            break;
        case SC_COND_GREATER:       eOper = sheet::ConditionOperator2::GREATER;         break;
        case SC_COND_EQLESS:        eOper = sheet::ConditionOperator2::LESS_EQUAL;      break;
        case SC_COND_EQGREATER:     eOper = sheet::ConditionOperator2::GREATER_EQUAL;   break;
        case SC_COND_NOTEQUAL:      eOper = sheet::ConditionOperator2::NOT_EQUAL;       break;
        case SC_COND_BETWEEN:       eOper = sheet::ConditionOperator2::BETWEEN;         break;
        case SC_COND_NOTBETWEEN:    eOper = sheet::ConditionOperator2::NOT_BETWEEN;     break;
        case SC_COND_DIRECT:        eOper = sheet::ConditionOperator2::FORMULA;         break;
        case SC_COND_DUPLICATE:     eOper = sheet::ConditionOperator2::DUPLICATE;       break;
        default:
        {
            // added to avoid warnings
        }
    }
    return eOper;
}

static sheet::ConditionOperator lcl_ConditionModeToOperator( ScConditionMode eMode )
{
    sheet::ConditionOperator eOper = sheet::ConditionOperator_NONE;
    switch (eMode)
    {
        case SC_COND_EQUAL:         eOper = sheet::ConditionOperator_EQUAL;         break;
        case SC_COND_LESS:          eOper = sheet::ConditionOperator_LESS;          break;
        case SC_COND_GREATER:       eOper = sheet::ConditionOperator_GREATER;       break;
        case SC_COND_EQLESS:        eOper = sheet::ConditionOperator_LESS_EQUAL;    break;
        case SC_COND_EQGREATER:     eOper = sheet::ConditionOperator_GREATER_EQUAL; break;
        case SC_COND_NOTEQUAL:      eOper = sheet::ConditionOperator_NOT_EQUAL;     break;
        case SC_COND_BETWEEN:       eOper = sheet::ConditionOperator_BETWEEN;       break;
        case SC_COND_NOTBETWEEN:    eOper = sheet::ConditionOperator_NOT_BETWEEN;   break;
        case SC_COND_DIRECT:        eOper = sheet::ConditionOperator_FORMULA;       break;
        default:
        {
            // added to avoid warnings
        }
    }
    return eOper;
}

static ScConditionMode lcl_ConditionOperatorToMode( sheet::ConditionOperator eOper )
{
    ScConditionMode eMode = SC_COND_NONE;
    switch (eOper)
    {
        case sheet::ConditionOperator_EQUAL:            eMode = SC_COND_EQUAL;      break;
        case sheet::ConditionOperator_LESS:             eMode = SC_COND_LESS;       break;
        case sheet::ConditionOperator_GREATER:          eMode = SC_COND_GREATER;    break;
        case sheet::ConditionOperator_LESS_EQUAL:       eMode = SC_COND_EQLESS;     break;
        case sheet::ConditionOperator_GREATER_EQUAL:    eMode = SC_COND_EQGREATER;  break;
        case sheet::ConditionOperator_NOT_EQUAL:        eMode = SC_COND_NOTEQUAL;   break;
        case sheet::ConditionOperator_BETWEEN:          eMode = SC_COND_BETWEEN;    break;
        case sheet::ConditionOperator_NOT_BETWEEN:      eMode = SC_COND_NOTBETWEEN; break;
        case sheet::ConditionOperator_FORMULA:          eMode = SC_COND_DIRECT;     break;
        default:
        {
            // added to avoid warnings
        }
    }
    return eMode;
}

//------------------------------------------------------------------------

ScCondFormatEntryItem::ScCondFormatEntryItem() :
    meGrammar1( FormulaGrammar::GRAM_UNSPECIFIED ),
    meGrammar2( FormulaGrammar::GRAM_UNSPECIFIED ),
    meMode( SC_COND_NONE )
{
}

//------------------------------------------------------------------------

ScTableConditionalFormat::ScTableConditionalFormat(
        ScDocument* pDoc, sal_uLong nKey, SCTAB nTab, FormulaGrammar::Grammar eGrammar)
{
    //  Eintrag aus dem Dokument lesen...

    if ( pDoc && nKey )
    {
        ScConditionalFormatList* pList = pDoc->GetCondFormList(nTab);
        if (pList)
        {
            const ScConditionalFormat* pFormat = pList->GetFormat( nKey );
            if (pFormat)
            {
                // During save to XML.
                if (pDoc->IsInExternalReferenceMarking())
                    pFormat->MarkUsedExternalReferences();

                size_t nEntryCount = pFormat->size();
                for (size_t i=0; i<nEntryCount; i++)
                {
                    ScCondFormatEntryItem aItem;
                    const ScFormatEntry* pFrmtEntry = pFormat->GetEntry(i);
                    if(pFrmtEntry->GetType() != condformat::CONDITION)
                        continue;

                    const ScCondFormatEntry* pFormatEntry = static_cast<const ScCondFormatEntry*>(pFrmtEntry);
                    aItem.meMode = pFormatEntry->GetOperation();
                    aItem.maPos = pFormatEntry->GetValidSrcPos();
                    aItem.maExpr1 = pFormatEntry->GetExpression(aItem.maPos, 0, 0, eGrammar);
                    aItem.maExpr2 = pFormatEntry->GetExpression(aItem.maPos, 1, 0, eGrammar);
                    aItem.meGrammar1 = aItem.meGrammar2 = eGrammar;
                    aItem.maStyle = pFormatEntry->GetStyle();

                    AddEntry_Impl(aItem);
                }
            }
        }
    }
}

namespace {

FormulaGrammar::Grammar lclResolveGrammar( FormulaGrammar::Grammar eExtGrammar, FormulaGrammar::Grammar eIntGrammar )
{
    if( eExtGrammar != FormulaGrammar::GRAM_UNSPECIFIED )
        return eExtGrammar;
    OSL_ENSURE( eIntGrammar != FormulaGrammar::GRAM_UNSPECIFIED, "lclResolveGrammar - unspecified grammar, using GRAM_PODF_A1" );
    return (eIntGrammar == FormulaGrammar::GRAM_UNSPECIFIED) ? FormulaGrammar::GRAM_PODF_A1 : eIntGrammar;
}

} // namespace

void ScTableConditionalFormat::FillFormat( ScConditionalFormat& rFormat,
        ScDocument* pDoc, FormulaGrammar::Grammar eGrammar) const
{
    //  ScConditionalFormat = Core-Struktur, muss leer sein

    OSL_ENSURE( rFormat.IsEmpty(), "FillFormat: Format nicht leer" );

    std::vector<ScTableConditionalEntry*>::const_iterator iter;
    for (iter = aEntries.begin(); iter != aEntries.end(); ++iter)
    {
        ScCondFormatEntryItem aData;
        (*iter)->GetData(aData);

        FormulaGrammar::Grammar eGrammar1 = lclResolveGrammar( eGrammar, aData.meGrammar1 );
        FormulaGrammar::Grammar eGrammar2 = lclResolveGrammar( eGrammar, aData.meGrammar2 );

        ScCondFormatEntry* pCoreEntry = new ScCondFormatEntry( aData.meMode, aData.maExpr1, aData.maExpr2,
            pDoc, aData.maPos, aData.maStyle, aData.maExprNmsp1, aData.maExprNmsp2, eGrammar1, eGrammar2 );

        if ( aData.maPosStr.Len() )
            pCoreEntry->SetSrcString( aData.maPosStr );

        if ( aData.maTokens1.getLength() )
        {
            ScTokenArray aTokenArray;
            if ( ScTokenConversion::ConvertToTokenArray(*pDoc, aTokenArray, aData.maTokens1) )
                pCoreEntry->SetFormula1(aTokenArray);
        }

        if ( aData.maTokens2.getLength() )
        {
            ScTokenArray aTokenArray;
            if ( ScTokenConversion::ConvertToTokenArray(*pDoc, aTokenArray, aData.maTokens2) )
                pCoreEntry->SetFormula2(aTokenArray);
        }
        rFormat.AddEntry( pCoreEntry );
    }
}

ScTableConditionalFormat::~ScTableConditionalFormat()
{
    std::for_each(aEntries.begin(),aEntries.end(),boost::bind(&ScTableConditionalEntry::release,_1));
}

void ScTableConditionalFormat::AddEntry_Impl(const ScCondFormatEntryItem& aEntry)
{
    ScTableConditionalEntry* pNew = new ScTableConditionalEntry(aEntry);
    pNew->acquire();
    aEntries.push_back(pNew);
}

// XSheetConditionalFormat

ScTableConditionalEntry* ScTableConditionalFormat::GetObjectByIndex_Impl(sal_uInt16 nIndex) const
{
    return nIndex < aEntries.size() ? aEntries[nIndex] : NULL;
}

void SAL_CALL ScTableConditionalFormat::addNew(
                    const uno::Sequence<beans::PropertyValue >& aConditionalEntry )
                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScCondFormatEntryItem aEntry;
    aEntry.meMode = SC_COND_NONE;

    const beans::PropertyValue* pPropArray = aConditionalEntry.getConstArray();
    long nPropCount = aConditionalEntry.getLength();
    for (long i = 0; i < nPropCount; i++)
    {
        const beans::PropertyValue& rProp = pPropArray[i];

        if ( rProp.Name == SC_UNONAME_OPERATOR )
        {
            sal_Int32 eOper = ScUnoHelpFunctions::GetEnumFromAny( rProp.Value );
            aEntry.meMode = ScConditionEntry::GetModeFromApi( eOper );
        }
        else if ( rProp.Name == SC_UNONAME_FORMULA1 )
        {
            rtl::OUString aStrVal;
            uno::Sequence<sheet::FormulaToken> aTokens;
            if ( rProp.Value >>= aStrVal )
                aEntry.maExpr1 = aStrVal;
            else if ( rProp.Value >>= aTokens )
            {
                aEntry.maExpr1.Erase();
                aEntry.maTokens1 = aTokens;
            }
        }
        else if ( rProp.Name == SC_UNONAME_FORMULA2 )
        {
            rtl::OUString aStrVal;
            uno::Sequence<sheet::FormulaToken> aTokens;
            if ( rProp.Value >>= aStrVal )
                aEntry.maExpr2 = aStrVal;
            else if ( rProp.Value >>= aTokens )
            {
                aEntry.maExpr2.Erase();
                aEntry.maTokens2 = aTokens;
            }
        }
        else if ( rProp.Name == SC_UNONAME_SOURCEPOS )
        {
            table::CellAddress aAddress;
            if ( rProp.Value >>= aAddress )
                aEntry.maPos = ScAddress( (SCCOL)aAddress.Column, (SCROW)aAddress.Row, aAddress.Sheet );
        }
        else if ( rProp.Name == SC_UNONAME_SOURCESTR )
        {
            rtl::OUString aStrVal;
            if ( rProp.Value >>= aStrVal )
                aEntry.maPosStr = String( aStrVal );
        }
        else if ( rProp.Name == SC_UNONAME_STYLENAME )
        {
            rtl::OUString aStrVal;
            if ( rProp.Value >>= aStrVal )
                aEntry.maStyle = ScStyleNameConversion::ProgrammaticToDisplayName(
                                                aStrVal, SFX_STYLE_FAMILY_PARA );
        }
        else if ( rProp.Name == SC_UNONAME_FORMULANMSP1 )
        {
            rtl::OUString aStrVal;
            if ( rProp.Value >>= aStrVal )
                aEntry.maExprNmsp1 = aStrVal;
        }
        else if ( rProp.Name == SC_UNONAME_FORMULANMSP2 )
        {
            rtl::OUString aStrVal;
            if ( rProp.Value >>= aStrVal )
                aEntry.maExprNmsp2 = aStrVal;
        }
        else if ( rProp.Name == SC_UNONAME_GRAMMAR1 )
        {
            sal_Int32 nVal = 0;
            if ( rProp.Value >>= nVal )
                aEntry.meGrammar1 = static_cast< FormulaGrammar::Grammar >( nVal );
        }
        else if ( rProp.Name == SC_UNONAME_GRAMMAR2 )
        {
            sal_Int32 nVal = 0;
            if ( rProp.Value >>= nVal )
                aEntry.meGrammar2 = static_cast< FormulaGrammar::Grammar >( nVal );
        }
        else
        {
            OSL_FAIL("falsche Property");
            //! Exception...
        }
    }

    AddEntry_Impl(aEntry);
}

void SAL_CALL ScTableConditionalFormat::removeByIndex( sal_Int32 nIndex )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (nIndex < static_cast<sal_Int32>(aEntries.size()))
    {
        std::vector<ScTableConditionalEntry*>::iterator iter = aEntries.begin()+nIndex;

        (*iter)->release();
        aEntries.erase(iter);
    }
}

void SAL_CALL ScTableConditionalFormat::clear() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    std::for_each(aEntries.begin(),aEntries.end(),
                  boost::bind(&ScTableConditionalEntry::release,_1));

    aEntries.clear();
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScTableConditionalFormat::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.TableConditionalEntryEnumeration")));
}

// XIndexAccess

sal_Int32 SAL_CALL ScTableConditionalFormat::getCount() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return aEntries.size();
}

uno::Any SAL_CALL ScTableConditionalFormat::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XSheetConditionalEntry> xEntry(GetObjectByIndex_Impl((sal_uInt16)nIndex));
    if (xEntry.is())
        return uno::makeAny(xEntry);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScTableConditionalFormat::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType((uno::Reference<sheet::XSheetConditionalEntry>*)0);
}

sal_Bool SAL_CALL ScTableConditionalFormat::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

//  conditional format entries have no real names
//  -> generate name from index

static rtl::OUString lcl_GetEntryNameFromIndex( sal_Int32 nIndex )
{
    rtl::OUString aRet( RTL_CONSTASCII_USTRINGPARAM( "Entry" ) );
    aRet += rtl::OUString::valueOf( nIndex );
    return aRet;
}

uno::Any SAL_CALL ScTableConditionalFormat::getByName( const rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Reference<sheet::XSheetConditionalEntry> xEntry;
    long nCount = aEntries.size();
    for (long i=0; i<nCount; i++)
        if ( aName == lcl_GetEntryNameFromIndex(i) )
        {
            xEntry.set(GetObjectByIndex_Impl((sal_uInt16)i));
            break;
        }

    if (xEntry.is())
        return uno::makeAny(xEntry);
    else
        throw container::NoSuchElementException();
}

uno::Sequence<rtl::OUString> SAL_CALL ScTableConditionalFormat::getElementNames()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    long nCount = aEntries.size();
    uno::Sequence<rtl::OUString> aNames(nCount);
    rtl::OUString* pArray = aNames.getArray();
    for (long i=0; i<nCount; i++)
        pArray[i] = lcl_GetEntryNameFromIndex(i);

    return aNames;
}

sal_Bool SAL_CALL ScTableConditionalFormat::hasByName( const rtl::OUString& aName )
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    long nCount = aEntries.size();
    for (long i=0; i<nCount; i++)
        if ( aName == lcl_GetEntryNameFromIndex(i) )
            return sal_True;

    return false;
}

// XUnoTunnel

sal_Int64 SAL_CALL ScTableConditionalFormat::getSomething(
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
    class theScTableConditionalFormatUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScTableConditionalFormatUnoTunnelId> {};
}

const uno::Sequence<sal_Int8>& ScTableConditionalFormat::getUnoTunnelId()
{
    return theScTableConditionalFormatUnoTunnelId::get().getSeq();
}

ScTableConditionalFormat* ScTableConditionalFormat::getImplementation(
                                const uno::Reference<sheet::XSheetConditionalEntries> xObj )
{
    ScTableConditionalFormat* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = reinterpret_cast<ScTableConditionalFormat*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}

//------------------------------------------------------------------------

ScTableConditionalEntry::ScTableConditionalEntry(const ScCondFormatEntryItem& aItem) :
    aData( aItem )
{
    // #i113668# only store the settings, keep no reference to parent object
}

ScTableConditionalEntry::~ScTableConditionalEntry()
{
}

void ScTableConditionalEntry::GetData(ScCondFormatEntryItem& rData) const
{
    rData = aData;
}

// XSheetCondition

sheet::ConditionOperator SAL_CALL ScTableConditionalEntry::getOperator()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return lcl_ConditionModeToOperator( aData.meMode );
}

void SAL_CALL ScTableConditionalEntry::setOperator( sheet::ConditionOperator nOperator )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    aData.meMode = lcl_ConditionOperatorToMode( nOperator );
}

sal_Int32 SAL_CALL ScTableConditionalEntry::getConditionOperator()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return lcl_ConditionModeToOperatorNew( aData.meMode );
}

void SAL_CALL ScTableConditionalEntry::setConditionOperator( sal_Int32 nOperator )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    aData.meMode = ScConditionEntry::GetModeFromApi( nOperator );
}

rtl::OUString SAL_CALL ScTableConditionalEntry::getFormula1() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return aData.maExpr1;
}

void SAL_CALL ScTableConditionalEntry::setFormula1( const rtl::OUString& aFormula1 )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    aData.maExpr1 = String( aFormula1 );
}

rtl::OUString SAL_CALL ScTableConditionalEntry::getFormula2() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return aData.maExpr2;
}

void SAL_CALL ScTableConditionalEntry::setFormula2( const rtl::OUString& aFormula2 )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    aData.maExpr2 = String( aFormula2 );
}

table::CellAddress SAL_CALL ScTableConditionalEntry::getSourcePosition() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    table::CellAddress aRet;
    aRet.Column = aData.maPos.Col();
    aRet.Row    = aData.maPos.Row();
    aRet.Sheet  = aData.maPos.Tab();
    return aRet;
}

void SAL_CALL ScTableConditionalEntry::setSourcePosition( const table::CellAddress& aSourcePosition )
                                            throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    aData.maPos.Set( (SCCOL)aSourcePosition.Column, (SCROW)aSourcePosition.Row, aSourcePosition.Sheet );
}

// XSheetConditionalEntry

rtl::OUString SAL_CALL ScTableConditionalEntry::getStyleName() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ScStyleNameConversion::DisplayToProgrammaticName( aData.maStyle, SFX_STYLE_FAMILY_PARA );
}

void SAL_CALL ScTableConditionalEntry::setStyleName( const rtl::OUString& aStyleName )
                                            throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    aData.maStyle = ScStyleNameConversion::ProgrammaticToDisplayName( aStyleName, SFX_STYLE_FAMILY_PARA );
}

//------------------------------------------------------------------------

ScTableValidationObj::ScTableValidationObj(ScDocument* pDoc, sal_uLong nKey,
                                            const formula::FormulaGrammar::Grammar eGrammar) :
    aPropSet( lcl_GetValidatePropertyMap() )
{
    //  Eintrag aus dem Dokument lesen...

    sal_Bool bFound = false;
    if ( pDoc && nKey )
    {
        const ScValidationData* pData = pDoc->GetValidationEntry( nKey );
        if (pData)
        {
            nMode = sal::static_int_cast<sal_uInt16>( pData->GetOperation() );
            aSrcPos = pData->GetValidSrcPos();  // valid pos for expressions
            aExpr1 = pData->GetExpression( aSrcPos, 0, 0, eGrammar );
            aExpr2 = pData->GetExpression( aSrcPos, 1, 0, eGrammar );
            meGrammar1 = meGrammar2 = eGrammar;
            nValMode = sal::static_int_cast<sal_uInt16>( pData->GetDataMode() );
            bIgnoreBlank = pData->IsIgnoreBlank();
            nShowList = pData->GetListType();
            bShowInput = pData->GetInput( aInputTitle, aInputMessage );
            ScValidErrorStyle eStyle;
            bShowError = pData->GetErrMsg( aErrorTitle, aErrorMessage, eStyle );
            nErrorStyle = sal::static_int_cast<sal_uInt16>( eStyle );

            // During save to XML, sheet::ValidationType_ANY formulas are not
            // saved, even if in the list, see
            // ScMyValidationsContainer::GetCondition(), so shall not mark
            // anything in use.
            if (nValMode != SC_VALID_ANY && pDoc->IsInExternalReferenceMarking())
                pData->MarkUsedExternalReferences();

            bFound = sal_True;
        }
    }
    if (!bFound)
        ClearData_Impl();       // Defaults
}

ScValidationData* ScTableValidationObj::CreateValidationData( ScDocument* pDoc,
                                            formula::FormulaGrammar::Grammar eGrammar ) const
{
    //  ScValidationData = Core-Struktur

    FormulaGrammar::Grammar eGrammar1 = lclResolveGrammar( eGrammar, meGrammar1 );
    FormulaGrammar::Grammar eGrammar2 = lclResolveGrammar( eGrammar, meGrammar2 );

    ScValidationData* pRet = new ScValidationData( (ScValidationMode)nValMode,
                                                   (ScConditionMode)nMode,
                                                   aExpr1, aExpr2, pDoc, aSrcPos,
                                                   maExprNmsp1, maExprNmsp2,
                                                   eGrammar1, eGrammar2 );
    pRet->SetIgnoreBlank(bIgnoreBlank);
    pRet->SetListType(nShowList);

    if ( aTokens1.getLength() )
    {
        ScTokenArray aTokenArray;
        if ( ScTokenConversion::ConvertToTokenArray(*pDoc, aTokenArray, aTokens1) )
            pRet->SetFormula1(aTokenArray);
    }

    if ( aTokens2.getLength() )
    {
        ScTokenArray aTokenArray;
        if ( ScTokenConversion::ConvertToTokenArray(*pDoc, aTokenArray, aTokens2) )
            pRet->SetFormula2(aTokenArray);
    }

    // set strings for error / input even if disabled (and disable afterwards)
    pRet->SetInput( aInputTitle, aInputMessage );
    if (!bShowInput)
        pRet->ResetInput();
    pRet->SetError( aErrorTitle, aErrorMessage, (ScValidErrorStyle)nErrorStyle );
    if (!bShowError)
        pRet->ResetError();

    if ( aPosString.Len() )
        pRet->SetSrcString( aPosString );

    return pRet;
}

void ScTableValidationObj::ClearData_Impl()
{
    nMode        = SC_COND_NONE;
    nValMode     = SC_VALID_ANY;
    bIgnoreBlank = sal_True;
    nShowList    = sheet::TableValidationVisibility::UNSORTED;
    bShowInput   = false;
    bShowError   = false;
    nErrorStyle  = SC_VALERR_STOP;
    aSrcPos.Set(0,0,0);
    aExpr1.Erase();
    aExpr2.Erase();
    maExprNmsp1.Erase();
    maExprNmsp2.Erase();
    meGrammar1 = meGrammar2 = FormulaGrammar::GRAM_UNSPECIFIED;  // will be overriden when needed
    aInputTitle.Erase();
    aInputMessage.Erase();
    aErrorTitle.Erase();
    aErrorMessage.Erase();
}

ScTableValidationObj::~ScTableValidationObj()
{
}

// XSheetCondition

sheet::ConditionOperator SAL_CALL ScTableValidationObj::getOperator()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return lcl_ConditionModeToOperator( (ScConditionMode)nMode );
}

void SAL_CALL ScTableValidationObj::setOperator( sheet::ConditionOperator nOperator )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    nMode = sal::static_int_cast<sal_uInt16>( lcl_ConditionOperatorToMode( nOperator ) );
}

sal_Int32 SAL_CALL ScTableValidationObj::getConditionOperator()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return lcl_ConditionModeToOperatorNew( (ScConditionMode)nMode );
}

void SAL_CALL ScTableValidationObj::setConditionOperator( sal_Int32 nOperator )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    nMode = sal::static_int_cast<sal_uInt16>( ScConditionEntry::GetModeFromApi( nOperator ) );
}

rtl::OUString SAL_CALL ScTableValidationObj::getFormula1() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return aExpr1;
}

void SAL_CALL ScTableValidationObj::setFormula1( const rtl::OUString& aFormula1 )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    aExpr1 = String( aFormula1 );
}

rtl::OUString SAL_CALL ScTableValidationObj::getFormula2() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return aExpr2;
}

void SAL_CALL ScTableValidationObj::setFormula2( const rtl::OUString& aFormula2 )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    aExpr2 = String( aFormula2 );
}

table::CellAddress SAL_CALL ScTableValidationObj::getSourcePosition() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    table::CellAddress aRet;
    aRet.Column = aSrcPos.Col();
    aRet.Row    = aSrcPos.Row();
    aRet.Sheet  = aSrcPos.Tab();
    return aRet;
}

void SAL_CALL ScTableValidationObj::setSourcePosition( const table::CellAddress& aSourcePosition )
                                            throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    aSrcPos.Set( (SCCOL)aSourcePosition.Column, (SCROW)aSourcePosition.Row, aSourcePosition.Sheet );
}

uno::Sequence<sheet::FormulaToken> SAL_CALL ScTableValidationObj::getTokens( sal_Int32 nIndex )
                                            throw(uno::RuntimeException,lang::IndexOutOfBoundsException)
{
    SolarMutexGuard aGuard;
    if (nIndex >= 2 || nIndex < 0)
        throw lang::IndexOutOfBoundsException();

    return nIndex == 0 ? aTokens1 : aTokens2;
}

void SAL_CALL ScTableValidationObj::setTokens( sal_Int32 nIndex, const uno::Sequence<sheet::FormulaToken>& aTokens )
                                            throw(uno::RuntimeException,lang::IndexOutOfBoundsException)
{
    SolarMutexGuard aGuard;
    if (nIndex >= 2 || nIndex < 0)
        throw lang::IndexOutOfBoundsException();

    if (nIndex == 0)
    {
        aTokens1 = aTokens;
        aExpr1.Erase();
    }
    else if (nIndex == 1)
    {
        aTokens2 = aTokens;
        aExpr2.Erase();
    }
}

sal_Int32 SAL_CALL ScTableValidationObj::getCount() throw(uno::RuntimeException)
{
    return 2;
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScTableValidationObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScTableValidationObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aString(aPropertyName);

    if ( aString.EqualsAscii( SC_UNONAME_SHOWINP ) )       bShowInput = ScUnoHelpFunctions::GetBoolFromAny( aValue );
    else if ( aString.EqualsAscii( SC_UNONAME_SHOWERR ) )  bShowError = ScUnoHelpFunctions::GetBoolFromAny( aValue );
    else if ( aString.EqualsAscii( SC_UNONAME_IGNOREBL ) ) bIgnoreBlank = ScUnoHelpFunctions::GetBoolFromAny( aValue );
    else if ( aString.EqualsAscii( SC_UNONAME_SHOWLIST ) ) aValue >>= nShowList;
    else if ( aString.EqualsAscii( SC_UNONAME_INPTITLE ) )
    {
        rtl::OUString aStrVal;
        if ( aValue >>= aStrVal )
            aInputTitle = String( aStrVal );
    }
    else if ( aString.EqualsAscii( SC_UNONAME_INPMESS ) )
    {
        rtl::OUString aStrVal;
        if ( aValue >>= aStrVal )
            aInputMessage = String( aStrVal );
    }
    else if ( aString.EqualsAscii( SC_UNONAME_ERRTITLE ) )
    {
        rtl::OUString aStrVal;
        if ( aValue >>= aStrVal )
            aErrorTitle = String( aStrVal );
    }
    else if ( aString.EqualsAscii( SC_UNONAME_ERRMESS ) )
    {
        rtl::OUString aStrVal;
        if ( aValue >>= aStrVal )
            aErrorMessage = String( aStrVal );
    }
    else if ( aString.EqualsAscii( SC_UNONAME_TYPE ) )
    {
        sheet::ValidationType eType = (sheet::ValidationType)
                                ScUnoHelpFunctions::GetEnumFromAny( aValue );
        switch (eType)
        {
            case sheet::ValidationType_ANY:      nValMode = SC_VALID_ANY;     break;
            case sheet::ValidationType_WHOLE:    nValMode = SC_VALID_WHOLE;   break;
            case sheet::ValidationType_DECIMAL:  nValMode = SC_VALID_DECIMAL; break;
            case sheet::ValidationType_DATE:     nValMode = SC_VALID_DATE;    break;
            case sheet::ValidationType_TIME:     nValMode = SC_VALID_TIME;    break;
            case sheet::ValidationType_TEXT_LEN: nValMode = SC_VALID_TEXTLEN; break;
            case sheet::ValidationType_LIST:     nValMode = SC_VALID_LIST;    break;
            case sheet::ValidationType_CUSTOM:   nValMode = SC_VALID_CUSTOM;  break;
            default:
            {
                // added to avoid warnings
            }
        }
    }
    else if ( aString.EqualsAscii( SC_UNONAME_ERRALSTY ) )
    {
        sheet::ValidationAlertStyle eStyle = (sheet::ValidationAlertStyle)
                                ScUnoHelpFunctions::GetEnumFromAny( aValue );
        switch (eStyle)
        {
            case sheet::ValidationAlertStyle_STOP:    nErrorStyle = SC_VALERR_STOP;    break;
            case sheet::ValidationAlertStyle_WARNING: nErrorStyle = SC_VALERR_WARNING; break;
            case sheet::ValidationAlertStyle_INFO:    nErrorStyle = SC_VALERR_INFO;    break;
            case sheet::ValidationAlertStyle_MACRO:   nErrorStyle = SC_VALERR_MACRO;   break;
            default:
            {
                // added to avoid warnings
            }
        }
    }
    else if ( aString.EqualsAscii( SC_UNONAME_SOURCESTR ) )
    {
        // internal - only for XML filter, not in PropertySetInfo, only set

        rtl::OUString aStrVal;
        if ( aValue >>= aStrVal )
            aPosString = String( aStrVal );
    }
    else if ( aString.EqualsAscii( SC_UNONAME_FORMULANMSP1 ) )
    {
        // internal - only for XML filter, not in PropertySetInfo, only set

        rtl::OUString aStrVal;
        if ( aValue >>= aStrVal )
            maExprNmsp1 = aStrVal;
    }
    else if ( aString.EqualsAscii( SC_UNONAME_FORMULANMSP2 ) )
    {
        // internal - only for XML filter, not in PropertySetInfo, only set

        rtl::OUString aStrVal;
        if ( aValue >>= aStrVal )
            maExprNmsp2 = aStrVal;
    }
    else if ( aString.EqualsAscii( SC_UNONAME_GRAMMAR1 ) )
    {
        // internal - only for XML filter, not in PropertySetInfo, only set

        sal_Int32 nVal = 0;
        if ( aValue >>= nVal )
            meGrammar1 = static_cast< FormulaGrammar::Grammar >(nVal);
    }
    else if ( aString.EqualsAscii( SC_UNONAME_GRAMMAR2 ) )
    {
        // internal - only for XML filter, not in PropertySetInfo, only set

        sal_Int32 nVal = 0;
        if ( aValue >>= nVal )
            meGrammar2 = static_cast< FormulaGrammar::Grammar >(nVal);
    }
}

uno::Any SAL_CALL ScTableValidationObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aString(aPropertyName);
    uno::Any aRet;

    if ( aString.EqualsAscii( SC_UNONAME_SHOWINP ) )       ScUnoHelpFunctions::SetBoolInAny( aRet, bShowInput );
    else if ( aString.EqualsAscii( SC_UNONAME_SHOWERR ) )  ScUnoHelpFunctions::SetBoolInAny( aRet, bShowError );
    else if ( aString.EqualsAscii( SC_UNONAME_IGNOREBL ) ) ScUnoHelpFunctions::SetBoolInAny( aRet, bIgnoreBlank );
    else if ( aString.EqualsAscii( SC_UNONAME_SHOWLIST ) ) aRet <<= nShowList;
    else if ( aString.EqualsAscii( SC_UNONAME_INPTITLE ) ) aRet <<= rtl::OUString( aInputTitle );
    else if ( aString.EqualsAscii( SC_UNONAME_INPMESS ) )  aRet <<= rtl::OUString( aInputMessage );
    else if ( aString.EqualsAscii( SC_UNONAME_ERRTITLE ) ) aRet <<= rtl::OUString( aErrorTitle );
    else if ( aString.EqualsAscii( SC_UNONAME_ERRMESS ) )  aRet <<= rtl::OUString( aErrorMessage );
    else if ( aString.EqualsAscii( SC_UNONAME_TYPE ) )
    {
        sheet::ValidationType eType = sheet::ValidationType_ANY;
        switch (nValMode)
        {
            case SC_VALID_ANY:      eType = sheet::ValidationType_ANY;      break;
            case SC_VALID_WHOLE:    eType = sheet::ValidationType_WHOLE;    break;
            case SC_VALID_DECIMAL:  eType = sheet::ValidationType_DECIMAL;  break;
            case SC_VALID_DATE:     eType = sheet::ValidationType_DATE;     break;
            case SC_VALID_TIME:     eType = sheet::ValidationType_TIME;     break;
            case SC_VALID_TEXTLEN:  eType = sheet::ValidationType_TEXT_LEN; break;
            case SC_VALID_LIST:     eType = sheet::ValidationType_LIST;     break;
            case SC_VALID_CUSTOM:   eType = sheet::ValidationType_CUSTOM;   break;
        }
        aRet <<= eType;
    }
    else if ( aString.EqualsAscii( SC_UNONAME_ERRALSTY ) )
    {
        sheet::ValidationAlertStyle eStyle = sheet::ValidationAlertStyle_STOP;
        switch (nErrorStyle)
        {
            case SC_VALERR_STOP:    eStyle = sheet::ValidationAlertStyle_STOP;    break;
            case SC_VALERR_WARNING: eStyle = sheet::ValidationAlertStyle_WARNING; break;
            case SC_VALERR_INFO:    eStyle = sheet::ValidationAlertStyle_INFO;    break;
            case SC_VALERR_MACRO:   eStyle = sheet::ValidationAlertStyle_MACRO;   break;
        }
        aRet <<= eStyle;
    }

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScTableValidationObj )

// XUnoTunnel

sal_Int64 SAL_CALL ScTableValidationObj::getSomething(
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
    class theScTableValidationObjUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScTableValidationObjUnoTunnelId> {};
}

const uno::Sequence<sal_Int8>& ScTableValidationObj::getUnoTunnelId()
{
    return theScTableValidationObjUnoTunnelId::get().getSeq();
}

ScTableValidationObj* ScTableValidationObj::getImplementation(
                                const uno::Reference<beans::XPropertySet> xObj )
{
    ScTableValidationObj* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = reinterpret_cast<ScTableValidationObj*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}

//------------------------------------------------------------------------




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
