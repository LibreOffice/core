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

#include <cppunit/TestAssert.h>

#include <test/unoapi_test.hxx>
#include <unotools/tempfile.hxx>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

class DBTestBase
    : public UnoApiTest
{
public:
    DBTestBase() : UnoApiTest("dbaccess/qa/unit/data") {};

    utl::TempFile createTempCopy(std::u16string_view pathname);

    uno::Reference< XOfficeDatabaseDocument >
        getDocumentForFileName(std::u16string_view sFileName);

    uno::Reference<XOfficeDatabaseDocument> getDocumentForUrl(OUString const & url);

    uno::Reference< XConnection >
        getConnectionForDocument(
            uno::Reference< XOfficeDatabaseDocument > const & xDocument);
};

utl::TempFile DBTestBase::createTempCopy(std::u16string_view pathname) {
    OUString url;
    createFileURL(pathname, url);
    utl::TempFile tmp;
    tmp.EnableKillingFile();
    auto const e = osl::File::copy(url, tmp.GetURL());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        (OString(
            "<" + OUStringToOString(url, RTL_TEXTENCODING_UTF8) + "> -> <"
            + OUStringToOString(tmp.GetURL(), RTL_TEXTENCODING_UTF8) + ">")
         .getStr()),
        osl::FileBase::E_None, e);
    return tmp;
}

uno::Reference< XOfficeDatabaseDocument >
    DBTestBase::getDocumentForFileName(std::u16string_view sFileName)
{
    OUString sFilePath;
    createFileURL(sFileName, sFilePath);
    return getDocumentForUrl(sFilePath);
}

uno::Reference<XOfficeDatabaseDocument> DBTestBase::getDocumentForUrl(OUString const & url) {
    uno::Reference< lang::XComponent > xComponent (loadFromDesktop(url));
    CPPUNIT_ASSERT(xComponent.is());

    uno::Reference< XOfficeDatabaseDocument > xDocument(xComponent, UNO_QUERY);
    CPPUNIT_ASSERT(xDocument.is());

    return xDocument;
}

uno::Reference< XConnection > DBTestBase::getConnectionForDocument(
    uno::Reference< XOfficeDatabaseDocument > const & xDocument)
{
    uno::Reference< XDataSource > xDataSource = xDocument->getDataSource();
    CPPUNIT_ASSERT(xDataSource.is());

    uno::Reference< XConnection > xConnection = xDataSource->getConnection("","");
    CPPUNIT_ASSERT(xConnection.is());

    return xConnection;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
