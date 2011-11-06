/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
