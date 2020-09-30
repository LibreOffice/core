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

class InputContext;

namespace weld
{
class VCL_DLLPUBLIC CustomWidgetController
{
private:
    Size m_aSize;
    weld::DrawingArea* m_pDrawingArea;
    DECL_LINK(DragBeginHdl, weld::DrawingArea&, bool);

public:
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible()
    {
        return css::uno::Reference<css::accessibility::XAccessible>();
    }
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) = 0;
    virtual void Resize() {}
    virtual bool MouseButtonDown(const MouseEvent&) { return false; }
    virtual bool MouseMove(const MouseEvent&) { return false; }
    virtual bool MouseButtonUp(const MouseEvent&) { return false; }
    virtual void GetFocus() {}
    virtual void LoseFocus() {}
    virtual void StyleUpdated() { Invalidate(); }
    virtual bool Command(const CommandEvent&) { return false; }
    virtual bool KeyInput(const KeyEvent&) { return false; }
    virtual tools::Rectangle GetFocusRect() { return tools::Rectangle(); }
    virtual FactoryFunction GetUITestFactory() const { return nullptr; }
    virtual OUString RequestHelp(tools::Rectangle&) { return OUString(); }
    virtual OUString GetHelpText() const { return m_pDrawingArea->get_tooltip_text(); }
    Size const& GetOutputSizePixel() const { return m_aSize; }
    void SetOutputSizePixel(const Size& rSize) { m_aSize = rSize; }
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) { m_pDrawingArea = pDrawingArea; }
    weld::DrawingArea* GetDrawingArea() const { return m_pDrawingArea; }
    void Invalidate()
    {
        if (!m_pDrawingArea)
            return;
        m_pDrawingArea->queue_draw();
    }
    static bool IsUpdateMode() { return true; }
    void Invalidate(const tools::Rectangle& rRect)
    {
        if (!m_pDrawingArea)
            return;
        m_pDrawingArea->queue_draw_area(rRect.Left(), rRect.Top(), rRect.GetWidth(),
                                        rRect.GetHeight());
    }
    virtual void Show() { m_pDrawingArea->show(); }
    virtual void Hide() { m_pDrawingArea->hide(); }
    void GrabFocus() { m_pDrawingArea->grab_focus(); }
    bool HasFocus() const { return m_pDrawingArea->has_focus(); }
    bool IsVisible() const { return m_pDrawingArea->get_visible(); }
    bool IsReallyVisible() const { return m_pDrawingArea->is_visible(); }
    bool IsEnabled() const { return m_pDrawingArea->get_sensitive(); }
    void Enable() const { m_pDrawingArea->set_sensitive(true); }
    void Disable() const { m_pDrawingArea->set_sensitive(false); }
    bool IsActive() const { return m_pDrawingArea->is_active(); }
    int GetTextHeight() const { return m_pDrawingArea->get_text_height(); }
    int GetTextWidth(const OUString& rText) const
    {
        return m_pDrawingArea->get_pixel_size(rText).Width();
    }
    OUString GetAccessibleName() const { return m_pDrawingArea->get_accessible_name(); }
    OUString GetAccessibleDescription() const
    {
        return m_pDrawingArea->get_accessible_description();
    }
    void CaptureMouse() { m_pDrawingArea->grab_add(); }
    bool IsMouseCaptured() const { return m_pDrawingArea->has_grab(); }
    void EnableRTL(bool bEnable) { m_pDrawingArea->set_direction(bEnable); }
    bool IsRTLEnabled() const { return m_pDrawingArea->get_direction(); }
    void ReleaseMouse() { m_pDrawingArea->grab_remove(); }
    void SetPointer(PointerStyle ePointerStyle) { m_pDrawingArea->set_cursor(ePointerStyle); }
    void SetHelpId(const OString& rHelpId) { m_pDrawingArea->set_help_id(rHelpId); }
    void SetAccessibleName(const OUString& rName) { m_pDrawingArea->set_accessible_name(rName); }
    void SetInputContext(const InputContext& rInputContext)
    {
        m_pDrawingArea->set_input_context(rInputContext);
    }
    void SetDragDataTransferrable(rtl::Reference<TransferDataContainer>& rTransferrable,
                                  sal_uInt8 eDNDConstants)
    {
        m_pDrawingArea->enable_drag_source(rTransferrable, eDNDConstants);
        m_pDrawingArea->connect_drag_begin(LINK(this, CustomWidgetController, DragBeginHdl));
    }
    // return true to disallow drag, false to allow
    virtual bool StartDrag() { return false; }
    void set_size_request(int nWidth, int nHeight)
    {
        m_pDrawingArea->set_size_request(nWidth, nHeight);
    }
    void queue_resize()
    {
        if (!m_pDrawingArea)
            return;
        m_pDrawingArea->queue_resize();
    }
    CustomWidgetController()
        : m_pDrawingArea(nullptr)
    {
    }
    virtual ~CustomWidgetController();

    CustomWidgetController(CustomWidgetController const&) = default;
    CustomWidgetController(CustomWidgetController&&) = default;
    CustomWidgetController& operator=(CustomWidgetController const&) = default;
    CustomWidgetController& operator=(CustomWidgetController&&) = default;
};

class VCL_DLLPUBLIC CustomWeld
{
private:
    weld::CustomWidgetController& m_rWidgetController;
    std::unique_ptr<weld::DrawingArea> m_xDrawingArea;

    DECL_LINK(DoResize, const Size& rSize, void);
    DECL_LINK(DoPaint, weld::DrawingArea::draw_args, void);
    DECL_LINK(DoMouseButtonDown, const MouseEvent&, bool);
    DECL_LINK(DoMouseMove, const MouseEvent&, bool);
    DECL_LINK(DoMouseButtonUp, const MouseEvent&, bool);
    DECL_LINK(DoGetFocus, weld::Widget&, void);
    DECL_LINK(DoLoseFocus, weld::Widget&, void);
    DECL_LINK(DoKeyPress, const KeyEvent&, bool);
    DECL_LINK(DoFocusRect, weld::Widget&, tools::Rectangle);
    DECL_LINK(DoCommand, const CommandEvent&, bool);
    DECL_LINK(DoStyleUpdated, weld::Widget&, void);
    DECL_LINK(DoRequestHelp, tools::Rectangle&, OUString);

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
    void hide() { m_xDrawingArea->hide(); }
    void set_margin_top(int nMargin) { m_xDrawingArea->set_margin_top(nMargin); }
    void set_margin_bottom(int nMargin) { m_xDrawingArea->set_margin_bottom(nMargin); }
    void set_sensitive(bool bSensitive) { m_xDrawingArea->set_sensitive(bSensitive); }
    bool get_sensitive() const { return m_xDrawingArea->get_sensitive(); }
    bool get_visible() const { return m_xDrawingArea->get_visible(); }
    void set_visible(bool bVisible) { m_xDrawingArea->set_visible(bVisible); }
    void set_grid_left_attach(int nAttach) { m_xDrawingArea->set_grid_left_attach(nAttach); }
    int get_grid_left_attach() const { return m_xDrawingArea->get_grid_left_attach(); }
    void set_help_id(const OString& rHelpId) { m_xDrawingArea->set_help_id(rHelpId); }
    void set_tooltip_text(const OUString& rTip) { m_xDrawingArea->set_tooltip_text(rTip); }
};
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
