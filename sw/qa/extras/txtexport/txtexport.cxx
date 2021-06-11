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

class TxtExportTest : public SwModelTestBase
{
public:
    TxtExportTest()
        : SwModelTestBase("/sw/qa/extras/txtexport/data/", "Text")
    {
    }

protected:
    template <class T> std::vector<T> readMemoryStream()
    {
        SvMemoryStream aMemoryStream;
        SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
        aStream.ReadStream(aMemoryStream);
        const T* pData = static_cast<const T*>(aMemoryStream.GetData());
        sal_uInt64 size = aMemoryStream.GetSize();
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(0), size % sizeof(T));
        return std::vector<T>(pData, pData + size / sizeof(T));
    }

    OString readExportedFile()
    {
        std::vector<char> aMemStream = readMemoryStream<char>();

        int offset = 0;
        if (aMemStream.size() > 2 && aMemStream[0] == '\xEF' && aMemStream[1] == '\xBB'
            && aMemStream[2] == '\xBF')
            offset = 3;

        return OString(aMemStream.data() + offset, aMemStream.size() - offset);
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

DECLARE_TXTEXPORT_TEST(testTdf120574_utf8bom, "UTF8BOMCRLF.txt")
{
    std::vector<char> aMemStream = readMemoryStream<char>();
    OString aData(std::string_view(aMemStream.data(), aMemStream.size()));
    CPPUNIT_ASSERT_EQUAL(OString(u8"\uFEFFフー\r\nバー\r\n"), aData);
}

DECLARE_TXTEXPORT_TEST(testTdf120574_utf16lebom, "UTF16LEBOMCRLF.txt")
{
    std::vector<sal_Unicode> aMemStream = readMemoryStream<sal_Unicode>();
    OUString aData(aMemStream.data(), aMemStream.size());
    CPPUNIT_ASSERT_EQUAL(OUString(u"\uFEFFフー\r\nバー\r\n"), aData);
}

DECLARE_TXTEXPORT_TEST(testTdf142669_utf8, "UTF8CRLF.txt")
{
    std::vector<char> aMemStream = readMemoryStream<char>();
    OString aData(std::string_view(aMemStream.data(), aMemStream.size()));
    CPPUNIT_ASSERT_EQUAL(OString(u8"フー\r\nバー\r\n"), aData);
}

DECLARE_TXTEXPORT_TEST(testTdf142669_utf16le, "UTF16LECRLF.txt")
{
    std::vector<sal_Unicode> aMemStream = readMemoryStream<sal_Unicode>();
    OUString aData(aMemStream.data(), aMemStream.size());
    CPPUNIT_ASSERT_EQUAL(OUString(u"フー\r\nバー\r\n"), aData);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
