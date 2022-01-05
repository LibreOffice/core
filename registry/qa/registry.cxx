/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <registry/registry.hxx>
#include <registry/reader.hxx>
#include <registry/writer.hxx>

#include <cstdlib>
#include <memory>

namespace
{
class RegistryTest : public CppUnit::TestFixture
{
public:
    void testShouldCreateRegistry();
    void testShouldOpenRootKey();
    void testShouldCreateModuleKey();

    CPPUNIT_TEST_SUITE(RegistryTest);
    CPPUNIT_TEST(testShouldCreateRegistry);
    CPPUNIT_TEST(testShouldOpenRootKey);
    CPPUNIT_TEST(testShouldCreateModuleKey);
    CPPUNIT_TEST_SUITE_END();
};

void RegistryTest::testShouldCreateRegistry()
{
    // make an in memory registry
    std::unique_ptr<Registry> myRegistry(new Registry());
    CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR, myRegistry->create(""));
}

void RegistryTest::testShouldOpenRootKey()
{
    std::unique_ptr<Registry> myRegistry(new Registry());
    RegistryKey rootKey;

    if (myRegistry->create("") == RegError::NO_ERROR)
        CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR, myRegistry->openRootKey(rootKey));
}

void RegistryTest::testShouldCreateModuleKey()
{
    std::unique_ptr<Registry> myRegistry(new Registry());
    RegistryKey rootKey;

    if (myRegistry->create("") == RegError::NO_ERROR)
    {
        RegistryKey key1, key2;

        myRegistry->openRootKey(rootKey);

        CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR, rootKey.createKey("UCR", key1));
        CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR, key1.createKey("ModuleA", key2));

        {
            const sal_uInt16 FIELDCOUNT = 11;
            typereg::Writer writer(TYPEREG_VERSION_1, "Hello I am a module", "", RT_TYPE_MODULE,
                                   true, "ModuleA", 0, FIELDCOUNT, 0, 0);

            RTConstValue aConst;

            aConst.m_type = RT_TYPE_BOOL;
            aConst.m_value.aBool = sal_True;
            writer.setFieldData(0, "I am a boolean", "", RTFieldAccess::CONST, "aConstBool",
                                "boolean", aConst);

            aConst.m_type = RT_TYPE_BYTE;
            aConst.m_value.aByte = 127;
            writer.setFieldData(1, "I am a byte", "", RTFieldAccess::CONST, "aConstByte", "byte",
                                aConst);

            aConst.m_type = RT_TYPE_INT16;
            aConst.m_value.aShort = -10;
            writer.setFieldData(2, "I am a short", "", RTFieldAccess::CONST, "aConstShort", "short",
                                aConst);

            aConst.m_type = RT_TYPE_UINT16;
            aConst.m_value.aUShort = 10;
            writer.setFieldData(3, "I am an unsigned short", "", RTFieldAccess::CONST,
                                "aConstUShort", "unsigned short", aConst);

            aConst.m_type = RT_TYPE_INT32;
            aConst.m_value.aLong = -100'000;
            writer.setFieldData(4, "I am a long", "", RTFieldAccess::CONST, "aConstLong", "long",
                                aConst);

            aConst.m_type = RT_TYPE_UINT32;
            aConst.m_value.aULong = 100'000;
            writer.setFieldData(5, "I am an unsigned long", "", RTFieldAccess::CONST, "aConstULong",
                                "unsigned long", aConst);

            aConst.m_type = RT_TYPE_INT64;
            aConst.m_value.aHyper = -100'000'000;
            writer.setFieldData(6, "I am a hyper", "", RTFieldAccess::CONST, "aConstHyper", "hyper",
                                aConst);

            aConst.m_type = RT_TYPE_UINT64;
            aConst.m_value.aUHyper = 100000000;
            writer.setFieldData(7, "I am an unsigned long", "", RTFieldAccess::CONST, "aConstULong",
                                "unsigned long", aConst);

            aConst.m_type = RT_TYPE_FLOAT;
            aConst.m_value.aFloat = -2e-10f;
            writer.setFieldData(8, "I am a float", "", RTFieldAccess::CONST, "aConstFloat", "float",
                                aConst);

            aConst.m_type = RT_TYPE_DOUBLE;
            aConst.m_value.aDouble = -2e-100;
            writer.setFieldData(9, "I am a double", "", RTFieldAccess::CONST, "aConstDouble",
                                "double", aConst);

            aConst.m_type = RT_TYPE_STRING;
            OUString tmpStr("this is a unicode string");
            aConst.m_value.aString = tmpStr.getStr();
            writer.setFieldData(10, "I am a string", "", RTFieldAccess::CONST, "aConstString",
                                "string", aConst);

            sal_uInt32 nSize;
            void const* pBlob = writer.getBlob(&nSize);
            CPPUNIT_ASSERT(pBlob);

            CPPUNIT_ASSERT_EQUAL(
                RegError::NO_ERROR,
                key2.setValue("", RegValueType::BINARY, const_cast<void*>(pBlob), nSize));

            sal_uInt8* readBlob = static_cast<sal_uInt8*>(std::malloc(nSize));
            CPPUNIT_ASSERT(readBlob);
            CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR,
                                 key2.getValue("", static_cast<void*>(readBlob)));

            typereg::Reader reader(static_cast<void*>(readBlob), nSize);

            CPPUNIT_ASSERT(reader.isValid());

            CPPUNIT_ASSERT_EQUAL(OUString("Hello I am a module"), reader.getDocumentation());
            CPPUNIT_ASSERT_EQUAL(OUString("ModuleA"), reader.getTypeName());
            CPPUNIT_ASSERT_EQUAL(FIELDCOUNT, reader.getFieldCount());

            CPPUNIT_ASSERT_EQUAL(OUString("I am a short"), reader.getFieldDocumentation(2));
            CPPUNIT_ASSERT_EQUAL(OUString("short"), reader.getFieldTypeName(2));
            CPPUNIT_ASSERT_EQUAL(OUString("aConstShort"), reader.getFieldName(2));

            RTConstValue aReadConst = reader.getFieldValue(2);
            CPPUNIT_ASSERT_EQUAL(RT_TYPE_INT16, aReadConst.m_type);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(-10), aReadConst.m_value.aShort);

            CPPUNIT_ASSERT_EQUAL(OUString("I am a hyper"), reader.getFieldDocumentation(6));
            CPPUNIT_ASSERT_EQUAL(OUString("hyper"), reader.getFieldTypeName(6));
            CPPUNIT_ASSERT_EQUAL(OUString("aConstHyper"), reader.getFieldName(6));

            aReadConst = reader.getFieldValue(6);
            CPPUNIT_ASSERT_EQUAL(RT_TYPE_INT64, aReadConst.m_type);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-100'000'000), aReadConst.m_value.aLong);
        }
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(RegistryTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
