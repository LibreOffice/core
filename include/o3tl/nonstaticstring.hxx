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

#include <string>

#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

namespace o3tl
{
// Some test code requires OUString instances that do not have their SAL_STRING_STATIC_FLAG set; so
// they cannot be created from u"..."_ustr literals, but should rather be created through this
// function:
OUString nonStaticString(std::u16string_view s) { return OUStringBuffer(s).makeStringAndClear(); }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
