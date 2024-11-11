/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pdf/PDFEncryptorR6.hxx>
#include <pdf/EncryptionHashTransporter.hxx>
#include <pdf/pdfwriter_impl.hxx>
#include <comphelper/crypto/Crypto.hxx>
#include <comphelper/hash.hxx>
#include <comphelper/random.hxx>

namespace vcl::pdf
{
namespace
{
/** Calculates modulo 3 of the 128-bit integer, using the first 16 bytes of the vector */
sal_Int32 calculateModulo3(std::vector<sal_uInt8> const& rInput)
{
    sal_Int32 nSum = 0;
    for (size_t i = 0; i < 16; ++i)
        nSum += rInput[i];
    return nSum % 3;
}
}

/** Algorithm 2.B: Computing a hash (revision 6 and later)
 *
 * Described in ISO 32000-2:2020(E) - 7.6.4.3.4
 */
std::vector<sal_uInt8> computeHashR6(const sal_uInt8* pPassword, size_t nPasswordLength,
                                     std::vector<sal_uInt8> const& rValidationSalt,
                                     std::vector<sal_uInt8> const& rUserKey)
{
    // Round 0
    comphelper::Hash aHash(comphelper::HashType::SHA256);
    aHash.update(pPassword, nPasswordLength);
    aHash.update(rValidationSalt);
    if (!rUserKey.empty()) // if calculating owner key
        aHash.update(rUserKey);

    std::vector<sal_uInt8> K = aHash.finalize();

    std::vector<sal_uInt8> E;

    sal_Int32 nRound = 1;
    do
    {
        // Step a)
        std::vector<sal_uInt8> K1;
        for (sal_Int32 nRepetition = 0; nRepetition < 64; ++nRepetition)
        {
            K1.insert(K1.end(), pPassword, pPassword + nPasswordLength);
            K1.insert(K1.end(), K.begin(), K.end());
            if (!rUserKey.empty()) // if calculating owner key
                K1.insert(K1.end(), rUserKey.begin(), rUserKey.end());
        }

        // Step b)
        std::vector<sal_uInt8> aKey(K.begin(), K.begin() + 16);
        std::vector<sal_uInt8> aInitVector(K.begin() + 16, K.end());

        E = std::vector<sal_uInt8>(K1.size(), 0);

        comphelper::Encrypt aEncrypt(aKey, aInitVector, comphelper::CryptoType::AES_128_CBC);
        aEncrypt.update(E, K1);

        // Step c)
        sal_Int32 nModulo3Result = calculateModulo3(E);

        // Step d)
        comphelper::HashType eType;
        switch (nModulo3Result)
        {
            case 0:
                eType = comphelper::HashType::SHA256;
                break;
            case 1:
                eType = comphelper::HashType::SHA384;
                break;
            default:
                eType = comphelper::HashType::SHA512;
                break;
        }
        K = comphelper::Hash::calculateHash(E.data(), E.size(), eType);

        nRound++;
    }
    // Step e) and f)
    // We stop iteration if we do at least 64 rounds and (the last element of E <= round number - 32)
    while (nRound < 64 || E.back() > (nRound - 32));

    // Output - first 32 bytes
    return std::vector<sal_uInt8>(K.begin(), K.begin() + 32);
}

} // end vcl::pdf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
