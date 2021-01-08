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

using namespace ::std;
using namespace ::utl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

namespace {
    const ::sal_Int32 s_nOffsetURL               = 0;
    const ::sal_Int32 s_nOffsetFilter            = 1;
    const ::sal_Int32 s_nOffsetTitle             = 2;
    const ::sal_Int32 s_nOffsetPassword          = 3;
    const ::sal_Int32 s_nOffsetThumbnail         = 4;

    constexpr OUStringLiteral s_sItemList = u"ItemList";
    constexpr OUStringLiteral s_sOrderList = u"OrderList";
    constexpr OUStringLiteral s_sHistoryItemRef = u"HistoryItemRef";
    constexpr OUStringLiteral s_sFilter = u"Filter";
    constexpr OUStringLiteral s_sTitle = u"Title";
    constexpr OUStringLiteral s_sPassword = u"Password";
    constexpr OUStringLiteral s_sThumbnail = u"Thumbnail";

    class theHistoryOptionsMutex : public rtl::Static<osl::Mutex, theHistoryOptionsMutex>{};
}

/// Internal implementation of the SvtHistoryOptions.
class SvtHistoryOptions_Impl
{
public:
    SvtHistoryOptions_Impl();

    /// Returns the maximum size of the internal lists, ie. the capacity not the size.
    sal_uInt32 GetCapacity(EHistoryType eHistory) const;

    /// Clear the specified history list.
    void Clear(EHistoryType eHistory);

    /// Get a sequence list from the items.
    Sequence< Sequence<PropertyValue> > GetList(EHistoryType eHistory);

    void AppendItem(EHistoryType eHistory,
        const OUString& sURL, const OUString& sFilter, const OUString& sTitle,
        const std::optional<OUString>& sThumbnail);

    void DeleteItem(EHistoryType eHistory, const OUString& sURL);

private:
    /// Return the appropriate list of recent documents (based on eHistory).
    uno::Reference<container::XNameAccess> GetListAccess(EHistoryType eHistory) const;

    void impl_truncateList(EHistoryType eHistory, sal_uInt32 nSize);

private:
    uno::Reference<container::XNameAccess> m_xCfg;
    uno::Reference<container::XNameAccess> m_xCommonXCU;
};

SvtHistoryOptions_Impl::SvtHistoryOptions_Impl()
{
    try
    {
        m_xCfg.set(
            ::comphelper::ConfigurationHelper::openConfig(
                ::comphelper::getProcessComponentContext(),
                "org.openoffice.Office.Histories/Histories",
                ::comphelper::EConfigurationModes::Standard),
            uno::UNO_QUERY);

        m_xCommonXCU.set(
            ::comphelper::ConfigurationHelper::openConfig(
                ::comphelper::getProcessComponentContext(),
                "org.openoffice.Office.Common/History",
                ::comphelper::EConfigurationModes::Standard),
            uno::UNO_QUERY);
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("unotools.config");
        m_xCfg.clear();
        m_xCommonXCU.clear();
    }
}

sal_uInt32 SvtHistoryOptions_Impl::GetCapacity(EHistoryType eHistory) const
{
    uno::Reference<beans::XPropertySet> xListAccess(m_xCommonXCU, uno::UNO_QUERY);

    if (!xListAccess.is())
        return 0;

    sal_uInt32 nSize = 0;

    try
    {
        switch (eHistory)
        {
        case EHistoryType::PickList:
            xListAccess->getPropertyValue("PickListSize") >>= nSize;
            break;

        case EHistoryType::HelpBookmarks:
            xListAccess->getPropertyValue("HelpBookmarkSize") >>= nSize;
            break;

        default:
            break;
        }
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("unotools.config");
    }

    return nSize;
}

uno::Reference<container::XNameAccess> SvtHistoryOptions_Impl::GetListAccess(EHistoryType eHistory) const
{
    uno::Reference<container::XNameAccess> xListAccess;

    try
    {
        switch (eHistory)
        {
        case EHistoryType::PickList:
            m_xCfg->getByName("PickList") >>= xListAccess;
            break;

        case EHistoryType::HelpBookmarks:
            m_xCfg->getByName("HelpBookmarks") >>= xListAccess;
            break;

        default:
            break;
        }
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("unotools.config");
    }

    return xListAccess;
}

void SvtHistoryOptions_Impl::impl_truncateList(EHistoryType eHistory, sal_uInt32 nSize)
{
    uno::Reference<container::XNameAccess> xList(GetListAccess(eHistory));
    if (!xList.is())
        return;

    uno::Reference<container::XNameContainer> xItemList;
    uno::Reference<container::XNameContainer> xOrderList;
    uno::Reference<beans::XPropertySet>       xSet;

    try
    {
        xList->getByName(s_sOrderList) >>= xOrderList;
        xList->getByName(s_sItemList)  >>= xItemList;

        const sal_uInt32 nLength = xOrderList->getElementNames().getLength();
        if (nSize < nLength)
        {
            for (sal_uInt32 i=nLength-1; i>=nSize; --i)
            {
                OUString sTmp;
                const OUString sRemove = OUString::number(i);
                xOrderList->getByName(sRemove) >>= xSet;
                xSet->getPropertyValue(s_sHistoryItemRef) >>= sTmp;
                xItemList->removeByName(sTmp);
                xOrderList->removeByName(sRemove);
            }

            ::comphelper::ConfigurationHelper::flush(m_xCfg);
        }
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("unotools.config");
    }
}

void SvtHistoryOptions_Impl::Clear( EHistoryType eHistory )
{
    uno::Reference<container::XNameAccess> xListAccess(GetListAccess(eHistory));
    if (!xListAccess.is())
        return;

    uno::Reference<container::XNameContainer> xNode;

    try
    {
        // clear ItemList
        xListAccess->getByName(s_sItemList) >>= xNode;
        Sequence<OUString> aStrings(xNode->getElementNames());

        for (const auto& rString : std::as_const(aStrings))
            xNode->removeByName(rString);

        // clear OrderList
        xListAccess->getByName(s_sOrderList) >>= xNode;
        aStrings = xNode->getElementNames();

        for (const auto& rString : std::as_const(aStrings))
            xNode->removeByName(rString);

        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("unotools.config");
    }
}

Sequence< Sequence<PropertyValue> > SvtHistoryOptions_Impl::GetList(EHistoryType eHistory)
{
    uno::Reference<container::XNameAccess> xListAccess(GetListAccess(eHistory));
    if (!xListAccess.is())
        return Sequence< Sequence<PropertyValue> >();

    impl_truncateList(eHistory, GetCapacity(eHistory));

    Sequence<PropertyValue> seqProperties(5);
    seqProperties[s_nOffsetURL       ].Name = HISTORY_PROPERTYNAME_URL;
    seqProperties[s_nOffsetFilter    ].Name = HISTORY_PROPERTYNAME_FILTER;
    seqProperties[s_nOffsetTitle     ].Name = HISTORY_PROPERTYNAME_TITLE;
    seqProperties[s_nOffsetPassword  ].Name = HISTORY_PROPERTYNAME_PASSWORD;
    seqProperties[s_nOffsetThumbnail ].Name = HISTORY_PROPERTYNAME_THUMBNAIL;

    uno::Reference<container::XNameAccess> xItemList;
    uno::Reference<container::XNameAccess> xOrderList;
    try
    {
        xListAccess->getByName(s_sItemList)  >>= xItemList;
        xListAccess->getByName(s_sOrderList) >>= xOrderList;
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("unotools.config");
    }

    const sal_Int32 nLength = xOrderList->getElementNames().getLength();
    Sequence< Sequence<PropertyValue> > aRet(nLength);
    sal_Int32 nCount = 0;

    for (sal_Int32 nItem = 0; nItem < nLength; ++nItem)
    {
        try
        {
            OUString sUrl;
            uno::Reference<beans::XPropertySet> xSet;
            xOrderList->getByName(OUString::number(nItem)) >>= xSet;
            xSet->getPropertyValue(s_sHistoryItemRef) >>= sUrl;

            xItemList->getByName(sUrl) >>= xSet;
            seqProperties[s_nOffsetURL  ].Value <<= sUrl;

            seqProperties[s_nOffsetFilter   ].Value = xSet->getPropertyValue(s_sFilter);
            seqProperties[s_nOffsetTitle    ].Value = xSet->getPropertyValue(s_sTitle);
            seqProperties[s_nOffsetPassword ].Value = xSet->getPropertyValue(s_sPassword);
            seqProperties[s_nOffsetThumbnail].Value = xSet->getPropertyValue(s_sThumbnail);
            aRet[nCount++] = seqProperties;
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
    assert(nCount <= nLength);
    aRet.realloc(nCount);
    return aRet;
}

void SvtHistoryOptions_Impl::AppendItem(EHistoryType eHistory,
        const OUString& sURL, const OUString& sFilter, const OUString& sTitle,
        const std::optional<OUString>& sThumbnail)
{
    uno::Reference<container::XNameAccess> xListAccess(GetListAccess(eHistory));
    if (!xListAccess.is())
        return;

    impl_truncateList(eHistory, GetCapacity(eHistory));

    sal_Int32 nMaxSize = GetCapacity(eHistory);
    if (nMaxSize == 0)
        return;

    uno::Reference<container::XNameContainer> xItemList;
    uno::Reference<container::XNameContainer> xOrderList;
    uno::Reference<beans::XPropertySet>       xSet;

    try
    {
        xListAccess->getByName(s_sItemList)  >>= xItemList;
        xListAccess->getByName(s_sOrderList) >>= xOrderList;
        sal_Int32 nLength = xOrderList->getElementNames().getLength();

        // The item to be appended already exists
        if (xItemList->hasByName(sURL))
        {
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

            ::comphelper::ConfigurationHelper::flush(m_xCfg);
        }
        else // The item to be appended does not exist yet
        {
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

            ::comphelper::ConfigurationHelper::flush(m_xCfg);
        }
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("unotools.config");
    }
}

void SvtHistoryOptions_Impl::DeleteItem(EHistoryType eHistory, const OUString& sURL)
{
    uno::Reference<container::XNameAccess> xListAccess(GetListAccess(eHistory));
    if (!xListAccess.is())
        return;

    uno::Reference<container::XNameContainer> xItemList;
    uno::Reference<container::XNameContainer> xOrderList;
    uno::Reference<beans::XPropertySet>       xSet;

    try
    {
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

        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("unotools.config");
    }
}

namespace {

std::weak_ptr<SvtHistoryOptions_Impl> g_pHistoryOptions;

}

SvtHistoryOptions::SvtHistoryOptions()
{
    MutexGuard aGuard(theHistoryOptionsMutex::get());

    m_pImpl = g_pHistoryOptions.lock();
    if( !m_pImpl )
    {
        m_pImpl = std::make_shared<SvtHistoryOptions_Impl>();
        g_pHistoryOptions = m_pImpl;
        ItemHolder1::holdConfigItem(EItem::HistoryOptions);
    }
}

SvtHistoryOptions::~SvtHistoryOptions()
{
    MutexGuard aGuard(theHistoryOptionsMutex::get());

    m_pImpl.reset();
}

void SvtHistoryOptions::Clear( EHistoryType eHistory )
{
    MutexGuard aGuard(theHistoryOptionsMutex::get());

    m_pImpl->Clear( eHistory );
}

Sequence< Sequence< PropertyValue > > SvtHistoryOptions::GetList( EHistoryType eHistory ) const
{
    MutexGuard aGuard(theHistoryOptionsMutex::get());

    return m_pImpl->GetList( eHistory );
}

void SvtHistoryOptions::AppendItem(EHistoryType eHistory,
        const OUString& sURL, const OUString& sFilter, const OUString& sTitle,
        const std::optional<OUString>& sThumbnail)
{
    MutexGuard aGuard(theHistoryOptionsMutex::get());

    m_pImpl->AppendItem(eHistory, sURL, sFilter, sTitle, sThumbnail);
}

void SvtHistoryOptions::DeleteItem(EHistoryType eHistory, const OUString& sURL)
{
    MutexGuard aGuard(theHistoryOptionsMutex::get());

    m_pImpl->DeleteItem(eHistory, sURL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
