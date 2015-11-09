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

#include <osl/diagnose.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <wrtsh.hxx>
#include "barcfg.hxx"

#include <unomid.h>

using namespace utl;
using namespace com::sun::star::uno;

#define SEL_TYPE_TABLE_TEXT     0
#define SEL_TYPE_LIST_TEXT      1
#define SEL_TYPE_TABLE_LIST     2
#define SEL_TYPE_BEZIER         3
#define SEL_TYPE_GRAPHIC        4

SwToolbarConfigItem::SwToolbarConfigItem( bool bWeb ) :
    ConfigItem(bWeb ? OUString("Office.WriterWeb/ObjectBar") : OUString("Office.Writer/ObjectBar"),
        ConfigItemMode::DelayedUpdate|ConfigItemMode::ReleaseTree)
{
    for(int i = 0; i <= SEL_TYPE_GRAPHIC; ++i)
        aTbxIdArray[i] = -1;

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
                sal_Int32 nVal = 0;
                pValues[nProp] >>= nVal;
                aTbxIdArray[nProp] = nVal;
            }
        }
    }
}

SwToolbarConfigItem::~SwToolbarConfigItem()
{
}

static sal_Int32 lcl_getArrayIndex(int nSelType)
{
    sal_Int32 nRet = -1;
    if(nSelType & nsSelectionType::SEL_NUM)
    {
        if(nSelType & nsSelectionType::SEL_TBL)
            nRet = SEL_TYPE_TABLE_LIST;
        else
            nRet = SEL_TYPE_LIST_TEXT;
    }
    else if(nSelType & nsSelectionType::SEL_TBL)
        nRet = SEL_TYPE_TABLE_TEXT;
    else if(nSelType & nsSelectionType::SEL_BEZ)
        nRet = SEL_TYPE_BEZIER;
    else if(nSelType & nsSelectionType::SEL_GRF)
        nRet = SEL_TYPE_GRAPHIC;
    return nRet;
}

void SwToolbarConfigItem::SetTopToolbar( sal_Int32 nSelType, sal_Int32 nBarId )
{
    sal_Int32 nProp = lcl_getArrayIndex(nSelType);
    if(nProp >= 0)
    {
        aTbxIdArray[nProp] = nBarId;
        SetModified();
    }
}

Sequence<OUString> SwToolbarConfigItem::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Selection/Table",                   //  SEL_TYPE_TABLE_TEXT
        "Selection/NumberedList",            //  SEL_TYPE_LIST_TEXT
        "Selection/NumberedList_InTable",     //  SEL_TYPE_TABLE_LIST
        "Selection/BezierObject",           //  SEL_TYPE_BEZIER
        "Selection/Graphic"                 //SEL_TYPE_GRAPHIC
    };
    const int nCount = 5;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    return aNames;
}

void SwToolbarConfigItem::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        pValues[nProp] <<= aTbxIdArray[nProp];
    PutProperties(aNames, aValues);
}

void SwToolbarConfigItem::Notify( const css::uno::Sequence< OUString >& ) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
