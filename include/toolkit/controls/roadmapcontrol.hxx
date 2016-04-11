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

#ifndef INCLUDED_TOOLKIT_CONTROLS_ROADMAPCONTROL_HXX
#define INCLUDED_TOOLKIT_CONTROLS_ROADMAPCONTROL_HXX


#include <toolkit/controls/unocontrols.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <toolkit/controls/roadmapentry.hxx>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/awt/XItemEventBroadcaster.hpp>
#include <cppuhelper/implbase.hxx>


#include <comphelper/uno3.hxx>


namespace toolkit
{


    typedef GraphicControlModel UnoControlRoadmapModel_Base;


    typedef ::cppu::ImplHelper  <   css::lang::XSingleServiceFactory
                                ,   css::container::XContainer
                                ,   css::container::XIndexContainer
                                >   UnoControlRoadmapModel_IBase;


    typedef UnoControlBase  UnoControlRoadmap_Base;
    typedef ::cppu::ImplHelper  <   css::awt::XItemEventBroadcaster
                                ,   css::container::XContainerListener
                                ,   css::awt::XItemListener
                                ,   css::beans::XPropertyChangeListener
                                >   UnoControlRoadmap_IBase;


    typedef ::cppu::ImplHelper < css::container::XContainerListener,
                                 css::awt::XItemEventBroadcaster>    SVTXRoadmap_Base;


    // = UnoControlRoadmapModel

    class UnoControlRoadmapModel : public UnoControlRoadmapModel_Base,
                                   public UnoControlRoadmapModel_IBase

    {
    private:
//        PropertyChangeListenerMultiplexer                 maPropertyListeners;

        typedef ::std::vector< css::uno::Reference< XInterface > >    RoadmapItemHolderList;

        ContainerListenerMultiplexer        maContainerListeners;
        RoadmapItemHolderList               maRoadmapItems;

        void                                MakeRMItemValidation( sal_Int32 Index, const css::uno::Reference< XInterface >& xRoadmapItem );
        css::container::ContainerEvent      GetContainerEvent(sal_Int32 Index, const css::uno::Reference< XInterface >& );
        void                                SetRMItemDefaultProperties( const sal_Int32 Index, const css::uno::Reference< XInterface >& );
        static sal_Int16                    GetCurrentItemID( const css::uno::Reference< css::beans::XPropertySet >& xPropertySet );
        sal_Int32                           GetUniqueID();


    protected:
        css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
        ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

    public:
        UnoControlRoadmapModel( const css::uno::Reference< css::uno::XComponentContext >& i_factory );
        UnoControlRoadmapModel( const UnoControlRoadmapModel& rModel ) :
            UnoControlRoadmapModel_Base( rModel ),
            UnoControlRoadmapModel_IBase( rModel ),
            maContainerListeners( *this ) {}
        UnoControlModel*    Clone() const override { return new UnoControlRoadmapModel( *this ); }


    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )


    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL insertByIndex( sal_Int32 Index, const css::uno::Any & Element) throw    (css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeByIndex( sal_Int32 Index ) throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const css::uno::Any & Element) throw (css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override { return UnoControlRoadmapModel_Base::queryInterface(rType); }
    css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void          SAL_CALL acquire() throw() override  { UnoControlRoadmapModel_Base::acquire(); }
    void          SAL_CALL release() throw() override  { UnoControlRoadmapModel_Base::release(); }


    // css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (css::uno::RuntimeException, std::exception) override;


    virtual css::uno::Reference< XInterface > SAL_CALL createInstance(  ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< XInterface > SAL_CALL createInstanceWithArguments( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException, std::exception) override;

    };


    // = UnoRoadmapControl

    class UnoRoadmapControl : public UnoControlRoadmap_Base,
                              public UnoControlRoadmap_IBase
    {
    private:
        ItemListenerMultiplexer                 maItemListeners;
    public:
                            UnoRoadmapControl();
        OUString     GetComponentServiceName() override;

    void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override { UnoControlBase::disposing( Source ); }

    void SAL_CALL   dispose(  ) throw (css::uno::RuntimeException, std::exception) override;


    sal_Bool SAL_CALL setModel(const css::uno::Reference< css::awt::XControlModel >& Model) throw ( css::uno::RuntimeException, std::exception ) override;

    void SAL_CALL elementInserted( const css::container::ContainerEvent& rEvent ) throw (css::uno::RuntimeException, std::exception) override;
    void SAL_CALL elementRemoved( const css::container::ContainerEvent& rEvent ) throw (css::uno::RuntimeException, std::exception) override;
    void SAL_CALL elementReplaced( const css::container::ContainerEvent& rEvent ) throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL addItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l ) throw (css::uno::RuntimeException, std::exception) override;


    virtual void SAL_CALL itemStateChanged( const css::awt::ItemEvent& rEvent ) throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& evt ) throw (css::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )
    DECLARE_XINTERFACE()

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
    };


} // toolkit


#endif // _ INCLUDED_TOOLKIT_CONTROLS_ROADMAPCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
