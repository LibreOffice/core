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


#undef SC_DLLIMPLEMENTATION

#include <scmod.hxx>
#include "tpcompatibility.hxx"
#include "optdlg.hrc"
#include "scresid.hxx"
#include "appoptio.hxx"

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

sal_Bool ScTpCompatOptions::FillItemSet(SfxItemSet &rCoreAttrs)
{
    bool bRet = false;
    if (maLbKeyBindings.GetSavedValue() != maLbKeyBindings.GetSelectEntryPos())
    {
        rCoreAttrs.Put(
            SfxUInt16Item(
                SID_SC_OPT_KEY_BINDING_COMPAT, maLbKeyBindings.GetSelectEntryPos()));
        bRet = true;
    }
    return bRet;
}

void ScTpCompatOptions::Reset(const SfxItemSet &rCoreAttrs)
{
    const SfxPoolItem* pItem;
    if (rCoreAttrs.HasItem(SID_SC_OPT_KEY_BINDING_COMPAT, &pItem))
    {
        const SfxUInt16Item* p16Item = static_cast<const SfxUInt16Item*>(pItem);
        ScOptionsUtil::KeyBindingType eKeyB =
            static_cast<ScOptionsUtil::KeyBindingType>(p16Item->GetValue());

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

    maLbKeyBindings.SaveValue();
}

int ScTpCompatOptions::DeactivatePage(SfxItemSet* /*pSet*/)
{
    return KEEP_PAGE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
