/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <chrono>
#include <cstring>
#include <thread>

#include <rtl/bootstrap.hxx>
#include <rtl/random.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/uri.hxx>
#include <osl/file.hxx>
#include <osl/security.hxx>
#include <osl/thread.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <osl/process.h>

#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/bridge/UnoUrlResolver.hpp>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>

#include "macro_expander.hxx"

using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using rtl::Bootstrap;

namespace cppu
{


OUString bootstrap_expandUri(OUString const & uri) {
    OUString rest;
    return uri.startsWith("vnd.sun.star.expand:", &rest)
        ? cppuhelper::detail::expandMacros(
            rtl::Uri::decode(
                rest, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8))
        : uri;
}

} // namespace cppu

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
