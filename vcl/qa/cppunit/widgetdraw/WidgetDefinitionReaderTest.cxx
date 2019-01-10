/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <unotest/bootstrapfixturebase.hxx>

#include <widgetdraw/WidgetDefinitionReader.hxx>

namespace
{
static OUString const gaDataUrl("/vcl/qa/cppunit/widgetdraw/data/");

class WidgetDefinitionReaderTest : public test::BootstrapFixtureBase
{
private:
    OUString getFullUrl(const OUString& sFileName)
    {
        return m_directories.getURLFromSrc(gaDataUrl) + sFileName;
    }

public:
    void testRead();

    CPPUNIT_TEST_SUITE(WidgetDefinitionReaderTest);
    CPPUNIT_TEST(testRead);
    CPPUNIT_TEST_SUITE_END();
};

void WidgetDefinitionReaderTest::testRead()
{
    vcl::WidgetDefinitionReader aWidgetDefinitionReader(getFullUrl("definition1.xml"));

    CPPUNIT_ASSERT_EQUAL(OUString("000000"), aWidgetDefinitionReader.maFaceColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(OUString("000000"),
                         aWidgetDefinitionReader.maCheckedColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(OUString("000000"), aWidgetDefinitionReader.maLightColor.AsRGBHexString());

    aWidgetDefinitionReader.read();

    CPPUNIT_ASSERT_EQUAL(OUString("f7f7f7"), aWidgetDefinitionReader.maFaceColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(OUString("c0c0c0"),
                         aWidgetDefinitionReader.maCheckedColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(OUString("ffffff"), aWidgetDefinitionReader.maLightColor.AsRGBHexString());
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(WidgetDefinitionReaderTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
