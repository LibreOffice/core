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
#include "TextUnderlineControl.hxx"
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/svxids.hrc>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <editeng/editids.hrc>
#include <editeng/udlnitem.hxx>
#include <vcl/settings.hxx>
#include "helpids.h"

namespace svx {

TextUnderlineControl::TextUnderlineControl(sal_uInt16 nId, vcl::Window* pParent)
:   SfxPopupWindow(nId, pParent, "TextUnderlineControl", "svx/ui/textunderlinecontrol.ui")
{
    get(maNone, "none");
    get(maSingle, "single");
    get(maDouble, "double");
    get(maBold, "bold");
    get(maDot, "dot");
    get(maDotBold, "dotbold");
    get(maDash, "dash");
    get(maDashLong, "dashlong");
    get(maDashDot, "dashdot");
    get(maDashDotDot, "dashdotdot");
    get(maWave, "wave");
    get(maMoreOptions, "moreoptions");

    maMoreOptions->SetHelpId(HID_UNDERLINE_BTN);

    Link<Button*,void> aLink = LINK(this, TextUnderlineControl, PBClickHdl);
    maNone->SetClickHdl(aLink);
    maSingle->SetClickHdl(aLink);
    maDouble->SetClickHdl(aLink);
    maBold->SetClickHdl(aLink);
    maDot->SetClickHdl(aLink);
    maDotBold->SetClickHdl(aLink);
    maDash->SetClickHdl(aLink);
    maDashLong->SetClickHdl(aLink);
    maDashDot->SetClickHdl(aLink);
    maDashDotDot->SetClickHdl(aLink);
    maWave->SetClickHdl(aLink);
    maMoreOptions->SetClickHdl(aLink);
}

TextUnderlineControl::~TextUnderlineControl()
{
    disposeOnce();
}

void TextUnderlineControl::dispose()
{
    maNone.clear();
    maSingle.clear();
    maDouble.clear();
    maBold.clear();
    maDot.clear();
    maDotBold.clear();
    maDash.clear();
    maDashLong.clear();
    maDashDot.clear();
    maDashDotDot.clear();
    maWave.clear();
    maMoreOptions.clear();

    SfxPopupWindow::dispose();
}

FontLineStyle TextUnderlineControl::getLineStyle(Button const * pButton)
{
    if(pButton == maSingle)
        return LINESTYLE_SINGLE;
    else if(pButton == maDouble)
        return LINESTYLE_DOUBLE;
    else if(pButton == maBold)
        return LINESTYLE_BOLD;
    else if(pButton == maDot)
        return LINESTYLE_DOTTED;
    else if(pButton == maDotBold)
        return LINESTYLE_BOLDDOTTED;
    else if(pButton == maDash)
        return LINESTYLE_DASH;
    else if(pButton == maDashLong)
        return LINESTYLE_LONGDASH;
    else if(pButton == maDashDot)
        return LINESTYLE_DASHDOT;
    else if(pButton == maDashDotDot)
        return LINESTYLE_DASHDOTDOT;
    else if(pButton == maWave)
        return LINESTYLE_WAVE;

    return LINESTYLE_NONE;
}

namespace {

Color GetUnderlineColor()
{
    const SfxPoolItem* pItem;
    SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState(SID_ATTR_CHAR_UNDERLINE, pItem);

    const SvxUnderlineItem* pUnderlineItem = static_cast<const SvxUnderlineItem*>(pItem);

    if(pUnderlineItem)
        return pUnderlineItem->GetColor();

    return COL_AUTO;
}

}

IMPL_LINK(TextUnderlineControl, PBClickHdl, Button*, pButton, void)
{
    if(pButton == maMoreOptions)
    {
        SfxDispatcher* pDisp = SfxViewFrame::Current()->GetBindings().GetDispatcher();
        pDisp->Execute(SID_CHAR_DLG_EFFECT, SfxCallMode::ASYNCHRON);

        EndPopupMode();
    }
    else
    {
        const FontLineStyle eUnderline = getLineStyle(pButton);

        SvxUnderlineItem aLineItem(eUnderline, SID_ATTR_CHAR_UNDERLINE);
        aLineItem.SetColor(GetUnderlineColor());

        SfxViewFrame::Current()->GetBindings().GetDispatcher()->ExecuteList(SID_ATTR_CHAR_UNDERLINE,
               SfxCallMode::RECORD, { &aLineItem });

        EndPopupMode();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
