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

JSDialogNotifyIdle::JSDialogNotifyIdle(VclPtr<vcl::Window> aWindow)
    : Idle("JSDialog notify")
    , m_aWindow(aWindow)
    , m_LastNotificationMessage()
    , m_bForce(false)
{
    SetPriority(TaskPriority::POST_PAINT);
}

void JSDialogNotifyIdle::ForceUpdate() { m_bForce = true; }

void JSDialogNotifyIdle::Invoke()
{
    try
    {
        if (!m_aWindow)
            return;

        const vcl::ILibreOfficeKitNotifier* pNotifier = m_aWindow->GetLOKNotifier();
        if (pNotifier)
        {
            tools::JsonWriter aJsonWriter;
            m_aWindow->DumpAsPropertyTree(aJsonWriter);
            aJsonWriter.put("id", m_aWindow->GetLOKWindowId());
            if (m_bForce || !aJsonWriter.isDataEquals(m_LastNotificationMessage))
            {
                m_bForce = false;
                m_LastNotificationMessage = aJsonWriter.extractAsStdString();
                pNotifier->libreOfficeKitViewCallback(LOK_CALLBACK_JSDIALOG,
                                                      m_LastNotificationMessage.c_str());
            }
        }
    }
    catch (boost::property_tree::json_parser::json_parser_error& rError)
    {
        SAL_WARN("vcl", rError.message());
    }
}

void JSDialogSender::notifyDialogState(bool bForce)
{
    if (bForce)
        mpIdleNotify->ForceUpdate();
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

JSInstanceBuilder::JSInstanceBuilder(weld::Widget* pParent, const OUString& rUIRoot,
                                     const OUString& rUIFile)
    : SalInstanceBuilder(extract_sal_widget(pParent), rUIRoot, rUIFile)
    , m_nWindowId(0)
    , m_aParentDialog(nullptr)
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
}

JSInstanceBuilder::JSInstanceBuilder(vcl::Window* pParent, const OUString& rUIRoot,
                                     const OUString& rUIFile,
                                     const css::uno::Reference<css::frame::XFrame>& rFrame,
                                     sal_uInt64 nWindowId)
    : SalInstanceBuilder(pParent, rUIRoot, rUIFile, rFrame)
    , m_nWindowId(0)
    , m_aParentDialog(nullptr)
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
}

JSInstanceBuilder::~JSInstanceBuilder()
{
    if (m_nWindowId && (m_bHasTopLevelDialog || m_bIsNotebookbar))
        GetLOKWeldWidgetsMap().erase(m_nWindowId);
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
    }
}

std::unique_ptr<weld::Dialog> JSInstanceBuilder::weld_dialog(const OString& id, bool bTakeOwnership)
{
    ::Dialog* pDialog = m_xBuilder->get<::Dialog>(id);
    m_nWindowId = pDialog->GetLOKWindowId();

    InsertWindowToMap(m_nWindowId);

    std::unique_ptr<weld::Dialog> pRet(
        pDialog ? new JSDialog(m_aOwnedToplevel, pDialog, this, false) : nullptr);
    if (bTakeOwnership && pDialog)
    {
        assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
        m_aOwnedToplevel.set(pDialog);
        m_xBuilder->drop_ownership(pDialog);
        m_bHasTopLevelDialog = true;
    }

    const vcl::ILibreOfficeKitNotifier* pNotifier = pDialog->GetLOKNotifier();
    if (pNotifier)
    {
        tools::JsonWriter aJsonWriter;
        m_aOwnedToplevel->DumpAsPropertyTree(aJsonWriter);
        aJsonWriter.put("id", m_aOwnedToplevel->GetLOKWindowId());
        pNotifier->libreOfficeKitViewCallback(LOK_CALLBACK_JSDIALOG, aJsonWriter.extractData());
    }

    return pRet;
}

std::unique_ptr<weld::Label> JSInstanceBuilder::weld_label(const OString& id, bool bTakeOwnership)
{
    ::FixedText* pLabel = m_xBuilder->get<FixedText>(id);
    auto pWeldWidget = std::make_unique<JSLabel>(
        m_bHasTopLevelDialog ? m_aOwnedToplevel : m_aParentDialog, pLabel, this, bTakeOwnership);

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Button> JSInstanceBuilder::weld_button(const OString& id, bool bTakeOwnership)
{
    ::Button* pButton = m_xBuilder->get<::Button>(id);
    auto pWeldWidget = pButton ? std::make_unique<JSButton>(m_bHasTopLevelDialog ? m_aOwnedToplevel
                                                                                 : m_aParentDialog,
                                                            pButton, this, bTakeOwnership)
                               : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Entry> JSInstanceBuilder::weld_entry(const OString& id, bool bTakeOwnership)
{
    Edit* pEntry = m_xBuilder->get<Edit>(id);
    auto pWeldWidget = pEntry ? std::make_unique<JSEntry>(m_bHasTopLevelDialog ? m_aOwnedToplevel
                                                                               : m_aParentDialog,
                                                          pEntry, this, bTakeOwnership)
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
        pWeldWidget = std::make_unique<JSComboBox>(m_bHasTopLevelDialog ? m_aOwnedToplevel
                                                                        : m_aParentDialog,
                                                   pComboBox, this, bTakeOwnership);
    }
    else
    {
        ListBox* pListBox = dynamic_cast<ListBox*>(pWidget);
        pWeldWidget = pListBox ? std::make_unique<JSListBox>(m_bHasTopLevelDialog ? m_aOwnedToplevel
                                                                                  : m_aParentDialog,
                                                             pListBox, this, bTakeOwnership)
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
    auto pWeldWidget = pNotebook ? std::make_unique<JSNotebook>(
                                       m_bHasTopLevelDialog ? m_aOwnedToplevel : m_aParentDialog,
                                       pNotebook, this, bTakeOwnership)
                                 : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::SpinButton> JSInstanceBuilder::weld_spin_button(const OString& id,
                                                                      bool bTakeOwnership)
{
    FormattedField* pSpinButton = m_xBuilder->get<FormattedField>(id);
    auto pWeldWidget = pSpinButton ? std::make_unique<JSSpinButton>(
                                         m_bHasTopLevelDialog ? m_aOwnedToplevel : m_aParentDialog,
                                         pSpinButton, this, bTakeOwnership)
                                   : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::CheckButton> JSInstanceBuilder::weld_check_button(const OString& id,
                                                                        bool bTakeOwnership)
{
    CheckBox* pCheckButton = m_xBuilder->get<CheckBox>(id);
    auto pWeldWidget = pCheckButton ? std::make_unique<JSCheckButton>(
                                          m_bHasTopLevelDialog ? m_aOwnedToplevel : m_aParentDialog,
                                          pCheckButton, this, bTakeOwnership)
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
    auto pWeldWidget = pArea
                           ? std::make_unique<JSDrawingArea>(
                                 m_bHasTopLevelDialog ? m_aOwnedToplevel : m_aParentDialog, pArea,
                                 this, rA11yImpl, pUITestFactoryFunction, pUserData, bTakeOwnership)
                           : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::Toolbar> JSInstanceBuilder::weld_toolbar(const OString& id,
                                                               bool bTakeOwnership)
{
    ToolBox* pToolBox = m_xBuilder->get<ToolBox>(id);
    auto pWeldWidget = pToolBox ? std::make_unique<JSToolbar>(
                                      m_bHasTopLevelDialog ? m_aOwnedToplevel : m_aParentDialog,
                                      pToolBox, this, bTakeOwnership)
                                : nullptr;

    if (pWeldWidget)
        RememberWidget(id, pWeldWidget.get());

    return pWeldWidget;
}

std::unique_ptr<weld::TextView> JSInstanceBuilder::weld_text_view(const OString& id,
                                                                  bool bTakeOwnership)
{
    VclMultiLineEdit* pTextView = m_xBuilder->get<VclMultiLineEdit>(id);
    auto pWeldWidget = pTextView ? std::make_unique<JSTextView>(
                                       m_bHasTopLevelDialog ? m_aOwnedToplevel : m_aParentDialog,
                                       pTextView, this, bTakeOwnership)
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
        tools::JsonWriter aJsonWriter;
        xMessageDialog->DumpAsPropertyTree(aJsonWriter);
        aJsonWriter.put("id", xMessageDialog->GetLOKWindowId());
        std::unique_ptr<char[], o3tl::free_delete> message(aJsonWriter.extractData());
        pNotifier->libreOfficeKitViewCallback(LOK_CALLBACK_JSDIALOG, message.get());
    }

    return new JSMessageDialog(xMessageDialog, nullptr, true);
}

JSDialog::JSDialog(VclPtr<vcl::Window> aOwnedToplevel, ::Dialog* pDialog,
                   SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceDialog, ::Dialog>(aOwnedToplevel, pDialog, pBuilder, bTakeOwnership)
{
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

JSLabel::JSLabel(VclPtr<vcl::Window> aOwnedToplevel, FixedText* pLabel,
                 SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceLabel, FixedText>(aOwnedToplevel, pLabel, pBuilder, bTakeOwnership)
{
}

void JSLabel::set_label(const OUString& rText)
{
    SalInstanceLabel::set_label(rText);
    notifyDialogState();
};

JSButton::JSButton(VclPtr<vcl::Window> aOwnedToplevel, ::Button* pButton,
                   SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceButton, ::Button>(aOwnedToplevel, pButton, pBuilder, bTakeOwnership)
{
}

JSEntry::JSEntry(VclPtr<vcl::Window> aOwnedToplevel, ::Edit* pEntry, SalInstanceBuilder* pBuilder,
                 bool bTakeOwnership)
    : JSWidget<SalInstanceEntry, ::Edit>(aOwnedToplevel, pEntry, pBuilder, bTakeOwnership)
{
}

void JSEntry::set_text(const OUString& rText)
{
    SalInstanceEntry::set_text(rText);
    notifyDialogState();
}

JSListBox::JSListBox(VclPtr<vcl::Window> aOwnedToplevel, ::ListBox* pListBox,
                     SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceComboBoxWithoutEdit, ::ListBox>(aOwnedToplevel, pListBox, pBuilder,
                                                          bTakeOwnership)
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

JSComboBox::JSComboBox(VclPtr<vcl::Window> aOwnedToplevel, ::ComboBox* pComboBox,
                       SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceComboBoxWithEdit, ::ComboBox>(aOwnedToplevel, pComboBox, pBuilder,
                                                        bTakeOwnership)
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

JSNotebook::JSNotebook(VclPtr<vcl::Window> aOwnedToplevel, ::TabControl* pControl,
                       SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceNotebook, ::TabControl>(aOwnedToplevel, pControl, pBuilder,
                                                  bTakeOwnership)
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

void JSNotebook::insert_page(const OString& rIdent, const OUString& rLabel, int nPos)
{
    SalInstanceNotebook::insert_page(rIdent, rLabel, nPos);
    notifyDialogState();
}

JSSpinButton::JSSpinButton(VclPtr<vcl::Window> aOwnedToplevel, ::FormattedField* pSpin,
                           SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceSpinButton, ::FormattedField>(aOwnedToplevel, pSpin, pBuilder,
                                                        bTakeOwnership)
{
}

void JSSpinButton::set_value(int value)
{
    SalInstanceSpinButton::set_value(value);
    notifyDialogState();
}

JSMessageDialog::JSMessageDialog(::MessageDialog* pDialog, SalInstanceBuilder* pBuilder,
                                 bool bTakeOwnership)
    : SalInstanceMessageDialog(pDialog, pBuilder, bTakeOwnership)
    , JSDialogSender(m_xMessageDialog)
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

JSCheckButton::JSCheckButton(VclPtr<vcl::Window> aOwnedToplevel, ::CheckBox* pCheckBox,
                             SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceCheckButton, ::CheckBox>(aOwnedToplevel, pCheckBox, pBuilder,
                                                   bTakeOwnership)
{
}

void JSCheckButton::set_active(bool active)
{
    SalInstanceCheckButton::set_active(active);
    notifyDialogState();
}

JSDrawingArea::JSDrawingArea(VclPtr<vcl::Window> aOwnedToplevel, VclDrawingArea* pDrawingArea,
                             SalInstanceBuilder* pBuilder, const a11yref& rAlly,
                             FactoryFunction pUITestFactoryFunction, void* pUserData,
                             bool bTakeOwnership)
    : SalInstanceDrawingArea(pDrawingArea, pBuilder, rAlly, pUITestFactoryFunction, pUserData,
                             bTakeOwnership)
    , JSDialogSender(aOwnedToplevel)
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

JSToolbar::JSToolbar(VclPtr<vcl::Window> aOwnedToplevel, ::ToolBox* pToolbox,
                     SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceToolbar, ::ToolBox>(aOwnedToplevel, pToolbox, pBuilder, bTakeOwnership)
{
}

void JSToolbar::signal_clicked(const OString& rIdent)
{
    SalInstanceToolbar::signal_clicked(rIdent);
    notifyDialogState();
}

JSTextView::JSTextView(VclPtr<vcl::Window> aOwnedToplevel, ::VclMultiLineEdit* pTextView,
                       SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : JSWidget<SalInstanceTextView, ::VclMultiLineEdit>(aOwnedToplevel, pTextView, pBuilder,
                                                        bTakeOwnership)
{
}

void JSTextView::set_text(const OUString& rText)
{
    SalInstanceTextView::set_text(rText);
    notifyDialogState();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
