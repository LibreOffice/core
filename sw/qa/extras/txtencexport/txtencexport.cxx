/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <shellio.hxx>
#include <unotextrange.hxx>
#include <iodetect.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

class TxtEncExportTest : public SwModelTestBase
{
public:
    TxtEncExportTest(const OUString & rFilterOptions)
        : SwModelTestBase("/sw/qa/extras/txtencexport/data/", "Text (encoded)")
    {
        setFilterOptions(rFilterOptions);
    }

protected:
    OUString readExportedFile()
    {
        SvMemoryStream aMemoryStream;
        SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
        aStream.ReadStream(aMemoryStream);
        const char* pData = static_cast<const char*>(aMemoryStream.GetData());

        int offset = 0;
        if (aMemoryStream.GetSize() > 2 && pData[0] == '\xEF' && pData[1] == '\xBB'
            && pData[2] == '\xBF')
            offset = 3;

        return OStringToOUString(OString(pData + offset, aMemoryStream.GetSize() - offset),
                                 RTL_TEXTENCODING_UTF8);
    }
};

#define DECLARE_TXTENCEXPORT_TEST(TestName, filename, filterOptions)      \
    class TestName ## Class : public TxtEncExportTest {                   \
    public:                                                               \
        TestName ## Class() : TxtEncExportTest(filterOptions) {}          \
    };                                                                    \
    DECLARE_SW_EXPORT_TEST(TestName, filename, nullptr, TestName ## Class)

DECLARE_TXTENCEXPORT_TEST(testBullets, "bullets.odt", "UTF8,,,,")
{
    OUString aData = readExportedFile();

    OUString aExpected =
        u"1 Heading 1" SAL_NEWLINE_STRING "1.A Heading 2" SAL_NEWLINE_STRING
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
        "Next paragraph" SAL_NEWLINE_STRING "Hidden paragraph" SAL_NEWLINE_STRING
        "Final paragraph" SAL_NEWLINE_STRING;

    CPPUNIT_ASSERT_EQUAL(aExpected, aData);
}

DECLARE_TXTENCEXPORT_TEST(testBulletsHidden, "bullets.odt", "UTF8,,,,,false")
{
    OUString aData = readExportedFile();

    OUString aExpected =
        u"1 Heading 1" SAL_NEWLINE_STRING "1.A Heading 2" SAL_NEWLINE_STRING
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
        "Next paragraph" SAL_NEWLINE_STRING "Hidden paragraph" SAL_NEWLINE_STRING
        "Final paragraph" SAL_NEWLINE_STRING;

    CPPUNIT_ASSERT_EQUAL(aExpected, aData);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
