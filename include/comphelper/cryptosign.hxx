/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>
#include <sal/types.h>

#include <memory>
#include <vector>

#include <comphelper/comphelperdllapi.h>
#include <comphelper/hash.hxx>
#include <vcl/pdfwriter.hxx>
#include <com/sun/star/security/XCertificate.hpp>

#if HAVE_FEATURE_NSS && !defined(_WIN32)
// NSS headers for PDF signing
#include "nss.h"
#include "cert.h"
#include "hasht.h"
#include "secerr.h"
#include "sechash.h"
#include "cms.h"
#include "cmst.h"

// We use curl for RFC3161 time stamp requests
#include <curl/curl.h>
#endif

#ifdef _WIN32
// WinCrypt headers for PDF signing
// Note: this uses Windows 7 APIs and requires the relevant data types
#include <prewin.h>
#include <wincrypt.h>
#include <postwin.h>
#include <comphelper/windowserrorstring.hxx>
#endif

namespace comphelper {

namespace crypto {

/// Helper to cryptographically sign and verify
/// arbitrary data blocks.
class COMPHELPER_DLLPUBLIC Signing
{
public:

    Signing(const css::uno::Reference<css::security::XCertificate>& xCertificate) :
        m_xCertificate(xCertificate)
    {
    }

    /// Add a range to sign.
    /// Note: for efficiency this takes a naked pointer, which must remain valid
    /// until this object is discarded.
    void AddDataRange(void* pData, sal_Int32 size)
    {
        m_dataBlocks.emplace_back(pData, size);
    }

    /// Signs one or more data blocks (as a single, contiguous, array).
    /// Returns the signature (in PKCS#7 format) as string (hex).
    bool Sign(OStringBuffer& rCMSHexBuffer);

private:
    /// The certificate to use for signing.
    const css::uno::Reference<css::security::XCertificate> m_xCertificate;

    /// Data blocks (pointer-size pairs).
    std::vector<std::pair<void*, sal_Int32>> m_dataBlocks;
    OUString m_aSignTSA;
    OUString m_aSignPassword;
};

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
