/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test/bootstrapfixture.hxx"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "svl/itempool.hxx"
#include "editeng/eerdll.hxx"
#include "editeng/eerdll2.hxx"
#include "editeng/editeng.hxx"
#include "editeng/eeitem.hxx"
#include "editeng/editids.hrc"
#include "editeng/editdoc.hxx"
#include "editeng/svxacorr.hxx"
#include "editeng/unofield.hxx"

#include <com/sun/star/text/textfield/Type.hpp>

using namespace com::sun::star;

namespace {

class Test : public test::BootstrapFixture
{
public:
    Test();

    virtual void setUp();
    virtual void tearDown();

    void testConstruction();

    /**
     * Test UNO service class that implements text field items.
     */
    void testUnoTextFields();

    /**
     * AutoCorrect tests
     */
    void testAutocorrect();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testConstruction);
    CPPUNIT_TEST(testUnoTextFields);
    CPPUNIT_TEST(testAutocorrect);
    CPPUNIT_TEST_SUITE_END();

private:
    EditEngineItemPool* mpItemPool;
};

Test::Test() : mpItemPool(NULL) {}

void Test::setUp()
{
    test::BootstrapFixture::setUp();

    mpItemPool = new EditEngineItemPool(true);
}

void Test::tearDown()
{
    SfxItemPool::Free(mpItemPool);
    test::BootstrapFixture::tearDown();
}

void Test::testConstruction()
{
    EditEngine aEngine(mpItemPool);

    OUString aParaText = "I am Edit Engine.";
    aEngine.SetText(aParaText);
}

namespace {

bool includes(const uno::Sequence<OUString>& rSeq, const OUString& rVal)
{
    for (sal_Int32 i = 0, n = rSeq.getLength(); i < n; ++i)
        if (rSeq[i] == rVal)
            return true;

    return false;
}

}

void Test::testUnoTextFields()
{
    {
        // DATE
        SvxUnoTextField aField(text::textfield::Type::DATE);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.DateTime");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // URL
        SvxUnoTextField aField(text::textfield::Type::URL);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.URL");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // PAGE
        SvxUnoTextField aField(text::textfield::Type::PAGE);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.PageNumber");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // PAGES
        SvxUnoTextField aField(text::textfield::Type::PAGES);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.PageCount");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // TIME
        SvxUnoTextField aField(text::textfield::Type::TIME);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.DateTime");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // FILE
        SvxUnoTextField aField(text::textfield::Type::DOCINFO_TITLE);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.docinfo.Title");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // TABLE
        SvxUnoTextField aField(text::textfield::Type::TABLE);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.SheetName");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // EXTENDED TIME
        SvxUnoTextField aField(text::textfield::Type::EXTENDED_TIME);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.DateTime");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // EXTENDED FILE
        SvxUnoTextField aField(text::textfield::Type::EXTENDED_FILE);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.FileName");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // AUTHOR
        SvxUnoTextField aField(text::textfield::Type::AUTHOR);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.Author");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // MEASURE
        SvxUnoTextField aField(text::textfield::Type::MEASURE);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.Measure");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // PRESENTATION HEADER
        SvxUnoTextField aField(text::textfield::Type::PRESENTATION_HEADER);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.presentation.textfield.Header");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // PRESENTATION FOOTER
        SvxUnoTextField aField(text::textfield::Type::PRESENTATION_FOOTER);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.presentation.textfield.Footer");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // PRESENTATION DATE TIME
        SvxUnoTextField aField(text::textfield::Type::PRESENTATION_DATE_TIME);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.presentation.textfield.DateTime");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }
}

class TestAutoCorrDoc : public SvxAutoCorrDoc
{
public:
    /// just like the real thing, this dummy modifies the rText parameter :(
    TestAutoCorrDoc(OUString &rText, LanguageType eLang)
        : m_rText(rText)
        , m_eLang(eLang)
    {
    }
    OUString const& getResult() const
    {
        return m_rText;
    }
private:
    OUString & m_rText;
    LanguageType m_eLang;
    virtual sal_Bool Delete( xub_StrLen nStt, xub_StrLen nEnd )
    {
        //fprintf(stderr, "TestAutoCorrDoc::Delete\n");
        m_rText = m_rText.replaceAt(nStt, nEnd-nStt, "");
        return true;
    }
    virtual sal_Bool Insert( xub_StrLen nPos, const String& rTxt )
    {
        //fprintf(stderr, "TestAutoCorrDoc::Insert\n");
        m_rText = m_rText.replaceAt(nPos, 0, rTxt);
        return true;
    }
    virtual sal_Bool Replace( xub_StrLen nPos, const String& rTxt )
    {
        //fprintf(stderr, "TestAutoCorrDoc::Replace\n");
        return ReplaceRange( nPos, rTxt.Len(), rTxt );
    }
    virtual sal_Bool ReplaceRange( xub_StrLen nPos, xub_StrLen nLen, const String& rTxt )
    {
        //fprintf(stderr, "TestAutoCorrDoc::ReplaceRange %d %d %s\n", nPos, nLen, OUStringToOString(rTxt, RTL_TEXTENCODING_UTF8).getStr());
        m_rText = m_rText.replaceAt(nPos, nLen, rTxt);
        return true;
    }
    virtual sal_Bool SetAttr( xub_StrLen, xub_StrLen, sal_uInt16, SfxPoolItem& )
    {
        //fprintf(stderr, "TestAutoCorrDoc::SetAttr\n");
        return true;
    }
    virtual sal_Bool SetINetAttr( xub_StrLen, xub_StrLen, const OUString& )
    {
        //fprintf(stderr, "TestAutoCorrDoc::SetINetAttr\n");
        return true;
    }
    virtual const String* GetPrevPara( sal_Bool )
    {
        //fprintf(stderr, "TestAutoCorrDoc::GetPrevPara\n");
        return NULL;
    }
    virtual sal_Bool ChgAutoCorrWord( sal_uInt16& rSttPos,
                sal_uInt16 nEndPos, SvxAutoCorrect& rACorrect,
                const String** ppPara )
    {
        //fprintf(stderr, "TestAutoCorrDoc::ChgAutoCorrWord\n");

        if (m_rText.isEmpty())
            return false;

        const SvxAutocorrWord* pFnd = rACorrect.SearchWordsInList(
                m_rText, rSttPos, nEndPos, *this, m_eLang);
        if (pFnd && pFnd->IsTextOnly())
        {
            m_rText = m_rText.replaceAt(rSttPos, nEndPos, pFnd->GetLong());
            if( ppPara )
                *ppPara = NULL;//&pCurNode->GetString();
            return true;
        }

        return false;
    }
};

//https://bugs.freedesktop.org/show_bug.cgi?id=55693
//Two capitalized letters are not corrected if dash or slash are directly
//before the two letters
void Test::testAutocorrect()
{
    OUString sShareAutocorrFile;
    OUString sUserAutocorrFile;
    SvxAutoCorrect aAutoCorrect(sShareAutocorrFile, sUserAutocorrFile);

    {
        OUString sInput("TEst-TEst");
        sal_Unicode cNextChar(' ');
        OUString sExpected("Test-Test ");

        TestAutoCorrDoc aFoo(sInput, LANGUAGE_ENGLISH_US);
        aAutoCorrect.DoAutoCorrect(aFoo, sInput, sInput.getLength(), cNextChar, true);

        CPPUNIT_ASSERT_MESSAGE("autocorrect", aFoo.getResult() == sExpected);
    }

    {
        OUString sInput("TEst/TEst");
        sal_Unicode cNextChar(' ');
        OUString sExpected("Test/Test ");

        TestAutoCorrDoc aFoo(sInput, LANGUAGE_ENGLISH_US);
        aAutoCorrect.DoAutoCorrect(aFoo, sInput, sInput.getLength(), cNextChar, true);

        CPPUNIT_ASSERT_MESSAGE("autocorrect", aFoo.getResult() == sExpected);
    }

    {
        // test auto-bolding with '*'
        OUString sInput("*foo");
        sal_Unicode cNextChar('*');
        OUString sExpected("foo");

        TestAutoCorrDoc aFoo(sInput, LANGUAGE_ENGLISH_US);
        String const& rInput2(reinterpret_cast<String const&>(aFoo.getResult()));
        aAutoCorrect.DoAutoCorrect(aFoo,
            rInput2,
            sInput.getLength(), cNextChar, true);

        CPPUNIT_ASSERT_EQUAL(sExpected, aFoo.getResult());
    }

}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
