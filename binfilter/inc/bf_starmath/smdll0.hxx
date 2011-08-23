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

#ifndef SMDLL0_HXX
#define SMDLL0_HXX

#include <bf_so3/pseudo.hxx>

#include <bf_sfx2/module.hxx>
#include <tools/shl.hxx>
class SvFactory; 
class SotFactory; 

namespace binfilter {

/*************************************************************************
|*
|* define dummy-version only if not in Sd-DLL
|*
\************************************************************************/

#define SM_MOD() ( *(SmModuleDummy**) GetAppData(BF_SHL_SM) )

/*************************************************************************
|*
|* This tricky class keeps pointers to the SvFactories while
|* the DLL isn`t loaded. A pointer to the one instance is available
|* through SD_MOD() (shared-lib-app-data).
|*
\************************************************************************/

class SmModuleDummy : public SfxModule
{
public:
    TYPEINFO();

                // SvFactory name convention:
                // 'p' + SfxObjectShell-subclass + 'Factory'
    SotFactory *pSmDocShellFactory;

    SmModuleDummy(ResMgr *pResMgrIn, BOOL bDummyIn, SotFactory *pObjFact) :
       SfxModule(pResMgrIn, bDummyIn, (SfxObjectFactory*) pObjFact, NULL), pSmDocShellFactory(pObjFact)
    {
    }

    virtual SfxModule *Load ();

    static const SvGlobalName GetID(USHORT nFileFormat);
    static USHORT HasID (const SvGlobalName& rID);
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
