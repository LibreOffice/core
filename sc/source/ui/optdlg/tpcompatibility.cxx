/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2010 Novell, Inc.
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
#include "docoptio.hxx"

ScTpCompatOptions::ScTpCompatOptions(Window *pParent, const SfxItemSet &rCoreAttrs) :
    SfxTabPage(pParent, ScResId(RID_SCPAGE_COMPATIBILITY), rCoreAttrs),
    maFlKeyBindings(this, ScResId(FL_KEY_BINDINGS)),
    maFtKeyBindings(this, ScResId(FT_KEY_BINDINGS)),
    maLbKeyBindings(this, ScResId(LB_KEY_BINDINGS))
{
    FreeResource();

    const ScTpCalcItem& rItem = static_cast<const ScTpCalcItem&>(
        rCoreAttrs.Get(GetWhich(SID_SCDOCOPTIONS)));
    mpOldOptions.reset(new ScDocOptions(rItem.GetDocOptions()));
    mpNewOptions.reset(new ScDocOptions(rItem.GetDocOptions()));
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
    ScOptionsUtil::KeyBindingType eKeyB = ScOptionsUtil::KEY_DEFAULT;
    switch (maLbKeyBindings.GetSelectEntryPos())
    {
        case 0:
            eKeyB = ScOptionsUtil::KEY_DEFAULT;
        break;
        case 1:
            eKeyB = ScOptionsUtil::KEY_OOO_LEGACY;
        break;
        default:
            ;
    }
    mpNewOptions->SetKeyBindingType(eKeyB);

    if (*mpNewOptions != *mpOldOptions)
    {
        rCoreAttrs.Put(ScTpCalcItem(GetWhich(SID_SCDOCOPTIONS), *mpNewOptions));
        return true;
    }
    else
        return false;
}

void ScTpCompatOptions::Reset(const SfxItemSet &/*rCoreAttrs*/)
{
    ScOptionsUtil::KeyBindingType eKeyB = mpOldOptions->GetKeyBindingType();
    switch (eKeyB)
    {
        case ScOptionsUtil::KEY_DEFAULT:
            maLbKeyBindings.SelectEntryPos(0);
        break;
        case ScOptionsUtil::KEY_OOO_LEGACY:
            maLbKeyBindings.SelectEntryPos(1);
        break;
        default:
            ;
    }
}

int ScTpCompatOptions::DeactivatePage(SfxItemSet* /*pSet*/)
{
    return KEEP_PAGE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
