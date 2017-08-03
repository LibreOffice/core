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

#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/uno/Reference.hxx>

#include <svl/svldllapi.h>
#include <svl/sigstruct.hxx>

namespace com {
namespace sun {
namespace star {
namespace security {
    class XCertificate; }
}}}

class SvStream;

namespace svl {

namespace crypto {

/// Converts a hex-encoded string into a byte array.
SVL_DLLPUBLIC std::vector<unsigned char> DecodeHexString(const OString& rHex);

/// Helper to cryptographically sign and verify
/// arbitrary data blocks.
class SVL_DLLPUBLIC Signing
{
public:

    Signing(const css::uno::Reference<css::security::XCertificate>& xCertificate) :
        m_xCertificate(xCertificate)
    {
    }

    /// Add a range to sign.
    /// Note: for efficiency this takes a naked pointer, which must remain valid
    /// until this object is discarded.
    void AddDataRange(const void* pData, sal_Int32 size)
    {
        m_dataBlocks.emplace_back(pData, size);
    }

    void SetSignTSA(const OUString& tsa) { m_aSignTSA = tsa; }
    void SetSignPassword(const OUString& password) { m_aSignPassword = password;; }

    /// Signs one or more data blocks (as a single, contiguous, array).
    /// Returns the signature (in PKCS#7 format) as string (hex).
    bool Sign(OStringBuffer& rCMSHexBuffer);

    /// Verify and get Signature Information given a byte array.
    static bool Verify(const std::vector<unsigned char>& aData,
                       const bool bNonDetached,
                       const std::vector<unsigned char>& aSignature,
                       SignatureInformation& rInformation);

    /// Verify and get Signature Information given a signature and stream.
    static bool Verify(SvStream& rStream,
                       const std::vector<std::pair<size_t, size_t>>& aByteRanges,
                       const bool bNonDetached,
                       const std::vector<unsigned char>& aSignature,
                       SignatureInformation& rInformation);

private:
    /// The certificate to use for signing.
    const css::uno::Reference<css::security::XCertificate> m_xCertificate;

    /// Data blocks (pointer-size pairs).
    std::vector<std::pair<const void*, sal_Int32>> m_dataBlocks;
    OUString m_aSignTSA;
    OUString m_aSignPassword;
};

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

