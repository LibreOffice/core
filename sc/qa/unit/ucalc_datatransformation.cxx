/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>

#include "helper/qahelper.hxx"
#include <document.hxx>
#include <datatransformation.hxx>
#include <svl/numformat.hxx>
#include <tools/time.hxx>

class ScDataTransformationTest : public ScUcalcTestBase
{
public:
    virtual void setUp() override;
};

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testColumnRemove)
{
    for (SCROW nRow = 0; nRow < 10; ++nRow)
    {
        for (SCCOL nCol = 0; nCol < 10; ++nCol)
        {
            m_pDoc->SetValue(nCol, nRow, 0, nRow*nCol);
        }
    }

    sc::ColumnRemoveTransformation aTransformation({5});
    aTransformation.Transform(*m_pDoc);

    for (SCROW nRow = 0; nRow < 10; ++nRow)
    {
        for (SCCOL nCol = 0; nCol < 9; ++nCol)
        {
            double nVal = m_pDoc->GetValue(nCol, nRow, 0);
            if (nCol < 5)
            {
                ASSERT_DOUBLES_EQUAL(static_cast<double>(nCol)*nRow, nVal);
            }
            else
            {
                ASSERT_DOUBLES_EQUAL(static_cast<double>(nCol+1)*nRow, nVal);
            }
        }
    }
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testColumnSplit)
{
    m_pDoc->SetString(2, 0, 0, u"Test1,Test2"_ustr);
    m_pDoc->SetString(2, 1, 0, u"Test1,"_ustr);
    m_pDoc->SetString(2, 2, 0, u",Test1"_ustr);
    m_pDoc->SetString(2, 3, 0, u"Test1,Test2,Test3"_ustr);
    m_pDoc->SetString(3, 0, 0, u"AnotherString"_ustr);

    sc::SplitColumnTransformation aTransform(2, ',');
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(u"AnotherString"_ustr, m_pDoc->GetString(4, 0, 0));

    CPPUNIT_ASSERT_EQUAL(u"Test1"_ustr, m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Test1"_ustr, m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u""_ustr, m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"Test1"_ustr, m_pDoc->GetString(2, 3, 0));

    CPPUNIT_ASSERT_EQUAL(u"Test2"_ustr, m_pDoc->GetString(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u""_ustr, m_pDoc->GetString(3, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"Test1"_ustr, m_pDoc->GetString(3, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"Test2,Test3"_ustr, m_pDoc->GetString(3, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testColumnMerge)
{
    m_pDoc->SetString(2, 0, 0, u"Berlin"_ustr);
    m_pDoc->SetString(2, 1, 0, u"Brussels"_ustr);
    m_pDoc->SetString(2, 2, 0, u"Paris"_ustr);
    m_pDoc->SetString(2, 3, 0, u"Peking"_ustr);

    m_pDoc->SetString(4, 0, 0, u"Germany"_ustr);
    m_pDoc->SetString(4, 1, 0, u"Belgium"_ustr);
    m_pDoc->SetString(4, 2, 0, u"France"_ustr);
    m_pDoc->SetString(4, 3, 0, u"China"_ustr);

    sc::MergeColumnTransformation aTransform({2, 4}, u", "_ustr);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(u"Berlin, Germany"_ustr, m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Brussels, Belgium"_ustr, m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"Paris, France"_ustr, m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"Peking, China"_ustr, m_pDoc->GetString(2, 3, 0));

    for (SCROW nRow = 0; nRow <= 3; ++nRow)
    {
        CPPUNIT_ASSERT(m_pDoc->GetString(4, nRow, 0).isEmpty());
    }
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testTextToLower)
{
    m_pDoc->SetString(2, 0, 0, u"Berlin"_ustr);
    m_pDoc->SetString(2, 1, 0, u"Brussels"_ustr);
    m_pDoc->SetString(2, 2, 0, u"Paris"_ustr);
    m_pDoc->SetString(2, 3, 0, u"Peking"_ustr);

    sc::TextTransformation aTransform({2}, sc::TEXT_TRANSFORM_TYPE::TO_LOWER);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(u"berlin"_ustr, m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"brussels"_ustr, m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"paris"_ustr, m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"peking"_ustr, m_pDoc->GetString(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testTextToUpper)
{
    m_pDoc->SetString(2, 0, 0, u"Berlin"_ustr);
    m_pDoc->SetString(2, 1, 0, u"Brussels"_ustr);
    m_pDoc->SetString(2, 2, 0, u"Paris"_ustr);
    m_pDoc->SetString(2, 3, 0, u"Peking"_ustr);

    sc::TextTransformation aTransform({2}, sc::TEXT_TRANSFORM_TYPE::TO_UPPER);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(u"BERLIN"_ustr, m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"BRUSSELS"_ustr, m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"PARIS"_ustr, m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"PEKING"_ustr, m_pDoc->GetString(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testTextCapitalize)
{
    m_pDoc->SetString(2, 0, 0, u"hello woRlD"_ustr);
    m_pDoc->SetString(2, 1, 0, u"qUe vA"_ustr);
    m_pDoc->SetString(2, 2, 0, u"si tu la ves"_ustr);
    m_pDoc->SetString(2, 3, 0, u"cUaNdO mE EnAmOro"_ustr);

    sc::TextTransformation aTransform({2}, sc::TEXT_TRANSFORM_TYPE::CAPITALIZE);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(u"Hello World"_ustr, m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Que Va"_ustr, m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"Si Tu La Ves"_ustr, m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"Cuando Me Enamoro"_ustr, m_pDoc->GetString(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testTextTrim)
{
    m_pDoc->SetString(2, 0, 0, u" Berlin"_ustr);
    m_pDoc->SetString(2, 1, 0, u"Brussels "_ustr);
    m_pDoc->SetString(2, 2, 0, u" Paris "_ustr);

    sc::TextTransformation aTransform({2}, sc::TEXT_TRANSFORM_TYPE::TRIM);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(u"Berlin"_ustr, m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Brussels"_ustr, m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"Paris"_ustr, m_pDoc->GetString(2, 2, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testAggregateSum)
{
    m_pDoc->SetValue(2, 0, 0, 2034);
    m_pDoc->SetValue(2, 1, 0, 2342);
    m_pDoc->SetValue(2, 2, 0, 57452);

    m_pDoc->SetValue(4, 0, 0, 4829.98);
    m_pDoc->SetValue(4, 1, 0, 53781.3);
    m_pDoc->SetValue(4, 2, 0, 9876.4);
    m_pDoc->SetValue(4, 3, 0, 0);

    sc::AggregateFunction aTransform({2, 4}, sc::AGGREGATE_FUNCTION::SUM);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(61828, m_pDoc->GetValue(2, 4, 0), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(68487.68, m_pDoc->GetValue(4, 4, 0), 1e-10);
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testAggregateAverage)
{
    m_pDoc->SetValue(2, 0, 0, 2034);
    m_pDoc->SetValue(2, 1, 0, 2342);
    m_pDoc->SetValue(2, 2, 0, 57453);

    m_pDoc->SetValue(3, 0, 0, 4);
    m_pDoc->SetValue(3, 1, 0, 4);
    m_pDoc->SetValue(3, 2, 0, 4);

    sc::AggregateFunction aTransform({2, 3}, sc::AGGREGATE_FUNCTION::AVERAGE);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(20609.6666666667, m_pDoc->GetValue(2, 3, 0), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4, m_pDoc->GetValue(3, 3, 0), 1e-10);
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testAggregateMin)
{
    m_pDoc->SetValue(2, 0, 0, 2034);
    m_pDoc->SetValue(2, 1, 0, 2342);
    m_pDoc->SetValue(2, 2, 0, 57453);

    m_pDoc->SetValue(3, 0, 0, 2034);
    m_pDoc->SetValue(3, 1, 0, -2342);
    m_pDoc->SetValue(3, 2, 0, 57453);

    sc::AggregateFunction aTransform({2, 3}, sc::AGGREGATE_FUNCTION::MIN);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(2034, m_pDoc->GetValue(2, 3, 0), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-2342, m_pDoc->GetValue(3, 3, 0), 1e-10);
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testAggregateMax)
{
    m_pDoc->SetValue(2, 0, 0, 2034);
    m_pDoc->SetValue(2, 1, 0, 2342);
    m_pDoc->SetValue(2, 2, 0, 57453);
    m_pDoc->SetValue(2, 3, 0, -453);

    m_pDoc->SetValue(3, 0, 0, 2034);
    m_pDoc->SetValue(3, 1, 0, -2342);
    m_pDoc->SetValue(3, 2, 0, -57453);
    m_pDoc->SetValue(3, 3, 0, -453);

    sc::AggregateFunction aTransform({2, 3}, sc::AGGREGATE_FUNCTION::MAX);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(57453, m_pDoc->GetValue(2, 4, 0), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2034, m_pDoc->GetValue(3, 4, 0), 1e-10);
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testNumberRound)
{
    m_pDoc->SetValue(2, 0, 0, 2034.342453456);
    m_pDoc->SetValue(2, 1, 0, 2342.252678567542);
    m_pDoc->SetValue(2, 2, 0, 57453.651345687654345676);
    m_pDoc->SetValue(2, 3, 0, -453.22234567543);

    sc::NumberTransformation aTransform({2}, sc::NUMBER_TRANSFORM_TYPE::ROUND, 4);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(2034.3425, m_pDoc->GetValue(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(2342.2527, m_pDoc->GetValue(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(57453.6513, m_pDoc->GetValue(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(-453.2223, m_pDoc->GetValue(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testNumberRoundUp)
{
    m_pDoc->SetValue(2, 0, 0, 2034.34);
    m_pDoc->SetValue(2, 1, 0, 2342.22);
    m_pDoc->SetValue(2, 2, 0, 57453.65);
    m_pDoc->SetValue(2, 3, 0, -453.22);

    sc::NumberTransformation aTransform({2}, sc::NUMBER_TRANSFORM_TYPE::ROUND_UP);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(2035.0, m_pDoc->GetValue(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(2343.0, m_pDoc->GetValue(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(57454.0, m_pDoc->GetValue(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(-453.0, m_pDoc->GetValue(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testNumberRoundDown)
{
    m_pDoc->SetValue(2, 0, 0, 2034.34);
    m_pDoc->SetValue(2, 1, 0, 2342.22);
    m_pDoc->SetValue(2, 2, 0, 57453.65);
    m_pDoc->SetValue(2, 3, 0, -453.22);

    sc::NumberTransformation aTransform({2}, sc::NUMBER_TRANSFORM_TYPE::ROUND_DOWN);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(2034.0, m_pDoc->GetValue(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(2342.0, m_pDoc->GetValue(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(57453.0, m_pDoc->GetValue(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(-454.0, m_pDoc->GetValue(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testNumberAbsolute)
{
    m_pDoc->SetValue(2, 0, 0, 2034.34);
    m_pDoc->SetValue(2, 1, 0, -2342.22);
    m_pDoc->SetValue(2, 2, 0, 57453.65);
    m_pDoc->SetValue(2, 3, 0, -453.22);

    sc::NumberTransformation aTransform({2}, sc::NUMBER_TRANSFORM_TYPE::ABSOLUTE);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(2034.34, m_pDoc->GetValue(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(2342.22, m_pDoc->GetValue(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(57453.65, m_pDoc->GetValue(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(453.22, m_pDoc->GetValue(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testNumberLogE)
{
    m_pDoc->SetValue(2, 0, 0, 1);
    m_pDoc->SetValue(2, 1, 0, 5);
    m_pDoc->SetValue(2, 2, 0, -9);
    m_pDoc->SetValue(2, 3, 0, 500);

    sc::NumberTransformation aTransform({2}, sc::NUMBER_TRANSFORM_TYPE::LOG_E);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, m_pDoc->GetValue(2, 0, 0), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.60943791243, m_pDoc->GetValue(2, 1, 0), 1e-10);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.21460809842, m_pDoc->GetValue(2, 3, 0), 1e-10);
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testNumberLog10)
{
    m_pDoc->SetValue(2, 0, 0, 1);
    m_pDoc->SetValue(2, 1, 0, 10);
    m_pDoc->SetValue(2, 2, 0, -9);
    m_pDoc->SetValue(2, 3, 0, 500);

    sc::NumberTransformation aTransform({2}, sc::NUMBER_TRANSFORM_TYPE::LOG_10);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, m_pDoc->GetValue(2, 0, 0), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, m_pDoc->GetValue(2, 1, 0), 1e-10);
    CPPUNIT_ASSERT_EQUAL(OUString(), m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.69897000434, m_pDoc->GetValue(2, 3, 0), 1e-10);
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testNumberCube)
{
    m_pDoc->SetValue(2, 0, 0, 2);
    m_pDoc->SetValue(2, 1, 0, -2);
    m_pDoc->SetValue(2, 2, 0, 8);
    m_pDoc->SetValue(2, 3, 0, -8);

    sc::NumberTransformation aTransform({2}, sc::NUMBER_TRANSFORM_TYPE::CUBE);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(8.0, m_pDoc->GetValue(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(-8.0, m_pDoc->GetValue(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(512.0, m_pDoc->GetValue(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(-512.0, m_pDoc->GetValue(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testNumberSquare)
{
    m_pDoc->SetValue(2, 0, 0, 2);
    m_pDoc->SetValue(2, 1, 0, -2);
    m_pDoc->SetValue(2, 2, 0, 8);
    m_pDoc->SetValue(2, 3, 0, -8);

    sc::NumberTransformation aTransform({2}, sc::NUMBER_TRANSFORM_TYPE::SQUARE);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(64.0, m_pDoc->GetValue(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(64.0, m_pDoc->GetValue(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testNumberSquareRoot)
{
    m_pDoc->SetValue(2, 0, 0, 8);
    m_pDoc->SetValue(2, 1, 0, 4);
    m_pDoc->SetValue(2, 2, 0, 9);

    sc::NumberTransformation aTransform({2}, sc::NUMBER_TRANSFORM_TYPE::SQUARE_ROOT);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.82842712475, m_pDoc->GetValue(2, 0, 0), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, m_pDoc->GetValue(2, 1, 0), 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, m_pDoc->GetValue(2, 2, 0), 1e-10);
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testNumberEven)
{
    m_pDoc->SetValue(2, 0, 0, 2034);
    m_pDoc->SetValue(2, 1, 0, 2343);
    m_pDoc->SetValue(2, 2, 0, 57453.65);
    m_pDoc->SetValue(2, 3, 0, -453);

    sc::NumberTransformation aTransform({2}, sc::NUMBER_TRANSFORM_TYPE::IS_EVEN);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testNumberOdd)
{
    m_pDoc->SetValue(2, 0, 0, 2034);
    m_pDoc->SetValue(2, 1, 0, 2343);
    m_pDoc->SetValue(2, 2, 0, 57453.65);
    m_pDoc->SetValue(2, 3, 0, -453);

    sc::NumberTransformation aTransform({2}, sc::NUMBER_TRANSFORM_TYPE::IS_ODD);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testNumberSign)
{
    m_pDoc->SetValue(2, 0, 0, 2034.34);
    m_pDoc->SetValue(2, 1, 0, -2342.22);
    m_pDoc->SetValue(2, 2, 0, 0);
    m_pDoc->SetValue(2, 3, 0, -453.22);

    sc::NumberTransformation aTransform({2}, sc::NUMBER_TRANSFORM_TYPE::SIGN);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(-1.0, m_pDoc->GetValue(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(-1.0, m_pDoc->GetValue(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testReplaceNull)
{
    m_pDoc->SetString(2, 0, 0, u"Berlin"_ustr);
    m_pDoc->SetString(2, 1, 0, u""_ustr);
    m_pDoc->SetString(2, 2, 0, u""_ustr);
    m_pDoc->SetString(2, 3, 0, u"Peking"_ustr);

    sc::ReplaceNullTransformation aTransform({2}, u"Empty"_ustr);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(u"Berlin"_ustr, m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Empty"_ustr, m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"Empty"_ustr, m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"Peking"_ustr, m_pDoc->GetString(2, 3, 0));

}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testGetDateString)
{
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    css::util::Date aDate1(25,1,2011);
    css::util::Date aDate2(12,10,1994);
    css::util::Date aDate3(23,9,1996);
    css::util::Date aDate4(15,8,1947);

    double nDate1 = static_cast<double>(aDate1 - pFormatter->GetNullDate());
    double nDate2 = static_cast<double>(aDate2 - pFormatter->GetNullDate());
    double nDate3 = static_cast<double>(aDate3 - pFormatter->GetNullDate());
    double nDate4 = static_cast<double>(aDate4 - pFormatter->GetNullDate());

    m_pDoc->SetValue(2, 0, 0, nDate1);
    m_pDoc->SetValue(2, 1, 0, nDate2);
    m_pDoc->SetValue(2, 2, 0, nDate3);
    m_pDoc->SetValue(2, 3, 0, nDate4);

    sc:: DateTimeTransformation aTransform({2}, sc::DATETIME_TRANSFORMATION_TYPE::DATE_STRING   );
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(u"01/25/11"_ustr, m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"10/12/94"_ustr, m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"09/23/96"_ustr, m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"08/15/47"_ustr, m_pDoc->GetString(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testGetYear)
{
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    css::util::Date aDate1(25,1,2011);
    css::util::Date aDate2(12,10,1994);
    css::util::Date aDate3(23,9,1996);
    css::util::Date aDate4(15,8,1947);

    double nDate1 = static_cast<double>(aDate1 - pFormatter->GetNullDate());
    double nDate2 = static_cast<double>(aDate2 - pFormatter->GetNullDate());
    double nDate3 = static_cast<double>(aDate3 - pFormatter->GetNullDate());
    double nDate4 = static_cast<double>(aDate4 - pFormatter->GetNullDate());

    m_pDoc->SetValue(2, 0, 0, nDate1);
    m_pDoc->SetValue(2, 1, 0, nDate2);
    m_pDoc->SetValue(2, 2, 0, nDate3);
    m_pDoc->SetValue(2, 3, 0, nDate4);

    sc:: DateTimeTransformation aTransform({2}, sc::DATETIME_TRANSFORMATION_TYPE::YEAR   );
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(2011, m_pDoc->GetValue(2, 0, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1994, m_pDoc->GetValue(2, 1, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1996, m_pDoc->GetValue(2, 2, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1947, m_pDoc->GetValue(2, 3, 0), 0);
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testGetStartOfYear)
{
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    css::util::Date aDate1(25,1,2011);
    css::util::Date aDate2(12,10,1994);
    css::util::Date aDate3(23,9,1996);
    css::util::Date aDate4(15,8,1947);

    double nDate1 = static_cast<double>(aDate1 - pFormatter->GetNullDate());
    double nDate2 = static_cast<double>(aDate2 - pFormatter->GetNullDate());
    double nDate3 = static_cast<double>(aDate3 - pFormatter->GetNullDate());
    double nDate4 = static_cast<double>(aDate4 - pFormatter->GetNullDate());

    m_pDoc->SetValue(2, 0, 0, nDate1);
    m_pDoc->SetValue(2, 1, 0, nDate2);
    m_pDoc->SetValue(2, 2, 0, nDate3);
    m_pDoc->SetValue(2, 3, 0, nDate4);

    sc:: DateTimeTransformation aTransform({2}, sc::DATETIME_TRANSFORMATION_TYPE::START_OF_YEAR   );
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(u"01/01/11"_ustr, m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"01/01/94"_ustr, m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"01/01/96"_ustr, m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"01/01/47"_ustr, m_pDoc->GetString(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testGetEndOfYear)
{
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    css::util::Date aDate1(25,1,2011);
    css::util::Date aDate2(12,10,1994);
    css::util::Date aDate3(23,9,1996);
    css::util::Date aDate4(15,8,1947);

    double nDate1 = static_cast<double>(aDate1 - pFormatter->GetNullDate());
    double nDate2 = static_cast<double>(aDate2 - pFormatter->GetNullDate());
    double nDate3 = static_cast<double>(aDate3 - pFormatter->GetNullDate());
    double nDate4 = static_cast<double>(aDate4 - pFormatter->GetNullDate());

    m_pDoc->SetValue(2, 0, 0, nDate1);
    m_pDoc->SetValue(2, 1, 0, nDate2);
    m_pDoc->SetValue(2, 2, 0, nDate3);
    m_pDoc->SetValue(2, 3, 0, nDate4);

    sc:: DateTimeTransformation aTransform({2}, sc::DATETIME_TRANSFORMATION_TYPE::END_OF_YEAR   );
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(u"12/31/11"_ustr, m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"12/31/94"_ustr, m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"12/31/96"_ustr, m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"12/31/47"_ustr, m_pDoc->GetString(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testGetMonth)
{
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    css::util::Date aDate1(25,1,2011);
    css::util::Date aDate2(12,10,1994);
    css::util::Date aDate3(23,9,1996);
    css::util::Date aDate4(15,8,1947);

    double nDate1 = static_cast<double>(aDate1 - pFormatter->GetNullDate());
    double nDate2 = static_cast<double>(aDate2 - pFormatter->GetNullDate());
    double nDate3 = static_cast<double>(aDate3 - pFormatter->GetNullDate());
    double nDate4 = static_cast<double>(aDate4 - pFormatter->GetNullDate());

    m_pDoc->SetValue(2, 0, 0, nDate1);
    m_pDoc->SetValue(2, 1, 0, nDate2);
    m_pDoc->SetValue(2, 2, 0, nDate3);
    m_pDoc->SetValue(2, 3, 0, nDate4);

    sc:: DateTimeTransformation aTransform({2}, sc::DATETIME_TRANSFORMATION_TYPE::MONTH   );
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1, m_pDoc->GetValue(2, 0, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10, m_pDoc->GetValue(2, 1, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9, m_pDoc->GetValue(2, 2, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8, m_pDoc->GetValue(2, 3, 0), 0);
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testGetMonthName)
{
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    css::util::Date aDate1(25,1,2011);
    css::util::Date aDate2(12,10,1994);
    css::util::Date aDate3(23,9,1996);
    css::util::Date aDate4(15,8,1947);

    double nDate1 = static_cast<double>(aDate1 - pFormatter->GetNullDate());
    double nDate2 = static_cast<double>(aDate2 - pFormatter->GetNullDate());
    double nDate3 = static_cast<double>(aDate3 - pFormatter->GetNullDate());
    double nDate4 = static_cast<double>(aDate4 - pFormatter->GetNullDate());

    m_pDoc->SetValue(2, 0, 0, nDate1);
    m_pDoc->SetValue(2, 1, 0, nDate2);
    m_pDoc->SetValue(2, 2, 0, nDate3);
    m_pDoc->SetValue(2, 3, 0, nDate4);

    sc:: DateTimeTransformation aTransform({2}, sc::DATETIME_TRANSFORMATION_TYPE::MONTH_NAME);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(u"January"_ustr, m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"October"_ustr, m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"September"_ustr, m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"August"_ustr, m_pDoc->GetString(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testGetStartOfMonth)
{
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    css::util::Date aDate1(25,1,2011);
    css::util::Date aDate2(12,10,1994);
    css::util::Date aDate3(23,9,1996);
    css::util::Date aDate4(15,8,1947);

    double nDate1 = static_cast<double>(aDate1 - pFormatter->GetNullDate());
    double nDate2 = static_cast<double>(aDate2 - pFormatter->GetNullDate());
    double nDate3 = static_cast<double>(aDate3 - pFormatter->GetNullDate());
    double nDate4 = static_cast<double>(aDate4 - pFormatter->GetNullDate());

    m_pDoc->SetValue(2, 0, 0, nDate1);
    m_pDoc->SetValue(2, 1, 0, nDate2);
    m_pDoc->SetValue(2, 2, 0, nDate3);
    m_pDoc->SetValue(2, 3, 0, nDate4);

    sc:: DateTimeTransformation aTransform({2}, sc::DATETIME_TRANSFORMATION_TYPE::START_OF_MONTH   );
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(u"01/01/11"_ustr, m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"10/01/94"_ustr, m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"09/01/96"_ustr, m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"08/01/47"_ustr, m_pDoc->GetString(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testGetEndOfMonth)
{
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    css::util::Date aDate1(25,1,2011);
    css::util::Date aDate2(12,10,1994);
    css::util::Date aDate3(23,9,1996);
    css::util::Date aDate4(15,8,1947);

    double nDate1 = static_cast<double>(aDate1 - pFormatter->GetNullDate());
    double nDate2 = static_cast<double>(aDate2 - pFormatter->GetNullDate());
    double nDate3 = static_cast<double>(aDate3 - pFormatter->GetNullDate());
    double nDate4 = static_cast<double>(aDate4 - pFormatter->GetNullDate());

    m_pDoc->SetValue(2, 0, 0, nDate1);
    m_pDoc->SetValue(2, 1, 0, nDate2);
    m_pDoc->SetValue(2, 2, 0, nDate3);
    m_pDoc->SetValue(2, 3, 0, nDate4);

    sc:: DateTimeTransformation aTransform({2}, sc::DATETIME_TRANSFORMATION_TYPE::END_OF_MONTH   );
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(u"01/31/11"_ustr, m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"10/31/94"_ustr, m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"09/30/96"_ustr, m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"08/31/47"_ustr, m_pDoc->GetString(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testGetDay)
{
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    css::util::Date aDate1(25,1,2011);
    css::util::Date aDate2(12,10,1994);
    css::util::Date aDate3(23,9,1996);
    css::util::Date aDate4(15,8,1947);

    double nDate1 = static_cast<double>(aDate1 - pFormatter->GetNullDate());
    double nDate2 = static_cast<double>(aDate2 - pFormatter->GetNullDate());
    double nDate3 = static_cast<double>(aDate3 - pFormatter->GetNullDate());
    double nDate4 = static_cast<double>(aDate4 - pFormatter->GetNullDate());

    m_pDoc->SetValue(2, 0, 0, nDate1);
    m_pDoc->SetValue(2, 1, 0, nDate2);
    m_pDoc->SetValue(2, 2, 0, nDate3);
    m_pDoc->SetValue(2, 3, 0, nDate4);

    sc:: DateTimeTransformation aTransform({2}, sc::DATETIME_TRANSFORMATION_TYPE::DAY   );
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(25, m_pDoc->GetValue(2, 0, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12, m_pDoc->GetValue(2, 1, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(23, m_pDoc->GetValue(2, 2, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(15, m_pDoc->GetValue(2, 3, 0), 0);
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testGetDayOfWeek)
{
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    css::util::Date aDate1(25,1,2011);
    css::util::Date aDate2(12,10,1994);
    css::util::Date aDate3(23,9,1996);
    css::util::Date aDate4(15,8,1947);

    double nDate1 = static_cast<double>(aDate1 - pFormatter->GetNullDate());
    double nDate2 = static_cast<double>(aDate2 - pFormatter->GetNullDate());
    double nDate3 = static_cast<double>(aDate3 - pFormatter->GetNullDate());
    double nDate4 = static_cast<double>(aDate4 - pFormatter->GetNullDate());

    m_pDoc->SetValue(2, 0, 0, nDate1);
    m_pDoc->SetValue(2, 1, 0, nDate2);
    m_pDoc->SetValue(2, 2, 0, nDate3);
    m_pDoc->SetValue(2, 3, 0, nDate4);

    sc:: DateTimeTransformation aTransform({2}, sc::DATETIME_TRANSFORMATION_TYPE::DAY_OF_WEEK  );
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1, m_pDoc->GetValue(2, 0, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2, m_pDoc->GetValue(2, 1, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, m_pDoc->GetValue(2, 2, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4, m_pDoc->GetValue(2, 3, 0), 0);
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testGetDayOfYear)
{
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    css::util::Date aDate1(25,1,2011);
    css::util::Date aDate2(12,10,1994);
    css::util::Date aDate3(23,9,1996);
    css::util::Date aDate4(15,8,1947);

    double nDate1 = static_cast<double>(aDate1 - pFormatter->GetNullDate());
    double nDate2 = static_cast<double>(aDate2 - pFormatter->GetNullDate());
    double nDate3 = static_cast<double>(aDate3 - pFormatter->GetNullDate());
    double nDate4 = static_cast<double>(aDate4 - pFormatter->GetNullDate());

    m_pDoc->SetValue(2, 0, 0, nDate1);
    m_pDoc->SetValue(2, 1, 0, nDate2);
    m_pDoc->SetValue(2, 2, 0, nDate3);
    m_pDoc->SetValue(2, 3, 0, nDate4);

    sc:: DateTimeTransformation aTransform({2}, sc::DATETIME_TRANSFORMATION_TYPE::DAY_OF_YEAR  );
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(25, m_pDoc->GetValue(2, 0, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(285, m_pDoc->GetValue(2, 1, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(267, m_pDoc->GetValue(2, 2, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(227, m_pDoc->GetValue(2, 3, 0), 0);
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testGetQuarter)
{
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    css::util::Date aDate1(25,1,2011);
    css::util::Date aDate2(12,10,1994);
    css::util::Date aDate3(23,9,1996);
    css::util::Date aDate4(15,8,1947);

    double nDate1 = static_cast<double>(aDate1 - pFormatter->GetNullDate());
    double nDate2 = static_cast<double>(aDate2 - pFormatter->GetNullDate());
    double nDate3 = static_cast<double>(aDate3 - pFormatter->GetNullDate());
    double nDate4 = static_cast<double>(aDate4 - pFormatter->GetNullDate());

    m_pDoc->SetValue(2, 0, 0, nDate1);
    m_pDoc->SetValue(2, 1, 0, nDate2);
    m_pDoc->SetValue(2, 2, 0, nDate3);
    m_pDoc->SetValue(2, 3, 0, nDate4);

    sc:: DateTimeTransformation aTransform({2}, sc::DATETIME_TRANSFORMATION_TYPE::QUARTER   );
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1, m_pDoc->GetValue(2, 0, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4, m_pDoc->GetValue(2, 1, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3, m_pDoc->GetValue(2, 2, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3, m_pDoc->GetValue(2, 3, 0), 0);
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testGetStartOfQuarter)
{
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    css::util::Date aDate1(25,1,2011);
    css::util::Date aDate2(12,10,1994);
    css::util::Date aDate3(23,9,1996);
    css::util::Date aDate4(15,8,1947);

    double nDate1 = static_cast<double>(aDate1 - pFormatter->GetNullDate());
    double nDate2 = static_cast<double>(aDate2 - pFormatter->GetNullDate());
    double nDate3 = static_cast<double>(aDate3 - pFormatter->GetNullDate());
    double nDate4 = static_cast<double>(aDate4 - pFormatter->GetNullDate());

    m_pDoc->SetValue(2, 0, 0, nDate1);
    m_pDoc->SetValue(2, 1, 0, nDate2);
    m_pDoc->SetValue(2, 2, 0, nDate3);
    m_pDoc->SetValue(2, 3, 0, nDate4);

    sc:: DateTimeTransformation aTransform({2}, sc::DATETIME_TRANSFORMATION_TYPE::START_OF_QUARTER   );
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(u"01/01/11"_ustr, m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"10/01/94"_ustr, m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"07/01/96"_ustr, m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"07/01/47"_ustr, m_pDoc->GetString(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testGetEndOfQuarter)
{
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    css::util::Date aDate1(25,1,2011);
    css::util::Date aDate2(12,10,1994);
    css::util::Date aDate3(23,9,1996);
    css::util::Date aDate4(15,8,1947);

    double nDate1 = static_cast<double>(aDate1 - pFormatter->GetNullDate());
    double nDate2 = static_cast<double>(aDate2 - pFormatter->GetNullDate());
    double nDate3 = static_cast<double>(aDate3 - pFormatter->GetNullDate());
    double nDate4 = static_cast<double>(aDate4 - pFormatter->GetNullDate());

    m_pDoc->SetValue(2, 0, 0, nDate1);
    m_pDoc->SetValue(2, 1, 0, nDate2);
    m_pDoc->SetValue(2, 2, 0, nDate3);
    m_pDoc->SetValue(2, 3, 0, nDate4);

    sc:: DateTimeTransformation aTransform({2}, sc::DATETIME_TRANSFORMATION_TYPE::END_OF_QUARTER   );
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(u"03/31/11"_ustr, m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"12/31/94"_ustr, m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"09/30/96"_ustr, m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"09/30/47"_ustr, m_pDoc->GetString(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testGetTime)
{
    tools::Time aTime1(5,30,12);
    tools::Time aTime2(7,23,9);
    tools::Time aTime3(9,34,40);
    tools::Time aTime4(22,9,49);

    m_pDoc->SetValue(2, 0, 0, aTime1.GetTimeInDays());
    m_pDoc->SetValue(2, 1, 0, aTime2.GetTimeInDays());
    m_pDoc->SetValue(2, 2, 0, aTime3.GetTimeInDays());
    m_pDoc->SetValue(2, 3, 0, aTime4.GetTimeInDays());

    sc:: DateTimeTransformation aTransform({2}, sc::DATETIME_TRANSFORMATION_TYPE::TIME  );
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(u"05:30:12 AM"_ustr, m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"07:23:09 AM"_ustr, m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"09:34:40 AM"_ustr, m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"10:09:49 PM"_ustr, m_pDoc->GetString(2, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testGetHour)
{
    tools::Time aTime1(5,30,12);
    tools::Time aTime2(7,23,9);
    tools::Time aTime3(9,34,40);
    tools::Time aTime4(22,9,49);

    m_pDoc->SetValue(2, 0, 0, aTime1.GetTimeInDays());
    m_pDoc->SetValue(2, 1, 0, aTime2.GetTimeInDays());
    m_pDoc->SetValue(2, 2, 0, aTime3.GetTimeInDays());
    m_pDoc->SetValue(2, 3, 0, aTime4.GetTimeInDays());

    sc:: DateTimeTransformation aTransform({2}, sc::DATETIME_TRANSFORMATION_TYPE::HOUR  );
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(5, m_pDoc->GetValue(2, 0, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7, m_pDoc->GetValue(2, 1, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9, m_pDoc->GetValue(2, 2, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(22, m_pDoc->GetValue(2, 3, 0), 0);
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testGetMinute)
{
    tools::Time aTime1(5,30,12);
    tools::Time aTime2(7,23,9);
    tools::Time aTime3(9,34,40);
    tools::Time aTime4(22,9,49);

    m_pDoc->SetValue(2, 0, 0, aTime1.GetTimeInDays());
    m_pDoc->SetValue(2, 1, 0, aTime2.GetTimeInDays());
    m_pDoc->SetValue(2, 2, 0, aTime3.GetTimeInDays());
    m_pDoc->SetValue(2, 3, 0, aTime4.GetTimeInDays());

    sc:: DateTimeTransformation aTransform({2}, sc::DATETIME_TRANSFORMATION_TYPE::MINUTE  );
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(30, m_pDoc->GetValue(2, 0, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(23, m_pDoc->GetValue(2, 1, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(34, m_pDoc->GetValue(2, 2, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9, m_pDoc->GetValue(2, 3, 0), 0);
}

CPPUNIT_TEST_FIXTURE(ScDataTransformationTest, testGetSecond)
{
    tools::Time aTime1(5,30,53);
    tools::Time aTime2(7,23,10);
    tools::Time aTime3(9,34,40);
    tools::Time aTime4(22,9,49);

    m_pDoc->SetValue(2, 0, 0, aTime1.GetTimeInDays());
    m_pDoc->SetValue(2, 1, 0, aTime2.GetTimeInDays());
    m_pDoc->SetValue(2, 2, 0, aTime3.GetTimeInDays());
    m_pDoc->SetValue(2, 3, 0, aTime4.GetTimeInDays());

    sc:: DateTimeTransformation aTransform({2}, sc::DATETIME_TRANSFORMATION_TYPE::SECOND   );
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(53, m_pDoc->GetValue(2, 0, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10, m_pDoc->GetValue(2, 1, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(40, m_pDoc->GetValue(2, 2, 0), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(49, m_pDoc->GetValue(2, 3, 0), 0);
}

void ScDataTransformationTest::setUp()
{
    ScUcalcTestBase::setUp();
    m_pDoc->InsertTab(0, u"Tab"_ustr);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
