/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <config_features.h>

#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <vcl/printer/OptionsHelper.hxx>

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <vector>

namespace
{
class OptionsTest : public CppUnit::TestFixture
{
public:
    void testGroupControlOpt();
    void testSubgroupControlOpt();

    CPPUNIT_TEST_SUITE(OptionsTest);
    CPPUNIT_TEST(testGroupControlOpt);
    CPPUNIT_TEST(testSubgroupControlOpt);
    CPPUNIT_TEST_SUITE_END();
};

void OptionsTest::testGroupControlOpt()
{
    css::uno::Sequence<css::beans::PropertyValue> aCtrl;
    vcl::printer::OptionsHelper::setGroupControlOpt("testcontrol", "test_title", ".HelpID:vcl:Test")
        >>= aCtrl;
    sal_uInt32 n = 0;
    OUString sName;
    sName = aCtrl[n].Name;
    CPPUNIT_ASSERT_EQUAL(OUString("Text"), sName);

    OUString sValue;
    aCtrl[n++].Value >>= sValue;
    CPPUNIT_ASSERT_EQUAL(OUString("test_title"), sValue);

    sName = aCtrl[n].Name;
    CPPUNIT_ASSERT_EQUAL(OUString("HelpId"), sName);

    css::uno::Sequence<OUString> sHelpIds;
    aCtrl[n++].Value >>= sHelpIds;
    sValue = sHelpIds[0];
    CPPUNIT_ASSERT_EQUAL(OUString(".HelpID:vcl:Test"), sValue);

    sName = aCtrl[n].Name;
    CPPUNIT_ASSERT_EQUAL(OUString("ControlType"), sName);

    aCtrl[n++].Value >>= sValue;
    CPPUNIT_ASSERT_EQUAL(OUString("Group"), sValue);

    sName = aCtrl[n].Name;
    CPPUNIT_ASSERT_EQUAL(OUString("ID"), sName);

    css::uno::Sequence<OUString> sIDs;
    aCtrl[n++].Value >>= sIDs;
    sValue = sIDs[0];
    CPPUNIT_ASSERT_EQUAL(OUString("testcontrol"), sValue);
}

void OptionsTest::testSubgroupControlOpt()
{
    vcl::printer::OptionsHelper::UIControlOptions aCtrlOptions;
    aCtrlOptions.maGroupHint = "TestHint";
    aCtrlOptions.mbInternalOnly = true;

    css::uno::Sequence<css::beans::PropertyValue> aCtrl;
    vcl::printer::OptionsHelper::setSubgroupControlOpt("testcontrol", "test_title",
                                                       ".HelpID:vcl:Test", aCtrlOptions)
        >>= aCtrl;

    sal_uInt32 n = 0;
    OUString sName;
    sName = aCtrl[n].Name;
    CPPUNIT_ASSERT_EQUAL(OUString("Text"), sName);

    OUString sValue;
    aCtrl[n++].Value >>= sValue;
    CPPUNIT_ASSERT_EQUAL(OUString("test_title"), sValue);

    sName = aCtrl[n].Name;
    CPPUNIT_ASSERT_EQUAL(OUString("HelpId"), sName);

    css::uno::Sequence<OUString> sHelpIds;
    aCtrl[n++].Value >>= sHelpIds;
    sValue = sHelpIds[0];
    CPPUNIT_ASSERT_EQUAL(OUString(".HelpID:vcl:Test"), sValue);

    sName = aCtrl[n].Name;
    CPPUNIT_ASSERT_EQUAL(OUString("ControlType"), sName);

    aCtrl[n++].Value >>= sValue;
    CPPUNIT_ASSERT_EQUAL(OUString("Subgroup"), sValue);

    sName = aCtrl[n].Name;
    CPPUNIT_ASSERT_EQUAL(OUString("ID"), sName);

    css::uno::Sequence<OUString> sIDs;
    aCtrl[n++].Value >>= sIDs;
    sValue = sIDs[0];
    CPPUNIT_ASSERT_EQUAL(OUString("testcontrol"), sValue);

    sName = aCtrl[n].Name;
    CPPUNIT_ASSERT_EQUAL(OUString("GroupingHint"), sName);

    aCtrl[n++].Value >>= sValue;
    CPPUNIT_ASSERT_EQUAL(OUString("TestHint"), sValue);

    sName = aCtrl[n].Name;
    CPPUNIT_ASSERT_EQUAL(OUString("InternalUIOnly"), sName);

    bool bInternalOnly;
    aCtrl[n].Value >>= bInternalOnly;
    CPPUNIT_ASSERT(bInternalOnly);
}

} // end anonymous namespace

CPPUNIT_TEST_SUITE_REGISTRATION(OptionsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
