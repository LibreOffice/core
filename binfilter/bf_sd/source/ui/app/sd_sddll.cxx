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

#include <bf_svx/svdobj.hxx>
#include <bf_svtools/moduleoptions.hxx>

#include "bf_sd/grdocsh.hxx"
#include "sdresid.hxx"
#include "sdobjfac.hxx"
#include "cfgids.hxx"

namespace binfilter {

void SdDLL::Init()
{
    // the SdModule must be created
    SdModuleDummy** ppShlPtr = (SdModuleDummy**) GetAppData(BF_SHL_DRAW);
    SvFactory* pDrawFact    = (SvFactory*)(*ppShlPtr)->pSdDrawDocShellFactory;
    SvFactory* pGraphicFact = (SvFactory*)(*ppShlPtr)->pSdGraphicDocShellFactory;
    delete (*ppShlPtr);
    (*ppShlPtr) = new SdModule(pDrawFact, pGraphicFact);
    (*ppShlPtr)->pSdDrawDocShellFactory    = pDrawFact;
    (*ppShlPtr)->pSdGraphicDocShellFactory = pGraphicFact;

    SdrObjFactory::InsertMakeUserDataHdl(LINK(&aSdObjectFactory, SdObjectFactory, MakeUserData));
}

void SdDLL::Exit()
{
    SdrObjFactory::RemoveMakeUserDataHdl(LINK(&aSdObjectFactory, SdObjectFactory, MakeUserData));

    SdModuleDummy** ppShlPtr = (SdModuleDummy**) GetAppData(BF_SHL_DRAW);
    delete (*ppShlPtr);
    (*ppShlPtr) = NULL;
}
}
