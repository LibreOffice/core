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

#include <deque>

class ToolBox;
class ComboBox;
class VclMultiLineEdit;
class SvTabListBox;
class IconView;

typedef std::map<OString, weld::Widget*> WidgetMap;

namespace jsdialog
{
enum MessageType
{
    FullUpdate,
    WidgetUpdate,
    Close
};
}

class JSDialogNotifyIdle : public Idle
{
    // used to send message
    VclPtr<vcl::Window> m_aNotifierWindow;
    // used to generate JSON
    VclPtr<vcl::Window> m_aContentWindow;
    std::string m_sTypeOfJSON;
    std::string m_LastNotificationMessage;
    bool m_bForce;

    std::deque<std::pair<jsdialog::MessageType, VclPtr<vcl::Window>>> m_aMessageQueue;

public:
    JSDialogNotifyIdle(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                       std::string sTypeOfJSON);

    void Invoke() override;

    void forceUpdate();
    void sendMessage(jsdialog::MessageType eType, VclPtr<vcl::Window> pWindow);

private:
    void send(tools::JsonWriter& aJsonWriter);
    std::unique_ptr<tools::JsonWriter> generateFullUpdate() const;
    std::unique_ptr<tools::JsonWriter> generateWidgetUpdate(VclPtr<vcl::Window> pWindow) const;
    std::unique_ptr<tools::JsonWriter> generateCloseMessage() const;
};

class JSDialogSender
{
    std::unique_ptr<JSDialogNotifyIdle> mpIdleNotify;

public:
    JSDialogSender() = default;
    JSDialogSender(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                   std::string sTypeOfJSON)
    {
        initializeSender(aNotifierWindow, aContentWindow, sTypeOfJSON);
    }

    virtual ~JSDialogSender();

    virtual void sendFullUpdate(bool bForce = false);
    void sendClose();
    virtual void sendUpdate(VclPtr<vcl::Window> pWindow, bool bForce = false);
    void flush() { mpIdleNotify->Invoke(); }

protected:
    void initializeSender(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                          std::string sTypeOfJSON)
    {
        mpIdleNotify.reset(new JSDialogNotifyIdle(aNotifierWindow, aContentWindow, sTypeOfJSON));
    }
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

class JSInstanceBuilder : public SalInstanceBuilder, public JSDialogSender
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
    friend VCL_DLLPUBLIC void jsdialog::SendFullUpdate(sal_uInt64 nWindowId,
                                                       const OString& rWidget);

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
    virtual std::unique_ptr<weld::IconView> weld_icon_view(const OString& id) override;
    virtual std::unique_ptr<weld::RadioButton> weld_radio_button(const OString& id) override;

    static weld::MessageDialog* CreateMessageDialog(weld::Widget* pParent,
                                                    VclMessageType eMessageType,
                                                    VclButtonsType eButtonType,
                                                    const OUString& rPrimaryMessage);

private:
    const std::string& GetTypeOfJSON();
    VclPtr<vcl::Window>& GetContentWindow();
    VclPtr<vcl::Window>& GetNotifierWindow();
};

class BaseJSWidget
{
public:
    virtual ~BaseJSWidget() = default;

    virtual void sendClose() = 0;

    virtual void sendUpdate(bool bForce = false) = 0;

    virtual void sendFullUpdate(bool bForce = false) = 0;
};

template <class BaseInstanceClass, class VclClass>
class JSWidget : public BaseInstanceClass, public BaseJSWidget
{
protected:
    rtl::Reference<JSDropTarget> m_xDropTarget;
    bool m_bIsFreezed;

    JSDialogSender* m_pSender;

public:
    JSWidget(JSDialogSender* pSender, VclClass* pObject, SalInstanceBuilder* pBuilder,
             bool bTakeOwnership)
        : BaseInstanceClass(pObject, pBuilder, bTakeOwnership)
        , m_bIsFreezed(false)
        , m_pSender(pSender)
    {
    }

    JSWidget(JSDialogSender* pSender, VclClass* pObject, SalInstanceBuilder* pBuilder,
             const a11yref& rAlly, FactoryFunction pUITestFactoryFunction, void* pUserData,
             bool bTakeOwnership)
        : BaseInstanceClass(pObject, pBuilder, rAlly, pUITestFactoryFunction, pUserData,
                            bTakeOwnership)
        , m_bIsFreezed(false)
        , m_pSender(pSender)
    {
    }

    virtual void show() override
    {
        BaseInstanceClass::show();
        sendFullUpdate();
    }

    virtual void hide() override
    {
        BaseInstanceClass::hide();
        sendFullUpdate();
    }

    using BaseInstanceClass::set_sensitive;
    virtual void set_sensitive(bool sensitive) override
    {
        bool bIsSensitive = BaseInstanceClass::get_sensitive();
        BaseInstanceClass::set_sensitive(sensitive);
        if (bIsSensitive != sensitive)
            sendUpdate();
    }

    virtual css::uno::Reference<css::datatransfer::dnd::XDropTarget> get_drop_target() override
    {
        if (!m_xDropTarget)
            m_xDropTarget.set(new JSDropTarget);

        return m_xDropTarget;
    }

    virtual void freeze() override
    {
        BaseInstanceClass::freeze();
        m_bIsFreezed = true;
    }

    virtual void thaw() override
    {
        BaseInstanceClass::thaw();
        m_bIsFreezed = false;
    }

    virtual void sendClose() override
    {
        if (m_pSender)
            m_pSender->sendClose();
    }

    virtual void sendUpdate(bool bForce = false) override
    {
        if (!m_bIsFreezed && m_pSender)
            m_pSender->sendUpdate(BaseInstanceClass::m_xWidget, bForce);
    }

    virtual void sendFullUpdate(bool bForce = false) override
    {
        if ((!m_bIsFreezed || bForce) && m_pSender)
            m_pSender->sendFullUpdate(bForce);
    }
};

class JSDialog : public JSWidget<SalInstanceDialog, ::Dialog>
{
public:
    JSDialog(JSDialogSender* pSender, ::Dialog* pDialog, SalInstanceBuilder* pBuilder,
             bool bTakeOwnership);

    virtual void collapse(weld::Widget* pEdit, weld::Widget* pButton) override;
    virtual void undo_collapse() override;
    virtual void response(int response) override;
};

class JSLabel : public JSWidget<SalInstanceLabel, FixedText>
{
public:
    JSLabel(JSDialogSender* pSender, FixedText* pLabel, SalInstanceBuilder* pBuilder,
            bool bTakeOwnership);
    virtual void set_label(const OUString& rText) override;
};

class JSButton : public JSWidget<SalInstanceButton, ::Button>
{
public:
    JSButton(JSDialogSender* pSender, ::Button* pButton, SalInstanceBuilder* pBuilder,
             bool bTakeOwnership);
};

class JSEntry : public JSWidget<SalInstanceEntry, ::Edit>
{
public:
    JSEntry(JSDialogSender* pSender, ::Edit* pEntry, SalInstanceBuilder* pBuilder,
            bool bTakeOwnership);
    virtual void set_text(const OUString& rText) override;
    void set_text_without_notify(const OUString& rText);
};

class JSListBox : public JSWidget<SalInstanceComboBoxWithoutEdit, ::ListBox>
{
public:
    JSListBox(JSDialogSender* pSender, ::ListBox* pListBox, SalInstanceBuilder* pBuilder,
              bool bTakeOwnership);
    virtual void insert(int pos, const OUString& rStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface) override;
    virtual void remove(int pos) override;
    virtual void set_active(int pos) override;
};

class JSComboBox : public JSWidget<SalInstanceComboBoxWithEdit, ::ComboBox>
{
public:
    JSComboBox(JSDialogSender* pSender, ::ComboBox* pComboBox, SalInstanceBuilder* pBuilder,
               bool bTakeOwnership);
    virtual void insert(int pos, const OUString& rStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface) override;
    virtual void remove(int pos) override;
    virtual void set_entry_text(const OUString& rText) override;
    virtual void set_active(int pos) override;
};

class JSNotebook : public JSWidget<SalInstanceNotebook, ::TabControl>
{
public:
    JSNotebook(JSDialogSender* pSender, ::TabControl* pControl, SalInstanceBuilder* pBuilder,
               bool bTakeOwnership);

    virtual void set_current_page(int nPage) override;

    virtual void set_current_page(const OString& rIdent) override;

    virtual void remove_page(const OString& rIdent) override;

    virtual void insert_page(const OString& rIdent, const OUString& rLabel, int nPos) override;
};

class JSSpinButton : public JSWidget<SalInstanceSpinButton, ::FormattedField>
{
public:
    JSSpinButton(JSDialogSender* pSender, ::FormattedField* pSpin, SalInstanceBuilder* pBuilder,
                 bool bTakeOwnership);

    virtual void set_value(int value) override;
};

class JSMessageDialog : public JSWidget<SalInstanceMessageDialog, ::MessageDialog>
{
    std::unique_ptr<JSDialogSender> m_pOwnedSender;

public:
    JSMessageDialog(JSDialogSender* pSender, ::MessageDialog* pDialog, SalInstanceBuilder* pBuilder,
                    bool bTakeOwnership);
    JSMessageDialog(::MessageDialog* pDialog, SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual void set_primary_text(const OUString& rText) override;

    virtual void set_secondary_text(const OUString& rText) override;
};

class JSCheckButton : public JSWidget<SalInstanceCheckButton, ::CheckBox>
{
public:
    JSCheckButton(JSDialogSender* pSender, ::CheckBox* pCheckBox, SalInstanceBuilder* pBuilder,
                  bool bTakeOwnership);

    virtual void set_active(bool active) override;
};

class JSDrawingArea : public JSWidget<SalInstanceDrawingArea, VclDrawingArea>
{
public:
    JSDrawingArea(JSDialogSender* pSender, VclDrawingArea* pDrawingArea,
                  SalInstanceBuilder* pBuilder, const a11yref& rAlly,
                  FactoryFunction pUITestFactoryFunction, void* pUserData);

    virtual void queue_draw() override;
    virtual void queue_draw_area(int x, int y, int width, int height) override;
};

class JSToolbar : public JSWidget<SalInstanceToolbar, ::ToolBox>
{
public:
    JSToolbar(JSDialogSender* pSender, ::ToolBox* pToolbox, SalInstanceBuilder* pBuilder,
              bool bTakeOwnership);
};

class JSTextView : public JSWidget<SalInstanceTextView, ::VclMultiLineEdit>
{
public:
    JSTextView(JSDialogSender* pSender, ::VclMultiLineEdit* pTextView, SalInstanceBuilder* pBuilder,
               bool bTakeOwnership);
    virtual void set_text(const OUString& rText) override;
};

class JSTreeView : public JSWidget<SalInstanceTreeView, ::SvTabListBox>
{
public:
    JSTreeView(JSDialogSender* pSender, ::SvTabListBox* pTextView, SalInstanceBuilder* pBuilder,
               bool bTakeOwnership);

    using SalInstanceTreeView::set_toggle;
    /// pos is used differently here, it defines how many steps of iterator we need to perform to take entry
    virtual void set_toggle(int pos, TriState eState, int col = -1) override;

    using SalInstanceTreeView::select;
    /// pos is used differently here, it defines how many steps of iterator we need to perform to take entry
    virtual void select(int pos) override;

    virtual weld::TreeView* get_drag_source() const override;

    using SalInstanceTreeView::insert;
    virtual void insert(const weld::TreeIter* pParent, int pos, const OUString* pStr,
                        const OUString* pId, const OUString* pIconName,
                        VirtualDevice* pImageSurface, bool bChildrenOnDemand,
                        weld::TreeIter* pRet) override;

    virtual void set_text(int row, const OUString& rText, int col = -1) override;
    virtual void set_text(const weld::TreeIter& rIter, const OUString& rStr, int col = -1) override;

    virtual void expand_row(const weld::TreeIter& rIter) override;
    virtual void collapse_row(const weld::TreeIter& rIter) override;

    void drag_start();
    void drag_end();
};

class JSExpander : public JSWidget<SalInstanceExpander, ::VclExpander>
{
public:
    JSExpander(JSDialogSender* pSender, ::VclExpander* pExpander, SalInstanceBuilder* pBuilder,
               bool bTakeOwnership);

    virtual void set_expanded(bool bExpand) override;
};

class JSIconView : public JSWidget<SalInstanceIconView, ::IconView>
{
public:
    JSIconView(JSDialogSender* pSender, ::IconView* pIconView, SalInstanceBuilder* pBuilder,
               bool bTakeOwnership);

    virtual void insert(int pos, const OUString* pStr, const OUString* pId,
                        const OUString* pIconName, weld::TreeIter* pRet) override;

    virtual void insert(int pos, const OUString* pStr, const OUString* pId,
                        const VirtualDevice* pIcon, weld::TreeIter* pRet) override;
    virtual void clear() override;
    virtual void select(int pos) override;
    virtual void unselect(int pos) override;
};

class JSRadioButton : public JSWidget<SalInstanceRadioButton, ::RadioButton>
{
public:
    JSRadioButton(JSDialogSender* pSender, ::RadioButton* pRadioButton,
                  SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual void set_active(bool active) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
