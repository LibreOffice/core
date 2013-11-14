/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "UAccCOM2.h"

#include "UAccCOM_i.c"
#include "ia2_api_all_i.c"

#include "MAccessible.h"
#include "EnumVariant.h"
#include "UNOXWrapper.h"
#include "AccComponent.h"
#include "AccRelation.h"
#include "AccAction.h"
#include "AccText.h"
#include "AccEditableText.h"
#include "AccImage.h"
#include "AccValue.h"
#include "AccTable.h"
#include "AccHyperLink.h"
#include "AccHyperText.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_MAccessible, CMAccessible)
OBJECT_ENTRY(CLSID_EnumVariant, CEnumVariant)
OBJECT_ENTRY(CLSID_AccComponent, CAccComponent)
OBJECT_ENTRY(CLSID_AccRelation, CAccRelation)
OBJECT_ENTRY(CLSID_AccAction, CAccAction)
OBJECT_ENTRY(CLSID_AccText, CAccText)
OBJECT_ENTRY(CLSID_AccEditableText, CAccEditableText)
OBJECT_ENTRY(CLSID_AccImage, CAccImage)
OBJECT_ENTRY(CLSID_AccValue, CAccValue)
OBJECT_ENTRY(CLSID_AccTable, CAccTable)
OBJECT_ENTRY(CLSID_AccHyperLink, CAccHyperLink)
OBJECT_ENTRY(CLSID_AccHypertext, CAccHypertext)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
    BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_UACCCOMLib);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    return _Module.RegisterServer(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}
