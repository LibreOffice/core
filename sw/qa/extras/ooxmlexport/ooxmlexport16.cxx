/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>

OUStringLiteral const DATA_DIRECTORY = u"/sw/qa/extras/ooxmlexport/data/";

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(DATA_DIRECTORY, "Office Open XML Text") {}

protected:
    /**
     * Denylist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf138892_noNumbering, "tdf138892_noNumbering.docx")
{
    CPPUNIT_ASSERT_MESSAGE("Para1: Bullet point", !getProperty<OUString>(getParagraph(1), "NumberingStyleName").isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Para2: <blank line>", getProperty<OUString>(getParagraph(2), "NumberingStyleName").isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Para3: <blank line>", getProperty<OUString>(getParagraph(3), "NumberingStyleName").isEmpty());
}

DECLARE_OOXMLEXPORT_TEST(testTdf136059, "tdf136059.odt")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Contour has not been exported!", true,
                                 getProperty<bool>(getShape(1), "SurroundContour"));
    // With the fix this shall pass, see tdf136059.
}

DECLARE_OOXMLEXPORT_TEST(testTdf138953, "croppedAndRotated.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Make sure the rotation is exported correctly, and size not distorted
    auto xShape(getShape(1));
    CPPUNIT_ASSERT_EQUAL(27000.0, getProperty<double>(xShape, "RotateAngle"));
    auto frameRect = getProperty<css::awt::Rectangle>(xShape, "FrameRect");
    // Before the fix, original object size (i.e., before cropping) was written to spPr in OOXML,
    // and the resulting object size was much larger than should be.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12961), frameRect.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8664), frameRect.Width);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
