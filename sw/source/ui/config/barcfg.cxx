/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <osl/diagnose.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <wrtsh.hxx>
#include "barcfg.hxx"

#include <unomid.h>

using namespace utl;
using rtl::OUString;
using namespace com::sun::star::uno;

#define SEL_TYPE_TABLE_TEXT     0
#define SEL_TYPE_LIST_TEXT      1
#define SEL_TYPE_TABLE_LIST     2
#define SEL_TYPE_BEZIER         3
#define SEL_TYPE_GRAPHIC        4

SwToolbarConfigItem::SwToolbarConfigItem( sal_Bool bWeb ) :
    ConfigItem(bWeb ? C2U("Office.WriterWeb/ObjectBar") :  C2U("Office.Writer/ObjectBar"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE)
{
    for(sal_uInt16 i = 0; i <= SEL_TYPE_GRAPHIC; i++ )
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

void SwToolbarConfigItem::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        pValues[nProp] <<= aTbxIdArray[nProp];
    PutProperties(aNames, aValues);
}

void SwToolbarConfigItem::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
