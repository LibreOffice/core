/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/weld.hxx>
#include <vcl/jsdialog/executor.hxx>
#include <comphelper/string.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/virdev.hxx>
#include <vcl/builder.hxx>
#include <salvtables.hxx>
#include <vcl/toolkit/button.hxx>
#include <vcl/toolkit/fmtfield.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <cppuhelper/compbase.hxx>
#include <boost/property_tree/ptree_fwd.hpp>

class ToolBox;
class ComboBox;
class VclMultiLineEdit;
class SvTabListBox;

typedef std::map<OString, weld::Widget*> WidgetMap;

class JSDialogNotifyIdle : public Idle
{
    // used to send message
    VclPtr<vcl::Window> m_aNotifierWindow;
    // used to generate JSON
    VclPtr<vcl::Window> m_aContentWindow;
    std::string m_sTypeOfJSON;
    std::string m_LastNotificationMessage;
    bool m_bForce;

public:
    JSDialogNotifyIdle(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                       std::string sTypeOfJSON);

    void Invoke() override;
    void ForceUpdate();
    void sendClose();
    VclPtr<vcl::Window> getNotifierWindow() { return m_aNotifierWindow; }
    void updateStatus(VclPtr<vcl::Window> pWindow);

private:
    void send(std::unique_ptr<tools::JsonWriter> aJsonWriter);
    std::unique_ptr<tools::JsonWriter> dumpStatus() const;
    std::unique_ptr<tools::JsonWriter> generateCloseMessage() const;
};

class JSDialogSender
{
    std::unique_ptr<JSDialogNotifyIdle> mpIdleNotify;

public:
    JSDialogSender(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                   std::string sTypeOfJSON)
        : mpIdleNotify(new JSDialogNotifyIdle(aNotifierWindow, aContentWindow, sTypeOfJSON))
    {
    }

    void notifyDialogState(bool bForce = false);
    void sendClose();
    void dumpStatus();
    void sendUpdate(VclPtr<vcl::Window> pWindow);
};

class JSDropTarget final
    : public cppu::WeakComponentImplHelper<css::datatransfer::dnd::XDropTarget,
                                           css::lang::XInitialization, css::lang::XServiceInfo>
{
    osl::Mutex m_aMutex;
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> m_aListeners;

public:
    JSDropTarget();

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rArgs) override;

    // XDropTarget
    virtual void SAL_CALL addDropTargetListener(
        const css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>&) override;
    virtual void SAL_CALL removeDropTargetListener(
        const css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>&) override;
    virtual sal_Bool SAL_CALL isActive() override;
    virtual void SAL_CALL setActive(sal_Bool active) override;
    virtual sal_Int8 SAL_CALL getDefaultActions() override;
    virtual void SAL_CALL setDefaultActions(sal_Int8 actions) override;

    OUString SAL_CALL getImplementationName() override;

    sal_Bool SAL_CALL supportsService(OUString const& ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    void fire_drop(const css::datatransfer::dnd::DropTargetDropEvent& dtde);

    void fire_dragEnter(const css::datatransfer::dnd::DropTargetDragEnterEvent& dtde);
};

class JSInstanceBuilder : public SalInstanceBuilder
{
    sal_uInt64 m_nWindowId;
    /// used in case of tab pages where dialog is not a direct top level
    VclPtr<vcl::Window> m_aParentDialog;
    VclPtr<vcl::Window> m_aContentWindow;
    std::list<std::string> m_aRememberedWidgets;
    std::string m_sTypeOfJSON;
    bool m_bHasTopLevelDialog;
    bool m_bIsNotebookbar;

    friend VCL_DLLPUBLIC bool jsdialog::ExecuteAction(sal_uInt64 nWindowId, const OString& rWidget,
                                                      StringMap& rData);

    static std::map<sal_uInt64, WidgetMap>& GetLOKWeldWidgetsMap();
    static void InsertWindowToMap(sal_uInt64 nWindowId);
    void RememberWidget(const OString& id, weld::Widget* pWidget);
    static weld::Widget* FindWeldWidgetsMap(sal_uInt64 nWindowId, const OString& rWidget);

    /// used for dialogs
    JSInstanceBuilder(weld::Widget* pParent, const OUString& rUIRoot, const OUString& rUIFile);
    /// used for notebookbar, optional nWindowId is used if getting parent id failed
    JSInstanceBuilder(vcl::Window* pParent, const OUString& rUIRoot, const OUString& rUIFile,
                      const css::uno::Reference<css::frame::XFrame>& rFrame,
                      sal_uInt64 nWindowId = 0);
    /// for autofilter dropdown
    JSInstanceBuilder(vcl::Window* pParent, const OUString& rUIRoot, const OUString& rUIFile);

public:
    static JSInstanceBuilder* CreateDialogBuilder(weld::Widget* pParent, const OUString& rUIRoot,
                                                  const OUString& rUIFile);
    static JSInstanceBuilder*
    CreateNotebookbarBuilder(vcl::Window* pParent, const OUString& rUIRoot, const OUString& rUIFile,
                             const css::uno::Reference<css::frame::XFrame>& rFrame,
                             sal_uInt64 nWindowId = 0);
    static JSInstanceBuilder* CreateAutofilterWindowBuilder(vcl::Window* pParent,
                                                            const OUString& rUIRoot,
                                                            const OUString& rUIFile);

    virtual ~JSInstanceBuilder() override;
    virtual std::unique_ptr<weld::MessageDialog> weld_message_dialog(const OString& id) override;
    virtual std::unique_ptr<weld::Dialog> weld_dialog(const OString& id) override;
    virtual std::unique_ptr<weld::Label> weld_label(const OString& id) override;
    virtual std::unique_ptr<weld::Button> weld_button(const OString& id) override;
    virtual std::unique_ptr<weld::Entry> weld_entry(const OString& id) override;
    virtual std::unique_ptr<weld::ComboBox> weld_combo_box(const OString& id) override;
    virtual std::unique_ptr<weld::Notebook> weld_notebook(const OString& id) override;
    virtual std::unique_ptr<weld::SpinButton> weld_spin_button(const OString& id) override;
    virtual std::unique_ptr<weld::CheckButton> weld_check_button(const OString& id) override;
    virtual std::unique_ptr<weld::DrawingArea>
    weld_drawing_area(const OString& id, const a11yref& rA11yImpl = nullptr,
                      FactoryFunction pUITestFactoryFunction = nullptr,
                      void* pUserData = nullptr) override;
    virtual std::unique_ptr<weld::Toolbar> weld_toolbar(const OString& id) override;
    virtual std::unique_ptr<weld::TextView> weld_text_view(const OString& id) override;
    virtual std::unique_ptr<weld::TreeView> weld_tree_view(const OString& id) override;
    virtual std::unique_ptr<weld::Expander> weld_expander(const OString& id) override;

    static weld::MessageDialog* CreateMessageDialog(weld::Widget* pParent,
                                                    VclMessageType eMessageType,
                                                    VclButtonsType eButtonType,
                                                    const OUString& rPrimaryMessage);

private:
    VclPtr<vcl::Window>& GetContentWindow();
    VclPtr<vcl::Window>& GetNotifierWindow();
};

template <class BaseInstanceClass, class VclClass>
class JSWidget : public BaseInstanceClass, public JSDialogSender
{
protected:
    rtl::Reference<JSDropTarget> m_xDropTarget;

public:
    JSWidget(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
             VclClass* pObject, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
             std::string sTypeOfJSON)
        : BaseInstanceClass(pObject, pBuilder, bTakeOwnership)
        , JSDialogSender(aNotifierWindow, aContentWindow, sTypeOfJSON)
    {
    }

    virtual void show() override
    {
        BaseInstanceClass::show();
        notifyDialogState();
    }

    virtual void hide() override
    {
        BaseInstanceClass::hide();
        notifyDialogState();
    }

    using BaseInstanceClass::set_sensitive;
    virtual void set_sensitive(bool sensitive) override
    {
        BaseInstanceClass::set_sensitive(sensitive);
        notifyDialogState();
    }

    virtual css::uno::Reference<css::datatransfer::dnd::XDropTarget> get_drop_target() override
    {
        if (!m_xDropTarget)
            m_xDropTarget.set(new JSDropTarget);

        return m_xDropTarget.get();
    }
};

class JSDialog : public JSWidget<SalInstanceDialog, ::Dialog>
{
    DECL_LINK(on_dump_status, void*, void);
    DECL_LINK(on_window_event, VclWindowEvent&, void);

    bool m_bNotifyCreated;

public:
    JSDialog(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
             ::Dialog* pDialog, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
             std::string sTypeOfJSON);

    virtual void collapse(weld::Widget* pEdit, weld::Widget* pButton) override;
    virtual void undo_collapse() override;
    virtual void response(int response) override;
};

class JSLabel : public JSWidget<SalInstanceLabel, FixedText>
{
public:
    JSLabel(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
            FixedText* pLabel, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
            std::string sTypeOfJSON);
    virtual void set_label(const OUString& rText) override;
};

class JSButton : public JSWidget<SalInstanceButton, ::Button>
{
public:
    JSButton(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
             ::Button* pButton, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
             std::string sTypeOfJSON);
};

class JSEntry : public JSWidget<SalInstanceEntry, ::Edit>
{
public:
    JSEntry(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow, ::Edit* pEntry,
            SalInstanceBuilder* pBuilder, bool bTakeOwnership, std::string sTypeOfJSON);
    virtual void set_text(const OUString& rText) override;
};

class JSListBox : public JSWidget<SalInstanceComboBoxWithoutEdit, ::ListBox>
{
public:
    JSListBox(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
              ::ListBox* pListBox, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
              std::string sTypeOfJSON);
    virtual void insert(int pos, const OUString& rStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface) override;
    virtual void remove(int pos) override;
    virtual void set_active(int pos) override;
};

class JSComboBox : public JSWidget<SalInstanceComboBoxWithEdit, ::ComboBox>
{
public:
    JSComboBox(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
               ::ComboBox* pComboBox, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
               std::string sTypeOfJSON);
    virtual void insert(int pos, const OUString& rStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface) override;
    virtual void remove(int pos) override;
    virtual void set_entry_text(const OUString& rText) override;
    virtual void set_active(int pos) override;
};

class JSNotebook : public JSWidget<SalInstanceNotebook, ::TabControl>
{
public:
    JSNotebook(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
               ::TabControl* pControl, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
               std::string sTypeOfJSON);

    virtual void set_current_page(int nPage) override;

    virtual void set_current_page(const OString& rIdent) override;

    virtual void remove_page(const OString& rIdent) override;

    virtual void insert_page(const OString& rIdent, const OUString& rLabel, int nPos) override;
};

class JSSpinButton : public JSWidget<SalInstanceSpinButton, ::FormattedField>
{
public:
    JSSpinButton(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                 ::FormattedField* pSpin, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
                 std::string sTypeOfJSON);

    virtual void set_value(int value) override;
};

class JSMessageDialog : public SalInstanceMessageDialog, public JSDialogSender
{
    DECL_LINK(on_dump_status, void*, void);
    DECL_LINK(on_window_event, VclWindowEvent&, void);

    bool m_bNotifyCreated;
public:
    JSMessageDialog(::MessageDialog* pDialog, VclPtr<vcl::Window> aContentWindow,
                    SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual void set_primary_text(const OUString& rText) override;

    virtual void set_secondary_text(const OUString& rText) override;
};

class JSCheckButton : public JSWidget<SalInstanceCheckButton, ::CheckBox>
{
public:
    JSCheckButton(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                  ::CheckBox* pCheckBox, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
                  std::string sTypeOfJSON);

    virtual void set_active(bool active) override;
};

class JSDrawingArea : public SalInstanceDrawingArea, public JSDialogSender
{
public:
    JSDrawingArea(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                  VclDrawingArea* pDrawingArea, SalInstanceBuilder* pBuilder, const a11yref& rAlly,
                  FactoryFunction pUITestFactoryFunction, void* pUserData, std::string sTypeOfJSON);

    virtual void queue_draw() override;
    virtual void queue_draw_area(int x, int y, int width, int height) override;
};

class JSToolbar : public JSWidget<SalInstanceToolbar, ::ToolBox>
{
public:
    JSToolbar(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
              ::ToolBox* pToolbox, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
              std::string sTypeOfJSON);

    virtual void signal_clicked(const OString& rIdent) override;
};

class JSTextView : public JSWidget<SalInstanceTextView, ::VclMultiLineEdit>
{
public:
    JSTextView(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
               ::VclMultiLineEdit* pTextView, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
               std::string sTypeOfJSON);
    virtual void set_text(const OUString& rText) override;
};

class JSTreeView : public JSWidget<SalInstanceTreeView, ::SvTabListBox>
{
    DECL_LINK(on_window_event, VclWindowEvent&, void);

public:
    JSTreeView(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
               ::SvTabListBox* pTextView, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
               std::string sTypeOfJSON);

    using SalInstanceTreeView::set_toggle;
    /// pos is used differently here, it defines how many steps of iterator we need to perform to take entry
    virtual void set_toggle(int pos, TriState eState, int col = -1) override;

    using SalInstanceTreeView::select;
    /// pos is used differently here, it defines how many steps of iterator we need to perform to take entry
    virtual void select(int pos) override;

    virtual weld::TreeView* get_drag_source() const override;

    void drag_start();
    void drag_end();
};

class JSExpander : public JSWidget<SalInstanceExpander, ::VclExpander>
{
public:
    JSExpander(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
               ::VclExpander* pExpander, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
               std::string sTypeOfJSON);

    virtual void set_expanded(bool bExpand) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
