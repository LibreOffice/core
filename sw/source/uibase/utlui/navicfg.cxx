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

#include <swtypes.hxx>
#include <navicfg.hxx>
#include <swcont.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <unomid.h>

using namespace ::utl;
using namespace ::com::sun::star::uno;

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

SwNavigationConfig::SwNavigationConfig() :
    utl::ConfigItem("Office.Writer/Navigator"),
    nRootType(ContentTypeId::UNKNOWN),
    nSelectedPos(0),
    nOutlineLevel(MAXLEVEL),
    nRegionMode(RegionMode::NONE),
    nActiveBlock(0),
    bIsSmall(false),
    bIsGlobalActive(true)
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case 0:
                    {
                        sal_uInt32 nTmp;
                        if (pValues[nProp] >>= nTmp)
                        {
                            if (nTmp > sal_uInt32(ContentTypeId::LAST)) {
                                nTmp = sal_uInt32(ContentTypeId::UNKNOWN);
                            }
                            nRootType = static_cast<ContentTypeId>(nTmp);
                        }
                        break;
                    }
                    case 1: pValues[nProp] >>= nSelectedPos;   break;
                    case 2: pValues[nProp] >>= nOutlineLevel;  break;
                    case 3:
                    {
                            sal_uInt16 nTmp;
                            if (pValues[nProp] >>= nTmp)
                                nRegionMode = static_cast<RegionMode>(nTmp);
                            break;
                    }
                    case 4: pValues[nProp] >>= nActiveBlock;    break;
                    case 5: bIsSmall        = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break;
                    case 6: bIsGlobalActive = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break;
                }
            }
        }
    }
}

SwNavigationConfig::~SwNavigationConfig()
{
}

void SwNavigationConfig::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case 0: pValues[nProp] <<= static_cast<sal_uInt32>(nRootType);     break;
            case 1: pValues[nProp] <<= nSelectedPos;  break;
            case 2: pValues[nProp] <<= nOutlineLevel; break;
            case 3: pValues[nProp] <<= static_cast<sal_uInt16>(nRegionMode); break;
            case 4: pValues[nProp] <<= nActiveBlock;    break;
            case 5: pValues[nProp] <<= bIsSmall; break;
            case 6: pValues[nProp] <<= bIsGlobalActive; break;
        }
    }
    PutProperties(aNames, aValues);
}

void SwNavigationConfig::Notify( const ::com::sun::star::uno::Sequence< OUString >& ) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
