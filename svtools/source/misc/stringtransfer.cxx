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

#include <sal/config.h>

#include <rtl/ref.hxx>
#include <sot/exchange.hxx>
#include <svtools/stringtransfer.hxx>
#include <utility>


namespace svt
{


    using namespace ::com::sun::star::datatransfer;


    //= OStringTransferable
    OStringTransferable::OStringTransferable(OUString aContent)
        : TransferDataContainer()
        , m_sContent(std::move(aContent))
    {
    }

    void OStringTransferable::AddSupportedFormats()
    {
        AddFormat(SotClipboardFormatId::STRING);
    }

    void OStringTransferable::SetData(const OUString& rContent)
    {
        m_sContent = rContent;
        ClearFormats(); // invalidate m_aAny so new data will take effect
    }

    bool OStringTransferable::GetData( const DataFlavor& _rFlavor, const OUString& /*rDestDoc*/ )
    {
        SotClipboardFormatId nFormat = SotExchange::GetFormat( _rFlavor );
        if (SotClipboardFormatId::STRING == nFormat)
            return SetString( m_sContent );

        return false;
    }

    //= OStringTransfer
    void OStringTransfer::CopyString( const OUString& _rContent, vcl::Window* _pWindow )
    {
        rtl::Reference<OStringTransferable> pTransferable = new OStringTransferable( _rContent );
        pTransferable->CopyToClipboard( _pWindow );
    }

    bool OStringTransfer::PasteString( OUString& _rContent, vcl::Window* _pWindow )
    {
        TransferableDataHelper aClipboardData = TransferableDataHelper::CreateFromSystemClipboard( _pWindow );

        // check for a string format
        const DataFlavorExVector& rFormats = aClipboardData.GetDataFlavorExVector();
        for (auto const& format : rFormats)
        {
            if (SotClipboardFormatId::STRING == format.mnSotId)
            {
                OUString sContent;
                bool bSuccess = aClipboardData.GetString( SotClipboardFormatId::STRING, sContent );
                _rContent = sContent;
                return bSuccess;
            }
        }

        return false;
    }

    void OStringTransfer::StartStringDrag( const OUString& _rContent, vcl::Window* _pWindow, sal_Int8 _nDragSourceActions )
    {
        rtl::Reference<OStringTransferable> pTransferable = new OStringTransferable( _rContent );
        pTransferable->StartDrag(_pWindow, _nDragSourceActions);
    }

}   // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
