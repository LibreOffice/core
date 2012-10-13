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

#ifndef _TOOLKIT_AWT_UNOCONTROLMODEL_HXX_
#define _TOOLKIT_AWT_UNOCONTROLMODEL_HXX_

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <cppuhelper/weakagg.hxx>
#include <osl/mutex.hxx>

#include <toolkit/helper/mutexandbroadcasthelper.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/implbase7.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/uno3.hxx>

#include <list>
#include <map>

typedef std::map<sal_uInt16, ::com::sun::star::uno::Any> ImplPropertyTable;

//  ----------------------------------------------------
//  class UnoControlModel
//  ----------------------------------------------------

typedef ::cppu::WeakAggImplHelper7  <   ::com::sun::star::awt::XControlModel
                                    ,   ::com::sun::star::beans::XPropertyState
                                    ,   ::com::sun::star::io::XPersistObject
                                    ,   ::com::sun::star::lang::XComponent
                                    ,   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::lang::XUnoTunnel
                                    ,   ::com::sun::star::util::XCloneable
                                    >   UnoControlModel_Base;

class TOOLKIT_DLLPUBLIC UnoControlModel :public UnoControlModel_Base
                                        ,public MutexAndBroadcastHelper
                                        ,public ::cppu::OPropertySetHelper
{
private:
    ImplPropertyTable                       maData;
    EventListenerMultiplexer                maDisposeListeners;

protected:
    const ::comphelper::ComponentContext    maContext;

protected:
    void                                        ImplRegisterProperty( sal_uInt16 nPropType );
    void                                        ImplRegisterProperties( const std::list< sal_uInt16 > &rIds );
    void                                        ImplRegisterProperty( sal_uInt16 nPropId, const ::com::sun::star::uno::Any& rDefault );
    ::com::sun::star::uno::Sequence<sal_Int32>  ImplGetPropertyIds() const;
    virtual ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    sal_Bool                                    ImplHasProperty( sal_uInt16 nPropId ) const;

    /** called before setting multiple properties, allows to care for property dependencies

        <p>When multiple property values are set (e.g. XPropertySet::setPropertyValues), it may happen that some
        of them are dependent. For this, derivees which know such dependencies can affect the order in which
        the properties are internally really set.</p>
    */
    virtual void ImplNormalizePropertySequence(
                    const sal_Int32                 _nCount,        /// the number of entries in the arrays
                    sal_Int32*                      _pHandles,      /// the handles of the properties to set
                    ::com::sun::star::uno::Any*     _pValues,       /// the values of the properties to set
                    sal_Int32*                      _pValidHandles  /// pointer to the valid handles, allowed to be adjusted
                )   const SAL_THROW(());

    /// ensures that two property values in a sequence have a certain order
    void    ImplEnsureHandleOrder(
                const sal_Int32 _nCount,                /// number of entries in the array
                sal_Int32* _pHandles,                   /// pointer to the handles
                ::com::sun::star::uno::Any* _pValues,   /// pointer to the values
                sal_Int32 _nFirstHandle,                /// first handle, which should precede _nSecondHandle in the sequence
                sal_Int32 _nSecondHandle                /// second handle, which should supersede _nFirstHandle in the sequence
            ) const;

protected:
    UnoControlModel() //do not use! needed by MSVC at compile time to satisfy WeakAggImplHelper7
        : UnoControlModel_Base()
        , MutexAndBroadcastHelper()
        , OPropertySetHelper( BrdcstHelper )
        , maDisposeListeners( *this )
        , maContext( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >() )
    {
        assert(false);
    }

public:
                UnoControlModel( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
                UnoControlModel( const UnoControlModel& rModel );

    virtual UnoControlModel*    Clone() const = 0;

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return OWeakAggObject::queryInterface(rType); }
    void                        SAL_CALL acquire() throw();
    void                        SAL_CALL release() throw();

    // ::com::sun::star::uno::XAggregation
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
    sal_Int64                                                   SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XCloneable
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XPropertyState
    ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL write( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream >& OutStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL read( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream >& InStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // ::cppu::OPropertySetHelper
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() = 0;
    sal_Bool SAL_CALL convertFastPropertyValue( ::com::sun::star::uno::Any & rConvertedValue, ::com::sun::star::uno::Any & rOldValue, sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::lang::IllegalArgumentException);
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception);
    using cppu::OPropertySetHelper::getFastPropertyValue;
    void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;

    // setValue-Methoden ueberladen, um die Einzelproperties des FontDescriptors abzufangen
    // ::com::sun::star::beans::XPropertySet
    void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    // ::com::sun::star::beans::XFastPropertySet
    void SAL_CALL setFastPropertyValue( sal_Int32 nHandle, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Values ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
};

#define UNO_CONTROL_MODEL_REGISTER_PROPERTIES(a) \
    do { \
        std::list< sal_uInt16 > aIds; \
        a::ImplGetPropertyIds( aIds ); \
        ImplRegisterProperties( aIds ); \
    } while (0)

#endif // _TOOLKIT_AWT_UNOCONTROLMODEL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
