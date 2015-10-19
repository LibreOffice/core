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

#include "dlg_DataSource.hxx"
#include "Strings.hrc"
#include "ResId.hxx"
#include "ChartTypeTemplateProvider.hxx"
#include "DiagramHelper.hxx"
#include "DialogModel.hxx"
#include "HelpIds.hrc"

#include "tp_RangeChooser.hxx"
#include "tp_DataSource.hxx"

#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx> // for RET_OK

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::chart;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

class DocumentChartTypeTemplateProvider : public ChartTypeTemplateProvider
{
public:
    explicit DocumentChartTypeTemplateProvider(
        const Reference< chart2::XChartDocument > & xDoc );
    virtual ~DocumentChartTypeTemplateProvider();

    // ____ ChartTypeTemplateProvider ____
    virtual Reference< chart2::XChartTypeTemplate > getCurrentTemplate() const override;

private:
    Reference< chart2::XChartTypeTemplate > m_xTemplate;
};

DocumentChartTypeTemplateProvider::DocumentChartTypeTemplateProvider(
    const Reference< chart2::XChartDocument > & xDoc )
{
    if( xDoc.is())
    {
        Reference< chart2::XDiagram > xDia( xDoc->getFirstDiagram());
        if( xDia.is())
        {
            DiagramHelper::tTemplateWithServiceName aResult(
                DiagramHelper::getTemplateForDiagram(
                    xDia,
                    Reference< lang::XMultiServiceFactory >(
                        xDoc->getChartTypeManager(), uno::UNO_QUERY ) ));
            m_xTemplate.set( aResult.first );
        }
    }
}

DocumentChartTypeTemplateProvider::~DocumentChartTypeTemplateProvider()
{}

Reference< chart2::XChartTypeTemplate > DocumentChartTypeTemplateProvider::getCurrentTemplate() const
{
    return m_xTemplate;
}

class DataSourceTabControl : public TabControl
{
public:
    explicit DataSourceTabControl(vcl::Window* pParent);

    virtual bool DeactivatePage() override;

    void DisableTabToggling();
    void EnableTabToggling();

private:
    bool m_bTogglingEnabled;
};

DataSourceTabControl::DataSourceTabControl(vcl::Window* pParent)
    : TabControl(pParent)
    , m_bTogglingEnabled(true)
{
}

bool DataSourceTabControl::DeactivatePage()
{
    return TabControl::DeactivatePage() && m_bTogglingEnabled;
}

void DataSourceTabControl::DisableTabToggling()
{
    m_bTogglingEnabled = false;
}

void DataSourceTabControl::EnableTabToggling()
{
    m_bTogglingEnabled = true;
}

sal_uInt16 DataSourceDialog::m_nLastPageId = 0;

enum DataSourceDialogPages
{
    TP_RANGECHOOSER = 1,
    TP_DATA_SOURCE = 2
};

DataSourceDialog::DataSourceDialog(vcl::Window * pParent,
    const Reference< XChartDocument > & xChartDocument,
    const Reference< uno::XComponentContext > & xContext)
    : TabDialog(pParent, "DataRangeDialog",
        "modules/schart/ui/datarangedialog.ui")
    , m_xChartDocument(xChartDocument)
    , m_xContext(xContext)
    , m_apDocTemplateProvider(new DocumentChartTypeTemplateProvider(xChartDocument))
    , m_apDialogModel(new DialogModel(xChartDocument, xContext))
    , m_pTabControl(VclPtr<DataSourceTabControl>::Create(get_content_area()))
    , m_pRangeChooserTabPage(0)
    , m_pDataSourceTabPage(0)
    , m_bRangeChooserTabIsValid(true)
    , m_bDataSourceTabIsValid(true)
{
    get(m_pBtnOK, "ok");

    m_pTabControl->Show();

    m_pRangeChooserTabPage = VclPtr<RangeChooserTabPage>::Create( m_pTabControl, *(m_apDialogModel.get()),
                                     m_apDocTemplateProvider.get(), this, true /* bHideDescription */ );
    m_pDataSourceTabPage = VclPtr<DataSourceTabPage>::Create( m_pTabControl, *(m_apDialogModel.get()),
                                    m_apDocTemplateProvider.get(), this, true /* bHideDescription */ );

    m_pTabControl->InsertPage( TP_RANGECHOOSER, SCH_RESSTR(STR_PAGE_DATA_RANGE) );
    m_pTabControl->InsertPage( TP_DATA_SOURCE,  SCH_RESSTR(STR_OBJECT_DATASERIES_PLURAL) );

    m_pTabControl->SetTabPage( TP_DATA_SOURCE,  m_pDataSourceTabPage );
    m_pTabControl->SetTabPage( TP_RANGECHOOSER, m_pRangeChooserTabPage );

    m_pTabControl->SelectTabPage( m_nLastPageId );
}

DataSourceDialog::~DataSourceDialog()
{
    disposeOnce();
}

void DataSourceDialog::dispose()
{
    m_pRangeChooserTabPage.disposeAndClear();
    m_pDataSourceTabPage.disposeAndClear();
    if (m_pTabControl)
        m_nLastPageId = m_pTabControl->GetCurPageId();
    m_pTabControl.disposeAndClear();
    m_pBtnOK.clear();
    TabDialog::dispose();
}

short DataSourceDialog::Execute()
{
    short nResult = TabDialog::Execute();
    if( nResult == RET_OK )
    {
        if( m_pRangeChooserTabPage )
            m_pRangeChooserTabPage->commitPage();
        if( m_pDataSourceTabPage )
            m_pDataSourceTabPage->commitPage();
    }
    return nResult;
}

void DataSourceDialog::setInvalidPage( TabPage * pTabPage )
{
    if( pTabPage == m_pRangeChooserTabPage )
        m_bRangeChooserTabIsValid = false;
    else if( pTabPage == m_pDataSourceTabPage )
        m_bDataSourceTabIsValid = false;

    if( ! (m_bRangeChooserTabIsValid && m_bDataSourceTabIsValid ))
    {
        m_pBtnOK->Enable( false );
        OSL_ASSERT( m_pTabControl );
        // note: there seems to be no suitable mechanism to address pages by
        // identifier, at least it is unclear what the page identifiers are.
        // @todo: change the fixed numbers to identifiers
        if( m_bRangeChooserTabIsValid )
            m_pTabControl->SetCurPageId( m_pTabControl->GetPageId( 1 ));
        else if( m_bDataSourceTabIsValid )
            m_pTabControl->SetCurPageId( m_pTabControl->GetPageId( 0 ));
        m_pTabControl->DisableTabToggling();
    }
}

void DataSourceDialog::setValidPage( TabPage * pTabPage )
{
    if( pTabPage == m_pRangeChooserTabPage )
        m_bRangeChooserTabIsValid = true;
    else if( pTabPage == m_pDataSourceTabPage )
        m_bDataSourceTabIsValid = true;

    if( m_bRangeChooserTabIsValid && m_bDataSourceTabIsValid )
    {
        m_pBtnOK->Enable();
        OSL_ASSERT( m_pTabControl );
        m_pTabControl->EnableTabToggling();
    }
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
