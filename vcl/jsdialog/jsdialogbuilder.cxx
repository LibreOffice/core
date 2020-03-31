#include <vcl/jsdialog/jsdialogbuilder.hxx>
#include <sal/log.hxx>
#include <boost/property_tree/json_parser.hpp>
#include <comphelper/lok.hxx>
#include <vcl/dialog.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

using namespace weld;

JSDialogNotifyIdle::JSDialogNotifyIdle(VclPtr<vcl::Window> aWindow)
    : Idle("JSDialog notify")
    , m_aWindow(aWindow)
    , m_LastNotificationMessage()
{
    SetPriority(TaskPriority::POST_PAINT);
}

void JSDialogNotifyIdle::Invoke()
{
    try
    {
        if (!m_aWindow)
            return;

        const vcl::ILibreOfficeKitNotifier* pNotifier = m_aWindow->GetLOKNotifier();
        if (pNotifier)
        {
            std::stringstream aStream;
            boost::property_tree::ptree aTree = m_aWindow->DumpAsPropertyTree();
            aTree.put("id", m_aWindow->GetLOKWindowId());
            boost::property_tree::write_json(aStream, aTree);
            const std::string message = aStream.str();
            if (message != m_LastNotificationMessage)
            {
                m_LastNotificationMessage = message;
                pNotifier->libreOfficeKitViewCallback(LOK_CALLBACK_JSDIALOG, message.c_str());
            }
        }
    }
    catch (boost::property_tree::json_parser::json_parser_error& rError)
    {
        SAL_WARN("vcl.jsdialog", rError.message());
    }
}

void JSDialogSender::notifyDialogState() { mpIdleNotify->Start(); }

JSInstanceBuilder::JSInstanceBuilder(weld::Widget* pParent, const OUString& rUIRoot,
                                     const OUString& rUIFile)
    : SalInstanceBuilder(dynamic_cast<SalInstanceWidget*>(pParent)
                             ? dynamic_cast<SalInstanceWidget*>(pParent)->getWidget()
                             : nullptr,
                         rUIRoot, rUIFile)
    , m_nWindowId(0)
    , m_aParentDialog(nullptr)
    , m_bHasTopLevelDialog(false)
{
    vcl::Window* pRoot = get_builder().get_widget_root();
    if (pRoot && pRoot->GetParent())
    {
        m_aParentDialog = pRoot->GetParent()->GetParentWithLOKNotifier();
        m_nWindowId = m_aParentDialog->GetLOKWindowId();
        InsertWindowToMap(m_nWindowId);
    }
}

JSInstanceBuilder::~JSInstanceBuilder()
{
    if (m_nWindowId && m_bHasTopLevelDialog)
        GetLOKWeldWidgetsMap().erase(m_nWindowId);
}

std::map<vcl::LOKWindowId, WidgetMap>& JSInstanceBuilder::GetLOKWeldWidgetsMap()
{
    // Map to remember the LOKWindowId <-> weld widgets binding.
    static std::map<vcl::LOKWindowId, WidgetMap> s_aLOKWeldBuildersMap;

    return s_aLOKWeldBuildersMap;
}

weld::Widget* JSInstanceBuilder::FindWeldWidgetsMap(vcl::LOKWindowId nWindowId,
                                                    const OString& rWidget)
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

void JSInstanceBuilder::InsertWindowToMap(int nWindowId)
{
    WidgetMap map;
    auto it = GetLOKWeldWidgetsMap().find(nWindowId);
    if (it == GetLOKWeldWidgetsMap().end())
        GetLOKWeldWidgetsMap().insert(
            std::map<vcl::LOKWindowId, WidgetMap>::value_type(nWindowId, map));
}

void JSInstanceBuilder::RememberWidget(const OString& id, weld::Widget* pWidget)
{
    auto it = GetLOKWeldWidgetsMap().find(m_nWindowId);
    if (it != GetLOKWeldWidgetsMap().end())
    {
        it->second.insert(WidgetMap::value_type(id, pWidget));
    }
}

std::unique_ptr<weld::Dialog> JSInstanceBuilder::weld_dialog(const OString& id, bool bTakeOwnership)
{
    ::Dialog* pDialog = m_xBuilder->get<::Dialog>(id);
    m_nWindowId = pDialog->GetLOKWindowId();

    InsertWindowToMap(m_nWindowId);

    std::unique_ptr<weld::Dialog> pRet(pDialog ? new SalInstanceDialog(pDialog, this, false)
                                               : nullptr);
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
        std::stringstream aStream;
        boost::property_tree::ptree aTree = m_aOwnedToplevel->DumpAsPropertyTree();
        aTree.put("id", m_aOwnedToplevel->GetLOKWindowId());
        boost::property_tree::write_json(aStream, aTree);
        const std::string message = aStream.str();
        pNotifier->libreOfficeKitViewCallback(LOK_CALLBACK_JSDIALOG, message.c_str());
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
    SalInstanceNotebook::set_current_page(nPage);
    notifyDialogState();
}

void JSNotebook::set_current_page(const OString& rIdent)
{
    SalInstanceNotebook::set_current_page(rIdent);
    notifyDialogState();
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
