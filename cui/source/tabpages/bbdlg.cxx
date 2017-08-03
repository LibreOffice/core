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

#include "bbdlg.hxx"
#include "border.hxx"
#include "backgrnd.hxx"
#include "svx/svxids.hrc"
#include "cuitabarea.hxx"

SvxBorderBackgroundDlg::SvxBorderBackgroundDlg(vcl::Window *pParent,
    const SfxItemSet& rCoreSet,
    bool bEnableSelector,
    bool bEnableDrawingLayerFillStyles)
    : SfxTabDialog(pParent,
        bEnableDrawingLayerFillStyles
            ? OUString("BorderAreaTransparencyDialog")
            : OUString("BorderBackgroundDialog"),
        bEnableDrawingLayerFillStyles
            ? OUString("cui/ui/borderareatransparencydialog.ui")
            : OUString("cui/ui/borderbackgrounddialog.ui"),
        &rCoreSet)
    , mbEnableBackgroundSelector(bEnableSelector)
    , mbEnableDrawingLayerFillStyles(bEnableDrawingLayerFillStyles)
    , m_nBackgroundPageId(0)
    , m_nAreaPageId(0)
    , m_nTransparencePageId(0)
{
    AddTabPage("borders", SvxBorderTabPage::Create, nullptr );
    if (mbEnableDrawingLayerFillStyles)
    {
        // Here we want full DrawingLayer FillStyle access, so add Area and Transparency TabPages
        m_nAreaPageId = AddTabPage("area", SvxAreaTabPage::Create, nullptr);
        m_nTransparencePageId = AddTabPage("transparence", SvxTransparenceTabPage::Create, nullptr);
    }
    else
    {
//TODO        m_nBackgroundPageId = AddTabPage("background", SvxBackgroundTabPage::Create, nullptr );
    }
}

void SvxBorderBackgroundDlg::PageCreated( sal_uInt16 nPageId, SfxTabPage& rTabPage )
{
    if (nPageId == m_nBackgroundPageId)
    {
        // allow switching between Color/graphic
        if(mbEnableBackgroundSelector)
        {
//TODO            static_cast< SvxBackgroundTabPage& >(rTabPage).ShowSelector();
        }
    }
    // inits for Area and Transparency TabPages
    // The selection attribute lists (XPropertyList derivates, e.g. XColorList for
    // the color table) need to be added as items (e.g. SvxColorTableItem) to make
    // these pages find the needed attributes for fill style suggestions.
    // These are added in SwDocStyleSheet::GetItemSet() for the SfxStyleFamily::Para on
    // demand, but could also be directly added from the DrawModel.
    else if (nPageId == m_nAreaPageId)
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
    else if (nPageId == m_nTransparencePageId)
    {
        rTabPage.PageCreated(*GetInputSetImpl());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
