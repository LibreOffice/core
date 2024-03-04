/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <utility>
#include <vcl/weld.hxx>
#include <vcl/jsdialog/executor.hxx>
#include <vcl/virdev.hxx>
#include <salvtables.hxx>
#include <vcl/toolkit/button.hxx>
#include <vcl/toolkit/fmtfield.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <comphelper/compbase.hxx>

#include <deque>
#include <list>
#include <mutex>

#define ACTION_TYPE "action_type"
#define PARENT_ID "parent_id"
#define WINDOW_ID "id"
#define CLOSE_ID "close_id"

class ToolBox;
class ComboBox;
class VclMultiLineEdit;
class SvTabListBox;
class IconView;
class VclScrolledWindow;

namespace vcl
{
class ILibreOfficeKitNotifier;
}

typedef std::map<OUString, weld::Widget*> WidgetMap;

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
    std::unique_ptr<jsdialog::ActionDataMap> m_pData;

private:
    void copy(const JSDialogMessageInfo& rInfo)
    {
        this->m_eType = rInfo.m_eType;
        this->m_pWindow = rInfo.m_pWindow;
        if (rInfo.m_pData)
        {
            std::unique_ptr<jsdialog::ActionDataMap> pData(
                new jsdialog::ActionDataMap(*rInfo.m_pData));
            this->m_pData = std::move(pData);
        }
    }

public:
    JSDialogMessageInfo(jsdialog::MessageType eType, VclPtr<vcl::Window> pWindow,
                        std::unique_ptr<jsdialog::ActionDataMap> pData)
        : m_eType(eType)
        , m_pWindow(std::move(pWindow))
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

class JSDialogNotifyIdle final : public Idle
{
    // used to send message
    VclPtr<vcl::Window> m_aNotifierWindow;
    // used to generate JSON
    VclPtr<vcl::Window> m_aContentWindow;
    OUString m_sTypeOfJSON;
    OString m_LastNotificationMessage;
    bool m_bForce;

    std::deque<JSDialogMessageInfo> m_aMessageQueue;
    std::mutex m_aQueueMutex;

public:
    JSDialogNotifyIdle(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                       const OUString& sTypeOfJSON);

    void Invoke() override;

    void clearQueue();
    void forceUpdate();
    void sendMessage(jsdialog::MessageType eType, VclPtr<vcl::Window> pWindow,
                     std::unique_ptr<jsdialog::ActionDataMap> pData = nullptr);

private:
    void send(const OString& sMsg);
    OString generateFullUpdate() const;
    OString generateWidgetUpdate(VclPtr<vcl::Window> pWindow) const;
    OString generateCloseMessage() const;
    OString generateActionMessage(VclPtr<vcl::Window> pWindow,
                                  std::unique_ptr<jsdialog::ActionDataMap> pData) const;
    OString generatePopupMessage(VclPtr<vcl::Window> pWindow, OUString sParentId,
                                 OUString sCloseId) const;
    OString generateClosePopupMessage(OUString sWindowId) const;
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
                   const OUString& sTypeOfJSON)
        : m_bCanClose(true)
    {
        initializeSender(aNotifierWindow, aContentWindow, sTypeOfJSON);
    }

    virtual ~JSDialogSender() COVERITY_NOEXCEPT_FALSE;

    virtual void sendFullUpdate(bool bForce = false);
    void sendClose();
    void sendUpdate(VclPtr<vcl::Window> pWindow, bool bForce = false);
    virtual void sendAction(VclPtr<vcl::Window> pWindow,
                            std::unique_ptr<jsdialog::ActionDataMap> pData);
    virtual void sendPopup(VclPtr<vcl::Window> pWindow, OUString sParentId, OUString sCloseId);
    virtual void sendClosePopup(vcl::LOKWindowId nWindowId);
    void flush() { mpIdleNotify->Invoke(); }

protected:
    void initializeSender(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                          const OUString& sTypeOfJSON)
    {
        mpIdleNotify.reset(new JSDialogNotifyIdle(aNotifierWindow, aContentWindow, sTypeOfJSON));
    }
};

class JSDropTarget final
    : public comphelper::WeakComponentImplHelper<
          css::datatransfer::dnd::XDropTarget, css::lang::XInitialization, css::lang::XServiceInfo>
{
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

class JSInstanceBuilder final : public SalInstanceBuilder, public JSDialogSender
{
    sal_uInt64 m_nWindowId;
    /// used in case of tab pages where dialog is not a direct top level
    VclPtr<vcl::Window> m_aParentDialog;
    VclPtr<vcl::Window> m_aContentWindow;
    std::list<OUString> m_aRememberedWidgets;
    OUString m_sTypeOfJSON;
    bool m_bHasTopLevelDialog;
    bool m_bIsNotebookbar;
    /// used to detect when we have to send Full Update in container handler
    bool m_bSentInitialUpdate;
    /// is true for tabpages, prevents from closing parent window on destroy
    bool m_bIsNestedBuilder;
    /// When LOKNotifier is set by jsdialogs code we need to release it
    VclPtr<vcl::Window> m_aWindowToRelease;

    friend class JSMessageDialog; // static message boxes have to be registered outside
    friend class JSDialog;
    friend class JSAssistant;

    friend VCL_DLLPUBLIC bool jsdialog::ExecuteAction(const OUString& nWindowId,
                                                      const OUString& rWidget, StringMap& rData);
    friend VCL_DLLPUBLIC void jsdialog::SendFullUpdate(const OUString& nWindowId,
                                                       const OUString& rWidget);
    friend VCL_DLLPUBLIC void jsdialog::SendAction(const OUString& nWindowId,
                                                   const OUString& rWidget,
                                                   std::unique_ptr<jsdialog::ActionDataMap> pData);

    static std::map<OUString, WidgetMap>& GetLOKWeldWidgetsMap();
    static void InsertWindowToMap(const OUString& nWindowId);
    void RememberWidget(OUString id, weld::Widget* pWidget);
    static void RememberWidget(const OUString& nWindowId, const OUString& id,
                               weld::Widget* pWidget);
    static weld::Widget* FindWeldWidgetsMap(const OUString& nWindowId, const OUString& rWidget);

    OUString getMapIdFromWindowId() const;

public:
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
    /// used for formulabar
    JSInstanceBuilder(vcl::Window* pParent, const OUString& rUIRoot, const OUString& rUIFile,
                      sal_uInt64 nLOKWindowId);

    static std::unique_ptr<JSInstanceBuilder>
    CreateDialogBuilder(weld::Widget* pParent, const OUString& rUIRoot, const OUString& rUIFile);
    static std::unique_ptr<JSInstanceBuilder>
    CreateNotebookbarBuilder(vcl::Window* pParent, const OUString& rUIRoot, const OUString& rUIFile,
                             const css::uno::Reference<css::frame::XFrame>& rFrame,
                             sal_uInt64 nWindowId = 0);
    static std::unique_ptr<JSInstanceBuilder> CreateSidebarBuilder(weld::Widget* pParent,
                                                                   const OUString& rUIRoot,
                                                                   const OUString& rUIFile,
                                                                   sal_uInt64 nLOKWindowId = 0);
    static std::unique_ptr<JSInstanceBuilder>
    CreatePopupBuilder(weld::Widget* pParent, const OUString& rUIRoot, const OUString& rUIFile);
    static std::unique_ptr<JSInstanceBuilder> CreateFormulabarBuilder(vcl::Window* pParent,
                                                                      const OUString& rUIRoot,
                                                                      const OUString& rUIFile,
                                                                      sal_uInt64 nLOKWindowId);

    virtual ~JSInstanceBuilder() override;
    virtual std::unique_ptr<weld::MessageDialog> weld_message_dialog(const OUString& id) override;
    virtual std::unique_ptr<weld::Dialog> weld_dialog(const OUString& id) override;
    virtual std::unique_ptr<weld::Assistant> weld_assistant(const OUString& id) override;
    virtual std::unique_ptr<weld::Container> weld_container(const OUString& id) override;
    virtual std::unique_ptr<weld::Label> weld_label(const OUString& id) override;
    virtual std::unique_ptr<weld::Button> weld_button(const OUString& id) override;
    virtual std::unique_ptr<weld::LinkButton> weld_link_button(const OUString& id) override;
    virtual std::unique_ptr<weld::ToggleButton> weld_toggle_button(const OUString& id) override;
    virtual std::unique_ptr<weld::Entry> weld_entry(const OUString& id) override;
    virtual std::unique_ptr<weld::ComboBox> weld_combo_box(const OUString& id) override;
    virtual std::unique_ptr<weld::Notebook> weld_notebook(const OUString& id) override;
    virtual std::unique_ptr<weld::SpinButton> weld_spin_button(const OUString& id) override;
    virtual std::unique_ptr<weld::FormattedSpinButton>
    weld_formatted_spin_button(const OUString& id) override;
    virtual std::unique_ptr<weld::CheckButton> weld_check_button(const OUString& id) override;
    virtual std::unique_ptr<weld::DrawingArea>
    weld_drawing_area(const OUString& id, const a11yref& rA11yImpl = nullptr,
                      FactoryFunction pUITestFactoryFunction = nullptr,
                      void* pUserData = nullptr) override;
    virtual std::unique_ptr<weld::Toolbar> weld_toolbar(const OUString& id) override;
    virtual std::unique_ptr<weld::TextView> weld_text_view(const OUString& id) override;
    virtual std::unique_ptr<weld::TreeView> weld_tree_view(const OUString& id) override;
    virtual std::unique_ptr<weld::Expander> weld_expander(const OUString& id) override;
    virtual std::unique_ptr<weld::IconView> weld_icon_view(const OUString& id) override;
    virtual std::unique_ptr<weld::ScrolledWindow>
    weld_scrolled_window(const OUString& id, bool bUserManagedScrolling = false) override;
    virtual std::unique_ptr<weld::RadioButton> weld_radio_button(const OUString& id) override;
    virtual std::unique_ptr<weld::Frame> weld_frame(const OUString& id) override;
    virtual std::unique_ptr<weld::MenuButton> weld_menu_button(const OUString& id) override;
    virtual std::unique_ptr<weld::Popover> weld_popover(const OUString& id) override;
    virtual std::unique_ptr<weld::Box> weld_box(const OUString& id) override;
    virtual std::unique_ptr<weld::Widget> weld_widget(const OUString& id) override;
    virtual std::unique_ptr<weld::Image> weld_image(const OUString& id) override;
    virtual std::unique_ptr<weld::Calendar> weld_calendar(const OUString& id) override;

    static weld::MessageDialog*
    CreateMessageDialog(weld::Widget* pParent, VclMessageType eMessageType,
                        VclButtonsType eButtonType, const OUString& rPrimaryMessage,
                        const vcl::ILibreOfficeKitNotifier* pNotifier = nullptr);

    static void AddChildWidget(const OUString& nWindowId, const OUString& id,
                               weld::Widget* pWidget);
    static void RemoveWindowWidget(const OUString& nWindowId);

    // we need to remember original popup window to close it properly (its handled by vcl)
    static void RememberPopup(const OUString& nWindowId, VclPtr<vcl::Window> pWidget);
    static void ForgetPopup(const OUString& nWindowId);
    static vcl::Window* FindPopup(const OUString& nWindowId);

private:
    const OUString& GetTypeOfJSON() const;
    VclPtr<vcl::Window>& GetContentWindow();
    VclPtr<vcl::Window>& GetNotifierWindow();
};

class SAL_LOPLUGIN_ANNOTATE("crosscast") BaseJSWidget
{
public:
    virtual ~BaseJSWidget() = default;

    virtual void sendClose() = 0;

    virtual void sendUpdate(bool bForce = false) = 0;

    virtual void sendFullUpdate(bool bForce = false) = 0;

    virtual void sendAction(std::unique_ptr<jsdialog::ActionDataMap> pData) = 0;

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
             bool bTakeOwnership, bool bUserManagedScrolling)
        : BaseInstanceClass(pObject, pBuilder, bTakeOwnership, bUserManagedScrolling)
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
            std::unique_ptr<jsdialog::ActionDataMap> pMap
                = std::make_unique<jsdialog::ActionDataMap>();
            (*pMap)[ACTION_TYPE ""_ostr] = "show";
            sendAction(std::move(pMap));
        }
    }

    virtual void hide() override
    {
        bool bWasVisible = BaseInstanceClass::get_visible();
        BaseInstanceClass::hide();
        if (bWasVisible)
        {
            std::unique_ptr<jsdialog::ActionDataMap> pMap
                = std::make_unique<jsdialog::ActionDataMap>();
            (*pMap)[ACTION_TYPE ""_ostr] = "hide";
            sendAction(std::move(pMap));
        }
    }

    using BaseInstanceClass::set_sensitive;
    virtual void set_sensitive(bool sensitive) override
    {
        bool bIsSensitive = BaseInstanceClass::get_sensitive();
        BaseInstanceClass::set_sensitive(sensitive);
        if (bIsSensitive != sensitive)
        {
            std::unique_ptr<jsdialog::ActionDataMap> pMap
                = std::make_unique<jsdialog::ActionDataMap>();
            (*pMap)[ACTION_TYPE ""_ostr] = (sensitive ? u"enable" : u"disable");
            sendAction(std::move(pMap));
        }
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
        sendUpdate();
    }

    virtual void grab_focus() override
    {
        BaseInstanceClass::grab_focus();
        std::unique_ptr<jsdialog::ActionDataMap> pMap = std::make_unique<jsdialog::ActionDataMap>();
        (*pMap)[ACTION_TYPE ""_ostr] = "grab_focus";
        sendAction(std::move(pMap));
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

    virtual void sendAction(std::unique_ptr<jsdialog::ActionDataMap> pData) override
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

    virtual void set_buildable_name(const OUString& rName) override
    {
        SalInstanceWidget::set_buildable_name(rName);
        assert(false); // we remember old name in GetLOKWeldWidgetsMap()
        // TODO: implement renaming or avoid it for LOK
    }
};

class JSDialog final : public JSWidget<SalInstanceDialog, ::Dialog>
{
public:
    JSDialog(JSDialogSender* pSender, ::Dialog* pDialog, SalInstanceBuilder* pBuilder,
             bool bTakeOwnership);

    virtual void collapse(weld::Widget* pEdit, weld::Widget* pButton) override;
    virtual void undo_collapse() override;
    virtual void response(int response) override;
    virtual weld::Button* weld_widget_for_response(int response) override;
    virtual int run() override;
    virtual bool runAsync(std::shared_ptr<weld::DialogController> aOwner,
                          const std::function<void(sal_Int32)>& rEndDialogFn) override;
    virtual bool runAsync(std::shared_ptr<Dialog> const& rxSelf,
                          const std::function<void(sal_Int32)>& func) override;
};

class JSAssistant final : public JSWidget<SalInstanceAssistant, vcl::RoadmapWizard>
{
public:
    JSAssistant(JSDialogSender* pSender, vcl::RoadmapWizard* pDialog, SalInstanceBuilder* pBuilder,
                bool bTakeOwnership);

    virtual void set_current_page(int nPage) override;
    virtual void set_current_page(const OUString& rIdent) override;
    virtual void response(int response) override;
    virtual weld::Button* weld_widget_for_response(int response) override;
    virtual int run() override;
    virtual bool runAsync(std::shared_ptr<weld::DialogController> aOwner,
                          const std::function<void(sal_Int32)>& rEndDialogFn) override;
    virtual bool runAsync(std::shared_ptr<Dialog> const& rxSelf,
                          const std::function<void(sal_Int32)>& func) override;
};

class JSContainer final : public JSWidget<SalInstanceContainer, vcl::Window>
{
public:
    JSContainer(JSDialogSender* pSender, vcl::Window* pContainer, SalInstanceBuilder* pBuilder,
                bool bTakeOwnership);
};

class JSScrolledWindow final : public JSWidget<SalInstanceScrolledWindow, ::VclScrolledWindow>
{
public:
    JSScrolledWindow(JSDialogSender* pSender, ::VclScrolledWindow* pWindow,
                     SalInstanceBuilder* pBuilder, bool bTakeOwnership, bool bUserManagedScrolling);

    virtual void vadjustment_configure(int value, int lower, int upper, int step_increment,
                                       int page_increment, int page_size) override;
    virtual void vadjustment_set_value(int value) override;
    void vadjustment_set_value_no_notification(int value);
    virtual void vadjustment_set_page_size(int size) override;
    virtual void set_vpolicy(VclPolicyType eVPolicy) override;

    virtual void hadjustment_configure(int value, int lower, int upper, int step_increment,
                                       int page_increment, int page_size) override;
    virtual void hadjustment_set_value(int value) override;
    void hadjustment_set_value_no_notification(int value);
    virtual void hadjustment_set_page_size(int size) override;
    virtual void set_hpolicy(VclPolicyType eVPolicy) override;
};

class JSLabel final : public JSWidget<SalInstanceLabel, Control>
{
public:
    JSLabel(JSDialogSender* pSender, Control* pLabel, SalInstanceBuilder* pBuilder,
            bool bTakeOwnership);
    virtual void set_label(const OUString& rText) override;
};

class JSButton final : public JSWidget<SalInstanceButton, ::Button>
{
public:
    JSButton(JSDialogSender* pSender, ::Button* pButton, SalInstanceBuilder* pBuilder,
             bool bTakeOwnership);
};

class JSLinkButton final : public JSWidget<SalInstanceLinkButton, ::FixedHyperlink>
{
public:
    JSLinkButton(JSDialogSender* pSender, ::FixedHyperlink* pButton, SalInstanceBuilder* pBuilder,
                 bool bTakeOwnership);
};

class JSToggleButton final : public JSWidget<SalInstanceToggleButton, ::PushButton>
{
public:
    JSToggleButton(JSDialogSender* pSender, ::PushButton* pButton, SalInstanceBuilder* pBuilder,
                   bool bTakeOwnership);
};

class JSEntry final : public JSWidget<SalInstanceEntry, ::Edit>
{
public:
    JSEntry(JSDialogSender* pSender, ::Edit* pEntry, SalInstanceBuilder* pBuilder,
            bool bTakeOwnership);
    virtual void set_text(const OUString& rText) override;
    void set_text_without_notify(const OUString& rText);
    virtual void replace_selection(const OUString& rText) override;
};

class JSListBox final : public JSWidget<SalInstanceComboBoxWithoutEdit, ::ListBox>
{
public:
    JSListBox(JSDialogSender* pSender, ::ListBox* pListBox, SalInstanceBuilder* pBuilder,
              bool bTakeOwnership);
    virtual void insert(int pos, const OUString& rStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface) override;
    virtual void remove(int pos) override;
    virtual void set_active(int pos) override;
};

class JSComboBox final : public JSWidget<SalInstanceComboBoxWithEdit, ::ComboBox>
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
    virtual void set_active_id(const OUString& rText) override;
    virtual bool changed_by_direct_pick() const override;

    void render_entry(int pos, int dpix, int dpiy);
};

class JSNotebook final : public JSWidget<SalInstanceNotebook, ::TabControl>
{
public:
    JSNotebook(JSDialogSender* pSender, ::TabControl* pControl, SalInstanceBuilder* pBuilder,
               bool bTakeOwnership);

    virtual void remove_page(const OUString& rIdent) override;
    virtual void insert_page(const OUString& rIdent, const OUString& rLabel, int nPos) override;
};

class JSVerticalNotebook final : public JSWidget<SalInstanceVerticalNotebook, ::VerticalTabControl>
{
public:
    JSVerticalNotebook(JSDialogSender* pSender, ::VerticalTabControl* pControl,
                       SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual void remove_page(const OUString& rIdent) override;
    virtual void insert_page(const OUString& rIdent, const OUString& rLabel, int nPos) override;
};

class JSSpinButton final : public JSWidget<SalInstanceSpinButton, ::FormattedField>
{
public:
    JSSpinButton(JSDialogSender* pSender, ::FormattedField* pSpin, SalInstanceBuilder* pBuilder,
                 bool bTakeOwnership);

    virtual void set_value(sal_Int64 value) override;
};

class JSFormattedSpinButton final
    : public JSWidget<SalInstanceFormattedSpinButton, ::FormattedField>
{
public:
    JSFormattedSpinButton(JSDialogSender* pSender, ::FormattedField* pSpin,
                          SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual void set_text(const OUString& rText) override;
    void set_text_without_notify(const OUString& rText);
};

class JSMessageDialog final : public JSWidget<SalInstanceMessageDialog, ::MessageDialog>
{
    std::unique_ptr<JSDialogSender> m_pOwnedSender;
    std::unique_ptr<JSButton> m_pOK;
    std::unique_ptr<JSButton> m_pCancel;

    // used for message dialogs created using static functions
    OUString m_sWindowId;

    DECL_LINK(OKHdl, weld::Button&, void);
    DECL_LINK(CancelHdl, weld::Button&, void);

    void RememberMessageDialog();

public:
    JSMessageDialog(JSDialogSender* pSender, ::MessageDialog* pDialog, SalInstanceBuilder* pBuilder,
                    bool bTakeOwnership);
    JSMessageDialog(::MessageDialog* pDialog, SalInstanceBuilder* pBuilder, bool bTakeOwnership);
    virtual ~JSMessageDialog();

    virtual void set_primary_text(const OUString& rText) override;

    virtual void set_secondary_text(const OUString& rText) override;

    virtual void response(int response) override;

    virtual int run() override;
    // TODO: move to dialog class so we will not send json when built but on run
    bool runAsync(std::shared_ptr<weld::DialogController> aOwner,
                  const std::function<void(sal_Int32)>& rEndDialogFn) override;

    bool runAsync(std::shared_ptr<Dialog> const& rxSelf,
                  const std::function<void(sal_Int32)>& rEndDialogFn) override;
};

class JSCheckButton final : public JSWidget<SalInstanceCheckButton, ::CheckBox>
{
public:
    JSCheckButton(JSDialogSender* pSender, ::CheckBox* pCheckBox, SalInstanceBuilder* pBuilder,
                  bool bTakeOwnership);

    virtual void set_active(bool active) override;
};

class JSDrawingArea final : public JSWidget<SalInstanceDrawingArea, VclDrawingArea>
{
public:
    JSDrawingArea(JSDialogSender* pSender, VclDrawingArea* pDrawingArea,
                  SalInstanceBuilder* pBuilder, const a11yref& rAlly,
                  FactoryFunction pUITestFactoryFunction, void* pUserData);

    virtual void queue_draw() override;
    virtual void queue_draw_area(int x, int y, int width, int height) override;
};

class JSToolbar final : public JSWidget<SalInstanceToolbar, ::ToolBox>
{
    std::map<sal_uInt16, weld::Widget*> m_pPopovers;

public:
    JSToolbar(JSDialogSender* pSender, ::ToolBox* pToolbox, SalInstanceBuilder* pBuilder,
              bool bTakeOwnership);

    virtual void set_menu_item_active(const OUString& rIdent, bool bActive) override;
    virtual void set_item_sensitive(const OUString& rIdent, bool bSensitive) override;
    virtual void set_item_icon_name(const OUString& rIdent, const OUString& rIconName) override;
};

class JSTextView final : public JSWidget<SalInstanceTextView, ::VclMultiLineEdit>
{
public:
    JSTextView(JSDialogSender* pSender, ::VclMultiLineEdit* pTextView, SalInstanceBuilder* pBuilder,
               bool bTakeOwnership);
    virtual void set_text(const OUString& rText) override;
    void set_text_without_notify(const OUString& rText);
    virtual void replace_selection(const OUString& rText) override;
};

class JSTreeView final : public JSWidget<SalInstanceTreeView, ::SvTabListBox>
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

    virtual void set_cursor(const weld::TreeIter& rIter) override;
    void set_cursor_without_notify(const weld::TreeIter& rIter);
    virtual void set_cursor(int pos) override;

    using SalInstanceTreeView::remove;
    virtual void remove(int pos) override;
    virtual void remove(const weld::TreeIter& rIter) override;

    virtual void clear() override;

    void drag_start();
    void drag_end();
};

class JSExpander final : public JSWidget<SalInstanceExpander, ::VclExpander>
{
public:
    JSExpander(JSDialogSender* pSender, ::VclExpander* pExpander, SalInstanceBuilder* pBuilder,
               bool bTakeOwnership);

    virtual void set_expanded(bool bExpand) override;
};

class JSIconView final : public JSWidget<SalInstanceIconView, ::IconView>
{
public:
    JSIconView(JSDialogSender* pSender, ::IconView* pIconView, SalInstanceBuilder* pBuilder,
               bool bTakeOwnership);

    virtual void insert(int pos, const OUString* pStr, const OUString* pId,
                        const OUString* pIconName, weld::TreeIter* pRet) override;

    virtual void insert(int pos, const OUString* pStr, const OUString* pId,
                        const VirtualDevice* pIcon, weld::TreeIter* pRet) override;

    virtual void insert_separator(int pos, const OUString* pId) override;

    virtual void clear() override;
    virtual void select(int pos) override;
    virtual void unselect(int pos) override;
};

class JSRadioButton final : public JSWidget<SalInstanceRadioButton, ::RadioButton>
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
    vcl::LOKWindowId mnWindowId;

public:
    JSPopover(JSDialogSender* pSender, DockingWindow* pPopover, SalInstanceBuilder* pBuilder,
              bool bTakeOwnership);

    virtual void popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect,
                               weld::Placement ePlace = weld::Placement::Under) override;
    virtual void popdown() override;

    void set_window_id(vcl::LOKWindowId nWindowId) { mnWindowId = nWindowId; }
};

class JSBox : public JSWidget<SalInstanceBox, VclBox>
{
public:
    JSBox(JSDialogSender* pSender, VclBox* pBox, SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    void reorder_child(weld::Widget* pWidget, int nNewPosition) override;
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
    virtual void set_image(VirtualDevice* pDevice) override;
    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage) override;
};

class JSCalendar : public JSWidget<SalInstanceCalendar, ::Calendar>
{
public:
    JSCalendar(JSDialogSender* pSender, ::Calendar* pCalendar, SalInstanceBuilder* pBuilder,
               bool bTakeOwnership);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
