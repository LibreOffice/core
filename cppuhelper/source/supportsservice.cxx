/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <cassert>

#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "cppuhelper/supportsservice.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace {

namespace css = com::sun::star;

}

bool cppu::supportsService(
    css::lang::XServiceInfo * implementation, rtl::OUString const & name)
{
    assert(implementation != 0);
    css::uno::Sequence< rtl::OUString > s(
        implementation->getSupportedServiceNames());
    for (sal_Int32 i = 0; i != s.getLength(); ++i) {
        if (s[i] == name) {
            return true;
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
