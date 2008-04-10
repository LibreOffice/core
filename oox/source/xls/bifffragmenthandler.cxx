/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bifffragmenthandler.cxx,v $
 * $Revision: 1.4 $
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

