/*************************************************************************
 *
 *  $RCSfile: swdll.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:46:16 $
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


#pragma hdrstop

#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#ifndef _GLOBDOC_HRC
#include "globdoc.hrc"
#endif

#ifndef _SWDLL_HXX
#include <swdll.hxx>
#endif
#ifndef _SWWDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _SWGLOBDOCSH_HXX
#include <globdoc.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _INIT_HXX
#include <init.hxx>
#endif
#ifndef _DOBJFAC_HXX
#include <dobjfac.hxx>
#endif
#ifndef _CFGID_H
#include <cfgid.h>
#endif

#include <svtools/moduleoptions.hxx>

/*************************************************************************
|*
|* Init
|*
\************************************************************************/

void SwDLL::Init()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDLL" );

    // the SdModule must be created
    SwModule** ppShlPtr = (SwModule**) GetAppData(SHL_WRITER);
    if ( *ppShlPtr )
        return;

    SvtModuleOptions aOpt;
    SfxObjectFactory* pDocFact = 0;
    SfxObjectFactory* pGlobDocFact = 0;
    if ( aOpt.IsWriter() )
    {
        pDocFact = &SwDocShell::Factory();
        pGlobDocFact = &SwGlobalDocShell::Factory();
    }

    SfxObjectFactory* pWDocFact = &SwWebDocShell::Factory();

    SwModule* pModule = new SwModule( pWDocFact, pDocFact, pGlobDocFact );
    (*ppShlPtr) = pModule;

    if ( aOpt.IsWriter() )
    {
        SwDocShell::RegisterFactory(    SDT_SW_DOCFACTPRIO      );
        SwGlobalDocShell::RegisterFactory(SDT_SW_DOCFACTPRIO + 2);
    }

    // WebWriter alway needed because it is used for the help viewer
    SwWebDocShell::RegisterFactory( SDT_SW_DOCFACTPRIO + 1  );

    SdrObjFactory::InsertMakeObjectHdl( LINK( &aSwObjectFactory, SwObjectFactory, MakeObject ) );

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "Init Core/UI/Filter" );

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
    SwModule** ppShlPtr = (SwModule**) GetAppData(SHL_WRITER);
    delete (*ppShlPtr);
    (*ppShlPtr) = NULL;
}

