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

#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>

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
        m_xSFactory->createInstanceWithArguments(u"com.sun.star.beans.PropertyBag"_ustr, aArgs),
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
