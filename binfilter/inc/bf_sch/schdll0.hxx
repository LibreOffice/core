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

#ifndef _SCHDLL0_HXX
#define _SCHDLL0_HXX

#include <bf_so3/pseudo.hxx>

#ifndef _SFXMODULE_HXX //autogen
#include <bf_sfx2/module.hxx>
#endif
#ifndef _TOOLS_RESMGR_HXX //autogen
#include <tools/resmgr.hxx>
#endif

class SotFactory;
class SvFactory;
namespace binfilter {

/*************************************************************************
|*
|* This tricky class keeps pointers to the SvFactories while
|* the DLL isn`t loaded. A pointer to the one instance is available
|* through SD_MOD() (shared-lib-app-data).
|*
\************************************************************************/

class SchModuleDummy : public SfxModule
{
public:
    TYPEINFO();

                // SvFactory name convention:
                // 'p' + SfxObjectShell-subclass + 'Factory'
    SotFactory *pSchChartDocShellFactory;  // Namensaufbau zwingend!!

    SchModuleDummy(ResMgr	 *pResMgr,
                   BOOL 	 bDummy,
                   SotFactory *pObjFact) :
        SfxModule(pResMgr, bDummy, (SfxObjectFactory*) pObjFact, NULL),
        pSchChartDocShellFactory(pObjFact)
    {
    }

    virtual SfxModule *Load ();

    static const SvGlobalName GetID(USHORT nFileFormat);
    static USHORT HasID (const SvGlobalName& rID);

};

#ifndef _SCH_DLL					  // Das define muss im Draw gesetzt werden
#define SCH_MOD() ( *(SchModuleDummy**) GetAppData(BF_SHL_SCH) )
#endif

} //namespace binfilter
#endif
