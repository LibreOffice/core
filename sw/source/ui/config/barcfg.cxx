/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: barcfg.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:45:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
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

/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
SwToolbarConfigItem::SwToolbarConfigItem( BOOL bWeb ) :
    ConfigItem(bWeb ? C2U("Office.WriterWeb/ObjectBar") :  C2U("Office.Writer/ObjectBar"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE)
{
    for(USHORT i = 0; i <= SEL_TYPE_GRAPHIC; i++ )
        aTbxIdArray[i] = -1;

    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
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
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
SwToolbarConfigItem::~SwToolbarConfigItem()
{
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
sal_Int32 lcl_getArrayIndex(int nSelType)
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
/* -----------------------------10.10.00 14:38--------------------------------

 ---------------------------------------------------------------------------*/
void SwToolbarConfigItem::SetTopToolbar( sal_Int32 nSelType, sal_Int32 nBarId )
{
    sal_Int32 nProp = lcl_getArrayIndex(nSelType);
    if(nProp >= 0)
    {
        aTbxIdArray[nProp] = nBarId;
        SetModified();
    }
}
/* -----------------------------10.10.00 13:33--------------------------------

 ---------------------------------------------------------------------------*/
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
/* -----------------------------10.10.00 13:36--------------------------------

 ---------------------------------------------------------------------------*/
void SwToolbarConfigItem::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        pValues[nProp] <<= aTbxIdArray[nProp];
    PutProperties(aNames, aValues);
}

