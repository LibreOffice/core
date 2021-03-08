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
#include <comphelper/lok.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/toolkit/button.hxx>
#include <vcl/toolkit/dialog.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/toolkit/combobox.hxx>
#include <messagedialog.hxx>
#include <tools/json_writer.hxx>
#include <o3tl/deleter.hxx>
#include <memory>
#include <vcl/toolbox.hxx>
#include <vcl/toolkit/vclmedit.hxx>
#include <boost/property_tree/json_parser.hpp>
#include <vcl/toolkit/treelistentry.hxx>
#include <vcl/jsdialog/executor.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <utility>

#define ACTION_TYPE "action_type"

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
        return;

    pButton->Click();
}
}

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

void JSDialogNotifyIdle::forceUpdate() { m_bForce = true; }

void JSDialogNotifyIdle::send(tools::JsonWriter& aJsonWriter)
{
    if (!m_aNotifierWindow)
    {
        free(aJsonWriter.extractData());
        return;
    }

    const vcl::ILibreOfficeKitNotifier* pNotifier = m_aNotifierWindow->GetLOKNotifier();
    if (pNotifier)
    {
        if (m_bForce || !aJsonWriter.isDataEquals(m_LastNotificationMessage))
        {
            m_bForce = false;
            m_LastNotificationMessage = aJsonWriter.extractAsStdString();
            pNotifier->libreOfficeKitViewCallback(LOK_CALLBACK_JSDIALOG,
                                                  m_LastNotificationMessage.c_str());
        }
        else
        {
            free(aJsonWriter.extractData());
        }
    }
    else
    {
        free(aJsonWriter.extractData());
    }
}

namespace
{
OUString extractActionType(const ActionDataMap& rData)
{
    auto it = rData.find(ACTION_TYPE);
    if (it != rData.end())
        return it->second;
    return "";
}
};

void JSDialogNotifyIdle::sendMessage(jsdialog::MessageType eType, VclPtr<vcl::Window> pWindow,
                                     std::unique_ptr<ActionDataMap> pData)
{
    // we want only the latest update of same type
    // TODO: also if we met full update - previous updates are not valid
    auto it = m_aMessageQueue.begin();

    while (it != m_aMessageQueue.end())
    {
        if (it->m_eType == eType && it->m_pWindow == pWindow)
        {
            if (it->m_pData && pData
                && extractActionType(*it->m_pData) != extractActionType(*pData))
            {
                it++;
                continue;
            }
            it = m_aMessageQueue.erase(it);
        }
        else
            it++;
    }

    JSDialogMessageInfo aMessage(eType, pWindow, std::move(pData));
    m_aMessageQueue.push_back(aMessage);
}

std::unique_ptr<tools::JsonWriter> JSDialogNotifyIdle::generateFullUpdate() const
{
    std::unique_ptr<tools::JsonWriter> aJsonWriter(new tools::JsonWriter());

    if (!m_aContentWindow || !m_aNotifierWindow)
        return aJsonWriter;

    m_aContentWindow->DumpAsPropertyTree(*aJsonWriter);
    aJsonWriter->put("id", m_aNotifierWindow->GetLOKWindowId());
    aJsonWriter->put("jsontype", m_sTypeOfJSON);

    if (m_sTypeOfJSON == "autofilter")
    {
        vcl::Window* pWindow = m_aContentWindow.get();
        DockingWindow* pDockingWindow = dynamic_cast<DockingWindow*>(pWindow);
        while (pWindow && !pDockingWindow)
        {
            pWindow = pWindow->GetParent();
            pDockingWindow = dynamic_cast<DockingWindow*>(pWindow);
        }

        if (pDockingWindow)
        {
            Point aPos = pDockingWindow->GetFloatingPos();
            aJsonWriter->put("posx", aPos.getX());
            aJsonWriter->put("posy", aPos.getY());
            if (!pDockingWindow->IsVisible())
                aJsonWriter->put("visible", "false");
        }
    }

    return aJsonWriter;
}

std::unique_ptr<tools::JsonWriter>
JSDialogNotifyIdle::generateWidgetUpdate(VclPtr<vcl::Window> pWindow) const
{
    std::unique_ptr<tools::JsonWriter> aJsonWriter(new tools::JsonWriter());

    aJsonWriter->put("jsontype", m_sTypeOfJSON);
    aJsonWriter->put("action", "update");
    aJsonWriter->put("id", m_aNotifierWindow->GetLOKWindowId());
    {
        auto aEntries = aJsonWriter->startNode("control");
        pWindow->DumpAsPropertyTree(*aJsonWriter);
    }

    return aJsonWriter;
}

std::unique_ptr<tools::JsonWriter> JSDialogNotifyIdle::generateCloseMessage() const
{
    std::unique_ptr<tools::JsonWriter> aJsonWriter(new tools::JsonWriter());
    if (m_aNotifierWindow)
        aJsonWriter->put("id", m_aNotifierWindow->GetLOKWindowId());
    aJsonWriter->put("jsontype", m_sTypeOfJSON);
    aJsonWriter->put("action", "close");

    return aJsonWriter;
}

std::unique_ptr<tools::JsonWriter>
JSDialogNotifyIdle::generateActionMessage(VclPtr<vcl::Window> pWindow,
                                          std::unique_ptr<ActionDataMap> pData) const
{
    std::unique_ptr<tools::JsonWriter> aJsonWriter(new tools::JsonWriter());

    aJsonWriter->put("jsontype", m_sTypeOfJSON);
    aJsonWriter->put("action", "action");
    aJsonWriter->put("id", m_aNotifierWindow->GetLOKWindowId());

    {
        auto aDataNode = aJsonWriter->startNode("data");
        aJsonWriter->put("control_id", pWindow->get_id());

        for (auto it = pData->begin(); it != pData->end(); it++)
            aJsonWriter->put(it->first.c_str(), it->second);
    }

    return aJsonWriter;
}

void JSDialogNotifyIdle::Invoke()
{
    for (auto& rMessage : m_aMessageQueue)
    {
        jsdialog::MessageType eType = rMessage.m_eType;

        switch (eType)
        {
            case jsdialog::MessageType::FullUpdate:
                send(*generateFullUpdate());
                break;

            case jsdialog::MessageType::WidgetUpdate:
                send(*generateWidgetUpdate(rMessage.m_pWindow));
                break;

            case jsdialog::MessageType::Close:
                send(*generateCloseMessage());
                break;

            case jsdialog::MessageType::Action:
                send(*generateActionMessage(rMessage.m_pWindow, std::move(rMessage.m_pData)));
                break;
        }
    }

    clearQueue();
}

void JSDialogNotifyIdle::clearQueue() { m_aMessageQueue.clear(); }

JSDialogSender::~JSDialogSender()
{
    sendClose();
    mpIdleNotify->Stop();
}

void JSDialogSender::sendFullUpdate(bool bForce)
{
    if (bForce)
        mpIdleNotify->forceUpdate();

    mpIdleNotify->sendMessage(jsdialog::MessageType::FullUpdate, nullptr);
    mpIdleNotify->Start();
}

void JSDialogSender::sendClose()
{
    mpIdleNotify->clearQueue();
    mpIdleNotify->sendMessage(jsdialog::MessageType::Close, nullptr);
    flush();
}

void JSDialogSender::sendUpdate(VclPtr<vcl::Window> pWindow, bool bForce)
{
    if (bForce)
        mpIdleNotify->forceUpdate();

    mpIdleNotify->sendMessage(jsdialog::MessageType::WidgetUpdate, pWindow);
    mpIdleNotify->Start();
}

void JSDialogSender::sendAction(VclPtr<vcl::Window> pWindow, std::unique_ptr<ActionDataMap> pData)
{
    mpIdleNotify->sendMessage(jsdialog::MessageType::Action, pWindow, std::move(pData));
    mpIdleNotify->Start();
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
    vcl::Window* pRoot = m_xBuilder->get_widget_root();
    if (pRoot && pRoot->GetParent())
    {
        m_aParentDialog = pRoot->GetParent()->GetParentWithLOKNotifier();
        if (m_aParentDialog)
            m_nWindowId = m_aParentDialog->GetLOKWindowId();
        InsertWindowToMap(m_nWindowId);
    }

    initializeSender(GetNotifierWindow(), GetContentWindow(), GetTypeOfJSON());
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
    vcl::Window* pRoot = m_xBuilder->get_widget_root();
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

    initializeSender(GetNotifierWindow(), GetContentWindow(), GetTypeOfJSON());
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
    vcl::Window* pRoot = m_xBuilder->get_widget_root();
    m_aContentWindow = pParent;
    if (pRoot && pRoot->GetParent())
    {
        m_aParentDialog = pRoot->GetParent()->GetParentWithLOKNotifier();
        if (m_aParentDialog)
            m_nWindowId = m_aParentDialog->GetLOKWindowId();
        InsertWindowToMap(m_nWindowId);
    }

    initializeSender(GetNotifierWindow(), GetContentWindow(), GetTypeOfJSON());
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

void JSInstanceBuilder::AddChildWidget(sal_uInt64 nWindowId, const OString& id,
                                       weld::Widget* pWidget)
{
    auto it = GetLOKWeldWidgetsMap().find(nWindowId);
    if (it != GetLOKWeldWidgetsMap().end())
    {
        it->second.erase(id);
        it->second.insert(WidgetMap::value_type(id, pWidget));
    }
}

void JSInstanceBuilder::RemoveWindowWidget(sal_uInt64 nWindowId)
{
    auto it = JSInstanceBuilder::GetLOKWeldWidgetsMap().find(nWindowId);
    if (it != JSInstanceBuilder::GetLOKWeldWidgetsMap().end())
    {
        JSInstanceBuilder::GetLOKWeldWidgetsMap().erase(it);
    }
}

const std::string& JSInstanceBuilder::GetTypeOfJSON() { return m_sTypeOfJSON; }

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

std::unique_ptr<weld::Dialog> JSInstanceBuilder::weld_dialog(const OString& id)
{
    std::unique_ptr<weld::Dialog> pRet;
    ::Dialog* pDialog = m_xBuilder->get<::Dialog>(id);

    if (pDialog)
    {
        m_nWindowId = pDialog->GetLOKWindowId();
        pDialog->SetLOKTunnelingState(false);

        InsertWindowToMap(m_nWindowId);

        assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
        m_aOwnedToplevel.set(pDialog);
        m_xBuilder->drop_ownership(pDialog);
        m_bHasTopLevelDialog = true;

        pRet.reset(new JSDialog(this, pDialog, this, false));

        RememberWidget("__DIALOG__", pRet.get());

        initializeSender(GetNotifierWindow(), GetContentWindow(), GetTypeOfJSON());
        sendFullUpdate();
    }

    return pRet;
}

std::unique_ptr<weld::MessageDialog> JSInstanceBuilder::weld_message_dialog(const OString& id)
{
    std::unique_ptr<weld::MessageDialog> pRet;
    ::MessageDialog* pMessageDialog = m_xBuilder->get<::MessageDialog>(id);

    if (pMessageDialog)
    {
        m_nWindowId = pMessageDialog->GetLOKWindowId();
        pMessageDialog->SetLOKTunnelingState(false);

        InsertWindowToMap(m_nWindowId);

        assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
        m_aOwnedToplevel.set(pMessageDialog);
        m_xBuilder->drop_ownership(pMessageDialog);
        m_bHasTopLevelDialog = true;

        initializeSender(GetNotifierWindow(), GetContentWindow(), GetTypeOfJSON());
    }

    pRet.reset(pMessageDialog ? new JSMessageDialog(this, pMessageDialog, this, false) : nullptr);

    if (pRet)
        RememberWidget("__DIALOG__", pRet.get());

    return pRet;
}

std::unique_ptr<weld::Label> JSInstanceBuilder::weld_label(const OString& id)
{
    ::FixedText* pLabel = m_xBuilder->get<FixedText>(id);
    auto pWeldWidget = std::make_unique<JSLabel>(this, pLabel, this, false);

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Button> JSInstanceBuilder::weld_button(const OString& id)
{
    ::Button* pButton = m_xBuilder->get<::Button>(id);
    auto pWeldWidget = pButton ? std::make_unique<JSButton>(this, pButton, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Entry> JSInstanceBuilder::weld_entry(const OString& id)
{
    Edit* pEntry = m_xBuilder->get<Edit>(id);
    auto pWeldWidget = pEntry ? std::make_unique<JSEntry>(this, pEntry, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::ComboBox> JSInstanceBuilder::weld_combo_box(const OString& id)
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

std::unique_ptr<weld::Notebook> JSInstanceBuilder::weld_notebook(const OString& id)
{
    TabControl* pNotebook = m_xBuilder->get<TabControl>(id);
    auto pWeldWidget
        = pNotebook ? std::make_unique<JSNotebook>(this, pNotebook, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::SpinButton> JSInstanceBuilder::weld_spin_button(const OString& id)
{
    FormattedField* pSpinButton = m_xBuilder->get<FormattedField>(id);
    auto pWeldWidget
        = pSpinButton ? std::make_unique<JSSpinButton>(this, pSpinButton, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::CheckButton> JSInstanceBuilder::weld_check_button(const OString& id)
{
    CheckBox* pCheckButton = m_xBuilder->get<CheckBox>(id);
    auto pWeldWidget
        = pCheckButton ? std::make_unique<JSCheckButton>(this, pCheckButton, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::DrawingArea>
JSInstanceBuilder::weld_drawing_area(const OString& id, const a11yref& rA11yImpl,
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

std::unique_ptr<weld::Toolbar> JSInstanceBuilder::weld_toolbar(const OString& id)
{
    ToolBox* pToolBox = m_xBuilder->get<ToolBox>(id);
    auto pWeldWidget
        = pToolBox ? std::make_unique<JSToolbar>(this, pToolBox, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::TextView> JSInstanceBuilder::weld_text_view(const OString& id)
{
    VclMultiLineEdit* pTextView = m_xBuilder->get<VclMultiLineEdit>(id);
    auto pWeldWidget
        = pTextView ? std::make_unique<JSTextView>(this, pTextView, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::TreeView> JSInstanceBuilder::weld_tree_view(const OString& id)
{
    SvTabListBox* pTreeView = m_xBuilder->get<SvTabListBox>(id);
    auto pWeldWidget
        = pTreeView ? std::make_unique<JSTreeView>(this, pTreeView, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Expander> JSInstanceBuilder::weld_expander(const OString& id)
{
    VclExpander* pExpander = m_xBuilder->get<VclExpander>(id);
    auto pWeldWidget
        = pExpander ? std::make_unique<JSExpander>(this, pExpander, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::IconView> JSInstanceBuilder::weld_icon_view(const OString& id)
{
    ::IconView* pIconView = m_xBuilder->get<::IconView>(id);
    auto pWeldWidget
        = pIconView ? std::make_unique<JSIconView>(this, pIconView, this, false) : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::RadioButton> JSInstanceBuilder::weld_radio_button(const OString& id)
{
    ::RadioButton* pRadioButton = m_xBuilder->get<::RadioButton>(id);
    auto pWeldWidget
        = pRadioButton ? std::make_unique<JSRadioButton>(this, pRadioButton, this, false) : nullptr;

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
        tools::JsonWriter aJsonWriter;
        xMessageDialog->DumpAsPropertyTree(aJsonWriter);
        aJsonWriter.put("id", xMessageDialog->GetLOKWindowId());
        aJsonWriter.put("jsontype", "dialog");
        std::unique_ptr<char[], o3tl::free_delete> message(aJsonWriter.extractData());
        pNotifier->libreOfficeKitViewCallback(LOK_CALLBACK_JSDIALOG, message.get());
    }

    xMessageDialog->SetLOKTunnelingState(false);
    InsertWindowToMap(xMessageDialog->GetLOKWindowId());
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

JSLabel::JSLabel(JSDialogSender* pSender, FixedText* pLabel, SalInstanceBuilder* pBuilder,
                 bool bTakeOwnership)
    : JSWidget<SalInstanceLabel, FixedText>(pSender, pLabel, pBuilder, bTakeOwnership)
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

void JSComboBox::set_entry_text(const OUString& rText)
{
    SalInstanceComboBoxWithEdit::set_entry_text(rText);
    sendUpdate();
}

void JSComboBox::set_active(int pos)
{
    SalInstanceComboBoxWithEdit::set_active(pos);
    sendUpdate();
}

JSNotebook::JSNotebook(JSDialogSender* pSender, ::TabControl* pControl,
                       SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceNotebook, ::TabControl>(pSender, pControl, pBuilder, bTakeOwnership)
{
}

void JSNotebook::set_current_page(int nPage)
{
    bool bForce = false;
    int nCurrent = get_current_page();
    if (nCurrent == nPage)
        bForce = true;

    SalInstanceNotebook::set_current_page(nPage);
    sendFullUpdate(bForce);
}

void JSNotebook::set_current_page(const OString& rIdent)
{
    bool bForce = false;
    OString sCurrent = get_current_page_ident();
    if (sCurrent == rIdent)
        bForce = true;

    SalInstanceNotebook::set_current_page(rIdent);
    sendFullUpdate(bForce);
}

void JSNotebook::remove_page(const OString& rIdent)
{
    SalInstanceNotebook::remove_page(rIdent);
    sendFullUpdate();
}

void JSNotebook::insert_page(const OString& rIdent, const OUString& rLabel, int nPos)
{
    SalInstanceNotebook::insert_page(rIdent, rLabel, nPos);
    sendFullUpdate();
}

JSSpinButton::JSSpinButton(JSDialogSender* pSender, ::FormattedField* pSpin,
                           SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceSpinButton, ::FormattedField>(pSender, pSpin, pBuilder, bTakeOwnership)
{
}

void JSSpinButton::set_value(int value)
{
    SalInstanceSpinButton::set_value(value);
    sendUpdate(true); // if input is limited we can receive the same JSON
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
    , m_pOwnedSender(new JSDialogSender(pDialog, pDialog, "dialog"))
{
    m_pSender = m_pOwnedSender.get();

    if (!pBuilder)
    {
        if (::OKButton* pOKBtn
            = dynamic_cast<::OKButton*>(m_xMessageDialog->get_widget_for_response(RET_OK)))
        {
            m_pOK.reset(new JSButton(m_pSender, pOKBtn, nullptr, false));
            JSInstanceBuilder::AddChildWidget(m_xMessageDialog->GetLOKWindowId(),
                                              pOKBtn->get_id().toUtf8(), m_pOK.get());
            m_pOK->connect_clicked(LINK(this, JSMessageDialog, OKHdl));
        }

        if (::CancelButton* pCancelBtn
            = dynamic_cast<::CancelButton*>(m_xMessageDialog->get_widget_for_response(RET_CANCEL)))
        {
            m_pCancel.reset(new JSButton(m_pSender, pCancelBtn, nullptr, false));
            JSInstanceBuilder::AddChildWidget(m_xMessageDialog->GetLOKWindowId(),
                                              pCancelBtn->get_id().toUtf8(), m_pCancel.get());
            m_pCancel->connect_clicked(LINK(this, JSMessageDialog, CancelHdl));
        }
    }
}

JSMessageDialog::~JSMessageDialog()
{
    if (m_pOK || m_pCancel)
        JSInstanceBuilder::RemoveWindowWidget(m_xMessageDialog->GetLOKWindowId());
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
    SalInstanceCheckButton::set_active(active);
    sendUpdate();
}

JSDrawingArea::JSDrawingArea(JSDialogSender* pSender, VclDrawingArea* pDrawingArea,
                             SalInstanceBuilder* pBuilder, const a11yref& rAlly,
                             FactoryFunction pUITestFactoryFunction, void* pUserData)
    : JSWidget<SalInstanceDrawingArea, VclDrawingArea>(pSender, pDrawingArea, pBuilder, rAlly,
                                                       pUITestFactoryFunction, pUserData, false)
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
    sendUpdate();
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

void JSTreeView::expand_row(const weld::TreeIter& rIter)
{
    SalInstanceTreeView::expand_row(rIter);
    sendUpdate();
}

void JSTreeView::collapse_row(const weld::TreeIter& rIter)
{
    SalInstanceTreeView::collapse_row(rIter);
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

void JSIconView::insert(int pos, const OUString* pStr, const OUString* pId,
                        const VirtualDevice* pIcon, weld::TreeIter* pRet)
{
    SalInstanceIconView::insert(pos, pStr, pId, pIcon, pRet);
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

    std::unique_ptr<ActionDataMap> pMap = std::make_unique<ActionDataMap>();
    (*pMap)[ACTION_TYPE] = "select";
    (*pMap)["position"] = OUString::number(pos);
    sendAction(std::move(pMap));
}

void JSIconView::unselect(int pos)
{
    SalInstanceIconView::unselect(pos);
    sendUpdate();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
