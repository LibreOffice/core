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

#ifndef __ACCCOMMON_H_
#define __ACCCOMMON_H_


//using namespace com::sun::star::accessibility;
//using namespace com::sun::star::uno;


//for MAccessible.cpp
struct ltComp
{
    bool operator()(REFGUID  rguid1, REFGUID  rguid2) const
    {
        if(((PLONG) &rguid1)[0] < ((PLONG) &rguid2)[0])
            return TRUE;
        else if(((PLONG) &rguid1)[0] > ((PLONG) &rguid2)[0])
            return FALSE;
        if(((PLONG) &rguid1)[1] < ((PLONG) &rguid2)[1])
            return TRUE;
        else if(((PLONG) &rguid1)[1] > ((PLONG) &rguid2)[1])
            return FALSE;
        if(((PLONG) &rguid1)[2] < ((PLONG) &rguid2)[2])
            return TRUE;
        else if(((PLONG) &rguid1)[2] > ((PLONG) &rguid2)[2])
            return FALSE;
        if(((PLONG) &rguid1)[3] < ((PLONG) &rguid2)[3])
            return TRUE;
        else if(((PLONG) &rguid1)[3] > ((PLONG) &rguid2)[3])
            return FALSE;
        return FALSE;
    }
};

#define BEGIN_AGGOBJECT_MAP(x) \
    static _UNO_AGGMAP_ENTRY* WINAPI _GetAggEntries() { \
    static _UNO_AGGMAP_ENTRY _aggentries[] = {
#define END_AGGOBJECT_MAP()   {NULL, NULL,NULL}}; \
    return _aggentries;}
#define AGGOBJECT_ENTRY(iid, clsid, ifindex) {&iid, \
    CComAggregateCreator< CComObject<CMAccessible>, &clsid >::CreateInstance,\
    XI_##ifindex},

enum DM_NIR {
    DM_FIRSTCHILD = 0x00,
    DM_LASTCHILD = 0x01,
    DM_NEXTCHILD = 0x02,
    DM_PREVCHILD = 0x03
};

inline BOOL ImplIsEqualGUID(REFGUID rguid1, REFGUID rguid2)
{
    return (
               ((PLONG) &rguid1)[0] == ((PLONG) &rguid2)[0] &&
               ((PLONG) &rguid1)[1] == ((PLONG) &rguid2)[1] &&
               ((PLONG) &rguid1)[2] == ((PLONG) &rguid2)[2] &&
               ((PLONG) &rguid1)[3] == ((PLONG) &rguid2)[3]);
}
#define InlineIsEqualGUID ImplIsEqualGUID
//End

//for AccAction.cpp
#define CODEENTRY(key) \
    {KEYCODE_##key, L#key}
#define countof(x)  (sizeof(x)/sizeof(x[0]))

#define SELECT       L"Select"
#define PRESS        L"Press"
#define UNCHECK      L"UnCheck"
#define CHECK        L"Check"
//End

static DWORD GetMSAAStateFromUNO(short xState);

#endif
