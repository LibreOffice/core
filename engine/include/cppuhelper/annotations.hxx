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

#pragma once

#include "sal/config.h"

#include <string_view>
#include <vector>

#include "com/sun/star/uno/Reference.hxx"
#include "cppuhelper/cppuhelperdllapi.h"
#include "rtl/ustring.hxx"

namespace cpo::uno { class XComponentContext; }

namespace cppuhelper {

//TODO: Going directly to the underlying TypeManager; to be removed again once annotations are
// exposed through typelib:
CPPUHELPER_DLLPUBLIC std::vector<OUString> getInterfaceMethodAnnotations(
    css::uno::Reference<cpo::uno::XComponentContext> const & context,
    OUString const & interfaceName, std::u16string_view methodName);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
