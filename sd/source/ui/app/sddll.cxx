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
#include <svtools/FilterConfigItem.hxx>
#include <com/sun/star/util/XArchiver.hpp>
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

    // SvDraw-Felder registrieren
    SdrRegisterFieldClasses();

    // 3D-Objekt-Factory eintragen
    E3dObjFactory();

    // ::com::sun::star::form::component::Form-Objekt-Factory eintragen
    FmFormObjFactory();

    // Objekt-Factory eintragen
    SdrObjFactory::InsertMakeUserDataHdl(LINK(&aSdObjectFactory, SdObjectFactory, MakeUserData));

    // register your exotic remote controlls here
    RegisterRemotes();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
