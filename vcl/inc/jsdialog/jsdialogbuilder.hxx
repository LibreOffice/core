/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <comphelper/string.hxx>
#include <osl/mutex.hxx>
#include <vcl/weld.hxx>
#include <vcl/jsdialog/executor.hxx>
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
#include <unordered_map>

#define ACTION_TYPE "action_type"
#define PARENT_ID "parent_id"
#define WINDOW_ID "id"
#define CLOSE_ID "close_id"

class ToolBox;
class ComboBox;
class VclMultiLineEdit;
class SvTabListBox;
class IconView;

typedef std::map<OString, weld::Widget*> WidgetMap;
typedef std::unordered_map<std::string, OUString> ActionDataMap;

namespace jsdialog
{
enum MessageType
{
    FullUpdate,
    WidgetUpdate,
    Close,
    Action,
    Popup,
    PopupClose
};
}

/// Class with the message description for storing in the queue
class JSDialogMessageInfo
{
public:
    jsdialog::MessageType m_eType;
    VclPtr<vcl::Window> m_pWindow;
    std::unique_ptr<ActionDataMap> m_pData;

private:
    void copy(const JSDialogMessageInfo& rInfo)
    {
        this->m_eType = rInfo.m_eType;
        this->m_pWindow = rInfo.m_pWindow;
        if (rInfo.m_pData)
        {
            std::unique_ptr<ActionDataMap> pData(new ActionDataMap(*rInfo.m_pData));
            this->m_pData = std::move(pData);
        }
    }

public:
    JSDialogMessageInfo(jsdialog::MessageType eType, VclPtr<vcl::Window> pWindow,
                        std::unique_ptr<ActionDataMap> pData)
        : m_eType(eType)
        , m_pWindow(pWindow)
        , m_pData(std::move(pData))
    {
    }

    JSDialogMessageInfo(const JSDialogMessageInfo& rInfo) { copy(rInfo); }

    JSDialogMessageInfo& operator=(JSDialogMessageInfo aInfo)
    {
        if (this == &aInfo)
            return *this;

        copy(aInfo);
        return *this;
    }
};

class JSDialogNotifyIdle : public Idle
{
    // used to send message
    VclPtr<vcl::Window> m_aNotifierWindow;
    // used to generate JSON
    VclPtr<vcl::Window> m_aContentWindow;
    std::string m_sTypeOfJSON;
    std::string m_LastNotificationMessage;
    bool m_bForce;

    std::deque<JSDialogMessageInfo> m_aMessageQueue;
    osl::Mutex m_aQueueMutex;

public:
    JSDialogNotifyIdle(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                       std::string sTypeOfJSON);

    void Invoke() override;

    void clearQueue();
    void forceUpdate();
    void sendMessage(jsdialog::MessageType eType, VclPtr<vcl::Window> pWindow,
                     std::unique_ptr<ActionDataMap> pData = nullptr);

private:
    void send(tools::JsonWriter& aJsonWriter);
    std::unique_ptr<tools::JsonWriter> generateFullUpdate() const;
    std::unique_ptr<tools::JsonWriter> generateWidgetUpdate(VclPtr<vcl::Window> pWindow) const;
    std::unique_ptr<tools::JsonWriter> generateCloseMessage() const;
    std::unique_ptr<tools::JsonWriter>
    generateActionMessage(VclPtr<vcl::Window> pWindow, std::unique_ptr<ActionDataMap> pData) const;
    std::unique_ptr<tools::JsonWriter>
    generatePopupMessage(VclPtr<vcl::Window> pWindow, OUString sParentId, OUString sCloseId) const;
    std::unique_ptr<tools::JsonWriter> generateClosePopupMessage(OUString sWindowId) const;
};

class JSDialogSender
{
    std::unique_ptr<JSDialogNotifyIdle> mpIdleNotify;

protected:
    bool m_bCanClose; // specifies if can send a close message

public:
    JSDialogSender()
        : m_bCanClose(true)
    {
    }
    JSDialogSender(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                   std::string sTypeOfJSON)
        : m_bCanClose(true)
    {
        initializeSender(aNotifierWindow, aContentWindow, sTypeOfJSON);
    }

    virtual ~JSDialogSender();

    virtual void sendFullUpdate(bool bForce = false);
    void sendClose();
    virtual void sendUpdate(VclPtr<vcl::Window> pWindow, bool bForce = false);
    virtual void sendAction(VclPtr<vcl::Window> pWindow, std::unique_ptr<ActionDataMap> pData);
    virtual void sendPopup(VclPtr<vcl::Window> pWindow, OUString sParentId, OUString sCloseId);
    virtual void sendClosePopup(vcl::LOKWindowId nWindowId);
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
    /// When LOKNotifier is set by jsdialogs code we need to release it
    VclPtr<vcl::Window> m_aWindowToRelease;

    friend VCL_DLLPUBLIC bool jsdialog::ExecuteAction(const std::string& nWindowId,
                                                      const OString& rWidget, StringMap& rData);
    friend VCL_DLLPUBLIC void jsdialog::SendFullUpdate(const std::string& nWindowId,
                                                       const OString& rWidget);

    static std::map<std::string, WidgetMap>& GetLOKWeldWidgetsMap();
    static void InsertWindowToMap(const std::string& nWindowId);
    void RememberWidget(const OString& id, weld::Widget* pWidget);
    static void RememberWidget(const std::string& nWindowId, const OString& id,
                               weld::Widget* pWidget);
    static weld::Widget* FindWeldWidgetsMap(const std::string& nWindowId, const OString& rWidget);

    std::string getMapIdFromWindowId() const;

    /// used for dialogs or popups
    JSInstanceBuilder(weld::Widget* pParent, const OUString& rUIRoot, const OUString& rUIFile,
                      bool bPopup = false);
    /// used for sidebar panels
    JSInstanceBuilder(weld::Widget* pParent, const OUString& rUIRoot, const OUString& rUIFile,
                      sal_uInt64 nLOKWindowId);
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
    static JSInstanceBuilder* CreateSidebarBuilder(weld::Widget* pParent, const OUString& rUIRoot,
                                                   const OUString& rUIFile,
                                                   sal_uInt64 nLOKWindowId = 0);
    static JSInstanceBuilder* CreatePopupBuilder(weld::Widget* pParent, const OUString& rUIRoot,
                                                 const OUString& rUIFile);

    virtual ~JSInstanceBuilder() override;
    virtual std::unique_ptr<weld::MessageDialog> weld_message_dialog(const OString& id) override;
    virtual std::unique_ptr<weld::Dialog> weld_dialog(const OString& id) override;
    virtual std::unique_ptr<weld::Container> weld_container(const OString& id) override;
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
    virtual std::unique_ptr<weld::Frame> weld_frame(const OString& id) override;
    virtual std::unique_ptr<weld::MenuButton> weld_menu_button(const OString& id) override;
    virtual std::unique_ptr<weld::Popover> weld_popover(const OString& id) override;
    virtual std::unique_ptr<weld::Box> weld_box(const OString& id) override;
    virtual std::unique_ptr<weld::Widget> weld_widget(const OString& id) override;
    virtual std::unique_ptr<weld::Image> weld_image(const OString& id) override;

    static weld::MessageDialog* CreateMessageDialog(weld::Widget* pParent,
                                                    VclMessageType eMessageType,
                                                    VclButtonsType eButtonType,
                                                    const OUString& rPrimaryMessage);

    static void AddChildWidget(const std::string& nWindowId, const OString& id,
                               weld::Widget* pWidget);
    static void RemoveWindowWidget(const std::string& nWindowId);

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

    virtual void sendAction(std::unique_ptr<ActionDataMap> pData) = 0;

    virtual void sendPopup(vcl::Window* pPopup, OUString sParentId, OUString sCloseId) = 0;

    virtual void sendClosePopup(vcl::LOKWindowId nWindowId) = 0;
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
        bool bWasVisible = BaseInstanceClass::get_visible();
        BaseInstanceClass::show();
        if (!bWasVisible)
        {
            std::unique_ptr<ActionDataMap> pMap = std::make_unique<ActionDataMap>();
            (*pMap)[ACTION_TYPE] = "show";
            sendAction(std::move(pMap));
        }
    }

    virtual void hide() override
    {
        bool bWasVisible = BaseInstanceClass::get_visible();
        BaseInstanceClass::hide();
        if (bWasVisible)
        {
            std::unique_ptr<ActionDataMap> pMap = std::make_unique<ActionDataMap>();
            (*pMap)[ACTION_TYPE] = "hide";
            sendAction(std::move(pMap));
        }
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

        return m_xDropTarget.get();
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
        sendUpdate();
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

    virtual void sendAction(std::unique_ptr<ActionDataMap> pData) override
    {
        if (!m_bIsFreezed && m_pSender && pData)
            m_pSender->sendAction(BaseInstanceClass::m_xWidget, std::move(pData));
    }

    virtual void sendPopup(vcl::Window* pPopup, OUString sParentId, OUString sCloseId) override
    {
        if (!m_bIsFreezed && m_pSender)
            m_pSender->sendPopup(pPopup, sParentId, sCloseId);
    }

    virtual void sendClosePopup(vcl::LOKWindowId nWindowId) override
    {
        if (!m_bIsFreezed && m_pSender)
            m_pSender->sendClosePopup(nWindowId);
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

class JSContainer : public JSWidget<SalInstanceContainer, vcl::Window>
{
public:
    JSContainer(JSDialogSender* pSender, vcl::Window* pContainer, SalInstanceBuilder* pBuilder,
                bool bTakeOwnership);
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
    virtual void set_entry_text_without_notify(const OUString& rText);
    virtual void set_entry_text(const OUString& rText) override;
    virtual void set_active(int pos) override;
    virtual bool changed_by_direct_pick() const override;
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
    std::unique_ptr<JSButton> m_pOK;
    std::unique_ptr<JSButton> m_pCancel;

    // used for message dialogs created using static functions
    std::string m_sWindowId;

    DECL_LINK(OKHdl, weld::Button&, void);
    DECL_LINK(CancelHdl, weld::Button&, void);

public:
    JSMessageDialog(JSDialogSender* pSender, ::MessageDialog* pDialog, SalInstanceBuilder* pBuilder,
                    bool bTakeOwnership);
    JSMessageDialog(::MessageDialog* pDialog, SalInstanceBuilder* pBuilder, bool bTakeOwnership);
    virtual ~JSMessageDialog();

    virtual void set_primary_text(const OUString& rText) override;

    virtual void set_secondary_text(const OUString& rText) override;

    virtual void response(int response) override;
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
    std::map<sal_uInt16, weld::Widget*> m_pPopovers;

public:
    JSToolbar(JSDialogSender* pSender, ::ToolBox* pToolbox, SalInstanceBuilder* pBuilder,
              bool bTakeOwnership);

    virtual void set_menu_item_active(const OString& rIdent, bool bActive) override;
    virtual void set_item_sensitive(const OString& rIdent, bool bSensitive) override;
    virtual void set_item_icon_name(const OString& rIdent, const OUString& rIconName) override;
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
    virtual void set_toggle(const weld::TreeIter& rIter, TriState bOn, int col = -1) override;

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

    using SalInstanceTreeView::remove;
    virtual void remove(int pos) override;
    virtual void remove(const weld::TreeIter& rIter) override;

    virtual void clear() override;

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

class JSFrame : public JSWidget<SalInstanceFrame, ::VclFrame>
{
public:
    JSFrame(JSDialogSender* pSender, ::VclFrame* pFrame, SalInstanceBuilder* pBuilder,
            bool bTakeOwnership);
};

class JSMenuButton : public JSWidget<SalInstanceMenuButton, ::MenuButton>
{
public:
    JSMenuButton(JSDialogSender* pSender, ::MenuButton* pMenuButton, SalInstanceBuilder* pBuilder,
                 bool bTakeOwnership);

    virtual void set_label(const OUString& rText) override;
    virtual void set_image(VirtualDevice* pDevice) override;
    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage) override;
    virtual void set_active(bool active) override;
};

class JSPopover : public JSWidget<SalInstancePopover, DockingWindow>
{
public:
    JSPopover(JSDialogSender* pSender, DockingWindow* pPopover, SalInstanceBuilder* pBuilder,
              bool bTakeOwnership);

    virtual void popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect) override;
    virtual void popdown() override;
};

class JSBox : public JSWidget<SalInstanceBox, VclBox>
{
public:
    JSBox(JSDialogSender* pSender, VclBox* pBox, SalInstanceBuilder* pBuilder, bool bTakeOwnership);
};

class JSWidgetInstance : public JSWidget<SalInstanceWidget, vcl::Window>
{
public:
    JSWidgetInstance(JSDialogSender* pSender, vcl::Window* pObject, SalInstanceBuilder* pBuilder,
                     bool bTakeOwnership)
        : JSWidget<SalInstanceWidget, vcl::Window>(pSender, pObject, pBuilder, bTakeOwnership)
    {
    }
};

class JSImage : public JSWidget<SalInstanceImage, FixedImage>
{
public:
    JSImage(JSDialogSender* pSender, FixedImage* pImage, SalInstanceBuilder* pBuilder,
            bool bTakeOwnership);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
