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
#include "editeng/fontitem.hxx"
#include "editeng/fhgtitem.hxx"
#include "editeng/lspcitem.hxx"
#include "editeng/svxacorr.hxx"
#include "editeng/unofield.hxx"
#include "editeng/wghtitem.hxx"
#include "editeng/postitem.hxx"
#include "editeng/section.hxx"
#include "editeng/editobj.hxx"
#include "editeng/flditem.hxx"
#include <editeng/adjustitem.hxx>
#include "svl/srchitem.hxx"

#include <com/sun/star/text/textfield/Type.hpp>

#include <memory>

using namespace com::sun::star;

namespace {

class Test : public test::BootstrapFixture
{
public:
    Test();

    virtual void setUp() override;
    virtual void tearDown() override;

    /// Test text portions position when percentage line spacing is set
    void testLineSpacing();

    void testConstruction();

    /// Test UNO service class that implements text field items.
    void testUnoTextFields();

    /// AutoCorrect tests
    void testAutocorrect();

    /// Test hyperlinks
    void testHyperlinkSearch();

    void testSectionAttributes();

    /// Test HoriAlignIgnoreTrailingWhitespace compatibility flag
    void testHoriAlignIgnoreTrailingWhitespace();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testConstruction);
    CPPUNIT_TEST(testLineSpacing);
    CPPUNIT_TEST(testUnoTextFields);
    CPPUNIT_TEST(testAutocorrect);
    CPPUNIT_TEST(testHyperlinkSearch);
    CPPUNIT_TEST(testSectionAttributes);
    CPPUNIT_TEST(testHoriAlignIgnoreTrailingWhitespace);
    CPPUNIT_TEST_SUITE_END();

private:
    EditEngineItemPool* mpItemPool;
};

Test::Test() : mpItemPool(nullptr) {}

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

void Test::testLineSpacing()
{
    // Create EditEngine's instance
    EditEngine aEditEngine(mpItemPool);

    if(aEditEngine.GetRefDevice()->GetDPIY() != 96
        || !basegfx::fTools::equal(aEditEngine.GetRefDevice()->GetDPIScaleFactor(), 1.0))
        return;

    // Get EditDoc for current EditEngine's instance
    EditDoc &rDoc = aEditEngine.GetEditDoc();

    // Initially no text should be there
    CPPUNIT_ASSERT_EQUAL(sal_uLong(0), rDoc.GetTextLen());
    CPPUNIT_ASSERT_EQUAL(OUString(), rDoc.GetParaAsString(sal_Int32(0)));

    // Set initial text
    OUString aText = "This is multi-line paragraph";

    sal_Int32 aTextLen = aText.getLength();
    aEditEngine.SetText(aText);

    // Assert changes - text insertion
    CPPUNIT_ASSERT_EQUAL(sal_uLong(aTextLen), rDoc.GetTextLen());
    CPPUNIT_ASSERT_EQUAL(aText, rDoc.GetParaAsString(sal_Int32(0)));

    // Get ItemSet for line spacing - 60%
    std::unique_ptr<SfxItemSet> pSet(new SfxItemSet(aEditEngine.GetEmptyItemSet()));
    SvxLineSpacingItem aLineSpacing(LINE_SPACE_DEFAULT_HEIGHT, EE_PARA_SBL);
    aLineSpacing.SetPropLineSpace(60);
    pSet->Put(aLineSpacing);

    // Set font
    SvxFontItem aFont(EE_CHAR_FONTINFO);
    aFont.SetFamilyName("Liberation Sans");
    pSet->Put(aFont);
    SvxFontHeightItem aFontSize(240, 100, EE_CHAR_FONTHEIGHT);
    pSet->Put(aFontSize);

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(3), pSet->Count());

    // Select all paragraphs and set spacing
    ESelection aSelection(0, 0, 0, aTextLen);
    aEditEngine.QuickSetAttribs(*pSet, aSelection);

    // Force multiple lines
    aEditEngine.SetPaperSize(Size(1000, 6000));
    CPPUNIT_ASSERT_EQUAL((sal_Int32)4, aEditEngine.GetLineCount(0));

    // Assert changes
    ParaPortion* pParaPortion = aEditEngine.GetParaPortions()[0];
    ContentNode* const pNode = pParaPortion->GetNode();
    const SfxPoolItem& rLSItem = pNode->GetContentAttribs().GetItem(EE_PARA_SBL);
    const SvxLineSpacingItem* pLSItem = static_cast<const SvxLineSpacingItem*>(&rLSItem);
    CPPUNIT_ASSERT(SvxInterLineSpaceRule::Prop == pLSItem->GetInterLineSpaceRule());
    CPPUNIT_ASSERT_EQUAL((sal_uInt16)60, pLSItem->GetPropLineSpace());

    // Check the first line
    ParagraphInfos aInfo = aEditEngine.GetParagraphInfos(0);
    CPPUNIT_ASSERT_EQUAL((sal_uInt16)122, aInfo.nFirstLineMaxAscent);
    CPPUNIT_ASSERT_EQUAL((sal_uInt16)153, (sal_uInt16)aEditEngine.GetLineHeight(0));

    // Prepare second case - 150%
    std::unique_ptr<SfxItemSet> pSet2(new SfxItemSet(aEditEngine.GetEmptyItemSet()));
    SvxLineSpacingItem aLineSpacing2(LINE_SPACE_DEFAULT_HEIGHT, EE_PARA_SBL);
    aLineSpacing2.SetPropLineSpace(150);
    pSet2->Put(aLineSpacing2);
    pSet2->Put(aFont);
    pSet2->Put(aFontSize);

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(3), pSet2->Count());

    // Select all paragraphs and set spacing
    aEditEngine.QuickSetAttribs(*pSet2, aSelection);

    // Assert changes
    const SfxPoolItem& rLSItem2 = pNode->GetContentAttribs().GetItem(EE_PARA_SBL);
    const SvxLineSpacingItem* pLSItem2 = static_cast<const SvxLineSpacingItem*>(&rLSItem2);
    CPPUNIT_ASSERT(SvxInterLineSpaceRule::Prop == pLSItem2->GetInterLineSpaceRule());
    CPPUNIT_ASSERT_EQUAL((sal_uInt16)150, pLSItem2->GetPropLineSpace());

    // Check the first line
    ParagraphInfos aInfo2 = aEditEngine.GetParagraphInfos(0);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(337), aInfo2.nFirstLineMaxAscent);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(382), static_cast<sal_uInt16>(aEditEngine.GetLineHeight(0)));
}

void Test::testUnoTextFields()
{
    {
        // DATE
        rtl::Reference<SvxUnoTextField> xField(new SvxUnoTextField(text::textfield::Type::DATE));
        uno::Sequence<OUString> aSvcs = xField->getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.DateTime");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // URL
        rtl::Reference<SvxUnoTextField> xField(new SvxUnoTextField(text::textfield::Type::URL));
        uno::Sequence<OUString> aSvcs = xField->getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.URL");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // PAGE
        rtl::Reference<SvxUnoTextField> xField(new SvxUnoTextField(text::textfield::Type::PAGE));
        uno::Sequence<OUString> aSvcs = xField->getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.PageNumber");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // PAGES
        rtl::Reference<SvxUnoTextField> xField(new SvxUnoTextField(text::textfield::Type::PAGES));
        uno::Sequence<OUString> aSvcs = xField->getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.PageCount");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // TIME
        rtl::Reference<SvxUnoTextField> xField(new SvxUnoTextField(text::textfield::Type::TIME));
        uno::Sequence<OUString> aSvcs = xField->getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.DateTime");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // FILE
        rtl::Reference<SvxUnoTextField> xField(new SvxUnoTextField(text::textfield::Type::DOCINFO_TITLE));
        uno::Sequence<OUString> aSvcs = xField->getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.docinfo.Title");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // TABLE
        rtl::Reference<SvxUnoTextField> xField(new SvxUnoTextField(text::textfield::Type::TABLE));
        uno::Sequence<OUString> aSvcs = xField->getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.SheetName");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // EXTENDED TIME
        rtl::Reference<SvxUnoTextField> xField(new SvxUnoTextField(text::textfield::Type::EXTENDED_TIME));
        uno::Sequence<OUString> aSvcs = xField->getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.DateTime");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // EXTENDED FILE
        rtl::Reference<SvxUnoTextField> xField(new SvxUnoTextField(text::textfield::Type::EXTENDED_FILE));
        uno::Sequence<OUString> aSvcs = xField->getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.FileName");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // AUTHOR
        rtl::Reference<SvxUnoTextField> xField(new SvxUnoTextField(text::textfield::Type::AUTHOR));
        uno::Sequence<OUString> aSvcs = xField->getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.Author");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // MEASURE
        rtl::Reference<SvxUnoTextField> xField(new SvxUnoTextField(text::textfield::Type::MEASURE));
        uno::Sequence<OUString> aSvcs = xField->getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.Measure");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // PRESENTATION HEADER
        rtl::Reference<SvxUnoTextField> xField(new SvxUnoTextField(text::textfield::Type::PRESENTATION_HEADER));
        uno::Sequence<OUString> aSvcs = xField->getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.presentation.textfield.Header");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // PRESENTATION FOOTER
        rtl::Reference<SvxUnoTextField> xField(new SvxUnoTextField(text::textfield::Type::PRESENTATION_FOOTER));
        uno::Sequence<OUString> aSvcs = xField->getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.presentation.textfield.Footer");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // PRESENTATION DATE TIME
        rtl::Reference<SvxUnoTextField> xField(new SvxUnoTextField(text::textfield::Type::PRESENTATION_DATE_TIME));
        uno::Sequence<OUString> aSvcs = xField->getSupportedServiceNames();
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
    virtual bool Delete( sal_Int32 nStt, sal_Int32 nEnd ) override
    {
        //fprintf(stderr, "TestAutoCorrDoc::Delete\n");
        m_rText = m_rText.replaceAt(nStt, nEnd-nStt, "");
        return true;
    }
    virtual bool Insert( sal_Int32 nPos, const OUString& rTxt ) override
    {
        //fprintf(stderr, "TestAutoCorrDoc::Insert\n");
        m_rText = m_rText.replaceAt(nPos, 0, rTxt);
        return true;
    }
    virtual bool Replace( sal_Int32 nPos, const OUString& rTxt ) override
    {
        //fprintf(stderr, "TestAutoCorrDoc::Replace\n");
        return ReplaceRange( nPos, rTxt.getLength(), rTxt );
    }
    virtual bool ReplaceRange( sal_Int32 nPos, sal_Int32 nLen, const OUString& rTxt ) override
    {
        //fprintf(stderr, "TestAutoCorrDoc::ReplaceRange %d %d %s\n", nPos, nLen, OUStringToOString(rTxt, RTL_TEXTENCODING_UTF8).getStr());
        m_rText = m_rText.replaceAt(nPos, nLen, rTxt);
        return true;
    }
    virtual void SetAttr( sal_Int32, sal_Int32, sal_uInt16, SfxPoolItem& ) override
    {
        //fprintf(stderr, "TestAutoCorrDoc::SetAttr\n");
    }
    virtual bool SetINetAttr( sal_Int32, sal_Int32, const OUString& ) override
    {
        //fprintf(stderr, "TestAutoCorrDoc::SetINetAttr\n");
        return true;
    }
    virtual OUString const* GetPrevPara(bool) override
    {
        //fprintf(stderr, "TestAutoCorrDoc::GetPrevPara\n");
        return nullptr;
    }
    virtual bool ChgAutoCorrWord( sal_Int32& rSttPos,
                sal_Int32 nEndPos, SvxAutoCorrect& rACorrect,
                OUString* pPara ) override
    {
        //fprintf(stderr, "TestAutoCorrDoc::ChgAutoCorrWord\n");

        if (m_rText.isEmpty())
            return false;

        LanguageTag aLanguageTag( m_eLang);
        const SvxAutocorrWord* pFnd = rACorrect.SearchWordsInList(
                m_rText, rSttPos, nEndPos, *this, aLanguageTag);
        if (pFnd && pFnd->IsTextOnly())
        {
            m_rText = m_rText.replaceAt(rSttPos, nEndPos, pFnd->GetLong());
            if( pPara )
                pPara->clear(); // =&pCurNode->GetString();
            return true;
        }

        return false;
    }
};

//https://bugs.libreoffice.org/show_bug.cgi?id=55693
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

        CPPUNIT_ASSERT_EQUAL_MESSAGE("autocorrect", sExpected, aFoo.getResult());
    }

    {
        OUString sInput("TEst/TEst");
        sal_Unicode cNextChar(' ');
        OUString sExpected("Test/Test ");

        TestAutoCorrDoc aFoo(sInput, LANGUAGE_ENGLISH_US);
        aAutoCorrect.DoAutoCorrect(aFoo, sInput, sInput.getLength(), cNextChar, true);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("autocorrect", sExpected, aFoo.getResult());
    }

    {
        // test auto-bolding with '*'
        OUString sInput("*foo");
        sal_Unicode cNextChar('*');
        OUString sExpected("foo");

        TestAutoCorrDoc aFoo(sInput, LANGUAGE_ENGLISH_US);
        aAutoCorrect.DoAutoCorrect(aFoo, sInput, sInput.getLength(), cNextChar, true);

        CPPUNIT_ASSERT_EQUAL(sExpected, aFoo.getResult());
    }

    {
        OUString sInput("Test. test");
        sal_Unicode cNextChar(' ');
        OUString sExpected("Test. Test ");

        TestAutoCorrDoc aFoo(sInput, LANGUAGE_ENGLISH_US);
        aAutoCorrect.DoAutoCorrect(aFoo, sInput, sInput.getLength(), cNextChar, true);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("autocorrect", sExpected, aFoo.getResult());
    }

    {
        OUString sInput("Test. \x01 test");
        sal_Unicode cNextChar(' ');
        OUString sExpected("Test. \x01 test ");

        TestAutoCorrDoc aFoo(sInput, LANGUAGE_ENGLISH_US);
        aAutoCorrect.DoAutoCorrect(aFoo, sInput, sInput.getLength(), cNextChar, true);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("autocorrect", sExpected, aFoo.getResult());
    }
}

namespace {
    class UrlEditEngine : public EditEngine
    {
    public:
        explicit UrlEditEngine(SfxItemPool *pPool) : EditEngine(pPool) {}

        virtual OUString CalcFieldValue( const SvxFieldItem&, sal_Int32, sal_Int32, Color*&, Color*& ) override
        {
            return OUString("jim@bob.com"); // a sophisticated view of value:
        }
    };
}

// Odd accounting for hyperlink position & size etc.
// https://bugzilla.novell.com/show_bug.cgi?id=467459
void Test::testHyperlinkSearch()
{
    UrlEditEngine aEngine(mpItemPool);
    EditDoc &rDoc = aEngine.GetEditDoc();

    OUString aSampleText = "Please write email to . if you find a fish(not a dog).";
    aEngine.SetText(aSampleText);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("set text", aSampleText, rDoc.GetParaAsString(sal_Int32(0)));

    ContentNode *pNode = rDoc.GetObject(0);
    EditSelection aSel(EditPaM(pNode, 22), EditPaM(pNode, 22));
    SvxURLField aURLField("mailto:///jim@bob.com", "jim@bob.com",
                          SVXURLFORMAT_REPR);
    SvxFieldItem aField(aURLField, EE_FEATURE_FIELD);

    aEngine.InsertField(aSel, aField);
    aEngine.UpdateFields();

    OUString aContent = pNode->GetExpandedText();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("get text", OUString("Please write email to jim@bob.com. if you find a fish(not a dog)."),
                           aContent);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong length", (sal_uLong)aContent.getLength(), rDoc.GetTextLen());

    // Check expansion and positioning re-work
    CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong length", (sal_uLong)aContent.getLength(),
                           pNode->GetExpandedLen());
    for (sal_Int32 n = 0; n < aContent.getLength(); n++)
    {
        sal_Int32 nStart = n, nEnd = n;
        pNode->UnExpandPositions(nStart,nEnd);
        CPPUNIT_ASSERT_MESSAGE("out of bound start", nStart < pNode->Len());
        CPPUNIT_ASSERT_MESSAGE("out of bound end", nEnd <= pNode->Len());
    }

    static const struct {
        sal_Int32 mnStart, mnEnd;
        sal_Int32 mnNewStart, mnNewEnd;
    } aTrickyOnes[] = {
        {  0,  1, /* -> */  0, 1 },
        { 21, 25, /* -> */ 21, 23 }, // the field is really just one char
        { 25, 27, /* -> */ 22, 23 },
        { 50, 56, /* -> */ 40, 46 }
    };
    for (size_t n = 0; n < SAL_N_ELEMENTS(aTrickyOnes); n++)
    {
        sal_Int32 nStart = aTrickyOnes[n].mnStart;
        sal_Int32 nEnd = aTrickyOnes[n].mnEnd;
        pNode->UnExpandPositions(nStart,nEnd);

        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OString("in row " + OString::number(n)).getStr(),
            aTrickyOnes[n].mnNewStart, nStart);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OString("in row " + OString::number(n)).getStr(),
            aTrickyOnes[n].mnNewEnd, nEnd);
    }

    SvxSearchItem aItem(1); //SID_SEARCH_ITEM);
    aItem.SetBackward(false);
    aItem.SetSelection(false);
    aItem.SetSearchString("fish");
    CPPUNIT_ASSERT_MESSAGE("no fish", aEngine.HasText(aItem));
    aItem.SetSearchString("dog");
    CPPUNIT_ASSERT_MESSAGE("no dog", aEngine.HasText(aItem));
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

    std::unique_ptr<SfxItemSet> pSet(new SfxItemSet(aEngine.GetEmptyItemSet()));
    SvxWeightItem aBold(WEIGHT_BOLD, EE_CHAR_WEIGHT);
    SvxPostureItem aItalic(ITALIC_NORMAL, EE_CHAR_ITALIC);

    {
        OUString aParaText = "aaabbbccc";
        aEngine.SetText(aParaText);
        pSet->Put(aBold);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be exactly one item.", static_cast<sal_uInt16>(1), pSet->Count());
        aEngine.QuickSetAttribs(*pSet, ESelection(0,0,0,6)); // 'aaabbb' - end point is not inclusive.
        pSet.reset(new SfxItemSet(aEngine.GetEmptyItemSet()));
        pSet->Put(aItalic);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be exactly one item.", static_cast<sal_uInt16>(1), pSet->Count());

        aEngine.QuickSetAttribs(*pSet, ESelection(0,3,0,9)); // 'bbbccc'
        std::unique_ptr<EditTextObject> pEditText(aEngine.CreateTextObject());
        CPPUNIT_ASSERT_MESSAGE("Failed to create text object.", pEditText.get());
        std::vector<editeng::Section> aAttrs;
        pEditText->GetAllSections(aAttrs);

        // Now, we should have a total of 3 sections.
        CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be 3 sections.", static_cast<size_t>(3), aAttrs.size());

        // First section should be 0-3 of paragraph 0, and it should only have boldness applied.
        const editeng::Section* pSecAttr = &aAttrs[0];
        CPPUNIT_ASSERT_EQUAL(0, (int)pSecAttr->mnParagraph);
        CPPUNIT_ASSERT_EQUAL(0, (int)pSecAttr->mnStart);
        CPPUNIT_ASSERT_EQUAL(3, (int)pSecAttr->mnEnd);
        CPPUNIT_ASSERT_EQUAL(1, (int)pSecAttr->maAttributes.size());
        CPPUNIT_ASSERT_MESSAGE("This section must be bold.", hasBold(*pSecAttr));

        // Second section should be 3-6, and it should be both bold and italic.
        pSecAttr = &aAttrs[1];
        CPPUNIT_ASSERT_EQUAL(0, (int)pSecAttr->mnParagraph);
        CPPUNIT_ASSERT_EQUAL(3, (int)pSecAttr->mnStart);
        CPPUNIT_ASSERT_EQUAL(6, (int)pSecAttr->mnEnd);
        CPPUNIT_ASSERT_EQUAL(2, (int)pSecAttr->maAttributes.size());
        CPPUNIT_ASSERT_MESSAGE("This section must be bold and italic.", hasBold(*pSecAttr) && hasItalic(*pSecAttr));

        // Third section should be 6-9, and it should be only italic.
        pSecAttr = &aAttrs[2];
        CPPUNIT_ASSERT_EQUAL(0, (int)pSecAttr->mnParagraph);
        CPPUNIT_ASSERT_EQUAL(6, (int)pSecAttr->mnStart);
        CPPUNIT_ASSERT_EQUAL(9, (int)pSecAttr->mnEnd);
        CPPUNIT_ASSERT_EQUAL(1, (int)pSecAttr->maAttributes.size());
        CPPUNIT_ASSERT_MESSAGE("This section must be italic.", hasItalic(*pSecAttr));
    }

    {
        // Set text consisting of 5 paragraphs with the 2nd and 4th paragraphs
        // being empty.
        aEngine.Clear();
        aEngine.SetText("one\n\ntwo\n\nthree");
        sal_Int32 nParaCount = aEngine.GetParagraphCount();
        sal_Int32 nCheck = 5;
        CPPUNIT_ASSERT_EQUAL(nCheck, nParaCount);

        // Apply boldness to paragraphs 1, 3, 5 only. Leave 2 and 4 unformatted.
        pSet.reset(new SfxItemSet(aEngine.GetEmptyItemSet()));
        pSet->Put(aBold);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be exactly one item.", static_cast<sal_uInt16>(1), pSet->Count());
        aEngine.QuickSetAttribs(*pSet, ESelection(0,0,0,3));
        aEngine.QuickSetAttribs(*pSet, ESelection(2,0,2,3));
        aEngine.QuickSetAttribs(*pSet, ESelection(4,0,4,5));

        std::unique_ptr<EditTextObject> pEditText(aEngine.CreateTextObject());
        CPPUNIT_ASSERT_MESSAGE("Failed to create text object.", pEditText.get());
        std::vector<editeng::Section> aAttrs;
        pEditText->GetAllSections(aAttrs);
        size_t nSecCountCheck = 5;
        CPPUNIT_ASSERT_EQUAL(nSecCountCheck, aAttrs.size());

        // 1st, 3rd and 5th sections should correspond with 1st, 3rd and 5th paragraphs.
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

        // The 2nd and 4th paragraphs should be empty.
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


    {
        aEngine.Clear();
        aEngine.SetText("one\ntwo");
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), aEngine.GetParagraphCount());

        // embolden 2nd paragraph
        pSet.reset(new SfxItemSet(aEngine.GetEmptyItemSet()));
        pSet->Put(aBold);
        aEngine.QuickSetAttribs(*pSet, ESelection(1,0,1,3));
        // disboldify 1st paragraph
        SvxWeightItem aNotSoBold(WEIGHT_NORMAL, EE_CHAR_WEIGHT);
        pSet->Put(aNotSoBold);
        aEngine.QuickSetAttribs(*pSet, ESelection(0,0,0,3));

        // now delete & join the paragraphs - this is fdo#85496 scenario
        aEngine.QuickDelete(ESelection(0,0,1,3));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aEngine.GetParagraphCount());

        std::unique_ptr<EditTextObject> pEditText(aEngine.CreateTextObject());
        CPPUNIT_ASSERT_MESSAGE("Failed to create text object.", pEditText.get());
        std::vector<editeng::Section> aAttrs;
        pEditText->GetAllSections(aAttrs);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aAttrs.size());

        const editeng::Section* pSecAttr = &aAttrs[0];
        CPPUNIT_ASSERT_EQUAL(0, (int)pSecAttr->mnParagraph);
        CPPUNIT_ASSERT_EQUAL(0, (int)pSecAttr->mnStart);
        CPPUNIT_ASSERT_EQUAL(0, (int)pSecAttr->mnEnd);
        std::set<sal_uInt16> whiches;
        for (size_t i = 0; i < pSecAttr->maAttributes.size(); ++i)
        {
            sal_uInt16 const nWhich(pSecAttr->maAttributes[i]->Which());
            CPPUNIT_ASSERT_MESSAGE("duplicate item in text portion attributes",
                whiches.insert(nWhich).second);
        }
    }
}

void Test::testHoriAlignIgnoreTrailingWhitespace()
{
    // Create EditEngine's instance
    EditEngine aEditEngine(mpItemPool);

    // Get EditDoc for current EditEngine's instance
    EditDoc &rDoc = aEditEngine.GetEditDoc();

    // Initially no text should be there
    CPPUNIT_ASSERT_EQUAL(sal_uLong(0), rDoc.GetTextLen());
    CPPUNIT_ASSERT_EQUAL(OUString(), rDoc.GetParaAsString(sal_Int32(0)));

    // Set initial text
    OUString aText = "Some text    ";
    sal_Int32 aTextLen = aText.getLength();
    aEditEngine.SetText(aText);

    // Assert changes - text insertion
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(aTextLen), rDoc.GetTextLen());
    CPPUNIT_ASSERT_EQUAL(aText, rDoc.GetParaAsString(static_cast<sal_Int32>(0)));

    // First test case: center alignment with compatibility option enabled
    {
        aEditEngine.SetHoriAlignIgnoreTrailingWhitespace(true);
        std::unique_ptr<SfxItemSet> pSet(new SfxItemSet(aEditEngine.GetEmptyItemSet()));
        pSet->Put(SvxAdjustItem( SVX_ADJUST_CENTER, EE_PARA_JUST ));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), pSet->Count());

        // Select all paragraphs and apply changes
        ESelection aSelection(0, 0, 0, aTextLen);
        aEditEngine.QuickSetAttribs(*pSet, aSelection);

        // Use a one line paragraph
        aEditEngine.SetPaperSize(Size(10000, 6000));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aEditEngine.GetLineCount(0));

        // Check horizontal position
        ParaPortion* pParaPortion = aEditEngine.GetParaPortions()[0];
        EditLine* pLine = &pParaPortion->GetLines()[0];
        CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<long>(4527), pLine->GetStartPosX(), 10);
    }

    // Second test case: center alignment with compatibility option disabled
    {
        aEditEngine.SetHoriAlignIgnoreTrailingWhitespace(false);
        std::unique_ptr<SfxItemSet> pSet(new SfxItemSet(aEditEngine.GetEmptyItemSet()));
        pSet->Put(SvxAdjustItem( SVX_ADJUST_CENTER, EE_PARA_JUST ));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), pSet->Count());

        // Select all paragraphs and apply changes
        ESelection aSelection(0, 0, 0, aTextLen);
        aEditEngine.QuickSetAttribs(*pSet, aSelection);

        // Use a one line paragraph
        aEditEngine.SetPaperSize(Size(10000, 6000));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aEditEngine.GetLineCount(0));

        // Check horizontal position
        ParaPortion* pParaPortion = aEditEngine.GetParaPortions()[0];
        EditLine* pLine = &pParaPortion->GetLines()[0];
        CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<long>(4407), pLine->GetStartPosX(), 10);
    }

    // Third test case: right alignment with compatibility option enabled
    {
        aEditEngine.SetHoriAlignIgnoreTrailingWhitespace(true);
        std::unique_ptr<SfxItemSet> pSet(new SfxItemSet(aEditEngine.GetEmptyItemSet()));
        pSet->Put(SvxAdjustItem( SVX_ADJUST_RIGHT, EE_PARA_JUST ));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), pSet->Count());

        // Select all paragraphs and apply changes
        ESelection aSelection(0, 0, 0, aTextLen);
        aEditEngine.QuickSetAttribs(*pSet, aSelection);

        // Use a one line paragraph
        aEditEngine.SetPaperSize(Size(10000, 6000));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aEditEngine.GetLineCount(0));

        // Check horizontal position
        ParaPortion* pParaPortion = aEditEngine.GetParaPortions()[0];
        EditLine* pLine = &pParaPortion->GetLines()[0];
        CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<long>(9054), pLine->GetStartPosX(), 10);
    }

    // Fourth test case: right alignment with compatibility option disabled
    {
        aEditEngine.SetHoriAlignIgnoreTrailingWhitespace(false);
        std::unique_ptr<SfxItemSet> pSet(new SfxItemSet(aEditEngine.GetEmptyItemSet()));
        pSet->Put(SvxAdjustItem( SVX_ADJUST_RIGHT, EE_PARA_JUST ));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), pSet->Count());

        // Select all paragraphs and apply changes
        ESelection aSelection(0, 0, 0, aTextLen);
        aEditEngine.QuickSetAttribs(*pSet, aSelection);

        // Use a one line paragraph
        aEditEngine.SetPaperSize(Size(10000, 6000));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aEditEngine.GetLineCount(0));

        // Check horizontal position
        ParaPortion* pParaPortion = aEditEngine.GetParaPortions()[0];
        EditLine* pLine = &pParaPortion->GetLines()[0];
        CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<long>(8815), pLine->GetStartPosX(), 10);
    }

    // Test multiple paragraph case
    {
        // Set initial text
        aText = "Some text    \nMore Text   ";
        aTextLen = aText.getLength();
        aEditEngine.SetText(aText);

        // Assert changes - text insertion
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(aTextLen - 1), rDoc.GetTextLen());
        CPPUNIT_ASSERT_EQUAL(OUString("Some text    "), rDoc.GetParaAsString(static_cast<sal_Int32>(0)));
        CPPUNIT_ASSERT_EQUAL(OUString("More Text   "), rDoc.GetParaAsString(static_cast<sal_Int32>(1)));

        aEditEngine.SetHoriAlignIgnoreTrailingWhitespace(true);
        std::unique_ptr<SfxItemSet> pSet(new SfxItemSet(aEditEngine.GetEmptyItemSet()));
        pSet->Put(SvxAdjustItem( SVX_ADJUST_CENTER, EE_PARA_JUST ));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), pSet->Count());

        // Select all paragraphs and apply changes
        ESelection aSelection(0, 0, 0, aTextLen);
        aEditEngine.QuickSetAttribs(*pSet, aSelection);

        // Get one line paragraphs
        aEditEngine.SetPaperSize(Size(10000, 6000));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aEditEngine.GetLineCount(0));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aEditEngine.GetLineCount(1));

        // Check horizontal position
        ParaPortion* pParaPortion = aEditEngine.GetParaPortions()[0];
        CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<long>(4527), pParaPortion->GetLines()[0].GetStartPosX(), 50);
        pParaPortion = aEditEngine.GetParaPortions()[1];
        CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<long>(8815), pParaPortion->GetLines()[0].GetStartPosX(), 50);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
