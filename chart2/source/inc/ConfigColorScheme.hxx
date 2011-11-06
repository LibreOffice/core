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


#ifndef CHART2_CONFIG_COLOR_SCHEME_HXX
#define CHART2_CONFIG_COLOR_SCHEME_HXX

#include "ServiceMacros.hxx"
#include "ConfigItemListener.hxx"
#include "charttoolsdllapi.hxx"
#include <cppuhelper/implbase2.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/chart2/XColorScheme.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <memory>


namespace chart
{

OOO_DLLPUBLIC_CHARTTOOLS ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XColorScheme > createConfigColorScheme(
    const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );

namespace impl
{
class ChartConfigItem;
}

class ConfigColorScheme :
    public ConfigItemListener,
    public ::cppu::WeakImplHelper2<
        ::com::sun::star::chart2::XColorScheme,
        ::com::sun::star::lang::XServiceInfo >
{
public:
    explicit ConfigColorScheme(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    SAL_DLLPRIVATE virtual ~ConfigColorScheme();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( ConfigColorScheme )
    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()

protected:
    // ____ XColorScheme ____
    SAL_DLLPRIVATE virtual ::sal_Int32 SAL_CALL getColorByIndex( ::sal_Int32 nIndex )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ ConfigItemListener ____
    SAL_DLLPRIVATE virtual void notify( const ::rtl::OUString & rPropertyName );

private:
    SAL_DLLPRIVATE void retrieveConfigColors();

    // member variables
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >        m_xContext;
    ::std::auto_ptr< impl::ChartConfigItem >              m_apChartConfigItem;
    mutable ::com::sun::star::uno::Sequence< sal_Int64 >  m_aColorSequence;
    mutable sal_Int32                                     m_nNumberOfColors;
    bool                                                  m_bNeedsUpdate;
};

} // namespace chart

// CHART2_CONFIG_COLOR_SCHEME_HXX
#endif
