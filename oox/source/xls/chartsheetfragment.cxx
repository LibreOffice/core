/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: chartsheetfragment.cxx,v $
 * $Revision: 1.4.4.2 $
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
#include "oox/helper/recordinputstream.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/pagesettings.hxx"
#include "oox/xls/viewsettings.hxx"
#include "oox/xls/workbooksettings.hxx"
#include "oox/xls/worksheetsettings.hxx"

using ::rtl::OUString;
using ::oox::core::ContextHandlerRef;
using ::oox::core::RecordInfo;

namespace oox {
namespace xls {

// ============================================================================

OoxChartsheetFragment::OoxChartsheetFragment( const WorkbookHelper& rHelper,
        const OUString& rFragmentPath, ISegmentProgressBarRef xProgressBar, sal_Int16 nSheet ) :
    OoxWorksheetFragmentBase( rHelper, rFragmentPath, xProgressBar, SHEETTYPE_CHARTSHEET, nSheet )
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextHandlerRef OoxChartsheetFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
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
                case XLS_TOKEN( evenFooter ):       return this;    // collect contents in onEndElement()
            }
        break;
    }
    return 0;
}

void OoxChartsheetFragment::onEndElement( const OUString& rChars )
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

ContextHandlerRef OoxChartsheetFragment::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == OOBIN_ID_WORKSHEET ) return this;
        break;

        case OOBIN_ID_WORKSHEET:
            switch( nRecId )
            {
                case OOBIN_ID_CHARTSHEETVIEWS:  return this;

                case OOBIN_ID_CHARTSHEETPR:     getWorksheetSettings().importChartSheetPr( rStrm );                 break;
                case OOBIN_ID_CHARTPROTECTION:  getWorksheetSettings().importChartProtection( rStrm );              break;
                case OOBIN_ID_PAGEMARGINS:      getPageSettings().importPageMargins( rStrm );                       break;
                case OOBIN_ID_CHARTPAGESETUP:   getPageSettings().importChartPageSetup( getRelations(), rStrm );    break;
                case OOBIN_ID_HEADERFOOTER:     getPageSettings().importHeaderFooter( rStrm );                      break;
                case OOBIN_ID_PICTURE:          getPageSettings().importPicture( getRelations(), rStrm );           break;
                case OOBIN_ID_DRAWING:          importDrawing( rStrm );                                             break;
            }
        break;

        case OOBIN_ID_CHARTSHEETVIEWS:
            if( nRecId == OOBIN_ID_CHARTSHEETVIEW ) getSheetViewSettings().importChartSheetView( rStrm );
        break;
    }
    return 0;
}

// oox.core.FragmentHandler2 interface ----------------------------------------

const RecordInfo* OoxChartsheetFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { OOBIN_ID_CHARTSHEETVIEW,      OOBIN_ID_CHARTSHEETVIEW + 1     },
        { OOBIN_ID_CHARTSHEETVIEWS,     OOBIN_ID_CHARTSHEETVIEWS + 1    },
        { OOBIN_ID_CUSTOMCHARTVIEW,     OOBIN_ID_CUSTOMCHARTVIEW + 1    },
        { OOBIN_ID_CUSTOMCHARTVIEWS,    OOBIN_ID_CUSTOMCHARTVIEWS + 1   },
        { OOBIN_ID_HEADERFOOTER,        OOBIN_ID_HEADERFOOTER + 1       },
        { OOBIN_ID_WORKSHEET,           OOBIN_ID_WORKSHEET + 1          },
        { -1,                           -1                              }
    };
    return spRecInfos;
}

void OoxChartsheetFragment::initializeImport()
{
    // initial processing in base class WorksheetHelper
    initializeWorksheetImport();
}

void OoxChartsheetFragment::finalizeImport()
{
    // final processing in base class WorksheetHelper
    finalizeWorksheetImport();
}

// private --------------------------------------------------------------------

void OoxChartsheetFragment::importDrawing( const AttributeList& rAttribs )
{
    setDrawingPath( getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) ) );
}

void OoxChartsheetFragment::importDrawing( RecordInputStream& rStrm )
{
    setDrawingPath( getFragmentPathFromRelId( rStrm.readString() ) );
}

// ============================================================================

BiffChartsheetFragment::BiffChartsheetFragment( const BiffWorkbookFragmentBase& rParent,
        ISegmentProgressBarRef xProgressBar, sal_Int16 nSheet ) :
    BiffWorksheetFragmentBase( rParent, xProgressBar, SHEETTYPE_CHARTSHEET, nSheet )
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
    while( mrStrm.startNextRecord() && (mrStrm.getRecId() != BIFF_ID_EOF) )
    {
        if( isBofRecord() )
        {
            // skip unknown embedded fragments (BOF/EOF blocks)
            skipFragment();
        }
        else
        {
            sal_uInt16 nRecId = mrStrm.getRecId();
            switch( nRecId )
            {
                // records in all BIFF versions
                case BIFF_ID_BOTTOMMARGIN:  rPageSett.importBottomMargin( mrStrm );     break;
                case BIFF_ID_CHBEGIN:       skipRecordBlock( BIFF_ID_CHEND );           break;
                case BIFF_ID_FOOTER:        rPageSett.importFooter( mrStrm );           break;
                case BIFF_ID_HEADER:        rPageSett.importHeader( mrStrm );           break;
                case BIFF_ID_LEFTMARGIN:    rPageSett.importLeftMargin( mrStrm );       break;
                case BIFF_ID_PASSWORD:      rWorksheetSett.importPassword( mrStrm );    break;
                case BIFF_ID_PROTECT:       rWorksheetSett.importProtect( mrStrm );     break;
                case BIFF_ID_RIGHTMARGIN:   rPageSett.importRightMargin( mrStrm );      break;
                case BIFF_ID_TOPMARGIN:     rPageSett.importTopMargin( mrStrm );        break;

                // BIFF specific records
                default: switch( getBiff() )
                {
                    case BIFF2: switch( nRecId )
                    {
                        case BIFF2_ID_WINDOW2:      rSheetViewSett.importWindow2( mrStrm );  break;
                    }
                    break;

                    case BIFF3: switch( nRecId )
                    {
                        case BIFF_ID_HCENTER:       rPageSett.importHorCenter( mrStrm );            break;
                        case BIFF_ID_OBJECTPROTECT: rWorksheetSett.importObjectProtect( mrStrm );   break;
                        case BIFF_ID_VCENTER:       rPageSett.importVerCenter( mrStrm );            break;
                        case BIFF3_ID_WINDOW2:      rSheetViewSett.importWindow2( mrStrm );         break;

                    }
                    break;

                    case BIFF4: switch( nRecId )
                    {
                        case BIFF_ID_HCENTER:       rPageSett.importHorCenter( mrStrm );            break;
                        case BIFF_ID_OBJECTPROTECT: rWorksheetSett.importObjectProtect( mrStrm );   break;
                        case BIFF_ID_PAGESETUP:     rPageSett.importPageSetup( mrStrm );            break;
                        case BIFF_ID_VCENTER:       rPageSett.importVerCenter( mrStrm );            break;
                        case BIFF3_ID_WINDOW2:      rSheetViewSett.importWindow2( mrStrm );         break;
                    }
                    break;

                    case BIFF5: switch( nRecId )
                    {
                        case BIFF_ID_HCENTER:       rPageSett.importHorCenter( mrStrm );            break;
                        case BIFF_ID_OBJECTPROTECT: rWorksheetSett.importObjectProtect( mrStrm );   break;
                        case BIFF_ID_PAGESETUP:     rPageSett.importPageSetup( mrStrm );            break;
                        case BIFF_ID_SCENPROTECT:   rWorksheetSett.importScenProtect( mrStrm );     break;
                        case BIFF_ID_SCL:           rSheetViewSett.importScl( mrStrm );             break;
                        case BIFF_ID_VCENTER:       rPageSett.importVerCenter( mrStrm );            break;
                        case BIFF3_ID_WINDOW2:      rSheetViewSett.importWindow2( mrStrm );         break;
                    }
                    break;

                    case BIFF8: switch( nRecId )
                    {
                        case BIFF_ID_HCENTER:       rPageSett.importHorCenter( mrStrm );            break;
                        case BIFF_ID_OBJECTPROTECT: rWorksheetSett.importObjectProtect( mrStrm );   break;
                        case BIFF_ID_PICTURE:       rPageSett.importPicture( mrStrm );              break;
                        case BIFF_ID_PAGESETUP:     rPageSett.importPageSetup( mrStrm );            break;
                        case BIFF_ID_SCL:           rSheetViewSett.importScl( mrStrm );             break;
                        case BIFF_ID_VCENTER:       rPageSett.importVerCenter( mrStrm );            break;
                        case BIFF3_ID_WINDOW2:      rSheetViewSett.importWindow2( mrStrm );         break;
                    }
                    break;

                    case BIFF_UNKNOWN: break;
                }
            }
        }
    }

    // final processing in base class WorksheetHelper
    finalizeWorksheetImport();
    return mrStrm.getRecId() == BIFF_ID_EOF;
}

// ============================================================================

} // namespace xls
} // namespace oox

