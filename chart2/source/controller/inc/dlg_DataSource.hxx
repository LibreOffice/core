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
#ifndef CHART2_DATASOURCEDIALOG_HXX
#define CHART2_DATASOURCEDIALOG_HXX

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
    OKButton              m_aBtnOK;
    CancelButton          m_aBtnCancel;
    HelpButton            m_aBtnHelp;

    RangeChooserTabPage * m_pRangeChooserTabePage;
    DataSourceTabPage *   m_pDataSourceTabPage;
    bool                  m_bRangeChooserTabIsValid;
    bool                  m_bDataSourceTabIsValid;

    static sal_uInt16         m_nLastPageId;
};

} //  namespace chart

// CHART2_DATASOURCEDIALOG_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
