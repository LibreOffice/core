/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"
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








