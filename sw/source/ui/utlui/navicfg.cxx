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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <swtypes.hxx>  // fuer Pathfinder
#include <navicfg.hxx>
#include <swcont.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <unomid.h>

using namespace ::utl;
using namespace ::rtl;
using namespace ::com::sun::star::uno;

/* -----------------------------08.09.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
Sequence<OUString> SwNavigationConfig::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "RootType",             //0
        "SelectedPosition",     //1
        "OutlineLevel",         //2
        "InsertMode",           //3
        "ActiveBlock",          //4
        "ShowListBox",          //5
        "GlobalDocMode"         //6
    };
    const int nCount = 7;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
    {
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}
/*-----------------13.11.96 11.03-------------------

--------------------------------------------------*/

SwNavigationConfig::SwNavigationConfig() :
    utl::ConfigItem(C2U("Office.Writer/Navigator")),
    nRootType(0xffff),
    nSelectedPos(0),
    nOutlineLevel(MAXLEVEL),
    nRegionMode(REGION_MODE_NONE),
    nActiveBlock(0),
    bIsSmall(sal_False),
    bIsGlobalActive(sal_True)
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
//  EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case 0: pValues[nProp] >>= nRootType;      break;
                    case 1: pValues[nProp] >>= nSelectedPos;   break;
                    case 2: pValues[nProp] >>= nOutlineLevel;  break;
                    case 3: pValues[nProp] >>= nRegionMode;    break;
                    case 4: pValues[nProp] >>= nActiveBlock;    break;
                    case 5: bIsSmall        = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 6: bIsGlobalActive = *(sal_Bool*)pValues[nProp].getValue();  break;
                }
            }
        }
    }
}
/* -----------------------------08.09.00 16:35--------------------------------

 ---------------------------------------------------------------------------*/
SwNavigationConfig::~SwNavigationConfig()
{}
/* -----------------------------08.09.00 16:35--------------------------------

 ---------------------------------------------------------------------------*/
void SwNavigationConfig::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();
    const Type& rType = ::getBooleanCppuType();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case 0: pValues[nProp] <<= nRootType;     break;
            case 1: pValues[nProp] <<= nSelectedPos;  break;
            case 2: pValues[nProp] <<= nOutlineLevel; break;
            case 3: pValues[nProp] <<= nRegionMode;   break;
            case 4: pValues[nProp] <<= nActiveBlock;    break;
            case 5: pValues[nProp].setValue(&bIsSmall, rType);          break;
            case 6: pValues[nProp].setValue(&bIsGlobalActive, rType);   break;
        }
    }
    PutProperties(aNames, aValues);
}

void SwNavigationConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}


