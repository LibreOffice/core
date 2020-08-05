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
#pragma once

#include <vcl/weld.hxx>

#include "TabPageNotifiable.hxx"

namespace com::sun::star::chart2 { class XChartDocument; }
namespace com::sun::star::uno { class XComponentContext; }

#include <memory>

class BuilderPage;

namespace chart
{

class RangeChooserTabPage;
class DataSourceTabPage;
class ChartTypeTemplateProvider;
class DialogModel;

class DataSourceDialog final :
        public weld::GenericDialogController,
        public TabPageNotifiable
{
public:
    explicit DataSourceDialog(
        weld::Window * pParent,
        const css::uno::Reference< css::chart2::XChartDocument > & xChartDocument,
        const css::uno::Reference< css::uno::XComponentContext > & xContext );
    virtual ~DataSourceDialog() override;

    // from GenericDialogController base
    virtual short run() override;

    // TabPageNotifiable
    virtual void setInvalidPage( BuilderPage * pTabPage ) override;
    virtual void setValidPage( BuilderPage * pTabPage ) override;

private:
    DECL_LINK(ActivatePageHdl, const OString&, void);
    DECL_LINK(DeactivatePageHdl, const OString&, bool);

    std::unique_ptr< ChartTypeTemplateProvider >  m_apDocTemplateProvider;
    std::unique_ptr< DialogModel >                m_apDialogModel;

    std::unique_ptr<RangeChooserTabPage> m_xRangeChooserTabPage;
    std::unique_ptr<DataSourceTabPage> m_xDataSourceTabPage;
    bool                  m_bRangeChooserTabIsValid;
    bool                  m_bDataSourceTabIsValid;
    bool                  m_bTogglingEnabled;

    std::unique_ptr<weld::Notebook> m_xTabControl;
    std::unique_ptr<weld::Button> m_xBtnOK;

    static sal_uInt16         m_nLastPageId;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
