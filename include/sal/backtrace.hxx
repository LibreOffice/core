/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SAL_BACKTRACE_HXX
#define INCLUDED_SAL_BACKTRACE_HXX

#include <sal/config.h>
#include <sal/saldllapi.h>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <memory>

/// @cond INTERNAL
/**
  Two stage API for recording and then later decoding stack backtraces.
  Useful for debugging facilities where we are only interested in decoding
  a small handful of recorded stack traces.

  @param backtraceDepth value indicating the maximum backtrace depth; must be > 0

  @since LibreOffice 6.0
*/
#if defined LIBO_INTERNAL_ONLY

namespace sal { namespace detail {

struct BacktraceState {
    void** buffer;
    int nDepth;
    ~BacktraceState() {delete[] buffer;}
};

} }

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
#endif

extern "C" SAL_DLLPUBLIC std::unique_ptr<sal::detail::BacktraceState>
sal_detail_backtrace_get(sal_uInt32 backtraceDepth);

extern "C" SAL_DLLPUBLIC OUString sal_detail_backtrace_to_string(
    sal::detail::BacktraceState* backtraceState);

#if defined __clang__
#pragma clang diagnostic pop
#endif

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
