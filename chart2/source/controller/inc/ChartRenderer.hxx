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


#ifndef CHART_RENDERER_HXX
#define CHART_RENDERER_HXX

#include <svtools/chartprettypainter.hxx>
#include "ServiceMacros.hxx"

#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/weakref.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

//#i82893#, #i75867#: charts must be painted resolution dependent!!
class ChartRenderer : public ::cppu::WeakImplHelper2<
          ::com::sun::star::lang::XServiceInfo
        , ::com::sun::star::lang::XUnoTunnel
        >, public ChartPrettyPainter
{
public:
    ChartRenderer( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel );
    virtual ~ChartRenderer();

    // ___lang::XServiceInfo___
    APPHELPER_XSERVICEINFO_DECL()

    // ____ XUnoTunnel ___
    virtual ::sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aIdentifier )
            throw (::com::sun::star::uno::RuntimeException);

    // ____ ChartPrettyPainter ___
    virtual bool DoPaint(OutputDevice* pOutDev, const Rectangle& rLogicObjectRect) const;

private:
    ChartRenderer();
    ChartRenderer( const ChartRenderer& );

    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XModel > m_xChartModel;
};

//.............................................................................
}  // namespace chart
//.............................................................................

#endif
