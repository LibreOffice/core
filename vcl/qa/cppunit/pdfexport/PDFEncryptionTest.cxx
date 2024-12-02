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

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
