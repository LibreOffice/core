/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_registry.hxx"

#include <iostream>
#include <stdio.h>
#include <string.h>

#include "registry/registry.hxx"
#include "registry/reflread.hxx"
#include "registry/reflwrit.hxx"
#include "regdiagnose.h"
#include <rtl/alloc.h>
#include <rtl/ustring.hxx>

using namespace std;
using namespace rtl;

void test_coreReflection()
{
    Registry *myRegistry = new Registry();

    RegistryKey rootKey, key1, key2, key3, key4 ,key5, key6, key7, key8;

    REG_ENSURE(!myRegistry->create(OUString::createFromAscii("ucrtest.rdb")), "testCoreReflection error 1");
    REG_ENSURE(!myRegistry->openRootKey(rootKey), "testCoreReflection error 2");

    REG_ENSURE(!rootKey.createKey(OUString::createFromAscii("UCR"), key1), "testCoreReflection error 3");
    REG_ENSURE(!key1.createKey(OUString::createFromAscii("ModuleA"), key2), "testCoreReflection error 4");
    REG_ENSURE(!key2.createKey(OUString::createFromAscii("StructA"), key3), "testCoreReflection error 5");
    REG_ENSURE(!key2.createKey(OUString::createFromAscii("EnumA"), key4), "testCoreReflection error 6");
    REG_ENSURE(!key2.createKey(OUString::createFromAscii("XInterfaceA"), key5), "testCoreReflection error 7");
    REG_ENSURE(!key2.createKey(OUString::createFromAscii("ExceptionA"), key6), "testCoreReflection error 8");
    REG_ENSURE(!key2.createKey(OUString::createFromAscii("ServiceA"), key7), "testCoreReflection error 8a");
    REG_ENSURE(!key2.createKey(OUString::createFromAscii("ConstantsA"), key8), "testCoreReflection error 8b");

    {
        RegistryTypeWriter writer(RT_TYPE_MODULE,
                                  OUString::createFromAscii("ModuleA"),
                                  OUString(), 11, 0, 0);

        RTConstValue aConst;

        writer.setDoku(OUString::createFromAscii("Hallo ich bin ein Modul"));
        writer.setFileName(OUString::createFromAscii("DummyFile"));

        aConst.m_type = RT_TYPE_BOOL;
        aConst.m_value.aBool = sal_True;
        writer.setFieldData(0, OUString::createFromAscii("aConstBool"),
                            OUString::createFromAscii("boolean"),
                            OUString::createFromAscii("ich bin ein boolean"),
                            OUString::createFromAscii("DummyFile"), RT_ACCESS_CONST, aConst);
        aConst.m_type = RT_TYPE_BYTE;
        aConst.m_value.aByte = 127;
        writer.setFieldData(1, OUString::createFromAscii("aConstByte"),
                            OUString::createFromAscii("byte"),
                            OUString::createFromAscii("ich bin ein byte"),
                            OUString::createFromAscii("DummyFile"),  RT_ACCESS_CONST, aConst);
        aConst.m_type = RT_TYPE_INT16;
        aConst.m_value.aShort = -10;
        writer.setFieldData(2, OUString::createFromAscii("aConstShort"),
                            OUString::createFromAscii("short"),
                            OUString::createFromAscii("ich bin ein short"),
                            OUString::createFromAscii("DummyFile"),  RT_ACCESS_CONST, aConst);
        aConst.m_type = RT_TYPE_UINT16;
        aConst.m_value.aUShort = 10;
        writer.setFieldData(3, OUString::createFromAscii("aConstUShort"),
                            OUString::createFromAscii("unsigned short"),
                            OUString::createFromAscii("ich bin ein unsigned short"),
                            OUString::createFromAscii("DummyFile"), RT_ACCESS_CONST, aConst);
        aConst.m_type = RT_TYPE_INT32;
        aConst.m_value.aLong = -100000;
        writer.setFieldData(4, OUString::createFromAscii("aConstLong"),
                            OUString::createFromAscii("long"),
                            OUString::createFromAscii("ich bin ein long"),
                            OUString::createFromAscii("DummyFile"), RT_ACCESS_CONST, aConst);
        aConst.m_type = RT_TYPE_UINT32;
        aConst.m_value.aULong = 100000;
        writer.setFieldData(5, OUString::createFromAscii("aConstULong"),
                            OUString::createFromAscii("unsigned long"),
                            OUString::createFromAscii("ich bin ein unsigned long"),
                            OUString::createFromAscii("DummyFile"), RT_ACCESS_CONST, aConst);
        aConst.m_type = RT_TYPE_INT64;
        aConst.m_value.aHyper = -100000000;
        writer.setFieldData(6, OUString::createFromAscii("aConstHyper"),
                            OUString::createFromAscii("hyper"),
                            OUString::createFromAscii("ich bin ein hyper"),
                            OUString::createFromAscii("DummyFile"), RT_ACCESS_CONST, aConst);
        aConst.m_type = RT_TYPE_UINT64;
        aConst.m_value.aUHyper = 100000000;
        writer.setFieldData(7, OUString::createFromAscii("aConstULong"),
                            OUString::createFromAscii("unsigned long"),
                            OUString::createFromAscii("ich bin ein unsigned long"),
                            OUString::createFromAscii("DummyFile"), RT_ACCESS_CONST, aConst);
        aConst.m_type = RT_TYPE_FLOAT;
        aConst.m_value.aFloat = -2e-10f;
        writer.setFieldData(8, OUString::createFromAscii("aConstFloat"),
                            OUString::createFromAscii("float"),
                            OUString::createFromAscii("ich bin ein float"),
                            OUString::createFromAscii("DummyFile"), RT_ACCESS_CONST, aConst);
        aConst.m_type = RT_TYPE_DOUBLE;
        aConst.m_value.aDouble = -2e-100; writer.setFieldData(9, OUString::createFromAscii("aConstDouble"),
                                                              OUString::createFromAscii("double"),
                                                              OUString::createFromAscii("ich bin ein double"),
                                                              OUString::createFromAscii("DummyFile"), RT_ACCESS_CONST, aConst);
        aConst.m_type = RT_TYPE_STRING;
        OUString tmpStr(OUString::createFromAscii( "dies ist ein unicode string" ));
        aConst.m_value.aString = tmpStr.getStr();

        writer.setFieldData(10, OUString::createFromAscii("aConstString"),
                            OUString::createFromAscii("string"),
                            OUString::createFromAscii("ich bin ein string"),
                            OUString::createFromAscii("DummyFile"), RT_ACCESS_CONST, aConst);

        const sal_uInt8* pBlop = writer.getBlop();
        sal_uInt32      aBlopSize = writer.getBlopSize();

        REG_ENSURE(!key2.setValue(OUString(), RG_VALUETYPE_BINARY, (void*)pBlop, aBlopSize), "testCoreReflection error 9");

        sal_uInt8* readBlop = (sal_uInt8*)rtl_allocateMemory(aBlopSize);
        REG_ENSURE(!key2.getValue(OUString(), (void*)readBlop) , "testCoreReflection error 9a");

        RegistryTypeReader reader(readBlop, aBlopSize, sal_True);

        if (reader.isValid())
        {
            REG_ENSURE(reader.getTypeName().equals(OUString::createFromAscii("ModuleA")), "testCoreReflection error 9a2");

            RTConstValue aReadConst = reader.getFieldConstValue(4);
            REG_ENSURE( aReadConst.m_type == RT_TYPE_INT32, "testCoreReflection error 9a3");
            REG_ENSURE( aReadConst.m_value.aLong == -100000, "testCoreReflection error 9a4");

            aReadConst = reader.getFieldConstValue(6);
            REG_ENSURE( aReadConst.m_type == RT_TYPE_INT64, "testCoreReflection error 9a5");
            REG_ENSURE( aReadConst.m_value.aHyper == -100000000, "testCoreReflection error 9a6");

            aReadConst = reader.getFieldConstValue(10);
            OString aConstStr = OUStringToOString(aConst.m_value.aString, RTL_TEXTENCODING_ASCII_US);
            REG_ENSURE(aConstStr.equals("dies ist ein unicode string"), "testCoreReflection error 9b");
        }

    }

    {
        RegistryTypeWriter writer(RT_TYPE_STRUCT,
                                  OUString::createFromAscii("ModuleA/StructA"),
                                  OUString(), 3, 0, 0);

        writer.setDoku(OUString::createFromAscii("Hallo ich bin eine Struktur"));
        writer.setFileName(OUString::createFromAscii("DummyFile"));

        writer.setFieldData(0, OUString::createFromAscii("asal_uInt32"),
                            OUString::createFromAscii("unsigned long"),
                            OUString(), OUString(), RT_ACCESS_READWRITE);
        writer.setFieldData(1, OUString::createFromAscii("aXInterface"),
                            OUString::createFromAscii("stardiv/uno/XInterface"),
                            OUString(), OUString(), RT_ACCESS_READWRITE);
        writer.setFieldData(2, OUString::createFromAscii("aSequence"),
                            OUString::createFromAscii("[]ModuleA/EnumA"),
                            OUString(), OUString(), RT_ACCESS_READWRITE);

        const sal_uInt8* pBlop = writer.getBlop();
        sal_uInt32      aBlopSize = writer.getBlopSize();

        REG_ENSURE(!key3.setValue(OUString(), RG_VALUETYPE_BINARY, (void*)pBlop, aBlopSize), "testCoreReflection error 9a");
    }

    {
        RegistryTypeWriter writer(RT_TYPE_ENUM,
                                  OUString::createFromAscii("ModuleA/EnumA"),
                                  OUString(), 2, 0, 0);

        RTConstValue aConst;

        aConst.m_type = RT_TYPE_UINT32;
        aConst.m_value.aULong = 10;

        writer.setDoku(OUString::createFromAscii("Hallo ich bin ein Enum"));
        writer.setFileName(OUString::createFromAscii("DummyFile"));

        writer.setFieldData(0, OUString::createFromAscii("ENUM_VAL_1"),
                            OUString(), OUString::createFromAscii("ich bin ein enum value"),
                            OUString(), RT_ACCESS_CONST, aConst);

        aConst.m_value.aULong = 10;
        writer.setFieldData(1, OUString::createFromAscii("ENUM_VAL_2"),
                            OUString(), OUString(), OUString(), RT_ACCESS_CONST, aConst);

        const sal_uInt8* pBlop = writer.getBlop();
        sal_uInt32      aBlopSize = writer.getBlopSize();

        REG_ENSURE(!key4.setValue(OUString(), RG_VALUETYPE_BINARY, (void*)pBlop, aBlopSize), "testCoreReflection error 9b");
    }

    {
        RegistryTypeWriter writer(RT_TYPE_INTERFACE,
                                  OUString::createFromAscii("ModuleA/XInterfaceA"),
                                  OUString::createFromAscii("stardiv/uno/XInterface"),
                                  4, 1, 0);
        RTConstValue aConst;

        RTUik aUik = {1,2,3,4,5};

        writer.setUik(aUik);
        writer.setDoku(OUString::createFromAscii("Hallo ich bin ein Interface"));
        writer.setFileName(OUString::createFromAscii("DummyFile"));

        writer.setFieldData(0, OUString::createFromAscii("aString"),
                            OUString::createFromAscii("string"), OUString(), OUString(), RT_ACCESS_READWRITE);
        writer.setFieldData(1, OUString::createFromAscii("aStruct"),
                            OUString::createFromAscii("ModuleA/StructA"),
                            OUString(), OUString(), RT_ACCESS_READONLY);
        writer.setFieldData(2, OUString::createFromAscii("aEnum"),
                            OUString::createFromAscii("ModuleA/EnumA"), OUString(), OUString(), RT_ACCESS_BOUND);
        aConst.m_type = RT_TYPE_UINT16;
        aConst.m_value.aUShort = 12;
        writer.setFieldData(3, OUString::createFromAscii("aConstUShort"),
                            OUString::createFromAscii("unsigned short"), OUString(),
                            OUString::createFromAscii("DummyFile"), RT_ACCESS_CONST, aConst);

        writer.setMethodData(0, OUString::createFromAscii("methodA"),
                             OUString::createFromAscii("double"), RT_MODE_TWOWAY, 2, 1,
                             OUString::createFromAscii("Hallo ich bin die methodA"));
        writer.setParamData(0, 0, OUString::createFromAscii("ModuleA/StructA"),
                            OUString::createFromAscii("aStruct"), RT_PARAM_IN);
        writer.setParamData(0, 1, OUString::createFromAscii("unsigned short"),
                            OUString::createFromAscii("aShort"), RT_PARAM_INOUT);
        writer.setExcData(0, 0, OUString::createFromAscii("ModuleA/ExceptionA"));

        const sal_uInt8* pBlop = writer.getBlop();
        sal_uInt32      aBlopSize = writer.getBlopSize();

        REG_ENSURE(!key5.setValue(OUString(), RG_VALUETYPE_BINARY, (void*)pBlop, aBlopSize), "testCoreReflection error 9c");

        sal_uInt8* readBlop = (sal_uInt8*)rtl_allocateMemory(aBlopSize);
        REG_ENSURE(!key5.getValue(OUString(), (void*)readBlop) , "testCoreReflection error 9c1");

        RegistryTypeReader reader(readBlop, aBlopSize, sal_True);

        if (reader.isValid())
        {
            REG_ENSURE(reader.getTypeName().equals(OUString::createFromAscii("ModuleA/XInterfaceA")), "testCoreReflection error 9c2");

            RTUik retUik;
            reader.getUik(retUik);
            REG_ENSURE(retUik.m_Data1 = 1, "testCoreReflection error 9c3");
            REG_ENSURE(retUik.m_Data2 = 2, "testCoreReflection error 9c4");
            REG_ENSURE(retUik.m_Data3 = 3, "testCoreReflection error 9c5");
            REG_ENSURE(retUik.m_Data4 = 4, "testCoreReflection error 9c6");
            REG_ENSURE(retUik.m_Data5 = 5, "testCoreReflection error 9c7");
        }

    }

    {
        RegistryTypeWriter writer(RT_TYPE_EXCEPTION,
                                  OUString::createFromAscii("ModuleA/ExceptionA"),
                                  OUString(), 1, 0, 0);

        writer.setDoku(OUString::createFromAscii("Hallo ich bin eine Exception"));

        writer.setFieldData(0, OUString::createFromAscii("aSource"),
                            OUString::createFromAscii("stardiv/uno/XInterface"),
                            OUString::createFromAscii("ich bin ein interface member"),
                            OUString(), RT_ACCESS_READWRITE);

        const sal_uInt8* pBlop = writer.getBlop();
        sal_uInt32      aBlopSize = writer.getBlopSize();

        REG_ENSURE(!key6.setValue(OUString(), RG_VALUETYPE_BINARY, (void*)pBlop, aBlopSize), "testCoreReflection error 9d");
    }

    {
        RegistryTypeWriter writer(RT_TYPE_SERVICE,
                                  OUString::createFromAscii("ModuleA/ServiceA"),
                                  OUString(), 1, 0, 4);

        writer.setDoku(OUString::createFromAscii("Hallo ich bin ein Service"));
        writer.setFileName(OUString::createFromAscii("DummyFile"));

        writer.setFieldData(0, OUString::createFromAscii("aProperty"),
                            OUString::createFromAscii("stardiv/uno/XInterface"),
                            OUString::createFromAscii("ich bin eine property"),
                            OUString(), RT_ACCESS_READWRITE);

        writer.setReferenceData(0, OUString::createFromAscii("ModuleA/XInterfaceA"), RT_REF_SUPPORTS,
                                OUString::createFromAscii("Hallo ich eine Reference auf ein supported interface"),
                                RT_ACCESS_OPTIONAL);
        writer.setReferenceData(1, OUString::createFromAscii("ModuleA/XInterfaceA"), RT_REF_OBSERVES,
                                OUString::createFromAscii("Hallo ich eine Reference auf ein observed interface"));
        writer.setReferenceData(2, OUString::createFromAscii("ModuleA/ServiceB"), RT_REF_EXPORTS,
                                OUString::createFromAscii("Hallo ich eine Reference auf einen exported service"));
        writer.setReferenceData(3, OUString::createFromAscii("ModuleA/ServiceB"), RT_REF_NEEDS,
                                OUString::createFromAscii("Hallo ich eine Reference auf einen needed service"));

        const sal_uInt8* pBlop = writer.getBlop();
        sal_uInt32      aBlopSize = writer.getBlopSize();

        REG_ENSURE(!key7.setValue(OUString(), RG_VALUETYPE_BINARY, (void*)pBlop, aBlopSize), "testCoreReflection error 9e");
        sal_uInt8* readBlop = (sal_uInt8*)rtl_allocateMemory(aBlopSize);
        REG_ENSURE(!key7.getValue(OUString(), (void*)readBlop) , "testCoreReflection error 9e2");

        RegistryTypeReader reader(readBlop, aBlopSize, sal_True);

        if (reader.isValid())
        {
            REG_ENSURE(reader.getTypeName().equals(OUString::createFromAscii("ModuleA/ServiceA")), "testCoreReflection error 9e3");

            sal_uInt32 referenceCount = reader.getReferenceCount();
            REG_ENSURE( referenceCount == 4, "testCoreReflection error 9e4");

            OUString refName = reader.getReferenceName(0);
            REG_ENSURE(refName.equals(OUString::createFromAscii("ModuleA/XInterfaceA")), "testCoreReflection error 9e5");
        }
    }

    {
        RegistryTypeWriter writer(RT_TYPE_CONSTANTS,
                                  OUString::createFromAscii("ModuleA/ConstansA"),
                                  OUString(), 3, 0, 0);

        RTConstValue aConst;

        writer.setDoku(OUString::createFromAscii("Hallo ich bin eine Constants Group"));
        writer.setFileName(OUString::createFromAscii("DummyFile"));

        aConst.m_type = RT_TYPE_BOOL;
        aConst.m_value.aBool = sal_True;
        writer.setFieldData(0, OUString::createFromAscii("ConstantsA_aConstBool"),
                            OUString::createFromAscii("boolean"),
                            OUString::createFromAscii("ich bin ein boolean"),
                            OUString::createFromAscii("DummyFile"), RT_ACCESS_CONST, aConst);
        aConst.m_type = RT_TYPE_BYTE;
        aConst.m_value.aByte = 127;
        writer.setFieldData(1, OUString::createFromAscii("ConstantsA_aConstByte"),
                            OUString::createFromAscii("byte"),
                            OUString::createFromAscii("ich bin ein byte"),
                            OUString::createFromAscii("DummyFile"),  RT_ACCESS_CONST, aConst);
        aConst.m_type = RT_TYPE_INT16;
        aConst.m_value.aShort = -10;
        writer.setFieldData(2, OUString::createFromAscii("ConstantsA_aConstShort"),
                            OUString::createFromAscii("short"),
                            OUString::createFromAscii("ich bin ein short"),
                            OUString::createFromAscii("DummyFile"),  RT_ACCESS_CONST, aConst);

        const sal_uInt8* pBlop = writer.getBlop();
        sal_uInt32      aBlopSize = writer.getBlopSize();

        REG_ENSURE(!key8.setValue(OUString(), RG_VALUETYPE_BINARY, (void*)pBlop, aBlopSize), "testCoreReflection error 9f");
    }

//  REG_ENSURE(!myRegistry->destroy(NULL), "testCoreReflection error 10");
    delete myRegistry;

    cout << "test_coreReflection() Ok!\n";
}

void test_registry_CppApi()
{
    Registry *myRegistry = new Registry();

    RegistryKey rootKey, key1, key2, key3, key4 ,key5, key6, key7, key8, key9;

    REG_ENSURE(!myRegistry->create(OUString::createFromAscii("test.rdb")), "test_registry_CppApi error 1");
    REG_ENSURE(!myRegistry->openRootKey(rootKey), "test_registry_CppApi error 2");

    REG_ENSURE(!rootKey.createKey(OUString::createFromAscii("myFirstKey"), key1), "test_registry_CppApi error 3");
    REG_ENSURE(!rootKey.createKey(OUString::createFromAscii("mySecondKey"), key2), "test_registry_CppApi error 4");
    REG_ENSURE(!key1.createKey(OUString::createFromAscii("X"), key3), "test_registry_CppApi error 5");
    REG_ENSURE(!key1.createKey(OUString::createFromAscii("mySecondSubKey"), key4), "test_registry_CppApi error 6");
    REG_ENSURE(!rootKey.createKey(OUString::createFromAscii("myThirdKey"), key5), "test_registry_CppApi error 6a");

    REG_ENSURE(!key5.createKey(OUString::createFromAscii("1"), key4), "test_registry_CppApi error 6b");
    REG_ENSURE(!key4.createKey(OUString::createFromAscii("2"), key3), "test_registry_CppApi error 6c");
    REG_ENSURE(!key5.openKey(OUString::createFromAscii("1"), key4), "test_registry_CppApi error 6d");
    REG_ENSURE(!rootKey.openKey(OUString::createFromAscii("/myThirdKey/1"), key4), "test_registry_CppApi error 6e");
    REG_ENSURE(key4.getName().equals(OUString::createFromAscii("/myThirdKey/1")), "test_registry_CppApi error 6f");

    REG_ENSURE(!rootKey.createKey(OUString::createFromAscii("myFourthKey"), key6), "test_registry_CppApi error 7");
    REG_ENSURE(!rootKey.createKey(OUString::createFromAscii("myFifthKey"), key6), "test_registry_CppApi error 7a");
    REG_ENSURE(!rootKey.createKey(OUString::createFromAscii("mySixthKey"), key6), "test_registry_CppApi error 7b");

    // Link Test
    //

    REG_ENSURE(!rootKey.createKey(OUString::createFromAscii("/myFourthKey/X"), key7), "test_registry_CppApi error 7c)");;
    REG_ENSURE(!key6.createLink(OUString::createFromAscii("myFirstLink"), OUString::createFromAscii("/myFourthKey/X")), "test_registry_CppApi error 7d");
    REG_ENSURE(!key6.createKey(OUString::createFromAscii("mySixthSubKey"), key7), "test_registry_CppApi error 7e");

    OUString linkTarget;
    REG_ENSURE(!key6.getLinkTarget(OUString::createFromAscii("myFirstLink"), linkTarget), "test_registry_CppApi error 7f");
    REG_ENSURE(linkTarget.equals(OUString::createFromAscii("/myFourthKey/X")), "test_registry_CppApi error 7g");

    RegistryKeyNames* pSubKeyNames = new RegistryKeyNames();
    sal_uInt32           nSubKeys=0;

    REG_ENSURE(!rootKey.getKeyNames(OUString::createFromAscii("mySixthKey"), *pSubKeyNames), "test_registry_CppApi error 7h)");
    REG_ENSURE(pSubKeyNames->getLength() == 2, "test_registry_CppApi error 7i)");

    for (sal_uInt32 i=0; i < pSubKeyNames->getLength(); i++)
    {
        if (pSubKeyNames->getElement(i).equals(OUString::createFromAscii("/mySixthKey/myFirstLink")))
        {
            RegKeyType keyType;
            REG_ENSURE(!rootKey.getKeyType(pSubKeyNames->getElement(i), &keyType), "test_registry_CppApi error 7j");
            REG_ENSURE(keyType == RG_LINKTYPE, "test_registry_CppApi error 7k");
        }
    }

    REG_ENSURE(!key7.closeKey(), "test_registry_CppApi error 7k1");
    delete pSubKeyNames;

    REG_ENSURE(!rootKey.openKey(OUString::createFromAscii("/mySixthKey/myFirstLink"), key6), "test_registry_CppApi error 7l");
//    REG_ENSURE(key6.getName().equals(OUString::createFromAscii("/myFourthKey/X")), "test_registry_CppApi error 7m");

    REG_ENSURE(!rootKey.openKey(OUString::createFromAscii("myFifthKey"), key6), "test_registry_CppApi error 7m1");
    REG_ENSURE(!key6.createLink(OUString::createFromAscii("mySecondLink"),
                                 OUString::createFromAscii("/mySixthKey/myFirstLink")), "test_registry_CppApi error 7m2");

    REG_ENSURE(!rootKey.openKey(OUString::createFromAscii("/myFifthKey/mySecondLink"), key6), "test_registry_CppApi error 7m3");
//    REG_ENSURE(key6.getName().equals(OUString::createFromAscii("/myFourthKey/X")), "test_registry_CppApi error 7m4");

    REG_ENSURE(!rootKey.createKey(OUString::createFromAscii("/myFifthKey/mySecondLink/myFirstLinkSubKey"), key7), "test_registry_CppApi error 7m5");
    REG_ENSURE(key7.getName().equals(OUString::createFromAscii("/myFourthKey/X/myFirstLinkSubKey")), "test_registry_CppApi error 7m6");

    REG_ENSURE(!key7.createLink(OUString::createFromAscii("myThirdLink"), OUString::createFromAscii("/myFifthKey/mySecondLink")), "test_registry_CppApi error 7m7");
    REG_ENSURE(!rootKey.openKey(OUString::createFromAscii("/myFourthKey/X/myFirstLinkSubKey/myThirdLink"), key7), "test_registry_CppApi error 7m8");
//    REG_ENSURE(!key7.openKey(OUString::createFromAscii("/myFirstLinkSubKey/myThirdLink/myFirstLinkSubKey/myThirdLink"), key6), "test_registry_CppApi error 7m9");
//    REG_ENSURE(key7.getName().equals(OUString::createFromAscii("/myFourthKey/X")), "test_registry_CppApi error 7m10");
    REG_ENSURE(!key7.closeKey(), "test_registry_CppApi error 7m11");

    REG_ENSURE(!rootKey.deleteLink(OUString::createFromAscii("/myFifthKey/mySecondLink")), "test_registry_CppApi error 7m12");

    REG_ENSURE(!rootKey.createLink(OUString::createFromAscii("/myFifthKey/mySecondLink"),
                                    OUString::createFromAscii("/myFourthKey/X/myFirstLinkSubKey/myThirdLink")),
                                    "test_registry_CppApi error 7m13");

//    REG_ENSURE(rootKey.openKey(OUString::createFromAscii("/myFourthKey/X/myFirstLinkSubKey/myThirdLink"), key7) == REG_DETECT_RECURSION,
//                "test_registry_CppApi error 7m14");

//    REG_ENSURE(key7.closeKey() == REG_INVALID_KEY, "test_registry_CppApi error 7m11");

    RegistryKeyNames subKeyNames;
    nSubKeys=0;

    REG_ENSURE(!rootKey.getKeyNames(OUString::createFromAscii("mySixthKey"), subKeyNames), "test_registry_CppApi error 7n");

    nSubKeys = subKeyNames.getLength();
    REG_ENSURE(nSubKeys == 2, "test_registry_CppApi error 7n1");
    REG_ENSURE(subKeyNames.getElement(0).equals(OUString::createFromAscii("/mySixthKey/myFirstLink")), "test_registry_CppApi error 7p1)");
    REG_ENSURE(subKeyNames.getElement(1).equals(OUString::createFromAscii("/mySixthKey/mySixthSubKey")), "test_registry_CppApi error 7p2");


    RegistryKeyArray subKeys;
    nSubKeys=0;

    REG_ENSURE(!rootKey.openSubKeys(OUString::createFromAscii("myFirstKey"), subKeys), "test_registry_CppApi error 7o");

    nSubKeys = subKeys.getLength();
    REG_ENSURE(nSubKeys == 2, "test_registry_CppApi error 7o1");
    REG_ENSURE(subKeys.getElement(0).getName().equals(OUString::createFromAscii("/myFirstKey/mySecondSubKey")), "test_registry_CppApi error 7p1)");
    REG_ENSURE(subKeys.getElement(1).getName().equals(OUString::createFromAscii("/myFirstKey/X")), "test_registry_CppApi error 7p2");

    REG_ENSURE(!rootKey.closeSubKeys(subKeys), "test_registry_CppApi error 7q)");


    REG_ENSURE(!rootKey.createKey(OUString::createFromAscii("/TEST"), key8), "test_registry_CppApi error 8");
    REG_ENSURE(!rootKey.createKey(OUString::createFromAscii("/TEST/Child1"), key8), "test_registry_CppApi error 8a");
    REG_ENSURE(!rootKey.createKey(OUString::createFromAscii("/TEST/Child2"), key8), "test_registry_CppApi error 8a1");
    REG_ENSURE(!rootKey.openKey(OUString::createFromAscii("/TEST"), key9), "test_registry_CppApi error 8b");
    REG_ENSURE(!key8.closeKey() && !key9.closeKey(),  "test_registry_CppApi error 8b1");
    REG_ENSURE(!rootKey.openKey(OUString::createFromAscii("/TEST"), key8), "test_registry_CppApi error 8b");
    REG_ENSURE(!key8.closeKey(),  "test_registry_CppApi error 8c");
    REG_ENSURE(!rootKey.openKey(OUString::createFromAscii("TEST"), key8), "test_registry_CppApi error 8c");
    REG_ENSURE(!key8.closeKey(),  "test_registry_CppApi error 8d");


    sal_Char* Value=(sal_Char*)"Mein erster Value";
    REG_ENSURE(!rootKey.setValue(OUString::createFromAscii("mySecondKey"), RG_VALUETYPE_STRING, Value, 18), "test_registry_CppApi error 9");

    RegValueType    valueType;
    sal_uInt32          valueSize;
    sal_Char*           readValue;
    REG_ENSURE(!rootKey.getValueInfo(OUString::createFromAscii("mySecondKey"), &valueType, &valueSize), "test_registry_CppApi error 9a");

    readValue = (sal_Char*)rtl_allocateMemory(valueSize);
    REG_ENSURE(!key2.getValue(OUString(), readValue), "test_registry_CppApi error 10");

    REG_ENSURE(valueType == RG_VALUETYPE_STRING, "test_registry_CppApi error 11");
    REG_ENSURE(valueSize == 18, "test_registry_CppApi error 12");
    REG_ENSURE(strcmp(readValue, Value) == 0, "test_registry_CppApi error 13");
    rtl_freeMemory(readValue);

    const sal_Char* pList[3];
    const sal_Char* n1= "Hallo";
    const sal_Char* n2= "jetzt komm";
    const sal_Char* n3= "ich";

    pList[0]=n1;
    pList[1]=n2;
    pList[2]=n3;

    REG_ENSURE(!rootKey.setStringListValue(OUString::createFromAscii("myFourthKey"), (sal_Char**)pList, 3), "test_registry_CppApi error 13a");

    RegistryValueList<sal_Char*> valueList;
    REG_ENSURE(!rootKey.getStringListValue(OUString::createFromAscii("myFourthKey"), valueList), "test_registry_CppApi error 13b");

    REG_ENSURE(strcmp(n1, valueList.getElement(0)) == 0, "test_registry_CppApi error 13c");
    REG_ENSURE(strcmp(n2, valueList.getElement(1)) == 0, "test_registry_CppApi error 13d");
    REG_ENSURE(strcmp(n3, valueList.getElement(2)) == 0, "test_registry_CppApi error 13e");

    REG_ENSURE(!rootKey.getValueInfo(OUString::createFromAscii("myFourthKey"), &valueType, &valueSize), "test_registry_CppApi error 13e1");
    REG_ENSURE(valueType == RG_VALUETYPE_STRINGLIST, "test_registry_CppApi error 13e2");
    REG_ENSURE(valueSize == 3, "test_registry_CppApi error 13e3");

    sal_Int32 pLong[3];
    pLong[0] = 123;
    pLong[1] = 456;
    pLong[2] = 789;

    REG_ENSURE(!rootKey.setLongListValue(OUString::createFromAscii("myFifthKey"), pLong, 3), "test_registry_CppApi error 13f");

    RegistryValueList<sal_Int32> longList;
    REG_ENSURE(!rootKey.getLongListValue(OUString::createFromAscii("myFifthKey"), longList), "test_registry_CppApi error 13g");

    REG_ENSURE(pLong[0] == longList.getElement(0), "test_registry_CppApi error 13h");
    REG_ENSURE(pLong[1] == longList.getElement(1), "test_registry_CppApi error 13i");
    REG_ENSURE(pLong[2] == longList.getElement(2), "test_registry_CppApi error 13j");


    OUString sWTestValue(OUString::createFromAscii( "Mein erster Unicode Value" ));
    const sal_Unicode* wTestValue= sWTestValue.getStr();
    REG_ENSURE(!rootKey.setValue(OUString::createFromAscii("mySixthKey"), RG_VALUETYPE_UNICODE, (void*)wTestValue,
                (rtl_ustr_getLength(wTestValue)+1)*sizeof(sal_Unicode)), "test_registry_CppApi error 13j1");

    REG_ENSURE(!rootKey.getValueInfo(OUString::createFromAscii("mySixthKey"), &valueType, &valueSize), "test_registry_CppApi error 13j2");
    sal_Unicode* pTmpValue = (sal_Unicode*)rtl_allocateMemory(valueSize);
    REG_ENSURE(!rootKey.getValue(OUString::createFromAscii("mySixthKey"), pTmpValue), "test_registry_CppApi error 13j3");
    REG_ENSURE(rtl_ustr_getLength(wTestValue) == rtl_ustr_getLength(pTmpValue), "test_registry_CppApi error 13j4");
    REG_ENSURE(rtl_ustr_compare(wTestValue, pTmpValue) == 0, "test_registry_CppApi error 13j4");

    const sal_Unicode* pUnicode[3];
    OUString w1(OUString::createFromAscii( "Hallo" ));
    OUString w2(OUString::createFromAscii( "jetzt komm" ));
    OUString w3(OUString::createFromAscii( "ich als unicode" ));

    pUnicode[0]=w1.getStr();
    pUnicode[1]=w2.getStr();
    pUnicode[2]=w3.getStr();

    REG_ENSURE(!rootKey.setUnicodeListValue(OUString::createFromAscii("mySixthKey"), (sal_Unicode**)pUnicode, 3), "test_registry_CppApi error 13k");

    RegistryValueList<sal_Unicode*> unicodeList;
    REG_ENSURE(!rootKey.getUnicodeListValue(OUString::createFromAscii("mySixthKey"), unicodeList), "test_registry_CppApi error 13l");

    REG_ENSURE(rtl_ustr_compare(w1, unicodeList.getElement(0)) == 0, "test_registry_CppApi error 13m");
    REG_ENSURE(rtl_ustr_compare(w2, unicodeList.getElement(1)) == 0, "test_registry_CppApi error 13n");
    REG_ENSURE(rtl_ustr_compare(w3, unicodeList.getElement(2)) == 0, "test_registry_CppApi error 13o");

    REG_ENSURE(!key6.closeKey(),  "test_registry_CppApi error 14");

    REG_ENSURE(!key1.closeKey() &&
               !key3.closeKey() &&
               !key4.closeKey(),  "test_registry_CppApi error 14");

    REG_ENSURE(!rootKey.deleteKey(OUString::createFromAscii("myFirstKey")), "test_registry_CppApi error 15");

    REG_ENSURE(!key2.closeKey(), "test_registry_CppApi error 16");
    REG_ENSURE(!rootKey.openKey(OUString::createFromAscii("mySecondKey"), key2), "test_registry_CppApi error 17");

    REG_ENSURE(!key5.closeKey(), "test_registry_CppApi error 18");

    REG_ENSURE(!rootKey.deleteKey(OUString::createFromAscii("myThirdKey")), "test_registry_CppApi error 19");

    REG_ENSURE(rootKey.openKey(OUString::createFromAscii("myThirdKey"), key5), "test_registry_CppApi error 20");

    REG_ENSURE(!key2.closeKey() &&
                !rootKey.closeKey(),  "test_registry_CppApi error 21");

    REG_ENSURE(!myRegistry->close(), "test_registry_CppApi error 22");

    // Test loadkey
    RegistryKey rootKey2, key21, key22, key23, key24 , key25;

    REG_ENSURE(!myRegistry->create(OUString::createFromAscii("test2.rdb")), "test_registry_CppApi error 23");
    REG_ENSURE(!myRegistry->openRootKey(rootKey2), "test_registry_CppApi error 24");

    REG_ENSURE(!rootKey2.createKey(OUString::createFromAscii("reg2FirstKey"), key21), "test_registry_CppApi error 25");
    REG_ENSURE(!rootKey2.createKey(OUString::createFromAscii("reg2SecondKey"), key22), "test_registry_CppApi error 26");
    REG_ENSURE(!key21.createKey(OUString::createFromAscii("reg2FirstSubKey"), key23), "test_registry_CppApi error 27");
    REG_ENSURE(!key21.createKey(OUString::createFromAscii("reg2SecondSubKey"), key24), "test_registry_CppApi error 28");
    REG_ENSURE(!rootKey2.createKey(OUString::createFromAscii("reg2ThirdKey"), key25), "test_registry_CppApi error 29");

    sal_uInt32 nValue= 123456789;
    REG_ENSURE(!key23.setValue(OUString(), RG_VALUETYPE_LONG, &nValue, sizeof(sal_uInt32)), "test_registry_CppApi error 30");

    REG_ENSURE(!key21.closeKey() &&
               !key22.closeKey() &&
               !key23.closeKey() &&
               !key24.closeKey() &&
               !key25.closeKey() &&
               !rootKey2.closeKey(), "test_registry_CppApi error 31");

    REG_ENSURE(!myRegistry->close(), "test_registry_CppApi error 32");

    REG_ENSURE(!myRegistry->open(OUString::createFromAscii("test.rdb"), REG_READWRITE), "test_registry_CppApi error 33");
    REG_ENSURE(!myRegistry->openRootKey(rootKey), "test_registry_CppApi error 34");

    REG_ENSURE(!myRegistry->loadKey(rootKey, OUString::createFromAscii("allFromTest2"),
                    OUString::createFromAscii("test2.rdb")), "test_registry_CppApi error 35");
    REG_ENSURE(!myRegistry->saveKey(rootKey, OUString::createFromAscii("allFromTest2"),
                    OUString::createFromAscii("test3.rdb")), "test_registry_CppApi error 36");

    REG_ENSURE(!rootKey.createKey(OUString::createFromAscii("allFromTest3"), key1), "test_registry_CppApi error 37");
    REG_ENSURE(!key1.createKey(OUString::createFromAscii("myFirstKey2"), key2), "test_registry_CppApi error 38");
    REG_ENSURE(!key1.createKey(OUString::createFromAscii("mySecondKey2"), key3), "test_registry_CppApi error 39");

    REG_ENSURE(!myRegistry->mergeKey(rootKey, OUString::createFromAscii("allFromTest3"),
                    OUString::createFromAscii("test3.rdb")), "test_registry_CppApi error 40");
    REG_ENSURE(!myRegistry->mergeKey(rootKey, OUString::createFromAscii("allFromTest3"),
                    OUString::createFromAscii("ucrtest.rdb"), sal_True), "test_registry_CppApi error 40.a)");

//     REG_ENSURE(myRegistry->mergeKey(rootKey, OUString::createFromAscii("allFromTest3"), OUString::createFromAscii("ucrtest.rdb"), sal_True)
//                  == REG_NO_ERROR/*REG_MERGE_CONFLICT*/, "test_registry_CppApi error 40.b)");

    REG_ENSURE(!key1.closeKey() &&
                !key2.closeKey(), "test_registry_CppApi error 41");

    const sal_Unicode* wValue= OUString::createFromAscii( "Mein erster Unicode Value" ).getStr();
    REG_ENSURE(!key3.setValue(OUString(), RG_VALUETYPE_UNICODE, (void*)wValue,
                (rtl_ustr_getLength(wValue)+1)*sizeof(sal_Unicode)), "test_registry_CppApi error 42");

    REG_ENSURE(!key3.closeKey(), "test_registry_CppApi error 43");

    REG_ENSURE(!rootKey.openKey(OUString::createFromAscii("/allFromTest3/reg2FirstKey/reg2FirstSubKey"), key1),
                "test_registry_CppApi error 43.a)");
    REG_ENSURE(!rootKey.deleteKey(OUString::createFromAscii("/allFromTest3/reg2FirstKey/reg2FirstSubKey")), "test_registry_CppApi error 44");
    REG_ENSURE(key1.getValueInfo(OUString(), &valueType, &valueSize) == REG_INVALID_KEY,
                "test_registry_CppApi error 44.a)");
    REG_ENSURE(!key1.closeKey(), "test_registry_CppApi error 44.b)");

    REG_ENSURE(!rootKey.closeKey(), "test_registry_CppApi error 45");

    REG_ENSURE(!myRegistry->close(), "test_registry_CppApi error 46");

    REG_ENSURE(!myRegistry->open(OUString::createFromAscii("test.rdb"), REG_READWRITE), "test_registry_CppApi error 47");

    REG_ENSURE(!myRegistry->destroy(OUString::createFromAscii("test2.rdb")), "test_registry_CppApi error 48");
//  REG_ENSURE(!myRegistry->destroy("test3.rdb"), "test_registry_CppApi error 49");

    Registry *myRegistry2 = new Registry(*myRegistry);

    REG_ENSURE(myRegistry->destroy(OUString()), "test_registry_CppApi error 50");

    delete(myRegistry2);

    REG_ENSURE(!myRegistry->create(OUString::createFromAscii("destroytest.rdb")), "test_registry_CppApi error 51");
    REG_ENSURE(!myRegistry->close(), "test_registry_CppApi error 52");
    REG_ENSURE(!myRegistry->open(OUString::createFromAscii("destroytest.rdb"), REG_READONLY), "test_registry_CppApi error 53");
    REG_ENSURE(!myRegistry->openRootKey(rootKey), "test_registry_CppApi error 54");

    REG_ENSURE(myRegistry->mergeKey(rootKey, OUString::createFromAscii("allFromTest3"),
                    OUString::createFromAscii("test3.rdb")), "test_registry_CppApi error 55");
    REG_ENSURE(!myRegistry->destroy(OUString::createFromAscii("test3.rdb")), "test_registry_CppApi error 56");

    REG_ENSURE(!rootKey.closeKey(), "test_registry_CppApi error 57");
    REG_ENSURE(!myRegistry->close(), "test_registry_CppApi error 58");
    REG_ENSURE(!myRegistry->open(OUString::createFromAscii("destroytest.rdb"), REG_READWRITE), "test_registry_CppApi error 59");
    REG_ENSURE(!myRegistry->destroy(OUString()), "test_registry_CppApi error 60");

    REG_ENSURE(!myRegistry->open(OUString::createFromAscii("test.rdb"), REG_READWRITE), "test_registry_CppApi error 61");
    REG_ENSURE(!myRegistry->destroy(OUString::createFromAscii("ucrtest.rdb")), "test_registry_CppApi error 62");
    REG_ENSURE(!myRegistry->destroy(OUString()), "test_registry_CppApi error 63");
    delete(myRegistry);

    cout << "test_registry_CppApi() Ok!\n";

    return;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
