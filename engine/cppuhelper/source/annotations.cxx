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

#include <sal/config.h>

#include <cassert>
#include <string_view>
#include <vector>

#include <com/sun/star/uno/Reference.hxx>
#include <cpo/uno/XComponentContext.hpp>
#include <cpo/uno/XInterface.hpp>
#include <cppuhelper/annotations.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include "typemanager.hxx"

std::vector<OUString> cppuhelper::getInterfaceMethodAnnotations(
    css::uno::Reference<cpo::uno::XComponentContext> const & context,
    OUString const & interfaceName, std::u16string_view methodName)
{
    rtl::Reference const mgr(
        dynamic_cast<TypeManager *>(
            css::uno::Reference<cpo::uno::XInterface>(
                context->getValueByName(
                    u"/singletons/com.sun.star.reflection.theTypeDescriptionManager"_ustr),
                css::uno::UNO_QUERY_THROW).get()));
    assert(mgr.is());
    return mgr->getInterfaceMethodAnnotations(interfaceName, methodName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
