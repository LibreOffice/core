/*************************************************************************
 *
 *  $RCSfile: testmerge.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-12 16:59:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <iostream.h>
#include <stdio.h>
#include <string.h>

#include "registry/registry.hxx"
#include "registry/reflread.hxx"
#include "registry/reflwrit.hxx"

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _RTL_ALLOC_H_
#include    <rtl/alloc.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include    <rtl/ustring.hxx>
#endif


using namespace rtl;

sal_Int32 lValue1   = 123456789;
sal_Int32 lValue2   = 54321;
sal_Int32 lValue3   = 111333111;
sal_Int32 lValue4   = 333111333;
sal_Char* sValue    = "string Value";
OUString wValue = rtl::OUString::createFromAscii( "unicode Value" );


void test_generateMerge1()
{
    RegistryLoader* pLoader = new RegistryLoader();

    if (!pLoader->isLoaded())
    {
        delete pLoader;
        return;
    }

    Registry *myRegistry = new Registry(*pLoader);
    delete pLoader;

    RegistryKey rootKey, key1, key2, key3, key4, key5, key6, key7, key8, key9;

    OSL_ENSURE(!myRegistry->create(OUString::createFromAscii("merge1.rdb")), "testGenerateMerge1 error 1");
    OSL_ENSURE(!myRegistry->openRootKey(rootKey), "testGenerateMerge1 error 2");

    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("MergeKey1"), key1), "testGenerateMerge1 error 3");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1/MK1SubKey1"), key2), "testGenerateMerge1 error 4");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1/MK1SubKey1/KeyWithLongValue"), key3), "testGenerateMerge1 error 5");
    OSL_ENSURE(!key3.setValue(OUString(), RG_VALUETYPE_LONG, &lValue1, sizeof(sal_Int32)), "testGenerateMerge1 error 5a");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1/MK1SubKey2"), key4), "testGenerateMerge1 error 6");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1/MK1SubKey2/KeyWithStringValue"), key5), "testGenerateMerge1 error 7");
    OSL_ENSURE(!key5.setValue(OUString(), RG_VALUETYPE_STRING, sValue, strlen(sValue)+1), "testGenerateMerge1 error 7a");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1/MK1SubKey3"), key6), "testGenerateMerge1 error 8");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1/MK1SubKey3/KeyWithUnicodeValue"), key7), "testGenerateMerge1 error 9");
    OSL_ENSURE(!key7.setValue(OUString(), RG_VALUETYPE_UNICODE, (void*)wValue.getStr(), ((wValue.getLength()+1)*sizeof(sal_Unicode))), "testGenerateMerge1 error 9a");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1/MK1SubKey4"), key8), "testGenerateMerge1 error 10");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1/MK1SubKey4/KeyWithBinaryValue"), key9), "testGenerateMerge1 error 11");
    OSL_ENSURE(!key9.setValue(OUString(), RG_VALUETYPE_BINARY, "abcdefghijklmnopqrstuvwxyz", 27), "testGenerateMerge1 error 11a");


    OSL_ENSURE(!key1.closeKey() &&
                !key2.closeKey() &&
                !key3.closeKey() &&
                !key4.closeKey() &&
                !key5.closeKey() &&
                !key6.closeKey() &&
                !key7.closeKey() &&
                !key8.closeKey() &&
                !key9.closeKey(), "testGenerateMerge1 error 12");

    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("MergeKey1u2"), key1), "testGenerateMerge1 error 13");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1u2/MK1SubKey11"), key2), "testGenerateMerge1 error 14");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1u2/MK1SubKey11/KeyWithLongValue"), key3), "testGenerateMerge1 error 15");
    OSL_ENSURE(!key3.setValue(OUString(), RG_VALUETYPE_LONG, &lValue2, sizeof(sal_Int32)), "testGenerateMerge1 error 15a");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1u2/MK1SubKey12"), key4), "testGenerateMerge1 error 16");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1u2/MK1SubKey12/KeyWithStringValue"), key5), "testGenerateMerge1 error 17");
    OSL_ENSURE(!key5.setValue(OUString(), RG_VALUETYPE_STRING, sValue, strlen(sValue)+1), "testGenerateMerge1 error 17a");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1u2/MK1SubKey13"), key6), "testGenerateMerge1 error 18");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1u2/MK1SubKey13/KeyWithUnicodeValue"), key7), "testGenerateMerge1 error 19");
    OSL_ENSURE(!key7.setValue(OUString(), RG_VALUETYPE_UNICODE, (void*)wValue.getStr(), ((wValue.getLength()+1)*sizeof(sal_Unicode))), "testGenerateMerge1 error 19a");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1u2/MK12SubKey1u2"), key8), "testGenerateMerge1 error 20");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1u2/MK12SubKey1u2/KeyWithLongValue"), key9), "testGenerateMerge1 error 21");
    OSL_ENSURE(!key9.setValue(OUString(), RG_VALUETYPE_LONG, &lValue3, sizeof(sal_Int32)), "testGenerateMerge1 error 21a");

    OSL_ENSURE(!rootKey.closeKey() &&
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
    RegistryLoader* pLoader = new RegistryLoader();

    if (!pLoader->isLoaded())
    {
        delete pLoader;
        return;
    }

    Registry *myRegistry = new Registry(*pLoader);
    delete pLoader;

    RegistryKey rootKey, key1, key2, key3, key4, key5, key6, key7, key8, key9;

    OSL_ENSURE(!myRegistry->create(OUString::createFromAscii("merge2.rdb")), "testGenerateMerge2 error 1");
    OSL_ENSURE(!myRegistry->openRootKey(rootKey), "testGenerateMerge2 error 2");

    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("MergeKey2"), key1), "testGenerateMerge2 error 3");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey2/MK2SubKey1"), key2), "testGenerateMerge2 error 4");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey2/MK2SubKey1/KeyWithBinaryValue"), key3), "testGenerateMerge2 error 5");
    OSL_ENSURE(!key3.setValue(OUString(), RG_VALUETYPE_BINARY, "1234567890", 11), "testGenerateMerge1 error 5a");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey2/MK2SubKey2"), key4), "testGenerateMerge2 error 6");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey2/MK2SubKey2/KeyWithUnicodeValue"), key5), "testGenerateMerge2 error 7");
    OSL_ENSURE(!key5.setValue(OUString(), RG_VALUETYPE_UNICODE, (void*)wValue.getStr(), ((wValue.getLength()+1)*sizeof(sal_Unicode))), "testGenerateMerge1 error 7a");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey2/MK2SubKey3"), key6), "testGenerateMerge2 error 8");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey2/MK2SubKey3/KeyWithStringValue"), key7), "testGenerateMerge2 error 9");
    OSL_ENSURE(!key7.setValue(OUString(), RG_VALUETYPE_STRING, sValue, strlen(sValue)+1), "testGenerateMerge1 error 9a");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey2/MK2SubKey4"), key8), "testGenerateMerge2 error 10");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey2/MK2SubKey4/KeyWithLongValue"), key9), "testGenerateMerge2 error 11");
    OSL_ENSURE(!key9.setValue(OUString(), RG_VALUETYPE_LONG, &lValue1, sizeof(sal_Int32)), "testGenerateMerge1 error 11a");

    OSL_ENSURE(!key1.closeKey() &&
                !key2.closeKey() &&
                !key3.closeKey() &&
                !key4.closeKey() &&
                !key5.closeKey() &&
                !key6.closeKey() &&
                !key7.closeKey() &&
                !key8.closeKey() &&
                !key9.closeKey(), "testGenerateMerge2 error 12");

    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("MergeKey1u2"), key1), "testGenerateMerge2 error 13");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1u2/MK2SubKey21"), key2), "testGenerateMerge2 error 14");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1u2/MK2SubKey21/KeyWithBinaryValue"), key3), "testGenerateMerge2 error 15");
    OSL_ENSURE(!key3.setValue(OUString(), RG_VALUETYPE_BINARY, "a1b2c3d4e5f6g7h8i9", 19), "testGenerateMerge1 error 15a");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1u2/MK2SubKey22"), key4), "testGenerateMerge2 error 16");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1u2/MK2SubKey22/KeyWithLongValue"), key5), "testGenerateMerge2 error 17");
    OSL_ENSURE(!key5.setValue(OUString(), RG_VALUETYPE_LONG, &lValue2, sizeof(sal_Int32)), "testGenerateMerge1 error 17a");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1u2/MK2SubKey23"), key6), "testGenerateMerge2 error 18");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1u2/MK2SubKey23/KeyWithStringValue"), key7), "testGenerateMerge2 error 19");
    OSL_ENSURE(!key7.setValue(OUString(), RG_VALUETYPE_STRING, sValue, strlen(sValue)+1), "testGenerateMerge1 error 19a");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1u2/MK12SubKey1u2"), key8), "testGenerateMerge2 error 20");
    OSL_ENSURE(!rootKey.createKey(OUString::createFromAscii("/MergeKey1u2/MK12SubKey1u2/KeyWithLongValue"), key9), "testGenerateMerge2 error 21");
    OSL_ENSURE(!key9.setValue(OUString(), RG_VALUETYPE_LONG, &lValue4, sizeof(sal_Int32)), "testGenerateMerge1 error 21a");

    OSL_ENSURE(!rootKey.closeKey() &&
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
    RegistryLoader* pLoader = new RegistryLoader();

    if (!pLoader->isLoaded())
    {
        delete pLoader;
        return;
    }

    Registry *myRegistry = new Registry(*pLoader);
    delete pLoader;

    RegistryKey rootKey, key1, mkey1, key2, mkey2, key1u2, mkey1u2;

    OSL_ENSURE(!myRegistry->create(OUString::createFromAscii("mergetest.rdb")), "testMerge error 1");
    OSL_ENSURE(myRegistry->getName().equals(OUString::createFromAscii("mergetest.rdb")), "testMerge error 1.a)");
    OSL_ENSURE(!myRegistry->openRootKey(rootKey), "testMerge error 2");
    OSL_ENSURE(!myRegistry->loadKey(rootKey, OUString::createFromAscii("/stardiv/IchbineinMergeKey"),
                OUString::createFromAscii("merge1.rdb")), "testMerge error 3");
    OSL_ENSURE(!myRegistry->mergeKey(rootKey, OUString::createFromAscii("/stardiv/IchbineinMergeKey"),
                OUString::createFromAscii("merge2.rdb")), "testMerge error 4");

    ///////////////////////////////////////////////////////////////////////////

    OSL_ENSURE(!rootKey.openKey(OUString::createFromAscii("/stardiv/IchbineinMergeKey"), key1), "testMerge error 5");

    OSL_ENSURE(!key1.openKey(OUString::createFromAscii("MergeKey1"), mkey1), "testMerge error 6");
    OSL_ENSURE(!mkey1.closeKey(), "testMerge error 7");

    OSL_ENSURE(!key1.openKey(OUString::createFromAscii("/MergeKey1/MK1SubKey1"), mkey1), "testMerge error 8");
    OSL_ENSURE(!mkey1.closeKey(), "testMerge error 9");
    OSL_ENSURE(!key1.openKey(OUString::createFromAscii("/MergeKey1/MK1SubKey1/KeyWithLongValue"), mkey1), "testMerge error 10");
    OSL_ENSURE(!mkey1.closeKey(), "testMerge error 11");

    OSL_ENSURE(!key1.openKey(OUString::createFromAscii("/MergeKey1/MK1SubKey2"), mkey1), "testMerge error 12");
    OSL_ENSURE(!mkey1.closeKey(), "testMerge error 13");
    OSL_ENSURE(!key1.openKey(OUString::createFromAscii("/MergeKey1/MK1SubKey2/KeyWithStringValue"), mkey1), "testMerge error 14");
    OSL_ENSURE(!mkey1.closeKey(), "testMerge error 15");

    OSL_ENSURE(!key1.openKey(OUString::createFromAscii("/MergeKey1/MK1SubKey3"), mkey1), "testMerge error 16");
    OSL_ENSURE(!mkey1.closeKey(), "testMerge error 17");
    OSL_ENSURE(!key1.openKey(OUString::createFromAscii("/MergeKey1/MK1SubKey3/KeyWithUnicodeValue"), mkey1), "testMerge error 18");
    OSL_ENSURE(!mkey1.closeKey(), "testMerge error 19");

    OSL_ENSURE(!key1.openKey(OUString::createFromAscii("/MergeKey1/MK1SubKey4"), mkey1), "testMerge error 20");
    OSL_ENSURE(!mkey1.closeKey(), "testMerge error 21");
    OSL_ENSURE(!key1.openKey(OUString::createFromAscii("/MergeKey1/MK1SubKey4/KeyWithBinaryValue"), mkey1), "testMerge error 22");
    OSL_ENSURE(!mkey1.closeKey(), "testMerge error 23");

    OSL_ENSURE(!key1.closeKey(), "testMerge error 24");

    ///////////////////////////////////////////////////////////////////////////

    OSL_ENSURE(!rootKey.openKey(OUString::createFromAscii("/stardiv/IchbineinMergeKey"), key2), "testMerge error 25");

    OSL_ENSURE(!key2.openKey(OUString::createFromAscii("MergeKey2"), mkey2), "testMerge error 26");
    OSL_ENSURE(!mkey2.closeKey(), "testMerge error 27");

    OSL_ENSURE(!key2.openKey(OUString::createFromAscii("/MergeKey2/MK2SubKey1"), mkey2), "testMerge error 28");
    OSL_ENSURE(!mkey2.closeKey(), "testMerge error 29");
    OSL_ENSURE(!key2.openKey(OUString::createFromAscii("/MergeKey2/MK2SubKey1/KeyWithBinaryValue"), mkey2), "testMerge error 30");
    OSL_ENSURE(!mkey2.closeKey(), "testMerge error 31");

    OSL_ENSURE(!key2.openKey(OUString::createFromAscii("/MergeKey2/MK2SubKey2"), mkey2), "testMerge error 31");
    OSL_ENSURE(!mkey2.closeKey(), "testMerge error 33");
    OSL_ENSURE(!key2.openKey(OUString::createFromAscii("/MergeKey2/MK2SubKey2/KeyWithUnicodeValue"), mkey2), "testMerge error 34");
    OSL_ENSURE(!mkey2.closeKey(), "testMerge error 35");

    OSL_ENSURE(!key2.openKey(OUString::createFromAscii("/MergeKey2/MK2SubKey3"), mkey2), "testMerge error 36");
    OSL_ENSURE(!mkey2.closeKey(), "testMerge error 37");
    OSL_ENSURE(!key2.openKey(OUString::createFromAscii("/MergeKey2/MK2SubKey3/KeyWithStringValue"), mkey2), "testMerge error 38");
    OSL_ENSURE(!mkey2.closeKey(), "testMerge error 39");

    OSL_ENSURE(!key2.openKey(OUString::createFromAscii("/MergeKey2/MK2SubKey4"), mkey2), "testMerge error 40");
    OSL_ENSURE(!mkey2.closeKey(), "testMerge error 41");
    OSL_ENSURE(!key2.openKey(OUString::createFromAscii("/MergeKey2/MK2SubKey4/KeyWithLongValue"), mkey2), "testMerge error 42");
    OSL_ENSURE(!mkey2.closeKey(), "testMerge error 43");

    OSL_ENSURE(!key2.closeKey(), "testMerge error 44");

    ///////////////////////////////////////////////////////////////////////////

    OSL_ENSURE(!rootKey.openKey(OUString::createFromAscii("/stardiv/IchbineinMergeKey"), key1u2), "testMerge error 40");

    OSL_ENSURE(!key1u2.openKey(OUString::createFromAscii("MergeKey1u2"), mkey1u2), "testMerge error 41");
    OSL_ENSURE(!mkey1u2.closeKey(), "testMerge error 42");

    OSL_ENSURE(!key1u2.openKey(OUString::createFromAscii("/MergeKey1u2/MK1SubKey11"), mkey1u2), "testMerge error 43");
    OSL_ENSURE(!mkey1u2.closeKey(), "testMerge error 44");
    OSL_ENSURE(!key1u2.openKey(OUString::createFromAscii("/MergeKey1u2/MK1SubKey11/KeyWithLongValue"), mkey1u2), "testMerge error 45");
    OSL_ENSURE(!mkey1u2.closeKey(), "testMerge error 46");

    OSL_ENSURE(!key1u2.openKey(OUString::createFromAscii("/MergeKey1u2/MK1SubKey12"), mkey1u2), "testMerge error 47");
    OSL_ENSURE(!mkey1u2.closeKey(), "testMerge error 48");
    OSL_ENSURE(!key1u2.openKey(OUString::createFromAscii("/MergeKey1u2/MK1SubKey12/KeyWithStringValue"), mkey1u2), "testMerge error 49");
    OSL_ENSURE(!mkey1u2.closeKey(), "testMerge error 50");

    OSL_ENSURE(!key1u2.openKey(OUString::createFromAscii("/MergeKey1u2/MK1SubKey13"), mkey1u2), "testMerge error 51");
    OSL_ENSURE(!mkey1u2.closeKey(), "testMerge error 52");
    OSL_ENSURE(!key1u2.openKey(OUString::createFromAscii("/MergeKey1u2/MK1SubKey13/KeyWithUnicodeValue"), mkey1u2), "testMerge error 53");
    OSL_ENSURE(!mkey1u2.closeKey(), "testMerge error 54");

    OSL_ENSURE(!key1u2.openKey(OUString::createFromAscii("/MergeKey1u2/MK2SubKey21"), mkey1u2), "testMerge error 55");
    OSL_ENSURE(!mkey1u2.closeKey(), "testMerge error 56");
    OSL_ENSURE(!key1u2.openKey(OUString::createFromAscii("/MergeKey1u2/MK2SubKey21/KeyWithBinaryValue"), mkey1u2), "testMerge error 57");
    OSL_ENSURE(!mkey1u2.closeKey(), "testMerge error 58");

    OSL_ENSURE(!key1u2.openKey(OUString::createFromAscii("/MergeKey1u2/MK2SubKey22"), mkey1u2), "testMerge error 59");
    OSL_ENSURE(!mkey1u2.closeKey(), "testMerge error 60");
    OSL_ENSURE(!key1u2.openKey(OUString::createFromAscii("/MergeKey1u2/MK2SubKey22/KeyWithLongValue"), mkey1u2), "testMerge error 61");
    OSL_ENSURE(!mkey1u2.closeKey(), "testMerge error 62");

    OSL_ENSURE(!key1u2.openKey(OUString::createFromAscii("/MergeKey1u2/MK2SubKey23"), mkey1u2), "testMerge error 63");
    OSL_ENSURE(!mkey1u2.closeKey(), "testMerge error 64");
    OSL_ENSURE(!key1u2.openKey(OUString::createFromAscii("/MergeKey1u2/MK2SubKey23/KeyWithStringValue"), mkey1u2), "testMerge error 65");
    OSL_ENSURE(!mkey1u2.closeKey(), "testMerge error 66");

    OSL_ENSURE(!key1u2.openKey(OUString::createFromAscii("/MergeKey1u2/MK12SubKey1u2"), mkey1u2), "testMerge error 67");
    OSL_ENSURE(!mkey1u2.closeKey(), "testMerge error 68");
    OSL_ENSURE(!key1u2.openKey(OUString::createFromAscii("/MergeKey1u2/MK12SubKey1u2/KeyWithLongValue"), mkey1u2), "testMerge error 69");
    OSL_ENSURE(!mkey1u2.closeKey(), "testMerge error 70");

    OSL_ENSURE(!key1u2.closeKey(), "testMerge error 71");

    ///////////////////////////////////////////////////////////////////////////

    RegValueType    valueType;
    sal_uInt32          valueSize;
    sal_Int32           int32Value;
    sal_uInt8           *Value;

    OSL_ENSURE(!rootKey.openKey(OUString::createFromAscii("/stardiv/IchbineinMergeKey"), key1), "testMerge error 72");

    OSL_ENSURE(!key1.getValueInfo(OUString::createFromAscii("/MergeKey1/MK1SubKey1/KeyWithLongValue"), &valueType, &valueSize), "testMerge error 73");
    OSL_ENSURE(valueType == RG_VALUETYPE_LONG && valueSize == sizeof(sal_Int32), "testMerge error 74");
    OSL_ENSURE(!key1.getValue(OUString::createFromAscii("/MergeKey1/MK1SubKey1/KeyWithLongValue"), (RegValue)&int32Value), "testMerge error 74.a)");
    OSL_ENSURE(int32Value == lValue1, "testMerge error 74.b)");

    OSL_ENSURE(!key1.getValueInfo(OUString::createFromAscii("/MergeKey1/MK1SubKey2/KeyWithStringValue"), &valueType, &valueSize), "testMerge error 75");
    OSL_ENSURE(valueType == RG_VALUETYPE_STRING && valueSize == strlen(sValue)+1, "testMerge error 76");
    Value = new sal_uInt8[valueSize];
    OSL_ENSURE(!key1.getValue(OUString::createFromAscii("/MergeKey1/MK1SubKey2/KeyWithStringValue"), (RegValue)Value), "testMerge error 76.a)");
    OSL_ENSURE(strcmp((const sal_Char*)Value, sValue) == 0, "testMerge error 76.b)");
    delete(Value);

    OSL_ENSURE(!key1.getValueInfo(OUString::createFromAscii("/MergeKey1/MK1SubKey3/KeyWithUnicodeValue"), &valueType, &valueSize), "testMerge error 77");
    OSL_ENSURE(valueType == RG_VALUETYPE_UNICODE && valueSize == (wValue.getLength()+1)*sizeof(sal_Unicode), "testMerge error 78");
    Value = new sal_uInt8[valueSize];
    OSL_ENSURE(!key1.getValue(OUString::createFromAscii("/MergeKey1/MK1SubKey3/KeyWithUnicodeValue"), (RegValue)Value), "testMerge error 78.a)");
    OSL_ENSURE(wValue.equals( (const sal_Unicode*)Value ), "testMerge error 78.b)");
    delete(Value);

    OSL_ENSURE(!key1.getValueInfo(OUString::createFromAscii("/MergeKey1/MK1SubKey4/KeyWithBinaryValue"), &valueType, &valueSize), "testMerge error 79");
    OSL_ENSURE(valueType == RG_VALUETYPE_BINARY && valueSize == 27, "testMerge error 80");
    Value = new sal_uInt8[valueSize];
    OSL_ENSURE(!key1.getValue(OUString::createFromAscii("/MergeKey1/MK1SubKey4/KeyWithBinaryValue"), (RegValue)Value), "testMerge error 80.a)");
    OSL_ENSURE(strcmp((const sal_Char*)Value, "abcdefghijklmnopqrstuvwxyz") == 0, "testMerge error 80.b)");
    delete(Value);

    OSL_ENSURE(!key1.getValueInfo(OUString::createFromAscii("/MergeKey2/MK2SubKey1/KeyWithBinaryValue"), &valueType, &valueSize), "testMerge error 81");
    OSL_ENSURE(valueType == RG_VALUETYPE_BINARY && valueSize == 11, "testMerge error 82");
    Value = new sal_uInt8[valueSize];
    OSL_ENSURE(!key1.getValue(OUString::createFromAscii("/MergeKey2/MK2SubKey1/KeyWithBinaryValue"), (RegValue)Value), "testMerge error 82.a)");
    OSL_ENSURE(strcmp((const sal_Char*)Value, "1234567890") == 0, "testMerge error 82.b)");
    delete(Value);

    OSL_ENSURE(!key1.getValueInfo(OUString::createFromAscii("/MergeKey2/MK2SubKey2/KeyWithUnicodeValue"), &valueType, &valueSize), "testMerge error 83");
    OSL_ENSURE(valueType == RG_VALUETYPE_UNICODE&& valueSize == (wValue.getLength()+1)*sizeof(sal_Unicode), "testMerge error 84");
    Value = new sal_uInt8[valueSize];
    OSL_ENSURE(!key1.getValue(OUString::createFromAscii("/MergeKey2/MK2SubKey2/KeyWithUnicodeValue"), (RegValue)Value), "testMerge error 84.a)");
    OSL_ENSURE(wValue.equals( (const sal_Unicode*)Value ), "testMerge error 84.b)");
    delete(Value);

    OSL_ENSURE(!key1.getValueInfo(OUString::createFromAscii("/MergeKey2/MK2SubKey3/KeyWithStringValue"), &valueType, &valueSize), "testMerge error 85");
    OSL_ENSURE(valueType == RG_VALUETYPE_STRING && valueSize == strlen(sValue)+1, "testMerge error 86");
    Value = new sal_uInt8[valueSize];
    OSL_ENSURE(!key1.getValue(OUString::createFromAscii("/MergeKey2/MK2SubKey3/KeyWithStringValue"), (RegValue)Value), "testMerge error 86.a)");
    OSL_ENSURE(strcmp((const sal_Char*)Value, sValue) == 0, "testMerge error 86.b)");
    delete(Value);

    OSL_ENSURE(!key1.getValueInfo(OUString::createFromAscii("/MergeKey2/MK2SubKey4/KeyWithLongValue"), &valueType, &valueSize), "testMerge error 87");
    OSL_ENSURE(valueType == RG_VALUETYPE_LONG && valueSize == sizeof(sal_Int32), "testMerge error 88");
    OSL_ENSURE(!key1.getValue(OUString::createFromAscii("/MergeKey2/MK2SubKey4/KeyWithLongValue"), (RegValue)&int32Value), "testMerge error 88.a)");
    OSL_ENSURE(int32Value == lValue1, "testMerge error 88.b)");

    OSL_ENSURE(!key1.getValueInfo(OUString::createFromAscii("/MergeKey1u2/MK1SubKey11/KeyWithLongValue"), &valueType, &valueSize), "testMerge error 89");
    OSL_ENSURE(valueType == RG_VALUETYPE_LONG && valueSize == sizeof(sal_Int32), "testMerge error 90");
    OSL_ENSURE(!key1.getValue(OUString::createFromAscii("/MergeKey1u2/MK1SubKey11/KeyWithLongValue"), (RegValue)&int32Value), "testMerge error 90.a)");
    OSL_ENSURE(int32Value == lValue2, "testMerge error 90.b)");

    OSL_ENSURE(!key1.getValueInfo(OUString::createFromAscii("/MergeKey1u2/MK1SubKey12/KeyWithStringValue"), &valueType, &valueSize), "testMerge error 91");
    OSL_ENSURE(valueType == RG_VALUETYPE_STRING && valueSize == strlen(sValue)+1, "testMerge error 92");
    Value = new sal_uInt8[valueSize];
    OSL_ENSURE(!key1.getValue(OUString::createFromAscii("/MergeKey1u2/MK1SubKey12/KeyWithStringValue"), (RegValue)Value), "testMerge error 92.a)");
    OSL_ENSURE(strcmp((const sal_Char*)Value, sValue) == 0, "testMerge error 92.b)");
    delete(Value);

    OSL_ENSURE(!key1.getValueInfo(OUString::createFromAscii("/MergeKey1u2/MK1SubKey13/KeyWithUnicodeValue"), &valueType, &valueSize), "testMerge error 93");
    OSL_ENSURE(valueType == RG_VALUETYPE_UNICODE && valueSize == (wValue.getLength()+1)*sizeof(sal_Unicode), "testMerge error 94");
    Value = new sal_uInt8[valueSize];
    OSL_ENSURE(!key1.getValue(OUString::createFromAscii("/MergeKey1u2/MK1SubKey13/KeyWithUnicodeValue"), (RegValue)Value), "testMerge error 94.a)");
    OSL_ENSURE(wValue.equals( (const sal_Unicode*)Value ), "testMerge error 94.b)");
    delete(Value);

    OSL_ENSURE(!key1.getValueInfo(OUString::createFromAscii("/MergeKey1u2/MK2SubKey21/KeyWithBinaryValue"), &valueType, &valueSize), "testMerge error 95");
    OSL_ENSURE(valueType == RG_VALUETYPE_BINARY && valueSize == 19, "testMerge error 96");
    Value = new sal_uInt8[valueSize];
    OSL_ENSURE(!key1.getValue(OUString::createFromAscii("/MergeKey1u2/MK2SubKey21/KeyWithBinaryValue"), (RegValue)Value), "testMerge error 96.a)");
    OSL_ENSURE(strcmp((const sal_Char*)Value, "a1b2c3d4e5f6g7h8i9") == 0, "testMerge error 96.b)");
    delete(Value);

    OSL_ENSURE(!key1.getValueInfo(OUString::createFromAscii("/MergeKey1u2/MK2SubKey22/KeyWithLongValue"), &valueType, &valueSize), "testMerge error 97");
    OSL_ENSURE(valueType == RG_VALUETYPE_LONG && valueSize == sizeof(sal_Int32), "testMerge error 98");
    OSL_ENSURE(!key1.getValue(OUString::createFromAscii("/MergeKey1u2/MK2SubKey22/KeyWithLongValue"), (RegValue)&int32Value), "testMerge error 98.a)");
    OSL_ENSURE(int32Value == lValue2, "testMerge error 98.b)");

    OSL_ENSURE(!key1.getValueInfo(OUString::createFromAscii("/MergeKey1u2/MK2SubKey23/KeyWithStringValue"), &valueType, &valueSize), "testMerge error 99");
    OSL_ENSURE(valueType == RG_VALUETYPE_STRING && valueSize == strlen(sValue)+1, "testMerge error 100");
    Value = new sal_uInt8[valueSize];
    OSL_ENSURE(!key1.getValue(OUString::createFromAscii("/MergeKey1u2/MK2SubKey23/KeyWithStringValue"), (RegValue)Value), "testMerge error 100.a)");
    OSL_ENSURE(strcmp((const sal_Char*)Value, sValue) == 0, "testMerge error 100.b)");
    delete(Value);

    OSL_ENSURE(!key1.getValueInfo(OUString::createFromAscii("/MergeKey1u2/MK12SubKey1u2/KeyWithLongValue"), &valueType, &valueSize), "testMerge error 101");
    OSL_ENSURE(valueType == RG_VALUETYPE_LONG && valueSize == sizeof(sal_Int32), "testMerge error 102");
    OSL_ENSURE(!key1.getValue(OUString::createFromAscii("/MergeKey1u2/MK12SubKey1u2/KeyWithLongValue"), (RegValue)&int32Value), "testMerge error 102.a)");
    OSL_ENSURE(int32Value == lValue4, "testMerge error 102.b)");


    OSL_ENSURE(!key1.closeKey(), "testMerge error 24");

    ///////////////////////////////////////////////////////////////////////////

    OSL_ENSURE(!rootKey.closeKey(), "testMerge error 10");

    OSL_ENSURE(!myRegistry->destroy( OUString::createFromAscii("merge1.rdb") ), "test_registry_CppApi error 11");
    OSL_ENSURE(!myRegistry->destroy( OUString::createFromAscii("merge2.rdb") ), "test_registry_CppApi error 12");
    OSL_ENSURE(!myRegistry->destroy( OUString() ), "test_registry_CppApi error 13");

    delete(myRegistry);

    cout << "test_merge() Ok!\n";
    return;
}


