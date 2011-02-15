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

#include "oox/xls/sharedstringsfragment.hxx"

#include "oox/xls/richstringcontext.hxx"
#include "oox/xls/sharedstringsbuffer.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::oox::core;

using ::rtl::OUString;

// ============================================================================

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
    return 0;
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
    return 0;
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

// ============================================================================

} // namespace xls
} // namespace oox
