/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <sal/config.h>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/stritem.hxx>

#include "helper/qahelper.hxx"
#include "helper/shared_test_impl.hxx"

#include "docsh.hxx"
#include "postit.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "document.hxx"
#include "cellform.hxx"
#include "formulacell.hxx"
#include "tokenarray.hxx"
#include "editutil.hxx"
#include "scopetools.hxx"
#include "cellvalue.hxx"
#include <postit.hxx>
#include <tokenstringcontext.hxx>

#include "svx/svdoole2.hxx"
#include "tabprotection.hxx"
#include "editeng/wghtitem.hxx"
#include "editeng/postitem.hxx"
#include "editeng/editdata.hxx"
#include "editeng/eeitem.hxx"
#include "editeng/editobj.hxx"
#include "editeng/section.hxx"
#include <editeng/crossedoutitem.hxx>
#include <editeng/borderline.hxx>
#include <formula/grammar.hxx>

#include <com/sun/star/table/BorderLineStyle.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScExportTest : public ScBootstrapFixture
{
public:
    ScExportTest();

    virtual void setUp();
    virtual void tearDown();

    ScDocShellRef saveAndReloadPassword( ScDocShell*, const OUString&, const OUString&, const OUString&, sal_uLong );

    void test();
    void testPasswordExport();
    void testConditionalFormatExportODS();
    void testConditionalFormatExportXLSX();
    void testColorScaleExportODS();
    void testColorScaleExportXLSX();
    void testDataBarExportODS();
    void testDataBarExportXLSX();
    void testMiscRowHeightExport();
    void testNamedRangeBugfdo62729();
    void testRichTextExportODS();
    void testFormulaRefSheetNameODS();

    void testCellValuesExportODS();
    void testCellNoteExportODS();
    void testCellNoteExportXLS();
    void testFormatExportODS();

    void testInlineArrayXLS();
    void testEmbeddedChartXLS();
    void testFormulaReferenceXLS();
    void testSheetProtectionXLSX();

    void testCellBordersXLS();
    void testCellBordersXLSX();

    void testSharedFormulaExportXLS();

    CPPUNIT_TEST_SUITE(ScExportTest);
    CPPUNIT_TEST(test);
#if !defined(MACOSX) && !defined(DRAGONFLY)
    CPPUNIT_TEST(testPasswordExport);
#endif
    CPPUNIT_TEST(testConditionalFormatExportODS);
    CPPUNIT_TEST(testConditionalFormatExportXLSX);
    CPPUNIT_TEST(testColorScaleExportODS);
    CPPUNIT_TEST(testColorScaleExportXLSX);
    CPPUNIT_TEST(testMiscRowHeightExport);
    CPPUNIT_TEST(testNamedRangeBugfdo62729);
    CPPUNIT_TEST(testRichTextExportODS);
    CPPUNIT_TEST(testFormulaRefSheetNameODS);
    CPPUNIT_TEST(testCellValuesExportODS);
    CPPUNIT_TEST(testCellNoteExportODS);
    CPPUNIT_TEST(testCellNoteExportXLS);
    CPPUNIT_TEST(testFormatExportODS);
    CPPUNIT_TEST(testInlineArrayXLS);
    CPPUNIT_TEST(testEmbeddedChartXLS);
    CPPUNIT_TEST(testFormulaReferenceXLS);
    CPPUNIT_TEST(testSheetProtectionXLSX);
    CPPUNIT_TEST(testCellBordersXLS);
    CPPUNIT_TEST(testCellBordersXLSX);
    CPPUNIT_TEST(testSharedFormulaExportXLS);

    CPPUNIT_TEST_SUITE_END();

private:
    void testExcelCellBorders( sal_uLong nFormatType );

    uno::Reference<uno::XInterface> m_xCalcComponent;

};

ScDocShellRef ScExportTest::saveAndReloadPassword(ScDocShell* pShell, const OUString &rFilter,
    const OUString &rUserData, const OUString& rTypeName, sal_uLong nFormatType)
{
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    SfxMedium aStoreMedium( aTempFile.GetURL(), STREAM_STD_WRITE );
    sal_uInt32 nExportFormat = 0;
    if (nFormatType == ODS_FORMAT_TYPE)
        nExportFormat = SFX_FILTER_EXPORT | SFX_FILTER_USESOPTIONS;
    SfxFilter* pExportFilter = new SfxFilter(
        rFilter,
        OUString(), nFormatType, nExportFormat, rTypeName, 0, OUString(),
        rUserData, OUString("private:factory/scalc*") );
    pExportFilter->SetVersion(SOFFICE_FILEFORMAT_CURRENT);
    aStoreMedium.SetFilter(pExportFilter);
    SfxItemSet* pExportSet = aStoreMedium.GetItemSet();
    uno::Sequence< beans::NamedValue > aEncryptionData = comphelper::OStorageHelper::CreatePackageEncryptionData( OUString("test") );
    uno::Any xEncryptionData;
    xEncryptionData <<= aEncryptionData;
    pExportSet->Put(SfxUnoAnyItem(SID_ENCRYPTIONDATA, xEncryptionData));

    uno::Reference< embed::XStorage > xMedStorage = aStoreMedium.GetStorage();
    ::comphelper::OStorageHelper::SetCommonStorageEncryptionData( xMedStorage, aEncryptionData );

    pShell->DoSaveAs( aStoreMedium );
    pShell->DoClose();

    

    sal_uInt32 nFormat = 0;
    if (nFormatType == ODS_FORMAT_TYPE)
        nFormat = SFX_FILTER_IMPORT | SFX_FILTER_USESOPTIONS;

    OUString aPass("test");
    return load(aTempFile.GetURL(), rFilter, rUserData, rTypeName, nFormatType, nFormat, SOFFICE_FILEFORMAT_CURRENT, &aPass);
}

void ScExportTest::test()
{
    ScDocShell* pShell = new ScDocShell(
        SFXMODEL_STANDARD |
        SFXMODEL_DISABLE_EMBEDDED_SCRIPTS |
        SFXMODEL_DISABLE_DOCUMENT_RECOVERY);
    pShell->DoInitNew();

    ScDocument* pDoc = pShell->GetDocument();

    pDoc->SetValue(0,0,0, 1.0);
    CPPUNIT_ASSERT(pDoc);

    ScDocShellRef xDocSh = saveAndReload( pShell, ODS );

    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument* pLoadedDoc = xDocSh->GetDocument();
    double aVal = pLoadedDoc->GetValue(0,0,0);
    ASSERT_DOUBLES_EQUAL(aVal, 1.0);
}

void ScExportTest::testPasswordExport()
{
    ScDocShell* pShell = new ScDocShell(
        SFXMODEL_STANDARD |
        SFXMODEL_DISABLE_EMBEDDED_SCRIPTS |
        SFXMODEL_DISABLE_DOCUMENT_RECOVERY);
    pShell->DoInitNew();

    ScDocument* pDoc = pShell->GetDocument();

    pDoc->SetValue(0,0,0, 1.0);
    CPPUNIT_ASSERT(pDoc);

    sal_Int32 nFormat = ODS;
    OUString aFilterName(getFileFormats()[nFormat].pFilterName, strlen(getFileFormats()[nFormat].pFilterName), RTL_TEXTENCODING_UTF8) ;
    OUString aFilterType(getFileFormats()[nFormat].pTypeName, strlen(getFileFormats()[nFormat].pTypeName), RTL_TEXTENCODING_UTF8);
    ScDocShellRef xDocSh = saveAndReloadPassword(pShell, aFilterName, OUString(), aFilterType, getFileFormats()[nFormat].nFormatType);

    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument* pLoadedDoc = xDocSh->GetDocument();
    double aVal = pLoadedDoc->GetValue(0,0,0);
    ASSERT_DOUBLES_EQUAL(aVal, 1.0);

    xDocSh->DoClose();
}

void ScExportTest::testConditionalFormatExportODS()
{
    ScDocShellRef xShell = loadDoc("new_cond_format_test.", ODS);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), ODS);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    OUString aCSVFile("new_cond_format_test.");
    OUString aCSVPath;
    createCSVPath( aCSVFile, aCSVPath );
    testCondFile(aCSVPath, pDoc, 0);

    xDocSh->DoClose();
}

void ScExportTest::testConditionalFormatExportXLSX()
{
    ScDocShellRef xShell = loadDoc("new_cond_format_test.", XLSX);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), XLSX);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    OUString aCSVFile("new_cond_format_test.");
    OUString aCSVPath;
    createCSVPath( aCSVFile, aCSVPath );
    testCondFile(aCSVPath, pDoc, 0);

    xDocSh->DoClose();
}

void ScExportTest::testColorScaleExportODS()
{
    ScDocShellRef xShell = loadDoc("colorscale.", ODS);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(xShell, ODS);
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    testColorScale2Entry_Impl(pDoc);
    testColorScale3Entry_Impl(pDoc);

    xDocSh->DoClose();
}

void ScExportTest::testColorScaleExportXLSX()
{
    ScDocShellRef xShell = loadDoc("colorscale.", XLSX);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(xShell, XLSX);
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    testColorScale2Entry_Impl(pDoc);
    testColorScale3Entry_Impl(pDoc);

    xDocSh->DoClose();
}

void ScExportTest::testDataBarExportODS()
{
    ScDocShellRef xShell = loadDoc("databar.", ODS);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(xShell, ODS);
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    testDataBar_Impl(pDoc);

    xDocSh->DoClose();
}

void ScExportTest::testFormatExportODS()
{
    ScDocShellRef xShell = loadDoc("formats.", ODS);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(xShell, ODS);
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();

    testFormats(this, pDoc, ODS);

    xDocSh->DoClose();
}

void ScExportTest::testDataBarExportXLSX()
{
    ScDocShellRef xShell = loadDoc("databar.", XLSX);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(xShell, XLSX);
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    testDataBar_Impl(pDoc);

    xDocSh->DoClose();
}

void ScExportTest::testMiscRowHeightExport()
{
    TestParam::RowData DfltRowData[] =
    {
        { 0, 4, 0, 529, 0, false },
        { 5, 10, 0, 1058, 0, false },
        { 17, 20, 0, 1767, 0, false },
        
        
        { 1048573, 1048575, 0, 529, 0, false },
    };

    TestParam::RowData EmptyRepeatRowData[] =
    {
        
        
        
        
        { 0, 4, 0, 529, 0, false },
        { 5, 10, 0, 1058, 0, false },
        { 17, 20, 0, 1767, 0, false },
    };

    TestParam aTestValues[] =
    {
        
        
        
        
        { "miscrowheights.", XLSX, XLSX, SAL_N_ELEMENTS(DfltRowData), DfltRowData },
        
        { "miscrowheights.", XLSX, XLS, SAL_N_ELEMENTS(DfltRowData), DfltRowData },
        
        { "miscemptyrepeatedrowheights.", ODS, XLSX, SAL_N_ELEMENTS(EmptyRepeatRowData), EmptyRepeatRowData },
        
        { "miscemptyrepeatedrowheights.", ODS, XLS, SAL_N_ELEMENTS(EmptyRepeatRowData), EmptyRepeatRowData },
    };
    miscRowHeightsTest( aTestValues, SAL_N_ELEMENTS(aTestValues) );
}

namespace {

void setAttribute( ScFieldEditEngine& rEE, sal_Int32 nPara, sal_Int32 nStart, sal_Int32 nEnd, sal_uInt16 nType )
{
    ESelection aSel;
    aSel.nStartPara = aSel.nEndPara = nPara;
    aSel.nStartPos = nStart;
    aSel.nEndPos = nEnd;

    SfxItemSet aItemSet = rEE.GetEmptyItemSet();
    switch (nType)
    {
        case EE_CHAR_WEIGHT:
        {
            SvxWeightItem aWeight(WEIGHT_BOLD, nType);
            aItemSet.Put(aWeight);
            rEE.QuickSetAttribs(aItemSet, aSel);
        }
        break;
        case EE_CHAR_ITALIC:
        {
            SvxPostureItem aItalic(ITALIC_NORMAL, nType);
            aItemSet.Put(aItalic);
            rEE.QuickSetAttribs(aItemSet, aSel);
        }
        break;
        case EE_CHAR_STRIKEOUT:
        {
            SvxCrossedOutItem aCrossOut(STRIKEOUT_SINGLE, nType);
            aItemSet.Put(aCrossOut);
            rEE.QuickSetAttribs(aItemSet, aSel);
        }
        break;
        default:
            ;
    }
}

}

void ScExportTest::testNamedRangeBugfdo62729()
{
    ScDocShellRef xShell = loadDoc("fdo62729.", ODS);
    CPPUNIT_ASSERT(xShell.Is());
    ScDocument* pDoc = xShell->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    ScRangeName* pNames = pDoc->GetRangeName();
    
    CPPUNIT_ASSERT(pNames->size() == 1 );
    pDoc->DeleteTab(0);
    
    CPPUNIT_ASSERT(pNames->size() == 1 );
    ScDocShellRef xDocSh = saveAndReload(xShell, ODS);
    xShell->DoClose();

    CPPUNIT_ASSERT(xDocSh.Is());
    pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    pNames = pDoc->GetRangeName();
    
    CPPUNIT_ASSERT(pNames->size() == 1 );

    xDocSh->DoClose();
}

void ScExportTest::testRichTextExportODS()
{
    struct
    {
        static bool isBold(const editeng::Section& rAttr)
        {
            if (rAttr.maAttributes.empty())
                return false;

            std::vector<const SfxPoolItem*>::const_iterator it = rAttr.maAttributes.begin(), itEnd = rAttr.maAttributes.end();
            for (; it != itEnd; ++it)
            {
                const SfxPoolItem* p = *it;
                if (p->Which() != EE_CHAR_WEIGHT)
                    continue;

                return static_cast<const SvxWeightItem*>(p)->GetWeight() == WEIGHT_BOLD;
            }
            return false;
        }

        static bool isItalic(const editeng::Section& rAttr)
        {
            if (rAttr.maAttributes.empty())
                return false;

            std::vector<const SfxPoolItem*>::const_iterator it = rAttr.maAttributes.begin(), itEnd = rAttr.maAttributes.end();
            for (; it != itEnd; ++it)
            {
                const SfxPoolItem* p = *it;
                if (p->Which() != EE_CHAR_ITALIC)
                    continue;

                return static_cast<const SvxPostureItem*>(p)->GetPosture() == ITALIC_NORMAL;
            }
            return false;
        }

        static bool isStrikeOut(const editeng::Section& rAttr)
        {
            if (rAttr.maAttributes.empty())
                return false;

            std::vector<const SfxPoolItem*>::const_iterator it = rAttr.maAttributes.begin(), itEnd = rAttr.maAttributes.end();
            for (; it != itEnd; ++it)
            {
                const SfxPoolItem* p = *it;
                if (p->Which() != EE_CHAR_STRIKEOUT)
                    continue;

                return static_cast<const SvxCrossedOutItem*>(p)->GetStrikeout() == STRIKEOUT_SINGLE;
            }
            return false;
        }

        bool checkB2(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 1)
                return false;

            if (pText->GetText(0) != "Bold and Italic")
                return false;

            std::vector<editeng::Section> aSecAttrs;
            pText->GetAllSections(aSecAttrs);
            if (aSecAttrs.size() != 3)
                return false;

            
            const editeng::Section* pAttr = &aSecAttrs[0];
            if (pAttr->mnParagraph != 0 ||pAttr->mnStart != 0 || pAttr->mnEnd != 4)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isBold(*pAttr))
                return false;

            
            pAttr = &aSecAttrs[1];
            if (pAttr->mnParagraph != 0 ||pAttr->mnStart != 4 || pAttr->mnEnd != 9)
                return false;

            if (!pAttr->maAttributes.empty())
                return false;

            
            pAttr = &aSecAttrs[2];
            if (pAttr->mnParagraph != 0 ||pAttr->mnStart != 9 || pAttr->mnEnd != 15)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isItalic(*pAttr))
                return false;

            return true;
        }

        bool checkB4(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 3)
                return false;

            if (pText->GetText(0) != "One")
                return false;

            if (pText->GetText(1) != "Two")
                return false;

            if (pText->GetText(2) != "Three")
                return false;

            return true;
        }

        bool checkB5(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 6)
                return false;

            if (pText->GetText(0) != "")
                return false;

            if (pText->GetText(1) != "Two")
                return false;

            if (pText->GetText(2) != "Three")
                return false;

            if (pText->GetText(3) != "")
                return false;

            if (pText->GetText(4) != "Five")
                return false;

            if (pText->GetText(5) != "")
                return false;

            return true;
        }

        bool checkB6(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 1)
                return false;

            if (pText->GetText(0) != "Strike Me")
                return false;

            std::vector<editeng::Section> aSecAttrs;
            pText->GetAllSections(aSecAttrs);
            if (aSecAttrs.size() != 2)
                return false;

            
            const editeng::Section* pAttr = &aSecAttrs[0];
            if (pAttr->mnParagraph != 0 ||pAttr->mnStart != 0 || pAttr->mnEnd != 6)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isStrikeOut(*pAttr))
                return false;

            
            pAttr = &aSecAttrs[1];
            if (pAttr->mnParagraph != 0 ||pAttr->mnStart != 6 || pAttr->mnEnd != 9)
                return false;

            return true;
        }

    } aCheckFunc;

    
    
    ScDocShellRef xOrigDocSh = loadDoc("empty.", ODS);
    ScDocument* pDoc = xOrigDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    CPPUNIT_ASSERT_MESSAGE("This document should at least have one sheet.", pDoc->GetTableCount() > 0);

    
    ScFieldEditEngine& rEE = pDoc->GetEditEngine();
    rEE.SetText("Bold and Italic");
    
    setAttribute(rEE, 0, 0, 4, EE_CHAR_WEIGHT);
    
    setAttribute(rEE, 0, 9, 15, EE_CHAR_ITALIC);
    ESelection aSel;
    aSel.nStartPara = aSel.nEndPara = 0;

    
    pDoc->SetEditText(ScAddress(1,1,0), rEE.CreateTextObject());
    const EditTextObject* pEditText = pDoc->GetEditText(ScAddress(1,1,0));
    CPPUNIT_ASSERT_MESSAGE("Incorret B2 value.", aCheckFunc.checkB2(pEditText));

    
    ScDocShellRef xNewDocSh = saveAndReload(xOrigDocSh, ODS);
    xOrigDocSh->DoClose();
    CPPUNIT_ASSERT(xNewDocSh.Is());
    pDoc = xNewDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    CPPUNIT_ASSERT_MESSAGE("Reloaded document should at least have one sheet.", pDoc->GetTableCount() > 0);

    
    CPPUNIT_ASSERT_MESSAGE("Incorret B2 value.", aCheckFunc.checkB2(pEditText));

    
    rEE.Clear();
    rEE.SetText("One\nTwo\nThree");
    pDoc->SetEditText(ScAddress(3,1,0), rEE.CreateTextObject());
    pEditText = pDoc->GetEditText(ScAddress(3,1,0));
    CPPUNIT_ASSERT_MESSAGE("Incorret B4 value.", aCheckFunc.checkB4(pEditText));

    
    ScDocShellRef xNewDocSh2 = saveAndReload(xNewDocSh, ODS);
    pDoc = xNewDocSh2->GetDocument();
    xNewDocSh->DoClose();

    pEditText = pDoc->GetEditText(ScAddress(1,1,0));
    CPPUNIT_ASSERT_MESSAGE("B2 should be an edit text.", pEditText);
    pEditText = pDoc->GetEditText(ScAddress(3,1,0));
    CPPUNIT_ASSERT_MESSAGE("Incorret B4 value.", aCheckFunc.checkB4(pEditText));

    
    rEE.Clear();
    rEE.SetText("\nTwo\nThree\n\nFive\n");
    pDoc->SetEditText(ScAddress(4,1,0), rEE.CreateTextObject());
    pEditText = pDoc->GetEditText(ScAddress(4,1,0));
    CPPUNIT_ASSERT_MESSAGE("Incorret B5 value.", aCheckFunc.checkB5(pEditText));

    
    rEE.Clear();
    rEE.SetText("Strike Me");
    
    setAttribute(rEE, 0, 0, 6, EE_CHAR_STRIKEOUT);
    pDoc->SetEditText(ScAddress(5,1,0), rEE.CreateTextObject());
    pEditText = pDoc->GetEditText(ScAddress(5,1,0));
    CPPUNIT_ASSERT_MESSAGE("Incorret B6 value.", aCheckFunc.checkB6(pEditText));

    
    ScDocShellRef xNewDocSh3 = saveAndReload(xNewDocSh2, ODS);
    pDoc = xNewDocSh3->GetDocument();
    xNewDocSh2->DoClose();

    pEditText = pDoc->GetEditText(ScAddress(1,1,0));
    CPPUNIT_ASSERT_MESSAGE("Incorret B2 value.", aCheckFunc.checkB2(pEditText));
    pEditText = pDoc->GetEditText(ScAddress(3,1,0));
    CPPUNIT_ASSERT_MESSAGE("Incorret B4 value.", aCheckFunc.checkB4(pEditText));
    pEditText = pDoc->GetEditText(ScAddress(4,1,0));
    CPPUNIT_ASSERT_MESSAGE("Incorret B5 value.", aCheckFunc.checkB5(pEditText));
    pEditText = pDoc->GetEditText(ScAddress(5,1,0));
    CPPUNIT_ASSERT_MESSAGE("Incorret B6 value.", aCheckFunc.checkB6(pEditText));

    xNewDocSh3->DoClose();
}

void ScExportTest::testFormulaRefSheetNameODS()
{
    ScDocShellRef xDocSh = loadDoc("formula-quote-in-sheet-name.", ODS, true);
    ScDocument* pDoc = xDocSh->GetDocument();

    sc::AutoCalcSwitch aACSwitch(*pDoc, true); 
    pDoc->SetString(ScAddress(1,1,0), "='90''s Data'.B2");
    CPPUNIT_ASSERT_EQUAL(1.1, pDoc->GetValue(ScAddress(1,1,0)));
    if (!checkFormula(*pDoc, ScAddress(1,1,0), "'90''s Data'.B2"))
        CPPUNIT_FAIL("Wrong formula");

    
    ScDocShellRef xNewDocSh = saveAndReload(xDocSh, ODS);
    xDocSh->DoClose();

    pDoc = xNewDocSh->GetDocument();
    pDoc->CalcAll();
    CPPUNIT_ASSERT_EQUAL(1.1, pDoc->GetValue(ScAddress(1,1,0)));
    if (!checkFormula(*pDoc, ScAddress(1,1,0), "'90''s Data'.B2"))
        CPPUNIT_FAIL("Wrong formula");

    xNewDocSh->DoClose();
}

void ScExportTest::testCellValuesExportODS()
{
    
    ScDocShellRef xOrigDocSh = loadDoc("empty.", ODS);
    ScDocument* pDoc = xOrigDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    CPPUNIT_ASSERT_MESSAGE("This document should at least have one sheet.", pDoc->GetTableCount() > 0);

    
    pDoc->SetValue(ScAddress(0,0,0), 2.0); 

    
    pDoc->SetValue(ScAddress(2,0,0), 3.0); 
    pDoc->SetValue(ScAddress(3,0,0), 3); 
    pDoc->SetString(ScAddress(4,0,0), "=10*C1/4"); 
    pDoc->SetValue(ScAddress(5,0,0), 3.0); 
    pDoc->SetString(ScAddress(7,0,0), "=SUM(C1:F1)"); 

    
    pDoc->SetString(ScAddress(0,2,0), "a simple line"); 

    
    pDoc->SetString(ScAddress(0,4,0), "'12"); 
    
    pDoc->SetValue(ScAddress(0,5,0), 12.0); 
    
    pDoc->SetString(ScAddress(0,6,0), "a string"); 
    
    pDoc->SetString(ScAddress(0,7,0), "=$A$6"); 

    
    ScDocShellRef xNewDocSh = saveAndReload(xOrigDocSh, ODS);
    xOrigDocSh->DoClose();
    CPPUNIT_ASSERT(xNewDocSh.Is());
    pDoc = xNewDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    CPPUNIT_ASSERT_MESSAGE("Reloaded document should at least have one sheet.", pDoc->GetTableCount() > 0);

    
    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(0,0,0));
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(2,0,0));
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(3,0,0));
    CPPUNIT_ASSERT_EQUAL(7.5, pDoc->GetValue(4,0,0));
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(5,0,0));

    
    if (!checkFormula(*pDoc, ScAddress(4,0,0), "10*C1/4"))
        CPPUNIT_FAIL("Wrong formula =10*C1/4");
    if (!checkFormula(*pDoc, ScAddress(7,0,0), "SUM(C1:F1)"))
        CPPUNIT_FAIL("Wrong formula =SUM(C1:F1)");
    CPPUNIT_ASSERT_EQUAL(16.5, pDoc->GetValue(7,0,0));

    
    ScRefCellValue aCell;
    aCell.assign(*pDoc, ScAddress(0,2,0));
    CPPUNIT_ASSERT_EQUAL( CELLTYPE_STRING, aCell.meType );

    
    aCell.assign(*pDoc, ScAddress(0,3,0));
    CPPUNIT_ASSERT_EQUAL( CELLTYPE_NONE, aCell.meType);

    
    aCell.assign(*pDoc, ScAddress(0,4,0));
    CPPUNIT_ASSERT_EQUAL( CELLTYPE_STRING, aCell.meType);

    
    CPPUNIT_ASSERT_EQUAL( 12.0, pDoc->GetValue(0,5,0) );
    CPPUNIT_ASSERT_EQUAL( OUString("a string"), pDoc->GetString(0,6,0) );
    if (!checkFormula(*pDoc, ScAddress(0,7,0), "$A$6"))
        CPPUNIT_FAIL("Wrong formula =$A$6");
    CPPUNIT_ASSERT_EQUAL( pDoc->GetValue(0,5,0), pDoc->GetValue(0,7,0) );

    xNewDocSh->DoClose();
}

void ScExportTest::testCellNoteExportODS()
{
    ScDocShellRef xOrigDocSh = loadDoc("single-note.", ODS);
    ScDocument* pDoc = xOrigDocSh->GetDocument();

    ScAddress aPos(0,0,0); 
    CPPUNIT_ASSERT_MESSAGE("There should be a note at A1.", pDoc->HasNote(aPos));

    aPos.IncRow(); 
    ScPostIt* pNote = pDoc->GetOrCreateNote(aPos);
    pNote->SetText(aPos, "Note One");
    pNote->SetAuthor("Author One");
    CPPUNIT_ASSERT_MESSAGE("There should be a note at A2.", pDoc->HasNote(aPos));

    
    ScDocShellRef xNewDocSh = saveAndReload(xOrigDocSh, ODS);
    xOrigDocSh->DoClose();
    CPPUNIT_ASSERT(xNewDocSh.Is());
    pDoc = xNewDocSh->GetDocument();

    aPos.SetRow(0); 
    CPPUNIT_ASSERT_MESSAGE("There should be a note at A1.", pDoc->HasNote(aPos));
    aPos.IncRow(); 
    CPPUNIT_ASSERT_MESSAGE("There should be a note at A2.", pDoc->HasNote(aPos));

    xNewDocSh->DoClose();
}

void ScExportTest::testCellNoteExportXLS()
{
    
    ScDocShellRef xOrigDocSh = loadDoc("notes-on-3-sheets.", ODS);
    ScDocument* pDoc = xOrigDocSh->GetDocument();
    CPPUNIT_ASSERT_MESSAGE("This document should have 3 sheets.", pDoc->GetTableCount() == 3);

    
    CPPUNIT_ASSERT( pDoc->HasNote(ScAddress(0,0,0)));
    CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0,1,0)));
    CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0,2,0)));

    CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0,0,1)));
    CPPUNIT_ASSERT( pDoc->HasNote(ScAddress(0,1,1)));
    CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0,2,1)));

    CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0,0,2)));
    CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0,1,2)));
    CPPUNIT_ASSERT( pDoc->HasNote(ScAddress(0,2,2)));

    
    ScDocShellRef xNewDocSh = saveAndReload(xOrigDocSh, XLS);
    xOrigDocSh->DoClose();
    CPPUNIT_ASSERT(xNewDocSh.Is());
    pDoc = xNewDocSh->GetDocument();
    CPPUNIT_ASSERT_MESSAGE("This document should have 3 sheets.", pDoc->GetTableCount() == 3);

    
    CPPUNIT_ASSERT( pDoc->HasNote(ScAddress(0,0,0)));
    CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0,1,0)));
    CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0,2,0)));

    CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0,0,1)));
    CPPUNIT_ASSERT( pDoc->HasNote(ScAddress(0,1,1)));
    CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0,2,1)));

    CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0,0,2)));
    CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0,1,2)));
    CPPUNIT_ASSERT( pDoc->HasNote(ScAddress(0,2,2)));

    xNewDocSh->DoClose();
}

namespace {

void checkMatrixRange(ScDocument& rDoc, const ScRange& rRange)
{
    ScRange aMatRange;
    ScAddress aMatOrigin;
    for (SCCOL nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
    {
        for (SCROW nRow = rRange.aStart.Row(); nRow <= rRange.aEnd.Row(); ++nRow)
        {
            ScAddress aPos(nCol, nRow, rRange.aStart.Tab());
            bool bIsMatrix = rDoc.GetMatrixFormulaRange(aPos, aMatRange);
            CPPUNIT_ASSERT_MESSAGE("Matrix expected, but not found.", bIsMatrix);
            CPPUNIT_ASSERT_MESSAGE("Wrong matrix range.", rRange == aMatRange);
            const ScFormulaCell* pCell = rDoc.GetFormulaCell(aPos);
            CPPUNIT_ASSERT_MESSAGE("This must be a formula cell.", pCell);

            bIsMatrix = pCell->GetMatrixOrigin(aMatOrigin);
            CPPUNIT_ASSERT_MESSAGE("Not a part of matrix formula.", bIsMatrix);
            CPPUNIT_ASSERT_MESSAGE("Wrong matrix origin.", aMatOrigin == aMatRange.aStart);
        }
    }
}

}

void ScExportTest::testInlineArrayXLS()
{
    ScDocShellRef xShell = loadDoc("inline-array.", XLS);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(xShell, XLS);
    xShell->DoClose();
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    
    checkMatrixRange(*pDoc, ScRange(1,1,0,2,2,0));

    
    checkMatrixRange(*pDoc, ScRange(1,4,0,3,5,0));

    
    checkMatrixRange(*pDoc, ScRange(1,7,0,2,9,0));

    xDocSh->DoClose();
}

void ScExportTest::testEmbeddedChartXLS()
{
    ScDocShellRef xShell = loadDoc("embedded-chart.", XLS);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(xShell, XLS);
    xShell->DoClose();
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    
    OUString aName;
    pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Chart1"), aName);

    const SdrOle2Obj* pOleObj = getSingleChartObject(*pDoc, 1);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve a chart object from the 2nd sheet.", pOleObj);

    ScRangeList aRanges = getChartRanges(*pDoc, *pOleObj);
    CPPUNIT_ASSERT_MESSAGE("Label range (B3:B5) not found.", aRanges.In(ScRange(1,2,1,1,4,1)));
    CPPUNIT_ASSERT_MESSAGE("Data label (C2) not found.", aRanges.In(ScAddress(2,1,1)));
    CPPUNIT_ASSERT_MESSAGE("Data range (C3:C5) not found.", aRanges.In(ScRange(2,2,1,2,4,1)));

    xDocSh->DoClose();
}

void ScExportTest::testFormulaReferenceXLS()
{
    ScDocShellRef xShell = loadDoc("formula-reference.", XLS);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(xShell, XLS);
    xShell->DoClose();
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    if (!checkFormula(*pDoc, ScAddress(3,1,0), "$A$2+$B$2+$C$2"))
        CPPUNIT_FAIL("Wrong formula in D2");

    if (!checkFormula(*pDoc, ScAddress(3,2,0), "A3+B3+C3"))
        CPPUNIT_FAIL("Wrong formula in D3");

    if (!checkFormula(*pDoc, ScAddress(3,5,0), "SUM($A$6:$C$6)"))
        CPPUNIT_FAIL("Wrong formula in D6");

    if (!checkFormula(*pDoc, ScAddress(3,6,0), "SUM(A7:C7)"))
        CPPUNIT_FAIL("Wrong formula in D7");

    if (!checkFormula(*pDoc, ScAddress(3,9,0), "$Two.$A$2+$Two.$B$2+$Two.$C$2"))
        CPPUNIT_FAIL("Wrong formula in D10");

    if (!checkFormula(*pDoc, ScAddress(3,10,0), "$Two.A3+$Two.B3+$Two.C3"))
        CPPUNIT_FAIL("Wrong formula in D11");

    if (!checkFormula(*pDoc, ScAddress(3,13,0), "MIN($Two.$A$2:$C$2)"))
        CPPUNIT_FAIL("Wrong formula in D14");

    if (!checkFormula(*pDoc, ScAddress(3,14,0), "MAX($Two.A3:C3)"))
        CPPUNIT_FAIL("Wrong formula in D15");

    xDocSh->DoClose();
}

void ScExportTest::testSheetProtectionXLSX()
{
    ScDocShellRef xShell = loadDoc("ProtecteSheet1234Pass.", XLSX);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(xShell, XLSX);
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    const ScTableProtection* pTabProtect = pDoc->GetTabProtection(0);
    CPPUNIT_ASSERT(pTabProtect);
    if ( pTabProtect )
    {
        Sequence<sal_Int8> aHash = pTabProtect->getPasswordHash(PASSHASH_XL);
        
        if (aHash.getLength() >= 2)
        {
            CPPUNIT_ASSERT( (sal_uInt8)aHash[0] == 204 );
            CPPUNIT_ASSERT( (sal_uInt8)aHash[1] == 61 );
        }
        
        CPPUNIT_ASSERT ( !pTabProtect->isOptionEnabled( ScTableProtection::OBJECTS ) );
        CPPUNIT_ASSERT ( !pTabProtect->isOptionEnabled( ScTableProtection::SCENARIOS ) );
    }
    xDocSh->DoClose();
}

namespace {

const char* toBorderName( sal_Int16 eStyle )
{
    switch (eStyle)
    {
        case table::BorderLineStyle::SOLID: return "SOLID";
        case table::BorderLineStyle::DOTTED: return "DOTTED";
        case table::BorderLineStyle::DASHED: return "DASHED";
        case table::BorderLineStyle::DOUBLE: return "DOUBLE";
        case table::BorderLineStyle::FINE_DASHED: return "FINE_DASHED";
        default:
            ;
    }

    return "";
}

}

void ScExportTest::testExcelCellBorders( sal_uLong nFormatType )
{
    ScDocShellRef xDocSh = loadDoc("cell-borders.", nFormatType);

    CPPUNIT_ASSERT_MESSAGE("Failed to load file", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();

    struct
    {
        SCROW mnRow;
        sal_Int16 mnStyle;
        long mnWidth;
    } aChecks[] = {
        {  1, table::BorderLineStyle::SOLID,        1L }, 
        {  3, table::BorderLineStyle::DOTTED,      15L }, 
        {  9, table::BorderLineStyle::FINE_DASHED, 15L }, 
        { 11, table::BorderLineStyle::SOLID,       15L }, 
        { 19, table::BorderLineStyle::DASHED,      35L }, 
        { 21, table::BorderLineStyle::SOLID,       35L }, 
        { 23, table::BorderLineStyle::SOLID,       50L }, 
        { 25, table::BorderLineStyle::DOUBLE,      -1L }, 
    };

    for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
    {
        const editeng::SvxBorderLine* pLine = NULL;
        pDoc->GetBorderLines(2, aChecks[i].mnRow, 0, NULL, &pLine, NULL, NULL);
        CPPUNIT_ASSERT(pLine);
        CPPUNIT_ASSERT_EQUAL(toBorderName(aChecks[i].mnStyle), toBorderName(pLine->GetBorderLineStyle()));
        if (aChecks[i].mnWidth >= 0)
            CPPUNIT_ASSERT_EQUAL(aChecks[i].mnWidth, pLine->GetWidth());
    }

    ScDocShellRef xNewDocSh = saveAndReload(xDocSh, nFormatType);
    xDocSh->DoClose();
    pDoc = xNewDocSh->GetDocument();
    for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
    {
        const editeng::SvxBorderLine* pLine = NULL;
        pDoc->GetBorderLines(2, aChecks[i].mnRow, 0, NULL, &pLine, NULL, NULL);
        CPPUNIT_ASSERT(pLine);
        CPPUNIT_ASSERT_EQUAL(toBorderName(aChecks[i].mnStyle), toBorderName(pLine->GetBorderLineStyle()));
        if (aChecks[i].mnWidth >= 0)
            CPPUNIT_ASSERT_EQUAL(aChecks[i].mnWidth, pLine->GetWidth());
    }

    xNewDocSh->DoClose();
}

void ScExportTest::testCellBordersXLS()
{
    testExcelCellBorders(XLS);
}

void ScExportTest::testCellBordersXLSX()
{
    testExcelCellBorders(XLSX);
}

void ScExportTest::testSharedFormulaExportXLS()
{
    struct
    {
        bool checkContent( ScDocument* pDoc )
        {
            formula::FormulaGrammar::Grammar eGram = formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1;
            pDoc->SetGrammar(eGram);
            sc::TokenStringContext aCxt(pDoc, eGram);

            

            OUString aActual = pDoc->GetString(0,1,0);
            OUString aExpected = "Response";
            if (aActual != aExpected)
            {
                cerr << "Wrong content in A2: expected='" << aExpected << "', actual='" << aActual << "'" << endl;
                return false;
            }

            aActual = pDoc->GetString(1,1,0);
            aExpected = "Response";
            if (aActual != aExpected)
            {
                cerr << "Wrong content in B2: expected='" << aExpected << "', actual='" << aActual << "'" << endl;
                return false;
            }

            
            for (SCROW i = 0; i <= 9; ++i)
            {
                double fExpected = i + 1.0;
                ScAddress aPos(0,i+2,0);
                double fActual = pDoc->GetValue(aPos);
                if (fExpected != fActual)
                {
                    cerr << "Wrong value in A" << (i+2) << ": expected=" << fExpected << ", actual=" << fActual << endl;
                    return false;
                }

                aPos.IncCol();
                ScFormulaCell* pFC = pDoc->GetFormulaCell(aPos);
                if (!pFC)
                {
                    cerr << "B" << (i+2) << " should be a formula cell." << endl;
                    return false;
                }

                OUString aFormula = pFC->GetCode()->CreateString(aCxt, aPos);
                aExpected = "Coefficients!RC[-1]";
                if (aFormula != aExpected)
                {
                    cerr << "Wrong formula in B" << (i+2) << ": expected='" << aExpected << "', actual='" << aFormula << "'" << endl;
                    return false;
                }

                fActual = pDoc->GetValue(aPos);
                if (fExpected != fActual)
                {
                    cerr << "Wrong value in B" << (i+2) << ": expected=" << fExpected << ", actual=" << fActual << endl;
                    return false;
                }
            }

            return true;
        }

    } aTest;

    ScDocShellRef xDocSh = loadDoc("shared-formula/3d-reference.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load file.", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();

    
    bool bRes = aTest.checkContent(pDoc);
    CPPUNIT_ASSERT_MESSAGE("Content check on the original document failed.", bRes);

    ScDocShellRef xDocSh2 = saveAndReload(xDocSh, XLS);
    xDocSh->DoClose();
    CPPUNIT_ASSERT_MESSAGE("Failed to reload file.", xDocSh2.Is());

    pDoc = xDocSh2->GetDocument();

    
    bRes = aTest.checkContent(pDoc);
    CPPUNIT_ASSERT_MESSAGE("Content check on the reloaded document failed.", bRes);

    xDocSh2->DoClose();
}

ScExportTest::ScExportTest()
      : ScBootstrapFixture("/sc/qa/unit/data")
{
}

void ScExportTest::setUp()
{
    test::BootstrapFixture::setUp();

    
    
    m_xCalcComponent =
        getMultiServiceFactory()->createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

void ScExportTest::tearDown()
{
    uno::Reference< lang::XComponent >( m_xCalcComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
