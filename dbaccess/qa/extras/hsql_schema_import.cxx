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

const ColumnDefinition* lcl_findByType(const std::vector<ColumnDefinition>& columns,
                                       sal_Int32 nType)
{
    for (const auto& column : columns)
    {
        if (column.getDataType() == nType)
            return &column;
    }
    return nullptr;
}
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
    void testDefaultValueNow();
    void testEvilNullColumnName();
    // TODO testForeign, testDecomposer

    CPPUNIT_TEST_SUITE(HsqlSchemaImportTest);

    CPPUNIT_TEST(testIntegerPrimaryKeyNotNull);
    CPPUNIT_TEST(testVarcharWithParam);
    CPPUNIT_TEST(testVarcharWithoutParam);
    CPPUNIT_TEST(testNumericWithTwoParam);
    CPPUNIT_TEST(testIntegerAutoincremental);
    CPPUNIT_TEST(testTimestampWithParam);
    CPPUNIT_TEST(testDefaultValueNow);
    CPPUNIT_TEST(testEvilNullColumnName);

    CPPUNIT_TEST_SUITE_END();
};

void HsqlSchemaImportTest::testIntegerPrimaryKeyNotNull()
{
    FbCreateStmtParser aCreateParser;
    aCreateParser.parse(u"CREATE CACHED TABLE \"myTable\"(\"id\" INTEGER NOT NULL PRIMARY KEY)");

    CPPUNIT_ASSERT_EQUAL(u"\"myTable\""_ustr, aCreateParser.getTableName());
    const auto& columns = aCreateParser.getColumnDef();
    CPPUNIT_ASSERT_EQUAL(1_z, columns.size());
    const auto& column = columns.at(0);
    CPPUNIT_ASSERT_EQUAL(u"\"id\""_ustr, column.getName());
    CPPUNIT_ASSERT_EQUAL(css::sdbc::DataType::INTEGER, column.getDataType());
    CPPUNIT_ASSERT(column.isPrimaryKey());
    CPPUNIT_ASSERT(!column.isNullable());
    CPPUNIT_ASSERT(!column.isAutoIncremental());
}

void HsqlSchemaImportTest::testVarcharWithParam()
{
    FbCreateStmtParser aCreateParser;
    aCreateParser.parse(
        u"CREATE CACHED TABLE \"myTable\"(\"id\" INTEGER NOT NULL PRIMARY KEY, \"myText\" "
        "VARCHAR(50))");

    const auto& columns = aCreateParser.getColumnDef();
    CPPUNIT_ASSERT_EQUAL(2_z, columns.size());
    const ColumnDefinition* colVarchar = lcl_findByType(columns, css::sdbc::DataType::VARCHAR);
    CPPUNIT_ASSERT(colVarchar != nullptr);
    const auto& params = colVarchar->getParams();
    CPPUNIT_ASSERT_EQUAL(1_z, params.size());
    constexpr sal_Int32 nParamExpected = 50;
    CPPUNIT_ASSERT_EQUAL(nParamExpected, params.at(0)); // VARCHAR(50)
}

/**
 * Special case:
 * HSQLDB might define a column VARCHAR without parameter. With Firebird
 * dialect, this is forbidden, so a default parameter has to be appended:
 **/
void HsqlSchemaImportTest::testVarcharWithoutParam()
{
    FbCreateStmtParser aCreateParser;
    aCreateParser.parse(
        u"CREATE CACHED TABLE \"myTable\"(\"id\" INTEGER NOT NULL PRIMARY KEY, \"myText\" "
        "VARCHAR)");

    const auto& columns = aCreateParser.getColumnDef();
    CPPUNIT_ASSERT_EQUAL(2_z, columns.size());
    const ColumnDefinition* colVarchar = lcl_findByType(columns, css::sdbc::DataType::VARCHAR);
    CPPUNIT_ASSERT(colVarchar != nullptr);
    const auto& params = colVarchar->getParams();
    CPPUNIT_ASSERT_EQUAL(1_z, params.size()); // parameter generated
}

void HsqlSchemaImportTest::testNumericWithTwoParam()
{
    FbCreateStmtParser aCreateParser;
    aCreateParser.parse(
        u"CREATE CACHED TABLE \"myTable\"(\"id\" INTEGER NOT NULL PRIMARY KEY, \"Betrag\" "
        "NUMERIC(8,2))");

    const auto& columns = aCreateParser.getColumnDef();
    CPPUNIT_ASSERT_EQUAL(2_z, columns.size());

    const ColumnDefinition* colNumeric = lcl_findByType(columns, css::sdbc::DataType::NUMERIC);
    CPPUNIT_ASSERT(colNumeric != nullptr);
    const auto& params = colNumeric->getParams();
    CPPUNIT_ASSERT_EQUAL(2_z, params.size());

    constexpr sal_Int32 nPrecision = 8;
    constexpr sal_Int32 nScale = 2;
    CPPUNIT_ASSERT_EQUAL(nPrecision, params.at(0));
    CPPUNIT_ASSERT_EQUAL(nScale, params.at(1));
}

void HsqlSchemaImportTest::testIntegerAutoincremental()
{
    FbCreateStmtParser aCreateParser;
    aCreateParser.parse(
        u"CREATE CACHED TABLE \"myTable\"(\"id\" INTEGER NOT NULL PRIMARY KEY GENERATED "
        "BY DEFAULT AS IDENTITY(START WITH 0), \"myText\" VARCHAR(50))");

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
    FbCreateStmtParser aCreateParser;
    aCreateParser.parse(
        u"CREATE CACHED TABLE \"myTable\"(\"id\" INTEGER NOT NULL PRIMARY KEY, \"myText\" "
        "TIMESTAMP(0))");

    const auto& columns = aCreateParser.getColumnDef();
    const ColumnDefinition* colTimeStamp = lcl_findByType(columns, css::sdbc::DataType::TIMESTAMP);

    CPPUNIT_ASSERT(colTimeStamp != nullptr);

    // instead of asserting parameter size, look at the deparsed string,
    // because it's Firebird specific
    OUString fbSql = aCreateParser.compose();
    CPPUNIT_ASSERT(fbSql.indexOf("0") < 0); //does not contain
}

/**
 * Special case:
 * HSQLDB uses keyword NOW without quotes. Firebird uses single quotes 'NOW'
 */
void HsqlSchemaImportTest::testDefaultValueNow()
{
    FbCreateStmtParser aCreateParser;
    aCreateParser.parse(
        u"CREATE CACHED TABLE \"myTable\"(\"id\" INTEGER NOT NULL PRIMARY KEY, \"myDate\" "
        "TIMESTAMP DEFAULT NOW)");

    const auto& columns = aCreateParser.getColumnDef();
    const ColumnDefinition* colTimeStamp = lcl_findByType(columns, css::sdbc::DataType::TIMESTAMP);

    CPPUNIT_ASSERT(colTimeStamp != nullptr);
    CPPUNIT_ASSERT_EQUAL(u"NOW"_ustr, colTimeStamp->getDefault()); // parsed NOW
    OUString fbSql = aCreateParser.compose();
    CPPUNIT_ASSERT(fbSql.indexOf("\'NOW\'") > 0); // composed 'NOW'
}

void HsqlSchemaImportTest::testEvilNullColumnName()
{
    FbCreateStmtParser aCreateParser;
    aCreateParser.parse(u"CREATE CACHED TABLE \"myTable\"(\"id\" INTEGER NOT NULL PRIMARY KEY, "
                        "\"myEvilNOT NULLName\" "
                        "VARCHAR(20))");

    const auto& columns = aCreateParser.getColumnDef();
    CPPUNIT_ASSERT_EQUAL(2_z, columns.size());
    const ColumnDefinition* colVarchar = lcl_findByType(columns, css::sdbc::DataType::VARCHAR);
    CPPUNIT_ASSERT(colVarchar != nullptr);
    CPPUNIT_ASSERT(colVarchar->isNullable());
}

CPPUNIT_TEST_SUITE_REGISTRATION(HsqlSchemaImportTest);

CPPUNIT_PLUGIN_IMPLEMENT();
