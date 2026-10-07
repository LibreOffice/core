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

/*
 * Convert Calc AddIn Anys ↔ dumb JSON (numbers / strings / null / nested lists).
 */

#pragma once

#include <cpo/uno/Any.hxx>
#include <cpo/uno/Sequence.hxx>
#include <formula/errorcodes.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <string>
#include <string_view>

namespace co::pythoncompute
{
/** Cell formula error for ScUnoAddInCall::SetResult: void → #N/A; else CreateDoubleError. */
SAL_DLLPUBLIC_EXPORT cpo::uno::Any makeFormulaErrorAny(FormulaError e);

/** Flatten 1×N / N×1 grids to 1D like Classic normalize_python_data_shape. */
SAL_DLLPUBLIC_EXPORT OUString anyToJsonFragment(const cpo::uno::Any& aValue);

/** Build body: {"id","code","mode", optional "data"}. */
SAL_DLLPUBLIC_EXPORT std::string
buildExecuteRequestJson(const OUString& sRequestId, const OUString& sCode,
                        const cpo::uno::Sequence<cpo::uno::Any>& aData);

/** How a service result envelope was classified. Every kind sets rOut to a cell value. */
enum class ResultKind
{
    Ok, ///< a result value
    ServiceError, ///< the user's code failed, or its reply cannot be parsed: deterministic,
                  ///< cached like Ok
    TransientError, ///< coolwsd/transport error ("transient":true) or disabled: retried later
};

/** Parse service result JSON into an Any suitable for ScUnoAddInCall::SetResult. */
SAL_DLLPUBLIC_EXPORT ResultKind jsonResultToAny(std::string_view jsonUtf8, cpo::uno::Any& rOut,
                                          OUString& rError);

/** Extract the string "id" field from a JSON object body. */
SAL_DLLPUBLIC_EXPORT bool extractRequestIdFromJson(std::string_view jsonUtf8, OUString& rId);
} // namespace co::pythoncompute

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
