/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <string>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    bool mustTestImportOf(const char* filename) const override {
        return (OString(filename).endsWith(".docx"));
    }
};

DECLARE_SW_ROUNDTRIP_TEST(testPassword2007, "Encrypted_MSO2007_abc.docx", "abc", Test)
{
    // Standard encryption format, AES 128, SHA1
    uno::Reference<text::XTextRange> xParagraph(getParagraph(1));
    CPPUNIT_ASSERT_EQUAL(OUString("abc"), xParagraph->getString());
}

DECLARE_SW_ROUNDTRIP_TEST(testPassword2010, "Encrypted_MSO2010_abc.docx", "abc", Test)
{
    // Agile encryption format, AES 128, CBC, SHA1
    uno::Reference<text::XTextRange> xParagraph(getParagraph(1));
    CPPUNIT_ASSERT_EQUAL(OUString("abc"), xParagraph->getString());
}

DECLARE_SW_ROUNDTRIP_TEST(testPassword2013, "Encrypted_MSO2013_abc.docx", "abc", Test)
{
    // Agile encryption format, AES 256, CBC, SHA512
    uno::Reference<text::XTextRange> xParagraph(getParagraph(1));
    CPPUNIT_ASSERT_EQUAL(OUString("ABC"), xParagraph->getString());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
