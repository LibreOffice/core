/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chartsheetfragment.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:57:17 $
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

#include "oox/xls/chartsheetfragment.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/pagesettings.hxx"
#include "oox/xls/viewsettings.hxx"
#include "oox/xls/workbooksettings.hxx"
#include "oox/xls/worksheetsettings.hxx"

using ::rtl::OUString;
using ::oox::core::RecordInfo;

namespace oox {
namespace xls {

// ============================================================================

OoxChartsheetFragment::OoxChartsheetFragment( const WorkbookHelper& rHelper,
        const OUString& rFragmentPath, ISegmentProgressBarRef xProgressBar, sal_Int32 nSheet ) :
    OoxWorksheetFragmentBase( rHelper, rFragmentPath, xProgressBar, SHEETTYPE_CHARTSHEET, nSheet )
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper OoxChartsheetFragment::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return  (nElement == XLS_TOKEN( chartsheet ));
        case XLS_TOKEN( chartsheet ):
            return  (nElement == XLS_TOKEN( sheetPr )) ||
                    (nElement == XLS_TOKEN( sheetProtection )) ||
                    (nElement == XLS_TOKEN( sheetViews )) ||
                    (nElement == XLS_TOKEN( pageMargins )) ||
                    (nElement == XLS_TOKEN( pageSetup )) ||
                    (nElement == XLS_TOKEN( headerFooter )) ||
                    (nElement == XLS_TOKEN( picture )) ||
                    (nElement == XLS_TOKEN( drawing ));
        case XLS_TOKEN( sheetViews ):
            return  (nElement == XLS_TOKEN( sheetView ));
        case XLS_TOKEN( headerFooter ):
            return  (nElement == XLS_TOKEN( firstHeader )) ||
                    (nElement == XLS_TOKEN( firstFooter )) ||
                    (nElement == XLS_TOKEN( oddHeader )) ||
                    (nElement == XLS_TOKEN( oddFooter )) ||
                    (nElement == XLS_TOKEN( evenHeader )) ||
                    (nElement == XLS_TOKEN( evenFooter ));
    }
    return false;
}

void OoxChartsheetFragment::onStartElement( const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( sheetPr ):          getWorksheetSettings().importChartSheetPr( rAttribs );              break;
        case XLS_TOKEN( sheetProtection ):  getWorksheetSettings().importChartProtection( rAttribs );           break;
        case XLS_TOKEN( sheetView ):        getSheetViewSettings().importChartSheetView( rAttribs );            break;
        case XLS_TOKEN( pageMargins ):      getPageSettings().importPageMargins( rAttribs );                    break;
        case XLS_TOKEN( pageSetup ):        getPageSettings().importChartPageSetup( getRelations(), rAttribs ); break;
        case XLS_TOKEN( headerFooter ):     getPageSettings().importHeaderFooter( rAttribs );                   break;
        case XLS_TOKEN( picture ):          getPageSettings().importPicture( getRelations(), rAttribs );        break;
        case XLS_TOKEN( drawing ):          importDrawing( rAttribs );                                          break;
    }
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

ContextWrapper OoxChartsheetFragment::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return  (nRecId == OOBIN_ID_WORKSHEET);
        case OOBIN_ID_WORKSHEET:
            return  (nRecId == OOBIN_ID_CHARTSHEETPR) ||
                    (nRecId == OOBIN_ID_CHARTPROTECTION) ||
                    (nRecId == OOBIN_ID_CHARTSHEETVIEWS) ||
                    (nRecId == OOBIN_ID_PAGEMARGINS) ||
                    (nRecId == OOBIN_ID_CHARTPAGESETUP) ||
                    (nRecId == OOBIN_ID_HEADERFOOTER) ||
                    (nRecId == OOBIN_ID_PICTURE) ||
                    (nRecId == OOBIN_ID_DRAWING);
        case OOBIN_ID_CHARTSHEETVIEWS:
            return  (nRecId == OOBIN_ID_CHARTSHEETVIEW);
    }
    return false;
}

void OoxChartsheetFragment::onStartRecord( RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case OOBIN_ID_CHARTSHEETPR:     getWorksheetSettings().importChartSheetPr( rStrm );                 break;
        case OOBIN_ID_CHARTPROTECTION:  getWorksheetSettings().importChartProtection( rStrm );              break;
        case OOBIN_ID_CHARTSHEETVIEW:   getSheetViewSettings().importChartSheetView( rStrm );               break;
        case OOBIN_ID_PAGEMARGINS:      getPageSettings().importPageMargins( rStrm );                       break;
        case OOBIN_ID_CHARTPAGESETUP:   getPageSettings().importChartPageSetup( getRelations(), rStrm );    break;
        case OOBIN_ID_HEADERFOOTER:     getPageSettings().importHeaderFooter( rStrm );                      break;
        case OOBIN_ID_PICTURE:          getPageSettings().importPicture( getRelations(), rStrm );           break;
        case OOBIN_ID_DRAWING:          importDrawing( rStrm );                                             break;
    }
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
    setDrawingPath( getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ) ) ) );
}

void OoxChartsheetFragment::importDrawing( RecordInputStream& rStrm )
{
    setDrawingPath( getFragmentPathFromRelId( rStrm.readString() ) );
}

// ============================================================================

BiffChartsheetFragment::BiffChartsheetFragment( const WorkbookHelper& rHelper, ISegmentProgressBarRef xProgressBar, sal_Int32 nSheet ) :
    BiffWorksheetFragmentBase( rHelper, xProgressBar, SHEETTYPE_CHARTSHEET, nSheet )
{
}

bool BiffChartsheetFragment::importFragment( BiffInputStream& rStrm )
{
    // initial processing in base class WorksheetHelper
    initializeWorksheetImport();

    WorksheetSettings& rWorksheetSett = getWorksheetSettings();
    SheetViewSettings& rSheetViewSett = getSheetViewSettings();
    PageSettings& rPageSett           = getPageSettings();

    // process all record in this sheet fragment
    while( rStrm.startNextRecord() && (rStrm.getRecId() != BIFF_ID_EOF) )
    {
        sal_uInt16 nRecId = rStrm.getRecId();

        if( isBofRecord( nRecId ) )
        {
            // skip unknown embedded fragments (BOF/EOF blocks)
            skipFragment( rStrm );
        }
        else switch( nRecId )
        {
            // records in all BIFF versions
            case BIFF_ID_BOTTOMMARGIN:  rPageSett.importBottomMargin( rStrm );  break;
            case BIFF_ID_FOOTER:        rPageSett.importFooter( rStrm );        break;
            case BIFF_ID_HEADER:        rPageSett.importHeader( rStrm );        break;
            case BIFF_ID_LEFTMARGIN:    rPageSett.importLeftMargin( rStrm );    break;
            case BIFF_ID_PASSWORD:      rWorksheetSett.importPassword( rStrm ); break;
            case BIFF_ID_PROTECT:       rWorksheetSett.importProtect( rStrm );  break;
            case BIFF_ID_RIGHTMARGIN:   rPageSett.importRightMargin( rStrm );   break;
            case BIFF_ID_TOPMARGIN:     rPageSett.importTopMargin( rStrm );     break;

            // BIFF specific records
            default: switch( getBiff() )
            {
                case BIFF2: switch( nRecId )
                {
                    case BIFF2_ID_WINDOW2:      rSheetViewSett.importWindow2( rStrm );  break;
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
                    case BIFF_ID_HCENTER:       rPageSett.importHorCenter( rStrm );             break;
                    case BIFF_ID_OBJECTPROTECT: rWorksheetSett.importObjectProtect( rStrm );    break;
                    case BIFF_ID_PICTURE:       rPageSett.importPicture( rStrm );               break;
                    case BIFF_ID_PAGESETUP:     rPageSett.importPageSetup( rStrm );             break;
                    case BIFF_ID_VCENTER:       rPageSett.importVerCenter( rStrm );             break;
                    case BIFF3_ID_WINDOW2:      rSheetViewSett.importWindow2( rStrm );          break;
                }
                break;

                case BIFF_UNKNOWN: break;
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

