/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>

#if defined(__COVERITY__)
#define THREAD_UNSAFE_DUMP_BEGIN                                                                   \
    _Pragma(                                                                                       \
        "coverity compliance block deviate MISSING_LOCK \"Intentionally thread-unsafe dumping\"")
#define THREAD_UNSAFE_DUMP_END _Pragma("coverity compliance end_block MISSING_LOCK")
#else
#define THREAD_UNSAFE_DUMP_BEGIN
#define THREAD_UNSAFE_DUMP_END
#endif

namespace rtl
{
class OStringBuffer;
}

class VCL_DLLPUBLIC CacheOwner
{
protected:
    CacheOwner();
    virtual ~CacheOwner();

public:
    virtual void dropCaches() = 0;
    virtual void dumpState(rtl::OStringBuffer& rState) = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
