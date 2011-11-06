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
#include <numberingtypelistbox.hxx>
#ifndef _MISC_HRC
#include <misc.hrc>
#endif
#include <cnttab.hxx>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/text/XNumberingTypeInfo.hpp>

#include <unomid.h>

using namespace com::sun::star;
using namespace rtl;

/* -----------------------------01.03.01 16:04--------------------------------

 ---------------------------------------------------------------------------*/
struct SwNumberingTypeListBox_Impl
{
    uno::Reference<text::XNumberingTypeInfo> xInfo;
};
/* -----------------------------01.03.01 14:46--------------------------------

 ---------------------------------------------------------------------------*/
SwNumberingTypeListBox::SwNumberingTypeListBox( Window* pWin, const ResId& rResId,
        sal_uInt16 nTypeFlags ) :
    ListBox(pWin, rResId),
    pImpl(new SwNumberingTypeListBox_Impl)
{
    uno::Reference< lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    uno::Reference < uno::XInterface > xI = xMSF->createInstance(
        ::rtl::OUString::createFromAscii( "com.sun.star.text.DefaultNumberingProvider" ) );
    uno::Reference<text::XDefaultNumberingProvider> xDefNum(xI, uno::UNO_QUERY);
    DBG_ASSERT(xDefNum.is(), "service missing: \"com.sun.star.text.DefaultNumberingProvider\"");

    pImpl->xInfo = uno::Reference<text::XNumberingTypeInfo>(xDefNum, uno::UNO_QUERY);
    Reload(nTypeFlags);
}
/* -----------------------------01.03.01 14:46--------------------------------

 ---------------------------------------------------------------------------*/
SwNumberingTypeListBox::~SwNumberingTypeListBox()
{
    delete pImpl;
}
/* -----------------------------01.03.01 16:02--------------------------------

 ---------------------------------------------------------------------------*/
void SwNumberingTypeListBox::Reload(sal_uInt16 nTypeFlags)
{
    Clear();
    uno::Sequence<sal_Int16> aTypes;
    const sal_Int16* pTypes = NULL;
    if(0 != (nTypeFlags&INSERT_NUM_EXTENDED_TYPES) )
    {
        if(pImpl->xInfo.is())
        {
            aTypes = pImpl->xInfo->getSupportedNumberingTypes();
            pTypes = aTypes.getConstArray();
        }
    }
    SwOLENames aNames(SW_RES(STRRES_NUMTYPES));
    ResStringArray& rNames = aNames.GetNames();
    for(sal_uInt16 i = 0; i < rNames.Count(); i++)
    {
        long nValue = rNames.GetValue(i);
        sal_Bool bInsert = sal_True;
        sal_uInt16 nPos = LISTBOX_APPEND;
        switch(nValue)
        {
            case  style::NumberingType::NUMBER_NONE:
                bInsert = 0 != (nTypeFlags&INSERT_NUM_TYPE_NO_NUMBERING);
                nPos = 0;
             break;
            case  style::NumberingType::CHAR_SPECIAL:   bInsert = 0 != (nTypeFlags&INSERT_NUM_TYPE_BULLET); break;
            case  style::NumberingType::PAGE_DESCRIPTOR:bInsert = 0 != (nTypeFlags&INSERT_NUM_TYPE_PAGE_STYLE_NUMBERING); break;
            case  style::NumberingType::BITMAP:bInsert = 0 != (nTypeFlags&INSERT_NUM_TYPE_BITMAP ); break;
            default:
                if (nValue >  style::NumberingType::CHARS_LOWER_LETTER_N)
                {
                    // Insert only if offered by i18n framework per configuration.
                    bInsert = sal_False;
                    if (pTypes)
                    {
                        for(sal_Int32 nType = 0; nType < aTypes.getLength(); nType++)
                        {
                            if (pTypes[nType] == nValue)
                            {
                                bInsert = sal_True;
                                break;  // for
                            }
                        }
                    }
                }
        }
        if(bInsert)
        {
            sal_uInt16 nEntry = InsertEntry(rNames.GetString(i), nPos);
            SetEntryData( nEntry, (void*)nValue );
        }
    }
    if(0 != (nTypeFlags&INSERT_NUM_EXTENDED_TYPES) )
    {
        if(pTypes)
        {
            for(sal_Int32 nType = 0; nType < aTypes.getLength(); nType++)
            {
                sal_Int16 nCurrent = pTypes[nType];
                if(nCurrent > style::NumberingType::CHARS_LOWER_LETTER_N)
                {
                    if(LISTBOX_ENTRY_NOTFOUND == GetEntryPos((void*)(sal_uLong)nCurrent))
                    {
                        OUString aIdent = pImpl->xInfo->getNumberingIdentifier( nCurrent );
                        sal_uInt16 nPos = InsertEntry(aIdent);
                        SetEntryData(nPos,(void*)(sal_uLong)nCurrent);
                    }
                }
            }
        }
        SelectEntryPos(0);
    }
}
/* -----------------------------01.03.01 14:46--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int16   SwNumberingTypeListBox::GetSelectedNumberingType()
{
    sal_Int16 nRet = 0;
    sal_uInt16 nSelPos = GetSelectEntryPos();
    if(LISTBOX_ENTRY_NOTFOUND != nSelPos)
        nRet = (sal_Int16)(sal_uLong)GetEntryData(nSelPos);
#ifdef DBG_UTIL
    else
        DBG_ERROR("SwNumberingTypeListBox not selected");
#endif
    return nRet;
}
/* -----------------------------01.03.01 14:46--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool    SwNumberingTypeListBox::SelectNumberingType(sal_Int16 nType)
{
    sal_uInt16 nPos = GetEntryPos((void*)(sal_uLong)nType);
    SelectEntryPos( nPos );
    return LISTBOX_ENTRY_NOTFOUND != nPos;
}

