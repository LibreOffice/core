/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dbtest_base.cxx"

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/XQueryDefinition.hpp>
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class SingleSelectQueryComposerTest : public DBTestBase
{
    Reference<XDataSource> m_xDataSource;
    Reference<XConnection> m_xConnection;
    Reference<XSingleSelectQueryComposer> m_xComposer;

public:
    virtual void setUp() override;
    virtual void tearDown() override;
    void testSetCommand();
    void testAttributes();
    void testSubQueries();
    void testParameters();
    void testConditionByColumn();
    void testDisjunctiveNormalForm();

    CPPUNIT_TEST_SUITE(SingleSelectQueryComposerTest);
    CPPUNIT_TEST(testSetCommand);
    CPPUNIT_TEST(testAttributes);
    CPPUNIT_TEST(testSubQueries);
    CPPUNIT_TEST(testParameters);
    CPPUNIT_TEST(testConditionByColumn);
    CPPUNIT_TEST(testDisjunctiveNormalForm);
    CPPUNIT_TEST_SUITE_END();
};

void SingleSelectQueryComposerTest::setUp()
{
    DBTestBase::setUp();
    createDBDocument(u"sdbc:embedded:hsqldb"_ustr);
    Reference<sdb::XOfficeDatabaseDocument> xDocument(mxComponent, UNO_QUERY_THROW);
    m_xDataSource = xDocument->getDataSource();

    // Create some common queries
    DBTestBase::createQueries(m_xDataSource);

    m_xConnection = m_xDataSource->getConnection(u""_ustr, u""_ustr);
    DBTestBase::createTables(m_xConnection);

    // For some reason we must close, then reopen the connection in
    // order for the tables to be visible to the connection
    m_xConnection->close();
    m_xConnection = m_xDataSource->getConnection(u""_ustr, u""_ustr);

    Reference<lang::XMultiServiceFactory> xFactory(m_xConnection, UNO_QUERY);
    Reference<XSingleSelectQueryComposer> xComposer(
        xFactory->createInstance(u"com.sun.star.sdb.SingleSelectQueryComposer"_ustr),
        UNO_QUERY_THROW);
    m_xComposer = xComposer;
}

void SingleSelectQueryComposerTest::tearDown()
{
    m_xComposer.clear();
    if (m_xConnection)
    {
        m_xConnection->close();
        m_xConnection.clear();
    }
    DBTestBase::tearDown();
}

// tests setCommand of the composer

void SingleSelectQueryComposerTest::testSetCommand()
{
    // CommandType::TABLE
    m_xComposer->setCommand(u"CUSTOMERS"_ustr, CommandType::TABLE);
    CPPUNIT_ASSERT_EQUAL(u"SELECT * FROM \"CUSTOMERS\""_ustr, m_xComposer->getQuery());

    // CommandType::QUERY
    DBTestBase::createQuery(u"SELECT * FROM \"orders for customer\" \"a\", \"customers\" "
                            "\"b\" WHERE \"a\".\"Product Name\" = \"b\".\"Name\""_ustr,
                            true, u"set command test"_ustr, m_xDataSource);

    Reference<XQueriesSupplier> xQuerySupplier(m_xConnection, UNO_QUERY_THROW);
    Reference<container::XNameAccess> xQueryAccess = xQuerySupplier->getQueries();
    CPPUNIT_ASSERT(xQueryAccess->hasElements());
    Reference<XPropertySet> xQuery(xQueryAccess->getByName(u"set command test"_ustr), UNO_QUERY);
    CPPUNIT_ASSERT(xQuery.is());

    m_xComposer->setCommand(u"set command test"_ustr, CommandType::QUERY);
    OUString sQuery;
    xQuery->getPropertyValue(u"Command"_ustr) >>= sQuery;

    CPPUNIT_ASSERT_EQUAL(sQuery, m_xComposer->getQuery());

    // CommandType::COMMAND
    const OUString sCommand
        = u"SELECT * FROM \"orders for customer\" WHERE \"Product Name\" = 'test'"_ustr;
    m_xComposer->setCommand(sCommand, CommandType::COMMAND);
    CPPUNIT_ASSERT_EQUAL(sCommand, m_xComposer->getQuery());
}

// tests accessing attributes of the composer (order, filter, group by, having)

void SingleSelectQueryComposerTest::testAttributes()
{
    const OUString sSimpleQuery2 = u"SELECT * FROM \"CUSTOMERS\" WHERE \"Name\" = 'oranges'"_ustr;
    m_xComposer->setElementaryQuery(sSimpleQuery2);
    CPPUNIT_ASSERT_EQUAL(sSimpleQuery2, m_xComposer->getQuery());

    const OUString sSimpleQuery = u"SELECT * FROM \"CUSTOMERS\""_ustr;
    m_xComposer->setQuery(sSimpleQuery);
    CPPUNIT_ASSERT_EQUAL(sSimpleQuery, m_xComposer->getQuery());

    // checkAttributeAccess "Filter"
    OUString sFilter = u"\"NAME\" = 'oranges'"_ustr;
    m_xComposer->setFilter(sFilter);
    CPPUNIT_ASSERT_EQUAL(sFilter, m_xComposer->getFilter());

    // checkAttributeAccess "Group"
    sFilter = u"\"CITY\""_ustr;
    m_xComposer->setGroup(sFilter);
    CPPUNIT_ASSERT_EQUAL(sFilter, m_xComposer->getGroup());

    // checkAttributeAccess "Order"
    sFilter = u"\"ADDRESS\""_ustr;
    m_xComposer->setOrder(sFilter);
    CPPUNIT_ASSERT_EQUAL(sFilter, m_xComposer->getOrder());

    // checkAttributeAccess "HavingClause"
    sFilter = u"\"ID\" <> 4"_ustr;
    m_xComposer->setHavingClause(sFilter);
    CPPUNIT_ASSERT_EQUAL(sFilter, m_xComposer->getHavingClause());

    // check getOrderColumns
    Reference<container::XIndexAccess> orderColumns(m_xComposer->getOrderColumns(), UNO_SET_THROW);
    CPPUNIT_ASSERT(orderColumns->hasElements());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), orderColumns->getCount());

    const Reference<XPropertySet> xOrderColumns(orderColumns->getByIndex(0), UNO_QUERY_THROW);
    OUString sColumnName;
    xOrderColumns->getPropertyValue(u"Name"_ustr) >>= sColumnName;
    CPPUNIT_ASSERT_EQUAL(u"ADDRESS"_ustr, sColumnName);

    // check getGroupColumns
    Reference<container::XIndexAccess> groupColumns(m_xComposer->getGroupColumns(), UNO_SET_THROW);
    CPPUNIT_ASSERT(groupColumns->hasElements());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), groupColumns->getCount());

    const Reference<XPropertySet> xGroupColumns(groupColumns->getByIndex(0), UNO_QUERY_THROW);
    OUString sGroupColumnName;
    xGroupColumns->getPropertyValue(u"Name"_ustr) >>= sGroupColumnName;
    CPPUNIT_ASSERT_EQUAL(u"CITY"_ustr, sGroupColumnName);

    // XColumnSupplier
    Reference<sdbcx::XColumnsSupplier> xSelectColumns(m_xComposer, UNO_QUERY_THROW);
    Reference<container::XNameAccess> xColumnAccess = xSelectColumns->getColumns();
    CPPUNIT_ASSERT(xColumnAccess->hasElements());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xColumnAccess->getElementNames().getLength());

    const OUString COMPLEXFILTER = u"( \"ID\" = 1 AND \"POSTAL\" = '4' )"
                                   " OR ( \"ID\" = 2 AND \"POSTAL\" = '5' )"
                                   " OR ( \"ID\" = 3 AND \"POSTAL\" = '6' AND \"ADDRESS\" = '7' )"
                                   " OR ( \"ADDRESS\" = '8' )"
                                   " OR ( \"POSTAL\" = '9' )"
                                   " OR ( NOW( ) = {d '2010-01-01' } )"_ustr;

    // structured filter
    m_xComposer->setQuery(u"SELECT \"ID\", \"POSTAL\", \"ADDRESS\" FROM \"CUSTOMERS\""_ustr);
    m_xComposer->setFilter(COMPLEXFILTER);
    Sequence<Sequence<PropertyValue>> aStructuredFilter = m_xComposer->getStructuredFilter();
    m_xComposer->setFilter(u""_ustr);
    m_xComposer->setStructuredFilter(aStructuredFilter);
    CPPUNIT_ASSERT_EQUAL(COMPLEXFILTER, m_xComposer->getFilter());

    // structured having clause
    m_xComposer->setHavingClause(COMPLEXFILTER);
    Sequence<Sequence<PropertyValue>> aStructuredHaving = m_xComposer->getStructuredHavingClause();
    m_xComposer->setHavingClause("");
    m_xComposer->setStructuredHavingClause(aStructuredHaving);
    CPPUNIT_ASSERT_EQUAL(COMPLEXFILTER, m_xComposer->getHavingClause());
}

// test various sub query related features ("queries in queries")

void SingleSelectQueryComposerTest::testSubQueries()
{
    const OUString INNERPRODUCTSQUERY = u"products (inner)"_ustr;
    DBTestBase::createQuery(u"SELECT * FROM \"PRODUCTS\""_ustr, true, INNERPRODUCTSQUERY,
                            m_xDataSource);

    m_xComposer->setQuery(u"SELECT * FROM \""_ustr + INNERPRODUCTSQUERY + u"\""_ustr);

    Reference<sdbcx::XTablesSupplier> suppTables(m_xComposer, UNO_QUERY_THROW);
    Reference<container::XNameAccess> tables = suppTables->getTables();
    CPPUNIT_ASSERT(tables->hasElements());
    CPPUNIT_ASSERT(tables->hasByName(INNERPRODUCTSQUERY));

    Reference<XQueriesSupplier> xQuerySupplier(m_xConnection, UNO_QUERY_THROW);
    Reference<container::XNameAccess> xQueryAccess = xQuerySupplier->getQueries();
    CPPUNIT_ASSERT(xQueryAccess->hasElements());

    Reference<XPropertySet> xQuery(xQueryAccess->getByName(INNERPRODUCTSQUERY), UNO_QUERY);
    OUString sInnerProductsQuery;
    xQuery->getPropertyValue(u"Command"_ustr) >>= sInnerProductsQuery;

    const OUString sExpectedQuery = u"SELECT * FROM ( "_ustr + sInnerProductsQuery
                                    + u" )"
                                      " AS \""_ustr
                                    + INNERPRODUCTSQUERY + u"\""_ustr;
    const OUString sExecutableQuery = m_xComposer->getQueryWithSubstitution();

    CPPUNIT_ASSERT_EQUAL(sExpectedQuery, sExecutableQuery);
}

// test the XParametersSupplier functionality

void SingleSelectQueryComposerTest::testParameters()
{
    // "orders for customers" is a query with a named parameter (based on another query)
    DBTestBase::createQuery(
        u"SELECT * FROM \"orders for customer\" WHERE \"Product Name\" LIKE ?"_ustr, true,
        u"orders for customer and product"_ustr, m_xDataSource);

    // "orders for customer and product" is query based on "orders for customers", adding an additional,
    DBTestBase::createQuery(
        u"SELECT * FROM \"all orders\" WHERE \"Customer Name\" LIKE :cname"_ustr, true,
        u"orders for customer"_ustr, m_xDataSource);

    Reference<XQueryDefinitionsSupplier> xQuerySupplier(m_xDataSource, UNO_QUERY);
    Reference<container::XNameAccess> xQueryAccess = xQuerySupplier->getQueryDefinitions();
    Reference<sdb::XQueryDefinition> xQueryDefinition(
        xQueryAccess->getByName(u"orders for customer and product"_ustr), UNO_QUERY);

    OUString sCustomersAndProduct;
    xQueryDefinition->getPropertyValue(u"Command"_ustr) >>= sCustomersAndProduct;

    m_xComposer->setQuery(sCustomersAndProduct);

    Reference<XParametersSupplier> xSuppParams(m_xComposer, UNO_QUERY_THROW);
    Reference<container::XIndexAccess> xParameters = xSuppParams->getParameters();

    Sequence<OUString> const expectedParameters{ u"cname"_ustr, u"Product Name"_ustr };
    CPPUNIT_ASSERT_EQUAL(expectedParameters.getLength(), xParameters->getCount());

    for (auto i = 0; i < expectedParameters.getLength(); ++i)
    {
        Reference<XPropertySet> xParam(xParameters->getByIndex(i), UNO_QUERY);
        OUString sParamName;
        xParam->getPropertyValue(u"Name"_ustr) >>= sParamName;
        CPPUNIT_ASSERT_EQUAL(expectedParameters[i], sParamName);
    }
}

void SingleSelectQueryComposerTest::testConditionByColumn()
{
    m_xComposer->setQuery("SELECT * FROM \"CUSTOMERS\"");
    Sequence<Any> aArgs{ Any(NamedValue(u"AutomaticAddition"_ustr, Any(true))) };
    Reference<beans::XPropertyContainer> filter(
        m_xSFactory->createInstanceWithArguments(u"com.sun.star.beans.PropertyBag"_ustr, aArgs),
        UNO_QUERY);

    filter->addProperty(u"Name"_ustr, PropertyAttribute::MAYBEVOID, Any(u"COMMENT"_ustr));
    filter->addProperty(u"RealName"_ustr, PropertyAttribute::MAYBEVOID, Any(u"COMMENT"_ustr));
    filter->addProperty(u"TableName"_ustr, PropertyAttribute::MAYBEVOID, Any(u"CUSTOMERS"_ustr));
    filter->addProperty(u"Value"_ustr, PropertyAttribute::MAYBEVOID, Any(u"Good one."_ustr));
    filter->addProperty(u"Type"_ustr, PropertyAttribute::MAYBEVOID,
                        Any(sal_Int32(sdbc::DataType::LONGVARCHAR)));

    Reference<XPropertySet> xColumn(filter, UNO_QUERY_THROW);
    m_xComposer->appendFilterByColumn(xColumn, true, SQLFilterOperator::LIKE);

    Reference<XStatement> xStatement = m_xConnection->createStatement();
    Reference<XResultSet> xResults = xStatement->executeQuery(m_xComposer->getQuery());
    CPPUNIT_ASSERT(xResults.is());

    // At least one row should exist
    Reference<XRow> xRow(xResults, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xResults->next());
    // The row returned should be the 4th, i.e. the one that
    // contains the Value "Good One"
    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, xRow->getString(1));
}

// tests the disjunctive normal form functionality, aka the structured filter,
// of the composer

void SingleSelectQueryComposerTest::testDisjunctiveNormalForm()
{
    // a simple case: WHERE clause simply is a combination of predicates knitted with AND
    OUString sQuery = u"SELECT \"CUSTOMERS\".\"NAME\", "
                      "\"CUSTOMERS\".\"ADDRESS\", "
                      "\"CUSTOMERS\".\"CITY\", "
                      "\"CUSTOMERS\".\"POSTAL\", "
                      "\"PRODUCTS\".\"NAME\" "
                      "FROM \"ORDERS\", \"CUSTOMERS\", \"ORDERS_DETAILS\", \"PRODUCTS\" "
                      "WHERE (   \"ORDERS\".\"CUSTOMERID\" = \"CUSTOMERS\".\"ID\" "
                      "AND \"ORDERS_DETAILS\".\"ORDERID\" = \"ORDERS\".\"ID\" "
                      "AND \"ORDERS_DETAILS\".\"PRODUCTID\" = \"PRODUCTS\".\"ID\" "
                      ") "_ustr;

    m_xComposer->setQuery(sQuery);

    {
        Sequence<Sequence<PropertyValue>> disjunctiveNormalForm
            = m_xComposer->getStructuredFilter();

        Sequence<PropertyValue> expectedDNF(3);
        PropertyValue* pExpectedDNF = expectedDNF.getArray();

        pExpectedDNF[0].Name = u"CUSTOMERID"_ustr;
        pExpectedDNF[0].Handle = SQLFilterOperator::EQUAL;
        pExpectedDNF[0].Value <<= u"\"CUSTOMERS\".\"ID\""_ustr;
        pExpectedDNF[0].State = PropertyState_DIRECT_VALUE;

        pExpectedDNF[1].Name = u"ORDERID"_ustr;
        pExpectedDNF[1].Handle = SQLFilterOperator::EQUAL;
        pExpectedDNF[1].Value <<= u"\"ORDER\".\"ID\""_ustr;
        pExpectedDNF[1].State = PropertyState_DIRECT_VALUE;

        pExpectedDNF[2].Name = u"PRODUCTID"_ustr;
        pExpectedDNF[2].Handle = SQLFilterOperator::EQUAL;
        pExpectedDNF[2].Value <<= u"\"PRODUCT\".\"ID\""_ustr;
        pExpectedDNF[2].State = PropertyState_DIRECT_VALUE;

        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), disjunctiveNormalForm.getLength());

        for (auto i = 0; i < expectedDNF.getLength(); ++i)
        {
            CPPUNIT_ASSERT_EQUAL(expectedDNF[i].Name, disjunctiveNormalForm[0][i].Name);
        }
    }

    // somewhat more challenging: One of the conjunction terms is a disjunction itself
    sQuery = u"SELECT \"CUSTOMERS\".\"NAME\", "
             "\"CUSTOMERS\".\"ADDRESS\", "
             "\"CUSTOMERS\".\"CITY\", "
             "\"CUSTOMERS\".\"POSTAL\", "
             "\"PRODUCTS\".\"NAME\" "
             "FROM \"ORDERS\", \"CUSTOMERS\", \"ORDERS_DETAILS\", \"PRODUCTS\" "
             "WHERE (   \"ORDERS\".\"CUSTOMERID\" = \"CUSTOMERS\".\"ID\" "
             "AND \"ORDERS_DETAILS\".\"ORDERID\" = \"ORDERS\".\"ID\" "
             "AND \"ORDERS_DETAILS\".\"PRODUCTID\" = \"PRODUCTS\".\"ID\" "
             ") "
             "AND "
             "(  \"PRODUCTS\".\"Name\" = 'Apples' "
             "OR \"products\".\"ID\" = 2 "
             ")"_ustr;

    m_xComposer->setQuery(sQuery);
    Sequence<Sequence<PropertyValue>> disjunctiveNormalForm = m_xComposer->getStructuredFilter();

    {
        Sequence<PropertyValue> firstExpectedDNF(4);
        PropertyValue* pFirstExpectedDNF = firstExpectedDNF.getArray();

        pFirstExpectedDNF[0].Name = u"CUSTOMERID"_ustr;
        pFirstExpectedDNF[0].Handle = SQLFilterOperator::EQUAL;
        pFirstExpectedDNF[0].Value <<= u"\"CUSTOMERS\".\"ID\""_ustr;
        pFirstExpectedDNF[0].State = PropertyState_DIRECT_VALUE;

        pFirstExpectedDNF[1].Name = u"ORDERID"_ustr;
        pFirstExpectedDNF[1].Handle = SQLFilterOperator::EQUAL;
        pFirstExpectedDNF[1].Value <<= u"\"ORDER\".\"ID\""_ustr;
        pFirstExpectedDNF[1].State = PropertyState_DIRECT_VALUE;

        pFirstExpectedDNF[2].Name = u"PRODUCTID"_ustr;
        pFirstExpectedDNF[2].Handle = SQLFilterOperator::EQUAL;
        pFirstExpectedDNF[2].Value <<= u"\"PRODUCT\".\"ID\""_ustr;
        pFirstExpectedDNF[2].State = PropertyState_DIRECT_VALUE;

        pFirstExpectedDNF[3].Name = u"Name"_ustr;
        pFirstExpectedDNF[3].Handle = SQLFilterOperator::EQUAL;
        pFirstExpectedDNF[3].Value <<= u"\"Apples\""_ustr;
        pFirstExpectedDNF[3].State = PropertyState_DIRECT_VALUE;

        Sequence<PropertyValue> secondExpectedDNF(4);
        PropertyValue* pSecondExpectedDNF = secondExpectedDNF.getArray();

        pSecondExpectedDNF[0].Name = u"CUSTOMERID"_ustr;
        pSecondExpectedDNF[0].Handle = SQLFilterOperator::EQUAL;
        pSecondExpectedDNF[0].Value <<= u"\"CUSTOMERS\".\"ID\""_ustr;
        pSecondExpectedDNF[0].State = PropertyState_DIRECT_VALUE;

        pSecondExpectedDNF[1].Name = u"ORDERID"_ustr;
        pSecondExpectedDNF[1].Handle = SQLFilterOperator::EQUAL;
        pSecondExpectedDNF[1].Value <<= u"\"ORDER\".\"ID\""_ustr;
        pSecondExpectedDNF[1].State = PropertyState_DIRECT_VALUE;

        pSecondExpectedDNF[2].Name = u"PRODUCTID"_ustr;
        pSecondExpectedDNF[2].Handle = SQLFilterOperator::EQUAL;
        pSecondExpectedDNF[2].Value <<= u"\"PRODUCT\".\"ID\""_ustr;
        pSecondExpectedDNF[2].State = PropertyState_DIRECT_VALUE;

        pSecondExpectedDNF[3].Name = u"ID"_ustr;
        pSecondExpectedDNF[3].Handle = SQLFilterOperator::EQUAL;
        pSecondExpectedDNF[3].Value <<= sal_Int32(2);
        pSecondExpectedDNF[3].State = PropertyState_DIRECT_VALUE;

        Sequence<Sequence<PropertyValue>> expectedDNF(2);
        Sequence<PropertyValue>* pExpectedDNF = expectedDNF.getArray();
        pExpectedDNF[0] = firstExpectedDNF;
        pExpectedDNF[1] = secondExpectedDNF;

        CPPUNIT_ASSERT_EQUAL(expectedDNF.getLength(), disjunctiveNormalForm.getLength());

        for (auto i = 0; i < expectedDNF.getLength(); ++i)
        {
            for (auto j = 0; j < expectedDNF[0].getLength(); ++j)
            {
                CPPUNIT_ASSERT_EQUAL(expectedDNF[i][j].Name, disjunctiveNormalForm[i][j].Name);
            }
        }
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(SingleSelectQueryComposerTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
