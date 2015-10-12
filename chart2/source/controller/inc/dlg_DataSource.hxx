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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DLG_DATASOURCE_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DLG_DATASOURCE_HXX

#include <vcl/tabdlg.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/button.hxx>

#include "TabPageNotifiable.hxx"
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>

#include <memory>

namespace chart
{

class DataSourceTabControl;
class RangeChooserTabPage;
class DataSourceTabPage;
class ChartTypeTemplateProvider;
class DialogModel;

class DataSourceDialog :
        public TabDialog,
        public TabPageNotifiable
{
public:
    explicit DataSourceDialog(
        vcl::Window * pParent,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDocument,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~DataSourceDialog();
    virtual void dispose() override;

    // from Dialog (base of TabDialog)
    virtual short Execute() override;

    // TabPageNotifiable
    virtual void setInvalidPage( TabPage * pTabPage ) override;
    virtual void setValidPage( TabPage * pTabPage ) override;

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >
        m_xChartDocument;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        m_xContext;
    ::std::unique_ptr< ChartTypeTemplateProvider >  m_apDocTemplateProvider;
    ::std::unique_ptr< DialogModel >                m_apDialogModel;

private:
    VclPtr<DataSourceTabControl> m_pTabControl;
    VclPtr<OKButton>             m_pBtnOK;

    VclPtr<RangeChooserTabPage> m_pRangeChooserTabPage;
    VclPtr<DataSourceTabPage>   m_pDataSourceTabPage;
    bool                  m_bRangeChooserTabIsValid;
    bool                  m_bDataSourceTabIsValid;

    static sal_uInt16         m_nLastPageId;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DLG_DATASOURCE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
