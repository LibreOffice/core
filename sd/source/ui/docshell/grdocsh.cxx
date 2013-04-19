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


#include <svx/svxids.hrc>
#include <sfx2/app.hxx>
#include <svl/srchitem.hxx>
#include <tools/globname.hxx>

#include <comphelper/classids.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
