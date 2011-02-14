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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#ifndef DBAUI_TABLEROW_EXCHANGE_HXX
#include "TableRowExchange.hxx"
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif
#ifndef DBAUI_TABLEROW_HXX
#include "TableRow.hxx"
#endif

namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    OTableRowExchange::OTableRowExchange(const ::std::vector< ::boost::shared_ptr<OTableRow> >& _rvTableRow)
        : m_vTableRow(_rvTableRow)
    {
    }
    // -----------------------------------------------------------------------------
    sal_Bool OTableRowExchange::WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& /*rFlavor*/ )
    {
        if(nUserObjectId == SOT_FORMATSTR_ID_SBA_TABED)
        {
            ::std::vector< ::boost::shared_ptr<OTableRow> >* pRows = reinterpret_cast< ::std::vector< ::boost::shared_ptr<OTableRow> >* >(pUserObject);
            if(pRows)
            {
                (*rxOStm) << (sal_Int32)pRows->size(); // first stream the size
                ::std::vector< ::boost::shared_ptr<OTableRow> >::const_iterator aIter = pRows->begin();
                ::std::vector< ::boost::shared_ptr<OTableRow> >::const_iterator aEnd = pRows->end();
                for(;aIter != aEnd;++aIter)
                    (*rxOStm) << *(*aIter);
                return sal_True;
            }
        }
        return sal_False;
    }
    // -----------------------------------------------------------------------------
    void OTableRowExchange::AddSupportedFormats()
    {
        if ( !m_vTableRow.empty() )
            AddFormat(SOT_FORMATSTR_ID_SBA_TABED);
    }
    // -----------------------------------------------------------------------------
    sal_Bool OTableRowExchange::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
    {
        sal_uLong nFormat = SotExchange::GetFormat(rFlavor);
        if(nFormat == SOT_FORMATSTR_ID_SBA_TABED)
            return SetObject(&m_vTableRow,SOT_FORMATSTR_ID_SBA_TABED,rFlavor);
        return sal_False;
    }
    // -----------------------------------------------------------------------------
    void OTableRowExchange::ObjectReleased()
    {
        m_vTableRow.clear();
    }
    // -----------------------------------------------------------------------------
}








