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

#include "ChartController.hxx"
#include "macros.hxx"

#include "dlg_DataEditor.hxx"
#include "DataSourceHelper.hxx"
#include "DiagramHelper.hxx"
#include "ControllerLockGuard.hxx"
#include "UndoGuard.hxx"
#include "ResId.hxx"
#include "strings.hrc"

#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;

namespace chart
{

void ChartController::executeDispatch_EditData()
{
    Reference< chart2::XChartDocument > xChartDoc( getModel(), uno::UNO_QUERY );
    if( xChartDoc.is())
    {
        SolarMutexGuard aSolarGuard;
        UndoLiveUpdateGuardWithData aUndoGuard(
            SchResId( STR_ACTION_EDIT_CHART_DATA ),
            m_xUndoManager );
        ScopedVclPtrInstance<DataEditor> aDataEditorDialog( nullptr, xChartDoc, m_xCC );
        if (aDataEditorDialog->Execute() == RET_OK)
            aDataEditorDialog->ApplyChangesToModel();
        aUndoGuard.commit();
    }
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
