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

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <comphelper/processfactory.hxx>
#include <cpo/uno/Any.hxx>
#include <cpo/uno/Sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sal/types.h>
#include <scriptinterop/XDocument.hpp>
#include <scriptinterop/XDrawing.hpp>
#include <scriptinterop/XFactory.hpp>
#include <scriptinterop/XPresentation.hpp>
#include <scriptinterop/XSpreadsheet.hpp>

#include "document.hxx"
#include "drawing.hxx"
#include "presentation.hxx"
#include "spreadsheet.hxx"

namespace
{
css::uno::Reference<css::frame::XModel> currentModel()
{
    auto const frame
        = css::frame::Desktop::create(comphelper::getProcessComponentContext())->getCurrentFrame();
    auto const controller = frame.is() ? frame->getController() : nullptr;
    return controller.is() ? controller->getModel() : nullptr;
}

css::uno::Reference<css::frame::XModel> modelRequiring(OUString const& requiredService,
                                                       OUString const& excludedService)
{
    auto const model = currentModel();
    css::uno::Reference<css::lang::XServiceInfo> const info(model, css::uno::UNO_QUERY);
    if (!info.is() || !info->supportsService(requiredService)
        || (!excludedService.isEmpty() && info->supportsService(excludedService)))
    {
        throw css::uno::RuntimeException(u"current document does not support "_ustr
                                         + requiredService);
    }
    return model;
}

class FactoryImpl : public cppu::WeakImplHelper<scriptinterop::XFactory>
{
public:
    css::uno::Reference<css::frame::XModel> SAL_CALL getActiveUnoModel() override
    {
        return currentModel();
    }

    css::uno::Reference<scriptinterop::XDocument> SAL_CALL getActiveDocument() override
    {
        return scriptinterop::detail::createDocument(
            modelRequiring(u"com.sun.star.text.TextDocument"_ustr, OUString()));
    }

    css::uno::Reference<scriptinterop::XSpreadsheet> SAL_CALL getActiveSpreadsheet() override
    {
        return scriptinterop::detail::createSpreadsheet(
            modelRequiring(u"com.sun.star.sheet.SpreadsheetDocument"_ustr, OUString()));
    }

    css::uno::Reference<scriptinterop::XPresentation> SAL_CALL getActivePresentation() override
    {
        return scriptinterop::detail::createPresentation(
            modelRequiring(u"com.sun.star.presentation.PresentationDocument"_ustr, OUString()));
    }

    css::uno::Reference<scriptinterop::XDrawing> SAL_CALL getActiveDrawing() override
    {
        return scriptinterop::detail::createDrawing(
            modelRequiring(u"com.sun.star.drawing.DrawingDocument"_ustr,
                           u"com.sun.star.presentation.PresentationDocument"_ustr));
    }
};
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
scriptinterop_Cool_get_implementation(css::uno::XComponentContext*,
                                      cpo::uno::Sequence<cpo::uno::Any> const&)
{
    return cppu::acquire(new FactoryImpl);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
