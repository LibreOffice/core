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

#include "excelhandlers.hxx"

#include "oox/core/filterbase.hxx"
#include "biffinputstream.hxx"

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

WorksheetFragmentBase::WorksheetFragmentBase(
        const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    FragmentHandler2( rHelper.getOoxFilter(), rFragmentPath ),
    WorksheetHelper( rHelper )
{
}

// ============================================================================
// ============================================================================

BiffContextHandler::~BiffContextHandler()
{
}

// ----------------------------------------------------------------------------

BiffWorksheetContextBase::BiffWorksheetContextBase( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper )
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

} // namespace

// ----------------------------------------------------------------------------

BiffFragmentHandler::BiffFragmentHandler( const FilterBase& rFilter, const OUString& rStrmName )
{
    // do not automatically close the root stream (indicated by empty stream name)
    bool bRootStrm = rStrmName.isEmpty();
    mxXInStrm.reset( new BinaryXInputStream( rFilter.openInputStream( rStrmName ), !bRootStrm ) );
    mxBiffStrm.reset( new BiffInputStream( *mxXInStrm ) );
}

BiffFragmentHandler::~BiffFragmentHandler()
{
}

bool BiffFragmentHandler::skipFragment()
{
    while( mxBiffStrm->startNextRecord() && (mxBiffStrm->getRecId() != BIFF_ID_EOF) )
        if( BiffHelper::isBofRecord( *mxBiffStrm ) )
            skipFragment();
    return !mxBiffStrm->isEof() && (mxBiffStrm->getRecId() == BIFF_ID_EOF);
}

// ============================================================================

BiffWorkbookFragmentBase::BiffWorkbookFragmentBase( const WorkbookHelper& rHelper, const OUString& rStrmName, bool bCloneDecoder ) :
    BiffFragmentHandler( rHelper.getBaseFilter(), rStrmName ),
    WorkbookHelper( rHelper )
{
    if( bCloneDecoder )
        getCodecHelper().cloneDecoder( getInputStream() );
}

// ----------------------------------------------------------------------------

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
