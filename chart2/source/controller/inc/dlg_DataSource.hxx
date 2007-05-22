/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_DataSource.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:58:32 $
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
#ifndef CHART2_DATASOURCEDIALOG_HXX
#define CHART2_DATASOURCEDIALOG_HXX

// header for class TabDialog
#ifndef _SV_TABDLG_HXX
#include <vcl/tabdlg.hxx>
#endif
// header for class TabControl
#ifndef _SV_TABCTRL_HXX
#include <vcl/tabctrl.hxx>
#endif
// header for class OKButton
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

#include "TabPageNotifiable.hxx"

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif

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

    static USHORT         m_nLastPageId;
};

} //  namespace chart

// CHART2_DATASOURCEDIALOG_HXX
#endif
