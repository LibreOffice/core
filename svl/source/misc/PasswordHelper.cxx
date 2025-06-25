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


#include <svl/PasswordHelper.hxx>
#include <comphelper/hash.hxx>
#include <rtl/digest.h>
#include <memory>
#include <unicode/regex.h>
#include <unicode/unistr.h>
#include <unicode/errorcode.h>
#include <zxcvbn.h>
#include <sal/log.hxx>

using namespace com::sun::star;

void SvPasswordHelper::GetHashPasswordSHA256(uno::Sequence<sal_Int8>& rPassHash, std::u16string_view rPassword)
{
    OString const tmp(OUStringToOString(rPassword, RTL_TEXTENCODING_UTF8));
    ::std::vector<unsigned char> const hash(::comphelper::Hash::calculateHash(
        tmp.getStr(), tmp.getLength(),
        ::comphelper::HashType::SHA256));
    rPassHash.realloc(hash.size());
    ::std::copy(hash.begin(), hash.end(), rPassHash.getArray());
    rtl_secureZeroMemory(const_cast<char *>(tmp.getStr()), tmp.getLength());
}

void SvPasswordHelper::GetHashPasswordSHA1UTF8(uno::Sequence<sal_Int8>& rPassHash, std::u16string_view rPassword)
{
    OString const tmp(OUStringToOString(rPassword, RTL_TEXTENCODING_UTF8));
    ::std::vector<unsigned char> const hash(::comphelper::Hash::calculateHash(
        tmp.getStr(), tmp.getLength(),
        ::comphelper::HashType::SHA1));
    rPassHash.realloc(hash.size());
    ::std::copy(hash.begin(), hash.end(), rPassHash.getArray());
    rtl_secureZeroMemory(const_cast<char *>(tmp.getStr()), tmp.getLength());
}

void SvPasswordHelper::GetHashPassword(uno::Sequence<sal_Int8>& rPassHash, const char* pPass, sal_uInt32 nLen)
{
    rPassHash.realloc(RTL_DIGEST_LENGTH_SHA1);

    rtlDigestError aError = rtl_digest_SHA1 (pPass, nLen, reinterpret_cast<sal_uInt8*>(rPassHash.getArray()), rPassHash.getLength());
    if (aError != rtl_Digest_E_None)
    {
        rPassHash.realloc(0);
    }
}

void SvPasswordHelper::GetHashPasswordLittleEndian(uno::Sequence<sal_Int8>& rPassHash, std::u16string_view sPass)
{
    sal_Int32 nSize(sPass.size());
    std::unique_ptr<char[]> pCharBuffer(new char[nSize * sizeof(sal_Unicode)]);

    for (sal_Int32 i = 0; i < nSize; ++i)
    {
        sal_Unicode ch(sPass[ i ]);
        pCharBuffer[2 * i] = static_cast< char >(ch & 0xFF);
        pCharBuffer[2 * i + 1] = static_cast< char >(ch >> 8);
    }

    GetHashPassword(rPassHash, pCharBuffer.get(), nSize * sizeof(sal_Unicode));
    rtl_secureZeroMemory(pCharBuffer.get(), nSize * sizeof(sal_Unicode));
}

void SvPasswordHelper::GetHashPasswordBigEndian(uno::Sequence<sal_Int8>& rPassHash, std::u16string_view sPass)
{
    sal_Int32 nSize(sPass.size());
    std::unique_ptr<char[]> pCharBuffer(new char[nSize * sizeof(sal_Unicode)]);

    for (sal_Int32 i = 0; i < nSize; ++i)
    {
        sal_Unicode ch(sPass[ i ]);
        pCharBuffer[2 * i] = static_cast< char >(ch >> 8);
        pCharBuffer[2 * i + 1] = static_cast< char >(ch & 0xFF);
    }

    GetHashPassword(rPassHash, pCharBuffer.get(), nSize * sizeof(sal_Unicode));
    rtl_secureZeroMemory(pCharBuffer.get(), nSize * sizeof(sal_Unicode));
}

void SvPasswordHelper::GetHashPassword(uno::Sequence<sal_Int8>& rPassHash, std::u16string_view sPass)
{
    GetHashPasswordLittleEndian(rPassHash, sPass);
}

bool SvPasswordHelper::CompareHashPassword(const uno::Sequence<sal_Int8>& rOldPassHash, std::u16string_view sNewPass)
{
    bool bResult = false;

    if (rOldPassHash.getLength() == RTL_DIGEST_LENGTH_SHA1)
    {
        uno::Sequence<sal_Int8> aNewPass(RTL_DIGEST_LENGTH_SHA1);
        GetHashPasswordSHA1UTF8(aNewPass, sNewPass);
        if (aNewPass == rOldPassHash)
        {
            bResult = true;
        }
        else
        {
            GetHashPasswordLittleEndian(aNewPass, sNewPass);
            if (aNewPass == rOldPassHash)
                bResult = true;
            else
            {
                GetHashPasswordBigEndian(aNewPass, sNewPass);
                bResult = (aNewPass == rOldPassHash);
            }
        }
    }
    else if (rOldPassHash.getLength() == 32)
    {
        uno::Sequence<sal_Int8> aNewPass;
        GetHashPasswordSHA256(aNewPass, sNewPass);
        bResult = aNewPass == rOldPassHash;
    }

    return bResult;
}

double SvPasswordHelper::GetPasswordStrengthPercentage(const char* pPassword)
{
    // Entropy bits ≥ 112 are mapped to 100% password strength.
    // 112 was picked since according to the linked below KeePass help page, it
    // corresponds to a strong password:
    // <http://web.archive.org/web/20231128131604/https://keepass.info/help/kb/pw_quality_est.html>
    static constexpr double fMaxPassStrengthEntropyBits = 112.0;
    return std::min(100.0,
                    ZxcvbnMatch(pPassword, nullptr, nullptr) * 100.0 / fMaxPassStrengthEntropyBits);
}

double SvPasswordHelper::GetPasswordStrengthPercentage(const OUString& aPassword)
{
    OString aPasswordUtf8 = aPassword.toUtf8();
    return GetPasswordStrengthPercentage(aPasswordUtf8.getStr());
}

bool SvPasswordHelper::PasswordMeetsPolicy(const char* pPassword,
                                           const std::optional<OUString>& oPasswordPolicy)
{
    if (oPasswordPolicy)
    {
        icu::ErrorCode aStatus;
        icu::UnicodeString sPassword(pPassword);
        icu::UnicodeString sRegex(oPasswordPolicy->getStr());
        icu::RegexMatcher aRegexMatcher(sRegex, sPassword, 0, aStatus);

        if (aRegexMatcher.matches(aStatus))
            return true;

        SAL_WARN_IF(
            aStatus.isFailure(), "svl.misc",
            "Password policy regular expression failed with error: " << aStatus.errorName());

        return false;
    }
    return true;
}

bool SvPasswordHelper::PasswordMeetsPolicy(const OUString& aPassword,
                                           const std::optional<OUString>& oPasswordPolicy)
{
    OString aPasswordUtf8 = aPassword.toUtf8();
    return PasswordMeetsPolicy(aPasswordUtf8.getStr(), oPasswordPolicy);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
