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

using ::rtl::OUString;

struct SwNumberingTypeListBox_Impl
{
    uno::Reference<text::XNumberingTypeInfo> xInfo;
};

SwNumberingTypeListBox::SwNumberingTypeListBox( Window* pWin, const ResId& rResId,
        sal_uInt16 nTypeFlags ) :
    ListBox(pWin, rResId),
    pImpl(new SwNumberingTypeListBox_Impl)
{
    uno::Reference<uno::XComponentContext>          xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference<text::XDefaultNumberingProvider> xDefNum = text::DefaultNumberingProvider::create(xContext);

    pImpl->xInfo = uno::Reference<text::XNumberingTypeInfo>(xDefNum, uno::UNO_QUERY);
    Reload(nTypeFlags);
}

SwNumberingTypeListBox::SwNumberingTypeListBox( Window* pWin, WinBits nStyle ) :
    ListBox(pWin, nStyle),
    pImpl(new SwNumberingTypeListBox_Impl)
{
    uno::Reference<uno::XComponentContext>          xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference<text::XDefaultNumberingProvider> xDefNum = text::DefaultNumberingProvider::create(xContext);

    pImpl->xInfo = uno::Reference<text::XNumberingTypeInfo>(xDefNum, uno::UNO_QUERY);
}

bool SwNumberingTypeListBox::set_property(const rtl::OString &rKey, const rtl::OString &rValue)
{
    if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("type")))
        Reload(rValue.toInt32());
    else
        return ListBox::set_property(rKey, rValue);
    return true;
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSwNumberingTypeListBox(Window *pParent, VclBuilder::stringmap &)
{
    SwNumberingTypeListBox *pListBox = new SwNumberingTypeListBox(pParent, WB_LEFT|WB_DROPDOWN|WB_VCENTER|WB_3DLOOK);
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
