/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef CHART2_LABELEDDATASEQUENCE_HXX
#define CHART2_LABELEDDATASEQUENCE_HXX
#include "ServiceMacros.hxx"
#include "MutexContainer.hxx"
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase4.hxx>

#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

namespace chart
{

namespace impl
{
typedef cppu::WeakImplHelper4<
        ::com::sun::star::chart2::data::XLabeledDataSequence,
        ::com::sun::star::util::XCloneable,
        ::com::sun::star::util::XModifyBroadcaster,
        ::com::sun::star::lang::XServiceInfo >
    LabeledDataSequence_Base;
}

class LabeledDataSequence :
        public MutexContainer,
        public impl::LabeledDataSequence_Base
{
public:
    explicit LabeledDataSequence(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    explicit LabeledDataSequence(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSequence > & rValues );
    explicit LabeledDataSequence(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSequence > & rValues,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSequence > & rLabels );

    virtual ~LabeledDataSequence();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( LabeledDataSequence )
    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()

protected:
    // ____ XLabeledDataSequence ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence > SAL_CALL getValues()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setValues(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >& xSequence )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence > SAL_CALL getLabel()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLabel(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >& xSequence )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence > m_xData;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence > m_xLabel;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >      m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >       m_xModifyEventForwarder;
};

} //  namespace chart

// CHART2_LABELEDDATASEQUENCE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
