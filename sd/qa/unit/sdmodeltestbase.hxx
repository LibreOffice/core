/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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
#include <com/sun/star/text/XTextField.hpp>

using namespace ::com::sun::star;

class SdModelTestBase : public UnoApiXmlTest
{
public:
    SdModelTestBase(OUString path)
        : UnoApiXmlTest(path)
    {
    }

    void createSdImpressDoc(const char* pName = nullptr, const char* pPassword = nullptr)
    {
        if (!pName)
            load(u"private:factory/simpress"_ustr);
        else
            loadFromFile(OUString::createFromAscii(pName), pPassword);

        uno::Reference<lang::XServiceInfo> xServiceInfo(mxComponent, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(
            xServiceInfo->supportsService(u"com.sun.star.presentation.PresentationDocument"_ustr));

        CPPUNIT_ASSERT(!getSdDocShell()->GetMedium()->GetWarningError());
    }

    void createSdDrawDoc(const char* pName = nullptr, const char* pPassword = nullptr)
    {
        if (!pName)
            load(u"private:factory/sdraw"_ustr);
        else
            loadFromFile(OUString::createFromAscii(pName), pPassword);

        uno::Reference<lang::XServiceInfo> xServiceInfo(mxComponent, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.drawing.DrawingDocument"_ustr));

        CPPUNIT_ASSERT(!getSdDocShell()->GetMedium()->GetWarningError());
    }

    sd::DrawDocShell* getSdDocShell()
    {
        SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pImpressDocument);
        return pImpressDocument->GetDocShell();
    }

    uno::Reference<drawing::XDrawPage> getPage(int nPage)
    {
        uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xDoc.is());
        uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(nPage),
                                                 uno::UNO_QUERY_THROW);
        return xPage;
    }

    uno::Reference<beans::XPropertySet> getShapeFromPage(int nShape, int nPage)
    {
        uno::Reference<drawing::XDrawPage> xPage(getPage(nPage));
        uno::Reference<beans::XPropertySet> xShape(getShape(nShape, xPage));
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

    uno::Reference<beans::XPropertySet> getShape(int nShape,
                                                 uno::Reference<drawing::XDrawPage> const& xPage)
    {
        uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(nShape), uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE("Failed to load shape", xShape.is());
        return xShape;
    }

    uno::Reference<text::XTextRange>
    getParagraphFromShape(int nPara, uno::Reference<beans::XPropertySet> const& xShape)
    {
        uno::Reference<text::XText> xText
            = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
        CPPUNIT_ASSERT_MESSAGE("Not a text shape", xText.is());

        uno::Reference<container::XEnumerationAccess> paraEnumAccess(xText, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> paraEnum(paraEnumAccess->createEnumeration());

        for (int i = 0; i < nPara; ++i)
            paraEnum->nextElement();

        uno::Reference<text::XTextRange> xParagraph(paraEnum->nextElement(), uno::UNO_QUERY_THROW);

        return xParagraph;
    }

    uno::Reference<text::XTextRange>
    getRunFromParagraph(int nRun, uno::Reference<text::XTextRange> const& xParagraph)
    {
        uno::Reference<container::XEnumerationAccess> runEnumAccess(xParagraph, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> runEnum = runEnumAccess->createEnumeration();

        for (int i = 0; i < nRun; ++i)
            runEnum->nextElement();

        uno::Reference<text::XTextRange> xRun(runEnum->nextElement(), uno::UNO_QUERY);

        return xRun;
    }

    uno::Reference<text::XTextField> getTextFieldFromPage(int nRun, int nPara, int nShape,
                                                          int nPage)
    {
        // get TextShape 1 from the first page
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(nShape, nPage));

        // Get first paragraph
        uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(nPara, xShape));

        // first chunk of text
        uno::Reference<text::XTextRange> xRun(getRunFromParagraph(nRun, xParagraph));

        uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

        uno::Reference<text::XTextField> xField;
        xPropSet->getPropertyValue(u"TextField"_ustr) >>= xField;
        return xField;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
