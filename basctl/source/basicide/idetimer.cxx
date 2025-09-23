/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL.2.0/.
 */

#include "idetimer.hxx"
#include <sal/log.hxx>
#include <rtl/string.hxx>

#define PERF_LOG_CHANNEL "basctl"

namespace basctl
{
IdeTimer::IdeTimer(const rtl::OUString& operationName)
    : m_sOperationName(operationName)
{
    osl_getSystemTime(&m_aStart);
}

IdeTimer::~IdeTimer()
{
    TimeValue aEnd;
    osl_getSystemTime(&aEnd);

    sal_uInt32 seconds = aEnd.Seconds - m_aStart.Seconds;
    sal_Int32 nanos = aEnd.Nanosec - m_aStart.Nanosec;

    double fElapsedSeconds = static_cast<double>(seconds) + static_cast<double>(nanos) / 1e9;

    // Log the result.
    SAL_INFO(PERF_LOG_CHANNEL, "Operation '"
                                   << OUStringToOString(m_sOperationName, RTL_TEXTENCODING_UTF8)
                                   << "' took " << fElapsedSeconds << " seconds.");
}

sal_Int64 IdeTimer::getElapsedTimeMs() const
{
    TimeValue aEnd;
    osl_getSystemTime(&aEnd);

    sal_Int64 nSeconds = static_cast<sal_Int64>(aEnd.Seconds) - m_aStart.Seconds;
    sal_Int64 nNanos = static_cast<sal_Int64>(aEnd.Nanosec) - m_aStart.Nanosec;

    return (nSeconds * 1000) + (nNanos / 1000000);
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
