/*************************************************************************
 *
 *  $RCSfile: recfloat.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mba $ $Date: 2002-07-18 07:13:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// includes *******************************************************************

#pragma hdrstop

#include <svtools/eitem.hxx>

#include "recfloat.hxx"
#include "dialog.hrc"
#include "sfxresid.hxx"
#include "app.hxx"
#include "bindings.hxx"
#include "dispatch.hxx"

SFX_IMPL_FLOATINGWINDOW( SfxRecordingFloatWrapper_Impl, SID_RECORDING_FLOATWINDOW );

SfxRecordingFloatWrapper_Impl::SfxRecordingFloatWrapper_Impl( Window* pParent ,
                                                USHORT nId ,
                                                SfxBindings* pBindings ,
                                                SfxChildWinInfo* pInfo )
                    : SfxChildWindow( pParent , nId )
{
    pWindow = new SfxRecordingFloat_Impl( pBindings, this, pParent );
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
    ( ( SfxFloatingWindow* ) pWindow )->Initialize( pInfo );
}

SfxRecordingFloat_Impl::SfxRecordingFloat_Impl( SfxBindings* pBindings ,
                                        SfxChildWindow* pChildWin ,
                                        Window* pParent )
                        : SfxFloatingWindow( pBindings ,
                                        pChildWin ,
                                        pParent ,
                                        SfxResId( SID_RECORDING_FLOATWINDOW ) )
                        , pWrapper( pChildWin )
                        , aTbx( this, GetBindings(), SfxResId(SID_RECORDING_FLOATWINDOW) )
{
    aTbx.Initialize();
    Size aSize = aTbx.GetToolBox().CalcWindowSizePixel();
    aTbx.GetToolBox().SetPosSizePixel( Point(), aSize );
    SetOutputSizePixel( aSize );
    SfxBoolItem aItem( SID_RECORDMACRO, TRUE );
    GetBindings().GetDispatcher()->Execute( SID_RECORDMACRO, SFX_CALLMODE_SYNCHRON, &aItem, 0L );
}

BOOL SfxRecordingFloat_Impl::Close()
{
    SfxBindings& rBindings = GetBindings();
    BOOL bRet = SfxFloatingWindow::Close();
    SfxBoolItem aItem( FN_PARAM_1, TRUE );
    rBindings.GetDispatcher()->Execute( SID_STOP_RECORDING, SFX_CALLMODE_SYNCHRON, &aItem, 0L );
    return bRet;
}


