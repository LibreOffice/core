/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_CUSTOMWELD_HXX
#define INCLUDED_VCL_CUSTOMWELD_HXX

#include <vcl/weld.hxx>

namespace weld
{
class VCL_DLLPUBLIC CustomWidgetController
{
private:
    Size m_aSize;
    weld::DrawingArea* m_pDrawingArea;

public:
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) = 0;
    virtual void Resize() {}
    virtual void MouseButtonDown(const MouseEvent&) {}
    virtual void MouseMove(const MouseEvent&) {}
    virtual void MouseButtonUp(const MouseEvent&) {}
    virtual void GetFocus() {}
    virtual void LoseFocus() {}
    virtual void StyleUpdated() { Invalidate(); }
    virtual bool KeyInput(const KeyEvent&) { return false; }
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible()
    {
        return css::uno::Reference<css::accessibility::XAccessible>();
    }
    virtual tools::Rectangle GetFocusRect() { return tools::Rectangle(); }
    virtual FactoryFunction GetUITestFactory() const { return nullptr; }
    Size const& GetOutputSizePixel() const { return m_aSize; }
    void SetOutputSizePixel(const Size& rSize) { m_aSize = rSize; }
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) { m_pDrawingArea = pDrawingArea; }
    weld::DrawingArea* GetDrawingArea() const { return m_pDrawingArea; }
    void Invalidate() { m_pDrawingArea->queue_draw(); }
    void Invalidate(const tools::Rectangle& rRect)
    {
        m_pDrawingArea->queue_draw_area(rRect.Left(), rRect.Top(), rRect.GetWidth(),
                                        rRect.GetHeight());
    }
    void Show() { m_pDrawingArea->show(); }
    void Hide() { m_pDrawingArea->hide(); }
    void GrabFocus() { m_pDrawingArea->grab_focus(); }
    bool HasFocus() const { return m_pDrawingArea->has_focus(); }
    bool IsVisible() const { return m_pDrawingArea->get_visible(); }
    bool IsEnabled() const { return m_pDrawingArea->get_sensitive(); }
    void grab_add() { m_pDrawingArea->grab_add(); }
    void grab_remove() { m_pDrawingArea->grab_remove(); }
    void set_size_request(int nWidth, int nHeight)
    {
        m_pDrawingArea->set_size_request(nWidth, nHeight);
    }
    virtual ~CustomWidgetController();
};

class VCL_DLLPUBLIC CustomWeld
{
private:
    weld::CustomWidgetController& m_rWidgetController;
    std::unique_ptr<weld::DrawingArea> m_xDrawingArea;

    DECL_LINK(DoResize, const Size& rSize, void);
    DECL_LINK(DoPaint, weld::DrawingArea::draw_args, void);
    DECL_LINK(DoMouseButtonDown, const MouseEvent&, void);
    DECL_LINK(DoMouseMove, const MouseEvent&, void);
    DECL_LINK(DoMouseButtonUp, const MouseEvent&, void);
    DECL_LINK(DoGetFocus, weld::Widget&, void);
    DECL_LINK(DoLoseFocus, weld::Widget&, void);
    DECL_LINK(DoKeyPress, const KeyEvent&, bool);
    DECL_LINK(DoFocusRect, weld::Widget&, tools::Rectangle);
    DECL_LINK(DoStyleUpdated, weld::Widget&, void);

public:
    CustomWeld(weld::Builder& rBuilder, const OString& rDrawingId,
               CustomWidgetController& rWidgetController);
    void queue_draw() { m_xDrawingArea->queue_draw(); }
    void queue_draw_area(int x, int y, int width, int height)
    {
        m_xDrawingArea->queue_draw_area(x, y, width, height);
    }
    void set_size_request(int nWidth, int nHeight)
    {
        m_xDrawingArea->set_size_request(nWidth, nHeight);
    }
    void show() { m_xDrawingArea->show(); }
    void set_margin_top(int nMargin) { m_xDrawingArea->set_margin_top(nMargin); }
    void set_margin_bottom(int nMargin) { m_xDrawingArea->set_margin_bottom(nMargin); }
};
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
