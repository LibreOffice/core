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
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>

#include <pam.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text")
    {
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf153909_followTextFlow, "tdf153909_followTextFlow.docx")
{
    // Although MSO's UI reports "layoutInCell" for the rectangle, it isn't specified or honored
    CPPUNIT_ASSERT(!getProperty<bool>(getShape(1), "IsFollowingTextFlow"));

    // Given a table with a rectangle anchored in it (wrap-through) that appears above the table...
    xmlDocUniquePtr pDump = parseLayoutDump();
    sal_Int32 nRectBottom
        = getXPath(pDump, "//anchored/SwAnchoredDrawObject/bounds"_ostr, "bottom"_ostr).toInt32();
    sal_Int32 nTableTop = getXPath(pDump, "//tab/row/infos/bounds"_ostr, "top"_ostr).toInt32();
    // The entire table must be below the rectangle
    CPPUNIT_ASSERT(nTableTop > nRectBottom);

    // pre-emptive test: rectangle "margin" offset against cell, not outside-table-paragraph.
    // Since layoutInCell is true (as a non-defined default), the cell is the fly reference, thus
    // the rectangle should start at the paper's edge, 1.3cm to the left of the start of the table.
    sal_Int32 nRectLeft
        = getXPath(pDump, "//anchored/SwAnchoredDrawObject/bounds"_ostr, "left"_ostr).toInt32();
    sal_Int32 nTableLeft = getXPath(pDump, "//tab/row/infos/bounds"_ostr, "left"_ostr).toInt32();
    CPPUNIT_ASSERT(nTableLeft > nRectLeft);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
