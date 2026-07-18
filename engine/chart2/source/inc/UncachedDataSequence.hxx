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

// helper classes
#include <comphelper/propcontainerimplhelper.hxx>
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

namespace chart
{
class InternalDataProvider;
class ModifyEventForwarder;

using UncachedDataSequence_Base = comphelper::OPropertyContainerImplHelper<
          comphelper::WeakComponentImplHelper<
              css::chart2::data::XDataSequence,
              css::chart2::data::XNumericalDataSequence,
              css::chart2::data::XTextualDataSequence,
              css::util::XCloneable,
              css::util::XModifiable,
              css::container::XIndexReplace,
              css::container::XNamed,
              css::lang::XServiceInfo>,
          class UncachedDataSequence>;

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
class UncachedDataSequence final
    : public UncachedDataSequence_Base
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
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

private:
    /// @see ::comphelper::OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const override;

    // ____ XDataSequence ____
    virtual cpo::uno::Sequence< cpo::uno::Any > getData() override;
    virtual OUString getSourceRangeRepresentation() override;
    virtual cpo::uno::Sequence< OUString > generateLabel(
        css::chart2::data::LabelOrigin nLabelOrigin ) override;
    virtual ::sal_Int32 getNumberFormatKeyByIndex( ::sal_Int32 nIndex ) override;

    // ____ XNumericalDataSequence ____
    /// @see css::chart::data::XNumericalDataSequence
    virtual cpo::uno::Sequence< double > getNumericalData() override;

    // ____ XTextualDataSequence ____
    /// @see css::chart::data::XTextualDataSequence
    virtual cpo::uno::Sequence< OUString > getTextualData() override;

    // ____ XIndexReplace ____
    virtual void replaceByIndex( ::sal_Int32 Index, const cpo::uno::Any& Element ) override;

    // ____ XIndexAccess (base of XIndexReplace) ____
    virtual ::sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( ::sal_Int32 Index ) override;

    // ____ XElementAccess (base of XIndexAccess) ____
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

    // ____ XNamed (for setting a new range representation) ____
    virtual OUString getName() override;
    virtual void setName( const OUString& aName ) override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > createClone() override;

    // ____ XModifiable ____
    virtual bool isModified() override;
    virtual void setModified( bool bModified ) override;

    // ____ XModifyBroadcaster (base of XModifiable) ____
    virtual void addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

    void fireModifyEvent();

    // <properties>
    sal_Int32                                       m_nNumberFormatKey;
    OUString                                 m_sRole;
    OUString                                 m_aXMLRange;
    OUString                                 m_aChartExFormula;
    OUString                                 m_aChartExNFormula;
    sal_Int32                                       m_nChartExDimType;
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
