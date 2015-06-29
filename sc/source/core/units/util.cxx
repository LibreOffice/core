/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#include "util.hxx"

#include <udunits2/udunits2.h>

namespace sc {
namespace units {

OUString getUTStatus()
{
    switch(ut_get_status())
    {
    case UT_SUCCESS:
        return OUString("UT_SUCCESS: successful!");
    case UT_BAD_ARG:
        return OUString("UT_BAD_ARG: invalid argument");
    case UT_EXISTS:
        return OUString("UT_EXISTS: unit/prefix/identifier already exists");
    case UT_NO_UNIT:
        return OUString("UT_NO_UNIT: no such unit exists");
    case UT_OS:
        return OUString("UT_OS: operating system error (check errno?)");
    case UT_NOT_SAME_SYSTEM:
        return OUString("UT_NOT_SAME_SYSTEM: units not in same unit system");
    case UT_MEANINGLESS:
        return OUString("UT_MEANINGLESS: operation is meaningless");
    case UT_NO_SECOND:
        return OUString("UT_NO_SECOND: no unit named second");
    case UT_VISIT_ERROR:
        return OUString("UT_VISIT_ERROR");
    case UT_CANT_FORMAT:
        return OUString("UT_CANT_FORMAT");
    case UT_SYNTAX:
        return OUString("UT_SYNTAX: syntax error in unit string");
    case UT_UNKNOWN:
        return OUString("UT_UNKNOWN: unknown unit encountered");
    case UT_OPEN_ARG:
        return OUString("UT_OPEN_ARG: can't open specified unit database (arg)");
    case UT_OPEN_ENV:
        return OUString("UT_OPEN_ENV: can't open specified unit databse (env)");
    case UT_OPEN_DEFAULT:
        return OUString("UT_OPEN_DEFAULT: can't open default unit database");
    default:
        return OUString("other (unspecified) error encountered");
    }
}

}} // namespace sc::units

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
