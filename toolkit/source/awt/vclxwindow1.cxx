/*************************************************************************
 *
 *  $RCSfile: vclxwindow1.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-20 10:19:17 $
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
    aSysParentData.aWindow = nHandle;
#endif
#endif

    // set system parent
    ((WorkWindow*)pWindow)->SetPluginParent( &aSysParentData );
}

