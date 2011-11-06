/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
