/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: javachild.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _SV_JAVACHILD_HXX
#define _SV_JAVACHILD_HXX

#include <vcl/dllapi.h>
#include <vcl/syschild.hxx>

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
