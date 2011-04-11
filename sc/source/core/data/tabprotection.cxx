/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include "tabprotection.hxx"
#include "tools/debug.hxx"
#include "svl/PasswordHelper.hxx"
#include <comphelper/docpasswordhelper.hxx>
#include "document.hxx"

#include <vector>

#define DEBUG_TAB_PROTECTION 0

#define URI_SHA1 "http://www.w3.org/2000/09/xmldsig#sha1"
#define URI_XLS_LEGACY "http://docs.oasis-open.org/office/ns/table/legacy-hash-excel"

using namespace ::com::sun::star;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::std::vector;

// ============================================================================

bool ScPassHashHelper::needsPassHashRegen(const ScDocument& rDoc, ScPasswordHash eHash1, ScPasswordHash eHash2)
{
    if (rDoc.IsDocProtected())
    {
        const ScDocProtection* p = rDoc.GetDocProtection();
        if (!p->isPasswordEmpty() && !p->hasPasswordHash(eHash1, eHash2))
            return true;
    }

    SCTAB nTabCount = rDoc.GetTableCount();
    for (SCTAB i = 0; i < nTabCount; ++i)
    {
        const ScTableProtection* p = rDoc.GetTabProtection(i);
        if (!p || !p->isProtected())
            // Sheet not protected.  Skip it.
            continue;

        if (!p->isPasswordEmpty() && !p->hasPasswordHash(eHash1, eHash2))
            return true;
    }

    return false;
}

OUString ScPassHashHelper::getHashURI(ScPasswordHash eHash)
{
    switch (eHash)
    {
        case PASSHASH_SHA1:
            return OUString(RTL_CONSTASCII_USTRINGPARAM(URI_SHA1));
        case PASSHASH_XL:
            return OUString(RTL_CONSTASCII_USTRINGPARAM(URI_XLS_LEGACY));
        case PASSHASH_UNSPECIFIED:
        default:
            ;
    }
    return OUString();
}

ScPasswordHash ScPassHashHelper::getHashTypeFromURI(const OUString& rURI)
{
    if (rURI.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(URI_SHA1)))
        return PASSHASH_SHA1;
    else if (rURI.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(URI_XLS_LEGACY)))
        return PASSHASH_XL;
    return PASSHASH_UNSPECIFIED;
}

// ============================================================================

ScPassHashProtectable::~ScPassHashProtectable()
{
}

// ============================================================================

class ScTableProtectionImpl
{
public:
    static Sequence<sal_Int8> hashPassword(const String& aPassText, ScPasswordHash eHash = PASSHASH_SHA1);
    static Sequence<sal_Int8> hashPassword(const Sequence<sal_Int8>& rPassHash, ScPasswordHash eHash = PASSHASH_SHA1);

    explicit ScTableProtectionImpl(SCSIZE nOptSize);
    explicit ScTableProtectionImpl(const ScTableProtectionImpl& r);

    bool isProtected() const;
    bool isProtectedWithPass() const;
    void setProtected(bool bProtected);

    bool isPasswordEmpty() const;
    bool hasPasswordHash(ScPasswordHash eHash, ScPasswordHash eHash2 = PASSHASH_UNSPECIFIED) const;
    void setPassword(const String& aPassText);
    ::com::sun::star::uno::Sequence<sal_Int8> getPasswordHash(
        ScPasswordHash eHash, ScPasswordHash eHash2 = PASSHASH_UNSPECIFIED) const;
    void setPasswordHash(
        const ::com::sun::star::uno::Sequence<sal_Int8>& aPassword,
        ScPasswordHash eHash = PASSHASH_SHA1, ScPasswordHash eHash2 = PASSHASH_UNSPECIFIED);
    bool verifyPassword(const String& aPassText) const;

    bool isOptionEnabled(SCSIZE nOptId) const;
    void setOption(SCSIZE nOptId, bool bEnabled);

private:
    String maPassText;
    ::com::sun::star::uno::Sequence<sal_Int8>   maPassHash;
    ::std::vector<bool> maOptions;
    bool mbEmptyPass;
    bool mbProtected;
    ScPasswordHash meHash1;
    ScPasswordHash meHash2;
};

Sequence<sal_Int8> ScTableProtectionImpl::hashPassword(const String& aPassText, ScPasswordHash eHash)
{
    Sequence<sal_Int8> aHash;
    switch (eHash)
    {
        case PASSHASH_XL:
            aHash = ::comphelper::DocPasswordHelper::GetXLHashAsSequence( aPassText, RTL_TEXTENCODING_UTF8 );
        break;
        case PASSHASH_SHA1:
            SvPasswordHelper::GetHashPassword(aHash, aPassText);
        break;
        default:
            ;
    }
    return aHash;
}

Sequence<sal_Int8> ScTableProtectionImpl::hashPassword(
    const Sequence<sal_Int8>& rPassHash, ScPasswordHash eHash)
{
    if (!rPassHash.getLength() || eHash == PASSHASH_UNSPECIFIED)
        return rPassHash;

    // TODO: Right now, we only support double-hash by SHA1.
    if (eHash == PASSHASH_SHA1)
    {
        vector<sal_Char> aChars;
        sal_Int32 n = rPassHash.getLength();
        aChars.reserve(n);
        for (sal_Int32 i = 0; i < n; ++i)
            aChars.push_back(static_cast<sal_Char>(rPassHash[i]));

        Sequence<sal_Int8> aNewHash;
        SvPasswordHelper::GetHashPassword(aNewHash, &aChars[0], aChars.size());
        return aNewHash;
    }

    return rPassHash;
}

ScTableProtectionImpl::ScTableProtectionImpl(SCSIZE nOptSize) :
    maOptions(nOptSize),
    mbEmptyPass(true),
    mbProtected(false),
    meHash1(PASSHASH_SHA1),
    meHash2(PASSHASH_UNSPECIFIED)
{
}

ScTableProtectionImpl::ScTableProtectionImpl(const ScTableProtectionImpl& r) :
    maPassText(r.maPassText),
    maPassHash(r.maPassHash),
    maOptions(r.maOptions),
    mbEmptyPass(r.mbEmptyPass),
    mbProtected(r.mbProtected),
    meHash1(r.meHash1),
    meHash2(r.meHash2)
{
}

bool ScTableProtectionImpl::isProtected() const
{
    return mbProtected;
}

bool ScTableProtectionImpl::isProtectedWithPass() const
{
    if (!mbProtected)
        return false;

    return maPassText.Len() || maPassHash.getLength();
}

void ScTableProtectionImpl::setProtected(bool bProtected)
{
    mbProtected = bProtected;
    // We need to keep the old password even when the protection is off.  So,
    // don't erase the password data here.
}

void ScTableProtectionImpl::setPassword(const String& aPassText)
{
    // We can't hash it here because we don't know whether this document will
    // get saved to Excel or ODF, depending on which we will need to use a
    // different hashing algorithm.  One alternative is to hash it using all
    // hash algorithms that we support, and store them all.

    maPassText = aPassText;
    mbEmptyPass = aPassText.Len() == 0;
    if (mbEmptyPass)
    {
        maPassHash = Sequence<sal_Int8>();
    }
}

bool ScTableProtectionImpl::isPasswordEmpty() const
{
    return mbEmptyPass;
}

bool ScTableProtectionImpl::hasPasswordHash(ScPasswordHash eHash, ScPasswordHash eHash2) const
{
    if (mbEmptyPass)
        return true;

    if (maPassText.Len())
        return true;

    if (meHash1 == eHash)
    {
        if (meHash2 == PASSHASH_UNSPECIFIED)
            // single hash.
            return true;

        return meHash2 == eHash2;
    }

    return false;
}

Sequence<sal_Int8> ScTableProtectionImpl::getPasswordHash(
    ScPasswordHash eHash, ScPasswordHash eHash2) const
{
    Sequence<sal_Int8> aPassHash;

    if (mbEmptyPass)
        // Flaged as empty.
        return aPassHash;

    if (maPassText.Len())
    {
        // Cleartext password exists.  Hash it.
        aPassHash = hashPassword(maPassText, eHash);
        if (eHash2 != PASSHASH_UNSPECIFIED)
            // Double-hash it.
            aPassHash = hashPassword(aPassHash, eHash2);

        return aPassHash;
    }
    else
    {
        // No clear text password.  Check if we have a hash value of the right hash type.
        if (meHash1 == eHash)
        {
            aPassHash = maPassHash;

            if (meHash2 == eHash2)
                // Matching double-hash requested.
                return aPassHash;
            else if (meHash2 == PASSHASH_UNSPECIFIED)
                // primary hashing type match.  Double hash it by the requested
                // double-hash type.
                return hashPassword(aPassHash, eHash2);
        }
    }

    // failed.
    return Sequence<sal_Int8>();
}

void ScTableProtectionImpl::setPasswordHash(
    const uno::Sequence<sal_Int8>& aPassword, ScPasswordHash eHash, ScPasswordHash eHash2)
{
    sal_Int32 nLen = aPassword.getLength();
    mbEmptyPass = nLen <= 0 ? true : false;
    meHash1 = eHash;
    meHash2 = eHash2;
    maPassHash = aPassword;

#if DEBUG_TAB_PROTECTION
    for (sal_Int32 i = 0; i < nLen; ++i)
        printf("%2.2X ", static_cast<sal_uInt8>(aPassword[i]));
    printf("\n");
#endif
}

bool ScTableProtectionImpl::verifyPassword(const String& aPassText) const
{
#if DEBUG_TAB_PROTECTION
    fprintf(stdout, "ScTableProtectionImpl::verifyPassword: input = '%s'\n",
            OUStringToOString(rtl::OUString(aPassText), RTL_TEXTENCODING_UTF8).getStr());
#endif

    if (mbEmptyPass)
        return aPassText.Len() == 0;

    if (maPassText.Len())
        // Clear text password exists, and this one takes precedence.
        return aPassText.Equals(maPassText);

    Sequence<sal_Int8> aHash = hashPassword(aPassText, meHash1);
    aHash = hashPassword(aHash, meHash2);

#if DEBUG_TAB_PROTECTION
    fprintf(stdout, "ScTableProtectionImpl::verifyPassword: hash = ");
    for (sal_Int32 i = 0; i < aHash.getLength(); ++i)
        printf("%2.2X ", static_cast<sal_uInt8>(aHash[i]));
    printf("\n");
#endif

    return aHash == maPassHash;
}

bool ScTableProtectionImpl::isOptionEnabled(SCSIZE nOptId) const
{
    if ( maOptions.size() <= static_cast<size_t>(nOptId) )
    {
        OSL_FAIL("ScTableProtectionImpl::isOptionEnabled: wrong size");
        return false;
    }

    return maOptions[nOptId];
}

void ScTableProtectionImpl::setOption(SCSIZE nOptId, bool bEnabled)
{
    if ( maOptions.size() <= static_cast<size_t>(nOptId) )
    {
        OSL_FAIL("ScTableProtectionImpl::setOption: wrong size");
        return;
    }

    maOptions[nOptId] = bEnabled;
}

// ============================================================================

ScDocProtection::ScDocProtection() :
    mpImpl(new ScTableProtectionImpl(static_cast<SCSIZE>(ScDocProtection::NONE)))
{
}

ScDocProtection::ScDocProtection(const ScDocProtection& r) :
    ScPassHashProtectable(),
    mpImpl(new ScTableProtectionImpl(*r.mpImpl))
{
}

ScDocProtection::~ScDocProtection()
{
}

bool ScDocProtection::isProtected() const
{
    return mpImpl->isProtected();
}

bool ScDocProtection::isProtectedWithPass() const
{
    return mpImpl->isProtectedWithPass();
}

void ScDocProtection::setProtected(bool bProtected)
{
    mpImpl->setProtected(bProtected);

    // Currently Calc doesn't support document protection options.  So, let's
    // assume that when the document is protected, its structure is protected.
    // We need to do this for Excel export.
    mpImpl->setOption(ScDocProtection::STRUCTURE, bProtected);
}

bool ScDocProtection::isPasswordEmpty() const
{
    return mpImpl->isPasswordEmpty();
}

bool ScDocProtection::hasPasswordHash(ScPasswordHash eHash, ScPasswordHash eHash2) const
{
    return mpImpl->hasPasswordHash(eHash, eHash2);
}

void ScDocProtection::setPassword(const String& aPassText)
{
    mpImpl->setPassword(aPassText);
}

uno::Sequence<sal_Int8> ScDocProtection::getPasswordHash(ScPasswordHash eHash, ScPasswordHash eHash2) const
{
    return mpImpl->getPasswordHash(eHash, eHash2);
}

void ScDocProtection::setPasswordHash(
    const uno::Sequence<sal_Int8>& aPassword, ScPasswordHash eHash, ScPasswordHash eHash2)
{
    mpImpl->setPasswordHash(aPassword, eHash, eHash2);
}

bool ScDocProtection::verifyPassword(const String& aPassText) const
{
    return mpImpl->verifyPassword(aPassText);
}

bool ScDocProtection::isOptionEnabled(Option eOption) const
{
    return mpImpl->isOptionEnabled(eOption);
}

void ScDocProtection::setOption(Option eOption, bool bEnabled)
{
    mpImpl->setOption(eOption, bEnabled);
}

// ============================================================================

ScTableProtection::ScTableProtection() :
    mpImpl(new ScTableProtectionImpl(static_cast<SCSIZE>(ScTableProtection::NONE)))
{
    // Set default values for the options.
    mpImpl->setOption(SELECT_LOCKED_CELLS,   true);
    mpImpl->setOption(SELECT_UNLOCKED_CELLS, true);
}

ScTableProtection::ScTableProtection(const ScTableProtection& r) :
    ScPassHashProtectable(),
    mpImpl(new ScTableProtectionImpl(*r.mpImpl))
{
}

ScTableProtection::~ScTableProtection()
{
}

bool ScTableProtection::isProtected() const
{
    return mpImpl->isProtected();
}

bool ScTableProtection::isProtectedWithPass() const
{
    return mpImpl->isProtectedWithPass();
}

void ScTableProtection::setProtected(bool bProtected)
{
    mpImpl->setProtected(bProtected);
}

bool ScTableProtection::isPasswordEmpty() const
{
    return mpImpl->isPasswordEmpty();
}

bool ScTableProtection::hasPasswordHash(ScPasswordHash eHash, ScPasswordHash eHash2) const
{
    return mpImpl->hasPasswordHash(eHash, eHash2);
}

void ScTableProtection::setPassword(const String& aPassText)
{
    mpImpl->setPassword(aPassText);
}

Sequence<sal_Int8> ScTableProtection::getPasswordHash(ScPasswordHash eHash, ScPasswordHash eHash2) const
{
    return mpImpl->getPasswordHash(eHash, eHash2);
}

void ScTableProtection::setPasswordHash(
    const uno::Sequence<sal_Int8>& aPassword, ScPasswordHash eHash, ScPasswordHash eHash2)
{
    mpImpl->setPasswordHash(aPassword, eHash, eHash2);
}

bool ScTableProtection::verifyPassword(const String& aPassText) const
{
    return mpImpl->verifyPassword(aPassText);
}

bool ScTableProtection::isOptionEnabled(Option eOption) const
{
    return mpImpl->isOptionEnabled(eOption);
}

void ScTableProtection::setOption(Option eOption, bool bEnabled)
{
    mpImpl->setOption(eOption, bEnabled);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
