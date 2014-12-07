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

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase() {}

    void testSwappedOutImageExport();
    void testLinkedGraphicRT();
    void testImageWithSpecialID();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testSwappedOutImageExport);
    CPPUNIT_TEST(testLinkedGraphicRT);
    CPPUNIT_TEST(testImageWithSpecialID);
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
        boost::shared_ptr< comphelper::ConfigurationChanges > batch(comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Cache::GraphicManager::TotalCacheSize::set(sal_Int32(1), batch);
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
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), sURL != OUString("vnd.sun.star.GraphicObject:00000000000000000000000000000000"));
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
                sFailedMessage.getStr(), sURL != OUString("vnd.sun.star.GraphicObject:00000000000000000000000000000000"));
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

        SwXTextDocument* pTxtDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pTxtDoc);
        SwDoc* pDoc = pTxtDoc->GetDocShell()->GetDoc();
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pDoc);
        SwNodes& aNodes = pDoc->GetNodes();

        // Find the image
        bool bImageFound = false;
        for( sal_uLong nIndex = 0; nIndex < aNodes.Count(); ++nIndex)
        {
            if( aNodes[nIndex]->IsGrfNode() )
            {
                SwGrfNode* pGrfNode = aNodes[nIndex]->GetGrfNode();
                // RT via DOCX makes linked graphic embedded?!
                if( aFilterNames[nFilter] != "Office Open XML Text" )
                {
                    CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pGrfNode->IsGrfLink());
                }
                const GraphicObject& rGraphicObj = pGrfNode->GetGrfObj(true);
                CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), !rGraphicObj.IsSwappedOut());
                CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), GRAPHIC_BITMAP, rGraphicObj.GetType());
                CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), sal_uLong(864900), rGraphicObj.GetSizeBytes());
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

    const OUString aFilterNames[] = {
        "writer8",
        "Rich Text Format",
        "MS Word 97",
        "Office Open XML Text",
    };

    // Trigger swap out mechanism to test swapped state factor too.
    boost::shared_ptr< comphelper::ConfigurationChanges > batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Cache::GraphicManager::TotalCacheSize::set(sal_Int32(1), batch);
    batch->commit();

    for( size_t nFilter = 0; nFilter < SAL_N_ELEMENTS(aFilterNames); ++nFilter )
    {
        if (mxComponent.is())
            mxComponent->dispose();
        mxComponent = loadFromDesktop(getURLFromSrc("/sw/qa/extras/globalfilter/data/images_with_special_IDs.odt"), "com.sun.star.text.TextDocument");

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

        // Check whether graphic exported well
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);

        const OString sFailedMessage = OString("Failed on filter: ")
            + OUStringToOString(aFilterNames[nFilter], RTL_TEXTENCODING_ASCII_US);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(2), xDraws->getCount());

        uno::Reference<drawing::XShape> xImage = getShape(1);
        uno::Reference< beans::XPropertySet > XPropSet( xImage, uno::UNO_QUERY_THROW );
        // Check URL
        {
            OUString sURL;
            XPropSet->getPropertyValue("GraphicURL") >>= sURL;
            CPPUNIT_ASSERT(sURL != OUString("vnd.sun.star.GraphicObject:00000000000000000000000000000000"));
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
            CPPUNIT_ASSERT(sURL != OUString("vnd.sun.star.GraphicObject:00000000000000000000000000000000"));
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

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
