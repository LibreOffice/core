/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: progress.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:28:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <ctype.h>
#include <stdio.h>

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _PAD_PROGRESS_HXX_
#include <progress.hxx>
#endif
#ifndef _PAD_HELPER_HXX_
#include <helper.hxx>
#endif
#ifndef _PAD_PADIALOG_HRC_
#include <padialog.hrc>
#endif

using namespace padmin;

ProgressDialog::ProgressDialog( Window* pParent,
                                BOOL bCancelable,
                                int nMin, int nMax ) :
        ModelessDialog( pParent, PaResId( RID_PROGRESS_DLG ) ),
        maOperation( this, PaResId( RID_PROGRESS_OPERATION_TXT ) ),
        maFilename( this, PaResId( RID_PROGRESS_FILENAME_TXT ) ),
        maProgressTxt( this, PaResId( RID_PROGRESS_PROGRESS_TXT ) ),
        maCancelButton( this, PaResId( RID_PROGRESS_BTN_CANCEL ) ),
        maProgressBar( this, PaResId( RID_PROGRESS_STATUSBAR ) ),
        mnMax( nMax ),
        mnMin( nMin ),
        mbCanceled( FALSE )
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
    mbCanceled = FALSE;
    if( ! IsVisible() )
        Show( TRUE );
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
        mbCanceled = TRUE;
    }
    return 0;
}
