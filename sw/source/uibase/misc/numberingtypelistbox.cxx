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
#include <vcl/builderfactory.hxx>
#include <editeng/numitem.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <svx/strarray.hxx>

#include <unomid.h>

using namespace com::sun::star;

struct SwNumberingTypeListBox_Impl
{
    uno::Reference<text::XNumberingTypeInfo> xInfo;
};

SwNumberingTypeListBox::SwNumberingTypeListBox( vcl::Window* pWin, WinBits nStyle ) :
    ListBox(pWin, nStyle),
    pImpl(new SwNumberingTypeListBox_Impl)
{
    uno::Reference<uno::XComponentContext>          xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference<text::XDefaultNumberingProvider> xDefNum = text::DefaultNumberingProvider::create(xContext);

    pImpl->xInfo.set(xDefNum, uno::UNO_QUERY);
}

bool SwNumberingTypeListBox::set_property(const OString &rKey, const OUString &rValue)
{
    if (rKey == "type")
        Reload(static_cast<SwInsertNumTypes>(rValue.toInt32()));
    else
        return ListBox::set_property(rKey, rValue);
    return true;
}

VCL_BUILDER_DECL_FACTORY(SwNumberingTypeListBox)
{
    (void)rMap;
    VclPtrInstance<SwNumberingTypeListBox> pListBox(pParent, WB_LEFT|WB_DROPDOWN|WB_VCENTER|WB_3DLOOK|WB_TABSTOP);
    pListBox->EnableAutoSize(true);
    rRet = pListBox;
}

SwNumberingTypeListBox::~SwNumberingTypeListBox()
{
    disposeOnce();
}

void SwNumberingTypeListBox::dispose()
{
    pImpl.reset();
    ListBox::dispose();
}

void SwNumberingTypeListBox::Reload(SwInsertNumTypes nTypeFlags)
{
    Clear();
    uno::Sequence<sal_Int16> aTypes;
    const sal_Int16* pTypes = nullptr;
    if(nTypeFlags & SwInsertNumTypes::Extended)
    {
        if(pImpl->xInfo.is())
        {
            aTypes = pImpl->xInfo->getSupportedNumberingTypes();
            pTypes = aTypes.getConstArray();
        }
    }

    for(size_t i = 0; i < SvxNumberingTypeTable::Count(); i++)
    {
        sal_IntPtr nValue = SvxNumberingTypeTable::GetValue(i);
        bool bInsert = true;
        sal_Int32 nPos = LISTBOX_APPEND;
        switch(nValue)
        {
            case  style::NumberingType::NUMBER_NONE:
                bInsert = bool(nTypeFlags & SwInsertNumTypes::NoNumbering);
                nPos = 0;

                break;
            case  style::NumberingType::CHAR_SPECIAL:
                bInsert = bool(nTypeFlags & SwInsertNumTypes::Bullet);

                break;
            case  style::NumberingType::PAGE_DESCRIPTOR:
                bInsert = bool(nTypeFlags & SwInsertNumTypes::PageStyleNumbering);

                break;
            case  style::NumberingType::BITMAP:
                bInsert = bool(nTypeFlags & SwInsertNumTypes::Bitmap );

                break;
            case  style::NumberingType::BITMAP | LINK_TOKEN:
                bInsert = false;

                break;
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
            sal_Int32 nEntry = InsertEntry(SvxNumberingTypeTable::GetString(i), nPos);
            SetEntryData( nEntry, reinterpret_cast<void*>(nValue) );
        }
    }
    if(nTypeFlags & SwInsertNumTypes::Extended)
    {
        if(pTypes)
        {
            for(sal_Int32 nType = 0; nType < aTypes.getLength(); nType++)
            {
                sal_Int16 nCurrent = pTypes[nType];
                if(nCurrent > style::NumberingType::CHARS_LOWER_LETTER_N)
                {
                    if(LISTBOX_ENTRY_NOTFOUND == GetEntryPos(reinterpret_cast<void*>((sal_uLong)nCurrent)))
                    {
                        OUString aIdent = pImpl->xInfo->getNumberingIdentifier( nCurrent );
                        sal_Int32 nPos = InsertEntry(aIdent);
                        SetEntryData(nPos, reinterpret_cast<void*>((sal_uLong)nCurrent));
                    }
                }
            }
        }
        SelectEntryPos(0);
    }
}

SvxNumType   SwNumberingTypeListBox::GetSelectedNumberingType()
{
    SvxNumType nRet = SVX_NUM_CHARS_UPPER_LETTER;
    sal_Int32 nSelPos = GetSelectEntryPos();
    if(LISTBOX_ENTRY_NOTFOUND != nSelPos)
        nRet = (SvxNumType)reinterpret_cast<sal_uLong>(GetEntryData(nSelPos));
#if OSL_DEBUG_LEVEL > 0
    else
        OSL_FAIL("SwNumberingTypeListBox not selected");
#endif
    return nRet;
}

bool    SwNumberingTypeListBox::SelectNumberingType(SvxNumType nType)
{
    sal_Int32 nPos = GetEntryPos(reinterpret_cast<void*>((sal_uLong)nType));
    SelectEntryPos( nPos );
    return LISTBOX_ENTRY_NOTFOUND != nPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
