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
#include <officecfg/Office/Common.hxx>
#include <comphelper/processfactory.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <ndgrf.hxx>
#include <drawdoc.hxx>
#include <unotools/fltrcfg.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase() {}

    void testSwappedOutImageExport();
    void testLinkedGraphicRT();
    void testImageWithSpecialID();
    void testGraphicShape();
    void testCharHighlight();
    void testCharHighlightBody();
    void testMSCharBackgroundEditing();
    void testCharBackgroundToHighlighting();
#if !defined(WNT)
    void testSkipImages();
#endif

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testSwappedOutImageExport);
    CPPUNIT_TEST(testLinkedGraphicRT);
    CPPUNIT_TEST(testImageWithSpecialID);
    CPPUNIT_TEST(testGraphicShape);
    CPPUNIT_TEST(testCharHighlight);
    CPPUNIT_TEST(testMSCharBackgroundEditing);
    CPPUNIT_TEST(testCharBackgroundToHighlighting);
#if !defined(WNT)
    CPPUNIT_TEST(testSkipImages);
#endif
    CPPUNIT_TEST_SUITE_END();
};

void Test::testSwappedOutImageExport()
{
    const char* aFilterNames[] = {
        "writer8",
        "Rich Text Format",
        "MS Word 97",
        "Office Open XML Text",
    };

    for( size_t nFilter = 0; nFilter < SAL_N_ELEMENTS(aFilterNames); ++nFilter )
    {
        // Check whether the export code swaps in the image which was swapped out before by auto mechanism

        // Set cache size to a very small value to make sure one of the images is swapped out
        std::shared_ptr< comphelper::ConfigurationChanges > batch(comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Cache::GraphicManager::TotalCacheSize::set(static_cast<sal_Int32>(1), batch);
        batch->commit();

        if (mxComponent.is())
            mxComponent->dispose();
        mxComponent = loadFromDesktop(getURLFromSrc("/sw/qa/extras/globalfilter/data/document_with_two_images.odt"), "com.sun.star.text.TextDocument");

        // Export the document and import again for a check
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);

        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= OUString::createFromAscii(aFilterNames[nFilter]);

        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        uno::Reference< lang::XComponent > xComponent(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.text.TextDocument");

        // Check whether graphic exported well after it was swapped out
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);

        const OString sFailedMessage = OString("Failed on filter: ") + aFilterNames[nFilter];
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(2), xDraws->getCount());

        // First image
        uno::Reference<drawing::XShape> xImage(xDraws->getByIndex(0), uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet > XPropSet( xImage, uno::UNO_QUERY_THROW );
        // Check URL
        {
            OUString sURL;
            XPropSet->getPropertyValue("GraphicURL") >>= sURL;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), sURL != "vnd.sun.star.GraphicObject:00000000000000000000000000000000");
        }
        // Check size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610), xBitmap->getSize().Width );
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381), xBitmap->getSize().Height );
        }

        // Second Image
        xImage.set(xDraws->getByIndex(1), uno::UNO_QUERY);
        XPropSet.set( xImage, uno::UNO_QUERY_THROW );
        // Check URL
        {
            OUString sURL;
            XPropSet->getPropertyValue("GraphicURL") >>= sURL;
            CPPUNIT_ASSERT_MESSAGE(
                sFailedMessage.getStr(), sURL != "vnd.sun.star.GraphicObject:00000000000000000000000000000000");
        }
        // Check size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(900), xBitmap->getSize().Width );
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(600), xBitmap->getSize().Height );
        }
    }
}

void Test::testLinkedGraphicRT()
{
    const std::vector<OUString> aFilterNames = {
        "writer8",
//        "Rich Text Format",  Note: picture is there, but SwGrfNode is not found?
        "MS Word 97",
        "Office Open XML Text",
    };

    for( size_t nFilter = 0; nFilter < aFilterNames.size(); ++nFilter )
    {
        if (mxComponent.is())
            mxComponent->dispose();
        mxComponent = loadFromDesktop(getURLFromSrc("/sw/qa/extras/globalfilter/data/document_with_linked_graphic.odt"), "com.sun.star.text.TextDocument");

        const OString sFailedMessage = OString("Failed on filter: ")
            + OUStringToOString(aFilterNames[nFilter], RTL_TEXTENCODING_ASCII_US);


        // Export the document and import again for a check
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);

        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= aFilterNames[nFilter];

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
        for( sal_uLong nIndex = 0; nIndex < aNodes.Count(); ++nIndex)
        {
            if( aNodes[nIndex]->IsGrfNode() )
            {
                SwGrfNode* pGrfNode = aNodes[nIndex]->GetGrfNode();
                CPPUNIT_ASSERT(pGrfNode);
                // RT via DOCX makes linked graphic embedded?!
                if( aFilterNames[nFilter] != "Office Open XML Text" )
                {
                    CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pGrfNode->IsGrfLink());
                }
                const GraphicObject& rGraphicObj = pGrfNode->GetGrfObj(true);
                CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), !rGraphicObj.IsSwappedOut());
                CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), GRAPHIC_BITMAP, rGraphicObj.GetType());
                CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), static_cast<sal_uLong>(864900), rGraphicObj.GetSizeBytes());
                bImageFound = true;
            }
        }
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), bImageFound);
    }
}

void Test::testImageWithSpecialID()
{
    // Check how LO handles when the imported graphic's ID is different from that one
    // which is generated by LO.

    const char* aFilterNames[] = {
        "writer8",
        "Rich Text Format",
        "MS Word 97",
        "Office Open XML Text",
    };

    // Trigger swap out mechanism to test swapped state factor too.
    std::shared_ptr< comphelper::ConfigurationChanges > batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Cache::GraphicManager::TotalCacheSize::set(static_cast<sal_Int32>(1), batch);
    batch->commit();

    for( size_t nFilter = 0; nFilter < SAL_N_ELEMENTS(aFilterNames); ++nFilter )
    {
        if (mxComponent.is())
            mxComponent->dispose();
        mxComponent = loadFromDesktop(getURLFromSrc("/sw/qa/extras/globalfilter/data/images_with_special_IDs.odt"), "com.sun.star.text.TextDocument");

        // Export the document and import again for a check
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);

        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= OUString::createFromAscii(aFilterNames[nFilter]);

        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        uno::Reference< lang::XComponent > xComponent(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.text.TextDocument");

        // Check whether graphic exported well
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);

        const OString sFailedMessage = OString("Failed on filter: ") + aFilterNames[nFilter];
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(2), xDraws->getCount());

        uno::Reference<drawing::XShape> xImage = getShape(1);
        uno::Reference< beans::XPropertySet > XPropSet( xImage, uno::UNO_QUERY_THROW );
        // Check URL
        {
            OUString sURL;
            XPropSet->getPropertyValue("GraphicURL") >>= sURL;
            CPPUNIT_ASSERT(sURL != "vnd.sun.star.GraphicObject:00000000000000000000000000000000");
        }
        // Check size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT(xBitmap.is());
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(610), xBitmap->getSize().Width );
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(381), xBitmap->getSize().Height );
        }
        // Second Image
        xImage = getShape(2);
        XPropSet.set( xImage, uno::UNO_QUERY_THROW );
        // Check URL
        {
            OUString sURL;
            XPropSet->getPropertyValue("GraphicURL") >>= sURL;
            CPPUNIT_ASSERT(sURL != "vnd.sun.star.GraphicObject:00000000000000000000000000000000");
        }
        // Check size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT(xBitmap.is());
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(900), xBitmap->getSize().Width );
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(600), xBitmap->getSize().Height );
        }
    }
}

void Test::testGraphicShape()
{
    // There are two kind of images in Writer: 1) Writer specific handled by SwGrfNode and
    // 2) graphic shape handled by SdrGrafObj (e.g. after copy&paste from Impress).

    const char* aFilterNames[] = {
        "writer8",
        "Rich Text Format",
        "MS Word 97",
        "Office Open XML Text",
    };

    // Trigger swap out mechanism to test swapped state factor too.
    std::shared_ptr< comphelper::ConfigurationChanges > batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Cache::GraphicManager::TotalCacheSize::set(static_cast<sal_Int32>(1), batch);
    batch->commit();

    for( size_t nFilter = 0; nFilter < SAL_N_ELEMENTS(aFilterNames); ++nFilter )
    {
        if (mxComponent.is())
            mxComponent->dispose();
        mxComponent = loadFromDesktop(getURLFromSrc("/sw/qa/extras/globalfilter/data/graphic_shape.odt"), "com.sun.star.text.TextDocument");

        // Export the document and import again for a check
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);

        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= OUString::createFromAscii(aFilterNames[nFilter]);

        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        uno::Reference< lang::XComponent > xComponent(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.text.TextDocument");

        // Check whether graphic exported well
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);

        const OString sFailedMessage = OString("Failed on filter: ") + aFilterNames[nFilter];
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(2), xDraws->getCount());

        uno::Reference<drawing::XShape> xImage = getShape(1);
        uno::Reference< beans::XPropertySet > XPropSet( xImage, uno::UNO_QUERY_THROW );
        // First image is embedded
        // Check URL
        {
            OUString sURL;
            XPropSet->getPropertyValue("GraphicURL") >>= sURL;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), sURL != "vnd.sun.star.GraphicObject:00000000000000000000000000000000");
        }
        // Check size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610), xBitmap->getSize().Width );
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381), xBitmap->getSize().Height );
        }

        // MS filters make this kind of linked images broken !?
        if( OUString::createFromAscii(aFilterNames[nFilter]) != "writer8" )
            return;

        // Second image is a linked one
        xImage = getShape(2);
        XPropSet.set( xImage, uno::UNO_QUERY_THROW );
        // Check URL
        {
            OUString sURL;
            XPropSet->getPropertyValue("GraphicURL") >>= sURL;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), sURL.endsWith("linked_graphic.jpg"));
        }
        // Check size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(620), xBitmap->getSize().Width );
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(465), xBitmap->getSize().Height );
        }
    }
}

void Test::testCharHighlightBody()
{
    // MS Word has two kind of character backgrounds called character shading and highlighting
    // MS filters handle these attributes separately, but ODF export merges them into one background attribute

    const char* aFilterNames[] = {
        "writer8",
        "Rich Text Format",
        "MS Word 97",
        "Office Open XML Text",
    };

    for( size_t nFilter = 0; nFilter < SAL_N_ELEMENTS(aFilterNames); ++nFilter )
    {
        if (mxComponent.is())
            mxComponent->dispose();
        mxComponent = loadFromDesktop(getURLFromSrc("/sw/qa/extras/globalfilter/data/char_highlight.docx"),
                                      "com.sun.star.text.TextDocument");

        const OString sFailedMessage = OString("Failed on filter: ") + aFilterNames[nFilter];

        // Export the document and import again for a check
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);

        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= OUString::createFromAscii(aFilterNames[nFilter]);

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

            if( strcmp(aFilterNames[nFilter], "writer8") == 0 )
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
            if( strcmp(aFilterNames[nFilter], "writer8") == 0 )
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

void Test::testMSCharBackgroundEditing()
{
    // Simulate the editing process of imported MSO character background attributes
    // and check how export behaves.

    const char* aFilterNames[] = {
        "writer8",
        "Rich Text Format",
        "MS Word 97",
        "Office Open XML Text",
    };

    for( size_t nFilter = 0; nFilter < SAL_N_ELEMENTS(aFilterNames); ++nFilter )
    {
        if (mxComponent.is())
            mxComponent->dispose();

        mxComponent = loadFromDesktop(getURLFromSrc("/sw/qa/extras/globalfilter/data/char_background_editing.docx"),
                                      "com.sun.star.text.TextDocument");

        const OString sFailedMessage = OString("Failed on filter: ") + aFilterNames[nFilter];

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
            xRun->setPropertyValue("CharBackColor", uno::makeAny(static_cast<sal_Int32>(nBackColor)));
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
                    rProp.Value = uno::makeAny(false);
                }
            }
            xRun->setPropertyValue("CharInteropGrabBag", uno::makeAny(aGrabBag));
        }

        SvtFilterOptions& rOpt = SvtFilterOptions::Get();
        rOpt.SetCharBackground2Highlighting();

        // Export the document and import again for a check
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);

        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= OUString::createFromAscii(aFilterNames[nFilter]);

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
            if( strcmp(aFilterNames[nFilter], "writer8") == 0 )
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(COL_TRANSPARENT), getProperty<sal_Int32>(xRun,"CharHighlight"));
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(nBackColor), getProperty<sal_Int32>(xRun,"CharBackColor"));
            }
            else
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(nBackColor), getProperty<sal_Int32>(xRun,"CharHighlight"));
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(COL_TRANSPARENT), getProperty<sal_Int32>(xRun,"CharBackColor"));
            }
        }
    }
}

void Test::testCharBackgroundToHighlighting()
{
    // MSO highlighting has less kind of values so let's see how LO character background is converted
    // to these values

    const char* aFilterNames[] = {
        "Rich Text Format",
        "MS Word 97",
        "Office Open XML Text",
    };

    for( size_t nFilter = 0; nFilter < SAL_N_ELEMENTS(aFilterNames); ++nFilter )
    {
        if (mxComponent.is())
            mxComponent->dispose();
        mxComponent = loadFromDesktop(getURLFromSrc("/sw/qa/extras/globalfilter/data/char_background.odt"),
                                      "com.sun.star.text.TextDocument");

        OString sFailedMessage = OString("Failed on filter: ") + aFilterNames[nFilter];


        SvtFilterOptions& rOpt = SvtFilterOptions::Get();
        rOpt.SetCharBackground2Highlighting();

        // Export the document and import again for a check
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);

        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= OUString::createFromAscii(aFilterNames[nFilter]);

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

#if !defined(WNT)
void Test::testSkipImages()
{
    // Check how LO skips image loading (but not texts of textboxes and custom shapes)
    // during DOC and DOCX import, using the "SkipImages" FilterOptions.

    const char* aFilterNames[][2] = {
        { "/sw/qa/extras/globalfilter/data/skipimages.doc", nullptr },
        { "/sw/qa/extras/globalfilter/data/skipimages.doc", "SkipImages" },
        { "/sw/qa/extras/globalfilter/data/skipimages.docx", nullptr },
        { "/sw/qa/extras/globalfilter/data/skipimages.docx", "SkipImages" }
    };

    for( size_t nFilter = 0; nFilter < SAL_N_ELEMENTS(aFilterNames); ++nFilter )
    {
        bool bSkipImages = aFilterNames[nFilter][1] != nullptr;
        OString sFailedMessage = OString("Failed on filter: ") + aFilterNames[nFilter][0];

        if (mxComponent.is())
            mxComponent->dispose();

        if (bSkipImages)
        {
            // FilterOptions parameter
            uno::Sequence<beans::PropertyValue> args(1);
            args[0].Name = "FilterOptions";
            args[0].Handle = -1;
            args[0].Value <<= OUString::createFromAscii(aFilterNames[nFilter][1]);
            args[0].State = beans::PropertyState_DIRECT_VALUE;
            mxComponent = loadFromDesktop(getURLFromSrc(aFilterNames[nFilter][0]), "com.sun.star.text.TextDocument", args);
            sFailedMessage = sFailedMessage + " - " + aFilterNames[nFilter][1];
        } else
            mxComponent = loadFromDesktop(getURLFromSrc(aFilterNames[nFilter][0]), "com.sun.star.text.TextDocument");

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

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
