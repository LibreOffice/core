/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OSL_BACKTRACE_HXX
#define INCLUDED_OSL_BACKTRACE_HXX

#include <sal/config.h>
#include <sal/saldllapi.h>
#include <sal/log.hxx>
#include <rtl/ustring.hxx>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Build a debugging backtrace from current PC location.
 */
SAL_DLLPUBLIC rtl_uString *osl_backtraceAsString(void);

#ifdef __cplusplus
} // extern "C"

namespace osl
{

class Backtrace {
public:
    /**
       Return current stack trace as a string.

       @since LibreOffice 5.
    */
    static inline OUString asString()
    {
        return OUString(osl_backtraceAsString(), SAL_NO_ACQUIRE);
    }
    };
}

#endif // __cplusplus

#endif // INCLUDED_OSL_BACKTRACE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
