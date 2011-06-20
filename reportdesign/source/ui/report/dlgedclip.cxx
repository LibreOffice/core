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
#include "precompiled_rptui.hxx"
#include "dlgedclip.hxx"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>

namespace rptui
{

using namespace comphelper;
using namespace ::com::sun::star;

//============================================================================
// OReportExchange
//============================================================================
//----------------------------------------------------------------------------
OReportExchange::OReportExchange(const TSectionElements& _rCopyElements )
: m_aCopyElements(_rCopyElements)
{
}
//--------------------------------------------------------------------
sal_uInt32 OReportExchange::getDescriptorFormatId()
{
    static sal_uInt32 s_nFormat = (sal_uInt32)-1;
    if ((sal_uInt32)-1 == s_nFormat)
    {
        s_nFormat = SotExchange::RegisterFormatName(String::CreateFromAscii("application/x-openoffice;windows_formatname=\"report.ReportObjectsTransfer\""));
        OSL_ENSURE((sal_uInt32)-1 != s_nFormat, "OReportExchange::getDescriptorFormatId: bad exchange id!");
    }
    return s_nFormat;
}
//--------------------------------------------------------------------
void OReportExchange::AddSupportedFormats()
{
    AddFormat(getDescriptorFormatId());
}
//--------------------------------------------------------------------
sal_Bool OReportExchange::GetData( const datatransfer::DataFlavor& _rFlavor )
{
    const sal_uInt32 nFormatId = SotExchange::GetFormat(_rFlavor);
    return (nFormatId == getDescriptorFormatId()) ?
        SetAny( uno::Any(m_aCopyElements), _rFlavor )
        : sal_False;
}
// -----------------------------------------------------------------------------
sal_Bool OReportExchange::canExtract(const DataFlavorExVector& _rFlavor)
{
    return IsFormatSupported(_rFlavor,getDescriptorFormatId());
}
// -----------------------------------------------------------------------------
OReportExchange::TSectionElements OReportExchange::extractCopies(const TransferableDataHelper& _rData)
{
    sal_Int32 nKnownFormatId = getDescriptorFormatId();
    if ( _rData.HasFormat( nKnownFormatId ) )
    {
        // extract the any from the transferable
        datatransfer::DataFlavor aFlavor;
#if OSL_DEBUG_LEVEL > 0
        sal_Bool bSuccess =
#endif
        SotExchange::GetFormatDataFlavor(nKnownFormatId, aFlavor);
        OSL_ENSURE(bSuccess, "OReportExchange::extractCopies: invalid data format (no flavor)!");

        uno::Any aDescriptor = _rData.GetAny(aFlavor);

        TSectionElements aCopies;
#if OSL_DEBUG_LEVEL > 0
        bSuccess =
#endif
        aDescriptor >>= aCopies;
        OSL_ENSURE(bSuccess, "OReportExchange::extractCopies: invalid clipboard format!");

        // build the real descriptor
        return aCopies;
    }

    return TSectionElements();
}
//============================================================================
} // rptui
//============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
