/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: workbookfragment.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 19:08:27 $
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

#include "oox/xls/workbookfragment.hxx"
#include <com/sun/star/table/CellAddress.hpp>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/progressbar.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/drawingml/themefragmenthandler.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/chartsheetfragment.hxx"
#include "oox/xls/connectionsfragment.hxx"
#include "oox/xls/externallinkbuffer.hxx"
#include "oox/xls/externallinkfragment.hxx"
#include "oox/xls/pivotcachefragment.hxx"
#include "oox/xls/sharedstringsbuffer.hxx"
#include "oox/xls/sharedstringsfragment.hxx"
#include "oox/xls/stylesfragment.hxx"
#include "oox/xls/tablebuffer.hxx"
#include "oox/xls/themebuffer.hxx"
#include "oox/xls/viewsettings.hxx"
#include "oox/xls/workbooksettings.hxx"
#include "oox/xls/worksheetbuffer.hxx"
#include "oox/xls/worksheetfragment.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::table::CellAddress;
using ::oox::core::FragmentHandlerRef;
using ::oox::core::RecordInfo;
using ::oox::core::Relation;
using ::oox::drawingml::ThemeFragmentHandler;

namespace oox {
namespace xls {

// ============================================================================

namespace {

const double PROGRESS_LENGTH_GLOBALS        = 0.1;      /// 10% of progress bar for globals import.

const sal_uInt16 BIFF_FILEPASS_BIFF2        = 0x0000;
const sal_uInt16 BIFF_FILEPASS_BIFF8        = 0x0001;
const sal_uInt16 BIFF_FILEPASS_BIFF8_RCF    = 0x0001;
const sal_uInt16 BIFF_FILEPASS_BIFF8_STRONG = 0x0002;

} // namespace

// ============================================================================

OoxWorkbookFragment::OoxWorkbookFragment(
        const WorkbookHelper& rHelper, const OUString& rFragmentPath ) :
    OoxWorkbookFragmentBase( rHelper, rFragmentPath )
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper OoxWorkbookFragment::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return  (nElement == XLS_TOKEN( workbook ));
        case XLS_TOKEN( workbook ):
            return  (nElement == XLS_TOKEN( workbookPr )) ||
                    (nElement == XLS_TOKEN( calcPr )) ||
                    (nElement == XLS_TOKEN( sheets )) ||
                    (nElement == XLS_TOKEN( bookViews )) ||
                    (nElement == XLS_TOKEN( externalReferences )) ||
                    (nElement == XLS_TOKEN( definedNames )) ||
                    (nElement == XLS_TOKEN( pivotCaches ));
        case XLS_TOKEN( sheets ):
            return  (nElement == XLS_TOKEN( sheet ));
        case XLS_TOKEN( bookViews ):
            return  (nElement == XLS_TOKEN( workbookView ));
        case XLS_TOKEN( externalReferences ):
            return  (nElement == XLS_TOKEN( externalReference ));
        case XLS_TOKEN( definedNames ):
            return  (nElement == XLS_TOKEN( definedName ));
        case XLS_TOKEN( pivotCaches ):
            return  (nElement == XLS_TOKEN( pivotCache ));
    }
    return false;
}

void OoxWorkbookFragment::onStartElement( const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( workbookPr ):           getWorkbookSettings().importWorkbookPr( rAttribs ); break;
        case XLS_TOKEN( calcPr ):               getWorkbookSettings().importCalcPr( rAttribs );     break;
        case XLS_TOKEN( sheet ):                getWorksheets().importSheet( rAttribs );            break;
        case XLS_TOKEN( workbookView ):         getViewSettings().importWorkbookView( rAttribs );   break;
        case XLS_TOKEN( externalReference ):    importExternalReference( rAttribs );                break;
        case XLS_TOKEN( definedName ):          importDefinedName( rAttribs );                      break;
        case XLS_TOKEN( pivotCache ):           importPivotCache( rAttribs );                       break;
    }
}

void OoxWorkbookFragment::onEndElement( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( definedName ):
            if( mxCurrName.get() ) mxCurrName->setFormula( rChars );
        break;
    }
}

ContextWrapper OoxWorkbookFragment::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return  (nRecId == OOBIN_ID_WORKBOOK);
        case OOBIN_ID_WORKBOOK:
            return  (nRecId == OOBIN_ID_WORKBOOKPR) ||
                    (nRecId == OOBIN_ID_CALCPR) ||
                    (nRecId == OOBIN_ID_SHEETS) ||
                    (nRecId == OOBIN_ID_BOOKVIEWS) ||
                    (nRecId == OOBIN_ID_EXTERNALREFS) ||
                    (nRecId == OOBIN_ID_DEFINEDNAME);
        case OOBIN_ID_SHEETS:
            return  (nRecId == OOBIN_ID_SHEET);
        case OOBIN_ID_BOOKVIEWS:
            return  (nRecId == OOBIN_ID_WORKBOOKVIEW);
        case OOBIN_ID_EXTERNALREFS:
            return  (nRecId == OOBIN_ID_EXTERNALREF) ||
                    (nRecId == OOBIN_ID_EXTERNALSELF) ||
                    (nRecId == OOBIN_ID_EXTERNALSAME) ||
                    (nRecId == OOBIN_ID_EXTERNALADDIN) ||
                    (nRecId == OOBIN_ID_EXTERNALSHEETS);
    }
    return false;
}

void OoxWorkbookFragment::onStartRecord( RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case OOBIN_ID_WORKBOOKPR:       getWorkbookSettings().importWorkbookPr( rStrm );    break;
        case OOBIN_ID_CALCPR:           getWorkbookSettings().importCalcPr( rStrm );        break;
        case OOBIN_ID_SHEET:            getWorksheets().importSheet( rStrm );               break;
        case OOBIN_ID_WORKBOOKVIEW:     getViewSettings().importWorkbookView( rStrm );      break;
        case OOBIN_ID_EXTERNALREF:      importExternalRef( rStrm );                         break;
        case OOBIN_ID_EXTERNALSELF:     getExternalLinks().importExternalSelf( rStrm );     break;
        case OOBIN_ID_EXTERNALSAME:     getExternalLinks().importExternalSame( rStrm );     break;
        case OOBIN_ID_EXTERNALADDIN:    getExternalLinks().importExternalAddin( rStrm );    break;
        case OOBIN_ID_EXTERNALSHEETS:   getExternalLinks().importExternalSheets( rStrm );   break;
        case OOBIN_ID_DEFINEDNAME:      getDefinedNames().importDefinedName( rStrm );       break;
    }
}

// oox.core.FragmentHandler2 interface ----------------------------------------

const RecordInfo* OoxWorkbookFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { OOBIN_ID_BOOKVIEWS,       OOBIN_ID_BOOKVIEWS + 1      },
        { OOBIN_ID_EXTERNALREFS,    OOBIN_ID_EXTERNALREFS + 1   },
        { OOBIN_ID_FUNCTIONGROUPS,  OOBIN_ID_FUNCTIONGROUPS + 2 },
        { OOBIN_ID_SHEETS,          OOBIN_ID_SHEETS + 1         },
        { OOBIN_ID_WORKBOOK,        OOBIN_ID_WORKBOOK + 1       },
        { -1,                       -1                          }
    };
    return spRecInfos;
}

void OoxWorkbookFragment::finalizeImport()
{
    ISegmentProgressBarRef xGlobalSegment = getProgressBar().createSegment( PROGRESS_LENGTH_GLOBALS );

    // read the theme substream
    OUString aThemeFragmentPath = getFragmentPathFromType( CREATE_OFFICEDOC_RELATIONSTYPE( "theme" ) );
    if( aThemeFragmentPath.getLength() > 0 )
        importOoxFragment( new ThemeFragmentHandler( getFilter(), aThemeFragmentPath, getTheme().getOrCreateCoreTheme() ) );
    xGlobalSegment->setPosition( 0.25 );

    // read the styles substream (requires finalized theme buffer)
    OUString aStylesFragmentPath = getFragmentPathFromType( CREATE_OFFICEDOC_RELATIONSTYPE( "styles" ) );
    if( aStylesFragmentPath.getLength() > 0 )
        importOoxFragment( new OoxStylesFragment( *this, aStylesFragmentPath ) );
    xGlobalSegment->setPosition( 0.5 );

    // read the shared string table substream (requires finalized styles buffer)
    OUString aSstFragmentPath = getFragmentPathFromType( CREATE_OFFICEDOC_RELATIONSTYPE( "sharedStrings" ) );
    if( aSstFragmentPath.getLength() > 0 )
        importOoxFragment( new OoxSharedStringsFragment( *this, aSstFragmentPath ) );
    xGlobalSegment->setPosition( 0.75 );

    // read the connections substream
    OUString aConnFragmentPath = getFragmentPathFromType( CREATE_OFFICEDOC_RELATIONSTYPE( "connections" ) );
    if( aConnFragmentPath.getLength() > 0 )
        importOoxFragment( new OoxConnectionsFragment( *this, aConnFragmentPath ) );
    xGlobalSegment->setPosition( 1.0 );

    /*  Create fragments for all sheets, before importing them. Needed to do
        some preprocessing in the fragment constructors, e.g. loading the table
        fragments for all sheets that are needed before the cell formulas are
        loaded. */
    typedef ::std::map< sal_Int32, FragmentHandlerRef > SheetFragmentMap;
    SheetFragmentMap aSheetFragments;
    WorksheetBuffer& rWorksheets = getWorksheets();
    sal_Int32 nSheetCount = rWorksheets.getInternalSheetCount();
    for( sal_Int32 nSheet = 0; nSheet < nSheetCount; ++nSheet )
    {
        if( const Relation* pRelation = getRelations().getRelationFromRelId( rWorksheets.getSheetRelId( nSheet ) ) )
        {
            // get fragment path of the sheet
            OUString aFragmentPath = getFragmentPathFromTarget( pRelation->maTarget );
            OSL_ENSURE( aFragmentPath.getLength() > 0, "OoxWorkbookFragment::finalizeImport - cannot access sheet fragment" );
            if( aFragmentPath.getLength() > 0 )
            {
                ::rtl::Reference< OoxWorksheetFragmentBase > xFragment;
                double fSegmentLength = getProgressBar().getFreeLength() / (nSheetCount - nSheet);
                ISegmentProgressBarRef xSheetSegment = getProgressBar().createSegment( fSegmentLength );

                // create the fragment according to the sheet type
                if( pRelation->maType == CREATE_OFFICEDOC_RELATIONSTYPE( "worksheet" ) )
                {
                    xFragment.set( new OoxWorksheetFragment( *this, aFragmentPath, xSheetSegment, SHEETTYPE_WORKSHEET, nSheet ) );
                }
                else if( pRelation->maType == CREATE_OFFICEDOC_RELATIONSTYPE( "chartsheet" ) )
                {
                    xFragment.set( new OoxChartsheetFragment( *this, aFragmentPath, xSheetSegment, nSheet ) );
                }
                else if( (pRelation->maType == CREATE_OUSTRING( "http://schemas.microsoft.com/office/2006/relationships/xlMacrosheet" )) ||
                         (pRelation->maType == CREATE_OUSTRING( "http://schemas.microsoft.com/office/2006/relationships/xlIntlMacrosheet" )) )
                {
                    xFragment.set( new OoxWorksheetFragment( *this, aFragmentPath, xSheetSegment, SHEETTYPE_MACROSHEET, nSheet ) );
                }
                else if( pRelation->maType == CREATE_OFFICEDOC_RELATIONSTYPE( "dialogsheet" ) )
                {
                    xFragment.set( new OoxWorksheetFragment( *this, aFragmentPath, xSheetSegment, SHEETTYPE_DIALOGSHEET, nSheet ) );
                }

                // insert the fragment into the map
                OSL_ENSURE( xFragment.is(), "OoxWorkbookFragment::finalizeImport - unknown sheet type" );
                OSL_ENSURE( !xFragment.is() || xFragment->isValidSheet(), "OoxWorkbookFragment::finalizeImport - missing sheet in document" );
                if( xFragment.is() && xFragment->isValidSheet() )
                    aSheetFragments[ nSheet ].set( xFragment.get() );
            }
        }
    }

    // create all defined names and database ranges
    getDefinedNames().finalizeImport();
    getTables().finalizeImport();

    // load all worksheets
    for( sal_Int32 nSheet = 0; nSheet < nSheetCount; ++nSheet )
    {
        SheetFragmentMap::iterator aIt = aSheetFragments.find( nSheet );
        if( aIt != aSheetFragments.end() )
        {
            // import the sheet fragment
            importOoxFragment( aIt->second );
            // delete fragment object, will free all allocated sheet buffers
            aSheetFragments.erase( aIt );
        }
    }

    // final conversions, e.g. calculation settings and view settings
    finalizeWorkbookImport();

    getPivotTables().finalizeImport();
}

// private --------------------------------------------------------------------

void OoxWorkbookFragment::importExternalReference( const AttributeList& rAttribs )
{
    if( ExternalLink* pExtLink = getExternalLinks().importExternalReference( rAttribs ).get() )
        importExternalLinkFragment( *pExtLink );
}

void OoxWorkbookFragment::importDefinedName( const AttributeList& rAttribs )
{
    mxCurrName = getDefinedNames().importDefinedName( rAttribs );
}

void OoxWorkbookFragment::importPivotCache( const AttributeList& rAttribs )
{
    OUString aFragmentPath = getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ) ) );
    if( (aFragmentPath.getLength() > 0) && rAttribs.hasAttribute( XML_cacheId ) )
    {
        sal_uInt32 nCacheId = rAttribs.getUnsignedInteger( XML_cacheId, 0 );
        importOoxFragment( new OoxPivotCacheFragment( *this, aFragmentPath, nCacheId ) );
    }
}

void OoxWorkbookFragment::importExternalRef( RecordInputStream& rStrm )
{
    if( ExternalLink* pExtLink = getExternalLinks().importExternalRef( rStrm ).get() )
        importExternalLinkFragment( *pExtLink );
}

void OoxWorkbookFragment::importExternalLinkFragment( ExternalLink& rExtLink )
{
    OUString aFragmentPath = getFragmentPathFromRelId( rExtLink.getRelId() );
    if( aFragmentPath.getLength() > 0 )
        importOoxFragment( new OoxExternalLinkFragment( *this, aFragmentPath, rExtLink ) );
}

// ============================================================================

namespace {

BiffDecoderRef lclImportFilePass_XOR( const WorkbookHelper& rHelper, BiffInputStream& rStrm )
{
    BiffDecoderRef xDecoder;
    OSL_ENSURE( rStrm.getRecLeft() == 4, "lclImportFilePass_XOR - wrong record size" );
    if( rStrm.getRecLeft() == 4 )
    {
        sal_uInt16 nBaseKey, nHash;
        rStrm >> nBaseKey >> nHash;
        xDecoder.reset( new BiffDecoder_XOR( rHelper, nBaseKey, nHash ) );
    }
    return xDecoder;
}

BiffDecoderRef lclImportFilePass_RCF( const WorkbookHelper& rHelper, BiffInputStream& rStrm )
{
    BiffDecoderRef xDecoder;
    OSL_ENSURE( rStrm.getRecLeft() == 48, "lclImportFilePass_RCF - wrong record size" );
    if( rStrm.getRecLeft() == 48 )
    {
        sal_uInt8 pnDocId[ 16 ];
        sal_uInt8 pnSaltData[ 16 ];
        sal_uInt8 pnSaltHash[ 16 ];
        rStrm.read( pnDocId, 16 );
        rStrm.read( pnSaltData, 16 );
        rStrm.read( pnSaltHash, 16 );
        xDecoder.reset( new BiffDecoder_RCF( rHelper, pnDocId, pnSaltData, pnSaltHash ) );
    }
    return xDecoder;
}

BiffDecoderRef lclImportFilePass_Strong( const WorkbookHelper& /*rHelper*/, BiffInputStream& /*rStrm*/ )
{
    // not supported
    return BiffDecoderRef();
}

BiffDecoderRef lclImportFilePass2( const WorkbookHelper& rHelper, BiffInputStream& rStrm )
{
    return lclImportFilePass_XOR( rHelper, rStrm );
}

BiffDecoderRef lclImportFilePass8( const WorkbookHelper& rHelper, BiffInputStream& rStrm )
{
    BiffDecoderRef xDecoder;

    switch( rStrm.readuInt16() )
    {
        case BIFF_FILEPASS_BIFF2:
            xDecoder = lclImportFilePass_XOR( rHelper, rStrm );
        break;

        case BIFF_FILEPASS_BIFF8:
            switch( rStrm.skip( 2 ).readuInt16() )
            {
                case BIFF_FILEPASS_BIFF8_RCF:
                    xDecoder = lclImportFilePass_RCF( rHelper, rStrm );
                break;
                case BIFF_FILEPASS_BIFF8_STRONG:
                    xDecoder = lclImportFilePass_Strong( rHelper, rStrm );
                break;
                default:
                    OSL_ENSURE( false, "lclImportFilePass8 - unknown BIFF8 encryption sub mode" );
            }
        break;

        default:
            OSL_ENSURE( false, "lclImportFilePass8 - unknown encryption mode" );
    }

    return xDecoder;
}

} // namespace

// ----------------------------------------------------------------------------

BiffWorkbookFragment::BiffWorkbookFragment( const WorkbookHelper& rHelper ) :
    BiffWorkbookFragmentBase( rHelper )
{
}

bool BiffWorkbookFragment::importFragment( BiffInputStream& rStrm )
{
    bool bRet = false;

    BiffFragmentType eFragment = startFragment( rStrm, getBiff() );
    switch( eFragment )
    {
        case BIFF_FRAGMENT_GLOBALS:
        {
            // import workbook globals fragment and create sheets in document
            ISegmentProgressBarRef xGlobalsProgress = getProgressBar().createSegment( PROGRESS_LENGTH_GLOBALS );
            bRet = importGlobalsFragment( rStrm, *xGlobalsProgress );
            // load sheet fragments (do not return false in bRet on missing/broken sheets)
            WorksheetBuffer& rWorksheets = getWorksheets();
            bool bNextSheet = bRet;
            for( sal_Int32 nSheet = 0, nSheetCount = rWorksheets.getInternalSheetCount(); bNextSheet && (nSheet < nSheetCount); ++nSheet )
            {
                // try to start a new sheet fragment
                double fSegmentLength = getProgressBar().getFreeLength() / (nSheetCount - nSheet);
                ISegmentProgressBarRef xSheetProgress = getProgressBar().createSegment( fSegmentLength );
                BiffFragmentType eSheetFragment = startFragment( rStrm, getBiff() );
                bNextSheet = importSheetFragment( rStrm, *xSheetProgress, eSheetFragment, nSheet );
            }
        }
        break;

        case BIFF_FRAGMENT_WORKSPACE:
        {
            bRet = importWorkspaceFragment( rStrm );
            // sheets are embedded in workspace fragment, nothing to do here
        }
        break;

        case BIFF_FRAGMENT_WORKSHEET:
        case BIFF_FRAGMENT_CHARTSHEET:
        case BIFF_FRAGMENT_MACROSHEET:
        {
            /*  Single sheet without globals
                - #i62752# possible in all BIFF versions
                - do not return false in bRet on missing/broken sheets. */
            getWorksheets().initializeSingleSheet();
            importSheetFragment( rStrm, getProgressBar(), eFragment, 0 );
            // success, even if stream is broken
            bRet = true;
        }
        break;

        default:;
    }

    // final conversions, e.g. calculation settings and view settings
    finalizeWorkbookImport();

    return bRet;
}

bool BiffWorkbookFragment::importWorkspaceFragment( BiffInputStream& rStrm )
{
    // enable workbook mode, has not been set yet in BIFF4 workspace files
    setIsWorkbookFile();

    WorksheetBuffer& rWorksheets = getWorksheets();
    bool bRet = true;

    // import the workspace globals
    ISegmentProgressBarRef xGlobalsProgress = getProgressBar().createSegment( PROGRESS_LENGTH_GLOBALS );
    bool bLoop = true;
    while( bRet && bLoop && rStrm.startNextRecord() && (rStrm.getRecId() != BIFF_ID_EOF) )
    {
        switch( rStrm.getRecId() )
        {
            case BIFF_ID_SHEET:         rWorksheets.importSheet( rStrm );       break;
            case BIFF_ID_CODEPAGE:      setCodePage( rStrm.readuInt16() );      break;
            case BIFF_ID_FILEPASS:      bRet = importFilePass( rStrm );         break;
            case BIFF_ID_SHEETHEADER:   rStrm.rewindRecord(); bLoop = false;    break;
        }
    }
    xGlobalsProgress->setPosition( 1.0 );

    // load sheet fragments (do not return false in bRet on missing/broken sheets)
    bool bNextSheet = bRet;
    for( sal_Int32 nSheet = 0, nSheetCount = rWorksheets.getInternalSheetCount(); bNextSheet && (nSheet < nSheetCount); ++nSheet )
    {
        // try to start a new sheet fragment (with leading SHEETHEADER record)
        bNextSheet = rStrm.startNextRecord() && (rStrm.getRecId() == BIFF_ID_SHEETHEADER);
        if( bNextSheet )
        {
            double fSegmentLength = getProgressBar().getFreeLength() / (nSheetCount - nSheet);
            ISegmentProgressBarRef xSheetProgress = getProgressBar().createSegment( fSegmentLength );
            /*  Read current sheet name (sheet substreams may not be in the
                same order as SHEET records are). */
            OUString aSheetName = rStrm.skip( 4 ).readByteString( false, getTextEncoding() );
            sal_Int32 nCurrSheet = rWorksheets.getFinalSheetIndex( aSheetName );
            // load the sheet fragment records
            BiffFragmentType eSheetFragment = startFragment( rStrm, getBiff() );
            bNextSheet = importSheetFragment( rStrm, *xSheetProgress, eSheetFragment, nCurrSheet );
            // do not return false in bRet on missing/broken sheets
        }
    }

    return bRet;
}

bool BiffWorkbookFragment::importGlobalsFragment( BiffInputStream& rStrm, ISegmentProgressBar& rProgressBar )
{
    WorkbookSettings& rWorkbookSett = getWorkbookSettings();
    ViewSettings& rViewSett = getViewSettings();
    SharedStringsBuffer& rSharedStrings = getSharedStrings();
    StylesBuffer& rStyles = getStyles();
    WorksheetBuffer& rWorksheets = getWorksheets();

    // collect records that need to be loaded in a second pass
    typedef ::std::vector< sal_Int64 > RecordHandleVec;
    RecordHandleVec aExtLinkRecs;

    bool bRet = true;
    bool bLoop = true;
    while( bRet && bLoop && rStrm.startNextRecord() )
    {
        sal_uInt16 nRecId = rStrm.getRecId();
        bool bExtLinkRec = false;

        /*  #i56376# BIFF5-BIFF8: If an EOF record for globals is missing,
            simulate it. The issue is about a document where the sheet fragment
            starts directly after the EXTSST record, without terminating the
            globals fragment with an EOF record. */
        if( isBofRecord( nRecId ) || (nRecId == BIFF_ID_EOF) )
        {
            bLoop = false;
        }
        else switch( nRecId )
        {
            // records in all BIFF versions
            case BIFF_ID_CODEPAGE:      setCodePage( rStrm.readuInt16() );      break;
            case BIFF_ID_DATEMODE:      rWorkbookSett.importDateMode( rStrm );  break;
            case BIFF_ID_FILEPASS:      bRet = importFilePass( rStrm );         break;
            case BIFF_ID_PRECISION:     rWorkbookSett.importPrecision( rStrm ); break;
            case BIFF_ID_WINDOW1:       rViewSett.importWindow1( rStrm );       break;

            // BIFF specific records
            default: switch( getBiff() )
            {
                case BIFF2: switch( nRecId )
                {
                    case BIFF2_ID_DEFINEDNAME:  bExtLinkRec = true;                 break;
                    case BIFF2_ID_EXTERNALNAME: bExtLinkRec = true;                 break;
                    case BIFF_ID_EXTERNSHEET:   bExtLinkRec = true;                 break;
                    case BIFF2_ID_FONT:         rStyles.importFont( rStrm );        break;
                    case BIFF_ID_FONTCOLOR:     rStyles.importFontColor( rStrm );   break;
                    case BIFF2_ID_FORMAT:       rStyles.importFormat( rStrm );      break;
                    case BIFF2_ID_XF:           rStyles.importXf( rStrm );          break;
                }
                break;

                case BIFF3: switch( nRecId )
                {
                    case BIFF_ID_CRN:           bExtLinkRec = true;                     break;
                    case BIFF3_ID_DEFINEDNAME:  bExtLinkRec = true;                     break;
                    case BIFF3_ID_EXTERNALNAME: bExtLinkRec = true;                     break;
                    case BIFF_ID_EXTERNSHEET:   bExtLinkRec = true;                     break;
                    case BIFF3_ID_FONT:         rStyles.importFont( rStrm );            break;
                    case BIFF2_ID_FORMAT:       rStyles.importFormat( rStrm );          break;
                    case BIFF_ID_HIDEOBJ:       rWorkbookSett.importHideObj( rStrm );   break;
                    case BIFF_ID_PALETTE:       rStyles.importPalette( rStrm );         break;
                    case BIFF_ID_STYLE:         rStyles.importStyle( rStrm );           break;
                    case BIFF_ID_XCT:           bExtLinkRec = true;                     break;
                    case BIFF3_ID_XF:           rStyles.importXf( rStrm );              break;
                }
                break;

                case BIFF4: switch( nRecId )
                {
                    case BIFF_ID_CRN:           bExtLinkRec = true;                     break;
                    case BIFF3_ID_DEFINEDNAME:  bExtLinkRec = true;                     break;
                    case BIFF3_ID_EXTERNALNAME: bExtLinkRec = true;                     break;
                    case BIFF_ID_EXTERNSHEET:   bExtLinkRec = true;                     break;
                    case BIFF3_ID_FONT:         rStyles.importFont( rStrm );            break;
                    case BIFF4_ID_FORMAT:       rStyles.importFormat( rStrm );          break;
                    case BIFF_ID_HIDEOBJ:       rWorkbookSett.importHideObj( rStrm );   break;
                    case BIFF_ID_PALETTE:       rStyles.importPalette( rStrm );         break;
                    case BIFF_ID_STYLE:         rStyles.importStyle( rStrm );           break;
                    case BIFF_ID_XCT:           bExtLinkRec = true;                     break;
                    case BIFF4_ID_XF:           rStyles.importXf( rStrm );              break;
                }
                break;

                case BIFF5: switch( nRecId )
                {
                    case BIFF_ID_BOOKBOOL:      rWorkbookSett.importBookBool( rStrm );  break;
                    case BIFF_ID_CRN:           bExtLinkRec = true;                     break;
                    case BIFF5_ID_DEFINEDNAME:  bExtLinkRec = true;                     break;
                    case BIFF5_ID_EXTERNALNAME: bExtLinkRec = true;                     break;
                    case BIFF_ID_EXTERNSHEET:   bExtLinkRec = true;                     break;
                    case BIFF5_ID_FONT:         rStyles.importFont( rStrm );            break;
                    case BIFF4_ID_FORMAT:       rStyles.importFormat( rStrm );          break;
                    case BIFF_ID_HIDEOBJ:       rWorkbookSett.importHideObj( rStrm );   break;
                    case BIFF_ID_PALETTE:       rStyles.importPalette( rStrm );         break;
                    case BIFF_ID_SHEET:         rWorksheets.importSheet( rStrm );       break;
                    case BIFF_ID_STYLE:         rStyles.importStyle( rStrm );           break;
                    case BIFF_ID_XCT:           bExtLinkRec = true;                     break;
                    case BIFF5_ID_XF:           rStyles.importXf( rStrm );              break;
                }
                break;

                case BIFF8: switch( nRecId )
                {
                    case BIFF_ID_BOOKBOOL:      rWorkbookSett.importBookBool( rStrm );  break;
                    case BIFF_ID_CODENAME:      rWorkbookSett.importCodeName( rStrm );  break;
                    case BIFF_ID_CRN:           bExtLinkRec = true;                     break;
                    case BIFF5_ID_DEFINEDNAME:  bExtLinkRec = true;                     break;
                    case BIFF_ID_EXTERNALBOOK:  bExtLinkRec = true;                     break;
                    case BIFF5_ID_EXTERNALNAME: bExtLinkRec = true;                     break;
                    case BIFF_ID_EXTERNSHEET:   bExtLinkRec = true;                     break;
                    case BIFF5_ID_FONT:         rStyles.importFont( rStrm );            break;
                    case BIFF4_ID_FORMAT:       rStyles.importFormat( rStrm );          break;
                    case BIFF_ID_HIDEOBJ:       rWorkbookSett.importHideObj( rStrm );   break;
                    case BIFF_ID_PALETTE:       rStyles.importPalette( rStrm );         break;
                    case BIFF_ID_SHEET:         rWorksheets.importSheet( rStrm );       break;
                    case BIFF_ID_SST:           rSharedStrings.importSst( rStrm );      break;
                    case BIFF_ID_STYLE:         rStyles.importStyle( rStrm );           break;
                    case BIFF_ID_USESELFS:      rWorkbookSett.importUsesElfs( rStrm );  break;
                    case BIFF_ID_XCT:           bExtLinkRec = true;                     break;
                    case BIFF5_ID_XF:           rStyles.importXf( rStrm );              break;
                }
                break;

                case BIFF_UNKNOWN: break;
            }
        }

        if( bExtLinkRec )
            aExtLinkRecs.push_back( rStrm.getRecHandle() );
    }

    // finalize global buffers
    rProgressBar.setPosition( 0.5 );
    rSharedStrings.finalizeImport();
    rStyles.finalizeImport();

    /*  Import external link data (EXTERNSHEET, EXTERNALNAME, DEFINEDNAME)
        which need existing internal sheets (SHEET records). The SHEET records
        may follow the external links records in some BIFF versions. */
    if( bRet && !aExtLinkRecs.empty() )
    {
        // remember current stream position (the EOF record)
        sal_Int64 nEofHandle = rStrm.getRecHandle();
        // this fragment class implements import of external link records
        BiffExternalLinkFragment aLinkFragment( *this, true );
        // import all records by using their cached record handle
        for( RecordHandleVec::const_iterator aIt = aExtLinkRecs.begin(), aEnd = aExtLinkRecs.end(); (aIt != aEnd) && rStrm.startRecordByHandle( *aIt ); ++aIt )
            aLinkFragment.importRecord( rStrm );
        // finalize global buffers
        aLinkFragment.finalizeImport();
        // seek back to the EOF record of the workbook globals fragment
        bRet = rStrm.startRecordByHandle( nEofHandle );
    }

    // #i56376# missing EOF - rewind before worksheet BOF record (see above)
    if( bRet && isBofRecord( rStrm.getRecId() ) )
        rStrm.rewindRecord();

    rProgressBar.setPosition( 1.0 );
    return bRet;
}

bool BiffWorkbookFragment::importSheetFragment( BiffInputStream& rStrm, ISegmentProgressBar& rProgressBar, BiffFragmentType eFragment, sal_Int32 nSheet )
{
    // find the sheet type for this fragment
    WorksheetType eSheetType = SHEETTYPE_WORKSHEET;
    bool bSkipSheet = false;
    switch( eFragment )
    {
        case BIFF_FRAGMENT_WORKSHEET:   eSheetType = SHEETTYPE_WORKSHEET;   break;
        case BIFF_FRAGMENT_CHARTSHEET:  eSheetType = SHEETTYPE_CHARTSHEET;  break;
        case BIFF_FRAGMENT_MACROSHEET:  eSheetType = SHEETTYPE_MACROSHEET;  break;
        case BIFF_FRAGMENT_MODULESHEET: eSheetType = SHEETTYPE_MODULESHEET; break;
        case BIFF_FRAGMENT_EMPTYSHEET:  bSkipSheet = true;                  break;
        default:                        return false;
    }

    // skip this worksheet fragment (e.g. fragment type is BIFF_FRAGMENT_EMPTYSHEET)
    if( bSkipSheet )
    {
        rProgressBar.setPosition( 1.0 );
        return skipFragment( rStrm );
    }

    /*  #i11183# Clear buffers that are used per-sheet, e.g. external links in
        BIFF4W and BIFF5 files, or defined names in BIFF4W files. */
    createBuffersPerSheet();

    // preprocess some records
    switch( getBiff() )
    {
        // load the workbook globals fragment records in BIFF2-BIFF4
        case BIFF2:
        case BIFF3:
        case BIFF4:
        {
            // set sheet index in defined names buffer to handle built-in names correctly
            getDefinedNames().setLocalSheetIndex( nSheet );
            // remember current record to seek back below
            sal_Int64 nRecHandle = rStrm.getRecHandle();
            // import the global records
            ISegmentProgressBarRef xGlobalsProgress = rProgressBar.createSegment( PROGRESS_LENGTH_GLOBALS );
            importGlobalsFragment( rStrm, *xGlobalsProgress );
            // rewind stream to fragment BOF record
            rStrm.startRecordByHandle( nRecHandle );
        }
        break;

        // load the external link records for this sheet in BIFF5
        case BIFF5:
        {
            // remember current record to seek back below
            sal_Int64 nRecHandle = rStrm.getRecHandle();
            // fragment implementing import of external link records
            BiffExternalLinkFragment( *this, false ).importFragment( rStrm );
            // rewind stream to fragment BOF record
            rStrm.startRecordByHandle( nRecHandle );
        }
        break;

        case BIFF8:
        break;

        case BIFF_UNKNOWN:
        break;
    }

    // create the worksheet fragment
    ISegmentProgressBarRef xSheetProgress = rProgressBar.createSegment( rProgressBar.getFreeLength() );
    ::boost::shared_ptr< BiffWorksheetFragmentBase > xFragment;
    switch( eSheetType )
    {
        case SHEETTYPE_WORKSHEET:
        case SHEETTYPE_MACROSHEET:
            xFragment.reset( new BiffWorksheetFragment( *this, xSheetProgress, eSheetType, nSheet ) );
        break;
        case SHEETTYPE_CHARTSHEET:
            xFragment.reset( new BiffChartsheetFragment( *this, xSheetProgress, nSheet ) );
        break;
        case SHEETTYPE_DIALOGSHEET:
        case SHEETTYPE_MODULESHEET:
            xFragment.reset( new BiffWorksheetFragmentBase( *this, xSheetProgress, eSheetType, nSheet ) );
        break;
    }
    // load the sheet fragment records
    return xFragment->isValidSheet() && xFragment->importFragment( rStrm );
}

bool BiffWorkbookFragment::importFilePass( BiffInputStream& rStrm )
{
    rStrm.enableDecoder( false );
    BiffDecoderRef xDecoder = (getBiff() == BIFF8) ?
        lclImportFilePass8( *this, rStrm ) : lclImportFilePass2( *this, rStrm );

    // set decoder at import stream
    rStrm.setDecoder( xDecoder );
    //! TODO remember encryption state for export
//    rStrm.GetRoot().GetExtDocOptions().GetDocSettings().mbEncrypted = true;

    return xDecoder.get() && xDecoder->isValid();
}

// ============================================================================

} // namespace xls
} // namespace oox

