/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxwindow1.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:02:33 $
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

#include <tools/svwin.h>

#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef _SV_WORKWIN
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _SV_SYSDATA_HXX
#include <vcl/sysdata.hxx>
#endif

/// helper method to set a window handle into a SystemParentData struct
void VCLXWindow::SetSystemParent_Impl( sal_Int64 nHandle )
{
    // does only work for WorkWindows
    Window *pWindow = GetWindow();
    if ( pWindow->GetType() != WINDOW_WORKWINDOW )
    {
        ::com::sun::star::uno::Exception *pException =
            new ::com::sun::star::uno::RuntimeException;
        pException->Message = ::rtl::OUString::createFromAscii( "not a work window" );
        throw pException;
    }

    // create system parent data
    SystemParentData aSysParentData;
    aSysParentData.nSize = sizeof ( SystemParentData );
#ifndef SYSDATA_ONLY_BASETYPE
#if defined( WIN ) || defined( WNT )
    aSysParentData.hWnd = (HWND) nHandle;
#elif defined( OS2 )
    aSysParentData.hWnd = (HWND) nHandle;
#elif defined( UNX )
    aSysParentData.aWindow = (long)nHandle;
#endif
#endif

    // set system parent
    ((WorkWindow*)pWindow)->SetPluginParent( &aSysParentData );
}

