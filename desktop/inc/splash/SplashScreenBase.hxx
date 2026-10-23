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

#include <rtl/ustring.hxx>
#include <sal/types.h>

/** Abstract base class for splash screen implementations. */
class SplashScreenBase
{
public:
    virtual ~SplashScreenBase() {}

    virtual void start(const OUString& rText, sal_Int32 nRange) = 0;
    virtual void end() = 0;
    virtual void setText(const OUString& rText) = 0;
    virtual void setValue(sal_Int32 nValue) = 0;
    virtual void reset() = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
