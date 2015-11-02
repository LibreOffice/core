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

#ifndef INCLUDED_FRAMEWORK_INC_HELPER_UICONFIGELEMENTWRAPPERBASE_HXX
#define INCLUDED_FRAMEWORK_INC_HELPER_UICONFIGELEMENTWRAPPERBASE_HXX

#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>

#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/XUIElementSettings.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfigurationListener.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/awt/XMenuBar.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/implbase.hxx>

namespace framework
{

typedef ::cppu::WeakImplHelper<
           css::ui::XUIElement,
           css::ui::XUIElementSettings,
           css::lang::XInitialization,
           css::lang::XComponent,
           css::util::XUpdatable,
           css::ui::XUIConfigurationListener > UIConfigElementWrapperBase_BASE;

class UIConfigElementWrapperBase : private cppu::BaseMutex,
                                   public ::cppu::OBroadcastHelper                               ,
                                   public ::cppu::OPropertySetHelper                             ,
                                   public UIConfigElementWrapperBase_BASE
{

    //  public methods

    public:
        UIConfigElementWrapperBase( sal_Int16 nType );
        virtual  ~UIConfigElementWrapperBase();

        //  XInterface
        virtual void SAL_CALL acquire() throw () override
            { OWeakObject::acquire(); }
        virtual void SAL_CALL release() throw () override
            { OWeakObject::release(); }
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& type) throw ( css::uno::RuntimeException, std::exception ) override;

        // XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;

        // XComponent
        virtual  void SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception) override = 0;
        virtual  void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual  void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

        // XInitialization
        virtual  void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

        // XUIElementSettings
        virtual  void SAL_CALL updateSettings() throw (css::uno::RuntimeException, std::exception) override = 0;
        virtual  css::uno::Reference< css::container::XIndexAccess > SAL_CALL getSettings( sal_Bool bWriteable ) throw (css::uno::RuntimeException, std::exception) override;
        virtual  void SAL_CALL setSettings( const css::uno::Reference< css::container::XIndexAccess >& UISettings ) throw (css::uno::RuntimeException, std::exception) override;

        // XUIElement
        virtual  css::uno::Reference< css::frame::XFrame > SAL_CALL getFrame() throw (css::uno::RuntimeException, std::exception) override;
        virtual  OUString SAL_CALL getResourceURL() throw (css::uno::RuntimeException, std::exception) override;
        virtual  ::sal_Int16 SAL_CALL getType() throw (css::uno::RuntimeException, std::exception) override;
        virtual  css::uno::Reference< css::uno::XInterface > SAL_CALL getRealInterface() throw (css::uno::RuntimeException, std::exception) override = 0;

        // XUpdatable
        virtual  void SAL_CALL update() throw (css::uno::RuntimeException, std::exception) override;

        //  XUIConfigurationListener
        virtual  void SAL_CALL elementInserted( const css::ui::ConfigurationEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
        virtual  void SAL_CALL elementRemoved( const css::ui::ConfigurationEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
        virtual  void SAL_CALL elementReplaced( const css::ui::ConfigurationEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;

        // XEventListener
        using cppu::OPropertySetHelper::disposing;
        virtual  void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw(css::uno::RuntimeException, std::exception) override;

    //  protected methods

    protected:

        //  OPropertySetHelper
        virtual  sal_Bool                                            SAL_CALL convertFastPropertyValue        ( css::uno::Any&        aConvertedValue ,
                                                                                                               css::uno::Any&        aOldValue       ,
                                                                                                               sal_Int32                        nHandle         ,
                                                                                                               const css::uno::Any&  aValue          ) throw( css::lang::IllegalArgumentException ) override;
        virtual  void                                                SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32                        nHandle         ,
                                                                                                               const css::uno::Any&  aValue          ) throw( css::uno::Exception, std::exception                 ) override;
        using cppu::OPropertySetHelper::getFastPropertyValue;
        virtual  void                                                SAL_CALL getFastPropertyValue( css::uno::Any&    aValue          ,
                                                                                                   sal_Int32                    nHandle         ) const override;
        virtual  ::cppu::IPropertyArrayHelper&                       SAL_CALL getInfoHelper() override;
        virtual  css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (css::uno::RuntimeException, std::exception) override;

        virtual  void impl_fillNewData();

        static  const css::uno::Sequence< css::beans::Property > impl_getStaticPropertyDescriptor();

        sal_Int16                                                                               m_nType;
        bool                                                                                    m_bPersistent : 1,
                                                                                                m_bInitialized : 1,
                                                                                                m_bConfigListener : 1,
                                                                                                m_bConfigListening : 1,
                                                                                                m_bDisposed : 1,
                                                                                                m_bNoClose : 1;
        OUString                                                                                m_aResourceURL;
        css::uno::Reference< css::ui::XUIConfigurationManager >         m_xConfigSource;
        css::uno::Reference< css::container::XIndexAccess >             m_xConfigData;
        css::uno::WeakReference< css::frame::XFrame >                   m_xWeakFrame;
        css::uno::Reference< css::awt::XMenuBar >                       m_xMenuBar;
        ::cppu::OMultiTypeInterfaceContainerHelper                      m_aListenerContainer;   /// container for ALL Listener
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_HELPER_UICONFIGELEMENTWRAPPERBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
