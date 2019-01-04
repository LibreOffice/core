/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_UIBASE_DOCVW_FLOATINGTABLEBUTTON_HXX
#define INCLUDED_SW_SOURCE_UIBASE_DOCVW_FLOATINGTABLEBUTTON_HXX

#include "FrameControl.hxx"
#include <vcl/menubtn.hxx>

class FloatingTableButton : public SwFrameMenuButtonBase
{
    OUString m_sLabel;

public:
    FloatingTableButton(SwEditWin* pEditWin, const SwFrame* pFrame);
    virtual ~FloatingTableButton() override;

    void SetOffset(Point aBottomRightPixel);

    virtual void MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

    virtual void ShowAll(bool bShow) override;
    virtual bool Contains(const Point& rDocPt) const override;

    virtual void SetReadonly(bool bReadonly) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */