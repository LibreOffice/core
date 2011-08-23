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

class ResMgr;
class SvFactory; 
class SotFactory;
class StatusBar;

class SfxMedium;
class SfxFilter;

#include <bf_sfx2/sfxdefs.hxx>
#include <bf_sfx2/module.hxx>

#include <bf_so3/pseudo.hxx>

namespace binfilter {

class SwDLL

/*	[Description]

    This class is a wrapper for a Load-On-Demand-DLL. One instance
    per SfxApplication will be created for the runtime of
    SfxApplication-subclass::Main().

    Remember: Do export this class! It is used by the application.
*/

{

    static void RegisterFactories();

public:
                    // Ctor/Dtor must be linked to the application
                    SwDLL();
                    ~SwDLL();

                    // static-init/exit-code must be linked to the application
    static void 	LibInit();	// called from SfxApplication-subclass::Init()
    static void 	LibExit();	// called from SfxApplication-subclass::Exit()

                    // DLL-init/exit-code must be linked to the DLL only
    static void 	Init(); 	// called directly after loading the DLL
    static void 	Exit(); 	// called directly befor unloading the DLL

    static ULONG	DetectFilter( SfxMedium& rMedium, const SfxFilter** ppFilter,
                                    SfxFilterFlags nMust, SfxFilterFlags nDont );
    static ULONG	GlobDetectFilter( SfxMedium& rMedium, const SfxFilter** ppFilter,
                                    SfxFilterFlags nMust, SfxFilterFlags nDont );

    static sal_Bool RegisterFrameLoaders( void* pServiceManager, void* pRegistryKey );
    static void*    CreateFrameLoader( const sal_Char* pImplementationName, void* pServiceManager, void *pRegistryKey );
};

//-------------------------------------------------------------------------

class SwModuleDummy : public SfxModule

/*	[Description]

    This tricky class keeps pointers to the SvFactories while
    the DLL isn`t loaded. A pointer to the one instance is available
    through SXX_MOD() (shared-lib-app-data).
*/

{
public:
    virtual ~SwModuleDummy();
    TYPEINFO();
                    // SvFactory name convention:
                    // 'p' + SfxObjectShell-subclass + 'Factory'
    SotFactory*		pSwDocShellFactory;
    SotFactory*		pSwWebDocShellFactory;
    SotFactory*		pSwGlobalDocShellFactory;

                    SwModuleDummy( ResMgr *pResMgrIn, BOOL bDummyIn,
                                    SotFactory* pWebFact,
                                    SotFactory* pFact,
                                    SotFactory* pGlobalFact )
                    :	SfxModule(pResMgrIn, bDummyIn,
                                (SfxObjectFactory*)pFact,
                                (SfxObjectFactory*)pWebFact,
                                (SfxObjectFactory*)pGlobalFact, 0L),
                        pSwDocShellFactory( pFact ),
                        pSwWebDocShellFactory( pWebFact ),
                        pSwGlobalDocShellFactory( pGlobalFact )
                    {}

    virtual SfxModule* Load();
};

//-------------------------------------------------------------------------

#define SW_DLL() ( *(SwModuleDummy**) GetAppData(BF_SHL_WRITER) )

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
