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

#ifndef TOOLKIT_ROADMAP_CONTROL_HXX
#define TOOLKIT_ROADMAP_CONTROL_HXX


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
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase4.hxx>


#include <comphelper/uno3.hxx>

//........................................................................
namespace toolkit
{
//........................................................................

    typedef GraphicControlModel UnoControlRoadmapModel_Base;


    typedef ::cppu::ImplHelper3 <   ::com::sun::star::lang::XSingleServiceFactory
                                ,   ::com::sun::star::container::XContainer
                                ,   ::com::sun::star::container::XIndexContainer
                                >   UnoControlRoadmapModel_IBase;



    typedef UnoControlBase  UnoControlRoadmap_Base;
    typedef ::cppu::ImplHelper4 <   ::com::sun::star::awt::XItemEventBroadcaster
                                ,   ::com::sun::star::container::XContainerListener
                                ,   ::com::sun::star::awt::XItemListener
                                ,   ::com::sun::star::beans::XPropertyChangeListener
                                >   UnoControlRoadmap_IBase;




    typedef ::cppu::ImplHelper2< ::com::sun::star::container::XContainerListener,
                                 ::com::sun::star::awt::XItemEventBroadcaster>    SVTXRoadmap_Base;


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;


    // ===================================================================
    // = UnoControlRoadmapModel
    // ===================================================================
    class UnoControlRoadmapModel : public UnoControlRoadmapModel_Base,
                                   public UnoControlRoadmapModel_IBase

    {
    private:
//        PropertyChangeListenerMultiplexer                 maPropertyListeners;

        typedef ::std::vector< Reference< XInterface > >    RoadmapItemHolderList;

        ContainerListenerMultiplexer        maContainerListeners;
        RoadmapItemHolderList               maRoadmapItems;

        void                                MakeRMItemValidation( sal_Int32 Index, Reference< XInterface > xRoadmapItem );
        ContainerEvent                      GetContainerEvent(sal_Int32 Index, Reference< XInterface > );
        void                                SetRMItemDefaultProperties( const sal_Int32 _Index, Reference< XInterface > );
        sal_Int16                           GetCurrentItemID( Reference< XPropertySet > xPropertySet );
        sal_Int32                           GetUniqueID();


    protected:
        ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
        ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    public:
        UnoControlRoadmapModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_factory );
        UnoControlRoadmapModel( const UnoControlRoadmapModel& rModel ) :
            UnoControlRoadmapModel_Base( rModel ),
            UnoControlRoadmapModel_IBase( rModel ),
            maContainerListeners( *this ) {}
        UnoControlModel*    Clone() const { return new UnoControlRoadmapModel( *this ); }


    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )


    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlRoadmapModel, UnoControlRoadmapModel_Base, szServiceName2_UnoControlRoadmapModel )

    sal_Int32 SAL_CALL getCount() throw (RuntimeException);
    virtual Any SAL_CALL getByIndex( sal_Int32 Index ) throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException );

    virtual void SAL_CALL insertByIndex( sal_Int32 Index, const Any & _Element) throw    (IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL removeByIndex( sal_Int32 Index ) throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const Any & _Element) throw (IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException );

    virtual void SAL_CALL addContainerListener( const Reference< XContainerListener >& xListener ) throw (RuntimeException);
    virtual void SAL_CALL removeContainerListener( const Reference< XContainerListener >& xListener ) throw (RuntimeException);

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlRoadmapModel_Base::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { UnoControlRoadmapModel_Base::acquire(); }
    void                        SAL_CALL release() throw()  { UnoControlRoadmapModel_Base::release(); }


    // ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (::com::sun::star::uno::RuntimeException);


    virtual Reference< XInterface > SAL_CALL createInstance(  ) throw (Exception, RuntimeException);
    virtual Reference< XInterface > SAL_CALL createInstanceWithArguments( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException);

    virtual Type SAL_CALL getElementType() throw (RuntimeException);

    virtual sal_Bool SAL_CALL hasElements() throw (RuntimeException);

    };


    // ===================================================================
    // = UnoRoadmapControl
    // ===================================================================
    class UnoRoadmapControl : public UnoControlRoadmap_Base,
                              public UnoControlRoadmap_IBase
    {
    private:
        ItemListenerMultiplexer                 maItemListeners;
    public:
                            UnoRoadmapControl();
        OUString     GetComponentServiceName();

    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException) { UnoControlBase::disposing( Source ); }

    void SAL_CALL   dispose(  ) throw(::com::sun::star::uno::RuntimeException);


    sal_Bool SAL_CALL setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& Model) throw ( ::com::sun::star::uno::RuntimeException );

    void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& rEvent )throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& rEvent )throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& rEvent )throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addItemListener( const Reference< XItemListener >& l ) throw (RuntimeException);
    virtual void SAL_CALL removeItemListener( const Reference< XItemListener >& l ) throw (RuntimeException);


    virtual void SAL_CALL itemStateChanged( const ItemEvent& rEvent ) throw (RuntimeException);

    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )
    DECLARE_XINTERFACE()

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoRoadmapControl, UnoControlBase, szServiceName2_UnoControlRoadmap )
    };

//........................................................................
} // toolkit
//........................................................................



#endif // _TOOLKIT_ROADMAP_CONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
