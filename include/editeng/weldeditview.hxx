/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_EDITENG_WELDEDITVIEW_HXX
#define INCLUDED_EDITENG_WELDEDITVIEW_HXX

#include <sal/config.h>
#include <editeng/editengdllapi.h>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <vcl/customweld.hxx>
#include <vcl/outdev.hxx>

class EDITENG_DLLPUBLIC WeldEditView : public weld::CustomWidgetController, public EditViewCallbacks
{
public:
    WeldEditView();
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual ~WeldEditView() override;

    std::unique_ptr<EditEngine> m_xEdEngine;
    std::unique_ptr<EditView> m_xEdView;

protected:
    virtual Size GetPreferredSize() const;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual bool MouseMove(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual bool KeyInput(const KeyEvent& rKEvt) override;
    virtual void GetFocus() override;
    virtual void Resize() override;

    virtual void EditViewInvalidate(const tools::Rectangle& rRect) const override
    {
        weld::DrawingArea* pDrawingArea = GetDrawingArea();
        pDrawingArea->queue_draw_area(rRect.Left(), rRect.Top(), rRect.GetWidth(),
                                      rRect.GetHeight());
    }

    virtual void EditViewSelectionChange() const override
    {
        weld::DrawingArea* pDrawingArea = GetDrawingArea();
        pDrawingArea->queue_draw();
    }

    virtual OutputDevice& EditViewOutputDevice() const override
    {
        return GetDrawingArea()->get_ref_device();
    }
};

#endif // INCLUDED_EDITENG_WELDEDITVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
