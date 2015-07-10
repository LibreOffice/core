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
    ::com::sun::star::chart2::data::XDataSequence,
    ::com::sun::star::chart2::data::XNumericalDataSequence,
    ::com::sun::star::chart2::data::XTextualDataSequence,
    ::com::sun::star::util::XCloneable,
    ::com::sun::star::util::XModifyBroadcaster,
    ::com::sun::star::lang::XInitialization,
    ::com::sun::star::lang::XServiceInfo >
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

    explicit CachedDataSequence(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );

    /** creates a sequence and initializes it with the given string.  This is
        especially useful for labels, which only have one element.
     */
    explicit CachedDataSequence( const OUString & rSingleText );

    /// Copy CTOR
    explicit CachedDataSequence( const CachedDataSequence & rSource );

    virtual ~CachedDataSequence();

    /// establish methods for factory instatiation
    static css::uno::Reference< css::uno::XInterface > SAL_CALL create( css::uno::Reference< css::uno::XComponentContext > const & xContext)
        throw(css::uno::Exception)
    {
        return static_cast<cppu::OWeakObject *>(new CachedDataSequence( xContext ));
    }
    /// declare XServiceInfo methods
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    /// merge XInterface implementations
    DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
    DECLARE_XTYPEPROVIDER()

protected:
    // ____ XPropertySet ____
    /// @see ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    /// @see ::comphelper::OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;
    /// @see ::comphelper::OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const SAL_OVERRIDE;

    // ____ XDataSequence ____
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getData()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getSourceRangeRepresentation()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL generateLabel(
        ::com::sun::star::chart2::data::LabelOrigin nLabelOrigin )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getNumberFormatKeyByIndex( ::sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ XNumericalDataSequence ____
    /// @see ::com::sun::star::chart::data::XNumericalDataSequence
    virtual ::com::sun::star::uno::Sequence< double > SAL_CALL getNumericalData() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ XTextualDataSequence ____
    /// @see ::com::sun::star::chart::data::XTextualDataSequence
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getTextualData() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XInitialization:
    virtual void SAL_CALL initialize(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > & aArguments)
        throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

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
    ::com::sun::star::uno::Sequence< double > Impl_getNumericalData() const;
    /** is used by interface method getTextualData().
     */
    ::com::sun::star::uno::Sequence< OUString > Impl_getTextualData() const;
    /** is used by interface method getData().
     */
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > Impl_getMixedData() const;

private:
    enum DataType                                       m_eCurrentDataType;

    ::com::sun::star::uno::Sequence< double >           m_aNumericalSequence;
    ::com::sun::star::uno::Sequence< OUString >  m_aTextualSequence;
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Any >                    m_aMixedSequence;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >
        m_xModifyEventForwarder;
};

}  // namespace chart

// INCLUDED_CHART2_SOURCE_INC_CACHEDDATASEQUENCE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
