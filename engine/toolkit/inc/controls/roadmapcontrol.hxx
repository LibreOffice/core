/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once


#include <toolkit/controls/unocontrols.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/awt/XItemEventBroadcaster.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase4.hxx>


#include <comphelper/uno3.hxx>


namespace toolkit
{


    typedef GraphicControlModel UnoControlRoadmapModel_Base;


    typedef ::cppu::ImplHelper3 <   css::lang::XSingleServiceFactory
                                ,   css::container::XContainer
                                ,   css::container::XIndexContainer
                                >   UnoControlRoadmapModel_IBase;


    typedef UnoControlBase  UnoControlRoadmap_Base;
    typedef ::cppu::ImplHelper4 <   css::awt::XItemEventBroadcaster
                                ,   css::container::XContainerListener
                                ,   css::awt::XItemListener
                                ,   css::beans::XPropertyChangeListener
                                >   UnoControlRoadmap_IBase;


    typedef ::cppu::ImplHelper2< css::container::XContainerListener,
                                 css::awt::XItemEventBroadcaster>    SVTXRoadmap_Base;


    // = UnoControlRoadmapModel

    class UnoControlRoadmapModel final : public UnoControlRoadmapModel_Base,
                                   public UnoControlRoadmapModel_IBase

    {
    private:
//        PropertyChangeListenerMultiplexer                 maPropertyListeners;

        typedef ::std::vector< css::uno::Reference< XInterface > >    RoadmapItemHolderList;

        ContainerListenerMultiplexer        maContainerListeners;
        RoadmapItemHolderList               maRoadmapItems;

        void                                MakeRMItemValidation( sal_Int32 Index, const css::uno::Reference< XInterface >& xRoadmapItem );
        css::container::ContainerEvent      GetContainerEvent(sal_Int32 Index, const css::uno::Reference< XInterface >& );
        void                                SetRMItemDefaultProperties( const css::uno::Reference< XInterface >& );
        static sal_Int16                    GetCurrentItemID( const css::uno::Reference< css::beans::XPropertySet >& xPropertySet );
        sal_Int32                           GetUniqueID();

        cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
        ::cppu::IPropertyArrayHelper& getInfoHelper() override;

    public:
        UnoControlRoadmapModel( const css::uno::Reference< css::uno::XComponentContext >& i_factory );
        UnoControlRoadmapModel( const UnoControlRoadmapModel& rModel ) :
            UnoControlRoadmapModel_Base( rModel ),
            UnoControlRoadmapModel_IBase( rModel ),
            maContainerListeners( *this ) {}
        rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlRoadmapModel( *this ); }


    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )


    // css::io::XPersistObject
    OUString getServiceName() override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

    sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

    virtual void insertByIndex( sal_Int32 Index, const cpo::uno::Any & Element) override;
    virtual void removeByIndex( sal_Int32 Index ) override;
    virtual void replaceByIndex( sal_Int32 Index, const cpo::uno::Any & Element) override;

    virtual void addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;
    virtual void removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;

    cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override { return UnoControlRoadmapModel_Base::queryInterface(rType); }
    cpo::uno::Any queryAggregation( const cpo::uno::Type & rType ) override;
    void          acquire() noexcept override  { UnoControlRoadmapModel_Base::acquire(); }
    void          release() noexcept override  { UnoControlRoadmapModel_Base::release(); }


    // css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;


    virtual css::uno::Reference< XInterface > createInstance(  ) override;
    virtual css::uno::Reference< XInterface > createInstanceWithArguments( const cpo::uno::Sequence< cpo::uno::Any >& aArguments ) override;

    virtual cpo::uno::Type getElementType() override;

    virtual bool hasElements() override;

    };


    // = UnoRoadmapControl

    class UnoRoadmapControl final : public UnoControlRoadmap_Base,
                              public UnoControlRoadmap_IBase
    {
    private:
        ItemListenerMultiplexer                 maItemListeners;
    public:
                            UnoRoadmapControl();
        OUString     GetComponentServiceName() const override;

    void disposing( const css::lang::EventObject& Source ) override { UnoControlBase::disposing( Source ); }

    void   dispose(  ) override;


    bool setModel(const css::uno::Reference< css::awt::XControlModel >& Model) override;

    void elementInserted( const css::container::ContainerEvent& rEvent ) override;
    void elementRemoved( const css::container::ContainerEvent& rEvent ) override;
    void elementReplaced( const css::container::ContainerEvent& rEvent ) override;

    virtual void addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;
    virtual void removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) override;


    virtual void itemStateChanged( const css::awt::ItemEvent& rEvent ) override;

    virtual void propertyChange( const css::beans::PropertyChangeEvent& evt ) override;

    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )
    DECLARE_UNO3_AGG_DEFAULTS(UnoRoadmapControl, UnoControlRoadmap_Base)

    cpo::uno::Any queryAggregation(cpo::uno::Type const & aType) override;

    // css::lang::XServiceInfo
    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
    };


} // toolkit


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
