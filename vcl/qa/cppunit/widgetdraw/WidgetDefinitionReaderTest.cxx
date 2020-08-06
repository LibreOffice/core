/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <unotest/bootstrapfixturebase.hxx>

#include <widgetdraw/WidgetDefinitionReader.hxx>

namespace
{
OUStringLiteral const gaDataUrl("/vcl/qa/cppunit/widgetdraw/data/");

class WidgetDefinitionReaderTest : public test::BootstrapFixtureBase
{
private:
    OUString getFullUrl(const OUString& sFileName)
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
        vcl::WidgetDefinitionReader aReader(getFullUrl("definitionSettings1.xml"), getFullUrl(""));
        CPPUNIT_ASSERT(aReader.read(aDefinition));
        CPPUNIT_ASSERT_EQUAL(OString(""), aDefinition.mpSettings->msCenteredTabs);
    }

    {
        vcl::WidgetDefinition aDefinition;
        vcl::WidgetDefinitionReader aReader(getFullUrl("definitionSettings2.xml"), getFullUrl(""));
        CPPUNIT_ASSERT(aReader.read(aDefinition));
        CPPUNIT_ASSERT_EQUAL(OString("true"), aDefinition.mpSettings->msCenteredTabs);
    }

    {
        vcl::WidgetDefinition aDefinition;
        vcl::WidgetDefinitionReader aReader(getFullUrl("definitionSettings3.xml"), getFullUrl(""));
        CPPUNIT_ASSERT(aReader.read(aDefinition));
        CPPUNIT_ASSERT_EQUAL(OString("true"), aDefinition.mpSettings->msNoActiveTabTextRaise);
        CPPUNIT_ASSERT_EQUAL(OString("false"), aDefinition.mpSettings->msCenteredTabs);
        CPPUNIT_ASSERT_EQUAL(OString("0"), aDefinition.mpSettings->msListBoxEntryMargin);
        CPPUNIT_ASSERT_EQUAL(OString("10"), aDefinition.mpSettings->msDefaultFontSize);
        CPPUNIT_ASSERT_EQUAL(OString("16"), aDefinition.mpSettings->msTitleHeight);
        CPPUNIT_ASSERT_EQUAL(OString("12"), aDefinition.mpSettings->msFloatTitleHeight);
        CPPUNIT_ASSERT_EQUAL(OString("15"),
                             aDefinition.mpSettings->msListBoxPreviewDefaultLogicWidth);
        CPPUNIT_ASSERT_EQUAL(OString("7"),
                             aDefinition.mpSettings->msListBoxPreviewDefaultLogicHeight);
    }
}

void WidgetDefinitionReaderTest::testRead()
{
    vcl::WidgetDefinition aDefinition;

    vcl::WidgetDefinitionReader aReader(getFullUrl("definition1.xml"), getFullUrl(""));
    CPPUNIT_ASSERT(aReader.read(aDefinition));

    CPPUNIT_ASSERT_EQUAL(OUString("123456"), aDefinition.mpStyle->maFaceColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(OUString("234567"), aDefinition.mpStyle->maCheckedColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(OUString("345678"), aDefinition.mpStyle->maLightColor.AsRGBHexString());

    CPPUNIT_ASSERT_EQUAL(OUString("ffffff"),
                         aDefinition.mpStyle->maVisitedLinkColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(OUString("ffffff"), aDefinition.mpStyle->maToolTextColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(OUString("ffffff"), aDefinition.mpStyle->maFontColor.AsRGBHexString());

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
