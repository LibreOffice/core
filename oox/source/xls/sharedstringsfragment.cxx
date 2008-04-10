/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sharedstringsfragment.cxx,v $
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

#include "oox/xls/sharedstringsfragment.hxx"
#include "oox/xls/sharedstringsbuffer.hxx"
#include "oox/xls/richstringcontext.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::oox::core::RecordInfo;

namespace oox {
namespace xls {

// ============================================================================

OoxSharedStringsFragment::OoxSharedStringsFragment(
        const WorkbookHelper& rHelper, const OUString& rFragmentPath ) :
    OoxWorkbookFragmentBase( rHelper, rFragmentPath )
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper OoxSharedStringsFragment::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return  (nElement == XLS_TOKEN( sst ));
        case XLS_TOKEN( sst ):
            if( nElement == XLS_TOKEN( si ) )
                return new OoxRichStringContext( *this, getSharedStrings().createRichString() );
        break;
    }
    return false;
}

ContextWrapper OoxSharedStringsFragment::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return  (nRecId == OOBIN_ID_SST);
        case OOBIN_ID_SST:
            return  (nRecId == OOBIN_ID_SI);
    }
    return false;
}

void OoxSharedStringsFragment::onStartRecord( RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case OOBIN_ID_SI:   getSharedStrings().createRichString()->importString( rStrm, true ); break;
    }
}

// oox.core.FragmentHandler2 interface ----------------------------------------

const RecordInfo* OoxSharedStringsFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { OOBIN_ID_SST,     OOBIN_ID_SST + 1    },
        { -1,               -1                  }
    };
    return spRecInfos;
}

void OoxSharedStringsFragment::finalizeImport()
{
    getSharedStrings().finalizeImport();
}

// ============================================================================

} // namespace xls
} // namespace oox

