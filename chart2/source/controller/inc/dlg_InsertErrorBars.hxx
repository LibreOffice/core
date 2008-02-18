/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_InsertErrorBars.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:54:31 $
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
#ifndef _CHART2_DLG_INSERT_ERRORBARS_HXX
#define _CHART2_DLG_INSERT_ERRORBARS_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <svtools/itemset.hxx>
#include <memory>
#include <com/sun/star/frame/XModel.hpp>

#include "res_ErrorBar.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

class InsertErrorBarsDialog : public ModalDialog
{
public:
    InsertErrorBarsDialog( Window* pParent, const SfxItemSet& rMyAttrs,
                           ErrorBarResources::tErrorBarType eType = ErrorBarResources::ERROR_BAR_Y );
    virtual ~InsertErrorBarsDialog();

    void SetAxisMinorStepWidthForErrorBarDecimals( double fMinorStepWidth );

    static double getAxisMinorStepWidthForErrorBarDecimals(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel >& xChartModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XInterface >& xChartView,
        const ::rtl::OUString& rSelectedObjectCID );

    void FillItemSet( SfxItemSet& rOutAttrs );
    virtual void DataChanged( const DataChangedEvent& rDCEvt );

private:
    void Reset();
    const SfxItemSet & rInAttrs;

    OKButton          aBtnOK;
    CancelButton      aBtnCancel;
    HelpButton        aBtnHelp;

    ::std::auto_ptr< ErrorBarResources >    m_apErrorBarResources;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
