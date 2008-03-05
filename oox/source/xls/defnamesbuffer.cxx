/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: defnamesbuffer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:58:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/xls/defnamesbuffer.hxx"
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XFormulaTokens.hpp>
#include <com/sun/star/sheet/XPrintAreas.hpp>
#include <com/sun/star/sheet/ReferenceFlags.hpp>
#include <com/sun/star/sheet/SingleReference.hpp>
#include <com/sun/star/sheet/ComplexReference.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>
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
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;
using ::com::sun::star::sheet::SingleReference;
using ::com::sun::star::sheet::ComplexReference;
using ::com::sun::star::sheet::XNamedRanges;
using ::com::sun::star::sheet::XNamedRange;
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

} // namespace

// ============================================================================

OoxDefinedNameData::OoxDefinedNameData() :
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
    maOoxData.mnSheet = nLocalSheet;
}

Any DefinedNameBase::getReference( const CellAddress& rBaseAddress ) const
{
    if( maRefAny.hasValue() && (maOoxData.maName.getLength() >= 2) && (maOoxData.maName[ 0 ] == '\x01') )
    {
        sal_Unicode cFlagsChar = maOoxData.maName.toAsciiUpperCase()[ 1 ];
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
    if( maOoxData.maFormula.getLength() > 0 )
    {
        rContext.setBaseAddress( CellAddress( static_cast< sal_Int16 >( maOoxData.mnSheet ), 0, 0 ) );
        getFormulaParser().importFormula( rContext, maOoxData.maFormula );
    }
    else
        getFormulaParser().convertErrorToFormula( rContext, BIFF_ERR_NAME );
}

void DefinedNameBase::importOobFormula( FormulaContext& rContext, RecordInputStream& rStrm )
{
    rContext.setBaseAddress( CellAddress( static_cast< sal_Int16 >( maOoxData.mnSheet ), 0, 0 ) );
    getFormulaParser().importFormula( rContext, rStrm );
}

void DefinedNameBase::importBiffFormula( FormulaContext& rContext, BiffInputStream& rStrm, const sal_uInt16* pnFmlaSize )
{
    rContext.setBaseAddress( CellAddress( static_cast< sal_Int16 >( maOoxData.mnSheet ), 0, 0 ) );
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

namespace {

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

OUString lclGetFinalName( sal_Unicode cBuiltinId )
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

// ----------------------------------------------------------------------------

DefinedName::DefinedName( const WorkbookHelper& rHelper, sal_Int32 nLocalSheet ) :
    DefinedNameBase( rHelper, nLocalSheet ),
    mnTokenIndex( -1 ),
    mcBuiltinId( OOX_DEFNAME_UNKNOWN ),
    mnFmlaSize( 0 )
{
}

void DefinedName::importDefinedName( const AttributeList& rAttribs )
{
    maOoxData.maName        = rAttribs.getString( XML_name );
    maOoxData.mnSheet       = rAttribs.getInteger( XML_localSheetId, -1 );
    maOoxData.mnFuncGroupId = rAttribs.getInteger( XML_functionGroupId, -1 );
    maOoxData.mbMacro       = rAttribs.getBool( XML_xlm, false );
    maOoxData.mbFunction    = rAttribs.getBool( XML_function, false );
    maOoxData.mbVBName      = rAttribs.getBool( XML_vbProcedure, false );
    maOoxData.mbHidden      = rAttribs.getBool( XML_hidden, false );
    mcBuiltinId = lclGetBuiltinIdFromOox( maOoxData.maName );
}

void DefinedName::setFormula( const OUString& rFormula )
{
    maOoxData.maFormula = rFormula;
}

void DefinedName::importDefinedName( RecordInputStream& rStrm )
{
    sal_uInt32 nFlags;
    rStrm >> nFlags;
    rStrm.skip( 1 );    // keyboard shortcut
    rStrm >> maOoxData.mnSheet >> maOoxData.maName;

    // macro function/command, hidden flag
    maOoxData.mnFuncGroupId = extractValue< sal_Int32 >( nFlags, 6, 9 );
    maOoxData.mbMacro       = getFlag( nFlags, OOBIN_DEFNAME_MACRO );
    maOoxData.mbFunction    = getFlag( nFlags, OOBIN_DEFNAME_FUNC );
    maOoxData.mbVBName      = getFlag( nFlags, OOBIN_DEFNAME_VBNAME );
    maOoxData.mbHidden      = getFlag( nFlags, OOBIN_DEFNAME_HIDDEN );

    // get builtin name index from name
    if( getFlag( nFlags, OOBIN_DEFNAME_BUILTIN ) )
        mcBuiltinId = lclGetBuiltinIdFromOob( maOoxData.maName );
    // unhide built-in names (_xlnm._FilterDatabase is always hidden)
    if( isBuiltinName() )
        maOoxData.mbHidden = false;

    // store token array data
    sal_Int32 nRecPos = rStrm.getRecPos();
    sal_Int32 nFmlaSize = rStrm.readInt32();
    rStrm.skip( nFmlaSize );
    sal_Int32 nAddDataSize = rStrm.readInt32();
    if( rStrm.isValid() && (nFmlaSize > 0) && (nAddDataSize >= 0) && (rStrm.getRecLeft() >= nAddDataSize) )
    {
        sal_Int32 nTotalSize = 8 + nFmlaSize + nAddDataSize;
        mxFormula.reset( new RecordDataSequence( nTotalSize ) );
        rStrm.seek( nRecPos );
        rStrm.read( mxFormula->getArray(), nTotalSize );
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
            maOoxData.maName = rStrm.readCharArray( nNameLen, getTextEncoding() );
        }
        break;
        case BIFF3:
        case BIFF4:
            rStrm >> nFlags >> nShortCut >> nNameLen >> mnFmlaSize;
            maOoxData.maName = rStrm.readCharArray( nNameLen, getTextEncoding() );
        break;
        case BIFF5:
            rStrm >> nFlags >> nShortCut >> nNameLen >> mnFmlaSize >> nRefId >> nTabId;
            maOoxData.maName = rStrm.skip( 4 ).readCharArray( nNameLen, getTextEncoding() );
        break;
        case BIFF8:
            rStrm >> nFlags >> nShortCut >> nNameLen >> mnFmlaSize >> nRefId >> nTabId;
            maOoxData.maName = rStrm.skip( 4 ).readUniString( nNameLen );
        break;
        case BIFF_UNKNOWN: break;
    }
    rStrm.enableNulChars( false );

    // macro function/command, hidden flag
    maOoxData.mnFuncGroupId = extractValue< sal_Int32 >( nFlags, 6, 6 );
    maOoxData.mbMacro       = getFlag( nFlags, BIFF_DEFNAME_MACRO );
    maOoxData.mbFunction    = getFlag( nFlags, BIFF_DEFNAME_FUNC );
    maOoxData.mbVBName      = getFlag( nFlags, BIFF_DEFNAME_VBNAME );
    maOoxData.mbHidden      = getFlag( nFlags, BIFF_DEFNAME_HIDDEN );

    // get builtin name index from name
    if( getFlag( nFlags, BIFF_DEFNAME_BUILTIN ) )
    {
        OSL_ENSURE( maOoxData.maName.getLength() == 1, "DefinedName::importDefinedName - wrong builtin name" );
        if( maOoxData.maName.getLength() > 0 )
            mcBuiltinId = maOoxData.maName[ 0 ];
    }
    /*  In BIFF5, _xlnm._FilterDatabase appears as hidden user name without
        built-in flag, and even worse, localized. */
    else if( (eBiff == BIFF5) && lclIsFilterDatabaseName( maOoxData.maName ) )
    {
        mcBuiltinId = OOX_DEFNAME_FILTERDATABASE;
    }

    // unhide built-in names (_xlnm._FilterDatabase is always hidden)
    if( isBuiltinName() )
        maOoxData.mbHidden = false;

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
                        maOoxData.mnSheet = pExtLink->getSheetIndex();
        break;
        case BIFF8:
            // one-based sheet index
            if( nTabId != BIFF_DEFNAME_GLOBAL )
                maOoxData.mnSheet = nTabId - 1;
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
    if( maOoxData.mbHidden || maOoxData.mbFunction )
        return;

    // convert original name to final Calc name
    if( maOoxData.mbVBName )
        maFinalName = maOoxData.maName;
    else if( isBuiltinName() )
        maFinalName = lclGetFinalName( mcBuiltinId );
    else
        maFinalName = maOoxData.maName;         //! TODO convert to valid name

    // append sheet index for local names in multi-sheet documents
    if( isWorkbookFile() && !isGlobalName() )
        maFinalName = OUStringBuffer( maFinalName ).append( sal_Unicode( '_' ) ).append( maOoxData.mnSheet + 1 ).makeStringAndClear();

    // special flags for this name
    sal_Int32 nNameFlags = 0;
    using namespace ::com::sun::star::sheet::NamedRangeFlag;
    if( !isGlobalName() ) switch( mcBuiltinId )
    {
        case OOX_DEFNAME_CRITERIA:      nNameFlags = FILTER_CRITERIA;               break;
        case OOX_DEFNAME_PRINTAREA:     nNameFlags = PRINT_AREA;                    break;
        case OOX_DEFNAME_PRINTTITLES:   nNameFlags = COLUMN_HEADER | ROW_HEADER;    break;
    }

    // create the name and insert it into the document, maFinalName will be changed to the resulting name
    mxNamedRange = getDefinedNames().createDefinedName( maFinalName, nNameFlags );
    // index of this defined name used in formula token arrays
    mnTokenIndex = getDefinedNames().getTokenIndex( mxNamedRange );
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
                Reference< XPrintAreas > xPrintAreas( getSheet( maOoxData.mnSheet ), UNO_QUERY );
                ApiCellRangeList aPrintRanges;
                getFormulaParser().extractCellRangeList( aPrintRanges, xTokens->getTokens(), maOoxData.mnSheet );
                if( xPrintAreas.is() && !aPrintRanges.empty() )
                    xPrintAreas->setPrintAreas( ContainerHelper::vectorToSequence( aPrintRanges ) );
            }
            break;
            case OOX_DEFNAME_PRINTTITLES:
            {
                Reference< XPrintAreas > xPrintAreas( getSheet( maOoxData.mnSheet ), UNO_QUERY );
                ApiCellRangeList aTitleRanges;
                getFormulaParser().extractCellRangeList( aTitleRanges, xTokens->getTokens(), maOoxData.mnSheet );
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
    else if( mxBiffStrm.get() && maOoxData.mbHidden && (maOoxData.maName.getLength() > 0) && (maOoxData.maName[ 0 ] == '\x01') )
    {
        // import BIFF2-BIFF4 external references
        TokensFormulaContext aContext( true, true );
        implImportBiffFormula( aContext );
        setReference( aContext.getTokens() );
    }
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
    BiffInputStreamGuard aStrmGuard( rStrm );
    if( mxBiffStrm->restorePosition() )
        importBiffFormula( rContext, rStrm, &mnFmlaSize );
}

// ============================================================================

DefinedNamesBuffer::DefinedNamesBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    maTokenIndexProp( CREATE_OUSTRING( "TokenIndex" ) ),
    mnLocalSheet( -1 )
{
}

Reference< XNamedRange > DefinedNamesBuffer::createDefinedName( OUString& orName, sal_Int32 nNameFlags ) const
{
    // find an unused name
    Reference< XNamedRanges > xNamedRanges = getNamedRanges();
    Reference< XNameAccess > xNameAccess( xNamedRanges, UNO_QUERY );
    if( xNameAccess.is() )
        orName = ContainerHelper::getUnusedName( xNameAccess, orName, '_' );

    // create the name and insert it into the Calc document
    Reference< XNamedRange > xNamedRange;
    if( xNamedRanges.is() && (orName.getLength() > 0) ) try
    {
        xNamedRanges->addNewByName( orName, OUString(), CellAddress( 0, 0, 0 ), nNameFlags );
        xNamedRange.set( xNamedRanges->getByName( orName ), UNO_QUERY );
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "DefinedNamesBuffer::createDefinedName - cannot create defined name" );
    }
    return xNamedRange;
}

sal_Int32 DefinedNamesBuffer::getTokenIndex( const Reference< XNamedRange >& rxNamedRange ) const
{
    PropertySet aPropSet( rxNamedRange );
    sal_Int32 nIndex = -1;
    return aPropSet.getProperty( nIndex, maTokenIndexProp ) ? nIndex : -1;
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
    /*  First insert all names without formula definition into the document. */
    maDefNames.forEachMem( &DefinedName::createNameObject );
    /*  Now convert all name formulas, so that the formula parser can find all
        names in case of circular dependencies. */
    maDefNames.forEachMem( &DefinedName::convertFormula );
}

DefinedNameRef DefinedNamesBuffer::getByIndex( sal_Int32 nIndex ) const
{
    return maDefNames.get( nIndex );
}

DefinedNameRef DefinedNamesBuffer::getByOoxName( const OUString& rOoxName, sal_Int32 nSheet ) const
{
    DefinedNameRef xGlobalName;   // a found global name
    DefinedNameRef xLocalName;    // a found local name
    for( DefNameVec::const_iterator aIt = maDefNames.begin(), aEnd = maDefNames.end(); (aIt != aEnd) && !xLocalName; ++aIt )
    {
        DefinedNameRef xCurrName = *aIt;
        if( xCurrName->getOoxName() == rOoxName )
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

