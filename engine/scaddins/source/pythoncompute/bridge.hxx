/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "volatile.hxx"

#include <cpo/uno/Any.hxx>
#include <cpo/uno/Sequence.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include <string>
#include <string_view>

namespace co::pythoncompute
{
/** Returns the volatile for these params: the cached one, or a new pending one whose request
 *  is queued for the kit (pythoncompute_take_requests). Caller must hold no lock but Solar. */
rtl::Reference<PythonComputeVolatileResult>
startCompute(const OUString& sCode, const cpo::uno::Sequence<cpo::uno::Any>& aData);

/** keyJson (built with the "_" id placeholder) with idKey as its id, without serializing
 *  the request again. */
SAL_DLLPUBLIC_EXPORT std::string withRequestId(std::string_view keyJson, std::string_view idKey);
} // namespace co::pythoncompute

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
