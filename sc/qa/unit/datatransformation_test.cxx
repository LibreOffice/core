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

    CPPUNIT_TEST_SUITE(ScDataTransformationTest);
    CPPUNIT_TEST(testColumnRemove);
    CPPUNIT_TEST(testColumnSplit);
    CPPUNIT_TEST(testColumnMerge);
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
