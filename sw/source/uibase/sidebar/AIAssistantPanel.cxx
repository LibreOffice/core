/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs AI Assistant Panel
 */

#include "AIAssistantPanel.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/svapp.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <docsh.hxx>

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
    : PanelLayout(pParent, u"ChatPanel"_ustr, u"modules/swriter/ui/chatpanel.ui"_ustr)
    , m_pBindings(pBindings)
    , m_pWrtShell(nullptr)
    , m_bProcessing(false)
{
    // Get UI components
    m_xStatusLabel = m_xBuilder->weld_label(u"status_label"_ustr);
    m_xChatHistory = m_xBuilder->weld_text_view(u"chat_history"_ustr);
    m_xInputField = m_xBuilder->weld_entry(u"input_field"_ustr);
    m_xSendButton = m_xBuilder->weld_button(u"send_button"_ustr);
    m_xInsertButton = m_xBuilder->weld_button(u"insert_button"_ustr);

    // Set up event handlers
    if (m_xSendButton)
        m_xSendButton->connect_clicked(LINK(this, AIAssistantPanel, SendClickHdl));

    if (m_xInputField)
        m_xInputField->connect_activate(LINK(this, AIAssistantPanel, InputActivateHdl));

    if (m_xInsertButton)
        m_xInsertButton->connect_clicked(LINK(this, AIAssistantPanel, InsertClickHdl));

    // Configure chat history
    if (m_xChatHistory)
        m_xChatHistory->set_editable(false);

    // Initialize backend connection
    m_pAgentConnection = std::make_unique<officelabs::AgentConnection>();

    // Get Writer shell for document access
    SwView* pView = GetActiveView();
    if (pView)
    {
        m_pWrtShell = &pView->GetWrtShell();
    }

    // Set initial status based on connection
    if (m_pAgentConnection->isConnected())
    {
        UpdateStatus(u"Connected to AI Backend"_ustr);
    }
    else
    {
        UpdateStatus(u"Backend offline - Start server at localhost:8765"_ustr);
    }
}

AIAssistantPanel::~AIAssistantPanel()
{
}

void AIAssistantPanel::UpdateStatus(const OUString& status)
{
    if (m_xStatusLabel)
        m_xStatusLabel->set_label(status);
}

void AIAssistantPanel::AppendToChat(const OUString& sender, const OUString& message)
{
    if (!m_xChatHistory)
        return;

    OUString currentText = m_xChatHistory->get_text();
    OUString newText = currentText;
    if (!currentText.isEmpty())
        newText += u"\n"_ustr;

    newText += sender + u": "_ustr + message + u"\n"_ustr;
    m_xChatHistory->set_text(newText);

    // Scroll to bottom
    m_xChatHistory->select_region(-1, -1);
}

OUString AIAssistantPanel::GetDocumentContext()
{
    if (!m_pWrtShell)
        return u"No document loaded"_ustr;

    // Get selection if any
    OUString sSelection;
    if (m_pWrtShell->HasSelection())
    {
        m_pWrtShell->GetSelectedText(sSelection, ParaBreakType::ToBlank);
    }

    // Build context string
    OUStringBuffer context;
    if (!sSelection.isEmpty())
    {
        context.append(u"Selected text: "_ustr);
        // Limit to 500 chars
        if (sSelection.getLength() > 500)
            context.append(sSelection.subView(0, 500) + u"..."_ustr);
        else
            context.append(sSelection);
    }
    else
    {
        context.append(u"(No selection)"_ustr);
    }

    return context.makeStringAndClear();
}

void AIAssistantPanel::ProcessResponse(const officelabs::AgentResponse& response)
{
    m_bProcessing = false;

    // Re-enable send button
    if (m_xSendButton)
        m_xSendButton->set_sensitive(true);

    // Display response
    if (!response.message.isEmpty())
    {
        AppendToChat(u"AI"_ustr, response.message);
        m_sLastAIResponse = response.message;
        UpdateStatus(u"Ready"_ustr);
    }
    else
    {
        AppendToChat(u"System"_ustr, u"No response from AI"_ustr);
        UpdateStatus(u"Error occurred"_ustr);
    }

    // Handle patch if present
    if (response.hasPatch && !response.patchDiff.isEmpty())
    {
        AppendToChat(u"System"_ustr, u"AI suggested document changes. Click 'Insert' to apply."_ustr);
    }
}

void AIAssistantPanel::SendMessage(const OUString& message)
{
    if (message.isEmpty())
        return;

    if (m_bProcessing)
    {
        UpdateStatus(u"Please wait for current request..."_ustr);
        return;
    }

    // Add user message to chat
    AppendToChat(u"You"_ustr, message);

    // Clear input immediately
    if (m_xInputField)
        m_xInputField->set_text(u""_ustr);

    // Check connection
    if (!m_pAgentConnection->isConnected())
    {
        if (!m_pAgentConnection->checkConnection())
        {
            AppendToChat(u"System"_ustr, u"Error: Backend not available at localhost:8765"_ustr);
            UpdateStatus(u"Backend offline"_ustr);
            return;
        }
    }

    // Update UI for processing state
    m_bProcessing = true;
    UpdateStatus(u"Thinking..."_ustr);
    if (m_xSendButton)
        m_xSendButton->set_sensitive(false);

    // Get document context
    OUString sContext = GetDocumentContext();

    // Make HTTP call to backend
    officelabs::AgentResponse response = m_pAgentConnection->sendMessage(message, sContext);
    ProcessResponse(response);
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

IMPL_LINK_NOARG(AIAssistantPanel, InsertClickHdl, weld::Button&, void)
{
    if (m_sLastAIResponse.isEmpty())
    {
        UpdateStatus(u"No AI response to insert"_ustr);
        return;
    }

    if (!m_pWrtShell)
    {
        // Try to get the shell again
        SwView* pView = GetActiveView();
        if (pView)
            m_pWrtShell = &pView->GetWrtShell();
    }

    if (!m_pWrtShell)
    {
        UpdateStatus(u"No document available"_ustr);
        return;
    }

    // Insert at cursor position
    m_pWrtShell->StartAllAction();
    m_pWrtShell->Insert(m_sLastAIResponse);
    m_pWrtShell->EndAllAction();

    AppendToChat(u"System"_ustr, u"Text inserted into document"_ustr);
    UpdateStatus(u"Text inserted"_ustr);
}

} // end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
