/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <editeng/eeitem.hxx>

#include <editeng/editeng.hxx>
#include <svx/svdobj.hxx>
#include <unotools/moduleoptions.hxx>
#include <svx/fmobjfac.hxx>
#include <svx/svdfield.hxx>
#include <svx/objfac3d.hxx>
#include <vcl/svapp.hxx>

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
#include <vcl/FilterConfigItem.hxx>
#include <comphelper/processfactory.hxx>

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
        ::sd::DrawDocShell::Factory().SetDocumentServiceName( "com.sun.star.presentation.PresentationDocument" );
    }

    if (SvtModuleOptions().IsDraw())
    {
        ::sd::GraphicDocShell::Factory().SetDocumentServiceName( "com.sun.star.drawing.DrawingDocument" );
    }

    // register your view-factories here
    RegisterFactorys();

    // register your shell-interfaces here
    RegisterInterfaces();

    // register your controllers here
    RegisterControllers();

    // register SvDraw-Fields
    SdrRegisterFieldClasses();

    // register 3D-Objekt-Factory
    E3dObjFactory();

    // register ::com::sun::star::form::component::Form-Object-Factory
    FmFormObjFactory();

    // register Object-Factory
    SdrObjFactory::InsertMakeUserDataHdl(LINK(&aSdObjectFactory, SdObjectFactory, MakeUserData));

    // register your exotic remote controlls here
#ifdef ENABLE_SDREMOTE
    if ( !Application::IsHeadlessModeRequested() )
        RegisterRemotes();
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
