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
#ifndef INCLUDED_CHART2_SOURCE_INC_CACHEDDATASEQUENCE_HXX
#define INCLUDED_CHART2_SOURCE_INC_CACHEDDATASEQUENCE_HXX

// helper classes
#include <cppuhelper/compbase.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>

// interfaces and types
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <vector>

namespace chart
{

namespace impl
{
typedef ::cppu::WeakComponentImplHelper<
    css::chart2::data::XDataSequence,
    css::chart2::data::XNumericalDataSequence,
    css::chart2::data::XTextualDataSequence,
    css::util::XCloneable,
    css::util::XModifyBroadcaster,
    css::lang::XInitialization,
    css::lang::XServiceInfo >
    CachedDataSequence_Base;
}

/**
 * This sequence object does store actual values within, hence "cached".
 */
class CachedDataSequence :
        public ::comphelper::OMutexAndBroadcastHelper,
        public ::comphelper::OPropertyContainer,
        public ::comphelper::OPropertyArrayUsageHelper< CachedDataSequence >,
        public impl::CachedDataSequence_Base
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

    virtual ~CachedDataSequence();

    /// declare XServiceInfo methods
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    /// merge XInterface implementations
    DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
    DECLARE_XTYPEPROVIDER()

protected:
    // ____ XPropertySet ____
    /// @see css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw (css::uno::RuntimeException, std::exception) override;
    /// @see ::comphelper::OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
    /// @see ::comphelper::OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const override;

    // ____ XDataSequence ____
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getData()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getSourceRangeRepresentation()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL generateLabel(
        css::chart2::data::LabelOrigin nLabelOrigin )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getNumberFormatKeyByIndex( ::sal_Int32 nIndex )
        throw (css::lang::IndexOutOfBoundsException,
               css::uno::RuntimeException, std::exception) override;

    // ____ XNumericalDataSequence ____
    /// @see css::chart::data::XNumericalDataSequence
    virtual css::uno::Sequence< double > SAL_CALL getNumericalData() throw (css::uno::RuntimeException, std::exception) override;

    // ____ XTextualDataSequence ____
    /// @see css::chart::data::XTextualDataSequence
    virtual css::uno::Sequence< OUString > SAL_CALL getTextualData() throw (css::uno::RuntimeException, std::exception) override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;

    // css::lang::XInitialization:
    virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any > & aArguments)
        throw (css::uno::RuntimeException, css::uno::Exception, std::exception) override;

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

private:
    /** is used by interface method getNumericalData().
     */
    css::uno::Sequence< double > Impl_getNumericalData() const;
    /** is used by interface method getTextualData().
     */
    css::uno::Sequence< OUString > Impl_getTextualData() const;
    /** is used by interface method getData().
     */
    css::uno::Sequence< css::uno::Any > Impl_getMixedData() const;

private:
    enum DataType                                          m_eCurrentDataType;

    css::uno::Sequence< double >                           m_aNumericalSequence;
    css::uno::Sequence< OUString >                         m_aTextualSequence;
    css::uno::Sequence< css::uno::Any >                    m_aMixedSequence;
    css::uno::Reference< css::util::XModifyListener >      m_xModifyEventForwarder;
};

}  // namespace chart

// INCLUDED_CHART2_SOURCE_INC_CACHEDDATASEQUENCE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
