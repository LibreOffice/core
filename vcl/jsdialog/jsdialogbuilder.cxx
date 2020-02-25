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

JSLabel::JSLabel(VclPtr<vcl::Window> aOwnedToplevel, FixedText* pLabel,
                 SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : SalInstanceLabel(pLabel, pBuilder, bTakeOwnership)
    , JSDialogSender(aOwnedToplevel)
{
}

void JSLabel::set_label(const OUString& rText)
{
    SalInstanceLabel::set_label(rText);
    notifyDialogState();
};

JSEntry::JSEntry(VclPtr<vcl::Window> aOwnedToplevel, ::Edit* pEntry, SalInstanceBuilder* pBuilder,
                 bool bTakeOwnership)
    : SalInstanceEntry(pEntry, pBuilder, bTakeOwnership)
    , JSDialogSender(aOwnedToplevel)
{
}

void JSEntry::set_text(const OUString& rText)
{
    SalInstanceEntry::set_text(rText);
    notifyDialogState();
}

JSListBox::JSListBox(VclPtr<vcl::Window> aOwnedToplevel, ::ListBox* pListBox,
                     SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : SalInstanceComboBoxWithoutEdit(pListBox, pBuilder, bTakeOwnership)
    , JSDialogSender(aOwnedToplevel)
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
    : SalInstanceComboBoxWithEdit(pComboBox, pBuilder, bTakeOwnership)
    , JSDialogSender(aOwnedToplevel)
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
