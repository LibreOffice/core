/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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


sal_Int32 lValue1   = 123456789;
sal_Int32 lValue2   = 54321;
sal_Int32 lValue3   = 111333111;
sal_Int32 lValue4   = 333111333;
sal_Char* sValue    = (sal_Char*)"string Value";
OUString wValue("unicode Value");


void test_generateMerge1()
{
    Registry *myRegistry = new Registry();

    RegistryKey rootKey, key1, key2, key3, key4, key5, key6, key7, key8, key9;

    REG_ENSURE(!myRegistry->create(OUString("merge1.rdb")), "testGenerateMerge1 error 1");
    REG_ENSURE(!myRegistry->openRootKey(rootKey), "testGenerateMerge1 error 2");

    REG_ENSURE(!rootKey.createKey(OUString("MergeKey1"), key1), "testGenerateMerge1 error 3");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1/MK1SubKey1"), key2), "testGenerateMerge1 error 4");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1/MK1SubKey1/KeyWithLongValue"), key3), "testGenerateMerge1 error 5");
    REG_ENSURE(!key3.setValue(OUString(), RG_VALUETYPE_LONG, &lValue1, sizeof(sal_Int32)), "testGenerateMerge1 error 5a");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1/MK1SubKey2"), key4), "testGenerateMerge1 error 6");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1/MK1SubKey2/KeyWithStringValue"), key5), "testGenerateMerge1 error 7");
    REG_ENSURE(!key5.setValue(OUString(), RG_VALUETYPE_STRING, sValue, strlen(sValue)+1), "testGenerateMerge1 error 7a");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1/MK1SubKey3"), key6), "testGenerateMerge1 error 8");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1/MK1SubKey3/KeyWithUnicodeValue"), key7), "testGenerateMerge1 error 9");
    REG_ENSURE(!key7.setValue(OUString(), RG_VALUETYPE_UNICODE, (void*)wValue.getStr(), ((wValue.getLength()+1)*sizeof(sal_Unicode))), "testGenerateMerge1 error 9a");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1/MK1SubKey4"), key8), "testGenerateMerge1 error 10");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1/MK1SubKey4/KeyWithBinaryValue"), key9), "testGenerateMerge1 error 11");
    REG_ENSURE(!key9.setValue(OUString(), RG_VALUETYPE_BINARY, (void*)"abcdefghijklmnopqrstuvwxyz", 27), "testGenerateMerge1 error 11a");


    REG_ENSURE(!key1.closeKey() &&
                !key2.closeKey() &&
                !key3.closeKey() &&
                !key4.closeKey() &&
                !key5.closeKey() &&
                !key6.closeKey() &&
                !key7.closeKey() &&
                !key8.closeKey() &&
                !key9.closeKey(), "testGenerateMerge1 error 12");

    REG_ENSURE(!rootKey.createKey(OUString("MergeKey1u2"), key1), "testGenerateMerge1 error 13");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1u2/MK1SubKey11"), key2), "testGenerateMerge1 error 14");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1u2/MK1SubKey11/KeyWithLongValue"), key3), "testGenerateMerge1 error 15");
    REG_ENSURE(!key3.setValue(OUString(), RG_VALUETYPE_LONG, &lValue2, sizeof(sal_Int32)), "testGenerateMerge1 error 15a");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1u2/MK1SubKey12"), key4), "testGenerateMerge1 error 16");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1u2/MK1SubKey12/KeyWithStringValue"), key5), "testGenerateMerge1 error 17");
    REG_ENSURE(!key5.setValue(OUString(), RG_VALUETYPE_STRING, sValue, strlen(sValue)+1), "testGenerateMerge1 error 17a");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1u2/MK1SubKey13"), key6), "testGenerateMerge1 error 18");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1u2/MK1SubKey13/KeyWithUnicodeValue"), key7), "testGenerateMerge1 error 19");
    REG_ENSURE(!key7.setValue(OUString(), RG_VALUETYPE_UNICODE, (void*)wValue.getStr(), ((wValue.getLength()+1)*sizeof(sal_Unicode))), "testGenerateMerge1 error 19a");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1u2/MK12SubKey1u2"), key8), "testGenerateMerge1 error 20");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1u2/MK12SubKey1u2/KeyWithLongValue"), key9), "testGenerateMerge1 error 21");
    REG_ENSURE(!key9.setValue(OUString(), RG_VALUETYPE_LONG, &lValue3, sizeof(sal_Int32)), "testGenerateMerge1 error 21a");

    REG_ENSURE(!rootKey.closeKey() &&
                !key1.closeKey() &&
                !key2.closeKey() &&
                !key3.closeKey() &&
                !key4.closeKey() &&
                !key5.closeKey() &&
                !key6.closeKey() &&
                !key7.closeKey() &&
                !key8.closeKey() &&
                !key9.closeKey(), "testGenerateMerge1 error 22");


    delete myRegistry;

    cout << "test_generateMerge1() Ok!\n";
    return;
}

void test_generateMerge2()
{
    Registry *myRegistry = new Registry();

    RegistryKey rootKey, key1, key2, key3, key4, key5, key6, key7, key8, key9;

    REG_ENSURE(!myRegistry->create(OUString("merge2.rdb")), "testGenerateMerge2 error 1");
    REG_ENSURE(!myRegistry->openRootKey(rootKey), "testGenerateMerge2 error 2");

    REG_ENSURE(!rootKey.createKey(OUString("MergeKey2"), key1), "testGenerateMerge2 error 3");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey2/MK2SubKey1"), key2), "testGenerateMerge2 error 4");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey2/MK2SubKey1/KeyWithBinaryValue"), key3), "testGenerateMerge2 error 5");
    REG_ENSURE(!key3.setValue(OUString(), RG_VALUETYPE_BINARY, (void*)"1234567890", 11), "testGenerateMerge1 error 5a");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey2/MK2SubKey2"), key4), "testGenerateMerge2 error 6");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey2/MK2SubKey2/KeyWithUnicodeValue"), key5), "testGenerateMerge2 error 7");
    REG_ENSURE(!key5.setValue(OUString(), RG_VALUETYPE_UNICODE, (void*)wValue.getStr(), ((wValue.getLength()+1)*sizeof(sal_Unicode))), "testGenerateMerge1 error 7a");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey2/MK2SubKey3"), key6), "testGenerateMerge2 error 8");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey2/MK2SubKey3/KeyWithStringValue"), key7), "testGenerateMerge2 error 9");
    REG_ENSURE(!key7.setValue(OUString(), RG_VALUETYPE_STRING, sValue, strlen(sValue)+1), "testGenerateMerge1 error 9a");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey2/MK2SubKey4"), key8), "testGenerateMerge2 error 10");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey2/MK2SubKey4/KeyWithLongValue"), key9), "testGenerateMerge2 error 11");
    REG_ENSURE(!key9.setValue(OUString(), RG_VALUETYPE_LONG, &lValue1, sizeof(sal_Int32)), "testGenerateMerge1 error 11a");

    REG_ENSURE(!key1.closeKey() &&
                !key2.closeKey() &&
                !key3.closeKey() &&
                !key4.closeKey() &&
                !key5.closeKey() &&
                !key6.closeKey() &&
                !key7.closeKey() &&
                !key8.closeKey() &&
                !key9.closeKey(), "testGenerateMerge2 error 12");

    REG_ENSURE(!rootKey.createKey(OUString("MergeKey1u2"), key1), "testGenerateMerge2 error 13");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1u2/MK2SubKey21"), key2), "testGenerateMerge2 error 14");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1u2/MK2SubKey21/KeyWithBinaryValue"), key3), "testGenerateMerge2 error 15");
    REG_ENSURE(!key3.setValue(OUString(), RG_VALUETYPE_BINARY, (void*)"a1b2c3d4e5f6g7h8i9", 19), "testGenerateMerge1 error 15a");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1u2/MK2SubKey22"), key4), "testGenerateMerge2 error 16");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1u2/MK2SubKey22/KeyWithLongValue"), key5), "testGenerateMerge2 error 17");
    REG_ENSURE(!key5.setValue(OUString(), RG_VALUETYPE_LONG, &lValue2, sizeof(sal_Int32)), "testGenerateMerge1 error 17a");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1u2/MK2SubKey23"), key6), "testGenerateMerge2 error 18");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1u2/MK2SubKey23/KeyWithStringValue"), key7), "testGenerateMerge2 error 19");
    REG_ENSURE(!key7.setValue(OUString(), RG_VALUETYPE_STRING, sValue, strlen(sValue)+1), "testGenerateMerge1 error 19a");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1u2/MK12SubKey1u2"), key8), "testGenerateMerge2 error 20");
    REG_ENSURE(!rootKey.createKey(OUString("/MergeKey1u2/MK12SubKey1u2/KeyWithLongValue"), key9), "testGenerateMerge2 error 21");
    REG_ENSURE(!key9.setValue(OUString(), RG_VALUETYPE_LONG, &lValue4, sizeof(sal_Int32)), "testGenerateMerge1 error 21a");

    REG_ENSURE(!rootKey.closeKey() &&
                !key1.closeKey() &&
                !key2.closeKey() &&
                !key3.closeKey() &&
                !key4.closeKey() &&
                !key5.closeKey() &&
                !key6.closeKey() &&
                !key7.closeKey() &&
                !key8.closeKey() &&
                !key9.closeKey(), "testGenerateMerge2 error 22");


    delete myRegistry;

    cout << "test_generateMerge2() Ok!\n";
    return;
}

void test_merge()
{
    Registry *myRegistry = new Registry();

    RegistryKey rootKey, key1, mkey1, key2, mkey2, key1u2, mkey1u2;

    REG_ENSURE(!myRegistry->create(OUString("mergetest.rdb")), "testMerge error 1");
    REG_ENSURE(myRegistry->getName() == "mergetest.rdb", "testMerge error 1.a)");
    REG_ENSURE(!myRegistry->openRootKey(rootKey), "testMerge error 2");
    REG_ENSURE(!myRegistry->loadKey(rootKey, OUString("/stardiv/IchbineinMergeKey"),
                OUString("merge1.rdb")), "testMerge error 3");
    REG_ENSURE(!myRegistry->mergeKey(rootKey, OUString("/stardiv/IchbineinMergeKey"),
                OUString("merge2.rdb")), "testMerge error 4");



    REG_ENSURE(!rootKey.openKey("/stardiv/IchbineinMergeKey", key1), "testMerge error 5");

    REG_ENSURE(!key1.openKey("MergeKey1", mkey1), "testMerge error 6");
    REG_ENSURE(!mkey1.closeKey(), "testMerge error 7");

    REG_ENSURE(!key1.openKey("/MergeKey1/MK1SubKey1", mkey1), "testMerge error 8");
    REG_ENSURE(!mkey1.closeKey(), "testMerge error 9");
    REG_ENSURE(!key1.openKey("/MergeKey1/MK1SubKey1/KeyWithLongValue", mkey1), "testMerge error 10");
    REG_ENSURE(!mkey1.closeKey(), "testMerge error 11");

    REG_ENSURE(!key1.openKey("/MergeKey1/MK1SubKey2", mkey1), "testMerge error 12");
    REG_ENSURE(!mkey1.closeKey(), "testMerge error 13");
    REG_ENSURE(!key1.openKey("/MergeKey1/MK1SubKey2/KeyWithStringValue", mkey1), "testMerge error 14");
    REG_ENSURE(!mkey1.closeKey(), "testMerge error 15");

    REG_ENSURE(!key1.openKey("/MergeKey1/MK1SubKey3", mkey1), "testMerge error 16");
    REG_ENSURE(!mkey1.closeKey(), "testMerge error 17");
    REG_ENSURE(!key1.openKey("/MergeKey1/MK1SubKey3/KeyWithUnicodeValue", mkey1), "testMerge error 18");
    REG_ENSURE(!mkey1.closeKey(), "testMerge error 19");

    REG_ENSURE(!key1.openKey("/MergeKey1/MK1SubKey4", mkey1), "testMerge error 20");
    REG_ENSURE(!mkey1.closeKey(), "testMerge error 21");
    REG_ENSURE(!key1.openKey("/MergeKey1/MK1SubKey4/KeyWithBinaryValue", mkey1), "testMerge error 22");
    REG_ENSURE(!mkey1.closeKey(), "testMerge error 23");

    REG_ENSURE(!key1.closeKey(), "testMerge error 24");



    REG_ENSURE(!rootKey.openKey("/stardiv/IchbineinMergeKey", key2), "testMerge error 25");

    REG_ENSURE(!key2.openKey("MergeKey2", mkey2), "testMerge error 26");
    REG_ENSURE(!mkey2.closeKey(), "testMerge error 27");

    REG_ENSURE(!key2.openKey("/MergeKey2/MK2SubKey1", mkey2), "testMerge error 28");
    REG_ENSURE(!mkey2.closeKey(), "testMerge error 29");
    REG_ENSURE(!key2.openKey("/MergeKey2/MK2SubKey1/KeyWithBinaryValue", mkey2), "testMerge error 30");
    REG_ENSURE(!mkey2.closeKey(), "testMerge error 31");

    REG_ENSURE(!key2.openKey("/MergeKey2/MK2SubKey2", mkey2), "testMerge error 31");
    REG_ENSURE(!mkey2.closeKey(), "testMerge error 33");
    REG_ENSURE(!key2.openKey("/MergeKey2/MK2SubKey2/KeyWithUnicodeValue", mkey2), "testMerge error 34");
    REG_ENSURE(!mkey2.closeKey(), "testMerge error 35");

    REG_ENSURE(!key2.openKey("/MergeKey2/MK2SubKey3", mkey2), "testMerge error 36");
    REG_ENSURE(!mkey2.closeKey(), "testMerge error 37");
    REG_ENSURE(!key2.openKey("/MergeKey2/MK2SubKey3/KeyWithStringValue", mkey2), "testMerge error 38");
    REG_ENSURE(!mkey2.closeKey(), "testMerge error 39");

    REG_ENSURE(!key2.openKey("/MergeKey2/MK2SubKey4", mkey2), "testMerge error 40");
    REG_ENSURE(!mkey2.closeKey(), "testMerge error 41");
    REG_ENSURE(!key2.openKey("/MergeKey2/MK2SubKey4/KeyWithLongValue", mkey2), "testMerge error 42");
    REG_ENSURE(!mkey2.closeKey(), "testMerge error 43");

    REG_ENSURE(!key2.closeKey(), "testMerge error 44");



    REG_ENSURE(!rootKey.openKey("/stardiv/IchbineinMergeKey", key1u2), "testMerge error 40");

    REG_ENSURE(!key1u2.openKey("MergeKey1u2", mkey1u2), "testMerge error 41");
    REG_ENSURE(!mkey1u2.closeKey(), "testMerge error 42");

    REG_ENSURE(!key1u2.openKey("/MergeKey1u2/MK1SubKey11", mkey1u2), "testMerge error 43");
    REG_ENSURE(!mkey1u2.closeKey(), "testMerge error 44");
    REG_ENSURE(!key1u2.openKey("/MergeKey1u2/MK1SubKey11/KeyWithLongValue", mkey1u2), "testMerge error 45");
    REG_ENSURE(!mkey1u2.closeKey(), "testMerge error 46");

    REG_ENSURE(!key1u2.openKey("/MergeKey1u2/MK1SubKey12", mkey1u2), "testMerge error 47");
    REG_ENSURE(!mkey1u2.closeKey(), "testMerge error 48");
    REG_ENSURE(!key1u2.openKey("/MergeKey1u2/MK1SubKey12/KeyWithStringValue", mkey1u2), "testMerge error 49");
    REG_ENSURE(!mkey1u2.closeKey(), "testMerge error 50");

    REG_ENSURE(!key1u2.openKey("/MergeKey1u2/MK1SubKey13", mkey1u2), "testMerge error 51");
    REG_ENSURE(!mkey1u2.closeKey(), "testMerge error 52");
    REG_ENSURE(!key1u2.openKey("/MergeKey1u2/MK1SubKey13/KeyWithUnicodeValue", mkey1u2), "testMerge error 53");
    REG_ENSURE(!mkey1u2.closeKey(), "testMerge error 54");

    REG_ENSURE(!key1u2.openKey("/MergeKey1u2/MK2SubKey21", mkey1u2), "testMerge error 55");
    REG_ENSURE(!mkey1u2.closeKey(), "testMerge error 56");
    REG_ENSURE(!key1u2.openKey("/MergeKey1u2/MK2SubKey21/KeyWithBinaryValue", mkey1u2), "testMerge error 57");
    REG_ENSURE(!mkey1u2.closeKey(), "testMerge error 58");

    REG_ENSURE(!key1u2.openKey("/MergeKey1u2/MK2SubKey22", mkey1u2), "testMerge error 59");
    REG_ENSURE(!mkey1u2.closeKey(), "testMerge error 60");
    REG_ENSURE(!key1u2.openKey("/MergeKey1u2/MK2SubKey22/KeyWithLongValue", mkey1u2), "testMerge error 61");
    REG_ENSURE(!mkey1u2.closeKey(), "testMerge error 62");

    REG_ENSURE(!key1u2.openKey("/MergeKey1u2/MK2SubKey23", mkey1u2), "testMerge error 63");
    REG_ENSURE(!mkey1u2.closeKey(), "testMerge error 64");
    REG_ENSURE(!key1u2.openKey("/MergeKey1u2/MK2SubKey23/KeyWithStringValue", mkey1u2), "testMerge error 65");
    REG_ENSURE(!mkey1u2.closeKey(), "testMerge error 66");

    REG_ENSURE(!key1u2.openKey("/MergeKey1u2/MK12SubKey1u2", mkey1u2), "testMerge error 67");
    REG_ENSURE(!mkey1u2.closeKey(), "testMerge error 68");
    REG_ENSURE(!key1u2.openKey("/MergeKey1u2/MK12SubKey1u2/KeyWithLongValue", mkey1u2), "testMerge error 69");
    REG_ENSURE(!mkey1u2.closeKey(), "testMerge error 70");

    REG_ENSURE(!key1u2.closeKey(), "testMerge error 71");



    RegValueType    valueType;
    sal_uInt32          valueSize;
    sal_Int32           int32Value;
    sal_uInt8           *Value;

    REG_ENSURE(!rootKey.openKey("/stardiv/IchbineinMergeKey", key1), "testMerge error 72");

    REG_ENSURE(!key1.getValueInfo(OUString("/MergeKey1/MK1SubKey1/KeyWithLongValue"), &valueType, &valueSize), "testMerge error 73");
    REG_ENSURE(valueType == RG_VALUETYPE_LONG && valueSize == sizeof(sal_Int32), "testMerge error 74");
    REG_ENSURE(!key1.getValue(OUString("/MergeKey1/MK1SubKey1/KeyWithLongValue"), (RegValue)&int32Value), "testMerge error 74.a)");
    REG_ENSURE(int32Value == lValue1, "testMerge error 74.b)");

    REG_ENSURE(!key1.getValueInfo(OUString("/MergeKey1/MK1SubKey2/KeyWithStringValue"), &valueType, &valueSize), "testMerge error 75");
    REG_ENSURE(valueType == RG_VALUETYPE_STRING && valueSize == strlen(sValue)+1, "testMerge error 76");
    Value = new sal_uInt8[valueSize];
    REG_ENSURE(!key1.getValue(OUString("/MergeKey1/MK1SubKey2/KeyWithStringValue"), (RegValue)Value), "testMerge error 76.a)");
    REG_ENSURE(strcmp((const sal_Char*)Value, sValue) == 0, "testMerge error 76.b)");
    delete [] Value;

    REG_ENSURE(!key1.getValueInfo(OUString("/MergeKey1/MK1SubKey3/KeyWithUnicodeValue"), &valueType, &valueSize), "testMerge error 77");
    REG_ENSURE(valueType == RG_VALUETYPE_UNICODE && valueSize == (wValue.getLength()+1)*sizeof(sal_Unicode), "testMerge error 78");
    Value = new sal_uInt8[valueSize];
    REG_ENSURE(!key1.getValue(OUString("/MergeKey1/MK1SubKey3/KeyWithUnicodeValue"), (RegValue)Value), "testMerge error 78.a)");
    REG_ENSURE(wValue.equals( (const sal_Unicode*)Value ), "testMerge error 78.b)");
    delete [] Value;

    REG_ENSURE(!key1.getValueInfo(OUString("/MergeKey1/MK1SubKey4/KeyWithBinaryValue"), &valueType, &valueSize), "testMerge error 79");
    REG_ENSURE(valueType == RG_VALUETYPE_BINARY && valueSize == 27, "testMerge error 80");
    Value = new sal_uInt8[valueSize];
    REG_ENSURE(!key1.getValue(OUString("/MergeKey1/MK1SubKey4/KeyWithBinaryValue"), (RegValue)Value), "testMerge error 80.a)");
    REG_ENSURE(strcmp((const sal_Char*)Value, "abcdefghijklmnopqrstuvwxyz") == 0, "testMerge error 80.b)");
    delete [] Value;

    REG_ENSURE(!key1.getValueInfo(OUString("/MergeKey2/MK2SubKey1/KeyWithBinaryValue"), &valueType, &valueSize), "testMerge error 81");
    REG_ENSURE(valueType == RG_VALUETYPE_BINARY && valueSize == 11, "testMerge error 82");
    Value = new sal_uInt8[valueSize];
    REG_ENSURE(!key1.getValue(OUString("/MergeKey2/MK2SubKey1/KeyWithBinaryValue"), (RegValue)Value), "testMerge error 82.a)");
    REG_ENSURE(strcmp((const sal_Char*)Value, "1234567890") == 0, "testMerge error 82.b)");
    delete [] Value;

    REG_ENSURE(!key1.getValueInfo(OUString("/MergeKey2/MK2SubKey2/KeyWithUnicodeValue"), &valueType, &valueSize), "testMerge error 83");
    REG_ENSURE(valueType == RG_VALUETYPE_UNICODE&& valueSize == (wValue.getLength()+1)*sizeof(sal_Unicode), "testMerge error 84");
    Value = new sal_uInt8[valueSize];
    REG_ENSURE(!key1.getValue(OUString("/MergeKey2/MK2SubKey2/KeyWithUnicodeValue"), (RegValue)Value), "testMerge error 84.a)");
    REG_ENSURE(wValue.equals( (const sal_Unicode*)Value ), "testMerge error 84.b)");
    delete [] Value;

    REG_ENSURE(!key1.getValueInfo(OUString("/MergeKey2/MK2SubKey3/KeyWithStringValue"), &valueType, &valueSize), "testMerge error 85");
    REG_ENSURE(valueType == RG_VALUETYPE_STRING && valueSize == strlen(sValue)+1, "testMerge error 86");
    Value = new sal_uInt8[valueSize];
    REG_ENSURE(!key1.getValue(OUString("/MergeKey2/MK2SubKey3/KeyWithStringValue"), (RegValue)Value), "testMerge error 86.a)");
    REG_ENSURE(strcmp((const sal_Char*)Value, sValue) == 0, "testMerge error 86.b)");
    delete [] Value;

    REG_ENSURE(!key1.getValueInfo(OUString("/MergeKey2/MK2SubKey4/KeyWithLongValue"), &valueType, &valueSize), "testMerge error 87");
    REG_ENSURE(valueType == RG_VALUETYPE_LONG && valueSize == sizeof(sal_Int32), "testMerge error 88");
    REG_ENSURE(!key1.getValue(OUString("/MergeKey2/MK2SubKey4/KeyWithLongValue"), (RegValue)&int32Value), "testMerge error 88.a)");
    REG_ENSURE(int32Value == lValue1, "testMerge error 88.b)");

    REG_ENSURE(!key1.getValueInfo(OUString("/MergeKey1u2/MK1SubKey11/KeyWithLongValue"), &valueType, &valueSize), "testMerge error 89");
    REG_ENSURE(valueType == RG_VALUETYPE_LONG && valueSize == sizeof(sal_Int32), "testMerge error 90");
    REG_ENSURE(!key1.getValue(OUString("/MergeKey1u2/MK1SubKey11/KeyWithLongValue"), (RegValue)&int32Value), "testMerge error 90.a)");
    REG_ENSURE(int32Value == lValue2, "testMerge error 90.b)");

    REG_ENSURE(!key1.getValueInfo(OUString("/MergeKey1u2/MK1SubKey12/KeyWithStringValue"), &valueType, &valueSize), "testMerge error 91");
    REG_ENSURE(valueType == RG_VALUETYPE_STRING && valueSize == strlen(sValue)+1, "testMerge error 92");
    Value = new sal_uInt8[valueSize];
    REG_ENSURE(!key1.getValue(OUString("/MergeKey1u2/MK1SubKey12/KeyWithStringValue"), (RegValue)Value), "testMerge error 92.a)");
    REG_ENSURE(strcmp((const sal_Char*)Value, sValue) == 0, "testMerge error 92.b)");
    delete [] Value;

    REG_ENSURE(!key1.getValueInfo(OUString("/MergeKey1u2/MK1SubKey13/KeyWithUnicodeValue"), &valueType, &valueSize), "testMerge error 93");
    REG_ENSURE(valueType == RG_VALUETYPE_UNICODE && valueSize == (wValue.getLength()+1)*sizeof(sal_Unicode), "testMerge error 94");
    Value = new sal_uInt8[valueSize];
    REG_ENSURE(!key1.getValue(OUString("/MergeKey1u2/MK1SubKey13/KeyWithUnicodeValue"), (RegValue)Value), "testMerge error 94.a)");
    REG_ENSURE(wValue.equals( (const sal_Unicode*)Value ), "testMerge error 94.b)");
    delete [] Value;

    REG_ENSURE(!key1.getValueInfo(OUString("/MergeKey1u2/MK2SubKey21/KeyWithBinaryValue"), &valueType, &valueSize), "testMerge error 95");
    REG_ENSURE(valueType == RG_VALUETYPE_BINARY && valueSize == 19, "testMerge error 96");
    Value = new sal_uInt8[valueSize];
    REG_ENSURE(!key1.getValue(OUString("/MergeKey1u2/MK2SubKey21/KeyWithBinaryValue"), (RegValue)Value), "testMerge error 96.a)");
    REG_ENSURE(strcmp((const sal_Char*)Value, "a1b2c3d4e5f6g7h8i9") == 0, "testMerge error 96.b)");
    delete [] Value;

    REG_ENSURE(!key1.getValueInfo(OUString("/MergeKey1u2/MK2SubKey22/KeyWithLongValue"), &valueType, &valueSize), "testMerge error 97");
    REG_ENSURE(valueType == RG_VALUETYPE_LONG && valueSize == sizeof(sal_Int32), "testMerge error 98");
    REG_ENSURE(!key1.getValue(OUString("/MergeKey1u2/MK2SubKey22/KeyWithLongValue"), (RegValue)&int32Value), "testMerge error 98.a)");
    REG_ENSURE(int32Value == lValue2, "testMerge error 98.b)");

    REG_ENSURE(!key1.getValueInfo(OUString("/MergeKey1u2/MK2SubKey23/KeyWithStringValue"), &valueType, &valueSize), "testMerge error 99");
    REG_ENSURE(valueType == RG_VALUETYPE_STRING && valueSize == strlen(sValue)+1, "testMerge error 100");
    Value = new sal_uInt8[valueSize];
    REG_ENSURE(!key1.getValue(OUString("/MergeKey1u2/MK2SubKey23/KeyWithStringValue"), (RegValue)Value), "testMerge error 100.a)");
    REG_ENSURE(strcmp((const sal_Char*)Value, sValue) == 0, "testMerge error 100.b)");
    delete [] Value;

    REG_ENSURE(!key1.getValueInfo(OUString("/MergeKey1u2/MK12SubKey1u2/KeyWithLongValue"), &valueType, &valueSize), "testMerge error 101");
    REG_ENSURE(valueType == RG_VALUETYPE_LONG && valueSize == sizeof(sal_Int32), "testMerge error 102");
    REG_ENSURE(!key1.getValue(OUString("/MergeKey1u2/MK12SubKey1u2/KeyWithLongValue"), (RegValue)&int32Value), "testMerge error 102.a)");
    REG_ENSURE(int32Value == lValue4, "testMerge error 102.b)");


    REG_ENSURE(!key1.closeKey(), "testMerge error 24");



    REG_ENSURE(!rootKey.closeKey(), "testMerge error 10");

    REG_ENSURE(!myRegistry->destroy( OUString("merge1.rdb") ), "test_registry_CppApi error 11");
    REG_ENSURE(!myRegistry->destroy( OUString("merge2.rdb") ), "test_registry_CppApi error 12");
    REG_ENSURE(!myRegistry->destroy( OUString() ), "test_registry_CppApi error 13");

    delete myRegistry;

    cout << "test_merge() Ok!\n";
    return;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
