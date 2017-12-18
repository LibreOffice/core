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

#ifndef INCLUDED_FRAMEWORK_INC_TABWIN_TABWINDOW_HXX
#define INCLUDED_FRAMEWORK_INC_TABWIN_TABWINDOW_HXX

#include <stdtypes.h>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <services.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/awt/XSimpleTabController.hpp>
#include <com/sun/star/awt/XTabListener.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>
#include <vcl/tabctrl.hxx>

namespace framework
{

class TabWindow final : public css::lang::XTypeProvider             ,
                   public css::lang::XServiceInfo              ,
                   public css::lang::XInitialization           ,
                   public css::lang::XComponent                ,
                   public css::awt::XWindowListener            ,
                   public css::awt::XTopWindowListener         ,
                   public css::awt::XSimpleTabController       ,
                   private cppu::BaseMutex,
                   public ::cppu::OBroadcastHelper                          ,
                   public ::cppu::OPropertySetHelper                        ,   // => XPropertySet / XFastPropertySet / XMultiPropertySet
                   public ::cppu::OWeakObject
{
    public:
        TabWindow( const css::uno::Reference< css::uno::XComponentContext >& xContext );
        virtual ~TabWindow() override;

        //  XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        DECLARE_XSERVICEINFO_NOFACTORY
        FWK_DECLARE_XTYPEPROVIDER

        using ::cppu::OPropertySetHelper::disposing;
        using ::cppu::OPropertySetHelper::getFastPropertyValue;

        //  XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

        //  XComponent
        virtual void SAL_CALL dispose() override;
        virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

        //  XSimpleTabController
        virtual ::sal_Int32 SAL_CALL insertTab() override;
        virtual void SAL_CALL removeTab( ::sal_Int32 ID ) override;
        virtual void SAL_CALL setTabProps( ::sal_Int32 ID, const css::uno::Sequence< css::beans::NamedValue >& Properties ) override;
        virtual css::uno::Sequence< css::beans::NamedValue > SAL_CALL getTabProps( ::sal_Int32 ID ) override;
        virtual void SAL_CALL activateTab( ::sal_Int32 ID ) override;
        virtual ::sal_Int32 SAL_CALL getActiveTabID(  ) override;
        virtual void SAL_CALL addTabListener( const css::uno::Reference< css::awt::XTabListener >& Listener ) override;
        virtual void SAL_CALL removeTabListener( const css::uno::Reference< css::awt::XTabListener >& Listener ) override;

        //  XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) override;

        //  XTopWindowListener
        virtual void SAL_CALL windowOpened( const css::lang::EventObject& e ) override;
        virtual void SAL_CALL windowClosing( const css::lang::EventObject& e ) override;
        virtual void SAL_CALL windowClosed( const css::lang::EventObject& e ) override;
        virtual void SAL_CALL windowMinimized( const css::lang::EventObject& e ) override;
        virtual void SAL_CALL windowNormalized( const css::lang::EventObject& e ) override;
        virtual void SAL_CALL windowActivated( const css::lang::EventObject& e ) override;
        virtual void SAL_CALL windowDeactivated( const css::lang::EventObject& e ) override;

        //  XWindowListener
        virtual void SAL_CALL windowResized( const css::awt::WindowEvent& aEvent ) override;
        virtual void SAL_CALL windowMoved( const css::awt::WindowEvent& aEvent ) override;
        virtual void SAL_CALL windowShown( const css::lang::EventObject& aEvent ) override;
        virtual void SAL_CALL windowHidden( const css::lang::EventObject& aEvent ) override;

    private:
        DECL_LINK( Activate, TabControl*, void );
        DECL_LINK( Deactivate, TabControl*, bool );

        //  OPropertySetHelper

        virtual sal_Bool                                            SAL_CALL convertFastPropertyValue( css::uno::Any&        aConvertedValue ,
                                                                                                       css::uno::Any&        aOldValue       ,
                                                                                                       sal_Int32                        nHandle         ,
                                                                                                       const css::uno::Any&  aValue          ) override;
        virtual void                                                SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32                        nHandle         ,
                                                                                                               const css::uno::Any&  aValue          ) override;
        virtual void                                                SAL_CALL getFastPropertyValue( css::uno::Any&    aValue          ,
                                                                                                   sal_Int32                    nHandle         ) const override;
        virtual ::cppu::IPropertyArrayHelper&                       SAL_CALL getInfoHelper() override;
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;

        static const css::uno::Sequence< css::beans::Property > impl_getStaticPropertyDescriptor();

        enum Notification
        {
            NOTIFY_INSERTED,
            NOTIFY_REMOVED,
            NOTIFY_CHANGED,
            NOTIFY_ACTIVATED,
            NOTIFY_DEACTIVATED
        };

        void        implts_LayoutWindows() const;
        void        impl_SetTitle( const OUString& rTitle );
        TabControl* impl_GetTabControl( const css::uno::Reference< css::awt::XWindow >& xTabControlWindow ) const;
        void        implts_SendNotification( Notification eNotify, sal_Int32 ID ) const;
        void        implts_SendNotification( Notification eNotify, sal_Int32 ID, const css::uno::Sequence< css::beans::NamedValue >& rSeq ) const;

        bool                                                   m_bInitialized : 1,
                                                               m_bDisposed : 1;
        sal_Int32                                              m_nNextTabID;
        OUString                                               m_aTitlePropName;
        OUString                                               m_aPosPropName;
        css::uno::Reference< css::uno::XComponentContext >     m_xContext;
        css::uno::Reference< css::awt::XTopWindow >            m_xTopWindow;
        css::uno::Reference< css::awt::XWindow >               m_xContainerWindow;
        css::uno::Reference< css::awt::XWindow >               m_xTabControlWindow;
        ::cppu::OMultiTypeInterfaceContainerHelper             m_aListenerContainer; // container for ALL Listener
};

}

#endif // INCLUDED_FRAMEWORK_INC_TABWIN_TABWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
