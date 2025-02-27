/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <test/unoapi_test.hxx>

#include <font/TTFStructure.hxx>
#include <font/TTFReader.hxx>
#include <font/TTFReader.hxx>

namespace
{
class TTFTest : public UnoApiTest
{
public:
    TTFTest()
        : UnoApiTest("/vcl/qa/cppunit/font/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(TTFTest, testReadTTFStructure)
{
    OUString aFileName = createFileURL(u"Ahem.ttf");
    SvFileStream aStream(aFileName, StreamMode::READ);
    std::vector<sal_uInt8> aFontBytes(aStream.remainingSize());
    aStream.ReadBytes(aFontBytes.data(), aFontBytes.size());
    CPPUNIT_ASSERT(aStream.good());
    aStream.Close();

    font::FontDataContainer aContainer(aFontBytes);
    font::TTFFont aFont(aContainer);
    auto pHanlder = aFont.getTableEntriesHandler();
    const font::OS2Table* pOS2 = pHanlder->getOS2Table();
    CPPUNIT_ASSERT(pOS2);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(400), sal_uInt16(pOS2->nWeightClass));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(5), sal_uInt16(pOS2->nWidthClass));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1000), sal_uInt16(pOS2->nXAvgCharWidth));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), sal_uInt16(pOS2->nFamilyClass));

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(175), sal_uInt16(pOS2->nUnicodeRange1));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(8264), sal_uInt16(pOS2->nUnicodeRange2));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), sal_uInt16(pOS2->nUnicodeRange3));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), sal_uInt16(pOS2->nUnicodeRange4));

    const font::HeadTable* pHeadTable = pHanlder->getHeadTable();
    CPPUNIT_ASSERT(pHeadTable);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1000), sal_uInt16(pHeadTable->nUnitsPerEm));

    auto pNameTableHanlder = pHanlder->getNameTableHandler();
    CPPUNIT_ASSERT(pNameTableHanlder);

    sal_uInt64 nOffset = 0;
    sal_uInt16 nLength = 0;

    CPPUNIT_ASSERT(pNameTableHanlder->findEnglishUnicodeNameOffset(font::NameID::FamilyName,
                                                                   nOffset, nLength));
    OUString aFamilyName = aFont.getNameTableString(nOffset, nLength);
    CPPUNIT_ASSERT_EQUAL(u"Ahem"_ustr, aFamilyName);

    CPPUNIT_ASSERT(pNameTableHanlder->findEnglishUnicodeNameOffset(font::NameID::FullFontName,
                                                                   nOffset, nLength));
    OUString aFullFontName = aFont.getNameTableString(nOffset, nLength);
    CPPUNIT_ASSERT_EQUAL(u"Ahem"_ustr, aFullFontName);

    CPPUNIT_ASSERT(pNameTableHanlder->findEnglishUnicodeNameOffset(font::NameID::SubfamilyName,
                                                                   nOffset, nLength));
    OUString aSubFamilyName = aFont.getNameTableString(nOffset, nLength);
    CPPUNIT_ASSERT_EQUAL(u"Regular"_ustr, aSubFamilyName);

    CPPUNIT_ASSERT(
        pNameTableHanlder->findEnglishUnicodeNameOffset(font::NameID::Version, nOffset, nLength));
    OUString aVersion = aFont.getNameTableString(nOffset, nLength);
    CPPUNIT_ASSERT_EQUAL(u"Version 1.1"_ustr, aVersion);
}

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
