/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "cairo_cairo.hxx"

#include <vcl/sysdata.hxx>
#include <vcl/syschild.hxx>

namespace cairo
{
 /***************************************************************************************
 * Platform independent part of surface backends for OpenOffice.org Cairo Canvas        *
 * For the rest of the functions (and the platform-specific derived                     *
 *  Surface classes), see platform specific cairo_<platform>_cairo.cxx                  *
 ****************************************************************************************/

    const SystemEnvData* GetSysData(const Window *pOutputWindow)
    {
        const SystemEnvData* pSysData = NULL;
        
        
        
        const SystemChildWindow* pSysChild = dynamic_cast< const SystemChildWindow* >( pOutputWindow );
        if( pSysChild )
            pSysData = pSysChild->GetSystemData();
        else
            pSysData = pOutputWindow->GetSystemData();
        return pSysData;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
