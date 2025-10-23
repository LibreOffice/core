/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs AI Assistant Panel
 */

#include "AIAssistantPanel.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/svapp.hxx>

namespace sw::sidebar {

std::unique_ptr<PanelLayout> AIAssistantPanel::Create(
    weld::Widget* pParent,
    SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException(
            u"no parent Window given to AIAssistantPanel::Create"_ustr, nullptr, 0);
    if (pBindings == nullptr)
        throw css::lang::IllegalArgumentException(
            u"no SfxBindings given to AIAssistantPanel::Create"_ustr, nullptr, 0);

    return std::make_unique<AIAssistantPanel>(pParent, pBindings);
}

AIAssistantPanel::AIAssistantPanel(
    weld::Widget* pParent,
    SfxBindings* pBindings)
    : PanelLayout(pParent, u"AIAssistantPanel"_ustr, u"modules/swriter/ui/chatpanel.ui"_ustr)
    , m_pBindings(pBindings)
{
    // Get UI components
    m_xStatusLabel = m_xBuilder->weld_label(u"status_label"_ustr);
    m_xChatHistory = m_xBuilder->weld_text_view(u"chat_history"_ustr);
    m_xInputField = m_xBuilder->weld_entry(u"input_field"_ustr);
    m_xSendButton = m_xBuilder->weld_button(u"send_button"_ustr);
    
    // Set up event handlers
    if (m_xSendButton)
        m_xSendButton->connect_clicked(LINK(this, AIAssistantPanel, SendClickHdl));
    
    if (m_xInputField)
        m_xInputField->connect_activate(LINK(this, AIAssistantPanel, InputActivateHdl));
    
    // Configure chat history
    if (m_xChatHistory)
        m_xChatHistory->set_editable(false);
    
    // Set initial status
    if (m_xStatusLabel)
        m_xStatusLabel->set_label(u"🤖 OfficeLabs AI - Ready"_ustr);
}

AIAssistantPanel::~AIAssistantPanel()
{
}

void AIAssistantPanel::SendMessage(const OUString& message)
{
    if (message.isEmpty())
        return;
    
    // Add user message to chat history
    if (m_xChatHistory)
    {
        OUString currentText = m_xChatHistory->get_text();
        OUString newText = currentText + u"\n👤 You: " + message + u"\n";
        
        // Simple echo response for now
        newText += u"🤖 AI: I received your message: \"" + message + u"\"\n";
        newText += u"    (Full AI integration coming soon!)\n";
        
        m_xChatHistory->set_text(newText);
        
        // Scroll to bottom
        m_xChatHistory->select_region(-1, -1);
    }
    
    // Clear input
    if (m_xInputField)
        m_xInputField->set_text(u""_ustr);
}

IMPL_LINK_NOARG(AIAssistantPanel, SendClickHdl, weld::Button&, void)
{
    if (m_xInputField)
    {
        OUString message = m_xInputField->get_text();
        if (!message.isEmpty())
            SendMessage(message);
    }
}

IMPL_LINK_NOARG(AIAssistantPanel, InputActivateHdl, weld::Entry&, bool)
{
    // Enter key pressed
    if (m_xSendButton)
        SendClickHdl(*m_xSendButton);
    return true;
}

} // end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */