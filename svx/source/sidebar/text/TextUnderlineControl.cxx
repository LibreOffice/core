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
#include <svx/svxids.hrc>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <TextUnderlinePopup.hxx>
#include <editeng/editids.hrc>
#include <editeng/udlnitem.hxx>
#include <svl/itemset.hxx>
#include <helpids.h>

namespace svx {

TextUnderlineControl::TextUnderlineControl(TextUnderlinePopup* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, u"svx/ui/textunderlinecontrol.ui"_ustr, u"TextUnderlineControl"_ustr)
    , mxNone(m_xBuilder->weld_button(u"none"_ustr))
    , mxSingle(m_xBuilder->weld_button(u"single"_ustr))
    , mxDouble(m_xBuilder->weld_button(u"double"_ustr))
    , mxBold(m_xBuilder->weld_button(u"bold"_ustr))
    , mxDot(m_xBuilder->weld_button(u"dot"_ustr))
    , mxDotBold(m_xBuilder->weld_button(u"dotbold"_ustr))
    , mxDash(m_xBuilder->weld_button(u"dash"_ustr))
    , mxDashLong(m_xBuilder->weld_button(u"dashlong"_ustr))
    , mxDashDot(m_xBuilder->weld_button(u"dashdot"_ustr))
    , mxDashDotDot(m_xBuilder->weld_button(u"dashdotdot"_ustr))
    , mxWave(m_xBuilder->weld_button(u"wave"_ustr))
    , mxMoreOptions(m_xBuilder->weld_button(u"moreoptions"_ustr))
    , mxControl(pControl)
{
    mxMoreOptions->set_help_id(HID_UNDERLINE_BTN);

    Link<weld::Button&,void> aLink = LINK(this, TextUnderlineControl, PBClickHdl);
    mxNone->connect_clicked(aLink);
    mxSingle->connect_clicked(aLink);
    mxDouble->connect_clicked(aLink);
    mxBold->connect_clicked(aLink);
    mxDot->connect_clicked(aLink);
    mxDotBold->connect_clicked(aLink);
    mxDash->connect_clicked(aLink);
    mxDashLong->connect_clicked(aLink);
    mxDashDot->connect_clicked(aLink);
    mxDashDotDot->connect_clicked(aLink);
    mxWave->connect_clicked(aLink);
    mxMoreOptions->connect_clicked(aLink);
}

void TextUnderlineControl::GrabFocus()
{
    mxNone->grab_focus();
}

TextUnderlineControl::~TextUnderlineControl()
{
}

FontLineStyle TextUnderlineControl::getLineStyle(const weld::Button& rButton) const
{
    if (&rButton == mxSingle.get())
        return LINESTYLE_SINGLE;
    else if (&rButton == mxDouble.get())
        return LINESTYLE_DOUBLE;
    else if (&rButton == mxBold.get())
        return LINESTYLE_BOLD;
    else if (&rButton == mxDot.get())
        return LINESTYLE_DOTTED;
    else if (&rButton == mxDotBold.get())
        return LINESTYLE_BOLDDOTTED;
    else if (&rButton == mxDash.get())
        return LINESTYLE_DASH;
    else if (&rButton == mxDashLong.get())
        return LINESTYLE_LONGDASH;
    else if (&rButton == mxDashDot.get())
        return LINESTYLE_DASHDOT;
    else if (&rButton == mxDashDotDot.get())
        return LINESTYLE_DASHDOTDOT;
    else if (&rButton == mxWave.get())
        return LINESTYLE_WAVE;

    return LINESTYLE_NONE;
}

namespace {

Color GetUnderlineColor()
{
    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
    {
        SfxPoolItemHolder aResult;
        pViewFrm->GetBindings().GetDispatcher()->QueryState(SID_ATTR_CHAR_UNDERLINE, aResult);
        const SvxUnderlineItem* pUnderlineItem(static_cast<const SvxUnderlineItem*>(aResult.getItem()));

        if (pUnderlineItem)
            return pUnderlineItem->GetColor();
    }

    return COL_AUTO;
}

}

IMPL_LINK(TextUnderlineControl, PBClickHdl, weld::Button&, rButton, void)
{
    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
    {
        if (&rButton == mxMoreOptions.get())
        {
            SfxDispatcher* pDisp = pViewFrm->GetBindings().GetDispatcher();
            pDisp->Execute(SID_CHAR_DLG_EFFECT, SfxCallMode::ASYNCHRON);
        }
        else
        {
            const FontLineStyle eUnderline = getLineStyle(rButton);

            SvxUnderlineItem aLineItem(eUnderline, SID_ATTR_CHAR_UNDERLINE);
            aLineItem.SetColor(GetUnderlineColor());

            pViewFrm->GetBindings().GetDispatcher()->ExecuteList(SID_ATTR_CHAR_UNDERLINE,
                   SfxCallMode::RECORD, { &aLineItem });
        }
    }
    mxControl->EndPopupMode();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
