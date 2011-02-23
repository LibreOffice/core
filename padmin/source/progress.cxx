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

#include <ctype.h>
#include <stdio.h>
#include <tools/string.hxx>
#include <tools/stream.hxx>
#include <tools/list.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <progress.hxx>
#include <helper.hxx>
#ifndef _PAD_PADIALOG_HRC_
#include <padialog.hrc>
#endif

using namespace padmin;

ProgressDialog::ProgressDialog( Window* pParent,
                                sal_Bool bCancelable,
                                int nMin, int nMax ) :
        ModelessDialog( pParent, PaResId( RID_PROGRESS_DLG ) ),
        maOperation( this, PaResId( RID_PROGRESS_OPERATION_TXT ) ),
        maFilename( this, PaResId( RID_PROGRESS_FILENAME_TXT ) ),
        maProgressTxt( this, PaResId( RID_PROGRESS_PROGRESS_TXT ) ),
        maCancelButton( this, PaResId( RID_PROGRESS_BTN_CANCEL ) ),
        maProgressBar( this, PaResId( RID_PROGRESS_STATUSBAR ) ),
        mnMax( nMax ),
        mnMin( nMin ),
        mbCanceled( sal_False )
{
    maFilename.SetStyle( maFilename.GetStyle() | WB_PATHELLIPSIS );
    if( ! bCancelable )
    {
        Point aPos = maProgressBar.GetPosPixel();
        Size aSize = maProgressBar.GetSizePixel();
        Size aMySize = GetOutputSizePixel();
        aMySize.Height() = aPos.Y() + aSize.Height() + 5;
        SetOutputSizePixel( aMySize );
    }
    else
        maCancelButton.SetClickHdl( LINK( this, ProgressDialog, ClickBtnHdl ) );
    FreeResource();
}

ProgressDialog::~ProgressDialog()
{
}

void ProgressDialog::startOperation( const String& rOperation )
{
    maOperation.SetText( rOperation );
    maProgressBar.SetValue( 0 );
    mbCanceled = sal_False;
    if( ! IsVisible() )
        Show( sal_True );
}

void ProgressDialog::setValue( int nValue )
{
    maProgressBar.SetValue( nValue * 100 / ( mnMax - mnMin ) );
    Application::Reschedule();
}

void ProgressDialog::setFilename( const String& rFilename )
{
    maFilename.SetText( rFilename );
    maFilename.Update();
    Flush();
}

IMPL_LINK( ProgressDialog, ClickBtnHdl, Button*, pButton )
{
    if( pButton == &maCancelButton )
    {
        mbCanceled = sal_True;
    }
    return 0;
}
