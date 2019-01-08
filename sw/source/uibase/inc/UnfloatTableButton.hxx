/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_UIBASE_DOCVW_UNFLOATTABLEBUTTON_HXX
#define INCLUDED_SW_SOURCE_UIBASE_DOCVW_UNFLOATTABLEBUTTON_HXX

#include "FrameControl.hxx"
#include <vcl/menubtn.hxx>

/** Class for unfloat table button
 *
 * This unfloat button is used to convert a floating table into a simple writer table embedded to the text body.
 * This unfloat operation is useful typically for documents imported from MSO file formats containing
 * multi-page floating tables. In case of a multi-page table the text frame cuts off the table because
 * the frame can't span across multiple pages. With unfloating we can get a multi-page table without
 * floating properties.
 *
 */
class UnfloatTableButton : public SwFrameMenuButtonBase
{
    OUString m_sLabel;

public:
    UnfloatTableButton(SwEditWin* pEditWin, const SwFrame* pFrame);
    virtual ~UnfloatTableButton() override;

    void SetOffset(Point aBottomRightPixel);

    virtual void MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

    virtual void ShowAll(bool bShow) override;
    virtual bool Contains(const Point& rDocPt) const override;

    virtual void SetReadonly(bool bReadonly) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
