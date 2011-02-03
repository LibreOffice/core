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
#ifndef CHART_DATAINTERPRETER_HXX
#define CHART_DATAINTERPRETER_HXX

#include "ServiceMacros.hxx"
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/chart2/XDataInterpreter.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace chart
{

class DataInterpreter : public ::cppu::WeakImplHelper2<
        ::com::sun::star::chart2::XDataInterpreter,
        ::com::sun::star::lang::XServiceInfo >
{
public:
    explicit DataInterpreter( const ::com::sun::star::uno::Reference<
                                  ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~DataInterpreter();

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        GetComponentContext() const;

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

    // convenience methods
    static  ::rtl::OUString GetRole(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSequence > & xSeq );

    static void SetRole(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSequence > & xSeq,
        const ::rtl::OUString & rRole );

    static ::com::sun::star::uno::Any GetProperty(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue > & aArguments,
        const ::rtl::OUString & rName );

    static bool HasCategories(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue > & rArguments,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XLabeledDataSequence > > & rData );

    static bool UseCategoriesAsX(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue > & rArguments );

protected:
    // ____ XDataInterpreter ____
    virtual ::com::sun::star::chart2::InterpretedData SAL_CALL interpretDataSource(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource >& xSource,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArguments,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > >& aSeriesToReUse )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::chart2::InterpretedData SAL_CALL reinterpretDataSeries(
        const ::com::sun::star::chart2::InterpretedData& aInterpretedData )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isDataCompatible(
        const ::com::sun::star::chart2::InterpretedData& aInterpretedData )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource > SAL_CALL mergeInterpretedData(
        const ::com::sun::star::chart2::InterpretedData& aInterpretedData )
        throw (::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        m_xContext;
};

} // namespace chart

// CHART_DATAINTERPRETER_HXX
#endif
