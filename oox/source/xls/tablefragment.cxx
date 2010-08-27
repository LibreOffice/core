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

#include "oox/xls/tablefragment.hxx"

using ::rtl::OUString;
using ::oox::core::ContextHandlerRef;
using ::oox::core::RecordInfo;

namespace oox {
namespace xls {

// ============================================================================

OoxTableFragment::OoxTableFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    OoxWorksheetFragmentBase( rHelper, rFragmentPath )
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextHandlerRef OoxTableFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( table ) )
                mxTable = getTables().importTable( rAttribs, getSheetIndex() );
        break;
    }
    return 0;
}

ContextHandlerRef OoxTableFragment::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == OOBIN_ID_TABLE )
                mxTable = getTables().importTable( rStrm, getSheetIndex() );
        break;
    }
    return 0;
}

// oox.core.FragmentHandler2 interface ----------------------------------------

const RecordInfo* OoxTableFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { OOBIN_ID_TABLE,   OOBIN_ID_TABLE + 1  },
        { -1,               -1                  }
    };
    return spRecInfos;
}

// ============================================================================

} // namespace xls
} // namespace oox

