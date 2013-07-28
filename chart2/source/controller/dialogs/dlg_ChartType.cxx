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

#include "dlg_ChartType.hxx"
#include "dlg_ChartType.hrc"
#include "ResId.hxx"
#include "ResourceIds.hrc"
#include "Strings.hrc"
#include "tp_ChartType.hxx"
#include "macros.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>

using namespace ::com::sun::star;

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

ChartTypeDialog::ChartTypeDialog( Window* pParent
                , const uno::Reference< frame::XModel >& xChartModel
                , const uno::Reference< uno::XComponentContext >& xContext )
                : ModalDialog( pParent, SchResId( DLG_DIAGRAM_TYPE ))
                , m_aFL( this, SchResId( FL_BUTTONS ) )
                , m_aBtnOK( this, SchResId( BTN_OK ) )
                , m_aBtnCancel( this, SchResId( BTN_CANCEL ) )
                , m_aBtnHelp( this, SchResId( BTN_HELP ) )
                , m_pChartTypeTabPage(0)
                , m_xChartModel(xChartModel)
                , m_xCC( xContext )
{
    FreeResource();

    this->SetText(SCH_RESSTR(STR_PAGE_CHARTTYPE));

    //don't create the tabpages before FreeResource, otherwise the help ids are not matched correctly
    m_pChartTypeTabPage = new ChartTypeTabPage(this,uno::Reference< XChartDocument >::query(m_xChartModel),m_xCC,true/*live update*/,true/*hide title description*/);
    m_pChartTypeTabPage->initializePage();
    m_pChartTypeTabPage->Show();
 }

ChartTypeDialog::~ChartTypeDialog()
{
    delete m_pChartTypeTabPage;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
