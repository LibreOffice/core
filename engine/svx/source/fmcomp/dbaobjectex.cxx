/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <svx/dbaobjectex.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <sot/formats.hxx>
#include <sot/exchange.hxx>


namespace svx
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::datatransfer;

    OComponentTransferable::OComponentTransferable()
    {
    }

    SotClipboardFormatId OComponentTransferable::getDescriptorFormatId(bool _bExtractForm)
    {
        static SotClipboardFormatId s_nReportFormat = static_cast<SotClipboardFormatId>(-1);
        static SotClipboardFormatId s_nFormFormat = static_cast<SotClipboardFormatId>(-1);
        if ( _bExtractForm && static_cast<SotClipboardFormatId>(-1) == s_nFormFormat )
        {
            s_nFormFormat = SotExchange::RegisterFormatName(u"application/x-openoffice;windows_formatname=\"dbaccess.FormComponentDescriptorTransfer\""_ustr);
            OSL_ENSURE(static_cast<SotClipboardFormatId>(-1) != s_nFormFormat, "OComponentTransferable::getDescriptorFormatId: bad exchange id!");
        }
        else if ( !_bExtractForm && static_cast<SotClipboardFormatId>(-1) == s_nReportFormat)
        {
            s_nReportFormat = SotExchange::RegisterFormatName(u"application/x-openoffice;windows_formatname=\"dbaccess.ReportComponentDescriptorTransfer\""_ustr);
            OSL_ENSURE(static_cast<SotClipboardFormatId>(-1) != s_nReportFormat, "OComponentTransferable::getDescriptorFormatId: bad exchange id!");
        }
        return _bExtractForm ? s_nFormFormat : s_nReportFormat;
    }


    void OComponentTransferable::AddSupportedFormats()
    {
        bool bForm = true;
        try
        {
            Reference<XPropertySet> xProp;
            m_aDescriptor[DataAccessDescriptorProperty::Component] >>= xProp;
            if ( xProp.is() )
                xProp->getPropertyValue(u"IsForm"_ustr) >>= bForm;
        }
        catch(const Exception&)
        {}
        AddFormat(getDescriptorFormatId(bForm));
    }


    bool OComponentTransferable::GetData( const DataFlavor& _rFlavor, const OUString& /*rDestDoc*/ )
    {
        const SotClipboardFormatId nFormatId = SotExchange::GetFormat(_rFlavor);
        if ( nFormatId == getDescriptorFormatId(true) || nFormatId == getDescriptorFormatId(false) )
            return SetAny( Any( m_aDescriptor.createPropertyValueSequence() ) );

        return false;
    }

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
