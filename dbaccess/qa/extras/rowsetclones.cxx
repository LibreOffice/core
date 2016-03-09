/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>
#include <test/unoapi_test.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/intitem.hxx>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;

class RowSetClones : public UnoApiTest
{
public:
    RowSetClones();

    void test();

    CPPUNIT_TEST_SUITE(RowSetClones);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

};


RowSetClones::RowSetClones()
    : UnoApiTest("")
{
}

void RowSetClones::test()
{
    const OUString sFilePath(m_directories.getURLFromWorkdir("CppunitTest/RowSetClones.odb"));

    uno::Reference< lang::XComponent > xComponent (loadFromDesktop(sFilePath));
    CPPUNIT_ASSERT(xComponent.is());

    uno::Reference< XOfficeDatabaseDocument > xDocument(xComponent, UNO_QUERY);
    CPPUNIT_ASSERT(xDocument.is());

    uno::Reference< XDataSource > xDataSource = xDocument->getDataSource();
    CPPUNIT_ASSERT(xDataSource.is());

    uno::Reference< XConnection > xConnection = xDataSource->getConnection("","");
    CPPUNIT_ASSERT(xConnection.is());

    uno::Reference< XRowSet > xRowSet (getMultiServiceFactory()->createInstance("com.sun.star.sdb.RowSet" ), UNO_QUERY);
    CPPUNIT_ASSERT(xRowSet.is());
    uno::Reference< XPropertySet > rowSetProperties ( xRowSet, UNO_QUERY );
    CPPUNIT_ASSERT(rowSetProperties.is());
    rowSetProperties->setPropertyValue("Command", Any(OUString("SELECT * FROM Assets ORDER BY AssetID")));
    rowSetProperties->setPropertyValue("CommandType", Any(CommandType::COMMAND));
    rowSetProperties->setPropertyValue("ActiveConnection", Any(xConnection));

    xRowSet->execute();
    uno::Reference< XResultSet > xResultSet(xRowSet, UNO_QUERY);
    CPPUNIT_ASSERT(xResultSet.is());
    // always starts at BeforeFirst position
    CPPUNIT_ASSERT(xResultSet->isBeforeFirst());
    CPPUNIT_ASSERT(xResultSet->next());
    CPPUNIT_ASSERT(xResultSet->isFirst());
    CPPUNIT_ASSERT(xResultSet->getRow() == 1);

    uno::Reference< XRow > xRow(xResultSet, UNO_QUERY);
    CPPUNIT_ASSERT(xRow.is());
    CPPUNIT_ASSERT(xRow->getInt(1) == 1);

    uno::Reference< XResultSetAccess > xResultSetAccess(xResultSet, UNO_QUERY);
    CPPUNIT_ASSERT(xResultSetAccess.is());
    uno::Reference< XResultSet > xResultSetClone = xResultSetAccess->createResultSet();
    CPPUNIT_ASSERT(xResultSetClone.is());

    uno::Reference< XRow > xRowClone(xResultSetClone, UNO_QUERY);
    CPPUNIT_ASSERT(xRowClone.is());

    // the clone starts at same position as what it is cloned from,
    // and does not move its source.
    CPPUNIT_ASSERT(xResultSetClone->isFirst());
    CPPUNIT_ASSERT(xResultSet->isFirst());
    CPPUNIT_ASSERT(xResultSet->getRow() == 1);
    CPPUNIT_ASSERT(xResultSetClone->getRow() == 1);
    CPPUNIT_ASSERT(xRow->getInt(1) == 1);
    CPPUNIT_ASSERT(xRowClone->getInt(1) == 1);

    // if we move the source, the clone does not move
    CPPUNIT_ASSERT(xResultSet->next());
    CPPUNIT_ASSERT(xResultSetClone->isFirst());
    CPPUNIT_ASSERT(xResultSet->getRow() == 2);
    CPPUNIT_ASSERT(xResultSetClone->getRow() == 1);
    CPPUNIT_ASSERT(xRow->getInt(1) == 2);
    CPPUNIT_ASSERT(xRowClone->getInt(1) == 1);

    CPPUNIT_ASSERT(xResultSet->last());
    CPPUNIT_ASSERT(xResultSet->isLast());
    CPPUNIT_ASSERT(xResultSetClone->isFirst());
    CPPUNIT_ASSERT(xRowClone->getInt(1) == 1);

    // and the other way round
    CPPUNIT_ASSERT(xResultSet->first());
    CPPUNIT_ASSERT(xResultSetClone->next());
    CPPUNIT_ASSERT(xResultSet->isFirst());
    CPPUNIT_ASSERT(xResultSetClone->getRow() == 2);
    CPPUNIT_ASSERT(xRowClone->getInt(1) == 2);
    CPPUNIT_ASSERT(xRow->getInt(1) == 1);

    CPPUNIT_ASSERT(xResultSetClone->last());
    CPPUNIT_ASSERT(xResultSetClone->isLast());
    CPPUNIT_ASSERT(xResultSet->isFirst());
    CPPUNIT_ASSERT(xRow->getInt(1) == 1);

    closeDocument(uno::Reference<lang::XComponent>(xDocument, uno::UNO_QUERY));
}

CPPUNIT_TEST_SUITE_REGISTRATION(RowSetClones);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
