/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <chrono>
#include <iostream>

struct ScopedNanoTimer
{
    OString msName;
    sal_Int64 mnIgnoreTimeMiliseconds;
    std::chrono::high_resolution_clock::time_point maStart;

    ScopedNanoTimer(OString const& sName, sal_Int32 nIgnoreTimeMiliseconds = 10)
        : msName(sName)
        , mnIgnoreTimeMiliseconds(nIgnoreTimeMiliseconds)
        , maStart(std::chrono::high_resolution_clock::now())
    {
    }

    ~ScopedNanoTimer()
    {
        auto aDurationInMilliSeconds = stop();
        if (aDurationInMilliSeconds > mnIgnoreTimeMiliseconds)
            std::cout << msName << " time : " << aDurationInMilliSeconds << " ms" << std::endl;
    }

    sal_Int64 stop()
    {
        auto aEnd = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(aEnd - maStart).count();
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
