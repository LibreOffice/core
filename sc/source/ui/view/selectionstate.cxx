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

#include "selectionstate.hxx"

#include <editeng/editview.hxx>
#include "viewdata.hxx"
#include "markdata.hxx"

ScSelectionState::ScSelectionState( ScViewData& rViewData ) :
    meType( SC_SELECTTYPE_NONE )
{
    maCursor.SetTab( rViewData.GetTabNo() );
    ScSplitPos eWhich = rViewData.GetActivePart();

    if( rViewData.HasEditView( eWhich ) )
    {
        meType = SC_SELECTTYPE_EDITCELL;
        maCursor.SetCol( rViewData.GetEditViewCol() );
        maCursor.SetRow( rViewData.GetEditViewRow() );
        maEditSel = rViewData.GetEditView( eWhich )->GetSelection();
    }
    else
    {
        maCursor.SetCol( rViewData.GetCurX() );
        maCursor.SetRow( rViewData.GetCurY() );

        ScMarkData& rMarkData = rViewData.GetMarkData();
        rMarkData.MarkToMulti();
        if( rMarkData.IsMultiMarked() )
        {
            meType = SC_SELECTTYPE_SHEET;
            rMarkData.FillRangeListWithMarks( &maSheetSel, false );
        }
        // else type is SC_SELECTTYPE_NONE - already initialized
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
