/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceWidget.hxx"

#include <QtWidgets/QLabel>

class QtInstanceDrawingArea : public QtInstanceWidget, public virtual weld::DrawingArea
{
    Q_OBJECT

    QLabel* m_pLabel;
    ScopedVclPtrInstance<VirtualDevice> m_xDevice;

public:
    QtInstanceDrawingArea(QLabel* pLabel);

    virtual void queue_draw() override;
    virtual void queue_draw_area(int x, int y, int width, int height) override;

    virtual void enable_drag_source(rtl::Reference<TransferDataContainer>& rTransferable,
                                    sal_uInt8 eDNDConstants) override;

    virtual void set_cursor(PointerStyle ePointerStyle) override;

    virtual Point get_pointer_position() const override;

    virtual void set_input_context(const InputContext& rInputContext) override;
    virtual void im_context_set_cursor_location(const tools::Rectangle& rCursorRect,
                                                int nExtTextInputWidth) override;

    virtual OutputDevice& get_ref_device() override;

    virtual a11yref get_accessible_parent() override;
    virtual a11yrelationset get_accessible_relation_set() override;
    virtual AbsoluteScreenPixelPoint get_accessible_location_on_screen() override;

    virtual bool eventFilter(QObject* pObject, QEvent* pEvent) override;

private:
    void handlePaintEvent();
    void handleResizeEvent();
    bool handleToolTipEvent(QHelpEvent& rEvent);

private:
    virtual void click(const Point&) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
