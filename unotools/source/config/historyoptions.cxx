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
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cassert>
#include <deque>
#include <algorithm>

#include <rtl/logfile.hxx>
#include "itemholder1.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/container/XNameAccess.hpp>

#include <com/sun/star/container/XNameContainer.hpp>

#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <comphelper/configurationhelper.hxx>

#include <comphelper/processfactory.hxx>

using namespace ::std     ;
using namespace ::utl     ;
using namespace ::rtl     ;
using namespace ::osl     ;
using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::beans ;

namespace css = ::com::sun::star;

namespace {
    static const ::sal_Int32 s_nOffsetURL               = 0;
    static const ::sal_Int32 s_nOffsetFilter            = 1;
    static const ::sal_Int32 s_nOffsetTitle             = 2;
    static const ::sal_Int32 s_nOffsetPassword          = 3;

    const char s_sCommonHistory[] = "org.openoffice.Office.Common/History";
    const char s_sHistories[] = "org.openoffice.Office.Histories/Histories";
    const char s_sPickListSize[] = "PickListSize";
    const char s_sURLHistorySize[] = "Size";
    const char s_sHelpBookmarksSize[] = "HelpBookmarkSize";
    const char s_sPickList[] = "PickList";
    const char s_sURLHistory[] = "URLHistory";
    const char s_sHelpBookmarks[] = "HelpBookmarks";
    const char s_sItemList[] = "ItemList";
    const char s_sOrderList[] = "OrderList";
    const char s_sHistoryItemRef[] = "HistoryItemRef";
    const char s_sFilter[] = "Filter";
    const char s_sTitle[] = "Title";
    const char s_sPassword[] = "Password";
}

struct IMPL_THistoryItem
{
    IMPL_THistoryItem()
    {
    }

    IMPL_THistoryItem( const OUString& sNewURL   ,
        const OUString& sNewFilter  ,
        const OUString& sNewTitle  ,
        const OUString& sNewPassword )
    {
        sURL  = sNewURL  ;
        sFilter  = sNewFilter ;
        sTitle  = sNewTitle  ;
        sPassword = sNewPassword ;
    }

    sal_Bool operator==( const OUString& sSearchedURL ) const
    {
        return( sURL == sSearchedURL );
    }

    OUString sURL  ;
    OUString sFilter  ;
    OUString sTitle  ;
    OUString sPassword ;
};

//*****************************************************************************************************************
//  class SvtHistoryOptions_Impl
//  redesigned
//*****************************************************************************************************************
class SvtHistoryOptions_Impl
{
public:
    SvtHistoryOptions_Impl();
    ~SvtHistoryOptions_Impl();

    sal_uInt32 GetSize( EHistoryType eHistory );
    void Clear( EHistoryType eHistory );
    Sequence< Sequence< PropertyValue > > GetList( EHistoryType eHistory );
    void                                  AppendItem(       EHistoryType eHistory ,
        const OUString&    sURL     ,
        const OUString&    sFilter  ,
        const OUString&    sTitle   ,
        const OUString&    sPassword );

private:
    void impl_truncateList (EHistoryType eHistory, sal_uInt32 nSize);

private:
    css::uno::Reference< css::container::XNameAccess > m_xCfg;
    css::uno::Reference< css::container::XNameAccess > m_xCommonXCU;
};

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtHistoryOptions_Impl::SvtHistoryOptions_Impl()
{
    try
    {
        m_xCfg = Reference< css::container::XNameAccess > (
            ::comphelper::ConfigurationHelper::openConfig(
            ::comphelper::getProcessComponentContext(),
            rtl::OUString(s_sHistories),
            ::comphelper::ConfigurationHelper::E_STANDARD),
            css::uno::UNO_QUERY );

        m_xCommonXCU = Reference< css::container::XNameAccess > (
            ::comphelper::ConfigurationHelper::openConfig(
            ::comphelper::getProcessComponentContext(),
            rtl::OUString(s_sCommonHistory),
            ::comphelper::ConfigurationHelper::E_STANDARD),
            css::uno::UNO_QUERY );
    }
    catch(const css::uno::Exception& ex)
    {
        m_xCfg.clear();
        m_xCommonXCU.clear();

        SAL_WARN("unotools", "Caught unexpected: " << ex.Message);
    }
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtHistoryOptions_Impl::~SvtHistoryOptions_Impl()
{
}

//*****************************************************************************************************************
//  public method
//  Attention: We return the max. size of our internal lists - That is the capacity not the size!
//*****************************************************************************************************************
sal_uInt32 SvtHistoryOptions_Impl::GetSize( EHistoryType eHistory )
{
    css::uno::Reference< css::beans::XPropertySet >  xListAccess(m_xCommonXCU, css::uno::UNO_QUERY);

    if (!xListAccess.is())
        return 0;

    sal_uInt32 nSize = 0  ;

    try
    {
        switch( eHistory )
        {
        case ePICKLIST:
            xListAccess->getPropertyValue(rtl::OUString(s_sPickListSize)) >>= nSize;
            break;

        case eHISTORY:
            xListAccess->getPropertyValue(rtl::OUString(s_sURLHistorySize)) >>= nSize;
            break;

        case eHELPBOOKMARKS:
            xListAccess->getPropertyValue(rtl::OUString(s_sHelpBookmarksSize)) >>= nSize;
            break;

        default:
            break;
        }
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("unotools", "Caught unexpected: " << ex.Message);
    }

    return nSize;
}

//*****************************************************************************************************************
void SvtHistoryOptions_Impl::impl_truncateList ( EHistoryType eHistory, sal_uInt32 nSize )
{
    css::uno::Reference< css::container::XNameAccess >    xList;
    css::uno::Reference< css::container::XNameContainer > xItemList;
    css::uno::Reference< css::container::XNameContainer > xOrderList;
    css::uno::Reference< css::beans::XPropertySet >       xSet;

    try
    {
        switch( eHistory )
        {
        case ePICKLIST:
            m_xCfg->getByName(rtl::OUString(s_sPickList)) >>= xList;
            break;

        case eHISTORY:
            m_xCfg->getByName(rtl::OUString(s_sURLHistory)) >>= xList;
            break;

        case eHELPBOOKMARKS:
            m_xCfg->getByName(rtl::OUString(s_sHelpBookmarks)) >>= xList;
            break;

        default:
            break;
        }

        // If too much items in current list ...
        // truncate the oldest items BEFORE you set the new one.
        if ( ! xList.is())
            return;

        xList->getByName(rtl::OUString(s_sOrderList)) >>= xOrderList;
        xList->getByName(rtl::OUString(s_sItemList))  >>= xItemList;

        const sal_uInt32 nLength = xOrderList->getElementNames().getLength();
        if (nSize < nLength)
        {
            for (sal_uInt32 i=nLength-1; i>=nSize; --i)
            {
                ::rtl::OUString sTmp;
                const ::rtl::OUString sRemove = ::rtl::OUString::valueOf((sal_Int32)i);
                xOrderList->getByName(sRemove) >>= xSet;
                xSet->getPropertyValue(rtl::OUString(s_sHistoryItemRef)) >>= sTmp;
                xItemList->removeByName(sTmp);
                xOrderList->removeByName(sRemove);
            }

            ::comphelper::ConfigurationHelper::flush(m_xCfg);
        }
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("unotools", "Caught unexpected: " << ex.Message);
    }
}

//*****************************************************************************************************************
//  public method
//  Clear specified history list
//*****************************************************************************************************************
void SvtHistoryOptions_Impl::Clear( EHistoryType eHistory )
{
    css::uno::Reference< css::container::XNameAccess >    xListAccess;
    css::uno::Reference< css::container::XNameContainer > xNode;
    Sequence< ::rtl::OUString >                           lOrders;

    try
    {
        switch( eHistory )
        {
        case ePICKLIST:
            {
                m_xCfg->getByName(rtl::OUString(s_sPickList)) >>= xListAccess;
                break;
            }

        case eHISTORY:
            {
                m_xCfg->getByName(rtl::OUString(s_sURLHistory)) >>= xListAccess;
                break;
            }

        case eHELPBOOKMARKS:
            {
                m_xCfg->getByName(rtl::OUString(s_sHelpBookmarks)) >>= xListAccess;
                break;
            }

        default:
            break;
        }

        if (xListAccess.is())
        {
            // clear ItemList
            xListAccess->getByName(rtl::OUString(s_sItemList))  >>= xNode  ;
            lOrders = xNode->getElementNames();
            const sal_Int32 nLength = lOrders.getLength();
            for(sal_Int32 i=0; i<nLength; ++i)
                xNode->removeByName(lOrders[i]);

            // clear OrderList
            xListAccess->getByName(rtl::OUString(s_sOrderList)) >>= xNode ;
            lOrders = xNode->getElementNames();
            for(sal_Int32 j=0; j<nLength; ++j)
                xNode->removeByName(lOrders[j]);

            ::comphelper::ConfigurationHelper::flush(m_xCfg);
        }
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("unotools", "Caught unexpected: " << ex.Message);
    }
}

//*****************************************************************************************************************
//  public method
//  get a sequence list from the items
//*****************************************************************************************************************
Sequence< Sequence< PropertyValue > > SvtHistoryOptions_Impl::GetList( EHistoryType eHistory )
{
    impl_truncateList (eHistory, GetSize (eHistory));

    Sequence< Sequence< PropertyValue > > seqReturn; // Set default return value.
    Sequence< PropertyValue >             seqProperties( 4 );
    Sequence< ::rtl::OUString >           lOrders;

    css::uno::Reference< css::container::XNameAccess > xListAccess;
    css::uno::Reference< css::container::XNameAccess > xItemList;
    css::uno::Reference< css::container::XNameAccess > xOrderList;
    css::uno::Reference< css::beans::XPropertySet >    xSet;

    seqProperties[s_nOffsetURL       ].Name = HISTORY_PROPERTYNAME_URL;
    seqProperties[s_nOffsetFilter    ].Name = HISTORY_PROPERTYNAME_FILTER;
    seqProperties[s_nOffsetTitle     ].Name = HISTORY_PROPERTYNAME_TITLE;
    seqProperties[s_nOffsetPassword  ].Name = HISTORY_PROPERTYNAME_PASSWORD;

    try
    {
        switch( eHistory )
        {
        case ePICKLIST:
            {
                m_xCfg->getByName(rtl::OUString(s_sPickList)) >>= xListAccess;
                break;
            }

        case eHISTORY:
            {
                m_xCfg->getByName(rtl::OUString(s_sURLHistory)) >>= xListAccess;
                break;
            }

        case eHELPBOOKMARKS:
            {
                m_xCfg->getByName(rtl::OUString(s_sHelpBookmarks)) >>= xListAccess;
                break;
            }

        default:
            break;
        }

        if (xListAccess.is())
        {
            xListAccess->getByName(rtl::OUString(s_sItemList))  >>= xItemList;
            xListAccess->getByName(rtl::OUString(s_sOrderList)) >>= xOrderList;

            const sal_Int32 nLength = xOrderList->getElementNames().getLength();
            Sequence< Sequence< PropertyValue > > aRet(nLength);
            sal_Int32 nCount = 0;

            for(sal_Int32 nItem=0; nItem<nLength; ++nItem)
            {
                try
                {
                    ::rtl::OUString sUrl;
                    xOrderList->getByName(::rtl::OUString::valueOf(nItem)) >>= xSet;
                    xSet->getPropertyValue(rtl::OUString(s_sHistoryItemRef)) >>= sUrl;

                    xItemList->getByName(sUrl) >>= xSet;
                    seqProperties[s_nOffsetURL  ].Value <<= sUrl;
                    xSet->getPropertyValue(rtl::OUString(s_sFilter))   >>= seqProperties[s_nOffsetFilter   ].Value;
                    xSet->getPropertyValue(rtl::OUString(s_sTitle))    >>= seqProperties[s_nOffsetTitle    ].Value;
                    xSet->getPropertyValue(rtl::OUString(s_sPassword)) >>= seqProperties[s_nOffsetPassword ].Value;
                    aRet[nCount++] = seqProperties;
                }
                catch(const css::uno::Exception& ex)
                {
                    // <https://bugs.freedesktop.org/show_bug.cgi?id=46074>
                    // "FILEOPEN: No Recent Documents..." discusses a problem
                    // with corrupted /org.openoffice.Office/Histories/Histories
                    // configuration items; to work around that problem, simply
                    // ignore such corrupted individual items here, so that at
                    // least newly added items are successfully reported back
                    // from this function:
                    SAL_WARN("unotools", "Caught unexpected: " << ex.Message);
                }
            }
            assert(nCount <= nLength);
            aRet.realloc(nCount);
            seqReturn = aRet;
        }
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("unotools", "Caught unexpected: " << ex.Message);
    }

    return seqReturn;
}

//*****************************************************************************************************************
//  public method
//  implements a deque in XML
//*****************************************************************************************************************
void SvtHistoryOptions_Impl::AppendItem(       EHistoryType eHistory ,
                                        const OUString& sURL        ,
                                        const OUString& sFilter     ,
                                        const OUString& sTitle      ,
                                        const OUString& sPassword   )
{
    impl_truncateList (eHistory, GetSize (eHistory));

    css::uno::Reference< css::container::XNameAccess > xListAccess;
    sal_Int32             nMaxSize = 0;

    switch(eHistory)
    {
    case ePICKLIST:
        {
            m_xCfg->getByName(rtl::OUString(s_sPickList)) >>= xListAccess;
            nMaxSize = GetSize(ePICKLIST);
        }
        break;
    case eHISTORY:
        {
            m_xCfg->getByName(rtl::OUString(s_sURLHistory)) >>= xListAccess;
            nMaxSize = GetSize(eHISTORY);
        }
        break;
    case eHELPBOOKMARKS:
        {
            m_xCfg->getByName(rtl::OUString(s_sHelpBookmarks)) >>= xListAccess;
            nMaxSize = GetSize(eHELPBOOKMARKS);
        }
        break;
    default:
        break;
    }

    if (nMaxSize==0)
        return;

    css::uno::Reference< css::container::XNameContainer > xItemList;
    css::uno::Reference< css::container::XNameContainer > xOrderList;
    css::uno::Reference< css::beans::XPropertySet >       xSet;

    try
    {
        xListAccess->getByName(rtl::OUString(s_sItemList))  >>= xItemList;
        xListAccess->getByName(rtl::OUString(s_sOrderList)) >>= xOrderList;
        sal_Int32 nLength = xOrderList->getElementNames().getLength();

        rtl::OUString sHistoryItemRef(s_sHistoryItemRef);
        // The item to be appended is already existing!
        if (xItemList->hasByName(sURL))
        {
            for (sal_Int32 i=0; i<nLength; ++i)
            {
                ::rtl::OUString sTmp;
                xOrderList->getByName(::rtl::OUString::valueOf(i)) >>= xSet;
                xSet->getPropertyValue(sHistoryItemRef) >>= sTmp;

                if(sURL == sTmp)
                {
                    ::rtl::OUString sFind;
                    xOrderList->getByName( ::rtl::OUString::valueOf(i) ) >>= xSet;
                    xSet->getPropertyValue(sHistoryItemRef) >>= sFind;
                    for (sal_Int32 j=i-1; j>=0; --j)
                    {
                        css::uno::Reference< css::beans::XPropertySet > xPrevSet;
                        css::uno::Reference< css::beans::XPropertySet > xNextSet;
                        xOrderList->getByName( ::rtl::OUString::valueOf(j+1) )   >>= xPrevSet;
                        xOrderList->getByName( ::rtl::OUString::valueOf(j) )     >>= xNextSet;

                        ::rtl::OUString sTemp;
                        xNextSet->getPropertyValue(sHistoryItemRef) >>= sTemp;
                        xPrevSet->setPropertyValue(sHistoryItemRef, css::uno::makeAny(sTemp));
                    }
                    xOrderList->getByName( ::rtl::OUString::valueOf((sal_Int32)0) ) >>= xSet;
                    xSet->setPropertyValue(sHistoryItemRef, css::uno::makeAny(sFind));

                    ::comphelper::ConfigurationHelper::flush(m_xCfg);
                    break;
                }
            }
        }

        // The item to be appended is not existing!
        else
        {
            css::uno::Reference< css::lang::XSingleServiceFactory > xFac;
            css::uno::Reference< css::uno::XInterface >             xInst;
            css::uno::Reference< css::beans::XPropertySet > xPrevSet;
            css::uno::Reference< css::beans::XPropertySet > xNextSet;

            // Append new item to OrderList.
            if ( nLength == nMaxSize )
            {
                ::rtl::OUString sRemove;
                xOrderList->getByName(::rtl::OUString::valueOf(nLength-1)) >>= xSet;
                xSet->getPropertyValue(sHistoryItemRef) >>= sRemove;
                try
                {
                    xItemList->removeByName(sRemove);
                }
                catch (css::container::NoSuchElementException &)
                {
                    // <https://bugs.freedesktop.org/show_bug.cgi?id=46074>
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
            if ( nLength != nMaxSize )
            {
                xFac = css::uno::Reference< css::lang::XSingleServiceFactory >(xOrderList, css::uno::UNO_QUERY);
                xInst = xFac->createInstance();
                ::rtl::OUString sPush = ::rtl::OUString::valueOf(nLength++);
                xOrderList->insertByName(sPush, css::uno::makeAny(xInst));
            }
            for (sal_Int32 j=nLength-1; j>0; --j)
            {
                xOrderList->getByName( ::rtl::OUString::valueOf(j) )   >>= xPrevSet;
                xOrderList->getByName( ::rtl::OUString::valueOf(j-1) ) >>= xNextSet;
                ::rtl::OUString sTemp;
                xNextSet->getPropertyValue(sHistoryItemRef) >>= sTemp;
                xPrevSet->setPropertyValue(sHistoryItemRef, css::uno::makeAny(sTemp));
            }
            xOrderList->getByName( ::rtl::OUString::valueOf((sal_Int32)0) ) >>= xSet;
            xSet->setPropertyValue(sHistoryItemRef, css::uno::makeAny(sURL));

            // Append the item to ItemList.
            xFac = css::uno::Reference< css::lang::XSingleServiceFactory >(xItemList, css::uno::UNO_QUERY);
            xInst = xFac->createInstance();
            xItemList->insertByName(sURL, css::uno::makeAny(xInst));
            xSet = css::uno::Reference< css::beans::XPropertySet >(xInst, css::uno::UNO_QUERY);
            xSet->setPropertyValue(rtl::OUString(s_sFilter), css::uno::makeAny(sFilter));
            xSet->setPropertyValue(rtl::OUString(s_sTitle), css::uno::makeAny(sTitle));
            xSet->setPropertyValue(rtl::OUString(s_sPassword), css::uno::makeAny(sPassword));

            ::comphelper::ConfigurationHelper::flush(m_xCfg);
        }
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("unotools", "Caught unexpected: " << ex.Message);
    }
}

//*****************************************************************************************************************
// initialize static member
// DON'T DO IT IN YOUR HEADER!
// see definition for further informations
//*****************************************************************************************************************
SvtHistoryOptions_Impl*  SvtHistoryOptions::m_pDataContainer = NULL ;
sal_Int32     SvtHistoryOptions::m_nRefCount  = 0  ;

//*****************************************************************************************************************
// constructor
//*****************************************************************************************************************
SvtHistoryOptions::SvtHistoryOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already exist!
    if( m_pDataContainer == NULL )
    {
        RTL_LOGFILE_CONTEXT(aLog, "unotools ( ??? ) ::SvtHistoryOptions_Impl::ctor()");
        m_pDataContainer = new SvtHistoryOptions_Impl;

        ItemHolder1::holdConfigItem(E_HISTORYOPTIONS);
    }
}

//*****************************************************************************************************************
// destructor
//*****************************************************************************************************************
SvtHistoryOptions::~SvtHistoryOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Decrease ouer refcount.
    --m_nRefCount;
    // If last instance was deleted ...
    // we must destroy ouer static data container!
    if( m_nRefCount <= 0 )
    {
        delete m_pDataContainer;
        m_pDataContainer = NULL;
    }
}

//*****************************************************************************************************************
// public method
//*****************************************************************************************************************
sal_uInt32 SvtHistoryOptions::GetSize( EHistoryType eHistory ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetSize( eHistory );
}

//*****************************************************************************************************************
// public method
//*****************************************************************************************************************
void SvtHistoryOptions::Clear( EHistoryType eHistory )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->Clear( eHistory );
}

//*****************************************************************************************************************
// public method
//*****************************************************************************************************************
Sequence< Sequence< PropertyValue > > SvtHistoryOptions::GetList( EHistoryType eHistory ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetList( eHistory );
}

//*****************************************************************************************************************
// public method
//*****************************************************************************************************************
void SvtHistoryOptions::AppendItem(   EHistoryType eHistory ,
                                   const OUString&  sURL  ,
                                   const OUString&  sFilter  ,
                                   const OUString&  sTitle  ,
                                   const OUString&  sPassword )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->AppendItem( eHistory, sURL, sFilter, sTitle, sPassword );
}

namespace
{
    class theHistoryOptionsMutex : public rtl::Static<osl::Mutex, theHistoryOptionsMutex>{};
}

//*****************************************************************************************************************
// private method
//*****************************************************************************************************************
Mutex& SvtHistoryOptions::GetOwnStaticMutex()
{
    return theHistoryOptionsMutex::get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
