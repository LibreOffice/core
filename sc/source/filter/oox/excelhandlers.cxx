/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "excelhandlers.hxx"

#include <oox/core/filterbase.hxx>
#include "biffinputstream.hxx"

namespace oox {
namespace xls {

using ::oox::core::FilterBase;
using ::oox::core::FragmentHandler2;

WorkbookFragmentBase::WorkbookFragmentBase(
        const WorkbookHelper& rHelper, const OUString& rFragmentPath ) :
    FragmentHandler2( rHelper.getOoxFilter(), rFragmentPath ),
    WorkbookHelper( rHelper )
{
}

WorksheetFragmentBase::WorksheetFragmentBase(
        const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    FragmentHandler2( rHelper.getOoxFilter(), rFragmentPath ),
    WorksheetHelper( rHelper )
{
}

BiffWorksheetContextBase::BiffWorksheetContextBase( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper )
{
}

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

BiffWorkbookFragmentBase::BiffWorkbookFragmentBase( const WorkbookHelper& rHelper, const OUString& rStrmName, bool bCloneDecoder ) :
    BiffFragmentHandler( rHelper.getBaseFilter(), rStrmName ),
    WorkbookHelper( rHelper )
{
    if( bCloneDecoder )
        getCodecHelper().cloneDecoder( getInputStream() );
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
