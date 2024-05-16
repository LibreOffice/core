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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <optional>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

namespace {
    constexpr OUString s_sItemList = u"ItemList"_ustr;
    constexpr OUString s_sOrderList = u"OrderList"_ustr;
    constexpr OUString s_sHistoryItemRef = u"HistoryItemRef"_ustr;
    constexpr OUString s_sFilter = u"Filter"_ustr;
    constexpr OUString s_sTitle = u"Title"_ustr;
    constexpr OUString s_sThumbnail = u"Thumbnail"_ustr;
    constexpr OUString s_sReadOnly = u"ReadOnly"_ustr;
    constexpr OUString s_sPinned = u"Pinned"_ustr;
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

static void PrependItem(const uno::Reference<container::XNameAccess>& xCfg,
                        uno::Reference<container::XNameContainer>& xList, std::u16string_view sURL);
static void MoveItemToUnpinned(const uno::Reference<container::XNameAccess>& xCfg,
                               uno::Reference<container::XNameContainer>& xOrderList,
                               uno::Reference<container::XNameContainer>& xItemList,
                               std::u16string_view sURL);

static sal_uInt32 GetCapacity(const uno::Reference<container::XNameAccess>& xCommonXCU, EHistoryType eHistory);

namespace SvtHistoryOptions
{

void Clear(EHistoryType eHistory, const bool bClearPinnedItems)
{
    try
    {
        uno::Reference<container::XNameAccess> xCfg = GetConfig();
        uno::Reference<container::XNameAccess> xListAccess(GetListAccess(xCfg, eHistory));

        // Retrieve order and item lists using name access to check properties of individual items
        uno::Reference<container::XNameAccess> xItemList;
        uno::Reference<container::XNameAccess> xOrderList;
        xListAccess->getByName(s_sItemList)  >>= xItemList;
        xListAccess->getByName(s_sOrderList) >>= xOrderList;

        // Retrieve order and item lists using name container to delete individual items
        uno::Reference<container::XNameContainer> xOrderNode;
        uno::Reference<container::XNameContainer> xItemNode;
        xListAccess->getByName(s_sItemList) >>= xItemNode;
        xListAccess->getByName(s_sOrderList) >>= xOrderNode;

        const sal_Int32 nLength = xOrderList->getElementNames().getLength();
        for (sal_Int32 nItem = 0; nItem < nLength; ++nItem)
        {
            // Retrieve single item from the order list
            OUString sUrl;
            uno::Reference<beans::XPropertySet> xSet;
            xOrderList->getByName(OUString::number(nItem)) >>= xSet;
            xSet->getPropertyValue(s_sHistoryItemRef) >>= sUrl;

            // tdf#155698 - check if pinned items should be cleared
            bool bIsItemPinned = false;
            if (!bClearPinnedItems)
            {
                xItemList->getByName(sUrl) >>= xSet;
                if (xSet->getPropertySetInfo()->hasPropertyByName(s_sPinned))
                    xSet->getPropertyValue(s_sPinned) >>= bIsItemPinned;
            }

            if (!bIsItemPinned)
            {
                xItemNode->removeByName(sUrl);
                xOrderNode->removeByName(OUString::number(nItem));
            }
        }

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
                xSet->getPropertyValue(s_sThumbnail) >>= aItem.sThumbnail;
                xSet->getPropertyValue(s_sReadOnly) >>= aItem.isReadOnly;
                if (xSet->getPropertySetInfo()->hasPropertyByName(s_sPinned))
                    xSet->getPropertyValue(s_sPinned) >>= aItem.isPinned;

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

void AppendItem(EHistoryType eHistory, const OUString& sURL, const OUString& sFilter,
                const OUString& sTitle, const std::optional<OUString>& sThumbnail,
                ::std::optional<bool> const oIsReadOnly)
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
            xItemList->getByName(sURL) >>= xSet;
            if (sThumbnail)
            {
                // update the thumbnail
                xSet->setPropertyValue(s_sThumbnail, uno::Any(*sThumbnail));
            }
            if (oIsReadOnly)
            {
                xSet->setPropertyValue(s_sReadOnly, uno::Any(*oIsReadOnly));
            }

            // tdf#38742 - check the current pinned state of the item and move it accordingly
            bool bIsItemPinned = false;
            if (xSet->getPropertySetInfo()->hasPropertyByName(s_sPinned))
                xSet->getPropertyValue(s_sPinned) >>= bIsItemPinned;
            if (bIsItemPinned)
                PrependItem(xCfg, xOrderList, sURL);
            else
                MoveItemToUnpinned(xCfg, xOrderList, xItemList, sURL);
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
                xOrderList->insertByName(sPush, uno::Any(xInst));
            }
            for (sal_Int32 j=nLength-1; j>0; --j)
            {
                xOrderList->getByName( OUString::number(j) )   >>= xPrevSet;
                xOrderList->getByName( OUString::number(j-1) ) >>= xNextSet;
                OUString sTemp;
                xNextSet->getPropertyValue(s_sHistoryItemRef) >>= sTemp;
                xPrevSet->setPropertyValue(s_sHistoryItemRef, uno::Any(sTemp));
            }
            xOrderList->getByName( OUString::number(0) ) >>= xSet;
            xSet->setPropertyValue(s_sHistoryItemRef, uno::Any(sURL));

            // Append the item to ItemList.
            xFac.set(xItemList, uno::UNO_QUERY);
            xInst = xFac->createInstance();
            xItemList->insertByName(sURL, uno::Any(xInst));

            xSet.set(xInst, uno::UNO_QUERY);
            xSet->setPropertyValue(s_sFilter, uno::Any(sFilter));
            xSet->setPropertyValue(s_sTitle, uno::Any(sTitle));
            xSet->setPropertyValue(s_sThumbnail, uno::Any(sThumbnail.value_or(OUString())));
            if (oIsReadOnly)
            {
                xSet->setPropertyValue(s_sReadOnly, uno::Any(*oIsReadOnly));
            }

            // tdf#38742 - check the current pinned state of the item and move it accordingly
            bool bIsItemPinned = false;
            if (xSet->getPropertySetInfo()->hasPropertyByName(s_sPinned))
                xSet->getPropertyValue(s_sPinned) >>= bIsItemPinned;
            if (bIsItemPinned)
                PrependItem(xCfg, xOrderList, sURL);
            else
                MoveItemToUnpinned(xCfg, xOrderList, xItemList, sURL);

            ::comphelper::ConfigurationHelper::flush(xCfg);
        }
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("unotools.config");
    }
}

void DeleteItem(EHistoryType eHistory, const OUString& sURL, const bool bClearPinned)
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

        // it's the last one and pinned items can be cleared, just clear the lists
        if (nLength == 1 && bClearPinned)
        {
            Clear(eHistory);
            return;
        }

        // find it in the OrderList
        sal_Int32 nFromWhere = 0;
        bool bIsItemPinned = false;
        for (; nFromWhere < nLength - 1; ++nFromWhere)
        {
            uno::Reference<beans::XPropertySet>       xSet;
            OUString aItem;
            xOrderList->getByName(OUString::number(nFromWhere)) >>= xSet;
            xSet->getPropertyValue(s_sHistoryItemRef) >>= aItem;
            // tdf#155698 - check if pinned item should be deleted
            if (!bClearPinned && xSet->getPropertySetInfo()->hasPropertyByName(s_sPinned))
                xSet->getPropertyValue(s_sPinned) >>= bIsItemPinned;

            if (aItem == sURL)
                break;
        }

        // tdf#155698 - check if pinned item should be deleted
        if (!bIsItemPinned)
        {
            // and shift the rest of the items in OrderList accordingly
            for (sal_Int32 i = nFromWhere; i < nLength - 1; ++i)
            {
                uno::Reference<beans::XPropertySet> xPrevSet;
                uno::Reference<beans::XPropertySet> xNextSet;
                xOrderList->getByName(OUString::number(i))     >>= xPrevSet;
                xOrderList->getByName(OUString::number(i + 1)) >>= xNextSet;

                OUString sTemp;
                xNextSet->getPropertyValue(s_sHistoryItemRef) >>= sTemp;
                xPrevSet->setPropertyValue(s_sHistoryItemRef, uno::Any(sTemp));
            }
            xOrderList->removeByName(OUString::number(nLength - 1));

            // and finally remove it from the ItemList
            xItemList->removeByName(sURL);

            ::comphelper::ConfigurationHelper::flush(xCfg);
        }
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("unotools.config");
    }
}

void TogglePinItem(EHistoryType eHistory, const OUString& sURL)
{
    try
    {
        uno::Reference<container::XNameAccess> xCfg = GetConfig();
        uno::Reference<container::XNameAccess> xListAccess(GetListAccess(xCfg, eHistory));

        uno::Reference<container::XNameContainer> xItemList;
        xListAccess->getByName(s_sItemList)  >>= xItemList;

        // Check if item exists
        if (xItemList->hasByName(sURL))
        {
            // Toggle pinned option
            uno::Reference<beans::XPropertySet> xSet;
            xItemList->getByName(sURL) >>= xSet;
            bool bIsItemPinned = false;
            if (xSet->getPropertySetInfo()->hasPropertyByName(s_sPinned))
                xSet->getPropertyValue(s_sPinned) >>= bIsItemPinned;
            xSet->setPropertyValue(s_sPinned, uno::Any(!bIsItemPinned));

            uno::Reference<container::XNameContainer> xOrderList;
            xListAccess->getByName(s_sOrderList) >>= xOrderList;

            // Shift item to the beginning of the document list if is not pinned now
            if (bIsItemPinned)
                MoveItemToUnpinned(xCfg, xOrderList, xItemList, sURL);
            else
                PrependItem(xCfg, xOrderList, sURL);
        }
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
                u"org.openoffice.Office.Histories/Histories"_ustr,
                ::comphelper::EConfigurationModes::Standard),
            uno::UNO_QUERY_THROW);
}

static uno::Reference<container::XNameAccess> GetCommonXCU()
{
    return uno::Reference<container::XNameAccess>(
            ::comphelper::ConfigurationHelper::openConfig(
                ::comphelper::getProcessComponentContext(),
                u"org.openoffice.Office.Common/History"_ustr,
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
        xCfg->getByName(u"PickList"_ustr) >>= xListAccess;
        break;

    case EHistoryType::HelpBookmarks:
        xCfg->getByName(u"HelpBookmarks"_ustr) >>= xListAccess;
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

static void PrependItem(const uno::Reference<container::XNameAccess>& xCfg,
                        uno::Reference<container::XNameContainer>& xList, std::u16string_view sURL)
{
    uno::Reference<beans::XPropertySet> xSet;
    const sal_Int32 nLength = xList->getElementNames().getLength();
    for (sal_Int32 i = 0; i < nLength; i++)
    {
        OUString aItem;
        xList->getByName(OUString::number(i)) >>= xSet;
        xSet->getPropertyValue(s_sHistoryItemRef) >>= aItem;

        if (aItem == sURL)
        {
            for (sal_Int32 j = i - 1; j >= 0; --j)
            {
                uno::Reference<beans::XPropertySet> xPrevSet;
                uno::Reference<beans::XPropertySet> xNextSet;
                xList->getByName(OUString::number(j + 1)) >>= xPrevSet;
                xList->getByName(OUString::number(j)) >>= xNextSet;

                OUString sTemp;
                xNextSet->getPropertyValue(s_sHistoryItemRef) >>= sTemp;
                xPrevSet->setPropertyValue(s_sHistoryItemRef, uno::Any(sTemp));
            }
            xList->getByName(OUString::number(0)) >>= xSet;
            xSet->setPropertyValue(s_sHistoryItemRef, uno::Any(aItem));
            ::comphelper::ConfigurationHelper::flush(xCfg);
            return;
        }
    }
}

static void MoveItemToUnpinned(const uno::Reference<container::XNameAccess>& xCfg,
                               uno::Reference<container::XNameContainer>& xOrderList,
                               uno::Reference<container::XNameContainer>& xItemList,
                               std::u16string_view sURL)
{
    uno::Reference<beans::XPropertySet> xSet;
    const sal_Int32 nLength = xOrderList->getElementNames().getLength();
    // Search for item in the ordered list list
    for (sal_Int32 i = 0; i < nLength; i++)
    {
        OUString aItem;
        xOrderList->getByName(OUString::number(i)) >>= xSet;
        xSet->getPropertyValue(s_sHistoryItemRef) >>= aItem;

        if (aItem == sURL)
        {
            // Move item to the unpinned document section to the right if it was previously pinned
            for (sal_Int32 j = i + 1; j < nLength; j++)
            {
                uno::Reference<beans::XPropertySet> xNextSet;
                xOrderList->getByName(OUString::number(j)) >>= xNextSet;

                OUString aNextItem;
                xNextSet->getPropertyValue(s_sHistoryItemRef) >>= aNextItem;

                uno::Reference<beans::XPropertySet> xNextItemSet;
                xItemList->getByName(aNextItem) >>= xNextItemSet;
                bool bIsItemPinned = false;
                if (xNextItemSet->getPropertySetInfo()->hasPropertyByName(s_sPinned))
                    xNextItemSet->getPropertyValue(s_sPinned) >>= bIsItemPinned;
                if (bIsItemPinned)
                {
                    xOrderList->getByName(OUString::number(j - 1)) >>= xSet;
                    xSet->getPropertyValue(s_sHistoryItemRef) >>= aItem;
                    xSet->setPropertyValue(s_sHistoryItemRef, uno::Any(aNextItem));
                    xNextSet->setPropertyValue(s_sHistoryItemRef, uno::Any(aItem));
                }
                else
                    break;
            }

            // Move item to the unpinned document section to the left if it was previously unpinned
            for (sal_Int32 j = i - 1; j >= 0; --j)
            {
                uno::Reference<beans::XPropertySet> xPrevSet;
                xOrderList->getByName(OUString::number(j)) >>= xPrevSet;

                OUString aPrevItem;
                xPrevSet->getPropertyValue(s_sHistoryItemRef) >>= aPrevItem;

                uno::Reference<beans::XPropertySet> xPrevItemSet;
                xItemList->getByName(aPrevItem) >>= xPrevItemSet;
                bool bIsItemPinned = false;
                if (xPrevItemSet->getPropertySetInfo()->hasPropertyByName(s_sPinned))
                    xPrevItemSet->getPropertyValue(s_sPinned) >>= bIsItemPinned;
                if (!bIsItemPinned)
                {
                    xOrderList->getByName(OUString::number(j + 1)) >>= xSet;
                    xSet->getPropertyValue(s_sHistoryItemRef) >>= aItem;
                    xSet->setPropertyValue(s_sHistoryItemRef, uno::Any(aPrevItem));
                    xPrevSet->setPropertyValue(s_sHistoryItemRef, uno::Any(aItem));
                }
                else
                    break;
            }

            ::comphelper::ConfigurationHelper::flush(xCfg);
            return;
        }
    }
}

static sal_uInt32 GetCapacity(const uno::Reference<container::XNameAccess>& xCommonXCU, EHistoryType eHistory)
{
    uno::Reference<beans::XPropertySet> xListAccess(xCommonXCU, uno::UNO_QUERY_THROW);

    sal_uInt32 nSize = 0;

    switch (eHistory)
    {
    case EHistoryType::PickList:
        xListAccess->getPropertyValue(u"PickListSize"_ustr) >>= nSize;
        break;

    case EHistoryType::HelpBookmarks:
        xListAccess->getPropertyValue(u"HelpBookmarkSize"_ustr) >>= nSize;
        break;
    }

    return nSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
