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
#ifndef _CHART_UNCACHEDDATASEQUENCE_HXX
#define _CHART_UNCACHEDDATASEQUENCE_HXX

// helper classes
#include <cppuhelper/compbase8.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>
#include "ServiceMacros.hxx"
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
typedef ::cppu::WeakComponentImplHelper8<
    ::com::sun::star::chart2::data::XDataSequence,
    ::com::sun::star::chart2::data::XNumericalDataSequence,
    ::com::sun::star::chart2::data::XTextualDataSequence,
    ::com::sun::star::util::XCloneable,
    ::com::sun::star::util::XModifiable, // contains util::XModifyBroadcaster
    ::com::sun::star::container::XIndexReplace,
    ::com::sun::star::container::XNamed, // for setting a new range representation
    ::com::sun::star::lang::XServiceInfo >
    UncachedDataSequence_Base;
}

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
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XInternalDataProvider > & xIntDataProv,
        const OUString & rRangeRepresentation );
    UncachedDataSequence(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XInternalDataProvider > & xIntDataProv,
        const OUString & rRangeRepresentation,
        const OUString & rRole );
    UncachedDataSequence( const UncachedDataSequence & rSource );
    virtual ~UncachedDataSequence();

    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()

    /// merge XInterface implementations
    DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
    DECLARE_XTYPEPROVIDER()

protected:
    // ____ XPropertySet ____
    /// @see ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);
    /// @see ::comphelper::OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    /// @see ::comphelper::OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const;

    // ____ XDataSequence ____
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getData()
        throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getSourceRangeRepresentation()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL generateLabel(
        ::com::sun::star::chart2::data::LabelOrigin nLabelOrigin )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getNumberFormatKeyByIndex( ::sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XNumericalDataSequence ____
    /// @see ::com::sun::star::chart::data::XNumericalDataSequence
    virtual ::com::sun::star::uno::Sequence< double > SAL_CALL getNumericalData() throw (::com::sun::star::uno::RuntimeException);

    // ____ XTextualDataSequence ____
    /// @see ::com::sun::star::chart::data::XTextualDataSequence
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getTextualData() throw (::com::sun::star::uno::RuntimeException);

    // ____ XIndexReplace ____
    virtual void SAL_CALL replaceByIndex( ::sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XIndexAccess (base of XIndexReplace) ____
    virtual ::sal_Int32 SAL_CALL getCount()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XElementAccess (base of XIndexAccess) ____
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasElements()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XNamed (for setting a new range representation) ____
    virtual OUString SAL_CALL getName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setName( const OUString& aName )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XModifiable ____
    virtual ::sal_Bool SAL_CALL isModified()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setModified( ::sal_Bool bModified )
        throw (::com::sun::star::beans::PropertyVetoException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XModifyBroadcaster (base of XModifiable) ____
    virtual void SAL_CALL addModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    void fireModifyEvent();

    mutable ::osl::Mutex                  m_aMutex;

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
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XInternalDataProvider > m_xDataProvider;
    OUString                 m_aSourceRepresentation;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >
        m_xModifyEventForwarder;
};

}  // namespace chart

// _CHART_UNCACHEDDATASEQUENCE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
