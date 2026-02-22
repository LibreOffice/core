/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVL_PASSWORDHELPER_HXX
#define INCLUDED_SVL_PASSWORDHELPER_HXX

#include <sal/config.h>

#include <string_view>
#include <optional>

#include <rtl/ustring.hxx>
#include <svl/svldllapi.h>
#include <sal/types.h>

namespace com::sun::star::uno { template <class E> class Sequence; }

class SvPasswordHelper
{
    static void     GetHashPasswordLittleEndian(css::uno::Sequence<sal_Int8>& rPassHash, std::u16string_view sPass);
    static void     GetHashPasswordBigEndian(css::uno::Sequence<sal_Int8>& rPassHash, std::u16string_view sPass);

public:
    SVL_DLLPUBLIC static void     GetHashPassword(css::uno::Sequence <sal_Int8>& rPassHash, const char* pPass, sal_uInt32 nLen);

    SVL_DLLPUBLIC static void     GetHashPassword(css::uno::Sequence<sal_Int8>& rPassHash, std::u16string_view sPass);
    SVL_DLLPUBLIC static void     GetHashPasswordSHA1UTF8(css::uno::Sequence<sal_Int8>& rPassHash, std::u16string_view sPass);
    SVL_DLLPUBLIC static void     GetHashPasswordSHA256(css::uno::Sequence<sal_Int8>& rPassHash, std::u16string_view sPass);
    /**
    Use this method to compare a given string with another given Hash value.
    This is necessary, because in older versions exists different hashes of the same string. They were endian dependent.
    We need this to handle old files. This method will compare against big and
    little endian UTF-16.
    tdf#115483: also check 2 different new ways of hashing that were added in
    ODF 1.2, requiring UTF-8 encoding.
    */
    SVL_DLLPUBLIC static bool     CompareHashPassword(const css::uno::Sequence<sal_Int8>& rOldPassHash, std::u16string_view sNewPass);

    /** Get password strength percentage

    Maps the calculated password entropy bit amount to password strength percentage:
    0 bits      -> 0%
    >= 112 bits -> 100%

    @param pPassword null terminated password string.
    @returns Password strength percentage in the range [0.0, 100.0]
    */
    SVL_DLLPUBLIC static double GetPasswordStrengthPercentage(const char* pPassword);
    SVL_DLLPUBLIC static double GetPasswordStrengthPercentage(const OUString& aPassword);

    /** Checks if the password meets the password policies

    @param pPassword null terminated password string.
    @param oPasswordPolicy Regular expression string that defines the password policy.

    @returns true if password meets the policy or there is no policy enforced.
    */
    SVL_DLLPUBLIC static bool PasswordMeetsPolicy(const char* pPassword,
                                                  const std::optional<OUString>& oPasswordPolicy);
    SVL_DLLPUBLIC static bool PasswordMeetsPolicy(const OUString& aPassword,
                                                  const std::optional<OUString>& oPasswordPolicy);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
