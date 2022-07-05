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

#include <vcl/htmltransferable.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <cppuhelper/queryinterface.hxx>
#include <boost/property_tree/json_parser.hpp>

using namespace ::com::sun::star;

namespace vcl::unohelper
{
HtmlTransferable::HtmlTransferable(OString sData)
    : data(sData)
{
}

HtmlTransferable::~HtmlTransferable() {}

// css::uno::XInterface
uno::Any HtmlTransferable::queryInterface(const uno::Type& rType)
{
    uno::Any aRet = ::cppu::queryInterface(rType, static_cast<datatransfer::XTransferable*>(this));
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface(rType));
}

// css::datatransfer::XTransferable
uno::Any HtmlTransferable::getTransferData(const datatransfer::DataFlavor& rFlavor)
{
    SotClipboardFormatId nT = SotExchange::GetFormat(rFlavor);
    if (nT != SotClipboardFormatId::HTML)
    {
        throw datatransfer::UnsupportedFlavorException();
    }
    size_t size = data.getLength();
    uno::Sequence<sal_Int8> sData(size);
    std::memcpy(sData.getArray(), data.getStr(), size);
    return uno::Any(sData);
}

uno::Sequence<datatransfer::DataFlavor> HtmlTransferable::getTransferDataFlavors()
{
    uno::Sequence<datatransfer::DataFlavor> aDataFlavors(1);
    auto ref = aDataFlavors.getArray()[0];
    ref.MimeType = "text/html";
    ref.DataType = cppu::UnoType<uno::Sequence<sal_Int8>>::get();
    SotExchange::GetFormatDataFlavor(SotClipboardFormatId::HTML, aDataFlavors.getArray()[0]);
    return aDataFlavors;
}

sal_Bool HtmlTransferable::isDataFlavorSupported(const datatransfer::DataFlavor& rFlavor)
{
    SotClipboardFormatId nT = SotExchange::GetFormat(rFlavor);
    return (nT == SotClipboardFormatId::HTML);
}

} // namespace vcl::unohelper
