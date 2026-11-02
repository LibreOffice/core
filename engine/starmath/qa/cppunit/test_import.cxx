/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

#include <sfx2/sfxbasemodel.hxx>

#include <document.hxx>
#include <smdll.hxx>

using namespace ::com::sun::star;

namespace
{
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"starmath/qa/cppunit/data/"_ustr)
    {
    }

    void testFontStyles();
    void testFormatChangeSurvivesPropertySet();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testFontStyles);
    CPPUNIT_TEST(testFormatChangeSurvivesPropertySet);
    CPPUNIT_TEST_SUITE_END();
};

void Test::testFontStyles()
{
    // tdf#143213
    loadFromFile(u"font-styles.odf");
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());

    const SmFormat& aFormat = pDocShell->GetFormat();

    CPPUNIT_ASSERT_EQUAL(ITALIC_NORMAL, aFormat.GetFont(FNT_MATH).GetItalic());
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, aFormat.GetFont(FNT_MATH).GetWeight());
    CPPUNIT_ASSERT_EQUAL(aFormat.GetBaseSize().Height(),
                         aFormat.GetFont(FNT_MATH).GetFontSize().Height());

    CPPUNIT_ASSERT_EQUAL(ITALIC_NORMAL, aFormat.GetFont(FNT_VARIABLE).GetItalic());
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, aFormat.GetFont(FNT_VARIABLE).GetWeight());
    CPPUNIT_ASSERT_EQUAL(aFormat.GetBaseSize().Height(),
                         aFormat.GetFont(FNT_VARIABLE).GetFontSize().Height());

    CPPUNIT_ASSERT_EQUAL(ITALIC_NORMAL, aFormat.GetFont(FNT_FUNCTION).GetItalic());
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, aFormat.GetFont(FNT_FUNCTION).GetWeight());
    CPPUNIT_ASSERT_EQUAL(aFormat.GetBaseSize().Height(),
                         aFormat.GetFont(FNT_FUNCTION).GetFontSize().Height());

    CPPUNIT_ASSERT_EQUAL(ITALIC_NORMAL, aFormat.GetFont(FNT_NUMBER).GetItalic());
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, aFormat.GetFont(FNT_NUMBER).GetWeight());
    CPPUNIT_ASSERT_EQUAL(aFormat.GetBaseSize().Height(),
                         aFormat.GetFont(FNT_NUMBER).GetFontSize().Height());

    CPPUNIT_ASSERT_EQUAL(ITALIC_NORMAL, aFormat.GetFont(FNT_TEXT).GetItalic());
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, aFormat.GetFont(FNT_TEXT).GetWeight());
    CPPUNIT_ASSERT_EQUAL(aFormat.GetBaseSize().Height(),
                         aFormat.GetFont(FNT_TEXT).GetFontSize().Height());

    CPPUNIT_ASSERT_EQUAL(ITALIC_NONE, aFormat.GetFont(FNT_SERIF).GetItalic());
    CPPUNIT_ASSERT_EQUAL(WEIGHT_NORMAL, aFormat.GetFont(FNT_SERIF).GetWeight());
    CPPUNIT_ASSERT_EQUAL(aFormat.GetBaseSize().Height(),
                         aFormat.GetFont(FNT_SERIF).GetFontSize().Height());

    CPPUNIT_ASSERT_EQUAL(ITALIC_NONE, aFormat.GetFont(FNT_SANS).GetItalic());
    CPPUNIT_ASSERT_EQUAL(WEIGHT_NORMAL, aFormat.GetFont(FNT_SANS).GetWeight());
    CPPUNIT_ASSERT_EQUAL(aFormat.GetBaseSize().Height(),
                         aFormat.GetFont(FNT_SANS).GetFontSize().Height());

    CPPUNIT_ASSERT_EQUAL(ITALIC_NONE, aFormat.GetFont(FNT_FIXED).GetItalic());
    CPPUNIT_ASSERT_EQUAL(WEIGHT_NORMAL, aFormat.GetFont(FNT_FIXED).GetWeight());
    CPPUNIT_ASSERT_EQUAL(aFormat.GetBaseSize().Height(),
                         aFormat.GetFont(FNT_FIXED).GetFontSize().Height());
}

void Test::testFormatChangeSurvivesPropertySet()
{
    // A formula that names its fonts in the settings, changed afterwards in the Font Size and the
    // Fonts dialogs, which set the format directly
    loadFromFile(u"font-styles.odf");
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());

    SmFormat aFormat = pDocShell->GetFormat();
    Size aSize = aFormat.GetBaseSize();
    aSize.setHeight(aSize.Height() * 2);
    aFormat.SetBaseSizeAndFonts(aSize);
    SmFace aVariable(aFormat.GetFont(FNT_VARIABLE));
    aVariable.SetItalic(ITALIC_NONE);
    aFormat.SetFont(FNT_VARIABLE, aVariable);
    pDocShell->SetFormat(aFormat);

    // Saving the formula sets the window state on the model
    auto xProps = mxComponent.queryThrow<beans::XPropertySet>();
    xProps->setPropertyValue(u"WindowState"_ustr, cpo::uno::Any(OUString()));

    const SmFormat& rFormat = pDocShell->GetFormat();
    // Without the fix in place, this was the font size from the file, half the base size
    CPPUNIT_ASSERT_EQUAL(aSize.Height(), rFormat.GetFont(FNT_MATH).GetFontSize().Height());
    CPPUNIT_ASSERT_EQUAL(aSize.Height(), rFormat.GetFont(FNT_VARIABLE).GetFontSize().Height());
    // Without the fix in place, the italic from the file came back
    CPPUNIT_ASSERT_EQUAL(ITALIC_NONE, rFormat.GetFont(FNT_VARIABLE).GetItalic());
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
