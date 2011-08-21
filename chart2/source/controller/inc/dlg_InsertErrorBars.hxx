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
#ifndef _CHART2_DLG_INSERT_ERRORBARS_HXX
#define _CHART2_DLG_INSERT_ERRORBARS_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <svl/itemset.hxx>
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
                           const ::com::sun::star::uno::Reference<
                               ::com::sun::star::chart2::XChartDocument > & xChartDocument,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
