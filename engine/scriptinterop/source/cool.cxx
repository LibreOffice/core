/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cpo/uno/Reference.hxx>
#include <cpo/uno/RuntimeException.hpp>
#include <cpo/uno/XComponentContext.hpp>
#include <cpo/uno/XInterface.hpp>
#include <comphelper/processfactory.hxx>
#include <cpo/uno/Any.hxx>
#include <cpo/uno/Sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sal/types.h>
#include <scriptinterop/XBlob.hpp>
#include <scriptinterop/XDocument.hpp>
#include <scriptinterop/XDrawing.hpp>
#include <scriptinterop/XFactory.hpp>
#include <scriptinterop/XPresentation.hpp>
#include <scriptinterop/XSpreadsheet.hpp>

#include "blob.hxx"
#include "document.hxx"
#include "drawing.hxx"
#include "presentation.hxx"
#include "spreadsheet.hxx"

namespace
{
cpo::uno::Reference<css::frame::XModel> currentModel()
{
    auto const frame
        = css::frame::Desktop::create(comphelper::getProcessComponentContext())->getCurrentFrame();
    auto const controller = frame.is() ? frame->getController() : nullptr;
    return controller.is() ? controller->getModel() : nullptr;
}

cpo::uno::Reference<css::frame::XModel> modelRequiring(OUString const& requiredService,
                                                       OUString const& excludedService)
{
    auto const model = currentModel();
    cpo::uno::Reference<css::lang::XServiceInfo> const info(model, cpo::uno::UNO_QUERY);
    if (!info.is() || !info->supportsService(requiredService)
        || (!excludedService.isEmpty() && info->supportsService(excludedService)))
    {
        throw cpo::uno::RuntimeException(u"current document does not support "_ustr
                                         + requiredService);
    }
    return model;
}

class FactoryImpl : public cppu::WeakImplHelper<scriptinterop::XFactory>
{
public:
    cpo::uno::Reference<css::frame::XModel> SAL_CALL getActiveUnoModel() override
    {
        return currentModel();
    }

    cpo::uno::Reference<scriptinterop::XDocument> SAL_CALL getActiveDocument() override
    {
        return scriptinterop::detail::createDocument(
            modelRequiring(u"com.sun.star.text.TextDocument"_ustr, OUString()));
    }

    cpo::uno::Reference<scriptinterop::XSpreadsheet> SAL_CALL getActiveSpreadsheet() override
    {
        return scriptinterop::detail::createSpreadsheet(
            modelRequiring(u"com.sun.star.sheet.SpreadsheetDocument"_ustr, OUString()));
    }

    cpo::uno::Reference<scriptinterop::XPresentation> SAL_CALL getActivePresentation() override
    {
        return scriptinterop::detail::createPresentation(
            modelRequiring(u"com.sun.star.presentation.PresentationDocument"_ustr, OUString()));
    }

    cpo::uno::Reference<scriptinterop::XDrawing> SAL_CALL getActiveDrawing() override
    {
        return scriptinterop::detail::createDrawing(
            modelRequiring(u"com.sun.star.drawing.DrawingDocument"_ustr,
                           u"com.sun.star.presentation.PresentationDocument"_ustr));
    }

    cpo::uno::Reference<scriptinterop::XBlob> newBlobBytes(
        cpo::uno::Sequence<sal_Int8> const & data) override
    {
        return scriptinterop::detail::createBlob(data, {false, {}}, {false, {}});
    }

    cpo::uno::Reference<scriptinterop::XBlob> newBlobBytesType(
        cpo::uno::Sequence<sal_Int8> const & data,
        css::beans::Optional<OUString> const & contentType) override
    {
        return scriptinterop::detail::createBlob(data, contentType, {false, {}});
    }

    cpo::uno::Reference<scriptinterop::XBlob> newBlobBytesTypeName(
        cpo::uno::Sequence<sal_Int8> const & data,
        css::beans::Optional<OUString> const & contentType,
        css::beans::Optional<OUString> const & name) override
    {
        return scriptinterop::detail::createBlob(data, contentType, name);
    }

    cpo::uno::Reference<scriptinterop::XBlob> newBlobString(OUString const & data) override {
        // Real GAS's Utilities.newBlob(String data) defaults the content type to "text/plain"
        // (the two- and three-arg string overloads take an explicit Optional<string>, so a
        // caller who wants no content type there passes null):
        return scriptinterop::detail::createBlob(
            stringToBytes(data), {true, u"text/plain"_ustr}, {false, {}});
    }

    cpo::uno::Reference<scriptinterop::XBlob> newBlobStringType(
        OUString const & data, css::beans::Optional<OUString> const & contentType) override
    {
        return scriptinterop::detail::createBlob(stringToBytes(data), contentType, {false, {}});
    }

    cpo::uno::Reference<scriptinterop::XBlob> newBlobStringTypeName(
        OUString const & data, css::beans::Optional<OUString> const & contentType,
        css::beans::Optional<OUString> const & name) override
    {
        return scriptinterop::detail::createBlob(stringToBytes(data), contentType, name);
    }

private:
    static cpo::uno::Sequence<sal_Int8> stringToBytes(OUString const & data) {
        auto const utf8 = data.toUtf8();
        return cpo::uno::Sequence<sal_Int8>(
            reinterpret_cast<sal_Int8 const *>(utf8.getStr()), utf8.getLength());
    }
};
}

extern "C" SAL_DLLPUBLIC_EXPORT cpo::uno::XInterface*
scriptinterop_Cool_get_implementation(cpo::uno::XComponentContext*,
                                      cpo::uno::Sequence<cpo::uno::Any> const&)
{
    return cppu::acquire(new FactoryImpl);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
