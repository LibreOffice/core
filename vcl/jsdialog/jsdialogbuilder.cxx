/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <jsdialog/jsdialogbuilder.hxx>
#include <sal/log.hxx>
#include <comphelper/base64.hxx>
#include <iconview.hxx>
#include <utility>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/toolkit/button.hxx>
#include <vcl/toolkit/combobox.hxx>
#include <vcl/toolkit/dialog.hxx>
#include <vcl/toolkit/treelistentry.hxx>
#include <vcl/toolkit/vclmedit.hxx>
#include <verticaltabctrl.hxx>
#include <messagedialog.hxx>
#include <tools/json_writer.hxx>
#include <memory>
#include <vcl/jsdialog/executor.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <tools/stream.hxx>
#include <vcl/cvtgrf.hxx>
#include <wizdlg.hxx>
#include <jsdialog/enabled.hxx>

namespace
{
void response_help(vcl::Window* pWindow)
{
    ::Dialog* pDialog = dynamic_cast<::Dialog*>(pWindow);
    if (!pDialog)
        return;

    vcl::Window* pButtonWindow = pDialog->get_widget_for_response(RET_HELP);
    ::Button* pButton = dynamic_cast<::Button*>(pButtonWindow);
    if (!pButton)
    {
        // Is it a wizard dialog?
        vcl::RoadmapWizard* pWizard = dynamic_cast<vcl::RoadmapWizard*>(pWindow);
        if (!pWizard || !pWizard->m_pHelp)
            return;
        pWizard->m_pHelp->Click();
        return;
    }

    pButton->Click();
}
}

namespace
{
vcl::Window* extract_sal_widget(weld::Widget* pParent)
{
    SalInstanceWidget* pInstanceWidget = dynamic_cast<SalInstanceWidget*>(pParent);
    return pInstanceWidget ? pInstanceWidget->getWidget() : nullptr;
}
}

// Drag and drop

namespace
{
class JSDropTargetDropContext
    : public cppu::WeakImplHelper<css::datatransfer::dnd::XDropTargetDropContext>
{
public:
    JSDropTargetDropContext() {}

    // XDropTargetDropContext
    virtual void SAL_CALL acceptDrop(sal_Int8 /*dragOperation*/) override {}

    virtual void SAL_CALL rejectDrop() override {}

    virtual void SAL_CALL dropComplete(sal_Bool /*bSuccess*/) override {}
};
}

static JSTreeView* g_DragSource;

JSDropTarget::JSDropTarget() {}

void JSDropTarget::initialize(const css::uno::Sequence<css::uno::Any>& /*rArgs*/) {}

void JSDropTarget::addDropTargetListener(
    const css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>& xListener)
{
    std::unique_lock aGuard(m_aMutex);

    m_aListeners.push_back(xListener);
}

void JSDropTarget::removeDropTargetListener(
    const css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>& xListener)
{
    std::unique_lock aGuard(m_aMutex);

    std::erase(m_aListeners, xListener);
}

sal_Bool JSDropTarget::isActive() { return false; }

void JSDropTarget::setActive(sal_Bool /*active*/) {}

sal_Int8 JSDropTarget::getDefaultActions() { return 0; }

void JSDropTarget::setDefaultActions(sal_Int8 /*actions*/) {}

OUString JSDropTarget::getImplementationName()
{
    return u"com.sun.star.datatransfer.dnd.JSDropTarget"_ustr;
}

sal_Bool JSDropTarget::supportsService(OUString const& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> JSDropTarget::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aRet{ u"com.sun.star.datatransfer.dnd.JSDropTarget"_ustr };
    return aRet;
}

void JSDropTarget::fire_drop(const css::datatransfer::dnd::DropTargetDropEvent& dtde)
{
    std::unique_lock aGuard(m_aMutex);
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(
        m_aListeners);
    aGuard.unlock();

    for (auto const& listener : aListeners)
    {
        listener->drop(dtde);
    }
}

void JSDropTarget::fire_dragEnter(const css::datatransfer::dnd::DropTargetDragEnterEvent& dtde)
{
    std::unique_lock aGuard(m_aMutex);
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(
        m_aListeners);
    aGuard.unlock();

    for (auto const& listener : aListeners)
    {
        listener->dragEnter(dtde);
    }
}

void JSInstanceBuilder::initializeDialogSender()
{
    m_sTypeOfJSON = "dialog";

    vcl::Window* pRoot = m_xBuilder->get_widget_root();

    if (pRoot && pRoot->GetParent())
    {
        m_aParentDialog = pRoot->GetParent()->GetParentWithLOKNotifier();
        if (m_aParentDialog)
            m_nWindowId = m_aParentDialog->GetLOKWindowId();
        InsertWindowToMap(getMapIdFromWindowId());
    }

    initializeSender(GetNotifierWindow(), GetContentWindow(), GetTypeOfJSON());
}

void JSInstanceBuilder::initializePopupSender()
{
    // when it is a popup we initialize sender in weld_popover
    m_sTypeOfJSON = "popup";
}

void JSInstanceBuilder::initializeSidebarSender(sal_uInt64 nLOKWindowId,
                                                const std::u16string_view& rUIFile,
                                                const std::u16string_view& sTypeOfJSON)
{
    m_sTypeOfJSON = sTypeOfJSON;
    m_nWindowId = nLOKWindowId;

    vcl::Window* pRoot = m_xBuilder->get_widget_root();

    m_aParentDialog = pRoot->GetParentWithLOKNotifier();

    bool bIsNavigatorPanel = jsdialog::isBuilderEnabledForNavigator(rUIFile);

    // builder for Panel, PanelLayout, and DockingWindow
    // get SidebarDockingWindow, or SwNavigatorWin as m_aContentWindow
    //    A11YCheckLevel : 12 levels up from pRoot
    //      PanelLayout  : 9 levels up from pRoot
    //      Panel        : 7 levels up from pRoot
    //      DockingWindow: 3 levels up from pRoot
    unsigned nLevelsUp = 100; // limit
    if (bIsNavigatorPanel)
        nLevelsUp = 3;

    if (nLevelsUp > 0)
    {
        // embedded fragments cannot send close message for whole sidebar
        if (rUIFile == u"modules/simpress/ui/customanimationfragment.ui")
            m_bCanClose = false;

        m_aContentWindow = pRoot;
        for (unsigned i = 0; i < nLevelsUp && m_aContentWindow; i++)
        {
            if (!bIsNavigatorPanel && m_aContentWindow->get_id() == "Deck")
                nLevelsUp = i + 3;

            // Useful to check if any panel doesn't appear
            // SAL _ DEBUG ("SIDEBAR CONTENT LOOK UP FROM: " << m_aContentWindow->get_id());
            m_aContentWindow = m_aContentWindow->GetParent();
        }
    }

    InsertWindowToMap(getMapIdFromWindowId());

    initializeSender(GetNotifierWindow(), GetContentWindow(), GetTypeOfJSON());
}

void JSInstanceBuilder::initializeNotebookbarSender(sal_uInt64 nLOKWindowId)
{
    m_sTypeOfJSON = "notebookbar";

    vcl::Window* pRoot = m_xBuilder->get_widget_root();
    if (pRoot && pRoot->GetParent())
    {
        m_aParentDialog = pRoot->GetParent()->GetParentWithLOKNotifier();
        if (m_aParentDialog)
            m_nWindowId = m_aParentDialog->GetLOKWindowId();
        if (!m_nWindowId && nLOKWindowId)
        {
            m_nWindowId = nLOKWindowId;
            m_bIsNotebookbar = true;
        }
        InsertWindowToMap(getMapIdFromWindowId());
    }

    initializeSender(GetNotifierWindow(), GetContentWindow(), GetTypeOfJSON());
}

void JSInstanceBuilder::initializeFormulabarSender(sal_uInt64 nLOKWindowId,
                                                   const std::u16string_view& sTypeOfJSON,
                                                   vcl::Window* pVclParent)
{
    m_sTypeOfJSON = sTypeOfJSON;
    m_nWindowId = nLOKWindowId;

    vcl::Window* pRoot = m_xBuilder->get_widget_root();
    m_aContentWindow = pVclParent;
    if (pRoot && pRoot->GetParent())
    {
        m_aParentDialog = pRoot->GetParent()->GetParentWithLOKNotifier();
        InsertWindowToMap(getMapIdFromWindowId());
    }

    initializeSender(GetNotifierWindow(), GetContentWindow(), GetTypeOfJSON());
}

void JSInstanceBuilder::initializeMenuSender(weld::Widget* pParent)
{
    m_sTypeOfJSON = "menu";
    m_aParentDialog = extract_sal_widget(pParent)->GetParentWithLOKNotifier();
    initializeSender(GetNotifierWindow(), GetContentWindow(), GetTypeOfJSON());
}

JSInstanceBuilder::JSInstanceBuilder(weld::Widget* pParent, vcl::Window* pVclParent,
                                     std::u16string_view rUIRoot, const OUString& rUIFile,
                                     JSInstanceBuilder::Type eBuilderType, sal_uInt64 nLOKWindowId,
                                     const std::u16string_view& sTypeOfJSON,
                                     const css::uno::Reference<css::frame::XFrame>& rFrame)
    : SalInstanceBuilder(pVclParent ? pVclParent : extract_sal_widget(pParent), rUIRoot, rUIFile,
                         rFrame)
    , m_nWindowId(0)
    , m_aParentDialog(nullptr)
    , m_aContentWindow(nullptr)
    , m_sTypeOfJSON("unknown")
    , m_bHasTopLevelDialog(false)
    , m_bIsNotebookbar(false)
    , m_bSentInitialUpdate(false)
    , m_bIsNestedBuilder(false)
    , m_aWindowToRelease(nullptr)
{
    switch (eBuilderType)
    {
        case JSInstanceBuilder::Type::Dialog:
            initializeDialogSender();
            break;

        case JSInstanceBuilder::Type::Popup:
            initializePopupSender();
            break;

        case JSInstanceBuilder::Type::Sidebar:
            initializeSidebarSender(nLOKWindowId, rUIFile, sTypeOfJSON);
            break;

        case JSInstanceBuilder::Type::Notebookbar:
            initializeNotebookbarSender(nLOKWindowId);
            break;

        case JSInstanceBuilder::Type::Formulabar:
            initializeFormulabarSender(nLOKWindowId, sTypeOfJSON, pVclParent);
            break;

        case JSInstanceBuilder::Type::Menu:
            initializeMenuSender(pParent);
            break;

        default:
            assert(false);
    };
}

std::unique_ptr<JSInstanceBuilder> JSInstanceBuilder::CreateDialogBuilder(weld::Widget* pParent,
                                                                          const OUString& rUIRoot,
                                                                          const OUString& rUIFile)
{
    return std::make_unique<JSInstanceBuilder>(pParent, nullptr, rUIRoot, rUIFile,
                                               JSInstanceBuilder::Type::Dialog);
}

std::unique_ptr<JSInstanceBuilder> JSInstanceBuilder::CreateNotebookbarBuilder(
    vcl::Window* pParent, const OUString& rUIRoot, const OUString& rUIFile,
    const css::uno::Reference<css::frame::XFrame>& rFrame, sal_uInt64 nWindowId)
{
    return std::make_unique<JSInstanceBuilder>(nullptr, pParent, rUIRoot, rUIFile,
                                               JSInstanceBuilder::Type::Notebookbar, nWindowId, u"",
                                               rFrame);
}

std::unique_ptr<JSInstanceBuilder> JSInstanceBuilder::CreateSidebarBuilder(weld::Widget* pParent,
                                                                           const OUString& rUIRoot,
                                                                           const OUString& rUIFile,
                                                                           const OUString& jsonType,
                                                                           sal_uInt64 nLOKWindowId)
{
    return std::make_unique<JSInstanceBuilder>(pParent, nullptr, rUIRoot, rUIFile,
                                               JSInstanceBuilder::Type::Sidebar, nLOKWindowId,
                                               jsonType);
}

std::unique_ptr<JSInstanceBuilder> JSInstanceBuilder::CreatePopupBuilder(weld::Widget* pParent,
                                                                         const OUString& rUIRoot,
                                                                         const OUString& rUIFile)
{
    return std::make_unique<JSInstanceBuilder>(pParent, nullptr, rUIRoot, rUIFile,
                                               JSInstanceBuilder::Type::Popup);
}

std::unique_ptr<JSInstanceBuilder> JSInstanceBuilder::CreateMenuBuilder(weld::Widget* pParent,
                                                                        const OUString& rUIRoot,
                                                                        const OUString& rUIFile)
{
    return std::make_unique<JSInstanceBuilder>(pParent, nullptr, rUIRoot, rUIFile,
                                               JSInstanceBuilder::Type::Menu);
}

std::unique_ptr<JSInstanceBuilder>
JSInstanceBuilder::CreateFormulabarBuilder(vcl::Window* pParent, const OUString& rUIRoot,
                                           const OUString& rUIFile, sal_uInt64 nLOKWindowId)
{
    return std::make_unique<JSInstanceBuilder>(nullptr, pParent, rUIRoot, rUIFile,
                                               JSInstanceBuilder::Type::Formulabar, nLOKWindowId,
                                               u"formulabar");
}

std::unique_ptr<JSInstanceBuilder>
JSInstanceBuilder::CreateAddressInputBuilder(vcl::Window* pParent, const OUString& rUIRoot,
                                             const OUString& rUIFile, sal_uInt64 nLOKWindowId)
{
    return std::make_unique<JSInstanceBuilder>(nullptr, pParent, rUIRoot, rUIFile,
                                               JSInstanceBuilder::Type::Formulabar, nLOKWindowId,
                                               u"addressinputfield");
}

JSInstanceBuilder::~JSInstanceBuilder()
{
    // tab page closed -> refresh parent window
    if (m_bIsNestedBuilder && m_sTypeOfJSON == "dialog")
    {
        jsdialog::SendFullUpdate(OUString::number(m_nWindowId), u"__DIALOG__"_ustr);
    }

    if (m_sTypeOfJSON == "popup" || m_sTypeOfJSON == "menu")
        sendClosePopup(m_nWindowId);

    if (m_aWindowToRelease)
    {
        m_aWindowToRelease->ReleaseLOKNotifier();
        m_aWindowToRelease.reset();
    }

    if (m_nWindowId && (m_bHasTopLevelDialog || m_bIsNotebookbar))
    {
        JSInstanceBuilder::Widgets().Forget(getMapIdFromWindowId());
    }
    else
    {
        auto aWindowMap = JSInstanceBuilder::Widgets().Find(getMapIdFromWindowId());
        if (aWindowMap)
        {
            std::for_each(m_aRememberedWidgets.begin(), m_aRememberedWidgets.end(),
                          [&aWindowMap](const OUString& sId) { aWindowMap->Forget(sId); });
        }
    }

    JSInstanceBuilder::Popups().Forget(OUString::number(m_nWindowId));
    JSInstanceBuilder::Menus().Forget(OUString::number(m_nWindowId));
}

const OUString& JSInstanceBuilder::GetTypeOfJSON() const { return m_sTypeOfJSON; }

VclPtr<vcl::Window>& JSInstanceBuilder::GetContentWindow()
{
    if (m_aContentWindow)
        return m_aContentWindow;
    else
        return m_bHasTopLevelDialog ? m_aOwnedToplevel : m_aParentDialog;
}

VclPtr<vcl::Window>& JSInstanceBuilder::GetNotifierWindow()
{
    return m_bHasTopLevelDialog ? m_aOwnedToplevel : m_aParentDialog;
}

std::unique_ptr<weld::Dialog> JSInstanceBuilder::weld_dialog(const OUString& id)
{
    std::unique_ptr<weld::Dialog> pRet;
    ::Dialog* pDialog = m_xBuilder->get<::Dialog>(id);

    if (pDialog)
    {
        if (!pDialog->GetLOKNotifier())
            pDialog->SetLOKNotifier(GetpApp());

        m_nWindowId = pDialog->GetLOKWindowId();
        pDialog->SetLOKTunnelingState(false);

        InsertWindowToMap(getMapIdFromWindowId());

        assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
        m_aOwnedToplevel.reset(pDialog);
        m_xBuilder->drop_ownership(pDialog);
        m_bHasTopLevelDialog = true;

        pRet.reset(new JSDialog(this, pDialog, this, false));

        RememberWidget(u"__DIALOG__"_ustr, pRet.get());

        initializeSender(GetNotifierWindow(), GetContentWindow(), GetTypeOfJSON());
        m_bSentInitialUpdate = true;
    }

    return pRet;
}

std::unique_ptr<weld::Assistant> JSInstanceBuilder::weld_assistant(const OUString& id)
{
    vcl::RoadmapWizard* pDialog = m_xBuilder->get<vcl::RoadmapWizard>(id);
    std::unique_ptr<JSAssistant> pRet(pDialog ? new JSAssistant(this, pDialog, this, false)
                                              : nullptr);
    if (pDialog)
    {
        m_nWindowId = pDialog->GetLOKWindowId();
        pDialog->SetLOKTunnelingState(false);

        InsertWindowToMap(getMapIdFromWindowId());

        assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
        m_aOwnedToplevel.reset(pDialog);
        m_xBuilder->drop_ownership(pDialog);
        m_bHasTopLevelDialog = true;

        pRet.reset(new JSAssistant(this, pDialog, this, false));

        RememberWidget(u"__DIALOG__"_ustr, pRet.get());

        initializeSender(GetNotifierWindow(), GetContentWindow(), GetTypeOfJSON());
        m_bSentInitialUpdate = true;
    }

    return pRet;
}

std::unique_ptr<weld::MessageDialog> JSInstanceBuilder::weld_message_dialog(const OUString& id)
{
    std::unique_ptr<weld::MessageDialog> pRet;
    ::MessageDialog* pMessageDialog = m_xBuilder->get<::MessageDialog>(id);

    if (pMessageDialog)
    {
        m_nWindowId = pMessageDialog->GetLOKWindowId();
        pMessageDialog->SetLOKTunnelingState(false);

        InsertWindowToMap(getMapIdFromWindowId());

        assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
        m_aOwnedToplevel.reset(pMessageDialog);
        m_xBuilder->drop_ownership(pMessageDialog);
        m_bHasTopLevelDialog = true;

        initializeSender(GetNotifierWindow(), GetContentWindow(), GetTypeOfJSON());
        m_bSentInitialUpdate = true;
    }

    pRet.reset(pMessageDialog ? new JSMessageDialog(this, pMessageDialog, this, false) : nullptr);

    if (pRet)
        RememberWidget(u"__DIALOG__"_ustr, pRet.get());

    return pRet;
}

std::unique_ptr<weld::Container> JSInstanceBuilder::weld_container(const OUString& id)
{
    vcl::Window* pContainer = m_xBuilder->get<vcl::Window>(id);
    auto pWeldWidget
        = pContainer ? std::make_unique<JSContainer>(this, pContainer, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    if (!m_bSentInitialUpdate && pContainer)
    {
        m_bSentInitialUpdate = true;

        // use parent builder to send update - avoid multiple calls from many builders
        vcl::Window* pParent = pContainer->GetParent();
        OUString sId = OUString::number(m_nWindowId);
        auto aWidgetMap = Widgets().Find(sId);
        while (pParent && aWidgetMap && !aWidgetMap->Find(pParent->get_id()))
        {
            pParent = pParent->GetParent();
        }

        if (pParent)
            jsdialog::SendFullUpdate(sId, pParent->get_id());

        // Navigator is currently just a panellayout but we treat it as its own dialog in online
        // this is a hack to get it to close atm but probably need a better solution
        if (id != u"NavigatorPanel")
            // this is nested builder, don't close parent dialog on destroy (eg. single tab page is closed)
            m_bCanClose = false;
        m_bIsNestedBuilder = true;
    }

    return pWeldWidget;
}

std::unique_ptr<weld::ScrolledWindow>
JSInstanceBuilder::weld_scrolled_window(const OUString& id, bool bUserManagedScrolling)
{
    VclScrolledWindow* pScrolledWindow = m_xBuilder->get<VclScrolledWindow>(id);
    auto pWeldWidget = pScrolledWindow
                           ? std::make_unique<JSScrolledWindow>(this, pScrolledWindow, this, false,
                                                                bUserManagedScrolling)
                           : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Label> JSInstanceBuilder::weld_label(const OUString& id)
{
    Control* pLabel = m_xBuilder->get<Control>(id);
    auto pWeldWidget = std::make_unique<JSLabel>(this, pLabel, this, false);

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Button> JSInstanceBuilder::weld_button(const OUString& id)
{
    ::Button* pButton = m_xBuilder->get<::Button>(id);
    auto pWeldWidget = pButton ? std::make_unique<JSButton>(this, pButton, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::LinkButton> JSInstanceBuilder::weld_link_button(const OUString& id)
{
    ::FixedHyperlink* pButton = m_xBuilder->get<::FixedHyperlink>(id);
    auto pWeldWidget
        = pButton ? std::make_unique<JSLinkButton>(this, pButton, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::ToggleButton> JSInstanceBuilder::weld_toggle_button(const OUString& id)
{
    ::PushButton* pButton = m_xBuilder->get<::PushButton>(id);
    auto pWeldWidget
        = pButton ? std::make_unique<JSToggleButton>(this, pButton, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Entry> JSInstanceBuilder::weld_entry(const OUString& id)
{
    Edit* pEntry = m_xBuilder->get<Edit>(id);
    auto pWeldWidget = pEntry ? std::make_unique<JSEntry>(this, pEntry, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::ComboBox> JSInstanceBuilder::weld_combo_box(const OUString& id)
{
    vcl::Window* pWidget = m_xBuilder->get<vcl::Window>(id);
    ::ComboBox* pComboBox = dynamic_cast<::ComboBox*>(pWidget);
    std::unique_ptr<weld::ComboBox> pWeldWidget;

    if (pComboBox)
    {
        pWeldWidget = std::make_unique<JSComboBox>(this, pComboBox, this, false);
    }
    else
    {
        ListBox* pListBox = dynamic_cast<ListBox*>(pWidget);
        pWeldWidget = pListBox ? std::make_unique<JSListBox>(this, pListBox, this, false) : nullptr;
    }

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Notebook> JSInstanceBuilder::weld_notebook(const OUString& id)
{
    std::unique_ptr<weld::Notebook> pWeldWidget;
    vcl::Window* pNotebook = m_xBuilder->get(id);

    if (pNotebook && pNotebook->GetType() == WindowType::TABCONTROL)
        pWeldWidget
            = std::make_unique<JSNotebook>(this, static_cast<TabControl*>(pNotebook), this, false);
    else if (pNotebook && pNotebook->GetType() == WindowType::VERTICALTABCONTROL)
        pWeldWidget = std::make_unique<JSVerticalNotebook>(
            this, static_cast<VerticalTabControl*>(pNotebook), this, false);

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::SpinButton> JSInstanceBuilder::weld_spin_button(const OUString& id)
{
    FormattedField* pSpinButton = m_xBuilder->get<FormattedField>(id);
    auto pWeldWidget
        = pSpinButton ? std::make_unique<JSSpinButton>(this, pSpinButton, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::FormattedSpinButton>
JSInstanceBuilder::weld_formatted_spin_button(const OUString& id)
{
    FormattedField* pSpinButton = m_xBuilder->get<FormattedField>(id);
    auto pWeldWidget = pSpinButton
                           ? std::make_unique<JSFormattedSpinButton>(this, pSpinButton, this, false)
                           : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::CheckButton> JSInstanceBuilder::weld_check_button(const OUString& id)
{
    CheckBox* pCheckButton = m_xBuilder->get<CheckBox>(id);
    auto pWeldWidget
        = pCheckButton ? std::make_unique<JSCheckButton>(this, pCheckButton, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::DrawingArea>
JSInstanceBuilder::weld_drawing_area(const OUString& id, const a11yref& rA11yImpl,
                                     FactoryFunction pUITestFactoryFunction, void* pUserData)
{
    VclDrawingArea* pArea = m_xBuilder->get<VclDrawingArea>(id);
    auto pWeldWidget = pArea ? std::make_unique<JSDrawingArea>(this, pArea, this, rA11yImpl,
                                                               pUITestFactoryFunction, pUserData)
                             : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Toolbar> JSInstanceBuilder::weld_toolbar(const OUString& id)
{
    ToolBox* pToolBox = m_xBuilder->get<ToolBox>(id);
    auto pWeldWidget
        = pToolBox ? std::make_unique<JSToolbar>(this, pToolBox, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::TextView> JSInstanceBuilder::weld_text_view(const OUString& id)
{
    VclMultiLineEdit* pTextView = m_xBuilder->get<VclMultiLineEdit>(id);
    auto pWeldWidget
        = pTextView ? std::make_unique<JSTextView>(this, pTextView, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::TreeView> JSInstanceBuilder::weld_tree_view(const OUString& id)
{
    SvTabListBox* pTreeView = m_xBuilder->get<SvTabListBox>(id);
    auto pWeldWidget
        = pTreeView ? std::make_unique<JSTreeView>(this, pTreeView, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Expander> JSInstanceBuilder::weld_expander(const OUString& id)
{
    VclExpander* pExpander = m_xBuilder->get<VclExpander>(id);
    auto pWeldWidget
        = pExpander ? std::make_unique<JSExpander>(this, pExpander, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::IconView> JSInstanceBuilder::weld_icon_view(const OUString& id)
{
    ::IconView* pIconView = m_xBuilder->get<::IconView>(id);
    auto pWeldWidget
        = pIconView ? std::make_unique<JSIconView>(this, pIconView, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::RadioButton> JSInstanceBuilder::weld_radio_button(const OUString& id)
{
    ::RadioButton* pRadioButton = m_xBuilder->get<::RadioButton>(id);
    auto pWeldWidget
        = pRadioButton ? std::make_unique<JSRadioButton>(this, pRadioButton, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Frame> JSInstanceBuilder::weld_frame(const OUString& id)
{
    ::VclFrame* pFrame = m_xBuilder->get<::VclFrame>(id);
    auto pWeldWidget = pFrame ? std::make_unique<JSFrame>(this, pFrame, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::MenuButton> JSInstanceBuilder::weld_menu_button(const OUString& id)
{
    ::MenuButton* pMenuButton = m_xBuilder->get<::MenuButton>(id);
    auto pWeldWidget
        = pMenuButton ? std::make_unique<JSMenuButton>(this, pMenuButton, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Menu> JSInstanceBuilder::weld_menu(const OUString& id)
{
    PopupMenu* pPopupMenu = m_xBuilder->get_menu(id);

    JSMenu* pMenu = pPopupMenu ? new JSMenu(this, pPopupMenu, this, false) : nullptr;

    std::unique_ptr<weld::Menu> pWeldWidget(pMenu);

    if (pWeldWidget)
        JSInstanceBuilder::Menus().Remember(getMapIdFromWindowId(), pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Popover> JSInstanceBuilder::weld_popover(const OUString& id)
{
    DockingWindow* pDockingWindow = m_xBuilder->get<DockingWindow>(id);
    JSPopover* pPopover
        = pDockingWindow ? new JSPopover(this, pDockingWindow, this, false) : nullptr;
    std::unique_ptr<weld::Popover> pWeldWidget(pPopover);
    if (pDockingWindow)
    {
        assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
        m_aOwnedToplevel.reset(pDockingWindow);
        m_xBuilder->drop_ownership(pDockingWindow);

        if (VclPtr<vcl::Window> pWin = pDockingWindow->GetParentWithLOKNotifier())
        {
            vcl::Window* pPopupRoot = pDockingWindow->GetChild(0);
            pPopupRoot->SetLOKNotifier(pWin->GetLOKNotifier());
            m_aParentDialog = pPopupRoot;
            m_aWindowToRelease = pPopupRoot;
            m_nWindowId = m_aParentDialog->GetLOKWindowId();

            pPopover->set_window_id(m_nWindowId);
            JSInstanceBuilder::Popups().Remember(OUString::number(m_nWindowId), pDockingWindow);

            InsertWindowToMap(getMapIdFromWindowId());
            initializeSender(GetNotifierWindow(), GetContentWindow(), GetTypeOfJSON());
        }
    }

    if (pWeldWidget)
        RememberWidget(u"__POPOVER__"_ustr, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Box> JSInstanceBuilder::weld_box(const OUString& id)
{
    VclBox* pContainer = m_xBuilder->get<VclBox>(id);
    auto pWeldWidget
        = pContainer ? std::make_unique<JSBox>(this, pContainer, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Widget> JSInstanceBuilder::weld_widget(const OUString& id)
{
    vcl::Window* pWidget = m_xBuilder->get(id);
    auto pWeldWidget
        = pWidget ? std::make_unique<JSWidgetInstance>(this, pWidget, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Image> JSInstanceBuilder::weld_image(const OUString& id)
{
    FixedImage* pImage = m_xBuilder->get<FixedImage>(id);

    auto pWeldWidget = pImage ? std::make_unique<JSImage>(this, pImage, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::LevelBar> JSInstanceBuilder::weld_level_bar(const OUString& id)
{
    ::ProgressBar* pLevelBar = m_xBuilder->get<::ProgressBar>(id);

    auto pWeldWidget
        = pLevelBar ? std::make_unique<JSLevelBar>(this, pLevelBar, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Calendar> JSInstanceBuilder::weld_calendar(const OUString& id)
{
    ::Calendar* pCalendar = m_xBuilder->get<::Calendar>(id);

    auto pWeldWidget
        = pCalendar ? std::make_unique<JSCalendar>(this, pCalendar, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

weld::MessageDialog*
JSInstanceBuilder::CreateMessageDialog(weld::Widget* pParent, VclMessageType eMessageType,
                                       VclButtonsType eButtonType, const OUString& rPrimaryMessage,
                                       const vcl::ILibreOfficeKitNotifier* pNotifier)
{
    SalInstanceWidget* pParentInstance = dynamic_cast<SalInstanceWidget*>(pParent);
    SystemWindow* pParentWidget = pParentInstance ? pParentInstance->getSystemWindow() : nullptr;
    VclPtrInstance<::MessageDialog> xMessageDialog(pParentWidget, rPrimaryMessage, eMessageType,
                                                   eButtonType);

    if (pNotifier)
        xMessageDialog->SetLOKNotifier(pNotifier);

    pNotifier = xMessageDialog->GetLOKNotifier();
    if (pNotifier)
    {
        OUString sWindowId = OUString::number(xMessageDialog->GetLOKWindowId());
        InsertWindowToMap(sWindowId);
        xMessageDialog->SetLOKTunnelingState(false);

        return new JSMessageDialog(xMessageDialog, nullptr, true);
    }
    else
        SAL_WARN("vcl", "No notifier in JSInstanceBuilder::CreateMessageDialog");

    return new JSMessageDialog(xMessageDialog, nullptr, true);
}

JSDialog::JSDialog(JSDialogSender* pSender, ::Dialog* pDialog, SalInstanceBuilder* pBuilder,
                   bool bTakeOwnership)
    : JSWidget<SalInstanceDialog, ::Dialog>(pSender, pDialog, pBuilder, bTakeOwnership)
{
}

void JSDialog::collapse(weld::Widget* pEdit, weld::Widget* pButton)
{
    SalInstanceDialog::collapse(pEdit, pButton);
    sendFullUpdate();
}

void JSDialog::undo_collapse()
{
    SalInstanceDialog::undo_collapse();
    sendFullUpdate();
}

void JSDialog::response(int response)
{
    if (response == RET_HELP)
    {
        response_help(m_xWidget.get());
        return;
    }

    sendClose();
    SalInstanceDialog::response(response);
}

void JSAssistant::response(int response)
{
    if (response == RET_HELP)
    {
        response_help(m_xWidget.get());
        return;
    }

    sendClose();
    SalInstanceAssistant::response(response);
}

int JSDialog::run()
{
    sendFullUpdate(true);
    int ret = SalInstanceDialog::run();
    return ret;
}

bool JSDialog::runAsync(std::shared_ptr<weld::DialogController> const& rxOwner,
                        const std::function<void(sal_Int32)>& rEndDialogFn)
{
    bool ret = SalInstanceDialog::runAsync(rxOwner, rEndDialogFn);
    sendFullUpdate();
    return ret;
}

bool JSDialog::runAsync(std::shared_ptr<Dialog> const& rxSelf,
                        const std::function<void(sal_Int32)>& func)
{
    bool ret = SalInstanceDialog::runAsync(rxSelf, func);
    sendFullUpdate();
    return ret;
}

int JSAssistant::run()
{
    sendFullUpdate(true);
    int ret = SalInstanceDialog::run();
    return ret;
}

bool JSAssistant::runAsync(std::shared_ptr<weld::DialogController> const& rxOwner,
                           const std::function<void(sal_Int32)>& rEndDialogFn)
{
    bool ret = SalInstanceDialog::runAsync(rxOwner, rEndDialogFn);
    sendFullUpdate();
    return ret;
}

bool JSAssistant::runAsync(std::shared_ptr<Dialog> const& rxSelf,
                           const std::function<void(sal_Int32)>& func)
{
    bool ret = SalInstanceDialog::runAsync(rxSelf, func);
    sendFullUpdate();
    return ret;
}

std::unique_ptr<weld::Button> JSDialog::weld_button_for_response(int nResponse)
{
    std::unique_ptr<weld::Button> xWeldWidget;
    if (PushButton* pButton
        = dynamic_cast<::PushButton*>(m_xDialog->get_widget_for_response(nResponse)))
    {
        xWeldWidget = std::make_unique<JSButton>(m_pSender, pButton, nullptr, false);

        auto pParentDialog = m_xDialog->GetParentWithLOKNotifier();
        if (pParentDialog)
            JSInstanceBuilder::RememberWidget(OUString::number(pParentDialog->GetLOKWindowId()),
                                              pButton->get_id(), xWeldWidget.get());
    }

    return xWeldWidget;
}

std::unique_ptr<weld::Button> JSAssistant::weld_button_for_response(int nResponse)
{
    ::PushButton* pButton = nullptr;
    std::unique_ptr<JSButton> xWeldWidget;
    if (nResponse == RET_YES)
        pButton = m_xWizard->m_pNextPage;
    else if (nResponse == RET_NO)
        pButton = m_xWizard->m_pPrevPage;
    else if (nResponse == RET_OK)
        pButton = m_xWizard->m_pFinish;
    else if (nResponse == RET_CANCEL)
        pButton = m_xWizard->m_pCancel;
    else if (nResponse == RET_HELP)
        pButton = m_xWizard->m_pHelp;

    if (pButton)
    {
        xWeldWidget = std::make_unique<JSButton>(m_pSender, pButton, nullptr, false);

        auto pParentDialog = m_xWizard->GetParentWithLOKNotifier();
        if (pParentDialog)
            JSInstanceBuilder::RememberWidget(OUString::number(pParentDialog->GetLOKWindowId()),
                                              pButton->get_id(), xWeldWidget.get());
    }

    return xWeldWidget;
}

JSAssistant::JSAssistant(JSDialogSender* pSender, vcl::RoadmapWizard* pDialog,
                         SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceAssistant, vcl::RoadmapWizard>(pSender, pDialog, pBuilder, bTakeOwnership)
{
}

void JSAssistant::set_current_page(int nPage)
{
    SalInstanceAssistant::set_current_page(nPage);
    sendFullUpdate();
}

void JSAssistant::set_current_page(const OUString& rIdent)
{
    SalInstanceAssistant::set_current_page(rIdent);
    sendFullUpdate();
}

JSContainer::JSContainer(JSDialogSender* pSender, vcl::Window* pContainer,
                         SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceContainer, vcl::Window>(pSender, pContainer, pBuilder, bTakeOwnership)
{
}

void JSContainer::move(weld::Widget* pWidget, weld::Container* pNewParent)
{
    SalInstanceContainer::move(pWidget, pNewParent);
    sendFullUpdate();
}

JSScrolledWindow::JSScrolledWindow(JSDialogSender* pSender, ::VclScrolledWindow* pContainer,
                                   SalInstanceBuilder* pBuilder, bool bTakeOwnership,
                                   bool bUserManagedScrolling)
    : JSWidget<SalInstanceScrolledWindow, ::VclScrolledWindow>(
          pSender, pContainer, pBuilder, bTakeOwnership, bUserManagedScrolling)
{
}

void JSScrolledWindow::vadjustment_configure(int value, int lower, int upper, int step_increment,
                                             int page_increment, int page_size)
{
    SalInstanceScrolledWindow::vadjustment_configure(value, lower, upper, step_increment,
                                                     page_increment, page_size);
    sendUpdate();
}

void JSScrolledWindow::vadjustment_set_value(int value)
{
    SalInstanceScrolledWindow::vadjustment_set_value(value);
    sendUpdate();
}

void JSScrolledWindow::vadjustment_set_value_no_notification(int value)
{
    SalInstanceScrolledWindow::vadjustment_set_value(value);
}

void JSScrolledWindow::vadjustment_set_page_size(int size)
{
    SalInstanceScrolledWindow::vadjustment_set_page_size(size);
    sendUpdate();
}

void JSScrolledWindow::set_vpolicy(VclPolicyType eVPolicy)
{
    SalInstanceScrolledWindow::set_vpolicy(eVPolicy);
    sendUpdate();
}

void JSScrolledWindow::hadjustment_configure(int value, int lower, int upper, int step_increment,
                                             int page_increment, int page_size)
{
    SalInstanceScrolledWindow::hadjustment_configure(value, lower, upper, step_increment,
                                                     page_increment, page_size);
    sendUpdate();
}

void JSScrolledWindow::hadjustment_set_value(int value)
{
    SalInstanceScrolledWindow::hadjustment_set_value(value);
    sendUpdate();
}

void JSScrolledWindow::hadjustment_set_value_no_notification(int value)
{
    SalInstanceScrolledWindow::hadjustment_set_value(value);
}

void JSScrolledWindow::hadjustment_set_page_size(int size)
{
    SalInstanceScrolledWindow::hadjustment_set_page_size(size);
    sendUpdate();
}

void JSScrolledWindow::set_hpolicy(VclPolicyType eVPolicy)
{
    SalInstanceScrolledWindow::set_hpolicy(eVPolicy);
    sendUpdate();
}

JSLabel::JSLabel(JSDialogSender* pSender, Control* pLabel, SalInstanceBuilder* pBuilder,
                 bool bTakeOwnership)
    : JSWidget<SalInstanceLabel, Control>(pSender, pLabel, pBuilder, bTakeOwnership)
{
}

void JSLabel::set_label(const OUString& rText)
{
    SalInstanceLabel::set_label(rText);
    sendUpdate();
};

JSButton::JSButton(JSDialogSender* pSender, ::Button* pButton, SalInstanceBuilder* pBuilder,
                   bool bTakeOwnership)
    : JSWidget<SalInstanceButton, ::Button>(pSender, pButton, pBuilder, bTakeOwnership)
{
}

JSLinkButton::JSLinkButton(JSDialogSender* pSender, ::FixedHyperlink* pButton,
                           SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceLinkButton, ::FixedHyperlink>(pSender, pButton, pBuilder, bTakeOwnership)
{
}

JSToggleButton::JSToggleButton(JSDialogSender* pSender, ::PushButton* pButton,
                               SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceToggleButton, ::PushButton>(pSender, pButton, pBuilder, bTakeOwnership)
{
}

JSEntry::JSEntry(JSDialogSender* pSender, ::Edit* pEntry, SalInstanceBuilder* pBuilder,
                 bool bTakeOwnership)
    : JSWidget<SalInstanceEntry, ::Edit>(pSender, pEntry, pBuilder, bTakeOwnership)
{
}

void JSEntry::set_text(const OUString& rText)
{
    SalInstanceEntry::set_text(rText);
    sendUpdate();
}

void JSEntry::set_text_without_notify(const OUString& rText) { SalInstanceEntry::set_text(rText); }

void JSEntry::replace_selection(const OUString& rText)
{
    SalInstanceEntry::replace_selection(rText);
    sendUpdate();
}

JSListBox::JSListBox(JSDialogSender* pSender, ::ListBox* pListBox, SalInstanceBuilder* pBuilder,
                     bool bTakeOwnership)
    : JSWidget<SalInstanceComboBoxWithoutEdit, ::ListBox>(pSender, pListBox, pBuilder,
                                                          bTakeOwnership)
{
}

void JSListBox::insert(int pos, const OUString& rStr, const OUString* pId,
                       const OUString* pIconName, VirtualDevice* pImageSurface)
{
    SalInstanceComboBoxWithoutEdit::insert(pos, rStr, pId, pIconName, pImageSurface);
    sendUpdate();
}

void JSListBox::remove(int pos)
{
    SalInstanceComboBoxWithoutEdit::remove(pos);
    sendUpdate();
}

void JSListBox::set_active(int pos)
{
    SalInstanceComboBoxWithoutEdit::set_active(pos);
    sendUpdate();
}

JSComboBox::JSComboBox(JSDialogSender* pSender, ::ComboBox* pComboBox, SalInstanceBuilder* pBuilder,
                       bool bTakeOwnership)
    : JSWidget<SalInstanceComboBoxWithEdit, ::ComboBox>(pSender, pComboBox, pBuilder,
                                                        bTakeOwnership)
{
}

void JSComboBox::insert(int pos, const OUString& rStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface)
{
    SalInstanceComboBoxWithEdit::insert(pos, rStr, pId, pIconName, pImageSurface);
    sendUpdate();
}

void JSComboBox::remove(int pos)
{
    SalInstanceComboBoxWithEdit::remove(pos);
    sendUpdate();
}

void JSComboBox::set_entry_text_without_notify(const OUString& rText)
{
    SalInstanceComboBoxWithEdit::set_entry_text(rText);
}

void JSComboBox::set_entry_text(const OUString& rText)
{
    SalInstanceComboBoxWithEdit::set_entry_text(rText);

    std::unique_ptr<jsdialog::ActionDataMap> pMap = std::make_unique<jsdialog::ActionDataMap>();
    (*pMap)[ACTION_TYPE ""_ostr] = "setText";
    (*pMap)["text"_ostr] = rText;
    sendAction(std::move(pMap));
}

void JSComboBox::set_active(int pos)
{
    if (pos == get_active())
        return;

    SalInstanceComboBoxWithEdit::set_active(pos);

    std::unique_ptr<jsdialog::ActionDataMap> pMap = std::make_unique<jsdialog::ActionDataMap>();
    (*pMap)[ACTION_TYPE ""_ostr] = "select";
    (*pMap)["position"_ostr] = OUString::number(pos);
    sendAction(std::move(pMap));
}

void JSComboBox::set_active_id(const OUString& rStr)
{
    sal_uInt16 nPos = find_id(rStr);
    set_active(nPos);
}

bool JSComboBox::changed_by_direct_pick() const { return true; }

void JSComboBox::render_entry(int pos, int dpix, int dpiy)
{
    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITH_ALPHA);
    pDevice->SetDPIX(96.0 * dpix / 100);
    pDevice->SetDPIY(96.0 * dpiy / 100);

    Size aRenderSize = signal_custom_get_size(*pDevice);
    pDevice->SetOutputSize(aRenderSize);

    signal_custom_render(*pDevice, tools::Rectangle(Point(0, 0), aRenderSize), false, get_id(pos));

    BitmapEx aImage = pDevice->GetBitmapEx(Point(0, 0), aRenderSize);

    SvMemoryStream aOStm(65535, 65535);
    if (GraphicConverter::Export(aOStm, aImage, ConvertDataFormat::PNG) == ERRCODE_NONE)
    {
        css::uno::Sequence<sal_Int8> aSeq(static_cast<sal_Int8 const*>(aOStm.GetData()),
                                          aOStm.Tell());
        OUStringBuffer aBuffer("data:image/png;base64,");
        ::comphelper::Base64::encode(aBuffer, aSeq);

        std::unique_ptr<jsdialog::ActionDataMap> pMap = std::make_unique<jsdialog::ActionDataMap>();
        (*pMap)[ACTION_TYPE ""_ostr] = "rendered_entry";
        (*pMap)["pos"_ostr] = OUString::number(pos);
        (*pMap)["image"_ostr] = aBuffer;
        sendAction(std::move(pMap));
    }
}

JSNotebook::JSNotebook(JSDialogSender* pSender, ::TabControl* pControl,
                       SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceNotebook, ::TabControl>(pSender, pControl, pBuilder, bTakeOwnership)
{
}

void JSNotebook::remove_page(const OUString& rIdent)
{
    SalInstanceNotebook::remove_page(rIdent);
    sendFullUpdate();
}

void JSNotebook::insert_page(const OUString& rIdent, const OUString& rLabel, int nPos)
{
    SalInstanceNotebook::insert_page(rIdent, rLabel, nPos);
    sendFullUpdate();
}

JSVerticalNotebook::JSVerticalNotebook(JSDialogSender* pSender, ::VerticalTabControl* pControl,
                                       SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceVerticalNotebook, ::VerticalTabControl>(pSender, pControl, pBuilder,
                                                                  bTakeOwnership)
{
}

void JSVerticalNotebook::remove_page(const OUString& rIdent)
{
    SalInstanceVerticalNotebook::remove_page(rIdent);
    sendFullUpdate();
}

void JSVerticalNotebook::insert_page(const OUString& rIdent, const OUString& rLabel, int nPos)
{
    SalInstanceVerticalNotebook::insert_page(rIdent, rLabel, nPos);
    sendFullUpdate();
}

JSSpinButton::JSSpinButton(JSDialogSender* pSender, ::FormattedField* pSpin,
                           SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceSpinButton, ::FormattedField>(pSender, pSpin, pBuilder, bTakeOwnership)
{
}

void JSSpinButton::set_floating_point_value(double fValue)
{
    SalInstanceSpinButton::set_floating_point_value(fValue);

    std::unique_ptr<jsdialog::ActionDataMap> pMap = std::make_unique<jsdialog::ActionDataMap>();
    (*pMap)[ACTION_TYPE ""_ostr] = "setText";
    (*pMap)["text"_ostr] = OUString::number(m_rFormatter.GetValue());
    sendAction(std::move(pMap));
}

void JSSpinButton::set_floating_point_range(double fMin, double fMax)
{
    SalInstanceSpinButton::set_floating_point_range(fMin, fMax);
    sendUpdate();
}

JSFormattedSpinButton::JSFormattedSpinButton(JSDialogSender* pSender, ::FormattedField* pSpin,
                                             SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceFormattedSpinButton, ::FormattedField>(pSender, pSpin, pBuilder,
                                                                 bTakeOwnership)
{
}

void JSFormattedSpinButton::set_text(const OUString& rText)
{
    SalInstanceFormattedSpinButton::set_text(rText);
    sendUpdate();
}

void JSFormattedSpinButton::set_text_without_notify(const OUString& rText)
{
    SalInstanceFormattedSpinButton::set_text(rText);
}

JSMessageDialog::JSMessageDialog(JSDialogSender* pSender, ::MessageDialog* pDialog,
                                 SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceMessageDialog, ::MessageDialog>(pSender, pDialog, pBuilder,
                                                          bTakeOwnership)
{
}

JSMessageDialog::JSMessageDialog(::MessageDialog* pDialog, SalInstanceBuilder* pBuilder,
                                 bool bTakeOwnership)
    : JSWidget<SalInstanceMessageDialog, ::MessageDialog>(nullptr, pDialog, pBuilder,
                                                          bTakeOwnership)
    , m_pOwnedSender(new JSDialogSender(pDialog, pDialog, u"dialog"_ustr))
{
    m_pSender = m_pOwnedSender.get();

    if (pBuilder)
        return;

    m_sWindowId = OUString::number(m_xMessageDialog->GetLOKWindowId());

    if (::OKButton* pOKBtn
        = dynamic_cast<::OKButton*>(m_xMessageDialog->get_widget_for_response(RET_OK)))
    {
        m_pOK.reset(new JSButton(m_pSender, pOKBtn, nullptr, false));
        JSInstanceBuilder::RememberWidget(m_sWindowId, pOKBtn->get_id(), m_pOK.get());
        m_pOK->connect_clicked(LINK(this, JSMessageDialog, OKHdl));
    }

    if (::CancelButton* pCancelBtn
        = dynamic_cast<::CancelButton*>(m_xMessageDialog->get_widget_for_response(RET_CANCEL)))
    {
        m_pCancel.reset(new JSButton(m_pSender, pCancelBtn, nullptr, false));
        JSInstanceBuilder::RememberWidget(m_sWindowId, pCancelBtn->get_id(), m_pCancel.get());
        m_pCancel->connect_clicked(LINK(this, JSMessageDialog, CancelHdl));
    }
}

JSMessageDialog::~JSMessageDialog()
{
    if (!m_pBuilder)
    {
        // For Message Dialogs created from Application::CreateMessageDialog
        // (where there is no builder to take care of this for us) explicitly
        // remove this window id on tear down
        JSInstanceBuilder::Widgets().Forget(m_sWindowId);
    }
}

void JSMessageDialog::RememberMessageDialog()
{
    static constexpr OUString sWidgetName = u"__DIALOG__"_ustr;
    OUString sWindowId = OUString::number(m_xMessageDialog->GetLOKWindowId());
    auto aWidgetMap = JSInstanceBuilder::Widgets().Find(sWindowId);
    if (aWidgetMap && aWidgetMap->Find(sWidgetName))
        return;

    JSInstanceBuilder::InsertWindowToMap(sWindowId);
    JSInstanceBuilder::RememberWidget(sWindowId, sWidgetName, this);
}

int JSMessageDialog::run()
{
    if (GetLOKNotifier())
    {
        RememberMessageDialog();
        sendFullUpdate();
    }

    int bRet = SalInstanceMessageDialog::run();
    return bRet;
}

bool JSMessageDialog::runAsync(std::shared_ptr<weld::DialogController> const& rxOwner,
                               const std::function<void(sal_Int32)>& rEndDialogFn)
{
    bool bRet = SalInstanceMessageDialog::runAsync(rxOwner, rEndDialogFn);

    RememberMessageDialog();
    sendFullUpdate();

    return bRet;
}

bool JSMessageDialog::runAsync(std::shared_ptr<Dialog> const& rxSelf,
                               const std::function<void(sal_Int32)>& rEndDialogFn)
{
    bool bRet = SalInstanceMessageDialog::runAsync(rxSelf, rEndDialogFn);

    RememberMessageDialog();
    sendFullUpdate();

    return bRet;
}

IMPL_LINK_NOARG(JSMessageDialog, OKHdl, weld::Button&, void) { response(RET_OK); }

IMPL_LINK_NOARG(JSMessageDialog, CancelHdl, weld::Button&, void) { response(RET_CANCEL); }

void JSMessageDialog::set_primary_text(const OUString& rText)
{
    SalInstanceMessageDialog::set_primary_text(rText);
    sendFullUpdate();
}

void JSMessageDialog::set_secondary_text(const OUString& rText)
{
    SalInstanceMessageDialog::set_secondary_text(rText);
    sendFullUpdate();
}

void JSMessageDialog::response(int response)
{
    if (response == RET_HELP)
    {
        response_help(m_xWidget.get());
        return;
    }

    sendClose();
    SalInstanceMessageDialog::response(response);
}

JSCheckButton::JSCheckButton(JSDialogSender* pSender, ::CheckBox* pCheckBox,
                             SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceCheckButton, ::CheckBox>(pSender, pCheckBox, pBuilder, bTakeOwnership)
{
}

void JSCheckButton::set_active(bool active)
{
    bool bWasActive = get_active();
    SalInstanceCheckButton::set_active(active);
    if (bWasActive != active)
        sendUpdate();
}

JSDrawingArea::JSDrawingArea(JSDialogSender* pSender, VclDrawingArea* pDrawingArea,
                             SalInstanceBuilder* pBuilder, const a11yref& rAlly,
                             FactoryFunction pUITestFactoryFunction, void* pUserData)
    : JSWidget<SalInstanceDrawingArea, VclDrawingArea>(pSender, pDrawingArea, pBuilder, rAlly,
                                                       std::move(pUITestFactoryFunction), pUserData,
                                                       false)
{
}

void JSDrawingArea::queue_draw()
{
    SalInstanceDrawingArea::queue_draw();
    sendUpdate();
}

void JSDrawingArea::queue_draw_area(int x, int y, int width, int height)
{
    SalInstanceDrawingArea::queue_draw_area(x, y, width, height);
    sendUpdate();
}

JSToolbar::JSToolbar(JSDialogSender* pSender, ::ToolBox* pToolbox, SalInstanceBuilder* pBuilder,
                     bool bTakeOwnership)
    : JSWidget<SalInstanceToolbar, ::ToolBox>(pSender, pToolbox, pBuilder, bTakeOwnership)
{
}

void JSToolbar::set_item_active(const OUString& rIdent, bool bActive)
{
    bool bWasActive = get_item_active(rIdent);
    SalInstanceToolbar::set_item_active(rIdent, bActive);

    if (bWasActive != bActive)
        sendUpdate();
}

void JSToolbar::set_menu_item_active(const OUString& rIdent, bool bActive)
{
    bool bWasActive = get_menu_item_active(rIdent);
    SalInstanceToolbar::set_menu_item_active(rIdent, bActive);

    ToolBoxItemId nItemId = m_xToolBox->GetItemId(rIdent);
    VclPtr<vcl::Window> pFloat = m_aFloats[nItemId];

    if (!pFloat)
        return;

    // See WeldToolbarPopup : include/svtools/toolbarmenu.hxx
    // TopLevel (Popover) -> Container -> main container of the popup
    vcl::Window* pPopupRoot = pFloat->GetChild(0);
    if (pPopupRoot)
        pPopupRoot = pPopupRoot->GetChild(0);

    if (pPopupRoot)
    {
        if (bActive)
        {
            JSInstanceBuilder::Popups().Remember(OUString::number(pPopupRoot->GetLOKWindowId()),
                                                 pFloat);
            sendPopup(pPopupRoot, m_xToolBox->get_id(), rIdent);
        }
        else if (bWasActive)
        {
            JSInstanceBuilder::Popups().Forget(OUString::number(pPopupRoot->GetLOKWindowId()));
            sendClosePopup(pPopupRoot->GetLOKWindowId());
        }
    }
}

void JSToolbar::set_item_sensitive(const OUString& rIdent, bool bSensitive)
{
    bool bWasSensitive = get_item_sensitive(rIdent);
    SalInstanceToolbar::set_item_sensitive(rIdent, bSensitive);
    if (bWasSensitive != bSensitive)
        sendUpdate();
}

void JSToolbar::set_item_icon_name(const OUString& rIdent, const OUString& rIconName)
{
    SalInstanceToolbar::set_item_icon_name(rIdent, rIconName);
    sendUpdate();
}

void JSToolbar::set_item_image(const OUString& rIdent,
                               const css::uno::Reference<css::graphic::XGraphic>& rImage)
{
    SalInstanceToolbar::set_item_image(rIdent, rImage);
    if (rIdent == u".uno:ChartColorPalette"_ustr)
    {
        sendUpdate();
    }
}

JSTextView::JSTextView(JSDialogSender* pSender, ::VclMultiLineEdit* pTextView,
                       SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceTextView, ::VclMultiLineEdit>(pSender, pTextView, pBuilder,
                                                        bTakeOwnership)
{
}

void JSTextView::set_text(const OUString& rText)
{
    SalInstanceTextView::set_text(rText);
    sendUpdate();
}

void JSTextView::set_text_without_notify(const OUString& rText)
{
    SalInstanceTextView::set_text(rText);
}

void JSTextView::replace_selection(const OUString& rText)
{
    SalInstanceTextView::replace_selection(rText);
    sendUpdate();
}

JSTreeView::JSTreeView(JSDialogSender* pSender, ::SvTabListBox* pTreeView,
                       SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceTreeView, ::SvTabListBox>(pSender, pTreeView, pBuilder, bTakeOwnership)
{
}

void JSTreeView::set_toggle(int pos, TriState eState, int col)
{
    SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, 0);

    while (pEntry && pos--)
        pEntry = m_xTreeView->Next(pEntry);

    if (pEntry)
    {
        SalInstanceTreeView::set_toggle(pEntry, eState, col);
        signal_toggled(iter_col(SalInstanceTreeIter(pEntry), col));

        sendUpdate();
    }
}

void JSTreeView::set_toggle(const weld::TreeIter& rIter, TriState bOn, int col)
{
    SalInstanceTreeView::set_toggle(rIter, bOn, col);
    sendUpdate();
}

void JSTreeView::set_sensitive(int pos, bool bSensitive, int col)
{
    SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, 0);

    while (pEntry && pos--)
        pEntry = m_xTreeView->Next(pEntry);

    if (pEntry)
    {
        SalInstanceTreeView::set_sensitive(pEntry, bSensitive, col);
        sendUpdate();
    }
}

void JSTreeView::set_sensitive(const weld::TreeIter& rIter, bool bSensitive, int col)
{
    SalInstanceTreeView::set_sensitive(rIter, bSensitive, col);
    sendUpdate();
}

void JSTreeView::select(int pos)
{
    assert(m_xTreeView->IsUpdateMode() && "don't select when frozen");
    disable_notify_events();
    if (pos == -1 || (pos == 0 && n_children() == 0))
        m_xTreeView->SelectAll(false);
    else
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, 0);

        while (pEntry && pos--)
            pEntry = m_xTreeView->Next(pEntry);

        if (pEntry)
        {
            m_xTreeView->Select(pEntry, true);
            m_xTreeView->MakeVisible(pEntry);
        }
    }
    enable_notify_events();

    std::unique_ptr<jsdialog::ActionDataMap> pMap = std::make_unique<jsdialog::ActionDataMap>();
    (*pMap)[ACTION_TYPE ""_ostr] = "select";
    (*pMap)["position"_ostr] = OUString::number(pos);
    sendAction(std::move(pMap));
}

weld::TreeView* JSTreeView::get_drag_source() const { return g_DragSource; }

void JSTreeView::drag_start() { g_DragSource = this; }

void JSTreeView::drag_end()
{
    css::datatransfer::dnd::XDropTarget* xDropTarget = m_xDropTarget.get();
    if (xDropTarget)
    {
        css::datatransfer::dnd::DropTargetDropEvent aEvent;
        aEvent.Source = xDropTarget;
        aEvent.Context = new JSDropTargetDropContext();
        // dummy values
        aEvent.LocationX = 50;
        aEvent.LocationY = 50;
        aEvent.DropAction = css::datatransfer::dnd::DNDConstants::ACTION_DEFAULT;
        aEvent.SourceActions = css::datatransfer::dnd::DNDConstants::ACTION_DEFAULT;

        m_xDropTarget->fire_drop(aEvent);

        sendUpdate();
        if (g_DragSource)
            g_DragSource->sendUpdate();
    }

    g_DragSource = nullptr;
}

void JSTreeView::insert(const weld::TreeIter* pParent, int pos, const OUString* pStr,
                        const OUString* pId, const OUString* pIconName,
                        VirtualDevice* pImageSurface, bool bChildrenOnDemand, weld::TreeIter* pRet)
{
    SalInstanceTreeView::insert(pParent, pos, pStr, pId, pIconName, pImageSurface,
                                bChildrenOnDemand, pRet);

    sendUpdate();
}

void JSTreeView::set_text(int row, const OUString& rText, int col)
{
    SalInstanceTreeView::set_text(row, rText, col);
    sendUpdate();
}

void JSTreeView::set_text(const weld::TreeIter& rIter, const OUString& rStr, int col)
{
    SalInstanceTreeView::set_text(rIter, rStr, col);
    sendUpdate();
}

void JSTreeView::remove(int pos)
{
    SalInstanceTreeView::remove(pos);
    sendUpdate();
}

void JSTreeView::remove(const weld::TreeIter& rIter)
{
    SalInstanceTreeView::remove(rIter);
    sendUpdate();
}

void JSTreeView::clear()
{
    SalInstanceTreeView::clear();
    sendUpdate();
}

void JSTreeView::set_cursor_without_notify(const weld::TreeIter& rIter)
{
    SalInstanceTreeView::set_cursor(rIter);
}

void JSTreeView::set_cursor(const weld::TreeIter& rIter)
{
    SalInstanceTreeView::set_cursor(rIter);
    sendUpdate();
}

void JSTreeView::set_cursor(int pos)
{
    SalInstanceTreeView::set_cursor(pos);
    sendUpdate();
}

void JSTreeView::expand_row(const weld::TreeIter& rIter)
{
    bool bNotify = false;
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    if (!m_xTreeView->IsExpanded(rVclIter.iter))
        bNotify = true;

    SalInstanceTreeView::expand_row(rIter);

    if (bNotify)
        sendUpdate();
}

void JSTreeView::collapse_row(const weld::TreeIter& rIter)
{
    bool bNotify = false;
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    if (m_xTreeView->IsExpanded(rVclIter.iter))
        bNotify = true;

    SalInstanceTreeView::collapse_row(rIter);

    if (bNotify)
        sendUpdate();
}

JSExpander::JSExpander(JSDialogSender* pSender, ::VclExpander* pExpander,
                       SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceExpander, ::VclExpander>(pSender, pExpander, pBuilder, bTakeOwnership)
{
}

void JSExpander::set_expanded(bool bExpand)
{
    SalInstanceExpander::set_expanded(bExpand);
    sendUpdate();
}

JSIconView::JSIconView(JSDialogSender* pSender, ::IconView* pIconView, SalInstanceBuilder* pBuilder,
                       bool bTakeOwnership)
    : JSWidget<SalInstanceIconView, ::IconView>(pSender, pIconView, pBuilder, bTakeOwnership)
{
}

void JSIconView::insert(int pos, const OUString* pStr, const OUString* pId,
                        const OUString* pIconName, weld::TreeIter* pRet)
{
    SalInstanceIconView::insert(pos, pStr, pId, pIconName, pRet);
    sendUpdate();
}

void JSIconView::insert(int pos, const OUString* pStr, const OUString* pId, const BitmapEx* pIcon,
                        weld::TreeIter* pRet)
{
    SalInstanceIconView::insert(pos, pStr, pId, pIcon, pRet);
    sendUpdate();
}

void JSIconView::insert_separator(int pos, const OUString* pId)
{
    SalInstanceIconView::insert_separator(pos, pId);
    sendUpdate();
}

void JSIconView::clear()
{
    SalInstanceIconView::clear();
    sendUpdate();
}

void JSIconView::select(int pos)
{
    SalInstanceIconView::select(pos);

    std::unique_ptr<jsdialog::ActionDataMap> pMap = std::make_unique<jsdialog::ActionDataMap>();
    (*pMap)[ACTION_TYPE ""_ostr] = "select";
    (*pMap)["position"_ostr] = OUString::number(pos);
    sendAction(std::move(pMap));
}

void JSIconView::unselect(int pos)
{
    SalInstanceIconView::unselect(pos);
    sendUpdate();
}

void JSIconView::render_entry(int pos, int dpix, int dpiy)
{
    OUString sImage = m_xIconView->renderEntry(pos, dpix, dpiy);
    if (sImage.isEmpty())
        return;

    std::unique_ptr<jsdialog::ActionDataMap> pMap = std::make_unique<jsdialog::ActionDataMap>();
    (*pMap)[ACTION_TYPE ""_ostr] = "rendered_entry";
    (*pMap)["pos"_ostr] = OUString::number(pos);
    (*pMap)["image"_ostr] = sImage;
    sendAction(std::move(pMap));
}

JSRadioButton::JSRadioButton(JSDialogSender* pSender, ::RadioButton* pRadioButton,
                             SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceRadioButton, ::RadioButton>(pSender, pRadioButton, pBuilder,
                                                      bTakeOwnership)
{
}

void JSRadioButton::set_active(bool active)
{
    SalInstanceRadioButton::set_active(active);
    sendUpdate();
}

JSFrame::JSFrame(JSDialogSender* pSender, ::VclFrame* pFrame, SalInstanceBuilder* pBuilder,
                 bool bTakeOwnership)
    : JSWidget<SalInstanceFrame, ::VclFrame>(pSender, pFrame, pBuilder, bTakeOwnership)
{
}

JSMenuButton::JSMenuButton(JSDialogSender* pSender, ::MenuButton* pMenuButton,
                           SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceMenuButton, ::MenuButton>(pSender, pMenuButton, pBuilder, bTakeOwnership)
{
}

void JSMenuButton::set_label(const OUString& rText)
{
    OUString aPreviousLabel = get_label();
    SalInstanceMenuButton::set_label(rText);
    if (aPreviousLabel != rText)
        sendUpdate();
}

void JSMenuButton::set_image(VirtualDevice* pDevice)
{
    SalInstanceMenuButton::set_image(pDevice);
    sendUpdate();
}

void JSMenuButton::set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage)
{
    SalInstanceMenuButton::set_image(rImage);
    sendUpdate();
}

void JSMenuButton::set_active(bool bActive)
{
    SalInstanceMenuButton::set_active(bActive);

    VclPtr<vcl::Window> pPopup = m_xMenuButton->GetPopover();
    if (pPopup)
    {
        if (bActive)
            sendPopup(pPopup->GetChild(0), m_xMenuButton->get_id(), m_xMenuButton->get_id());
        else
            sendClosePopup(pPopup->GetChild(0)->GetLOKWindowId());
    }
}

JSMenu::JSMenu(JSDialogSender* pSender, PopupMenu* pPopupMenu, SalInstanceBuilder* /*pBuilder*/,
               bool bTakeOwnership)
    : SalInstanceMenu(pPopupMenu, bTakeOwnership)
    , m_pPopupMenu(pPopupMenu)
    , m_pSender(pSender)
{
}

OUString JSMenu::popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect,
                               weld::Placement /*ePlace*/)
{
    // Do not block with SalInstanceMenu::popup_at_rect(pParent, rRect, ePlace);

    // we find position based on parent widget id and row text inside TreeView for context menu
    OUString sCancelId;
    weld::TreeView* pTree = dynamic_cast<weld::TreeView*>(pParent);
    if (pTree)
    {
        std::unique_ptr<weld::TreeIter> itEntry(pTree->make_iterator());
        if (pTree->get_dest_row_at_pos(rRect.Center(), itEntry.get(), false, false))
            sCancelId = pTree->get_text(*itEntry);
        else
            SAL_WARN("vcl", "No entry detected in JSMenu::popup_at_rect");
    }

    m_pSender->sendMenu(m_pPopupMenu, pParent ? pParent->get_buildable_name() : "", sCancelId);

    // Don't return any action - simulate canceled menu
    return "";
}

JSPopover::JSPopover(JSDialogSender* pSender, DockingWindow* pDockingWindow,
                     SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstancePopover, DockingWindow>(pSender, pDockingWindow, pBuilder, bTakeOwnership)
    , mnWindowId(0)
{
}

void JSPopover::popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect,
                              weld::Placement ePlace)
{
    SalInstancePopover::popup_at_rect(pParent, rRect, ePlace);
    sendPopup(getWidget()->GetChild(0), u"_POPOVER_"_ustr, u"_POPOVER_"_ustr);
}

void JSPopover::popdown()
{
    VclPtr<vcl::Window> pPopup = JSInstanceBuilder::Popups().Find(OUString::number(mnWindowId));

    if (pPopup)
    {
        sendClosePopup(mnWindowId);
        vcl::Window::GetDockingManager()->EndPopupMode(pPopup);
    }

    if (getWidget() && getWidget()->GetChild(0))
        sendClosePopup(getWidget()->GetChild(0)->GetLOKWindowId());

    SalInstancePopover::popdown();
}

JSBox::JSBox(JSDialogSender* pSender, VclBox* pBox, SalInstanceBuilder* pBuilder,
             bool bTakeOwnership)
    : JSWidget<SalInstanceBox, VclBox>(pSender, pBox, pBuilder, bTakeOwnership)
{
}

void JSBox::reorder_child(weld::Widget* pWidget, int nNewPosition)
{
    SalInstanceBox::reorder_child(pWidget, nNewPosition);
    sendFullUpdate();
}

JSImage::JSImage(JSDialogSender* pSender, FixedImage* pImage, SalInstanceBuilder* pBuilder,
                 bool bTakeOwnership)
    : JSWidget<SalInstanceImage, FixedImage>(pSender, pImage, pBuilder, bTakeOwnership)
{
}

void JSImage::set_image(VirtualDevice* pDevice)
{
    SalInstanceImage::set_image(pDevice);
    sendUpdate();
}

void JSImage::set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage)
{
    SalInstanceImage::set_image(rImage);
    sendUpdate();
}

JSLevelBar::JSLevelBar(JSDialogSender* pSender, ::ProgressBar* pProgressBar,
                       SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceLevelBar, ::ProgressBar>(pSender, pProgressBar, pBuilder, bTakeOwnership)
{
}

void JSLevelBar::set_percentage(double fPercentage)
{
    SalInstanceLevelBar::set_percentage(fPercentage);
    sendUpdate();
}

JSCalendar::JSCalendar(JSDialogSender* pSender, ::Calendar* pCalendar, SalInstanceBuilder* pBuilder,
                       bool bTakeOwnership)
    : JSWidget<SalInstanceCalendar, ::Calendar>(pSender, pCalendar, pBuilder, bTakeOwnership)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
