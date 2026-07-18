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

// interfaces and types
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include "ModifyListenerHelper.hxx"

namespace com::sun::star::uno { class XComponentContext; }

namespace chart
{
using CachedDataSequence_Base = comphelper::OPropertyContainerImplHelper<
          comphelper::WeakComponentImplHelper<
              css::chart2::data::XDataSequence,
              css::chart2::data::XNumericalDataSequence,
              css::chart2::data::XTextualDataSequence,
              css::util::XCloneable,
              css::util::XModifyBroadcaster,
              css::lang::XInitialization,
              css::lang::XServiceInfo>,
          class CachedDataSequence>;

/**
 * This sequence object does store actual values within, hence "cached".
 */
class CachedDataSequence final
    : public CachedDataSequence_Base
{
public:
    /** constructs an empty sequence
     */
    CachedDataSequence();

    explicit CachedDataSequence( const css::uno::Reference< css::uno::XComponentContext > & xContext );

    /** creates a sequence and initializes it with the given string.  This is
        especially useful for labels, which only have one element.
     */
    explicit CachedDataSequence( const OUString & rSingleText );

    /// Copy CTOR
    explicit CachedDataSequence( const CachedDataSequence & rSource );

    virtual ~CachedDataSequence() override;

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

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > createClone() override;

    // ____ XModifyBroadcaster ____
    virtual void addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

    // css::lang::XInitialization:
    virtual void initialize(const cpo::uno::Sequence< cpo::uno::Any > & aArguments) override;

    // <properties>
    sal_Int32                                       m_nNumberFormatKey;
    OUString                                 m_sRole;
    // </properties>

    enum DataType
    {
        NUMERICAL,
        TEXTUAL,
        MIXED
    };

    /** This method registers all properties.  It should be called by all
        constructors.
     */
    void registerProperties();

    /** is used by interface method getNumericalData().
     */
    cpo::uno::Sequence< double > Impl_getNumericalData() const;
    /** is used by interface method getTextualData().
     */
    cpo::uno::Sequence< OUString > Impl_getTextualData() const;
    /** is used by interface method getData().
     */
    cpo::uno::Sequence< cpo::uno::Any > Impl_getMixedData() const;

    enum DataType                                          m_eCurrentDataType;

    cpo::uno::Sequence< double >                           m_aNumericalSequence;
    cpo::uno::Sequence< OUString >                         m_aTextualSequence;
    cpo::uno::Sequence< cpo::uno::Any >                    m_aMixedSequence;
    rtl::Reference<ModifyEventForwarder> m_xModifyEventForwarder;
};

}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
