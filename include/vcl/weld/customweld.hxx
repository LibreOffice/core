/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <comphelper/OAccessible.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <vcl/weld/DrawingArea.hxx>
#include <rtl/ref.hxx>
#include <vcl/uitest/factory.hxx>
#include <vcl/weld/weld.hxx>

class InputContext;

namespace weld
{
class Builder;

// Reasons for invalidation, accumulated via OR between paints
enum class InvalidateFlags : sal_uInt8
{
    NONE = 0x00,
    Cursor = 0x01, // Cursor blink only
    All = 0xff // Default
};
}

namespace o3tl
{
template <> struct typed_flags<weld::InvalidateFlags> : is_typed_flags<weld::InvalidateFlags, 0xff>
{
};
}

namespace weld
{
class VCL_DLLPUBLIC CustomWidgetController
{
private:
    Size m_aSize;
    weld::DrawingArea* m_pDrawingArea;
    InvalidateFlags m_eInvalidateFlags;
    DECL_LINK(DragBeginHdl, weld::DrawingArea&, bool);

public:
    virtual rtl::Reference<comphelper::OAccessible> CreateAccessible() { return {}; }
    // rRect is in Logical units rather than Pixels
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) = 0;
    virtual void Resize() { Invalidate(); }
    virtual bool MouseButtonDown(const MouseEvent&) { return false; }
    virtual bool MouseMove(const MouseEvent&) { return false; }
    virtual bool MouseButtonUp(const MouseEvent&) { return false; }
    virtual void GetFocus() {}
    virtual void LoseFocus() {}
    virtual void StyleUpdated() { Invalidate(); }
    virtual bool Command(const CommandEvent&) { return false; }
    virtual bool KeyInput(const KeyEvent&) { return false; }
    virtual bool KeyUp(const KeyEvent&) { return false; }
    virtual tools::Rectangle GetFocusRect() { return tools::Rectangle(); }
    virtual FactoryFunction GetUITestFactory() const { return nullptr; }
    virtual OUString RequestHelp(tools::Rectangle&) { return OUString(); }
    OUString GetHelpText() const { return m_pDrawingArea->get_tooltip_text(); }
    Size const& GetOutputSizePixel() const { return m_aSize; }
    void SetOutputSizePixel(const Size& rSize) { m_aSize = rSize; }
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) { m_pDrawingArea = pDrawingArea; }
    weld::DrawingArea* GetDrawingArea() const { return m_pDrawingArea; }
    void Invalidate(InvalidateFlags eFlags = InvalidateFlags::All)
    {
        m_eInvalidateFlags |= eFlags;
        if (!m_pDrawingArea)
            return;
        m_pDrawingArea->queue_draw();
    }
    void Invalidate(const tools::Rectangle& rRect, InvalidateFlags eFlags = InvalidateFlags::All)
    {
        m_eInvalidateFlags |= eFlags;
        if (!m_pDrawingArea)
            return;
        m_pDrawingArea->queue_draw_area(rRect.Left(), rRect.Top(), rRect.GetWidth(),
                                        rRect.GetHeight());
    }
    InvalidateFlags GetInvalidateFlags() const { return m_eInvalidateFlags; }
    void ClearInvalidateFlags() { m_eInvalidateFlags = InvalidateFlags::NONE; }
    virtual void Show() { m_pDrawingArea->show(); }
    virtual void Hide() { m_pDrawingArea->hide(); }
    void SetCursor(void* pData) { m_pDrawingArea->set_cursor_data(pData); }
    void GrabFocus() { m_pDrawingArea->grab_focus(); }
    bool HasFocus() const { return m_pDrawingArea->has_focus(); }
    bool HasChildFocus() const { return m_pDrawingArea->has_child_focus(); }
    bool IsVisible() const { return m_pDrawingArea->get_visible(); }
    bool IsReallyVisible() const { return m_pDrawingArea->is_visible(); }
    bool IsEnabled() const { return m_pDrawingArea->get_sensitive(); }
    void Enable() const { m_pDrawingArea->set_sensitive(true); }
    void Disable() const { m_pDrawingArea->set_sensitive(false); }
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
    OUString GetAccessibleId() const
    {
        if (m_pDrawingArea)
            return m_pDrawingArea->get_accessible_id();
        return OUString();
    }
    void CaptureMouse() { m_pDrawingArea->grab_mouse(); }
    bool IsMouseCaptured() const { return m_pDrawingArea->has_mouse_grab(); }
    Point GetPointerPosPixel() const { return m_pDrawingArea->get_pointer_position(); }
    void EnableRTL(bool bEnable) { m_pDrawingArea->set_direction(bEnable); }
    bool IsRTLEnabled() const { return m_pDrawingArea->get_direction(); }
    void ReleaseMouse() { m_pDrawingArea->release_mouse(); }
    void SetPointer(PointerStyle ePointerStyle) { m_pDrawingArea->set_cursor(ePointerStyle); }
    void SetHelpId(const OUString& rHelpId) { m_pDrawingArea->set_help_id(rHelpId); }
    void SetAccessibleName(const OUString& rName) { m_pDrawingArea->set_accessible_name(rName); }
    void SetInputContext(const InputContext& rInputContext)
    {
        m_pDrawingArea->set_input_context(rInputContext);
    }
    void SetCursorRect(const tools::Rectangle& rCursorRect, int nExtTextInputWidth)
    {
        m_pDrawingArea->im_context_set_cursor_location(rCursorRect, nExtTextInputWidth);
    }
    virtual int GetSurroundingText(OUString& /*rSurrounding*/) { return -1; }
    virtual bool DeleteSurroundingText(const Selection& /*rRange*/) { return false; }
    css::uno::Reference<css::datatransfer::dnd::XDropTarget> GetDropTarget()
    {
        return m_pDrawingArea->get_drop_target();
    }
    css::uno::Reference<css::datatransfer::clipboard::XClipboard> GetClipboard() const
    {
        return m_pDrawingArea->get_clipboard();
    }
    void SetDragDataTransferable(rtl::Reference<TransferDataContainer>& rTransferable,
                                 sal_uInt8 eDNDConstants)
    {
        m_pDrawingArea->enable_drag_source(rTransferable, eDNDConstants);
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
        , m_eInvalidateFlags(InvalidateFlags::NONE)
    {
    }
    virtual ~CustomWidgetController();

    CustomWidgetController(CustomWidgetController const&) = default;
    CustomWidgetController(CustomWidgetController&&) = default;
    CustomWidgetController& operator=(CustomWidgetController const&) = default;
    CustomWidgetController& operator=(CustomWidgetController&&) = default;
};

class VCL_DLLPUBLIC CustomWeld final
{
private:
    weld::CustomWidgetController& m_rWidgetController;
    std::unique_ptr<weld::DrawingArea> m_xDrawingArea;

    DECL_DLLPRIVATE_LINK(DoResize, const Size& rSize, void);
    DECL_DLLPRIVATE_LINK(DoPaint, weld::DrawingArea::draw_args, void);
    DECL_DLLPRIVATE_LINK(DoMouseButtonDown, const MouseEvent&, bool);
    DECL_DLLPRIVATE_LINK(DoMouseMove, const MouseEvent&, bool);
    DECL_DLLPRIVATE_LINK(DoMouseButtonUp, const MouseEvent&, bool);
    DECL_DLLPRIVATE_LINK(DoGetFocus, weld::Widget&, void);
    DECL_DLLPRIVATE_LINK(DoLoseFocus, weld::Widget&, void);
    DECL_DLLPRIVATE_LINK(DoKeyPress, const KeyEvent&, bool);
    DECL_DLLPRIVATE_LINK(DoKeyRelease, const KeyEvent&, bool);
    DECL_DLLPRIVATE_LINK(DoFocusRect, weld::Widget&, tools::Rectangle);
    DECL_DLLPRIVATE_LINK(DoCommand, const CommandEvent&, bool);
    DECL_DLLPRIVATE_LINK(DoStyleUpdated, weld::Widget&, void);
    DECL_DLLPRIVATE_LINK(DoRequestHelp, tools::Rectangle&, OUString);
    DECL_DLLPRIVATE_LINK(DoGetSurrounding, OUString&, int);
    DECL_DLLPRIVATE_LINK(DoDeleteSurrounding, const Selection&, bool);

public:
    CustomWeld(weld::Builder& rBuilder, const OUString& rDrawingId,
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
    void set_help_id(const OUString& rHelpId) { m_xDrawingArea->set_help_id(rHelpId); }
    void set_tooltip_text(const OUString& rTip) { m_xDrawingArea->set_tooltip_text(rTip); }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
