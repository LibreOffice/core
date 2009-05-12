/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: syschild.hxx,v $
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

#ifndef _SV_SYSCHILD_HXX
#define _SV_SYSCHILD_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/window.hxx>

struct SystemEnvData;
struct SystemWindowData;

// ---------------------
// - SystemChildWindow -
// ---------------------

class VCL_DLLPUBLIC SystemChildWindow : public Window
{
private:
    using Window::ImplInit;
    SAL_DLLPRIVATE void     ImplInitSysChild( Window* pParent, WinBits nStyle, SystemWindowData *pData, BOOL bShow = TRUE );

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE          SystemChildWindow (const SystemChildWindow &);
    SAL_DLLPRIVATE          SystemChildWindow & operator= (const SystemChildWindow &);

public:
                            SystemChildWindow( Window* pParent, WinBits nStyle = 0 );
                            // create a SystemChildWindow using the given SystemWindowData
                            SystemChildWindow( Window* pParent, WinBits nStyle, SystemWindowData *pData, BOOL bShow = TRUE );
                            SystemChildWindow( Window* pParent, const ResId& rResId );
                            ~SystemChildWindow();

    const SystemEnvData*    GetSystemData() const;

    //  per default systemchildwindows erase their background for better plugin support
    //  however, this might not always be required
    void                    EnableEraseBackground( BOOL bEnable = TRUE );
    BOOL                    IsEraseBackgroundEnabled();
};

#endif // _SV_SYSCHILD_HXX
