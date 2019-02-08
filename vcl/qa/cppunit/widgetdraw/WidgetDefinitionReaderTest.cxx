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
    vcl::WidgetDefinition aDefinition;

    CPPUNIT_ASSERT_EQUAL(OUString("000000"), aDefinition.maFaceColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(OUString("000000"), aDefinition.maCheckedColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(OUString("000000"), aDefinition.maLightColor.AsRGBHexString());

    vcl::WidgetDefinitionReader aReader(getFullUrl("definition1.xml"), getFullUrl(""));
    aReader.read(aDefinition);

    CPPUNIT_ASSERT_EQUAL(OUString("123456"), aDefinition.maFaceColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(OUString("234567"), aDefinition.maCheckedColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(OUString("345678"), aDefinition.maLightColor.AsRGBHexString());

    CPPUNIT_ASSERT_EQUAL(OUString("ffffff"), aDefinition.maVisitedLinkColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(OUString("ffffff"), aDefinition.maToolTextColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(OUString("ffffff"), aDefinition.maFontColor.AsRGBHexString());

    // Pushbutton
    {
        std::vector<std::shared_ptr<vcl::WidgetDefinitionState>> aStates
            = aDefinition.getDefinition(ControlType::Pushbutton, ControlPart::Entire)
                  ->getStates(ControlState::DEFAULT | ControlState::ENABLED
                                  | ControlState::ROLLOVER,
                              ImplControlValue());

        CPPUNIT_ASSERT_EQUAL(size_t(2), aStates.size());

        CPPUNIT_ASSERT_EQUAL(size_t(2), aStates[0]->mpDrawCommands.size());
        CPPUNIT_ASSERT_EQUAL(vcl::DrawCommandType::RECTANGLE,
                             aStates[0]->mpDrawCommands[0]->maType);
        CPPUNIT_ASSERT_EQUAL(vcl::DrawCommandType::CIRCLE, aStates[0]->mpDrawCommands[1]->maType);
    }

    // Radiobutton
    {
        std::vector<std::shared_ptr<vcl::WidgetDefinitionState>> aStates
            = aDefinition.getDefinition(ControlType::Radiobutton, ControlPart::Entire)
                  ->getStates(ControlState::NONE, ImplControlValue(ButtonValue::On));
        CPPUNIT_ASSERT_EQUAL(size_t(1), aStates.size());
        CPPUNIT_ASSERT_EQUAL(size_t(2), aStates[0]->mpDrawCommands.size());
    }

    {
        std::vector<std::shared_ptr<vcl::WidgetDefinitionState>> aStates
            = aDefinition.getDefinition(ControlType::Radiobutton, ControlPart::Entire)
                  ->getStates(ControlState::NONE, ImplControlValue(ButtonValue::Off));
        CPPUNIT_ASSERT_EQUAL(size_t(1), aStates.size());
        CPPUNIT_ASSERT_EQUAL(size_t(1), aStates[0]->mpDrawCommands.size());
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(WidgetDefinitionReaderTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
