/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CPPUHELPER_SOURCE_LOADSHAREDLIBCOMPONENTFACTORY_HXX
#define CPPUHELPER_SOURCE_LOADSHAREDLIBCOMPONENTFACTORY_HXX

#include <sal/config.h>

#include <com/sun/star/uno/Reference.hxx>

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace uno { class XInterface; }
} } }
namespace rtl { class OUString; }

namespace cppuhelper { namespace detail {

css::uno::Reference<css::uno::XInterface> loadSharedLibComponentFactory(
    rtl::OUString const & uri, rtl::OUString const & prefix,
    rtl::OUString const & rImplName,
    css::uno::Reference<css::lang::XMultiServiceFactory> const & xMgr);

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
