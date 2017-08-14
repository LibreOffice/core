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
#include "document.hxx"
#include <stringutil.hxx>
#include "address.hxx"
#include "dataprovider.hxx"
#include "datatransformation.hxx"
#include <vcl/scheduler.hxx>

#include <memory>

class ScDataTransformationTest : public ScBootstrapFixture
{
public:

    ScDataTransformationTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    void testColumnRemove();

    CPPUNIT_TEST_SUITE(ScDataTransformationTest);
    CPPUNIT_TEST(testColumnRemove);
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

    sc::ColumnRemoveTransformation aTransformation(5);
    aTransformation.Transform(*m_pDoc);

    for (SCROW nRow = 0; nRow < 10; ++nRow)
    {
        for (SCCOL nCol = 0; nCol < 9; ++nCol)
        {
            double nVal = m_pDoc->GetValue(nCol, nRow, 0);
            if (nCol < 5)
            {
                ASSERT_DOUBLES_EQUAL((double)(nCol)*nRow, nVal);
            }
            else
            {
                ASSERT_DOUBLES_EQUAL((double)(nCol+1)*nRow, nVal);
            }
        }
    }
}

ScDataTransformationTest::ScDataTransformationTest() :
    ScBootstrapFixture( "/sc/qa/unit/data/dataprovider" ),
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
