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

#include "sfx2/app.hxx"
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
#include "editeng/flditem.hxx"
#include "editeng/udlnitem.hxx"
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

    void testConstruction();

    /// Test UNO service class that implements text field items.
    void testUnoTextFields();

    /// AutoCorrect tests
    void testAutocorrect();

    /// Test Copy/Paste with hyperlinks in text using Legacy Format
    void testHyperlinkCopyPaste();

    /// Test Copy/Paste using Legacy Format
    void testCopyPaste();

    /// Test Copy/Paste with selective selection over multiple paragraphs
    void testMultiParaSelCopyPaste();

    /// Test Copy/Paste with Tabs
    void testTabsCopyPaste();

    /// Test hyperlinks
    void testHyperlinkSearch();

    /// Test Copy/Paste with Bold/Italic text using Legacy Format
    void testBoldItalicCopyPaste();

    /// Test Copy/Paste with Underline text using Legacy Format
    void testUnderlineCopyPaste();

    /// Test Copy/Paste with multiple paragraphs
    void testMultiParaCopyPaste();

    /// Test Copy/Paste with multiple paragraphs having Bold/Italic text
    void testParaBoldItalicCopyPaste();

    void testParaStartCopyPaste();

    void testSectionAttributes();

    void testLargeParaCopyPaste();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testConstruction);
    CPPUNIT_TEST(testUnoTextFields);
    CPPUNIT_TEST(testAutocorrect);
    CPPUNIT_TEST(testHyperlinkCopyPaste);
    CPPUNIT_TEST(testCopyPaste);
    CPPUNIT_TEST(testMultiParaSelCopyPaste);
    CPPUNIT_TEST(testTabsCopyPaste);
    CPPUNIT_TEST(testHyperlinkSearch);
    CPPUNIT_TEST(testBoldItalicCopyPaste);
    CPPUNIT_TEST(testUnderlineCopyPaste);
    CPPUNIT_TEST(testMultiParaCopyPaste);
    CPPUNIT_TEST(testParaBoldItalicCopyPaste);
    CPPUNIT_TEST(testParaStartCopyPaste);
    CPPUNIT_TEST(testSectionAttributes);
    CPPUNIT_TEST(testLargeParaCopyPaste);
    CPPUNIT_TEST_SUITE_END();

private:
    EditEngineItemPool* mpItemPool;
};

Test::Test() : mpItemPool(nullptr) {}

void Test::setUp()
{
    test::BootstrapFixture::setUp();

    mpItemPool = new EditEngineItemPool(true);

    SfxApplication::GetOrCreate();
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
        sal_Unicode const cNextChar(' ');
        OUString const sExpected("Test-Test ");

        TestAutoCorrDoc aFoo(sInput, LANGUAGE_ENGLISH_US);
        aAutoCorrect.DoAutoCorrect(aFoo, sInput, sInput.getLength(), cNextChar, true);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("autocorrect", sExpected, aFoo.getResult());
    }

    {
        OUString sInput("TEst/TEst");
        sal_Unicode const cNextChar(' ');
        OUString const sExpected("Test/Test ");

        TestAutoCorrDoc aFoo(sInput, LANGUAGE_ENGLISH_US);
        aAutoCorrect.DoAutoCorrect(aFoo, sInput, sInput.getLength(), cNextChar, true);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("autocorrect", sExpected, aFoo.getResult());
    }

    {
        // test auto-bolding with '*'
        OUString sInput("*foo");
        sal_Unicode const cNextChar('*');
        OUString const sExpected("foo");

        TestAutoCorrDoc aFoo(sInput, LANGUAGE_ENGLISH_US);
        aAutoCorrect.DoAutoCorrect(aFoo, sInput, sInput.getLength(), cNextChar, true);

        CPPUNIT_ASSERT_EQUAL(sExpected, aFoo.getResult());
    }

    {
        OUString sInput("Test. test");
        sal_Unicode const cNextChar(' ');
        OUString const sExpected("Test. Test ");

        TestAutoCorrDoc aFoo(sInput, LANGUAGE_ENGLISH_US);
        aAutoCorrect.DoAutoCorrect(aFoo, sInput, sInput.getLength(), cNextChar, true);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("autocorrect", sExpected, aFoo.getResult());
    }

    // don't autocapitalize after a field mark
    {
        OUString sInput("Test. \x01 test");
        sal_Unicode const cNextChar(' ');
        OUString const sExpected("Test. \x01 test ");

        TestAutoCorrDoc aFoo(sInput, LANGUAGE_ENGLISH_US);
        aAutoCorrect.DoAutoCorrect(aFoo, sInput, sInput.getLength(), cNextChar, true);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("autocorrect", sExpected, aFoo.getResult());
    }

    // consider field contents as text for auto quotes
    {
        OUString sInput("T\x01");
        sal_Unicode cNextChar('"');
        const sal_Unicode EXPECTED[] = { 'T', 0x01, 0x0201d };
        OUString sExpected(EXPECTED, SAL_N_ELEMENTS(EXPECTED));

        TestAutoCorrDoc aFoo(sInput, LANGUAGE_ENGLISH_US);
        aAutoCorrect.SetAutoCorrFlag(ChgQuotes, true);
        aAutoCorrect.DoAutoCorrect(aFoo, sInput, sInput.getLength(), cNextChar, true);
        fprintf(stderr, "text is %x\n", aFoo.getResult()[aFoo.getResult().getLength() - 1]);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("autocorrect", sExpected, aFoo.getResult());
    }

}

void Test::testHyperlinkCopyPaste()
{
    // Create EditEngine's instance
    EditEngine aEditEngine( mpItemPool );

    // Get EditDoc for current EditEngine's instance
    EditDoc &rDoc = aEditEngine.GetEditDoc();

    // New instance must be empty - no initial text
    CPPUNIT_ASSERT_EQUAL( sal_uLong(0), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( OUString(), rDoc.GetParaAsString(sal_Int32(0)) );

    // Get corresponding Field Item for inserting URLs in text
    // URL 1
    OUString aURL1 = "mailto:///user@example.com";
    OUString aRepres1 = "user@example.com";
    SvxURLField aURLField1( aURL1, aRepres1, SVXURLFORMAT_REPR );
    SvxFieldItem aField1( aURLField1, EE_FEATURE_FIELD );
    // URL 2
    OUString aURL2 = "mailto:///example@domain.com";
    OUString aRepres2 = "example@domain.com";
    SvxURLField aURLField2( aURL2, aRepres2, SVXURLFORMAT_REPR );
    SvxFieldItem aField2( aURLField2, EE_FEATURE_FIELD );

    // Insert initial text
    OUString aParaText = "sampletextfortestingfeaturefields";
    // Positions Ref      .............*13....*20..........
    sal_Int32 aTextLen = aParaText.getLength();
    aEditEngine.SetText( aParaText );

    // Assert changes
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( aParaText, rDoc.GetParaAsString(sal_Int32(0)) );

    // Insert URL 1
    ContentNode *pNode = rDoc.GetObject(0);
    EditSelection aSel1( EditPaM(pNode, 13), EditPaM(pNode, 13) );
    aEditEngine.InsertField( aSel1, aField1 );

    // Assert Field Count
    CPPUNIT_ASSERT_EQUAL( sal_uInt16(1), aEditEngine.GetFieldCount(0) );

    // Insert URL 2
    EditSelection aSel2( EditPaM(pNode, 20 + 1), EditPaM(pNode, 20 + 1) );
    aEditEngine.InsertField( aSel2, aField2 );

    // Assert Field Count
    CPPUNIT_ASSERT_EQUAL( sal_uInt16(2), aEditEngine.GetFieldCount(0) );

    // Update Fields
    aEditEngine.UpdateFields();

    // Assert URL Fields and text before copy
    // Check text
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen + 2), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( OUString("sampletextfor testing featurefields"), rDoc.GetParaAsString(sal_Int32(0)) );

    // Check Field 1
    EFieldInfo aURLFieldInfo1 = aEditEngine.GetFieldInfo( sal_Int32(0), sal_uInt16(0) );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(13), aURLFieldInfo1.aPosition.nIndex );
    CPPUNIT_ASSERT_EQUAL( sal_uInt16(EE_FEATURE_FIELD), aURLFieldInfo1.pFieldItem->Which() );
    SvxURLField* pURLField1 = dynamic_cast<SvxURLField*> ( const_cast<SvxFieldData*> (aURLFieldInfo1.pFieldItem->GetField()) );
    CPPUNIT_ASSERT_EQUAL( aURL1, pURLField1->GetURL() );
    CPPUNIT_ASSERT_EQUAL( aRepres1, pURLField1->GetRepresentation() );

    // Check Field 2
    EFieldInfo aURLFieldInfo2 = aEditEngine.GetFieldInfo( sal_Int32(0), sal_uInt16(1) );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(21), aURLFieldInfo2.aPosition.nIndex );
    CPPUNIT_ASSERT_EQUAL( sal_uInt16(EE_FEATURE_FIELD), aURLFieldInfo2.pFieldItem->Which() );
    SvxURLField* pURLField2 = dynamic_cast<SvxURLField*> ( const_cast<SvxFieldData*> (aURLFieldInfo2.pFieldItem->GetField()) );
    CPPUNIT_ASSERT_EQUAL( aURL2, pURLField2->GetURL() );
    CPPUNIT_ASSERT_EQUAL( aRepres2, pURLField2->GetRepresentation() );

    // Copy text using legacy format
    uno::Reference< datatransfer::XTransferable > xData = aEditEngine.CreateTransferable( ESelection(0,10,0,21) );

    // Paste text at the end
    aEditEngine.InsertText( xData, OUString(), rDoc.GetEndPaM(), true );

    // Assert Changes ACP, ACP: after Copy/Paste

    // Check the fields count
    // TODO: Fix copy/paste of hyperlinks: currently hyperlinks are not copied properly, there is some bug
    // For now we expect the following
    CPPUNIT_ASSERT_EQUAL( sal_uInt16(2), aEditEngine.GetFieldCount(0) );
    // After having a fix - we expect the following as a replacement of above
    // CPPUNIT_ASSERT_EQUAL( sal_uInt16(3), aEditEngine.GetFieldCount(0) );

    // Check the updated text length
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen + 10 + 2 + 1), rDoc.GetTextLen() );

    // Check the updated text contents
    // TODO: Fix copy/paste of hyperlinks: currently hyperlinks are not copied properly, there is some bug
    // For now we expect the following
    CPPUNIT_ASSERT_EQUAL( OUString("sampletextfor testing featurefieldsfor\001testing"), rDoc.GetParaAsString(sal_Int32(0)) );
    // After having a fix - we expect the following as a replacement of above
    // CPPUNIT_ASSERT_EQUAL( OUString("sampletextfor testing featurefieldsfor testing"), rDoc.GetParaAsString(sal_Int32(0)) );

    // Check the Fields and their values

    // Field 1
    EFieldInfo aACPURLFieldInfo1 = aEditEngine.GetFieldInfo( sal_Int32(0), sal_uInt16(0) );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(13), aACPURLFieldInfo1.aPosition.nIndex );
    CPPUNIT_ASSERT_EQUAL( sal_uInt16(EE_FEATURE_FIELD), aACPURLFieldInfo1.pFieldItem->Which() );
    SvxURLField* pACPURLField1 = dynamic_cast<SvxURLField*> ( const_cast<SvxFieldData*> (aACPURLFieldInfo1.pFieldItem->GetField()) );
    CPPUNIT_ASSERT_EQUAL( aURL1, pACPURLField1->GetURL() );
    CPPUNIT_ASSERT_EQUAL( aRepres1, pACPURLField1->GetRepresentation() );

    // Field 2
    EFieldInfo aACPURLFieldInfo2 = aEditEngine.GetFieldInfo( sal_Int32(0), sal_uInt16(1) );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(21), aACPURLFieldInfo2.aPosition.nIndex );
    CPPUNIT_ASSERT_EQUAL( sal_uInt16(EE_FEATURE_FIELD), aACPURLFieldInfo2.pFieldItem->Which() );
    SvxURLField* pACPURLField2 = dynamic_cast<SvxURLField*> ( const_cast<SvxFieldData*> (aACPURLFieldInfo2.pFieldItem->GetField()) );
    CPPUNIT_ASSERT_EQUAL( aURL2, pACPURLField2->GetURL() );
    CPPUNIT_ASSERT_EQUAL( aRepres2, pACPURLField2->GetRepresentation() )    ;

    // Field 3
    // TODO: Fix copy/paste of hyperlinks: currently hyperlinks are not copied properly, there is some bug
    // After having a fix we expect the following
    //EFieldInfo aACPURLFieldInfo3 = aEditEngine.GetFieldInfo( sal_Int32(0), sal_uInt16(2) );
    //CPPUNIT_ASSERT_EQUAL( sal_Int32(38), aACPURLFieldInfo3.aPosition.nIndex );
    //CPPUNIT_ASSERT_EQUAL( sal_uInt16(EE_FEATURE_FIELD), aACPURLFieldInfo3.pFieldItem->Which() );
    //SvxURLField* pACPURLField3 = dynamic_cast<SvxURLField*> ( const_cast<SvxFieldData*> (aACPURLFieldInfo3.pFieldItem->GetField()) );
    //CPPUNIT_ASSERT_EQUAL( aURL1, pACPURLField3->GetURL() );
    //CPPUNIT_ASSERT_EQUAL( aRepres1, pACPURLField3->GetRepresentation() );
}

void Test::testCopyPaste()
{
    // Create EditEngine's instance
    EditEngine aEditEngine( mpItemPool );

    // Get EditDoc for current EditEngine's instance
    EditDoc &rDoc = aEditEngine.GetEditDoc();

    // Initially no text should be there
    CPPUNIT_ASSERT_EQUAL( sal_uLong(0), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( OUString(), rDoc.GetParaAsString(sal_Int32(0)) );

    // Set initial text
    OUString aText = "This is custom initial text";
    sal_Int32 aTextLen = aText.getLength();
    aEditEngine.SetText( aText );

    // Assert changes
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( aText, rDoc.GetParaAsString(sal_Int32(0)) );

    // Copy initial text using legacy format
    uno::Reference< datatransfer::XTransferable > xData = aEditEngine.CreateTransferable( ESelection(0,0,0,aTextLen) );

    // Paste text at the end
    aEditEngine.InsertText( xData, OUString(), rDoc.GetEndPaM(), true );

    // Assert changes
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen + aTextLen), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( OUString(aText + aText), rDoc.GetParaAsString(sal_Int32(0)) );
}

void Test::testMultiParaSelCopyPaste()
{
    // Create EditEngine's instance
    EditEngine aEditEngine( mpItemPool );

    // Get EditDoc for current EditEngine's instance
    EditDoc &rDoc = aEditEngine.GetEditDoc();

    // Initially no text should be there
    CPPUNIT_ASSERT_EQUAL( sal_uLong(0), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( OUString(), rDoc.GetParaAsString(sal_Int32(0)) );

    // Insert initial text
    OUString aFirstPara = "This is first paragraph";
    // Selection Ref       ........8..............
    OUString aSecondPara = "This is second paragraph";
    // Selection Ref        .............14.........
    OUString aThirdPara = "This is third paragraph";
    OUString aText = aFirstPara + "\n" + aSecondPara + "\n" + aThirdPara;
    sal_Int32 aTextLen = aFirstPara.getLength() + aSecondPara.getLength() + aThirdPara.getLength();
    aEditEngine.SetText( aText );
    OUString aCopyText = "first paragraphThis is second";
    sal_Int32 aCopyTextLen = aCopyText.getLength();

    // Assert changes
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( aFirstPara, rDoc.GetParaAsString(sal_Int32(0)) );
    CPPUNIT_ASSERT_EQUAL( aSecondPara, rDoc.GetParaAsString(sal_Int32(1)) );
    CPPUNIT_ASSERT_EQUAL( aThirdPara, rDoc.GetParaAsString(sal_Int32(2)) );

    // Copy initial text using legacy format
    uno::Reference< datatransfer::XTransferable > xData = aEditEngine.CreateTransferable( ESelection(0,8,1,14) );

    // Paste text at the end
    aEditEngine.InsertText( xData, OUString(), rDoc.GetEndPaM(), true );

    // Assert changes
    OUString aThirdParaAfterCopyPaste = aThirdPara + "first paragraph";
    OUString aFourthPara = "This is second";
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen + aCopyTextLen), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( aFirstPara, rDoc.GetParaAsString(sal_Int32(0)) );
    CPPUNIT_ASSERT_EQUAL( aSecondPara, rDoc.GetParaAsString(sal_Int32(1)) );
    CPPUNIT_ASSERT_EQUAL( aThirdParaAfterCopyPaste, rDoc.GetParaAsString(sal_Int32(2)) );
    CPPUNIT_ASSERT_EQUAL( aFourthPara, rDoc.GetParaAsString(sal_Int32(3)) );
}

void Test::testTabsCopyPaste()
{
    // Create EditEngine's instance
    EditEngine aEditEngine( mpItemPool );

    // Get EditDoc for current EditEngine's instance
    EditDoc &rDoc = aEditEngine.GetEditDoc();

    // New instance must be empty - no initial text
    CPPUNIT_ASSERT_EQUAL( sal_uLong(0), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( OUString(), rDoc.GetParaAsString(sal_Int32(0)) );

    // Get corresponding Item for inserting tabs in the text
    SfxVoidItem aTab( EE_FEATURE_TAB );

    // Insert initial text
    OUString aParaText = "sampletextfortestingtab";
    // Positions Ref      ......*6...............*23
    sal_Int32 aTextLen = aParaText.getLength();
    aEditEngine.SetText( aParaText );

    // Assert changes
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( aParaText, rDoc.GetParaAsString(sal_Int32(0)) );

    // Insert tab 1 at desired position
    ContentNode *pNode = rDoc.GetObject(0);
    EditSelection aSel1( EditPaM(pNode, 6), EditPaM(pNode, 6) );
    aEditEngine.InsertFeature( aSel1, aTab );

    // Assert changes
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen + 1), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( OUString("sample\ttextfortestingtab"), rDoc.GetParaAsString(sal_Int32(0)) );

    // Insert tab 2 at desired position
    EditSelection aSel2( EditPaM(pNode, 23+1), EditPaM(pNode, 23+1) );
    aEditEngine.InsertFeature( aSel2, aTab );

    // Assert changes
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen + 2), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( OUString("sample\ttextfortestingtab\t"), rDoc.GetParaAsString(sal_Int32(0)) );

    // Copy text using legacy format
    uno::Reference< datatransfer::XTransferable > xData = aEditEngine.CreateTransferable( ESelection(0,6,0,aTextLen+2) );

    // Paste text at the end
    aEditEngine.InsertText( xData, OUString(), rDoc.GetEndPaM(), true );

    // Assert changes
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen + aTextLen - 6 + 4 ), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( OUString("sample\ttextfortestingtab\t\ttextfortestingtab\t"), rDoc.GetParaAsString(sal_Int32(0)) );
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

void Test::testBoldItalicCopyPaste()
{
    // Create EditEngine's instance
    EditEngine aEditEngine( mpItemPool );

    // Get EditDoc for current EditEngine's instance
    EditDoc &rDoc = aEditEngine.GetEditDoc();

    // New instance must be empty - no initial text
    CPPUNIT_ASSERT_EQUAL( sal_uLong(0), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( OUString(), rDoc.GetParaAsString(sal_Int32(0)) );

    // Get corresponding ItemSet for inserting Bold/Italic text
    std::unique_ptr<SfxItemSet> pSet( new SfxItemSet(aEditEngine.GetEmptyItemSet()) );
    SvxWeightItem aBold( WEIGHT_BOLD, EE_CHAR_WEIGHT );
    SvxPostureItem aItalic( ITALIC_NORMAL, EE_CHAR_ITALIC );

    // Insert initial text
    OUString aParaText = "boldeditengineitalic";
    // Positions Ref      ..*2....*8...*13.*17
    // Bold Ref           ..[   BOLD   ]......
    // Italic Ref         ........[ ITALIC ]..
    sal_Int32 aTextLen = aParaText.getLength();
    aEditEngine.SetText( aParaText );

    // Assert changes - text insertion
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( aParaText, rDoc.GetParaAsString(sal_Int32(0)) );

    // Apply Bold to appropriate selection
    pSet->Put(aBold);
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(1), pSet->Count() );
    aEditEngine.QuickSetAttribs( *pSet, ESelection(0,2,0,14) );

    // Assert changes
    std::unique_ptr<EditTextObject> pEditText1( aEditEngine.CreateTextObject() );
    std::vector<editeng::Section> aAttrs1;
    pEditText1->GetAllSections( aAttrs1 );
    // There should be 3 sections - woBold - wBold - woBold (w - with, wo - without)
    CPPUNIT_ASSERT_EQUAL( size_t(3), aAttrs1.size() );

    const editeng::Section* pSecAttr = &aAttrs1[0];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    pSecAttr = &aAttrs1[1];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 14, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold.", hasBold(*pSecAttr) );

    pSecAttr = &aAttrs1[2];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 14, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 20, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    // Apply Italic to appropriate selection
    pSet.reset( new SfxItemSet(aEditEngine.GetEmptyItemSet()) );
    pSet->Put(aItalic);
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(1), pSet->Count() );
    aEditEngine.QuickSetAttribs( *pSet, ESelection(0,8,0,18) );

    // Assert changes
    std::unique_ptr<EditTextObject> pEditText2( aEditEngine.CreateTextObject() );
    std::vector<editeng::Section> aAttrs2;
    pEditText2->GetAllSections( aAttrs2 );
    // There should be 5 sections - woBold&woItalic - wBold&woItalic - wBold&wItalic - woBold&wItalic - woBold&woItalic (w - with, wo - without)
    CPPUNIT_ASSERT_EQUAL( size_t(5), aAttrs2.size() );

    pSecAttr = &aAttrs2[0];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    pSecAttr = &aAttrs2[1];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 8, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold.", hasBold(*pSecAttr) );

    pSecAttr = &aAttrs2[2];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 8, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 14, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold and italic.", hasBold(*pSecAttr) && hasItalic(*pSecAttr) );

    pSecAttr = &aAttrs2[3];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 14, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 18, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be italic.", hasItalic(*pSecAttr) );

    pSecAttr = &aAttrs2[4];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 18, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 20, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    // Copy text using legacy format
    uno::Reference< datatransfer::XTransferable > xData = aEditEngine.CreateTransferable( ESelection(0,1,0,aTextLen-1) );

    // Paste text at the end
    aEditEngine.InsertText( xData, OUString(), rDoc.GetEndPaM(), true );

    // Assert changes
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen + aTextLen - 2), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( OUString(aParaText + "oldeditengineitali" ), rDoc.GetParaAsString(sal_Int32(0)) );

    // Check updated text for appropriate Bold/Italics
    std::unique_ptr<EditTextObject> pEditText3( aEditEngine.CreateTextObject() );
    std::vector<editeng::Section> aAttrs3;
    pEditText3->GetAllSections( aAttrs3 );
    // There should be 9 sections - woBold&woItalic - wBold&woItalic - wBold&wItalic - woBold&wItalic - woBold&woItalic - wBold&woItalic
    // - wBold&wItalic - woBold&wItalic - woBold&woItalic(w - with, wo - without)
    CPPUNIT_ASSERT_EQUAL( size_t(9), aAttrs3.size() );

    pSecAttr = &aAttrs3[0];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    pSecAttr = &aAttrs3[1];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 8, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold.", hasBold(*pSecAttr) );

    pSecAttr = &aAttrs3[2];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 8, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 14, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold and italic.", hasBold(*pSecAttr) && hasItalic(*pSecAttr) );

    pSecAttr = &aAttrs3[3];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 14, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 18, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be italic.", hasItalic(*pSecAttr) );

    pSecAttr = &aAttrs3[4];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 18, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 21, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    pSecAttr = &aAttrs3[5];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 21, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 27, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold.", hasBold(*pSecAttr) );

    pSecAttr = &aAttrs3[6];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 27, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 33, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold and italic.", hasBold(*pSecAttr) && hasItalic(*pSecAttr) );

    pSecAttr = &aAttrs3[7];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 33, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 37, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be italic.", hasItalic(*pSecAttr) );

    pSecAttr = &aAttrs3[8];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 37, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 38, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );
}

// Auxiliary function to test Underline text Copy/Paste using Legacy Format
bool hasUnderline(const editeng::Section& rSecAttr)
{
    std::vector<const SfxPoolItem*>::const_iterator it = rSecAttr.maAttributes.begin(), itEnd = rSecAttr.maAttributes.end();
    for (; it != itEnd; ++it)
    {
        const SfxPoolItem* p = *it;
        if (p->Which() != EE_CHAR_UNDERLINE)
            continue;

        if (static_cast<const SvxUnderlineItem*>(p)->GetLineStyle() != LINESTYLE_SINGLE)
            continue;

        return true;
    }
    return false;
}

void Test::testUnderlineCopyPaste()
{
    // Create EditEngine's instance
    EditEngine aEditEngine( mpItemPool );

    // Get EditDoc for current EditEngine's instance
    EditDoc &rDoc = aEditEngine.GetEditDoc();

    // New instance must be empty - no initial text
    CPPUNIT_ASSERT_EQUAL( sal_uLong(0), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( OUString(), rDoc.GetParaAsString(sal_Int32(0)) );

    // Get corresponding ItemSet for inserting Underline text
    std::unique_ptr<SfxItemSet> pSet( new SfxItemSet(aEditEngine.GetEmptyItemSet()) );
    SvxUnderlineItem aULine( LINESTYLE_SINGLE, EE_CHAR_UNDERLINE );

    // Insert initial text
    OUString aParaText = "sampletextforunderline";
    // Positions Ref      ......*6.........*17..
    // Underline Ref      ......[UNDERLINE ]....
    sal_Int32 aTextLen = aParaText.getLength();
    aEditEngine.SetText( aParaText );

    // Apply Underline style
    pSet->Put( aULine );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(1), pSet->Count() );
    aEditEngine.QuickSetAttribs( *pSet, ESelection(0,6,0,18) );

    // Assert changes
    std::unique_ptr<EditTextObject> pEditText1( aEditEngine.CreateTextObject() );
    std::vector<editeng::Section> aAttrs1;
    pEditText1->GetAllSections( aAttrs1 );

    // There should be 3 sections - woUnderline - wUnderline - woUnderline (w - with, wo - without)
    CPPUNIT_ASSERT_EQUAL( size_t(3), aAttrs1.size() );

    const editeng::Section* pSecAttr = &aAttrs1[0];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 6, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    pSecAttr = &aAttrs1[1];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 6, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 18, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be underlined.", hasUnderline(*pSecAttr) );

    pSecAttr = &aAttrs1[2];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 18, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 22, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    // Copy text using legacy format
    uno::Reference< datatransfer::XTransferable > xData = aEditEngine.CreateTransferable( ESelection(0,6,0,aTextLen-4) );

    // Paste text at the end
    aEditEngine.InsertText( xData, OUString(), rDoc.GetEndPaM(), true );

    // Assert changes
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen + (OUString("textforunder")).getLength()), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( OUString(aParaText + "textforunder" ), rDoc.GetParaAsString(sal_Int32(0)) );

    // Check updated text for appropriate Underline
    std::unique_ptr<EditTextObject> pEditText2( aEditEngine.CreateTextObject() );
    std::vector<editeng::Section> aAttrs2;
    pEditText2->GetAllSections( aAttrs2 );

    // There should be 4 sections - woUnderline - wUnderline - woUnderline - wUnderline (w - with, wo - without)
    CPPUNIT_ASSERT_EQUAL( size_t(4), aAttrs2.size() );

    pSecAttr = &aAttrs2[0];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 6, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    pSecAttr = &aAttrs2[1];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 6, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 18, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be underlined.", hasUnderline(*pSecAttr) );

    pSecAttr = &aAttrs2[2];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 18, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 22, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    pSecAttr = &aAttrs2[3];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 22, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 34, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be underlined.", hasUnderline(*pSecAttr) );
}

void Test::testMultiParaCopyPaste()
{
    // Create EditEngine's instance
    EditEngine aEditEngine( mpItemPool );

    // Get EditDoc for current EditEngine's instance
    EditDoc &rDoc = aEditEngine.GetEditDoc();

    // Initially no text should be there
    CPPUNIT_ASSERT_EQUAL( sal_uLong(0), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( OUString(), rDoc.GetParaAsString(sal_Int32(0)) );

    // Insert initial text
    OUString aFirstPara = "This is first paragraph";
    OUString aSecondPara = "This is second paragraph";
    OUString aThirdPara = "This is third paragraph";
    OUString aText = aFirstPara + "\n" + aSecondPara + "\n" + aThirdPara;
    sal_Int32 aTextLen = aFirstPara.getLength() + aSecondPara.getLength() + aThirdPara.getLength();
    aEditEngine.SetText( aText );
    sal_Int32 aCopyTextLen = aFirstPara.getLength() + aSecondPara.getLength();

    // Assert changes
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( aFirstPara, rDoc.GetParaAsString(sal_Int32(0)) );
    CPPUNIT_ASSERT_EQUAL( aSecondPara, rDoc.GetParaAsString(sal_Int32(1)) );
    CPPUNIT_ASSERT_EQUAL( aThirdPara, rDoc.GetParaAsString(sal_Int32(2)) );

    // Copy initial text using legacy format
    uno::Reference< datatransfer::XTransferable > xData = aEditEngine.CreateTransferable( ESelection(0,0,1,aSecondPara.getLength()) );

    // Paste text at the end
    aEditEngine.InsertText( xData, OUString(), rDoc.GetEndPaM(), true );

    // Assert changes
    OUString aThirdParaAfterCopyPaste = aThirdPara + aFirstPara;
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen + aCopyTextLen), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( aFirstPara, rDoc.GetParaAsString(sal_Int32(0)) );
    CPPUNIT_ASSERT_EQUAL( aSecondPara, rDoc.GetParaAsString(sal_Int32(1)) );
    CPPUNIT_ASSERT_EQUAL( aThirdParaAfterCopyPaste, rDoc.GetParaAsString(sal_Int32(2)) );
    CPPUNIT_ASSERT_EQUAL( aSecondPara, rDoc.GetParaAsString(sal_Int32(3)) );
}

void Test::testParaBoldItalicCopyPaste()
{
    // Create EditEngine's instance
    EditEngine aEditEngine( mpItemPool );

    // Get EditDoc for current EditEngine's instance
    EditDoc &rDoc = aEditEngine.GetEditDoc();

    // Initially no text should be there
    CPPUNIT_ASSERT_EQUAL( sal_uLong(0), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( OUString(), rDoc.GetParaAsString(sal_Int32(0)) );

    // Get corresponding ItemSet for inserting Bold/Italic text
    std::unique_ptr<SfxItemSet> pSet( new SfxItemSet(aEditEngine.GetEmptyItemSet()) );
    SvxWeightItem aBold( WEIGHT_BOLD, EE_CHAR_WEIGHT );
    SvxPostureItem aItalic( ITALIC_NORMAL, EE_CHAR_ITALIC );

    // Insert initial text
    OUString aFirstPara = "This is first paragraph";
    // Positions Ref       .....*5.*8....*14*17...
    // Bold Ref            .....[    BOLD   ].....
    // Italic Ref          ..............[     ITA
    // Copy Ref            ........[      Copy Sel
    OUString aSecondPara = "This is second paragraph";
    // Positions Ref        .....*5.*8...*13..*18...
    // Bold Ref             .....[    BOLD    ].....
    // Italic Ref           LIC     ]...............
    // Copy Ref             ection       ]..........
    OUString aThirdPara = "This is third paragraph";
    OUString aText = aFirstPara + "\n" + aSecondPara + "\n" + aThirdPara;
    sal_Int32 aTextLen = aFirstPara.getLength() + aSecondPara.getLength() + aThirdPara.getLength();
    aEditEngine.SetText( aText );
    OUString aCopyText = "first paragraphThis is second";
    sal_Int32 aCopyTextLen = aCopyText.getLength();

    // Assert changes - text insertion
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( aFirstPara, rDoc.GetParaAsString(sal_Int32(0)) );
    CPPUNIT_ASSERT_EQUAL( aSecondPara, rDoc.GetParaAsString(sal_Int32(1)) );
    CPPUNIT_ASSERT_EQUAL( aThirdPara, rDoc.GetParaAsString(sal_Int32(2)) );

    // Apply Bold to appropriate selections
    pSet->Put(aBold);
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(1), pSet->Count() );
    aEditEngine.QuickSetAttribs( *pSet, ESelection(0,5,0,18) );
    aEditEngine.QuickSetAttribs( *pSet, ESelection(1,5,1,19) );

    // Assert changes
    std::unique_ptr<EditTextObject> pEditText1( aEditEngine.CreateTextObject() );
    std::vector<editeng::Section> aAttrs1;
    pEditText1->GetAllSections( aAttrs1 );
    // There should be 7 sections - woB - wB - woB -woB -wB -woB -woB (w - with, wo - without, B - Bold, I - Italic)
    CPPUNIT_ASSERT_EQUAL( size_t(7), aAttrs1.size() );

    const editeng::Section* pSecAttr = &aAttrs1[0];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 5, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    pSecAttr = &aAttrs1[1];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 5, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 18, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold.", hasBold(*pSecAttr) );

    pSecAttr = &aAttrs1[2];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 18, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 23, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    pSecAttr = &aAttrs1[3];
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 5, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    pSecAttr = &aAttrs1[4];
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 5, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 19, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold.", hasBold(*pSecAttr) );

    pSecAttr = &aAttrs1[5];
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 19, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 24, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    pSecAttr = &aAttrs1[6];
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 23, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    // Apply Italic to appropriate selection
    pSet.reset( new SfxItemSet(aEditEngine.GetEmptyItemSet()) );
    pSet->Put(aItalic);
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(1), pSet->Count() );
    aEditEngine.QuickSetAttribs( *pSet, ESelection(0,14,1,9) );

    // Assert changes
    std::unique_ptr<EditTextObject> pEditText2( aEditEngine.CreateTextObject() );
    std::vector<editeng::Section> aAttrs2;
    pEditText2->GetAllSections( aAttrs2 );
    // There should be 9 sections - woB&woI - wB&woI - wB&wI -woB&wI - woB&wI - wB&wI - wB&woI - woB&woI - woB&woI (w - with, wo - without, B - Bold, I - Italic)
    CPPUNIT_ASSERT_EQUAL( size_t(9), aAttrs2.size() );

    pSecAttr = &aAttrs2[0];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 5, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    pSecAttr = &aAttrs2[1];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 5, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 14, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold.", hasBold(*pSecAttr) );

    pSecAttr = &aAttrs2[2];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 14, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 18, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold and italic.", hasBold(*pSecAttr) && hasItalic(*pSecAttr) );

    pSecAttr = &aAttrs2[3];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 18, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 23, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be italic.", hasItalic(*pSecAttr) );

    pSecAttr = &aAttrs2[4];
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 5, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be italic.", hasItalic(*pSecAttr) );

    pSecAttr = &aAttrs2[5];
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 5, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 9, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold and italic.", hasBold(*pSecAttr) && hasItalic(*pSecAttr) );

    pSecAttr = &aAttrs2[6];
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 9, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 19, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold.", hasBold(*pSecAttr) );

    pSecAttr = &aAttrs2[7];
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 19, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 24, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    pSecAttr = &aAttrs2[8];
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 23, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    // Copy text using legacy format
    uno::Reference< datatransfer::XTransferable > xData = aEditEngine.CreateTransferable( ESelection(0,8,1,14) );

    // Paste text at the end
    aEditEngine.InsertText( xData, OUString(), rDoc.GetEndPaM(), true );

    // Assert changes
    OUString aThirdParaAfterCopyPaste = aThirdPara + "first paragraph";
    OUString aFourthParaAfterCopyPaste = "This is second";
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen + aCopyTextLen), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( aFirstPara, rDoc.GetParaAsString(sal_Int32(0)) );
    CPPUNIT_ASSERT_EQUAL( aSecondPara, rDoc.GetParaAsString(sal_Int32(1)) );
    CPPUNIT_ASSERT_EQUAL( aThirdParaAfterCopyPaste, rDoc.GetParaAsString(sal_Int32(2)) );
    CPPUNIT_ASSERT_EQUAL( aFourthParaAfterCopyPaste, rDoc.GetParaAsString(sal_Int32(3)) );

    // Check updated text for appropriate Bold/Italics
    std::unique_ptr<EditTextObject> pEditText3( aEditEngine.CreateTextObject() );
    std::vector<editeng::Section> aAttrs3;
    pEditText3->GetAllSections( aAttrs3 );
    // There should be 15 sections - woB&woI - wB&woI - wB&wI -woB&wI - woB&wI - wB&wI - wB&woI - woB&woI - woB&woI
    // - wB&woI - wB&wI - woB&wI - -woB&wI - wB&wI - wB&woI (w - with, wo - without, B - Bold, I - Italic)
    CPPUNIT_ASSERT_EQUAL( size_t(15), aAttrs3.size() );

    pSecAttr = &aAttrs3[0];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 5, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    pSecAttr = &aAttrs3[1];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 5, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 14, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold.", hasBold(*pSecAttr) );

    pSecAttr = &aAttrs3[2];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 14, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 18, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold and italic.", hasBold(*pSecAttr) && hasItalic(*pSecAttr) );

    pSecAttr = &aAttrs3[3];
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 18, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 23, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be italic.", hasItalic(*pSecAttr) );

    pSecAttr = &aAttrs3[4];
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 5, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be italic.", hasItalic(*pSecAttr) );

    pSecAttr = &aAttrs3[5];
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 5, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 9, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold and italic.", hasBold(*pSecAttr) && hasItalic(*pSecAttr) );

    pSecAttr = &aAttrs3[6];
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 9, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 19, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold.", hasBold(*pSecAttr) );

    pSecAttr = &aAttrs3[7];
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 19, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 24, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    pSecAttr = &aAttrs3[8];
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 23, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->maAttributes.size() );

    pSecAttr = &aAttrs3[9];
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 23, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 29, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold.", hasBold(*pSecAttr) );

    pSecAttr = &aAttrs3[10];
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 29, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 33, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold and italic.", hasBold(*pSecAttr) && hasItalic(*pSecAttr) );

    pSecAttr = &aAttrs3[11];
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 33, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 38, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be italic.", hasItalic(*pSecAttr) );

    pSecAttr = &aAttrs3[12];
    CPPUNIT_ASSERT_EQUAL( 3, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 0, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 5, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be italic.", hasItalic(*pSecAttr) );

    pSecAttr = &aAttrs3[13];
    CPPUNIT_ASSERT_EQUAL( 3, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 5, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 9, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 2, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold and italic.", hasBold(*pSecAttr) && hasItalic(*pSecAttr) );

    pSecAttr = &aAttrs3[14];
    CPPUNIT_ASSERT_EQUAL( 3, (int)pSecAttr->mnParagraph );
    CPPUNIT_ASSERT_EQUAL( 9, (int)pSecAttr->mnStart );
    CPPUNIT_ASSERT_EQUAL( 14, (int)pSecAttr->mnEnd );
    CPPUNIT_ASSERT_EQUAL( 1, (int)pSecAttr->maAttributes.size() );
    CPPUNIT_ASSERT_MESSAGE( "This section must be bold.", hasBold(*pSecAttr) );
}

void Test::testParaStartCopyPaste()
{
    // Create EditEngine's instance
    EditEngine aEditEngine( mpItemPool );

    // Get EditDoc for current EditEngine's instance
    EditDoc &rDoc = aEditEngine.GetEditDoc();

    // Initially no text should be there
    CPPUNIT_ASSERT_EQUAL( sal_uLong(0), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( OUString(), rDoc.GetParaAsString(sal_Int32(0)) );

    // Insert initial text
    OUString aFirstPara = "This is first paragraph";
    // Selection Ref       ........8..............
    OUString aSecondPara = "This is second paragraph";
    // Selection Ref        .............14.........
    OUString aThirdPara = "This is third paragraph";
    OUString aText = aFirstPara + "\n" + aSecondPara + "\n" + aThirdPara;
    sal_Int32 aTextLen = aFirstPara.getLength() + aSecondPara.getLength() + aThirdPara.getLength();
    aEditEngine.SetText( aText );
    OUString aCopyText = "first paragraphThis is second";
    sal_Int32 aCopyTextLen = aCopyText.getLength();

    // Assert changes
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( aFirstPara, rDoc.GetParaAsString(sal_Int32(0)) );
    CPPUNIT_ASSERT_EQUAL( aSecondPara, rDoc.GetParaAsString(sal_Int32(1)) );
    CPPUNIT_ASSERT_EQUAL( aThirdPara, rDoc.GetParaAsString(sal_Int32(2)) );

    // Copy initial text using legacy format
    uno::Reference< datatransfer::XTransferable > xData = aEditEngine.CreateTransferable( ESelection(0,8,1,14) );

    // Paste text at the start
    aEditEngine.InsertText( xData, OUString(), rDoc.GetStartPaM(), true );

    // Assert changes
    OUString aFirstParaAfterCopyPaste = "first paragraph";
    OUString aSecondParaAfterCopyPaste = "This is second" + aFirstPara;
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen + aCopyTextLen), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( aFirstParaAfterCopyPaste, rDoc.GetParaAsString(sal_Int32(0)) );
    CPPUNIT_ASSERT_EQUAL( aSecondParaAfterCopyPaste, rDoc.GetParaAsString(sal_Int32(1)) );
    CPPUNIT_ASSERT_EQUAL( aSecondPara, rDoc.GetParaAsString(sal_Int32(2)) );
    CPPUNIT_ASSERT_EQUAL( aThirdPara, rDoc.GetParaAsString(sal_Int32(3)) );
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
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), nParaCount);

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
        CPPUNIT_ASSERT_EQUAL(size_t(5), aAttrs.size());

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

void Test::testLargeParaCopyPaste()
{
    // Create EditEngine's instance
    EditEngine aEditEngine( mpItemPool );

    // Get EditDoc for current EditEngine's instance
    EditDoc &rDoc = aEditEngine.GetEditDoc();

    // Initially no text should be there
    CPPUNIT_ASSERT_EQUAL( sal_uLong(0), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( OUString(), rDoc.GetParaAsString(sal_Int32(0)) );

    // Insert initial text
    OUString aFirstPara = "This is first paragraph";
    OUString aSecondPara = "This is second paragraph";
    OUString aThirdPara = "This is third paragraph";
    OUString aFourthPara = "This is fourth paragraph";
    OUString aFifthPara = "This is fifth paragraph";
    OUString aSixthPara = "This is sixth paragraph";
    //Positions Ref:       ........*8.............
    OUString aSeventhPara = "This is seventh paragraph";
    OUString aEighthPara = "This is eighth paragraph";
    //Positions Ref:        .............*13
    OUString aNinthPara = "This is ninth paragraph";
    OUString aTenthPara = "This is tenth paragraph";
    OUString aText = aFirstPara + "\n" + aSecondPara + "\n" + aThirdPara + "\n" +
        aFourthPara + "\n" + aFifthPara + "\n" + aSixthPara + "\n" + aSeventhPara + "\n" +
        aEighthPara + "\n" + aNinthPara + "\n" + aTenthPara;
    sal_Int32 aTextLen = aFirstPara.getLength() + aSecondPara.getLength() + aThirdPara.getLength() +
        aFourthPara.getLength() + aFifthPara.getLength() + aSixthPara.getLength() +
        aSeventhPara.getLength() + aEighthPara.getLength() + aNinthPara.getLength() + aTenthPara.getLength();
    aEditEngine.SetText( aText );
    OUString aCopyText = "sixth paragraphThis is seventh paragraphThis is eighth";
    sal_Int32 aCopyTextLen = aCopyText.getLength();

    // Assert changes
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( aFirstPara, rDoc.GetParaAsString(sal_Int32(0)) );
    CPPUNIT_ASSERT_EQUAL( aSecondPara, rDoc.GetParaAsString(sal_Int32(1)) );
    CPPUNIT_ASSERT_EQUAL( aThirdPara, rDoc.GetParaAsString(sal_Int32(2)) );
    CPPUNIT_ASSERT_EQUAL( aFourthPara, rDoc.GetParaAsString(sal_Int32(3)) );
    CPPUNIT_ASSERT_EQUAL( aFifthPara, rDoc.GetParaAsString(sal_Int32(4)) );
    CPPUNIT_ASSERT_EQUAL( aSixthPara, rDoc.GetParaAsString(sal_Int32(5)) );
    CPPUNIT_ASSERT_EQUAL( aSeventhPara, rDoc.GetParaAsString(sal_Int32(6)) );
    CPPUNIT_ASSERT_EQUAL( aEighthPara, rDoc.GetParaAsString(sal_Int32(7)) );
    CPPUNIT_ASSERT_EQUAL( aNinthPara, rDoc.GetParaAsString(sal_Int32(8)) );
    CPPUNIT_ASSERT_EQUAL( aTenthPara, rDoc.GetParaAsString(sal_Int32(9)) );

    // Copy initial text using legacy format
    uno::Reference< datatransfer::XTransferable > xData = aEditEngine.CreateTransferable( ESelection(5,8,7,14) );

    // Paste text at the end of 4th Para
    ContentNode* pLastNode = const_cast<ContentNode*>(rDoc.GetObject(3));
    aEditEngine.InsertText( xData, OUString(), EditPaM( pLastNode, pLastNode->Len() ), true );

    // Assert changes
    OUString aFourthParaAfterCopyPaste = aFourthPara + "sixth paragraph";
    OUString aSixthParaAfterCopyPaste = "This is eighth";
    CPPUNIT_ASSERT_EQUAL( sal_uLong(aTextLen + aCopyTextLen), rDoc.GetTextLen() );
    CPPUNIT_ASSERT_EQUAL( aFirstPara, rDoc.GetParaAsString(sal_Int32(0)) );
    CPPUNIT_ASSERT_EQUAL( aSecondPara, rDoc.GetParaAsString(sal_Int32(1)) );
    CPPUNIT_ASSERT_EQUAL( aThirdPara, rDoc.GetParaAsString(sal_Int32(2)) );
    CPPUNIT_ASSERT_EQUAL( aFourthParaAfterCopyPaste, rDoc.GetParaAsString(sal_Int32(3)) );
    CPPUNIT_ASSERT_EQUAL( aSeventhPara, rDoc.GetParaAsString(sal_Int32(4)) );
    CPPUNIT_ASSERT_EQUAL( aSixthParaAfterCopyPaste, rDoc.GetParaAsString(sal_Int32(5)) );
    CPPUNIT_ASSERT_EQUAL( aFifthPara, rDoc.GetParaAsString(sal_Int32(6)) );
    CPPUNIT_ASSERT_EQUAL( aSixthPara, rDoc.GetParaAsString(sal_Int32(7)) );
    CPPUNIT_ASSERT_EQUAL( aSeventhPara, rDoc.GetParaAsString(sal_Int32(8)) );
    CPPUNIT_ASSERT_EQUAL( aEighthPara, rDoc.GetParaAsString(sal_Int32(9)) );
    CPPUNIT_ASSERT_EQUAL( aNinthPara, rDoc.GetParaAsString(sal_Int32(10)) );
    CPPUNIT_ASSERT_EQUAL( aTenthPara, rDoc.GetParaAsString(sal_Int32(11)) );
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
