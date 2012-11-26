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
#include "precompiled_sd.hxx"
#include <editeng/eeitem.hxx>
#include <editeng/editeng.hxx>
#include <svx/svdobj.hxx>
#include <unotools/moduleoptions.hxx>
#include <svx/fmobjfac.hxx>
#include <svx/svdfield.hxx>
#include <svx/objfac3d.hxx>
#include "sddll.hxx"
#include "DrawDocShell.hxx"
#include "GraphicDocShell.hxx"
#include "sdresid.hxx"
#include "sdobjfac.hxx"
#include "cfgids.hxx"
#include "strmname.h"
#include "SdShapeTypes.hxx"
#include <svx/SvxShapeTypes.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <tools/urlobj.hxx>
#include <svtools/FilterConfigItem.hxx>
#include <com/sun/star/util/XArchiver.hpp>
#include <comphelper/processfactory.hxx>
#include <svx/sdrobjectfactory.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;


/*************************************************************************
|*
|* Init
|*
\************************************************************************/

void SdDLL::Init()
{
    if ( SD_MOD() )
        return;

    SfxObjectFactory* pDrawFact = NULL;
    SfxObjectFactory* pImpressFact = NULL;

    if (SvtModuleOptions().IsImpress())
        pImpressFact = &::sd::DrawDocShell::Factory();

    if (SvtModuleOptions().IsDraw())
        pDrawFact = &::sd::GraphicDocShell::Factory();

    // the SdModule must be created
     SdModule** ppShlPtr = (SdModule**) GetAppData(SHL_DRAW);

     // #i46427#
     // The SfxModule::SfxModule stops when the first given factory
     // is 0, so we must not give a 0 as first factory
     if( pImpressFact )
     {
        (*ppShlPtr) = new SdModule( pImpressFact, pDrawFact );
     }
     else
     {
        (*ppShlPtr) = new SdModule( pDrawFact, pImpressFact );
     }

    if (SvtModuleOptions().IsImpress())
    {
        // Register the Impress shape types in order to make the shapes accessible.
        ::accessibility::RegisterImpressShapeTypes ();
        ::sd::DrawDocShell::Factory().SetDocumentServiceName( String( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.PresentationDocument" ) ) );
    }

    if (SvtModuleOptions().IsDraw())
    {
        ::sd::GraphicDocShell::Factory().SetDocumentServiceName( String( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.DrawingDocument" ) ) );
    }

    // register your view-factories here
    RegisterFactorys();

    // register your shell-interfaces here
    RegisterInterfaces();

    // register your controllers here
    RegisterControllers();

    // SvDraw-Felder registrieren
    SdrRegisterFieldClasses();

    // 3D-Objekt-Factory eintragen
    E3dObjFactory();

    // ::com::sun::star::form::component::Form-Objekt-Factory eintragen
    FmFormObjFactory();

    // Objekt-Factory eintragen
    SdrObjFactory::InsertMakeUserDataHdl(LINK(&aSdObjectFactory, SdObjectFactory, MakeUserData));
}



/*************************************************************************
|*
|* Exit
|*
\************************************************************************/

void SdDLL::Exit()
{
    // called directly befor unloading the DLL
    // do whatever you want, Sd-DLL is accessible

    // Objekt-Factory austragen
    SdrObjFactory::RemoveMakeUserDataHdl(LINK(&aSdObjectFactory, SdObjectFactory, MakeUserData));

    // the SdModule must be destroyed
    SdModule** ppShlPtr = (SdModule**) GetAppData(SHL_DRAW);
    delete (*ppShlPtr);
    (*ppShlPtr) = NULL;
}

