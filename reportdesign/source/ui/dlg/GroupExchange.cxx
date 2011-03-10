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
#include "precompiled_reportdesign.hxx"
#include "GroupExchange.hxx"
#include <sot/formats.hxx>
#include <sot/storage.hxx>

namespace rptui
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;

    sal_uInt32 OGroupExchange::getReportGroupId()
    {
        static sal_uInt32 s_nReportFormat = (sal_uInt32)-1;
        if ( (sal_uInt32)-1 == s_nReportFormat )
        {
            s_nReportFormat = SotExchange::RegisterFormatName(String::CreateFromAscii("application/x-openoffice;windows_formatname=\"reportdesign.GroupFormat\"" ));
            OSL_ENSURE((sal_uInt32)-1 != s_nReportFormat, "Bad exchange id!");
        }
        return s_nReportFormat;
    }
    OGroupExchange::OGroupExchange(const uno::Sequence< uno::Any >& _aGroupRow)
        : m_aGroupRow(_aGroupRow)
    {
    }
    // -----------------------------------------------------------------------------
    void OGroupExchange::AddSupportedFormats()
    {
        if ( m_aGroupRow.getLength() )
        {
            AddFormat(OGroupExchange::getReportGroupId());
        }
    }
    // -----------------------------------------------------------------------------
    sal_Bool OGroupExchange::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
    {
        sal_uLong nFormat = SotExchange::GetFormat(rFlavor);
        if(nFormat == OGroupExchange::getReportGroupId() )
        {
            return SetAny(uno::makeAny(m_aGroupRow),rFlavor);
        }
        return sal_False;
    }
    // -----------------------------------------------------------------------------
    void OGroupExchange::ObjectReleased()
    {
        m_aGroupRow.realloc(0);
    }
    // -----------------------------------------------------------------------------
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
