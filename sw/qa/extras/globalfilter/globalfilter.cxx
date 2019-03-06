/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <sfx2/linkmgr.hxx>
#include <comphelper/propertysequence.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <editsh.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentContentOperations.hxx>
#include <doc.hxx>
#include <ndgrf.hxx>
#include <drawdoc.hxx>
#include <ndindex.hxx>
#include <pam.hxx>
#include <unotools/fltrcfg.hxx>
#include <xmloff/odffields.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IMark.hxx>
#include <bookmrk.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase() {}

    void testEmbeddedGraphicRoundtrip();
    void testLinkedGraphicRT();
    void testImageWithSpecialID();
    void testGraphicShape();
    void testCharHighlight();
    void testCharHighlightODF();
    void testCharHighlightBody();
    void testMSCharBackgroundEditing();
    void testCharBackgroundToHighlighting();
#if !defined(_WIN32)
    void testSkipImages();
#endif
    void testRedlineFlags();
    void testBulletAsImage();
    void testTextFormField();
    void testCheckBoxFormField();
    void testDropDownFormField();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testEmbeddedGraphicRoundtrip);
    CPPUNIT_TEST(testLinkedGraphicRT);
    CPPUNIT_TEST(testImageWithSpecialID);
    CPPUNIT_TEST(testGraphicShape);
    CPPUNIT_TEST(testCharHighlight);
    CPPUNIT_TEST(testCharHighlightODF);
    CPPUNIT_TEST(testMSCharBackgroundEditing);
    CPPUNIT_TEST(testCharBackgroundToHighlighting);
#if !defined(_WIN32)
    CPPUNIT_TEST(testSkipImages);
#endif
    CPPUNIT_TEST(testRedlineFlags);
    CPPUNIT_TEST(testBulletAsImage);
    CPPUNIT_TEST(testTextFormField);
    CPPUNIT_TEST(testCheckBoxFormField);
    CPPUNIT_TEST(testDropDownFormField);
    CPPUNIT_TEST_SUITE_END();
};

void Test::testEmbeddedGraphicRoundtrip()
{
    OUString aFilterNames[] = {
        "writer8",
        "Rich Text Format",
        "MS Word 97",
        "Office Open XML Text",
    };

    for (OUString const & rFilterName : aFilterNames)
    {
        // Check whether the export code swaps in the image which was swapped out before by auto mechanism

        if (mxComponent.is())
            mxComponent->dispose();

        mxComponent = loadFromDesktop(m_directories.getURLFromSrc("/sw/qa/extras/globalfilter/data/document_with_two_images.odt"), "com.sun.star.text.TextDocument");

        // Export the document and import again for a check
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);

        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= rFilterName;

        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        uno::Reference< lang::XComponent > xComponent(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.text.TextDocument");

        // Check whether graphic exported well after it was swapped out
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);

        const OString sFailedMessage = OString("Failed on filter: ") + rFilterName.toUtf8();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(2), xDraws->getCount());

        // First image
        uno::Reference<drawing::XShape> xImage(xDraws->getByIndex(0), uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet > XPropSet( xImage, uno::UNO_QUERY_THROW );

        // Check graphic, size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), graphic::GraphicType::PIXEL, xGraphic->getType());
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610), xBitmap->getSize().Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381), xBitmap->getSize().Height);
        }

        // Second Image
        xImage.set(xDraws->getByIndex(1), uno::UNO_QUERY);
        XPropSet.set( xImage, uno::UNO_QUERY_THROW );

        // Check graphic, size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), graphic::GraphicType::PIXEL, xGraphic->getType());
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(900), xBitmap->getSize().Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(600), xBitmap->getSize().Height);
        }
    }
}

void Test::testLinkedGraphicRT()
{
    const OUString aFilterNames[] = {
        "writer8",
//        "Rich Text Format",  Note: picture is there, but SwGrfNode is not found?
        "MS Word 97",
        "Office Open XML Text",
    };

    for (OUString const & rFilterName : aFilterNames)
    {
        if (mxComponent.is())
            mxComponent->dispose();
        mxComponent = loadFromDesktop(m_directories.getURLFromSrc("/sw/qa/extras/globalfilter/data/document_with_linked_graphic.odt"), "com.sun.star.text.TextDocument");

        const OString sFailedMessage = OString("Failed on filter: ") + rFilterName.toUtf8();

        // Export the document and import again for a check
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);

        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= rFilterName;

        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        uno::Reference< lang::XComponent > xComponent(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.text.TextDocument");

        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pTextDoc);
        SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pDoc);
        SwNodes& aNodes = pDoc->GetNodes();

        // Find the image
        bool bImageFound = false;
        Graphic aGraphic;
        for (sal_uLong nIndex = 0; nIndex < aNodes.Count(); ++nIndex)
        {
            if (aNodes[nIndex]->IsGrfNode())
            {
                SwGrfNode* pGrfNode = aNodes[nIndex]->GetGrfNode();
                CPPUNIT_ASSERT(pGrfNode);

                const GraphicObject& rGraphicObj = pGrfNode->GetGrfObj(true);
                aGraphic = rGraphicObj.GetGraphic();
                bImageFound = true;
            }
        }

        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), bImageFound);

        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), GraphicType::Bitmap, aGraphic.GetType());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_uLong(864900), aGraphic.GetSizeBytes());

        // Check if linked graphic is registered in LinkManager
        sfx2::LinkManager& rLinkManager = pTextDoc->GetDocShell()->GetDoc()->GetEditShell()->GetLinkManager();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), size_t(1), rLinkManager.GetLinks().size());
        const tools::SvRef<sfx2::SvBaseLink> & rLink = rLinkManager.GetLinks()[0];
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), rLink->GetLinkSourceName().indexOf("linked_graphic.jpg") >= 0);
    }
}

void Test::testImageWithSpecialID()
{
    // Check how LO handles when the imported graphic's ID is different from that one
    // which is generated by LO.

    const OUString aFilterNames[] = {
        "writer8",
        "Rich Text Format",
        "MS Word 97",
        "Office Open XML Text",
    };

    for (OUString const & rFilterName : aFilterNames)
    {
        if (mxComponent.is())
            mxComponent->dispose();
        mxComponent = loadFromDesktop(m_directories.getURLFromSrc("/sw/qa/extras/globalfilter/data/images_with_special_IDs.odt"), "com.sun.star.text.TextDocument");

        // Export the document and import again for a check
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);

        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= rFilterName;

        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        uno::Reference< lang::XComponent > xComponent(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.text.TextDocument");

        // Check whether graphic exported well
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);

        const OString sFailedMessage = OString("Failed on filter: ") + rFilterName.toUtf8();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(2), xDraws->getCount());

        uno::Reference<drawing::XShape> xImage = getShape(1);
        uno::Reference< beans::XPropertySet > XPropSet( xImage, uno::UNO_QUERY_THROW );

        // Check graphic, size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), graphic::GraphicType::PIXEL, xGraphic->getType());
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610), xBitmap->getSize().Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381), xBitmap->getSize().Height);
        }

        // Second Image
        xImage.set(xDraws->getByIndex(1), uno::UNO_QUERY);
        XPropSet.set( xImage, uno::UNO_QUERY_THROW );

        // Check graphic, size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), graphic::GraphicType::PIXEL, xGraphic->getType());
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(900), xBitmap->getSize().Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(600), xBitmap->getSize().Height);
        }
    }
}

/// Gives the first embedded or linked image in a document.
static uno::Reference<drawing::XShape> lcl_getShape(const uno::Reference<lang::XComponent>& xComponent, bool bEmbedded)
{
    uno::Reference<drawing::XShape> xShape;

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    for (sal_Int32 i = 0; i < xDrawPage->getCount(); ++i)
    {
        uno::Reference<beans::XPropertySet> xShapeProperties(xDrawPage->getByIndex(i), uno::UNO_QUERY);
        uno::Reference<graphic::XGraphic> xGraphic;
        xShapeProperties->getPropertyValue("Graphic") >>= xGraphic;
        if (xGraphic.is())
        {
            Graphic aGraphic(xGraphic);

            if (bEmbedded == aGraphic.getOriginURL().isEmpty())
            {
                xShape.set(xShapeProperties, uno::UNO_QUERY);
                return xShape;
            }
        }
    }

    return xShape;
}

void Test::testGraphicShape()
{
    // There are two kind of images in Writer: 1) Writer specific handled by SwGrfNode and
    // 2) graphic shape handled by SdrGrafObj (e.g. after copy&paste from Impress).

    const OUString aFilterNames[] = {
        "writer8",
        "Rich Text Format",
        "MS Word 97",
        "Office Open XML Text",
    };

    for (OUString const & rFilterName : aFilterNames)
    {
        if (mxComponent.is())
            mxComponent->dispose();
        mxComponent = loadFromDesktop(m_directories.getURLFromSrc("/sw/qa/extras/globalfilter/data/graphic_shape.odt"), "com.sun.star.text.TextDocument");

        // Export the document and import again for a check
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);

        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= rFilterName;

        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        uno::Reference< lang::XComponent > xComponent(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.text.TextDocument");

        // Check whether graphic exported well
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);

        const OString sFailedMessage = OString("Failed on filter: ") + rFilterName.toUtf8();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(2), xDraws->getCount());

        uno::Reference<drawing::XShape> xImage = lcl_getShape(mxComponent, true);
        CPPUNIT_ASSERT_MESSAGE("Couldn't load the shape/image", xImage.is());
        uno::Reference< beans::XPropertySet > XPropSet( xImage, uno::UNO_QUERY );
        // First image is embedded
        // Check size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610), xBitmap->getSize().Width );
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381), xBitmap->getSize().Height );
        }

        // MS filters make this kind of linked images broken !?
        if (rFilterName != "writer8")
            return;

        // Second image is a linked one
        xImage = lcl_getShape(mxComponent, false);
        XPropSet.set(xImage, uno::UNO_QUERY);
        const OString sFailedImageLoad = OString("Couldn't load the shape/image for ") + rFilterName.toUtf8();
        CPPUNIT_ASSERT_MESSAGE(sFailedImageLoad.getStr(), xImage.is());

        // Check size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());

            Graphic aGraphic(xGraphic);
            OUString sURL = aGraphic.getOriginURL();
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), sURL.endsWith("linked_graphic.jpg"));

            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(620), xBitmap->getSize().Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(465), xBitmap->getSize().Height);
        }
    }
}

void Test::testCharHighlightBody()
{
    // MS Word has two kind of character backgrounds called character shading and highlighting
    // MS filters handle these attributes separately, but ODF export merges them into one background attribute

    const OUString aFilterNames[] = {
        "writer8",
        "Rich Text Format",
        "MS Word 97",
        "Office Open XML Text",
    };

    for (OUString const & rFilterName : aFilterNames)
    {
        if (mxComponent.is())
            mxComponent->dispose();
        mxComponent = loadFromDesktop(m_directories.getURLFromSrc("/sw/qa/extras/globalfilter/data/char_highlight.docx"),
                                      "com.sun.star.text.TextDocument");

        const OString sFailedMessage = OString("Failed on filter: ") + rFilterName.toUtf8();

        // Export the document and import again for a check
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);

        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= rFilterName;

        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        uno::Reference< lang::XComponent > xComponent(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.text.TextDocument");

        const uno::Reference< text::XTextRange > xPara = getParagraph(1);
        // Both highlight and background
        const sal_Int32 nBackColor(0x4F81BD);
        for( int nRun = 1; nRun <= 16; ++nRun )
        {
            const uno::Reference<beans::XPropertySet> xRun(getRun(xPara,nRun), uno::UNO_QUERY);
            sal_Int32 nHighlightColor = 0;
            switch( nRun )
            {
                case 1: nHighlightColor = 0x000000; break; //black
                case 2: nHighlightColor = 0x0000ff; break; //blue
                case 3: nHighlightColor = 0x00ffff; break; //cyan
                case 4: nHighlightColor = 0x00ff00; break; //green
                case 5: nHighlightColor = 0xff00ff; break; //magenta
                case 6: nHighlightColor = 0xff0000; break; //red
                case 7: nHighlightColor = 0xffff00; break; //yellow
                case 8: nHighlightColor = 0xffffff; break; //white
                case 9: nHighlightColor = 0x000080;  break;//dark blue
                case 10: nHighlightColor = 0x008080; break; //dark cyan
                case 11: nHighlightColor = 0x008000; break; //dark green
                case 12: nHighlightColor = 0x800080; break; //dark magenta
                case 13: nHighlightColor = 0x800000; break; //dark red
                case 14: nHighlightColor = 0x808000; break; //dark yellow
                case 15: nHighlightColor = 0x808080; break; //dark gray
                case 16: nHighlightColor = 0xC0C0C0; break; //light gray
            }

            if (rFilterName == "writer8")
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(COL_TRANSPARENT), getProperty<sal_Int32>(xRun,"CharHighlight"));
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), nHighlightColor, getProperty<sal_Int32>(xRun,"CharBackColor"));
            }
            else // MS filters
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), nHighlightColor, getProperty<sal_Int32>(xRun,"CharHighlight"));
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), nBackColor, getProperty<sal_Int32>(xRun,"CharBackColor"));
            }
        }

        // Only highlight
        {
            const uno::Reference<beans::XPropertySet> xRun(getRun(xPara,18), uno::UNO_QUERY);
            if (rFilterName == "writer8")
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(COL_TRANSPARENT), getProperty<sal_Int32>(xRun,"CharHighlight"));
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(0xff0000), getProperty<sal_Int32>(xRun,"CharBackColor"));
            }
            else
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(0xff0000), getProperty<sal_Int32>(xRun,"CharHighlight"));
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(COL_TRANSPARENT), getProperty<sal_Int32>(xRun,"CharBackColor"));
            }
        }

        // Only background
        {
            const uno::Reference<beans::XPropertySet> xRun(getRun(xPara,19), uno::UNO_QUERY);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(COL_TRANSPARENT), getProperty<sal_Int32>(xRun,"CharHighlight"));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(0x0000ff), getProperty<sal_Int32>(xRun,"CharBackColor"));
        }
    }
}

void Test::testCharHighlight()
{
    SvtFilterOptions& rOpt = SvtFilterOptions::Get();
    rOpt.SetCharBackground2Shading();

    testCharHighlightBody();

    rOpt.SetCharBackground2Highlighting();

    testCharHighlightBody();
}

void Test::testCharHighlightODF()
{
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc("/sw/qa/extras/globalfilter/data/char_background_editing.docx"),
                                      "com.sun.star.text.TextDocument");

    // don't check import, testMSCharBackgroundEditing already does that

    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    for (int i = 1; i <= 4; ++i)
    {
        uno::Reference<beans::XPropertySet> xRun(getRun(xPara,i), uno::UNO_QUERY);
        switch (i)
        {
            case 1: // non-transparent highlight
            xRun->setPropertyValue("CharBackColor", uno::makeAny(static_cast<sal_Int32>(128)));
            xRun->setPropertyValue("CharBackTransparent", uno::makeAny(true));
            xRun->setPropertyValue("CharHighlight", uno::makeAny(static_cast<sal_Int32>(64)));
            break;

            case 2: // transparent backcolor
            xRun->setPropertyValue("CharBackColor", uno::makeAny(static_cast<sal_Int32>(128)));
            xRun->setPropertyValue("CharBackTransparent", uno::makeAny(true));
            xRun->setPropertyValue("CharHighlight", uno::makeAny(static_cast<sal_Int32>(COL_TRANSPARENT)));
            break;

            case 3: // non-transparent backcolor
            xRun->setPropertyValue("CharBackColor", uno::makeAny(static_cast<sal_Int32>(128)));
            xRun->setPropertyValue("CharBackTransparent", uno::makeAny(false));
            xRun->setPropertyValue("CharHighlight", uno::makeAny(static_cast<sal_Int32>(COL_TRANSPARENT)));
            break;

            case 4: // non-transparent highlight again
            xRun->setPropertyValue("CharBackColor", uno::makeAny(static_cast<sal_Int32>(128)));
            xRun->setPropertyValue("CharBackTransparent", uno::makeAny(false));
            xRun->setPropertyValue("CharHighlight", uno::makeAny(static_cast<sal_Int32>(64)));
            break;
        }
    }

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer8");

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    uno::Reference< lang::XComponent > xComponent(xStorable, uno::UNO_QUERY);
    xComponent->dispose();
    mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.text.TextDocument");

    xPara.set(getParagraph(1));
    for (int i = 1; i <= 4; ++i)
    {
        uno::Reference<beans::XPropertySet> xRun(getRun(xPara,i), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(COL_TRANSPARENT), getProperty<sal_Int32>(xRun, "CharHighlight"));
        switch (i)
        {
            case 1: // non-transparent highlight
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(64), getProperty<sal_Int32>(xRun, "CharBackColor"));
            CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xRun, "CharBackTransparent"));
            break;
            case 2: // transparent backcolor
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(COL_TRANSPARENT), getProperty<sal_Int32>(xRun, "CharBackColor"));
            CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xRun, "CharBackTransparent"));
            break;
            case 3: // non-transparent backcolor
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(128), getProperty<sal_Int32>(xRun, "CharBackColor"));
            CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xRun, "CharBackTransparent"));
            break;
            case 4: // non-transparent highlight again
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(64), getProperty<sal_Int32>(xRun, "CharBackColor"));
            CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xRun, "CharBackTransparent"));
            break;
        }
    }
}

void Test::testMSCharBackgroundEditing()
{
    // Simulate the editing process of imported MSO character background attributes
    // and check how export behaves.

    const OUString aFilterNames[] = {
        "writer8",
        "Rich Text Format",
        "MS Word 97",
        "Office Open XML Text",
    };

    for (OUString const & rFilterName : aFilterNames)
    {
        if (mxComponent.is())
            mxComponent->dispose();

        mxComponent = loadFromDesktop(m_directories.getURLFromSrc("/sw/qa/extras/globalfilter/data/char_background_editing.docx"),
                                      "com.sun.star.text.TextDocument");

        const OString sFailedMessage = OString("Failed on filter: ") + rFilterName.toUtf8();

        // Check whether import was done on the right way
        uno::Reference< text::XTextRange > xPara = getParagraph(1);
        {
            uno::Reference<beans::XPropertySet> xRun(getRun(xPara,1), uno::UNO_QUERY);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(COL_TRANSPARENT), getProperty<sal_Int32>(xRun,"CharHighlight"));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(0xff0000), getProperty<sal_Int32>(xRun,"CharBackColor"));

            xRun.set(getRun(xPara,2), uno::UNO_QUERY);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(0x0000ff), getProperty<sal_Int32>(xRun,"CharHighlight"));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(COL_TRANSPARENT), getProperty<sal_Int32>(xRun,"CharBackColor"));

            xRun.set(getRun(xPara,3), uno::UNO_QUERY);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(0x0000ff), getProperty<sal_Int32>(xRun,"CharHighlight"));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(0xff0000), getProperty<sal_Int32>(xRun,"CharBackColor"));

            xRun.set(getRun(xPara,4), uno::UNO_QUERY);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(COL_TRANSPARENT), getProperty<sal_Int32>(xRun,"CharHighlight"));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(COL_TRANSPARENT), getProperty<sal_Int32>(xRun,"CharBackColor"));
        }

        // Simulate editing
        for( int i = 1; i <= 4; ++i )
        {
            uno::Reference<beans::XPropertySet> xRun(getRun(xPara,i), uno::UNO_QUERY);
            // Change background
            sal_Int32 nBackColor = 0;
            switch( i )
            {
                case 1: nBackColor = 0x000000; break; //black
                case 2: nBackColor = 0x00ffff; break; //cyan
                case 3: nBackColor = 0x00ff00; break; //green
                case 4: nBackColor = 0xff00ff; break; //magenta
            }
            xRun->setPropertyValue("CharBackColor", uno::makeAny(nBackColor));
            // Remove highlighting
            xRun->setPropertyValue("CharHighlight", uno::makeAny(static_cast<sal_Int32>(COL_TRANSPARENT)));
            // Remove shading marker
            uno::Sequence<beans::PropertyValue> aGrabBag = getProperty<uno::Sequence<beans::PropertyValue> >(xRun,"CharInteropGrabBag");
            for (int j = 0; j < aGrabBag.getLength(); ++j)
            {
                beans::PropertyValue& rProp = aGrabBag[j];
                if (rProp.Name == "CharShadingMarker")
                {
                    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), true, rProp.Value.get<bool>());
                    rProp.Value <<= false;
                }
            }
            xRun->setPropertyValue("CharInteropGrabBag", uno::makeAny(aGrabBag));
        }

        SvtFilterOptions& rOpt = SvtFilterOptions::Get();
        rOpt.SetCharBackground2Highlighting();

        // Export the document and import again for a check
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);

        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= rFilterName;

        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        uno::Reference< lang::XComponent > xComponent(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.text.TextDocument");

        // Check whether background was exported as highlighting
        xPara.set(getParagraph(1));
        for( int i = 1; i <= 4; ++i )
        {
            sal_Int32 nBackColor = 0;
            switch( i )
            {
                case 1: nBackColor = 0x000000; break; //black
                case 2: nBackColor = 0x00ffff; break; //cyan
                case 3: nBackColor = 0x00ff00; break; //green
                case 4: nBackColor = 0xff00ff; break; //magenta
            }
            const uno::Reference<beans::XPropertySet> xRun(getRun(xPara,i), uno::UNO_QUERY);
            if (rFilterName == "writer8")
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(COL_TRANSPARENT), getProperty<sal_Int32>(xRun,"CharHighlight"));
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), nBackColor, getProperty<sal_Int32>(xRun,"CharBackColor"));
            }
            else
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), nBackColor, getProperty<sal_Int32>(xRun,"CharHighlight"));
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(COL_TRANSPARENT), getProperty<sal_Int32>(xRun,"CharBackColor"));
            }
        }
    }
}

void Test::testCharBackgroundToHighlighting()
{
    // MSO highlighting has less kind of values so let's see how LO character background is converted
    // to these values

    const OUString aFilterNames[] = {
        "Rich Text Format",
        "MS Word 97",
        "Office Open XML Text",
    };

    for (OUString const & rFilterName : aFilterNames)
    {
        if (mxComponent.is())
            mxComponent->dispose();
        mxComponent = loadFromDesktop(m_directories.getURLFromSrc("/sw/qa/extras/globalfilter/data/char_background.odt"),
                                      "com.sun.star.text.TextDocument");

        OString sFailedMessage = OString("Failed on filter: ") + rFilterName.toUtf8();


        SvtFilterOptions& rOpt = SvtFilterOptions::Get();
        rOpt.SetCharBackground2Highlighting();

        // Export the document and import again for a check
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);

        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= rFilterName;

        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        uno::Reference< lang::XComponent > xComponent(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.text.TextDocument");

        // Check highlight color
        const uno::Reference< text::XTextRange > xPara = getParagraph(1);
        for( int nRun = 1; nRun <= 19; ++nRun )
        {
            const uno::Reference<beans::XPropertySet> xRun(getRun(xPara,nRun), uno::UNO_QUERY);
            sal_Int32 nHighlightColor = 0;
            switch( nRun )
            {
                case 1: nHighlightColor = 0x000000; break; //black
                case 2: nHighlightColor = 0xffff00; break; //yellow
                case 3: nHighlightColor = 0xff00ff; break; //magenta
                case 4: nHighlightColor = 0x00ffff; break; //cyan
                case 5: nHighlightColor = 0xffff00; break; //yellow
                case 6: nHighlightColor = 0xff0000; break; //red
                case 7: nHighlightColor = 0x0000ff; break; //blue
                case 8: nHighlightColor = 0x00ff00; break; //green
                case 9: nHighlightColor = 0x008000; break; //dark green
                case 10: nHighlightColor = 0x800080; break; //dark magenta
                case 11: nHighlightColor = 0x000080; break; //dark blue
                case 12: nHighlightColor = 0x808000; break; //dark yellow
                case 13: nHighlightColor = 0x808080; break; //dark gray
                case 14: nHighlightColor = 0x000000; break; //black
                case 15: nHighlightColor = 0xff0000; break; //red
                case 16: nHighlightColor = 0xC0C0C0; break; //light gray
                case 17: nHighlightColor = 0x800000; break; //dark red
                case 18: nHighlightColor = 0x808080; break; //dark gray
                case 19: nHighlightColor = 0xffff00; break; //yellow
            }
            const OString sMessage = sFailedMessage +". Index of run with unmatched color: " + OString::number(nRun);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), nHighlightColor, getProperty<sal_Int32>(xRun,"CharHighlight"));
        }
    }
}

#if !defined(_WIN32)
void Test::testSkipImages()
{
    // Check how LO skips image loading (but not texts of textboxes and custom shapes)
    // during DOC and DOCX import, using the "SkipImages" FilterOptions.

    std::pair<OUString, OUString> aFilterNames[] = {
        { "/sw/qa/extras/globalfilter/data/skipimages.doc", "" },
        { "/sw/qa/extras/globalfilter/data/skipimages.doc", "SkipImages" },
        { "/sw/qa/extras/globalfilter/data/skipimages.docx", "" },
        { "/sw/qa/extras/globalfilter/data/skipimages.docx", "SkipImages" }
    };

    for (auto const & rFilterNamePair : aFilterNames)
    {
        bool bSkipImages = !rFilterNamePair.second.isEmpty();
        OString sFailedMessage = OString("Failed on filter: ") + rFilterNamePair.first.toUtf8();

        if (mxComponent.is())
            mxComponent->dispose();

        if (bSkipImages)
        {
            // FilterOptions parameter
            uno::Sequence<beans::PropertyValue> args(comphelper::InitPropertySequence({
                    { "FilterOptions", uno::Any(rFilterNamePair.second) }
            }));
            mxComponent = loadFromDesktop(m_directories.getURLFromSrc(rFilterNamePair.first), "com.sun.star.text.TextDocument", args);
            sFailedMessage = sFailedMessage + " - " + rFilterNamePair.second.toUtf8();
        }
        else
        {
            mxComponent = loadFromDesktop(m_directories.getURLFromSrc(rFilterNamePair.first), "com.sun.star.text.TextDocument");
        }

        // Check shapes (images, textboxes, custom shapes)
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
        uno::Reference<drawing::XShape> xShape;
        uno::Reference<graphic::XGraphic> xGraphic;
        uno::Reference< beans::XPropertySet > XPropSet;
        uno::Reference<awt::XBitmap> xBitmap;

        bool bHasTextboxText = false;
        bool bHasCustomShapeText = false;
        sal_Int32 nImageCount = 0;

        for (int i = 1; i<= xDraws->getCount(); i++)
        {
            xShape = getShape(i);
            XPropSet.set( xShape, uno::UNO_QUERY_THROW );
            try
            {
                XPropSet->getPropertyValue("Graphic") >>= xGraphic;
                xBitmap.set(xGraphic, uno::UNO_QUERY);
                if (xBitmap.is())
                    nImageCount++;
            }
            catch (beans::UnknownPropertyException &)
            { /* ignore */ }

            uno::Reference<text::XTextRange> xText(xShape, uno::UNO_QUERY);
            if (xText.is())
            {
                OUString shapeText = xText->getString();
                if (shapeText.startsWith("Lorem ipsum"))
                    bHasTextboxText = true;
                else if (shapeText.startsWith("Nam pretium"))
                    bHasCustomShapeText = true;
            }
        }

        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), bHasTextboxText);
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), bHasCustomShapeText);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(bSkipImages ? 0 : 3), nImageCount );
    }
}
#endif

void Test::testRedlineFlags()
{
    const OUString aFilterNames[] = {
        "writer8",
        "Rich Text Format",
        "MS Word 97",
        "Office Open XML Text",
    };

    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();

    SwPaM pam(SwPosition(SwNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1)));
    pDoc->getIDocumentContentOperations().InsertString(pam, "foo bar baz");

    IDocumentRedlineAccess & rIDRA(pDoc->getIDocumentRedlineAccess());
    // enable change tracking
    rIDRA.SetRedlineFlags(rIDRA.GetRedlineFlags()
        | RedlineFlags::On | RedlineFlags::ShowDelete);

    // need a delete redline to trigger mode switching
    pam.Move(fnMoveForward, GoInDoc);
    pam.SetMark();
    pam.Move(fnMoveBackward, GoInDoc);
    pDoc->getIDocumentContentOperations().DeleteAndJoin(pam);

    // hide delete redlines
    RedlineFlags const nRedlineFlags =
        rIDRA.GetRedlineFlags() & ~RedlineFlags::ShowDelete;
    rIDRA.SetRedlineFlags(nRedlineFlags);

    for (OUString const & rFilterName : aFilterNames)
    {
        // export the document
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);

        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= rFilterName;
        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(),
                aMediaDescriptor.getAsConstPropertyValueList());

        // tdf#97103 check that redline mode is properly restored
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OString(OString("redline mode not restored in ") + rFilterName.toUtf8()).getStr(),
            static_cast<int>(nRedlineFlags), static_cast<int>(rIDRA.GetRedlineFlags()));
    }
}

void Test::testBulletAsImage()
{
    OUString aFilterNames[] = {
        "writer8",
        "MS Word 97",
        "Office Open XML Text",
        "Rich Text Format",
    };

    for (OUString const & rFilterName : aFilterNames)
    {
        OString sFailedMessage = OString("Failed on filter: ") + rFilterName.toUtf8();

        if (mxComponent.is())
            mxComponent->dispose();

        mxComponent = loadFromDesktop(m_directories.getURLFromSrc("/sw/qa/extras/globalfilter/data/BulletAsImage.odt"), "com.sun.star.text.TextDocument");

        // Check if import was successful
        {
            uno::Reference<text::XTextRange> xPara(getParagraph(1));
            uno::Reference<beans::XPropertySet> xPropertySet(xPara, uno::UNO_QUERY);
            uno::Reference<container::XIndexAccess> xLevels;
            xLevels.set(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
            uno::Sequence<beans::PropertyValue> aProperties;
            xLevels->getByIndex(0) >>= aProperties;
            uno::Reference<awt::XBitmap> xBitmap;
            awt::Size aSize;
            sal_Int16 nNumberingType = -1;

            for (beans::PropertyValue const & rProperty : aProperties)
            {
                if (rProperty.Name == "NumberingType")
                {
                    nNumberingType = rProperty.Value.get<sal_Int16>();
                }
                else if (rProperty.Name == "GraphicBitmap")
                {
                    if (rProperty.Value.has<uno::Reference<awt::XBitmap>>())
                    {
                        xBitmap = rProperty.Value.get<uno::Reference<awt::XBitmap>>();
                    }
                }
                else if (rProperty.Name == "GraphicSize")
                {
                    aSize = rProperty.Value.get<awt::Size>();
                }
            }

            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), style::NumberingType::BITMAP, nNumberingType);

            // Graphic Bitmap
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            Graphic aGraphic(uno::Reference<graphic::XGraphic>(xBitmap, uno::UNO_QUERY));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), GraphicType::Bitmap, aGraphic.GetType());
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), aGraphic.GetSizeBytes() > sal_uLong(0));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), 16L, aGraphic.GetSizePixel().Width());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), 16L, aGraphic.GetSizePixel().Height());

            // Graphic Size
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(400), aSize.Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(400), aSize.Height);
        }

        // Export the document and import again for a check
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);

        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= rFilterName;


        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        uno::Reference<lang::XComponent> xComponent(xStorable, uno::UNO_QUERY);
        xComponent->dispose();

        mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.text.TextDocument");

        {
            uno::Reference<text::XTextRange> xPara(getParagraph(1));
            uno::Reference<beans::XPropertySet> xPropertySet(xPara, uno::UNO_QUERY);
            uno::Reference<container::XIndexAccess> xLevels;
            xLevels.set(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
            uno::Sequence<beans::PropertyValue> aProperties;
            xLevels->getByIndex(0) >>= aProperties;
            uno::Reference<awt::XBitmap> xBitmap;
            awt::Size aSize;
            sal_Int16 nNumberingType = -1;

            for (beans::PropertyValue const & rProperty : aProperties)
            {
                if (rProperty.Name == "NumberingType")
                {
                    nNumberingType = rProperty.Value.get<sal_Int16>();
                }
                else if (rProperty.Name == "GraphicBitmap")
                {
                    if (rProperty.Value.has<uno::Reference<awt::XBitmap>>())
                    {
                        xBitmap = rProperty.Value.get<uno::Reference<awt::XBitmap>>();
                    }
                }
                else if (rProperty.Name == "GraphicSize")
                {
                    aSize = rProperty.Value.get<awt::Size>();
                }
            }

            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), style::NumberingType::BITMAP, nNumberingType);

            // Graphic Bitmap
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            Graphic aGraphic(uno::Reference<graphic::XGraphic>(xBitmap, uno::UNO_QUERY));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), GraphicType::Bitmap, aGraphic.GetType());
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), aGraphic.GetSizeBytes() > sal_uLong(0));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), 16L, aGraphic.GetSizePixel().Width());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), 16L, aGraphic.GetSizePixel().Height());

            // Graphic Size
            if (rFilterName == "write8") // ODT is correct
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(400), aSize.Width);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(400), aSize.Height);
            }
            // FIXME: MS Filters don't work correctly for graphic bullet size
            else if (rFilterName == "Office Open XML Text" || rFilterName == "Rich Text Format")
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(279), aSize.Width);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(279), aSize.Height);
            }
            else if (rFilterName == "MS Word 97")
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(296), aSize.Width);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(296), aSize.Height);
            }
        }
    }
}

void Test::testTextFormField()
{
    const OUString aFilterNames[] = {
        "writer8",
        "MS Word 97",
        "Office Open XML Text",
    };

    for (const OUString& rFilterName : aFilterNames)
    {
        if (mxComponent.is())
            mxComponent->dispose();
        mxComponent = loadFromDesktop(m_directories.getURLFromSrc("/sw/qa/extras/globalfilter/data/text_form_field.odt"), "com.sun.star.text.TextDocument");

        const OString sFailedMessage = OString("Failed on filter: ") + rFilterName.toUtf8();

        // Export the document and import again for a check
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= rFilterName;
        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        uno::Reference< lang::XComponent > xComponent(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.text.TextDocument");

        // Check the document after round trip
        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pTextDoc);
        SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
        IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();

        // We have two text form fields
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(2), pMarkAccess->getAllMarksCount());

        // Check whether all fieldmarks are text form fields
        for(auto aIter = pMarkAccess->getAllMarksBegin(); aIter != pMarkAccess->getAllMarksEnd(); ++aIter)
        {
            ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(aIter->get());
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pFieldmark);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), OUString(ODF_FORMTEXT), pFieldmark->GetFieldname());
        }

        // In the first paragraph we have an empty text form field with the placeholder spaces
        const uno::Reference< text::XTextRange > xPara = getParagraph(1);
        sal_Unicode vEnSpaces[5] = {8194, 8194, 8194, 8194, 8194};
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), OUString(vEnSpaces, 5), xPara->getString());

        // In the second paragraph we have a set text
        const uno::Reference< text::XTextRange > xPara2 = getParagraph(2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), OUString("xxxxx"), xPara2->getString());
    }
}

void Test::testCheckBoxFormField()
{
    const OUString aFilterNames[] = {
        "writer8",
        "MS Word 97",
        "Office Open XML Text",
    };

    for (const OUString& rFilterName : aFilterNames)
    {
        if (mxComponent.is())
            mxComponent->dispose();
        mxComponent = loadFromDesktop(m_directories.getURLFromSrc("/sw/qa/extras/globalfilter/data/checkbox_form_field.odt"), "com.sun.star.text.TextDocument");

        const OString sFailedMessage = OString("Failed on filter: ") + rFilterName.toUtf8();

        // Export the document and import again for a check
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= rFilterName;
        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        uno::Reference< lang::XComponent > xComponent(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.text.TextDocument");

        // Check the document after round trip
        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pTextDoc);
        SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
        IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();

        // We have two check box form fields
        if(rFilterName == "Office Open XML Text")
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(4), pMarkAccess->getAllMarksCount());
        else
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(2), pMarkAccess->getAllMarksCount());

        int nIndex = 0;
        for(auto aIter = pMarkAccess->getAllMarksBegin(); aIter != pMarkAccess->getAllMarksEnd(); ++aIter)
        {
            ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(aIter->get());

            if(rFilterName == "Office Open XML Text") // OOXML import also generates bookmarks
            {
                if(!pFieldmark)
                    continue;
            }

            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pFieldmark);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), OUString(ODF_FORMCHECKBOX), pFieldmark->GetFieldname());
            ::sw::mark::ICheckboxFieldmark* pCheckBox = dynamic_cast< ::sw::mark::ICheckboxFieldmark* >(pFieldmark);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pCheckBox);

            // The first one is unchecked, the other one is checked
            if(nIndex == 0)
                CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), !pCheckBox->IsChecked());
            else
                CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pCheckBox->IsChecked());
            ++nIndex;
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), int(2), nIndex);
    }
}

void Test::testDropDownFormField()
{
    const OUString aFilterNames[] = {
        "writer8",
        "MS Word 97",
        "Office Open XML Text",
    };

    for (const OUString& rFilterName : aFilterNames)
    {
        if (mxComponent.is())
            mxComponent->dispose();
        mxComponent = loadFromDesktop(m_directories.getURLFromSrc("/sw/qa/extras/globalfilter/data/dropdown_form_field.odt"), "com.sun.star.text.TextDocument");

        const OString sFailedMessage = OString("Failed on filter: ") + rFilterName.toUtf8();

        // Export the document and import again for a check
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= rFilterName;
        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        uno::Reference< lang::XComponent > xComponent(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.text.TextDocument");

        // Check the document after round trip
        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pTextDoc);
        SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
        IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();

        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(2), pMarkAccess->getAllMarksCount());

        int nIndex = 0;
        for(auto aIter = pMarkAccess->getAllMarksBegin(); aIter != pMarkAccess->getAllMarksEnd(); ++aIter)
        {
            ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(aIter->get());

            if(!pFieldmark)
                continue;

            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pFieldmark);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), OUString(ODF_FORMDROPDOWN), pFieldmark->GetFieldname());

            // Check drop down field's parameters.
            const sw::mark::IFieldmark::parameter_map_t* const pParameters = pFieldmark->GetParameters();
            css::uno::Sequence<OUString> vListEntries;
            sal_Int32 nSelection = -1;
            auto pListEntries = pParameters->find(ODF_FORMDROPDOWN_LISTENTRY);
            if (pListEntries != pParameters->end())
            {
                pListEntries->second >>= vListEntries;

                if(vListEntries.getLength())
                {
                    auto pResult = pParameters->find(ODF_FORMDROPDOWN_RESULT);
                    if (pResult != pParameters->end())
                    {
                        pResult->second >>= nSelection;
                    }
                }
            }

            // The first one is empty
            if(nIndex == 0)
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(0), vListEntries.getLength());
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(-1), nSelection);
            }
            else // The second one has list and also a selected item
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(4), vListEntries.getLength());
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(1), nSelection);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), OUString("1000"), vListEntries[0]);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), OUString("2000"), vListEntries[1]);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), OUString("3000"), vListEntries[2]);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), OUString("4000"), vListEntries[3]);
            }
            ++nIndex;
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), int(2), nIndex);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
