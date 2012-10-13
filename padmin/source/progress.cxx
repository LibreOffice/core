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

#include <ctype.h>
#include <stdio.h>
#include <tools/string.hxx>
#include <tools/stream.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <progress.hxx>
#include <helper.hxx>
#include <padialog.hrc>

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
        mbCanceled( false )
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
    mbCanceled = false;
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
        mbCanceled = true;
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
