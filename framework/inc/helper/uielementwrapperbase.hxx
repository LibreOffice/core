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

#ifndef INCLUDED_FRAMEWORK_INC_HELPER_UIELEMENTWRAPPERBASE_HXX
#define INCLUDED_FRAMEWORK_INC_HELPER_UIELEMENTWRAPPERBASE_HXX

#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>

#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/XUpdatable.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/implbase.hxx>

namespace framework
{

typedef ::cppu::WeakImplHelper<
           ::com::sun::star::ui::XUIElement,
           ::com::sun::star::lang::XInitialization,
           ::com::sun::star::lang::XComponent,
           ::com::sun::star::util::XUpdatable > UIElementWrapperBase_BASE;

class UIElementWrapperBase : private cppu::BaseMutex,
                             public ::cppu::OBroadcastHelper                    ,
                             public ::cppu::OPropertySetHelper                  ,
                             public UIElementWrapperBase_BASE
{

    //  public methods

    public:
         UIElementWrapperBase( sal_Int16 nType );
        virtual  ~UIElementWrapperBase();

        // XInterface
        virtual void SAL_CALL acquire() throw () override
            { OWeakObject::acquire(); }
        virtual void SAL_CALL release() throw () override
            { OWeakObject::release(); }
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& type) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // XComponent
        virtual  void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException, std::exception) override = 0;
        virtual  void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual  void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XInitialization
        virtual  void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XUpdatable
        virtual  void SAL_CALL update() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XUIElement
        virtual  ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > SAL_CALL getFrame() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual  OUString SAL_CALL getResourceURL() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual  ::sal_Int16 SAL_CALL getType() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual  ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getRealInterface() throw (::com::sun::star::uno::RuntimeException, std::exception) override = 0;

    //  protected methods

    protected:

        //  OPropertySetHelper
        virtual  sal_Bool                                            SAL_CALL convertFastPropertyValue        ( com::sun::star::uno::Any&        aConvertedValue ,
                                                                                                               com::sun::star::uno::Any&        aOldValue       ,
                                                                                                               sal_Int32                        nHandle         ,
                                                                                                               const com::sun::star::uno::Any&  aValue          ) throw( com::sun::star::lang::IllegalArgumentException ) override;
        virtual  void                                                SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32                        nHandle         ,
                                                                                                               const com::sun::star::uno::Any&  aValue          ) throw( com::sun::star::uno::Exception, std::exception                 ) override;
        using cppu::OPropertySetHelper::getFastPropertyValue;
        virtual  void                                                SAL_CALL getFastPropertyValue( com::sun::star::uno::Any&    aValue          ,
                                                                                                   sal_Int32                    nHandle         ) const override;
        virtual  ::cppu::IPropertyArrayHelper&                       SAL_CALL getInfoHelper() override;
        virtual  ::com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        static  const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > impl_getStaticPropertyDescriptor();

        ::cppu::OMultiTypeInterfaceContainerHelper                          m_aListenerContainer;   /// container for ALL Listener
        OUString                                                       m_aResourceURL;
        com::sun::star::uno::WeakReference< com::sun::star::frame::XFrame > m_xWeakFrame;
        sal_Int16                                                           m_nType;
        bool                                                            m_bInitialized : 1,
                                                                            m_bDisposed;
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_HELPER_UIELEMENTWRAPPERBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
