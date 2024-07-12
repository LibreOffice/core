/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <vcl/embeddedfontshelper.hxx>

using namespace com::sun::star;

namespace
{
/// Tests for sw/source/writerfilter/dmapper/FontTable.cxx.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"/writerfilter/qa/cppunittests/dmapper/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testSubsettedEmbeddedFont)
{
    // Given a document with a subsetted embedded font, loaded for editing:
    loadFromFile(u"subsetted-embedded-font.docx");

    // When checking if the font is available:
    OUString aUrl = EmbeddedFontsHelper::fontFileUrl(
        u"IBM Plex Serif Light", FAMILY_ROMAN, ITALIC_NONE, WEIGHT_NORMAL, PITCH_VARIABLE,
        EmbeddedFontsHelper::FontRights::ViewingAllowed);

    // Then make sure the subsetted font is not available, given that the newly inserted characters
    // during editing may be missing from the subsetted font:
    CPPUNIT_ASSERT(aUrl.isEmpty());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
