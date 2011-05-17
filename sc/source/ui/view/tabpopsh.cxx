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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


#include "tabpopsh.hxx"
#include "sc.hrc"

#undef ShellClass
#define ShellClass ScTabPopShell
SFX_SLOTMAP(ScTabPopShell)
{
    SFX_SLOT( 0,0, DummyExec, DummyState, 0, SfxVoidItem )
};


TYPEINIT1(ScTabPopShell,SfxShell);

//SFX_IMPL_IDL_INTERFACE(ScTabPopShell, SfxShell, 0)
SFX_IMPL_INTERFACE(ScTabPopShell, SfxShell, ResId( 0, NULL))
{
    SFX_POPUPMENU_REGISTRATION( RID_POPUP_TAB );
}




ScTabPopShell::ScTabPopShell(SfxItemPool& rItemPool)
{
    SetPool( &rItemPool );
}


ScTabPopShell::~ScTabPopShell()
{
}


void ScTabPopShell::DummyExec( SfxRequest& rReq )
{
}


void ScTabPopShell::DummyState( SfxItemSet& rSet )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
