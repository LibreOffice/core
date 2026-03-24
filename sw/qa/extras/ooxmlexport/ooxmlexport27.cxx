/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertyState.hpp>

#include <pam.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <IDocumentSettingAccess.hxx>
#include <wrtsh.hxx>

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf124398_groupshapeChart)
{
    // given a document with grouped chart and textbox
    // TODO: seeing the chart is broken since 24.8.4

    createSwDoc("tdf124398_groupshapeChart.docx");

    // MS Word considered this document to be corrupt for several reasons.
    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // DOCX/wps: doesn't use nvGraphicFramePr - only PPTX/p: and XLSX/xdr: do.
    assertXPath(pXmlDoc, "//wpg:graphicFrame/wpg:cNvPr", 1);
    assertXPath(pXmlDoc, "//wpg:graphicFrame/wpg:xfrm", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf171433_equation)
{
    // given a document with formula
    createSwDoc("tdf171433_equation.docx");

    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the fix, the formula was completely broken - missing SubSup entirely
    assertXPath(pXmlDoc, "//m:sSubSup/m:sup", 1);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
