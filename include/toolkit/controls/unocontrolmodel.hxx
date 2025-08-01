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

#include <toolkit/dllapi.h>

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <cppuhelper/weakagg.hxx>

#include <toolkit/helper/listenermultiplexer.hxx>

#include <comphelper/propshlp.hxx>
#include <cppuhelper/implbase6.hxx>
#include <comphelper/uno3.hxx>
#include <rtl/ref.hxx>

#include <vector>
#include <map>

namespace com::sun::star::uno { class XComponentContext; }

typedef std::map<sal_uInt16, css::uno::Any> ImplPropertyTable;

typedef ::cppu::WeakAggImplHelper6  <   css::awt::XControlModel
                                    ,   css::beans::XPropertyState
                                    ,   css::io::XPersistObject
                                    ,   css::lang::XComponent
                                    ,   css::lang::XServiceInfo
                                    ,   css::util::XCloneable
                                    >   UnoControlModel_Base;

class TOOLKIT_DLLPUBLIC UnoControlModel : public UnoControlModel_Base
                       ,public ::comphelper::OPropertySetHelper
{
private:
    ImplPropertyTable                       maData;
    EventListenerMultiplexer                maDisposeListeners;

protected:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

protected:
    void                                        ImplRegisterProperty( sal_uInt16 nPropType );
    void                                        ImplRegisterProperties( const std::vector< sal_uInt16 > &rIds );
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

    template<typename T> void UNO_CONTROL_MODEL_REGISTER_PROPERTIES() {
        std::vector< sal_uInt16 > aIds;
        T::ImplGetPropertyIds( aIds );
        ImplRegisterProperties( aIds );
    }

protected:
#ifdef _MSC_VER
    UnoControlModel() //do not use! needed by MSVC at compile time to satisfy WeakAggImplHelper7
        : UnoControlModel_Base()
        , OPropertySetHelper()
        , maDisposeListeners( *this )
        , m_xContext( css::uno::Reference< css::uno::XComponentContext >() )
    {
        assert(false);
    }
#endif

public:
                UnoControlModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                UnoControlModel( const UnoControlModel& rModel );

    virtual rtl::Reference<UnoControlModel> Clone() const = 0;

    // css::uno::XInterface
    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) override { return OWeakAggObject::queryInterface(rType); }
    void                        SAL_CALL acquire() noexcept override;
    void                        SAL_CALL release() noexcept override;

    // css::uno::XAggregation
    css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) override;

    // css::util::XCloneable
    css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    // css::lang::XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // css::lang::XComponent
    void SAL_CALL dispose(  ) override;
    void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // css::beans::XPropertyState
    css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;
    css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) override;
    void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) override;
    css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;
    void SAL_CALL write( const css::uno::Reference< css::io::XObjectOutputStream >& OutStream ) override;
    void SAL_CALL read( const css::uno::Reference< css::io::XObjectInputStream >& InStream ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName(  ) override;
    sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // ::cppu::OPropertySetHelper
    ::cppu::IPropertyArrayHelper& getInfoHelper() override = 0;
    bool convertFastPropertyValue( std::unique_lock<std::mutex>& rGuard, css::uno::Any & rConvertedValue, css::uno::Any & rOldValue, sal_Int32 nHandle, const css::uno::Any& rValue ) override;
    void setFastPropertyValue_NoBroadcast(
                std::unique_lock<std::mutex>& rGuard,
                sal_Int32 nHandle, const css::uno::Any& rValue ) override;
    using comphelper::OPropertySetHelper::getFastPropertyValue;
    void getFastPropertyValue( std::unique_lock<std::mutex>& rGuard, css::uno::Any& rValue, sal_Int32 nHandle ) const override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& PropertyNames, const css::uno::Sequence< css::uno::Any >& Values ) override;
protected:
    // override setValue methods to handle properties of FontDescriptor
    // css::beans::XFastPropertySet
    void setFastPropertyValueImpl( std::unique_lock<std::mutex>& rGuard, sal_Int32 nHandle, const css::uno::Any& aValue ) override;
    css::beans::PropertyState getPropertyStateImpl( std::unique_lock<std::mutex>& rGuard, const OUString& PropertyName );
    void setPropertyValuesImpl( std::unique_lock<std::mutex>& rGuard, const css::uno::Sequence< OUString >& PropertyNames, const css::uno::Sequence< css::uno::Any >& Values );
};

#endif // INCLUDED_TOOLKIT_CONTROLS_UNOCONTROLMODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
