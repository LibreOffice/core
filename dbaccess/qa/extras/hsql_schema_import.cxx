/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <fbcreateparser.hxx>
#include <columndef.hxx>
#include <cppunit/plugin/TestPlugIn.h>
#include <com/sun/star/sdbc/DataType.hpp>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace dbahsql;

namespace
{
constexpr std::size_t operator"" _z(unsigned long long n) { return n; }
}

class HsqlSchemaImportTest : public CppUnit::TestFixture
{
public:
    void testIntegerPrimaryKeyNotNull();
    void testVarcharWithParam();
    void testVarcharWithoutParam();
    void testNumericWithTwoParam();
    void testIntegerAutoincremental();
    void testTimestampWithParam();
    // TODO testForeign, testDecomposer

    CPPUNIT_TEST_SUITE(HsqlSchemaImportTest);

    CPPUNIT_TEST(testIntegerPrimaryKeyNotNull);
    CPPUNIT_TEST(testVarcharWithParam);
    CPPUNIT_TEST(testVarcharWithoutParam);
    CPPUNIT_TEST(testNumericWithTwoParam);
    CPPUNIT_TEST(testIntegerAutoincremental);
    CPPUNIT_TEST(testTimestampWithParam);

    CPPUNIT_TEST_SUITE_END();
};

void HsqlSchemaImportTest::testIntegerPrimaryKeyNotNull()
{
    OUString sql{ "CREATE CACHED TABLE \"myTable\"(\"id\" INTEGER NOT NULL PRIMARY KEY)" };

    FbCreateStmtParser aCreateParser;
    aCreateParser.parse(sql);

    CPPUNIT_ASSERT_EQUAL(OUString{ "\"myTable\"" }, aCreateParser.getTableName());
    const auto& columns = aCreateParser.getColumnDef();
    CPPUNIT_ASSERT_EQUAL(1_z, columns.size());
    const auto& column = columns.at(0);
    CPPUNIT_ASSERT_EQUAL(OUString{ "\"id\"" }, column.getName());
    CPPUNIT_ASSERT_EQUAL(css::sdbc::DataType::INTEGER, column.getDataType());
    CPPUNIT_ASSERT(column.isPrimaryKey());
    CPPUNIT_ASSERT(!column.isNullable());
    CPPUNIT_ASSERT(!column.isAutoIncremental());
}

void HsqlSchemaImportTest::testVarcharWithParam()
{
    OUString sql{ "CREATE CACHED TABLE \"myTable\"(\"id\" INTEGER NOT NULL PRIMARY KEY, \"myText\" "
                  "VARCHAR(50))" };

    FbCreateStmtParser aCreateParser;
    aCreateParser.parse(sql);

    const auto& columns = aCreateParser.getColumnDef();
    CPPUNIT_ASSERT_EQUAL(2_z, columns.size());
    const auto& it = std::find_if(columns.begin(), columns.end(), [](auto const& column) {
        return column.getDataType() == css::sdbc::DataType::VARCHAR;
    });
    CPPUNIT_ASSERT(it != columns.end());
    const auto& params = it->getParams();
    CPPUNIT_ASSERT_EQUAL(1_z, params.size());
    CPPUNIT_ASSERT_EQUAL(50, params.at(0)); // VARCHAR(50)
}

/**
 * Special case:
 * HSQLDB might define a column VARCHAR without parameter. With Firebird
 * dialect, this is forbidden, so a default parameter has to be appended:
 **/
void HsqlSchemaImportTest::testVarcharWithoutParam()
{
    OUString sql{ "CREATE CACHED TABLE \"myTable\"(\"id\" INTEGER NOT NULL PRIMARY KEY, \"myText\" "
                  "VARCHAR)" };

    FbCreateStmtParser aCreateParser;
    aCreateParser.parse(sql);

    const auto& columns = aCreateParser.getColumnDef();
    CPPUNIT_ASSERT_EQUAL(2_z, columns.size());
    const auto& it = std::find_if(columns.begin(), columns.end(), [](auto const& column) {
        return column.getDataType() == css::sdbc::DataType::VARCHAR;
    });
    CPPUNIT_ASSERT(it != columns.end());
    const auto& params = it->getParams();
    CPPUNIT_ASSERT_EQUAL(1_z, params.size()); // parameter generated
}

void HsqlSchemaImportTest::testNumericWithTwoParam()
{
    OUString sql{ "CREATE CACHED TABLE \"myTable\"(\"id\" INTEGER NOT NULL PRIMARY KEY, \"Betrag\" "
                  "NUMERIC(8,2))" };

    FbCreateStmtParser aCreateParser;
    aCreateParser.parse(sql);

    const auto& columns = aCreateParser.getColumnDef();
    CPPUNIT_ASSERT_EQUAL(2_z, columns.size());

    const auto& it = std::find_if(columns.begin(), columns.end(), [](auto const& column) {
        return column.getDataType() == css::sdbc::DataType::NUMERIC;
    });
    CPPUNIT_ASSERT(it != columns.end());
    const auto& params = it->getParams();
    CPPUNIT_ASSERT_EQUAL(2_z, params.size());
    CPPUNIT_ASSERT_EQUAL(8, params.at(0)); // precision
    CPPUNIT_ASSERT_EQUAL(2, params.at(1)); // scale
}

void HsqlSchemaImportTest::testIntegerAutoincremental()
{
    OUString sql{ "CREATE CACHED TABLE \"myTable\"(\"id\" INTEGER NOT NULL PRIMARY KEY GENERATED "
                  "BY DEFAULT AS IDENTITY(START WITH 0), \"myText\" VARCHAR(50))" };

    FbCreateStmtParser aCreateParser;
    aCreateParser.parse(sql);

    const auto& columns = aCreateParser.getColumnDef();
    const auto column = columns.at(0);
    CPPUNIT_ASSERT_EQUAL(css::sdbc::DataType::INTEGER, column.getDataType());
    CPPUNIT_ASSERT(column.isAutoIncremental());
    CPPUNIT_ASSERT(column.isPrimaryKey());
    CPPUNIT_ASSERT(!column.isNullable());
}

/**
 * Special case:
 * Hsqldb might use one parameter for defining column with type TIMESTAMP.
 * With Firebird this is illegal.
 */
void HsqlSchemaImportTest::testTimestampWithParam()
{
    OUString sql{ "CREATE CACHED TABLE \"myTable\"(\"id\" INTEGER NOT NULL PRIMARY KEY, \"myText\" "
                  "TIMESTAMP(0))" };

    FbCreateStmtParser aCreateParser;
    aCreateParser.parse(sql);

    const auto& columns = aCreateParser.getColumnDef();
    const auto& it = std::find_if(columns.begin(), columns.end(), [](auto const& column) {
        return column.getDataType() == css::sdbc::DataType::TIMESTAMP;
    });
    CPPUNIT_ASSERT(it != columns.end());

    // instead of asserting parameter size, look at the deparsed string,
    // because it's Firebird specific
    OUString fbSql = aCreateParser.compose();
    CPPUNIT_ASSERT(fbSql.indexOf("0") < 0); //does not contain
}

CPPUNIT_TEST_SUITE_REGISTRATION(HsqlSchemaImportTest);

CPPUNIT_PLUGIN_IMPLEMENT();
