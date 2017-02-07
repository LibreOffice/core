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

#include <test/xmltesttools.hxx>

#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

xmlDocPtr XPathHelper::parseExport(ScDocShell* pShell, uno::Reference<lang::XMultiServiceFactory> xSFactory, const OUString& rFile, sal_Int32 nFormat)
{
    std::shared_ptr<utl::TempFile> pTempFile = ScBootstrapFixture::exportTo(pShell, nFormat);

    return parseExport(pTempFile, xSFactory, rFile);
}

xmlDocPtr XPathHelper::parseExport(std::shared_ptr<utl::TempFile> const & pTempFile, uno::Reference<lang::XMultiServiceFactory> const & xSFactory, const OUString& rFile)
{
    // Read the XML stream we're interested in.
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(xSFactory), pTempFile->GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName(rFile), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xInputStream.is());
    std::shared_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));

    return XmlTestTools::parseXmlStream(pStream.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
