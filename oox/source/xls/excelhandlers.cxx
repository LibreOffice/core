/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: excelhandlers.cxx,v $
 * $Revision: 1.3.20.2 $
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

#include "oox/xls/excelhandlers.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/xls/biffinputstream.hxx"

using ::rtl::OUString;
using ::oox::core::FilterBase;
using ::oox::core::FragmentHandler2;

namespace oox {
namespace xls {

// ============================================================================
// ============================================================================

OoxWorkbookFragmentBase::OoxWorkbookFragmentBase(
        const WorkbookHelper& rHelper, const OUString& rFragmentPath ) :
    FragmentHandler2( rHelper.getOoxFilter(), rFragmentPath ),
    WorkbookHelper( rHelper )
{
}

// ============================================================================

OoxWorksheetFragmentBase::OoxWorksheetFragmentBase( const WorkbookHelper& rHelper,
        const OUString& rFragmentPath, ISegmentProgressBarRef xProgressBar, WorksheetType eSheetType, sal_Int32 nSheet ) :
    FragmentHandler2( rHelper.getOoxFilter(), rFragmentPath ),
    WorksheetHelperRoot( rHelper, xProgressBar, eSheetType, nSheet )
{
}

OoxWorksheetFragmentBase::OoxWorksheetFragmentBase(
        const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    FragmentHandler2( rHelper.getOoxFilter(), rFragmentPath ),
    WorksheetHelperRoot( rHelper )
{
}

// ============================================================================
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

BiffHandlerBase::~BiffHandlerBase()
{
}

bool BiffHandlerBase::skipRecordBlock( sal_uInt16 nEndRecId )
{
    sal_uInt16 nStartRecId = mrStrm.getRecId();
    while( mrStrm.startNextRecord() && (mrStrm.getRecId() != nEndRecId) )
        if( mrStrm.getRecId() == nStartRecId )
            skipRecordBlock( nEndRecId );
    return !mrStrm.isEof() && (mrStrm.getRecId() == nEndRecId);
}

bool BiffHandlerBase::isBofRecord() const
{
    return
        (mrStrm.getRecId() == BIFF2_ID_BOF) ||
        (mrStrm.getRecId() == BIFF3_ID_BOF) ||
        (mrStrm.getRecId() == BIFF4_ID_BOF) ||
        (mrStrm.getRecId() == BIFF5_ID_BOF);
}

// ============================================================================

BiffContextHandler::BiffContextHandler( const BiffHandlerBase& rParent ) :
    BiffHandlerBase( rParent )
{
}

// ============================================================================

namespace prv {

BiffFragmentStreamOwner::BiffFragmentStreamOwner( const FilterBase& rFilter, const OUString& rStrmName )
{
    // do not automatically close the root stream (indicated by empty stream name)
    mxXInStrm.reset( new BinaryXInputStream( rFilter.openInputStream( rStrmName ), rStrmName.getLength() > 0 ) );
    mxBiffStrm.reset( new BiffInputStream( *mxXInStrm ) );
}

BiffFragmentStreamOwner::~BiffFragmentStreamOwner()
{
}

} // namespace prv

// ----------------------------------------------------------------------------

BiffFragmentHandler::BiffFragmentHandler( const FilterBase& rFilter, const OUString& rStrmName ) :
    prv::BiffFragmentStreamOwner( rFilter, rStrmName ),
    BiffHandlerBase( *mxBiffStrm )
{
}

BiffFragmentHandler::BiffFragmentHandler( const BiffFragmentHandler& rHandler ) :
    prv::BiffFragmentStreamOwner( rHandler ),
    BiffHandlerBase( rHandler )
{
}

BiffFragmentType BiffFragmentHandler::startFragment( BiffType eBiff )
{
    BiffFragmentType eFragment = BIFF_FRAGMENT_UNKNOWN;
    if( mrStrm.startNextRecord() )
    {
        /*  #i23425# Don't rely on BOF record ID to read BOF contents, but on
            the detected BIFF version. */
        if( isBofRecord() )
        {
            // BOF is always written unencrypted
            mrStrm.enableDecoder( false );
            mrStrm.skip( 2 );
            sal_uInt16 nType = mrStrm.readuInt16();

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

bool BiffFragmentHandler::skipFragment()
{
    while( mrStrm.startNextRecord() && (mrStrm.getRecId() != BIFF_ID_EOF) )
        if( isBofRecord() )
            skipFragment();
    return !mrStrm.isEof() && (mrStrm.getRecId() == BIFF_ID_EOF);
}

// ============================================================================

BiffWorkbookFragmentBase::BiffWorkbookFragmentBase( const WorkbookHelper& rHelper, const OUString& rStrmName, bool bCloneDecoder ) :
    BiffFragmentHandler( rHelper.getBaseFilter(), rStrmName ),
    WorkbookHelper( rHelper )
{
    if( bCloneDecoder )
        getCodecHelper().cloneDecoder( mrStrm );
}

// ============================================================================

BiffWorksheetFragmentBase::BiffWorksheetFragmentBase( const BiffWorkbookFragmentBase& rParent,
        ISegmentProgressBarRef xProgressBar, WorksheetType eSheetType, sal_Int32 nSheet ) :
    BiffFragmentHandler( rParent ),
    WorksheetHelperRoot( rParent, xProgressBar, eSheetType, nSheet )
{
}

// ============================================================================

BiffSkipWorksheetFragment::BiffSkipWorksheetFragment(
        const BiffWorkbookFragmentBase& rParent, ISegmentProgressBarRef xProgressBar, sal_Int32 nSheet ) :
    BiffWorksheetFragmentBase( rParent, xProgressBar, SHEETTYPE_EMPTYSHEET, nSheet )
{
}

bool BiffSkipWorksheetFragment::importFragment()
{
    return skipFragment();
}

// ============================================================================
// ============================================================================

} // namespace xls
} // namespace oox

