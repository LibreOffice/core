/*************************************************************************
 *
 *  $RCSfile: regtest.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:43 $
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

#include "registry/registry.h"

#ifndef _RTL_USTRING_HXX_
#include    <rtl/ustring.hxx>
#endif

#ifndef _RTL_ALLOC_H_
#include    <rtl/alloc.h>
#endif

using namespace rtl;

#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
{
    RegHandle       hReg;
    RegKeyHandle    hRootKey, hKey1, hKey2, hKey3, hKey4, hKey5;

    cout << "\n     Simple Registry Test !!!     \n\n";

    if (reg_createRegistry(OUString::createFromAscii("test4.rdb").pData, &hReg))
        cout << "\t0. creating registry \"test4.rdb\" failed\n";
    else
        cout << "0. registry test4.rdb is created\n";

    if (reg_openRootKey(hReg, &hRootKey))
        cout << "1. open root key \"test4.rdb\" failed\n";
    else
        cout << "1. root key of \"test4.rdb\" is opened\n";

    if (reg_createKey(hRootKey, OUString::createFromAscii("myFirstKey").pData, &hKey1))
        cout << "\t2. creating key \"myFirstKey\" failed\n";
    else
        cout << "2. key \"myFirstKey\" is created\n";
    if (reg_createKey(hRootKey, OUString::createFromAscii("mySecondKey").pData, &hKey2))
        cout << "\t3. creating key \"mySecondKey\" failed\n";
    else
        cout << "3. key \"mySecondKey\" is created\n";
    if (reg_createKey(hKey1, OUString::createFromAscii("myFirstSubKey").pData, &hKey3))
        cout << "\t4. creating subkey \"myFirstSubKey\" failed\n";
    else
        cout << "4. subkey \"myFirstSubKey\" is created\n";
    if (reg_createKey(hKey1, OUString::createFromAscii("mySecondSubKey").pData, &hKey4))
        cout << "\t5. creating subkey \"mySecondSubKey\" failed\n";
    else
        cout << "5. subkey \"mySecondSubKey\" is created\n";
    if (reg_createKey(hRootKey, OUString::createFromAscii("myThirdKey").pData, &hKey5))
        cout << "\t6. creating key \"myThirdKey\" is created\n\n";
    else
        cout << "6. key \"myThirdKey\" is created\n\n";


    RegKeyHandle*   phSubKeys;
    sal_uInt32      nSubKeys;
    if (reg_openSubKeys(hRootKey, OUString::createFromAscii("myFirstKey").pData, &phSubKeys, &nSubKeys))
        cout << "\t7. open subkeys of \"myfirstKey\" failed\n";
    else
        cout << "7. open " << nSubKeys << "subkeys of \"myfirstKey\"\n";

    OUString keyName;
    if (reg_getKeyName(phSubKeys[0], &keyName.pData))
        cout << "\tname of subkey 1 = " << OUStringToOString(keyName, RTL_TEXTENCODING_ASCII_US).getStr() << "\n";
    if (reg_getKeyName(phSubKeys[1], &keyName.pData))
        cout << "\tname of subkey 2 = " << OUStringToOString(keyName, RTL_TEXTENCODING_ASCII_US).getStr() << "\n";

    if (reg_closeSubKeys(phSubKeys, nSubKeys))
        cout << "\t8. close subkeys of \"myfirstKey\" failed\n\n";
    else
        cout << "8. close " << nSubKeys << "subkeys of \"myfirstKey\"\n\n";


    char* Value="Mein erster Value";
    if (reg_setValue(hRootKey, OUString::createFromAscii("mySecondKey").pData, RG_VALUETYPE_STRING, Value, 18))
        cout << "\t9. setValue of key \"mySecondKey\" failed\n";
    else
        cout << "9. setValue (string Value)  of key \"mySecondKey\"\n";

    RegValueType    valueType;
    sal_uInt32      valueSize;
    sal_Char*       readValue;
    if (reg_getValueInfo(hRootKey, OUString::createFromAscii("mySecondKey").pData, &valueType, &valueSize))
        cout << "\t10. getValueInfo of key \"mySecondKey\" failed\n";
    else
        cout << "10. getValueInfo of key \"mySecondKey\"\n";

    readValue = (sal_Char*)rtl_allocateMemory(valueSize);
    if (reg_getValue(hKey2, OUString().pData, readValue))
        cout << "\t11. getValue of key \"mySecondKey\" failed\n";
    else
    {
        cout << "11. getValue of key \"mySecondKey\"\n";

        cout << "read Value,\n\tvalueType = " << (long)valueType
              << "\n\tvalueSize = " << valueSize
             << "\n\tvalue = " << readValue << "\n\n";
    }
    rtl_freeMemory(readValue);

    if (reg_closeKey(hKey1) ||
        reg_closeKey(hKey3) ||
        reg_closeKey(hKey4))
        cout << "\t12. closing \"myFirstKey\" \"myfistSubKey\" \"mySecondSubKey\" failed\n";
    else
        cout << "12. keys \"myFirstKey\" \"myfistSubKey\" \"mySecondSubKey\" are closed\n";

    if (reg_deleteKey(hRootKey, OUString::createFromAscii("myFirstKey").pData))
        cout << "13.\t delete key \"myFirstKey\" failed\n";
    else
        cout << "13. key \"myFirstKey\" is deleted\n";

    if (reg_closeKey(hKey2))
        cout << "\t14. closing key \"mySecondKey\" failed\n";
    else
        cout << "14. key \"mySecondKey\" is closed\n";

    if (reg_openKey(hRootKey, OUString::createFromAscii("mySecondKey").pData, &hKey2))
        cout << "\n15. open key \"mySecondKey\" failed\n";
    else
        cout << "15. key \"mySecondKey\" is opended\n";

    if (reg_closeKey(hKey5))
        cout << "\t15. closing key \"myThirdSubKey\" failed\n";
    else
        cout << "15. key \"myThirdSubKey\" is closed\n";
    if (reg_deleteKey(hRootKey, OUString::createFromAscii("myThirdKey").pData))
        cout << "\t16. delete key \"myThirdKey\" failed\n";
    else
        cout << "16. key \"myThirdKey\" is deleted\n";

    if (reg_openKey(hRootKey, OUString::createFromAscii("myThirdKey").pData, &hKey5))
        cout << "\t17. open key \"myThirdKey\" failed\n";
    else
        cout << "17. key \"myThirdKey\" is opened\n";

    cout << "\n close open keys\n\n";

    if (reg_closeKey(hKey2))
        cout << "\t18. closing key \"mySecondKey\" failed\n";
    else
        cout << "18. key \"mySecondKey\" is closed\n";

    if (reg_closeKey(hRootKey))
        cout << "\t19. closing root key failed\n";
    else
        cout << "19. root key is closed\n";

    if (reg_closeRegistry(hReg))
        cout << "\t20. closing registry \"test4.rdb\" failed\n";
    else
        cout << "20. registry \"test4.rdb\" is closed\n";

    // Test loadkey
    cout << "\nTest load key\n\n";

    RegHandle       hReg2;
    RegKeyHandle    hRootKey2, h2Key1, h2Key2, h2Key3, h2Key4, h2Key5;

    if (reg_createRegistry(OUString::createFromAscii("test5.rdb").pData, &hReg2))
        cout << "\t21. creating registry \"test5.rdb\" failed\n";
    else
        cout << "21. registry \"test5.rdb\" is created\n";

    if (reg_openRootKey(hReg2, &hRootKey2))
        cout << "\t22. open root key of \"test5.rdb\" failed\n";
    else
        cout << "22. root key of \"test5.rdb\" is opened\n";

    if (reg_createKey(hRootKey2, OUString::createFromAscii("reg2FirstKey").pData, &h2Key1))
        cout << "\t23. creating key \"reg2FirstKey\" failed\n";
    else
        cout << "23. key \"reg2FirstKey\" is created\n";
    if (reg_createKey(hRootKey2, OUString::createFromAscii("reg2SecondKey").pData, &h2Key2))
        cout << "\t24. creating key \"reg2SecondKey\" failed\n";
    else
        cout << "24. key \"reg2SecondKey\" is created\n";
    if (reg_createKey(h2Key1, OUString::createFromAscii("reg2FirstSubKey").pData, &h2Key3))
        cout << "\t25. creating key \"reg2FirstSubKey\" failed\n";
    else
        cout << "25. key \"reg2FirstSubKey\" is created\n";
    if (reg_createKey(h2Key1, OUString::createFromAscii("reg2SecondSubKey").pData, &h2Key4))
        cout << "\26. creating key \"reg2SecondSubKey\" failed\n";
    else
        cout << "26. key \"reg2SecondSubKey\" is created\n";
    if (reg_createKey(hRootKey2, OUString::createFromAscii("reg2ThirdKey").pData, &h2Key5))
        cout << "\n27. creating key \"reg2ThirdKey\" failed\n";
    else
        cout << "27. key \"reg2ThirdKey\" is created\n";

    sal_uInt32 nValue= 123456789;
    if (reg_setValue(h2Key3, OUString().pData, RG_VALUETYPE_LONG, &nValue, sizeof(sal_uInt32)))
        cout << "\t27.b) setValue of key \"reg2FirstSubKey\" failed\n";
    else
        cout << "27.b). setValue (long Value)  of key \"reg2FirstSubKey\"\n";

    if (reg_closeKey(h2Key1) ||
        reg_closeKey(h2Key2) ||
        reg_closeKey(h2Key3) ||
        reg_closeKey(h2Key4) ||
        reg_closeKey(h2Key5))
        cout << "\n\t28. closing keys of \"test5.rdb\" failed\n";
    else
        cout << "\n28. all keys of \"test5.rdb\" closed\n";

    if (reg_closeKey(hRootKey2))
        cout << "\t29. root key of \"test5.rdb\" failed\n";
    else
        cout << "29. root key of \"test5.rdb\" is closed\n";

    if (reg_closeRegistry(hReg2))
        cout << "\t30. registry test5.rdb is closed\n";
    else
        cout << "30. registry test5.rdb is closed\n";

    if (reg_openRegistry(OUString::createFromAscii("test4.rdb").pData, &hReg, REG_READWRITE))
        cout << "\t31. registry test4.rdb is opened\n";
    else
        cout << "31. registry test4.rdb is opened\n";

    if (reg_openRootKey(hReg, &hRootKey))
        cout << "\t32. open root key of \"test4.rdb\" is failed\n";
    else
        cout << "32. root key of \"test4.rdb\" is opened\n";

    if (reg_loadKey(hRootKey, OUString::createFromAscii("allFromTest2").pData,
                    OUString::createFromAscii("test5.rdb").pData))
        cout << "\n\t33. load all keys from \"test5.rdb\" under key \"allFromTest2\" failed\n";
    else
        cout << "\n33. load all keys from test5.rdb under key \"allFromTest2\"\n";

    if (reg_saveKey(hRootKey, OUString::createFromAscii("allFromTest2").pData,
                    OUString::createFromAscii("test6.rdb").pData))
        cout << "\n\t34. save all keys under \"allFromTest2\" in test6.rdb\n";
    else
        cout << "\n34. save all keys under \"allFromTest2\" in test6.rdb\n";


    if (reg_createKey(hRootKey, OUString::createFromAscii("allFromTest3").pData, &hKey1))
        cout << "\t35. creating key \"allFromTest3\" failed\n";
    else
        cout << "36. key \"allFromTest3\" is created\n";
    if (reg_createKey(hKey1, OUString::createFromAscii("myFirstKey2").pData, &hKey2))
        cout << "\t37. creating key \"myFirstKey2\" failed\n";
    else
        cout << "37. key \"myFirstKey2\" is created\n";
    if (reg_createKey(hKey1, OUString::createFromAscii("mySecondKey2").pData, &hKey3))
        cout << "\t38. creating key \"mySecondKey2\" failed\n";
    else
        cout << "38. key \"mySecondKey2\" is created\n";

    if (reg_mergeKey(hRootKey, OUString::createFromAscii("allFromTest3").pData,
                    OUString::createFromAscii("test6.rdb").pData, sal_False, sal_False))
        cout << "\n\t39. merge all keys under \"allFromTest2\" with all in test6.rdb\n";
    else
        cout << "\n39. merge all keys under \"allFromTest2\" with all in test6.rdb\n";

    if (reg_closeKey(hKey1))
        cout << "\n\t40. closing key \"allFromTest3\" of \"test5.rdb\" failed\n";
    else
        cout << "\n40. closing key \"allFromTest3\" of \"test5.rdb\"\n";
    if (reg_closeKey(hKey2))
        cout << "\n\t41. closing key \"myFirstKey2\" of \"test5.rdb\" failed\n";
    else
        cout << "\n41. closing key \"myFirstKey2\" of \"test5.rdb\"\n";
    if (reg_closeKey(hKey3))
        cout << "\n\t42. closing key \"mySecondKey2\" of \"test5.rdb\" failed\n";
    else
        cout << "\n42. closing key \"mySecondKey2\" of \"test5.rdb\"\n";


    if (reg_deleteKey(hRootKey, OUString::createFromAscii("/allFromTest3/reg2FirstKey/reg2FirstSubKey").pData))
        cout << "\n\t43. delete key \"/allFromTest3/reg2FirstKey/reg2FirstSubKey\" failed\n";
    else
        cout << "\n43. key \"/allFromTest3/reg2FirstKey/reg2FirstSubKey\" is deleted\n";

    if (reg_openRegistry(OUString::createFromAscii("test4.rdb").pData, &hReg2, REG_READONLY))
        cout << "\n\t44. registry test4.rdb is opened for read only\n";
    else
        cout << "\n44. registry test4.rdb is opened for read only\n";

    RegHandle hReg3;
    if (reg_openRegistry(OUString::createFromAscii("test4.rdb").pData, &hReg3, REG_READONLY))
        cout << "\n\t44.a). registry test4.rdb is opened for read only\n";
    else
        cout << "\n44.a). registry test4.rdb is opened for read only\n";

    if (reg_closeRegistry(hReg2))
        cout << "\t45. closing registry \"test4.rdb\" failed\n";
    else
        cout << "45. registry \"test4.rdb\" is closed\n";

    if (reg_closeKey(hRootKey))
        cout << "\n\t46. closing root key of \"test4.rdb\" failed\n";
    else
        cout << "\n46. root key of \"test4.rdb\" is closed\n";

    if (reg_closeRegistry(hReg))
        cout << "\t47. closing registry \"test4.rdb\" failed\n";
    else
        cout << "47. registry \"test4.rdb\" is closed\n";

    if (reg_closeRegistry(hReg3))
        cout << "\t47.a). closing registry \"test4.rdb\" failed\n";
    else
        cout << "47.a). registry \"test4.rdb\" is closed\n";

    return(0);
}


