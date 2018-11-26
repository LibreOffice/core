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

#include <bbdlg.hxx>
#include <border.hxx>
#include <backgrnd.hxx>
#include <svx/svxids.hrc>
#include <cuitabarea.hxx>

SvxBorderBackgroundDlg::SvxBorderBackgroundDlg(weld::Window *pParent,
    const SfxItemSet& rCoreSet,
    bool bEnableSelector,
    bool bEnableDrawingLayerFillStyles)
    : SfxTabDialogController(pParent,
        bEnableDrawingLayerFillStyles
            ? OUString("cui/ui/borderareatransparencydialog.ui")
            : OUString("cui/ui/borderbackgrounddialog.ui"),
        bEnableDrawingLayerFillStyles
            ? OString("BorderAreaTransparencyDialog")
            : OString("BorderBackgroundDialog"),
        &rCoreSet)
    , mbEnableBackgroundSelector(bEnableSelector)
{
    AddTabPage("borders", SvxBorderTabPage::Create, nullptr );
    if (bEnableDrawingLayerFillStyles)
    {
        // Here we want full DrawingLayer FillStyle access, so add Area and Transparency TabPages
        AddTabPage("area", SvxAreaTabPage::Create, nullptr);
        AddTabPage("transparence", SvxTransparenceTabPage::Create, nullptr);
    }
    else
    {
        AddTabPage("background", SvxBkgTabPage::Create, nullptr );
    }
}

void SvxBorderBackgroundDlg::PageCreated(const OString& rPageId, SfxTabPage& rTabPage)
{
    if (rPageId == "background")
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        // allow switching between Color/graphic
        if (mbEnableBackgroundSelector)
            aSet.Put(SfxUInt32Item(SID_FLAG_TYPE, static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_SELECTOR)));
        rTabPage.PageCreated(aSet);
    }
    // inits for Area and Transparency TabPages
    // The selection attribute lists (XPropertyList derivates, e.g. XColorList for
    // the color table) need to be added as items (e.g. SvxColorTableItem) to make
    // these pages find the needed attributes for fill style suggestions.
    // These are added in SwDocStyleSheet::GetItemSet() for the SfxStyleFamily::Para on
    // demand, but could also be directly added from the DrawModel.
    else if (rPageId == "area")
    {
        SfxItemSet aNew(
            *GetInputSetImpl()->GetPool(),
            svl::Items<SID_COLOR_TABLE, SID_PATTERN_LIST,
            SID_OFFER_IMPORT, SID_OFFER_IMPORT>{});

        aNew.Put(*GetInputSetImpl());

        // add flag for direct graphic content selection
        aNew.Put(SfxBoolItem(SID_OFFER_IMPORT, true));

        rTabPage.PageCreated(aNew);
    }
    else if (rPageId == "transparence")
    {
        rTabPage.PageCreated(*GetInputSetImpl());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
