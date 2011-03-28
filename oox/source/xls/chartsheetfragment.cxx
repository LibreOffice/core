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

#include "oox/xls/chartsheetfragment.hxx"

#include "oox/helper/attributelist.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/pagesettings.hxx"
#include "oox/xls/viewsettings.hxx"
#include "oox/xls/workbooksettings.hxx"
#include "oox/xls/worksheetsettings.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::oox::core;

using ::rtl::OUString;

// ============================================================================

ChartsheetFragment::ChartsheetFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    WorksheetFragmentBase( rHelper, rFragmentPath )
{
}

ContextHandlerRef ChartsheetFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( chartsheet ) ) return this;
        break;

        case XLS_TOKEN( chartsheet ):
            switch( nElement )
            {
                case XLS_TOKEN( sheetViews ):       return this;

                case XLS_TOKEN( sheetPr ):          getWorksheetSettings().importChartSheetPr( rAttribs );              break;
                case XLS_TOKEN( sheetProtection ):  getWorksheetSettings().importChartProtection( rAttribs );           break;
                case XLS_TOKEN( pageMargins ):      getPageSettings().importPageMargins( rAttribs );                    break;
                case XLS_TOKEN( pageSetup ):        getPageSettings().importChartPageSetup( getRelations(), rAttribs ); break;
                case XLS_TOKEN( headerFooter ):     getPageSettings().importHeaderFooter( rAttribs );                   return this;
                case XLS_TOKEN( picture ):          getPageSettings().importPicture( getRelations(), rAttribs );        break;
                case XLS_TOKEN( drawing ):          importDrawing( rAttribs );                                          break;
            }
        break;

        case XLS_TOKEN( sheetViews ):
            if( nElement == XLS_TOKEN( sheetView ) ) getSheetViewSettings().importChartSheetView( rAttribs );
        break;

        case XLS_TOKEN( headerFooter ):
            switch( nElement )
            {
                case XLS_TOKEN( firstHeader ):
                case XLS_TOKEN( firstFooter ):
                case XLS_TOKEN( oddHeader ):
                case XLS_TOKEN( oddFooter ):
                case XLS_TOKEN( evenHeader ):
                case XLS_TOKEN( evenFooter ):       return this;    // collect contents in onCharacters()
            }
        break;
    }
    return 0;
}

void ChartsheetFragment::onCharacters( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( firstHeader ):
        case XLS_TOKEN( firstFooter ):
        case XLS_TOKEN( oddHeader ):
        case XLS_TOKEN( oddFooter ):
        case XLS_TOKEN( evenHeader ):
        case XLS_TOKEN( evenFooter ):
            getPageSettings().importHeaderFooterCharacters( rChars, getCurrentElement() );
        break;
    }
}

ContextHandlerRef ChartsheetFragment::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == BIFF12_ID_WORKSHEET ) return this;
        break;

        case BIFF12_ID_WORKSHEET:
            switch( nRecId )
            {
                case BIFF12_ID_CHARTSHEETVIEWS:  return this;

                case BIFF12_ID_CHARTSHEETPR:    getWorksheetSettings().importChartSheetPr( rStrm );                 break;
                case BIFF12_ID_CHARTPROTECTION: getWorksheetSettings().importChartProtection( rStrm );              break;
                case BIFF12_ID_PAGEMARGINS:     getPageSettings().importPageMargins( rStrm );                       break;
                case BIFF12_ID_CHARTPAGESETUP:  getPageSettings().importChartPageSetup( getRelations(), rStrm );    break;
                case BIFF12_ID_HEADERFOOTER:    getPageSettings().importHeaderFooter( rStrm );                      break;
                case BIFF12_ID_PICTURE:         getPageSettings().importPicture( getRelations(), rStrm );           break;
                case BIFF12_ID_DRAWING:         importDrawing( rStrm );                                             break;
            }
        break;

        case BIFF12_ID_CHARTSHEETVIEWS:
            if( nRecId == BIFF12_ID_CHARTSHEETVIEW ) getSheetViewSettings().importChartSheetView( rStrm );
        break;
    }
    return 0;
}

const RecordInfo* ChartsheetFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { BIFF12_ID_CHARTSHEETVIEW,     BIFF12_ID_CHARTSHEETVIEW + 1    },
        { BIFF12_ID_CHARTSHEETVIEWS,    BIFF12_ID_CHARTSHEETVIEWS + 1   },
        { BIFF12_ID_CUSTOMCHARTVIEW,    BIFF12_ID_CUSTOMCHARTVIEW + 1   },
        { BIFF12_ID_CUSTOMCHARTVIEWS,   BIFF12_ID_CUSTOMCHARTVIEWS + 1  },
        { BIFF12_ID_HEADERFOOTER,       BIFF12_ID_HEADERFOOTER + 1      },
        { BIFF12_ID_WORKSHEET,          BIFF12_ID_WORKSHEET + 1         },
        { -1,                           -1                              }
    };
    return spRecInfos;
}

void ChartsheetFragment::initializeImport()
{
    // initial processing in base class WorksheetHelper
    initializeWorksheetImport();
}

void ChartsheetFragment::finalizeImport()
{
    // final processing in base class WorksheetHelper
    finalizeWorksheetImport();
}

// private --------------------------------------------------------------------

void ChartsheetFragment::importDrawing( const AttributeList& rAttribs )
{
    setDrawingPath( getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) ) );
}

void ChartsheetFragment::importDrawing( SequenceInputStream& rStrm )
{
    setDrawingPath( getFragmentPathFromRelId( BiffHelper::readString( rStrm ) ) );
}

// ============================================================================

BiffChartsheetFragment::BiffChartsheetFragment( const WorksheetHelper& rHelper, const BiffWorkbookFragmentBase& rParent ) :
    BiffWorksheetFragmentBase( rHelper, rParent )
{
}

bool BiffChartsheetFragment::importFragment()
{
    // initial processing in base class WorksheetHelper
    initializeWorksheetImport();

    WorksheetSettings& rWorksheetSett = getWorksheetSettings();
    SheetViewSettings& rSheetViewSett = getSheetViewSettings();
    PageSettings& rPageSett           = getPageSettings();

    // process all record in this sheet fragment
    BiffInputStream& rStrm = getInputStream();
    while( rStrm.startNextRecord() && (rStrm.getRecId() != BIFF_ID_EOF) )
    {
        if( BiffHelper::isBofRecord( rStrm ) )
        {
            // skip unknown embedded fragments (BOF/EOF blocks)
            skipFragment();
        }
        else
        {
            sal_uInt16 nRecId = rStrm.getRecId();
            switch( nRecId )
            {
                // records in all BIFF versions
                case BIFF_ID_BOTTOMMARGIN:  rPageSett.importBottomMargin( rStrm );                  break;
                case BIFF_ID_CHBEGIN:       BiffHelper::skipRecordBlock( rStrm, BIFF_ID_CHEND );    break;
                case BIFF_ID_FOOTER:        rPageSett.importFooter( rStrm );                        break;
                case BIFF_ID_HEADER:        rPageSett.importHeader( rStrm );                        break;
                case BIFF_ID_LEFTMARGIN:    rPageSett.importLeftMargin( rStrm );                    break;
                case BIFF_ID_PASSWORD:      rWorksheetSett.importPassword( rStrm );                 break;
                case BIFF_ID_PROTECT:       rWorksheetSett.importProtect( rStrm );                  break;
                case BIFF_ID_RIGHTMARGIN:   rPageSett.importRightMargin( rStrm );                   break;
                case BIFF_ID_TOPMARGIN:     rPageSett.importTopMargin( rStrm );                     break;

                // BIFF specific records
                default: switch( getBiff() )
                {
                    case BIFF2: switch( nRecId )
                    {
                        case BIFF2_ID_WINDOW2:      rSheetViewSett.importWindow2( rStrm );          break;
                    }
                    break;

                    case BIFF3: switch( nRecId )
                    {
                        case BIFF_ID_HCENTER:       rPageSett.importHorCenter( rStrm );             break;
                        case BIFF_ID_OBJECTPROTECT: rWorksheetSett.importObjectProtect( rStrm );    break;
                        case BIFF_ID_VCENTER:       rPageSett.importVerCenter( rStrm );             break;
                        case BIFF3_ID_WINDOW2:      rSheetViewSett.importWindow2( rStrm );          break;

                    }
                    break;

                    case BIFF4: switch( nRecId )
                    {
                        case BIFF_ID_HCENTER:       rPageSett.importHorCenter( rStrm );             break;
                        case BIFF_ID_OBJECTPROTECT: rWorksheetSett.importObjectProtect( rStrm );    break;
                        case BIFF_ID_PAGESETUP:     rPageSett.importPageSetup( rStrm );             break;
                        case BIFF_ID_VCENTER:       rPageSett.importVerCenter( rStrm );             break;
                        case BIFF3_ID_WINDOW2:      rSheetViewSett.importWindow2( rStrm );          break;
                    }
                    break;

                    case BIFF5: switch( nRecId )
                    {
                        case BIFF_ID_HCENTER:       rPageSett.importHorCenter( rStrm );             break;
                        case BIFF_ID_OBJECTPROTECT: rWorksheetSett.importObjectProtect( rStrm );    break;
                        case BIFF_ID_PAGESETUP:     rPageSett.importPageSetup( rStrm );             break;
                        case BIFF_ID_SCENPROTECT:   rWorksheetSett.importScenProtect( rStrm );      break;
                        case BIFF_ID_SCL:           rSheetViewSett.importScl( rStrm );              break;
                        case BIFF_ID_VCENTER:       rPageSett.importVerCenter( rStrm );             break;
                        case BIFF3_ID_WINDOW2:      rSheetViewSett.importWindow2( rStrm );          break;
                    }
                    break;

                    case BIFF8: switch( nRecId )
                    {
                        case BIFF_ID_CODENAME:      rWorksheetSett.importCodeName( rStrm );         break;
                        case BIFF_ID_HCENTER:       rPageSett.importHorCenter( rStrm );             break;
                        case BIFF_ID_OBJECTPROTECT: rWorksheetSett.importObjectProtect( rStrm );    break;
                        case BIFF_ID_PICTURE:       rPageSett.importPicture( rStrm );               break;
                        case BIFF_ID_PAGESETUP:     rPageSett.importPageSetup( rStrm );             break;
                        case BIFF_ID_SCL:           rSheetViewSett.importScl( rStrm );              break;
                        case BIFF_ID_SHEETEXT:      rWorksheetSett.importSheetExt( rStrm );         break;
                        case BIFF_ID_VCENTER:       rPageSett.importVerCenter( rStrm );             break;
                        case BIFF3_ID_WINDOW2:      rSheetViewSett.importWindow2( rStrm );          break;
                    }
                    break;

                    case BIFF_UNKNOWN: break;
                }
            }
        }
    }

    // final processing in base class WorksheetHelper
    finalizeWorksheetImport();
    return rStrm.getRecId() == BIFF_ID_EOF;
}

// ============================================================================

} // namespace xls
} // namespace oox
