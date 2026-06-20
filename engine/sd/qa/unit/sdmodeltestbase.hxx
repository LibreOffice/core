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
#include <COKit/COKitEnums.h>
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

        css::uno::Reference<css::lang::XServiceInfo> xServiceInfo(mxComponent,
                                                                  css::uno::UNO_QUERY_THROW);
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

        css::uno::Reference<css::lang::XServiceInfo> xServiceInfo(mxComponent,
                                                                  css::uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.drawing.DrawingDocument"_ustr));

        CPPUNIT_ASSERT(!getSdDocShell()->GetMedium()->GetWarningError());
    }

    sd::DrawDocShell* getSdDocShell()
    {
        SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pImpressDocument);
        return pImpressDocument->GetDocShell();
    }

    css::uno::Reference<css::drawing::XDrawPage> getPage(int nPage)
    {
        css::uno::Reference<css::drawing::XDrawPagesSupplier> xDoc(mxComponent,
                                                                   css::uno::UNO_QUERY);
        CPPUNIT_ASSERT(xDoc.is());
        css::uno::Reference<css::drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(nPage),
                                                           css::uno::UNO_QUERY_THROW);
        return xPage;
    }

    css::uno::Reference<css::beans::XPropertySet> getShapeFromPage(int nShape, int nPage)
    {
        css::uno::Reference<css::drawing::XDrawPage> xPage(getPage(nPage));
        css::uno::Reference<css::beans::XPropertySet> xShape(getShape(nShape, xPage));
        CPPUNIT_ASSERT_MESSAGE("Failed to load shape", xShape.is());

        return xShape;
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

    css::uno::Reference<css::beans::XPropertySet>
    getShape(int nShape, css::uno::Reference<css::drawing::XDrawPage> const& xPage)
    {
        css::uno::Reference<css::beans::XPropertySet> xShape(xPage->getByIndex(nShape),
                                                             css::uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE("Failed to load shape", xShape.is());
        return xShape;
    }

    css::uno::Reference<css::text::XTextRange>
    getParagraphFromShape(int nPara, css::uno::Reference<css::beans::XPropertySet> const& xShape)
    {
        css::uno::Reference<css::text::XText> xText
            = css::uno::Reference<css::text::XTextRange>(xShape, css::uno::UNO_QUERY_THROW)
                  ->getText();
        CPPUNIT_ASSERT_MESSAGE("Not a text shape", xText.is());

        css::uno::Reference<css::container::XEnumerationAccess> paraEnumAccess(xText,
                                                                               css::uno::UNO_QUERY);
        css::uno::Reference<css::container::XEnumeration> paraEnum(
            paraEnumAccess->createEnumeration());

        for (int i = 0; i < nPara; ++i)
            paraEnum->nextElement();

        css::uno::Reference<css::text::XTextRange> xParagraph(paraEnum->nextElement(),
                                                              css::uno::UNO_QUERY_THROW);

        return xParagraph;
    }

    css::uno::Reference<css::text::XTextRange>
    getRunFromParagraph(int nRun, css::uno::Reference<css::text::XTextRange> const& xParagraph)
    {
        css::uno::Reference<css::container::XEnumerationAccess> runEnumAccess(xParagraph,
                                                                              css::uno::UNO_QUERY);
        css::uno::Reference<css::container::XEnumeration> runEnum
            = runEnumAccess->createEnumeration();

        for (int i = 0; i < nRun; ++i)
            runEnum->nextElement();

        css::uno::Reference<css::text::XTextRange> xRun(runEnum->nextElement(),
                                                        css::uno::UNO_QUERY);

        return xRun;
    }

    css::uno::Reference<css::text::XTextField> getTextFieldFromPage(int nRun, int nPara, int nShape,
                                                                    int nPage)
    {
        // get TextShape 1 from the first page
        css::uno::Reference<css::beans::XPropertySet> xShape(getShapeFromPage(nShape, nPage));

        // Get first paragraph
        css::uno::Reference<css::text::XTextRange> xParagraph(getParagraphFromShape(nPara, xShape));

        // first chunk of text
        css::uno::Reference<css::text::XTextRange> xRun(getRunFromParagraph(nRun, xParagraph));

        css::uno::Reference<css::beans::XPropertySet> xPropSet(xRun, css::uno::UNO_QUERY_THROW);

        css::uno::Reference<css::text::XTextField> xField;
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
            rImpressDocument->postKeyEvent(KIT_KEYEVENT_KEYINPUT, c, 0);
            rImpressDocument->postKeyEvent(KIT_KEYEVENT_KEYUP, c, 0);
            Scheduler::ProcessEventsToIdle();
        }
    }

    void typeKey(SdXImpressDocument* rImpressDocument, const sal_uInt16 nKey)
    {
        rImpressDocument->postKeyEvent(KIT_KEYEVENT_KEYINPUT, 0, nKey);
        rImpressDocument->postKeyEvent(KIT_KEYEVENT_KEYUP, 0, nKey);
        Scheduler::ProcessEventsToIdle();
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
