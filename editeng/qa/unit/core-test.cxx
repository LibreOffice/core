/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
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
#include "editeng/wghtitem.hxx"
#include "editeng/postitem.hxx"
#include "editeng/section.hxx"
#include "editeng/editobj.hxx"

#include <com/sun/star/text/textfield/Type.hpp>

#include <boost/scoped_ptr.hpp>

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

    void testSectionAttributes();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testConstruction);
    CPPUNIT_TEST(testUnoTextFields);
    CPPUNIT_TEST(testAutocorrect);
    CPPUNIT_TEST(testSectionAttributes);
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
        
        SvxUnoTextField aField(text::textfield::Type::DATE);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.DateTime");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        
        SvxUnoTextField aField(text::textfield::Type::URL);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.URL");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        
        SvxUnoTextField aField(text::textfield::Type::PAGE);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.PageNumber");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        
        SvxUnoTextField aField(text::textfield::Type::PAGES);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.PageCount");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        
        SvxUnoTextField aField(text::textfield::Type::TIME);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.DateTime");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        
        SvxUnoTextField aField(text::textfield::Type::DOCINFO_TITLE);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.docinfo.Title");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        
        SvxUnoTextField aField(text::textfield::Type::TABLE);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.SheetName");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        
        SvxUnoTextField aField(text::textfield::Type::EXTENDED_TIME);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.DateTime");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        
        SvxUnoTextField aField(text::textfield::Type::EXTENDED_FILE);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.FileName");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        
        SvxUnoTextField aField(text::textfield::Type::AUTHOR);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.Author");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        
        SvxUnoTextField aField(text::textfield::Type::MEASURE);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.Measure");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        
        SvxUnoTextField aField(text::textfield::Type::PRESENTATION_HEADER);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.presentation.textfield.Header");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        
        SvxUnoTextField aField(text::textfield::Type::PRESENTATION_FOOTER);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.presentation.textfield.Footer");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        
        SvxUnoTextField aField(text::textfield::Type::PRESENTATION_DATE_TIME);
        uno::Sequence<OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.presentation.textfield.DateTime");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }
}

class TestAutoCorrDoc : public SvxAutoCorrDoc
{
public:
    /
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
    virtual sal_Bool Delete( sal_Int32 nStt, sal_Int32 nEnd )
    {
        
        m_rText = m_rText.replaceAt(nStt, nEnd-nStt, "");
        return true;
    }
    virtual sal_Bool Insert( sal_Int32 nPos, const OUString& rTxt )
    {
        
        m_rText = m_rText.replaceAt(nPos, 0, rTxt);
        return true;
    }
    virtual sal_Bool Replace( sal_Int32 nPos, const OUString& rTxt )
    {
        
        return ReplaceRange( nPos, rTxt.getLength(), rTxt );
    }
    virtual sal_Bool ReplaceRange( sal_Int32 nPos, sal_Int32 nLen, const OUString& rTxt )
    {
        
        m_rText = m_rText.replaceAt(nPos, nLen, rTxt);
        return true;
    }
    virtual sal_Bool SetAttr( sal_Int32, sal_Int32, sal_uInt16, SfxPoolItem& )
    {
        
        return true;
    }
    virtual sal_Bool SetINetAttr( sal_Int32, sal_Int32, const OUString& )
    {
        
        return true;
    }
    virtual OUString const* GetPrevPara(bool)
    {
        
        return 0;
    }
    virtual bool ChgAutoCorrWord( sal_Int32& rSttPos,
                sal_Int32 nEndPos, SvxAutoCorrect& rACorrect,
                OUString* pPara )
    {
        

        if (m_rText.isEmpty())
            return false;

        LanguageTag aLanguageTag( m_eLang);
        const SvxAutocorrWord* pFnd = rACorrect.SearchWordsInList(
                m_rText, rSttPos, nEndPos, *this, aLanguageTag);
        if (pFnd && pFnd->IsTextOnly())
        {
            m_rText = m_rText.replaceAt(rSttPos, nEndPos, pFnd->GetLong());
            if( pPara )
                *pPara = "";
            return true;
        }

        return false;
    }
};




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
        
        OUString sInput("*foo");
        sal_Unicode cNextChar('*');
        OUString sExpected("foo");

        TestAutoCorrDoc aFoo(sInput, LANGUAGE_ENGLISH_US);
        aAutoCorrect.DoAutoCorrect(aFoo, sInput, sInput.getLength(), cNextChar, true);

        CPPUNIT_ASSERT_EQUAL(sExpected, aFoo.getResult());
    }
}

bool hasBold(const editeng::Section& rSecAttr)
{
    std::vector<const SfxPoolItem*>::const_iterator it = rSecAttr.maAttributes.begin(), itEnd = rSecAttr.maAttributes.end();
    for (; it != itEnd; ++it)
    {
        const SfxPoolItem* p = *it;
        if (p->Which() != EE_CHAR_WEIGHT)
            continue;

        if (static_cast<const SvxWeightItem*>(p)->GetWeight() != WEIGHT_BOLD)
            continue;

        return true;
    }
    return false;
}

bool hasItalic(const editeng::Section& rSecAttr)
{
    std::vector<const SfxPoolItem*>::const_iterator it = rSecAttr.maAttributes.begin(), itEnd = rSecAttr.maAttributes.end();
    for (; it != itEnd; ++it)
    {
        const SfxPoolItem* p = *it;
        if (p->Which() != EE_CHAR_ITALIC)
            continue;

        if (static_cast<const SvxPostureItem*>(p)->GetPosture() != ITALIC_NORMAL)
            continue;

        return true;
    }
    return false;
}

void Test::testSectionAttributes()
{
    EditEngine aEngine(mpItemPool);

    boost::scoped_ptr<SfxItemSet> pSet(new SfxItemSet(aEngine.GetEmptyItemSet()));
    SvxWeightItem aBold(WEIGHT_BOLD, EE_CHAR_WEIGHT);
    SvxPostureItem aItalic(ITALIC_NORMAL, EE_CHAR_ITALIC);

    {
        OUString aParaText = "aaabbbccc";
        aEngine.SetText(aParaText);
        pSet->Put(aBold);
        CPPUNIT_ASSERT_MESSAGE("There should be exactly one item.", pSet->Count() == 1);
        aEngine.QuickSetAttribs(*pSet, ESelection(0,0,0,6)); 
        pSet.reset(new SfxItemSet(aEngine.GetEmptyItemSet()));
        pSet->Put(aItalic);
        CPPUNIT_ASSERT_MESSAGE("There should be exactly one item.", pSet->Count() == 1);

        aEngine.QuickSetAttribs(*pSet, ESelection(0,3,0,9)); 
        boost::scoped_ptr<EditTextObject> pEditText(aEngine.CreateTextObject());
        CPPUNIT_ASSERT_MESSAGE("Failed to create text object.", pEditText.get());
        std::vector<editeng::Section> aAttrs;
        pEditText->GetAllSections(aAttrs);

        
        CPPUNIT_ASSERT_MESSAGE("There should be 3 sections.", aAttrs.size() == 3);

        
        const editeng::Section* pSecAttr = &aAttrs[0];
        CPPUNIT_ASSERT_EQUAL(0, (int)pSecAttr->mnParagraph);
        CPPUNIT_ASSERT_EQUAL(0, (int)pSecAttr->mnStart);
        CPPUNIT_ASSERT_EQUAL(3, (int)pSecAttr->mnEnd);
        CPPUNIT_ASSERT_EQUAL(1, (int)pSecAttr->maAttributes.size());
        CPPUNIT_ASSERT_MESSAGE("This section must be bold.", hasBold(*pSecAttr));

        
        pSecAttr = &aAttrs[1];
        CPPUNIT_ASSERT_EQUAL(0, (int)pSecAttr->mnParagraph);
        CPPUNIT_ASSERT_EQUAL(3, (int)pSecAttr->mnStart);
        CPPUNIT_ASSERT_EQUAL(6, (int)pSecAttr->mnEnd);
        CPPUNIT_ASSERT_EQUAL(2, (int)pSecAttr->maAttributes.size());
        CPPUNIT_ASSERT_MESSAGE("This section must be bold and italic.", hasBold(*pSecAttr) && hasItalic(*pSecAttr));

        
        pSecAttr = &aAttrs[2];
        CPPUNIT_ASSERT_EQUAL(0, (int)pSecAttr->mnParagraph);
        CPPUNIT_ASSERT_EQUAL(6, (int)pSecAttr->mnStart);
        CPPUNIT_ASSERT_EQUAL(9, (int)pSecAttr->mnEnd);
        CPPUNIT_ASSERT_EQUAL(1, (int)pSecAttr->maAttributes.size());
        CPPUNIT_ASSERT_MESSAGE("This section must be italic.", hasItalic(*pSecAttr));
    }

    {
        
        
        aEngine.Clear();
        aEngine.SetText("one\n\ntwo\n\nthree");
        sal_Int32 nParaCount = aEngine.GetParagraphCount();
        sal_Int32 nCheck = 5;
        CPPUNIT_ASSERT_EQUAL(nCheck, nParaCount);

        
        pSet.reset(new SfxItemSet(aEngine.GetEmptyItemSet()));
        pSet->Put(aBold);
        CPPUNIT_ASSERT_MESSAGE("There should be exactly one item.", pSet->Count() == 1);
        aEngine.QuickSetAttribs(*pSet, ESelection(0,0,0,3));
        aEngine.QuickSetAttribs(*pSet, ESelection(2,0,2,3));
        aEngine.QuickSetAttribs(*pSet, ESelection(4,0,4,5));

        boost::scoped_ptr<EditTextObject> pEditText(aEngine.CreateTextObject());
        CPPUNIT_ASSERT_MESSAGE("Failed to create text object.", pEditText.get());
        std::vector<editeng::Section> aAttrs;
        pEditText->GetAllSections(aAttrs);
        size_t nSecCountCheck = 5;
        CPPUNIT_ASSERT_EQUAL(nSecCountCheck, aAttrs.size());

        
        const editeng::Section* pSecAttr = &aAttrs[0];
        CPPUNIT_ASSERT_EQUAL(0, (int)pSecAttr->mnParagraph);
        CPPUNIT_ASSERT_EQUAL(0, (int)pSecAttr->mnStart);
        CPPUNIT_ASSERT_EQUAL(3, (int)pSecAttr->mnEnd);
        CPPUNIT_ASSERT_EQUAL(1, (int)pSecAttr->maAttributes.size());
        CPPUNIT_ASSERT_MESSAGE("This section must be bold.", hasBold(*pSecAttr));

        pSecAttr = &aAttrs[2];
        CPPUNIT_ASSERT_EQUAL(2, (int)pSecAttr->mnParagraph);
        CPPUNIT_ASSERT_EQUAL(0, (int)pSecAttr->mnStart);
        CPPUNIT_ASSERT_EQUAL(3, (int)pSecAttr->mnEnd);
        CPPUNIT_ASSERT_EQUAL(1, (int)pSecAttr->maAttributes.size());
        CPPUNIT_ASSERT_MESSAGE("This section must be bold.", hasBold(*pSecAttr));

        pSecAttr = &aAttrs[4];
        CPPUNIT_ASSERT_EQUAL(4, (int)pSecAttr->mnParagraph);
        CPPUNIT_ASSERT_EQUAL(0, (int)pSecAttr->mnStart);
        CPPUNIT_ASSERT_EQUAL(5, (int)pSecAttr->mnEnd);
        CPPUNIT_ASSERT_EQUAL(1, (int)pSecAttr->maAttributes.size());
        CPPUNIT_ASSERT_MESSAGE("This section must be bold.", hasBold(*pSecAttr));

        
        pSecAttr = &aAttrs[1];
        CPPUNIT_ASSERT_EQUAL(1, (int)pSecAttr->mnParagraph);
        CPPUNIT_ASSERT_EQUAL(0, (int)pSecAttr->mnStart);
        CPPUNIT_ASSERT_EQUAL(0, (int)pSecAttr->mnEnd);
        CPPUNIT_ASSERT_MESSAGE("Attribute array should be empty.", pSecAttr->maAttributes.empty());

        pSecAttr = &aAttrs[3];
        CPPUNIT_ASSERT_EQUAL(3, (int)pSecAttr->mnParagraph);
        CPPUNIT_ASSERT_EQUAL(0, (int)pSecAttr->mnStart);
        CPPUNIT_ASSERT_EQUAL(0, (int)pSecAttr->mnEnd);
        CPPUNIT_ASSERT_MESSAGE("Attribute array should be empty.", pSecAttr->maAttributes.empty());
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
