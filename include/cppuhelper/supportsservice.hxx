/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CPPUHELPER_SUPPORTSSERVICE_HXX
#define INCLUDED_CPPUHELPER_SUPPORTSSERVICE_HXX

#include "sal/config.h"

#include "cppuhelper/cppuhelperdllapi.h"

namespace com { namespace sun { namespace star { namespace lang {
    class XServiceInfo;
} } } }
namespace rtl { class OUString; }

namespace cppu {

/** A helper for implementations of com.sun.star.lang.XServiceInfo.

    This function is supposed to be called from implementations of
    css::lang::XServiceInfo::supportsService (and therefore, for
    easier coding takes the caller's this pointer by pointer rather than by
    css::uno::Reference).

    @param implementation points to the service implementation whose
    getSupportedServices method is consulted; must be non-null

    @param name the service name to test

    @return true iff the sequence returned by the given implementation's
    getSupportedServices method contains the given name

    @since LibreOffice 4.0
*/
bool CPPUHELPER_DLLPUBLIC supportsService(
    css::lang::XServiceInfo * implementation,
    rtl::OUString const & name);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
