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
#ifndef INCLUDED_CHART2_SOURCE_INC_ERRORBAR_HXX
#define INCLUDED_CHART2_SOURCE_INC_ERRORBAR_HXX

#include "MutexContainer.hxx"
#include "ModifyListenerHelper.hxx"
#include "charttoolsdllapi.hxx"
#include "LineProperties.hxx"

#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>

namespace chart
{

OOO_DLLPUBLIC_CHARTTOOLS    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createErrorBar(
    const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );

namespace impl
{
typedef ::cppu::WeakImplHelper<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::util::XCloneable,
        ::com::sun::star::util::XModifyBroadcaster,
        ::com::sun::star::util::XModifyListener,
        ::com::sun::star::chart2::data::XDataSource,
        ::com::sun::star::chart2::data::XDataSink,
        ::com::sun::star::beans::XPropertySet,
        ::com::sun::star::beans::XPropertyState >
    ErrorBar_Base;
}

class ErrorBar :
        public MutexContainer,
        public impl::ErrorBar_Base,
        public LineProperties
{
private:
    bool mbShowPositiveError;
    bool mbShowNegativeError;
    double mfPositiveError;
    double mfNegativeError;
    double mfWeight;
    sal_Int32 meStyle;

public:
    explicit ErrorBar(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~ErrorBar();

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener )
        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertyState
    virtual com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const OUString& rPropName )
        throw (com::sun::star::beans::UnknownPropertyException, std::exception) override;
    virtual com::sun::star::uno::Sequence< com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates(
            const com::sun::star::uno::Sequence< OUString >& rPropNames )
        throw (com::sun::star::beans::UnknownPropertyException, std::exception) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& rPropName )
        throw (com::sun::star::beans::UnknownPropertyException, std::exception) override;
    virtual com::sun::star::uno::Any SAL_CALL getPropertyDefault( const OUString& rPropName )
        throw (com::sun::star::beans::UnknownPropertyException, com::sun::star::lang::WrappedTargetException, std::exception) override;

protected:
    ErrorBar( const ErrorBar & rOther );

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XDataSink ____
    virtual void SAL_CALL setData( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence > >& aData )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XDataSource ____
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence > > SAL_CALL getDataSequences()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >
                        m_xContext;

    typedef ::std::vector< ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XLabeledDataSequence > > tDataSequenceContainer;
    tDataSequenceContainer m_aDataSequences;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > m_xModifyEventForwarder;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > m_xParent;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_INC_ERRORBAR_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
