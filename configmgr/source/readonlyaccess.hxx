/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CONFIGMGR_SOURCE_READONLYACCESS_HXX
#define INCLUDED_CONFIGMGR_SOURCE_READONLYACCESS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <sal/types.h>

namespace com { namespace sun { namespace star {
    namespace uno {
        class XComponentContext;
        class XInterface;
    }
} } }

namespace configmgr { namespace read_only_access {

css::uno::Reference< css::uno::XInterface >
create(
    css::uno::Reference< css::uno::XComponentContext >
        const &);

OUString getImplementationName();

css::uno::Sequence< OUString >
getSupportedServiceNames();

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
