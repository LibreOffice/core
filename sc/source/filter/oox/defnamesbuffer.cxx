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

#include "defnamesbuffer.hxx"

#include <com/sun/star/sheet/ComplexReference.hpp>
#include <com/sun/star/sheet/ExternalReference.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>
#include <com/sun/star/sheet/ReferenceFlags.hpp>
#include <com/sun/star/sheet/SingleReference.hpp>
#include <com/sun/star/sheet/XFormulaTokens.hpp>
#include <com/sun/star/sheet/XPrintAreas.hpp>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/propertyset.hxx>
#include "addressconverter.hxx"
#include "biffinputstream.hxx"
#include "externallinkbuffer.hxx"
#include "formulaparser.hxx"
#include "worksheetbuffer.hxx"
#include "tokenarray.hxx"
#include "tokenuno.hxx"
#include "compiler.hxx"

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

const sal_uInt16 BIFF_REFFLAG_COL1REL       = 0x0001;
const sal_uInt16 BIFF_REFFLAG_ROW1REL       = 0x0002;
const sal_uInt16 BIFF_REFFLAG_COL2REL       = 0x0004;
const sal_uInt16 BIFF_REFFLAG_ROW2REL       = 0x0008;

const sal_Char* const spcOoxPrefix = "_xlnm.";

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
    return OUStringBuffer().appendAscii( spcOoxPrefix ).append( lclGetBaseName( cBuiltinId ) ).makeStringAndClear();
}

/** returns the built-in name identifier from a prefixed built-in name, e.g. '_xlnm.Print_Area'. */
sal_Unicode lclGetBuiltinIdFromPrefixedName( const OUString& rModelName )
{
    OUString aPrefix = OUString::createFromAscii( spcOoxPrefix );
    sal_Int32 nPrefixLen = aPrefix.getLength();
    if( rModelName.matchIgnoreAsciiCase( aPrefix ) )
    {
        for( sal_Unicode cBuiltinId = 0; cBuiltinId < SAL_N_ELEMENTS( sppcBaseNames ); ++cBuiltinId )
        {
            OUString aBaseName = lclGetBaseName( cBuiltinId );
            sal_Int32 nBaseNameLen = aBaseName.getLength();
            if( (rModelName.getLength() == nPrefixLen + nBaseNameLen) && rModelName.matchIgnoreAsciiCase( aBaseName, nPrefixLen ) )
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

void lclConvertRefFlags( sal_Int32& ornFlags, sal_Int32& ornAbsPos, sal_Int32& ornRelPos, sal_Int32 nBasePos, sal_Int32 nApiRelFlag, bool bRel )
{
    if( getFlag( ornFlags, nApiRelFlag ) && !bRel )
    {
        // convert relative to absolute
        setFlag( ornFlags, nApiRelFlag, false );
        ornAbsPos = nBasePos + ornRelPos;
    }
    else if( !getFlag( ornFlags, nApiRelFlag ) && bRel )
    {
        // convert absolute to relative
        setFlag( ornFlags, nApiRelFlag, true );
        ornRelPos = ornAbsPos - nBasePos;
    }
}

void lclConvertSingleRefFlags( SingleReference& orApiRef, const CellAddress& rBaseAddr, bool bColRel, bool bRowRel )
{
    using namespace ::com::sun::star::sheet::ReferenceFlags;
    lclConvertRefFlags(
        orApiRef.Flags, orApiRef.Column, orApiRef.RelativeColumn,
        rBaseAddr.Column, COLUMN_RELATIVE, bColRel );
    lclConvertRefFlags(
        orApiRef.Flags, orApiRef.Row, orApiRef.RelativeRow,
        rBaseAddr.Row, ROW_RELATIVE, bRowRel );
}

Any lclConvertReference( const Any& rRefAny, const CellAddress& rBaseAddr, sal_uInt16 nRelFlags )
{
    if( rRefAny.has< SingleReference >() && !getFlag( nRelFlags, BIFF_REFFLAG_COL2REL ) && !getFlag( nRelFlags, BIFF_REFFLAG_ROW2REL ) )
    {
        SingleReference aApiRef;
        rRefAny >>= aApiRef;
        lclConvertSingleRefFlags( aApiRef, rBaseAddr, getFlag( nRelFlags, BIFF_REFFLAG_COL1REL ), getFlag( nRelFlags, BIFF_REFFLAG_ROW1REL ) );
        return Any( aApiRef );
    }
    if( rRefAny.has< ComplexReference >() )
    {
        ComplexReference aApiRef;
        rRefAny >>= aApiRef;
        lclConvertSingleRefFlags( aApiRef.Reference1, rBaseAddr, getFlag( nRelFlags, BIFF_REFFLAG_COL1REL ), getFlag( nRelFlags, BIFF_REFFLAG_ROW1REL ) );
        lclConvertSingleRefFlags( aApiRef.Reference2, rBaseAddr, getFlag( nRelFlags, BIFF_REFFLAG_COL2REL ), getFlag( nRelFlags, BIFF_REFFLAG_ROW2REL ) );
        return Any( aApiRef );
    }
    return Any();
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

Any DefinedNameBase::getReference( const CellAddress& rBaseAddr ) const
{
    if( maRefAny.hasValue() && (maModel.maName.getLength() >= 2) && (maModel.maName[ 0 ] == '\x01') )
    {
        sal_Unicode cFlagsChar = getUpcaseModelName()[ 1 ];
        if( ('A' <= cFlagsChar) && (cFlagsChar <= 'P') )
        {
            sal_uInt16 nRelFlags = static_cast< sal_uInt16 >( cFlagsChar - 'A' );
            if( maRefAny.has< ExternalReference >() )
            {
                ExternalReference aApiExtRef;
                maRefAny >>= aApiExtRef;
                Any aRefAny = lclConvertReference( aApiExtRef.Reference, rBaseAddr, nRelFlags );
                if( aRefAny.hasValue() )
                {
                    aApiExtRef.Reference <<= aRefAny;
                    return Any( aApiExtRef );
                }
            }
            else
            {
                return lclConvertReference( maRefAny, rBaseAddr, nRelFlags );
            }
        }
    }
    return Any();
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

    // skip BIFF names without stream position (e.g. BIFF3-BIFF4 internal 3D references)
    if( (getFilterType() == FILTER_BIFF) && !mxBiffStrm.get() )
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

std::unique_ptr<ScTokenArray> DefinedName::getScTokens()
{
    ScTokenArray aTokenArray;
    ScCompiler aCompiler(&getScDocument(), ScAddress(0, 0, mnCalcSheet));
    aCompiler.SetGrammar(formula::FormulaGrammar::GRAM_OOXML);
    std::unique_ptr<ScTokenArray> pArray(aCompiler.CompileString(maModel.maFormula));
    // Compile the tokens into RPN once to populate information into tokens
    // where necessary, e.g. for TableRef inner reference. RPN can be discarded
    // after, a resulting error must be reset.
    sal_uInt16 nErr = pArray->GetCodeError();
    aCompiler.CompileTokenArray();
    pArray->DelRPN();
    pArray->SetCodeError(nErr);

    return pArray;
}

void DefinedName::convertFormula()
{
    // macro function or vba procedure
    if(!mpScRangeData)
        return;

    // convert and set formula of the defined name
    if ( getFilterType() == FILTER_OOXML )
    {
        std::unique_ptr<ScTokenArray> pTokenArray = getScTokens();
        mpScRangeData->SetCode( *pTokenArray );
    }

    ScTokenArray* pTokenArray = mpScRangeData->GetCode();
    Sequence< FormulaToken > aFTokenSeq;
    (void)ScTokenConversion::ConvertToTokenSequence( this->getScDocument(), aFTokenSeq, *pTokenArray );
    // set built-in names (print ranges, repeated titles, filter ranges)
    if( !isGlobalName() ) switch( mcBuiltinId )
    {
        case BIFF_DEFNAME_PRINTAREA:
        {
            Reference< XPrintAreas > xPrintAreas( getSheetFromDoc( mnCalcSheet ), UNO_QUERY );
            ApiCellRangeList aPrintRanges;
            getFormulaParser().extractCellRangeList( aPrintRanges, aFTokenSeq, false, mnCalcSheet );
            if( xPrintAreas.is() && !aPrintRanges.empty() )
                xPrintAreas->setPrintAreas( aPrintRanges.toSequence() );
        }
        break;
        case BIFF_DEFNAME_PRINTTITLES:
        {
            Reference< XPrintAreas > xPrintAreas( getSheetFromDoc( mnCalcSheet ), UNO_QUERY );
            ApiCellRangeList aTitleRanges;
            getFormulaParser().extractCellRangeList( aTitleRanges, aFTokenSeq, false, mnCalcSheet );
            if( xPrintAreas.is() && !aTitleRanges.empty() )
            {
                bool bHasRowTitles = false;
                bool bHasColTitles = false;
                const CellAddress& rMaxPos = getAddressConverter().getMaxAddress();
                for( ::std::vector< CellRangeAddress >::const_iterator aIt = aTitleRanges.begin(), aEnd = aTitleRanges.end(); (aIt != aEnd) && (!bHasRowTitles || !bHasColTitles); ++aIt )
                {
                    bool bFullRow = (aIt->StartColumn == 0) && (aIt->EndColumn >= rMaxPos.Column);
                    bool bFullCol = (aIt->StartRow == 0) && (aIt->EndRow >= rMaxPos.Row);
                    if( !bHasRowTitles && bFullRow && !bFullCol )
                    {
                        xPrintAreas->setTitleRows( *aIt );
                        xPrintAreas->setPrintTitleRows( true );
                        bHasRowTitles = true;
                    }
                    else if( !bHasColTitles && bFullCol && !bFullRow )
                    {
                        xPrintAreas->setTitleColumns( *aIt );
                        xPrintAreas->setPrintTitleColumns( true );
                        bHasColTitles = true;
                    }
                }
            }
        }
        break;
    }
}

bool DefinedName::getAbsoluteRange( CellRangeAddress& orRange ) const
{
    ScTokenArray* pTokenArray = mpScRangeData->GetCode();
    Sequence< FormulaToken > aFTokenSeq;
    ScTokenConversion::ConvertToTokenSequence(getScDocument(), aFTokenSeq, *pTokenArray);
    return getFormulaParser().extractCellRange( orRange, aFTokenSeq, false );
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
    for( DefNameVector::iterator aIt = maDefNames.begin(), aEnd = maDefNames.end(); aIt != aEnd; ++aIt )
    {
        DefinedNameRef xDefName = *aIt;
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
    maDefNames.forEachMem( &DefinedName::convertFormula );
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
