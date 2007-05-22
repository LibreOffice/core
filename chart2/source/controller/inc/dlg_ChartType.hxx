/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_ChartType.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:57:53 $
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
#ifndef _CHART2_DLG_CHARTTYPE_HXX
#define _CHART2_DLG_CHARTTYPE_HXX

// header for class ModalDialog
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
// header for class FixedLine
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
// header for class OKButton
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

class ChartTypeTabPage;
class ChartTypeDialog : public ModalDialog
{
public:
    ChartTypeDialog( Window* pWindow
        , const ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XModel >& xChartModel
        , const ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >& xContext );
    ~ChartTypeDialog();

private:
    FixedLine           m_aFL;
    OKButton            m_aBtnOK;
    CancelButton        m_aBtnCancel;
    HelpButton          m_aBtnHelp;

    ChartTypeTabPage*   m_pChartTypeTabPage;

    ::com::sun::star::uno::Reference<
                       ::com::sun::star::frame::XModel >            m_xChartModel;
    ::com::sun::star::uno::Reference<
                       ::com::sun::star::uno::XComponentContext>    m_xCC;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif

