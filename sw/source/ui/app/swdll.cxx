/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <svx/svdobj.hxx>
#include <rtl/logfile.hxx>
#include "globdoc.hrc"
#include <swdll.hxx>
#include <wdocsh.hxx>
#include <globdoc.hxx>
#include <initui.hxx>
#include <swmodule.hxx>
#include <swtypes.hxx>
#include <init.hxx>
#include <dobjfac.hxx>
#include <cfgid.h>
#include <unotools/moduleoptions.hxx>
#include <svx/fmobjfac.hxx>
#include <svx/svdfield.hxx>
#include <svx/objfac3d.hxx>
#include <unomid.h>
#include <svx/sdrobjectfactory.hxx>

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

