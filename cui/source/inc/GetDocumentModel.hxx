/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <com/sun/star/uno/Reference.hxx>

namespace com::sun::star::uno
{
class XComponentContext;
}

namespace com::sun::star::frame
{
class XModel;
}

css::uno::Reference<css::frame::XModel>
getDocumentModel(const css::uno::Reference<css::uno::XComponentContext>& xContext,
                 std::u16string_view sDocName);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
