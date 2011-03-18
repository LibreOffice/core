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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------


#include "dbfunc.hxx"
#include "docsh.hxx"
#include "global.hxx"
#include "document.hxx"
#include "sc.hrc"
#include "globstr.hrc"


// STATIC DATA -----------------------------------------------------------


#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif

//==================================================================

class ScDrawLayer;
class ScChartCollection;

void ScDBFunc::UpdateCharts( sal_Bool bAllCharts )
{
    sal_uInt16 nFound = 0;
    ScViewData* pViewData = GetViewData();
    ScDocument* pDoc = pViewData->GetDocument();

    if ( pDoc->GetDrawLayer() )
        nFound = DoUpdateCharts( ScAddress( pViewData->GetCurX(),
                                            pViewData->GetCurY(),
                                            pViewData->GetTabNo()),
                                 pDoc,
                                 bAllCharts );

    if ( !nFound && !bAllCharts )
        ErrorMessage(STR_NOCHARTATCURSOR);
}






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
