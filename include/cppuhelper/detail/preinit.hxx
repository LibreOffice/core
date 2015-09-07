/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CPPUHELPER_DETAIL_PREINIT_HXX
#define INCLUDED_CPPUHELPER_DETAIL_PREINIT_HXX

#include <cppuhelper/cppuhelperdllapi.h>

namespace cppu
{

#if defined LIBO_INTERNAL_ONLY

/** Preload all shared library components with service manager upon
    information from bootstrap variables.

    This function tries to find its parameters via these bootstrap variables:

      - UNO_SERVICES      -- a space separated list of file urls of service rdbs

*/
CPPUHELPER_DLLPUBLIC void SAL_CALL
preInitBootstrap(css::uno::Reference< css::uno::XComponentContext > const & xContext);

#endif // LIBO_INTERNAL_ONLY

} // namespace cppu

#endif // INCLUDED_CPPUHELPER_DETAIL_PREINIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
