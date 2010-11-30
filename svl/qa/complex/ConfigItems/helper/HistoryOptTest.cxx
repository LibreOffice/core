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

#include "HistoryOptTest.hxx"
#include <unotools/historyoptions_const.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

#include <comphelper/configurationhelper.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <unotools/processfactory.hxx>

namespace css = ::com::sun::star;

//=============================================================================
static const ::rtl::OUString MESSAGE_CLEAR_FAILED       (RTL_CONSTASCII_USTRINGPARAM("Clearing the list failed."));
static const ::rtl::OUString MESSAGE_SETSIZE_FAILED     (RTL_CONSTASCII_USTRINGPARAM("Setting a new size for a list failed."));
static const ::rtl::OUString MESSAGE_MISS_HISTORY       (RTL_CONSTASCII_USTRINGPARAM("Could not get config access to history list inside config."));
static const ::rtl::OUString MESSAGE_MISS_ITEMLIST      (RTL_CONSTASCII_USTRINGPARAM("Could not get config access to item list inside config."));
static const ::rtl::OUString MESSAGE_MISS_ORDERLIST     (RTL_CONSTASCII_USTRINGPARAM("Could not get config access to order list inside config."));
static const ::rtl::OUString MESSAGE_MISS_ITEM          (RTL_CONSTASCII_USTRINGPARAM("Could not locate item."));
static const ::rtl::OUString MESSAGE_UNEXPECTED_ITEM    (RTL_CONSTASCII_USTRINGPARAM("Found an unexpected item."));
static const ::rtl::OUString MESSAGE_WRONG_ORDER        (RTL_CONSTASCII_USTRINGPARAM("Wrong order in history list."));

//=============================================================================
HistoryOptTest::HistoryOptTest()
    : m_aConfigItem  (         )
    , m_eList        (ePICKLIST)
    , m_xHistoriesXCU(         )
    , m_xCommonXCU   (         )
{
}

//=============================================================================
HistoryOptTest::~HistoryOptTest()
{
    m_xHistoriesXCU.clear();
    m_xCommonXCU.clear();
}

//=============================================================================
void HistoryOptTest::checkPicklist()
{
    impl_testHistory(ePICKLIST, 4);
}

//=============================================================================
void HistoryOptTest::checkURLHistory()
{
    impl_testHistory(eHISTORY, 10);
}

//=============================================================================
void HistoryOptTest::checkHelpBookmarks()
{
    impl_testHistory(eHELPBOOKMARKS, 100);
}

//=============================================================================
void HistoryOptTest::impl_testHistory(EHistoryType eHistory ,
                                      ::sal_Int32  nMaxItems)
{
    try
    {
        m_eList       = eHistory;
        ::sal_Int32 c = nMaxItems;
        ::sal_Int32 i = 0;

        impl_clearList( );
        impl_setSize  (c);

        // a) fill list completely and check if all items could be realy created.
        //    But dont check its order here! Because every new item will change that order.
        for (i=0; i<c; ++i)
        {
            impl_appendItem(i);
            if ( ! impl_existsItem(i))
                throw css::uno::Exception(MESSAGE_MISS_ITEM, 0);
        }

        // b) Check order of all items in list now.
        //    It must be reverse to the item number ...
        //      item max   = index 0
        //      item max-1 = index 1
        //      ...
        for (i=0; i<c; ++i)
        {
            ::sal_Int32 nExpectedIndex = (c-1)-i;
            if ( ! impl_existsItemAtIndex(i, nExpectedIndex))
                throw css::uno::Exception(MESSAGE_WRONG_ORDER, 0);
        }

        // c) increase prio of "first" item so it will switch
        //    to "second" and "second" will switch to "first" :-)
        //    Check also if all other items was not touched.
        ::sal_Int32 nFirstItem  = (c-1);
        ::sal_Int32 nSecondItem = (c-2);
        impl_appendItem(nSecondItem);

        if (
            ( ! impl_existsItemAtIndex(nSecondItem, 0)) ||
            ( ! impl_existsItemAtIndex(nFirstItem , 1))
           )
            throw css::uno::Exception(MESSAGE_WRONG_ORDER, 0);

        for (i=0; i<nSecondItem; ++i)
        {
            ::sal_Int32 nExpectedIndex = (c-1)-i;
            if ( ! impl_existsItemAtIndex(i, nExpectedIndex))
                throw css::uno::Exception(MESSAGE_WRONG_ORDER, 0);
        }

        // d) Check if appending new items will destroy the oldest one.
        ::sal_Int32 nNewestItem = c;
        ::sal_Int32 nOldestItem = 0;

        impl_appendItem(nNewestItem);

        if ( ! impl_existsItemAtIndex(nNewestItem, 0))
            throw css::uno::Exception(MESSAGE_WRONG_ORDER, 0);

        if (impl_existsItem(nOldestItem))
            throw css::uno::Exception(MESSAGE_UNEXPECTED_ITEM, 0);

        // e) Check if decreasing list size will remove oldest items.
        //    Note: impl_setSize() will make sure that 3 items exists only.
        //    Otherwhise it throws an exception. If we further check
        //    positions of three items no further items must be checked.
        //    They cant exists :-)
        ::sal_Int32 nNewSize = 3;
        impl_setSize(nNewSize);
        if (
            ( ! impl_existsItemAtIndex(nNewestItem, 0)) ||
            ( ! impl_existsItemAtIndex(nSecondItem, 1)) ||
            ( ! impl_existsItemAtIndex(nFirstItem , 2))
           )
            throw css::uno::Exception(MESSAGE_WRONG_ORDER, 0);

        // finaly we should try to clean up all used structures so the same office can be used
        // without problems :-)
        impl_clearList();
    }
    catch (const css::uno::Exception& ex)
    {
        impl_clearList();
        throw ex;
    }

}

//=============================================================================
void HistoryOptTest::impl_clearList()
{
    m_aConfigItem.Clear(m_eList);
    ::sal_Int32 nCount = m_aConfigItem.GetList(m_eList).getLength();

    if (nCount != 0)
        throw css::uno::Exception(MESSAGE_CLEAR_FAILED, 0);

    css::uno::Reference< css::container::XNameAccess > xList;
    xList  = impl_getItemList();
    nCount = xList->getElementNames().getLength();

    if (nCount != 0)
        throw css::uno::Exception(MESSAGE_CLEAR_FAILED, 0);

    xList  = impl_getOrderList();
    nCount = xList->getElementNames().getLength();

    if (nCount != 0)
        throw css::uno::Exception(MESSAGE_CLEAR_FAILED, 0);
}

//=============================================================================
void HistoryOptTest::impl_setSize(::sal_Int32 nSize)
{
    m_aConfigItem.SetSize (m_eList, nSize);

    // a) size info returned by GetSize() means "MaxSize"
    //    so it must match exactly !
    ::sal_Int32 nCheck = m_aConfigItem.GetSize(m_eList);
    if (nCheck != nSize)
        throw css::uno::Exception(MESSAGE_SETSIZE_FAILED, 0);

    // b) current size of used XCU lists reflects the current state of
    //    history list and not max size. So it can be less then size !
    css::uno::Reference< css::container::XNameAccess > xList;
    xList  = impl_getItemList();
    nCheck = xList->getElementNames().getLength();
    if (nCheck > nSize)
        throw css::uno::Exception(MESSAGE_SETSIZE_FAILED, 0);

    xList  = impl_getOrderList();
    nCheck = xList->getElementNames().getLength();
    if (nCheck > nSize)
        throw css::uno::Exception(MESSAGE_SETSIZE_FAILED, 0);
}

//=============================================================================
void HistoryOptTest::impl_appendItem(::sal_Int32 nItem)
{
    const ::rtl::OUString sURL      = impl_createItemURL     (nItem);
    const ::rtl::OUString sTitle    = impl_createItemTitle   (nItem);
    const ::rtl::OUString sPassword = impl_createItemPassword(nItem);

    m_aConfigItem.AppendItem(m_eList, sURL, ::rtl::OUString(), sTitle, sPassword);
}

//=============================================================================
::rtl::OUString HistoryOptTest::impl_createItemURL(::sal_Int32 nItem)
{
    ::rtl::OUStringBuffer sURL(256);
    sURL.appendAscii("file:///ooo_api_test/non_existing_test_url_");
    sURL.append     ((::sal_Int32)nItem  );
    sURL.appendAscii(".odt"              );

    return sURL.makeStringAndClear();
}

//=============================================================================
::rtl::OUString HistoryOptTest::impl_createItemTitle(::sal_Int32 nItem)
{
    ::rtl::OUStringBuffer sTitle(256);
    sTitle.appendAscii("Non Existing Test Item Nr ");
    sTitle.append     ((::sal_Int32)nItem          );

    return sTitle.makeStringAndClear();
}

//=============================================================================
::rtl::OUString HistoryOptTest::impl_createItemPassword(::sal_Int32 nItem)
{
    ::rtl::OUStringBuffer sPassword(256);
    sPassword.appendAscii("Password_"       );
    sPassword.append     ((::sal_Int32)nItem);

    return sPassword.makeStringAndClear();
}

//=============================================================================
::sal_Bool HistoryOptTest::impl_existsItem(::sal_Int32 nItem)
{
    const ::rtl::OUString                                                       sURL     = impl_createItemURL(nItem);
    const css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > lItems   = m_aConfigItem.GetList(m_eList);
    const ::sal_Int32                                                           c        = lItems.getLength ();
          ::sal_Int32                                                           i        = 0;
          ::sal_Bool                                                            bFound   = sal_False;

    for (i=0; i<c; ++i)
    {
        const ::comphelper::SequenceAsHashMap aItem(lItems[i]);
        const ::rtl::OUString&                sCheck = aItem.getUnpackedValueOrDefault(s_sURL, ::rtl::OUString());

        bFound = sCheck.equals(sURL);
        if (bFound)
            break;
    }

    if ( ! bFound)
        return sal_False;
    bFound = sal_False;

    try
    {
        css::uno::Reference< css::container::XNameAccess > xItemList = impl_getItemList();
        css::uno::Reference< css::container::XNameAccess > xItem     ;
        xItemList->getByName(sURL) >>= xItem;

        bFound = xItem.is();
    }
    catch(const css::container::NoSuchElementException&)
        {}

    return bFound;
}

//=============================================================================
::sal_Bool HistoryOptTest::impl_existsItemAtIndex(::sal_Int32 nItem ,
                                                  ::sal_Int32 nIndex)
{
    const ::rtl::OUString                                                       sURL     = impl_createItemURL(nItem);
    const css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > lItems   = m_aConfigItem.GetList(m_eList);
    const ::sal_Int32                                                           c        = lItems.getLength ();
          ::sal_Bool                                                            bFound   = sal_False;

    if (nIndex >= c)
        return sal_False;

    const ::comphelper::SequenceAsHashMap aItem(lItems[nIndex]);
          ::rtl::OUString                 sCheck = aItem.getUnpackedValueOrDefault(s_sURL, ::rtl::OUString());

    bFound = sCheck.equals(sURL);
    if ( ! bFound)
        return sal_False;
    bFound = sal_False;

    try
    {
        css::uno::Reference< css::container::XNameAccess > xItemList = impl_getItemList();
        css::uno::Reference< css::container::XNameAccess > xItem     ;
        xItemList->getByName(sURL) >>= xItem;

        bFound = xItem.is();
    }
    catch(const css::container::NoSuchElementException&)
        {}

    if ( ! bFound)
        return sal_False;
    bFound = sal_False;

    try
    {
        const ::rtl::OUString                              sOrder     = ::rtl::OUString::valueOf(nIndex);
        css::uno::Reference< css::container::XNameAccess > xOrderList = impl_getOrderList();
        css::uno::Reference< css::container::XNameAccess > xOrder     ;
        xOrderList->getByName(sOrder) >>= xOrder;

        if (xOrder.is())
        {
            xOrder->getByName(s_sHistoryItemRef) >>= sCheck;
            bFound = sCheck.equals(sURL);
        }
    }
    catch(const css::container::NoSuchElementException&)
        {}

    return bFound;
}

//=============================================================================
css::uno::Reference< css::container::XNameAccess > HistoryOptTest::impl_getItemList()
{
    css::uno::Reference< css::container::XNameAccess > xHistory = impl_getNewHistory();
    css::uno::Reference< css::container::XNameAccess > xList    ;
    xHistory->getByName (s_sItemList) >>= xList;

    if ( ! xList.is())
        throw css::uno::Exception(MESSAGE_MISS_ITEMLIST, 0);

    return xList;
}

//=============================================================================
css::uno::Reference< css::container::XNameAccess > HistoryOptTest::impl_getOrderList()
{
    css::uno::Reference< css::container::XNameAccess > xHistory = impl_getNewHistory();
    css::uno::Reference< css::container::XNameAccess > xList    ;
    xHistory->getByName (s_sOrderList) >>= xList;

    if ( ! xList.is())
        throw css::uno::Exception(MESSAGE_MISS_ORDERLIST, 0);

    return xList;
}

//=============================================================================
css::uno::Reference< css::container::XNameAccess > HistoryOptTest::impl_getNewHistory()
{
    if ( ! m_xHistoriesXCU.is())
    {
        m_xHistoriesXCU = css::uno::Reference< css::container::XNameAccess >(
            ::comphelper::ConfigurationHelper::openConfig(
            ::utl::getProcessServiceFactory(),
            s_sHistories,
            ::comphelper::ConfigurationHelper::E_STANDARD),
            css::uno::UNO_QUERY_THROW);
    }

    css::uno::Reference< css::container::XNameAccess > xHistory;

    switch (m_eList)
    {
        case ePICKLIST :
                m_xHistoriesXCU->getByName(s_sPickList) >>= xHistory;
                break;

        case eHISTORY :
                m_xHistoriesXCU->getByName(s_sURLHistory) >>= xHistory;
                break;

        case eHELPBOOKMARKS :
                m_xHistoriesXCU->getByName(s_sHelpBookmarks) >>= xHistory;
                break;
    }

    if ( ! xHistory.is())
        throw css::uno::Exception(MESSAGE_MISS_HISTORY, 0);

    return xHistory;
}

//=============================================================================
css::uno::Reference< css::container::XNameAccess > HistoryOptTest::impl_getOldHistory()
{
    if ( ! m_xCommonXCU.is())
    {
        m_xCommonXCU = css::uno::Reference< css::container::XNameAccess >(
            ::comphelper::ConfigurationHelper::openConfig(
            ::utl::getProcessServiceFactory(),
            s_sCommonHistory,
            ::comphelper::ConfigurationHelper::E_STANDARD),
            css::uno::UNO_QUERY_THROW);
    }

    css::uno::Reference< css::container::XNameAccess > xHistory;

    switch (m_eList)
    {
        case ePICKLIST :
                m_xCommonXCU->getByName(s_sPickList) >>= xHistory;
                break;

        case eHISTORY :
                m_xCommonXCU->getByName(s_sURLHistory) >>= xHistory;
                break;

        case eHELPBOOKMARKS :
                m_xCommonXCU->getByName(s_sHelpBookmarks) >>= xHistory;
                break;
    }

    if ( ! xHistory.is())
        throw css::uno::Exception(MESSAGE_MISS_HISTORY, 0);

    return xHistory;
}

/*
//=============================================================================
// clear the list in XML directly when using the new Histories.xcs
void HistoryOptTest::impl_clearList(const ::rtl::OUString& sList)
{
    css::uno::Reference< css::container::XNameAccess >    xListAccess;
    css::uno::Reference< css::container::XNameContainer > xItemOrder;
    css::uno::Reference< css::beans::XPropertySet >       xFirstItem;
    css::uno::Sequence< ::rtl::OUString >                 sFileList;

    if (sList.equalsAscii("PickList"))
        m_xCfg->getByName(s_sPickList) >>= xListAccess;

    else if (sList.equalsAscii("URLHistory"))
        m_xCfg->getByName(s_sURLHistory) >>= xListAccess;

    else if (sList.equalsAscii("HelpBookmarks"))
        m_xCfg->getByName(s_sHelpBookmarks) >>= xListAccess;

    if (xListAccess.is())
    {
        xListAccess->getByName(s_sItemList)  >>= xItemOrder  ;
        sFileList = xItemOrder->getElementNames();
        for(sal_Int32 i=0; i<sFileList.getLength(); ++i)
            xItemOrder->removeByName(sFileList[i]);

        xListAccess->getByName(s_sOrderList) >>= xItemOrder ;
        sFileList = xItemOrder->getElementNames();
        for(sal_Int32 j=0; j<sFileList.getLength(); ++j)
            xItemOrder->removeByName(sFileList[j]);

        xFirstItem = css::uno::Reference< css::beans::XPropertySet >(xListAccess, css::uno::UNO_QUERY);
        xFirstItem->setPropertyValue( s_sFirstItem, css::uno::makeAny((sal_Int32)0) );

        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
}

//=============================================================================
// use configuration API (not ConfigItem!) to verify the results within XML !
sal_Bool HistoryOptTest::impl_isListEmpty(const ::rtl::OUString& sList)
{
    css::uno::Reference< css::container::XNameAccess > xListAccess;
    css::uno::Reference< css::container::XNameAccess > xItemList;
    css::uno::Reference< css::container::XNameAccess > xOrderList;
    sal_Bool bRet = sal_True;

    if (sList.equalsAscii("PickList"))
        m_xCfg->getByName(s_sPickList) >>= xListAccess;

    else if (sList.equalsAscii("URLHistory"))
        m_xCfg->getByName(s_sURLHistory) >>= xListAccess;

    else if (sList.equalsAscii("HelpBookmarks"))
        m_xCfg->getByName(s_sHelpBookmarks) >>= xListAccess;

    if (xListAccess.is())
    {
        xListAccess->getByName(s_sItemList)  >>= xItemList;
        xListAccess->getByName(s_sOrderList) >>= xOrderList;

        css::uno::Sequence< ::rtl::OUString > sItemList  = xItemList->getElementNames();
        css::uno::Sequence< ::rtl::OUString > sOrderList = xOrderList->getElementNames();
        if (sItemList.getLength()!=0 || sOrderList.getLength()!=0)
            bRet = sal_False;
    }

    return bRet;
}

//=============================================================================
// append a item: use configuration API (not ConfigItem!) to verify the results within XML !
void HistoryOptTest::impl_appendItem(const ::rtl::OUString& sList)
{//to do...
}

//=============================================================================
// test SvtHistoryOptions::GetSize()
void HistoryOptTest::impl_checkGetSize(const ::rtl::OUString& sList)
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCommonXCU, css::uno::UNO_QUERY);

    sal_uInt32 nSize  = 0;
    sal_uInt32 nSize_ = 0;

    if (sList.equalsAscii("PickList"))
    {
        nSize  = aHistoryOpt.GetSize(ePICKLIST);

        xSet->setPropertyValue(s_sPickListSize, css::uno::makeAny(nSize+1));
        ::comphelper::ConfigurationHelper::flush(m_xCommonXCU);

        nSize_ = aHistoryOpt.GetSize(ePICKLIST);
        if (nSize_ == nSize)
            throw css::uno::RuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GetSize(ePICKLIST) error!")), 0);
    }

    else if (sList.equalsAscii("URLHistory"))
    {
        nSize  = aHistoryOpt.GetSize(eHISTORY);

        xSet->setPropertyValue(s_sURLHistorySize, css::uno::makeAny(nSize+1));
        ::comphelper::ConfigurationHelper::flush(m_xCommonXCU);

        nSize_ = aHistoryOpt.GetSize(eHISTORY);

        if (nSize_ == nSize)
            throw css::uno::RuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GetSize(eHISTORY) error!")), 0);
    }

    else if (sList.equalsAscii("HelpBookmarks"))
    {
        nSize  = aHistoryOpt.GetSize(eHELPBOOKMARKS);

        xSet->setPropertyValue(s_sHelpBookmarksSize, css::uno::makeAny(nSize+1));
        ::comphelper::ConfigurationHelper::flush(m_xCommonXCU);

        nSize_ = aHistoryOpt.GetSize(eHELPBOOKMARKS);

        if (nSize_ == nSize)
            throw css::uno::RuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GetSize(eHELPBOOKMARKS) error!")), 0);
    }
}

//=============================================================================
// test SvtHistoryOptions::SetSize()
void HistoryOptTest::impl_checkSetSize(const ::rtl::OUString& sList)
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCommonXCU, css::uno::UNO_QUERY);

    sal_uInt32 nSize  = 0;
    sal_uInt32 nSize_ = 0;

    if (sList.equalsAscii("PickList"))
    {
        xSet->getPropertyValue(s_sPickListSize) >>= nSize;
        aHistoryOpt.SetSize(ePICKLIST, (nSize+1));
        xSet->getPropertyValue(s_sPickListSize) >>= nSize_;

        if (nSize_ == nSize) //old config item will throw error
            throw css::uno::RuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SetSize(ePICKLIST) error!")), 0);
    }

    else if (sList.equalsAscii("URLHistory"))
    {
        xSet->getPropertyValue(s_sURLHistorySize) >>= nSize;
        aHistoryOpt.SetSize(eHISTORY, (nSize+1));
        xSet->getPropertyValue(s_sURLHistorySize) >>= nSize_;

        if (nSize_ == nSize) //old config item will throw error
            throw css::uno::RuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SetSize(eHISTORY) error!")), 0);
    }

    else if (sList.equalsAscii("HelpBookmarks"))
    {
        xSet->getPropertyValue(s_sHelpBookmarksSize) >>= nSize;
        aHistoryOpt.SetSize(eHELPBOOKMARKS, (nSize+1));
        xSet->getPropertyValue(s_sHelpBookmarksSize) >>= nSize_;

        if (nSize_ == nSize) //old config item will throw error
            throw css::uno::RuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SetSize(eHELPBOOKMARKS) error!")), 0);
    }
}

//=============================================================================
// test SvtHistoryOptions::Clear()
void HistoryOptTest::impl_checkClear(const ::rtl::OUString& sList)
{
    if (sList.equalsAscii("PickList"))
    {
        aHistoryOpt.Clear(ePICKLIST);
        if ( !impl_isListEmpty(s_sPickList) )
            throw css::uno::RuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Clear(ePICKLIST) error!")), 0);
    }

    else if (sList.equalsAscii("URLHistory"))
    {
        aHistoryOpt.Clear(eHISTORY);
        if ( !impl_isListEmpty(s_sURLHistory) )
            throw css::uno::RuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Clear(eHISTORY) error!")), 0);
    }

    else if (sList.equalsAscii("HelpBookmarks"))
    {
        aHistoryOpt.Clear(eHELPBOOKMARKS);
        if ( !impl_isListEmpty(s_sHelpBookmarks) )
            throw css::uno::RuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Clear(eHELPBOOKMARKS) error!")), 0);
    }
}

//=============================================================================
// test SvtHistoryOptions::GetList()
void HistoryOptTest::impl_checkGetList(const ::rtl::OUString& sList)
{
    if (sList.equalsAscii("PickList"))
    {
        impl_clearList(s_sPickList);
        aHistoryOpt.AppendItem( ePICKLIST  ,
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///c/test1")),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")) );
        css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > aHistoryList = aHistoryOpt.GetList( ePICKLIST );

        if ( aHistoryList.getLength()==0 )
            throw css::uno::RuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GetList(ePICKLIST) error!")), 0);
    }

    else if (sList.equalsAscii("URLHistory"))
    {
        impl_clearList(s_sURLHistory);
        aHistoryOpt.AppendItem( eHISTORY  ,
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///c/test1")),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")) );
        css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > aHistoryList = aHistoryOpt.GetList( eHISTORY );

        if ( aHistoryList.getLength()==0 )
            throw css::uno::RuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GetList(eHISTORY) error!")), 0);
    }

    else if (sList.equalsAscii("HelpBookmarks"))
    {
        impl_clearList(s_sHelpBookmarks);
        aHistoryOpt.AppendItem( eHELPBOOKMARKS  ,
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///c/test1")),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")) );
        css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > aHistoryList = aHistoryOpt.GetList( eHELPBOOKMARKS );

        if ( aHistoryList.getLength()==0 )
            throw css::uno::RuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GetList(eHELPBOOKMARKS) error!")), 0);
    }
}

void HistoryOptTest::impl_checkAppendItem(const ::rtl::OUString& sList)
{
    if (sList.equalsAscii("PickList"))
    {
        impl_clearList(s_sPickList);
        sal_Int32 nListSize = aHistoryOpt.GetSize(ePICKLIST);

        for (sal_Int32 i=0; i<nListSize; ++i)
            aHistoryOpt.AppendItem( ePICKLIST  ,
            ::rtl::OUString::valueOf(i),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")) );

        aHistoryOpt.AppendItem( ePICKLIST  ,
            ::rtl::OUString::valueOf(nListSize),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")) );
    }

    else if (sList.equalsAscii("URLHistory"))
    {
        impl_clearList(s_sURLHistory);
        sal_Int32 nListSize = aHistoryOpt.GetSize(eHISTORY);

        for (sal_Int32 i=0; i<nListSize; ++i)
            aHistoryOpt.AppendItem( eHISTORY  ,
            ::rtl::OUString::valueOf(i),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")) );

        aHistoryOpt.AppendItem( eHISTORY  ,
            ::rtl::OUString::valueOf(nListSize),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")) );
    }

    else if (sList.equalsAscii("HelpBookmarks"))
    {
        //impl_clearList(s_sHelpBookmarks);
        //sal_Int32 nListSize = aHistoryOpt.GetSize(eHELPBOOKMARKS);

        //for (sal_Int32 i=0; i<nListSize; ++i)
        //  aHistoryOpt.AppendItem( eHELPBOOKMARKS  ,
        //  ::rtl::OUString::valueOf(i),
        //  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")),
        //  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")),
        //  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")) );

        //aHistoryOpt.AppendItem( eHELPBOOKMARKS  ,
        //  ::rtl::OUString::valueOf(nListSize),
        //  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")),
        //  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")),
        //  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")) );
    }
}

//=============================================================================
void HistoryOptTest::impl_checkPicklist()
{
    impl_checkGetSize(s_sPickList);
    impl_checkSetSize(s_sPickList);

    impl_checkClear(s_sPickList);
    impl_checkGetList(s_sPickList);
    impl_checkAppendItem(s_sPickList);
}

//=============================================================================
void HistoryOptTest::impl_checkURLHistory()
{
    impl_checkGetSize(s_sURLHistory);
    impl_checkSetSize(s_sURLHistory);

    impl_checkClear(s_sURLHistory);
    impl_checkGetList(s_sURLHistory);
    impl_checkAppendItem(s_sURLHistory);
}

//=============================================================================
void HistoryOptTest::impl_checkHelpBookmarks()
{
    impl_checkGetSize(s_sHelpBookmarks);
    impl_checkSetSize(s_sHelpBookmarks);

    impl_checkClear(s_sHelpBookmarks);
    impl_checkGetList(s_sHelpBookmarks);
    impl_checkAppendItem(s_sHelpBookmarks);
}
*/

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
