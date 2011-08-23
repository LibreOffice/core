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

#ifndef _SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif
#define ITEMID_SEARCH                   SID_SEARCH_ITEM
#ifndef _SFXAPP_HXX
#include <bf_sfx2/app.hxx>
#endif


#ifndef _SO_CLSIDS_HXX //autogen
#include <comphelper/classids.hxx>
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "strings.hrc"
#include "glob.hrc"

#include "bf_sd/grdocsh.hxx"
#include "sdresid.hxx"

namespace binfilter
{

TYPEINIT1(SdGraphicDocShell, SdDrawDocShell);

SFX_IMPL_OBJECTFACTORY_LOD(SdGraphicDocShell, sdraw, SvGlobalName(BF_SO3_SDRAW_CLASSID), Sd)

SdGraphicDocShell::SdGraphicDocShell(SfxObjectCreateMode eMode, BOOL bDataObject, DocumentType eDocType)
: SdDrawDocShell(eMode, bDataObject, eDocType)
{
    SetStyleFamily( 2 );
}

SdGraphicDocShell::~SdGraphicDocShell()
{
}

}
