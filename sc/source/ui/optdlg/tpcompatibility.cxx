/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
