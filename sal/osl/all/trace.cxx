/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <cstdarg>

#include "osl/diagnose.h"
#include "sal/detail/log.h"
#include "sal/log.hxx"

#include "logformat.hxx"

void osl_trace(char const * pszFormat, ...) {
    std::va_list args;
    va_start(args, pszFormat);
    osl::detail::logFormat(
        SAL_DETAIL_LOG_LEVEL_INFO, "legacy.osl", SAL_WHERE, pszFormat, args);
    va_end(args);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
