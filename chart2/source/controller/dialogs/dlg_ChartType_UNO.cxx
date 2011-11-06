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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "dlg_ChartType_UNO.hxx"
#include "dlg_ChartType.hxx"
#include "servicenames.hxx"
#include <osl/mutex.hxx>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
uno::Reference< uno::XInterface >
            SAL_CALL ChartTypeUnoDlg::Create(const uno::Reference< uno::XComponentContext >& _xContext)
{
    return *(new ChartTypeUnoDlg(_xContext));
}
// -----------------------------------------------------------------------------
ChartTypeUnoDlg::ChartTypeUnoDlg( const uno::Reference< uno::XComponentContext >& _xContext )
                    : ChartTypeUnoDlg_BASE( _xContext )
{
}
// -----------------------------------------------------------------------------
ChartTypeUnoDlg::~ChartTypeUnoDlg()
{
    // we do this here cause the base class' call to destroyDialog won't reach us anymore : we're within an dtor,
    // so this virtual-method-call the base class does does not work, we're already dead then ...
    if (m_pDialog)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if (m_pDialog)
            destroyDialog();
    }
}
//-------------------------------------------------------------------------
// lang::XServiceInfo
//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL ChartTypeUnoDlg::getImplementationName() throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}

//-------------------------------------------------------------------------
::rtl::OUString ChartTypeUnoDlg::getImplementationName_Static() throw(uno::RuntimeException)
{
    return CHART_TYPE_DIALOG_SERVICE_IMPLEMENTATION_NAME;
}

//-------------------------------------------------------------------------
::comphelper::StringSequence SAL_CALL ChartTypeUnoDlg::getSupportedServiceNames() throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//-------------------------------------------------------------------------

uno::Sequence< rtl::OUString > ChartTypeUnoDlg::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = CHART_TYPE_DIALOG_SERVICE_NAME;
    return aSNS;
}
//-------------------------------------------------------------------------
uno::Sequence< sal_Int8 > SAL_CALL ChartTypeUnoDlg::getImplementationId( void ) throw( uno::RuntimeException )
{
    static ::cppu::OImplementationId aId;
    return aId.getImplementationId();
}
//------------------------------------------------------------------------------
void ChartTypeUnoDlg::implInitialize(const uno::Any& _rValue)
{
    beans::PropertyValue aProperty;
    if (_rValue >>= aProperty)
    {
        if (0 == aProperty.Name.compareToAscii("ChartModel"))
            m_xChartModel.set(aProperty.Value,uno::UNO_QUERY);
        else
            ChartTypeUnoDlg_BASE::implInitialize(_rValue);
    }
    else
        ChartTypeUnoDlg_BASE::implInitialize(_rValue);
}
//------------------------------------------------------------------------------
Dialog* ChartTypeUnoDlg::createDialog(Window* _pParent)
{
    return new ChartTypeDialog( _pParent, m_xChartModel, m_aContext.getUNOContext() );
}
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
uno::Reference<beans::XPropertySetInfo>  SAL_CALL ChartTypeUnoDlg::getPropertySetInfo() throw(uno::RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() );
}

//-------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& ChartTypeUnoDlg::getInfoHelper()
{
    return *const_cast<ChartTypeUnoDlg*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ChartTypeUnoDlg::createArrayHelper( ) const
{
    uno::Sequence< beans::Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

//.............................................................................
} //namespace chart
//.............................................................................
