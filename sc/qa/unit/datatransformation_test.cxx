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
#include <stringutil.hxx>
#include <address.hxx>
#include <dataprovider.hxx>
#include <datatransformation.hxx>
#include <vcl/scheduler.hxx>

#include <memory>

class ScDataTransformationTest : public ScBootstrapFixture
{
public:

    ScDataTransformationTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    void testColumnRemove();
    void testColumnSplit();
    void testColumnMerge();
    void testTextToLower();
    void testTextToUpper();
    void testTextCapitalize();
    void testTextTrim();

    CPPUNIT_TEST_SUITE(ScDataTransformationTest);
    CPPUNIT_TEST(testColumnRemove);
    CPPUNIT_TEST(testColumnSplit);
    CPPUNIT_TEST(testColumnMerge);
    CPPUNIT_TEST(testTextToLower);
    CPPUNIT_TEST(testTextToUpper);
    CPPUNIT_TEST(testTextCapitalize);
    CPPUNIT_TEST(testTextTrim);
    CPPUNIT_TEST_SUITE_END();

private:
    ScDocShellRef m_xDocShell;
    ScDocument *m_pDoc;
};

void ScDataTransformationTest::testColumnRemove()
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

void ScDataTransformationTest::testColumnSplit()
{
    m_pDoc->SetString(2, 0, 0, "Test1,Test2");
    m_pDoc->SetString(2, 1, 0, "Test1,");
    m_pDoc->SetString(2, 2, 0, ",Test1");
    m_pDoc->SetString(2, 3, 0, "Test1,Test2,Test3");
    m_pDoc->SetString(3, 0, 0, "AnotherString");

    sc::SplitColumnTransformation aTransform(2, ',');
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("AnotherString"), m_pDoc->GetString(4, 0, 0));

    CPPUNIT_ASSERT_EQUAL(OUString("Test1"), m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Test1"), m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString(""), m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Test1"), m_pDoc->GetString(2, 3, 0));

    CPPUNIT_ASSERT_EQUAL(OUString("Test2"), m_pDoc->GetString(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString(""), m_pDoc->GetString(3, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Test1"), m_pDoc->GetString(3, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Test2,Test3"), m_pDoc->GetString(3, 3, 0));
}

void ScDataTransformationTest::testColumnMerge()
{
    m_pDoc->SetString(2, 0, 0, "Berlin");
    m_pDoc->SetString(2, 1, 0, "Brussels");
    m_pDoc->SetString(2, 2, 0, "Paris");
    m_pDoc->SetString(2, 3, 0, "Peking");

    m_pDoc->SetString(4, 0, 0, "Germany");
    m_pDoc->SetString(4, 1, 0, "Belgium");
    m_pDoc->SetString(4, 2, 0, "France");
    m_pDoc->SetString(4, 3, 0, "China");

    sc::MergeColumnTransformation aTransform({2, 4}, ", ");
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("Berlin, Germany"), m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Brussels, Belgium"), m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Paris, France"), m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Peking, China"), m_pDoc->GetString(2, 3, 0));

    for (SCROW nRow = 0; nRow <= 3; ++nRow)
    {
        CPPUNIT_ASSERT(m_pDoc->GetString(4, nRow, 0).isEmpty());
    }
}

void ScDataTransformationTest::testTextToLower()
{
    m_pDoc->SetString(2, 0, 0, "Berlin");
    m_pDoc->SetString(2, 1, 0, "Brussels");
    m_pDoc->SetString(2, 2, 0, "Paris");
    m_pDoc->SetString(2, 3, 0, "Peking");

    sc::TextTransformation aTransform(2, sc::TEXT_TRANSFORM_TYPE::TO_LOWER);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("berlin"), m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("brussels"), m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("paris"), m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("peking"), m_pDoc->GetString(2, 3, 0));
}

void ScDataTransformationTest::testTextToUpper()
{
    m_pDoc->SetString(2, 0, 0, "Berlin");
    m_pDoc->SetString(2, 1, 0, "Brussels");
    m_pDoc->SetString(2, 2, 0, "Paris");
    m_pDoc->SetString(2, 3, 0, "Peking");

    sc::TextTransformation aTransform(2, sc::TEXT_TRANSFORM_TYPE::TO_UPPER);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("BERLIN"), m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("BRUSSELS"), m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("PARIS"), m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("PEKING"), m_pDoc->GetString(2, 3, 0));
}

void ScDataTransformationTest::testTextCapitalize()
{
    m_pDoc->SetString(2, 0, 0, "hello woRlD");
    m_pDoc->SetString(2, 1, 0, "qUe vA");
    m_pDoc->SetString(2, 2, 0, "si tu la ves");
    m_pDoc->SetString(2, 3, 0, "cUaNdO mE EnAmOro");

    sc::TextTransformation aTransform(2, sc::TEXT_TRANSFORM_TYPE::CAPITALIZE);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("Hello World"), m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Que Va"), m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Si Tu La Ves"), m_pDoc->GetString(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Cuando Me Enamoro"), m_pDoc->GetString(2, 3, 0));
}

void ScDataTransformationTest::testTextTrim()
{
    m_pDoc->SetString(2, 0, 0, " Berlin");
    m_pDoc->SetString(2, 1, 0, "Brussels ");
    m_pDoc->SetString(2, 2, 0, " Paris ");

    sc::TextTransformation aTransform(2, sc::TEXT_TRANSFORM_TYPE::TRIM);
    aTransform.Transform(*m_pDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("Berlin"), m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Brussels"), m_pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Paris"), m_pDoc->GetString(2, 2, 0));
}

ScDataTransformationTest::ScDataTransformationTest() :
    ScBootstrapFixture( "sc/qa/unit/data/dataprovider" ),
    m_pDoc(nullptr)
{
}

void ScDataTransformationTest::setUp()
{
    ScBootstrapFixture::setUp();

    ScDLL::Init();
    m_xDocShell = new ScDocShell(
        SfxModelFlags::EMBEDDED_OBJECT |
        SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
        SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);

    m_xDocShell->SetIsInUcalc();
    m_xDocShell->DoInitUnitTest();
    m_pDoc = &m_xDocShell->GetDocument();
    m_pDoc->InsertTab(0, "Tab");
}

void ScDataTransformationTest::tearDown()
{
    m_xDocShell->DoClose();
    m_xDocShell.clear();
    ScBootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDataTransformationTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
