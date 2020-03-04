#include <jsdialog/jsdialogbuilder.hxx>
#include <sal/log.hxx>
#include <boost/property_tree/json_parser.hpp>
#include <comphelper/lok.hxx>
#include <vcl/toolkit/dialog.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

void JSDialogSender::notifyDialogState()
{
    if (!m_aOwnedToplevel)
        return;

    const vcl::ILibreOfficeKitNotifier* pNotifier = m_aOwnedToplevel->GetLOKNotifier();
    if (pNotifier)
    {
        std::stringstream aStream;
        boost::property_tree::ptree aTree = m_aOwnedToplevel->DumpAsPropertyTree();
        aTree.put("id", m_aOwnedToplevel->GetLOKWindowId());
        boost::property_tree::write_json(aStream, aTree);
        const std::string message = aStream.str();
        pNotifier->libreOfficeKitViewCallback(LOK_CALLBACK_JSDIALOG, message.c_str());
    }
}

JSInstanceBuilder::JSInstanceBuilder(weld::Widget* pParent, const OUString& rUIRoot,
                                     const OUString& rUIFile)
    : SalInstanceBuilder(dynamic_cast<SalInstanceWidget*>(pParent)
                             ? dynamic_cast<SalInstanceWidget*>(pParent)->getWidget()
                             : nullptr,
                         rUIRoot, rUIFile)
{
}

std::unique_ptr<weld::Dialog> JSInstanceBuilder::weld_dialog(const OString& id, bool bTakeOwnership)
{
    ::Dialog* pDialog = m_xBuilder->get<::Dialog>(id);
    std::unique_ptr<weld::Dialog> pRet(pDialog ? new SalInstanceDialog(pDialog, this, false)
                                               : nullptr);
    if (bTakeOwnership && pDialog)
    {
        assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
        m_aOwnedToplevel.set(pDialog);
        m_xBuilder->drop_ownership(pDialog);
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
    return std::make_unique<JSLabel>(m_aOwnedToplevel, pLabel, this, bTakeOwnership);
}

std::unique_ptr<weld::Button> JSInstanceBuilder::weld_button(const OString& id, bool bTakeOwnership)
{
    ::Button* pButton = m_xBuilder->get<::Button>(id);
    return pButton ? std::make_unique<JSButton>(m_aOwnedToplevel, pButton, this, bTakeOwnership)
                   : nullptr;
}

std::unique_ptr<weld::Entry> JSInstanceBuilder::weld_entry(const OString& id, bool bTakeOwnership)
{
    Edit* pEntry = m_xBuilder->get<Edit>(id);
    return pEntry ? std::make_unique<JSEntry>(m_aOwnedToplevel, pEntry, this, bTakeOwnership)
                  : nullptr;
}

std::unique_ptr<weld::ComboBox> JSInstanceBuilder::weld_combo_box(const OString& id,
                                                                  bool bTakeOwnership)
{
    vcl::Window* pWidget = m_xBuilder->get<vcl::Window>(id);
    ::ComboBox* pComboBox = dynamic_cast<::ComboBox*>(pWidget);
    if (pComboBox)
        return std::make_unique<JSComboBox>(m_aOwnedToplevel, pComboBox, this, bTakeOwnership);
    ListBox* pListBox = dynamic_cast<ListBox*>(pWidget);
    return pListBox ? std::make_unique<JSListBox>(m_aOwnedToplevel, pListBox, this, bTakeOwnership)
                    : nullptr;
}

std::unique_ptr<weld::Notebook> JSInstanceBuilder::weld_notebook(const OString& id,
                                                                 bool bTakeOwnership)
{
    TabControl* pNotebook = m_xBuilder->get<TabControl>(id);
    return pNotebook
               ? std::make_unique<JSNotebook>(m_aOwnedToplevel, pNotebook, this, bTakeOwnership)
               : nullptr;
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

void JSNotebook::insert_page(const OString& rIdent, const OUString& rLabel, int nPos)
{
    SalInstanceNotebook::insert_page(rIdent, rLabel, nPos);
    notifyDialogState();
}
