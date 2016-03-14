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
#ifndef INCLUDED_CHART2_SOURCE_INC_UNCACHEDDATASEQUENCE_HXX
#define INCLUDED_CHART2_SOURCE_INC_UNCACHEDDATASEQUENCE_HXX

// helper classes
#include <cppuhelper/compbase.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>
#include "charttoolsdllapi.hxx"

// interfaces and types
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifiable.hpp>

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
 * 'label 2', ....</p>
 */
class UncachedDataSequence :
        public ::comphelper::OMutexAndBroadcastHelper,
        public ::comphelper::OPropertyContainer,
        public ::comphelper::OPropertyArrayUsageHelper< UncachedDataSequence >,
        public impl::UncachedDataSequence_Base
{
public:
    /** The referring data provider is held as uno reference to ensure its
        lifetime is at least as long as the one of this object.
     */
    UncachedDataSequence(
        const css::uno::Reference< css::chart2::XInternalDataProvider > & xIntDataProv,
        const OUString & rRangeRepresentation );
    UncachedDataSequence(
        const css::uno::Reference< css::chart2::XInternalDataProvider > & xIntDataProv,
        const OUString & rRangeRepresentation,
        const OUString & rRole );
    UncachedDataSequence( const UncachedDataSequence & rSource );
    virtual ~UncachedDataSequence();

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

    // ____ XIndexReplace ____
    virtual void SAL_CALL replaceByIndex( ::sal_Int32 Index, const css::uno::Any& Element )
        throw (css::lang::IllegalArgumentException,
               css::lang::IndexOutOfBoundsException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception) override;

    // ____ XIndexAccess (base of XIndexReplace) ____
    virtual ::sal_Int32 SAL_CALL getCount()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index )
        throw (css::lang::IndexOutOfBoundsException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception) override;

    // ____ XElementAccess (base of XIndexAccess) ____
    virtual css::uno::Type SAL_CALL getElementType()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XNamed (for setting a new range representation) ____
    virtual OUString SAL_CALL getName()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setName( const OUString& aName )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XModifiable ____
    virtual sal_Bool SAL_CALL isModified()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setModified( sal_Bool bModified )
        throw (css::beans::PropertyVetoException,
               css::uno::RuntimeException, std::exception) override;

    // ____ XModifyBroadcaster (base of XModifiable) ____
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;

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

private:
    css::uno::Reference< css::chart2::XInternalDataProvider > m_xDataProvider;
    OUString                 m_aSourceRepresentation;
    css::uno::Reference< css::util::XModifyListener >
        m_xModifyEventForwarder;
};

}  // namespace chart

// INCLUDED_CHART2_SOURCE_INC_UNCACHEDDATASEQUENCE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
