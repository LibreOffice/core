/*************************************************************************
 *
 *  $RCSfile: scdll.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:50 $
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

#ifndef SC_SCDLL_HXX
#define SC_SCDLL_HXX

class ResMgr;
class SvFactory;
class SotFactory;
class StatusBar;
class SfxMedium;
class SfxFilter;

#ifndef _SFXDEFS_HXX //autogen
#include <sfx2/sfxdefs.hxx>
#endif
#ifndef _SFXMODULE_HXX //autogen
#include <sfx2/module.hxx>
#endif
#ifndef _SFX_OBJFAC_HXX
#include <sfx2/docfac.hxx>
#endif

//-------------------------------------------------------------------------

class ScDLL

/*  [Description]

    This class is a wrapper for a Load-On-Demand-DLL. One instance
    per SfxApplication will be created for the runtime of
    SfxApplication-subclass::Main().

    Remember: Do export this class! It is used by the application.
*/

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
    static void     Init();     // called directly after loading the DLL
    static void     Exit();     // called directly befor unloading the DLL


    static void     FillStatusBar(StatusBar &rBar);

    static ULONG    DetectFilter( SfxMedium& rMedium, const SfxFilter** ppFilter,
                                    SfxFilterFlags nMust, SfxFilterFlags nDont );
};

//-------------------------------------------------------------------------

#ifdef WNT
typedef void ( __cdecl * ScSigCatchFunc )( int );
#else
typedef void ( * ScSigCatchFunc )( int );
#endif
typedef ScSigCatchFunc ( * ScLibSignalFunc )( int, ScSigCatchFunc );

class ScModuleDummy: public SfxModule

/*  [Description]

    This tricky class keeps pointers to the SvFactories while
    the DLL isn`t loaded. A pointer to the one instance is available
    through SXX_MOD() (shared-lib-app-data).
*/

{
public:
                    // SvFactory name convention:
                    // 'p' + SfxObjectShell-subclass + 'Factory'
    SotFactory*     pScDocShellFactory;
    ScLibSignalFunc pSignalFunc;

                    ScModuleDummy( ResMgr *pResMgr, BOOL bDummy, SfxObjectFactory* pFact )
                    :   SfxModule(pResMgr, bDummy, pFact, NULL),
                        pScDocShellFactory(pFact), pSignalFunc(0)
                    {}

    virtual SfxModule* Load();

    ScLibSignalFunc GetSignalFunc() { return pSignalFunc; }

    static SvGlobalName GetID(USHORT nFileFormat);
    static USHORT       HasID(const SvGlobalName& rName);
};

//-------------------------------------------------------------------------

#define SC_DLL() ( *(ScModuleDummy**) GetAppData(SHL_CALC) )

#endif


