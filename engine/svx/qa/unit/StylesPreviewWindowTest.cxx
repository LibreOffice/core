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

#include <sfx2/objsh.hxx>

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
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
