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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class SingleSelectQueryComposerTest : public DBTestBase
{
};

// test the XParametersSupplier functionality
CPPUNIT_TEST_FIXTURE(SingleSelectQueryComposerTest, testConditionByColumn)
{
    uno::Reference<XConnection> xConnection = setUpDBConnection();

    uno::Reference<lang::XMultiServiceFactory> xFactory(xConnection, UNO_QUERY);
    CPPUNIT_ASSERT(xFactory.is());

    Reference<XSingleSelectQueryComposer> xComposer(
        xFactory->createInstance(u"com.sun.star.sdb.SingleSelectQueryComposer"_ustr),
        UNO_QUERY_THROW);

    xComposer->setQuery("SELECT * FROM \"CUSTOMERS\"");
    Sequence<Any> aArgs{ Any(NamedValue(u"AutomaticAddition"_ustr, Any(true))) };
    Reference<beans::XPropertyContainer> filter(
        xFactory->createInstanceWithArguments(u"com.sun.star.beans.PropertyBag"_ustr, aArgs),
        UNO_QUERY);

    filter->addProperty(u"Name"_ustr, PropertyAttribute::MAYBEVOID, Any(u"COMMENT"_ustr));
    filter->addProperty(u"RealName"_ustr, PropertyAttribute::MAYBEVOID, Any(u"COMMENT"_ustr));
    filter->addProperty(u"TableName"_ustr, PropertyAttribute::MAYBEVOID, Any(u"CUSTOMERS"_ustr));
    filter->addProperty(u"Value"_ustr, PropertyAttribute::MAYBEVOID, Any(u"Good one."_ustr));
    filter->addProperty(u"Type"_ustr, PropertyAttribute::MAYBEVOID,
                        Any(sal_Int32(sdbc::DataType::LONGVARCHAR)));

    Reference<XPropertySet> xColumn(filter, UNO_QUERY_THROW);
    xComposer->appendFilterByColumn(xColumn, true, SQLFilterOperator::LIKE);

    Reference<XStatement> xStatement = xConnection->createStatement();
    Reference<XResultSet> xResults = xStatement->executeQuery(xComposer->getQuery());
    CPPUNIT_ASSERT(xResults.is());

    // At least one row should exist
    Reference<XRow> xRow(xResults, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xResults->next());
    // The row returned should be the 4th, i.e. the one that
    // contains the Value "Good One"
    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, xRow->getString(1));
}

// test various sub query related features ("queries in queries")
CPPUNIT_TEST_FIXTURE(SingleSelectQueryComposerTest, testSubQueries)
{
    uno::Reference<XConnection> xConnection = setUpDBConnection();

    uno::Reference<lang::XMultiServiceFactory> xFactory(xConnection, UNO_QUERY);
    CPPUNIT_ASSERT(xFactory.is());

    Reference<XSingleSelectQueryComposer> xComposer(
        xFactory->createInstance(u"com.sun.star.sdb.SingleSelectQueryComposer"_ustr),
        UNO_QUERY_THROW);

    uno::Reference<sdb::XOfficeDatabaseDocument> xDocument(mxComponent, UNO_QUERY_THROW);
    const OUString INNERPRODUCTSQUERY = u"products (inner)"_ustr;
    DBTestBase::createQuery(u"SELECT * FROM \"PRODUCTS\""_ustr, true, INNERPRODUCTSQUERY,
                            xDocument->getDataSource());

    xComposer->setQuery(u"SELECT * FROM \""_ustr + INNERPRODUCTSQUERY + u"\""_ustr);

    Reference<sdbcx::XTablesSupplier> suppTables(xComposer, UNO_QUERY_THROW);
    Reference<container::XNameAccess> tables = suppTables->getTables();
    CPPUNIT_ASSERT(tables->hasElements());
    CPPUNIT_ASSERT(tables->hasByName(INNERPRODUCTSQUERY));

    Reference<XQueriesSupplier> xQuerySupplier(xConnection, UNO_QUERY_THROW);
    Reference<container::XNameAccess> xQueryAccess = xQuerySupplier->getQueries();
    CPPUNIT_ASSERT(xQueryAccess->hasElements());

    Reference<XPropertySet> xQuery(xQueryAccess->getByName(INNERPRODUCTSQUERY), UNO_QUERY);
    OUString sInnerProductsQuery;
    xQuery->getPropertyValue(u"Command"_ustr) >>= sInnerProductsQuery;

    const OUString sExpectedQuery = u"SELECT * FROM ( "_ustr + sInnerProductsQuery
                                    + u" )"
                                      " AS \""_ustr
                                    + INNERPRODUCTSQUERY + u"\""_ustr;
    const OUString sExecutableQuery = xComposer->getQueryWithSubstitution();

    CPPUNIT_ASSERT_EQUAL(sExpectedQuery, sExecutableQuery);
}

CPPUNIT_TEST_FIXTURE(SingleSelectQueryComposerTest, testDisjunctiveNormalForm)
{
    uno::Reference<XConnection> xConnection = setUpDBConnection();

    uno::Reference<lang::XMultiServiceFactory> xFactory(xConnection, UNO_QUERY);
    CPPUNIT_ASSERT(xFactory.is());

    Reference<XSingleSelectQueryComposer> xComposer(
        xFactory->createInstance(u"com.sun.star.sdb.SingleSelectQueryComposer"_ustr),
        UNO_QUERY_THROW);

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
    xComposer->setQuery(sQuery);

    {
        Sequence<Sequence<PropertyValue>> disjunctiveNormalForm = xComposer->getStructuredFilter();
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
    xComposer->setQuery(sQuery);

    Sequence<Sequence<PropertyValue>> disjunctiveNormalForm = xComposer->getStructuredFilter();
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
