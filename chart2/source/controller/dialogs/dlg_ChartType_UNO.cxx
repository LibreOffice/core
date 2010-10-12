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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
