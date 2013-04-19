/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtfdocumentimpl.hxx>

namespace writerfilter {
namespace rtftok {

RTFDocument::Pointer_t RTFDocumentFactory::createDocument(uno::Reference< uno::XComponentContext > const & xContext,
        uno::Reference< io::XInputStream > const & xInputStream,
        uno::Reference< lang::XComponent > const & xDstDoc,
        uno::Reference< frame::XFrame > const & xFrame,
        uno::Reference< task::XStatusIndicator > const & xStatusIndicator)
{
    return RTFDocument::Pointer_t(new RTFDocumentImpl(xContext, xInputStream, xDstDoc, xFrame, xStatusIndicator));
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
