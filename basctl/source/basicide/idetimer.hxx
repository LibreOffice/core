/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <osl/time.h>
#include <rtl/ustring.hxx>

namespace basctl
{
/**
     * A simple RAII timer to measure the duration of an operation and log it.
     * Starts timing upon construction, logs the elapsed time upon destruction.
     */
class IdeTimer
{
public:
    explicit IdeTimer(const rtl::OUString& operationName);
    ~IdeTimer();
    sal_Int64 getElapsedTimeMs() const;

private:
    IdeTimer(const IdeTimer&) = delete;
    IdeTimer& operator=(const IdeTimer&) = delete;

    rtl::OUString m_sOperationName;
    TimeValue m_aStart;
};

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
