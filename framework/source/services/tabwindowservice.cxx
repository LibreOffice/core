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

#include <services/tabwindowservice.hxx>
#include <classes/fwktabwindow.hxx>
#include <threadhelp/resetableguard.hxx>
#include <services.h>
#include <properties.h>

#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>

namespace framework{

//*****************************************************************************************************************
//  css::uno::XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************

DEFINE_XINTERFACE_6                 (   TabWindowService                                ,
                                        OWeakObject                                     ,
                                        DIRECT_INTERFACE(css::lang::XTypeProvider      ),
                                        DIRECT_INTERFACE(css::lang::XServiceInfo       ),
                                        DIRECT_INTERFACE(css::lang::XComponent),
                                        DIRECT_INTERFACE(css::awt::XSimpleTabController),
                                        DIRECT_INTERFACE(css::beans::XPropertySet      ),
                                        DIRECT_INTERFACE(css::beans::XPropertySetInfo  )
                                    )

DEFINE_XTYPEPROVIDER_6              (   TabWindowService               ,
                                        css::lang::XTypeProvider       ,
                                        css::lang::XServiceInfo        ,
                                        css::lang::XComponent          ,
                                        css::awt::XSimpleTabController ,
                                        css::beans::XPropertySet       ,
                                        css::beans::XPropertySetInfo
                                    )

DEFINE_XSERVICEINFO_MULTISERVICE    (   TabWindowService                   ,
                                        OWeakObject                        ,
                                        SERVICENAME_TABWINDOWSERVICE       ,
                                        IMPLEMENTATIONNAME_TABWINDOWSERVICE
                                    )

DEFINE_INIT_SERVICE                 (   TabWindowService,
                                        {
                                            impl_initializePropInfo();
                                            m_aTransactionManager.setWorkingMode( E_WORK );
                                        }
                                    )

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
TabWindowService::TabWindowService( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory )
        //  Init baseclasses first
        //  Attention:
        //      Don't change order of initialization!
        //      ThreadHelpBase is a struct with a mutex as member. We can't use a mutex as member, while
        //      we must garant right initialization and a valid value of this! First initialize
        //      baseclasses and then members. And we need the mutex for other baseclasses !!!
        :   ThreadHelpBase          ( &Application::GetSolarMutex() )
        ,   TransactionBase         (                               )
        ,   PropertySetHelper       ( &m_aLock                      ,
                                      &m_aTransactionManager        ,
                                      sal_False                     ) // sal_False => dont release shared mutex on calling us!
        ,   OWeakObject             (                               )

        // Init member
        ,   m_xFactory              ( xFactory                      )
        ,   m_xTabWin               (                               )
        ,   m_pTabWin               ( NULL                          )
        ,   m_lTabPageInfos         (                               )
        ,   m_lListener             ( m_aLock.getShareableOslMutex())
        ,   m_nPageIndexCounter     ( 1                             )
        ,   m_nCurrentPageIndex     ( 0                             )
{
    // Safe impossible cases.
    // Method not defined for all incoming parameter.
    SAL_WARN_IF( !xFactory.is(), "fwk", "TabWindowService::TabWindowService(): Invalid parameter detected!" );
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
TabWindowService::~TabWindowService()
{
    // SAFE->
    ResetableGuard aGuard(m_aLock);

    if (m_pTabWin)
        m_pTabWin->RemoveEventListener( LINK( this, TabWindowService, EventListener ) );
}

//*****************************************************************************************************************
//  XSimpleTabController
//*****************************************************************************************************************
::sal_Int32 SAL_CALL TabWindowService::insertTab()
    throw ( css::uno::RuntimeException )
{
    // SAFE ->
    ResetableGuard aGuard( m_aLock );

    ::sal_Int32  nID  = m_nPageIndexCounter++;
    TTabPageInfo aInfo(nID);

    m_lTabPageInfos[nID] = aInfo;

    return nID;
}

//*****************************************************************************************************************
//  XSimpleTabController
//*****************************************************************************************************************
void SAL_CALL TabWindowService::removeTab(::sal_Int32 nID)
    throw (css::lang::IndexOutOfBoundsException,
           css::uno::RuntimeException          )
{
    // SAFE ->
    ResetableGuard aGuard(m_aLock);

    // throws suitable IndexOutOfBoundsException .-)
    TTabPageInfoHash::iterator pIt = impl_getTabPageInfo (nID);
    m_lTabPageInfos.erase(pIt);

    FwkTabWindow* pTabWin = mem_TabWin ();
    if (pTabWin)
        pTabWin->RemovePage(nID);
}

//*****************************************************************************************************************
//  XSimpleTabController
//*****************************************************************************************************************
void SAL_CALL TabWindowService::setTabProps(      ::sal_Int32                                   nID        ,
                                              const css::uno::Sequence< css::beans::NamedValue >& lProperties)
    throw (css::lang::IndexOutOfBoundsException,
           css::uno::RuntimeException          )
{
    // SAFE ->
    ResetableGuard aGuard(m_aLock);

    // throws suitable IndexOutOfBoundsException .-)
    TTabPageInfoHash::iterator pIt   = impl_getTabPageInfo (nID);
    TTabPageInfo&              rInfo = pIt->second;
    rInfo.m_lProperties = lProperties;

    if ( ! rInfo.m_bCreated)
    {
        FwkTabWindow* pTabWin = mem_TabWin ();
        if (pTabWin)
        {
            pTabWin->AddTabPage(rInfo.m_nIndex, rInfo.m_lProperties);
            rInfo.m_bCreated = sal_True;
        }
    }
}

//*****************************************************************************************************************
//  XSimpleTabController
//*****************************************************************************************************************
css::uno::Sequence< css::beans::NamedValue > SAL_CALL TabWindowService::getTabProps(::sal_Int32 nID)
    throw (css::lang::IndexOutOfBoundsException,
           css::uno::RuntimeException          )
{
    // SAFE ->
    ResetableGuard aGuard(m_aLock);

    // throws suitable IndexOutOfBoundsException .-)
    TTabPageInfoHash::const_iterator pIt   = impl_getTabPageInfo (nID);
    const TTabPageInfo&              rInfo = pIt->second;

    return rInfo.m_lProperties;
}

//*****************************************************************************************************************
//  XSimpleTabController
//*****************************************************************************************************************
void SAL_CALL TabWindowService::activateTab(::sal_Int32 nID)
    throw (css::lang::IndexOutOfBoundsException,
           css::uno::RuntimeException          )
{
    // SAFE ->
    ResetableGuard aGuard(m_aLock);

    // throws suitable IndexOutOfBoundsException .-)
    impl_checkTabIndex (nID);
    m_nCurrentPageIndex = nID;

    FwkTabWindow* pTabWin = mem_TabWin ();
    if (pTabWin)
        pTabWin->ActivatePage(nID);
}

//*****************************************************************************************************************
//  XSimpleTabController
//*****************************************************************************************************************
::sal_Int32 SAL_CALL TabWindowService::getActiveTabID()
    throw (css::uno::RuntimeException)
{
    // SAFE->
    ResetableGuard aGuard( m_aLock );
    return m_nCurrentPageIndex;
}

//*****************************************************************************************************************
//  XSimpleTabController
//*****************************************************************************************************************
void SAL_CALL TabWindowService::addTabListener(const css::uno::Reference< css::awt::XTabListener >& xListener)
    throw (css::uno::RuntimeException)
{
    m_lListener.addInterface(::getCppuType((const css::uno::Reference< css::awt::XTabListener >*)NULL), xListener);
}

//*****************************************************************************************************************
//  XSimpleTabController
//*****************************************************************************************************************
void SAL_CALL TabWindowService::removeTabListener(const css::uno::Reference< css::awt::XTabListener >& xListener)
    throw (css::uno::RuntimeException)
{
    m_lListener.removeInterface(::getCppuType((const css::uno::Reference< css::awt::XTabListener >*)NULL), xListener);
}

//*****************************************************************************************************************
//  XComponent
//*****************************************************************************************************************
void SAL_CALL TabWindowService::dispose()
    throw (css::uno::RuntimeException)
{
    // SAFE->
    ResetableGuard aGuard(m_aLock);

    css::uno::Reference< css::uno::XInterface > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
    css::lang::EventObject aEvent(xThis);

    m_lListener.disposeAndClear (aEvent);

    if (m_pTabWin)
        m_pTabWin->RemoveEventListener( LINK( this, TabWindowService, EventListener ) );

    m_pTabWin = NULL;
    m_xTabWin.clear();
}

//*****************************************************************************************************************
//  XComponent
//*****************************************************************************************************************
void SAL_CALL TabWindowService::addEventListener(const css::uno::Reference< css::lang::XEventListener >& xListener)
    throw (css::uno::RuntimeException)
{
    m_lListener.addInterface(::getCppuType((const css::uno::Reference< css::lang::XEventListener >*)NULL), xListener);
}

//*****************************************************************************************************************
//  XComponent
//*****************************************************************************************************************
void SAL_CALL TabWindowService::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener)
    throw (css::uno::RuntimeException)
{
    m_lListener.removeInterface(::getCppuType((const css::uno::Reference< css::lang::XEventListener >*)NULL), xListener);
}

//*****************************************************************************************************************
void TabWindowService::impl_initializePropInfo()
{
    impl_setPropertyChangeBroadcaster(static_cast< css::awt::XSimpleTabController* >(this));

    impl_addPropertyInfo(
        css::beans::Property(
            OUString("Window"),
            TABWINDOWSERVICE_PROPHANDLE_WINDOW,
            ::getCppuType((const css::uno::Reference< css::awt::XWindow >*)NULL),
            css::beans::PropertyAttribute::TRANSIENT));
}

//*****************************************************************************************************************
void SAL_CALL TabWindowService::impl_setPropertyValue(const OUString& /*sProperty*/,
                                                              sal_Int32        /*nHandle  */,
                                                        const css::uno::Any&   /*aValue   */)

{
}

//*****************************************************************************************************************
css::uno::Any SAL_CALL TabWindowService::impl_getPropertyValue(const OUString& /*sProperty*/,
                                                                       sal_Int32        nHandle      )
{
    /* There is no need to lock any mutex here. Because we share the
       solar mutex with our base class. And we said to our base class: "dont release it on calling us" .-)
       see ctor of PropertySetHelper for further information.
    */
    css::uno::Any aValue;

    switch (nHandle)
    {
        case TABWINDOWSERVICE_PROPHANDLE_WINDOW:
            {
                mem_TabWin (); // force "creation on demand" of m_xTabWin :-)
                aValue <<= m_xTabWin;
            }
            break;
    }

    return aValue;
}

//*****************************************************************************************************************
//  TabWindowService
//*****************************************************************************************************************
IMPL_LINK( TabWindowService, EventListener, VclSimpleEvent*, pEvent )
{

    if ( !pEvent && !pEvent->ISA(VclWindowEvent))
        return 0;

    sal_uLong           nEventId = pEvent->GetId();
    VclWindowEvent* pWinEvt  = static_cast< VclWindowEvent* >(pEvent);

    css::uno::Reference< css::uno::XInterface > xThis ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::lang::EventObject aEvent( xThis );

    if (nEventId == VCLEVENT_OBJECT_DYING)
    {
        m_lListener.disposeAndClear (aEvent);

        m_pTabWin->RemoveEventListener( LINK( this, TabWindowService, EventListener ) );
        m_pTabWin = NULL;
        m_xTabWin.clear();

        return 0;
    }

    ::cppu::OInterfaceContainerHelper* pContainer = m_lListener.getContainer(::getCppuType((const css::uno::Reference< css::awt::XTabListener >*) NULL));
    if ( ! pContainer)
        return 0;

    ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
    while (pIterator.hasMoreElements())
    {
        try
        {
            css::awt::XTabListener* pListener = (css::awt::XTabListener*)pIterator.next();

            switch (nEventId)
            {
                case VCLEVENT_TABPAGE_ACTIVATE :
                    pListener->activated( (sal_Int32)(sal_uLong)pWinEvt->GetData() );
                    break;

                case VCLEVENT_TABPAGE_DEACTIVATE :
                    pListener->deactivated( (sal_Int32)(sal_uLong)pWinEvt->GetData() );
                    break;

                case VCLEVENT_TABPAGE_INSERTED :
                    pListener->inserted( (sal_Int32)(sal_uLong)pWinEvt->GetData() );
                    break;

                case VCLEVENT_TABPAGE_REMOVED :
                    pListener->removed( (sal_Int32)(sal_uLong)pWinEvt->GetData() );
                    break;

                case VCLEVENT_TABPAGE_PAGETEXTCHANGED :
                case VCLEVENT_TABPAGE_REMOVEDALL :
                    break;
            }
        }
        catch(const css::uno::RuntimeException&)
        {
            pIterator.remove();
        }
    }

    return 0;
}

//*****************************************************************************************************************
//  TabWindowService
//*****************************************************************************************************************
void TabWindowService::impl_checkTabIndex (::sal_Int32 nID)
    throw (css::lang::IndexOutOfBoundsException)
{
    if (
        (nID <= 0                  ) ||
        (nID >  m_nPageIndexCounter)
       )
    {
        throw css::lang::IndexOutOfBoundsException(
                OUString("Tab index out of bounds."),
                css::uno::Reference< css::uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY ));
    }
}

//*****************************************************************************************************************
//  TabWindowService
//*****************************************************************************************************************
TTabPageInfoHash::iterator TabWindowService::impl_getTabPageInfo(::sal_Int32 nID)
    throw (css::lang::IndexOutOfBoundsException)
{
    TTabPageInfoHash::iterator pIt = m_lTabPageInfos.find(nID);
    if (pIt == m_lTabPageInfos.end ())
        throw css::lang::IndexOutOfBoundsException(
                OUString("Tab index out of bounds."),
                css::uno::Reference< css::uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY ));
    return pIt;
}

//  TabWindowService
FwkTabWindow* TabWindowService::mem_TabWin ()
{
    FwkTabWindow* pWin = NULL;

    if ( ! m_xTabWin.is ())
    {
        Window* pFakeParent = dynamic_cast< Window* >(Application::GetDefaultDevice ());

        m_pTabWin = new FwkTabWindow (pFakeParent);
        m_xTabWin = VCLUnoHelper::GetInterface (m_pTabWin);

        m_pTabWin->AddEventListener( LINK( this, TabWindowService, EventListener ) );
    }

    if (m_xTabWin.is ())
        pWin = m_pTabWin;

    return pWin;
}

} //    namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
