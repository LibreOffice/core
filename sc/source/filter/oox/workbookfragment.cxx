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

#include "workbookfragment.hxx"

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/XCalculatable.hpp>
#include "oox/core/filterbase.hxx"
#include "oox/drawingml/themefragmenthandler.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/progressbar.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/ole/olestorage.hxx"
#include "biffinputstream.hxx"
#include "chartsheetfragment.hxx"
#include "connectionsfragment.hxx"
#include "externallinkbuffer.hxx"
#include "externallinkfragment.hxx"
#include "pivotcachebuffer.hxx"
#include "sharedstringsbuffer.hxx"
#include "sharedstringsfragment.hxx"
#include "stylesfragment.hxx"
#include "tablebuffer.hxx"
#include "themebuffer.hxx"
#include "viewsettings.hxx"
#include "workbooksettings.hxx"
#include "worksheetbuffer.hxx"
#include "worksheetfragment.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sheet;
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
    if( !aThemeFragmentPath.isEmpty() )
        importOoxFragment( new ThemeFragmentHandler( getFilter(), aThemeFragmentPath, getTheme() ) );
    xGlobalSegment->setPosition( 0.25 );

    // read the styles substream (requires finalized theme buffer)
    OUString aStylesFragmentPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "styles" ) );
    if( !aStylesFragmentPath.isEmpty() )
        importOoxFragment( new StylesFragment( *this, aStylesFragmentPath ) );
    xGlobalSegment->setPosition( 0.5 );

    // read the shared string table substream (requires finalized styles buffer)
    OUString aSstFragmentPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "sharedStrings" ) );
    if( !aSstFragmentPath.isEmpty() )
        importOoxFragment( new SharedStringsFragment( *this, aSstFragmentPath ) );
    xGlobalSegment->setPosition( 0.75 );

    // read the connections substream
    OUString aConnFragmentPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "connections" ) );
    if( !aConnFragmentPath.isEmpty() )
        importOoxFragment( new ConnectionsFragment( *this, aConnFragmentPath ) );
    xGlobalSegment->setPosition( 1.0 );


    /*  Create fragments for all sheets, before importing them. Needed to do
        some preprocessing in the fragment constructors, e.g. loading the table
        fragments for all sheets that are needed before the cell formulas are
        loaded. Additionally, the instances of the WorkbookGlobals structures
        have to be stored for every sheet. */
    typedef ::std::pair< WorksheetGlobalsRef, FragmentHandlerRef > SheetFragmentHandler;
    typedef ::std::vector< SheetFragmentHandler > SheetFragmentVector;
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
            OSL_ENSURE( !aFragmentPath.isEmpty(), "WorkbookFragment::finalizeImport - cannot access sheet fragment" );
            if( !aFragmentPath.isEmpty() )
            {
                // leave space for formula processing ( calcuate the segments as
                // if there is an extra sheet )
                double fSegmentLength = getProgressBar().getFreeLength() / (nWorksheetCount - ( nWorksheet - 1) );
                ISegmentProgressBarRef xSheetSegment = getProgressBar().createSegment( fSegmentLength );

                // get the sheet type according to the relations type
                WorksheetType eSheetType = SHEETTYPE_EMPTYSHEET;
                if( pRelation->maType == CREATE_OFFICEDOC_RELATION_TYPE( "worksheet" ) )
                    eSheetType = SHEETTYPE_WORKSHEET;
                else if( pRelation->maType == CREATE_OFFICEDOC_RELATION_TYPE( "chartsheet" ) )
                    eSheetType = SHEETTYPE_CHARTSHEET;
                else if( (pRelation->maType == CREATE_MSOFFICE_RELATION_TYPE( "xlMacrosheet" )) ||
                         (pRelation->maType == CREATE_MSOFFICE_RELATION_TYPE( "xlIntlMacrosheet" )) )
                    eSheetType = SHEETTYPE_MACROSHEET;
                else if( pRelation->maType == CREATE_OFFICEDOC_RELATION_TYPE( "dialogsheet" ) )
                    eSheetType = SHEETTYPE_DIALOGSHEET;
                OSL_ENSURE( eSheetType != SHEETTYPE_EMPTYSHEET, "WorkbookFragment::finalizeImport - unknown sheet type" );
                if( eSheetType != SHEETTYPE_EMPTYSHEET )
                {
                    // create the WorksheetGlobals object
                    WorksheetGlobalsRef xSheetGlob = WorksheetHelper::constructGlobals( *this, xSheetSegment, eSheetType, nCalcSheet );
                    OSL_ENSURE( xSheetGlob.get(), "WorkbookFragment::finalizeImport - missing sheet in document" );
                    if( xSheetGlob.get() )
                    {
                        // create the sheet fragment handler
                        ::rtl::Reference< WorksheetFragmentBase > xFragment;
                        switch( eSheetType )
                        {
                            case SHEETTYPE_WORKSHEET:
                            case SHEETTYPE_MACROSHEET:
                            case SHEETTYPE_DIALOGSHEET:
                                xFragment.set( new WorksheetFragment( *xSheetGlob, aFragmentPath ) );
                            break;
                            case SHEETTYPE_CHARTSHEET:
                                xFragment.set( new ChartsheetFragment( *xSheetGlob, aFragmentPath ) );
                            break;
                            default:
                                OSL_ENSURE( false, "WorkbookFragment::finalizeImport - unexpected sheet type" );
                        }

                        // insert the fragment into the map
                        if( xFragment.is() )
                            aSheetFragments.push_back( SheetFragmentHandler( xSheetGlob, xFragment.get() ) );
                    }
                }
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
        importOoxFragment( aIt->second );
        // delete fragment object and WorkbookGlobals object, will free all allocated sheet buffers
        aIt->second.clear();
        aIt->first.reset();
    }

    // open the VBA project storage
    OUString aVbaFragmentPath = getFragmentPathFromFirstType( CREATE_MSOFFICE_RELATION_TYPE( "vbaProject" ) );
    if( !aVbaFragmentPath.isEmpty() )
    {
        Reference< XInputStream > xInStrm = getBaseFilter().openInputStream( aVbaFragmentPath );
        if( xInStrm.is() )
            setVbaProjectStorage( StorageRef( new ::oox::ole::OleStorage( getBaseFilter().getComponentContext(), xInStrm, false ) ) );
    }

    // final conversions, e.g. calculation settings and view settings
    finalizeWorkbookImport();

    // Recalculate (only changed ones)
    Reference< XCalculatable > xCalculatable( getDocument(), UNO_QUERY );
    if( xCalculatable.is() )
        xCalculatable->calculate();
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
    if( !aFragmentPath.isEmpty() )
        importOoxFragment( new ExternalLinkFragment( *this, aFragmentPath, rExtLink ) );
}

void WorkbookFragment::importPivotCacheDefFragment( const OUString& rRelId, sal_Int32 nCacheId )
{
    // pivot caches will be imported on demand, here we just store the fragment path in the buffer
    getPivotCaches().registerPivotCacheFragment( nCacheId, getFragmentPathFromRelId( rRelId ) );
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
