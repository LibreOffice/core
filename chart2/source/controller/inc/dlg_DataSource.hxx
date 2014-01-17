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

// header for class TabDialog
#include <vcl/tabdlg.hxx>
// header for class TabControl
#include <vcl/tabctrl.hxx>
// header for class OKButton
#include <vcl/button.hxx>


#include "TabPageNotifiable.hxx"
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>

// for auto_ptr
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
        Window * pParent,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDocument,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~DataSourceDialog();

    // from Dialog (base of TabDialog)
    virtual short Execute();

    // TabPageNotifiable
    virtual void setInvalidPage( TabPage * pTabPage );
    virtual void setValidPage( TabPage * pTabPage );

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >
        m_xChartDocument;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        m_xContext;
    ::std::auto_ptr< ChartTypeTemplateProvider >  m_apDocTemplateProvider;
    ::std::auto_ptr< DialogModel >                m_apDialogModel;

private:
    DataSourceTabControl* m_pTabControl;
    OKButton*             m_pBtnOK;

    RangeChooserTabPage * m_pRangeChooserTabePage;
    DataSourceTabPage *   m_pDataSourceTabPage;
    bool                  m_bRangeChooserTabIsValid;
    bool                  m_bDataSourceTabIsValid;

    static sal_uInt16         m_nLastPageId;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DLG_DATASOURCE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
