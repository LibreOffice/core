/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <osl/thread.hxx>
#include <string>

class TxtExportTest : public SwModelTestBase
{
public:
    TxtExportTest()
        : SwModelTestBase("/sw/qa/extras/txtexport/data/", "Text")
    {
    }

protected:
    OString readExportedFile()
    {
        SvMemoryStream aMemoryStream;
        SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
        aStream.ReadStream(aMemoryStream);
        const char* pData = static_cast<const char*>(aMemoryStream.GetData());

        int offset = 0;
        if (aMemoryStream.GetSize() > 2 && pData[0] == '\xEF' && pData[1] == '\xBB'
            && pData[2] == '\xBF')
            offset = 3;

        return OString(pData + offset, aMemoryStream.GetSize() - offset);
    }
};

#define DECLARE_TXTEXPORT_TEST(TestName, filename)                                                 \
    DECLARE_SW_EXPORT_TEST(TestName, filename, nullptr, TxtExportTest)

DECLARE_TXTEXPORT_TEST(testBullets, "bullets.odt")
{
    OString aData = readExportedFile();

    OUString aString = OStringToOUString(
        "1 Heading 1" SAL_NEWLINE_STRING "1.A Heading 2" SAL_NEWLINE_STRING
        "Paragraph" SAL_NEWLINE_STRING "" SAL_NEWLINE_STRING
        "    \xe2\x80\xa2 First bullet" SAL_NEWLINE_STRING
        "    \xe2\x80\xa2 Second bullet" SAL_NEWLINE_STRING
        "        \xe2\x97\xa6 Sub-second bullet" SAL_NEWLINE_STRING
        "      Third bullet, but deleted" SAL_NEWLINE_STRING
        "    \xe2\x80\xa2 Fourth bullet" SAL_NEWLINE_STRING "" SAL_NEWLINE_STRING
        "Numbering" SAL_NEWLINE_STRING "" SAL_NEWLINE_STRING "    1. First" SAL_NEWLINE_STRING
        "    2. Second" SAL_NEWLINE_STRING "        1. Second-first" SAL_NEWLINE_STRING
        "       Third, but deleted" SAL_NEWLINE_STRING "    3. Actual third" SAL_NEWLINE_STRING
        "" SAL_NEWLINE_STRING "Paragraph after numbering" SAL_NEWLINE_STRING
        "Next paragraph" SAL_NEWLINE_STRING "Final paragraph" SAL_NEWLINE_STRING,
        RTL_TEXTENCODING_UTF8);

    // To get the stuff back in the system's encoding
    OString aExpected(OUStringToOString(aString, osl_getThreadTextEncoding()));

    CPPUNIT_ASSERT_EQUAL(aExpected, aData);
}

DECLARE_TXTEXPORT_TEST(testTdf120574_utf8, "UTF8BOMCRLF.txt")
{
    SvMemoryStream aMemoryStream;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    aStream.ReadStream(aMemoryStream);
    const char* pData = static_cast<const char*>(aMemoryStream.GetData());

    CPPUNIT_ASSERT_EQUAL('\xEF', pData[0]);
    CPPUNIT_ASSERT_EQUAL('\xBB', pData[1]);
    CPPUNIT_ASSERT_EQUAL('\xBF', pData[2]);

    OUString aData = OStringToOUString(std::string_view(pData + 3, aMemoryStream.GetSize() - 3),
                                       RTL_TEXTENCODING_UTF8);

    CPPUNIT_ASSERT_EQUAL(OUString(u"フー\r\nバー\r\n"), aData);
}

DECLARE_TXTEXPORT_TEST(testTdf120574_utf16le, "UTF16LECRLF.txt")
{
    SvMemoryStream aMemoryStream;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    aStream.ReadStream(aMemoryStream);

    const char16_t* pData = static_cast<const char16_t*>(aMemoryStream.GetData());
    const int16_t* pBom = static_cast<const int16_t*>(aMemoryStream.GetData());

    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(0xFEFF), *pBom);

    OUString aData(pData + 1, aMemoryStream.GetSize() / sizeof(char16_t) - 1);

    CPPUNIT_ASSERT_EQUAL(OUString(u"フー\r\nバー\r\n"), aData);
}
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
