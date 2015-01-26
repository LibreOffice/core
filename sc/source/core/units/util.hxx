/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#ifndef INCLUDED_SC_SOURCE_CORE_UNITS_UTIL_HXX
#define INCLUDED_SC_SOURCE_CORE_UNITS_UTIL_HXX

#include <rtl/ustring.hxx>

namespace sc {
    namespace units {
        /*
         * Get an english text description for ut_get_status().
         * Should only be used for internal debugging purposes.
         */
        OUString getUTStatus();
    }
}

#endif // INCLUDED_SC_SOURCE_CORE_UNITS_UTIL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
