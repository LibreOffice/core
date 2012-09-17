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

#ifndef SC_SCDLL_HXX
#define SC_SCDLL_HXX

class SfxMedium;
class SfxFilter;

#include <sfx2/sfxdefs.hxx>
#include <sfx2/module.hxx>
#include <sfx2/docfac.hxx>

#include "scdllapi.h"

/**
 * This class is a wrapper for a Load-On-Demand-DLL. One instance per
 * SfxApplication will be created for the runtime of
 * SfxApplication-subclass::Main().
 *
 * Remember: Do export this class! It is used by the application.
 */
class ScDLL
{
public:
                    // Ctor/Dtor must be linked to the application
                    ScDLL();
                    ~ScDLL();

                    // static-init/exit-code must be linked to the application
    static void     LibInit();  // called from SfxApplication-subclass::Init()
    static void     LibExit();  // called from SfxApplication-subclass::Exit()
    static void     PreExit();  // muss vor LibExit gerufen werden

                    // DLL-init/exit-code must be linked to the DLL only
    static SC_DLLPUBLIC void     Init();     // called directly after loading the DLL

    static sal_uLong    DetectFilter( SfxMedium& rMedium, const SfxFilter** ppFilter,
                                    SfxFilterFlags nMust, SfxFilterFlags nDont );
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
