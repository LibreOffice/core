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

#include <sal/log.hxx>

namespace
{
class RegistryTest : public CppUnit::TestFixture
{
public:
    void testShouldCreateRegistry();
    void testShouldOpenRootKey();
    void testShouldCreateModuleKey();
    void testShouldCreateStructKey();
    void testShouldCreateEnumKey();
    void testShouldCreateInterface();
    void testShouldCreateException();
    void testShouldCreateService();
    void testShouldCreateConstantsGroup();

    CPPUNIT_TEST_SUITE(RegistryTest);
    CPPUNIT_TEST(testShouldCreateRegistry);
    CPPUNIT_TEST(testShouldOpenRootKey);
    CPPUNIT_TEST(testShouldCreateModuleKey);
    CPPUNIT_TEST(testShouldCreateStructKey);
    CPPUNIT_TEST(testShouldCreateEnumKey);
    CPPUNIT_TEST(testShouldCreateInterface);
    CPPUNIT_TEST(testShouldCreateException);
    CPPUNIT_TEST(testShouldCreateService);
    CPPUNIT_TEST(testShouldCreateConstantsGroup);
    CPPUNIT_TEST_SUITE_END();
};

void RegistryTest::testShouldCreateRegistry()
{
    // make an in memory registry
    std::unique_ptr<Registry> registry(new Registry());
    CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR, registry->create(""));
}

void RegistryTest::testShouldOpenRootKey()
{
    std::unique_ptr<Registry> registry(new Registry());
    RegistryKey rootKey;

    if (registry->create("") == RegError::NO_ERROR)
        CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR, registry->openRootKey(rootKey));
}

void RegistryTest::testShouldCreateModuleKey()
{
    std::unique_ptr<Registry> registry(new Registry());
    RegistryKey rootKey;

    if (registry->create("") == RegError::NO_ERROR)
    {
        RegistryKey key1, key2;

        registry->openRootKey(rootKey);

        CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR, rootKey.createKey("UCR", key1));
        CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR, key1.createKey("ModuleA", key2));

        const sal_uInt16 FIELDCOUNT = 11;
        typereg::Writer writer(TYPEREG_VERSION_1, "Hello I am a module", "", RT_TYPE_MODULE, true,
                               "ModuleA", 0, FIELDCOUNT, 0, 0);

        RTConstValue aConst;

        aConst.m_type = RT_TYPE_BOOL;
        aConst.m_value.aBool = true;
        writer.setFieldData(0, "I am a boolean", "", RTFieldAccess::CONST, "aConstBool", "boolean",
                            aConst);

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
        writer.setFieldData(3, "I am an unsigned short", "", RTFieldAccess::CONST, "aConstUShort",
                            "unsigned short", aConst);

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
        aConst.m_value.aUHyper = 100'000'000;
        writer.setFieldData(7, "I am an unsigned hyper", "", RTFieldAccess::CONST, "aConstUHyper",
                            "unsigned hyper", aConst);

        aConst.m_type = RT_TYPE_FLOAT;
        aConst.m_value.aFloat = -2e-10f;
        writer.setFieldData(8, "I am a float", "", RTFieldAccess::CONST, "aConstFloat", "float",
                            aConst);

        aConst.m_type = RT_TYPE_DOUBLE;
        aConst.m_value.aDouble = -2e-100;
        writer.setFieldData(9, "I am a double", "", RTFieldAccess::CONST, "aConstDouble", "double",
                            aConst);

        aConst.m_type = RT_TYPE_STRING;
        OUString tmpStr("this is a unicode string");
        aConst.m_value.aString = tmpStr.getStr();
        writer.setFieldData(10, "I am a string", "", RTFieldAccess::CONST, "aConstString", "string",
                            aConst);

        sal_uInt32 nSize;
        void const* pBlob = writer.getBlob(&nSize);
        CPPUNIT_ASSERT(pBlob);

        CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR, key2.setValue("", RegValueType::BINARY,
                                                               const_cast<void*>(pBlob), nSize));

        void* readBlob = std::malloc(nSize);
        CPPUNIT_ASSERT(readBlob);
        CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR, key2.getValue("", readBlob));

        typereg::Reader reader(readBlob, nSize);

        CPPUNIT_ASSERT(reader.isValid());

        CPPUNIT_ASSERT_EQUAL(OUString("Hello I am a module"), reader.getDocumentation());
        CPPUNIT_ASSERT_EQUAL(OUString("ModuleA"), reader.getTypeName());
        CPPUNIT_ASSERT_EQUAL(FIELDCOUNT, reader.getFieldCount());

        CPPUNIT_ASSERT_EQUAL(OUString("I am a boolean"), reader.getFieldDocumentation(0));
        CPPUNIT_ASSERT_EQUAL(OUString("boolean"), reader.getFieldTypeName(0));
        CPPUNIT_ASSERT_EQUAL(OUString("aConstBool"), reader.getFieldName(0));

        RTConstValue aReadConst = reader.getFieldValue(0);
        CPPUNIT_ASSERT_EQUAL(RT_TYPE_BOOL, aReadConst.m_type);
        CPPUNIT_ASSERT_EQUAL(true, aReadConst.m_value.aBool);

        CPPUNIT_ASSERT_EQUAL(OUString("I am a byte"), reader.getFieldDocumentation(1));
        CPPUNIT_ASSERT_EQUAL(OUString("byte"), reader.getFieldTypeName(1));
        CPPUNIT_ASSERT_EQUAL(OUString("aConstByte"), reader.getFieldName(1));

        aReadConst = reader.getFieldValue(1);
        CPPUNIT_ASSERT_EQUAL(RT_TYPE_BYTE, aReadConst.m_type);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int8>(127), aReadConst.m_value.aByte);

        CPPUNIT_ASSERT_EQUAL(OUString("I am a short"), reader.getFieldDocumentation(2));
        CPPUNIT_ASSERT_EQUAL(OUString("short"), reader.getFieldTypeName(2));
        CPPUNIT_ASSERT_EQUAL(OUString("aConstShort"), reader.getFieldName(2));

        aReadConst = reader.getFieldValue(2);
        CPPUNIT_ASSERT_EQUAL(RT_TYPE_INT16, aReadConst.m_type);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(-10), aReadConst.m_value.aShort);

        CPPUNIT_ASSERT_EQUAL(OUString("I am an unsigned short"), reader.getFieldDocumentation(3));
        CPPUNIT_ASSERT_EQUAL(OUString("unsigned short"), reader.getFieldTypeName(3));
        CPPUNIT_ASSERT_EQUAL(OUString("aConstUShort"), reader.getFieldName(3));

        aReadConst = reader.getFieldValue(3);
        CPPUNIT_ASSERT_EQUAL(RT_TYPE_UINT16, aReadConst.m_type);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(10), aReadConst.m_value.aUShort);

        CPPUNIT_ASSERT_EQUAL(OUString("I am a long"), reader.getFieldDocumentation(4));
        CPPUNIT_ASSERT_EQUAL(OUString("long"), reader.getFieldTypeName(4));
        CPPUNIT_ASSERT_EQUAL(OUString("aConstLong"), reader.getFieldName(4));

        aReadConst = reader.getFieldValue(4);
        CPPUNIT_ASSERT_EQUAL(RT_TYPE_INT32, aReadConst.m_type);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-100'000), aReadConst.m_value.aLong);

        CPPUNIT_ASSERT_EQUAL(OUString("I am an unsigned long"), reader.getFieldDocumentation(5));
        CPPUNIT_ASSERT_EQUAL(OUString("unsigned long"), reader.getFieldTypeName(5));
        CPPUNIT_ASSERT_EQUAL(OUString("aConstULong"), reader.getFieldName(5));

        aReadConst = reader.getFieldValue(5);
        CPPUNIT_ASSERT_EQUAL(RT_TYPE_UINT32, aReadConst.m_type);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(100'000), aReadConst.m_value.aULong);

        CPPUNIT_ASSERT_EQUAL(OUString("I am a hyper"), reader.getFieldDocumentation(6));
        CPPUNIT_ASSERT_EQUAL(OUString("hyper"), reader.getFieldTypeName(6));
        CPPUNIT_ASSERT_EQUAL(OUString("aConstHyper"), reader.getFieldName(6));

        aReadConst = reader.getFieldValue(6);
        CPPUNIT_ASSERT_EQUAL(RT_TYPE_INT64, aReadConst.m_type);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(-100'000'000), aReadConst.m_value.aHyper);

        CPPUNIT_ASSERT_EQUAL(OUString("I am an unsigned hyper"), reader.getFieldDocumentation(7));
        CPPUNIT_ASSERT_EQUAL(OUString("unsigned hyper"), reader.getFieldTypeName(7));
        CPPUNIT_ASSERT_EQUAL(OUString("aConstUHyper"), reader.getFieldName(7));

        aReadConst = reader.getFieldValue(7);
        CPPUNIT_ASSERT_EQUAL(RT_TYPE_UINT64, aReadConst.m_type);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt64>(100'000'000), aReadConst.m_value.aUHyper);

        CPPUNIT_ASSERT_EQUAL(OUString("I am a float"), reader.getFieldDocumentation(8));
        CPPUNIT_ASSERT_EQUAL(OUString("float"), reader.getFieldTypeName(8));
        CPPUNIT_ASSERT_EQUAL(OUString("aConstFloat"), reader.getFieldName(8));

        aReadConst = reader.getFieldValue(8);
        CPPUNIT_ASSERT_EQUAL(RT_TYPE_FLOAT, aReadConst.m_type);
        CPPUNIT_ASSERT_EQUAL(static_cast<float>(-2e-10f), aReadConst.m_value.aFloat);

        CPPUNIT_ASSERT_EQUAL(OUString("I am a double"), reader.getFieldDocumentation(9));
        CPPUNIT_ASSERT_EQUAL(OUString("double"), reader.getFieldTypeName(9));
        CPPUNIT_ASSERT_EQUAL(OUString("aConstDouble"), reader.getFieldName(9));

        aReadConst = reader.getFieldValue(9);
        CPPUNIT_ASSERT_EQUAL(RT_TYPE_DOUBLE, aReadConst.m_type);
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(-2e-100), aReadConst.m_value.aDouble);

        CPPUNIT_ASSERT_EQUAL(OUString("I am a string"), reader.getFieldDocumentation(10));
        CPPUNIT_ASSERT_EQUAL(OUString("string"), reader.getFieldTypeName(10));
        CPPUNIT_ASSERT_EQUAL(OUString("aConstString"), reader.getFieldName(10));

        aReadConst = reader.getFieldValue(10);
        CPPUNIT_ASSERT_EQUAL(RT_TYPE_STRING, aReadConst.m_type);
        CPPUNIT_ASSERT_EQUAL(OUString("this is a unicode string"),
                             OUString(aReadConst.m_value.aString));
    }
}

RegistryKey createKey(Registry* registry, OUString const& rType)
{
    RegistryKey rootkey, ucrkey, modulekey, key;

    if (registry->create("") != RegError::NO_ERROR)
        assert(false && "could not create in memory registry");

    registry->openRootKey(rootkey);

    rootkey.createKey("UCR", ucrkey);
    ucrkey.createKey("ModuleA", modulekey);
    modulekey.createKey(rType, key);

    return key;
}

typereg::Reader getStructReader(Registry* registry, sal_uInt32& nSize)
{
    RegistryKey key = createKey(registry, "StructA");

    const sal_uInt16 FIELDCOUNT = 3;
    typereg::Writer writer(TYPEREG_VERSION_1, "Hello I am a structure", "", RT_TYPE_STRUCT, true,
                           "ModuleA/StructA", 0, FIELDCOUNT, 0, 0);

    writer.setFieldData(0, "I am an unsigned integer", "", RTFieldAccess::READWRITE, "asal_uInt32",
                        "unsigned integer", RTConstValue());

    writer.setFieldData(1, "I am an XInterface", "", RTFieldAccess::READWRITE, "aXInterface",
                        "stardiv/uno/XInterface", RTConstValue());

    writer.setFieldData(2, "I am a sequence", "", RTFieldAccess::READWRITE, "aSequence",
                        "[]ModuleA/EnumA", RTConstValue());

    void const* pBlob = writer.getBlob(&nSize);
    CPPUNIT_ASSERT(pBlob);

    CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR,
                         key.setValue("", RegValueType::BINARY, const_cast<void*>(pBlob), nSize));

    void* readBlob = std::malloc(nSize);
    CPPUNIT_ASSERT(readBlob);
    CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR, key.getValue("", readBlob));

    typereg::Reader reader(readBlob, nSize);
    CPPUNIT_ASSERT(reader.isValid());

    return reader;
}

void RegistryTest::testShouldCreateStructKey()
{
    std::unique_ptr<Registry> myRegistry(new Registry());
    sal_uInt32 nSize;

    const sal_uInt16 FIELDCOUNT = 3;

    typereg::Reader reader = getStructReader(myRegistry.get(), nSize);

    CPPUNIT_ASSERT_EQUAL(OUString("Hello I am a structure"), reader.getDocumentation());
    CPPUNIT_ASSERT_EQUAL(OUString("ModuleA/StructA"), reader.getTypeName());
    CPPUNIT_ASSERT_EQUAL(FIELDCOUNT, reader.getFieldCount());

    CPPUNIT_ASSERT_EQUAL(OUString("I am an unsigned integer"), reader.getFieldDocumentation(0));
    CPPUNIT_ASSERT_EQUAL(OUString("unsigned integer"), reader.getFieldTypeName(0));
    CPPUNIT_ASSERT_EQUAL(OUString("asal_uInt32"), reader.getFieldName(0));

    CPPUNIT_ASSERT_EQUAL(OUString("I am an XInterface"), reader.getFieldDocumentation(1));
    CPPUNIT_ASSERT_EQUAL(OUString("stardiv/uno/XInterface"), reader.getFieldTypeName(1));
    CPPUNIT_ASSERT_EQUAL(OUString("aXInterface"), reader.getFieldName(1));

    CPPUNIT_ASSERT_EQUAL(OUString("I am a sequence"), reader.getFieldDocumentation(2));
    CPPUNIT_ASSERT_EQUAL(OUString("[]ModuleA/EnumA"), reader.getFieldTypeName(2));
    CPPUNIT_ASSERT_EQUAL(OUString("aSequence"), reader.getFieldName(2));
}

typereg::Reader getEnumReader(Registry* registry, sal_uInt32& nSize)
{
    RegistryKey key = createKey(registry, "EnumA");

    const sal_uInt16 FIELDCOUNT = 2;
    typereg::Writer writer(TYPEREG_VERSION_1, "Hello I am an enum", "", RT_TYPE_ENUM, true,
                           "ModuleA/EnumA", 0, FIELDCOUNT, 0, 0);

    RTConstValue aConst;
    aConst.m_type = RT_TYPE_UINT32;
    aConst.m_value.aULong = 1;

    writer.setFieldData(0, "I am an enum value", "", RTFieldAccess::CONST, "ENUM_VAL_1", "",
                        aConst);

    aConst.m_value.aULong = 10;
    writer.setFieldData(1, "I am an enum value", "", RTFieldAccess::CONST, "ENUM_VAL_2", "",
                        aConst);

    void const* pBlob = writer.getBlob(&nSize);
    CPPUNIT_ASSERT(pBlob);

    CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR,
                         key.setValue("", RegValueType::BINARY, const_cast<void*>(pBlob), nSize));

    void* readBlob = std::malloc(nSize);
    CPPUNIT_ASSERT(readBlob);
    CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR, key.getValue("", readBlob));

    typereg::Reader reader(readBlob, nSize);
    CPPUNIT_ASSERT(reader.isValid());

    return reader;
}

void RegistryTest::testShouldCreateEnumKey()
{
    std::unique_ptr<Registry> myRegistry(new Registry());
    sal_uInt32 nSize;

    const sal_uInt16 FIELDCOUNT = 2;

    typereg::Reader reader = getEnumReader(myRegistry.get(), nSize);

    CPPUNIT_ASSERT_EQUAL(OUString("Hello I am an enum"), reader.getDocumentation());
    CPPUNIT_ASSERT_EQUAL(OUString("ModuleA/EnumA"), reader.getTypeName());
    CPPUNIT_ASSERT_EQUAL(FIELDCOUNT, reader.getFieldCount());

    CPPUNIT_ASSERT_EQUAL(OUString("I am an enum value"), reader.getFieldDocumentation(0));
    CPPUNIT_ASSERT_EQUAL(OUString(""), reader.getFieldTypeName(0));
    CPPUNIT_ASSERT_EQUAL(OUString("ENUM_VAL_1"), reader.getFieldName(0));

    RTConstValue aEnumConst = reader.getFieldValue(0);
    CPPUNIT_ASSERT_EQUAL(RT_TYPE_UINT32, aEnumConst.m_type);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(1), aEnumConst.m_value.aULong);

    CPPUNIT_ASSERT_EQUAL(OUString("I am an enum value"), reader.getFieldDocumentation(1));
    CPPUNIT_ASSERT_EQUAL(OUString(""), reader.getFieldTypeName(1));
    CPPUNIT_ASSERT_EQUAL(OUString("ENUM_VAL_2"), reader.getFieldName(1));

    aEnumConst = reader.getFieldValue(1);
    CPPUNIT_ASSERT_EQUAL(RT_TYPE_UINT32, aEnumConst.m_type);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(10), aEnumConst.m_value.aULong);
}

typereg::Reader getInterfaceReader(Registry* registry, sal_uInt32& nSize)
{
    RegistryKey key = createKey(registry, "XInterfaceA");

    const sal_uInt16 FIELDCOUNT = 4;
    const sal_uInt16 METHODCOUNT = 1;

    typereg::Writer writer(TYPEREG_VERSION_1, "Hello I am an interface", "", RT_TYPE_INTERFACE,
                           true, "ModuleA/XInterfaceA", 0, FIELDCOUNT, METHODCOUNT, 0);

    writer.setFieldData(0, "I am a string", "", RTFieldAccess::READWRITE, "aString", "string",
                        RTConstValue());

    writer.setFieldData(1, "I am a structure", "", RTFieldAccess::READONLY, "aStruct",
                        "ModuleA/StructA", RTConstValue());

    writer.setFieldData(2, "I am an enum", "", RTFieldAccess::BOUND, "aEnum", "ModuleA/EnumA",
                        RTConstValue());

    RTConstValue aConst;
    aConst.m_type = RT_TYPE_UINT16;
    aConst.m_value.aUShort = 12;
    writer.setFieldData(3, "I am an unsigned short", "", RTFieldAccess::CONST, "aConstUShort",
                        "unsigned short", aConst);

    writer.setMethodData(0, "Hello I am the methodA", RTMethodMode::TWOWAY, "methodA", "double", 2,
                         1);
    writer.setMethodParameterData(0, 0, RT_PARAM_IN, "aStruct", "ModuleA/StructA");
    writer.setMethodParameterData(0, 1, RT_PARAM_INOUT, "aShort", "unsigned short");
    writer.setMethodExceptionTypeName(0, 0, "ModuleA/ExceptionA");

    void const* pBlob = writer.getBlob(&nSize);
    CPPUNIT_ASSERT(pBlob);

    CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR,
                         key.setValue("", RegValueType::BINARY, const_cast<void*>(pBlob), nSize));

    void* readBlob = std::malloc(nSize);
    CPPUNIT_ASSERT(readBlob);
    CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR, key.getValue("", readBlob));

    typereg::Reader reader(readBlob, nSize);
    CPPUNIT_ASSERT(reader.isValid());

    return reader;
}

void RegistryTest::testShouldCreateInterface()
{
    std::unique_ptr<Registry> myRegistry(new Registry());
    sal_uInt32 nSize;

    const sal_uInt16 FIELDCOUNT = 4;
    typereg::Reader reader = getInterfaceReader(myRegistry.get(), nSize);

    CPPUNIT_ASSERT_EQUAL(OUString("Hello I am an interface"), reader.getDocumentation());
    CPPUNIT_ASSERT_EQUAL(OUString("ModuleA/XInterfaceA"), reader.getTypeName());
    CPPUNIT_ASSERT_EQUAL(FIELDCOUNT, reader.getFieldCount());

    CPPUNIT_ASSERT_EQUAL(OUString("I am a string"), reader.getFieldDocumentation(0));
    CPPUNIT_ASSERT_EQUAL(OUString("string"), reader.getFieldTypeName(0));
    CPPUNIT_ASSERT_EQUAL(OUString("aString"), reader.getFieldName(0));

    CPPUNIT_ASSERT_EQUAL(OUString("I am a structure"), reader.getFieldDocumentation(1));
    CPPUNIT_ASSERT_EQUAL(OUString("ModuleA/StructA"), reader.getFieldTypeName(1));
    CPPUNIT_ASSERT_EQUAL(OUString("aStruct"), reader.getFieldName(1));

    CPPUNIT_ASSERT_EQUAL(OUString("I am an enum"), reader.getFieldDocumentation(2));
    CPPUNIT_ASSERT_EQUAL(OUString("ModuleA/EnumA"), reader.getFieldTypeName(2));
    CPPUNIT_ASSERT_EQUAL(OUString("aEnum"), reader.getFieldName(2));

    CPPUNIT_ASSERT_EQUAL(OUString("Hello I am the methodA"), reader.getMethodDocumentation(0));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), reader.getMethodCount());
    CPPUNIT_ASSERT_EQUAL(RTMethodMode::TWOWAY, reader.getMethodFlags(0));
    CPPUNIT_ASSERT_EQUAL(OUString("methodA"), reader.getMethodName(0));
    CPPUNIT_ASSERT_EQUAL(OUString("double"), reader.getMethodReturnTypeName(0));
    CPPUNIT_ASSERT_EQUAL(OUString("ModuleA/ExceptionA"), reader.getMethodExceptionTypeName(0, 0));

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2), reader.getMethodParameterCount(0));

    CPPUNIT_ASSERT_EQUAL(RT_PARAM_IN, reader.getMethodParameterFlags(0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("aStruct"), reader.getMethodParameterName(0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("ModuleA/StructA"), reader.getMethodParameterTypeName(0, 0));

    CPPUNIT_ASSERT_EQUAL(RT_PARAM_INOUT, reader.getMethodParameterFlags(0, 1));
    CPPUNIT_ASSERT_EQUAL(OUString("aShort"), reader.getMethodParameterName(0, 1));
    CPPUNIT_ASSERT_EQUAL(OUString("unsigned short"), reader.getMethodParameterTypeName(0, 1));
}

typereg::Reader getExceptionReader(Registry* registry, sal_uInt32& nSize)
{
    RegistryKey key = createKey(registry, "ExceptionA");

    const sal_uInt16 FIELDCOUNT = 1;

    typereg::Writer writer(TYPEREG_VERSION_1, "Hello I am an exception", "", RT_TYPE_EXCEPTION,
                           true, "ModuleA/ExceptionA", 0, FIELDCOUNT, 0, 0);

    writer.setFieldData(0, "I am an interface member", "", RTFieldAccess::READWRITE, "aSource",
                        "stardiv/uno/XInterface", RTConstValue());

    void const* pBlob = writer.getBlob(&nSize);
    CPPUNIT_ASSERT(pBlob);

    CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR,
                         key.setValue("", RegValueType::BINARY, const_cast<void*>(pBlob), nSize));

    void* readBlob = std::malloc(nSize);
    CPPUNIT_ASSERT(readBlob);
    CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR, key.getValue("", readBlob));

    typereg::Reader reader(readBlob, nSize);
    CPPUNIT_ASSERT(reader.isValid());

    return reader;
}

void RegistryTest::testShouldCreateException()
{
    std::unique_ptr<Registry> myRegistry(new Registry());
    sal_uInt32 nSize;

    const sal_uInt16 FIELDCOUNT = 1;
    typereg::Reader reader = getExceptionReader(myRegistry.get(), nSize);

    CPPUNIT_ASSERT_EQUAL(OUString("Hello I am an exception"), reader.getDocumentation());
    CPPUNIT_ASSERT_EQUAL(OUString("ModuleA/ExceptionA"), reader.getTypeName());
    CPPUNIT_ASSERT_EQUAL(FIELDCOUNT, reader.getFieldCount());

    CPPUNIT_ASSERT_EQUAL(OUString("I am an interface member"), reader.getFieldDocumentation(0));
    CPPUNIT_ASSERT_EQUAL(OUString("stardiv/uno/XInterface"), reader.getFieldTypeName(0));
    CPPUNIT_ASSERT_EQUAL(OUString("aSource"), reader.getFieldName(0));
}

typereg::Reader getServiceReader(Registry* registry, sal_uInt32& nSize)
{
    RegistryKey key = createKey(registry, "ServiceA");

    const sal_uInt16 FIELDCOUNT = 1;
    const sal_uInt16 REFERENCECOUNT = 4;

    typereg::Writer writer(TYPEREG_VERSION_1, "Hello I am a service", "", RT_TYPE_SERVICE, true,
                           "ModuleA/ServiceA", 0, FIELDCOUNT, 0, REFERENCECOUNT);

    writer.setReferenceData(0, "I am a reference to a supported interface",
                            RTReferenceType::SUPPORTS, RTFieldAccess::OPTIONAL,
                            "ModuleA/XInterfaceA");
    writer.setReferenceData(1, "I am a reference to an observed interface",
                            RTReferenceType::OBSERVES, RTFieldAccess::INVALID,
                            "ModuleA/XInterfaceA");
    writer.setReferenceData(2, "I am a reference to an exported service", RTReferenceType::EXPORTS,
                            RTFieldAccess::INVALID, "ModuleA/XServiceB");
    writer.setReferenceData(3, "I am a reference to a needed service", RTReferenceType::NEEDS,
                            RTFieldAccess::INVALID, "ModuleA/XServiceB");

    void const* pBlob = writer.getBlob(&nSize);
    CPPUNIT_ASSERT(pBlob);

    CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR,
                         key.setValue("", RegValueType::BINARY, const_cast<void*>(pBlob), nSize));

    void* readBlob = std::malloc(nSize);
    CPPUNIT_ASSERT(readBlob);
    CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR, key.getValue("", readBlob));

    typereg::Reader reader(readBlob, nSize);
    CPPUNIT_ASSERT(reader.isValid());

    return reader;
}

void RegistryTest::testShouldCreateService()
{
    std::unique_ptr<Registry> myRegistry(new Registry());
    sal_uInt32 nSize;

    const sal_uInt16 FIELDCOUNT = 1;
    const sal_uInt16 REFERENCECOUNT = 4;

    typereg::Reader reader = getServiceReader(myRegistry.get(), nSize);

    CPPUNIT_ASSERT_EQUAL(OUString("Hello I am a service"), reader.getDocumentation());
    CPPUNIT_ASSERT_EQUAL(OUString("ModuleA/ServiceA"), reader.getTypeName());
    CPPUNIT_ASSERT_EQUAL(FIELDCOUNT, reader.getFieldCount());
    CPPUNIT_ASSERT_EQUAL(REFERENCECOUNT, reader.getReferenceCount());

    CPPUNIT_ASSERT_EQUAL(OUString("I am a reference to a supported interface"),
                         reader.getReferenceDocumentation(0));
    CPPUNIT_ASSERT_EQUAL(RTFieldAccess::OPTIONAL, reader.getReferenceFlags(0));
    CPPUNIT_ASSERT_EQUAL(RTReferenceType::SUPPORTS, reader.getReferenceSort(0));
    CPPUNIT_ASSERT_EQUAL(OUString("ModuleA/XInterfaceA"), reader.getReferenceTypeName(0));

    CPPUNIT_ASSERT_EQUAL(OUString("I am a reference to an observed interface"),
                         reader.getReferenceDocumentation(1));
    CPPUNIT_ASSERT_EQUAL(RTFieldAccess::INVALID, reader.getReferenceFlags(1));
    CPPUNIT_ASSERT_EQUAL(RTReferenceType::OBSERVES, reader.getReferenceSort(1));
    CPPUNIT_ASSERT_EQUAL(OUString("ModuleA/XInterfaceA"), reader.getReferenceTypeName(1));

    CPPUNIT_ASSERT_EQUAL(OUString("I am a reference to an exported service"),
                         reader.getReferenceDocumentation(2));
    CPPUNIT_ASSERT_EQUAL(RTFieldAccess::INVALID, reader.getReferenceFlags(2));
    CPPUNIT_ASSERT_EQUAL(RTReferenceType::EXPORTS, reader.getReferenceSort(2));
    CPPUNIT_ASSERT_EQUAL(OUString("ModuleA/XServiceB"), reader.getReferenceTypeName(2));

    CPPUNIT_ASSERT_EQUAL(OUString("I am a reference to a needed service"),
                         reader.getReferenceDocumentation(3));
    CPPUNIT_ASSERT_EQUAL(RTFieldAccess::INVALID, reader.getReferenceFlags(3));
    CPPUNIT_ASSERT_EQUAL(RTReferenceType::NEEDS, reader.getReferenceSort(3));
    CPPUNIT_ASSERT_EQUAL(OUString("ModuleA/XServiceB"), reader.getReferenceTypeName(3));
}

typereg::Reader getConstantsGroupReader(Registry* registry, sal_uInt32& nSize)
{
    RegistryKey key = createKey(registry, "ServiceA");

    const sal_uInt16 FIELDCOUNT = 11;

    typereg::Writer writer(TYPEREG_VERSION_1, "Hello I am a constants group", "", RT_TYPE_CONSTANTS,
                           true, "ModuleA/ConstansA", 0, FIELDCOUNT, 0, 0);

    RTConstValue aConst;

    aConst.m_type = RT_TYPE_BOOL;
    aConst.m_value.aBool = true;
    writer.setFieldData(0, "I am a boolean", "", RTFieldAccess::CONST, "aConstBool", "boolean",
                        aConst);

    aConst.m_type = RT_TYPE_BYTE;
    aConst.m_value.aByte = 127;
    writer.setFieldData(1, "I am a byte", "", RTFieldAccess::CONST, "aConstByte", "byte", aConst);

    aConst.m_type = RT_TYPE_INT16;
    aConst.m_value.aShort = -10;
    writer.setFieldData(2, "I am a short", "", RTFieldAccess::CONST, "aConstShort", "short",
                        aConst);

    aConst.m_type = RT_TYPE_UINT16;
    aConst.m_value.aUShort = 10;
    writer.setFieldData(3, "I am an unsigned short", "", RTFieldAccess::CONST, "aConstUShort",
                        "unsigned short", aConst);

    aConst.m_type = RT_TYPE_INT32;
    aConst.m_value.aLong = -100'000;
    writer.setFieldData(4, "I am a long", "", RTFieldAccess::CONST, "aConstLong", "long", aConst);

    aConst.m_type = RT_TYPE_UINT32;
    aConst.m_value.aULong = 100'000;
    writer.setFieldData(5, "I am an unsigned long", "", RTFieldAccess::CONST, "aConstULong",
                        "unsigned long", aConst);

    aConst.m_type = RT_TYPE_INT64;
    aConst.m_value.aHyper = -100'000'000;
    writer.setFieldData(6, "I am a hyper", "", RTFieldAccess::CONST, "aConstHyper", "hyper",
                        aConst);

    aConst.m_type = RT_TYPE_UINT64;
    aConst.m_value.aUHyper = 100'000'000;
    writer.setFieldData(7, "I am an unsigned hyper", "", RTFieldAccess::CONST, "aConstUHyper",
                        "unsigned hyper", aConst);

    aConst.m_type = RT_TYPE_FLOAT;
    aConst.m_value.aFloat = -2e-10f;
    writer.setFieldData(8, "I am a float", "", RTFieldAccess::CONST, "aConstFloat", "float",
                        aConst);

    aConst.m_type = RT_TYPE_DOUBLE;
    aConst.m_value.aDouble = -2e-100;
    writer.setFieldData(9, "I am a double", "", RTFieldAccess::CONST, "aConstDouble", "double",
                        aConst);

    aConst.m_type = RT_TYPE_STRING;
    OUString tmpStr("this is a unicode string");
    aConst.m_value.aString = tmpStr.getStr();
    writer.setFieldData(10, "I am a string", "", RTFieldAccess::CONST, "aConstString", "string",
                        aConst);

    void const* pBlob = writer.getBlob(&nSize);
    CPPUNIT_ASSERT(pBlob);

    CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR,
                         key.setValue("", RegValueType::BINARY, const_cast<void*>(pBlob), nSize));

    void* readBlob = std::malloc(nSize);
    CPPUNIT_ASSERT(readBlob);
    CPPUNIT_ASSERT_EQUAL(RegError::NO_ERROR, key.getValue("", readBlob));

    typereg::Reader reader(readBlob, nSize);
    CPPUNIT_ASSERT(reader.isValid());

    return reader;
}

void RegistryTest::testShouldCreateConstantsGroup()
{
    std::unique_ptr<Registry> myRegistry(new Registry());
    sal_uInt32 nSize;

    const sal_uInt16 FIELDCOUNT = 11;

    typereg::Reader reader = getConstantsGroupReader(myRegistry.get(), nSize);

    CPPUNIT_ASSERT_EQUAL(OUString("Hello I am a constants group"), reader.getDocumentation());
    CPPUNIT_ASSERT_EQUAL(OUString("ModuleA/ConstansA"), reader.getTypeName());
    CPPUNIT_ASSERT_EQUAL(FIELDCOUNT, reader.getFieldCount());

    CPPUNIT_ASSERT_EQUAL(OUString("I am a boolean"), reader.getFieldDocumentation(0));
    CPPUNIT_ASSERT_EQUAL(OUString("boolean"), reader.getFieldTypeName(0));
    CPPUNIT_ASSERT_EQUAL(OUString("aConstBool"), reader.getFieldName(0));

    RTConstValue aReadConst = reader.getFieldValue(0);
    CPPUNIT_ASSERT_EQUAL(RT_TYPE_BOOL, aReadConst.m_type);
    CPPUNIT_ASSERT_EQUAL(true, aReadConst.m_value.aBool);

    CPPUNIT_ASSERT_EQUAL(OUString("I am a byte"), reader.getFieldDocumentation(1));
    CPPUNIT_ASSERT_EQUAL(OUString("byte"), reader.getFieldTypeName(1));
    CPPUNIT_ASSERT_EQUAL(OUString("aConstByte"), reader.getFieldName(1));

    aReadConst = reader.getFieldValue(1);
    CPPUNIT_ASSERT_EQUAL(RT_TYPE_BYTE, aReadConst.m_type);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int8>(127), aReadConst.m_value.aByte);

    CPPUNIT_ASSERT_EQUAL(OUString("I am a short"), reader.getFieldDocumentation(2));
    CPPUNIT_ASSERT_EQUAL(OUString("short"), reader.getFieldTypeName(2));
    CPPUNIT_ASSERT_EQUAL(OUString("aConstShort"), reader.getFieldName(2));

    aReadConst = reader.getFieldValue(2);
    CPPUNIT_ASSERT_EQUAL(RT_TYPE_INT16, aReadConst.m_type);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(-10), aReadConst.m_value.aShort);

    CPPUNIT_ASSERT_EQUAL(OUString("I am an unsigned short"), reader.getFieldDocumentation(3));
    CPPUNIT_ASSERT_EQUAL(OUString("unsigned short"), reader.getFieldTypeName(3));
    CPPUNIT_ASSERT_EQUAL(OUString("aConstUShort"), reader.getFieldName(3));

    aReadConst = reader.getFieldValue(3);
    CPPUNIT_ASSERT_EQUAL(RT_TYPE_UINT16, aReadConst.m_type);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(10), aReadConst.m_value.aUShort);

    CPPUNIT_ASSERT_EQUAL(OUString("I am a long"), reader.getFieldDocumentation(4));
    CPPUNIT_ASSERT_EQUAL(OUString("long"), reader.getFieldTypeName(4));
    CPPUNIT_ASSERT_EQUAL(OUString("aConstLong"), reader.getFieldName(4));

    aReadConst = reader.getFieldValue(4);
    CPPUNIT_ASSERT_EQUAL(RT_TYPE_INT32, aReadConst.m_type);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-100'000), aReadConst.m_value.aLong);

    CPPUNIT_ASSERT_EQUAL(OUString("I am an unsigned long"), reader.getFieldDocumentation(5));
    CPPUNIT_ASSERT_EQUAL(OUString("unsigned long"), reader.getFieldTypeName(5));
    CPPUNIT_ASSERT_EQUAL(OUString("aConstULong"), reader.getFieldName(5));

    aReadConst = reader.getFieldValue(5);
    CPPUNIT_ASSERT_EQUAL(RT_TYPE_UINT32, aReadConst.m_type);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(100'000), aReadConst.m_value.aULong);

    CPPUNIT_ASSERT_EQUAL(OUString("I am a hyper"), reader.getFieldDocumentation(6));
    CPPUNIT_ASSERT_EQUAL(OUString("hyper"), reader.getFieldTypeName(6));
    CPPUNIT_ASSERT_EQUAL(OUString("aConstHyper"), reader.getFieldName(6));

    aReadConst = reader.getFieldValue(6);
    CPPUNIT_ASSERT_EQUAL(RT_TYPE_INT64, aReadConst.m_type);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(-100'000'000), aReadConst.m_value.aHyper);

    CPPUNIT_ASSERT_EQUAL(OUString("I am an unsigned hyper"), reader.getFieldDocumentation(7));
    CPPUNIT_ASSERT_EQUAL(OUString("unsigned hyper"), reader.getFieldTypeName(7));
    CPPUNIT_ASSERT_EQUAL(OUString("aConstUHyper"), reader.getFieldName(7));

    aReadConst = reader.getFieldValue(7);
    CPPUNIT_ASSERT_EQUAL(RT_TYPE_UINT64, aReadConst.m_type);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt64>(100'000'000), aReadConst.m_value.aUHyper);

    CPPUNIT_ASSERT_EQUAL(OUString("I am a float"), reader.getFieldDocumentation(8));
    CPPUNIT_ASSERT_EQUAL(OUString("float"), reader.getFieldTypeName(8));
    CPPUNIT_ASSERT_EQUAL(OUString("aConstFloat"), reader.getFieldName(8));

    aReadConst = reader.getFieldValue(8);
    CPPUNIT_ASSERT_EQUAL(RT_TYPE_FLOAT, aReadConst.m_type);
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(-2e-10f), aReadConst.m_value.aFloat);

    CPPUNIT_ASSERT_EQUAL(OUString("I am a double"), reader.getFieldDocumentation(9));
    CPPUNIT_ASSERT_EQUAL(OUString("double"), reader.getFieldTypeName(9));
    CPPUNIT_ASSERT_EQUAL(OUString("aConstDouble"), reader.getFieldName(9));

    aReadConst = reader.getFieldValue(9);
    CPPUNIT_ASSERT_EQUAL(RT_TYPE_DOUBLE, aReadConst.m_type);
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(-2e-100), aReadConst.m_value.aDouble);

    CPPUNIT_ASSERT_EQUAL(OUString("I am a string"), reader.getFieldDocumentation(10));
    CPPUNIT_ASSERT_EQUAL(OUString("string"), reader.getFieldTypeName(10));
    CPPUNIT_ASSERT_EQUAL(OUString("aConstString"), reader.getFieldName(10));

    aReadConst = reader.getFieldValue(10);
    CPPUNIT_ASSERT_EQUAL(RT_TYPE_STRING, aReadConst.m_type);
    CPPUNIT_ASSERT_EQUAL(OUString("this is a unicode string"),
                         OUString(aReadConst.m_value.aString));
}

CPPUNIT_TEST_SUITE_REGISTRATION(RegistryTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
