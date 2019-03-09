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
#include <tools/globname.hxx>

#include <comphelper/classids.hxx>

#include <sfx2/objface.hxx>

#include <GraphicDocShell.hxx>
#include <DrawDocShell.hxx>

using namespace sd;
#define ShellClass_GraphicDocShell
#include <sdgslots.hxx>

namespace sd
{

SFX_IMPL_SUPERCLASS_INTERFACE(GraphicDocShell, SfxObjectShell)

void GraphicDocShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterChildWindow(SID_SEARCH_DLG);
}

SFX_IMPL_OBJECTFACTORY( GraphicDocShell, SvGlobalName(SO3_SDRAW_CLASSID_60), "sdraw" )

GraphicDocShell::GraphicDocShell(SfxObjectCreateMode eMode) :
    DrawDocShell(eMode, /*bDataObject*/true, DocumentType::Draw)
{
    SetStyleFamily( SfxStyleFamily::Para );
}

GraphicDocShell::GraphicDocShell(SfxModelFlags nModelCreationFlags) :
    DrawDocShell(nModelCreationFlags, /*bDataObject*/false, DocumentType::Draw)
{
    SetStyleFamily( SfxStyleFamily::Para );
}

GraphicDocShell::~GraphicDocShell()
{
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
