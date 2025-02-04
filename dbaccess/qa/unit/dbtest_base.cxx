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

#include <test/unoapi_test.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/util/XCloseable.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

class DBTestBase
    : public UnoApiTest
{
public:
    DBTestBase() : UnoApiTest(u"dbaccess/qa/unit/data"_ustr) {};

    uno::Reference<XOfficeDatabaseDocument> getDocumentForUrl(OUString const & url);

    uno::Reference< XConnection >
        getConnectionForDocument(
            uno::Reference< XOfficeDatabaseDocument > const & xDocument);

    void createDBDocument(const OUString& rDriverURL);
    void createTables(const Reference< XConnection >& xConnection);
    void createQueries(const Reference< XDataSource >& xDataSource);
    void createQuery(const OUString& sQuery, bool bEscapeProcessing,
            const OUString& sQueryName, const Reference<XDataSource> & xDataSource);

    virtual void tearDown() override;
};

uno::Reference<XOfficeDatabaseDocument> DBTestBase::getDocumentForUrl(OUString const & url) {
    loadFromURL(url);
    uno::Reference< XOfficeDatabaseDocument > xDocument(mxComponent, UNO_QUERY_THROW);
    return xDocument;
}

uno::Reference< XConnection > DBTestBase::getConnectionForDocument(
    uno::Reference< XOfficeDatabaseDocument > const & xDocument)
{
    uno::Reference< XDataSource > xDataSource = xDocument->getDataSource();
    CPPUNIT_ASSERT(xDataSource.is());

    uno::Reference< XConnection > xConnection = xDataSource->getConnection(u""_ustr,u""_ustr);
    CPPUNIT_ASSERT(xConnection.is());

    return xConnection;
}

void DBTestBase::createDBDocument(const OUString& rDriverURL)
{
    uno::Reference< XOfficeDatabaseDocument > xDocument(
        m_xSFactory->createInstance(u"com.sun.star.sdb.OfficeDatabaseDocument"_ustr),
        UNO_QUERY_THROW);
    uno::Reference< css::frame::XStorable > xStorable(xDocument, UNO_QUERY_THROW);

    uno::Reference< XDataSource > xDataSource = xDocument->getDataSource();
    uno::Reference< XPropertySet > xPropertySet(xDataSource, UNO_QUERY_THROW);
    xPropertySet->setPropertyValue(u"URL"_ustr, Any(rDriverURL));

    xStorable->storeAsURL(maTempFile.GetURL(), uno::Sequence< beans::PropertyValue >());

    loadFromURL(maTempFile.GetURL());
}

void DBTestBase::createTables(const Reference<XConnection>& xConnection)
{
    uno::Reference<XStatement> xStatement = xConnection->createStatement();

    xStatement->execute(
        u"CREATE TABLE \"CATEGORIES\" ( \"ID\" INTEGER NOT NULL PRIMARY KEY "
        ", \"NAME\" VARCHAR (50) , \"DESCRIPTION\" VARCHAR (1024), \"IMAGE\" LONGVARBINARY"
    ")"_ustr);

    xStatement->execute(u"INSERT INTO \"CATEGORIES\" (\"ID\", \"NAME\") VALUES ( 1, 'Food' )"_ustr);
    xStatement->execute(u"INSERT INTO \"CATEGORIES\" (\"ID\", \"NAME\") VALUES ( 2, 'Furniture' )"_ustr);

    xStatement->execute(
        u"CREATE TABLE \"PRODUCTS\" ( \"ID\" INTEGER NOT NULL PRIMARY KEY "
        ", \"NAME\" VARCHAR (50) , \"CATEGORYID\" INTEGER NOT NULL, FOREIGN KEY (\"CATEGORYID\")"
        "REFERENCES \"CATEGORIES\" (\"ID\") )"_ustr);

    xStatement->execute(u"INSERT INTO \"PRODUCTS\" VALUES ( 1, 'Orange', 1 )"_ustr);
    xStatement->execute(u"INSERT INTO \"PRODUCTS\" VALUES ( 2, 'Apples', 1 )"_ustr);
    xStatement->execute(u"INSERT INTO \"PRODUCTS\" VALUES ( 3, 'Pears', 1 )"_ustr);
    xStatement->execute(u"INSERT INTO \"PRODUCTS\" VALUES ( 4, 'Strawberries', 1 )"_ustr);

    xStatement->execute(
        u"CREATE TABLE \"CUSTOMERS\" ( \"ID\" INTEGER NOT NULL PRIMARY KEY "
        ", \"NAME\" VARCHAR (50) , \"ADDRESS\" VARCHAR (50), \"CITY\" VARCHAR (50), \"POSTAL\" VARCHAR (50)"
        ", \"COMMENT\" LONGVARCHAR )"_ustr);

    xStatement->execute(u"INSERT INTO \"CUSTOMERS\" VALUES(1,'Food, Inc.','Down Under','Melbourne','509','Preferred')"_ustr);
    xStatement->execute(u"INSERT INTO \"CUSTOMERS\" VALUES(2,'Simply Delicious','Down Under','Melbourne','518',null)"_ustr);
    xStatement->execute(u"INSERT INTO \"CUSTOMERS\" VALUES(3,'Pure Health','10 Fish St.','San Francisco','94107',null)"_ustr);
    xStatement->execute(u"INSERT INTO \"CUSTOMERS\" VALUES(4,'Milk And More','Arlington Road 21','Dublin','31021','Good one.')"_ustr);

    xStatement->execute(
        u"CREATE TABLE \"ORDERS\" ( \"ID\" INTEGER NOT NULL PRIMARY KEY, \"CUSTOMERID\" INTEGER NOT NULL "
        ", \"ORDERDATE\" DATE, \"SHIPDATE\" DATE, FOREIGN KEY (\"CUSTOMERID\")"
        "REFERENCES \"CUSTOMERS\" (\"ID\") )"_ustr);

    xStatement->execute(u"INSERT INTO \"ORDERS\" (\"ID\", \"CUSTOMERID\", \"ORDERDATE\") VALUES(1, 1, {D '2009-01-01'})"_ustr);
    xStatement->execute(u"INSERT INTO \"ORDERS\" VALUES(2, 2, {D '2009-01-01'}, {D '2009-01-23'})"_ustr);

    xStatement->execute(
        u"CREATE TABLE \"ORDERS_DETAILS\" ( \"ORDERID\" INTEGER NOT NULL, \"PRODUCTID\" INTEGER NOT NULL, \"QUANTITY\" INTEGER"
        ", FOREIGN KEY (\"ORDERID\") REFERENCES \"ORDERS\" (\"ID\") "
        ", FOREIGN KEY (\"PRODUCTID\") REFERENCES \"PRODUCTS\" (\"ID\"), PRIMARY KEY(\"ORDERID\", \"PRODUCTID\") )"_ustr);

    xStatement->execute(u"INSERT INTO \"ORDERS_DETAILS\" VALUES(1, 1, 100)"_ustr);
    xStatement->execute(u"INSERT INTO \"ORDERS_DETAILS\" VALUES(1, 2, 100)"_ustr);
    xStatement->execute(u"INSERT INTO \"ORDERS_DETAILS\" VALUES(2, 2, 2000)"_ustr);
    xStatement->execute(u"INSERT INTO \"ORDERS_DETAILS\" VALUES(2, 3, 2000)"_ustr);
    xStatement->execute(u"INSERT INTO \"ORDERS_DETAILS\" VALUES(2, 4, 2000)"_ustr);
    xConnection->commit();
}

void DBTestBase::createQueries(const Reference<XDataSource>& xDataSource)
{
    createQuery(
            u"SELECT \"ORDERS\".\"ID\" AS \"Order No.\", "
                    "\"CUSTOMERS\".\"NAME\" AS \"Customer Name\", "
                    "\"ORDERS\".\"ORDERDATE\" AS \"Order Date\", "
                    "\"ORDERS\".\"SHIPDATE\" AS \"Ship Date\", "
                    "\"ORDERS_DETAILS\".\"QUANTITY\", "
                    "\"PRODUCTS\".\"NAME\" AS \"Product Name\" "
            "FROM \"ORDERS_DETAILS\" AS \"ORDERS_DETAILS\", "
                  "\"ORDERS\" AS \"ORDERS\", "
                  "\"PRODUCTS\" AS \"PRODUCTS\", "
                  "\"CUSTOMERS\" AS \"CUSTOMERS\" "
            "WHERE  ( \"ORDERS_DETAILS\".\"ORDERID\" = \"ORDERS\".\"ID\" "
                 "AND \"ORDERS_DETAILS\".\"PRODUCTID\" = \"PRODUCTS\".\"ID\" "
                 "AND \"ORDERS\".\"CUSTOMERID\" = \"CUSTOMERS\".\"ID\" )"_ustr,
                    true, u"all orders"_ustr, xDataSource);

    createQuery(
            u"SELECT *"
            "FROM \"all orders\""
            "WHERE ( \"SHIPDATE\" IS NULL )"_ustr, true, u"unshipped orders"_ustr, xDataSource);

    createQuery(u"SELECT * FROM \"CUSTOMERS\""_ustr, true, u"parseable"_ustr, xDataSource);
    createQuery(u"SELECT * FROM INFORMATION_SCHEMA.SYSTEM_VIEWS"_ustr, false, u"parseable native"_ustr, xDataSource);
    createQuery(u"SELECT {fn DAYOFMONTH ('2001-01-01')} AS \"ID_VARCHAR\" FROM \"PRODUCTS\""_ustr,
                false, u"parse odbc escape"_ustr, xDataSource);
}

void DBTestBase::createQuery(const OUString& sQuery, bool bEscapeProcessing, const OUString& sQueryName, const Reference<XDataSource> & xDataSource)
{
    Reference<XQueryDefinitionsSupplier> xQuerySupplier(xDataSource, UNO_QUERY_THROW);
    Reference<container::XNameAccess> xQueryAccess = xQuerySupplier->getQueryDefinitions();
    CPPUNIT_ASSERT(xQueryAccess.is());

    Reference<lang::XSingleServiceFactory> xSingleServiceFactory(xQueryAccess, UNO_QUERY_THROW);
    Reference<beans::XPropertySet> xQueryProp(xSingleServiceFactory->createInstance(), UNO_QUERY_THROW);

    xQueryProp->setPropertyValue(u"Command"_ustr, Any(sQuery));
    xQueryProp->setPropertyValue(u"EscapeProcessing"_ustr, Any(bEscapeProcessing));

    Reference<container::XNameContainer> xNameContainer(xQueryAccess, UNO_QUERY_THROW);
    xNameContainer->insertByName(sQueryName, Any(xQueryProp));
}

void DBTestBase::tearDown()
{
    if (mxComponent)
    {
        // In order to close all windows
        css::uno::Reference<util::XCloseable> xCloseable(mxComponent, css::uno::UNO_QUERY_THROW);
        xCloseable->close(false);
    }

    UnoApiTest::tearDown();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
