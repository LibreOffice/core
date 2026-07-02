/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <algorithm>
#include <string_view>

#include <sfx2/objsh.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/BitmapReadAccess.hxx>

#include <StylesPreviewWindow.hxx>

using namespace ::com::sun::star;

namespace
{
class StylesPreviewWindowTest : public UnoApiTest
{
public:
    StylesPreviewWindowTest()
        : UnoApiTest(u"svx/qa/unit/data/"_ustr)
    {
    }
};

bool lcl_HasStyle(const StylePreviewList& rStyles, std::u16string_view rName)
{
    return std::any_of(rStyles.begin(), rStyles.end(),
                       [&rName](const StylePreviewDescriptor& r) { return r.commonName == rName; });
}

// Whether the rendered preview contains any non-background pixel, i.e. the
// sample text was actually drawn.
bool lcl_HasContent(const Bitmap& rBmp)
{
    Bitmap aBmp(rBmp);
    BitmapScopedReadAccess pAcc(aBmp);
    if (!pAcc)
        return false;
    for (tools::Long y = 0; y < pAcc->Height(); ++y)
        for (tools::Long x = 0; x < pAcc->Width(); ++x)
            if (pAcc->GetColor(y, x) != COL_WHITE)
                return true;
    return false;
}

// A paragraph style that carries an alias imported from DOCX is listed in the
// styles preview with the alias shown in place of the style name. The real
// style name stays as the id used to apply the style.
CPPUNIT_TEST_FIXTURE(StylesPreviewWindowTest, testStyleAliasAsPreviewName)
{
    loadFromFile(u"styleAliases.docx");

    SfxObjectShell* pDocShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT(pDocShell);

    const StylePreviewList aStyles
        = StylesPreviewWindow_Base::GetStyleList(pDocShell, StylePreviewList());

    const auto aHeading3
        = std::find_if(aStyles.begin(), aStyles.end(), [](const StylePreviewDescriptor& rStyle) {
              return rStyle.commonName == "Heading 3";
          });
    CPPUNIT_ASSERT_MESSAGE("Heading 3 is missing from the styles preview list",
                           aHeading3 != aStyles.end());

    // Without the fix the display name equalled the style name. With aliases
    // imported and honored, the alias is shown instead.
    CPPUNIT_ASSERT_EQUAL(u"Testskrift 1"_ustr, aHeading3->translatedName);

    // Recommended character styles are surfaced in the preview alongside paragraph
    // styles, tagged with the character family and their alias as the display name.
    const auto aEmphasis
        = std::find_if(aStyles.begin(), aStyles.end(), [](const StylePreviewDescriptor& rStyle) {
              return rStyle.commonName == "Emphasis";
          });
    CPPUNIT_ASSERT_MESSAGE("Character style Emphasis is missing from the preview",
                           aEmphasis != aStyles.end());
    CPPUNIT_ASSERT_EQUAL(SfxStyleFamily::Char, aEmphasis->eFamily);
    // Its alias is used as the display name, just like paragraph styles.
    CPPUNIT_ASSERT_EQUAL(u"Testskrift 2"_ustr, aEmphasis->translatedName);

    // A character style's preview draws its sample text (it must be looked up in the
    // character family, and falls back to the document default font when the style
    // itself sets none).
    const Bitmap aPreview = StylesPreviewWindow_Base::GetCachedPreview(
        { u"Emphasis"_ustr, u"Testskrift 2"_ustr, SfxStyleFamily::Char });
    CPPUNIT_ASSERT_MESSAGE("Character style preview is blank", lcl_HasContent(aPreview));
}

// The document sets the DOCX style pane filter to "Recommended" (visibleStyles).
// The preview then lists the recommended (qFormat) styles and hides plain custom
// ones that are neither recommended nor otherwise selected.
CPPUNIT_TEST_FIXTURE(StylesPreviewWindowTest, testStylePaneFilterRecommended)
{
    loadFromFile(u"stylePaneRecommended.docx");

    SfxObjectShell* pDocShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT(pDocShell);

    const StylePreviewList aStyles
        = StylesPreviewWindow_Base::GetStyleList(pDocShell, StylePreviewList());

    // The recommended (qFormat) paragraph and character styles are shown.
    CPPUNIT_ASSERT_MESSAGE("Recommended paragraph style missing from preview",
                           lcl_HasStyle(aStyles, u"Reco Style"));
    CPPUNIT_ASSERT_MESSAGE("Recommended character style missing from preview",
                           lcl_HasStyle(aStyles, u"Reco Char"));
    // Styles that are not recommended stay hidden, in both families - even the
    // in-use character style, which the character iterator would otherwise list.
    CPPUNIT_ASSERT_MESSAGE("Plain paragraph style should not appear under Recommended",
                           !lcl_HasStyle(aStyles, u"Plain Style"));
    CPPUNIT_ASSERT_MESSAGE("Plain character style should not appear under Recommended",
                           !lcl_HasStyle(aStyles, u"Plain Char"));
}

// A document with no style pane filter defaults to the Recommended view, and a
// recommended style that is marked semiHidden is kept out of that list.
CPPUNIT_TEST_FIXTURE(StylesPreviewWindowTest, testStylePaneFilterAbsentAndSemiHidden)
{
    loadFromFile(u"stylePaneAbsentFilter.docx");

    SfxObjectShell* pDocShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT(pDocShell);

    const StylePreviewList aStyles
        = StylesPreviewWindow_Base::GetStyleList(pDocShell, StylePreviewList());

    // No filter in the document, yet the recommended style is shown.
    CPPUNIT_ASSERT_MESSAGE("Recommended style missing with no filter present",
                           lcl_HasStyle(aStyles, u"Visible Reco"));
    // semiHidden keeps a recommended style out of the list.
    CPPUNIT_ASSERT_MESSAGE("semiHidden style should be hidden from Recommended",
                           !lcl_HasStyle(aStyles, u"Hidden Reco"));
    // A non-recommended style is not shown either.
    CPPUNIT_ASSERT_MESSAGE("Non-recommended style should not appear",
                           !lcl_HasStyle(aStyles, u"Plain One"));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
