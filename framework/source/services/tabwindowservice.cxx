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

#include <classes/fwktabwindow.hxx>
#include <services.h>
#include <properties.h>

#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XSimpleTabController.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <classes/propertysethelper.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <general.h>
#include <stdtypes.h>

using namespace framework;

namespace {

struct TTabPageInfo
{
    public:

        TTabPageInfo()
            : m_nIndex      ( -1      )
            , m_bCreated    (false)
            , m_pPage       ( NULL    )
            , m_lProperties (         )
        {}

        TTabPageInfo(::sal_Int32 nID)
            : m_nIndex      ( nID     )
            , m_bCreated    (false)
            , m_pPage       ( NULL    )
            , m_lProperties (         )
        {}

    public:

        ::sal_Int32                                   m_nIndex;
        bool                                          m_bCreated;
        VclPtr<FwkTabPage>                            m_pPage;
        css::uno::Sequence< css::beans::NamedValue >  m_lProperties;
};

typedef std::unordered_map< ::sal_Int32                    ,
                            TTabPageInfo                   ,
                            Int32HashCode                  ,
                            std::equal_to< ::sal_Int32 > > TTabPageInfoHash;

/*-************************************************************************************************************
    @short  implements a helper service providing a dockable tab control window
*//*-*************************************************************************************************************/

class TabWindowService :  public css::lang::XTypeProvider
                         ,  public css::lang::XServiceInfo
                         ,  public css::awt::XSimpleTabController
                         ,  public css::lang::XComponent
                         ,  public TransactionBase
                         , private cppu::BaseMutex
                         ,  public PropertySetHelper
                         ,  public ::cppu::OWeakObject
{
public:
    TabWindowService();
    virtual ~TabWindowService();

    /// Initialization function after having acquire()'d.
    void initProperties();

    FWK_DECLARE_XINTERFACE
    FWK_DECLARE_XTYPEPROVIDER

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return OUString("com.sun.star.comp.framework.TabWindowService");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override
    {
        css::uno::Sequence< OUString > aSeq(1);
        aSeq[0] = "com.sun.star.ui.dialogs.TabContainerWindow";
        return aSeq;
    }

    //  XSimpleTabController

    virtual sal_Int32 SAL_CALL insertTab() throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeTab( sal_Int32 nID ) throw ( css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setTabProps( sal_Int32 nID, const css::uno::Sequence< css::beans::NamedValue >& aProperties ) throw ( css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::beans::NamedValue > SAL_CALL getTabProps( sal_Int32 nID ) throw ( css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL activateTab( sal_Int32 nID ) throw ( css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int32 SAL_CALL getActiveTabID() throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addTabListener( const css::uno::Reference< css::awt::XTabListener >& Listener ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeTabListener( const css::uno::Reference< css::awt::XTabListener >& Listener ) throw ( css::uno::RuntimeException, std::exception ) override;

    //  XComponent

    virtual void SAL_CALL dispose() throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw ( css::uno::RuntimeException, std::exception ) override;

private:

    void impl_initializePropInfo();
    virtual void SAL_CALL impl_setPropertyValue(const OUString& sProperty,
                                                      sal_Int32        nHandle  ,
                                                const css::uno::Any&   aValue   ) override;
    virtual css::uno::Any SAL_CALL impl_getPropertyValue(const OUString& sProperty,
                                                               sal_Int32        nHandle  ) override;

    DECL_DLLPRIVATE_LINK_TYPED( EventListener, VclWindowEvent&, void );

    void impl_checkTabIndex (::sal_Int32 nID) throw (css::lang::IndexOutOfBoundsException);
    TTabPageInfoHash::iterator impl_getTabPageInfo(::sal_Int32 nID) throw (css::lang::IndexOutOfBoundsException);
    FwkTabWindow* mem_TabWin ();

private:

    /// the tab window as XWindow ( to hold window* alive !)
    css::uno::Reference< css::awt::XWindow > m_xTabWin;

    /// the VCL tab window
    VclPtr<FwkTabWindow> m_pTabWin;

    /// container of inserted tab pages
    TTabPageInfoHash m_lTabPageInfos;

    /// container of the added TabListener
    ::cppu::OMultiTypeInterfaceContainerHelper m_lListener;

    /// counter of the tabpage indexes
    ::sal_Int32 m_nPageIndexCounter;

    /// index of the current active page
    ::sal_Int32 m_nCurrentPageIndex;
};

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

//  constructor

TabWindowService::TabWindowService()
        :   TransactionBase         (                               )
        ,   PropertySetHelper       ( m_aMutex,
                                      &m_aTransactionManager        ,
                                      false                     ) // sal_False => don't release shared mutex on calling us!
        ,   OWeakObject             (                               )

        // Init member
        ,   m_xTabWin               (                               )
        ,   m_pTabWin               ( NULL                          )
        ,   m_lTabPageInfos         (                               )
        ,   m_lListener             ( m_aMutex )
        ,   m_nPageIndexCounter     ( 1                             )
        ,   m_nCurrentPageIndex     ( 0                             )
{
}

void TabWindowService::initProperties()
{
    impl_initializePropInfo();
    m_aTransactionManager.setWorkingMode( E_WORK );
}

//  destructor

TabWindowService::~TabWindowService()
{
    SolarMutexGuard g;
    if (m_pTabWin)
        m_pTabWin->RemoveEventListener( LINK( this, TabWindowService, EventListener ) );
}

//  XSimpleTabController

::sal_Int32 SAL_CALL TabWindowService::insertTab()
    throw ( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard g;

    ::sal_Int32  nID  = m_nPageIndexCounter++;
    TTabPageInfo aInfo(nID);

    m_lTabPageInfos[nID] = aInfo;

    return nID;
}

//  XSimpleTabController

void SAL_CALL TabWindowService::removeTab(::sal_Int32 nID)
    throw (css::lang::IndexOutOfBoundsException,
           css::uno::RuntimeException, std::exception          )
{
    SolarMutexGuard g;

    // throws suitable IndexOutOfBoundsException .-)
    TTabPageInfoHash::iterator pIt = impl_getTabPageInfo (nID);
    m_lTabPageInfos.erase(pIt);

    FwkTabWindow* pTabWin = mem_TabWin ();
    if (pTabWin)
        pTabWin->RemovePage(nID);
}

//  XSimpleTabController

void SAL_CALL TabWindowService::setTabProps(      ::sal_Int32                                   nID        ,
                                              const css::uno::Sequence< css::beans::NamedValue >& lProperties)
    throw (css::lang::IndexOutOfBoundsException,
           css::uno::RuntimeException, std::exception          )
{
    SolarMutexGuard g;

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
            rInfo.m_bCreated = true;
        }
    }
}

//  XSimpleTabController

css::uno::Sequence< css::beans::NamedValue > SAL_CALL TabWindowService::getTabProps(::sal_Int32 nID)
    throw (css::lang::IndexOutOfBoundsException,
           css::uno::RuntimeException, std::exception          )
{
    SolarMutexGuard g;

    // throws suitable IndexOutOfBoundsException .-)
    TTabPageInfoHash::const_iterator pIt   = impl_getTabPageInfo (nID);
    const TTabPageInfo&              rInfo = pIt->second;

    return rInfo.m_lProperties;
}

//  XSimpleTabController

void SAL_CALL TabWindowService::activateTab(::sal_Int32 nID)
    throw (css::lang::IndexOutOfBoundsException,
           css::uno::RuntimeException, std::exception          )
{
    SolarMutexGuard g;

    // throws suitable IndexOutOfBoundsException .-)
    impl_checkTabIndex (nID);
    m_nCurrentPageIndex = nID;

    FwkTabWindow* pTabWin = mem_TabWin ();
    if (pTabWin)
        pTabWin->ActivatePage(nID);
}

//  XSimpleTabController

::sal_Int32 SAL_CALL TabWindowService::getActiveTabID()
    throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;
    return m_nCurrentPageIndex;
}

//  XSimpleTabController

void SAL_CALL TabWindowService::addTabListener(const css::uno::Reference< css::awt::XTabListener >& xListener)
    throw (css::uno::RuntimeException, std::exception)
{
    m_lListener.addInterface(cppu::UnoType<css::awt::XTabListener>::get(), xListener);
}

//  XSimpleTabController

void SAL_CALL TabWindowService::removeTabListener(const css::uno::Reference< css::awt::XTabListener >& xListener)
    throw (css::uno::RuntimeException, std::exception)
{
    m_lListener.removeInterface(cppu::UnoType<css::awt::XTabListener>::get(), xListener);
}

//  XComponent

void SAL_CALL TabWindowService::dispose()
    throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    css::uno::Reference< css::uno::XInterface > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
    css::lang::EventObject aEvent(xThis);

    m_lListener.disposeAndClear (aEvent);

    if (m_pTabWin)
        m_pTabWin->RemoveEventListener( LINK( this, TabWindowService, EventListener ) );

    m_pTabWin.clear();
    m_xTabWin.clear();
}

//  XComponent

void SAL_CALL TabWindowService::addEventListener(const css::uno::Reference< css::lang::XEventListener >& xListener)
    throw (css::uno::RuntimeException, std::exception)
{
    m_lListener.addInterface(cppu::UnoType<css::lang::XEventListener>::get(), xListener);
}

//  XComponent

void SAL_CALL TabWindowService::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener)
    throw (css::uno::RuntimeException, std::exception)
{
    m_lListener.removeInterface(cppu::UnoType<css::lang::XEventListener>::get(), xListener);
}

void TabWindowService::impl_initializePropInfo()
{
    impl_setPropertyChangeBroadcaster(static_cast< css::awt::XSimpleTabController* >(this));

    impl_addPropertyInfo(
        css::beans::Property(
            OUString("Window"),
            TABWINDOWSERVICE_PROPHANDLE_WINDOW,
            cppu::UnoType<css::awt::XWindow>::get(),
            css::beans::PropertyAttribute::TRANSIENT));
}

void SAL_CALL TabWindowService::impl_setPropertyValue(const OUString& /*sProperty*/,
                                                              sal_Int32        /*nHandle  */,
                                                        const css::uno::Any&   /*aValue   */)

{
}

css::uno::Any SAL_CALL TabWindowService::impl_getPropertyValue(const OUString& /*sProperty*/,
                                                                       sal_Int32        nHandle      )
{
    /* There is no need to lock any mutex here. Because we share the
       solar mutex with our base class. And we said to our base class: "don't release it on calling us" .-)
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

//  TabWindowService
IMPL_LINK_TYPED( TabWindowService, EventListener, VclWindowEvent&, rEvent, void )
{
    sal_uLong           nEventId = rEvent.GetId();

    css::uno::Reference< css::uno::XInterface > xThis ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::lang::EventObject aEvent( xThis );

    if (nEventId == VCLEVENT_OBJECT_DYING)
    {
        m_lListener.disposeAndClear (aEvent);

        m_pTabWin->RemoveEventListener( LINK( this, TabWindowService, EventListener ) );
        m_pTabWin = NULL;
        m_xTabWin.clear();

        return;
    }

    ::cppu::OInterfaceContainerHelper* pContainer = m_lListener.getContainer(cppu::UnoType<css::awt::XTabListener>::get());
    if ( ! pContainer)
        return;

    ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
    while (pIterator.hasMoreElements())
    {
        try
        {
            css::awt::XTabListener* pListener = static_cast<css::awt::XTabListener*>(pIterator.next());

            switch (nEventId)
            {
                case VCLEVENT_TABPAGE_ACTIVATE :
                    pListener->activated( (sal_Int32)reinterpret_cast<sal_uLong>(rEvent.GetData()) );
                    break;

                case VCLEVENT_TABPAGE_DEACTIVATE :
                    pListener->deactivated( (sal_Int32)reinterpret_cast<sal_uLong>(rEvent.GetData()) );
                    break;

                case VCLEVENT_TABPAGE_INSERTED :
                    pListener->inserted( (sal_Int32)reinterpret_cast<sal_uLong>(rEvent.GetData()) );
                    break;

                case VCLEVENT_TABPAGE_REMOVED :
                    pListener->removed( (sal_Int32)reinterpret_cast<sal_uLong>(rEvent.GetData()) );
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
}

//  TabWindowService

void TabWindowService::impl_checkTabIndex (::sal_Int32 nID)
    throw (css::lang::IndexOutOfBoundsException)
{
    if (
        (nID <= 0                  ) ||
        (nID >  m_nPageIndexCounter)
       )
    {
        throw css::lang::IndexOutOfBoundsException(
                "Tab index out of bounds.",
                static_cast< ::cppu::OWeakObject* >(this) );
    }
}

//  TabWindowService

TTabPageInfoHash::iterator TabWindowService::impl_getTabPageInfo(::sal_Int32 nID)
    throw (css::lang::IndexOutOfBoundsException)
{
    TTabPageInfoHash::iterator pIt = m_lTabPageInfos.find(nID);
    if (pIt == m_lTabPageInfos.end ())
        throw css::lang::IndexOutOfBoundsException(
                "Tab index out of bounds.",
                static_cast< ::cppu::OWeakObject* >(this) );
    return pIt;
}

//  TabWindowService
FwkTabWindow* TabWindowService::mem_TabWin ()
{
    FwkTabWindow* pWin = NULL;

    if ( ! m_xTabWin.is ())
    {
        vcl::Window* pFakeParent = dynamic_cast< vcl::Window* >(Application::GetDefaultDevice ());

        m_pTabWin = VclPtr<FwkTabWindow>::Create(pFakeParent);
        m_xTabWin = VCLUnoHelper::GetInterface (m_pTabWin);

        m_pTabWin->AddEventListener( LINK( this, TabWindowService, EventListener ) );
    }

    if (m_xTabWin.is ())
        pWin = m_pTabWin;

    return pWin;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_TabWindowService_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    TabWindowService *inst = new TabWindowService;
    css::uno::XInterface *acquired_inst = cppu::acquire(inst);

    inst->initProperties();

    return acquired_inst;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
