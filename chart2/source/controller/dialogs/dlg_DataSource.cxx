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

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <dlg_DataSource.hxx>
#include "ChartTypeTemplateProvider.hxx"
#include <DiagramHelper.hxx>
#include "DialogModel.hxx"

#include "tp_RangeChooser.hxx"
#include "tp_DataSource.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::chart;

using ::com::sun::star::uno::Reference;

namespace chart
{

class DocumentChartTypeTemplateProvider : public ChartTypeTemplateProvider
{
public:
    explicit DocumentChartTypeTemplateProvider(
        const Reference< chart2::XChartDocument > & xDoc );

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

Reference< chart2::XChartTypeTemplate > DocumentChartTypeTemplateProvider::getCurrentTemplate() const
{
    return m_xTemplate;
}

sal_uInt16 DataSourceDialog::m_nLastPageId = 0;

DataSourceDialog::DataSourceDialog(weld::Window * pParent,
    const Reference< XChartDocument > & xChartDocument,
    const Reference< uno::XComponentContext > & xContext)
    : GenericDialogController(pParent, "modules/schart/ui/datarangedialog.ui",
                              "DataRangeDialog")
    , m_apDocTemplateProvider(new DocumentChartTypeTemplateProvider(xChartDocument))
    , m_apDialogModel(new DialogModel(xChartDocument, xContext))
    , m_pRangeChooserTabPage(nullptr)
    , m_pDataSourceTabPage(nullptr)
    , m_bRangeChooserTabIsValid(true)
    , m_bDataSourceTabIsValid(true)
    , m_bTogglingEnabled(true)
    , m_xTabControl(m_xBuilder->weld_notebook("notebook"))
    , m_xBtnOK(m_xBuilder->weld_button("ok"))
{
    TabPageParent aRangeParent(m_xTabControl->get_page("range"), this);
    m_pRangeChooserTabPage = VclPtr<RangeChooserTabPage>::Create(aRangeParent, *(m_apDialogModel.get()),
                                     m_apDocTemplateProvider.get(), nullptr, true /* bHideDescription */ );
    TabPageParent aSeriesParent(m_xTabControl->get_page("series"), this);
    m_pDataSourceTabPage = VclPtr<DataSourceTabPage>::Create(aSeriesParent, *(m_apDialogModel.get()),
                                    m_apDocTemplateProvider.get(), nullptr, true /* bHideDescription */ );
    m_xTabControl->connect_enter_page(LINK(this, DataSourceDialog, ActivatePageHdl));
    m_xTabControl->connect_leave_page(LINK(this, DataSourceDialog, DeactivatePageHdl));
    ActivatePageHdl(m_xTabControl->get_current_page_ident());
    if (m_nLastPageId != 0)
    {
        m_xTabControl->set_current_page(m_nLastPageId);
        ActivatePageHdl(m_xTabControl->get_current_page_ident());
    }
}

DataSourceDialog::~DataSourceDialog()
{
    m_pRangeChooserTabPage.disposeAndClear();
    m_pDataSourceTabPage.disposeAndClear();
    m_nLastPageId = m_xTabControl->get_current_page();
}

short DataSourceDialog::run()
{
    short nResult = GenericDialogController::run();
    if( nResult == RET_OK )
    {
        if( m_pRangeChooserTabPage )
            m_pRangeChooserTabPage->commitPage();
        if( m_pDataSourceTabPage )
            m_pDataSourceTabPage->commitPage();
    }
    return nResult;
}

IMPL_LINK(DataSourceDialog, ActivatePageHdl, const OString&, rPage, void)
{
    if (rPage == "range")
        m_pRangeChooserTabPage->ActivatePage();
    else if (rPage == "series")
        m_pDataSourceTabPage->ActivatePage();
}

// allow/disallow user to leave page
IMPL_LINK_NOARG(DataSourceDialog, DeactivatePageHdl, const OString&, bool)
{
    return m_bTogglingEnabled;
}

void DataSourceDialog::DisableTabToggling()
{
    m_bTogglingEnabled = false;
}

void DataSourceDialog::EnableTabToggling()
{
    m_bTogglingEnabled = true;
}

void DataSourceDialog::setInvalidPage( TabPage * pTabPage )
{
    if (pTabPage == m_pRangeChooserTabPage)
        m_bRangeChooserTabIsValid = false;
    else if (pTabPage == m_pDataSourceTabPage)
        m_bDataSourceTabIsValid = false;

    if (!(m_bRangeChooserTabIsValid && m_bDataSourceTabIsValid))
    {
        m_xBtnOK->set_sensitive(false);
        // note: there seems to be no suitable mechanism to address pages by
        // identifier, at least it is unclear what the page identifiers are.
        // @todo: change the fixed numbers to identifiers
        if( m_bRangeChooserTabIsValid )
            m_xTabControl->set_current_page(1);
        else if( m_bDataSourceTabIsValid )
            m_xTabControl->set_current_page(0);
        DisableTabToggling();
    }
}

void DataSourceDialog::setValidPage( TabPage * pTabPage )
{
    if( pTabPage == m_pRangeChooserTabPage )
        m_bRangeChooserTabIsValid = true;
    else if( pTabPage == m_pDataSourceTabPage )
        m_bDataSourceTabIsValid = true;

    if (m_bRangeChooserTabIsValid && m_bDataSourceTabIsValid)
    {
        m_xBtnOK->set_sensitive(true);
        EnableTabToggling();
    }
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
