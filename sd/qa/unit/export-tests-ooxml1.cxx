/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <officecfg/Office/Common.hxx>
#include "sdmodeltestbase.hxx"
#include <Outliner.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/postitem.hxx>

#include <oox/drawingml/drawingmltypes.hxx>

#include <svx/svdoutl.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdogrp.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/sdooitm.hxx>
#include <svx/sdmetitm.hxx>
#include <unotools/mediadescriptor.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>

#include <svx/svdotable.hxx>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

using namespace css;

class SdOOXMLExportTest1 : public SdModelTestBaseXML
{
public:
    void testFdo90607();
    void testTdf127237();
    void testBnc887230();
    void testBnc870233_1();
    void testBnc870233_2();
    void testN828390_4();
    void testN828390_5();
    void testFdo71961();
    void testN828390();
    void testBnc880763();
    void testBnc862510_5();
    void testBnc822347_EmptyBullet();
    void testFdo83751();
    void testFdo79731();
    void testTableCellFillProperties();
    void testBulletStartNumber();
    void testLineStyle();
    void testCellLeftAndRightMargin();
    void testRightToLeftParaghraph();
    void testTextboxWithHyperlink();
    void testMergedCells();
    void testTableCellBorder();
    void testBulletColor();
    void testBulletCharAndFont();
    void testBulletMarginAndIndentation();
    void testParaMarginAndindentation();
    void testTdf111884();
    void testTdf112633();
    void testTdf128952();
    void testTdf127090();
    void testCustomXml();
    void testTdf94238();
    void testPictureTransparency();
    void testTdf125554();
    void testRoundtripOwnLineStyles();
    void testRoundtripPrstDash();
    void testDashOnHairline();
    void testCustomshapeBitmapfillSrcrect();
    void testTdf100348FontworkBitmapFill();
    void testTdf100348FontworkGradientGlow();
    void testTdf128345FullTransparentGradient();
    void testTdf128345GradientLinear();
    void testTdf128345GradientRadial();
    void testTdf128345GradientAxial();
    void testTdf134969TransparencyOnColorGradient();
    void testTdf136911();

    CPPUNIT_TEST_SUITE(SdOOXMLExportTest1);

    CPPUNIT_TEST(testFdo90607);
    CPPUNIT_TEST(testTdf127237);
    CPPUNIT_TEST(testBnc887230);
    CPPUNIT_TEST(testBnc870233_1);
    CPPUNIT_TEST(testBnc870233_2);
    CPPUNIT_TEST(testN828390_4);
    CPPUNIT_TEST(testN828390_5);
    CPPUNIT_TEST(testFdo71961);
    CPPUNIT_TEST(testN828390);
    CPPUNIT_TEST(testBnc880763);
    CPPUNIT_TEST(testBnc862510_5);
    CPPUNIT_TEST(testBnc822347_EmptyBullet);
    CPPUNIT_TEST(testFdo83751);
    CPPUNIT_TEST(testFdo79731);
    CPPUNIT_TEST(testTableCellFillProperties);
    CPPUNIT_TEST(testBulletStartNumber);
    CPPUNIT_TEST(testLineStyle);
    CPPUNIT_TEST(testCellLeftAndRightMargin);
    CPPUNIT_TEST(testRightToLeftParaghraph);
    CPPUNIT_TEST(testTextboxWithHyperlink);
    CPPUNIT_TEST(testMergedCells);
    CPPUNIT_TEST(testTableCellBorder);
    CPPUNIT_TEST(testBulletColor);
    CPPUNIT_TEST(testBulletCharAndFont);
    CPPUNIT_TEST(testBulletMarginAndIndentation);
    CPPUNIT_TEST(testParaMarginAndindentation);
    CPPUNIT_TEST(testTdf111884);
    CPPUNIT_TEST(testTdf112633);
    CPPUNIT_TEST(testTdf128952);
    CPPUNIT_TEST(testTdf127090);
    CPPUNIT_TEST(testCustomXml);
    CPPUNIT_TEST(testTdf94238);
    CPPUNIT_TEST(testTdf125554);
    CPPUNIT_TEST(testPictureTransparency);
    CPPUNIT_TEST(testRoundtripOwnLineStyles);
    CPPUNIT_TEST(testRoundtripPrstDash);
    CPPUNIT_TEST(testDashOnHairline);
    CPPUNIT_TEST(testCustomshapeBitmapfillSrcrect);
    CPPUNIT_TEST(testTdf100348FontworkBitmapFill);
    CPPUNIT_TEST(testTdf100348FontworkGradientGlow);
    CPPUNIT_TEST(testTdf128345FullTransparentGradient);
    CPPUNIT_TEST(testTdf128345GradientLinear);
    CPPUNIT_TEST(testTdf128345GradientRadial);
    CPPUNIT_TEST(testTdf128345GradientAxial);
    CPPUNIT_TEST(testTdf134969TransparencyOnColorGradient);
    CPPUNIT_TEST(testTdf136911);

    CPPUNIT_TEST_SUITE_END();

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override
    {
        static const struct { char const * pPrefix; char const * pURI; } namespaces[] =
        {
            // OOXML
            { "ContentType", "http://schemas.openxmlformats.org/package/2006/content-types" },
            { "rels", "http://schemas.openxmlformats.org/package/2006/relationships" },
            { "mc", "http://schemas.openxmlformats.org/markup-compatibility/2006" },
            { "v", "urn:schemas-microsoft-com:vml" },
            { "a", "http://schemas.openxmlformats.org/drawingml/2006/main" },
            { "c", "http://schemas.openxmlformats.org/drawingml/2006/chart" },
            { "m", "http://schemas.openxmlformats.org/officeDocument/2006/math" },
            { "pic", "http://schemas.openxmlformats.org/drawingml/2006/picture" },
            { "wp", "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" },
            { "p", "http://schemas.openxmlformats.org/presentationml/2006/main" },
            { "w", "http://schemas.openxmlformats.org/wordprocessingml/2006/main" },
            { "a14", "http://schemas.microsoft.com/office/drawing/2010/main" },
            { "wps", "http://schemas.microsoft.com/office/word/2010/wordprocessingShape" },
            { "wpg", "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup" },
        };
        for (size_t i = 0; i < SAL_N_ELEMENTS(namespaces); ++i)
        {
            xmlXPathRegisterNs(pXmlXPathCtx,
                reinterpret_cast<xmlChar const *>(namespaces[i].pPrefix),
                reinterpret_cast<xmlChar const *>(namespaces[i].pURI));
        }
    }
};

namespace {

template< typename ItemValue, typename ItemType >
void checkFontAttributes( const SdrTextObj* pObj, ItemValue nVal)
{
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
    const EditTextObject& aEdit = pObj->GetOutlinerParaObject()->GetTextObject();
    std::vector<EECharAttrib> rLst;
    aEdit.GetCharAttribs(0, rLst);
    for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
    {
        const ItemType* pAttrib = dynamic_cast<const ItemType *>((*it).pAttr);
        if (pAttrib)
        {
            CPPUNIT_ASSERT_EQUAL( nVal, static_cast<ItemValue>(pAttrib->GetValue()));
        }
    }
}

}

void SdOOXMLExportTest1::testTdf127237()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf127237.pptx"), PPTX );
    xDocShRef = saveAndReload(xDocShRef.get(), ODP);

    const SdrPage* pPage = GetPage(1, xDocShRef);
    CPPUNIT_ASSERT(pPage != nullptr);

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj != nullptr);
    uno::Reference< table::XCellRange > xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);

    sal_Int32 nFillColor = 0;
    uno::Reference< beans::XPropertySet > xCell(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nFillColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x0070C0), nFillColor);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testBnc887230()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bnc887230.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 0 ) );
    // Without the fix in place, this test would have failed with
    //- Expected: 255
    //- Actual  : 13421823
    checkFontAttributes<Color, SvxColorItem>( pObj, Color(0x0000ff) );

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testBnc870233_1()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bnc870233_1.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    // The problem was all shapes had the same font (the last parsed font attributes overwrote all previous ones)

    // First shape has red, bold font
    {
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 0 ) );
        checkFontAttributes<Color, SvxColorItem>( pObj, Color(0xff0000) );
        checkFontAttributes<FontWeight, SvxWeightItem>( pObj, WEIGHT_BOLD );
    }

    // Second shape has blue, italic font
    {
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 1 ) );
        checkFontAttributes<Color, SvxColorItem>( pObj, Color(0x0000ff) );
        checkFontAttributes<FontItalic, SvxPostureItem>( pObj, ITALIC_NORMAL );
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testBnc870233_2()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bnc870233_2.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    // The problem was in some SmartArts font color was wrong

    // First smart art has blue font color (direct formatting)
    {
        const SdrObjGroup *pObjGroup = dynamic_cast<SdrObjGroup *>(pPage->GetObj(0));
        CPPUNIT_ASSERT(pObjGroup);
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>(pObjGroup->GetSubList()->GetObj(0));
        checkFontAttributes<Color, SvxColorItem>(pObj, Color(0x0000ff));
    }

    // Second smart art has "dk2" font color (style)
    {
        const SdrObjGroup *pObjGroup = dynamic_cast<SdrObjGroup *>(pPage->GetObj(1));
        CPPUNIT_ASSERT(pObjGroup);
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>(pObjGroup->GetSubList()->GetObj(0));
        checkFontAttributes<Color, SvxColorItem>( pObj, Color(0x1F497D) );
    }

    // Third smart art has white font color (style)
    {
        const SdrObjGroup *pObjGroup = dynamic_cast<SdrObjGroup *>(pPage->GetObj(2));
        CPPUNIT_ASSERT(pObjGroup);
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>(pObjGroup->GetSubList()->GetObj(0));
        checkFontAttributes<Color, SvxColorItem>(pObj, Color(0xffffff));
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testN828390_4()
{
    bool bPassed = false;
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/n828390_4.odp"), ODP );

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        std::vector<EECharAttrib> rLst;
        SdrObject *pObj = pPage->GetObj(0);
        SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
        CPPUNIT_ASSERT( pTxtObj );
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        aEdit.GetCharAttribs(0, rLst);
        for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
        {
            const SvxFontHeightItem * pFontHeight = dynamic_cast<const SvxFontHeightItem *>((*it).pAttr);
            if( pFontHeight && (*it).nStart == 18 )
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Font height is wrong", static_cast<sal_uInt32>(1129), pFontHeight->GetHeight() );
            const SvxFontItem *pFont = dynamic_cast<const SvxFontItem *>((*it).pAttr);
            if( pFont )
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Font is wrong", OUString("Arial"), pFont->GetFamilyName() );
                bPassed = true;
            }
            const SvxWeightItem *pWeight = dynamic_cast<const SvxWeightItem *>((*it).pAttr);
            if( pWeight && (*it).nStart == 18 )
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Font Weight is wrong", WEIGHT_BOLD, pWeight->GetWeight() );
        }
    }
    CPPUNIT_ASSERT(bPassed);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testN828390_5()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/n828390_5.odp"), ODP );

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        SdrObject *pObj = pPage->GetObj(0);
        SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
        CPPUNIT_ASSERT( pTxtObj );
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        const SvxNumBulletItem& rNumFmt = aEdit.GetParaAttribs(3).Get(EE_PARA_NUMBULLET);
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Bullet's relative size is wrong!", sal_uInt16(75), rNumFmt.GetNumRule()->GetLevel(1).GetBulletRelSize() ); // != 25
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testFdo71961()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/fdo71961.odp"), ODP);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    // Export to .pptx changes all text frames to custom shape objects, which obey TextWordWrap property
    // (which is false for text frames otherwise and is ignored). Check that frames that should wrap still do.
    SdrObjCustomShape *pTxtObj = dynamic_cast<SdrObjCustomShape *>( pPage->GetObj( 1 ));
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( OUString( "Text to be always wrapped" ), pTxtObj->GetOutlinerParaObject()->GetTextObject().GetText(0));
    CPPUNIT_ASSERT_EQUAL( true, pTxtObj->GetMergedItem(SDRATTR_TEXT_WORDWRAP).GetValue());

    pTxtObj = dynamic_cast<SdrObjCustomShape *>( pPage->GetObj( 2 ));
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( OUString( "Custom shape non-wrapped text" ), pTxtObj->GetOutlinerParaObject()->GetTextObject().GetText(0));
    CPPUNIT_ASSERT_EQUAL( false, pTxtObj->GetMergedItem(SDRATTR_TEXT_WORDWRAP).GetValue());

    pTxtObj = dynamic_cast<SdrObjCustomShape *>( pPage->GetObj( 3 ));
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( OUString( "Custom shape wrapped text" ), pTxtObj->GetOutlinerParaObject()->GetTextObject().GetText(0));
    CPPUNIT_ASSERT_EQUAL( true, pTxtObj->GetMergedItem(SDRATTR_TEXT_WORDWRAP).GetValue());

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testN828390()
{
    bool bPassed = false;
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/n828390.pptx"), PPTX );

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        std::vector<EECharAttrib> rLst;
        // Get the object
        SdrObject *pObj = pPage->GetObj(0);
        SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
        CPPUNIT_ASSERT( pTxtObj );
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        aEdit.GetCharAttribs(0, rLst);
        bPassed = std::any_of(rLst.rbegin(), rLst.rend(),
            [](const EECharAttrib& rCharAttr) {
                const SvxEscapementItem *pFontEscapement = dynamic_cast<const SvxEscapementItem *>(rCharAttr.pAttr);
                return pFontEscapement && (pFontEscapement->GetEsc() == -25);
            });
    }
    CPPUNIT_ASSERT_MESSAGE("Subscript not exported properly", bPassed);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testBnc880763()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bnc880763.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    // Check z-order of the two shapes, use background color to identify them
    // First object in the background has blue background color
    const SdrObjGroup *pObjGroup = dynamic_cast<SdrObjGroup *>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObjGroup);
    const SdrObject *pObj = pObjGroup->GetSubList()->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( Color(0x0000ff),(static_cast< const XColorItem& >(pObj->GetMergedItem(XATTR_FILLCOLOR))).GetColorValue());

    // Second object at the front has green background color
    pObj = pPage->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( Color(0x00ff00),(static_cast< const XColorItem& >(pObj->GetMergedItem(XATTR_FILLCOLOR))).GetColorValue());

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testBnc862510_5()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bnc862510_5.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    // Same as testBnc870237, but here we check the horizontal spacing
    const SdrObjGroup *pObjGroup = dynamic_cast<SdrObjGroup *>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObjGroup);
    const SdrObject* pObj = pObjGroup->GetSubList()->GetObj(2);
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), pObj->GetMergedItem(SDRATTR_TEXT_UPPERDIST).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), pObj->GetMergedItem(SDRATTR_TEXT_LOWERDIST).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(7510), pObj->GetMergedItem(SDRATTR_TEXT_RIGHTDIST).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), pObj->GetMergedItem(SDRATTR_TEXT_LEFTDIST).GetValue());

    xDocShRef->DoClose();
}

// In numbering a bullet could be defined as empty (no character).
// When exporting to OOXML make sure that the bullet is ignored and
// not written into the file.
void SdOOXMLExportTest1::testBnc822347_EmptyBullet()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/bnc822347_EmptyBullet.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX);

    SdDrawDocument* pDoc = xDocShRef->GetDoc();
    SdrOutliner* pOutliner = pDoc->GetInternalOutliner();
    const SdrPage* pPage = pDoc->GetPage(1);
    SdrObject* pObject = pPage->GetObj(0);
    SdrTextObj* pTextObject = dynamic_cast<SdrTextObj*>(pObject);
    CPPUNIT_ASSERT(pTextObject);

    OutlinerParaObject* pOutlinerParagraphObject = pTextObject->GetOutlinerParaObject();
    const EditTextObject& aEdit = pOutlinerParagraphObject->GetTextObject();

    OUString sText = aEdit.GetText(0);
    CPPUNIT_ASSERT_EQUAL(OUString("M3 Feature Test"), sText);

    pOutliner->SetText(*pOutlinerParagraphObject);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pOutliner->GetParagraphCount());

    const sal_Int16 nDepth = pOutliner->GetDepth(0);

    CPPUNIT_ASSERT_EQUAL(sal_Int16(-1), nDepth); // depth >= 0 means that the paragraph has bullets enabled

    xDocShRef->DoClose();
}

//Bullets not having  any text following them are not getting exported to pptx correctly.
void SdOOXMLExportTest1::testFdo90607()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/fdo90607.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(1) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    OutlinerParaObject* pOutlinerParagraphObject = pTxtObj->GetOutlinerParaObject();
    const sal_Int16 nDepth = pOutlinerParagraphObject->GetDepth(0);
    CPPUNIT_ASSERT_MESSAGE("not equal", nDepth != -1);
    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testFdo83751()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/fdo83751.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );

    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier( xDocShRef->GetModel(), uno::UNO_QUERY );
    uno::Reference<document::XDocumentProperties> xProps = xDocumentPropertiesSupplier->getDocumentProperties();
    uno::Reference<beans::XPropertySet> xUDProps( xProps->getUserDefinedProperties(), uno::UNO_QUERY );
    OUString propValue;
    xUDProps->getPropertyValue("Testing") >>= propValue;
    CPPUNIT_ASSERT_EQUAL(OUString("Document"), propValue);
    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testFdo79731()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/fdo79731.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT(pDoc);
    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testTableCellFillProperties()
{
    std::shared_ptr< comphelper::ConfigurationChanges > batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Cache::GraphicManager::TotalCacheSize::set(sal_Int32(1), batch);
    batch->commit();

    // Load the original file
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/Table_with_Cell_Fill.odp"), ODP);

    // Export the document and import again for a check
    uno::Reference< lang::XComponent > xComponent = xDocShRef->GetModel();
    uno::Reference<frame::XStorable> xStorable(xComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OStringToOUString(std::string_view(aFileFormats[PPTX].pFilterName), RTL_TEXTENCODING_UTF8);

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    xComponent.set(xStorable, uno::UNO_QUERY);
    xComponent->dispose();
    xDocShRef = loadURL(aTempFile.GetURL(), PPTX);

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );
    uno::Reference< table::XCellRange > xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xCell;

    // Test Solid fill color
    sal_Int32 nColor;
    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6750207), nColor);

    // Test Picture fill type for cell
    drawing::FillStyle aFillStyle( drawing::FillStyle_NONE );
    xCell.set(xTable->getCellByPosition(0, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillStyle") >>= aFillStyle;
    CPPUNIT_ASSERT_EQUAL(int(drawing::FillStyle_BITMAP), static_cast<int>(aFillStyle));

    // Test Gradient fill type for cell
    xCell.set(xTable->getCellByPosition(1, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillStyle") >>= aFillStyle;
    CPPUNIT_ASSERT_EQUAL(int(drawing::FillStyle_GRADIENT), static_cast<int>(aFillStyle));

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testBulletStartNumber()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/n90255.pptx"), PPTX );
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(0) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem *pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Bullet's start number is wrong!", sal_Int16(3), sal_Int16(pNumFmt->GetNumRule()->GetLevel(0).GetStart()) );
    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testLineStyle()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/lineStyle.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    SdrObject const* pShape = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("no shape", pShape != nullptr);

    const XLineStyleItem& rStyleItem = dynamic_cast<const XLineStyleItem&>(
                pShape->GetMergedItem(XATTR_LINESTYLE));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong style", int(drawing::LineStyle_SOLID), static_cast<int>(rStyleItem.GetValue()));

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testRightToLeftParaghraph()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/rightToLeftParagraph.pptx"), PPTX);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    // Get first paragraph
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< beans::XPropertySet > xPropSet( xParagraph, uno::UNO_QUERY_THROW );

    sal_Int16 nWritingMode = 0;
    xPropSet->getPropertyValue( "WritingMode" ) >>= nWritingMode;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong paragraph WritingMode", text::WritingMode2::RL_TB, nWritingMode);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testTextboxWithHyperlink()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/hyperlinktest.pptx"), PPTX);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    // Get first paragraph
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );

    // first chunk of text
    uno::Reference<text::XTextRange> xRun( getRunFromParagraph( 0, xParagraph ) );
    uno::Reference< beans::XPropertySet > xPropSet( xRun, uno::UNO_QUERY_THROW );

    uno::Reference<text::XTextField> xField;
    xPropSet->getPropertyValue("TextField") >>= xField;
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is() );

    xPropSet.set(xField, uno::UNO_QUERY);
    OUString aURL;
    xPropSet->getPropertyValue("URL") >>= aURL;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("URLs don't match", OUString("http://www.xkcd.com/"), aURL);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testTdf136911()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/ppt/tdf136911.ppt"), PPT);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    // Get second paragraph
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );

    // first chunk of text
    uno::Reference<text::XTextRange> xRun( getRunFromParagraph( 0, xParagraph ) );
    uno::Reference< beans::XPropertySet > xPropSet( xRun, uno::UNO_QUERY_THROW );

    uno::Reference<text::XTextField> xField;
    xPropSet->getPropertyValue("TextField") >>= xField;
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is() );

    xPropSet.set(xField, uno::UNO_QUERY);
    OUString aURL;
    xPropSet->getPropertyValue("URL") >>= aURL;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("URLs don't match", OUString("http://google.com"), aURL);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testBulletColor()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bulletColor.pptx"), PPTX );

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(0) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);

    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem *pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Bullet's color is wrong!", Color(0xff0000),pNumFmt->GetNumRule()->GetLevel(0).GetBulletColor());

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testBulletCharAndFont()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/bulletCharAndFont.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< beans::XPropertySet > xPropSet( xParagraph, uno::UNO_QUERY_THROW );

    uno::Reference<container::XIndexAccess> xLevels(xPropSet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level
    OUString    sBulletChar(u'\xf06c');
    for (beans::PropertyValue const & rProp : std::as_const(aProps))
    {
        if (rProp.Name == "BulletChar")
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "BulletChar incorrect.", sBulletChar ,rProp.Value.get<OUString>());
        if (rProp.Name == "BulletFont")
        {
            awt::FontDescriptor aFontDescriptor;
            rProp.Value >>= aFontDescriptor;
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "BulletFont incorrect.", OUString("Wingdings"),aFontDescriptor.Name);
        }
    }
    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testBulletMarginAndIndentation()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bulletMarginAndIndent.pptx"), PPTX );
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(0) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);

    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem *pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Bullet's left margin is wrong!", sal_Int32(1000),pNumFmt->GetNumRule()->GetLevel(0).GetAbsLSpace() ); // left margin is 0.79 cm
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Bullet's indentation is wrong!", sal_Int32(-998),pNumFmt->GetNumRule()->GetLevel(0). GetFirstLineOffset());

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testParaMarginAndindentation()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/paraMarginAndIndentation.pptx"), PPTX);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    // Get first paragraph
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< beans::XPropertySet > xPropSet( xParagraph, uno::UNO_QUERY_THROW );

    sal_Int32 nParaLeftMargin = 0;
    xPropSet->getPropertyValue( "ParaLeftMargin" ) >>= nParaLeftMargin;
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(1000), sal_uInt32(nParaLeftMargin));

    sal_Int32 nParaFirstLineIndent = 0;
    xPropSet->getPropertyValue( "ParaFirstLineIndent" ) >>= nParaFirstLineIndent;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1268), nParaFirstLineIndent);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testCellLeftAndRightMargin()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/n90223.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    sal_Int32 nLeftMargin, nRightMargin;

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );

    uno::Reference< css::table::XTable > xTable (pTableObj->getTable(), uno::UNO_SET_THROW);
    uno::Reference< css::table::XMergeableCell > xCell( xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xCellPropSet(xCell, uno::UNO_QUERY_THROW);

    uno::Any aLeftMargin = xCellPropSet->getPropertyValue("TextLeftDistance");
    aLeftMargin >>= nLeftMargin ;

    uno::Any aRightMargin = xCellPropSet->getPropertyValue("TextRightDistance");
    aRightMargin >>= nRightMargin ;

    // Convert values to EMU
    nLeftMargin  =  oox::drawingml::convertHmmToEmu( nLeftMargin );
    nRightMargin =  oox::drawingml::convertHmmToEmu( nRightMargin );

    CPPUNIT_ASSERT_EQUAL(sal_Int32(45720), nLeftMargin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(45720), nRightMargin);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testMergedCells()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/cellspan.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));

    CPPUNIT_ASSERT( pTableObj );
    uno::Reference< table::XTable > xTable(pTableObj->getTable(), uno::UNO_SET_THROW);
    uno::Reference< text::XTextRange > xText1(xTable->getCellByPosition(3, 0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL( OUString("0,3"), xText1->getString() );

    uno::Reference< text::XTextRange > xText2(xTable->getCellByPosition(3, 2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL( OUString("2,3"), xText2->getString() );
}

void SdOOXMLExportTest1::testTableCellBorder()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/n90190.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );

    table::BorderLine2 aBorderLine;

    uno::Reference< table::XTable > xTable(pTableObj->getTable(), uno::UNO_SET_THROW);
    uno::Reference< css::table::XMergeableCell > xCell(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xCellPropSet (xCell, uno::UNO_QUERY_THROW);

    xCellPropSet->getPropertyValue("LeftBorder") >>= aBorderLine;
// While importing the table cell border line width, it converts EMU->Hmm then divided result by 2.
// To get original value of LineWidth need to multiple by 2.
    sal_Int32 nLeftBorder = aBorderLine.LineWidth * 2;
    nLeftBorder = oox::drawingml::convertHmmToEmu( nLeftBorder );
    CPPUNIT_ASSERT(nLeftBorder);
    CPPUNIT_ASSERT_EQUAL(util::Color(45296), aBorderLine.Color);

    xCellPropSet->getPropertyValue("RightBorder") >>= aBorderLine;
    sal_Int32 nRightBorder = aBorderLine.LineWidth * 2;
    nRightBorder = oox::drawingml::convertHmmToEmu( nRightBorder );
    CPPUNIT_ASSERT(nRightBorder);
    CPPUNIT_ASSERT_EQUAL(util::Color(16777215), aBorderLine.Color);

    xCellPropSet->getPropertyValue("TopBorder") >>= aBorderLine;
    sal_Int32 nTopBorder = aBorderLine.LineWidth * 2;
    nTopBorder = oox::drawingml::convertHmmToEmu( nTopBorder );
    CPPUNIT_ASSERT(nTopBorder);
    CPPUNIT_ASSERT_EQUAL(util::Color(45296), aBorderLine.Color);


    xCellPropSet->getPropertyValue("BottomBorder") >>= aBorderLine;
    sal_Int32 nBottomBorder = aBorderLine.LineWidth * 2;
    nBottomBorder = oox::drawingml::convertHmmToEmu( nBottomBorder );
    CPPUNIT_ASSERT(nBottomBorder);
    CPPUNIT_ASSERT_EQUAL(util::Color(45296), aBorderLine.Color);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testTdf111884()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf111884.pptx"), PPTX);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);

    const SdrPage *pPage = GetPage(1, xDocShRef);
    SdrObject const* pShape = pPage->GetObj(2);
    CPPUNIT_ASSERT_MESSAGE("no shape", pShape != nullptr);

    // must be a group shape
    CPPUNIT_ASSERT_EQUAL(OBJ_GRUP, pShape->GetObjIdentifier());

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testTdf112633()
{
    // Load document and export it to a temporary file
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf112633.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    xmlDocUniquePtr pRelsDoc = parseExport(tempFile, "ppt/slides/_rels/slide1.xml.rels");

    // Check image with artistic effect exists in the slide
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/"
            "a14:imgLayer/a14:imgEffect/a14:artisticPencilGrayscale",
            "pencilSize", "80");

    // Check there is a relation with the .wdp file that contains the backed up image
    OUString sEmbedId1 = getXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:extLst/"
            "a:ext/a14:imgProps/a14:imgLayer", "embed");
    OUString sXmlPath = "/rels:Relationships/rels:Relationship[@Id='" + sEmbedId1 + "']";
    assertXPath(pRelsDoc, OUStringToOString( sXmlPath, RTL_TEXTENCODING_UTF8 ), "Target", "../media/hdphoto1.wdp");

    // Check the .wdp file exists
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(
            comphelper::getComponentContext(m_xSFactory), tempFile.GetURL());
    CPPUNIT_ASSERT_EQUAL(true, bool(xNameAccess->hasByName("ppt/media/hdphoto1.wdp")));
}

void SdOOXMLExportTest1::testTdf128952()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf128952.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:xfrm/a:off", "x", "360");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:xfrm/a:off", "y", "-360");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:xfrm/a:ext", "cx", "1919880");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:xfrm/a:ext", "cy", "1462680");
}

void SdOOXMLExportTest1::testTdf127090()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf127090.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr", "rot", "-5400000");
}

void SdOOXMLExportTest1::testCustomXml()
{
    // Load document and export it to a temporary file
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/customxml.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "customXml/item1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    xmlDocUniquePtr pRelsDoc = parseExport(tempFile, "customXml/_rels/item1.xml.rels");
    CPPUNIT_ASSERT(pRelsDoc);

    // Check there is a relation to itemProps1.xml.
    assertXPath(pRelsDoc, "/rels:Relationships/rels:Relationship", 1);
    assertXPath(pRelsDoc, "/rels:Relationships/rels:Relationship[@Id='rId1']", "Target", "itemProps1.xml");

    std::unique_ptr<SvStream> pStream = parseExportStream(tempFile, "ddp/ddpfile.xen");
    CPPUNIT_ASSERT(pStream);
}

void SdOOXMLExportTest1::testTdf94238()
{
    // Load document and export it to a temporary file.
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf94238.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocShRef->GetDoc()->getUnoModel(),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());

    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPage.is());

    uno::Reference<beans::XPropertySet> xShape(getShape(0, xPage));
    CPPUNIT_ASSERT(xShape.is());

    awt::Gradient aGradient;
    CPPUNIT_ASSERT(xShape->getPropertyValue("FillGradient") >>= aGradient);

    // Without the accompanying fix in place, this test would have failed with
    // the following details:
    // - aGradient.Style was awt::GradientStyle_ELLIPTICAL
    // - aGradient.YOffset was 70
    // - aGradient.Border was 0
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_RADIAL, aGradient.Style);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(100), aGradient.YOffset);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(39), aGradient.Border);

    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 0, Actual  : 10592673', i.e. the start color of the gradient
    // was incorrect.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x8B8B8B), aGradient.EndColor);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testPictureTransparency()
{
    // Load document and export it to a temporary file.
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/image_transparency.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocShRef->GetDoc()->getUnoModel(),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());

    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPage.is());

    uno::Reference<beans::XPropertySet> xGraphicShape(getShape(0, xPage));
    CPPUNIT_ASSERT(xGraphicShape.is());

    sal_Int16 nTransparency = 0;
    CPPUNIT_ASSERT(xGraphicShape->getPropertyValue("Transparency") >>= nTransparency);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(51), nTransparency);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testTdf125554()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf125554.pptx"), PPTX);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);

    uno::Reference<beans::XPropertySet> xShape = getShapeFromPage(0, 0, xDocShRef);
    uno::Any aFillTransparenceGradientName
        = xShape->getPropertyValue("FillTransparenceGradientName");
    CPPUNIT_ASSERT(aFillTransparenceGradientName.has<OUString>());
    // Without the accompanying fix in place, this test would have failed, i.e. the transparency of
    // the shape has no gradient, so it looked like a solid fill instead of a gradient fill.
    CPPUNIT_ASSERT(!aFillTransparenceGradientName.get<OUString>().isEmpty());

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testRoundtripOwnLineStyles()
{
    // Load odp document and read the LineDash values.
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/LineStylesOwn.odp"), ODP);
    uno::Reference<drawing::XDrawPagesSupplier> xDocodp(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDocodp.is());
    uno::Reference<drawing::XDrawPage> xPageodp(xDocodp->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPageodp.is());
    drawing::LineDash aLineDashodp[10];
    for (sal_uInt16 i= 0; i < 10; i++)
    {
        uno::Reference<beans::XPropertySet> xShapeodp(getShape(i, xPageodp));
        CPPUNIT_ASSERT(xShapeodp.is());
        xShapeodp->getPropertyValue("LineDash") >>= aLineDashodp[i];
    }

    // Save to pptx, reload and compare the LineDash values
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    uno::Reference<drawing::XDrawPagesSupplier> xDocpptx(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDocpptx.is());
    uno::Reference<drawing::XDrawPage> xPagepptx(xDocpptx->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPagepptx.is());

    for (sal_uInt16 i = 0; i < 10; i++)
    {
        drawing::LineDash aLineDashpptx;
        uno::Reference<beans::XPropertySet> xShapepptx(getShape(i, xPagepptx));
        CPPUNIT_ASSERT(xShapepptx.is());
        xShapepptx->getPropertyValue("LineDash") >>= aLineDashpptx;
        bool bIsSameLineDash = (aLineDashodp[i].Style == aLineDashpptx.Style
                                && aLineDashodp[i].Dots == aLineDashpptx.Dots
                                && aLineDashodp[i].DotLen == aLineDashpptx.DotLen
                                && aLineDashodp[i].Dashes == aLineDashpptx.Dashes
                                && aLineDashodp[i].DashLen == aLineDashpptx.DashLen
                                && aLineDashodp[i].Distance == aLineDashpptx.Distance);
        CPPUNIT_ASSERT_MESSAGE("LineDash differ", bIsSameLineDash);
    }
    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testRoundtripPrstDash()
{
    // load and save document, compare prstDash values in saved document with original.
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/presetDashDot.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    const OUString sOriginal[] = {
        "dash",
        "dashDot",
        "dot",
        "lgDash",
        "lgDashDot",
        "lgDashDotDot",
        "sysDash",
        "sysDashDot",
        "sysDashDotDot",
        "sysDot"
    };
    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sStart = "/p:sld/p:cSld/p:spTree/p:sp[";
    const OString sEnd = "]/p:spPr/a:ln/a:prstDash";
    for (sal_uInt16 i = 0; i < 10; i++)
    {
        OString sXmlPath = sStart + OString::number(i+1) + sEnd;
        OUString sResaved = getXPath(pXmlDoc, sXmlPath, "val");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong prstDash", sOriginal[i], sResaved);
    }

    // tdf#126746: Make sure that dash-dot pattern starts with the longer dash, as defined in OOXML
    // Make sure Style is drawing::DashStyle_RECTRELATIVE
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPage.is());
    for (sal_uInt16 i = 0; i < 10; i++)
    {
        drawing::LineDash aLineDash;
        uno::Reference<beans::XPropertySet> xShape(getShape(i, xPage));
        CPPUNIT_ASSERT(xShape.is());
        xShape->getPropertyValue("LineDash") >>= aLineDash;
        CPPUNIT_ASSERT_MESSAGE("First dash is short", aLineDash.DotLen >= aLineDash.DashLen);
        bool bIsRectRelative = aLineDash.Style == drawing::DashStyle_RECTRELATIVE;
        CPPUNIT_ASSERT_MESSAGE("not RECTRELATIVE", bIsRectRelative);
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testDashOnHairline()
{
    // load and save document, make sure the custDash has 11 child elements.
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf127267DashOnHairline.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:ln/a:custDash/a:ds", 11);
}

void SdOOXMLExportTest1::testCustomshapeBitmapfillSrcrect()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/customshape-bitmapfill-srcrect.pptx"),
        PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sXmlPath = "//a:blipFill/a:srcRect";
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//a:blipFill/a:srcRect' number of nodes is incorrect
    // i.e. <a:srcRect> was exported as <a:fillRect> in <a:stretch>, which made part of the image
    // invisible.
    double fLeftPercent = std::round(getXPath(pXmlDoc, sXmlPath, "l").toDouble() / 1000);
    CPPUNIT_ASSERT_EQUAL(4.0, fLeftPercent);
    double fRightPercent = std::round(getXPath(pXmlDoc, sXmlPath, "r").toDouble() / 1000);
    CPPUNIT_ASSERT_EQUAL(4.0, fRightPercent);
}

void SdOOXMLExportTest1::testTdf100348FontworkBitmapFill()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf100348_FontworkBitmapFill.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    // Make sure the fontwork shape has a blip bitmap fill and a colored outline.
    // Without the patch, fill and outline were black.
    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sPathStart("//p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr");
    assertXPath(pXmlDoc, sPathStart + "/a:blipFill/a:blip", 1);
    assertXPath(pXmlDoc, sPathStart + "/a:ln/a:solidFill/a:srgbClr", "val", "ffbf00");
}

void SdOOXMLExportTest1::testTdf100348FontworkGradientGlow()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf100348_FontworkGradientGlow.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    // Make sure the fontwork shape has a gradient fill and a colored glow.
    // Without the patch, fill was black and no glow applied.
    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sPathStart("//p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr");
    assertXPath(pXmlDoc, sPathStart + "/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr", "val", "8d281e");
    assertXPath(pXmlDoc, sPathStart + "/a:effectLst/a:glow", "rad", "63360");
    assertXPath(pXmlDoc, sPathStart + "/a:effectLst/a:glow/a:srgbClr", "val", "ff4500");
}

void SdOOXMLExportTest1::testTdf128345FullTransparentGradient()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf128345_FullTransparentGradient.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    // Make sure the shape has no fill. Without the patch, fill was solid red.
    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:noFill");
}

void SdOOXMLExportTest1::testTdf128345GradientLinear()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf128345_GradientLinear.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    // Make sure the shape has a lin fill. Without the patch, fill was solid red.
    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sPathStart("//p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:gradFill");
    assertXPath(pXmlDoc, sPathStart + "/a:lin", "ang", "3600000");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs",2);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]", "pos", "25000");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]/a:srgbClr", "val", "ff0000");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]/a:srgbClr/a:alpha", "val", "20000");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]", "pos", "100000");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]/a:srgbClr", "val", "ff0000");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]/a:srgbClr/a:alpha", "val", "80000");
}

void SdOOXMLExportTest1::testTdf128345GradientRadial()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf128345_GradientRadial.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    // Make sure the shape has transparency. In OOXML alpha means 'opacity' with default
    // 100000 for full opak, so only the full transparency with val 0 should be written.
    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sPathStart("//p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:gradFill");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs",2);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]/a:srgbClr", "val", "ff0000");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]/a:srgbClr/a:alpha", 0);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]/a:srgbClr", "val", "ffffff");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]/a:srgbClr/a:alpha", "val", "0");
}

void SdOOXMLExportTest1::testTdf128345GradientAxial()
{
    // Without the patch, symmetric linear gradient with full transparence outside and
    // full opak in the middle were imported as full transparent.
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf128345_GradientAxial.odp"), ODP);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);
    uno::Reference<beans::XPropertySet> xShapePropSet(getShapeFromPage(0, 0, xDocShRef));

    awt::Gradient aTransparenceGradient;
    xShapePropSet->getPropertyValue("FillTransparenceGradient") >>= aTransparenceGradient;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), aTransparenceGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xffffff), aTransparenceGradient.EndColor);
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aTransparenceGradient.Style);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testTdf134969TransparencyOnColorGradient()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf134969_TransparencyOnColorGradient.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    // Make sure the shape has a transparency in gradient stops.
    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sPathStart("//p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:gradFill");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs",2);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]/a:srgbClr/a:alpha", "val", "60000");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]/a:srgbClr/a:alpha", "val", "60000");
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdOOXMLExportTest1);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
