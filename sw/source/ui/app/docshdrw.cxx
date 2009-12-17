/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: docshdrw.cxx,v $
 * $Revision: 1.13 $
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

#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif








#include <svl/stritem.hxx>
#include <svx/drawitem.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdoutl.hxx>
#include <svx/xtable.hxx>

#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#include <doc.hxx>

using namespace ::com::sun::star;

/*--------------------------------------------------------------------
    Beschreibung: Document laden
 --------------------------------------------------------------------*/


void  SwDocShell::InitDraw()
{
    SdrModel *pDrDoc = pDoc->GetDrawModel();
    if( pDrDoc )
    {
        // Listen, bzw. Tables im ItemSet der DocShell anlegen
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



