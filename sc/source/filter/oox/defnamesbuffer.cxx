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

#include "defnamesbuffer.hxx"

#include <com/sun/star/sheet/ComplexReference.hpp>
#include <com/sun/star/sheet/ExternalReference.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>
#include <com/sun/star/sheet/ReferenceFlags.hpp>
#include <com/sun/star/sheet/SingleReference.hpp>
#include <com/sun/star/sheet/XFormulaTokens.hpp>
#include <com/sun/star/sheet/XPrintAreas.hpp>
#include <rtl/ustrbuf.hxx>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "addressconverter.hxx"
#include "biffinputstream.hxx"
#include "externallinkbuffer.hxx"
#include "formulaparser.hxx"
#include "worksheetbuffer.hxx"
#include "tokenarray.hxx"
#include "tokenuno.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ============================================================================

namespace {

const sal_uInt32 BIFF12_DEFNAME_HIDDEN      = 0x00000001;
const sal_uInt32 BIFF12_DEFNAME_FUNC        = 0x00000002;
const sal_uInt32 BIFF12_DEFNAME_VBNAME      = 0x00000004;
const sal_uInt32 BIFF12_DEFNAME_MACRO       = 0x00000008;
const sal_uInt32 BIFF12_DEFNAME_CALCEXP     = 0x00000010;
const sal_uInt32 BIFF12_DEFNAME_BUILTIN     = 0x00000020;
const sal_uInt32 BIFF12_DEFNAME_PUBLISHED   = 0x00008000;
const sal_uInt32 BIFF12_DEFNAME_WBPARAM     = 0x00010000;

const sal_uInt16 BIFF_DEFNAME_HIDDEN        = 0x0001;
const sal_uInt16 BIFF_DEFNAME_FUNC          = 0x0002;
const sal_uInt16 BIFF_DEFNAME_VBNAME        = 0x0004;
const sal_uInt16 BIFF_DEFNAME_MACRO         = 0x0008;
const sal_uInt16 BIFF_DEFNAME_CALCEXP       = 0x0010;
const sal_uInt16 BIFF_DEFNAME_BUILTIN       = 0x0020;
const sal_uInt16 BIFF_DEFNAME_BIG           = 0x1000;

const sal_uInt8 BIFF2_DEFNAME_FUNC          = 0x02;     /// BIFF2 function/command flag.

const sal_uInt16 BIFF_DEFNAME_GLOBAL        = 0;        /// 0 = Globally defined name.

const sal_uInt16 BIFF_REFFLAG_COL1REL       = 0x0001;
const sal_uInt16 BIFF_REFFLAG_ROW1REL       = 0x0002;
const sal_uInt16 BIFF_REFFLAG_COL2REL       = 0x0004;
const sal_uInt16 BIFF_REFFLAG_ROW2REL       = 0x0008;

// ----------------------------------------------------------------------------

const sal_Char* const spcLegacyPrefix = "Excel_BuiltIn_";
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

/** Localized names for _xlnm._FilterDatabase as used in BIFF5. */
const sal_Char* const sppcFilterDbNames[] =
{
    "_FilterDatabase",      // English
    "_FilterDatenbank"      // German
};

OUString lclGetBaseName( sal_Unicode cBuiltinId )
{
    OSL_ENSURE( cBuiltinId < STATIC_ARRAY_SIZE( sppcBaseNames ), "lclGetBaseName - unsupported built-in identifier" );
    OUStringBuffer aBuffer;
    if( cBuiltinId < STATIC_ARRAY_SIZE( sppcBaseNames ) )
        aBuffer.appendAscii( sppcBaseNames[ cBuiltinId ] );
    else
        aBuffer.append( static_cast< sal_Int32 >( cBuiltinId ) );
    return aBuffer.makeStringAndClear();
}

OUString lclGetPrefixedName( sal_Unicode cBuiltinId )
{
    return OUStringBuffer().appendAscii( spcOoxPrefix ).append( lclGetBaseName( cBuiltinId ) ).makeStringAndClear();
}

/** returns the built-in name identifier from a perfixed built-in name, e.g. '_xlnm.Print_Area'. */
sal_Unicode lclGetBuiltinIdFromPrefixedName( const OUString& rModelName )
{
    OUString aPrefix = OUString::createFromAscii( spcOoxPrefix );
    sal_Int32 nPrefixLen = aPrefix.getLength();
    if( rModelName.matchIgnoreAsciiCase( aPrefix ) )
    {
        for( sal_Unicode cBuiltinId = 0; cBuiltinId < STATIC_ARRAY_SIZE( sppcBaseNames ); ++cBuiltinId )
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
    for( sal_Unicode cBuiltinId = 0; cBuiltinId < STATIC_ARRAY_SIZE( sppcBaseNames ); ++cBuiltinId )
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

// ============================================================================

DefinedNameModel::DefinedNameModel() :
    mnSheet( -1 ),
    mnFuncGroupId( -1 ),
    mbMacro( false ),
    mbFunction( false ),
    mbVBName( false ),
    mbHidden( false )
{
}

// ============================================================================

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

ApiTokenSequence DefinedNameBase::importOoxFormula( sal_Int16 nBaseSheet )
{
    return (!maModel.maFormula.isEmpty()) ?
        getFormulaParser().importFormula( CellAddress( nBaseSheet, 0, 0 ), maModel.maFormula ) :
        getFormulaParser().convertErrorToFormula( BIFF_ERR_NAME );
}

ApiTokenSequence DefinedNameBase::importBiff12Formula( sal_Int16 nBaseSheet, SequenceInputStream& rStrm )
{
    return getFormulaParser().importFormula( CellAddress( nBaseSheet, 0, 0 ), FORMULATYPE_DEFINEDNAME, rStrm );
}

ApiTokenSequence DefinedNameBase::importBiffFormula( sal_Int16 nBaseSheet, BiffInputStream& rStrm, const sal_uInt16* pnFmlaSize )
{
    return (!pnFmlaSize || (*pnFmlaSize > 0)) ?
        getFormulaParser().importFormula( CellAddress( nBaseSheet, 0, 0 ), FORMULATYPE_DEFINEDNAME, rStrm, pnFmlaSize ) :
        getFormulaParser().convertErrorToFormula( BIFF_ERR_NAME );
}

// ============================================================================

DefinedName::DefinedName( const WorkbookHelper& rHelper ) :
    DefinedNameBase( rHelper ),
    mnTokenIndex( -1 ),
    mcBuiltinId( BIFF_DEFNAME_UNKNOWN ),
    mnFmlaSize( 0 )
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
        Built-in names are prexixed with '_xlnm.' instead. */
    mcBuiltinId = lclGetBuiltinIdFromPrefixedName( maModel.maName );
}

void DefinedName::setFormula( const OUString& rFormula )
{
    maModel.maFormula = rFormula;
}

void DefinedName::importDefinedName( SequenceInputStream& rStrm )
{
    sal_uInt32 nFlags;
    rStrm >> nFlags;
    rStrm.skip( 1 );    // keyboard shortcut
    rStrm >> maModel.mnSheet >> maModel.maName;
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

ApiTokenSequence
DefinedName::getTokens()
{
    // convert and set formula of the defined name
    ApiTokenSequence aTokens;
    switch( getFilterType() )
    {
        case FILTER_OOXML:
        {
            if( mxFormula.get() )
            {
                SequenceInputStream aStrm( *mxFormula );
                aTokens = importBiff12Formula( mnCalcSheet, aStrm );
            }
            else
                aTokens = importOoxFormula( mnCalcSheet );
        }
        break;
        case FILTER_BIFF:
        {
            OSL_ENSURE( mxBiffStrm.get(), "DefinedName::convertFormula - missing BIFF stream" );
            if( mxBiffStrm.get() )
            {
                BiffInputStream& rStrm = mxBiffStrm->getStream();
                BiffInputStreamPosGuard aStrmGuard( rStrm );
                if( mxBiffStrm->restorePosition() )
                    aTokens = importBiffFormula( mnCalcSheet, rStrm, &mnFmlaSize );
            }
        }
        break;
        case FILTER_UNKNOWN:
        break;
    }
    return aTokens;
}

void DefinedName::convertFormula()
{
    // macro function or vba procedure
    if(!mpScRangeData)
        return;

    // convert and set formula of the defined name
    if ( getFilterType() == FILTER_OOXML )
    {
        ApiTokenSequence aTokens = getTokens();
        ScTokenArray aTokenArray;
        (void)ScTokenConversion::ConvertToTokenArray( this->getScDocument(), aTokenArray, aTokens );
        mpScRangeData->SetCode( aTokenArray );
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
                xPrintAreas->setPrintAreas( ContainerHelper::vectorToSequence( aPrintRanges ) );
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
                for( ApiCellRangeList::const_iterator aIt = aTitleRanges.begin(), aEnd = aTitleRanges.end(); (aIt != aEnd) && (!bHasRowTitles || !bHasColTitles); ++aIt )
                {
                    bool bFullRow = (aIt->StartColumn == 0) && (aIt->EndColumn >= rMaxPos.Column);
                    bool bFullCol = (aIt->StartRow == 0) && (aIt->EndRow >= rMaxPos.Row);
                    if( !bHasRowTitles && bFullRow && !bFullCol )
                    {
                        xPrintAreas->setTitleRows( *aIt );
                        xPrintAreas->setPrintTitleRows( sal_True );
                        bHasRowTitles = true;
                    }
                    else if( !bHasColTitles && bFullCol && !bFullRow )
                    {
                        xPrintAreas->setTitleColumns( *aIt );
                        xPrintAreas->setPrintTitleColumns( sal_True );
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
    ScTokenConversion::ConvertToTokenSequence( this->getScDocument(), aFTokenSeq, *pTokenArray );
    return getFormulaParser().extractCellRange( orRange, aFTokenSeq, false );
}

// ============================================================================

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

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
