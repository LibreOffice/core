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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <sfx2/app.hxx>
#include <svl/srchitem.hxx>
#include <tools/globname.hxx>

#ifndef _SO_CLSIDS_HXX //autogen
#include <sot/clsids.hxx>
#endif

#include <sfx2/objface.hxx>


#include "app.hrc"
#include "strings.hrc"
#include "glob.hrc"
#include "GraphicDocShell.hxx"
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"

using namespace sd;
#define GraphicDocShell
#include "sdgslots.hxx"

namespace sd
{
TYPEINIT1(GraphicDocShell, DrawDocShell);

SFX_IMPL_INTERFACE(GraphicDocShell, SfxObjectShell, SdResId(0))
{
    SFX_CHILDWINDOW_REGISTRATION(SID_SEARCH_DLG);
    SFX_CHILDWINDOW_REGISTRATION( SID_HYPERLINK_INSERT );
}

SFX_IMPL_OBJECTFACTORY( GraphicDocShell, SvGlobalName(SO3_SDRAW_CLASSID_60), SFXOBJECTSHELL_STD_NORMAL, "sdraw" )

GraphicDocShell::GraphicDocShell(SfxObjectCreateMode eMode,
                                     sal_Bool bDataObject,
                                     DocumentType eDocType) :
    DrawDocShell(eMode, bDataObject, eDocType)
{
    SetStyleFamily( SD_STYLE_FAMILY_GRAPHICS );
}

GraphicDocShell::GraphicDocShell(const sal_uInt64 nModelCreationFlags,
                                     sal_Bool bDataObject,
                                     DocumentType eDocType) :
    DrawDocShell(nModelCreationFlags, bDataObject, eDocType)
{
    SetStyleFamily( SD_STYLE_FAMILY_GRAPHICS );
}

GraphicDocShell::~GraphicDocShell()
{
}


} // end of namespace sd
