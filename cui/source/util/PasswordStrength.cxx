/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <PasswordStrength.hxx>
#include <officecfg/Office/Common.hxx>
#include <unicode/regex.h>
#include <unicode/unistr.h>
#include <unicode/errorcode.h>
#include <sal/log.hxx>
#include <zxcvbn.h>

double getPasswordStrengthPercentage(const char* pPassword)
{
    static constexpr double fMaxStrengthBits = 112.0;
    return std::min(100.0, ZxcvbnMatch(pPassword, nullptr, nullptr) * 100.0 / fMaxStrengthBits);
}

bool passwordCompliesPolicy(const char* pPassword)
{
    std::optional<OUString> oPasswordPolicy
        = officecfg::Office::Common::Security::Scripting::PasswordPolicy::get();
    if (oPasswordPolicy)
    {
        icu::ErrorCode aStatus;
        icu::UnicodeString sPassword(pPassword);
        icu::UnicodeString sRegex(oPasswordPolicy->getStr());
        icu::RegexMatcher aRegexMatcher(sRegex, sPassword, 0, aStatus);

        if (aRegexMatcher.matches(aStatus))
            return true;

        if (aStatus.isFailure())
        {
            SAL_INFO("cui.util", "Password policy regular expression failed with error: "
                                     << aStatus.errorName());
        }
        return false;
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
