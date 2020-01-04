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

#include <svx/SvxNumOptionsTabPageHelper.hxx>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/text/DefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingTypeInfo.hpp>
#include <comphelper/processfactory.hxx>

using namespace css;
using namespace css::uno;
using namespace css::text;
using namespace css::style;

Reference<XDefaultNumberingProvider> SvxNumOptionsTabPageHelper::GetNumberingProvider()
{
    Reference<XComponentContext>         xContext( ::comphelper::getProcessComponentContext() );
    Reference<XDefaultNumberingProvider> xRet = text::DefaultNumberingProvider::create(xContext);
    return xRet;
}

void SvxNumOptionsTabPageHelper::GetI18nNumbering( ListBox& rFmtLB, sal_uInt16 nDoNotRemove )
{
    Reference<XDefaultNumberingProvider> xDefNum = GetNumberingProvider();
    Reference<XNumberingTypeInfo> xInfo(xDefNum, UNO_QUERY);

    // Extended numbering schemes present in the resource but not offered by
    // the i18n framework per configuration must be removed from the listbox.
    // Do not remove a special entry matching nDoNotRemove.
    const sal_uInt16 nDontRemove = SAL_MAX_UINT16;
    ::std::vector< sal_uInt16> aRemove( rFmtLB.GetEntryCount(), nDontRemove);
    for (size_t i=0; i<aRemove.size(); ++i)
    {
        sal_uInt16 nEntryData = static_cast<sal_uInt16>(reinterpret_cast<sal_uLong>(rFmtLB.GetEntryData(
                sal::static_int_cast< sal_Int32 >(i))));
        if (nEntryData > NumberingType::CHARS_LOWER_LETTER_N && nEntryData != nDoNotRemove)
            aRemove[i] = nEntryData;
    }
    if(xInfo.is())
    {
        Sequence<sal_Int16> aTypes = xInfo->getSupportedNumberingTypes(  );
        for(const sal_Int16 nCurrent : aTypes)
        {
            if(nCurrent > NumberingType::CHARS_LOWER_LETTER_N)
            {
                bool bInsert = true;
                for(sal_Int32 nEntry = 0; nEntry < rFmtLB.GetEntryCount(); nEntry++)
                {
                    sal_uInt16 nEntryData = static_cast<sal_uInt16>(reinterpret_cast<sal_uLong>(rFmtLB.GetEntryData(nEntry)));
                    if(nEntryData == static_cast<sal_uInt16>(nCurrent))
                    {
                        bInsert = false;
                        aRemove[nEntry] = nDontRemove;
                        break;
                    }
                }
                if(bInsert)
                {
                    OUString aIdent = xInfo->getNumberingIdentifier( nCurrent );
                    sal_Int32 nPos = rFmtLB.InsertEntry(aIdent);
                    rFmtLB.SetEntryData(nPos, reinterpret_cast<void*>(static_cast<sal_uLong>(nCurrent)));
                }
            }
        }
    }
    for (unsigned short i : aRemove)
    {
        if (i != nDontRemove)
        {
            sal_Int32 nPos = rFmtLB.GetEntryPos( reinterpret_cast<void*>(static_cast<sal_uLong>(i)));
            rFmtLB.RemoveEntry( nPos);
        }
    }
}

void SvxNumOptionsTabPageHelper::GetI18nNumbering(weld::ComboBox& rFmtLB, sal_uInt16 nDoNotRemove)
{
    Reference<XDefaultNumberingProvider> xDefNum = GetNumberingProvider();
    Reference<XNumberingTypeInfo> xInfo(xDefNum, UNO_QUERY);

    // Extended numbering schemes present in the resource but not offered by
    // the i18n framework per configuration must be removed from the listbox.
    // Do not remove a special entry matching nDoNotRemove.
    const sal_uInt16 nDontRemove = SAL_MAX_UINT16;
    ::std::vector< sal_uInt16> aRemove( rFmtLB.get_count(), nDontRemove);
    for (size_t i=0; i<aRemove.size(); ++i)
    {
        sal_uInt16 nEntryData = rFmtLB.get_id(i).toInt32();
        if (nEntryData > NumberingType::CHARS_LOWER_LETTER_N && nEntryData != nDoNotRemove)
            aRemove[i] = nEntryData;
    }
    if(xInfo.is())
    {
        Sequence<sal_Int16> aTypes = xInfo->getSupportedNumberingTypes(  );
        for(const sal_Int16 nCurrent : aTypes)
        {
            if(nCurrent > NumberingType::CHARS_LOWER_LETTER_N)
            {
                bool bInsert = true;
                for (int nEntry = 0; nEntry < rFmtLB.get_count(); ++nEntry)
                {
                    sal_uInt16 nEntryData = rFmtLB.get_id(nEntry).toInt32();
                    if (nEntryData == static_cast<sal_uInt16>(nCurrent))
                    {
                        bInsert = false;
                        aRemove[nEntry] = nDontRemove;
                        break;
                    }
                }
                if(bInsert)
                {
                    OUString aIdent = xInfo->getNumberingIdentifier( nCurrent );
                    rFmtLB.append(OUString::number(nCurrent), aIdent);
                }
            }
        }
    }
    for (unsigned short i : aRemove)
    {
        if (i == nDontRemove)
            continue;
        rFmtLB.remove_id(OUString::number(i));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
