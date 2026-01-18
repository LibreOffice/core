/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of OfficeLabs.
 *
 * AgenticPanel implementation - Full agentic AI assistant
 */

#include <officelabs/AgenticPanel.hxx>
#include <officelabs/AgentConnection.hxx>
#include <officelabs/DocumentController.hxx>

#include <vcl/svapp.hxx>
#include <comphelper/random.hxx>

#include <sstream>
#include <iomanip>
#include <ctime>

namespace officelabs
{

namespace
{
    // Generate a simple UUID-like ID
    OUString GenerateId()
    {
        std::stringstream ss;
        for (int i = 0; i < 8; ++i)
        {
            ss << std::hex << (comphelper::rng::uniform_uint_distribution(0, 15));
        }
        return OUString::fromUtf8(ss.str().c_str());
    }

    // Get current timestamp
    OUString GetTimestamp()
    {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M");
        return OUString::fromUtf8(oss.str().c_str());
    }
}

AgenticPanel::AgenticPanel(weld::Widget* pParent)
    : PanelLayout(pParent, u"AgenticPanel"_ustr, u"officelabs/ui/agenticpanel.ui"_ustr)
{
    // Initialize backend connections
    m_pAgentConnection = std::make_unique<AgentConnection>();
    m_pDocController = std::make_unique<DocumentController>();

    InitializeUI();

    // Create initial session
    CreateNewSession(u"Chat 1"_ustr);

    // Check connection
    SetConnected(m_pAgentConnection->checkConnection());
}

AgenticPanel::~AgenticPanel()
{
}

void AgenticPanel::InitializeUI()
{
    // Get UI components from .ui file
    // Note: These will be connected once the .ui file is created
    // For now, we'll create them programmatically as fallback

    // Tab bar
    m_xTabBar = m_xBuilder->weld_notebook(u"session_tabs"_ustr);
    m_xNewTabButton = m_xBuilder->weld_button(u"new_tab_button"_ustr);

    // Status bar
    m_xStatusBar = m_xBuilder->weld_box(u"status_bar"_ustr);
    m_xConnectionStatus = m_xBuilder->weld_label(u"connection_status"_ustr);
    m_xModelLabel = m_xBuilder->weld_label(u"model_label"_ustr);
    m_xSettingsButton = m_xBuilder->weld_button(u"settings_button"_ustr);

    // Chat area
    m_xChatScroll = m_xBuilder->weld_scrolled_window(u"chat_scroll"_ustr);
    m_xChatHistory = m_xBuilder->weld_text_view(u"chat_history"_ustr);

    // Input area
    m_xInputArea = m_xBuilder->weld_box(u"input_area"_ustr);
    m_xAttachButton = m_xBuilder->weld_button(u"attach_button"_ustr);
    m_xContextButton = m_xBuilder->weld_button(u"context_button"_ustr);
    m_xInputField = m_xBuilder->weld_entry(u"input_field"_ustr);
    m_xStopButton = m_xBuilder->weld_button(u"stop_button"_ustr);
    m_xResetButton = m_xBuilder->weld_button(u"reset_button"_ustr);
    m_xSendButton = m_xBuilder->weld_button(u"send_button"_ustr);

    // Edit approval bar
    m_xEditApprovalBar = m_xBuilder->weld_box(u"edit_approval_bar"_ustr);
    m_xPendingEditsLabel = m_xBuilder->weld_label(u"pending_edits_label"_ustr);
    m_xApproveAllButton = m_xBuilder->weld_button(u"approve_all_button"_ustr);
    m_xRejectAllButton = m_xBuilder->weld_button(u"reject_all_button"_ustr);
    m_xReviewButton = m_xBuilder->weld_button(u"review_button"_ustr);

    // Connect event handlers
    if (m_xSendButton)
        m_xSendButton->connect_clicked(LINK(this, AgenticPanel, SendClickHdl));
    if (m_xStopButton)
    {
        m_xStopButton->connect_clicked(LINK(this, AgenticPanel, StopClickHdl));
        m_xStopButton->set_sensitive(false);  // Disabled until agent runs
    }
    if (m_xResetButton)
        m_xResetButton->connect_clicked(LINK(this, AgenticPanel, ResetClickHdl));
    if (m_xAttachButton)
        m_xAttachButton->connect_clicked(LINK(this, AgenticPanel, AttachClickHdl));
    if (m_xContextButton)
        m_xContextButton->connect_clicked(LINK(this, AgenticPanel, ContextClickHdl));
    if (m_xNewTabButton)
        m_xNewTabButton->connect_clicked(LINK(this, AgenticPanel, NewTabClickHdl));
    if (m_xTabBar)
        m_xTabBar->connect_enter_page(LINK(this, AgenticPanel, TabChangedHdl));
    if (m_xSettingsButton)
        m_xSettingsButton->connect_clicked(LINK(this, AgenticPanel, SettingsClickHdl));
    if (m_xApproveAllButton)
        m_xApproveAllButton->connect_clicked(LINK(this, AgenticPanel, ApproveAllClickHdl));
    if (m_xRejectAllButton)
        m_xRejectAllButton->connect_clicked(LINK(this, AgenticPanel, RejectAllClickHdl));
    if (m_xReviewButton)
        m_xReviewButton->connect_clicked(LINK(this, AgenticPanel, ReviewClickHdl));
    if (m_xInputField)
        m_xInputField->connect_key_press(LINK(this, AgenticPanel, InputKeyPressHdl));

    // Initial UI state
    UpdateUI();
}

void AgenticPanel::UpdateUI()
{
    UpdateStatusBar();
    UpdateEditApprovalBar();
}

void AgenticPanel::UpdateStatusBar()
{
    if (m_xConnectionStatus)
    {
        if (m_bConnected)
        {
            m_xConnectionStatus->set_label(u"\u25CF Connected"_ustr);
        }
        else
        {
            m_xConnectionStatus->set_label(u"\u25CB Disconnected"_ustr);
        }
    }

    if (m_xModelLabel)
    {
        m_xModelLabel->set_label(u"Model: GPT-4o"_ustr);
    }

    // Update stop button state
    if (m_xStopButton)
    {
        m_xStopButton->set_sensitive(m_bAgentRunning);
    }
}

void AgenticPanel::UpdateEditApprovalBar()
{
    ChatSession* session = GetCurrentSession();
    if (!session)
        return;

    // Count pending edits
    size_t pendingCount = 0;
    for (const auto& edit : session->pendingEdits)
    {
        if (!edit.approved && !edit.rejected)
            pendingCount++;
    }

    if (m_xEditApprovalBar)
    {
        m_xEditApprovalBar->set_visible(pendingCount > 0);
    }

    if (m_xPendingEditsLabel && pendingCount > 0)
    {
        OUString label = OUString::number(pendingCount) + " pending edit" +
                         (pendingCount > 1 ? "s" : "");
        m_xPendingEditsLabel->set_label(label);
    }
}

// Session Management

void AgenticPanel::CreateNewSession(const OUString& name)
{
    ChatSession session;
    session.id = GenerateId();
    session.name = name.isEmpty() ? "Chat " + OUString::number(m_sessions.size() + 1) : name;
    session.isActive = true;

    m_sessions[session.id] = session;
    m_currentSessionId = session.id;

    // Update tab bar
    if (m_xTabBar)
    {
        // Add new tab
        m_xTabBar->append_page(session.id, session.name);
        m_xTabBar->set_current_page(m_xTabBar->get_n_pages() - 1);
    }

    RenderMessages();
}

void AgenticPanel::SwitchToSession(const OUString& sessionId)
{
    if (m_sessions.find(sessionId) == m_sessions.end())
        return;

    m_currentSessionId = sessionId;
    RenderMessages();
    UpdateEditApprovalBar();
}

void AgenticPanel::CloseSession(const OUString& sessionId)
{
    auto it = m_sessions.find(sessionId);
    if (it == m_sessions.end())
        return;

    m_sessions.erase(it);

    // If this was the current session, switch to another
    if (m_currentSessionId == sessionId)
    {
        if (!m_sessions.empty())
        {
            m_currentSessionId = m_sessions.begin()->first;
        }
        else
        {
            // Create a new session if none left
            CreateNewSession();
        }
    }

    // Update tab bar
    if (m_xTabBar)
    {
        int pageIdx = m_xTabBar->get_page_index(sessionId);
        if (pageIdx >= 0)
        {
            m_xTabBar->remove_page(sessionId);
        }
    }

    RenderMessages();
}

void AgenticPanel::RenameSession(const OUString& sessionId, const OUString& newName)
{
    auto it = m_sessions.find(sessionId);
    if (it == m_sessions.end())
        return;

    it->second.name = newName;

    // Update tab
    if (m_xTabBar)
    {
        m_xTabBar->set_tab_label_text(sessionId, newName);
    }
}

ChatSession* AgenticPanel::GetCurrentSession()
{
    auto it = m_sessions.find(m_currentSessionId);
    if (it != m_sessions.end())
        return &it->second;
    return nullptr;
}

// Message Handling

void AgenticPanel::SendMessage(const OUString& message)
{
    if (message.isEmpty())
        return;

    // Build message with optional selection context
    OUString fullMessage = message;
    OUString selection;

    if (m_bIncludeSelection)
    {
        selection = GetSelectedText();
        if (!selection.isEmpty())
        {
            fullMessage = u"[Selection: \""_ustr + selection + u"\"]\n\n"_ustr + message;
        }
        // Reset selection mode after use
        m_bIncludeSelection = false;
        if (m_xContextButton)
        {
            m_xContextButton->set_label(u"@"_ustr);
            m_xContextButton->set_tooltip_text(u"Include selected text (@selection)"_ustr);
        }
        if (m_xInputField)
            m_xInputField->set_placeholder_text(u"Type your message..."_ustr);
    }

    // Add user message (show original message, context is implicit)
    if (!selection.isEmpty())
    {
        AppendUserMessage(message + u" (with selection)"_ustr);
    }
    else
    {
        AppendUserMessage(message);
    }

    // Clear input
    if (m_xInputField)
        m_xInputField->set_text(u""_ustr);

    // Start agent
    m_bAgentRunning = true;
    UpdateStatusBar();

    // Refresh and get document context
    RefreshDocumentContext();
    OUString context = GetDocumentContext();

    // Send to backend (async)
    // The response will come back through OnStreamEvent callbacks
    AgentResponse response = m_pAgentConnection->sendMessage(fullMessage, context);

    // For now, handle synchronously (will be async with SSE later)
    if (!response.message.isEmpty())
    {
        AppendAssistantMessage(response.message);

        // Handle patch if present
        if (response.hasPatch && response.patchType == "INSERT")
        {
            PendingEdit edit;
            edit.id = GenerateId();
            edit.editType = response.patchType;
            edit.newText = response.patchNewValue;
            edit.description = u"Insert content at cursor"_ustr;
            AppendPendingEdit(edit);
        }
    }

    m_bAgentRunning = false;
    UpdateStatusBar();
}

void AgenticPanel::AppendUserMessage(const OUString& message)
{
    ChatSession* session = GetCurrentSession();
    if (!session)
        return;

    ChatMessage msg;
    msg.role = ChatMessage::Role::USER;
    msg.content = message;
    msg.timestamp = GetTimestamp();

    session->messages.push_back(msg);
    RenderMessages();
}

void AgenticPanel::AppendAssistantMessage(const OUString& message)
{
    ChatSession* session = GetCurrentSession();
    if (!session)
        return;

    ChatMessage msg;
    msg.role = ChatMessage::Role::ASSISTANT;
    msg.content = message;
    msg.timestamp = GetTimestamp();

    session->messages.push_back(msg);
    RenderMessages();
}

void AgenticPanel::AppendToolCall(const ToolCallInfo& toolCall)
{
    ChatSession* session = GetCurrentSession();
    if (!session || session->messages.empty())
        return;

    // Add to the last assistant message
    ChatMessage& lastMsg = session->messages.back();
    if (lastMsg.role == ChatMessage::Role::ASSISTANT)
    {
        lastMsg.toolCalls.push_back(toolCall);
        RenderMessages();
    }
}

void AgenticPanel::AppendPendingEdit(const PendingEdit& edit)
{
    ChatSession* session = GetCurrentSession();
    if (!session)
        return;

    session->pendingEdits.push_back(edit);

    // Also add to last assistant message
    if (!session->messages.empty())
    {
        ChatMessage& lastMsg = session->messages.back();
        if (lastMsg.role == ChatMessage::Role::ASSISTANT)
        {
            lastMsg.pendingEdits.push_back(edit);
        }
    }

    UpdateEditApprovalBar();
    RenderMessages();
}

void AgenticPanel::RenderMessages()
{
    if (!m_xChatHistory)
        return;

    ChatSession* session = GetCurrentSession();
    if (!session)
    {
        m_xChatHistory->set_text(u""_ustr);
        return;
    }

    // Build chat history text
    OUStringBuffer buffer;

    for (const auto& msg : session->messages)
    {
        // Role prefix
        if (msg.role == ChatMessage::Role::USER)
        {
            buffer.append(u"\U0001F464 You: "_ustr);  // 👤
        }
        else
        {
            buffer.append(u"\U0001F916 Assistant: "_ustr);  // 🤖
        }

        // Content
        buffer.append(msg.content);
        buffer.append(u"\n"_ustr);

        // Tool calls
        for (const auto& tc : msg.toolCalls)
        {
            buffer.append(u"  \u2514 "_ustr);  // └
            if (tc.status == "success")
                buffer.append(u"\u2713 "_ustr);  // ✓
            else if (tc.status == "running")
                buffer.append(u"\u23F3 "_ustr);  // ⏳
            else
                buffer.append(u"\u2717 "_ustr);  // ✗

            buffer.append(tc.name);
            if (!tc.result.isEmpty())
            {
                buffer.append(u" - "_ustr);
                buffer.append(tc.result.getLength() > 50 ?
                    tc.result.copy(0, 50) + "..." : tc.result);
            }
            buffer.append(u"\n"_ustr);
        }

        // Pending edits
        for (const auto& edit : msg.pendingEdits)
        {
            buffer.append(u"  \u250C\u2500 Pending Edit: "_ustr);  // ┌─
            buffer.append(edit.editType);
            buffer.append(u"\n"_ustr);

            if (!edit.newText.isEmpty())
            {
                // Show preview (first few lines)
                OUString preview = edit.newText.getLength() > 100 ?
                    edit.newText.copy(0, 100) + "..." : edit.newText;
                buffer.append(u"  \u2502 + "_ustr);  // │
                buffer.append(preview);
                buffer.append(u"\n"_ustr);
            }

            buffer.append(u"  \u2514 [Accept] [Reject]\n"_ustr);  // └
        }

        buffer.append(u"\n"_ustr);
    }

    m_xChatHistory->set_text(buffer.makeStringAndClear());
    ScrollToBottom();
}

void AgenticPanel::ScrollToBottom()
{
    // Scroll chat to bottom
    if (m_xChatScroll)
    {
        // Get the adjustment and scroll to end
        // Implementation depends on GTK/weld specifics
    }
}

OUString AgenticPanel::GetDocumentContext()
{
    if (!m_pDocController)
        return OUString();

    return m_pDocController->getDocumentText();
}

// Edit Approval

void AgenticPanel::ApproveEdit(const OUString& editId)
{
    ChatSession* session = GetCurrentSession();
    if (!session)
        return;

    for (auto& edit : session->pendingEdits)
    {
        if (edit.id == editId)
        {
            edit.approved = true;

            // Apply the edit to the document
            if (edit.editType == "INSERT" && m_pDocController)
            {
                // Get current document length for insert position
                OUString docText = m_pDocController->getDocumentText();
                m_pDocController->insertText(edit.newText, docText.getLength());
            }
            break;
        }
    }

    UpdateEditApprovalBar();
    RenderMessages();
}

void AgenticPanel::RejectEdit(const OUString& editId)
{
    ChatSession* session = GetCurrentSession();
    if (!session)
        return;

    for (auto& edit : session->pendingEdits)
    {
        if (edit.id == editId)
        {
            edit.rejected = true;
            break;
        }
    }

    UpdateEditApprovalBar();
    RenderMessages();
}

void AgenticPanel::ApproveAllEdits()
{
    ChatSession* session = GetCurrentSession();
    if (!session)
        return;

    for (auto& edit : session->pendingEdits)
    {
        if (!edit.approved && !edit.rejected)
        {
            ApproveEdit(edit.id);
        }
    }
}

void AgenticPanel::RejectAllEdits()
{
    ChatSession* session = GetCurrentSession();
    if (!session)
        return;

    for (auto& edit : session->pendingEdits)
    {
        if (!edit.approved && !edit.rejected)
        {
            edit.rejected = true;
        }
    }

    UpdateEditApprovalBar();
    RenderMessages();
}

// Agent Control

void AgenticPanel::StopAgent()
{
    if (!m_bAgentRunning)
        return;

    // Send cancel request to backend
    if (!m_currentExecutionId.isEmpty())
    {
        // TODO: Call cancel endpoint
        // m_pAgentConnection->cancelExecution(m_currentExecutionId);
    }

    m_bAgentRunning = false;
    UpdateStatusBar();

    AppendAssistantMessage(u"[Stopped by user]"_ustr);
}

void AgenticPanel::ResetSession()
{
    ChatSession* session = GetCurrentSession();
    if (!session)
        return;

    // Stop any running agent
    if (m_bAgentRunning)
        StopAgent();

    // Clear messages and pending edits
    session->messages.clear();
    session->pendingEdits.clear();

    // Refresh document context
    RefreshDocumentContext();

    // Update UI
    RenderMessages();
    UpdateEditApprovalBar();

    // Add welcome message
    AppendAssistantMessage(u"Session reset. How can I help you with your document?"_ustr);
}

// Document context methods

void AgenticPanel::RefreshDocumentContext()
{
    if (!m_pDocController)
        return;

    m_currentContext.title = m_pDocController->getDocumentTitle();
    m_currentContext.app = m_pDocController->getApplicationType();
    m_currentContext.content = m_pDocController->getDocumentText();
    m_currentContext.selection = m_pDocController->getSelectedText();
    m_currentContext.cursorPosition = m_pDocController->getCursorPosition();
    m_currentContext.url = m_pDocController->getDocumentURL();
}

DocumentContextInfo AgenticPanel::GetCurrentDocumentContext()
{
    RefreshDocumentContext();
    return m_currentContext;
}

OUString AgenticPanel::GetSelectedText()
{
    if (!m_pDocController)
        return OUString();

    return m_pDocController->getSelectedText();
}

void AgenticPanel::InsertAtCursor(const OUString& text)
{
    if (!m_pDocController || text.isEmpty())
        return;

    m_pDocController->insertText(text, m_pDocController->getCursorPosition());
}

void AgenticPanel::ReplaceSelection(const OUString& text)
{
    if (!m_pDocController)
        return;

    m_pDocController->replaceSelection(text);
}

void AgenticPanel::ApplyFormatting(const OUString& formatType, const OUString& value)
{
    if (!m_pDocController)
        return;

    // Apply formatting to current selection
    if (formatType == "bold")
        m_pDocController->applyBold();
    else if (formatType == "italic")
        m_pDocController->applyItalic();
    else if (formatType == "underline")
        m_pDocController->applyUnderline();
    else if (formatType == "heading")
        m_pDocController->applyHeading(value.toInt32());
    else if (formatType == "fontsize")
        m_pDocController->setFontSize(value.toFloat());
    else if (formatType == "fontname")
        m_pDocController->setFontName(value);
    else if (formatType == "color")
        m_pDocController->setTextColor(value);
}

void AgenticPanel::SetConnected(bool connected)
{
    m_bConnected = connected;
    UpdateStatusBar();
}

// Event Handlers

IMPL_LINK_NOARG(AgenticPanel, SendClickHdl, weld::Button&, void)
{
    if (m_xInputField)
    {
        OUString message = m_xInputField->get_text();
        SendMessage(message);
    }
}

IMPL_LINK_NOARG(AgenticPanel, StopClickHdl, weld::Button&, void)
{
    StopAgent();
}

IMPL_LINK_NOARG(AgenticPanel, ResetClickHdl, weld::Button&, void)
{
    ResetSession();
}

IMPL_LINK_NOARG(AgenticPanel, AttachClickHdl, weld::Button&, void)
{
    // TODO: Open file picker dialog for attachments
    // For now, we could open a file dialog to attach documents
}

IMPL_LINK_NOARG(AgenticPanel, ContextClickHdl, weld::Button&, void)
{
    // Toggle selection reference mode
    m_bIncludeSelection = !m_bIncludeSelection;

    if (m_xContextButton)
    {
        if (m_bIncludeSelection)
        {
            // Show that selection will be included
            m_xContextButton->set_label(u"@✓"_ustr);
            m_xContextButton->set_tooltip_text(u"Selection will be included (click to disable)"_ustr);

            // Get and show preview of selection
            OUString selection = GetSelectedText();
            if (!selection.isEmpty())
            {
                // Show a preview in the input field placeholder
                OUString preview = selection.getLength() > 30 ?
                    selection.copy(0, 30) + u"..."_ustr : selection;
                if (m_xInputField)
                    m_xInputField->set_placeholder_text(u"Message with selection: \""_ustr + preview + u"\""_ustr);
            }
            else
            {
                if (m_xInputField)
                    m_xInputField->set_placeholder_text(u"No text selected - select text first"_ustr);
            }
        }
        else
        {
            // Reset to default state
            m_xContextButton->set_label(u"@"_ustr);
            m_xContextButton->set_tooltip_text(u"Include selected text (@selection)"_ustr);
            if (m_xInputField)
                m_xInputField->set_placeholder_text(u"Type your message..."_ustr);
        }
    }
}

IMPL_LINK_NOARG(AgenticPanel, NewTabClickHdl, weld::Button&, void)
{
    CreateNewSession();
}

IMPL_LINK(AgenticPanel, TabChangedHdl, weld::Notebook&, rNotebook, void)
{
    OUString pageId = rNotebook.get_current_page_ident();
    SwitchToSession(pageId);
}

IMPL_LINK_NOARG(AgenticPanel, SettingsClickHdl, weld::Button&, void)
{
    // TODO: Open settings dialog
}

IMPL_LINK_NOARG(AgenticPanel, ApproveAllClickHdl, weld::Button&, void)
{
    ApproveAllEdits();
}

IMPL_LINK_NOARG(AgenticPanel, RejectAllClickHdl, weld::Button&, void)
{
    RejectAllEdits();
}

IMPL_LINK_NOARG(AgenticPanel, ReviewClickHdl, weld::Button&, void)
{
    // TODO: Open review dialog with diff view
}

IMPL_LINK(AgenticPanel, InputKeyPressHdl, const KeyEvent&, rKEvt, bool)
{
    if (rKEvt.GetKeyCode().GetCode() == KEY_RETURN &&
        !rKEvt.GetKeyCode().IsShift())
    {
        // Enter without Shift - send message
        if (m_xInputField)
        {
            OUString message = m_xInputField->get_text();
            if (!message.isEmpty())
            {
                SendMessage(message);
                return true;  // Consumed
            }
        }
    }
    return false;  // Not consumed
}

// Streaming response handling (for future SSE integration)

void AgenticPanel::OnStreamEvent(const OUString& eventType, const OUString& eventData)
{
    if (eventType == "text")
    {
        // Append to current assistant message
        // (For streaming, we'd update incrementally)
    }
    else if (eventType == "tool_start")
    {
        ToolCallInfo tc;
        tc.status = u"running"_ustr;
        // Parse eventData for tool info
        AppendToolCall(tc);
    }
    else if (eventType == "tool_complete")
    {
        // Update tool call status
    }
    else if (eventType == "pending_edit")
    {
        PendingEdit edit;
        // Parse eventData
        AppendPendingEdit(edit);
    }
    else if (eventType == "execution_id")
    {
        m_currentExecutionId = eventData;
    }
}

void AgenticPanel::OnStreamComplete()
{
    m_bAgentRunning = false;
    m_currentExecutionId = OUString();
    UpdateStatusBar();
}

void AgenticPanel::OnStreamError(const OUString& error)
{
    m_bAgentRunning = false;
    m_currentExecutionId = OUString();
    UpdateStatusBar();

    AppendAssistantMessage(u"Error: "_ustr + error);
}

} // namespace officelabs

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
