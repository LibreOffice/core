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
#ifndef _CHART2_DLG_INSERT_ERRORBARS_HXX
#define _CHART2_DLG_INSERT_ERRORBARS_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <svl/itemset.hxx>
#include <memory>
#include <com/sun/star/frame/XModel.hpp>

#include "res_ErrorBar.hxx"

namespace chart
{

class InsertErrorBarsDialog : public ModalDialog
{
public:
    InsertErrorBarsDialog( Window* pParent, const SfxItemSet& rMyAttrs,
                           const ::com::sun::star::uno::Reference<
                               ::com::sun::star::chart2::XChartDocument > & xChartDocument,
                           ErrorBarResources::tErrorBarType eType );
    virtual ~InsertErrorBarsDialog();

    void SetAxisMinorStepWidthForErrorBarDecimals( double fMinorStepWidth );

    static double getAxisMinorStepWidthForErrorBarDecimals(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel >& xChartModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XInterface >& xChartView,
        const OUString& rSelectedObjectCID );

    void FillItemSet( SfxItemSet& rOutAttrs );
    virtual void DataChanged( const DataChangedEvent& rDCEvt );

private:
    const SfxItemSet & rInAttrs;

    OKButton          aBtnOK;
    CancelButton      aBtnCancel;
    HelpButton        aBtnHelp;

    ::std::auto_ptr< ErrorBarResources >    m_apErrorBarResources;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
