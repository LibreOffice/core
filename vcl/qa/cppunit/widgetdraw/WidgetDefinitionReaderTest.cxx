/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <unotest/bootstrapfixturebase.hxx>

#include <widgetdraw/WidgetDefinitionReader.hxx>

namespace
{
constexpr OUStringLiteral gaDataUrl(u"/vcl/qa/cppunit/widgetdraw/data/");

class WidgetDefinitionReaderTest : public test::BootstrapFixtureBase
{
private:
    OUString getFullUrl(std::u16string_view sFileName)
    {
        return m_directories.getURLFromSrc(gaDataUrl) + sFileName;
    }

public:
    void testRead();
    void testReadSettings();

    CPPUNIT_TEST_SUITE(WidgetDefinitionReaderTest);
    CPPUNIT_TEST(testRead);
    CPPUNIT_TEST(testReadSettings);
    CPPUNIT_TEST_SUITE_END();
};

void WidgetDefinitionReaderTest::testReadSettings()
{
    {
        vcl::WidgetDefinition aDefinition;
        vcl::WidgetDefinitionReader aReader(getFullUrl(u"definitionSettings1.xml"),
                                            getFullUrl(u""));
        CPPUNIT_ASSERT(aReader.read(aDefinition));
        CPPUNIT_ASSERT_EQUAL(""_ostr, aDefinition.mpSettings->msCenteredTabs);
    }

    {
        vcl::WidgetDefinition aDefinition;
        vcl::WidgetDefinitionReader aReader(getFullUrl(u"definitionSettings2.xml"),
                                            getFullUrl(u""));
        CPPUNIT_ASSERT(aReader.read(aDefinition));
        CPPUNIT_ASSERT_EQUAL("true"_ostr, aDefinition.mpSettings->msCenteredTabs);
    }

    {
        vcl::WidgetDefinition aDefinition;
        vcl::WidgetDefinitionReader aReader(getFullUrl(u"definitionSettings3.xml"),
                                            getFullUrl(u""));
        CPPUNIT_ASSERT(aReader.read(aDefinition));
        CPPUNIT_ASSERT_EQUAL("true"_ostr, aDefinition.mpSettings->msNoActiveTabTextRaise);
        CPPUNIT_ASSERT_EQUAL("false"_ostr, aDefinition.mpSettings->msCenteredTabs);
        CPPUNIT_ASSERT_EQUAL("0"_ostr, aDefinition.mpSettings->msListBoxEntryMargin);
        CPPUNIT_ASSERT_EQUAL("10"_ostr, aDefinition.mpSettings->msDefaultFontSize);
        CPPUNIT_ASSERT_EQUAL("16"_ostr, aDefinition.mpSettings->msTitleHeight);
        CPPUNIT_ASSERT_EQUAL("12"_ostr, aDefinition.mpSettings->msFloatTitleHeight);
        CPPUNIT_ASSERT_EQUAL("15"_ostr, aDefinition.mpSettings->msListBoxPreviewDefaultLogicWidth);
        CPPUNIT_ASSERT_EQUAL("7"_ostr, aDefinition.mpSettings->msListBoxPreviewDefaultLogicHeight);
    }
}

void WidgetDefinitionReaderTest::testRead()
{
    vcl::WidgetDefinition aDefinition;

    vcl::WidgetDefinitionReader aReader(getFullUrl(u"definition1.xml"), getFullUrl(u""));
    CPPUNIT_ASSERT(aReader.read(aDefinition));

    CPPUNIT_ASSERT_EQUAL(u"123456"_ustr, aDefinition.mpStyle->maFaceColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(u"234567"_ustr, aDefinition.mpStyle->maCheckedColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(u"345678"_ustr, aDefinition.mpStyle->maLightColor.AsRGBHexString());

    CPPUNIT_ASSERT_EQUAL(u"ffffff"_ustr, aDefinition.mpStyle->maVisitedLinkColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(u"ffffff"_ustr, aDefinition.mpStyle->maToolTextColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(u"ffffff"_ustr, aDefinition.mpStyle->maWindowTextColor.AsRGBHexString());

    // Pushbutton
    {
        ControlState eState
            = ControlState::DEFAULT | ControlState::ENABLED | ControlState::ROLLOVER;
        std::vector<std::shared_ptr<vcl::WidgetDefinitionState>> aStates
            = aDefinition.getDefinition(ControlType::Pushbutton, ControlPart::Entire)
                  ->getStates(ControlType::Pushbutton, ControlPart::Entire, eState,
                              PushButtonValue());

        CPPUNIT_ASSERT_EQUAL(size_t(2), aStates.size());

        CPPUNIT_ASSERT_EQUAL(size_t(2), aStates[0]->mpWidgetDrawActions.size());
        CPPUNIT_ASSERT_EQUAL(vcl::WidgetDrawActionType::RECTANGLE,
                             aStates[0]->mpWidgetDrawActions[0]->maType);
        CPPUNIT_ASSERT_EQUAL(vcl::WidgetDrawActionType::LINE,
                             aStates[0]->mpWidgetDrawActions[1]->maType);
    }

    // Radiobutton
    {
        std::vector<std::shared_ptr<vcl::WidgetDefinitionState>> aStates
            = aDefinition.getDefinition(ControlType::Radiobutton, ControlPart::Entire)
                  ->getStates(ControlType::Radiobutton, ControlPart::Entire, ControlState::NONE,
                              ImplControlValue(ButtonValue::On));
        CPPUNIT_ASSERT_EQUAL(size_t(1), aStates.size());
        CPPUNIT_ASSERT_EQUAL(size_t(2), aStates[0]->mpWidgetDrawActions.size());
    }

    {
        std::vector<std::shared_ptr<vcl::WidgetDefinitionState>> aStates
            = aDefinition.getDefinition(ControlType::Radiobutton, ControlPart::Entire)
                  ->getStates(ControlType::Radiobutton, ControlPart::Entire, ControlState::NONE,
                              ImplControlValue(ButtonValue::Off));
        CPPUNIT_ASSERT_EQUAL(size_t(1), aStates.size());
        CPPUNIT_ASSERT_EQUAL(size_t(1), aStates[0]->mpWidgetDrawActions.size());
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(WidgetDefinitionReaderTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
