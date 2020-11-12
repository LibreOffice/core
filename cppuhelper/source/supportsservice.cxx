/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <algorithm>
#include <cassert>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ustring.hxx>

bool cppu::supportsService(css::lang::XServiceInfo* implementation, OUString const& name)
{
    assert(implementation != nullptr);
    const css::uno::Sequence<OUString> s(implementation->getSupportedServiceNames());
    return std::find(s.begin(), s.end(), name) != s.end();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
