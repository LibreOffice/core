/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <rtl/ustring.hxx>

#include <tools/config.hxx>

class ToolsConfigTest : public test::BootstrapFixture
{
public:
    ToolsConfigTest()
        : BootstrapFixture(true, false)
    {
    }

    virtual void setUp() override
    {
        maConfigFile = m_directories.getURLFromSrc("/tools/qa/data/");
        maConfigFile += "testconfig.ini";
    }

    void testOpenConfigFile()
    {
        Config* pConfig = new Config(maConfigFile);
        CPPUNIT_ASSERT(pConfig);
        delete pConfig;
    }

    void testHasGroup()
    {
        Config aConfig(maConfigFile);
        CPPUNIT_ASSERT(aConfig.HasGroup("TestGroup"));
        CPPUNIT_ASSERT(aConfig.HasGroup("TestGroup2"));
    }

    void testGetGroup()
    {
        Config aConfig(maConfigFile);
        CPPUNIT_ASSERT_EQUAL(OString(""), aConfig.GetGroup());

        CPPUNIT_ASSERT_EQUAL(OString("TestGroup"), aConfig.GetGroupName(0));
        CPPUNIT_ASSERT_EQUAL(OString("TestGroup2"), aConfig.GetGroupName(1));
        CPPUNIT_ASSERT_EQUAL(OString(""), aConfig.GetGroupName(2));
    }

    void testSetGroup()
    {
        Config aConfig(maConfigFile);

        aConfig.SetGroup(OString("TestGroup"));
        CPPUNIT_ASSERT_EQUAL(OString("TestGroup"), aConfig.GetGroup());

        // so this is a quirk of Config - you can set the group name,
        // but it might not exist so you really should first check if
        // it exists via HasGroup()
        aConfig.SetGroup(OString("TestGroupA"));
        CPPUNIT_ASSERT(!aConfig.HasGroup("TestGroupA"));
        CPPUNIT_ASSERT_EQUAL(OString("TestGroupA"), aConfig.GetGroup());
    }

    void testDeleteGroup()
    {
        // note that we are not going to flush this (i.e. don't want the test file
        // to change)
        {
            Config aConfig(maConfigFile);

            aConfig.DeleteGroup(OString("TestGroup"));
            CPPUNIT_ASSERT(!aConfig.HasGroup("TestGroup"));
            CPPUNIT_ASSERT_EQUAL(OString("TestGroup2"), aConfig.GetGroupName(0));

            sal_uInt16 nExpected = 1;
            sal_uInt16 nActual = aConfig.GetGroupCount();
            CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
        }

        {
            Config aConfig(maConfigFile);

            CPPUNIT_ASSERT(!aConfig.HasGroup("NonExistentTestGroup"));
            aConfig.DeleteGroup(OString("NonExistentTestGroup"));
            CPPUNIT_ASSERT_EQUAL(OString("TestGroup"), aConfig.GetGroupName(0));

            sal_uInt16 nExpected = 2;
            sal_uInt16 nActual = aConfig.GetGroupCount();
            CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
        }
    }

    void testGetGroupCount()
    {
        Config aConfig(maConfigFile);
        sal_uInt16 nExpected = 2;
        sal_uInt16 nActual = aConfig.GetGroupCount();
        CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
    }

    CPPUNIT_TEST_SUITE(ToolsConfigTest);
    CPPUNIT_TEST(testOpenConfigFile);
    CPPUNIT_TEST(testHasGroup);
    CPPUNIT_TEST(testGetGroup);
    CPPUNIT_TEST(testSetGroup);
    CPPUNIT_TEST(testGetGroupCount);
    CPPUNIT_TEST_SUITE_END();

private:
    OUString maConfigFile;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ToolsConfigTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
