/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: externallinkbuffer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:59:59 $
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
 *
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

#include "oox/xls/externallinkbuffer.hxx"
#include <rtl/strbuf.hxx>
#include <com/sun/star/sheet/XDDELinks.hpp>
#include <com/sun/star/sheet/XDDELink.hpp>
#include <com/sun/star/sheet/XDDELinkResults.hpp>
#include "oox/helper/attributelist.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/excelhandlers.hxx"
#include "oox/xls/formulaparser.hxx"
#include "oox/xls/worksheetbuffer.hxx"

using ::rtl::OString;
using ::rtl::OStringBuffer;
using ::rtl::OStringToOUString;
using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::sheet::XDDELinks;
using ::com::sun::star::sheet::XDDELinkResults;
using ::oox::core::Relations;

namespace oox {
namespace xls {

// ============================================================================

namespace {

const sal_uInt16 OOBIN_EXTERNALBOOK_BOOK    = 0;
const sal_uInt16 OOBIN_EXTERNALBOOK_DDE     = 1;
const sal_uInt16 OOBIN_EXTERNALBOOK_OLE     = 2;

const sal_uInt16 OOBIN_EXTNAME_AUTOMATIC    = 0x0002;
const sal_uInt16 OOBIN_EXTNAME_PREFERPIC    = 0x0004;
const sal_uInt16 OOBIN_EXTNAME_STDDOCNAME   = 0x0008;
const sal_uInt16 OOBIN_EXTNAME_OLEOBJECT    = 0x0010;
const sal_uInt16 OOBIN_EXTNAME_ICONIFIED    = 0x0020;

const sal_uInt16 BIFF_EXTNAME_BUILTIN       = 0x0001;
const sal_uInt16 BIFF_EXTNAME_AUTOMATIC     = 0x0002;
const sal_uInt16 BIFF_EXTNAME_PREFERPIC     = 0x0004;
const sal_uInt16 BIFF_EXTNAME_STDDOCNAME    = 0x0008;
const sal_uInt16 BIFF_EXTNAME_OLEOBJECT     = 0x0010;
const sal_uInt16 BIFF_EXTNAME_ICONIFIED     = 0x8000;

} // namespace

// ============================================================================

OoxExternalNameData::OoxExternalNameData() :
    mbBuiltIn( false ),
    mbNotify( false ),
    mbPreferPic( false ),
    mbStdDocName( false ),
    mbOleObj( false ),
    mbIconified( false )
{
}

// ============================================================================

ExternalName::ExternalName( const ExternalLink& rParentLink, sal_Int32 nLocalSheet ) :
    DefinedNameBase( rParentLink, nLocalSheet ),
    mrParentLink( rParentLink ),
    mnStorageId( 0 ),
    mbDdeLinkCreated( false )
{
}

void ExternalName::importDefinedName( const AttributeList& rAttribs )
{
    maOoxData.maName = rAttribs.getString( XML_name );
    OSL_ENSURE( maOoxData.maName.getLength() > 0, "ExternalName::importDefinedName - empty name" );
    // zero-based index into sheet list of externalBook
    maOoxData.mnSheet = mrParentLink.getSheetIndex( rAttribs.getInteger( XML_sheetId, -1 ) );
}

void ExternalName::importDdeItem( const AttributeList& rAttribs )
{
    maOoxData.maName = rAttribs.getString( XML_name );
    OSL_ENSURE( maOoxData.maName.getLength() > 0, "ExternalName::importDdeItem - empty name" );
    maOoxExtNameData.mbOleObj     = false;
    maOoxExtNameData.mbStdDocName = rAttribs.getBool( XML_ole, false );
    maOoxExtNameData.mbNotify     = rAttribs.getBool( XML_advise, false );
    maOoxExtNameData.mbPreferPic  = rAttribs.getBool( XML_preferPic, false );
}

void ExternalName::importValues( const AttributeList& rAttribs )
{
    setResultSize( rAttribs.getInteger( XML_cols, 1 ), rAttribs.getInteger( XML_rows, 1 ) );
}

void ExternalName::importOleItem( const AttributeList& rAttribs )
{
    maOoxData.maName = rAttribs.getString( XML_name );
    OSL_ENSURE( maOoxData.maName.getLength() > 0, "ExternalName::importOleItem - empty name" );
    maOoxExtNameData.mbOleObj    = true;
    maOoxExtNameData.mbNotify    = rAttribs.getBool( XML_advise, false );
    maOoxExtNameData.mbPreferPic = rAttribs.getBool( XML_preferPic, false );
    maOoxExtNameData.mbIconified = rAttribs.getBool( XML_icon, false );
}

void ExternalName::importExternalName( RecordInputStream& rStrm )
{
    rStrm >> maOoxData.maName;
    OSL_ENSURE( maOoxData.maName.getLength() > 0, "ExternalName::importExternalName - empty name" );
}

void ExternalName::importExternalNameFlags( RecordInputStream& rStrm )
{
    sal_uInt16 nFlags;
    sal_Int32 nSheetId;
    rStrm >> nFlags >> nSheetId;
    // one-based index into sheet list of EXTSHEETNAMES
    maOoxData.mnSheet = mrParentLink.getSheetIndex( nSheetId - 1 );
    // no flag for built-in names, as in OOX...
    maOoxExtNameData.mbNotify     = getFlag( nFlags, OOBIN_EXTNAME_AUTOMATIC );
    maOoxExtNameData.mbPreferPic  = getFlag( nFlags, OOBIN_EXTNAME_PREFERPIC );
    maOoxExtNameData.mbStdDocName = getFlag( nFlags, OOBIN_EXTNAME_STDDOCNAME );
    maOoxExtNameData.mbOleObj     = getFlag( nFlags, OOBIN_EXTNAME_OLEOBJECT );
    maOoxExtNameData.mbIconified  = getFlag( nFlags, OOBIN_EXTNAME_ICONIFIED );
    OSL_ENSURE( (mrParentLink.getLinkType() == LINKTYPE_OLE) == maOoxExtNameData.mbOleObj,
        "ExternalName::importExternalNameFlags - wrong flags in external name" );
}

void ExternalName::importDdeItemValues( RecordInputStream& rStrm )
{
    sal_Int32 nRows, nCols;
    rStrm >> nRows >> nCols;
    setResultSize( nCols, nRows );
}

void ExternalName::importDdeItemBool( RecordInputStream& rStrm )
{
    appendResultValue< double >( (rStrm.readuInt8() == 0) ? 0.0 : 1.0 );
}

void ExternalName::importDdeItemDouble( RecordInputStream& rStrm )
{
    appendResultValue( rStrm.readDouble() );
}

void ExternalName::importDdeItemError( RecordInputStream& rStrm )
{
    appendResultValue( BiffHelper::calcDoubleFromError( rStrm.readuInt8() ) );
}

void ExternalName::importDdeItemString( RecordInputStream& rStrm )
{
    appendResultValue( rStrm.readString() );
}

void ExternalName::importExternalName( BiffInputStream& rStrm )
{
    sal_uInt16 nFlags = 0;
    if( getBiff() >= BIFF3 )
    {
        rStrm >> nFlags;
        maOoxExtNameData.mbBuiltIn    = getFlag( nFlags, BIFF_EXTNAME_BUILTIN );
        maOoxExtNameData.mbNotify     = getFlag( nFlags, BIFF_EXTNAME_AUTOMATIC );
        maOoxExtNameData.mbPreferPic  = getFlag( nFlags, BIFF_EXTNAME_PREFERPIC );

        // BIFF5-BIFF8: sheet index for sheet-local names, OLE settings
        if( getBiff() >= BIFF5 )
        {
            maOoxExtNameData.mbStdDocName = getFlag( nFlags, BIFF_EXTNAME_STDDOCNAME );
            maOoxExtNameData.mbOleObj     = getFlag( nFlags, BIFF_EXTNAME_OLEOBJECT );
            maOoxExtNameData.mbIconified  = getFlag( nFlags, BIFF_EXTNAME_ICONIFIED );

            if( maOoxExtNameData.mbOleObj )
            {
                rStrm >> mnStorageId;
            }
            else
            {
                // get sheet index for sheet-local names
                sal_Int16 nRefId = rStrm.skip( 2 ).readuInt16();
                switch( getBiff() )
                {
                    case BIFF2:
                    case BIFF3:
                    case BIFF4:
                    break;
                    case BIFF5:
                        // resolve nRefId to sheet index, zero is global name
                        if( nRefId > 0 )
                            if( const ExternalLink* pExtLink = getExternalLinks().getExternalLink( nRefId ).get() )
                                if( pExtLink->getLinkType() == LINKTYPE_EXTERNAL )
                                    maOoxData.mnSheet = pExtLink->getSheetIndex();
                    break;
                    case BIFF8:
                        // one-based index into sheet list of EXTERNALBOOK record, zero is global name
                        if( nRefId > 0 )
                            maOoxData.mnSheet = mrParentLink.getSheetIndex( nRefId - 1 );
                    break;
                    case BIFF_UNKNOWN:
                    break;
                }
            }
        }
    }

    maOoxData.maName = (getBiff() == BIFF8) ?
        rStrm.readUniString( rStrm.readuInt8() ) :
        rStrm.readByteString( false, getTextEncoding() );
    OSL_ENSURE( maOoxData.maName.getLength() > 0, "ExternalName::importExternalName - empty name" );

    switch( mrParentLink.getLinkType() )
    {
        case LINKTYPE_INTERNAL:
        case LINKTYPE_EXTERNAL:
            // cell references that are stored in hidden external names (seen in BIFF3-BIFF4)
            if( (getBiff() <= BIFF4) && (maOoxData.maName.getLength() > 0) && (maOoxData.maName[ 0 ] == '\x01') && (rStrm.getRecLeft() > 2) )
            {
                TokensFormulaContext aContext( true, true );
                importBiffFormula( aContext, rStrm );
                setReference( aContext.getTokens() );
            }
        break;

        case LINKTYPE_DDE:
        case LINKTYPE_OLE:
        case LINKTYPE_MAYBE_DDE_OLE:
            // DDE/OLE link results
            if( rStrm.getRecLeft() > 3 )
            {
                bool bBiff8 = getBiff() == BIFF8;
                sal_Int32 nCols = rStrm.readuInt8();
                sal_Int32 nRows = rStrm.readuInt16();
                if( bBiff8 ) { ++nCols; ++nRows; } else if( nCols == 0 ) nCols = 256;
                setResultSize( nCols, nRows );

                bool bLoop = true;
                while( bLoop && rStrm.isValid() && (maCurrIt != maResults.end()) )
                {
                    switch( rStrm.readuInt8() )
                    {
                        case BIFF_DATATYPE_EMPTY:
                            appendResultValue( OUString() );
                            rStrm.skip( 8 );
                        break;
                        case BIFF_DATATYPE_DOUBLE:
                            appendResultValue( rStrm.readDouble() );
                        break;
                        case BIFF_DATATYPE_STRING:
                            appendResultValue( bBiff8 ? rStrm.readUniString() : rStrm.readByteString( false, getTextEncoding() ) );
                        break;
                        case BIFF_DATATYPE_BOOL:
                            appendResultValue< double >( (rStrm.readuInt8() == 0) ? 0.0 : 1.0 );
                            rStrm.skip( 7 );
                        break;
                        case BIFF_DATATYPE_ERROR:
                            appendResultValue( BiffHelper::calcDoubleFromError( rStrm.readuInt8() ) );
                            rStrm.skip( 7 );
                        break;
                        default:
                            bLoop = false;
                    }
                }
                OSL_ENSURE( bLoop && rStrm.isValid() && (maCurrIt == maResults.end()),
                    "ExternalName::importExternalName - stream error in result set" );
            }
        break;

        default:;
    }
}

bool ExternalName::getDdeLinkData( OUString& orDdeServer, OUString& orDdeTopic, OUString& orDdeItem )
{
    if( (mrParentLink.getLinkType() == LINKTYPE_DDE) && (maOoxData.maName.getLength() > 0) )
    {
        // try to create a DDE link and to set the imported link results
        if( !mbDdeLinkCreated ) try
        {
            Reference< XDDELinks > xDdeLinks( getDdeLinks(), UNO_QUERY_THROW );
            mxDdeLink = xDdeLinks->addDDELink( mrParentLink.getClassName(), mrParentLink.getTargetUrl(), maOoxData.maName, ::com::sun::star::sheet::DDELinkMode_DEFAULT );
            if( !maResults.empty() )
            {
                Reference< XDDELinkResults > xResults( mxDdeLink, UNO_QUERY_THROW );
                xResults->setResults( ContainerHelper::matrixToSequenceSequence( maResults ) );
            }
            mbDdeLinkCreated = true;
        }
        catch( Exception& )
        {
            OSL_ENSURE( false, "ExternalName::getDdeLinkData - cannot create DDE link" );
        }
        // get link data from created DDE link
        if( mxDdeLink.is() )
        {
            orDdeServer = mxDdeLink->getApplication();
            orDdeTopic = mxDdeLink->getTopic();
            orDdeItem = mxDdeLink->getItem();
            return true;
        }
    }
    return false;
}

void ExternalName::setResultSize( sal_Int32 nColumns, sal_Int32 nRows )
{
    OSL_ENSURE( (mrParentLink.getLinkType() == LINKTYPE_DDE) || (mrParentLink.getLinkType() == LINKTYPE_OLE) ||
        (mrParentLink.getLinkType() == LINKTYPE_MAYBE_DDE_OLE), "ExternalName::setResultSize - wrong link type" );
    OSL_ENSURE( (nRows > 0) && (nColumns > 0), "ExternalName::setResultSize - invalid matrix size" );
    const CellAddress& rMaxPos = getAddressConverter().getMaxApiAddress();
    if( (0 < nRows) && (nRows <= rMaxPos.Row + 1) && (0 < nColumns) && (nColumns <= rMaxPos.Column + 1) )
        maResults.resize( static_cast< size_t >( nColumns ), static_cast< size_t >( nRows ), Any( BiffHelper::calcDoubleFromError( BIFF_ERR_NA ) ) );
    else
        maResults.clear();
    maCurrIt = maResults.begin();
}

// ============================================================================

ExternalLink::ExternalLink( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    meLinkType( LINKTYPE_UNKNOWN )
{
}

void ExternalLink::importExternalReference( const AttributeList& rAttribs )
{
    maRelId = rAttribs.getString( R_TOKEN( id ) );
}

void ExternalLink::importExternalBook( const Relations& rRelations, const AttributeList& rAttribs )
{
    OUString aTargetUrl = rRelations.getTargetFromRelId( rAttribs.getString( R_TOKEN( id ) ) );
    setExternalTargetUrl( aTargetUrl );
}

void ExternalLink::importSheetName( const AttributeList& rAttribs )
{
    OUString aSheetName = rAttribs.getString( XML_val );
    OSL_ENSURE( aSheetName.getLength() > 0, "ExternalLink::importSheetName - empty sheet name" );
    if( meLinkType == LINKTYPE_EXTERNAL )
        maSheetIndexes.push_back( getWorksheets().insertExternalSheet( maTargetUrl, aSheetName ) );
}

void ExternalLink::importDefinedName( const AttributeList& rAttribs )
{
    createExternalName()->importDefinedName( rAttribs );
}

void ExternalLink::importDdeLink( const AttributeList& rAttribs )
{
    OUString aDdeService = rAttribs.getString( XML_ddeService );
    OUString aDdeTopic = rAttribs.getString( XML_ddeTopic );
    setDdeOleTargetUrl( aDdeService, aDdeTopic, LINKTYPE_DDE );
}

ExternalNameRef ExternalLink::importDdeItem( const AttributeList& rAttribs )
{
    ExternalNameRef xExtName = createExternalName();
    xExtName->importDdeItem( rAttribs );
    return xExtName;
}

void ExternalLink::importOleLink( const Relations& rRelations, const AttributeList& rAttribs )
{
    OUString aProgId = rAttribs.getString( XML_progId );
    OUString aTargetUrl = rRelations.getTargetFromRelId( rAttribs.getString( R_TOKEN( id ) ) );
    setDdeOleTargetUrl( aProgId, aTargetUrl, LINKTYPE_OLE );
}

ExternalNameRef ExternalLink::importOleItem( const AttributeList& rAttribs )
{
    ExternalNameRef xExtName = createExternalName();
    xExtName->importOleItem( rAttribs );
    return xExtName;
}

void ExternalLink::importExternalRef( RecordInputStream& rStrm )
{
    rStrm >> maRelId;
}

void ExternalLink::importExternalSelf( RecordInputStream& )
{
    meLinkType = LINKTYPE_SELF;
}

void ExternalLink::importExternalSame( RecordInputStream& )
{
    meLinkType = LINKTYPE_SAME;
}

void ExternalLink::importExternalAddin( RecordInputStream& )
{
    meLinkType = LINKTYPE_UNKNOWN;
}

void ExternalLink::importExternalBook( const Relations& rRelations, RecordInputStream& rStrm )
{
    switch( rStrm.readuInt16() )
    {
        case OOBIN_EXTERNALBOOK_BOOK:
        {
            OUString aTargetUrl = rRelations.getTargetFromRelId( rStrm.readString() );
            setExternalTargetUrl( aTargetUrl );
        }
        break;
        case OOBIN_EXTERNALBOOK_DDE:
        {
            OUString aDdeService, aDdeTopic;
            rStrm >> aDdeService >> aDdeTopic;
            setDdeOleTargetUrl( aDdeService, aDdeTopic, LINKTYPE_DDE );
        }
        break;
        case OOBIN_EXTERNALBOOK_OLE:
        {
            OUString aTargetUrl = rRelations.getTargetFromRelId( rStrm.readString() );
            OUString aProgId = rStrm.readString();
            setDdeOleTargetUrl( aProgId, aTargetUrl, LINKTYPE_OLE );
        }
        break;
        default:
            OSL_ENSURE( false, "ExternalLink::importExternalBook - unknown link type" );
    }
}

void ExternalLink::importExtSheetNames( RecordInputStream& rStrm )
{
    // load external sheet names and create the linked sheets in the Calc document
    OSL_ENSURE( meLinkType == LINKTYPE_EXTERNAL, "ExternalLink::importExtSheetNames - invalid link type" );
    if( meLinkType == LINKTYPE_EXTERNAL )
    {
        WorksheetBuffer& rWorksheets = getWorksheets();
        for( sal_Int32 nSheet = 0, nCount = rStrm.readInt32(); rStrm.isValid() && (nSheet < nCount); ++nSheet )
        {
            OUString aSheetName = rStrm.readString();
            OSL_ENSURE( aSheetName.getLength() > 0, "ExternalLink::importExtSheetNames - empty sheet name" );
            maSheetIndexes.push_back( rWorksheets.insertExternalSheet( maTargetUrl, aSheetName ) );
        }
    }
}

ExternalNameRef ExternalLink::importExternalName( RecordInputStream& rStrm )
{
    ExternalNameRef xExtName = createExternalName();
    xExtName->importExternalName( rStrm );
    return xExtName;
}

void ExternalLink::importExternSheet( BiffInputStream& rStrm )
{
    rStrm.enableNulChars( true );
    OStringBuffer aTargetBuffer( rStrm.readByteString( false ) );
    rStrm.enableNulChars( false );
    // references to own sheets have wrong string length field (off by 1)
    if( (aTargetBuffer.getLength() > 0) && (aTargetBuffer[ 0 ] == 3) )
        aTargetBuffer.append( static_cast< sal_Char >( rStrm.readuInt8() ) );
    // parse the encoded URL
    OUString aBiffTarget = OStringToOUString( aTargetBuffer.makeStringAndClear(), getTextEncoding() );
    OUString aSheetName = parseBiffTargetUrl( aBiffTarget );
    switch( meLinkType )
    {
        case LINKTYPE_INTERNAL:
            maSheetIndexes.push_back( getWorksheets().getFinalSheetIndex( aSheetName ) );
        break;
        case LINKTYPE_EXTERNAL:
            maSheetIndexes.push_back( getWorksheets().insertExternalSheet( maTargetUrl, aSheetName ) );
        break;
        default:;
    }
}

void ExternalLink::importExternalBook( BiffInputStream& rStrm )
{
    OUString aTarget;
    sal_uInt16 nSheetCount;
    rStrm >> nSheetCount;
    if( rStrm.getRecLeft() == 2 )
    {
        if( rStrm.readuInt8() == 1 )
        {
            sal_Char cChar = static_cast< sal_Char >( rStrm.readuInt8() );
            if( cChar != 0 )
                aTarget = OStringToOUString( OString( cChar ), getTextEncoding() );
        }
    }
    else if( rStrm.getRecLeft() >= 3 )
    {
        // NUL characters may occur
        rStrm.enableNulChars( true );
        aTarget = rStrm.readUniString();
        rStrm.enableNulChars( false );
    }

    // parse the encoded URL
    OUString aDummySheetName = parseBiffTargetUrl( aTarget );
    OSL_ENSURE( aDummySheetName.getLength() == 0, "ExternalLink::importExternalBook - sheet name in encoded URL" );
    (void)aDummySheetName;  // prevent compiler warning

    // load external sheet names and create the linked sheets in the Calc document
    if( meLinkType == LINKTYPE_EXTERNAL )
    {
        WorksheetBuffer& rWorksheets = getWorksheets();
        for( sal_uInt16 nSheet = 0; rStrm.isValid() && (nSheet < nSheetCount); ++nSheet )
        {
            OUString aSheetName = rStrm.readUniString();
            OSL_ENSURE( aSheetName.getLength() > 0, "ExternalLink::importExternalBook - empty sheet name" );
            maSheetIndexes.push_back( rWorksheets.insertExternalSheet( maTargetUrl, aSheetName ) );
        }
    }
}

void ExternalLink::importExternalName( BiffInputStream& rStrm )
{
    ExternalNameRef xExtName = createExternalName();
    xExtName->importExternalName( rStrm );
    switch( meLinkType )
    {
        case LINKTYPE_DDE:
            OSL_ENSURE( !xExtName->isOleObject(), "ExternalLink::importExternalName - OLE object in DDE link" );
        break;
        case LINKTYPE_OLE:
            OSL_ENSURE( xExtName->isOleObject(), "ExternalLink::importExternalName - anything but OLE object in OLE link" );
        break;
        case LINKTYPE_MAYBE_DDE_OLE:
            meLinkType = xExtName->isOleObject() ? LINKTYPE_OLE : LINKTYPE_DDE;
        break;
        default:
            OSL_ENSURE( !xExtName->isOleObject(), "ExternalLink::importExternalName - OLE object in external name" );
    }
}

sal_Int32 ExternalLink::getSheetIndex( sal_Int32 nTabId ) const
{
    OSL_ENSURE( (nTabId == 0) || (getFilterType() == FILTER_OOX) || (getBiff() == BIFF8),
        "ExternalLink::getSheetIndex - invalid sheet index" );
    return ((0 <= nTabId) && (static_cast< size_t >( nTabId ) < maSheetIndexes.size())) ?
        maSheetIndexes[ static_cast< size_t >( nTabId ) ] : -1;
}

void ExternalLink::getSheetRange( LinkSheetRange& orSheetRange, sal_Int32 nTabId1, sal_Int32 nTabId2 ) const
{
    switch( meLinkType )
    {
        case LINKTYPE_SAME:
            orSheetRange.setRelative();
        break;

        case LINKTYPE_SELF:
        case LINKTYPE_INTERNAL:
            orSheetRange.setRange( nTabId1, nTabId2 );
        break;

        case LINKTYPE_EXTERNAL: switch( getFilterType() )
        {
            case FILTER_OOX:
                // OOBIN: passed indexes point into sheet list of EXTSHEETLIST
                orSheetRange.setRange( getSheetIndex( nTabId1 ), getSheetIndex( nTabId2 ) );
            break;
            case FILTER_BIFF:
                switch( getBiff() )
                {
                    case BIFF2:
                    case BIFF3:
                    case BIFF4:
                        orSheetRange.setRange( getSheetIndex( nTabId1 ), getSheetIndex( nTabId2 ) );
                    break;
                    case BIFF5:
                        // BIFF5: first sheet from this external link, last sheet is passed in nTabId2
                        if( const ExternalLink* pExtLink2 = getExternalLinks().getExternalLink( nTabId2 ).get() )
                            if( (pExtLink2->getLinkType() == LINKTYPE_EXTERNAL) && (maTargetUrl == pExtLink2->getTargetUrl()) )
                                orSheetRange.setRange( getSheetIndex(), pExtLink2->getSheetIndex() );
                    break;
                    case BIFF8:
                        // BIFF8: passed indexes point into sheet list of EXTERNALBOOK
                        orSheetRange.setRange( getSheetIndex( nTabId1 ), getSheetIndex( nTabId2 ) );
                    break;
                    case BIFF_UNKNOWN: break;
                }
            break;
            case FILTER_UNKNOWN: break;
        }
        break;

        default:
            // unsupported/unexpected link type: #REF! error
            orSheetRange.setDeleted();
    }
}

ExternalNameRef ExternalLink::getNameByIndex( sal_Int32 nIndex ) const
{
    return maExtNames.get( nIndex );
}

// private --------------------------------------------------------------------

void ExternalLink::setExternalTargetUrl( const OUString& rTargetUrl )
{
    maTargetUrl = getBaseFilter().getAbsoluteUrl( rTargetUrl );
    meLinkType = (maTargetUrl.getLength() > 0) ? LINKTYPE_EXTERNAL : LINKTYPE_UNKNOWN;
    OSL_ENSURE( meLinkType == LINKTYPE_EXTERNAL, "ExternalLink::setExternalTargetUrl - empty target URL" );
}

void ExternalLink::setDdeOleTargetUrl( const OUString& rClassName, const OUString& rTargetUrl, ExternalLinkType eLinkType )
{
    maClassName = rClassName;
    maTargetUrl = rTargetUrl;
    meLinkType = ((maClassName.getLength() > 0) && (maTargetUrl.getLength() > 0)) ? eLinkType : LINKTYPE_UNKNOWN;
    OSL_ENSURE( meLinkType == eLinkType, "ExternalLink::setDdeOleTargetUrl - missing classname or target" );
}

OUString ExternalLink::parseBiffTargetUrl( const OUString& rBiffTargetUrl )
{
    OUString aClassName, aTargetUrl, aSheetName;
    bool bSameSheet = false;
    meLinkType = LINKTYPE_UNKNOWN;
    if( getAddressConverter().parseBiffTargetUrl( aClassName, aTargetUrl, aSheetName, bSameSheet, rBiffTargetUrl ) )
    {
        if( aClassName.getLength() > 0 )
        {
            setDdeOleTargetUrl( aClassName, aTargetUrl, LINKTYPE_MAYBE_DDE_OLE );
        }
        else if( aTargetUrl.getLength() == 0 )
        {
            meLinkType = (aSheetName.getLength() > 0) ? LINKTYPE_INTERNAL : (bSameSheet ? LINKTYPE_SAME : LINKTYPE_SELF);
        }
        else if( (aTargetUrl.getLength() == 1) && (aTargetUrl[ 0 ] == ':') )
        {
            if( getBiff() >= BIFF4 )
                meLinkType = LINKTYPE_ANALYSIS;
        }
        else if( (aTargetUrl.getLength() == 1) && (aTargetUrl[ 0 ] == ' ') )
        {
            meLinkType = LINKTYPE_UNKNOWN;
        }
        else
        {
            setExternalTargetUrl( aTargetUrl );
        }
    }
    return aSheetName;
}

ExternalNameRef ExternalLink::createExternalName()
{
    ExternalNameRef xExtName( new ExternalName( *this, getSheetIndex() ) );
    maExtNames.push_back( xExtName );
    return xExtName;
}

// ============================================================================

OoxRefSheets::OoxRefSheets() :
    mnExtRefId( -1 ),
    mnTabId1( -1 ),
    mnTabId2( -1 )
{
}

void OoxRefSheets::readOobData( RecordInputStream& rStrm )
{
    rStrm >> mnExtRefId >> mnTabId1 >> mnTabId2;
}

void OoxRefSheets::readBiff8Data( BiffInputStream& rStrm )
{
    mnExtRefId = rStrm.readuInt16();
    mnTabId1 = rStrm.readInt16();
    mnTabId2 = rStrm.readInt16();
}

// ----------------------------------------------------------------------------

ExternalLinkBuffer::ExternalLinkBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mbUseRefSheets( false )
{
}

ExternalLinkRef ExternalLinkBuffer::importExternalReference( const AttributeList& rAttribs )
{
    ExternalLinkRef xExtLink = createExternalLink();
    xExtLink->importExternalReference( rAttribs );
    return xExtLink;
}

ExternalLinkRef ExternalLinkBuffer::importExternalRef( RecordInputStream& rStrm )
{
    mbUseRefSheets = true;
    ExternalLinkRef xExtLink = createExternalLink();
    xExtLink->importExternalRef( rStrm );
    return xExtLink;
}

void ExternalLinkBuffer::importExternalSelf( RecordInputStream& rStrm )
{
    mbUseRefSheets = true;
    createExternalLink()->importExternalSelf( rStrm );
}

void ExternalLinkBuffer::importExternalSame( RecordInputStream& rStrm )
{
    mbUseRefSheets = true;
    createExternalLink()->importExternalSame( rStrm );
}

void ExternalLinkBuffer::importExternalAddin( RecordInputStream& rStrm )
{
    mbUseRefSheets = true;
    createExternalLink()->importExternalAddin( rStrm );
}

void ExternalLinkBuffer::importExternalSheets( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbUseRefSheets, "ExternalLinkBuffer::importExternalSheets - missing EXTERNALREFS records" );
    mbUseRefSheets = true;
    OSL_ENSURE( maRefSheets.empty(), "ExternalLinkBuffer::importExternalSheets - multiple EXTERNALSHEETS records" );
    maRefSheets.clear();
    sal_Int32 nRefCount;
    rStrm >> nRefCount;
    size_t nMaxCount = getLimitedValue< size_t, sal_Int32 >( nRefCount, 0, rStrm.getRecLeft() / 12 );
    maRefSheets.reserve( nMaxCount );
    for( size_t nRefId = 0; rStrm.isValid() && (nRefId < nMaxCount); ++nRefId )
    {
        OoxRefSheets aRefSheets;
        aRefSheets.readOobData( rStrm );
        maRefSheets.push_back( aRefSheets );
    }
}

ExternalLinkRef ExternalLinkBuffer::importExternSheet( BiffInputStream& rStrm )
{
    OSL_ENSURE( getBiff() <= BIFF5, "ExternalLinkBuffer::importExternSheet - wrong BIFF version" );
    ExternalLinkRef xExtLink = createExternalLink();
    xExtLink->importExternSheet( rStrm );
    return xExtLink;
}

ExternalLinkRef ExternalLinkBuffer::importExternalBook( BiffInputStream& rStrm )
{
    ExternalLinkRef xExtLink = createExternalLink();
    xExtLink->importExternalBook( rStrm );
    return xExtLink;
}

void ExternalLinkBuffer::importExternalName( BiffInputStream& rStrm )
{
    if( !maExtLinks.empty() )
        maExtLinks.back()->importExternalName( rStrm );
}

void ExternalLinkBuffer::importExternSheet8( BiffInputStream& rStrm )
{
    OSL_ENSURE( getBiff() == BIFF8, "ExternalLinkBuffer::importExternSheet - wrong BIFF version" );
    OSL_ENSURE( maRefSheets.empty(), "ExternalLinkBuffer::importExternSheet - multiple EXTERNSHEET records" );
    maRefSheets.clear();
    sal_uInt16 nRefCount;
    rStrm >> nRefCount;
    maRefSheets.reserve( nRefCount );
    for( sal_uInt16 nRefId = 0; rStrm.isValid() && (nRefId < nRefCount); ++nRefId )
    {
        OoxRefSheets aRefSheets;
        aRefSheets.readBiff8Data( rStrm );
        maRefSheets.push_back( aRefSheets );
    }
}

ExternalLinkRef ExternalLinkBuffer::getExternalLink( sal_Int32 nRefId ) const
{
    ExternalLinkRef xExtLink;
    switch( getFilterType() )
    {
        case FILTER_OOX:
            // OOXML: one-based index
            if( !mbUseRefSheets )
                xExtLink = maExtLinks.get( nRefId - 1 );
            // OOBIN: zero-based index into ref-sheets list
            else if( const OoxRefSheets* pRefSheets = getRefSheets( nRefId ) )
                xExtLink = maExtLinks.get( pRefSheets->mnExtRefId );
        break;
        case FILTER_BIFF:
            switch( getBiff() )
            {
                case BIFF2:
                case BIFF3:
                case BIFF4:
                    // one-based index to EXTERNSHEET records
                    xExtLink = maExtLinks.get( nRefId - 1 );
                break;
                case BIFF5:
                    if( nRefId < 0 )
                    {
                        // internal links in formula tokens have negative index
                        xExtLink = maExtLinks.get( -nRefId - 1 );
                        if( xExtLink.get() && !xExtLink->isInternalLink() )
                            xExtLink.reset();
                    }
                    else
                    {
                        // one-based index to EXTERNSHEET records
                        xExtLink = maExtLinks.get( nRefId - 1 );
                    }
                break;
                case BIFF8:
                    // zero-based index into REF list in EXTERNSHEET record
                    if( const OoxRefSheets* pRefSheets = getRefSheets( nRefId ) )
                        xExtLink = maExtLinks.get( pRefSheets->mnExtRefId );
                break;
                case BIFF_UNKNOWN: break;
            }
        break;
        case FILTER_UNKNOWN: break;
    }
    return xExtLink;
}

LinkSheetRange ExternalLinkBuffer::getSheetRange( sal_Int32 nRefId, sal_Int16 nTabId1, sal_Int16 nTabId2 ) const
{
    OSL_ENSURE( getBiff() <= BIFF5, "ExternalLinkBuffer::getSheetRange - wrong BIFF version" );
    LinkSheetRange aSheetRange;
    if( const ExternalLink* pExtLink = getExternalLink( nRefId ).get() )
        pExtLink->getSheetRange( aSheetRange, nTabId1, nTabId2 );
    return aSheetRange;
}

LinkSheetRange ExternalLinkBuffer::getSheetRange( sal_Int32 nRefId ) const
{
    OSL_ENSURE( ((getFilterType() == FILTER_OOX) && mbUseRefSheets) || (getBiff() == BIFF8), "ExternalLinkBuffer::getSheetRange - wrong BIFF version" );
    LinkSheetRange aSheetRange;
    if( const ExternalLink* pExtLink = getExternalLink( nRefId ).get() )
        if( const OoxRefSheets* pRefSheets = getRefSheets( nRefId ) )
            pExtLink->getSheetRange( aSheetRange, pRefSheets->mnTabId1, pRefSheets->mnTabId2 );
    return aSheetRange;
}

// private --------------------------------------------------------------------

ExternalLinkRef ExternalLinkBuffer::createExternalLink()
{
    ExternalLinkRef xExtLink( new ExternalLink( *this ) );
    maExtLinks.push_back( xExtLink );
    return xExtLink;
}

const OoxRefSheets* ExternalLinkBuffer::getRefSheets( sal_Int32 nRefId ) const
{
    return ((0 <= nRefId) && (static_cast< size_t >( nRefId ) < maRefSheets.size())) ?
        &maRefSheets[ static_cast< size_t >( nRefId ) ] : 0;
}

// ============================================================================

} // namespace xls
} // namespace oox

