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
#ifndef SW_SWDLL_HXX
#define SW_SWDLL_HXX

class StatusBar;

#include <sfx2/sfxdefs.hxx>
#include <sfx2/module.hxx>

#include <tools/shl.hxx>

//-------------------------------------------------------------------------

class SwDLL

/**
 * This class is a wrapper for a Load-On-Demand-DLL. One instance
 * per SfxApplication will be created for the runtime of
 * SfxApplication-subclass::Main().
 *
 * Remember: Do export this class! It is used by the application.
*/

{

    static void RegisterFactories();
    static void RegisterInterfaces();
    static void RegisterControls();
public:
                    // Ctor/Dtor must be linked to the application
                    SwDLL();
                    ~SwDLL();

    static void     Init();     // called directly after loading the DLL
    static void     Exit();     // called directly befor unloading the DLL
};

//-------------------------------------------------------------------------

#define SW_DLL() ( *(SwModule**) GetAppData(SHL_WRITER) )

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
