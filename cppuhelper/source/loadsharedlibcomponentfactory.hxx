/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CPPUHELPER_SOURCE_LOADSHAREDLIBCOMPONENTFACTORY_HXX
#define INCLUDED_CPPUHELPER_SOURCE_LOADSHAREDLIBCOMPONENTFACTORY_HXX

#include <sal/config.h>

#include <com/sun/star/uno/Reference.hxx>

#include "servicemanager.hxx"

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace uno {
        class Environment;
        class XInterface;
    }
} } }

namespace cppuhelper { namespace detail {

css::uno::Environment getEnvironment(
    OUString const & name, OUString const & implementation);

void loadSharedLibComponentFactory(
    OUString const & uri, OUString const & environment,
    OUString const & prefix, OUString const & implementation,
    OUString const & constructor,
    css::uno::Reference<css::lang::XMultiServiceFactory> const & serviceManager,
    WrapperConstructorFn * constructorFunction,
    css::uno::Reference<css::uno::XInterface> * factory);

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
