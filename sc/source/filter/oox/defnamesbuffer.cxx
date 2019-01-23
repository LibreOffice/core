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
#include <defnamesbuffer.hxx>

#include <com/sun/star/sheet/NamedRangeFlag.hpp>
#include <com/sun/star/sheet/XPrintAreas.hpp>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/tokens.hxx>
#include <addressconverter.hxx>
#include <biffhelper.hxx>
#include <externallinkbuffer.hxx>
#include <formulabase.hxx>
#include <formulaparser.hxx>
#include <worksheetbuffer.hxx>
#include <tokenarray.hxx>
#include <tokenuno.hxx>
#include <compiler.hxx>
#include <document.hxx>

namespace oox {
namespace xls {

using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;

namespace {

const sal_uInt32 BIFF12_DEFNAME_HIDDEN      = 0x00000001;
const sal_uInt32 BIFF12_DEFNAME_FUNC        = 0x00000002;
const sal_uInt32 BIFF12_DEFNAME_VBNAME      = 0x00000004;
const sal_uInt32 BIFF12_DEFNAME_MACRO       = 0x00000008;
const sal_uInt32 BIFF12_DEFNAME_BUILTIN     = 0x00000020;

const OUStringLiteral spcOoxPrefix("_xlnm.");

const sal_Char* const sppcBaseNames[] =
{
    "Consolidate_Area",
    "Auto_Open",
    "Auto_Close",
    "Extract",
    "Database",
    "Criteria",
    "Print_Area",
    "Print_Titles",
    "Recorder",
    "Data_Form",
    "Auto_Activate",
    "Auto_Deactivate",
    "Sheet_Title",
    "_FilterDatabase"
};

OUString lclGetBaseName( sal_Unicode cBuiltinId )
{
    OSL_ENSURE( cBuiltinId < SAL_N_ELEMENTS( sppcBaseNames ), "lclGetBaseName - unsupported built-in identifier" );
    OUStringBuffer aBuffer;
    if( cBuiltinId < SAL_N_ELEMENTS( sppcBaseNames ) )
        aBuffer.appendAscii( sppcBaseNames[ cBuiltinId ] );
    else
        aBuffer.append( static_cast< sal_Int32 >( cBuiltinId ) );
    return aBuffer.makeStringAndClear();
}

OUString lclGetPrefixedName( sal_Unicode cBuiltinId )
{
    return spcOoxPrefix + lclGetBaseName( cBuiltinId );
}

/** returns the built-in name identifier from a prefixed built-in name, e.g. '_xlnm.Print_Area'. */
sal_Unicode lclGetBuiltinIdFromPrefixedName( const OUString& rModelName )
{
    if( rModelName.matchIgnoreAsciiCase( spcOoxPrefix ) )
    {
        for( sal_Unicode cBuiltinId = 0; cBuiltinId < SAL_N_ELEMENTS( sppcBaseNames ); ++cBuiltinId )
        {
            OUString aBaseName = lclGetBaseName( cBuiltinId );
            sal_Int32 nBaseNameLen = aBaseName.getLength();
            if( (rModelName.getLength() == spcOoxPrefix.size + nBaseNameLen) && rModelName.matchIgnoreAsciiCase( aBaseName, spcOoxPrefix.size ) )
                return cBuiltinId;
        }
    }
    return BIFF_DEFNAME_UNKNOWN;
}

/** returns the built-in name identifier from a built-in base name, e.g. 'Print_Area'. */
sal_Unicode lclGetBuiltinIdFromBaseName( const OUString& rModelName )
{
    for( sal_Unicode cBuiltinId = 0; cBuiltinId < SAL_N_ELEMENTS( sppcBaseNames ); ++cBuiltinId )
        if( rModelName.equalsIgnoreAsciiCaseAscii( sppcBaseNames[ cBuiltinId ] ) )
            return cBuiltinId;
    return BIFF_DEFNAME_UNKNOWN;
}

OUString lclGetUpcaseModelName( const OUString& rModelName )
{
    // TODO: i18n?
    return rModelName.toAsciiUpperCase();
}

} // namespace

DefinedNameModel::DefinedNameModel() :
    mnSheet( -1 ),
    mnFuncGroupId( -1 ),
    mbMacro( false ),
    mbFunction( false ),
    mbVBName( false ),
    mbHidden( false )
{
}

DefinedNameBase::DefinedNameBase( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

const OUString& DefinedNameBase::getUpcaseModelName() const
{
    if( maUpModelName.isEmpty() )
        maUpModelName = lclGetUpcaseModelName( maModel.maName );
    return maUpModelName;
}

DefinedName::DefinedName( const WorkbookHelper& rHelper ) :
    DefinedNameBase( rHelper ),
    mpScRangeData(nullptr),
    mnTokenIndex( -1 ),
    mnCalcSheet( 0 ),
    mcBuiltinId( BIFF_DEFNAME_UNKNOWN )
{
}

void DefinedName::importDefinedName( const AttributeList& rAttribs )
{
    maModel.maName        = rAttribs.getXString( XML_name, OUString() );
    maModel.mnSheet       = rAttribs.getInteger( XML_localSheetId, -1 );
    maModel.mnFuncGroupId = rAttribs.getInteger( XML_functionGroupId, -1 );
    maModel.mbMacro       = rAttribs.getBool( XML_xlm, false );
    maModel.mbFunction    = rAttribs.getBool( XML_function, false );
    maModel.mbVBName      = rAttribs.getBool( XML_vbProcedure, false );
    maModel.mbHidden      = rAttribs.getBool( XML_hidden, false );
    mnCalcSheet = (maModel.mnSheet >= 0) ? getWorksheets().getCalcSheetIndex( maModel.mnSheet ) : -1;

    /*  Detect built-in state from name itself, there is no built-in flag.
        Built-in names are prefixed with '_xlnm.' instead. */
    mcBuiltinId = lclGetBuiltinIdFromPrefixedName( maModel.maName );
}

void DefinedName::setFormula( const OUString& rFormula )
{
    maModel.maFormula = rFormula;
}

void DefinedName::importDefinedName( SequenceInputStream& rStrm )
{
    sal_uInt32 nFlags;
    nFlags = rStrm.readuInt32();
    rStrm.skip( 1 );    // keyboard shortcut
    maModel.mnSheet = rStrm.readInt32();
    rStrm >> maModel.maName;
    mnCalcSheet = (maModel.mnSheet >= 0) ? getWorksheets().getCalcSheetIndex( maModel.mnSheet ) : -1;

    // macro function/command, hidden flag
    maModel.mnFuncGroupId = extractValue< sal_Int32 >( nFlags, 6, 9 );
    maModel.mbMacro       = getFlag( nFlags, BIFF12_DEFNAME_MACRO );
    maModel.mbFunction    = getFlag( nFlags, BIFF12_DEFNAME_FUNC );
    maModel.mbVBName      = getFlag( nFlags, BIFF12_DEFNAME_VBNAME );
    maModel.mbHidden      = getFlag( nFlags, BIFF12_DEFNAME_HIDDEN );

    // get built-in name index from name
    if( getFlag( nFlags, BIFF12_DEFNAME_BUILTIN ) )
        mcBuiltinId = lclGetBuiltinIdFromBaseName( maModel.maName );

    // store token array data
    sal_Int64 nRecPos = rStrm.tell();
    sal_Int32 nFmlaSize = rStrm.readInt32();
    rStrm.skip( nFmlaSize );
    sal_Int32 nAddDataSize = rStrm.readInt32();
    if( !rStrm.isEof() && (nFmlaSize > 0) && (nAddDataSize >= 0) && (rStrm.getRemaining() >= nAddDataSize) )
    {
        sal_Int32 nTotalSize = 8 + nFmlaSize + nAddDataSize;
        mxFormula.reset( new StreamDataSequence );
        rStrm.seek( nRecPos );
        rStrm.readData( *mxFormula, nTotalSize );
    }
}

void DefinedName::createNameObject( sal_Int32 nIndex )
{
    // do not create names for (macro) functions or VBA procedures
    // #163146# do not ignore hidden names (may be regular names created by VBA scripts)
    if( /*maModel.mbHidden ||*/ maModel.mbFunction || maModel.mbVBName )
        return;

    // convert original name to final Calc name (TODO: filter invalid characters from model name)
    maCalcName = isBuiltinName() ? lclGetPrefixedName( mcBuiltinId ) : maModel.maName;

    // #163146# do not rename sheet-local names by default, this breaks VBA scripts

    // special flags for this name
    sal_Int32 nNameFlags = 0;
    using namespace ::com::sun::star::sheet::NamedRangeFlag;
    if( !isGlobalName() ) switch( mcBuiltinId )
    {
        case BIFF_DEFNAME_CRITERIA:     nNameFlags = FILTER_CRITERIA;               break;
        case BIFF_DEFNAME_PRINTAREA:    nNameFlags = PRINT_AREA;                    break;
        case BIFF_DEFNAME_PRINTTITLES:  nNameFlags = COLUMN_HEADER | ROW_HEADER;    break;
    }

    // create the name and insert it into the document, maCalcName will be changed to the resulting name
    if (maModel.mnSheet >= 0)
        mpScRangeData = createLocalNamedRangeObject( maCalcName, ApiTokenSequence(), nIndex, nNameFlags, maModel.mnSheet );
    else
        mpScRangeData = createNamedRangeObject( maCalcName, ApiTokenSequence(), nIndex, nNameFlags );
    mnTokenIndex = nIndex;
}

std::unique_ptr<ScTokenArray> DefinedName::getScTokens(
        const css::uno::Sequence<css::sheet::ExternalLinkInfo>& rExternalLinks )
{
    ScCompiler aCompiler(&getScDocument(), ScAddress(0, 0, mnCalcSheet), formula::FormulaGrammar::GRAM_OOXML);
    aCompiler.SetExternalLinks( rExternalLinks);
    std::unique_ptr<ScTokenArray> pArray(aCompiler.CompileString(maModel.maFormula));
    // Compile the tokens into RPN once to populate information into tokens
    // where necessary, e.g. for TableRef inner reference. RPN can be discarded
    // after, a resulting error must be reset.
    FormulaError nErr = pArray->GetCodeError();
    aCompiler.CompileTokenArray();
    getScDocument().CheckLinkFormulaNeedingCheck( *pArray);
    pArray->DelRPN();
    pArray->SetCodeError(nErr);

    return pArray;
}

void DefinedName::convertFormula( const css::uno::Sequence<css::sheet::ExternalLinkInfo>& rExternalLinks )
{
    // macro function or vba procedure
    if(!mpScRangeData)
        return;

    // convert and set formula of the defined name
    {
        std::unique_ptr<ScTokenArray> pTokenArray = getScTokens( rExternalLinks);
        mpScRangeData->SetCode( *pTokenArray );
    }

    ScTokenArray* pTokenArray = mpScRangeData->GetCode();
    Sequence< FormulaToken > aFTokenSeq;
    ScTokenConversion::ConvertToTokenSequence( getScDocument(), aFTokenSeq, *pTokenArray );
    // set built-in names (print ranges, repeated titles, filter ranges)
    if( !isGlobalName() ) switch( mcBuiltinId )
    {
        case BIFF_DEFNAME_PRINTAREA:
        {
            Reference< XPrintAreas > xPrintAreas( getSheetFromDoc( mnCalcSheet ), UNO_QUERY );
            ScRangeList aPrintRanges;
            getFormulaParser().extractCellRangeList( aPrintRanges, aFTokenSeq, mnCalcSheet );
            if( xPrintAreas.is() && !aPrintRanges.empty() )
                xPrintAreas->setPrintAreas( AddressConverter::toApiSequence(aPrintRanges) );
        }
        break;
        case BIFF_DEFNAME_PRINTTITLES:
        {
            Reference< XPrintAreas > xPrintAreas( getSheetFromDoc( mnCalcSheet ), UNO_QUERY );
            ScRangeList aTitleRanges;
            getFormulaParser().extractCellRangeList( aTitleRanges, aFTokenSeq, mnCalcSheet );
            if( xPrintAreas.is() && !aTitleRanges.empty() )
            {
                bool bHasRowTitles = false;
                bool bHasColTitles = false;
                const ScAddress& rMaxPos = getAddressConverter().getMaxAddress();
                for (size_t i = 0, nSize = aTitleRanges.size(); i < nSize; ++i)
                {
                    const ScRange& rRange = aTitleRanges[i];
                    bool bFullRow = (rRange.aStart.Col() == 0) && ( rRange.aEnd.Col() >= rMaxPos.Col() );
                    bool bFullCol = (rRange.aStart.Row() == 0) && ( rRange.aEnd.Row() >= rMaxPos.Row() );
                    if( !bHasRowTitles && bFullRow && !bFullCol )
                    {
                        xPrintAreas->setTitleRows( CellRangeAddress(rRange.aStart.Tab(),
                                                                    rRange.aStart.Col(), rRange.aStart.Row(),
                                                                    rRange.aEnd.Col(), rRange.aEnd.Row()) );
                        xPrintAreas->setPrintTitleRows( true );
                        bHasRowTitles = true;
                    }
                    else if( !bHasColTitles && bFullCol && !bFullRow )
                    {
                        xPrintAreas->setTitleColumns( CellRangeAddress(rRange.aStart.Tab(),
                                                                       rRange.aStart.Col(), rRange.aStart.Row(),
                                                                       rRange.aEnd.Col(), rRange.aEnd.Row()) );
                        xPrintAreas->setPrintTitleColumns( true );
                        bHasColTitles = true;
                    }
                }
            }
        }
        break;
    }
}

bool DefinedName::getAbsoluteRange( ScRange& orRange ) const
{
    ScTokenArray* pTokenArray = mpScRangeData->GetCode();
    Sequence< FormulaToken > aFTokenSeq;
    ScTokenConversion::ConvertToTokenSequence(getScDocument(), aFTokenSeq, *pTokenArray);
    return getFormulaParser().extractCellRange( orRange, aFTokenSeq );
}


DefinedNamesBuffer::DefinedNamesBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

DefinedNameRef DefinedNamesBuffer::importDefinedName( const AttributeList& rAttribs )
{
    DefinedNameRef xDefName = createDefinedName();
    xDefName->importDefinedName( rAttribs );
    return xDefName;
}

void DefinedNamesBuffer::importDefinedName( SequenceInputStream& rStrm )
{
    createDefinedName()->importDefinedName( rStrm );
}

void DefinedNamesBuffer::finalizeImport()
{
    // first insert all names without formula definition into the document, and insert them into the maps
    int index = 0;
    for( DefinedNameRef& xDefName : maDefNames )
    {
        xDefName->createNameObject( ++index );
        // map by sheet index and original model name
        maModelNameMap[ SheetNameKey( xDefName->getLocalCalcSheet(), xDefName->getUpcaseModelName() ) ] = xDefName;
        // map by sheet index and built-in identifier
        if( !xDefName->isGlobalName() && xDefName->isBuiltinName() )
            maBuiltinMap[ BuiltinKey( xDefName->getLocalCalcSheet(), xDefName->getBuiltinId() ) ] = xDefName;
        // map by API formula token identifier
        sal_Int32 nTokenIndex = xDefName->getTokenIndex();
        if( nTokenIndex >= 0 )
            maTokenIdMap[ nTokenIndex ] = xDefName;
    }

    /*  Now convert all name formulas, so that the formula parser can find all
        names in case of circular dependencies. */
    maDefNames.forEachMem( &DefinedName::convertFormula, getExternalLinks().getLinkInfos());
}

DefinedNameRef DefinedNamesBuffer::getByIndex( sal_Int32 nIndex ) const
{
    return maDefNames.get( nIndex );
}

DefinedNameRef DefinedNamesBuffer::getByTokenIndex( sal_Int32 nIndex ) const
{
    return maTokenIdMap.get( nIndex );
}

DefinedNameRef DefinedNamesBuffer::getByModelName( const OUString& rModelName, sal_Int16 nCalcSheet ) const
{
    OUString aUpcaseName = lclGetUpcaseModelName( rModelName );
    DefinedNameRef xDefName = maModelNameMap.get( SheetNameKey( nCalcSheet, aUpcaseName ) );
    // lookup global name, if no local name exists
    if( !xDefName && (nCalcSheet >= 0) )
        xDefName = maModelNameMap.get( SheetNameKey( -1, aUpcaseName ) );
    return xDefName;
}

DefinedNameRef DefinedNamesBuffer::getByBuiltinId( sal_Unicode cBuiltinId, sal_Int16 nCalcSheet ) const
{
    return maBuiltinMap.get( BuiltinKey( nCalcSheet, cBuiltinId ) );
}

DefinedNameRef DefinedNamesBuffer::createDefinedName()
{
    DefinedNameRef xDefName( new DefinedName( *this ) );
    maDefNames.push_back( xDefName );
    return xDefName;
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
