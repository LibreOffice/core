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

#include <editeng/borderline.hxx>

#include <tabvwsh.hxx>
#include <document.hxx>

void ScTabViewShell::SetDefaultFrameLine( const ::editeng::SvxBorderLine* pLine )
{
    if ( pLine )
    {
        pCurFrameLine.reset( new ::editeng::SvxBorderLine( &pLine->GetColor(),
                                            pLine->GetWidth(),
                                            pLine->GetBorderLineStyle() ) );
    }
    else
        pCurFrameLine.reset();
}

bool ScTabViewShell::HasSelection( bool bText ) const
{
    bool bHas = false;
    ScViewData* pData = const_cast<ScViewData*>(&GetViewData());
    if ( bText )
    {
        // Content contained: Count2 >= 1
        ScDocument& rDoc = pData->GetDocument();
        ScMarkData& rMark = pData->GetMarkData();
        ScAddress aCursor( pData->GetCurX(), pData->GetCurY(), pData->GetTabNo() );
        double fVal = 0.0;
        if ( rDoc.GetSelectionFunction( SUBTOTAL_FUNC_CNT2, aCursor, rMark, fVal ) )
            bHas = ( fVal > 0.5 );
    }
    else
    {
        ScRange aRange;
        ScMarkType eMarkType = pData->GetSimpleArea( aRange );
        if ( eMarkType == SC_MARK_SIMPLE )
            bHas = ( aRange.aStart != aRange.aEnd );    // more than 1 cell
        else
            bHas = true;                                // multiple selection or filtered
    }
    return bHas;
}

void ScTabViewShell::UIDeactivated( SfxInPlaceClient* pClient )
{
    ClearHighlightRanges();

    // Move in the ViewShell should really be called from Sfx, when the
    // frame window is moved due to different toolboxes or other things
    // (to not  move painted objects by mistake, #56515#).
    // this mechanism does however not work at the moment, that is why this
    // call is here (in Move it is checked if the position has really changed).
    ForceMove();
    SfxViewShell::UIDeactivated( pClient );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
