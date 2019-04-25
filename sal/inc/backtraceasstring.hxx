/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace osl { namespace detail {

/// Build a debugging backtrace from current PC location.
OUString backtraceAsString(sal_uInt32 maxDepth);

} }


