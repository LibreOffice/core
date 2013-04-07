/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Kohei Yoshida <kohei.yoshida@suse.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
    TestAutoCorrDoc(const OUString &rText, LanguageType eLang)
        : m_sText(rText)
        , m_eLang(eLang)
    {
    }
    OUString getResult() const
    {
        return m_sText.toString();
    }
private:
    OUStringBuffer m_sText;
    LanguageType m_eLang;
    virtual sal_Bool Delete( xub_StrLen nStt, xub_StrLen nEnd )
    {
        //fprintf(stderr, "TestAutoCorrDoc::Delete\n");
        m_sText.remove(nStt, nEnd-nStt);
        return true;
    }
    virtual sal_Bool Insert( xub_StrLen nPos, const String& rTxt )
    {
        //fprintf(stderr, "TestAutoCorrDoc::Insert\n");
        m_sText.insert(nPos, rTxt);
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
        m_sText.remove(nPos, nLen);
        m_sText.insert(nPos, rTxt);
        return true;
    }
    virtual sal_Bool SetAttr( xub_StrLen, xub_StrLen, sal_uInt16, SfxPoolItem& )
    {
        //fprintf(stderr, "TestAutoCorrDoc::SetAttr\n");
        return true;
    }
    virtual sal_Bool SetINetAttr( xub_StrLen, xub_StrLen, const String& )
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

        if (m_sText.isEmpty())
            return false;

        const SvxAutocorrWord* pFnd = rACorrect.SearchWordsInList(m_sText.toString(), rSttPos, nEndPos, *this, m_eLang);
        if (pFnd && pFnd->IsTextOnly())
        {
            m_sText.remove(rSttPos, nEndPos);
            m_sText.insert(rSttPos, pFnd->GetLong());
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
        aAutoCorrect.AutoCorrect(aFoo, sInput, sInput.getLength(), cNextChar, true);

        CPPUNIT_ASSERT_MESSAGE("autocorrect", aFoo.getResult() == sExpected);
    }

    {
        OUString sInput("TEst/TEst");
        sal_Unicode cNextChar(' ');
        OUString sExpected("Test/Test ");

        TestAutoCorrDoc aFoo(sInput, LANGUAGE_ENGLISH_US);
        aAutoCorrect.AutoCorrect(aFoo, sInput, sInput.getLength(), cNextChar, true);

        CPPUNIT_ASSERT_MESSAGE("autocorrect", aFoo.getResult() == sExpected);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
