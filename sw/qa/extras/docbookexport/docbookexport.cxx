/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <rtl/byteseq.hxx>

#include <swmodule.hxx>
#include <swdll.hxx>
#include <usrpref.hxx>

#include <test/htmltesttools.hxx>

class DocbookExportTest : public SwModelTestBase, public HtmlTestTools
{
private:
    FieldUnit m_eUnit;

public:
    DocbookExportTest() :
        SwModelTestBase("/sw/qa/extras/docbookexport/data/", "DocBook File"), //"HTML (StarWriter)"),  // the HTML filter works
        m_eUnit(FUNIT_NONE)
    {}

private:
    bool mustCalcLayoutOf(const char* filename) SAL_OVERRIDE
    {
        return true;
    }

    bool mustTestImportOf(const char* filename) const SAL_OVERRIDE
    {
        return true;
    }

};

#define DECLARE_DOCBOOKEXPORT_TEST(TestName, filename) DECLARE_SW_EXPORT_TEST(TestName, filename, DocbookExportTest)

DECLARE_DOCBOOKEXPORT_TEST(testsimple, "simple.docx")
{
    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
}

/* the test actually should fail
DECLARE_DOCBOOKEXPORT_TEST(testtdf91095, "tdf91095.docx")
{
    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
}
*/


CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
