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

#include "WriterInspectorTextPanel.hxx"
#include <svx/svxids.hrc>
#include <doc.hxx>
#include <editsh.hxx>
#include <ndtxt.hxx>
#include <fmtftn.hxx>
#include <txatbase.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <editeng/wghtitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>

using namespace css;

namespace sw::sidebar
{
VclPtr<vcl::Window>
WriterInspectorTextPanel::Create(vcl::Window* pParent,
                                 const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                 SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(
            "no parent Window given to WriterInspectorTextPanel::Create", nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to WriterInspectorTextPanel::Create",
                                             nullptr, 1);
    if (pBindings == nullptr)
        throw ::com::sun::star::lang::IllegalArgumentException(
            "no SfxBindings given to PageHeaderPanel::Create", nullptr, 0);

    return VclPtr<WriterInspectorTextPanel>::Create(pParent, rxFrame, pBindings);
}

WriterInspectorTextPanel::WriterInspectorTextPanel(
    vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
    : InspectorTextPanel(pParent, rxFrame)
    , maCharFontName(SID_ATTR_CHAR_FONT, *pBindings, *this)
    , maCharFontHeight(SID_ATTR_CHAR_FONTHEIGHT, *pBindings, *this)
    , maCharFontWeight(SID_ATTR_CHAR_WEIGHT, *pBindings, *this)
{
}

void WriterInspectorTextPanel::NotifyItemUpdate(const sal_uInt16 nSId,
                                                const SfxItemState /*eState*/,
                                                const SfxPoolItem* /*pState*/)
{
    SwDocShell* pDocSh = static_cast<SwDocShell*>(SfxObjectShell::Current());
    std::vector<OUString> store;
    switch (nSId)
    {
        case SID_ATTR_CHAR_WEIGHT:
        case SID_ATTR_CHAR_FONT:
        case SID_ATTR_CHAR_FONTHEIGHT:
        {
            if (pDocSh)
            {
                SwDoc* pDoc = pDocSh->GetDoc();
                if (pDoc)
                {
                    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
                    const sal_Int32 nPos = pCursor->GetPoint()->nContent.GetIndex();

                    SfxItemSet aSetWeight(pDoc->GetAttrPool(),
                                          svl::Items<RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT>{});
                    pCursor->GetNode().GetTextNode()->GetParaAttr(aSetWeight, nPos, nPos);
                    SvxWeightItem const* sWeightProp = aSetWeight.GetItem(RES_CHRATR_WEIGHT);
                    OUString fontweight
                        = "FONT WEIGHT        " + getFontWeight(sWeightProp->GetWeight());

                    SfxItemSet aSetHeight(pDoc->GetAttrPool(),
                                          svl::Items<RES_CHRATR_FONTSIZE, RES_CHRATR_FONTSIZE>{});
                    pCursor->GetNode().GetTextNode()->GetParaAttr(aSetHeight, nPos, nPos);
                    SvxFontHeightItem const* sHeightProp = aSetHeight.GetItem(RES_CHRATR_FONTSIZE);
                    float height = sHeightProp->GetHeight() / 20;
                    OUString fontheight = "FONT HEIGHT        " + OUString::number(height);

                    SfxItemSet aSetFont(pDoc->GetAttrPool(),
                                        svl::Items<RES_CHRATR_FONT, RES_CHRATR_FONT>{});
                    pCursor->GetNode().GetTextNode()->GetParaAttr(aSetFont, nPos, nPos);
                    SvxFontItem const* sFontProp = aSetFont.GetItem(RES_CHRATR_FONT);
                    OUString fontname = "FONT NAME            " + sFontProp->GetFamilyName();

                    store.push_back(fontname);
                    store.push_back(fontheight);
                    store.push_back(fontweight);
                }
            }
        }
        break;
    }
    InspectorTextPanel::updateEntries(store);
}

OUString WriterInspectorTextPanel::getFontWeight(FontWeight f)
{
    switch (f)
    {
        case WEIGHT_BOLD:
            return "bold";
        case WEIGHT_NORMAL:
            return "normal";
        default: /* Will add more cases later*/
            return "none";
    }
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
