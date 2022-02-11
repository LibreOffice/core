/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xpath.hxx"

#include <cppunit/TestAssert.h>
#include "qahelper.hxx"

#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/processfactory.hxx>

#include <test/xmltesttools.hxx>

#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

xmlDocUniquePtr XPathHelper::parseExport2(ScBootstrapFixture & rFixture, ScDocShell& rShell, uno::Reference<lang::XMultiServiceFactory> const & xSFactory, const OUString& rFile, sal_Int32 nFormat)
{
    std::shared_ptr<utl::TempFile> pTempFile = rFixture.exportTo(rShell, nFormat);

    return parseExport(pTempFile, xSFactory, rFile);
}

std::unique_ptr<SvStream> XPathHelper::parseExportStream(std::shared_ptr<utl::TempFile> const & pTempFile, uno::Reference<lang::XMultiServiceFactory> const & xSFactory, const OUString& rFile)
{
    // Read the XML stream we're interested in.
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(xSFactory), pTempFile->GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName(rFile), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xInputStream.is());
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    return pStream;
}

xmlDocUniquePtr XPathHelper::parseExport(std::shared_ptr<utl::TempFile> const & pTempFile, uno::Reference<lang::XMultiServiceFactory> const & xSFactory, const OUString& rFile)
{
    std::unique_ptr<SvStream> pStream = parseExportStream(pTempFile, xSFactory, rFile);
    return XmlTestTools::parseXmlStream(pStream.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
