/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

// A convenience guard class that captures a given object's value on guard construction and restores
// it on guard destruction:
namespace o3tl
{
template <typename T> class RestoreGuard
{
public:
    RestoreGuard(T& object)
        : object_(object)
        , value_(object)
    {
    }

    ~RestoreGuard() { object_ = value_; }

private:
    RestoreGuard(RestoreGuard&) = delete;
    RestoreGuard(RestoreGuard&&) = delete;
    void operator=(RestoreGuard&) = delete;
    void operator=(RestoreGuard&&) = delete;

    T& object_;
    T value_;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
