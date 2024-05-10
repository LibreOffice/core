/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_WELDUTILS_HXX
#define INCLUDED_VCL_WELDUTILS_HXX

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <comphelper/compbase.hxx>
#include <tools/time.hxx>
#include <vcl/dllapi.h>
#include <vcl/formatter.hxx>
#include <vcl/timer.hxx>
#include <vcl/transfer.hxx>
#include <vcl/weld.hxx>

class CalendarWrapper;

namespace vcl
{
class Window;
}

namespace weld
{
typedef comphelper::WeakComponentImplHelper<css::awt::XWindow> TransportAsXWindow_Base;

class VCL_DLLPUBLIC TransportAsXWindow : public TransportAsXWindow_Base
{
private:
    weld::Widget* m_pWeldWidget;
    weld::Builder* m_pWeldWidgetBuilder;

    comphelper::OInterfaceContainerHelper4<css::awt::XWindowListener> m_aWindowListeners;
    comphelper::OInterfaceContainerHelper4<css::awt::XKeyListener> m_aKeyListeners;
    comphelper::OInterfaceContainerHelper4<css::awt::XFocusListener> m_aFocusListeners;
    comphelper::OInterfaceContainerHelper4<css::awt::XMouseListener> m_aMouseListeners;
    comphelper::OInterfaceContainerHelper4<css::awt::XMouseMotionListener> m_aMotionListeners;
    comphelper::OInterfaceContainerHelper4<css::awt::XPaintListener> m_aPaintListeners;

public:
    TransportAsXWindow(weld::Widget* pWeldWidget, weld::Builder* pWeldWidgetBuilder = nullptr)
        : m_pWeldWidget(pWeldWidget)
        , m_pWeldWidgetBuilder(pWeldWidgetBuilder)
    {
    }

    weld::Widget* getWidget() const { return m_pWeldWidget; }

    weld::Builder* getBuilder() const { return m_pWeldWidgetBuilder; }

    virtual void clear()
    {
        m_pWeldWidget = nullptr;
        m_pWeldWidgetBuilder = nullptr;
    }

    // css::awt::XWindow
    void SAL_CALL setPosSize(sal_Int32, sal_Int32, sal_Int32, sal_Int32, sal_Int16) override
    {
        throw css::uno::RuntimeException(u"not implemented"_ustr);
    }

    css::awt::Rectangle SAL_CALL getPosSize() override
    {
        throw css::uno::RuntimeException(u"not implemented"_ustr);
    }

    void SAL_CALL setVisible(sal_Bool bVisible) override { m_pWeldWidget->set_visible(bVisible); }

    void SAL_CALL setEnable(sal_Bool bSensitive) override
    {
        m_pWeldWidget->set_sensitive(bSensitive);
    }

    void SAL_CALL setFocus() override { m_pWeldWidget->grab_focus(); }

    void SAL_CALL
    addWindowListener(const css::uno::Reference<css::awt::XWindowListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aWindowListeners.addInterface(g, rListener);
    }

    void SAL_CALL
    removeWindowListener(const css::uno::Reference<css::awt::XWindowListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aWindowListeners.removeInterface(g, rListener);
    }

    void SAL_CALL
    addFocusListener(const css::uno::Reference<css::awt::XFocusListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aFocusListeners.addInterface(g, rListener);
    }

    void SAL_CALL
    removeFocusListener(const css::uno::Reference<css::awt::XFocusListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aFocusListeners.removeInterface(g, rListener);
    }

    void SAL_CALL
    addKeyListener(const css::uno::Reference<css::awt::XKeyListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aKeyListeners.addInterface(g, rListener);
    }

    void SAL_CALL
    removeKeyListener(const css::uno::Reference<css::awt::XKeyListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aKeyListeners.removeInterface(g, rListener);
    }

    void SAL_CALL
    addMouseListener(const css::uno::Reference<css::awt::XMouseListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aMouseListeners.addInterface(g, rListener);
    }

    void SAL_CALL
    removeMouseListener(const css::uno::Reference<css::awt::XMouseListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aMouseListeners.removeInterface(g, rListener);
    }

    void SAL_CALL addMouseMotionListener(
        const css::uno::Reference<css::awt::XMouseMotionListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aMotionListeners.addInterface(g, rListener);
    }

    void SAL_CALL removeMouseMotionListener(
        const css::uno::Reference<css::awt::XMouseMotionListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aMotionListeners.removeInterface(g, rListener);
    }

    void SAL_CALL
    addPaintListener(const css::uno::Reference<css::awt::XPaintListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aPaintListeners.addInterface(g, rListener);
    }

    void SAL_CALL
    removePaintListener(const css::uno::Reference<css::awt::XPaintListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aPaintListeners.removeInterface(g, rListener);
    }
};

// don't export to avoid duplicate WeakImplHelper definitions with MSVC
class SAL_DLLPUBLIC_TEMPLATE WidgetStatusListener_Base
    : public cppu::WeakImplHelper<css::frame::XStatusListener>
{
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) WidgetStatusListener final : public WidgetStatusListener_Base
{
public:
    WidgetStatusListener(weld::Widget* widget, const OUString& rCommand);

private:
    weld::Widget* mWidget; /** The widget on which actions are performed */

    /** Dispatcher. Need to keep a reference to it as long as this StatusListener exists. */
    css::uno::Reference<css::frame::XDispatch> mxDispatch;
    css::util::URL maCommandURL;
    css::uno::Reference<css::frame::XFrame> mxFrame;

public:
    SAL_DLLPRIVATE void SAL_CALL
    statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

    SAL_DLLPRIVATE void SAL_CALL disposing(const css::lang::EventObject& /*Source*/) override;

    const css::uno::Reference<css::frame::XFrame>& getFrame() const { return mxFrame; }

    void startListening();

    void dispose();
};

class VCL_DLLPUBLIC EntryFormatter : public Formatter
{
public:
    EntryFormatter(weld::Entry& rEntry);
    EntryFormatter(weld::FormattedSpinButton& rSpinButton);

    weld::Entry& get_widget() { return m_rEntry; }

    // public Formatter overrides, drives interactions with the Entry
    SAL_DLLPRIVATE virtual Selection GetEntrySelection() const override;
    SAL_DLLPRIVATE virtual OUString GetEntryText() const override;
    SAL_DLLPRIVATE virtual void SetEntryText(const OUString& rText, const Selection& rSel) override;
    SAL_DLLPRIVATE virtual void SetEntryTextColor(const Color* pColor) override;
    SAL_DLLPRIVATE virtual SelectionOptions GetEntrySelectionOptions() const override;
    SAL_DLLPRIVATE virtual void FieldModified() override;

    // public Formatter overrides, drives optional SpinButton settings
    SAL_DLLPRIVATE virtual void ClearMinValue() override;
    SAL_DLLPRIVATE virtual void SetMinValue(double dMin) override;
    SAL_DLLPRIVATE virtual void ClearMaxValue() override;
    SAL_DLLPRIVATE virtual void SetMaxValue(double dMin) override;

    SAL_DLLPRIVATE virtual void SetSpinSize(double dStep) override;

    void SetEntrySelectionOptions(SelectionOptions eOptions) { m_eOptions = eOptions; }

    /* EntryFormatter will set listeners to "changed" and "focus-out" of the
       Entry so users that want to add their own listeners to those must set
       them through this formatter and not directly on that entry.

       If EntryFormatter is used with a weld::FormattedSpinButton this is
       handled transparently by the FormattedSpinButton for the user and the
       handlers can be set on the FormattedSpinButton
    */
    void connect_changed(const Link<weld::Entry&, void>& rLink) { m_aModifyHdl = rLink; }
    void connect_focus_out(const Link<weld::Widget&, void>& rLink) { m_aFocusOutHdl = rLink; }

    SAL_DLLPRIVATE virtual ~EntryFormatter() override;

private:
    weld::Entry& m_rEntry;
    weld::FormattedSpinButton* m_pSpinButton;
    Link<weld::Entry&, void> m_aModifyHdl;
    Link<weld::Widget&, void> m_aFocusOutHdl;
    SelectionOptions m_eOptions;
    DECL_DLLPRIVATE_LINK(ModifyHdl, weld::Entry&, void);
    DECL_DLLPRIVATE_LINK(FocusOutHdl, weld::Widget&, void);
    SAL_DLLPRIVATE void Init();

    // private Formatter overrides
    SAL_DLLPRIVATE virtual void UpdateCurrentValue(double dCurrentValue) override;
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) DoubleNumericFormatter final : public EntryFormatter
{
public:
    DoubleNumericFormatter(weld::Entry& rEntry);
    DoubleNumericFormatter(weld::FormattedSpinButton& rSpinButton);

    SAL_DLLPRIVATE virtual ~DoubleNumericFormatter() override;

private:
    SAL_DLLPRIVATE virtual bool CheckText(const OUString& sText) const override;

    SAL_DLLPRIVATE virtual void FormatChanged(FORMAT_CHANGE_TYPE nWhat) override;
    SAL_DLLPRIVATE void ResetConformanceTester();

    std::unique_ptr<validation::NumberValidator> m_pNumberValidator;
};

class VCL_DLLPUBLIC LongCurrencyFormatter final : public EntryFormatter
{
public:
    LongCurrencyFormatter(weld::Entry& rEntry);
    LongCurrencyFormatter(weld::FormattedSpinButton& rSpinButton);

    void SetUseThousandSep(bool b);
    void SetCurrencySymbol(const OUString& rStr);

    SAL_DLLPRIVATE virtual ~LongCurrencyFormatter() override;

private:
    DECL_DLLPRIVATE_LINK(FormatOutputHdl, LinkParamNone*, bool);
    DECL_DLLPRIVATE_LINK(ParseInputHdl, sal_Int64*, TriState);

    SAL_DLLPRIVATE void Init();

    OUString m_aCurrencySymbol;
    bool m_bThousandSep;
};

class VCL_DLLPUBLIC TimeFormatter final : public EntryFormatter
{
public:
    TimeFormatter(weld::Entry& rEntry);
    TimeFormatter(weld::FormattedSpinButton& rSpinButton);

    void SetExtFormat(ExtTimeFieldFormat eFormat);
    void SetDuration(bool bDuration);
    void SetTimeFormat(TimeFieldFormat eTimeFormat);

    void SetMin(const tools::Time& rNewMin);
    void SetMax(const tools::Time& rNewMax);

    void SetTime(const tools::Time& rNewTime);
    tools::Time GetTime();

    virtual ~TimeFormatter() override;

private:
    DECL_DLLPRIVATE_LINK(FormatOutputHdl, LinkParamNone*, bool);
    DECL_DLLPRIVATE_LINK(ParseInputHdl, sal_Int64*, TriState);
    DECL_DLLPRIVATE_LINK(CursorChangedHdl, weld::Entry&, void);

    SAL_DLLPRIVATE void Init();

    SAL_DLLPRIVATE static tools::Time ConvertValue(int nValue);
    SAL_DLLPRIVATE static int ConvertValue(const tools::Time& rTime);

    SAL_DLLPRIVATE OUString FormatNumber(int nValue) const;

    TimeFieldFormat m_eFormat;
    TimeFormat m_eTimeFormat;
    bool m_bDuration;
};

class VCL_DLLPUBLIC DateFormatter final : public EntryFormatter
{
public:
    DateFormatter(weld::Entry& rEntry);

    void SetMin(const Date& rNewMin);
    void SetMax(const Date& rNewMax);

    void SetDate(const Date& rNewDate);
    Date GetDate();

    void SetExtDateFormat(ExtDateFieldFormat eFormat);
    void SetShowDateCentury(bool bShowCentury);

    virtual ~DateFormatter() override;

private:
    DECL_DLLPRIVATE_LINK(FormatOutputHdl, LinkParamNone*, bool);
    DECL_DLLPRIVATE_LINK(ParseInputHdl, sal_Int64*, TriState);
    DECL_DLLPRIVATE_LINK(CursorChangedHdl, weld::Entry&, void);

    SAL_DLLPRIVATE void Init();
    SAL_DLLPRIVATE CalendarWrapper& GetCalendarWrapper() const;

    SAL_DLLPRIVATE OUString FormatNumber(int nValue) const;

    ExtDateFieldFormat m_eFormat;
    mutable std::unique_ptr<CalendarWrapper> m_xCalendarWrapper;
};

class VCL_DLLPUBLIC PatternFormatter final
{
public:
    PatternFormatter(weld::Entry& rEntry);
    ~PatternFormatter();

    weld::Entry& get_widget() { return m_rEntry; }

    void SetMask(const OString& rEditMask, const OUString& rLiteralMask);
    void SetStrictFormat(bool bStrict);
    void ReformatAll();

    /* PatternFormatter will set listeners to "changed", "focus-out", "focus-in"
       and  "key-press" of the Entry so users that want to add their own listeners
       to those must set them through this formatter and not directly on that entry.
    */
    void connect_changed(const Link<weld::Entry&, void>& rLink) { m_aModifyHdl = rLink; }
    void connect_focus_out(const Link<weld::Widget&, void>& rLink) { m_aFocusOutHdl = rLink; }
    void connect_focus_in(const Link<weld::Widget&, void>& rLink) { m_aFocusInHdl = rLink; }
    void connect_key_press(const Link<const KeyEvent&, bool>& rLink) { m_aKeyPressHdl = rLink; }

    SAL_DLLPRIVATE void Modify();

private:
    weld::Entry& m_rEntry;
    Link<weld::Entry&, void> m_aModifyHdl;
    Link<weld::Widget&, void> m_aFocusInHdl;
    Link<weld::Widget&, void> m_aFocusOutHdl;
    Link<const KeyEvent&, bool> m_aKeyPressHdl;
    bool m_bStrictFormat;
    bool m_bSameMask;
    bool m_bReformat;
    bool m_bInPattKeyInput;
    OString m_aEditMask;
    OUString m_aLiteralMask;

    SAL_DLLPRIVATE void EntryGainFocus();
    SAL_DLLPRIVATE void EntryLostFocus();
    DECL_DLLPRIVATE_LINK(ModifyHdl, weld::Entry&, void);
    DECL_DLLPRIVATE_LINK(FocusInHdl, weld::Widget&, void);
    DECL_DLLPRIVATE_LINK(FocusOutHdl, weld::Widget&, void);
    DECL_DLLPRIVATE_LINK(KeyInputHdl, const KeyEvent&, bool);
};

class VCL_DLLPUBLIC ButtonPressRepeater final
    : public std::enable_shared_from_this<ButtonPressRepeater>
{
private:
    weld::Button& m_rButton;
    AutoTimer m_aRepeat;
    const Link<Button&, void> m_aLink;
    const Link<const CommandEvent&, void> m_aContextLink;
    bool m_bModKey;

    DECL_DLLPRIVATE_LINK(MousePressHdl, const MouseEvent&, bool);
    DECL_DLLPRIVATE_LINK(MouseReleaseHdl, const MouseEvent&, bool);
    DECL_DLLPRIVATE_LINK(RepeatTimerHdl, Timer*, void);

public:
    ButtonPressRepeater(weld::Button& rButton, const Link<Button&, void>& rLink,
                        const Link<const CommandEvent&, void>& rContextLink
                        = Link<const CommandEvent&, void>());
    void Stop() { m_aRepeat.Stop(); }
    bool IsModKeyPressed() const { return m_bModKey; }
};

/*
  If a TreeView is used as a list, rather than a tree, and DnD should just
  reorder rows, then this can be used to implement that.

  Because the TreeView doesn't want or need subnodes, the drop target can be
  simply visually indicated as being between rows (the issue of a final drop
  location of a child of the drop target doesn't arise), and the meaning of
  what a drop before or after the last row should do is unambiguous.
*/
class VCL_DLLPUBLIC ReorderingDropTarget : public DropTargetHelper
{
    weld::TreeView& m_rTreeView;

protected:
    virtual sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt) override;
    virtual sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt) override;

public:
    ReorderingDropTarget(weld::TreeView& rTreeView);
};

// get the row the iterator is on
VCL_DLLPUBLIC size_t GetAbsPos(const weld::TreeView& rTreeView, const weld::TreeIter& rIter);

// an entry is visible if all parents are expanded
VCL_DLLPUBLIC bool IsEntryVisible(const weld::TreeView& rTreeView, const weld::TreeIter& rIter);

// A Parent's Children are turned into Children of the Parent which comes next in hierarchy
VCL_DLLPUBLIC void RemoveParentKeepChildren(weld::TreeView& rTreeView,
                                            const weld::TreeIter& rParent);

// return the min height of a weld::Entry
VCL_DLLPUBLIC int GetMinimumEditHeight();

// return the weld::Window of the SalFrame rOutWin is in, and convert rRect
// from relative to rOutWin to relative to that weld::Window suitable for use
// with popup_at_rect
VCL_DLLPUBLIC weld::Window* GetPopupParent(vcl::Window& rOutWin, tools::Rectangle& rRect);

// Use Application::GetDefaultDevice to set the PointFont rFont to the OutputDevice
VCL_DLLPUBLIC void SetPointFont(OutputDevice& rDevice, const vcl::Font& rFont,
                                bool bUseDeviceDPI = false);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
