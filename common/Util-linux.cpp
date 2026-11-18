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
#include <config.h>

#include "Util.hpp"

#ifdef __linux__
#include <sys/time.h>
#include <sys/resource.h>
#elif defined __FreeBSD__
#include <sys/resource.h>
#endif

#include <dirent.h>

#include <fstream>
#include <iomanip>
#include <unistd.h>

#include <Poco/Exception.h>
#include "Log.hpp"

namespace Util
{

class CounterImpl {
private:
    DIR* _dir = nullptr;

public:
    CounterImpl(const char* procPath)
        : _dir(opendir(procPath))
    {
        if (!_dir)
            LOG_ERR("No proc mounted for procPath " << procPath << ", can't count threads");
    }

    ~CounterImpl() { closedir(_dir); }

    int count()
    {
        if (!_dir)
            return -1;

        rewinddir(_dir);

        int tasks = 0;
        struct dirent* i;
        while ((i = readdir(_dir)))
        {
            if (i->d_name[0] != '.')
                tasks++;
        }

        return tasks;
    }
};

ThreadCounter::ThreadCounter() : _impl(new CounterImpl("/proc/self/task")) {}

ThreadCounter::~ThreadCounter() = default;

int ThreadCounter::count() { return _impl->count(); }

FDCounter::FDCounter() : _impl(new CounterImpl("/proc/self/fd")) {}

FDCounter::~FDCounter() = default;

int FDCounter::count() { return _impl->count(); }

} // namespace Util

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
