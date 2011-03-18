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

#include "oox/xls/excelhandlers.hxx"

#include "oox/core/filterbase.hxx"
#include "oox/xls/biffinputstream.hxx"

namespace oox {
namespace xls {

// ============================================================================

using ::oox::core::FilterBase;
using ::oox::core::FragmentHandler2;
using ::rtl::OUString;

// ============================================================================
// ============================================================================

WorkbookFragmentBase::WorkbookFragmentBase(
        const WorkbookHelper& rHelper, const OUString& rFragmentPath ) :
    FragmentHandler2( rHelper.getOoxFilter(), rFragmentPath ),
    WorkbookHelper( rHelper )
{
}

// ============================================================================

WorksheetFragmentBase::WorksheetFragmentBase( const WorkbookHelper& rHelper,
        const OUString& rFragmentPath, const ISegmentProgressBarRef& rxProgressBar, WorksheetType eSheetType, sal_Int16 nSheet ) :
    FragmentHandler2( rHelper.getOoxFilter(), rFragmentPath ),
    WorksheetHelperRoot( rHelper, rxProgressBar, eSheetType, nSheet )
{
}

WorksheetFragmentBase::WorksheetFragmentBase(
        const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    FragmentHandler2( rHelper.getOoxFilter(), rFragmentPath ),
    WorksheetHelperRoot( rHelper )
{
}

// ============================================================================
// ============================================================================

BiffContextHandler::~BiffContextHandler()
{
}

// ----------------------------------------------------------------------------

BiffWorkbookContextBase::BiffWorkbookContextBase( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

// ----------------------------------------------------------------------------

BiffWorksheetContextBase::BiffWorksheetContextBase( const WorkbookHelper& rHelper,
        const ISegmentProgressBarRef& rxProgressBar, WorksheetType eSheetType, sal_Int16 nSheet ) :
    WorksheetHelperRoot( rHelper, rxProgressBar, eSheetType, nSheet )
{
}

BiffWorksheetContextBase::BiffWorksheetContextBase( const WorksheetHelper& rHelper ) :
    WorksheetHelperRoot( rHelper )
{
}

// ============================================================================

namespace {

const sal_uInt16 BIFF_BOF_GLOBALS           = 0x0005;   /// BIFF5-BIFF8 workbook globals.
const sal_uInt16 BIFF_BOF_MODULE            = 0x0006;   /// BIFF5-BIFF8 Visual Basic module.
const sal_uInt16 BIFF_BOF_SHEET             = 0x0010;   /// BIFF2-BIFF8 worksheet/dialog sheet.
const sal_uInt16 BIFF_BOF_CHART             = 0x0020;   /// BIFF2-BIFF8 chart sheet.
const sal_uInt16 BIFF_BOF_MACRO             = 0x0040;   /// BIFF4-BIFF8 macro sheet.
const sal_uInt16 BIFF_BOF_WORKSPACE         = 0x0100;   /// BIFF3-BIFF8 workspace.

BiffFragmentType lclStartFragment( BiffInputStream& rStrm, BiffType eBiff )
{
    BiffFragmentType eFragment = BIFF_FRAGMENT_UNKNOWN;
    /*  #i23425# Don't rely on BOF record ID to read BOF contents, but on
        the detected BIFF version. */
    if( BiffHelper::isBofRecord( rStrm ) )
    {
        // BOF is always written unencrypted
        rStrm.enableDecoder( false );
        rStrm.skip( 2 );
        sal_uInt16 nType = rStrm.readuInt16();

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
    return eFragment;
}

} // namespace

// ----------------------------------------------------------------------------

BiffFragmentHandler::BiffFragmentHandler( const FilterBase& rFilter, const OUString& rStrmName )
{
    // do not automatically close the root stream (indicated by empty stream name)
    bool bRootStrm = rStrmName.getLength() == 0;
    mxXInStrm.reset( new BinaryXInputStream( rFilter.openInputStream( rStrmName ), !bRootStrm ) );
    mxBiffStrm.reset( new BiffInputStream( *mxXInStrm ) );
}

BiffFragmentHandler::~BiffFragmentHandler()
{
}

BiffFragmentType BiffFragmentHandler::startFragment( BiffType eBiff )
{
    return mxBiffStrm->startNextRecord() ? lclStartFragment( *mxBiffStrm, eBiff ) : BIFF_FRAGMENT_UNKNOWN;
}

BiffFragmentType BiffFragmentHandler::startFragment( BiffType eBiff, sal_Int64 nRecHandle )
{
    return mxBiffStrm->startRecordByHandle( nRecHandle ) ? lclStartFragment( *mxBiffStrm, eBiff ) : BIFF_FRAGMENT_UNKNOWN;
}

bool BiffFragmentHandler::skipFragment()
{
    while( mxBiffStrm->startNextRecord() && (mxBiffStrm->getRecId() != BIFF_ID_EOF) )
        if( BiffHelper::isBofRecord( *mxBiffStrm ) )
            skipFragment();
    return !mxBiffStrm->isEof() && (mxBiffStrm->getRecId() == BIFF_ID_EOF);
}

// ----------------------------------------------------------------------------

BiffWorkbookFragmentBase::BiffWorkbookFragmentBase( const WorkbookHelper& rHelper, const OUString& rStrmName, bool bCloneDecoder ) :
    BiffFragmentHandler( rHelper.getBaseFilter(), rStrmName ),
    WorkbookHelper( rHelper )
{
    if( bCloneDecoder )
        getCodecHelper().cloneDecoder( getInputStream() );
}

// ----------------------------------------------------------------------------

BiffWorksheetFragmentBase::BiffWorksheetFragmentBase( const BiffWorkbookFragmentBase& rParent,
        const ISegmentProgressBarRef& rxProgressBar, WorksheetType eSheetType, sal_Int16 nSheet ) :
    BiffFragmentHandler( rParent ),
    WorksheetHelperRoot( rParent, rxProgressBar, eSheetType, nSheet )
{
}

// ----------------------------------------------------------------------------

BiffSkipWorksheetFragment::BiffSkipWorksheetFragment( const BiffWorkbookFragmentBase& rParent,
        const ISegmentProgressBarRef& rxProgressBar, sal_Int16 nSheet ) :
    BiffWorksheetFragmentBase( rParent, rxProgressBar, SHEETTYPE_EMPTYSHEET, nSheet )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */