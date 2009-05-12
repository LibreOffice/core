/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: defnamesbuffer.cxx,v $
 * $Revision: 1.5.4.2 $
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

#include "oox/xls/defnamesbuffer.hxx"
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/sheet/ComplexReference.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>
#include <com/sun/star/sheet/ReferenceFlags.hpp>
#include <com/sun/star/sheet/SingleReference.hpp>
#include <com/sun/star/sheet/XFormulaTokens.hpp>
#include <com/sun/star/sheet/XPrintAreas.hpp>
#include "properties.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/externallinkbuffer.hxx"
#include "oox/xls/formulaparser.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;
using ::com::sun::star::sheet::ComplexReference;
using ::com::sun::star::sheet::SingleReference;
using ::com::sun::star::sheet::XFormulaTokens;
using ::com::sun::star::sheet::XPrintAreas;
using namespace ::com::sun::star::sheet::ReferenceFlags;

namespace oox {
namespace xls {

// ============================================================================

namespace {

const sal_uInt32 OOBIN_DEFNAME_HIDDEN       = 0x00000001;
const sal_uInt32 OOBIN_DEFNAME_FUNC         = 0x00000002;
const sal_uInt32 OOBIN_DEFNAME_VBNAME       = 0x00000004;
const sal_uInt32 OOBIN_DEFNAME_MACRO        = 0x00000008;
const sal_uInt32 OOBIN_DEFNAME_CALCEXP      = 0x00000010;
const sal_uInt32 OOBIN_DEFNAME_BUILTIN      = 0x00000020;
const sal_uInt32 OOBIN_DEFNAME_PUBLISHED    = 0x00008000;
const sal_uInt32 OOBIN_DEFNAME_WBPARAM      = 0x00010000;

const sal_uInt16 BIFF_DEFNAME_HIDDEN        = 0x0001;
const sal_uInt16 BIFF_DEFNAME_FUNC          = 0x0002;
const sal_uInt16 BIFF_DEFNAME_VBNAME        = 0x0004;
const sal_uInt16 BIFF_DEFNAME_MACRO         = 0x0008;
const sal_uInt16 BIFF_DEFNAME_CALCEXP       = 0x0010;
const sal_uInt16 BIFF_DEFNAME_BUILTIN       = 0x0020;
const sal_uInt16 BIFF_DEFNAME_BIG           = 0x1000;

const sal_uInt8 BIFF2_DEFNAME_FUNC          = 0x02;     /// BIFF2 function/command flag.

const sal_uInt16 BIFF_DEFNAME_GLOBAL        = 0;        /// 0 = Globally defined name.

// ----------------------------------------------------------------------------

const sal_Char* const spcLegacyPrefix = "Excel_BuiltIn_";
const sal_Char* const spcOoxPrefix = "_xlnm.";

const sal_Char* const sppcBaseNames[] =
{
    "Consolidate_Area", /* OOX */
    "Auto_Open",
    "Auto_Close",
    "Extract",          /* OOX */
    "Database",         /* OOX */
    "Criteria",         /* OOX */
    "Print_Area",       /* OOX */
    "Print_Titles",     /* OOX */
    "Recorder",
    "Data_Form",
    "Auto_Activate",
    "Auto_Deactivate",
    "Sheet_Title",      /* OOX */
    "_FilterDatabase"   /* OOX */
};

/** Localized names for _xlnm._FilterDatabase as used in BIFF5. */
const sal_Char* const sppcFilterDbNames[] =
{
    "_FilterDatabase",      // English
    "_FilterDatenbank"      // German
};

OUString lclGetBaseName( sal_Unicode cBuiltinId )
{
    OSL_ENSURE( cBuiltinId < STATIC_ARRAY_SIZE( sppcBaseNames ), "lclGetBaseName - unknown builtin name" );
    OUStringBuffer aBuffer;
    if( cBuiltinId < STATIC_ARRAY_SIZE( sppcBaseNames ) )
        aBuffer.appendAscii( sppcBaseNames[ cBuiltinId ] );
    else
        aBuffer.append( static_cast< sal_Int32 >( cBuiltinId ) );
    return aBuffer.makeStringAndClear();
}

OUString lclGetBuiltinName( sal_Unicode cBuiltinId )
{
    return OUStringBuffer().appendAscii( spcOoxPrefix ).append( lclGetBaseName( cBuiltinId ) ).makeStringAndClear();
}

sal_Unicode lclGetBuiltinIdFromOox( const OUString& rOoxName )
{
    OUString aPrefix = OUString::createFromAscii( spcOoxPrefix );
    sal_Int32 nPrefixLen = aPrefix.getLength();
    if( rOoxName.matchIgnoreAsciiCase( aPrefix ) )
    {
        for( sal_Unicode cBuiltinId = 0; cBuiltinId < STATIC_ARRAY_SIZE( sppcBaseNames ); ++cBuiltinId )
        {
            OUString aBaseName = lclGetBaseName( cBuiltinId );
            sal_Int32 nBaseNameLen = aBaseName.getLength();
            if( (rOoxName.getLength() == nPrefixLen + nBaseNameLen) && rOoxName.matchIgnoreAsciiCase( aBaseName, nPrefixLen ) )
                return cBuiltinId;
        }
    }
    return OOX_DEFNAME_UNKNOWN;
}

sal_Unicode lclGetBuiltinIdFromOob( const OUString& rOobName )
{
    for( sal_Unicode cBuiltinId = 0; cBuiltinId < STATIC_ARRAY_SIZE( sppcBaseNames ); ++cBuiltinId )
        if( rOobName.equalsIgnoreAsciiCaseAscii( sppcBaseNames[ cBuiltinId ] ) )
            return cBuiltinId;
    return OOX_DEFNAME_UNKNOWN;
}

bool lclIsFilterDatabaseName( const OUString& rName )
{
    for( const sal_Char* const* ppcName = sppcFilterDbNames; ppcName < STATIC_ARRAY_END( sppcFilterDbNames ); ++ppcName )
        if( rName.equalsIgnoreAsciiCaseAscii( *ppcName ) )
            return true;
    return false;
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

namespace {

const sal_uInt16 BIFF_REFFLAG_COL1REL       = 0x0001;
const sal_uInt16 BIFF_REFFLAG_ROW1REL       = 0x0002;
const sal_uInt16 BIFF_REFFLAG_COL2REL       = 0x0004;
const sal_uInt16 BIFF_REFFLAG_ROW2REL       = 0x0008;

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

void lclConvertSingleRefFlags( SingleReference& orApiRef, const CellAddress& rBaseAddress, bool bColRel, bool bRowRel )
{
    lclConvertRefFlags(
        orApiRef.Flags, orApiRef.Column, orApiRef.RelativeColumn,
        rBaseAddress.Column, COLUMN_RELATIVE, bColRel );
    lclConvertRefFlags(
        orApiRef.Flags, orApiRef.Row, orApiRef.RelativeRow,
        rBaseAddress.Row, ROW_RELATIVE, bRowRel );
}

} // namespace

// ----------------------------------------------------------------------------

DefinedNameBase::DefinedNameBase( const WorkbookHelper& rHelper, sal_Int32 nLocalSheet ) :
    WorkbookHelper( rHelper )
{
    maModel.mnSheet = nLocalSheet;
}

const OUString& DefinedNameBase::getUpcaseModelName() const
{
    if( maUpModelName.getLength() == 0 )
        maUpModelName = maModel.maName.toAsciiUpperCase();
    return maUpModelName;
}

Any DefinedNameBase::getReference( const CellAddress& rBaseAddress ) const
{
    if( maRefAny.hasValue() && (maModel.maName.getLength() >= 2) && (maModel.maName[ 0 ] == '\x01') )
    {
        sal_Unicode cFlagsChar = getUpcaseModelName()[ 1 ];
        if( ('A' <= cFlagsChar) && (cFlagsChar <= 'P') )
        {
            sal_uInt16 nFlags = static_cast< sal_uInt16 >( cFlagsChar - 'A' );
            if( maRefAny.has< SingleReference >() && (cFlagsChar <= 'D') )
            {
                SingleReference aApiRef;
                maRefAny >>= aApiRef;
                lclConvertSingleRefFlags( aApiRef, rBaseAddress, getFlag( nFlags, BIFF_REFFLAG_COL1REL ), getFlag( nFlags, BIFF_REFFLAG_ROW1REL ) );
                return Any( aApiRef );
            }
            if( maRefAny.has< ComplexReference >() )
            {
                ComplexReference aApiRef;
                maRefAny >>= aApiRef;
                lclConvertSingleRefFlags( aApiRef.Reference1, rBaseAddress, getFlag( nFlags, BIFF_REFFLAG_COL1REL ), getFlag( nFlags, BIFF_REFFLAG_ROW1REL ) );
                lclConvertSingleRefFlags( aApiRef.Reference2, rBaseAddress, getFlag( nFlags, BIFF_REFFLAG_COL2REL ), getFlag( nFlags, BIFF_REFFLAG_ROW2REL ) );
                return Any( aApiRef );
            }
        }
    }
    return Any();
}

void DefinedNameBase::importOoxFormula( FormulaContext& rContext )
{
    if( maModel.maFormula.getLength() > 0 )
    {
        rContext.setBaseAddress( CellAddress( static_cast< sal_Int16 >( maModel.mnSheet ), 0, 0 ) );
        getFormulaParser().importFormula( rContext, maModel.maFormula );
    }
    else
        getFormulaParser().convertErrorToFormula( rContext, BIFF_ERR_NAME );
}

void DefinedNameBase::importOobFormula( FormulaContext& rContext, RecordInputStream& rStrm )
{
    rContext.setBaseAddress( CellAddress( static_cast< sal_Int16 >( maModel.mnSheet ), 0, 0 ) );
    getFormulaParser().importFormula( rContext, rStrm );
}

void DefinedNameBase::importBiffFormula( FormulaContext& rContext, BiffInputStream& rStrm, const sal_uInt16* pnFmlaSize )
{
    rContext.setBaseAddress( CellAddress( static_cast< sal_Int16 >( maModel.mnSheet ), 0, 0 ) );
    if( !pnFmlaSize || (*pnFmlaSize > 0) )
        getFormulaParser().importFormula( rContext, rStrm, pnFmlaSize );
    else
        getFormulaParser().convertErrorToFormula( rContext, BIFF_ERR_NAME );
}

void DefinedNameBase::setReference( const ApiTokenSequence& rTokens )
{
    maRefAny = getFormulaParser().extractReference( rTokens );
}

// ============================================================================

DefinedName::DefinedName( const WorkbookHelper& rHelper, sal_Int32 nLocalSheet ) :
    DefinedNameBase( rHelper, nLocalSheet ),
    mnTokenIndex( -1 ),
    mcBuiltinId( OOX_DEFNAME_UNKNOWN ),
    mnFmlaSize( 0 )
{
}

void DefinedName::importDefinedName( const AttributeList& rAttribs )
{
    maModel.maName        = rAttribs.getString( XML_name, OUString() );
    maModel.mnSheet       = rAttribs.getInteger( XML_localSheetId, -1 );
    maModel.mnFuncGroupId = rAttribs.getInteger( XML_functionGroupId, -1 );
    maModel.mbMacro       = rAttribs.getBool( XML_xlm, false );
    maModel.mbFunction    = rAttribs.getBool( XML_function, false );
    maModel.mbVBName      = rAttribs.getBool( XML_vbProcedure, false );
    maModel.mbHidden      = rAttribs.getBool( XML_hidden, false );
    mcBuiltinId = lclGetBuiltinIdFromOox( maModel.maName );
}

void DefinedName::setFormula( const OUString& rFormula )
{
    maModel.maFormula = rFormula;
}

void DefinedName::importDefinedName( RecordInputStream& rStrm )
{
    sal_uInt32 nFlags;
    rStrm >> nFlags;
    rStrm.skip( 1 );    // keyboard shortcut
    rStrm >> maModel.mnSheet >> maModel.maName;

    // macro function/command, hidden flag
    maModel.mnFuncGroupId = extractValue< sal_Int32 >( nFlags, 6, 9 );
    maModel.mbMacro       = getFlag( nFlags, OOBIN_DEFNAME_MACRO );
    maModel.mbFunction    = getFlag( nFlags, OOBIN_DEFNAME_FUNC );
    maModel.mbVBName      = getFlag( nFlags, OOBIN_DEFNAME_VBNAME );
    maModel.mbHidden      = getFlag( nFlags, OOBIN_DEFNAME_HIDDEN );

    // get builtin name index from name
    if( getFlag( nFlags, OOBIN_DEFNAME_BUILTIN ) )
        mcBuiltinId = lclGetBuiltinIdFromOob( maModel.maName );
    // unhide built-in names (_xlnm._FilterDatabase is always hidden)
    if( isBuiltinName() )
        maModel.mbHidden = false;

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

void DefinedName::importDefinedName( BiffInputStream& rStrm )
{
    BiffType eBiff = getBiff();
    sal_uInt16 nFlags = 0;
    sal_Int16 nRefId = BIFF_DEFNAME_GLOBAL;
    sal_Int16 nTabId = BIFF_DEFNAME_GLOBAL;
    sal_uInt8 nNameLen = 0, nShortCut = 0;

    rStrm.enableNulChars( true );
    switch( eBiff )
    {
        case BIFF2:
        {
            sal_uInt8 nFlagsBiff2;
            rStrm >> nFlagsBiff2;
            rStrm.skip( 1 );
            rStrm >> nShortCut >> nNameLen;
            mnFmlaSize = rStrm.readuInt8();
            setFlag( nFlags, BIFF_DEFNAME_FUNC, getFlag( nFlagsBiff2, BIFF2_DEFNAME_FUNC ) );
            maModel.maName = rStrm.readCharArray( nNameLen, getTextEncoding() );
        }
        break;
        case BIFF3:
        case BIFF4:
            rStrm >> nFlags >> nShortCut >> nNameLen >> mnFmlaSize;
            maModel.maName = rStrm.readCharArray( nNameLen, getTextEncoding() );
        break;
        case BIFF5:
            rStrm >> nFlags >> nShortCut >> nNameLen >> mnFmlaSize >> nRefId >> nTabId;
            rStrm.skip( 4 );
            maModel.maName = rStrm.readCharArray( nNameLen, getTextEncoding() );
        break;
        case BIFF8:
            rStrm >> nFlags >> nShortCut >> nNameLen >> mnFmlaSize >> nRefId >> nTabId;
            rStrm.skip( 4 );
            maModel.maName = rStrm.readUniString( nNameLen );
        break;
        case BIFF_UNKNOWN: break;
    }
    rStrm.enableNulChars( false );

    // macro function/command, hidden flag
    maModel.mnFuncGroupId = extractValue< sal_Int32 >( nFlags, 6, 6 );
    maModel.mbMacro       = getFlag( nFlags, BIFF_DEFNAME_MACRO );
    maModel.mbFunction    = getFlag( nFlags, BIFF_DEFNAME_FUNC );
    maModel.mbVBName      = getFlag( nFlags, BIFF_DEFNAME_VBNAME );
    maModel.mbHidden      = getFlag( nFlags, BIFF_DEFNAME_HIDDEN );

    // get builtin name index from name
    if( getFlag( nFlags, BIFF_DEFNAME_BUILTIN ) )
    {
        OSL_ENSURE( maModel.maName.getLength() == 1, "DefinedName::importDefinedName - wrong builtin name" );
        if( maModel.maName.getLength() > 0 )
            mcBuiltinId = maModel.maName[ 0 ];
    }
    /*  In BIFF5, _xlnm._FilterDatabase appears as hidden user name without
        built-in flag, and even worse, localized. */
    else if( (eBiff == BIFF5) && lclIsFilterDatabaseName( maModel.maName ) )
    {
        mcBuiltinId = OOX_DEFNAME_FILTERDATABASE;
    }

    // unhide built-in names (_xlnm._FilterDatabase is always hidden)
    if( isBuiltinName() )
        maModel.mbHidden = false;

    // get sheet index for sheet-local names in BIFF5-BIFF8
    switch( getBiff() )
    {
        case BIFF2:
        case BIFF3:
        case BIFF4:
        break;
        case BIFF5:
            // #i44019# nTabId may be invalid, resolve nRefId to sheet index
            if( nRefId != BIFF_DEFNAME_GLOBAL )
                if( const ExternalLink* pExtLink = getExternalLinks().getExternalLink( nRefId ).get() )
                    if( pExtLink->getLinkType() == LINKTYPE_INTERNAL )
                        maModel.mnSheet = pExtLink->getSheetIndex();
        break;
        case BIFF8:
            // convert one-based sheet index to zero-based
            OSL_ENSURE( nTabId >= 0, "DefinedName::importDefinedName - invalid local sheet index" );
            if( nTabId != BIFF_DEFNAME_GLOBAL )
                maModel.mnSheet = nTabId - 1;
        break;
        case BIFF_UNKNOWN:
        break;
    }

    // store record position to be able to import token array later
    mxBiffStrm.reset( new BiffInputStreamPos( rStrm ) );
}

void DefinedName::createNameObject()
{
    // do not create hidden names and names for (macro) functions
    if( maModel.mbHidden || maModel.mbFunction )
        return;

    // convert original name to final Calc name
    if( maModel.mbVBName )
        maCalcName = maModel.maName;
    else if( isBuiltinName() )
        maCalcName = lclGetBuiltinName( mcBuiltinId );
    else
        maCalcName = maModel.maName;         //! TODO convert to valid name

    // append sheet index for local names in multi-sheet documents
    if( isWorkbookFile() && !isGlobalName() )
        maCalcName = OUStringBuffer( maCalcName ).append( sal_Unicode( '_' ) ).append( maModel.mnSheet + 1 ).makeStringAndClear();

    // special flags for this name
    sal_Int32 nNameFlags = 0;
    using namespace ::com::sun::star::sheet::NamedRangeFlag;
    if( !isGlobalName() ) switch( mcBuiltinId )
    {
        case OOX_DEFNAME_CRITERIA:      nNameFlags = FILTER_CRITERIA;               break;
        case OOX_DEFNAME_PRINTAREA:     nNameFlags = PRINT_AREA;                    break;
        case OOX_DEFNAME_PRINTTITLES:   nNameFlags = COLUMN_HEADER | ROW_HEADER;    break;
    }

    // create the name and insert it into the document, maCalcName will be changed to the resulting name
    mxNamedRange = createNamedRangeObject( maCalcName, nNameFlags );
    // index of this defined name used in formula token arrays
    PropertySet aPropSet( mxNamedRange );
    aPropSet.getProperty( mnTokenIndex, PROP_TokenIndex );
}

void DefinedName::convertFormula()
{
    Reference< XFormulaTokens > xTokens( mxNamedRange, UNO_QUERY );
    if( xTokens.is() )
    {
        // convert and set formula of the defined name
        switch( getFilterType() )
        {
            case FILTER_OOX:
            {
                SimpleFormulaContext aContext( xTokens, true, false );
                implImportOoxFormula( aContext );
            }
            break;
            case FILTER_BIFF:
            {
                SimpleFormulaContext aContext( xTokens, true, getBiff() <= BIFF4 );
                implImportBiffFormula( aContext );
            }
            break;
            case FILTER_UNKNOWN: break;
        }

        // set builtin names (print ranges, repeated titles, filter ranges)
        if( !isGlobalName() ) switch( mcBuiltinId )
        {
            case OOX_DEFNAME_PRINTAREA:
            {
                Reference< XPrintAreas > xPrintAreas( getSheetFromDoc( maModel.mnSheet ), UNO_QUERY );
                ApiCellRangeList aPrintRanges;
                getFormulaParser().extractCellRangeList( aPrintRanges, xTokens->getTokens(), maModel.mnSheet );
                if( xPrintAreas.is() && !aPrintRanges.empty() )
                    xPrintAreas->setPrintAreas( ContainerHelper::vectorToSequence( aPrintRanges ) );
            }
            break;
            case OOX_DEFNAME_PRINTTITLES:
            {
                Reference< XPrintAreas > xPrintAreas( getSheetFromDoc( maModel.mnSheet ), UNO_QUERY );
                ApiCellRangeList aTitleRanges;
                getFormulaParser().extractCellRangeList( aTitleRanges, xTokens->getTokens(), maModel.mnSheet );
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
    else if( mxBiffStrm.get() && maModel.mbHidden && (maModel.maName.getLength() > 0) && (maModel.maName[ 0 ] == '\x01') )
    {
        // import BIFF2-BIFF4 external references
        TokensFormulaContext aContext( true, true );
        implImportBiffFormula( aContext );
        setReference( aContext.getTokens() );
    }
}

bool DefinedName::getAbsoluteRange( CellRangeAddress& orRange ) const
{
    /*  ScNamedRangeObj::XCellRangeReferrer::getReferredCells is buggy with
        relative references, so we extract an absolute reference by hand. */
    Reference< XFormulaTokens > xTokens( mxNamedRange, UNO_QUERY );
    return xTokens.is() && getFormulaParser().extractAbsoluteRange( orRange, xTokens->getTokens() );
}

void DefinedName::implImportOoxFormula( FormulaContext& rContext )
{
    if( mxFormula.get() )
    {
        RecordInputStream aStrm( *mxFormula );
        importOobFormula( rContext, aStrm );
    }
    else
        importOoxFormula( rContext );
}

void DefinedName::implImportBiffFormula( FormulaContext& rContext )
{
    OSL_ENSURE( mxBiffStrm.get(), "DefinedName::importBiffFormula - missing BIFF stream" );
    BiffInputStream& rStrm = mxBiffStrm->getStream();
    BiffInputStreamPosGuard aStrmGuard( rStrm );
    if( mxBiffStrm->restorePosition() )
        importBiffFormula( rContext, rStrm, &mnFmlaSize );
}

// ============================================================================

DefinedNamesBuffer::DefinedNamesBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mnLocalSheet( -1 )
{
}

void DefinedNamesBuffer::setLocalSheetIndex( sal_Int32 nLocalSheet )
{
    mnLocalSheet = nLocalSheet;
}

DefinedNameRef DefinedNamesBuffer::importDefinedName( const AttributeList& rAttribs )
{
    DefinedNameRef xDefName = createDefinedName();
    xDefName->importDefinedName( rAttribs );
    return xDefName;
}

void DefinedNamesBuffer::importDefinedName( RecordInputStream& rStrm )
{
    createDefinedName()->importDefinedName( rStrm );
}

void DefinedNamesBuffer::importDefinedName( BiffInputStream& rStrm )
{
    createDefinedName()->importDefinedName( rStrm );
}

void DefinedNamesBuffer::finalizeImport()
{
    // first insert all names without formula definition into the document
    for( DefNameVector::iterator aIt = maDefNames.begin(), aEnd = maDefNames.end(); aIt != aEnd; ++aIt )
    {
        DefinedNameRef xDefName = *aIt;
        xDefName->createNameObject();
        sal_Int32 nTokenIndex = xDefName->getTokenIndex();
        if( nTokenIndex >= 0 )
            maDefNameMap[ nTokenIndex ] = xDefName;
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
    return maDefNameMap.get( nIndex );
}

DefinedNameRef DefinedNamesBuffer::getByModelName( const OUString& rModelName, sal_Int32 nSheet ) const
{
    DefinedNameRef xGlobalName;   // a found global name
    DefinedNameRef xLocalName;    // a found local name
    for( DefNameVector::const_iterator aIt = maDefNames.begin(), aEnd = maDefNames.end(); (aIt != aEnd) && !xLocalName; ++aIt )
    {
        DefinedNameRef xCurrName = *aIt;
        if( xCurrName->getModelName() == rModelName )
        {
            if( xCurrName->getSheetIndex() == nSheet )
                xLocalName = xCurrName;
            else if( xCurrName->isGlobalName() )
                xGlobalName = xCurrName;
        }
    }
    return xLocalName.get() ? xLocalName : xGlobalName;
}

DefinedNameRef DefinedNamesBuffer::createDefinedName()
{
    DefinedNameRef xDefName( new DefinedName( *this, mnLocalSheet ) );
    maDefNames.push_back( xDefName );
    return xDefName;
}

// ============================================================================

} // namespace xls
} // namespace oox

