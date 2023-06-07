/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <o3tl/string_view.hxx>
#include <docmodel/uno/UnoComplexColor.hxx>
#include <docmodel/color/ComplexColor.hxx>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text")
    {
    }
};

DECLARE_SW_ROUNDTRIP_TEST(testThemePortionLevelCharColor_ODF,
                          "Test_ThemePortionLevel_CharColor.fodt", nullptr, Test)
{
    auto xParagraph = getParagraph(1);
    CPPUNIT_ASSERT(xParagraph.is());
    auto xComplexColor = getProperty<uno::Reference<util::XComplexColor>>(getRun(xParagraph, 1),
                                                                          "CharComplexColor");
    auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent2, aComplexColor.getSchemeType());
    auto const& rTransforms = aComplexColor.getTransformations();
    CPPUNIT_ASSERT_EQUAL(size_t(1), rTransforms.size());
    CPPUNIT_ASSERT_EQUAL(model::TransformationType::Tint, rTransforms[0].meType);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(4000), rTransforms[0].mnValue);
}

DECLARE_SW_ROUNDTRIP_TEST(testThemePortionLevelCharColor_DOCX,
                          "Test_ThemePortionLevel_CharColor.docx", nullptr, Test)
{
    auto xParagraph = getParagraph(1);
    CPPUNIT_ASSERT(xParagraph.is());
    auto xComplexColor = getProperty<uno::Reference<util::XComplexColor>>(getRun(xParagraph, 1),
                                                                          "CharComplexColor");
    auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent2, aComplexColor.getSchemeType());
    auto const& rTransforms = aComplexColor.getTransformations();
    CPPUNIT_ASSERT_EQUAL(size_t(1), rTransforms.size());
    CPPUNIT_ASSERT_EQUAL(model::TransformationType::Tint, rTransforms[0].meType);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(4000), rTransforms[0].mnValue);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
