/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_TOOLS_CPUID_HXX
#define INCLUDED_TOOLS_CPUID_HXX

#include <sal/config.h>
#include <tools/toolsdllapi.h>

#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)) && defined(__SSE2__)
#define LO_SSE2_AVAILABLE 1
#elif defined(_MSC_VER)
#define LO_SSE2_AVAILABLE 1
#endif

namespace tools
{
namespace cpuid
{
    TOOLS_DLLPUBLIC bool hasSSE2();
    TOOLS_DLLPUBLIC bool hasHyperThreading();
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
