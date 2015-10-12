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
#ifndef INCLUDED_CHART2_SOURCE_INC_LABELEDDATASEQUENCE_HXX
#define INCLUDED_CHART2_SOURCE_INC_LABELEDDATASEQUENCE_HXX

#include "MutexContainer.hxx"
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/chart2/data/XLabeledDataSequence2.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

namespace chart
{

namespace impl
{
typedef cppu::WeakImplHelper<
        ::com::sun::star::chart2::data::XLabeledDataSequence2,
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

    /// declare XServiceInfo methods
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

protected:
    // ____ XLabeledDataSequence ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence > SAL_CALL getValues()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setValues(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >& xSequence )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence > SAL_CALL getLabel()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLabel(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >& xSequence )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

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

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence > m_xData;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence > m_xLabel;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >      m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >       m_xModifyEventForwarder;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_INC_LABELEDDATASEQUENCE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
