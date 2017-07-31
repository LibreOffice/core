/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/processfactory.hxx>
#include <test/unoapi_test.hxx>

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

    uno::Reference< XOfficeDatabaseDocument >
        getDocumentForFileName(const OUString &sFileName);

    uno::Reference< XConnection >
        getConnectionForDocument(
            uno::Reference< XOfficeDatabaseDocument > const & xDocument);
};

uno::Reference< XOfficeDatabaseDocument >
    DBTestBase::getDocumentForFileName(const OUString &sFileName)
{
    OUString sFilePath;
    createFileURL(sFileName, sFilePath);
    uno::Reference< lang::XComponent > xComponent (loadFromDesktop(sFilePath));
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
