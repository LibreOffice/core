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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_UPDOWNBARWRAPPER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_UPDOWNBARWRAPPER_HXX

#include "MutexContainer.hxx"
#include <comphelper/interfacecontainer2.hxx>

#include <com/sun/star/chart2/XDiagram.hpp>

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <memory>

namespace chart
{

namespace wrapper
{

class Chart2ModelContact;

class UpDownBarWrapper : public MutexContainer
                       , public ::cppu::WeakImplHelper
                        < css::lang::XComponent
                        , css::lang::XServiceInfo
                        , css::beans::XPropertySet
                        , css::beans::XMultiPropertySet
                        , css::beans::XPropertyState
                        , css::beans::XMultiPropertyStates
                        >
{
public:
    UpDownBarWrapper( bool bUp, std::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~UpDownBarWrapper();

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    // ____ XComponent ____
    virtual void SAL_CALL dispose()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference<
                                            css::lang::XEventListener >& xListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference<
                                               css::lang::XEventListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    //XMultiPropertySet
    //getPropertySetInfo() already declared in XPropertySet
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) throw (css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

    //XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    //XMultiPropertyStates
    //getPropertyStates() already declared in XPropertyState
    virtual void SAL_CALL setAllPropertiesToDefault(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertiesToDefault( const css::uno::Sequence< OUString >& aPropertyNames ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyDefaults( const css::uno::Sequence< OUString >& aPropertyNames ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

private: //methods
    static ::cppu::IPropertyArrayHelper&   getInfoHelper();

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    ::comphelper::OInterfaceContainerHelper2      m_aEventListenerContainer;

    OUString       m_aPropertySetName;
};

} //  namespace wrapper
} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_UPDOWNBARWRAPPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
