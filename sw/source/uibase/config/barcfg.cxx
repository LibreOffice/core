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
#include <com/sun/star/uno/Sequence.hxx>
#include <wrtsh.hxx>
#include <barcfg.hxx>

using namespace utl;
using namespace com::sun::star::uno;

#define SEL_TYPE_TABLE_TEXT     0
#define SEL_TYPE_LIST_TEXT      1
#define SEL_TYPE_TABLE_LIST     2
#define SEL_TYPE_BEZIER         3
#define SEL_TYPE_GRAPHIC        4

SwToolbarConfigItem::SwToolbarConfigItem( bool bWeb ) :
    ConfigItem(bWeb ? OUString("Office.WriterWeb/ObjectBar") : OUString("Office.Writer/ObjectBar"),
        ConfigItemMode::ReleaseTree)
{
    for(int i = 0; i <= SEL_TYPE_GRAPHIC; ++i)
        m_aTbxIdArray[i] = -1;

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
                m_aTbxIdArray[nProp] = nVal;
            }
        }
    }
}

SwToolbarConfigItem::~SwToolbarConfigItem()
{
}

static sal_Int32 lcl_getArrayIndex(SelectionType nSelType)
{
    sal_Int32 nRet = -1;
    if(nSelType & SelectionType::NumberList)
    {
        if(nSelType & SelectionType::Table)
            nRet = SEL_TYPE_TABLE_LIST;
        else
            nRet = SEL_TYPE_LIST_TEXT;
    }
    else if(nSelType & SelectionType::Table)
        nRet = SEL_TYPE_TABLE_TEXT;
    else if(nSelType & SelectionType::Ornament)
        nRet = SEL_TYPE_BEZIER;
    else if(nSelType & SelectionType::Graphic)
        nRet = SEL_TYPE_GRAPHIC;
    return nRet;
}

void SwToolbarConfigItem::SetTopToolbar(SelectionType nSelType, ToolbarId eBarId)
{
    sal_Int32 nProp = lcl_getArrayIndex(nSelType);
    if(nProp >= 0)
    {
        m_aTbxIdArray[nProp] = static_cast<sal_Int32>(eBarId);
        SetModified();
    }
}

Sequence<OUString> SwToolbarConfigItem::GetPropertyNames()
{
    static constexpr OUString aPropNames[] =
    {
        u"Selection/Table"_ustr,                   //  SEL_TYPE_TABLE_TEXT
        u"Selection/NumberedList"_ustr,            //  SEL_TYPE_LIST_TEXT
        u"Selection/NumberedList_InTable"_ustr,     //  SEL_TYPE_TABLE_LIST
        u"Selection/BezierObject"_ustr,           //  SEL_TYPE_BEZIER
        u"Selection/Graphic"_ustr                 //SEL_TYPE_GRAPHIC
    };
    const int nCount = 5;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
        pNames[i] = aPropNames[i];
    return aNames;
}

void SwToolbarConfigItem::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        pValues[nProp] <<= m_aTbxIdArray[nProp];
    PutProperties(aNames, aValues);
}

void SwToolbarConfigItem::Notify( const css::uno::Sequence< OUString >& ) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
