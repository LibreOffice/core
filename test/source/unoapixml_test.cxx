/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>

#include <o3tl/string_view.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <utility>

using namespace css;
using namespace css::uno;

UnoApiXmlTest::UnoApiXmlTest(OUString path)
    : UnoApiTest(path)
{
}

xmlDocUniquePtr UnoApiXmlTest::parseExport(OUString const& rStreamName)
{
    if (rStreamName.indexOf("chart") == -1)
    {
        std::unique_ptr<SvStream> const pStream(
            parseExportStream(maTempFile.GetURL(), rStreamName));
        xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
        return pXmlDoc;
    }
    else
    {
        // we have a static counter that increments chart export file names. So
        // only the first exported file gets the file name chart1.xml and all the
        // following charts are incremented by one even if they are in a different file.
        OUString aStreamName(rStreamName);
        uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
            = packages::zip::ZipFileAccess::createWithURL(
                comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());
        const uno::Sequence<OUString> aNames = xNameAccess->getElementNames();

        if (aStreamName.endsWith(".xml"))
            // remove "1.xml"
            aStreamName = aStreamName.copy(0, aStreamName.getLength() - 5);

        uno::Reference<io::XInputStream> xInputStream;

        int nCount = 0;
        for (const auto& rElementName : aNames)
        {
            if (o3tl::starts_with(rElementName, aStreamName))
            {
                xInputStream.set(xNameAccess->getByName(rElementName), uno::UNO_QUERY);
                ++nCount;
                assert(nCount == 1 && "Only use one chart per document");
            }
        }

        CPPUNIT_ASSERT(xInputStream.is());
        std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));

        return parseXmlStream(pStream.get());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
