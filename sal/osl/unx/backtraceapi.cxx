/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <rtl/ustring.hxx>
#include "backtraceasstring.hxx"

// FIXME: no-op for now; it needs implementing, cf. above.
OUString osl::detail::backtraceAsString(int /*maxNoStackFramesToDisplay*/)
{
    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
