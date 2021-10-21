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

#include <unotools/historyoptions.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cassert>
#include <algorithm>

#include "itemholder1.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>
#include <optional>

using namespace ::utl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

namespace {
    constexpr OUStringLiteral s_sItemList = u"ItemList";
    constexpr OUStringLiteral s_sOrderList = u"OrderList";
    constexpr OUStringLiteral s_sHistoryItemRef = u"HistoryItemRef";
    constexpr OUStringLiteral s_sFilter = u"Filter";
    constexpr OUStringLiteral s_sTitle = u"Title";
    constexpr OUStringLiteral s_sPassword = u"Password";
    constexpr OUStringLiteral s_sThumbnail = u"Thumbnail";
}

static uno::Reference<container::XNameAccess> GetConfig();
static uno::Reference<container::XNameAccess> GetCommonXCU();
static uno::Reference<container::XNameAccess> GetListAccess(
        uno::Reference<container::XNameAccess> const & xCfg,
        EHistoryType eHistory);
static void TruncateList(
        const uno::Reference<container::XNameAccess>& xCfg,
        const uno::Reference<container::XNameAccess>& xList,
        sal_uInt32 nSize);
static sal_uInt32 GetCapacity(const uno::Reference<container::XNameAccess>& xCommonXCU, EHistoryType eHistory);

namespace SvtHistoryOptions
{

void Clear( EHistoryType eHistory )
{
    try
    {
        uno::Reference<container::XNameAccess> xCfg = GetConfig();
        uno::Reference<container::XNameAccess> xListAccess(GetListAccess(xCfg, eHistory));

        // clear ItemList
        uno::Reference<container::XNameContainer> xNode;
        xListAccess->getByName(s_sItemList) >>= xNode;
        Sequence<OUString> aStrings(xNode->getElementNames());

        for (const auto& rString : std::as_const(aStrings))
            xNode->removeByName(rString);

        // clear OrderList
        xListAccess->getByName(s_sOrderList) >>= xNode;
        aStrings = xNode->getElementNames();

        for (const auto& rString : std::as_const(aStrings))
            xNode->removeByName(rString);

        ::comphelper::ConfigurationHelper::flush(xCfg);
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("unotools.config");
    }
}

std::vector< HistoryItem > GetList( EHistoryType eHistory )
{
    std::vector< HistoryItem > aRet;
    try
    {
        uno::Reference<container::XNameAccess> xCfg = GetConfig();
        uno::Reference<container::XNameAccess> xCommonXCU = GetCommonXCU();
        uno::Reference<container::XNameAccess> xListAccess(GetListAccess(xCfg, eHistory));

        TruncateList(xCfg, xListAccess, GetCapacity(xCommonXCU, eHistory));

        uno::Reference<container::XNameAccess> xItemList;
        uno::Reference<container::XNameAccess> xOrderList;
        xListAccess->getByName(s_sItemList)  >>= xItemList;
        xListAccess->getByName(s_sOrderList) >>= xOrderList;

        const sal_Int32 nLength = xOrderList->getElementNames().getLength();
        aRet.reserve(nLength);

        for (sal_Int32 nItem = 0; nItem < nLength; ++nItem)
        {
            try
            {
                OUString sUrl;
                uno::Reference<beans::XPropertySet> xSet;
                xOrderList->getByName(OUString::number(nItem)) >>= xSet;
                xSet->getPropertyValue(s_sHistoryItemRef) >>= sUrl;

                xItemList->getByName(sUrl) >>= xSet;
                HistoryItem aItem;
                aItem.sURL = sUrl;
                xSet->getPropertyValue(s_sFilter) >>= aItem.sFilter;
                xSet->getPropertyValue(s_sTitle) >>= aItem.sTitle;
                xSet->getPropertyValue(s_sPassword) >>= aItem.sPassword;
                xSet->getPropertyValue(s_sThumbnail) >>= aItem.sThumbnail;
                aRet.push_back(aItem);
            }
            catch(const uno::Exception&)
            {
                // <https://bugs.libreoffice.org/show_bug.cgi?id=46074>
                // "FILEOPEN: No Recent Documents..." discusses a problem
                // with corrupted /org.openoffice.Office/Histories/Histories
                // configuration items; to work around that problem, simply
                // ignore such corrupted individual items here, so that at
                // least newly added items are successfully reported back
                // from this function:
                DBG_UNHANDLED_EXCEPTION("unotools.config");
            }
        }
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("unotools.config");
    }
    return aRet;
}

void AppendItem(EHistoryType eHistory,
        const OUString& sURL, const OUString& sFilter, const OUString& sTitle,
        const std::optional<OUString>& sThumbnail)
{
    try
    {
        uno::Reference<container::XNameAccess> xCfg = GetConfig();
        uno::Reference<container::XNameAccess> xCommonXCU = GetCommonXCU();
        uno::Reference<container::XNameAccess> xListAccess(GetListAccess(xCfg, eHistory));

        TruncateList(xCfg, xListAccess, GetCapacity(xCommonXCU, eHistory));

        sal_Int32 nMaxSize = GetCapacity(xCommonXCU, eHistory);
        if (nMaxSize == 0)
            return;

        uno::Reference<container::XNameContainer> xItemList;
        uno::Reference<container::XNameContainer> xOrderList;
        xListAccess->getByName(s_sItemList)  >>= xItemList;
        xListAccess->getByName(s_sOrderList) >>= xOrderList;
        sal_Int32 nLength = xOrderList->getElementNames().getLength();

        // The item to be appended already exists
        if (xItemList->hasByName(sURL))
        {
            uno::Reference<beans::XPropertySet>       xSet;
            if (sThumbnail)
            {
                // update the thumbnail
                xItemList->getByName(sURL) >>= xSet;
                xSet->setPropertyValue(s_sThumbnail, uno::makeAny(*sThumbnail));
            }

            for (sal_Int32 i=0; i<nLength; ++i)
            {
                OUString aItem;
                xOrderList->getByName(OUString::number(i)) >>= xSet;
                xSet->getPropertyValue(s_sHistoryItemRef) >>= aItem;

                if (aItem == sURL)
                {
                    for (sal_Int32 j = i - 1; j >= 0; --j)
                    {
                        uno::Reference<beans::XPropertySet> xPrevSet;
                        uno::Reference<beans::XPropertySet> xNextSet;
                        xOrderList->getByName(OUString::number(j+1)) >>= xPrevSet;
                        xOrderList->getByName(OUString::number(j))   >>= xNextSet;

                        OUString sTemp;
                        xNextSet->getPropertyValue(s_sHistoryItemRef) >>= sTemp;
                        xPrevSet->setPropertyValue(s_sHistoryItemRef, uno::makeAny(sTemp));
                    }
                    xOrderList->getByName(OUString::number(0)) >>= xSet;
                    xSet->setPropertyValue(s_sHistoryItemRef, uno::makeAny(aItem));
                    break;
                }
            }

            ::comphelper::ConfigurationHelper::flush(xCfg);
        }
        else // The item to be appended does not exist yet
        {
            uno::Reference<beans::XPropertySet>       xSet;
            uno::Reference<lang::XSingleServiceFactory> xFac;
            uno::Reference<uno::XInterface>             xInst;
            uno::Reference<beans::XPropertySet> xPrevSet;
            uno::Reference<beans::XPropertySet> xNextSet;

            // Append new item to OrderList.
            if ( nLength == nMaxSize )
            {
                OUString sRemove;
                xOrderList->getByName(OUString::number(nLength-1)) >>= xSet;
                xSet->getPropertyValue(s_sHistoryItemRef) >>= sRemove;
                try
                {
                    xItemList->removeByName(sRemove);
                }
                catch (container::NoSuchElementException &)
                {
                    // <https://bugs.libreoffice.org/show_bug.cgi?id=46074>
                    // "FILEOPEN: No Recent Documents..." discusses a problem
                    // with corrupted /org.openoffice.Office/Histories/Histories
                    // configuration items; to work around that problem, simply
                    // ignore such corrupted individual items here, so that at
                    // least newly added items are successfully added:
                    if (!sRemove.isEmpty())
                    {
                        throw;
                    }
                }
            }
            if (nLength != nMaxSize)
            {
                xFac.set(xOrderList, uno::UNO_QUERY);
                xInst = xFac->createInstance();
                OUString sPush = OUString::number(nLength++);
                xOrderList->insertByName(sPush, uno::makeAny(xInst));
            }
            for (sal_Int32 j=nLength-1; j>0; --j)
            {
                xOrderList->getByName( OUString::number(j) )   >>= xPrevSet;
                xOrderList->getByName( OUString::number(j-1) ) >>= xNextSet;
                OUString sTemp;
                xNextSet->getPropertyValue(s_sHistoryItemRef) >>= sTemp;
                xPrevSet->setPropertyValue(s_sHistoryItemRef, uno::makeAny(sTemp));
            }
            xOrderList->getByName( OUString::number(0) ) >>= xSet;
            xSet->setPropertyValue(s_sHistoryItemRef, uno::makeAny(sURL));

            // Append the item to ItemList.
            xFac.set(xItemList, uno::UNO_QUERY);
            xInst = xFac->createInstance();
            xItemList->insertByName(sURL, uno::makeAny(xInst));

            xSet.set(xInst, uno::UNO_QUERY);
            xSet->setPropertyValue(s_sFilter, uno::makeAny(sFilter));
            xSet->setPropertyValue(s_sTitle, uno::makeAny(sTitle));
            xSet->setPropertyValue(s_sPassword, uno::makeAny(OUString()));
            xSet->setPropertyValue(s_sThumbnail, uno::makeAny(sThumbnail.value_or(OUString())));

            ::comphelper::ConfigurationHelper::flush(xCfg);
        }
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("unotools.config");
    }
}

void DeleteItem(EHistoryType eHistory, const OUString& sURL)
{
    try
    {
        uno::Reference<container::XNameAccess> xCfg = GetConfig();
        uno::Reference<container::XNameAccess> xListAccess(GetListAccess(xCfg, eHistory));

        uno::Reference<container::XNameContainer> xItemList;
        uno::Reference<container::XNameContainer> xOrderList;
        xListAccess->getByName(s_sItemList)  >>= xItemList;
        xListAccess->getByName(s_sOrderList) >>= xOrderList;
        sal_Int32 nLength = xOrderList->getElementNames().getLength();

        // if it does not exist, nothing to do
        if (!xItemList->hasByName(sURL))
            return;

        // it's the last one, just clear the lists
        if (nLength == 1)
        {
            Clear(eHistory);
            return;
        }

        // find it in the OrderList
        sal_Int32 nFromWhere = 0;
        for (; nFromWhere < nLength - 1; ++nFromWhere)
        {
            uno::Reference<beans::XPropertySet>       xSet;
            OUString aItem;
            xOrderList->getByName(OUString::number(nFromWhere)) >>= xSet;
            xSet->getPropertyValue(s_sHistoryItemRef) >>= aItem;

            if (aItem == sURL)
                break;
        }

        // and shift the rest of the items in OrderList accordingly
        for (sal_Int32 i = nFromWhere; i < nLength - 1; ++i)
        {
            uno::Reference<beans::XPropertySet> xPrevSet;
            uno::Reference<beans::XPropertySet> xNextSet;
            xOrderList->getByName(OUString::number(i))     >>= xPrevSet;
            xOrderList->getByName(OUString::number(i + 1)) >>= xNextSet;

            OUString sTemp;
            xNextSet->getPropertyValue(s_sHistoryItemRef) >>= sTemp;
            xPrevSet->setPropertyValue(s_sHistoryItemRef, uno::makeAny(sTemp));
        }
        xOrderList->removeByName(OUString::number(nLength - 1));

        // and finally remove it from the ItemList
        xItemList->removeByName(sURL);

        ::comphelper::ConfigurationHelper::flush(xCfg);
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("unotools.config");
    }
}

} // namespace


static uno::Reference<container::XNameAccess> GetConfig()
{
    return uno::Reference<container::XNameAccess>(
            ::comphelper::ConfigurationHelper::openConfig(
                ::comphelper::getProcessComponentContext(),
                "org.openoffice.Office.Histories/Histories",
                ::comphelper::EConfigurationModes::Standard),
            uno::UNO_QUERY_THROW);
}

static uno::Reference<container::XNameAccess> GetCommonXCU()
{
    return uno::Reference<container::XNameAccess>(
            ::comphelper::ConfigurationHelper::openConfig(
                ::comphelper::getProcessComponentContext(),
                "org.openoffice.Office.Common/History",
                ::comphelper::EConfigurationModes::Standard),
            uno::UNO_QUERY_THROW);
}

static uno::Reference<container::XNameAccess> GetListAccess(
    const uno::Reference<container::XNameAccess>& xCfg,
    EHistoryType eHistory)
{
    uno::Reference<container::XNameAccess> xListAccess;
    switch (eHistory)
    {
    case EHistoryType::PickList:
        xCfg->getByName("PickList") >>= xListAccess;
        break;

    case EHistoryType::HelpBookmarks:
        xCfg->getByName("HelpBookmarks") >>= xListAccess;
        break;
    }
    return xListAccess;
}

static void TruncateList(
    const uno::Reference<container::XNameAccess>& xCfg,
    const uno::Reference<container::XNameAccess>& xList,
    sal_uInt32 nSize)
{
    uno::Reference<container::XNameContainer> xItemList;
    uno::Reference<container::XNameContainer> xOrderList;
    xList->getByName(s_sOrderList) >>= xOrderList;
    xList->getByName(s_sItemList)  >>= xItemList;

    const sal_uInt32 nLength = xOrderList->getElementNames().getLength();
    if (nSize >= nLength)
        return;

    for (sal_uInt32 i=nLength-1; i>=nSize; --i)
    {
        uno::Reference<beans::XPropertySet>       xSet;
        OUString sTmp;
        const OUString sRemove = OUString::number(i);
        xOrderList->getByName(sRemove) >>= xSet;
        xSet->getPropertyValue(s_sHistoryItemRef) >>= sTmp;
        xItemList->removeByName(sTmp);
        xOrderList->removeByName(sRemove);
    }

    ::comphelper::ConfigurationHelper::flush(xCfg);
}



static sal_uInt32 GetCapacity(const uno::Reference<container::XNameAccess>& xCommonXCU, EHistoryType eHistory)
{
    uno::Reference<beans::XPropertySet> xListAccess(xCommonXCU, uno::UNO_QUERY_THROW);

    sal_uInt32 nSize = 0;

    switch (eHistory)
    {
    case EHistoryType::PickList:
        xListAccess->getPropertyValue("PickListSize") >>= nSize;
        break;

    case EHistoryType::HelpBookmarks:
        xListAccess->getPropertyValue("HelpBookmarkSize") >>= nSize;
        break;
    }

    return nSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
