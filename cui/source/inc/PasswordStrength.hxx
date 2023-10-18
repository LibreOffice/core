/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/** Get password strength percentage

    Maps the received password entropy bits to password strength percentage.
    0 bits      -> 0%
    >= 112 bits -> 100%

    @param pPassword null terminated password string.
    @returns Password strength percentage in the range [0.0, 100.0]
*/
double getPasswordStrengthPercentage(const char* pPassword);

/** Checks if the password meets the password policies

    @param pPassword null terminated password string.
    @returns true if password meets the policy or there is no policy enforced.
*/
bool passwordCompliesPolicy(const char* pPassword);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
