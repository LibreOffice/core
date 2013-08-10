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

#include <numberingtypelistbox.hxx>
#include <misc.hrc>
#include <cnttab.hxx>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/text/DefaultNumberingProvider.hpp>
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/text/XNumberingTypeInfo.hpp>

#include <unomid.h>

using namespace com::sun::star;


struct SwNumberingTypeListBox_Impl
{
    uno::Reference<text::XNumberingTypeInfo> xInfo;
};

SwNumberingTypeListBox::SwNumberingTypeListBox( Window* pWin, WinBits nStyle ) :
    ListBox(pWin, nStyle),
    pImpl(new SwNumberingTypeListBox_Impl)
{
    uno::Reference<uno::XComponentContext>          xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference<text::XDefaultNumberingProvider> xDefNum = text::DefaultNumberingProvider::create(xContext);

    pImpl->xInfo = uno::Reference<text::XNumberingTypeInfo>(xDefNum, uno::UNO_QUERY);
}

bool SwNumberingTypeListBox::set_property(const OString &rKey, const OString &rValue)
{
    if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("type")))
        Reload(rValue.toInt32());
    else
        return ListBox::set_property(rKey, rValue);
    return true;
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSwNumberingTypeListBox(Window *pParent, VclBuilder::stringmap &)
{
    SwNumberingTypeListBox *pListBox = new SwNumberingTypeListBox(pParent, WB_LEFT|WB_DROPDOWN|WB_VCENTER|WB_3DLOOK|WB_TABSTOP);
    pListBox->EnableAutoSize(true);
    return pListBox;
}

SwNumberingTypeListBox::~SwNumberingTypeListBox()
{
    delete pImpl;
}

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
        sal_IntPtr nValue = rNames.GetValue(i);
        bool bInsert = true;
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
                    bInsert = false;
                    if (pTypes)
                    {
                        for(sal_Int32 nType = 0; nType < aTypes.getLength(); nType++)
                        {
                            if (pTypes[nType] == nValue)
                            {
                                bInsert = true;
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

sal_Int16   SwNumberingTypeListBox::GetSelectedNumberingType()
{
    sal_Int16 nRet = 0;
    sal_uInt16 nSelPos = GetSelectEntryPos();
    if(LISTBOX_ENTRY_NOTFOUND != nSelPos)
        nRet = (sal_Int16)(sal_uLong)GetEntryData(nSelPos);
#if OSL_DEBUG_LEVEL > 0
    else
        OSL_FAIL("SwNumberingTypeListBox not selected");
#endif
    return nRet;
}

sal_Bool    SwNumberingTypeListBox::SelectNumberingType(sal_Int16 nType)
{
    sal_uInt16 nPos = GetEntryPos((void*)(sal_uLong)nType);
    SelectEntryPos( nPos );
    return LISTBOX_ENTRY_NOTFOUND != nPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
