/*************************************************************************
 *
 *  $RCSfile: swdll.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-10-12 08:24:08 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif


#include "swdll.hxx"
#ifndef _SWWDOCSH_HXX //autogen
#include <wdocsh.hxx>
#endif
#ifndef _SWGLOBDOCSH_HXX //autogen
#include <globdoc.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>
#endif
#ifndef _SWMODULE_HXX //autogen
#include <swmodule.hxx>
#endif
#ifndef _SWTYPES_HXX //autogen
#include <swtypes.hxx>
#endif
#ifndef _INIT_HXX
#include <init.hxx>
#endif
#ifndef _DOBJFAC_HXX //autogen
#include <dobjfac.hxx>
#endif
#ifndef _CFGID_H
#include <cfgid.h>
#endif

/*************************************************************************
|*
|* Init
|*
\************************************************************************/

void SwDLL::Init()
{
    // called directly after loading the DLL
    // do whatever you want, you may use Sw-DLL too

    // the SdModule must be created
    SwModuleDummy** ppShlPtr = (SwModuleDummy**) GetAppData(SHL_WRITER);

    SvFactory* pDocFact     = (SvFactory*)(*ppShlPtr)->pSwDocShellFactory;
    SvFactory* pWDocFact    = (SvFactory*)(*ppShlPtr)->pSwWebDocShellFactory;
    SvFactory* pGlobDocFact = (SvFactory*)(*ppShlPtr)->pSwGlobalDocShellFactory;
    delete (*ppShlPtr);
    SwModule* pModule = new SwModule(pDocFact, pWDocFact, pGlobDocFact);;
    (*ppShlPtr) = pModule;
    (*ppShlPtr)->pSwDocShellFactory         = pDocFact    ;
    (*ppShlPtr)->pSwWebDocShellFactory      = pWDocFact   ;
    (*ppShlPtr)->pSwGlobalDocShellFactory   = pGlobDocFact;

    SwDocShell::Factory().RegisterPluginMenuBar( SW_RES(CFG_SW_MENU_PORTAL));
    SwDocShell::Factory().RegisterMenuBar(SW_RES(CFG_SW_MENU));
    SwDocShell::Factory().RegisterAccel(SW_RES(CFG_SW_ACCEL));
    SwWebDocShell::Factory().RegisterPluginMenuBar( SW_RES(CFG_SWWEB_MENU_PORTAL));
    SwWebDocShell::Factory().RegisterMenuBar(SW_RES(CFG_SWWEB_MENU));
    SwWebDocShell::Factory().RegisterAccel(SW_RES(CFG_SWWEB_ACCEL));
    SwGlobalDocShell::Factory().RegisterMenuBar(SW_RES(CFG_SW_MENU));
    SwGlobalDocShell::Factory().RegisterAccel(SW_RES(CFG_SW_ACCEL));


    SdrObjFactory::InsertMakeObjectHdl( LINK( &aSwObjectFactory, SwObjectFactory, MakeObject ) );
    //Initialisierung der Statics
    ::_InitCore();
    ::_InitFilter();
    ::_InitUI();

    pModule->InitAttrPool();
    //jetzt darf das SwModule seinen Pool anlegen

    // register your view-factories here
    RegisterFactories();

    // register your shell-interfaces here
    RegisterInterfaces();

    // register your controllers here
    RegisterControls();

}



/*************************************************************************
|*
|* Exit
|*
\************************************************************************/

void SwDLL::Exit()
{
    // called directly befor unloading the DLL
    // do whatever you want, Sw-DLL is accessible

    // der Pool muss vor den statics geloescht werden
    SW_MOD()->RemoveAttrPool();

    ::_FinitUI();
    ::_FinitFilter();
    ::_FinitCore();
    // Objekt-Factory austragen
    SdrObjFactory::RemoveMakeObjectHdl(LINK(&aSwObjectFactory, SwObjectFactory, MakeObject ));
   // the SwModule must be destroyed
    SwModuleDummy** ppShlPtr = (SwModuleDummy**) GetAppData(SHL_WRITER);
    delete (*ppShlPtr);
    (*ppShlPtr) = NULL;
}

