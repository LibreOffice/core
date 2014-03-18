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
#include <macros/generic.hxx>
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

class TabWindow :  public ::com::sun::star::lang::XTypeProvider             ,
                   public ::com::sun::star::lang::XServiceInfo              ,
                   public ::com::sun::star::lang::XInitialization           ,
                   public ::com::sun::star::lang::XComponent                ,
                   public ::com::sun::star::awt::XWindowListener            ,
                   public ::com::sun::star::awt::XTopWindowListener         ,
                   public ::com::sun::star::awt::XSimpleTabController       ,
                   private cppu::BaseMutex,
                   public ::cppu::OBroadcastHelper                          ,
                   public ::cppu::OPropertySetHelper                        ,   // => XPropertySet / XFastPropertySet / XMultiPropertySet
                   public ::cppu::OWeakObject
{
    public:
        TabWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );
        virtual ~TabWindow();

        //  XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        DECLARE_XSERVICEINFO_NOFACTORY
        FWK_DECLARE_XTYPEPROVIDER

        using ::cppu::OPropertySetHelper::disposing;
        using ::cppu::OPropertySetHelper::getFastPropertyValue;


        //  XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception);

        //  XComponent
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception);

        //  XSimpleTabController
        virtual ::sal_Int32 SAL_CALL insertTab() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL removeTab( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setTabProps( ::sal_Int32 ID, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Properties ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > SAL_CALL getTabProps( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL activateTab( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::sal_Int32 SAL_CALL getActiveTabID(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL addTabListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabListener >& Listener ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL removeTabListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabListener >& Listener ) throw (::com::sun::star::uno::RuntimeException, std::exception);


        //  XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException, std::exception );

        //  XTopWindowListener
        virtual void SAL_CALL windowOpened( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL windowClosing( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL windowClosed( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL windowMinimized( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL windowNormalized( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL windowActivated( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL windowDeactivated( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException, std::exception);

        //  XWindowListener
        virtual void SAL_CALL windowResized( const css::awt::WindowEvent& aEvent ) throw( css::uno::RuntimeException, std::exception );
        virtual void SAL_CALL windowMoved( const css::awt::WindowEvent& aEvent ) throw( css::uno::RuntimeException, std::exception );
        virtual void SAL_CALL windowShown( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException, std::exception );
        virtual void SAL_CALL windowHidden( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException, std::exception );

    protected:
        DECL_LINK( Activate, TabControl* );
        DECL_LINK( Deactivate, TabControl* );

    private:

        //  OPropertySetHelper

        virtual sal_Bool                                            SAL_CALL convertFastPropertyValue( com::sun::star::uno::Any&        aConvertedValue ,
                                                                                                       com::sun::star::uno::Any&        aOldValue       ,
                                                                                                       sal_Int32                        nHandle         ,
                                                                                                       const com::sun::star::uno::Any&  aValue          ) throw( com::sun::star::lang::IllegalArgumentException );
        virtual void                                                SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32                        nHandle         ,
                                                                                                               const com::sun::star::uno::Any&  aValue          ) throw( com::sun::star::uno::Exception, std::exception                 );
        virtual void                                                SAL_CALL getFastPropertyValue( com::sun::star::uno::Any&    aValue          ,
                                                                                                   sal_Int32                    nHandle         ) const;
        virtual ::cppu::IPropertyArrayHelper&                       SAL_CALL getInfoHelper();
        virtual ::com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException, std::exception);

        static const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > impl_getStaticPropertyDescriptor();

    private:
        enum Notification
        {
            NOTIFY_INSERTED,
            NOTIFY_REMOVED,
            NOTIFY_CHANGED,
            NOTIFY_ACTIVATED,
            NOTIFY_DEACTIVATED
        };

        sal_Int32   impl_GetPageIdFromIndex( ::sal_Int32 nIndex ) const;
        sal_Bool    impl_CheckIndex( ::sal_Int32 nIndex ) const;
        void        implts_LayoutWindows() const;
        void        impl_SetTitle( const OUString& rTitle );
        TabControl* impl_GetTabControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& xTabControlWindow ) const;
        void        implts_SendNotification( Notification eNotify, sal_Int32 ID ) const;
        void        implts_SendNotification( Notification eNotify, sal_Int32 ID, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rSeq ) const;

        typedef std::vector< sal_uInt16 > PageIdVector;

        sal_Bool                                                                         m_bInitialized : 1,
                                                                                         m_bDisposed : 1;
        sal_Int32                                                                        m_nNextTabID;
        OUString                                                                  m_aTitlePropName;
        OUString                                                                  m_aPosPropName;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >     m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindow >            m_xTopWindow;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >               m_xContainerWindow;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >               m_xTabControlWindow;
        ::cppu::OMultiTypeInterfaceContainerHelper                                       m_aListenerContainer; // container for ALL Listener
};

}

#endif // INCLUDED_FRAMEWORK_INC_TABWIN_TABWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
