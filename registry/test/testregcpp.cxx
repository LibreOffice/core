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

void test_registry_CppApi()
{
    Registry *myRegistry = new Registry();

    RegistryKey rootKey, key1, key2, key3, key4 ,key5, key6, key7, key8, key9;

    REG_ENSURE(!myRegistry->create(OUString("test.rdb")), "test_registry_CppApi error 1");
    REG_ENSURE(!myRegistry->openRootKey(rootKey), "test_registry_CppApi error 2");

    REG_ENSURE(!rootKey.createKey(OUString("myFirstKey"), key1), "test_registry_CppApi error 3");
    REG_ENSURE(!rootKey.createKey(OUString("mySecondKey"), key2), "test_registry_CppApi error 4");
    REG_ENSURE(!key1.createKey(OUString("X"), key3), "test_registry_CppApi error 5");
    REG_ENSURE(!key1.createKey(OUString("mySecondSubKey"), key4), "test_registry_CppApi error 6");
    REG_ENSURE(!rootKey.createKey(OUString("myThirdKey"), key5), "test_registry_CppApi error 6a");

    REG_ENSURE(!key5.createKey(OUString("1"), key4), "test_registry_CppApi error 6b");
    REG_ENSURE(!key4.createKey(OUString("2"), key3), "test_registry_CppApi error 6c");
    REG_ENSURE(!key5.openKey("1", key4), "test_registry_CppApi error 6d");
    REG_ENSURE(!rootKey.openKey("/myThirdKey/1", key4), "test_registry_CppApi error 6e");
    REG_ENSURE(key4.getName() == "/myThirdKey/1", "test_registry_CppApi error 6f");

    REG_ENSURE(!rootKey.createKey(OUString("myFourthKey"), key6), "test_registry_CppApi error 7");
    REG_ENSURE(!rootKey.createKey(OUString("myFifthKey"), key6), "test_registry_CppApi error 7a");
    REG_ENSURE(!rootKey.createKey(OUString("mySixthKey"), key6), "test_registry_CppApi error 7b");

    // Link Test


    REG_ENSURE(!rootKey.createKey(OUString("/myFourthKey/X"), key7), "test_registry_CppApi error 7c)");
    REG_ENSURE(!key6.createLink(OUString("myFirstLink"), OUString("/myFourthKey/X")), "test_registry_CppApi error 7d");
    REG_ENSURE(!key6.createKey(OUString("mySixthSubKey"), key7), "test_registry_CppApi error 7e");

    OUString linkTarget;
    REG_ENSURE(!key6.getLinkTarget(OUString("myFirstLink"), linkTarget), "test_registry_CppApi error 7f");
    REG_ENSURE(linkTarget == "/myFourthKey/X", "test_registry_CppApi error 7g");

    RegistryKeyNames* pSubKeyNames = new RegistryKeyNames();
    sal_uInt32           nSubKeys=0;

    REG_ENSURE(!rootKey.getKeyNames(OUString("mySixthKey"), *pSubKeyNames), "test_registry_CppApi error 7h)");
    REG_ENSURE(pSubKeyNames->getLength() == 2, "test_registry_CppApi error 7i)");

    for (sal_uInt32 i=0; i < pSubKeyNames->getLength(); i++)
    {
        if ( pSubKeyNames->getElement(i) == "/mySixthKey/myFirstLink" )
        {
            RegKeyType keyType;
            REG_ENSURE(!rootKey.getKeyType(pSubKeyNames->getElement(i), &keyType), "test_registry_CppApi error 7j");
            REG_ENSURE(keyType == RG_LINKTYPE, "test_registry_CppApi error 7k");
        }
    }

    REG_ENSURE(!key7.closeKey(), "test_registry_CppApi error 7k1");
    delete pSubKeyNames;

    REG_ENSURE(!rootKey.openKey("/mySixthKey/myFirstLink", key6), "test_registry_CppApi error 7l");
//    REG_ENSURE(key6.getName() == "/myFourthKey/X", "test_registry_CppApi error 7m");

    REG_ENSURE(!rootKey.openKey("myFifthKey", key6), "test_registry_CppApi error 7m1");
    REG_ENSURE(!key6.createLink(OUString("mySecondLink"),
                                 OUString("/mySixthKey/myFirstLink")), "test_registry_CppApi error 7m2");

    REG_ENSURE(!rootKey.openKey("/myFifthKey/mySecondLink", key6), "test_registry_CppApi error 7m3");
//    REG_ENSURE(key6.getName() == "/myFourthKey/X", "test_registry_CppApi error 7m4");

    REG_ENSURE(!rootKey.createKey(OUString("/myFifthKey/mySecondLink/myFirstLinkSubKey"), key7), "test_registry_CppApi error 7m5");
    REG_ENSURE(key7.getName() == "/myFourthKey/X/myFirstLinkSubKey", "test_registry_CppApi error 7m6");

    REG_ENSURE(!key7.createLink(OUString("myThirdLink"), OUString("/myFifthKey/mySecondLink")), "test_registry_CppApi error 7m7");
    REG_ENSURE(!rootKey.openKey("/myFourthKey/X/myFirstLinkSubKey/myThirdLink", key7), "test_registry_CppApi error 7m8");
//    REG_ENSURE(!key7.openKey("/myFirstLinkSubKey/myThirdLink/myFirstLinkSubKey/myThirdLink", key6), "test_registry_CppApi error 7m9");
//    REG_ENSURE(key7.getName() == "/myFourthKey/X", "test_registry_CppApi error 7m10");
    REG_ENSURE(!key7.closeKey(), "test_registry_CppApi error 7m11");

    REG_ENSURE(!rootKey.deleteLink(OUString("/myFifthKey/mySecondLink")), "test_registry_CppApi error 7m12");

    REG_ENSURE(!rootKey.createLink(OUString("/myFifthKey/mySecondLink"),
                                    OUString("/myFourthKey/X/myFirstLinkSubKey/myThirdLink")),
                                    "test_registry_CppApi error 7m13");

//    REG_ENSURE(rootKey.openKey("/myFourthKey/X/myFirstLinkSubKey/myThirdLink", key7) == REG_DETECT_RECURSION,
//                "test_registry_CppApi error 7m14");

//    REG_ENSURE(key7.closeKey() == REG_INVALID_KEY, "test_registry_CppApi error 7m11");

    RegistryKeyNames subKeyNames;
    nSubKeys=0;

    REG_ENSURE(!rootKey.getKeyNames(OUString("mySixthKey"), subKeyNames), "test_registry_CppApi error 7n");

    nSubKeys = subKeyNames.getLength();
    REG_ENSURE(nSubKeys == 2, "test_registry_CppApi error 7n1");
    REG_ENSURE(subKeyNames.getElement(0) == "/mySixthKey/myFirstLink", "test_registry_CppApi error 7p1)");
    REG_ENSURE(subKeyNames.getElement(1) =="/mySixthKey/mySixthSubKey", "test_registry_CppApi error 7p2");


    RegistryKeyArray subKeys;
    nSubKeys=0;

    REG_ENSURE(!rootKey.openSubKeys(OUString("myFirstKey"), subKeys), "test_registry_CppApi error 7o");

    nSubKeys = subKeys.getLength();
    REG_ENSURE(nSubKeys == 2, "test_registry_CppApi error 7o1");
    REG_ENSURE(subKeys.getElement(0).getName() == "/myFirstKey/mySecondSubKey", "test_registry_CppApi error 7p1)");
    REG_ENSURE(subKeys.getElement(1).getName() == "/myFirstKey/X", "test_registry_CppApi error 7p2");

    REG_ENSURE(!rootKey.closeSubKeys(subKeys), "test_registry_CppApi error 7q)");


    REG_ENSURE(!rootKey.createKey(OUString("/TEST"), key8), "test_registry_CppApi error 8");
    REG_ENSURE(!rootKey.createKey(OUString("/TEST/Child1"), key8), "test_registry_CppApi error 8a");
    REG_ENSURE(!rootKey.createKey(OUString("/TEST/Child2"), key8), "test_registry_CppApi error 8a1");
    REG_ENSURE(!rootKey.openKey("/TEST", key9), "test_registry_CppApi error 8b");
    REG_ENSURE(!key8.closeKey() && !key9.closeKey(),  "test_registry_CppApi error 8b1");
    REG_ENSURE(!rootKey.openKey("/TEST", key8), "test_registry_CppApi error 8b");
    REG_ENSURE(!key8.closeKey(),  "test_registry_CppApi error 8c");
    REG_ENSURE(!rootKey.openKey("TEST", key8), "test_registry_CppApi error 8c");
    REG_ENSURE(!key8.closeKey(),  "test_registry_CppApi error 8d");


    char* Value=(char*)"My first value";
    REG_ENSURE(!rootKey.setValue(OUString("mySecondKey"), RegValueType::STRING, Value, 18), "test_registry_CppApi error 9");

    RegValueType    valueType;
    sal_uInt32          valueSize;
    char*           readValue;
    REG_ENSURE(!rootKey.getValueInfo(OUString("mySecondKey"), &valueType, &valueSize), "test_registry_CppApi error 9a");

    readValue = (char*)std::malloc(valueSize);
    REG_ENSURE(!key2.getValue(OUString(), readValue), "test_registry_CppApi error 10");

    REG_ENSURE(valueType == RegValueType::STRING, "test_registry_CppApi error 11");
    REG_ENSURE(valueSize == 18, "test_registry_CppApi error 12");
    REG_ENSURE(strcmp(readValue, Value) == 0, "test_registry_CppApi error 13");
    std::free(readValue);

    const char* pList[3];
    const char* n1= "Hello";
    const char* n2= "now I";
    const char* n3= "come";

    pList[0]=n1;
    pList[1]=n2;
    pList[2]=n3;

    REG_ENSURE(!rootKey.setStringListValue(OUString("myFourthKey"), (char**)pList, 3), "test_registry_CppApi error 13a");

    RegistryValueList<char*> valueList;
    REG_ENSURE(!rootKey.getStringListValue(OUString("myFourthKey"), valueList), "test_registry_CppApi error 13b");

    REG_ENSURE(strcmp(n1, valueList.getElement(0)) == 0, "test_registry_CppApi error 13c");
    REG_ENSURE(strcmp(n2, valueList.getElement(1)) == 0, "test_registry_CppApi error 13d");
    REG_ENSURE(strcmp(n3, valueList.getElement(2)) == 0, "test_registry_CppApi error 13e");

    REG_ENSURE(!rootKey.getValueInfo(OUString("myFourthKey"), &valueType, &valueSize), "test_registry_CppApi error 13e1");
    REG_ENSURE(valueType == RegValueType::STRINGLIST, "test_registry_CppApi error 13e2");
    REG_ENSURE(valueSize == 3, "test_registry_CppApi error 13e3");

    sal_Int32 pLong[3];
    pLong[0] = 123;
    pLong[1] = 456;
    pLong[2] = 789;

    REG_ENSURE(!rootKey.setLongListValue(OUString("myFifthKey"), pLong, 3), "test_registry_CppApi error 13f");

    RegistryValueList<sal_Int32> longList;
    REG_ENSURE(!rootKey.getLongListValue(OUString("myFifthKey"), longList), "test_registry_CppApi error 13g");

    REG_ENSURE(pLong[0] == longList.getElement(0), "test_registry_CppApi error 13h");
    REG_ENSURE(pLong[1] == longList.getElement(1), "test_registry_CppApi error 13i");
    REG_ENSURE(pLong[2] == longList.getElement(2), "test_registry_CppApi error 13j");


    OUString sWTestValue("My first unicode value");
    const sal_Unicode* wTestValue= sWTestValue.getStr();
    REG_ENSURE(!rootKey.setValue(OUString("mySixthKey"), RegValueType::UNICODE, (void*)wTestValue,
                (rtl_ustr_getLength(wTestValue)+1)*sizeof(sal_Unicode)), "test_registry_CppApi error 13j1");

    REG_ENSURE(!rootKey.getValueInfo(OUString("mySixthKey"), &valueType, &valueSize), "test_registry_CppApi error 13j2");
    sal_Unicode* pTmpValue = (sal_Unicode*)std::malloc(valueSize);
    REG_ENSURE(!rootKey.getValue(OUString("mySixthKey"), pTmpValue), "test_registry_CppApi error 13j3");
    REG_ENSURE(rtl_ustr_getLength(wTestValue) == rtl_ustr_getLength(pTmpValue), "test_registry_CppApi error 13j4");
    REG_ENSURE(rtl_ustr_compare(wTestValue, pTmpValue) == 0, "test_registry_CppApi error 13j4");

    const sal_Unicode* pUnicode[3];
    OUString w1("Hello");
    OUString w2("now I");
    OUString w3("come as unicode");

    pUnicode[0]=w1.getStr();
    pUnicode[1]=w2.getStr();
    pUnicode[2]=w3.getStr();

    REG_ENSURE(!rootKey.setUnicodeListValue(OUString("mySixthKey"), (sal_Unicode**)pUnicode, 3), "test_registry_CppApi error 13k");

    RegistryValueList<sal_Unicode*> unicodeList;
    REG_ENSURE(!rootKey.getUnicodeListValue(OUString("mySixthKey"), unicodeList), "test_registry_CppApi error 13l");

    REG_ENSURE(rtl_ustr_compare(w1, unicodeList.getElement(0)) == 0, "test_registry_CppApi error 13m");
    REG_ENSURE(rtl_ustr_compare(w2, unicodeList.getElement(1)) == 0, "test_registry_CppApi error 13n");
    REG_ENSURE(rtl_ustr_compare(w3, unicodeList.getElement(2)) == 0, "test_registry_CppApi error 13o");

    REG_ENSURE(!key6.closeKey(),  "test_registry_CppApi error 14");

    REG_ENSURE(!key1.closeKey() &&
               !key3.closeKey() &&
               !key4.closeKey(),  "test_registry_CppApi error 14");

    REG_ENSURE(!rootKey.deleteKey(OUString("myFirstKey")), "test_registry_CppApi error 15");

    REG_ENSURE(!key2.closeKey(), "test_registry_CppApi error 16");
    REG_ENSURE(!rootKey.openKey("mySecondKey", key2), "test_registry_CppApi error 17");

    REG_ENSURE(!key5.closeKey(), "test_registry_CppApi error 18");

    REG_ENSURE(!rootKey.deleteKey(OUString("myThirdKey")), "test_registry_CppApi error 19");

    REG_ENSURE(rootKey.openKey("myThirdKey", key5), "test_registry_CppApi error 20");

    REG_ENSURE(!key2.closeKey() &&
                !rootKey.closeKey(),  "test_registry_CppApi error 21");

    REG_ENSURE(!myRegistry->close(), "test_registry_CppApi error 22");

    // Test loadkey
    RegistryKey rootKey2, key21, key22, key23, key24 , key25;

    REG_ENSURE(!myRegistry->create(OUString("test2.rdb")), "test_registry_CppApi error 23");
    REG_ENSURE(!myRegistry->openRootKey(rootKey2), "test_registry_CppApi error 24");

    REG_ENSURE(!rootKey2.createKey(OUString("reg2FirstKey"), key21), "test_registry_CppApi error 25");
    REG_ENSURE(!rootKey2.createKey(OUString("reg2SecondKey"), key22), "test_registry_CppApi error 26");
    REG_ENSURE(!key21.createKey(OUString("reg2FirstSubKey"), key23), "test_registry_CppApi error 27");
    REG_ENSURE(!key21.createKey(OUString("reg2SecondSubKey"), key24), "test_registry_CppApi error 28");
    REG_ENSURE(!rootKey2.createKey(OUString("reg2ThirdKey"), key25), "test_registry_CppApi error 29");

    sal_uInt32 nValue= 123456789;
    REG_ENSURE(!key23.setValue(OUString(), RegValueType::LONG, &nValue, sizeof(sal_uInt32)), "test_registry_CppApi error 30");

    REG_ENSURE(!key21.closeKey() &&
               !key22.closeKey() &&
               !key23.closeKey() &&
               !key24.closeKey() &&
               !key25.closeKey() &&
               !rootKey2.closeKey(), "test_registry_CppApi error 31");

    REG_ENSURE(!myRegistry->close(), "test_registry_CppApi error 32");

    REG_ENSURE(!myRegistry->open(OUString("test.rdb"), RegAccessMode::READWRITE), "test_registry_CppApi error 33");
    REG_ENSURE(!myRegistry->openRootKey(rootKey), "test_registry_CppApi error 34");

    REG_ENSURE(!myRegistry->loadKey(rootKey, OUString("allFromTest2"),
                    OUString("test2.rdb")), "test_registry_CppApi error 35");
    REG_ENSURE(!myRegistry->saveKey(rootKey, OUString("allFromTest2"),
                    OUString("test3.rdb")), "test_registry_CppApi error 36");

    REG_ENSURE(!rootKey.createKey(OUString("allFromTest3"), key1), "test_registry_CppApi error 37");
    REG_ENSURE(!key1.createKey(OUString("myFirstKey2"), key2), "test_registry_CppApi error 38");
    REG_ENSURE(!key1.createKey(OUString("mySecondKey2"), key3), "test_registry_CppApi error 39");

    REG_ENSURE(!myRegistry->mergeKey(rootKey, OUString("allFromTest3"),
                    OUString("test3.rdb")), "test_registry_CppApi error 40");
    REG_ENSURE(!myRegistry->mergeKey(rootKey, OUString("allFromTest3"),
                    OUString("ucrtest.rdb"), sal_True), "test_registry_CppApi error 40.a)");

//     REG_ENSURE(myRegistry->mergeKey(rootKey, OUString("allFromTest3"), OUString("ucrtest.rdb"), sal_True)
//                  == REG_NO_ERROR/*REG_MERGE_CONFLICT*/, "test_registry_CppApi error 40.b)");

    REG_ENSURE(!key1.closeKey() &&
                !key2.closeKey(), "test_registry_CppApi error 41");

    const sal_Unicode* wValue= OUString("My first unicode value").getStr();
    REG_ENSURE(!key3.setValue(OUString(), RegValueType::UNICODE, (void*)wValue,
                (rtl_ustr_getLength(wValue)+1)*sizeof(sal_Unicode)), "test_registry_CppApi error 42");

    REG_ENSURE(!key3.closeKey(), "test_registry_CppApi error 43");

    REG_ENSURE(!rootKey.openKey("/allFromTest3/reg2FirstKey/reg2FirstSubKey", key1),
                "test_registry_CppApi error 43.a)");
    REG_ENSURE(!rootKey.deleteKey(OUString("/allFromTest3/reg2FirstKey/reg2FirstSubKey")), "test_registry_CppApi error 44");
    REG_ENSURE(key1.getValueInfo(OUString(), &valueType, &valueSize) == REG_INVALID_KEY,
                "test_registry_CppApi error 44.a)");
    REG_ENSURE(!key1.closeKey(), "test_registry_CppApi error 44.b)");

    REG_ENSURE(!rootKey.closeKey(), "test_registry_CppApi error 45");

    REG_ENSURE(!myRegistry->close(), "test_registry_CppApi error 46");

    REG_ENSURE(!myRegistry->open(OUString("test.rdb"), RegAccessMode::READWRITE), "test_registry_CppApi error 47");

    REG_ENSURE(!myRegistry->destroy(OUString("test2.rdb")), "test_registry_CppApi error 48");
//  REG_ENSURE(!myRegistry->destroy("test3.rdb"), "test_registry_CppApi error 49");

    Registry *myRegistry2 = new Registry(*myRegistry);

    REG_ENSURE(myRegistry->destroy(OUString()), "test_registry_CppApi error 50");

    delete(myRegistry2);

    REG_ENSURE(!myRegistry->create(OUString("destroytest.rdb")), "test_registry_CppApi error 51");
    REG_ENSURE(!myRegistry->close(), "test_registry_CppApi error 52");
    REG_ENSURE(!myRegistry->open(OUString("destroytest.rdb"), RegAccessMode::READONLY), "test_registry_CppApi error 53");
    REG_ENSURE(!myRegistry->openRootKey(rootKey), "test_registry_CppApi error 54");

    REG_ENSURE(myRegistry->mergeKey(rootKey, OUString("allFromTest3"),
                    OUString("test3.rdb")), "test_registry_CppApi error 55");
    REG_ENSURE(!myRegistry->destroy(OUString("test3.rdb")), "test_registry_CppApi error 56");

    REG_ENSURE(!rootKey.closeKey(), "test_registry_CppApi error 57");
    REG_ENSURE(!myRegistry->close(), "test_registry_CppApi error 58");
    REG_ENSURE(!myRegistry->open(OUString("destroytest.rdb"), RegAccessMode::READWRITE), "test_registry_CppApi error 59");
    REG_ENSURE(!myRegistry->destroy(OUString()), "test_registry_CppApi error 60");

    REG_ENSURE(!myRegistry->open(OUString("test.rdb"), RegAccessMode::READWRITE), "test_registry_CppApi error 61");
    REG_ENSURE(!myRegistry->destroy(OUString("ucrtest.rdb")), "test_registry_CppApi error 62");
    REG_ENSURE(!myRegistry->destroy(OUString()), "test_registry_CppApi error 63");
    delete(myRegistry);

    std::cout << "test_registry_CppApi() Ok!\n";

    return;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
