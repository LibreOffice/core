/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cstdint>
#include <cstdlib>
#include <osl/time.h>
#include <time.h>
#ifdef WIN32
#include <stdlib.h>
#endif

// FIXME: in-time we should move all timezone related code using ICU here.
void SAL_CALL osl_setTimezone(const char* pTimezone)
{
    if (pTimezone)
#ifdef WIN32
        _putenv_s("TZ", pTimezone);
#else
        setenv("TZ", pTimezone, 1);
#endif
}

void SAL_CALL osl_resetTimezone()
{
#ifdef WIN32
    _putenv_s("TZ", "");
#else
    unsetenv("TZ");
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
