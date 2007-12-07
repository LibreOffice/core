/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: javachild.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:49:58 $
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

#ifndef _SV_JAVACHILD_HXX
#define _SV_JAVACHILD_HXX

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_SYSCHILD_HXX
#include <vcl/syschild.hxx>
#endif

// -------------------
// - JavaChildWindow -
// -------------------

class VCL_DLLPUBLIC JavaChildWindow : public SystemChildWindow
{
public:

                    JavaChildWindow( Window* pParent, WinBits nStyle = 0 );
                    JavaChildWindow( Window* pParent, const ResId& rResId );
                   ~JavaChildWindow();

    sal_IntPtr      getParentWindowHandleForJava();

private:

    SAL_DLLPRIVATE void implTestJavaException( void* pEnv );

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE      JavaChildWindow (const JavaChildWindow &);
    SAL_DLLPRIVATE      JavaChildWindow & operator= (const JavaChildWindow &);
};

#endif // _SV_JAVACHILD_HXX
