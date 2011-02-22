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
#include "precompiled_sw.hxx"

#include <hintids.hxx>
#include <svx/svxids.hrc>
#include <svl/stritem.hxx>
#include <svx/drawitem.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdoutl.hxx>
#include <svx/xtable.hxx>

#include <docsh.hxx>
#include <doc.hxx>

using namespace ::com::sun::star;

/*--------------------------------------------------------------------
    Description: Load Document
 --------------------------------------------------------------------*/


void  SwDocShell::InitDraw()
{
    SdrModel *pDrDoc = pDoc->GetDrawModel();
    if( pDrDoc )
    {
        // create Lists and Tables DocShell's ItemSet
        PutItem( SvxGradientListItem( pDrDoc->GetGradientList(), SID_GRADIENT_LIST ) );
        PutItem( SvxHatchListItem( pDrDoc->GetHatchList(), SID_HATCH_LIST ) );
        PutItem( SvxBitmapListItem( pDrDoc->GetBitmapList(), SID_BITMAP_LIST ) );
        PutItem( SvxDashListItem( pDrDoc->GetDashList(), SID_DASH_LIST ) );
        PutItem( SvxLineEndListItem( pDrDoc->GetLineEndList(), SID_LINEEND_LIST ) );

        Outliner& rOutliner = pDrDoc->GetDrawOutliner();
        uno::Reference<linguistic2::XHyphenator> xHyphenator( ::GetHyphenator() );
        rOutliner.SetHyphenator( xHyphenator );
    }
    else
        PutItem( SvxColorTableItem( XColorTable::GetStdColorTable(), SID_COLOR_TABLE ));
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
