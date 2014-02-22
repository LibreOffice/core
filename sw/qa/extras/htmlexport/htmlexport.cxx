/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <swmodeltestbase.hxx>

#if !defined(MACOSX) && !defined(WNT)

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <swmodule.hxx>
#include <usrpref.hxx>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/htmlexport/data/", "HTML (StarWriter)"),
        m_eUnit(FUNIT_NONE)
    {
    }

private:
    bool mustCalcLayoutOf(const char* filename) SAL_OVERRIDE
    {
        return OString(filename) != "fdo62336.docx";
    }

    bool mustTestImportOf(const char* filename) const SAL_OVERRIDE
    {
        return OString(filename) != "fdo62336.docx";
    }

    void preTest(const char* filename) SAL_OVERRIDE
    {
        if (OString(filename) == "charborder.odt" && SW_MOD())
        {
            
            SwMasterUsrPref* pPref = const_cast<SwMasterUsrPref*>(SW_MOD()->GetUsrPref(false));
            m_eUnit = pPref->GetMetric();
            pPref->SetMetric(FUNIT_CM);
        }
    }

    void postTest(const char* filename) SAL_OVERRIDE
    {
        if (OString(filename) == "charborder.odt" && SW_MOD())
        {
            SwMasterUsrPref* pPref = const_cast<SwMasterUsrPref*>(SW_MOD()->GetUsrPref(false));
            pPref->SetMetric(m_eUnit);
        }
    }

    FieldUnit m_eUnit;
};

#define DECLARE_HTMLEXPORT_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, Test)

DECLARE_HTMLEXPORT_TEST(testFdo62336, "fdo62336.docx")
{
    
    
}

DECLARE_HTMLEXPORT_TEST(testCharacterBorder, "charborder.odt")
{

    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1),1), uno::UNO_QUERY);
    
    {
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0x6666FF,12,12,12,3,37), getProperty<table::BorderLine2>(xRun,"CharTopBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF9900,0,99,0,2,99), getProperty<table::BorderLine2>(xRun,"CharLeftBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF0000,0,169,0,1,169), getProperty<table::BorderLine2>(xRun,"CharBottomBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0x0000FF,0,169,0,0,169), getProperty<table::BorderLine2>(xRun,"CharRightBorder"));
    }

    
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(450), getProperty<sal_Int32>(xRun,"CharTopBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(550), getProperty<sal_Int32>(xRun,"CharLeftBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xRun,"CharBottomBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(250), getProperty<sal_Int32>(xRun,"CharRightBorderDistance"));
    }

    
}

#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
