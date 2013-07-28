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

#include "dlg_ChartType_UNO.hxx"
#include "dlg_ChartType.hxx"
#include "servicenames.hxx"
#include <osl/mutex.hxx>

namespace chart
{
using namespace ::com::sun::star;
uno::Reference< uno::XInterface >
            SAL_CALL ChartTypeUnoDlg::Create(const uno::Reference< uno::XComponentContext >& _xContext)
{
    return *(new ChartTypeUnoDlg(_xContext));
}
ChartTypeUnoDlg::ChartTypeUnoDlg( const uno::Reference< uno::XComponentContext >& _xContext )
                    : ChartTypeUnoDlg_BASE( _xContext )
{
}
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
// lang::XServiceInfo
OUString SAL_CALL ChartTypeUnoDlg::getImplementationName() throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}

OUString ChartTypeUnoDlg::getImplementationName_Static() throw(uno::RuntimeException)
{
    return CHART_TYPE_DIALOG_SERVICE_IMPLEMENTATION_NAME;
}

::comphelper::StringSequence SAL_CALL ChartTypeUnoDlg::getSupportedServiceNames() throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}

uno::Sequence< OUString > ChartTypeUnoDlg::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = CHART_TYPE_DIALOG_SERVICE_NAME;
    return aSNS;
}
uno::Sequence< sal_Int8 > SAL_CALL ChartTypeUnoDlg::getImplementationId( void ) throw( uno::RuntimeException )
{
    static ::cppu::OImplementationId aId;
    return aId.getImplementationId();
}
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
Dialog* ChartTypeUnoDlg::createDialog(Window* _pParent)
{
    return new ChartTypeDialog( _pParent, m_xChartModel, m_aContext );
}
uno::Reference<beans::XPropertySetInfo>  SAL_CALL ChartTypeUnoDlg::getPropertySetInfo() throw(uno::RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() );
}

::cppu::IPropertyArrayHelper& ChartTypeUnoDlg::getInfoHelper()
{
    return *const_cast<ChartTypeUnoDlg*>(this)->getArrayHelper();
}

::cppu::IPropertyArrayHelper* ChartTypeUnoDlg::createArrayHelper( ) const
{
    uno::Sequence< beans::Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
