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
#include "dlgedclip.hxx"
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>
#include <sot/exchange.hxx>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>

namespace rptui
{

using namespace comphelper;
using namespace ::com::sun::star;


// OReportExchange


OReportExchange::OReportExchange(const TSectionElements& _rCopyElements )
: m_aCopyElements(_rCopyElements)
{
}

SotClipboardFormatId OReportExchange::getDescriptorFormatId()
{
    static SotClipboardFormatId s_nFormat = static_cast<SotClipboardFormatId>(-1);
    if (static_cast<SotClipboardFormatId>(-1) == s_nFormat)
    {
        s_nFormat = SotExchange::RegisterFormatName("application/x-openoffice;windows_formatname=\"report.ReportObjectsTransfer\"");
        OSL_ENSURE(static_cast<SotClipboardFormatId>(-1) != s_nFormat, "OReportExchange::getDescriptorFormatId: bad exchange id!");
    }
    return s_nFormat;
}

void OReportExchange::AddSupportedFormats()
{
    AddFormat(getDescriptorFormatId());
}

bool OReportExchange::GetData( const datatransfer::DataFlavor& _rFlavor, const OUString& /*rDestDoc*/ )
{
    const SotClipboardFormatId nFormatId = SotExchange::GetFormat(_rFlavor);
    return (nFormatId == getDescriptorFormatId()) && SetAny( uno::Any(m_aCopyElements), _rFlavor );
}

bool OReportExchange::canExtract(const DataFlavorExVector& _rFlavor)
{
    return IsFormatSupported(_rFlavor,getDescriptorFormatId());
}

OReportExchange::TSectionElements OReportExchange::extractCopies(const TransferableDataHelper& _rData)
{
    SotClipboardFormatId nKnownFormatId = getDescriptorFormatId();
    if ( _rData.HasFormat( nKnownFormatId ) )
    {
        // extract the any from the transferable
        datatransfer::DataFlavor aFlavor;
        bool bSuccess =
            SotExchange::GetFormatDataFlavor(nKnownFormatId, aFlavor);
        OSL_ENSURE(bSuccess, "OReportExchange::extractCopies: invalid data format (no flavor)!");

        uno::Any aDescriptor = _rData.GetAny(aFlavor, OUString());

        TSectionElements aCopies;
        bSuccess = aDescriptor >>= aCopies;
        OSL_ENSURE(bSuccess, "OReportExchange::extractCopies: invalid clipboard format!");

        // build the real descriptor
        return aCopies;
    }

    return TSectionElements();
}

} // rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
