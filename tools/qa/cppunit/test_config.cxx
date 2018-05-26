/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <osl/file.hxx>
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
        maOriginalConfigFile = m_directories.getURLFromSrc("/tools/qa/data/");
        maOriginalConfigFile += "testconfig.ini";

        if (osl::FileBase::getTempDirURL(maConfigFile) == osl::File::RC::E_None)
            maConfigFile += "/config.ini";
        else
            assert("cannot create temp folder");

        osl::File::copy(maOriginalConfigFile, maConfigFile);
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
        {
            Config aConfig(maConfigFile);

            aConfig.DeleteGroup(OString("TestGroup"));
            CPPUNIT_ASSERT(!aConfig.HasGroup("TestGroup"));
            CPPUNIT_ASSERT_EQUAL(OString("TestGroup2"), aConfig.GetGroupName(0));

            sal_uInt16 nExpected = 1;
            sal_uInt16 nActual = aConfig.GetGroupCount();
            CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
        }

        osl::File::copy(maOriginalConfigFile, maConfigFile);

        {
            Config aConfig(maConfigFile);

            CPPUNIT_ASSERT(!aConfig.HasGroup("NonExistentTestGroup"));
            aConfig.DeleteGroup(OString("NonExistentTestGroup"));
            CPPUNIT_ASSERT_EQUAL(OString("TestGroup"), aConfig.GetGroupName(0));

            sal_uInt16 nExpected = 2;
            sal_uInt16 nActual = aConfig.GetGroupCount();
            CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
        }

        osl::File::copy(maOriginalConfigFile, maConfigFile);
    }

    void testGetGroupCount()
    {
        Config aConfig(maConfigFile);
        sal_uInt16 nExpected = 2;
        sal_uInt16 nActual = aConfig.GetGroupCount();
        CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
    }

    void testReadKey()
    {
        Config aConfig(maConfigFile);
        aConfig.SetGroup(OString("TestGroup"));
        CPPUNIT_ASSERT_EQUAL(OString("testvalue"), aConfig.ReadKey(OString("testkey")));
        CPPUNIT_ASSERT_EQUAL(OString(), aConfig.ReadKey(OString("nonexistenttestkey")));
        CPPUNIT_ASSERT_EQUAL(OString("notexists"),
                             aConfig.ReadKey(OString("nonexistenttestkey"), OString("notexists")));

        aConfig.SetGroup(OString("TestGroup2"));
        CPPUNIT_ASSERT_EQUAL(OString("testvalue"), aConfig.ReadKey(OString("testkey2")));
        CPPUNIT_ASSERT_EQUAL(OString(), aConfig.ReadKey(OString("nonexistenttestkey")));
        CPPUNIT_ASSERT_EQUAL(OString("notexists"),
                             aConfig.ReadKey(OString("nonexistenttestkey"), OString("notexists")));
    }

    void testGetKeyName()
    {
        Config aConfig(maConfigFile);
        aConfig.SetGroup(OString("TestGroup"));
        CPPUNIT_ASSERT_EQUAL(OString("testkey"), aConfig.GetKeyName(0));

        aConfig.SetGroup(OString("TestGroup2"));
        CPPUNIT_ASSERT_EQUAL(OString("testkey2"), aConfig.GetKeyName(0));
    }

    void testWriteDeleteKey()
    {
        Config aConfig(maConfigFile);
        aConfig.SetGroup(OString("TestGroup"));
        aConfig.WriteKey(OString("testkey_new"), OString("testvalue"));

        sal_uInt16 nExpected = 2;
        sal_uInt16 nActual = aConfig.GetKeyCount();
        CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
        CPPUNIT_ASSERT_EQUAL(OString("testvalue"), aConfig.ReadKey(OString("testkey_new")));

        aConfig.DeleteKey(OString("testkey_new"));

        nExpected = 1;
        nActual = aConfig.GetKeyCount();
        CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
        CPPUNIT_ASSERT_EQUAL(OString(), aConfig.ReadKey(OString("testkey_new")));

        aConfig.SetGroup(OString("TestGroup2"));
        aConfig.WriteKey(OString("testkey_new"), OString("testvalue"));

        nActual = aConfig.GetKeyCount();
        nExpected = 2;
        CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
        CPPUNIT_ASSERT_EQUAL(OString("testvalue"), aConfig.ReadKey(OString("testkey_new")));

        aConfig.DeleteKey(OString("testkey_new"));

        nActual = aConfig.GetKeyCount();
        nExpected = 1;
        CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
        CPPUNIT_ASSERT_EQUAL(OString(), aConfig.ReadKey(OString("testkey_new")));

        aConfig.SetGroup(OString("TestGroup3"));
        aConfig.WriteKey(OString("testkey_new_group3"), OString("testvalue"));

        nActual = aConfig.GetKeyCount();
        nExpected = 1;
        CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
        CPPUNIT_ASSERT_EQUAL(OString("testvalue"), aConfig.ReadKey(OString("testkey_new_group3")));

        nExpected = 3;
        CPPUNIT_ASSERT_EQUAL(nExpected, aConfig.GetGroupCount());

        osl::File::copy(maOriginalConfigFile, maConfigFile);
    }

    CPPUNIT_TEST_SUITE(ToolsConfigTest);
    CPPUNIT_TEST(testHasGroup);
    CPPUNIT_TEST(testGetGroup);
    CPPUNIT_TEST(testSetGroup);
    CPPUNIT_TEST(testDeleteGroup);
    CPPUNIT_TEST(testReadKey);
    CPPUNIT_TEST(testGetGroupCount);
    CPPUNIT_TEST(testGetKeyName);
    CPPUNIT_TEST(testWriteDeleteKey);
    CPPUNIT_TEST_SUITE_END();

private:
    OUString maOriginalConfigFile;
    OUString maConfigFile;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ToolsConfigTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
