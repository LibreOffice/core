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
#pragma once

// helper classes
#include <comphelper/compbase.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/propertycontainer2.hxx>
#include <comphelper/proparrhlp.hxx>
#include <rtl/ref.hxx>

// interfaces and types
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifiable.hpp>

namespace com::sun::star::beans { class XPropertySetInfo; }
namespace com::sun::star::chart2 { class XInternalDataProvider; }

namespace chart
{
class InternalDataProvider;
class ModifyEventForwarder;

namespace impl
{
typedef ::comphelper::WeakComponentImplHelper<
    css::chart2::data::XDataSequence,
    css::chart2::data::XNumericalDataSequence,
    css::chart2::data::XTextualDataSequence,
    css::util::XCloneable,
    css::util::XModifiable, // contains util::XModifyBroadcaster
    css::container::XIndexReplace,
    css::container::XNamed, // for setting a new range representation
    css::lang::XServiceInfo >
    UncachedDataSequence_Base;
}

/**
 * This sequence object does NOT store actual sequence data.  Instead, it
 * references a column inside the internal data table (represented by class
 * InternalData) via range representation string.  The range representation
 * string ends with a numeric value that indicates the column index within
 * the internal data table.
 *
 * <p>A range representation can be either '0', '1', '2', ..., or 'label 1',
 * 'label 2', ...</p>
 */
class UncachedDataSequence final :
        public ::comphelper::OPropertyContainer2,
        public ::comphelper::OPropertyArrayUsageHelper< UncachedDataSequence >,
        public impl::UncachedDataSequence_Base
{
public:
    /** The referring data provider is held as uno reference to ensure its
        lifetime is at least as long as the one of this object.
     */
    UncachedDataSequence(
        rtl::Reference< InternalDataProvider > xIntDataProv,
        OUString aRangeRepresentation );
    UncachedDataSequence(
        rtl::Reference< InternalDataProvider > xIntDataProv,
        OUString aRangeRepresentation,
        const OUString & rRole );
    UncachedDataSequence( const UncachedDataSequence & rSource );
    virtual ~UncachedDataSequence() override;

    /// declare XServiceInfo methods
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    /// merge XInterface implementations
    DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
    DECLARE_XTYPEPROVIDER()

private:
    // ____ XPropertySet ____
    /// @see css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    /// @see ::comphelper::OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& getInfoHelper() override;
    /// @see ::comphelper::OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const override;

    // ____ XDataSequence ____
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getData() override;
    virtual OUString SAL_CALL getSourceRangeRepresentation() override;
    virtual css::uno::Sequence< OUString > SAL_CALL generateLabel(
        css::chart2::data::LabelOrigin nLabelOrigin ) override;
    virtual ::sal_Int32 SAL_CALL getNumberFormatKeyByIndex( ::sal_Int32 nIndex ) override;

    // ____ XNumericalDataSequence ____
    /// @see css::chart::data::XNumericalDataSequence
    virtual css::uno::Sequence< double > SAL_CALL getNumericalData() override;

    // ____ XTextualDataSequence ____
    /// @see css::chart::data::XTextualDataSequence
    virtual css::uno::Sequence< OUString > SAL_CALL getTextualData() override;

    // ____ XIndexReplace ____
    virtual void SAL_CALL replaceByIndex( ::sal_Int32 Index, const css::uno::Any& Element ) override;

    // ____ XIndexAccess (base of XIndexReplace) ____
    virtual ::sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override;

    // ____ XElementAccess (base of XIndexAccess) ____
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // ____ XNamed (for setting a new range representation) ____
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName( const OUString& aName ) override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    // ____ XModifiable ____
    virtual sal_Bool SAL_CALL isModified() override;
    virtual void SAL_CALL setModified( sal_Bool bModified ) override;

    // ____ XModifyBroadcaster (base of XModifiable) ____
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

    void fireModifyEvent();

    // <properties>
    sal_Int32                                       m_nNumberFormatKey;
    OUString                                 m_sRole;
    OUString                                 m_aXMLRange;
    // </properties>

    /** This method registers all properties.  It should be called by all
        constructors.
     */
    void registerProperties();

    rtl::Reference< InternalDataProvider > m_xDataProvider;
    OUString m_aSourceRepresentation;
    rtl::Reference<ModifyEventForwarder> m_xModifyEventForwarder;
};

}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
