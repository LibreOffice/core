/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: introwin.cxx,v $
 * $Revision: 1.11 $
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
#include "precompiled_vcl.hxx"

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#include <vcl/salframe.hxx>
#include <tools/debug.hxx>
#include <vcl/svdata.hxx>
#include <vcl/wrkwin.hxx>

#include <vcl/bitmap.hxx>
#include <vcl/impbmp.hxx>
#include <vcl/introwin.hxx>




// =======================================================================

void IntroWindow::ImplInitIntroWindowData()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mpIntroWindow = this;
}

// -----------------------------------------------------------------------

IntroWindow::IntroWindow( ) :
    WorkWindow( WINDOW_INTROWINDOW )
{
    ImplInitIntroWindowData();
    WorkWindow::ImplInit( 0, WB_INTROWIN, NULL );
}

// -----------------------------------------------------------------------

IntroWindow::~IntroWindow()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->mpIntroWindow == this )
        pSVData->mpIntroWindow = NULL;
}

void IntroWindow::SetBackgroundBitmap( const Bitmap& rBitmap )
{
    if( ! rBitmap.IsEmpty() )
    {
        SalBitmap* pBmp = rBitmap.ImplGetImpBitmap()->ImplGetSalBitmap();
        ImplGetFrame()->SetBackgroundBitmap( pBmp );
    }
}
