/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <memory>
#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <wrtsh.hxx>
#include <drawdoc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <svx/svdpage.hxx>
#include <docmodel/uno/UnoThemeColor.hxx>

using namespace css;

class SwCoreThemeTest : public SwModelTestBase
{
public:
    SwCoreThemeTest()
        : SwModelTestBase("/sw/qa/core/theme/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwCoreThemeTest, testThemeColorInHeading)
{
    createSwDoc("ThemeColorInHeading.docx");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

    auto xThemeColor = getProperty<uno::Reference<util::XThemeColor>>(getParagraph(1),
                                                                      "CharColorThemeReference");
    model::ThemeColor aThemeColor;
    model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent1, aThemeColor.getType());
}

CPPUNIT_TEST_FIXTURE(SwCoreThemeTest, testDrawPageThemeExists)
{
    createSwDoc("ThemeColorInHeading.docx");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    model::Theme* pTheme = pPage->getSdrPageProperties().GetTheme();
    CPPUNIT_ASSERT(pTheme);
    CPPUNIT_ASSERT_EQUAL(OUString(u"Office Theme"), pTheme->GetName());

    model::ColorSet* pColorSet = pTheme->GetColorSet();
    CPPUNIT_ASSERT(pColorSet);
    CPPUNIT_ASSERT_EQUAL(OUString(u"Orange"), pColorSet->getName());

    CPPUNIT_ASSERT_EQUAL(Color(0xE48312), pTheme->GetColor(model::ThemeColorType::Accent1));
    CPPUNIT_ASSERT_EQUAL(Color(0xBD582C), pTheme->GetColor(model::ThemeColorType::Accent2));
    CPPUNIT_ASSERT_EQUAL(Color(0x865640), pTheme->GetColor(model::ThemeColorType::Accent3));
    CPPUNIT_ASSERT_EQUAL(Color(0x9B8357), pTheme->GetColor(model::ThemeColorType::Accent4));
    CPPUNIT_ASSERT_EQUAL(Color(0xC2BC80), pTheme->GetColor(model::ThemeColorType::Accent5));
    CPPUNIT_ASSERT_EQUAL(Color(0x94A088), pTheme->GetColor(model::ThemeColorType::Accent6));
    CPPUNIT_ASSERT_EQUAL(Color(0x000000), pTheme->GetColor(model::ThemeColorType::Dark1));
    CPPUNIT_ASSERT_EQUAL(Color(0x637052), pTheme->GetColor(model::ThemeColorType::Dark2));
    CPPUNIT_ASSERT_EQUAL(Color(0xFFFFFF), pTheme->GetColor(model::ThemeColorType::Light1));
    CPPUNIT_ASSERT_EQUAL(Color(0xCCDDEA), pTheme->GetColor(model::ThemeColorType::Light2));

    model::FontScheme const& rFontScheme = pTheme->getFontScheme();
    CPPUNIT_ASSERT_EQUAL(OUString(u"Calibri Light"), rFontScheme.getMajorLatin().maTypeface);
    CPPUNIT_ASSERT_EQUAL(OUString(u"Calibri"), rFontScheme.getMinorLatin().maTypeface);
    CPPUNIT_ASSERT_EQUAL(true, rFontScheme.getMajorAsian().maTypeface.isEmpty());
    CPPUNIT_ASSERT_EQUAL(true, rFontScheme.getMinorAsian().maTypeface.isEmpty());
    CPPUNIT_ASSERT_EQUAL(true, rFontScheme.getMajorComplex().maTypeface.isEmpty());
    CPPUNIT_ASSERT_EQUAL(true, rFontScheme.getMinorComplex().maTypeface.isEmpty());
    CPPUNIT_ASSERT_EQUAL(size_t(47), rFontScheme.getMajorSupplementalFontList().size());
    CPPUNIT_ASSERT_EQUAL(size_t(47), rFontScheme.getMinorSupplementalFontList().size());
    CPPUNIT_ASSERT_EQUAL(OUString(u"Angsana New"),
                         rFontScheme.findMajorSupplementalTypeface(u"Thai"));
    CPPUNIT_ASSERT_EQUAL(OUString(u"Cordia New"),
                         rFontScheme.findMinorSupplementalTypeface(u"Thai"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
