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

#include "sharedstringsfragment.hxx"

#include "richstringcontext.hxx"
#include "sharedstringsbuffer.hxx"

namespace oox {
namespace xls {

using namespace ::oox::core;

SharedStringsFragment::SharedStringsFragment(
        const WorkbookHelper& rHelper, const OUString& rFragmentPath ) :
    WorkbookFragmentBase( rHelper, rFragmentPath )
{
}

ContextHandlerRef SharedStringsFragment::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( sst ) )
                return this;
        break;

        case XLS_TOKEN( sst ):
            if( nElement == XLS_TOKEN( si ) )
                return new RichStringContext( *this, getSharedStrings().createRichString() );
        break;
    }
    return nullptr;
}

ContextHandlerRef SharedStringsFragment::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == BIFF12_ID_SST )
                return this;
        break;

        case BIFF12_ID_SST:
            if( nRecId == BIFF12_ID_SI )
                getSharedStrings().createRichString()->importString( rStrm, true );
        break;
    }
    return nullptr;
}

const RecordInfo* SharedStringsFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { BIFF12_ID_SST,    BIFF12_ID_SST + 1   },
        { -1,               -1                  }
    };
    return spRecInfos;
}

void SharedStringsFragment::finalizeImport()
{
    getSharedStrings().finalizeImport();
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
