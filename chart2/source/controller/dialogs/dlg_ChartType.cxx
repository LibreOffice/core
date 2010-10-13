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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "dlg_ChartType.hxx"
#include "dlg_ChartType.hrc"
#include "ResId.hxx"
#include "ResourceIds.hrc"
#include "Strings.hrc"
#include "tp_ChartType.hxx"
#include "macros.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>

using namespace ::com::sun::star;

//.............................................................................
namespace chart
{
//.............................................................................
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

    this->SetText(String(SchResId(STR_PAGE_CHARTTYPE)));

    //don't create the tabpages before FreeResource, otherwise the help ids are not matched correctly
    m_pChartTypeTabPage = new ChartTypeTabPage(this,uno::Reference< XChartDocument >::query(m_xChartModel),m_xCC,true/*live update*/,true/*hide title description*/);
    m_pChartTypeTabPage->initializePage();
    m_pChartTypeTabPage->Show();
 }

ChartTypeDialog::~ChartTypeDialog()
{
    delete m_pChartTypeTabPage;
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
