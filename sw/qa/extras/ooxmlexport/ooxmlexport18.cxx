/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <queue>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/util/XRefreshable.hpp>


#include <comphelper/propertysequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <o3tl/string_view.hxx>
#include <comphelper/propertyvalue.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/extras/ooxmlexport/data/";

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(DATA_DIRECTORY, "Office Open XML Text") {}
};

CPPUNIT_TEST_FIXTURE(Test, testTdf150197_predefinedNumbering)
{
    createSwDoc();

    // The exact numbering style doesn't matter - just any non-bullet pre-defined numbering style.
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(OUString("Numbering 123")) },
        { "FamilyName", uno::Any(OUString("NumberingStyles")) },
    });
    dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(getParagraph(1), "ListLabelString"));

    reload("Office Open XML Text", "");
    CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(getParagraph(1), "ListLabelString"));
}

CPPUNIT_TEST_FIXTURE(Test, testInlineSdtHeader)
{
    // Without the accompanying fix in place, this test would have failed with an assertion failure,
    // we produced not-well-formed XML on save.
    loadAndSave("inline-sdt-header.docx");
}

CPPUNIT_TEST_FIXTURE(Test, testCellSdtRedline)
{
    // Without the accompanying fix in place, this test would have failed with an assertion failure,
    // we produced not-well-formed XML on save.
    loadAndSave("cell-sdt-redline.docx");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
