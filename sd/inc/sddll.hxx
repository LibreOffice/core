/*************************************************************************
 *
 *  $RCSfile: sddll.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SDDLL_HXX
#define _SDDLL_HXX

#ifndef _SFXMODULE_HXX //autogen
#include <sfx2/module.hxx>
#endif
#ifndef _SFXDEFS_HXX //autogen
#include <sfx2/sfxdefs.hxx>
#endif

class SvFactory;
class SotFactory;
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
protected:
    static void     RegisterFactorys();
    static void     RegisterInterfaces();
    static void     RegisterControllers();

public:
                    // Ctor/Dtor must be linked to the application
                    SdDLL();
                    ~SdDLL();

                    // static-init/exit-code must be linked to the application
    static void     LibInit();  // called from SfxApplication-subclass::Init()
    static void     LibExit();  // called from SfxApplication-subclass::Exit()

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
#ifndef SO3
    SvFactory* pSdDrawDocShellFactory;
    SvFactory* pSdGraphicDocShellFactory;

               SdModuleDummy(ResMgr* pResMgr, BOOL bDummy,
                             SvFactory* pDrawObjFact, SvFactory* pGraphicObjFact)
#else
    SotFactory* pSdDrawDocShellFactory;
    SotFactory* pSdGraphicDocShellFactory;

               SdModuleDummy(ResMgr* pResMgr, BOOL bDummy,
                             SotFactory* pDrawObjFact, SotFactory* pGraphicObjFact)
#endif
               : SfxModule(pResMgr, bDummy,
                            // Der erste Factory-Pointer muss gueltig sein!
                           (SfxObjectFactory*) (pDrawObjFact ? pDrawObjFact    : pGraphicObjFact),
                           (SfxObjectFactory*) (pDrawObjFact ? pGraphicObjFact : pDrawObjFact),
                           0L),
                 pSdDrawDocShellFactory( pDrawObjFact ),
                 pSdGraphicDocShellFactory( pGraphicObjFact )
               {}

    virtual SfxModule*  Load();

    static SvGlobalName GetID(USHORT nFileFormat);
    static USHORT       HasID(const SvGlobalName& rName);
};




#ifndef _SD_DLL                      // Das define muss im Draw gesetzt werden
#define SD_MOD() ( *(SdModuleDummy**) GetAppData(SHL_DRAW) )
#endif

#endif                               // _SDDLL_HXX

