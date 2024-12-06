/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <config_oox.h>

#include <algorithm>
#include <memory>
#include <string_view>

#include <test/unoapi_test.hxx>
#include <o3tl/string_view.hxx>

#include <vcl/filter/PDFiumLibrary.hxx>
#include <vcl/pdfread.hxx>
#include <comphelper/propertyvalue.hxx>
#include <cmath>

#include <comphelper/crypto/Crypto.hxx>
#include <comphelper/hash.hxx>
#include <comphelper/random.hxx>

#include <pdf/PDFEncryptorR6.hxx>

#if USE_TLS_NSS
#include <nss.h>
#endif

using namespace ::com::sun::star;

namespace
{
class PDFEncryptionTest : public UnoApiTest
{
public:
    PDFEncryptionTest()
        : UnoApiTest("/vcl/qa/cppunit/pdfexport/data/")
    {
    }

    ~PDFEncryptionTest()
    {
#if USE_TLS_NSS
        NSS_Shutdown();
#endif
    }
};

// TODO: taken from GUID
sal_uInt8 gethex(char nChar)
{
    if (nChar >= '0' && nChar <= '9')
        return nChar - '0';
    else if (nChar >= 'a' && nChar <= 'f')
        return nChar - 'a' + 10;
    else if (nChar >= 'A' && nChar <= 'F')
        return nChar - 'A' + 10;
    else
        return 0;
}

// TODO: taken from GUID
sal_uInt8 convertHexChar(char high, char low) { return (gethex(high) << 4) + gethex(low); }

std::vector<sal_uInt8> parseHex(std::string_view rString)
{
    std::vector<sal_uInt8> aResult;
    aResult.reserve(rString.size() / 2);
    for (size_t i = 0; i < rString.size(); i += 2)
    {
        aResult.push_back(convertHexChar(rString[i], rString[i + 1]));
    }
    return aResult;
}

CPPUNIT_TEST_FIXTURE(PDFEncryptionTest, testComputeHashForR6)
{
    const sal_uInt8 pOwnerPass[] = { 'T', 'e', 's', 't' };
    const sal_uInt8 pUserPass[] = { 'T', 'e', 's', 't' };

    std::vector<sal_uInt8> U = parseHex("7BD210807A0277FECC52C261C442F02E1AD62C1A23553348B8F8AF7320"
                                        "DC9978FAB7E65E1BF4CA76F4BE5E6D2AA8C7D5");
    CPPUNIT_ASSERT_EQUAL(size_t(48), U.size());

    std::vector<sal_uInt8> O = parseHex("E4507A474CEFBBA1AF76BA0EB40EC322C91C1900D3FD65FEC98B873BA1"
                                        "9B27F89FBC9331D5E14DBCEE2A0ADDA52267C9");
    CPPUNIT_ASSERT_EQUAL(size_t(48), O.size());

    // User Password
    {
        std::vector<sal_uInt8> aUserHash(U.begin(), U.begin() + 32);
        CPPUNIT_ASSERT_EQUAL(size_t(32), aUserHash.size());

        CPPUNIT_ASSERT_EQUAL(
            std::string("7bd210807a0277fecc52c261c442f02e1ad62c1a23553348b8f8af7320dc9978"),
            comphelper::hashToString(aUserHash));

        std::vector<sal_uInt8> aUserValidationSalt(U.begin() + 32, U.begin() + 32 + 8);
        auto aComputedHash = vcl::pdf::computeHashR6(pUserPass, 4, aUserValidationSalt);
        CPPUNIT_ASSERT_EQUAL(
            std::string("7bd210807a0277fecc52c261c442f02e1ad62c1a23553348b8f8af7320dc9978"),
            comphelper::hashToString(aComputedHash));
    }

    // Owner Password
    {
        std::vector<sal_uInt8> aOwnerHash(O.begin(), O.begin() + 32);
        CPPUNIT_ASSERT_EQUAL(size_t(32), aOwnerHash.size());

        std::vector<sal_uInt8> aOwnerValidationSalt(O.begin() + 32, O.begin() + 32 + 8);
        CPPUNIT_ASSERT_EQUAL(size_t(8), aOwnerValidationSalt.size());

        CPPUNIT_ASSERT_EQUAL(
            std::string("e4507a474cefbba1af76ba0eb40ec322c91c1900d3fd65fec98b873ba19b27f8"),
            comphelper::hashToString(aOwnerHash));

        auto RO = vcl::pdf::computeHashR6(pOwnerPass, 4, aOwnerValidationSalt, U);
        CPPUNIT_ASSERT_EQUAL(
            std::string("e4507a474cefbba1af76ba0eb40ec322c91c1900d3fd65fec98b873ba19b27f8"),
            comphelper::hashToString(RO));
    }
}

CPPUNIT_TEST_FIXTURE(PDFEncryptionTest, testGenerateUandUE)
{
    // Checks we calculate U and UE correctly
    const sal_uInt8 pUserPass[] = { 'T', 'e', 's', 't' };

    std::vector<sal_uInt8> aInputKey = vcl::pdf::generateKey();

    std::vector<sal_uInt8> U;
    std::vector<sal_uInt8> UE;

    // Generate the U and UE from the user password and encrypt the
    // encryption key into UE
    vcl::pdf::generateUandUE(pUserPass, 4, aInputKey, U, UE);

    // Checks that the U validates the password (would fail if the U
    // would be calculated wrongly).
    CPPUNIT_ASSERT_EQUAL(true, vcl::pdf::validateUserPassword(pUserPass, 4, U));

    // Decrypt the key - this would fail if U and UE would be calculated
    // wrongly
    auto aDecryptedKey = vcl::pdf::decryptKey(pUserPass, 4, U, UE);

    // Decrypted key and input key need to match
    CPPUNIT_ASSERT_EQUAL(comphelper::hashToString(aInputKey),
                         comphelper::hashToString(aDecryptedKey));
}

CPPUNIT_TEST_FIXTURE(PDFEncryptionTest, testGenerateOandOE)
{
    // Checks we calculate O and OE correctly

    const auto aUserPass = std::to_array<sal_uInt8>({ 'T', 'e', 's', 't' });
    const auto aOwnerPass = std::to_array<sal_uInt8>({ 'T', 'e', 's', 't', '2' });

    std::vector<sal_uInt8> aInputKey = vcl::pdf::generateKey();

    std::vector<sal_uInt8> U;
    std::vector<sal_uInt8> UE;
    std::vector<sal_uInt8> O;
    std::vector<sal_uInt8> OE;

    // Generates U and UE - we need U in generateOandOE
    vcl::pdf::generateUandUE(aUserPass.data(), aUserPass.size(), aInputKey, U, UE);
    vcl::pdf::generateOandOE(aOwnerPass.data(), aOwnerPass.size(), aInputKey, U, O, OE);

    // Checks the user password is valid
    CPPUNIT_ASSERT_EQUAL(true,
                         vcl::pdf::validateUserPassword(aUserPass.data(), aUserPass.size(), U));

    // Checks the owner password is valid
    CPPUNIT_ASSERT_EQUAL(
        true, vcl::pdf::validateOwnerPassword(aOwnerPass.data(), aOwnerPass.size(), U, O));
}

CPPUNIT_TEST_FIXTURE(PDFEncryptionTest, testPermsEncryption)
{
    // Encrypts file permissions for /Perms entry

    // We use a existing encrypted /Perm to validate the decryption and re-encryption
    // algorithm works correctly.

    const sal_uInt8 pUserPass[] = { 'T', 'e', 's', 't' };

    // U and UE taken from an PDF that was encrypted with "Test" as password
    std::vector<sal_uInt8> U = parseHex("7BD210807A0277FECC52C261C442F02E1AD62C1A23553348B8F8AF7320"
                                        "DC9978FAB7E65E1BF4CA76F4BE5E6D2AA8C7D5");
    std::vector<sal_uInt8> UE
        = parseHex("67022D91A6BDF3179F488DC9658E54B78A0AD05C6A9C419DCD17A6941C151197");

    // We decrypt the key, which is needed to decrypt and encrypt the /Perms content
    std::vector<sal_uInt8> aKey = vcl::pdf::decryptKey(pUserPass, 4, U, UE);

    // Known encrypted /Perms content taken from the PDF
    std::vector<sal_uInt8> aPermEncrypted = parseHex("6a2306c6e5e71a5bbd8404b07abec38f");

    // Decrypt
    std::vector<sal_uInt8> aPermsDecrpyted = vcl::pdf::decryptPerms(aPermEncrypted, aKey);

    // Encrypt again
    std::vector<sal_uInt8> aPermsReencrypted = vcl::pdf::encryptPerms(aPermsDecrpyted, aKey);

    // Original encrypted /Perm content should be equal to decrypted and encrypted again
    CPPUNIT_ASSERT_EQUAL(std::string("6a2306c6e5e71a5bbd8404b07abec38f"),
                         comphelper::hashToString(aPermsReencrypted));

    // Always should be a,b,d
    CPPUNIT_ASSERT_EQUAL(sal_uInt8('a'), aPermsDecrpyted[9]);
    CPPUNIT_ASSERT_EQUAL(sal_uInt8('d'), aPermsDecrpyted[10]);
    CPPUNIT_ASSERT_EQUAL(sal_uInt8('b'), aPermsDecrpyted[11]);

    // Metadata encrypted? - T or F
    CPPUNIT_ASSERT_EQUAL(sal_uInt8('T'), aPermsDecrpyted[8]);

    // Decrypting the access permissions
    sal_Int32 aAccessPermissions
        = sal_Int32(aPermsDecrpyted[0]) + sal_Int32(aPermsDecrpyted[1] << 8)
          + sal_Int32(aPermsDecrpyted[2] << 16) + sal_Int32(aPermsDecrpyted[3] << 24);

    // Taken from the PDF (/P entry)
    sal_Int32 nExpectedAccessPermisssions = -4;
    CPPUNIT_ASSERT_EQUAL(nExpectedAccessPermisssions, aAccessPermissions);

    // the whole decrypted /Perms content
    CPPUNIT_ASSERT_EQUAL(std::string("fcffffffffffffff54616462bb609a8a"),
                         comphelper::hashToString(aPermsDecrpyted));

    // Check the creating /Perm content from access permissions works correctly
    std::vector<sal_uInt8> aPermsCreated = vcl::pdf::createPerms(nExpectedAccessPermisssions, true);

    // Last 12 bytes are random, so we shouldn't check those
    std::vector<sal_uInt8> aPermsWithoutRandomBytes(aPermsCreated.begin(),
                                                    aPermsCreated.begin() + 12);

    // Should match the decrypted content
    CPPUNIT_ASSERT_EQUAL(std::string("fcffffffffffffff54616462"),
                         comphelper::hashToString(aPermsWithoutRandomBytes));
}
} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
