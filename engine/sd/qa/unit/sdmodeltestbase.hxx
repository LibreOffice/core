/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <string_view>

#include <test/unoapixml_test.hxx>
#include <test/xmldiff.hxx>

#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <GraphicDocShell.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/color.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/processfactory.hxx>
#include <o3tl/safeint.hxx>
#include <rtl/strbuf.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svl/itemset.hxx>
#include <unomodel.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <drawinglayer/XShapeDumper.hxx>
#include <COKit/COKit.hxx>
#include <vcl/scheduler.hxx>
#include <com/sun/star/text/XTextField.hpp>

class SdModelTestBase : public UnoApiXmlTest
{
public:
    SdModelTestBase(const OUString& path)
        : UnoApiXmlTest(path)
    {
    }

    void createSdImpressDoc(const char* pName = nullptr, const char* pPassword = nullptr)
    {
        if (!pName)
            loadFromURL(u"private:factory/simpress"_ustr);
        else
            loadFromFile(OUString::createFromAscii(pName), pPassword);

        cpo::uno::Reference<css::lang::XServiceInfo> xServiceInfo(mxComponent,
                                                                  cpo::uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(
            xServiceInfo->supportsService(u"com.sun.star.presentation.PresentationDocument"_ustr));

        CPPUNIT_ASSERT(!getSdDocShell()->GetMedium()->GetWarningError());
    }

    void createSdDrawDoc(const char* pName = nullptr, const char* pPassword = nullptr)
    {
        if (!pName)
            loadFromURL(u"private:factory/sdraw"_ustr);
        else
            loadFromFile(OUString::createFromAscii(pName), pPassword);

        cpo::uno::Reference<css::lang::XServiceInfo> xServiceInfo(mxComponent,
                                                                  cpo::uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.drawing.DrawingDocument"_ustr));

        CPPUNIT_ASSERT(!getSdDocShell()->GetMedium()->GetWarningError());
    }

    sd::DrawDocShell* getSdDocShell()
    {
        SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pImpressDocument);
        return pImpressDocument->GetDocShell();
    }

    cpo::uno::Reference<css::drawing::XDrawPage> getPage(int nPage)
    {
        cpo::uno::Reference<css::drawing::XDrawPagesSupplier> xDoc(mxComponent,
                                                                   cpo::uno::UNO_QUERY);
        CPPUNIT_ASSERT(xDoc.is());
        cpo::uno::Reference<css::drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(nPage),
                                                           cpo::uno::UNO_QUERY_THROW);
        return xPage;
    }

    cpo::uno::Reference<css::beans::XPropertySet> getShapeFromPage(int nShape, int nPage)
    {
        cpo::uno::Reference<css::drawing::XDrawPage> xPage(getPage(nPage));
        cpo::uno::Reference<css::beans::XPropertySet> xShape(getShape(nShape, xPage));
        CPPUNIT_ASSERT_MESSAGE("Failed to load shape", xShape.is());

        return xShape;
    }

    /** The exported slide layout carrying this name, whatever number its part got

        A layout's part number is its position in the master's list, and that moves as soon as a
        document gains, loses or reorders one; the name says what the layout is for.
    */
    xmlDocUniquePtr parseExportedLayoutNamed(std::u16string_view aName)
    {
        xmlDocUniquePtr pMaster = parseExport(u"ppt/slideMasters/slideMaster1.xml"_ustr);
        const int nLayouts
            = countXPathNodes(pMaster, "/p:sldMaster/p:sldLayoutIdLst/p:sldLayoutId");
        xmlDocUniquePtr pFound;
        for (int i = 1; i <= nLayouts && !pFound; ++i)
        {
            xmlDocUniquePtr pLayout
                = parseExport("ppt/slideLayouts/slideLayout" + OUString::number(i) + ".xml");
            if (getXPath(pLayout, "/p:sldLayout/p:cSld", "name") == aName)
                pFound = std::move(pLayout);
        }
        CPPUNIT_ASSERT_MESSAGE("no exported slide layout of that name", pFound);
        return pFound;
    }

    // very confusing ... UNO index-based access to pages is 0-based. This one is 1-based
    const SdrPage* GetPage(int nPage)
    {
        SdXImpressDocument* pXImpressDocument
            = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pXImpressDocument);
        SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
        CPPUNIT_ASSERT_MESSAGE("no document", pDoc != nullptr);

        const SdrPage* pPage = pDoc->GetPage(nPage);
        CPPUNIT_ASSERT_MESSAGE("no page", pPage != nullptr);
        return pPage;
    }

    cpo::uno::Reference<css::beans::XPropertySet>
    getShape(int nShape, cpo::uno::Reference<css::drawing::XDrawPage> const& xPage)
    {
        cpo::uno::Reference<css::beans::XPropertySet> xShape(xPage->getByIndex(nShape),
                                                             cpo::uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE("Failed to load shape", xShape.is());
        return xShape;
    }

    cpo::uno::Reference<css::text::XTextRange>
    getParagraphFromShape(int nPara, cpo::uno::Reference<css::beans::XPropertySet> const& xShape)
    {
        cpo::uno::Reference<css::text::XText> xText
            = cpo::uno::Reference<css::text::XTextRange>(xShape, cpo::uno::UNO_QUERY_THROW)
                  ->getText();
        CPPUNIT_ASSERT_MESSAGE("Not a text shape", xText.is());

        cpo::uno::Reference<css::container::XEnumerationAccess> paraEnumAccess(xText,
                                                                               cpo::uno::UNO_QUERY);
        cpo::uno::Reference<css::container::XEnumeration> paraEnum(
            paraEnumAccess->createEnumeration());

        for (int i = 0; i < nPara; ++i)
            paraEnum->nextElement();

        cpo::uno::Reference<css::text::XTextRange> xParagraph(paraEnum->nextElement(),
                                                              cpo::uno::UNO_QUERY_THROW);

        return xParagraph;
    }

    cpo::uno::Reference<css::text::XTextRange>
    getRunFromParagraph(int nRun, cpo::uno::Reference<css::text::XTextRange> const& xParagraph)
    {
        cpo::uno::Reference<css::container::XEnumerationAccess> runEnumAccess(xParagraph,
                                                                              cpo::uno::UNO_QUERY);
        cpo::uno::Reference<css::container::XEnumeration> runEnum
            = runEnumAccess->createEnumeration();

        for (int i = 0; i < nRun; ++i)
            runEnum->nextElement();

        cpo::uno::Reference<css::text::XTextRange> xRun(runEnum->nextElement(),
                                                        cpo::uno::UNO_QUERY);

        return xRun;
    }

    cpo::uno::Reference<css::text::XTextField> getTextFieldFromPage(int nRun, int nPara, int nShape,
                                                                    int nPage)
    {
        // get TextShape 1 from the first page
        cpo::uno::Reference<css::beans::XPropertySet> xShape(getShapeFromPage(nShape, nPage));

        // Get first paragraph
        cpo::uno::Reference<css::text::XTextRange> xParagraph(getParagraphFromShape(nPara, xShape));

        // first chunk of text
        cpo::uno::Reference<css::text::XTextRange> xRun(getRunFromParagraph(nRun, xParagraph));

        cpo::uno::Reference<css::beans::XPropertySet> xPropSet(xRun, cpo::uno::UNO_QUERY_THROW);

        cpo::uno::Reference<css::text::XTextField> xField;
        xPropSet->getPropertyValue(u"TextField"_ustr) >>= xField;
        return xField;
    }

    xmlDocUniquePtr parseLayout() const
    {
        SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
        CPPUNIT_ASSERT(pModel);
        SfxObjectShell* pShell = pModel->GetObjectShell();
        std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
        MetafileXmlDump dumper;

        xmlDocUniquePtr pXmlDoc = XmlTestTools::dumpAndParse(dumper, *xMetaFile);
        CPPUNIT_ASSERT(pXmlDoc);

        return pXmlDoc;
    }

    void typeString(SdXImpressDocument* rImpressDocument, std::u16string_view rStr)
    {
        for (const char16_t c : rStr)
        {
            rImpressDocument->postKeyEvent(COKitKeyEventType::DOWN, c, 0);
            rImpressDocument->postKeyEvent(COKitKeyEventType::UP, c, 0);
            Scheduler::ProcessEventsToIdle();
        }
    }

    void typeKey(SdXImpressDocument* rImpressDocument, const sal_uInt16 nKey)
    {
        rImpressDocument->postKeyEvent(COKitKeyEventType::DOWN, 0, nKey);
        rImpressDocument->postKeyEvent(COKitKeyEventType::UP, 0, nKey);
        Scheduler::ProcessEventsToIdle();
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
