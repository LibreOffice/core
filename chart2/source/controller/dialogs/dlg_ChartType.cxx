/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_ChartType.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:31:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif

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
