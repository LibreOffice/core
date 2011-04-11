/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
    SAL_DLLPRIVATE void     ImplInitSysChild( Window* pParent, WinBits nStyle, SystemWindowData *pData, sal_Bool bShow = sal_False );
    SAL_DLLPRIVATE void     ImplTestJavaException( void* pEnv );

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE          SystemChildWindow (const SystemChildWindow &);
    SAL_DLLPRIVATE          SystemChildWindow & operator= (const SystemChildWindow &);

public:
                            SystemChildWindow( Window* pParent, WinBits nStyle = 0 );
                            // create a SystemChildWindow using the given SystemWindowData
                            SystemChildWindow( Window* pParent, WinBits nStyle, SystemWindowData *pData, sal_Bool bShow = sal_True );
                            SystemChildWindow( Window* pParent, const ResId& rResId );
                            ~SystemChildWindow();

    const SystemEnvData*    GetSystemData() const;

    //  per default systemchildwindows erase their background for better plugin support
    //  however, this might not always be required
    void                    EnableEraseBackground( sal_Bool bEnable = sal_True );
    sal_Bool                IsEraseBackgroundEnabled();
    void                    SetForwardKey( sal_Bool bEnable );
    // return the platform specific handle/id of this window;
    // in case the flag bUseJava is set, a java compatible overlay window
    // is created on which other java windows can be created (plugin interface)
    sal_IntPtr              GetParentWindowHandle( sal_Bool bUseJava = sal_False );
};

#endif // _SV_SYSCHILD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
