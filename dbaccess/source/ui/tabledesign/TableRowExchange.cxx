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

#include "TableRowExchange.hxx"
#include <sot/formats.hxx>
#include <sot/storage.hxx>
#include "TableRow.hxx"

namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    OTableRowExchange::OTableRowExchange(const ::std::vector< ::boost::shared_ptr<OTableRow> >& _rvTableRow)
        : m_vTableRow(_rvTableRow)
    {
    }
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
    void OTableRowExchange::AddSupportedFormats()
    {
        if ( !m_vTableRow.empty() )
            AddFormat(SOT_FORMATSTR_ID_SBA_TABED);
    }
    sal_Bool OTableRowExchange::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
    {
        sal_uLong nFormat = SotExchange::GetFormat(rFlavor);
        if(nFormat == SOT_FORMATSTR_ID_SBA_TABED)
            return SetObject(&m_vTableRow,SOT_FORMATSTR_ID_SBA_TABED,rFlavor);
        return sal_False;
    }
    void OTableRowExchange::ObjectReleased()
    {
        m_vTableRow.clear();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
