/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <comphelper/propertyvalue.hxx>

namespace
{
/// Covers sw/source/core/doc/number.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/doc/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testBadHeadingIndent)
{
    // Given a DOCX file with a single paragraph, no heading styles present:
    createSwDoc("bad-heading-indent.docx");

    // When marking that paragraph heading 1:
    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue(u"Style"_ustr, uno::Any(u"Heading 1"_ustr)),
        comphelper::makePropertyValue(u"FamilyName"_ustr, uno::Any(u"ParagraphStyles"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues);

    // Then make sure that doesn't result in unexpected indent:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    int nTabCount
        = getXPathContent(pXmlDoc, "count(//SwLineLayout/child::*[@type='PortionType::TabLeft'])")
              .toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. an unexpected tab portion was inserted before the paragraph text.
    CPPUNIT_ASSERT_EQUAL(0, nTabCount);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
