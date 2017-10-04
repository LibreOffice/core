/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "rtfdocumentimpl.hxx"

namespace writerfilter
{
namespace rtftok
{

RTFDocument::Pointer_t RTFDocumentFactory::createDocument(css::uno::Reference< css::uno::XComponentContext > const& xContext,
        css::uno::Reference< css::io::XInputStream > const& xInputStream,
        css::uno::Reference< css::lang::XComponent > const& xDstDoc,
        css::uno::Reference< css::frame::XFrame > const& xFrame,
        css::uno::Reference< css::task::XStatusIndicator > const& xStatusIndicator,
        const utl::MediaDescriptor& rMediaDescriptor)
{
    return std::make_shared<RTFDocumentImpl>(xContext, xInputStream, xDstDoc, xFrame, xStatusIndicator, rMediaDescriptor);
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
