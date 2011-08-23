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

#ifndef _SDDLL_HXX
#define _SDDLL_HXX

#include <bf_so3/pseudo.hxx>

#ifndef _SFXMODULE_HXX //autogen
#include <bf_sfx2/module.hxx>
#endif
#ifndef _SFXDEFS_HXX //autogen
#include <bf_sfx2/sfxdefs.hxx>
#endif
class SvFactory;
class SotFactory;
namespace binfilter {

class SfxMedium;
class SfxFilter;

/*************************************************************************
|*
|* This class is a wrapper for a Load-On-Demand-DLL. One instance
|* per SfxApplication will be created for the runtime of
|* SfxApplication-subclass::Main().
|*
|* Remember: Do export this class! It is used by the application.
|*
\************************************************************************/

class SdDLL
{
public:
                    // Ctor/Dtor must be linked to the application
                    SdDLL();
                    ~SdDLL();

                    // static-init/exit-code must be linked to the application
    static void 	LibInit();	// called from SfxApplication-subclass::Init()
    static void 	LibExit();	// called from SfxApplication-subclass::Exit()

                    // DLL-init/exit-code must be linked to the DLL only
    static void     Init();     // called directly after loading the DLL
    static void     Exit();     // called directly befor unloading the DLL

    static ULONG    DetectFilter(SfxMedium& rMedium, const SfxFilter** pFilter,
                                 SfxFilterFlags nMust, SfxFilterFlags nDont);
};

/*************************************************************************
|*
|* This tricky class keeps pointers to the SvFactories while
|* the DLL isn`t loaded. A pointer to the one instance is available
|* through SD_MOD() (shared-lib-app-data).
|*
\************************************************************************/

class SdModuleDummy : public SfxModule
{
public:
    TYPEINFO();

                // SvFactory name convention:
                // 'p' + SfxObjectShell-subclass + 'Factory'
    SotFactory* pSdDrawDocShellFactory;
    SotFactory* pSdGraphicDocShellFactory;

               SdModuleDummy(ResMgr* pResMgr, BOOL bDummy,
                             SotFactory* pDrawObjFact, SotFactory* pGraphicObjFact)
               : SfxModule(pResMgr, bDummy,
                            // Der erste Factory-Pointer muss gueltig sein!
                           (SfxObjectFactory*) (pDrawObjFact ? pDrawObjFact    : pGraphicObjFact),
                           (SfxObjectFactory*) (pDrawObjFact ? pGraphicObjFact : pDrawObjFact),
                           0L),
                 pSdDrawDocShellFactory( pDrawObjFact ),
                 pSdGraphicDocShellFactory( pGraphicObjFact )
               {}

    virtual SfxModule*  Load();
};

#ifndef _SD_DLL                      // Das define muss im Draw gesetzt werden
#define SD_MOD() ( *(SdModuleDummy**) GetAppData(BF_SHL_DRAW) )
#endif

} //namespace binfilter
#endif                               // _SDDLL_HXX

