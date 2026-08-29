/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#ifdef MACOSX

#include <premac.h>
#include <Security/Security.h>
#include <postmac.h>

#include <vector>

#include <cpo/uno/Sequence.hxx>

#include <svl/svldllapi.h>

namespace svl::crypto
{
/// Minimal RAII owner for CoreFoundation objects.
template <typename T> class CFRef
{
public:
    CFRef() = default;

    /// Takes ownership of an already-retained (Copy/Create rule) reference.
    explicit CFRef(T ref)
        : m_ref(ref)
    {
    }

    CFRef(const CFRef&) = delete;
    CFRef& operator=(const CFRef&) = delete;

    CFRef(CFRef&& rOther) noexcept
        : m_ref(rOther.m_ref)
    {
        rOther.m_ref = nullptr;
    }

    CFRef& operator=(CFRef&& rOther) noexcept
    {
        if (this != &rOther)
        {
            reset(rOther.m_ref);
            rOther.m_ref = nullptr;
        }
        return *this;
    }

    ~CFRef() { reset(nullptr); }

    void reset(T ref)
    {
        if (m_ref)
            CFRelease(m_ref);
        m_ref = ref;
    }

    /// Releases ownership without releasing the reference.
    T release()
    {
        T ref = m_ref;
        m_ref = nullptr;
        return ref;
    }

    T get() const { return m_ref; }
    bool is() const { return m_ref != nullptr; }

private:
    T m_ref = nullptr;
};

/** Returns the keychain search list to use for signing certificate lookups.

    Normally this is the user's default search list (i.e. a null return, meaning "don't restrict
    the query"). When the COKIT_TEST_KEYCHAIN environment variable points to a keychain file, the
    returned array contains just that keychain, which makes unit tests hermetic.
*/
SVL_DLLPUBLIC CFRef<CFArrayRef> CopyKeychainSearchList();

/** Enumerates all identities (certificate + private key pairs) in the keychain search list.

    Returns a CFArray of SecIdentityRef, or a null ref if none are available.
*/
SVL_DLLPUBLIC CFRef<CFArrayRef> CopyAllKeychainIdentities();

/** Finds the Keychain identity whose certificate matches the given DER encoding.

    Returns a null ref when the certificate has no private key in the keychain, in which case
    the caller is expected to fall back to NSS.
*/
SVL_DLLPUBLIC CFRef<SecIdentityRef>
CopyKeychainIdentityForCertificate(const cpo::uno::Sequence<sal_Int8>& rDerCertificate);

/** Returns the DER-encoded certificate chain of pLeaf (leaf first).

    The chain is built by evaluating the leaf against the system trust store; when that fails,
    at least the leaf itself is returned.
*/
SVL_DLLPUBLIC std::vector<std::vector<unsigned char>>
CopyKeychainCertificateChain(SecCertificateRef pLeaf);
}

#endif // MACOSX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
