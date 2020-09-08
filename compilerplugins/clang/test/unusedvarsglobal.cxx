/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "config_clang.h"

// CLANG_VERSION = older versions of clang need something different in getParentFunctionDecl
// WIN32 = TODO, see corresponding TODO in compilerplugins/clang/unusedfields.cxx
#if CLANG_VERSION < 110000 || defined _WIN32
// expected-no-diagnostics
#else

#include <rtl/ustring.hxx>

namespace something
{
// expected-error@+1 {{write [loplugin:unusedvarsglobal]}}
extern const OUStringLiteral literal1;
}
const OUStringLiteral something::literal1(u"xxx");

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
