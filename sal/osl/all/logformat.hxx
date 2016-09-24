/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SAL_OSL_ALL_LOGFORMAT_HXX
#define INCLUDED_SAL_OSL_ALL_LOGFORMAT_HXX

#include "sal/config.h"

#include <cstdarg>

#include "sal/detail/log.h"

namespace osl { namespace detail {

void logFormat(
    sal_detail_LogLevel level, char const * area, char const * where,
    char const * format, std::va_list arguments);

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
