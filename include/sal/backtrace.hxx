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

#include "sal/config.h"
#include "sal/saldllapi.h"
#include "sal/types.h"
#include "rtl/ustring.hxx"
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

namespace sal
{
struct BacktraceState
{
    void** buffer;
    int nDepth;
    ~BacktraceState() { delete[] buffer; }
};

SAL_DLLPUBLIC std::unique_ptr<BacktraceState> backtrace_get(sal_uInt32 backtraceDepth);

SAL_DLLPUBLIC OUString backtrace_to_string(BacktraceState* backtraceState);
}

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
