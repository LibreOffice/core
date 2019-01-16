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

#include <dlg_ChartType_UNO.hxx>
#include <dlg_ChartType.hxx>
#include <servicenames.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/frame/XModel.hpp>

namespace com { namespace sun { namespace star { namespace awt { class XWindow; } } } }

namespace chart
{
using namespace ::com::sun::star;
ChartTypeUnoDlg::ChartTypeUnoDlg( const uno::Reference< uno::XComponentContext >& _xContext )
                    : ChartTypeUnoDlg_BASE( _xContext )
{
}
ChartTypeUnoDlg::~ChartTypeUnoDlg()
{
    // we do this here cause the base class' call to destroyDialog won't reach us anymore: we're within an dtor,
    // so this virtual-method-call the base class does not work, we're already dead then...
    if (m_aDialog)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if (m_aDialog)
            destroyDialog();
    }
}
// lang::XServiceInfo
OUString SAL_CALL ChartTypeUnoDlg::getImplementationName()
{
    return OUString(CHART_TYPE_DIALOG_SERVICE_IMPLEMENTATION_NAME);
}

css::uno::Sequence<OUString> SAL_CALL ChartTypeUnoDlg::getSupportedServiceNames()
{
    return { CHART_TYPE_DIALOG_SERVICE_NAME };
}
uno::Sequence< sal_Int8 > SAL_CALL ChartTypeUnoDlg::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}
void ChartTypeUnoDlg::implInitialize(const uno::Any& _rValue)
{
    beans::PropertyValue aProperty;
    if (_rValue >>= aProperty)
    {
        if (aProperty.Name == "ChartModel")
            m_xChartModel.set(aProperty.Value,uno::UNO_QUERY);
        else
            ChartTypeUnoDlg_BASE::implInitialize(_rValue);
    }
    else
        ChartTypeUnoDlg_BASE::implInitialize(_rValue);
}
svt::OGenericUnoDialog::Dialog ChartTypeUnoDlg::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
{
    return svt::OGenericUnoDialog::Dialog(std::make_unique<ChartTypeDialog>(Application::GetFrameWeld(rParent), m_xChartModel));
}
uno::Reference<beans::XPropertySetInfo>  SAL_CALL ChartTypeUnoDlg::getPropertySetInfo()
{
    return createPropertySetInfo( getInfoHelper() );
}

::cppu::IPropertyArrayHelper& ChartTypeUnoDlg::getInfoHelper()
{
    return *getArrayHelper();
}

::cppu::IPropertyArrayHelper* ChartTypeUnoDlg::createArrayHelper( ) const
{
    uno::Sequence< beans::Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

} //namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_ChartTypeDialog_get_implementation(css::uno::XComponentContext *context,
                                                            css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new chart::ChartTypeUnoDlg(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
