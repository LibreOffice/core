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

#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>

#include <pgbrksh.hxx>
#include <tabvwsh.hxx>
#include <document.hxx>

#define ShellClass_ScPageBreakShell
#include <scslots.hxx>


SFX_IMPL_INTERFACE(ScPageBreakShell, SfxShell)

void ScPageBreakShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu("pagebreak");
}

ScPageBreakShell::ScPageBreakShell( ScTabViewShell* pViewSh ) :
    SfxShell(pViewSh)
{
    SetPool( &pViewSh->GetPool() );
    ScViewData& rViewData = pViewSh->GetViewData();
    SfxUndoManager* pMgr = rViewData.GetSfxDocShell()->GetUndoManager();
    SetUndoManager( pMgr );
    if ( !rViewData.GetDocument().IsUndoEnabled() )
    {
        pMgr->SetMaxUndoActionCount( 0 );
    }
    SetName("PageBreak");
}

ScPageBreakShell::~ScPageBreakShell()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
