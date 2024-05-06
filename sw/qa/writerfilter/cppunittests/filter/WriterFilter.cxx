/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/ColorMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/qa/XDumper.hpp>

using namespace ::com::sun::star;

namespace
{
/// Tests for sw/source/writerfilter/filter/WriterFilter.cxx.
class Test : public UnoApiXmlTest
{
public:
    Test()
        : UnoApiXmlTest("/sw/qa/writerfilter/cppunittests/filter/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testDoNotMirrorRtlDrawObjs)
{
    // Given a document with a shape, anchored in an RTL paragraph:
    // When loading that document:
    loadFromFile(u"draw-obj-rtl-no-mirror.docx");

    // Then make sure the shape is on the right margin:
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    css::uno::Reference<qa::XDumper> xDumper(xModel->getCurrentController(), uno::UNO_QUERY);
    OString aDump = xDumper->dump("layout").toUtf8();
    auto pCharBuffer = reinterpret_cast<const xmlChar*>(aDump.getStr());
    xmlDocUniquePtr pXmlDoc(xmlParseDoc(pCharBuffer));
    sal_Int32 nBodyRight = getXPath(pXmlDoc, "//body/infos/bounds"_ostr, "right"_ostr).toInt32();
    sal_Int32 nShapeLeft
        = getXPath(pXmlDoc, "//SwAnchoredDrawObject/bounds"_ostr, "left"_ostr).toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 11083
    // - Actual  : 722
    // i.e. the shape was on the left margin.
    CPPUNIT_ASSERT_GREATER(nBodyRight, nShapeLeft);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
