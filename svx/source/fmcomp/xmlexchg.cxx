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

#include <svx/xmlexchg.hxx>
#include <sot/formats.hxx>
#include <sot/exchange.hxx>

namespace svx
{


    using namespace ::com::sun::star::datatransfer;

    OXFormsTransferable::OXFormsTransferable(const std::function<OXFormsDescriptor()>& getDescriptorFunc) :
        m_getDescriptorFunc(getDescriptorFunc)
    {
    }

    void OXFormsTransferable::AddSupportedFormats()
    {
        AddFormat( SotClipboardFormatId::XFORMS );
    }

    bool OXFormsTransferable::GetData( const DataFlavor& _rFlavor, const OUString& /*rDestDoc*/ )
    {
        const SotClipboardFormatId nFormatId = SotExchange::GetFormat( _rFlavor );
        if ( SotClipboardFormatId::XFORMS == nFormatId )
        {
            return SetString(u"XForms-Transferable"_ustr);
        }
        return false;
    }

    OXFormsDescriptor OXFormsTransferable::extractDescriptor( const TransferableDataHelper &_rData )
    {
        using namespace ::com::sun::star::uno;
        Reference<XTransferable> &transfer = const_cast<Reference<XTransferable> &>(_rData.GetTransferable());
        XTransferable *pInterface = transfer.get();
        OXFormsTransferable& rThis = dynamic_cast<OXFormsTransferable&>(*pInterface);
        return rThis.m_getDescriptorFunc();
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
