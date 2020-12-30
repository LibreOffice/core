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
#include <boost/property_tree/json_parser.hpp>
#include <comphelper/lok.hxx>
#include <vcl/dialog.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/toolbox.hxx>
#include <vcl/vclmedit.hxx>
#include <vcl/treelistentry.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace weld;

JSDialogNotifyIdle::JSDialogNotifyIdle(VclPtr<vcl::Window> aNotifierWindow,
                                       VclPtr<vcl::Window> aContentWindow, std::string sTypeOfJSON)
    : Idle("JSDialog notify")
    , m_aNotifierWindow(aNotifierWindow)
    , m_aContentWindow(aContentWindow)
    , m_sTypeOfJSON(sTypeOfJSON)
    , m_LastNotificationMessage()
    , m_bForce(false)
{
    SetPriority(TaskPriority::POST_PAINT);
}

void JSDialogNotifyIdle::ForceUpdate() { m_bForce = true; }

void JSDialogNotifyIdle::send(const boost::property_tree::ptree& rTree)
{
    try
    {
        if (!m_aNotifierWindow)
            return;

        const vcl::ILibreOfficeKitNotifier* pNotifier = m_aNotifierWindow->GetLOKNotifier();
        if (pNotifier)
        {
            std::stringstream aStream;
            boost::property_tree::write_json(aStream, rTree);
            const std::string message = aStream.str();
            if (m_bForce || message != m_LastNotificationMessage)
            {
                m_bForce = false;
                m_LastNotificationMessage = message;
                pNotifier->libreOfficeKitViewCallback(LOK_CALLBACK_JSDIALOG, message.c_str());
            }
        }
    }
    catch (boost::property_tree::json_parser::json_parser_error& rError)
    {
        SAL_WARN("vcl", rError.message());
    }
}

boost::property_tree::ptree JSDialogNotifyIdle::dumpStatus() const
{
    if (!m_aContentWindow || !m_aNotifierWindow)
        return boost::property_tree::ptree();

    boost::property_tree::ptree aTree = m_aContentWindow->DumpAsPropertyTree();
    aTree.put("id", m_aNotifierWindow->GetLOKWindowId());
    aTree.put("jsontype", m_sTypeOfJSON);

    if (m_sTypeOfJSON == "autofilter")
    {
        vcl::Window* pWindow = m_aContentWindow.get();
        DockingWindow* pDockingWIndow = dynamic_cast<DockingWindow*>(pWindow);
        while (pWindow && !pDockingWIndow)
        {
            pWindow = pWindow->GetParent();
            pDockingWIndow = dynamic_cast<DockingWindow*>(pWindow);
        }

        if (pDockingWIndow)
        {
            Point aPos = pDockingWIndow->GetFloatingPos();
            aTree.put("posx", aPos.getX());
            aTree.put("posy", aPos.getY());
        }
    }

    return aTree;
}

void JSDialogNotifyIdle::updateStatus(VclPtr<vcl::Window> pWindow)
{
    if (!m_aNotifierWindow)
        return;

    if (m_aNotifierWindow->IsReallyVisible())
    {
        if (const vcl::ILibreOfficeKitNotifier* pNotifier = m_aNotifierWindow->GetLOKNotifier())
        {
            boost::property_tree::ptree aTree;

            aTree.put("commandName", ".uno:jsdialog");
            aTree.put("success", "true");
            {
                boost::property_tree::ptree aResult;
                aResult.put("dialog_id", m_aNotifierWindow->GetLOKWindowId());
                aResult.put("control_id", pWindow->get_id());
                {
                    boost::property_tree::ptree aControl;
                    aControl = pWindow->DumpAsPropertyTree();
                    aResult.add_child("control", aControl);
                }
                aTree.add_child("result", aResult);
            }

            std::stringstream aStream;
            boost::property_tree::write_json(aStream, aTree);
            const std::string message = aStream.str();
            pNotifier->libreOfficeKitViewCallback(LOK_CALLBACK_UNO_COMMAND_RESULT,
                                                  message.c_str());
        }
    }
}

boost::property_tree::ptree JSDialogNotifyIdle::generateCloseMessage() const
{
    boost::property_tree::ptree aTree;
    if (m_aNotifierWindow)
        aTree.put("id", m_aNotifierWindow->GetLOKWindowId());
    aTree.put("jsontype", m_sTypeOfJSON);
    aTree.put("action", "close");

    return aTree;
}

void JSDialogNotifyIdle::Invoke() { send(dumpStatus()); }

void JSDialogNotifyIdle::sendClose() { send(generateCloseMessage()); }

void JSDialogSender::notifyDialogState(bool bForce)
{
    auto aNotifierWnd = mpIdleNotify->getNotifierWindow();
    if (aNotifierWnd && aNotifierWnd->IsDisableIdleNotify())
        return;

    if (bForce)
        mpIdleNotify->ForceUpdate();
    mpIdleNotify->Start();
}

void JSDialogSender::sendClose() { mpIdleNotify->sendClose(); }

void JSDialogSender::dumpStatus() { mpIdleNotify->Invoke(); }

void JSDialogSender::sendUpdate(VclPtr<vcl::Window> pWindow)
{
    mpIdleNotify->updateStatus(pWindow);
}

// Drag and drop

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

static JSTreeView* g_DragSource;

JSDropTarget::JSDropTarget()
    : WeakComponentImplHelper(m_aMutex)
{
}

void JSDropTarget::initialize(const css::uno::Sequence<css::uno::Any>& /*rArgs*/) {}

void JSDropTarget::addDropTargetListener(
    const css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>& xListener)
{
    ::osl::Guard<::osl::Mutex> aGuard(m_aMutex);

    m_aListeners.push_back(xListener);
}

void JSDropTarget::removeDropTargetListener(
    const css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>& xListener)
{
    ::osl::Guard<::osl::Mutex> aGuard(m_aMutex);

    m_aListeners.erase(std::remove(m_aListeners.begin(), m_aListeners.end(), xListener),
                       m_aListeners.end());
}

sal_Bool JSDropTarget::isActive() { return false; }

void JSDropTarget::setActive(sal_Bool /*active*/) {}

sal_Int8 JSDropTarget::getDefaultActions() { return 0; }

void JSDropTarget::setDefaultActions(sal_Int8 /*actions*/) {}

OUString JSDropTarget::getImplementationName()
{
    return "com.sun.star.datatransfer.dnd.JSDropTarget";
}

sal_Bool JSDropTarget::supportsService(OUString const& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> JSDropTarget::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aRet{ "com.sun.star.datatransfer.dnd.JSDropTarget" };
    return aRet;
}

void JSDropTarget::fire_drop(const css::datatransfer::dnd::DropTargetDropEvent& dtde)
{
    osl::ClearableGuard<osl::Mutex> aGuard(m_aMutex);
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(
        m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->drop(dtde);
    }
}

void JSDropTarget::fire_dragEnter(const css::datatransfer::dnd::DropTargetDragEnterEvent& dtde)
{
    osl::ClearableGuard<::osl::Mutex> aGuard(m_aMutex);
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(
        m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->dragEnter(dtde);
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

// used for dialogs
JSInstanceBuilder::JSInstanceBuilder(weld::Widget* pParent, const OUString& rUIRoot,
                                     const OUString& rUIFile)
    : SalInstanceBuilder(extract_sal_widget(pParent), rUIRoot, rUIFile)
    , m_nWindowId(0)
    , m_aParentDialog(nullptr)
    , m_aContentWindow(nullptr)
    , m_sTypeOfJSON("dialog")
    , m_bHasTopLevelDialog(false)
    , m_bIsNotebookbar(false)
{
    vcl::Window* pRoot = get_builder().get_widget_root();
    if (pRoot && pRoot->GetParent())
    {
        m_aParentDialog = pRoot->GetParent()->GetParentWithLOKNotifier();
        if (m_aParentDialog)
            m_nWindowId = m_aParentDialog->GetLOKWindowId();
        InsertWindowToMap(m_nWindowId);
    }
}

// used for notebookbar
JSInstanceBuilder::JSInstanceBuilder(vcl::Window* pParent, const OUString& rUIRoot,
                                     const OUString& rUIFile,
                                     const css::uno::Reference<css::frame::XFrame>& rFrame,
                                     sal_uInt64 nWindowId)
    : SalInstanceBuilder(pParent, rUIRoot, rUIFile, rFrame)
    , m_nWindowId(0)
    , m_aParentDialog(nullptr)
    , m_aContentWindow(nullptr)
    , m_sTypeOfJSON("notebookbar")
    , m_bHasTopLevelDialog(false)
    , m_bIsNotebookbar(false)
{
    vcl::Window* pRoot = get_builder().get_widget_root();
    if (pRoot && pRoot->GetParent())
    {
        m_aParentDialog = pRoot->GetParent()->GetParentWithLOKNotifier();
        if (m_aParentDialog)
            m_nWindowId = m_aParentDialog->GetLOKWindowId();
        if (!m_nWindowId && nWindowId)
        {
            m_nWindowId = nWindowId;
            m_bIsNotebookbar = true;
        }
        InsertWindowToMap(m_nWindowId);
    }
}

// used for autofilter dropdown
JSInstanceBuilder::JSInstanceBuilder(vcl::Window* pParent, const OUString& rUIRoot,
                                     const OUString& rUIFile)
    : SalInstanceBuilder(pParent, rUIRoot, rUIFile)
    , m_nWindowId(0)
    , m_aParentDialog(nullptr)
    , m_aContentWindow(nullptr)
    , m_sTypeOfJSON("autofilter")
    , m_bHasTopLevelDialog(false)
    , m_bIsNotebookbar(false)
{
    vcl::Window* pRoot = get_builder().get_widget_root();
    m_aContentWindow = pParent;
    if (pRoot && pRoot->GetParent())
    {
        m_aParentDialog = pRoot->GetParent()->GetParentWithLOKNotifier();
        if (m_aParentDialog)
            m_nWindowId = m_aParentDialog->GetLOKWindowId();
        InsertWindowToMap(m_nWindowId);
    }
}

JSInstanceBuilder* JSInstanceBuilder::CreateDialogBuilder(weld::Widget* pParent,
                                                          const OUString& rUIRoot,
                                                          const OUString& rUIFile)
{
    return new JSInstanceBuilder(pParent, rUIRoot, rUIFile);
}

JSInstanceBuilder* JSInstanceBuilder::CreateNotebookbarBuilder(
    vcl::Window* pParent, const OUString& rUIRoot, const OUString& rUIFile,
    const css::uno::Reference<css::frame::XFrame>& rFrame, sal_uInt64 nWindowId)
{
    return new JSInstanceBuilder(pParent, rUIRoot, rUIFile, rFrame, nWindowId);
}

JSInstanceBuilder* JSInstanceBuilder::CreateAutofilterWindowBuilder(vcl::Window* pParent,
                                                                    const OUString& rUIRoot,
                                                                    const OUString& rUIFile)
{
    return new JSInstanceBuilder(pParent, rUIRoot, rUIFile);
}

JSInstanceBuilder::~JSInstanceBuilder()
{
    if (m_nWindowId && (m_bHasTopLevelDialog || m_bIsNotebookbar))
    {
        GetLOKWeldWidgetsMap().erase(m_nWindowId);
    }
    else
    {
        auto it = GetLOKWeldWidgetsMap().find(m_nWindowId);
        if (it != GetLOKWeldWidgetsMap().end())
        {
            std::for_each(m_aRememberedWidgets.begin(), m_aRememberedWidgets.end(),
                          [it](std::string& sId) { it->second.erase(sId.c_str()); });
        }
    }
}

std::map<sal_uInt64, WidgetMap>& JSInstanceBuilder::GetLOKWeldWidgetsMap()
{
    // Map to remember the LOKWindowId <-> weld widgets binding.
    static std::map<sal_uInt64, WidgetMap> s_aLOKWeldBuildersMap;

    return s_aLOKWeldBuildersMap;
}

weld::Widget* JSInstanceBuilder::FindWeldWidgetsMap(sal_uInt64 nWindowId, const OString& rWidget)
{
    const auto it = GetLOKWeldWidgetsMap().find(nWindowId);

    if (it != GetLOKWeldWidgetsMap().end())
    {
        auto widgetIt = it->second.find(rWidget);
        if (widgetIt != it->second.end())
            return widgetIt->second;
    }

    return nullptr;
}

void JSInstanceBuilder::InsertWindowToMap(sal_uInt64 nWindowId)
{
    WidgetMap map;
    auto it = GetLOKWeldWidgetsMap().find(nWindowId);
    if (it == GetLOKWeldWidgetsMap().end())
        GetLOKWeldWidgetsMap().insert(std::map<sal_uInt64, WidgetMap>::value_type(nWindowId, map));
}

void JSInstanceBuilder::RememberWidget(const OString& id, weld::Widget* pWidget)
{
    auto it = GetLOKWeldWidgetsMap().find(m_nWindowId);
    if (it != GetLOKWeldWidgetsMap().end())
    {
        it->second.erase(id);
        it->second.insert(WidgetMap::value_type(id, pWidget));
        m_aRememberedWidgets.push_back(id.getStr());
    }
}

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

std::unique_ptr<weld::Dialog> JSInstanceBuilder::weld_dialog(const OString& id, bool bTakeOwnership)
{
    std::unique_ptr<weld::Dialog> pRet;
    ::Dialog* pDialog = m_xBuilder->get<::Dialog>(id);

    if (bTakeOwnership && pDialog)
    {
        m_nWindowId = pDialog->GetLOKWindowId();
        pDialog->SetLOKTunnelingState(false);

        InsertWindowToMap(m_nWindowId);

        assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
        m_aOwnedToplevel.set(pDialog);
        m_xBuilder->drop_ownership(pDialog);
        m_bHasTopLevelDialog = true;

        if (id == "MacroSelectorDialog")
            pDialog->SetDisableIdleNotify(true);

        pRet.reset(pDialog ? new JSDialog(m_aOwnedToplevel, m_aOwnedToplevel, pDialog, this, false,
                                          m_sTypeOfJSON)
                           : nullptr);

        RememberWidget("__DIALOG__", pRet.get());

        const vcl::ILibreOfficeKitNotifier* pNotifier = pDialog->GetLOKNotifier();
        if (pNotifier && id != "MacroSelectorDialog")
        {
            std::stringstream aStream;
            boost::property_tree::ptree aTree = m_aOwnedToplevel->DumpAsPropertyTree();
            aTree.put("id", m_aOwnedToplevel->GetLOKWindowId());
            boost::property_tree::write_json(aStream, aTree);
            const std::string message = aStream.str();
            pNotifier->libreOfficeKitViewCallback(LOK_CALLBACK_JSDIALOG, message.c_str());
        }
    }

    return pRet;
}

std::unique_ptr<weld::MessageDialog> JSInstanceBuilder::weld_message_dialog(const OString& id, bool bTakeOwnership)
{
    std::unique_ptr<weld::MessageDialog> pRet;
    ::MessageDialog* pMessageDialog = m_xBuilder->get<::MessageDialog>(id);

    if (bTakeOwnership && pMessageDialog)
    {
        m_nWindowId = pMessageDialog->GetLOKWindowId();
        pMessageDialog->SetLOKTunnelingState(false);

        InsertWindowToMap(m_nWindowId);

        assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
        m_aOwnedToplevel.set(pMessageDialog);
        m_xBuilder->drop_ownership(pMessageDialog);

        if (id == "MacroWarnMedium")
            pMessageDialog->SetDisableIdleNotify(true);
    }

    pRet.reset(pMessageDialog ? new JSMessageDialog(pMessageDialog, m_aOwnedToplevel, this, false)
                              : nullptr);

    return pRet;
}

std::unique_ptr<weld::Label> JSInstanceBuilder::weld_label(const OString& id, bool bTakeOwnership)
{
    ::FixedText* pLabel = m_xBuilder->get<FixedText>(id);
    auto pWeldWidget = std::make_unique<JSLabel>(GetNotifierWindow(), GetContentWindow(), pLabel,
                                                 this, bTakeOwnership, m_sTypeOfJSON);

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Button> JSInstanceBuilder::weld_button(const OString& id, bool bTakeOwnership)
{
    ::Button* pButton = m_xBuilder->get<::Button>(id);
    auto pWeldWidget
        = pButton ? std::make_unique<JSButton>(GetNotifierWindow(), GetContentWindow(), pButton,
                                               this, bTakeOwnership, m_sTypeOfJSON)
                  : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Entry> JSInstanceBuilder::weld_entry(const OString& id, bool bTakeOwnership)
{
    Edit* pEntry = m_xBuilder->get<Edit>(id);
    auto pWeldWidget = pEntry
                           ? std::make_unique<JSEntry>(GetNotifierWindow(), GetContentWindow(),
                                                       pEntry, this, bTakeOwnership, m_sTypeOfJSON)
                           : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::ComboBox> JSInstanceBuilder::weld_combo_box(const OString& id,
                                                                  bool bTakeOwnership)
{
    vcl::Window* pWidget = m_xBuilder->get<vcl::Window>(id);
    ::ComboBox* pComboBox = dynamic_cast<::ComboBox*>(pWidget);
    std::unique_ptr<weld::ComboBox> pWeldWidget;

    if (pComboBox)
    {
        pWeldWidget = std::make_unique<JSComboBox>(GetNotifierWindow(), GetContentWindow(),
                                                   pComboBox, this, bTakeOwnership, m_sTypeOfJSON);
    }
    else
    {
        ListBox* pListBox = dynamic_cast<ListBox*>(pWidget);
        pWeldWidget
            = pListBox ? std::make_unique<JSListBox>(GetNotifierWindow(), GetContentWindow(),
                                                     pListBox, this, bTakeOwnership, m_sTypeOfJSON)
                       : nullptr;
    }

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Notebook> JSInstanceBuilder::weld_notebook(const OString& id,
                                                                 bool bTakeOwnership)
{
    TabControl* pNotebook = m_xBuilder->get<TabControl>(id);
    auto pWeldWidget
        = pNotebook ? std::make_unique<JSNotebook>(GetNotifierWindow(), GetContentWindow(),
                                                   pNotebook, this, bTakeOwnership, m_sTypeOfJSON)
                    : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::SpinButton> JSInstanceBuilder::weld_spin_button(const OString& id,
                                                                      bool bTakeOwnership)
{
    FormattedField* pSpinButton = m_xBuilder->get<FormattedField>(id);
    auto pWeldWidget
        = pSpinButton
              ? std::make_unique<JSSpinButton>(GetNotifierWindow(), GetContentWindow(), pSpinButton,
                                               this, bTakeOwnership, m_sTypeOfJSON)
              : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::CheckButton> JSInstanceBuilder::weld_check_button(const OString& id,
                                                                        bool bTakeOwnership)
{
    CheckBox* pCheckButton = m_xBuilder->get<CheckBox>(id);
    auto pWeldWidget
        = pCheckButton
              ? std::make_unique<JSCheckButton>(GetNotifierWindow(), GetContentWindow(),
                                                pCheckButton, this, bTakeOwnership, m_sTypeOfJSON)
              : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::DrawingArea>
JSInstanceBuilder::weld_drawing_area(const OString& id, const a11yref& rA11yImpl,
                                     FactoryFunction pUITestFactoryFunction, void* pUserData,
                                     bool bTakeOwnership)
{
    VclDrawingArea* pArea = m_xBuilder->get<VclDrawingArea>(id);
    auto pWeldWidget = pArea ? std::make_unique<JSDrawingArea>(
                                   GetNotifierWindow(), GetContentWindow(), pArea, this, rA11yImpl,
                                   pUITestFactoryFunction, pUserData, m_sTypeOfJSON, bTakeOwnership)
                             : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Toolbar> JSInstanceBuilder::weld_toolbar(const OString& id,
                                                               bool bTakeOwnership)
{
    ToolBox* pToolBox = m_xBuilder->get<ToolBox>(id);
    auto pWeldWidget
        = pToolBox ? std::make_unique<JSToolbar>(GetNotifierWindow(), GetContentWindow(), pToolBox,
                                                 this, bTakeOwnership, m_sTypeOfJSON)
                   : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::TextView> JSInstanceBuilder::weld_text_view(const OString& id,
                                                                  bool bTakeOwnership)
{
    VclMultiLineEdit* pTextView = m_xBuilder->get<VclMultiLineEdit>(id);
    auto pWeldWidget
        = pTextView ? std::make_unique<JSTextView>(GetNotifierWindow(), GetContentWindow(),
                                                   pTextView, this, bTakeOwnership, m_sTypeOfJSON)
                    : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::TreeView> JSInstanceBuilder::weld_tree_view(const OString& id,
                                                                  bool bTakeOwnership)
{
    SvTabListBox* pTreeView = m_xBuilder->get<SvTabListBox>(id);
    auto pWeldWidget
        = pTreeView ? std::make_unique<JSTreeView>(GetNotifierWindow(), GetContentWindow(),
                                                   pTreeView, this, bTakeOwnership, m_sTypeOfJSON)
                    : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Expander> JSInstanceBuilder::weld_expander(const OString& id,
                                                                 bool bTakeOwnership)
{
    VclExpander* pExpander = m_xBuilder->get<VclExpander>(id);
    auto pWeldWidget
        = pExpander ? std::make_unique<JSExpander>(GetNotifierWindow(), GetContentWindow(),
                                                   pExpander, this, bTakeOwnership, m_sTypeOfJSON)
                    : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::IconView> JSInstanceBuilder::weld_icon_view(const OString& id,
                                                                  bool bTakeOwnership)
{
    ::IconView* pIconView = m_xBuilder->get<::IconView>(id);
    auto pWeldWidget
        = pIconView ? std::make_unique<JSIconView>(GetNotifierWindow(), GetContentWindow(),
                                                   pIconView, this, bTakeOwnership, m_sTypeOfJSON)
                    : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

weld::MessageDialog* JSInstanceBuilder::CreateMessageDialog(weld::Widget* pParent,
                                                            VclMessageType eMessageType,
                                                            VclButtonsType eButtonType,
                                                            const OUString& rPrimaryMessage)
{
    SalInstanceWidget* pParentInstance = dynamic_cast<SalInstanceWidget*>(pParent);
    SystemWindow* pParentWidget = pParentInstance ? pParentInstance->getSystemWindow() : nullptr;
    VclPtrInstance<::MessageDialog> xMessageDialog(pParentWidget, rPrimaryMessage, eMessageType,
                                                   eButtonType);

    const vcl::ILibreOfficeKitNotifier* pNotifier = xMessageDialog->GetLOKNotifier();
    if (pNotifier)
    {
        std::stringstream aStream;
        boost::property_tree::ptree aTree = xMessageDialog->DumpAsPropertyTree();
        aTree.put("id", xMessageDialog->GetLOKWindowId());
        aTree.put("jsontype", "dialog");
        boost::property_tree::write_json(aStream, aTree);
        const std::string message = aStream.str();
        pNotifier->libreOfficeKitViewCallback(LOK_CALLBACK_JSDIALOG, message.c_str());
    }

    return new JSMessageDialog(xMessageDialog, xMessageDialog, nullptr, true);
}

JSDialog::JSDialog(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                   ::Dialog* pDialog, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
                   std::string sTypeOfJSON)
    : JSWidget<SalInstanceDialog, ::Dialog>(aNotifierWindow, aContentWindow, pDialog, pBuilder,
                                            bTakeOwnership, sTypeOfJSON)
{
    if (aNotifierWindow && aNotifierWindow->IsDisableIdleNotify())
    {
        pDialog->AddEventListener(LINK(this, JSDialog, on_window_event));
        m_bNotifyCreated = false;
    }
}

void JSDialog::collapse(weld::Widget* pEdit, weld::Widget* pButton)
{
    SalInstanceDialog::collapse(pEdit, pButton);
    notifyDialogState();
}

void JSDialog::undo_collapse()
{
    SalInstanceDialog::undo_collapse();
    notifyDialogState();
}

void JSDialog::response(int response)
{
    sendClose();
    SalInstanceDialog::response(response);
}

IMPL_LINK_NOARG(JSDialog, on_dump_status, void*, void) { JSDialogSender::dumpStatus(); }

IMPL_LINK(JSDialog, on_window_event, VclWindowEvent&, rEvent, void)
{
    if (rEvent.GetId() == VclEventId::WindowShow && !m_bNotifyCreated)
    {
        Application::PostUserEvent(LINK(this, JSDialog, on_dump_status));
        m_bNotifyCreated = true;
    }
}

JSLabel::JSLabel(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                 FixedText* pLabel, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
                 std::string sTypeOfJSON)
    : JSWidget<SalInstanceLabel, FixedText>(aNotifierWindow, aContentWindow, pLabel, pBuilder,
                                            bTakeOwnership, sTypeOfJSON)
{
}

void JSLabel::set_label(const OUString& rText)
{
    SalInstanceLabel::set_label(rText);
    notifyDialogState();
};

JSButton::JSButton(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                   ::Button* pButton, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
                   std::string sTypeOfJSON)
    : JSWidget<SalInstanceButton, ::Button>(aNotifierWindow, aContentWindow, pButton, pBuilder,
                                            bTakeOwnership, sTypeOfJSON)
{
}

JSEntry::JSEntry(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                 ::Edit* pEntry, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
                 std::string sTypeOfJSON)
    : JSWidget<SalInstanceEntry, ::Edit>(aNotifierWindow, aContentWindow, pEntry, pBuilder,
                                         bTakeOwnership, sTypeOfJSON)
{
}

void JSEntry::set_text(const OUString& rText)
{
    SalInstanceEntry::set_text(rText);
    notifyDialogState();
}

JSListBox::JSListBox(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                     ::ListBox* pListBox, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
                     std::string sTypeOfJSON)
    : JSWidget<SalInstanceComboBoxWithoutEdit, ::ListBox>(aNotifierWindow, aContentWindow, pListBox,
                                                          pBuilder, bTakeOwnership, sTypeOfJSON)
{
}

void JSListBox::insert(int pos, const OUString& rStr, const OUString* pId,
                       const OUString* pIconName, VirtualDevice* pImageSurface)
{
    SalInstanceComboBoxWithoutEdit::insert(pos, rStr, pId, pIconName, pImageSurface);
    notifyDialogState();
}

void JSListBox::remove(int pos)
{
    SalInstanceComboBoxWithoutEdit::remove(pos);
    notifyDialogState();
}

void JSListBox::set_active(int pos)
{
    SalInstanceComboBoxWithoutEdit::set_active(pos);
    notifyDialogState();
}

JSComboBox::JSComboBox(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                       ::ComboBox* pComboBox, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
                       std::string sTypeOfJSON)
    : JSWidget<SalInstanceComboBoxWithEdit, ::ComboBox>(aNotifierWindow, aContentWindow, pComboBox,
                                                        pBuilder, bTakeOwnership, sTypeOfJSON)
{
}

void JSComboBox::insert(int pos, const OUString& rStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface)
{
    SalInstanceComboBoxWithEdit::insert(pos, rStr, pId, pIconName, pImageSurface);
    notifyDialogState();
}

void JSComboBox::remove(int pos)
{
    SalInstanceComboBoxWithEdit::remove(pos);
    notifyDialogState();
}

void JSComboBox::set_entry_text(const OUString& rText)
{
    SalInstanceComboBoxWithEdit::set_entry_text(rText);
    notifyDialogState();
}

void JSComboBox::set_active(int pos)
{
    SalInstanceComboBoxWithEdit::set_active(pos);
    notifyDialogState();
}

JSNotebook::JSNotebook(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                       ::TabControl* pControl, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
                       std::string sTypeOfJSON)
    : JSWidget<SalInstanceNotebook, ::TabControl>(aNotifierWindow, aContentWindow, pControl,
                                                  pBuilder, bTakeOwnership, sTypeOfJSON)
{
}

void JSNotebook::set_current_page(int nPage)
{
    bool bForce = false;
    int nCurrent = get_current_page();
    if (nCurrent == nPage)
        bForce = true;

    SalInstanceNotebook::set_current_page(nPage);
    notifyDialogState(bForce);
}

void JSNotebook::set_current_page(const OString& rIdent)
{
    bool bForce = false;
    OString sCurrent = get_current_page_ident();
    if (sCurrent == rIdent)
        bForce = true;

    SalInstanceNotebook::set_current_page(rIdent);
    notifyDialogState(bForce);
}

void JSNotebook::remove_page(const OString& rIdent)
{
    SalInstanceNotebook::remove_page(rIdent);
    notifyDialogState();
}

void JSNotebook::append_page(const OString& rIdent, const OUString& rLabel)
{
    SalInstanceNotebook::append_page(rIdent, rLabel);
    notifyDialogState();
}

JSSpinButton::JSSpinButton(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                           ::FormattedField* pSpin, SalInstanceBuilder* pBuilder,
                           bool bTakeOwnership, std::string sTypeOfJSON)
    : JSWidget<SalInstanceSpinButton, ::FormattedField>(aNotifierWindow, aContentWindow, pSpin,
                                                        pBuilder, bTakeOwnership, sTypeOfJSON)
{
}

void JSSpinButton::set_value(int value)
{
    SalInstanceSpinButton::set_value(value);
    notifyDialogState(true); // if input is limited we can receive the same JSON
}

JSMessageDialog::JSMessageDialog(::MessageDialog* pDialog, VclPtr<vcl::Window> aContentWindow,
                                 SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : SalInstanceMessageDialog(pDialog, pBuilder, bTakeOwnership)
    , JSDialogSender(m_xMessageDialog, aContentWindow, "dialog")
{
}

void JSMessageDialog::set_primary_text(const OUString& rText)
{
    SalInstanceMessageDialog::set_primary_text(rText);
    notifyDialogState();
}

void JSMessageDialog::set_secondary_text(const OUString& rText)
{
    SalInstanceMessageDialog::set_secondary_text(rText);
    notifyDialogState();
}

JSCheckButton::JSCheckButton(VclPtr<vcl::Window> aNotifierWindow,
                             VclPtr<vcl::Window> aContentWindow, ::CheckBox* pCheckBox,
                             SalInstanceBuilder* pBuilder, bool bTakeOwnership,
                             std::string sTypeOfJSON)
    : JSWidget<SalInstanceCheckButton, ::CheckBox>(aNotifierWindow, aContentWindow, pCheckBox,
                                                   pBuilder, bTakeOwnership, sTypeOfJSON)
{
}

void JSCheckButton::set_active(bool active)
{
    SalInstanceCheckButton::set_active(active);
    sendUpdate(m_xCheckButton);
}

JSDrawingArea::JSDrawingArea(VclPtr<vcl::Window> aNotifierWindow,
                             VclPtr<vcl::Window> aContentWindow, VclDrawingArea* pDrawingArea,
                             SalInstanceBuilder* pBuilder, const a11yref& rAlly,
                             FactoryFunction pUITestFactoryFunction, void* pUserData,
                             std::string sTypeOfJSON, bool bTakeOwnership)
    : SalInstanceDrawingArea(pDrawingArea, pBuilder, rAlly, pUITestFactoryFunction, pUserData,
                             bTakeOwnership)
    , JSDialogSender(aNotifierWindow, aContentWindow, sTypeOfJSON)
{
}

void JSDrawingArea::queue_draw()
{
    SalInstanceDrawingArea::queue_draw();
    notifyDialogState();
}

void JSDrawingArea::queue_draw_area(int x, int y, int width, int height)
{
    SalInstanceDrawingArea::queue_draw_area(x, y, width, height);
    notifyDialogState();
}

JSToolbar::JSToolbar(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                     ::ToolBox* pToolbox, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
                     std::string sTypeOfJSON)
    : JSWidget<SalInstanceToolbar, ::ToolBox>(aNotifierWindow, aContentWindow, pToolbox, pBuilder,
                                              bTakeOwnership, sTypeOfJSON)
{
}

void JSToolbar::signal_clicked(const OString& rIdent)
{
    SalInstanceToolbar::signal_clicked(rIdent);
    notifyDialogState();
}

JSTextView::JSTextView(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                       ::VclMultiLineEdit* pTextView, SalInstanceBuilder* pBuilder,
                       bool bTakeOwnership, std::string sTypeOfJSON)
    : JSWidget<SalInstanceTextView, ::VclMultiLineEdit>(aNotifierWindow, aContentWindow, pTextView,
                                                        pBuilder, bTakeOwnership, sTypeOfJSON)
{
}

void JSTextView::set_text(const OUString& rText)
{
    SalInstanceTextView::set_text(rText);
    notifyDialogState();
}

JSTreeView::JSTreeView(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                       ::SvTabListBox* pTreeView, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
                       std::string sTypeOfJSON)
    : JSWidget<SalInstanceTreeView, ::SvTabListBox>(aNotifierWindow, aContentWindow, pTreeView,
                                                    pBuilder, bTakeOwnership, sTypeOfJSON)
{
    if (aNotifierWindow && aNotifierWindow->IsDisableIdleNotify())
        pTreeView->AddEventListener(LINK(this, JSTreeView, on_window_event));
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

        sendUpdate(m_xTreeView);
    }
}

void JSTreeView::set_toggle(const weld::TreeIter& rIter, TriState bOn, int col)
{
    SalInstanceTreeView::set_toggle(rIter, bOn, col);
    sendUpdate(m_xTreeView);
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

        sendUpdate(m_xTreeView);
    }

    g_DragSource = nullptr;
}

void JSTreeView::insert(const weld::TreeIter* pParent, int pos, const OUString* pStr,
                        const OUString* pId, const OUString* pIconName,
                        VirtualDevice* pImageSurface, const OUString* pExpanderName,
                        bool bChildrenOnDemand, weld::TreeIter* pRet)
{
    SalInstanceTreeView::insert(pParent, pos, pStr, pId, pIconName, pImageSurface, pExpanderName,
                                bChildrenOnDemand, pRet);

    sendUpdate(m_xTreeView);
}

void JSTreeView::set_text(int row, const OUString& rText, int col)
{
    SalInstanceTreeView::set_text(row, rText, col);
    sendUpdate(m_xTreeView);
}

void JSTreeView::set_text(const weld::TreeIter& rIter, const OUString& rStr, int col)
{
    SalInstanceTreeView::set_text(rIter, rStr, col);
    sendUpdate(m_xTreeView);
}

void JSTreeView::expand_row(const weld::TreeIter& rIter)
{
    SalInstanceTreeView::expand_row(rIter);
    sendUpdate(m_xTreeView);
}

void JSTreeView::collapse_row(const weld::TreeIter& rIter)
{
    SalInstanceTreeView::collapse_row(rIter);
    sendUpdate(m_xTreeView);
}

IMPL_LINK(JSTreeView, on_window_event, VclWindowEvent&, rEvent, void)
{
    if (rEvent.GetId() == VclEventId::WindowPaint && get_visible() && m_xTreeView->IsDirtyModel())
    {
        sendUpdate(m_xTreeView);
        m_xTreeView->SetDirtyModel(false);
    }
}

JSExpander::JSExpander(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                       ::VclExpander* pExpander, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
                       std::string sTypeOfJSON)
    : JSWidget<SalInstanceExpander, ::VclExpander>(aNotifierWindow, aContentWindow, pExpander,
                                                   pBuilder, bTakeOwnership, sTypeOfJSON)
{
}

void JSExpander::set_expanded(bool bExpand)
{
    SalInstanceExpander::set_expanded(bExpand);
    notifyDialogState();
}

JSIconView::JSIconView(VclPtr<vcl::Window> aNotifierWindow, VclPtr<vcl::Window> aContentWindow,
                       ::IconView* pIconView, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
                       std::string sTypeOfJSON)
    : JSWidget<SalInstanceIconView, ::IconView>(aNotifierWindow, aContentWindow, pIconView,
                                                pBuilder, bTakeOwnership, sTypeOfJSON)
{
}

void JSIconView::insert(int pos, const OUString* pStr, const OUString* pId,
                        const OUString* pIconName, weld::TreeIter* pRet)
{
    SalInstanceIconView::insert(pos, pStr, pId, pIconName, pRet);
    notifyDialogState();
}

void JSIconView::insert(int pos, const OUString* pStr, const OUString* pId,
                        const VirtualDevice* pIcon, weld::TreeIter* pRet)
{
    SalInstanceIconView::insert(pos, pStr, pId, pIcon, pRet);
    notifyDialogState();
}

void JSIconView::clear()
{
    SalInstanceIconView::clear();
    notifyDialogState();
}

void JSIconView::select(int pos)
{
    SalInstanceIconView::select(pos);
    notifyDialogState();
}

void JSIconView::unselect(int pos)
{
    SalInstanceIconView::unselect(pos);
    notifyDialogState();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
