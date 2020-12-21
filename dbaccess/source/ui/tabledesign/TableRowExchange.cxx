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

#include <TableRowExchange.hxx>
#include <sot/formats.hxx>
#include <sot/storage.hxx>
#include <TableRow.hxx>

namespace dbaui
{
    constexpr sal_uInt32 FORMAT_OBJECT_ID_SBA_TABED = 1;

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    OTableRowExchange::OTableRowExchange(const std::vector< std::shared_ptr<OTableRow> >& _rvTableRow)
        : m_vTableRow(_rvTableRow)
    {
    }
    bool OTableRowExchange::WriteObject( tools::SvRef<SotTempStream>& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const css::datatransfer::DataFlavor& /*rFlavor*/ )
    {
        if(nUserObjectId == FORMAT_OBJECT_ID_SBA_TABED)
        {
            std::vector< std::shared_ptr<OTableRow> >* pRows = static_cast< std::vector< std::shared_ptr<OTableRow> >* >(pUserObject);
            if(pRows)
            {
                (*rxOStm).WriteInt32( pRows->size() ); // first stream the size
                for (auto const& row : *pRows)
                    WriteOTableRow(*rxOStm, *row);
                return true;
            }
        }
        return false;
    }
    void OTableRowExchange::AddSupportedFormats()
    {
        if ( !m_vTableRow.empty() )
            AddFormat(SotClipboardFormatId::SBA_TABED);
    }
    bool OTableRowExchange::GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& /*rDestDoc*/ )
    {
        SotClipboardFormatId nFormat = SotExchange::GetFormat(rFlavor);
        if(nFormat == SotClipboardFormatId::SBA_TABED)
            return SetObject(&m_vTableRow,FORMAT_OBJECT_ID_SBA_TABED,rFlavor);
        return false;
    }
    void OTableRowExchange::ObjectReleased()
    {
        m_vTableRow.clear();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
