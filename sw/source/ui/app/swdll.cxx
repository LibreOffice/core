/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swdll.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 10:17:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



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

#ifndef _FM_FMOBJFAC_HXX
#include <svx/fmobjfac.hxx>
#endif
#ifndef _SVDFIELD_HXX
#include <svx/svdfield.hxx>
#endif
#ifndef _OBJFAC3D_HXX
#include <svx/objfac3d.hxx>
#endif

#include <unomid.h>


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

    pWDocFact->SetDocumentServiceName(C2S("com.sun.star.text.WebDocument"));

    if ( aOpt.IsWriter() )
    {
        pGlobDocFact->SetDocumentServiceName(C2S("com.sun.star.text.GlobalDocument"));
        pDocFact->SetDocumentServiceName(C2S("com.sun.star.text.TextDocument"));
    }

    // SvDraw-Felder registrieren
    SdrRegisterFieldClasses();

    // 3D-Objekt-Factory eintragen
    E3dObjFactory();

    // form::component::Form-Objekt-Factory eintragen
    FmFormObjFactory();

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

