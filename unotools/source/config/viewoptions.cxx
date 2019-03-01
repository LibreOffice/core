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

#include <unotools/viewoptions.hxx>
#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <unotools/configpaths.hxx>
#include <unotools/configmgr.hxx>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>

#include "itemholder1.hxx"

#define PACKAGE_VIEWS                           "org.openoffice.Office.Views"

#define LIST_DIALOGS                            "Dialogs"
#define LIST_TABDIALOGS                         "TabDialogs"
#define LIST_TABPAGES                           "TabPages"
#define LIST_WINDOWS                            "Windows"

#define PROPERTY_WINDOWSTATE                    "WindowState"
#define PROPERTY_PAGEID                         "PageID"
#define PROPERTY_VISIBLE                        "Visible"
#define PROPERTY_USERDATA                       "UserData"

//#define DEBUG_VIEWOPTIONS

#ifdef DEBUG_VIEWOPTIONS
    #define _LOG_COUNTER_( _SVIEW_, _NREAD_, _NWRITE_ )                                                                                     \
                {                                                                                                                           \
                    FILE* pFile = fopen( "viewdbg.txt", "a" );                                                                              \
                    fprintf( pFile, "%s[%d, %d]\n", OUStringToOString(_SVIEW_, RTL_TEXTENCODING_UTF8).getStr(), _NREAD_, _NWRITE_ ); \
                    fclose( pFile );                                                                                                        \
                }
#endif // DEBUG_VIEWOPTIONS

//  initialization!

SvtViewOptionsBase_Impl*     SvtViewOptions::m_pDataContainer_Dialogs    =   nullptr    ;
sal_Int32                    SvtViewOptions::m_nRefCount_Dialogs         =   0       ;
SvtViewOptionsBase_Impl*     SvtViewOptions::m_pDataContainer_TabDialogs =   nullptr    ;
sal_Int32                    SvtViewOptions::m_nRefCount_TabDialogs      =   0       ;
SvtViewOptionsBase_Impl*     SvtViewOptions::m_pDataContainer_TabPages   =   nullptr    ;
sal_Int32                    SvtViewOptions::m_nRefCount_TabPages        =   0       ;
SvtViewOptionsBase_Impl*     SvtViewOptions::m_pDataContainer_Windows    =   nullptr    ;
sal_Int32                    SvtViewOptions::m_nRefCount_Windows         =   0       ;

/*-************************************************************************************************************
    @descr          Implement base data container for view options elements.
                    Every item support ALL possible configuration information.
                    But not every superclass should use them! Because some view types don't
                    have it really.

    @attention      We implement a write-through cache! We use it for reading - but write all changes directly to
                    configuration. (changes are made on internal cache too!). So it's easier to distinguish
                    between added/changed/removed elements without any complex mask or bool flag information.
                    Caches from configuration and our own one are synchronized every time - if we do so.
*//*-*************************************************************************************************************/
class SvtViewOptionsBase_Impl final
{

    public:
        enum State { STATE_NONE, STATE_FALSE, STATE_TRUE };

        explicit SvtViewOptionsBase_Impl(const OUString& rList);
                 ~SvtViewOptionsBase_Impl (                                                                );
        bool                                            Exists                  ( const OUString&                                sName    );
        void                                            Delete                  ( const OUString&                                sName    );
        OUString                                        GetWindowState          ( const OUString&                                sName    );
        void                                            SetWindowState          ( const OUString&                                sName    ,
                                                                                  const OUString&                                sState   );
        css::uno::Sequence< css::beans::NamedValue >    GetUserData             ( const OUString&                                sName    );
        void                                            SetUserData             ( const OUString&                                sName    ,
                                                                                  const css::uno::Sequence< css::beans::NamedValue >&   lData    );
        OString                                         GetPageID               ( const OUString&                                sName    );
        void                                            SetPageID               ( const OUString&                                sName    ,
                                                                                  const OString&                                 sID      );
        State                                           GetVisible              ( const OUString&                                sName    );
        void                                            SetVisible              ( const OUString&                                sName    ,
                                                                                        bool                                        bVisible );
        css::uno::Any                                   GetUserItem             ( const OUString&                                sName    ,
                                                                                  const OUString&                                sItem    );
        void                                            SetUserItem             ( const OUString&                                sName    ,
                                                                                  const OUString&                                sItem    ,
                                                                                  const css::uno::Any&                                  aValue   );

    private:
        css::uno::Reference< css::uno::XInterface > impl_getSetNode( const OUString& sNode           ,
                                                                           bool         bCreateIfMissing);

    private:
        OUString const                                    m_sListName;
        css::uno::Reference< css::container::XNameAccess > m_xRoot;
        css::uno::Reference< css::container::XNameAccess > m_xSet;

        #ifdef DEBUG_VIEWOPTIONS
        sal_Int32           m_nReadCount;
        sal_Int32           m_nWriteCount;
        #endif
};

/*-************************************************************************************************************
    @descr  Implement the base data container.
*//*-*************************************************************************************************************/

/*-************************************************************************************************************
    @short          ctor
    @descr          We use it to open right configuration file and let configuration objects fill her caches.
                    Then we read all existing entries from right list and cached it inside our object too.
                    Normally we should enable notifications for changes on these values too ... but these feature
                    isn't full implemented in the moment.

    @seealso        baseclass ::utl::ConfigItem
    @seealso        method Notify()
*//*-*************************************************************************************************************/
SvtViewOptionsBase_Impl::SvtViewOptionsBase_Impl( const OUString& sList )
        :   m_sListName  ( sList )    // we must know, which view type we must support
        #ifdef DEBUG_VIEWOPTIONS
        ,   m_nReadCount ( 0     )
        ,   m_nWriteCount( 0     )
        #endif
{
    if (utl::ConfigManager::IsFuzzing())
        return;

    try
    {
        m_xRoot.set( ::comphelper::ConfigurationHelper::openConfig(
                            ::comphelper::getProcessComponentContext(),
                            PACKAGE_VIEWS,
                            ::comphelper::EConfigurationModes::Standard),
                     css::uno::UNO_QUERY);
        if (m_xRoot.is())
            m_xRoot->getByName(sList) >>= m_xSet;
    }
    catch(const css::uno::Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            m_xRoot.clear();
            m_xSet.clear();

            SAL_WARN("unotools", "Unexpected exception caught. " << exceptionToString(ex));
        }
}

/*-************************************************************************************************************
    @short          dtor
    @descr          clean up something

    @attention      We implement a write through cache! So we mustn't do it really. All changes was written to cfg directly.
                    Commit isn't necessary then.

    @seealso        baseclass ::utl::ConfigItem
    @seealso        method IsModified()
    @seealso        method SetModified()
    @seealso        method Commit()
*//*-*************************************************************************************************************/
SvtViewOptionsBase_Impl::~SvtViewOptionsBase_Impl()
{
    // don't flush configuration changes here to m_xRoot.
    // That must be done inside every SetXXX() method already !
    // Here its to late - DisposedExceptions from used configuration access can occur otherwise.

    m_xRoot.clear();
    m_xSet.clear();

    #ifdef DEBUG_VIEWOPTIONS
    _LOG_COUNTER_( m_sListName, m_nReadCount, m_nWriteCount )
    #endif // DEBUG_VIEWOPTIONS
}

/*-************************************************************************************************************
    @short          checks for already existing entries
    @descr          If user don't know, if an entry already exist - he can get this information by calling this method.

    @seealso        member m_aList

    @param          "sName", name of entry to check exist state
    @return         true , if item exist
                    false, otherwise
*//*-*************************************************************************************************************/
bool SvtViewOptionsBase_Impl::Exists( const OUString& sName )
{
    bool bExists = false;

    try
    {
        if (m_xSet.is())
            bExists = m_xSet->hasByName(sName);
    }
    catch(const css::uno::Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            bExists = false;
            SAL_WARN("unotools", "Unexpected exception caught. " << exceptionToString(ex));
        }

    return bExists;
}

/*-************************************************************************************************************
    @short          delete entry
    @descr          Use it to delete set entry by given name.

    @seealso        member m_aList

    @param          "sName", name of entry to delete it
*//*-*************************************************************************************************************/
void SvtViewOptionsBase_Impl::Delete( const OUString& sName )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nWriteCount;
    #endif

    try
    {
        css::uno::Reference< css::container::XNameContainer > xSet(m_xSet, css::uno::UNO_QUERY_THROW);
        xSet->removeByName(sName);
        ::comphelper::ConfigurationHelper::flush(m_xRoot);
    }
    catch(const css::container::NoSuchElementException&)
        { }
    catch(const css::uno::Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("unotools", "Unexpected exception caught. " << exceptionToString(ex));
        }
}

/*-************************************************************************************************************
    @short          read/write access to cache view items and her properties
    @descr          Follow methods support read/write access to all cache view items.

    @seealso        member m_sList
*//*-*************************************************************************************************************/
OUString SvtViewOptionsBase_Impl::GetWindowState( const OUString& sName )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nReadCount;
    #endif

    OUString sWindowState;
    try
    {
        css::uno::Reference< css::beans::XPropertySet > xNode(
            impl_getSetNode(sName, false),
            css::uno::UNO_QUERY);
        if (xNode.is())
            xNode->getPropertyValue(PROPERTY_WINDOWSTATE) >>= sWindowState;
    }
    catch(const css::uno::Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            sWindowState.clear();
            SAL_WARN("unotools", "Unexpected exception caught. " << exceptionToString(ex));
        }

    return sWindowState;
}

void SvtViewOptionsBase_Impl::SetWindowState( const OUString& sName  ,
                                              const OUString& sState )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nWriteCount;
    #endif

    try
    {
        css::uno::Reference< css::beans::XPropertySet > xNode(
            impl_getSetNode(sName, true),
            css::uno::UNO_QUERY_THROW);
        xNode->setPropertyValue(PROPERTY_WINDOWSTATE, css::uno::makeAny(sState));
        ::comphelper::ConfigurationHelper::flush(m_xRoot);
    }
    catch(const css::uno::Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("unotools", "Unexpected exception caught. " << exceptionToString(ex));
        }
}

css::uno::Sequence< css::beans::NamedValue > SvtViewOptionsBase_Impl::GetUserData( const OUString& sName )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nReadCount;
    #endif

    try
    {
        css::uno::Reference< css::container::XNameAccess > xNode(
            impl_getSetNode(sName, false),
            css::uno::UNO_QUERY); // no _THROW ! because we don't create missing items here. So we have to live with zero references .-)
        css::uno::Reference< css::container::XNameAccess > xUserData;
        if (xNode.is())
            xNode->getByName(PROPERTY_USERDATA) >>= xUserData;
        if (xUserData.is())
        {
            const css::uno::Sequence<OUString> lNames = xUserData->getElementNames();
            const OUString* pNames = lNames.getConstArray();
            sal_Int32 c = lNames.getLength();
            sal_Int32 i = 0;
            css::uno::Sequence< css::beans::NamedValue > lUserData(c);

            for (i=0; i<c; ++i)
            {
                lUserData[i].Name  = pNames[i];
                lUserData[i].Value = xUserData->getByName(pNames[i]);
            }

            return lUserData;
        }
    }
    catch(const css::uno::Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("unotools", "Unexpected exception caught. " << exceptionToString(ex));
        }

    return css::uno::Sequence< css::beans::NamedValue >();
}

void SvtViewOptionsBase_Impl::SetUserData( const OUString&                              sName  ,
                                           const css::uno::Sequence< css::beans::NamedValue >& lData  )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nWriteCount;
    #endif

    try
    {
        css::uno::Reference< css::container::XNameAccess > xNode(
            impl_getSetNode(sName, true),
            css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::container::XNameContainer > xUserData;
        xNode->getByName(PROPERTY_USERDATA) >>= xUserData;
        if (xUserData.is())
        {
            const css::beans::NamedValue* pData = lData.getConstArray();
            sal_Int32               c     = lData.getLength();
            sal_Int32               i     = 0;
            for (i=0; i<c; ++i)
            {
                if (xUserData->hasByName(pData[i].Name))
                    xUserData->replaceByName(pData[i].Name, pData[i].Value);
                else
                    xUserData->insertByName(pData[i].Name, pData[i].Value);
            }
        }
        ::comphelper::ConfigurationHelper::flush(m_xRoot);
    }
    catch(const css::uno::Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("unotools", "Unexpected exception caught. " << exceptionToString(ex));
        }
}

css::uno::Any SvtViewOptionsBase_Impl::GetUserItem( const OUString& sName ,
                                                    const OUString& sItem )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nReadCount;
    #endif

    css::uno::Any aItem;
    try
    {
        css::uno::Reference< css::container::XNameAccess > xNode(
            impl_getSetNode(sName, false),
            css::uno::UNO_QUERY);
        css::uno::Reference< css::container::XNameAccess > xUserData;
        if (xNode.is())
            xNode->getByName(PROPERTY_USERDATA) >>= xUserData;
        if (xUserData.is())
            aItem = xUserData->getByName(sItem);
    }
    catch(const css::container::NoSuchElementException&)
        { aItem.clear(); }
    catch(const css::uno::Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            aItem.clear();
            SAL_WARN("unotools", "Unexpected exception caught. " << exceptionToString(ex));
        }

    return aItem;
}

void SvtViewOptionsBase_Impl::SetUserItem( const OUString& sName  ,
                                           const OUString& sItem  ,
                                           const css::uno::Any&   aValue )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nWriteCount;
    #endif

    try
    {
        css::uno::Reference< css::container::XNameAccess > xNode(
            impl_getSetNode(sName, true),
            css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::container::XNameContainer > xUserData;
        xNode->getByName(PROPERTY_USERDATA) >>= xUserData;
        if (xUserData.is())
        {
            if (xUserData->hasByName(sItem))
                xUserData->replaceByName(sItem, aValue);
            else
                xUserData->insertByName(sItem, aValue);
        }
        ::comphelper::ConfigurationHelper::flush(m_xRoot);
    }
    catch(const css::uno::Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("unotools", "Unexpected exception caught. " << exceptionToString(ex));
        }
}

OString SvtViewOptionsBase_Impl::GetPageID( const OUString& sName )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nReadCount;
    #endif

    OUString sID;
    try
    {
        css::uno::Reference< css::beans::XPropertySet > xNode(
            impl_getSetNode(sName, false),
            css::uno::UNO_QUERY);
        if (xNode.is())
            xNode->getPropertyValue(PROPERTY_PAGEID) >>= sID;
    }
    catch(const css::uno::Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("unotools", "Unexpected exception caught. " << exceptionToString(ex));
        }

    return sID.toUtf8();
}

void SvtViewOptionsBase_Impl::SetPageID( const OUString& sName ,
                                         const OString& sID )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nWriteCount;
    #endif

    try
    {
        css::uno::Reference< css::beans::XPropertySet > xNode(
            impl_getSetNode(sName, true),
            css::uno::UNO_QUERY_THROW);
        xNode->setPropertyValue(PROPERTY_PAGEID, css::uno::makeAny(OUString::fromUtf8(sID)));
        ::comphelper::ConfigurationHelper::flush(m_xRoot);
    }
    catch(const css::uno::Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("unotools", "Unexpected exception caught. " << exceptionToString(ex));
        }
}

SvtViewOptionsBase_Impl::State SvtViewOptionsBase_Impl::GetVisible( const OUString& sName )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nReadCount;
    #endif

    State eState = STATE_NONE;
    try
    {
        css::uno::Reference< css::beans::XPropertySet > xNode(
            impl_getSetNode(sName, false),
            css::uno::UNO_QUERY);
        if (xNode.is())
        {
            bool bVisible = false;
            if (xNode->getPropertyValue(PROPERTY_VISIBLE) >>= bVisible)
            {
                eState = bVisible ? STATE_TRUE : STATE_FALSE;
            }
        }
    }
    catch(const css::uno::Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("unotools", "Unexpected exception caught. " << exceptionToString(ex));
        }

    return eState;
}

void SvtViewOptionsBase_Impl::SetVisible( const OUString& sName    ,
                                                bool         bVisible )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nWriteCount;
    #endif

    try
    {
        css::uno::Reference< css::beans::XPropertySet > xNode(
            impl_getSetNode(sName, true),
            css::uno::UNO_QUERY_THROW);
        xNode->setPropertyValue(PROPERTY_VISIBLE, css::uno::makeAny(bVisible));
        ::comphelper::ConfigurationHelper::flush(m_xRoot);
    }
    catch(const css::uno::Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("unotools", "Unexpected exception caught. " << exceptionToString(ex));
        }
}

/*-************************************************************************************************************
    @short          create new set node with default values on disk
    @descr          To create a new UserData item - the super node of these property must already exist!
                    You can call this method to create these new entry with default values and change UserData then.

    @seealso        method impl_writeDirectProp()

    @param          "sNode", name of new entry
*//*-*************************************************************************************************************/
css::uno::Reference< css::uno::XInterface > SvtViewOptionsBase_Impl::impl_getSetNode( const OUString& sNode           ,
                                                                                            bool         bCreateIfMissing)
{
    css::uno::Reference< css::uno::XInterface > xNode;

    try
    {
        if (bCreateIfMissing)
            xNode = ::comphelper::ConfigurationHelper::makeSureSetNodeExists(m_xRoot, m_sListName, sNode);
        else
        {
            if (m_xSet.is() && m_xSet->hasByName(sNode) )
                m_xSet->getByName(sNode) >>= xNode;
        }
    }
    catch(const css::container::NoSuchElementException&)
        { xNode.clear(); }
    catch(const css::uno::Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            xNode.clear();
            SAL_WARN("unotools", "Unexpected exception caught. " << exceptionToString(ex));
        }

    return xNode;
}

//  constructor

SvtViewOptions::SvtViewOptions(       EViewType        eType     ,
                                const OUString& sViewName )
    :   m_eViewType ( eType     )
    ,   m_sViewName ( sViewName )
{
    // Global access, must be guarded (multithreading!)
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    // Search for right dat container for this view type and initialize right data container or set right ref count!
    switch( eType )
    {
        case EViewType::Dialog: {
                                    // Increase ref count for dialog data container first.
                                    ++m_nRefCount_Dialogs;
                                    // If these instance the first user of the dialog data container - create these impl static container!
                                    if( m_nRefCount_Dialogs == 1 )
                                    {
                                        //m_pDataContainer_Dialogs = new SvtViewDialogOptions_Impl( LIST_DIALOGS );
                                        m_pDataContainer_Dialogs = new SvtViewOptionsBase_Impl( LIST_DIALOGS );
                                        ItemHolder1::holdConfigItem(EItem::ViewOptionsDialog);
                                    }
                                }
                                break;
        case EViewType::TabDialog: {
                                    // Increase ref count for tab-dialog data container first.
                                    ++m_nRefCount_TabDialogs;
                                    // If these instance the first user of the tab-dialog data container - create these impl static container!
                                    if( m_nRefCount_TabDialogs == 1 )
                                    {
                                        m_pDataContainer_TabDialogs = new SvtViewOptionsBase_Impl( LIST_TABDIALOGS );
                                        ItemHolder1::holdConfigItem(EItem::ViewOptionsTabDialog);
                                    }
                                }
                                break;
        case EViewType::TabPage:{
                                    // Increase ref count for tab-page data container first.
                                    ++m_nRefCount_TabPages;
                                    // If these instance the first user of the tab-page data container - create these impl static container!
                                    if( m_nRefCount_TabPages == 1 )
                                    {
                                        m_pDataContainer_TabPages = new SvtViewOptionsBase_Impl( LIST_TABPAGES );
                                        ItemHolder1::holdConfigItem(EItem::ViewOptionsTabPage);
                                    }
                                }
                                break;
        case EViewType::Window: {
                                    // Increase ref count for window data container first.
                                    ++m_nRefCount_Windows;
                                    // If these instance the first user of the window data container - create these impl static container!
                                    if( m_nRefCount_Windows == 1 )
                                    {
                                        m_pDataContainer_Windows = new SvtViewOptionsBase_Impl( LIST_WINDOWS );
                                        ItemHolder1::holdConfigItem(EItem::ViewOptionsWindow);
                                    }
                                }
                                break;
        default             :   OSL_FAIL( "SvtViewOptions::SvtViewOptions()\nThese view type is unknown! All following calls at these instance will do nothing!" );
    }
}

//  destructor

SvtViewOptions::~SvtViewOptions()
{
    // Global access, must be guarded (multithreading!)
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    // Search for right dat container for this view type and deinitialize right data container or set right ref count!
    switch( m_eViewType )
    {
        case EViewType::Dialog: {
                                    // Decrease ref count for dialog data container first.
                                    --m_nRefCount_Dialogs;
                                    // If these instance the last user of the dialog data container - delete these impl static container!
                                    if( m_nRefCount_Dialogs == 0 )
                                    {
                                        delete m_pDataContainer_Dialogs;
                                        m_pDataContainer_Dialogs = nullptr;
                                    }
                                }
                                break;
        case EViewType::TabDialog: {
                                    // Decrease ref count for tab-dialog data container first.
                                    --m_nRefCount_TabDialogs;
                                    // If these instance the last user of the tab-dialog data container - delete these impl static container!
                                    if( m_nRefCount_TabDialogs == 0 )
                                    {
                                        delete m_pDataContainer_TabDialogs;
                                        m_pDataContainer_TabDialogs = nullptr;
                                    }
                                }
                                break;
        case EViewType::TabPage:{
                                    // Decrease ref count for tab-page data container first.
                                    --m_nRefCount_TabPages;
                                    // If these instance the last user of the tab-page data container - delete these impl static container!
                                    if( m_nRefCount_TabPages == 0 )
                                    {
                                        delete m_pDataContainer_TabPages;
                                        m_pDataContainer_TabPages = nullptr;
                                    }
                                }
                                break;
        case EViewType::Window: {
                                    // Decrease ref count for window data container first.
                                    --m_nRefCount_Windows;
                                    // If these instance the last user of the window data container - delete these impl static container!
                                    if( m_nRefCount_Windows == 0 )
                                    {
                                        delete m_pDataContainer_Windows;
                                        m_pDataContainer_Windows = nullptr;
                                    }
                                }
                                break;
    }
}

//  public method

bool SvtViewOptions::Exists() const
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    bool bExists = false;
    switch( m_eViewType )
    {
        case EViewType::Dialog: {
                                    bExists = m_pDataContainer_Dialogs->Exists( m_sViewName );
                                }
                                break;
        case EViewType::TabDialog: {
                                    bExists = m_pDataContainer_TabDialogs->Exists( m_sViewName );
                                }
                                break;
        case EViewType::TabPage:{
                                    bExists = m_pDataContainer_TabPages->Exists( m_sViewName );
                                }
                                break;
        case EViewType::Window: {
                                    bExists = m_pDataContainer_Windows->Exists( m_sViewName );
                                }
                                break;
    }
    return bExists;
}

//  public method

void SvtViewOptions::Delete()
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    switch( m_eViewType )
    {
        case EViewType::Dialog    :  m_pDataContainer_Dialogs->Delete( m_sViewName );
                            break;
        case EViewType::TabDialog :  m_pDataContainer_TabDialogs->Delete( m_sViewName );
                            break;
        case EViewType::TabPage   :  m_pDataContainer_TabPages->Delete( m_sViewName );
                            break;
        case EViewType::Window    :  m_pDataContainer_Windows->Delete( m_sViewName );
                            break;
    }
}

//  public method

OUString SvtViewOptions::GetWindowState() const
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    OUString sState;
    switch( m_eViewType )
    {
        case EViewType::Dialog: {
                                    sState = m_pDataContainer_Dialogs->GetWindowState( m_sViewName );
                                }
                                break;
        case EViewType::TabDialog:{
                                    sState = m_pDataContainer_TabDialogs->GetWindowState( m_sViewName );
                                }
                                break;
        case EViewType::TabPage:{
                                    sState = m_pDataContainer_TabPages->GetWindowState( m_sViewName );
                                }
                                break;
        case EViewType::Window: {
                                    sState = m_pDataContainer_Windows->GetWindowState( m_sViewName );
                                }
                                break;
    }
    return sState;
}

//  public method

void SvtViewOptions::SetWindowState( const OUString& sState )
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    switch( m_eViewType )
    {
        case EViewType::Dialog: {
                                    m_pDataContainer_Dialogs->SetWindowState( m_sViewName, sState );
                                }
                                break;
        case EViewType::TabDialog: {
                                    m_pDataContainer_TabDialogs->SetWindowState( m_sViewName, sState );
                                }
                                break;
        case EViewType::TabPage:{
                                    m_pDataContainer_TabPages->SetWindowState( m_sViewName, sState );
                                }
                                break;
        case EViewType::Window: {
                                    m_pDataContainer_Windows->SetWindowState( m_sViewName, sState );
                                }
                                break;
    }
}

//  public method

OString SvtViewOptions::GetPageID() const
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    // Safe impossible cases.
    // These call isn't allowed for dialogs, tab-pages or windows!
    OSL_ENSURE( !(m_eViewType==EViewType::Dialog||m_eViewType==EViewType::TabPage||m_eViewType==EViewType::Window), "SvtViewOptions::GetPageID()\nCall not allowed for Dialogs, TabPages or Windows! I do nothing!" );

    OString sID;
    if( m_eViewType == EViewType::TabDialog )
        sID = m_pDataContainer_TabDialogs->GetPageID( m_sViewName );
    return sID;
}

//  public method

void SvtViewOptions::SetPageID(const OString& rID)
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    // Safe impossible cases.
    // These call isn't allowed for dialogs, tab-pages or windows!
    OSL_ENSURE( !(m_eViewType==EViewType::Dialog||m_eViewType==EViewType::TabPage||m_eViewType==EViewType::Window), "SvtViewOptions::SetPageID()\nCall not allowed for Dialogs, TabPages or Windows! I do nothing!" );

    if( m_eViewType == EViewType::TabDialog )
        m_pDataContainer_TabDialogs->SetPageID(m_sViewName, rID);
}

//  public method

bool SvtViewOptions::IsVisible() const
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    // Safe impossible cases.
    // These call isn't allowed for dialogs, tab-dialogs or tab-pages!
    OSL_ENSURE( !(m_eViewType==EViewType::Dialog||m_eViewType==EViewType::TabDialog||m_eViewType==EViewType::TabPage), "SvtViewOptions::IsVisible()\nCall not allowed for Dialogs, TabDialogs or TabPages! I do nothing!" );

    bool bState = false;
    if( m_eViewType == EViewType::Window )
        bState = m_pDataContainer_Windows->GetVisible( m_sViewName ) == SvtViewOptionsBase_Impl::STATE_TRUE;

    return bState;
}

//  public method

void SvtViewOptions::SetVisible( bool bState )
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    // Safe impossible cases.
    // These call isn't allowed for dialogs, tab-dialogs or tab-pages!
    OSL_ENSURE( !(m_eViewType==EViewType::Dialog||m_eViewType==EViewType::TabDialog||m_eViewType==EViewType::TabPage), "SvtViewOptions::SetVisible()\nCall not allowed for Dialogs, TabDialogs or TabPages! I do nothing!" );

    if( m_eViewType == EViewType::Window )
        m_pDataContainer_Windows->SetVisible( m_sViewName, bState );
}

//  public method

bool SvtViewOptions::HasVisible() const
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    // Safe impossible cases.
    // These call isn't allowed for dialogs, tab-dialogs or tab-pages!
    OSL_ENSURE( !(m_eViewType==EViewType::Dialog||m_eViewType==EViewType::TabDialog||m_eViewType==EViewType::TabPage), "SvtViewOptions::IsVisible()\nCall not allowed for Dialogs, TabDialogs or TabPages! I do nothing!" );

    bool bState = false;
    if( m_eViewType == EViewType::Window )
        bState = m_pDataContainer_Windows->GetVisible( m_sViewName ) != SvtViewOptionsBase_Impl::STATE_NONE;

    return bState;
}

css::uno::Sequence< css::beans::NamedValue > SvtViewOptions::GetUserData() const
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    css::uno::Sequence< css::beans::NamedValue > lData;
    switch( m_eViewType )
    {
        case EViewType::Dialog: {
                                    lData = m_pDataContainer_Dialogs->GetUserData( m_sViewName );
                                }
                                break;
        case EViewType::TabDialog: {
                                    lData = m_pDataContainer_TabDialogs->GetUserData( m_sViewName );
                                }
                                break;
        case EViewType::TabPage:{
                                    lData = m_pDataContainer_TabPages->GetUserData( m_sViewName );
                                }
                                break;
        case EViewType::Window: {
                                    lData = m_pDataContainer_Windows->GetUserData( m_sViewName );
                                }
                                break;
    }
    return lData;
}

void SvtViewOptions::SetUserData( const css::uno::Sequence< css::beans::NamedValue >& lData )
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    switch( m_eViewType )
    {
        case EViewType::Dialog: {
                                    m_pDataContainer_Dialogs->SetUserData( m_sViewName, lData );
                                }
                                break;
        case EViewType::TabDialog: {
                                    m_pDataContainer_TabDialogs->SetUserData( m_sViewName, lData );
                                }
                                break;
        case EViewType::TabPage:{
                                    m_pDataContainer_TabPages->SetUserData( m_sViewName, lData );
                                }
                                break;
        case EViewType::Window: {
                                    m_pDataContainer_Windows->SetUserData( m_sViewName, lData );
                                }
                                break;
    }
}

css::uno::Any SvtViewOptions::GetUserItem( const OUString& sName ) const
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    css::uno::Any aItem;
    switch( m_eViewType )
    {
        case EViewType::Dialog: {
                                    aItem = m_pDataContainer_Dialogs->GetUserItem( m_sViewName, sName );
                                }
                                break;
        case EViewType::TabDialog: {
                                    aItem = m_pDataContainer_TabDialogs->GetUserItem( m_sViewName, sName );
                                }
                                break;
        case EViewType::TabPage:{
                                    aItem = m_pDataContainer_TabPages->GetUserItem( m_sViewName, sName );
                                }
                                break;
        case EViewType::Window: {
                                    aItem = m_pDataContainer_Windows->GetUserItem( m_sViewName, sName );
                                }
                                break;
    }
    return aItem;
}

void SvtViewOptions::SetUserItem( const OUString& sName  ,
                                  const css::uno::Any&   aValue )
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    switch( m_eViewType )
    {
        case EViewType::Dialog: {
                                    m_pDataContainer_Dialogs->SetUserItem( m_sViewName, sName, aValue );
                                }
                                break;
        case EViewType::TabDialog: {
                                    m_pDataContainer_TabDialogs->SetUserItem( m_sViewName, sName, aValue );
                                }
                                break;
        case EViewType::TabPage:{
                                    m_pDataContainer_TabPages->SetUserItem( m_sViewName, sName, aValue );
                                }
                                break;
        case EViewType::Window: {
                                    m_pDataContainer_Windows->SetUserItem( m_sViewName, sName, aValue );
                                }
                                break;
    }
}

namespace
{
    class theViewOptionsMutex : public rtl::Static<osl::Mutex, theViewOptionsMutex>{};
}

//  private method

::osl::Mutex& SvtViewOptions::GetOwnStaticMutex()
{
    return theViewOptionsMutex::get();
}

void SvtViewOptions::AcquireOptions()
{
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );
    if( ++m_nRefCount_Dialogs == 1 )
    {
        m_pDataContainer_Dialogs = new SvtViewOptionsBase_Impl( LIST_DIALOGS );
        ItemHolder1::holdConfigItem(EItem::ViewOptionsDialog);
    }
    if( ++m_nRefCount_TabDialogs == 1 )
    {
        m_pDataContainer_TabDialogs = new SvtViewOptionsBase_Impl( LIST_TABDIALOGS );
        ItemHolder1::holdConfigItem(EItem::ViewOptionsTabDialog);
    }
    if( ++m_nRefCount_TabPages == 1 )
    {
        m_pDataContainer_TabPages = new SvtViewOptionsBase_Impl( LIST_TABPAGES );
        ItemHolder1::holdConfigItem(EItem::ViewOptionsTabPage);
    }
    if( ++m_nRefCount_Windows == 1 )
    {
        m_pDataContainer_Windows = new SvtViewOptionsBase_Impl( LIST_WINDOWS );
        ItemHolder1::holdConfigItem(EItem::ViewOptionsWindow);
    }
}

void SvtViewOptions::ReleaseOptions()
{
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );
    if( --m_nRefCount_Dialogs == 0 )
    {
        delete m_pDataContainer_Dialogs;
        m_pDataContainer_Dialogs = nullptr;
    }
    if( --m_nRefCount_TabDialogs == 0 )
    {
        delete m_pDataContainer_TabDialogs;
        m_pDataContainer_TabDialogs = nullptr;
    }
    if( --m_nRefCount_TabPages == 0 )
    {
        delete m_pDataContainer_TabPages;
        m_pDataContainer_TabPages = nullptr;
    }
    if( --m_nRefCount_Windows == 0 )
    {
        delete m_pDataContainer_Windows;
        m_pDataContainer_Windows = nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
