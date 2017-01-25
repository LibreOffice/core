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

#include "tabprotection.hxx"
#include <svl/PasswordHelper.hxx>
#include <comphelper/docpasswordhelper.hxx>
#include <osl/diagnose.h>
#include "document.hxx"

#include <vector>

#define DEBUG_TAB_PROTECTION 0

#define URI_SHA1 "http://www.w3.org/2000/09/xmldsig#sha1"
#define URI_XLS_LEGACY "http://docs.oasis-open.org/office/ns/table/legacy-hash-excel"

using namespace ::com::sun::star;
using ::com::sun::star::uno::Sequence;
using ::std::vector;

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
            return OUString(URI_SHA1);
        case PASSHASH_XL:
            return OUString(URI_XLS_LEGACY);
        case PASSHASH_UNSPECIFIED:
        default:
            ;
    }
    return OUString();
}

ScPasswordHash ScPassHashHelper::getHashTypeFromURI(const OUString& rURI)
{
    if ( rURI == URI_SHA1 )
        return PASSHASH_SHA1;
    else if ( rURI == URI_XLS_LEGACY )
        return PASSHASH_XL;
    return PASSHASH_UNSPECIFIED;
}

ScPassHashProtectable::~ScPassHashProtectable()
{
}

class ScTableProtectionImpl
{
public:
    static Sequence<sal_Int8> hashPassword(const OUString& aPassText, ScPasswordHash eHash);
    static Sequence<sal_Int8> hashPassword(const Sequence<sal_Int8>& rPassHash, ScPasswordHash eHash);

    explicit ScTableProtectionImpl(SCSIZE nOptSize);
    explicit ScTableProtectionImpl(const ScTableProtectionImpl& r);

    bool isProtected() const { return mbProtected;}
    bool isProtectedWithPass() const;
    void setProtected(bool bProtected);

    bool isPasswordEmpty() const { return mbEmptyPass;}
    bool hasPasswordHash(ScPasswordHash eHash, ScPasswordHash eHash2) const;
    void setPassword(const OUString& aPassText);
    css::uno::Sequence<sal_Int8> getPasswordHash(
        ScPasswordHash eHash, ScPasswordHash eHash2) const;
    void setPasswordHash(
        const css::uno::Sequence<sal_Int8>& aPassword,
        ScPasswordHash eHash, ScPasswordHash eHash2);
    bool verifyPassword(const OUString& aPassText) const;

    bool isOptionEnabled(SCSIZE nOptId) const;
    void setOption(SCSIZE nOptId, bool bEnabled);

    void setEnhancedProtection( const ::std::vector< ScEnhancedProtection > & rProt );
    const ::std::vector< ScEnhancedProtection > & getEnhancedProtection() const { return maEnhancedProtection;}
    bool updateReference( UpdateRefMode, ScDocument*, const ScRange& rWhere, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    bool isBlockEditable( const ScRange& rRange ) const;
    bool isSelectionEditable( const ScRangeList& rRangeList ) const;

private:
    OUString maPassText;
    css::uno::Sequence<sal_Int8>   maPassHash;
    ::std::vector<bool> maOptions;
    bool mbEmptyPass;
    bool mbProtected;
    ScPasswordHash meHash1;
    ScPasswordHash meHash2;
    ::std::vector< ScEnhancedProtection > maEnhancedProtection;
};

Sequence<sal_Int8> ScTableProtectionImpl::hashPassword(const OUString& aPassText, ScPasswordHash eHash)
{
    Sequence<sal_Int8> aHash;
    switch (eHash)
    {
        case PASSHASH_XL:
            aHash = ::comphelper::DocPasswordHelper::GetXLHashAsSequence( aPassText );
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
    meHash2(r.meHash2),
    maEnhancedProtection(r.maEnhancedProtection)
{
}

bool ScTableProtectionImpl::isProtectedWithPass() const
{
    if (!mbProtected)
        return false;

    return !maPassText.isEmpty() || maPassHash.getLength();
}

void ScTableProtectionImpl::setProtected(bool bProtected)
{
    mbProtected = bProtected;
    // We need to keep the old password even when the protection is off.  So,
    // don't erase the password data here.
}

void ScTableProtectionImpl::setPassword(const OUString& aPassText)
{
    // We can't hash it here because we don't know whether this document will
    // get saved to Excel or ODF, depending on which we will need to use a
    // different hashing algorithm.  One alternative is to hash it using all
    // hash algorithms that we support, and store them all.

    maPassText = aPassText;
    mbEmptyPass = aPassText.isEmpty();
    if (mbEmptyPass)
    {
        maPassHash = Sequence<sal_Int8>();
    }
}

bool ScTableProtectionImpl::hasPasswordHash(ScPasswordHash eHash, ScPasswordHash eHash2) const
{
    if (mbEmptyPass)
        return true;

    if (!maPassText.isEmpty())
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
        // Flagged as empty.
        return aPassHash;

    if (!maPassText.isEmpty())
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
    mbEmptyPass = nLen <= 0;
    meHash1 = eHash;
    meHash2 = eHash2;
    maPassHash = aPassword;

#if DEBUG_TAB_PROTECTION
    for (sal_Int32 i = 0; i < nLen; ++i)
        printf("%2.2X ", static_cast<sal_uInt8>(aPassword[i]));
    printf("\n");
#endif
}

bool ScTableProtectionImpl::verifyPassword(const OUString& aPassText) const
{
#if DEBUG_TAB_PROTECTION
    fprintf(stdout, "ScTableProtectionImpl::verifyPassword: input = '%s'\n",
            OUStringToOString(OUString(aPassText), RTL_TEXTENCODING_UTF8).getStr());
#endif

    if (mbEmptyPass)
        return aPassText.isEmpty();

    if (!maPassText.isEmpty())
        // Clear text password exists, and this one takes precedence.
        return aPassText == maPassText;

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

void ScTableProtectionImpl::setEnhancedProtection( const ::std::vector< ScEnhancedProtection > & rProt )
{
    maEnhancedProtection = rProt;
}

bool ScTableProtectionImpl::updateReference( UpdateRefMode eMode, ScDocument* pDoc,
        const ScRange& rWhere, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    bool bChanged = false;
    for (::std::vector<ScEnhancedProtection>::iterator it(maEnhancedProtection.begin());
            it != maEnhancedProtection.end(); ++it)
    {
        if ((*it).maRangeList.is())
            bChanged |= (*it).maRangeList->UpdateReference( eMode, pDoc, rWhere, nDx, nDy, nDz);
    }
    return bChanged;
}

bool ScTableProtectionImpl::isBlockEditable( const ScRange& rRange ) const
{
    /* TODO: ask for password (and remember) if a password was set for
     * a matching range and no matching range without password was encountered.
     * Would need another return type than boolean to reflect
     * "password required for a specific protection". */

    // No protection exception or overriding permission to edit if empty.
    if (maEnhancedProtection.empty())
        return false;

    // No security descriptor in an enhanced protection means the ranges of
    // that protection are editable. If there is any security descriptor
    // present we assume the permission to edit is not granted. Until we
    // actually can evaluate the descriptors..

    for (::std::vector<ScEnhancedProtection>::const_iterator it(maEnhancedProtection.begin()),
            itEnd(maEnhancedProtection.end()); it != itEnd; ++it)
    {
        if (!(*it).hasSecurityDescriptor() && (*it).maRangeList.is())
        {
            if ((*it).maRangeList->In( rRange))
            {
                // Range is editable if no password is assigned.
                if (!(*it).hasPassword())
                    return true;
            }
        }
    }

    // For a single address, a simple check with single ranges was sufficient.
    if (rRange.aStart == rRange.aEnd)
        return false;

    // Test also for cases where rRange is encompassed by a union of two or
    // more ranges of the list. The original ranges are not necessarily joined.
    for (::std::vector<ScEnhancedProtection>::const_iterator it(maEnhancedProtection.begin()),
            itEnd(maEnhancedProtection.end()); it != itEnd; ++it)
    {
        if (!(*it).hasSecurityDescriptor() && (*it).maRangeList.is())
        {
            ScRangeList aList( (*it).maRangeList->GetIntersectedRange( rRange));
            if (aList.size() == 1 && *aList[0] == rRange)
            {
                // Range is editable if no password is assigned.
                if (!(*it).hasPassword())
                    return true;
            }
        }
    }

    // Ranges may even be distributed over different protection records, for
    // example if they are assigned different names, and can have different
    // passwords. Combine the ones that can be edited.
    /* TODO: once we handle passwords, remember a successful unlock at
     * ScEnhancedProtection so we can use that here. */
    ScRangeList aRangeList;
    for (::std::vector<ScEnhancedProtection>::const_iterator it(maEnhancedProtection.begin()),
            itEnd(maEnhancedProtection.end()); it != itEnd; ++it)
    {
        if (!(*it).hasSecurityDescriptor() && (*it).maRangeList.is())
        {
            // Ranges are editable if no password is assigned.
            if (!(*it).hasPassword())
            {
                const ScRangeList& rRanges = *(*it).maRangeList;
                size_t nRanges = rRanges.size();
                for (size_t i=0; i < nRanges; ++i)
                {
                    aRangeList.Append( *rRanges[i]);
                }
            }
        }
    }
    ScRangeList aResultList( aRangeList.GetIntersectedRange( rRange));
    if (aResultList.size() == 1 && *aResultList[0] == rRange)
        return true;

    return false;
}

bool ScTableProtectionImpl::isSelectionEditable( const ScRangeList& rRangeList ) const
{
    if (rRangeList.empty())
        return false;

    for (size_t i=0, nRanges = rRangeList.size(); i < nRanges; ++i)
    {
        if (!isBlockEditable( *rRangeList[i]))
            return false;
    }
    return true;
}

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

void ScDocProtection::setPassword(const OUString& aPassText)
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

bool ScDocProtection::verifyPassword(const OUString& aPassText) const
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

void ScTableProtection::setPassword(const OUString& aPassText)
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

bool ScTableProtection::verifyPassword(const OUString& aPassText) const
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

void ScTableProtection::setEnhancedProtection( const ::std::vector< ScEnhancedProtection > & rProt )
{
    mpImpl->setEnhancedProtection(rProt);
}

const ::std::vector< ScEnhancedProtection > & ScTableProtection::getEnhancedProtection() const
{
    return mpImpl->getEnhancedProtection();
}

bool ScTableProtection::updateReference( UpdateRefMode eMode, ScDocument* pDoc, const ScRange& rWhere,
        SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    return mpImpl->updateReference( eMode, pDoc, rWhere, nDx, nDy, nDz);
}

bool ScTableProtection::isBlockEditable( const ScRange& rRange ) const
{
    return mpImpl->isBlockEditable( rRange);
}

bool ScTableProtection::isSelectionEditable( const ScRangeList& rRangeList ) const
{
    return mpImpl->isSelectionEditable( rRangeList);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
