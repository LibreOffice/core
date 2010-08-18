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

#define DEBUG_TAB_PROTECTION 0

using namespace ::com::sun::star;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

// ============================================================================

bool ScPassHashHelper::needsPassHashRegen(const ScDocument& rDoc, ScPasswordHash eHash)
{
    if (rDoc.IsDocProtected())
    {
        const ScDocProtection* p = rDoc.GetDocProtection();
        if (!p->isPasswordEmpty() && !p->hasPasswordHash(eHash))
            return true;
    }

    SCTAB nTabCount = rDoc.GetTableCount();
    for (SCTAB i = 0; i < nTabCount; ++i)
    {
        const ScTableProtection* p = rDoc.GetTabProtection(i);
        if (!p || !p->isProtected())
            // Sheet not protected.  Skip it.
            continue;

        if (!p->isPasswordEmpty() && !p->hasPasswordHash(eHash))
            return true;
    }

    return false;
}

// ============================================================================

ScPassHashProtectable::~ScPassHashProtectable()
{
}

// ============================================================================

class ScTableProtectionImpl
{
public:
    static ::com::sun::star::uno::Sequence<sal_Int8> hashPassword(const String& aPassText, ScPasswordHash eHash = PASSHASH_OOO);

    explicit ScTableProtectionImpl(SCSIZE nOptSize);
    explicit ScTableProtectionImpl(const ScTableProtectionImpl& r);

    bool isProtected() const;
    bool isProtectedWithPass() const;
    void setProtected(bool bProtected);

    bool isPasswordEmpty() const;
    bool hasPasswordHash(ScPasswordHash eHash) const;
    void setPassword(const String& aPassText);
    ::com::sun::star::uno::Sequence<sal_Int8> getPasswordHash(ScPasswordHash eHash) const;
    void setPasswordHash(const ::com::sun::star::uno::Sequence<sal_Int8>& aPassword, ScPasswordHash eHash = PASSHASH_OOO);
    bool verifyPassword(const String& aPassText) const;

    bool isOptionEnabled(SCSIZE nOptId) const;
    void setOption(SCSIZE nOptId, bool bEnabled);

private:
    String maPassText;
    ::com::sun::star::uno::Sequence<sal_Int8>   maPassHash;
    ::std::vector<bool> maOptions;
    bool mbEmptyPass;
    bool mbProtected;
    ScPasswordHash meHash;
};

Sequence<sal_Int8> ScTableProtectionImpl::hashPassword(const String& aPassText, ScPasswordHash eHash)
{
    Sequence<sal_Int8> aHash;
    switch (eHash)
    {
        case PASSHASH_XL:
            aHash = ::comphelper::DocPasswordHelper::GetXLHashAsSequence( aPassText, RTL_TEXTENCODING_UTF8 );
        break;
        case PASSHASH_OOO:
        default:
            SvPasswordHelper::GetHashPassword(aHash, aPassText);
        break;
    }
    return aHash;
}

ScTableProtectionImpl::ScTableProtectionImpl(SCSIZE nOptSize) :
    maOptions(nOptSize),
    mbEmptyPass(true),
    mbProtected(false),
    meHash(PASSHASH_OOO)
{
}

ScTableProtectionImpl::ScTableProtectionImpl(const ScTableProtectionImpl& r) :
    maPassText(r.maPassText),
    maPassHash(r.maPassHash),
    maOptions(r.maOptions),
    mbEmptyPass(r.mbEmptyPass),
    mbProtected(r.mbProtected),
    meHash(r.meHash)
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

bool ScTableProtectionImpl::hasPasswordHash(ScPasswordHash eHash) const
{
    if (mbEmptyPass)
        return true;

    if (maPassText.Len())
        return true;

    if (meHash == eHash)
        return true;

    return false;
}

Sequence<sal_Int8> ScTableProtectionImpl::getPasswordHash(ScPasswordHash eHash) const
{
    if (mbEmptyPass)
        // Flaged as empty.
        return Sequence<sal_Int8>();

    if (maPassText.Len())
        // Cleartext password exists.  Hash it.
        return hashPassword(maPassText, eHash);

    if (meHash == eHash)
        // Stored hash exists.
        return maPassHash;

    // Failed to find a matching hash.
    return Sequence<sal_Int8>();
}

void ScTableProtectionImpl::setPasswordHash(const uno::Sequence<sal_Int8>& aPassword, ScPasswordHash eHash)
{
    sal_Int32 nLen = aPassword.getLength();
    mbEmptyPass = nLen <= 0 ? true : false;
    meHash = eHash;
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

    Sequence<sal_Int8> aHash = hashPassword(aPassText, meHash);

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
        DBG_ERROR("ScTableProtectionImpl::isOptionEnabled: wrong size");
        return false;
    }

    return maOptions[nOptId];
}

void ScTableProtectionImpl::setOption(SCSIZE nOptId, bool bEnabled)
{
    if ( maOptions.size() <= static_cast<size_t>(nOptId) )
    {
        DBG_ERROR("ScTableProtectionImpl::setOption: wrong size");
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

bool ScDocProtection::hasPasswordHash(ScPasswordHash eHash) const
{
    return mpImpl->hasPasswordHash(eHash);
}

void ScDocProtection::setPassword(const String& aPassText)
{
    mpImpl->setPassword(aPassText);
}

uno::Sequence<sal_Int8> ScDocProtection::getPasswordHash(ScPasswordHash eHash) const
{
    return mpImpl->getPasswordHash(eHash);
}

void ScDocProtection::setPasswordHash(const uno::Sequence<sal_Int8>& aPassword, ScPasswordHash eHash)
{
    mpImpl->setPasswordHash(aPassword, eHash);
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

bool ScTableProtection::hasPasswordHash(ScPasswordHash eHash) const
{
    return mpImpl->hasPasswordHash(eHash);
}

void ScTableProtection::setPassword(const String& aPassText)
{
    mpImpl->setPassword(aPassText);
}

Sequence<sal_Int8> ScTableProtection::getPasswordHash(ScPasswordHash eHash) const
{
    return mpImpl->getPasswordHash(eHash);
}

void ScTableProtection::setPasswordHash(const uno::Sequence<sal_Int8>& aPassword, ScPasswordHash eHash)
{
    mpImpl->setPasswordHash(aPassword, eHash);
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

