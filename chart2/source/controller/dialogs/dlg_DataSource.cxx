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

#include <dlg_DataSource.hxx>
#include <ChartTypeTemplateProvider.hxx>
#include <ChartTypeTemplate.hxx>
#include <Diagram.hxx>
#include "DialogModel.hxx"
#include <ChartModel.hxx>

#include "tp_RangeChooser.hxx"
#include "tp_DataSource.hxx"

using namespace ::com::sun::star;
using namespace ::chart;

namespace chart
{

namespace {

class DocumentChartTypeTemplateProvider : public ChartTypeTemplateProvider
{
public:
    explicit DocumentChartTypeTemplateProvider(
        const rtl::Reference<::chart::ChartModel> & xDoc );

    // ____ ChartTypeTemplateProvider ____
    virtual rtl::Reference< ::chart::ChartTypeTemplate > getCurrentTemplate() const override;

private:
    rtl::Reference< ::chart::ChartTypeTemplate > m_xTemplate;
};

}

DocumentChartTypeTemplateProvider::DocumentChartTypeTemplateProvider(
    const rtl::Reference<::chart::ChartModel> & xDoc )
{
    if( !xDoc.is())
        return;

    rtl::Reference< Diagram > xDia( xDoc->getFirstChartDiagram());
    if( xDia.is())
    {
        Diagram::tTemplateWithServiceName aResult(
            xDia->getTemplate( xDoc->getTypeManager() ));
        m_xTemplate = aResult.xChartTypeTemplate;
    }
}

rtl::Reference< ::chart::ChartTypeTemplate > DocumentChartTypeTemplateProvider::getCurrentTemplate() const
{
    return m_xTemplate;
}

sal_uInt16 DataSourceDialog::m_nLastPageId = 0;

DataSourceDialog::DataSourceDialog(weld::Window * pParent,
    const rtl::Reference<::chart::ChartModel> & xChartDocument)
    : GenericDialogController(pParent, u"modules/schart/ui/datarangedialog.ui"_ustr,
                              u"DataRangeDialog"_ustr)
    , m_apDocTemplateProvider(new DocumentChartTypeTemplateProvider(xChartDocument))
    , m_apDialogModel(new DialogModel(xChartDocument))
    , m_bRangeChooserTabIsValid(true)
    , m_bDataSourceTabIsValid(true)
    , m_bTogglingEnabled(true)
    , m_xTabControl(m_xBuilder->weld_notebook(u"tabcontrol"_ustr))
    , m_xBtnOK(m_xBuilder->weld_button(u"ok"_ustr))
{
    m_xRangeChooserTabPage = std::make_unique<RangeChooserTabPage>(m_xTabControl->get_page(u"range"_ustr), this,
                                     *m_apDialogModel,
                                     m_apDocTemplateProvider.get(), true /* bHideDescription */ );
    m_xDataSourceTabPage = std::make_unique<DataSourceTabPage>(m_xTabControl->get_page(u"series"_ustr), this,
                                    *m_apDialogModel,
                                    m_apDocTemplateProvider.get(), true /* bHideDescription */ );
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
    m_xRangeChooserTabPage.reset();
    m_xDataSourceTabPage.reset();
    m_nLastPageId = m_xTabControl->get_current_page();
}

short DataSourceDialog::run()
{
    short nResult = GenericDialogController::run();
    if( nResult == RET_OK )
    {
        if( m_xRangeChooserTabPage )
            m_xRangeChooserTabPage->commitPage();
        if( m_xDataSourceTabPage )
            m_xDataSourceTabPage->commitPage();
    }
    return nResult;
}

IMPL_LINK(DataSourceDialog, ActivatePageHdl, const OUString&, rPage, void)
{
    if (rPage == "range")
        m_xRangeChooserTabPage->Activate();
    else if (rPage == "series")
        m_xDataSourceTabPage->Activate();
}

// allow/disallow user to leave page
IMPL_LINK_NOARG(DataSourceDialog, DeactivatePageHdl, const OUString&, bool)
{
    return m_bTogglingEnabled;
}

void DataSourceDialog::setInvalidPage(BuilderPage* pTabPage)
{
    if (pTabPage == m_xRangeChooserTabPage.get())
        m_bRangeChooserTabIsValid = false;
    else if (pTabPage == m_xDataSourceTabPage.get())
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
        m_bTogglingEnabled = false;
    }
}

void DataSourceDialog::setValidPage(BuilderPage* pTabPage)
{
    if( pTabPage == m_xRangeChooserTabPage.get() )
        m_bRangeChooserTabIsValid = true;
    else if( pTabPage == m_xDataSourceTabPage.get() )
        m_bDataSourceTabIsValid = true;

    if (m_bRangeChooserTabIsValid && m_bDataSourceTabIsValid)
    {
        m_xBtnOK->set_sensitive(true);
        m_bTogglingEnabled = true;
    }
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
