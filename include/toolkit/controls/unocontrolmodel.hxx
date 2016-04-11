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

#ifndef INCLUDED_TOOLKIT_CONTROLS_UNOCONTROLMODEL_HXX
#define INCLUDED_TOOLKIT_CONTROLS_UNOCONTROLMODEL_HXX

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/weakagg.hxx>
#include <osl/mutex.hxx>

#include <toolkit/helper/mutexandbroadcasthelper.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>

#include <list>
#include <map>

typedef std::map<sal_uInt16, css::uno::Any> ImplPropertyTable;


//  class UnoControlModel


typedef ::cppu::WeakAggImplHelper   <   css::awt::XControlModel
                                    ,   css::beans::XPropertyState
                                    ,   css::io::XPersistObject
                                    ,   css::lang::XComponent
                                    ,   css::lang::XServiceInfo
                                    ,   css::lang::XUnoTunnel
                                    ,   css::util::XCloneable
                                    >   UnoControlModel_Base;

class TOOLKIT_DLLPUBLIC UnoControlModel :public UnoControlModel_Base
                                        ,public MutexAndBroadcastHelper
                                        ,public ::cppu::OPropertySetHelper
{
private:
    ImplPropertyTable                       maData;
    EventListenerMultiplexer                maDisposeListeners;

protected:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

protected:
    void                                        ImplRegisterProperty( sal_uInt16 nPropType );
    void                                        ImplRegisterProperties( const std::list< sal_uInt16 > &rIds );
    void                                        ImplRegisterProperty( sal_uInt16 nPropId, const css::uno::Any& rDefault );
    css::uno::Sequence<sal_Int32>  ImplGetPropertyIds() const;
    virtual css::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    bool                                    ImplHasProperty( sal_uInt16 nPropId ) const;

    /** called before setting multiple properties, allows to care for property dependencies

        <p>When multiple property values are set (e.g. XPropertySet::setPropertyValues), it may happen that some
        of them are dependent. For this, derivees which know such dependencies can affect the order in which
        the properties are internally really set.</p>
    */
    virtual void ImplNormalizePropertySequence(
                    const sal_Int32                 _nCount,        /// the number of entries in the arrays
                    sal_Int32*                      _pHandles,      /// the handles of the properties to set
                    css::uno::Any*     _pValues,       /// the values of the properties to set
                    sal_Int32*                      _pValidHandles  /// pointer to the valid handles, allowed to be adjusted
                )   const;

    /// ensures that two property values in a sequence have a certain order
    static void ImplEnsureHandleOrder(
                const sal_Int32 _nCount,                /// number of entries in the array
                sal_Int32* _pHandles,                   /// pointer to the handles
                css::uno::Any* _pValues,   /// pointer to the values
                sal_Int32 _nFirstHandle,                /// first handle, which should precede _nSecondHandle in the sequence
                sal_Int32 _nSecondHandle                /// second handle, which should supersede _nFirstHandle in the sequence
            );

protected:
#ifdef _MSC_VER
    UnoControlModel() //do not use! needed by MSVC at compile time to satisfy WeakAggImplHelper
        : UnoControlModel_Base()
        , MutexAndBroadcastHelper()
        , OPropertySetHelper( BrdcstHelper )
        , maDisposeListeners( *this )
        , m_xContext( css::uno::Reference< css::uno::XComponentContext >() )
    {
        assert(false);
    }
#endif

public:
                UnoControlModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                UnoControlModel( const UnoControlModel& rModel );

    virtual UnoControlModel*    Clone() const = 0;

    // css::uno::XInterface
    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override { return OWeakAggObject::queryInterface(rType); }
    void                        SAL_CALL acquire() throw() override;
    void                        SAL_CALL release() throw() override;

    // css::uno::XAggregation
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XUnoTunnel
    static const css::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
    sal_Int64                                                   SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rIdentifier ) throw(css::uno::RuntimeException, std::exception) override;

    // css::util::XCloneable
    css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // css::lang::XComponent
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw(css::uno::RuntimeException, std::exception) override;

    // css::beans::XPropertyState
    css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL write( const css::uno::Reference< css::io::XObjectOutputStream >& OutStream ) throw(css::io::IOException, css::uno::RuntimeException, std::exception) override;
    void SAL_CALL read( const css::uno::Reference< css::io::XObjectInputStream >& InStream ) throw(css::io::IOException, css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    // ::cppu::OPropertySetHelper
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override = 0;
    sal_Bool SAL_CALL convertFastPropertyValue( css::uno::Any & rConvertedValue, css::uno::Any & rOldValue, sal_Int32 nHandle, const css::uno::Any& rValue ) throw (css::lang::IllegalArgumentException, std::exception) override;
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) throw (css::uno::Exception, std::exception) override;
    using cppu::OPropertySetHelper::getFastPropertyValue;
    void SAL_CALL getFastPropertyValue( css::uno::Any& rValue, sal_Int32 nHandle ) const override;

    // override setValue methods to handle properties of FontDescriptor
    // css::beans::XPropertySet
    void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    // css::beans::XFastPropertySet
    void SAL_CALL setFastPropertyValue( sal_Int32 nHandle, const css::uno::Any& aValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& PropertyNames, const css::uno::Sequence< css::uno::Any >& Values ) throw(css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
};

#define UNO_CONTROL_MODEL_REGISTER_PROPERTIES(a) \
    do { \
        std::list< sal_uInt16 > aIds; \
        a::ImplGetPropertyIds( aIds ); \
        ImplRegisterProperties( aIds ); \
    } while (false)

#endif // INCLUDED_TOOLKIT_CONTROLS_UNOCONTROLMODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
