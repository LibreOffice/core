/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <jsuno/detail/dllapi.hxx>
#include <rtl/ustring.hxx>

namespace jsuno
{
// @return JSON-stringified result (empty string for a value that JSON.stringify drops: `undefined`,
// a function, or a symbol)
//
// @throws css.script.provider.ScriptExceptionRaisedException
LO_DLLPUBLIC_JSUNO OUString execute(OUString const& script);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
