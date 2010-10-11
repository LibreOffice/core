/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyrigt 2010 Novell, Inc.
 *
 * LibreOffice - a multi-platform office productivity suite
 *
 * This file is part of LibreOffice.
 *
 * LibreOffice is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * LibreOffice is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with LibreOffice.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION

#include "tpcompatibility.hxx"
#include "optdlg.hrc"
#include "scresid.hxx"

ScTpCompatOptions::ScTpCompatOptions(Window *pParent, const SfxItemSet &rCoreAttrs) :
    SfxTabPage(pParent, ScResId(RID_SCPAGE_COMPATIBILITY), rCoreAttrs),
    maFlKeyBindings(this, ScResId(FL_KEY_BINDINGS)),
    maFtKeyBindings(this, ScResId(FT_KEY_BINDINGS)),
    maLbKeyBindings(this, ScResId(LB_KEY_BINDINGS))
{
    FreeResource();
}

ScTpCompatOptions::~ScTpCompatOptions()
{
}

SfxTabPage* ScTpCompatOptions::Create(Window *pParent, const SfxItemSet &rCoreAttrs)
{
    return new ScTpCompatOptions(pParent, rCoreAttrs);
}

BOOL ScTpCompatOptions::FillItemSet(SfxItemSet &rCoreAttrs)
{
    return false;
}

void ScTpCompatOptions::Reset(const SfxItemSet &rCoreAttrs)
{
}

int ScTpCompatOptions::DeactivatePage(SfxItemSet* /*pSet*/)
{
    return KEEP_PAGE;
}
