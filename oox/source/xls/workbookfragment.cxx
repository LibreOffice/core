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

#include "oox/xls/workbookfragment.hxx"

#include <com/sun/star/table/CellAddress.hpp>
#include "oox/core/filterbase.hxx"
#include "oox/drawingml/themefragmenthandler.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/progressbar.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/ole/olestorage.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/chartsheetfragment.hxx"
#include "oox/xls/connectionsfragment.hxx"
#include "oox/xls/externallinkbuffer.hxx"
#include "oox/xls/externallinkfragment.hxx"
#include "oox/xls/pivotcachebuffer.hxx"
#include "oox/xls/sharedstringsbuffer.hxx"
#include "oox/xls/sharedstringsfragment.hxx"
#include "oox/xls/stylesfragment.hxx"
#include "oox/xls/tablebuffer.hxx"
#include "oox/xls/themebuffer.hxx"
#include "oox/xls/viewsettings.hxx"
#include "oox/xls/workbooksettings.hxx"
#include "oox/xls/worksheetbuffer.hxx"
#include "oox/xls/worksheetfragment.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;
using namespace ::oox::core;

using ::oox::drawingml::ThemeFragmentHandler;
using ::rtl::OUString;

// ============================================================================

namespace {

const double PROGRESS_LENGTH_GLOBALS        = 0.1;      /// 10% of progress bar for globals import.

} // namespace

// ============================================================================

WorkbookFragment::WorkbookFragment( const WorkbookHelper& rHelper, const OUString& rFragmentPath ) :
    WorkbookFragmentBase( rHelper, rFragmentPath )
{
}

ContextHandlerRef WorkbookFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( workbook ) ) return this;
        break;

        case XLS_TOKEN( workbook ):
            switch( nElement )
            {
                case XLS_TOKEN( sheets ):
                case XLS_TOKEN( bookViews ):
                case XLS_TOKEN( externalReferences ):
                case XLS_TOKEN( definedNames ):
                case XLS_TOKEN( pivotCaches ):          return this;

                case XLS_TOKEN( fileSharing ):          getWorkbookSettings().importFileSharing( rAttribs );    break;
                case XLS_TOKEN( workbookPr ):           getWorkbookSettings().importWorkbookPr( rAttribs );     break;
                case XLS_TOKEN( calcPr ):               getWorkbookSettings().importCalcPr( rAttribs );         break;
                case XLS_TOKEN( oleSize ):              getViewSettings().importOleSize( rAttribs );            break;
            }
        break;

        case XLS_TOKEN( sheets ):
            if( nElement == XLS_TOKEN( sheet ) ) getWorksheets().importSheet( rAttribs );
        break;
        case XLS_TOKEN( bookViews ):
            if( nElement == XLS_TOKEN( workbookView ) ) getViewSettings().importWorkbookView( rAttribs );
        break;
        case XLS_TOKEN( externalReferences ):
            if( nElement == XLS_TOKEN( externalReference ) ) importExternalReference( rAttribs );
        break;
        case XLS_TOKEN( definedNames ):
            if( nElement == XLS_TOKEN( definedName ) ) { importDefinedName( rAttribs ); return this; } // collect formula
        break;
        case XLS_TOKEN( pivotCaches ):
            if( nElement == XLS_TOKEN( pivotCache ) ) importPivotCache( rAttribs );
        break;
    }
    return 0;
}

void WorkbookFragment::onCharacters( const OUString& rChars )
{
    if( isCurrentElement( XLS_TOKEN( definedName ) ) && mxCurrName.get() )
        mxCurrName->setFormula( rChars );
}

ContextHandlerRef WorkbookFragment::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == BIFF12_ID_WORKBOOK ) return this;
        break;

        case BIFF12_ID_WORKBOOK:
            switch( nRecId )
            {
                case BIFF12_ID_SHEETS:
                case BIFF12_ID_BOOKVIEWS:
                case BIFF12_ID_EXTERNALREFS:
                case BIFF12_ID_PIVOTCACHES:     return this;

                case BIFF12_ID_FILESHARING:     getWorkbookSettings().importFileSharing( rStrm );   break;
                case BIFF12_ID_WORKBOOKPR:      getWorkbookSettings().importWorkbookPr( rStrm );    break;
                case BIFF12_ID_CALCPR:          getWorkbookSettings().importCalcPr( rStrm );        break;
                case BIFF12_ID_OLESIZE:         getViewSettings().importOleSize( rStrm );           break;
                case BIFF12_ID_DEFINEDNAME:     getDefinedNames().importDefinedName( rStrm );       break;
            }
        break;

        case BIFF12_ID_SHEETS:
            if( nRecId == BIFF12_ID_SHEET ) getWorksheets().importSheet( rStrm );
        break;
        case BIFF12_ID_BOOKVIEWS:
            if( nRecId == BIFF12_ID_WORKBOOKVIEW ) getViewSettings().importWorkbookView( rStrm );
        break;

        case BIFF12_ID_EXTERNALREFS:
            switch( nRecId )
            {
                case BIFF12_ID_EXTERNALREF:     importExternalRef( rStrm );                         break;
                case BIFF12_ID_EXTERNALSELF:    getExternalLinks().importExternalSelf( rStrm );     break;
                case BIFF12_ID_EXTERNALSAME:    getExternalLinks().importExternalSame( rStrm );     break;
                case BIFF12_ID_EXTERNALADDIN:   getExternalLinks().importExternalAddin( rStrm );    break;
                case BIFF12_ID_EXTERNALSHEETS:  getExternalLinks().importExternalSheets( rStrm );   break;
            }
        break;

        case BIFF12_ID_PIVOTCACHES:
            if( nRecId == BIFF12_ID_PIVOTCACHE ) importPivotCache( rStrm );
    }
    return 0;
}

const RecordInfo* WorkbookFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { BIFF12_ID_BOOKVIEWS,      BIFF12_ID_BOOKVIEWS + 1         },
        { BIFF12_ID_EXTERNALREFS,   BIFF12_ID_EXTERNALREFS + 1      },
        { BIFF12_ID_FUNCTIONGROUPS, BIFF12_ID_FUNCTIONGROUPS + 2    },
        { BIFF12_ID_PIVOTCACHE,     BIFF12_ID_PIVOTCACHE + 1        },
        { BIFF12_ID_PIVOTCACHES,    BIFF12_ID_PIVOTCACHES + 1       },
        { BIFF12_ID_SHEETS,         BIFF12_ID_SHEETS + 1            },
        { BIFF12_ID_WORKBOOK,       BIFF12_ID_WORKBOOK + 1          },
        { -1,                       -1                              }
    };
    return spRecInfos;
}

void WorkbookFragment::finalizeImport()
{
    ISegmentProgressBarRef xGlobalSegment = getProgressBar().createSegment( PROGRESS_LENGTH_GLOBALS );

    // read the theme substream
    OUString aThemeFragmentPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "theme" ) );
    if( aThemeFragmentPath.getLength() > 0 )
        importOoxFragment( new ThemeFragmentHandler( getFilter(), aThemeFragmentPath, getTheme() ) );
    xGlobalSegment->setPosition( 0.25 );

    // read the styles substream (requires finalized theme buffer)
    OUString aStylesFragmentPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "styles" ) );
    if( aStylesFragmentPath.getLength() > 0 )
        importOoxFragment( new StylesFragment( *this, aStylesFragmentPath ) );
    xGlobalSegment->setPosition( 0.5 );

    // read the shared string table substream (requires finalized styles buffer)
    OUString aSstFragmentPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "sharedStrings" ) );
    if( aSstFragmentPath.getLength() > 0 )
        importOoxFragment( new SharedStringsFragment( *this, aSstFragmentPath ) );
    xGlobalSegment->setPosition( 0.75 );

    // read the connections substream
    OUString aConnFragmentPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "connections" ) );
    if( aConnFragmentPath.getLength() > 0 )
        importOoxFragment( new ConnectionsFragment( *this, aConnFragmentPath ) );
    xGlobalSegment->setPosition( 1.0 );

    /*  Create fragments for all sheets, before importing them. Needed to do
        some preprocessing in the fragment constructors, e.g. loading the table
        fragments for all sheets that are needed before the cell formulas are
        loaded. */
    typedef ::std::vector< FragmentHandlerRef > SheetFragmentVector;
    SheetFragmentVector aSheetFragments;
    WorksheetBuffer& rWorksheets = getWorksheets();
    sal_Int32 nWorksheetCount = rWorksheets.getWorksheetCount();
    for( sal_Int32 nWorksheet = 0; nWorksheet < nWorksheetCount; ++nWorksheet )
    {
        sal_Int16 nCalcSheet = rWorksheets.getCalcSheetIndex( nWorksheet );
        const Relation* pRelation = getRelations().getRelationFromRelId( rWorksheets.getWorksheetRelId( nWorksheet ) );
        if( (nCalcSheet >= 0) && pRelation )
        {
            // get fragment path of the sheet
            OUString aFragmentPath = getFragmentPathFromRelation( *pRelation );
            OSL_ENSURE( aFragmentPath.getLength() > 0, "WorkbookFragment::finalizeImport - cannot access sheet fragment" );
            if( aFragmentPath.getLength() > 0 )
            {
                ::rtl::Reference< WorksheetFragmentBase > xFragment;
                double fSegmentLength = getProgressBar().getFreeLength() / (nWorksheetCount - nWorksheet);
                ISegmentProgressBarRef xSheetSegment = getProgressBar().createSegment( fSegmentLength );

                // create the fragment according to the sheet type
                if( pRelation->maType == CREATE_OFFICEDOC_RELATION_TYPE( "worksheet" ) )
                {
                    xFragment.set( new WorksheetFragment( *this, aFragmentPath, xSheetSegment, SHEETTYPE_WORKSHEET, nCalcSheet ) );
                }
                else if( pRelation->maType == CREATE_OFFICEDOC_RELATION_TYPE( "chartsheet" ) )
                {
                    xFragment.set( new ChartsheetFragment( *this, aFragmentPath, xSheetSegment, nCalcSheet ) );
                }
                else if( (pRelation->maType == CREATE_MSOFFICE_RELATION_TYPE( "xlMacrosheet" )) ||
                         (pRelation->maType == CREATE_MSOFFICE_RELATION_TYPE( "xlIntlMacrosheet" )) )
                {
                    xFragment.set( new WorksheetFragment( *this, aFragmentPath, xSheetSegment, SHEETTYPE_MACROSHEET, nCalcSheet ) );
                }
                else if( pRelation->maType == CREATE_OFFICEDOC_RELATION_TYPE( "dialogsheet" ) )
                {
                    xFragment.set( new WorksheetFragment( *this, aFragmentPath, xSheetSegment, SHEETTYPE_DIALOGSHEET, nCalcSheet ) );
                }

                // insert the fragment into the map
                OSL_ENSURE( xFragment.is(), "WorkbookFragment::finalizeImport - unknown sheet type" );
                OSL_ENSURE( !xFragment.is() || xFragment->isValidSheet(), "WorkbookFragment::finalizeImport - missing sheet in document" );
                if( xFragment.is() && xFragment->isValidSheet() )
                    aSheetFragments.push_back( xFragment.get() );
            }
        }
    }

    // create all defined names and database ranges
    getDefinedNames().finalizeImport();
    getTables().finalizeImport();

    // load all worksheets
    for( SheetFragmentVector::iterator aIt = aSheetFragments.begin(), aEnd = aSheetFragments.end(); aIt != aEnd; ++aIt )
    {
        // import the sheet fragment
        importOoxFragment( *aIt );
        // delete fragment object, will free all allocated sheet buffers
        aIt->clear();
    }

    // open the VBA project storage
    OUString aVbaFragmentPath = getFragmentPathFromFirstType( CREATE_MSOFFICE_RELATION_TYPE( "vbaProject" ) );
    if( aVbaFragmentPath.getLength() > 0 )
    {
        Reference< XInputStream > xInStrm = getBaseFilter().openInputStream( aVbaFragmentPath );
        if( xInStrm.is() )
            setVbaProjectStorage( StorageRef( new ::oox::ole::OleStorage( getGlobalFactory(), xInStrm, false ) ) );
    }

    // final conversions, e.g. calculation settings and view settings
    finalizeWorkbookImport();
}

// private --------------------------------------------------------------------

void WorkbookFragment::importExternalReference( const AttributeList& rAttribs )
{
    if( ExternalLink* pExtLink = getExternalLinks().importExternalReference( rAttribs ).get() )
        importExternalLinkFragment( *pExtLink );
}

void WorkbookFragment::importDefinedName( const AttributeList& rAttribs )
{
    mxCurrName = getDefinedNames().importDefinedName( rAttribs );
}

void WorkbookFragment::importPivotCache( const AttributeList& rAttribs )
{
    sal_Int32 nCacheId = rAttribs.getInteger( XML_cacheId, -1 );
    OUString aRelId = rAttribs.getString( R_TOKEN( id ), OUString() );
    importPivotCacheDefFragment( aRelId, nCacheId );
}

void WorkbookFragment::importExternalRef( SequenceInputStream& rStrm )
{
    if( ExternalLink* pExtLink = getExternalLinks().importExternalRef( rStrm ).get() )
        importExternalLinkFragment( *pExtLink );
}

void WorkbookFragment::importPivotCache( SequenceInputStream& rStrm )
{
    sal_Int32 nCacheId = rStrm.readInt32();
    OUString aRelId = BiffHelper::readString( rStrm );
    importPivotCacheDefFragment( aRelId, nCacheId );
}

void WorkbookFragment::importExternalLinkFragment( ExternalLink& rExtLink )
{
    OUString aFragmentPath = getFragmentPathFromRelId( rExtLink.getRelId() );
    if( aFragmentPath.getLength() > 0 )
        importOoxFragment( new ExternalLinkFragment( *this, aFragmentPath, rExtLink ) );
}

void WorkbookFragment::importPivotCacheDefFragment( const OUString& rRelId, sal_Int32 nCacheId )
{
    // pivot caches will be imported on demand, here we just store the fragment path in the buffer
    getPivotCaches().registerPivotCacheFragment( nCacheId, getFragmentPathFromRelId( rRelId ) );
}

// ============================================================================

BiffWorkbookFragment::BiffWorkbookFragment( const WorkbookHelper& rHelper, const OUString& rStrmName ) :
    BiffWorkbookFragmentBase( rHelper, rStrmName )
{
}

bool BiffWorkbookFragment::importFragment()
{
    bool bRet = false;

    BiffFragmentType eFragment = startFragment( getBiff() );
    switch( eFragment )
    {
        case BIFF_FRAGMENT_GLOBALS:
        {
            // import workbook globals fragment and create sheets in document
            ISegmentProgressBarRef xGlobalsProgress = getProgressBar().createSegment( PROGRESS_LENGTH_GLOBALS );
            bRet = importGlobalsFragment( *xGlobalsProgress );
            // load sheet fragments (do not return false in bRet on missing/broken sheets)
            WorksheetBuffer& rWorksheets = getWorksheets();
            bool bNextSheet = bRet;
            for( sal_Int32 nWorksheet = 0, nWorksheetCount = rWorksheets.getWorksheetCount(); bNextSheet && (nWorksheet < nWorksheetCount); ++nWorksheet )
            {
                // try to start a new sheet fragment
                double fSegmentLength = getProgressBar().getFreeLength() / (nWorksheetCount - nWorksheet);
                ISegmentProgressBarRef xSheetProgress = getProgressBar().createSegment( fSegmentLength );
                BiffFragmentType eSheetFragment = startFragment( getBiff(), rWorksheets.getBiffRecordHandle( nWorksheet ) );
                sal_Int16 nCalcSheet = rWorksheets.getCalcSheetIndex( nWorksheet );
                bNextSheet = importSheetFragment( *xSheetProgress, eSheetFragment, nCalcSheet );
            }
        }
        break;

        case BIFF_FRAGMENT_WORKSPACE:
        {
            bRet = importWorkspaceFragment();
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
            importSheetFragment( getProgressBar(), eFragment, 0 );
            // success, even if stream is broken
            bRet = true;
        }
        break;

        default:;
    }

    // final conversions, e.g. calculation settings and view settings
    if( bRet )
        finalizeWorkbookImport();

    return bRet;
}

bool BiffWorkbookFragment::importWorkspaceFragment()
{
    // enable workbook mode, has not been set yet in BIFF4 workspace files
    setIsWorkbookFile();

    WorksheetBuffer& rWorksheets = getWorksheets();
    bool bRet = true;

    // import the workspace globals
    ISegmentProgressBarRef xGlobalsProgress = getProgressBar().createSegment( PROGRESS_LENGTH_GLOBALS );
    bool bLoop = true;
    BiffInputStream& rStrm = getInputStream();
    while( bRet && bLoop && rStrm.startNextRecord() && (rStrm.getRecId() != BIFF_ID_EOF) )
    {
        switch( rStrm.getRecId() )
        {
            case BIFF_ID_SHEET:         rWorksheets.importSheet( rStrm );                   break;
            case BIFF_ID_CODEPAGE:      setCodePage( rStrm.readuInt16() );                  break;
            case BIFF_ID_FILEPASS:      bRet = getCodecHelper().importFilePass( rStrm );    break;
            case BIFF_ID_SHEETHEADER:   rStrm.rewindRecord(); bLoop = false;                break;
        }
    }
    xGlobalsProgress->setPosition( 1.0 );

    // load sheet fragments (do not return false in bRet on missing/broken sheets)
    bool bNextSheet = bRet;
    for( sal_Int32 nWorksheet = 0, nWorksheetCount = rWorksheets.getWorksheetCount(); bNextSheet && (nWorksheet < nWorksheetCount); ++nWorksheet )
    {
        // try to start a new sheet fragment (with leading SHEETHEADER record)
        bNextSheet = rStrm.startNextRecord() && (rStrm.getRecId() == BIFF_ID_SHEETHEADER);
        if( bNextSheet )
        {
            double fSegmentLength = getProgressBar().getFreeLength() / (nWorksheetCount - nWorksheet);
            ISegmentProgressBarRef xSheetProgress = getProgressBar().createSegment( fSegmentLength );
            /*  Read current sheet name (sheet substreams may not be in the
                same order as SHEET records are). */
            rStrm.skip( 4 );
            OUString aSheetName = rStrm.readByteStringUC( false, getTextEncoding() );
            sal_Int16 nCurrSheet = rWorksheets.getCalcSheetIndex( aSheetName );
            // load the sheet fragment records
            BiffFragmentType eSheetFragment = startFragment( getBiff() );
            bNextSheet = importSheetFragment( *xSheetProgress, eSheetFragment, nCurrSheet );
            // do not return false in bRet on missing/broken sheets
        }
    }

    return bRet;
}

bool BiffWorkbookFragment::importGlobalsFragment( ISegmentProgressBar& rProgressBar )
{
    WorkbookSettings& rWorkbookSett = getWorkbookSettings();
    ViewSettings& rViewSett = getViewSettings();
    SharedStringsBuffer& rSharedStrings = getSharedStrings();
    StylesBuffer& rStyles = getStyles();
    WorksheetBuffer& rWorksheets = getWorksheets();
    PivotCacheBuffer& rPivotCaches = getPivotCaches();
    bool bHasVbaProject = false;
    bool bEmptyVbaProject = false;

    // collect records that need to be loaded in a second pass
    typedef ::std::vector< sal_Int64 > RecordHandleVec;
    RecordHandleVec aExtLinkRecs;

    bool bRet = true;
    bool bLoop = true;
    BiffInputStream& rStrm = getInputStream();
    while( bRet && bLoop && rStrm.startNextRecord() )
    {
        sal_uInt16 nRecId = rStrm.getRecId();
        bool bExtLinkRec = false;

        /*  #i56376# BIFF5-BIFF8: If an EOF record for globals is missing,
            simulate it. The issue is about a document where the sheet fragment
            starts directly after the EXTSST record, without terminating the
            globals fragment with an EOF record. */
        if( BiffHelper::isBofRecord( rStrm ) || (nRecId == BIFF_ID_EOF) )
        {
            bLoop = false;
        }
        else switch( nRecId )
        {
            // records in all BIFF versions
            case BIFF_ID_CODEPAGE:      setCodePage( rStrm.readuInt16() );                  break;
            case BIFF_ID_DATEMODE:      rWorkbookSett.importDateMode( rStrm );              break;
            case BIFF_ID_FILEPASS:      bRet = getCodecHelper().importFilePass( rStrm );    break;
            case BIFF_ID_PRECISION:     rWorkbookSett.importPrecision( rStrm );             break;
            case BIFF_ID_WINDOW1:       rViewSett.importWindow1( rStrm );                   break;

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
                    case BIFF_ID_CRN:           bExtLinkRec = true;                         break;
                    case BIFF3_ID_DEFINEDNAME:  bExtLinkRec = true;                         break;
                    case BIFF3_ID_EXTERNALNAME: bExtLinkRec = true;                         break;
                    case BIFF_ID_EXTERNSHEET:   bExtLinkRec = true;                         break;
                    case BIFF_ID_FILESHARING:   rWorkbookSett.importFileSharing( rStrm );   break;
                    case BIFF3_ID_FONT:         rStyles.importFont( rStrm );                break;
                    case BIFF2_ID_FORMAT:       rStyles.importFormat( rStrm );              break;
                    case BIFF_ID_HIDEOBJ:       rWorkbookSett.importHideObj( rStrm );       break;
                    case BIFF_ID_PALETTE:       rStyles.importPalette( rStrm );             break;
                    case BIFF_ID_STYLE:         rStyles.importStyle( rStrm );               break;
                    case BIFF_ID_XCT:           bExtLinkRec = true;                         break;
                    case BIFF3_ID_XF:           rStyles.importXf( rStrm );                  break;
                }
                break;

                case BIFF4: switch( nRecId )
                {
                    case BIFF_ID_CRN:           bExtLinkRec = true;                         break;
                    case BIFF3_ID_DEFINEDNAME:  bExtLinkRec = true;                         break;
                    case BIFF3_ID_EXTERNALNAME: bExtLinkRec = true;                         break;
                    case BIFF_ID_EXTERNSHEET:   bExtLinkRec = true;                         break;
                    case BIFF_ID_FILESHARING:   rWorkbookSett.importFileSharing( rStrm );   break;
                    case BIFF3_ID_FONT:         rStyles.importFont( rStrm );                break;
                    case BIFF4_ID_FORMAT:       rStyles.importFormat( rStrm );              break;
                    case BIFF_ID_HIDEOBJ:       rWorkbookSett.importHideObj( rStrm );       break;
                    case BIFF_ID_PALETTE:       rStyles.importPalette( rStrm );             break;
                    case BIFF_ID_STYLE:         rStyles.importStyle( rStrm );               break;
                    case BIFF_ID_XCT:           bExtLinkRec = true;                         break;
                    case BIFF4_ID_XF:           rStyles.importXf( rStrm );                  break;
                }
                break;

                case BIFF5: switch( nRecId )
                {
                    case BIFF_ID_BOOKBOOL:      rWorkbookSett.importBookBool( rStrm );      break;
                    case BIFF_ID_CRN:           bExtLinkRec = true;                         break;
                    case BIFF5_ID_DEFINEDNAME:  bExtLinkRec = true;                         break;
                    case BIFF5_ID_EXTERNALNAME: bExtLinkRec = true;                         break;
                    case BIFF_ID_EXTERNSHEET:   bExtLinkRec = true;                         break;
                    case BIFF_ID_FILESHARING:   rWorkbookSett.importFileSharing( rStrm );   break;
                    case BIFF5_ID_FONT:         rStyles.importFont( rStrm );                break;
                    case BIFF4_ID_FORMAT:       rStyles.importFormat( rStrm );              break;
                    case BIFF_ID_HIDEOBJ:       rWorkbookSett.importHideObj( rStrm );       break;
                    case BIFF_ID_OLESIZE:       rViewSett.importOleSize( rStrm );           break;
                    case BIFF_ID_PALETTE:       rStyles.importPalette( rStrm );             break;
                    case BIFF_ID_PIVOTCACHE:    rPivotCaches.importPivotCacheRef( rStrm );  break;
                    case BIFF_ID_SHEET:         rWorksheets.importSheet( rStrm );           break;
                    case BIFF_ID_STYLE:         rStyles.importStyle( rStrm );               break;
                    case BIFF_ID_XCT:           bExtLinkRec = true;                         break;
                    case BIFF5_ID_XF:           rStyles.importXf( rStrm );                  break;
                }
                break;

                case BIFF8: switch( nRecId )
                {
                    case BIFF_ID_BOOKBOOL:          rWorkbookSett.importBookBool( rStrm );      break;
                    case BIFF_ID_CODENAME:          rWorkbookSett.importCodeName( rStrm );      break;
                    case BIFF_ID_CRN:               bExtLinkRec = true;                         break;
                    case BIFF5_ID_DEFINEDNAME:      bExtLinkRec = true;                         break;
                    case BIFF_ID_EXTERNALBOOK:      bExtLinkRec = true;                         break;
                    case BIFF5_ID_EXTERNALNAME:     bExtLinkRec = true;                         break;
                    case BIFF_ID_EXTERNSHEET:       bExtLinkRec = true;                         break;
                    case BIFF_ID_FILESHARING:       rWorkbookSett.importFileSharing( rStrm );   break;
                    case BIFF5_ID_FONT:             rStyles.importFont( rStrm );                break;
                    case BIFF4_ID_FORMAT:           rStyles.importFormat( rStrm );              break;
                    case BIFF_ID_HIDEOBJ:           rWorkbookSett.importHideObj( rStrm );       break;
                    case BIFF_ID_OLESIZE:           rViewSett.importOleSize( rStrm );           break;
                    case BIFF_ID_PALETTE:           rStyles.importPalette( rStrm );             break;
                    case BIFF_ID_PIVOTCACHE:        rPivotCaches.importPivotCacheRef( rStrm );  break;
                    case BIFF_ID_SHEET:             rWorksheets.importSheet( rStrm );           break;
                    case BIFF_ID_SST:               rSharedStrings.importSst( rStrm );          break;
                    case BIFF_ID_STYLE:             rStyles.importStyle( rStrm );               break;
                    case BIFF_ID_USESELFS:          rWorkbookSett.importUsesElfs( rStrm );      break;
                    case BIFF_ID_VBAPROJECT:        bHasVbaProject = true;                      break;
                    case BIFF_ID_VBAPROJECTEMPTY:   bEmptyVbaProject = true;                    break;
                    case BIFF_ID_XCT:               bExtLinkRec = true;                         break;
                    case BIFF5_ID_XF:               rStyles.importXf( rStrm );                  break;
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
    if( bRet )
    {
        rSharedStrings.finalizeImport();
        rStyles.finalizeImport();
    }

    /*  Import external link data (EXTERNSHEET, EXTERNALNAME, DEFINEDNAME)
        which need existing internal sheets (SHEET records). The SHEET records
        may follow the external links records in some BIFF versions. */
    if( bRet && !aExtLinkRecs.empty() )
    {
        // remember current stream position (the EOF record)
        sal_Int64 nEofHandle = rStrm.getRecHandle();
        // context handler implementing import of external link records
        BiffExternalSheetDataContext aSheetContext( *this, true );
        // import all records by using their cached record handle
        for( RecordHandleVec::const_iterator aIt = aExtLinkRecs.begin(), aEnd = aExtLinkRecs.end(); (aIt != aEnd) && rStrm.startRecordByHandle( *aIt ); ++aIt )
            aSheetContext.importRecord( rStrm );
        // finalize global buffers
        getDefinedNames().finalizeImport();
        // seek back to the EOF record of the workbook globals fragment
        bRet = rStrm.startRecordByHandle( nEofHandle );
    }

    // open the VBA project storage
    if( bHasVbaProject && !bEmptyVbaProject )
        setVbaProjectStorage( getBaseFilter().openSubStorage( CREATE_OUSTRING( "_VBA_PROJECT_CUR" ), false ) );

    // #i56376# missing EOF - rewind before worksheet BOF record (see above)
    if( bRet && BiffHelper::isBofRecord( rStrm ) )
        rStrm.rewindRecord();

    rProgressBar.setPosition( 1.0 );
    return bRet;
}

bool BiffWorkbookFragment::importSheetFragment( ISegmentProgressBar& rProgressBar, BiffFragmentType eFragment, sal_Int16 nCalcSheet )
{
    // no Calc sheet - skip the fragment
    if( nCalcSheet < 0 )
        return skipFragment();

    // find the sheet type for this fragment
    WorksheetType eSheetType = SHEETTYPE_EMPTYSHEET;
    switch( eFragment )
    {
        case BIFF_FRAGMENT_WORKSHEET:   eSheetType = SHEETTYPE_WORKSHEET;   break;
        case BIFF_FRAGMENT_CHARTSHEET:  eSheetType = SHEETTYPE_CHARTSHEET;  break;
        case BIFF_FRAGMENT_MACROSHEET:  eSheetType = SHEETTYPE_MACROSHEET;  break;
        case BIFF_FRAGMENT_MODULESHEET: eSheetType = SHEETTYPE_MODULESHEET; break;
        case BIFF_FRAGMENT_EMPTYSHEET:  eSheetType = SHEETTYPE_EMPTYSHEET;  break;
        default:                        return false;
    }

    /*  #i11183# Clear buffers that are used per-sheet, e.g. external links in
        BIFF4W and BIFF5 files, or defined names in BIFF4W files. */
    createBuffersPerSheet( nCalcSheet );

    // preprocess some records
    BiffInputStream& rStrm = getInputStream();
    switch( getBiff() )
    {
        // load the workbook globals fragment records in BIFF2-BIFF4
        case BIFF2:
        case BIFF3:
        case BIFF4:
        {
            // remember current record to seek back below
            sal_Int64 nRecHandle = rStrm.getRecHandle();
            // import the global records
            ISegmentProgressBarRef xGlobalsProgress = rProgressBar.createSegment( PROGRESS_LENGTH_GLOBALS );
            importGlobalsFragment( *xGlobalsProgress );
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
            BiffExternalLinkFragment( *this ).importFragment();
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
        case SHEETTYPE_DIALOGSHEET:
            xFragment.reset( new BiffWorksheetFragment( *this, xSheetProgress, eSheetType, nCalcSheet ) );
        break;
        case SHEETTYPE_CHARTSHEET:
            xFragment.reset( new BiffChartsheetFragment( *this, xSheetProgress, nCalcSheet ) );
        break;
        case SHEETTYPE_MODULESHEET:
        case SHEETTYPE_EMPTYSHEET:
            xFragment.reset( new BiffSkipWorksheetFragment( *this, xSheetProgress, nCalcSheet ) );
        break;
    }
    // load the sheet fragment records
    return xFragment->isValidSheet() && xFragment->importFragment();
}

// ============================================================================

} // namespace xls
} // namespace oox
