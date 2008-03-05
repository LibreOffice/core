/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bifffragmenthandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:56:45 $
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

#include "oox/xls/bifffragmenthandler.hxx"
#include "oox/xls/biffhelper.hxx"
#include "oox/xls/biffinputstream.hxx"

namespace oox {
namespace xls {

// ============================================================================

namespace {

const sal_uInt16 BIFF_BOF_GLOBALS           = 0x0005;   /// BIFF5-BIFF8 workbook globals.
const sal_uInt16 BIFF_BOF_MODULE            = 0x0006;   /// BIFF5-BIFF8 Visual Basic module.
const sal_uInt16 BIFF_BOF_SHEET             = 0x0010;   /// BIFF2-BIFF8 worksheet/dialog sheet.
const sal_uInt16 BIFF_BOF_CHART             = 0x0020;   /// BIFF2-BIFF8 chart sheet.
const sal_uInt16 BIFF_BOF_MACRO             = 0x0040;   /// BIFF4-BIFF8 macro sheet.
const sal_uInt16 BIFF_BOF_WORKSPACE         = 0x0100;   /// BIFF3-BIFF8 workspace.

} // namespace

// ============================================================================

BiffFragmentHandler::~BiffFragmentHandler()
{
}

bool BiffFragmentHandler::importFragment( BiffInputStream& rStrm )
{
    // default implementation: skip the entire fragment
    return skipFragment( rStrm );
}

bool BiffFragmentHandler::isBofRecord( sal_uInt16 nRecId )
{
    return (nRecId == BIFF2_ID_BOF) || (nRecId == BIFF3_ID_BOF) || (nRecId == BIFF4_ID_BOF) || (nRecId == BIFF5_ID_BOF);
}

BiffFragmentType BiffFragmentHandler::startFragment( BiffInputStream& rStrm, BiffType eBiff )
{
    BiffFragmentType eFragment = BIFF_FRAGMENT_UNKNOWN;
    if( rStrm.startNextRecord() )
    {
        /*  #i23425# Don't rely on BOF record ID to read BOF contents, but on
            the detected BIFF version. */
        if( isBofRecord( rStrm.getRecId() ) )
        {
            // BOF is always written unencrypted
            rStrm.enableDecoder( false );
            sal_uInt16 nType = rStrm.skip( 2 ).readuInt16();

            // decide which fragment types are valid for current BIFF version
            switch( eBiff )
            {
                case BIFF2: switch( nType )
                {
                    case BIFF_BOF_CHART:    eFragment = BIFF_FRAGMENT_EMPTYSHEET;   break;
                    case BIFF_BOF_MACRO:    eFragment = BIFF_FRAGMENT_MACROSHEET;   break;
                    // #i51490# Excel interprets invalid types as worksheet
                    default:                eFragment = BIFF_FRAGMENT_WORKSHEET;
                }
                break;

                case BIFF3: switch( nType )
                {
                    case BIFF_BOF_CHART:    eFragment = BIFF_FRAGMENT_EMPTYSHEET;   break;
                    case BIFF_BOF_MACRO:    eFragment = BIFF_FRAGMENT_MACROSHEET;   break;
                    case BIFF_BOF_WORKSPACE:eFragment = BIFF_FRAGMENT_UNKNOWN;      break;
                    // #i51490# Excel interprets invalid types as worksheet
                    default:                eFragment = BIFF_FRAGMENT_WORKSHEET;
                };
                break;

                case BIFF4: switch( nType )
                {
                    case BIFF_BOF_CHART:    eFragment = BIFF_FRAGMENT_EMPTYSHEET;   break;
                    case BIFF_BOF_MACRO:    eFragment = BIFF_FRAGMENT_MACROSHEET;   break;
                    case BIFF_BOF_WORKSPACE:eFragment = BIFF_FRAGMENT_WORKSPACE;    break;
                    // #i51490# Excel interprets invalid types as worksheet
                    default:                eFragment = BIFF_FRAGMENT_WORKSHEET;
                };
                break;

                case BIFF5:
                case BIFF8: switch( nType )
                {
                    case BIFF_BOF_GLOBALS:  eFragment = BIFF_FRAGMENT_GLOBALS;      break;
                    case BIFF_BOF_CHART:    eFragment = BIFF_FRAGMENT_CHARTSHEET;   break;
                    case BIFF_BOF_MACRO:    eFragment = BIFF_FRAGMENT_MACROSHEET;   break;
                    case BIFF_BOF_MODULE:   eFragment = BIFF_FRAGMENT_MODULESHEET;  break;
                    case BIFF_BOF_WORKSPACE:eFragment = BIFF_FRAGMENT_UNKNOWN;      break;
                    // #i51490# Excel interprets invalid types as worksheet
                    default:                eFragment = BIFF_FRAGMENT_WORKSHEET;
                };
                break;

                case BIFF_UNKNOWN: break;
            }
        }
    }
    return eFragment;
}

bool BiffFragmentHandler::skipFragment( BiffInputStream& rStrm )
{
    while( rStrm.startNextRecord() && (rStrm.getRecId() != BIFF_ID_EOF) )
        if( isBofRecord( rStrm.getRecId() ) )
            skipFragment( rStrm );
    return rStrm.isValid() && (rStrm.getRecId() == BIFF_ID_EOF);
}

// ============================================================================

BiffWorkbookFragmentBase::BiffWorkbookFragmentBase( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

// ============================================================================

BiffWorksheetFragmentBase::BiffWorksheetFragmentBase( const WorkbookHelper& rHelper,
        ISegmentProgressBarRef xProgressBar, WorksheetType eSheetType, sal_Int32 nSheet ) :
    WorksheetHelperRoot( rHelper, xProgressBar, eSheetType, nSheet )
{
}

BiffWorksheetFragmentBase::BiffWorksheetFragmentBase( const WorksheetHelper& rHelper ) :
    WorksheetHelperRoot( rHelper )
{
}

// ============================================================================

} // namespace xls
} // namespace oox

