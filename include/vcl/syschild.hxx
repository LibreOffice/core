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

#ifndef INCLUDED_VCL_SYSCHILD_HXX
#define INCLUDED_VCL_SYSCHILD_HXX

#include <vcl/dllapi.h>
#include <vcl/window.hxx>

struct SystemEnvData;
struct SystemWindowData;

class VCL_DLLPUBLIC SystemChildWindow : public vcl::Window
{
private:
    using Window::ImplInit;
    SAL_DLLPRIVATE void     ImplInitSysChild( vcl::Window* pParent, WinBits nStyle, SystemWindowData *pData, bool bShow = false );
    SAL_DLLPRIVATE void     ImplTestJavaException( void* pEnv );

    // Copy assignment is forbidden and not implemented.
                            SystemChildWindow (const SystemChildWindow &) = delete;
                            SystemChildWindow & operator= (const SystemChildWindow &) = delete;

public:
    explicit                SystemChildWindow( vcl::Window* pParent, WinBits nStyle = 0 );
                            // create a SystemChildWindow using the given SystemWindowData
    explicit                SystemChildWindow( vcl::Window* pParent, WinBits nStyle, SystemWindowData *pData, bool bShow = true );
    virtual                 ~SystemChildWindow();
    virtual void            dispose() SAL_OVERRIDE;

    const SystemEnvData*    GetSystemData() const;

    //  per default systemchildwindows erase their background for better plugin support
    //  however, this might not always be required
    void                    EnableEraseBackground( bool bEnable = true );
    void                    SetForwardKey( bool bEnable );
    // return the platform specific handle/id of this window;
    // in case the flag bUseJava is set, a java compatible overlay window
    // is created on which other java windows can be created (plugin interface)
    sal_IntPtr              GetParentWindowHandle( bool bUseJava = false );
};

#endif // INCLUDED_VCL_SYSCHILD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
