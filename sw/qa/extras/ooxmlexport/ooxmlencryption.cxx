/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <o3tl/string_view.hxx>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testPasswordMSO2007)
{
    const char* const sPass = "abc";
    createSwDoc("Encrypted_MSO2007_abc.docx", /*rParams*/ {}, sPass);
    // Standard encryption format, AES 128, SHA1
    uno::Reference<text::XTextRange> xParagraph(getParagraph(1));
    CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, getParagraph(1)->getString());
    saveAndReload(TestFilter::DOCX, /*rParams*/ {}, sPass);
    CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testPasswordMSO2010)
{
    const char* const sPass = "abc";
    createSwDoc("Encrypted_MSO2010_abc.docx", /*rParams*/ {}, sPass);
    // Agile encryption format, AES 128, CBC, SHA1
    CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, getParagraph(1)->getString());
    saveAndReload(TestFilter::DOCX, /*rParams*/ {}, sPass);
    CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testPasswordMSO2013)
{
    const char* const sPass = "abc";
    createSwDoc("Encrypted_MSO2013_abc.docx", /*rParams*/ {}, sPass);
    // Agile encryption format, AES 256, CBC, SHA512
    uno::Reference<text::XTextRange> xParagraph(getParagraph(1));
    CPPUNIT_ASSERT_EQUAL(u"ABC"_ustr, getParagraph(1)->getString());
    saveAndReload(TestFilter::DOCX, /*rParams*/ {}, sPass);
    CPPUNIT_ASSERT_EQUAL(u"ABC"_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testPasswordLOStandard)
{
    const char* const sPass = "abc";
    createSwDoc("Encrypted_LO_Standard_abc.docx", /*rParams*/ {}, sPass);
    // Standard encryption format, AES 128, SHA1
    uno::Reference<text::XTextRange> xParagraph(getParagraph(1));
    CPPUNIT_ASSERT_EQUAL(u"ABC"_ustr, getParagraph(1)->getString());
    saveAndReload(TestFilter::DOCX, /*rParams*/ {}, sPass);
    CPPUNIT_ASSERT_EQUAL(u"ABC"_ustr, getParagraph(1)->getString());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
