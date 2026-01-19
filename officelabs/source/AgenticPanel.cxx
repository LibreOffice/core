/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of OfficeLabs.
 *
 * AgenticPanel implementation - Full agentic AI assistant using VCL toolkit
 *
 * This provides a Cursor-like AI experience for LibreOffice with:
 * - Multi-session chat tabs
 * - Document context and selection reference (@selection)
 * - Tool call visualization (read_document, edit_section, etc.)
 * - Pending edit approval workflow
 * - Real-time agent status
 */

#include <officelabs/AgenticPanel.hxx>
#include <officelabs/AgentConnection.hxx>
#include <officelabs/DocumentController.hxx>
#include <sfx2/bindings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <comphelper/random.hxx>

#include <sstream>
#include <iomanip>
#include <ctime>

namespace officelabs
{

namespace
{
    // Generate a simple ID for sessions and edits
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

AgenticPanel::AgenticPanel(SfxBindings* pBindings, SfxChildWindow* pChildWin, vcl::Window* pParent)
    : SfxDockingWindow(pBindings, pChildWin, pParent, WB_STDDOCKWIN | WB_CLOSEABLE | WB_SIZEABLE)
    , m_bConnected(false)
    , m_bAgentRunning(false)
    , m_bIncludeSelection(false)
    , m_nNextSessionNum(1)
{
    SetText("OfficeLabs AI Agent");
    SetSizePixel(Size(450, 700));

    // Create backend connections
    m_pAgent = std::make_unique<AgentConnection>();
    m_pDocController = std::make_unique<DocumentController>();

    // ========================================
    // Create UI Components - Header area
    // ========================================
    m_pStatusLabel = VclPtr<FixedText>::Create(this, WB_LEFT);
    m_pModelLabel = VclPtr<FixedText>::Create(this, WB_LEFT);
    m_pSettingsButton = VclPtr<PushButton>::Create(this);

    // ========================================
    // Create UI Components - Tabs
    // ========================================
    m_pTabControl = VclPtr<TabControl>::Create(this);
    m_pNewTabButton = VclPtr<PushButton>::Create(this);

    // ========================================
    // Create UI Components - Chat area
    // ========================================
    m_pChatHistory = VclPtr<Control>::Create(this, WB_BORDER);

    // ========================================
    // Create UI Components - Edit approval bar
    // ========================================
    m_pEditApprovalBar = VclPtr<Control>::Create(this, WB_BORDER);
    m_pPendingEditsLabel = VclPtr<FixedText>::Create(this, WB_LEFT);
    m_pApproveAllButton = VclPtr<PushButton>::Create(this);
    m_pRejectAllButton = VclPtr<PushButton>::Create(this);
    m_pReviewButton = VclPtr<PushButton>::Create(this);

    // ========================================
    // Create UI Components - Input area
    // ========================================
    m_pAttachButton = VclPtr<PushButton>::Create(this);
    m_pContextButton = VclPtr<PushButton>::Create(this);
    m_pInputField = VclPtr<Edit>::Create(this, WB_BORDER);
    m_pStopButton = VclPtr<PushButton>::Create(this);
    m_pResetButton = VclPtr<PushButton>::Create(this);
    m_pSendButton = VclPtr<PushButton>::Create(this);

    // ========================================
    // Set button labels
    // ========================================
    m_pSettingsButton->SetText("Settings");
    m_pNewTabButton->SetText("+");
    m_pApproveAllButton->SetText("Accept All");
    m_pRejectAllButton->SetText("Reject All");
    m_pReviewButton->SetText("Review");
    m_pAttachButton->SetText("Attach");
    m_pContextButton->SetText("@");
    m_pStopButton->SetText("Stop");
    m_pResetButton->SetText("Reset");
    m_pSendButton->SetText("Send");

    // ========================================
    // Set tooltips
    // ========================================
    m_pContextButton->SetQuickHelpText("Include selected text in message (@selection)");
    m_pStopButton->SetQuickHelpText("Stop the running agent");
    m_pResetButton->SetQuickHelpText("Clear and reset the current session");
    m_pNewTabButton->SetQuickHelpText("Create a new chat session");
    m_pApproveAllButton->SetQuickHelpText("Accept all pending edits");
    m_pRejectAllButton->SetQuickHelpText("Reject all pending edits");
    m_pReviewButton->SetQuickHelpText("Review pending edits with diff view");

    // ========================================
    // Connect event handlers
    // ========================================
    m_pSendButton->SetClickHdl(LINK(this, AgenticPanel, SendClickHdl));
    m_pStopButton->SetClickHdl(LINK(this, AgenticPanel, StopClickHdl));
    m_pResetButton->SetClickHdl(LINK(this, AgenticPanel, ResetClickHdl));
    m_pContextButton->SetClickHdl(LINK(this, AgenticPanel, ContextClickHdl));
    m_pAttachButton->SetClickHdl(LINK(this, AgenticPanel, AttachClickHdl));
    m_pNewTabButton->SetClickHdl(LINK(this, AgenticPanel, NewTabClickHdl));
    m_pSettingsButton->SetClickHdl(LINK(this, AgenticPanel, SettingsClickHdl));
    m_pTabControl->SetActivatePageHdl(LINK(this, AgenticPanel, TabChangedHdl));
    m_pApproveAllButton->SetClickHdl(LINK(this, AgenticPanel, ApproveAllClickHdl));
    m_pRejectAllButton->SetClickHdl(LINK(this, AgenticPanel, RejectAllClickHdl));
    m_pReviewButton->SetClickHdl(LINK(this, AgenticPanel, ReviewClickHdl));

    // ========================================
    // Initial button states
    // ========================================
    m_pStopButton->Enable(false);  // Disabled until agent runs

    // ========================================
    // Show all components
    // ========================================
    m_pStatusLabel->Show();
    m_pModelLabel->Show();
    m_pSettingsButton->Show();
    m_pTabControl->Show();
    m_pNewTabButton->Show();
    m_pChatHistory->Show();
    m_pAttachButton->Show();
    m_pContextButton->Show();
    m_pInputField->Show();
    m_pStopButton->Show();
    m_pResetButton->Show();
    m_pSendButton->Show();

    // Edit approval bar hidden initially
    m_pEditApprovalBar->Hide();
    m_pPendingEditsLabel->Hide();
    m_pApproveAllButton->Hide();
    m_pRejectAllButton->Hide();
    m_pReviewButton->Hide();

    // ========================================
    // Initialize
    // ========================================
    CheckConnection();
    CreateNewSession(u"Chat 1"_ustr);
    DoLayout();
}

AgenticPanel::~AgenticPanel()
{
    disposeOnce();
}

void AgenticPanel::dispose()
{
    m_pStatusLabel.disposeAndClear();
    m_pModelLabel.disposeAndClear();
    m_pSettingsButton.disposeAndClear();
    m_pTabControl.disposeAndClear();
    m_pNewTabButton.disposeAndClear();
    m_pChatHistory.disposeAndClear();
    m_pEditApprovalBar.disposeAndClear();
    m_pPendingEditsLabel.disposeAndClear();
    m_pApproveAllButton.disposeAndClear();
    m_pRejectAllButton.disposeAndClear();
    m_pReviewButton.disposeAndClear();
    m_pAttachButton.disposeAndClear();
    m_pContextButton.disposeAndClear();
    m_pInputField.disposeAndClear();
    m_pStopButton.disposeAndClear();
    m_pResetButton.disposeAndClear();
    m_pSendButton.disposeAndClear();
    SfxDockingWindow::dispose();
}

void AgenticPanel::DoLayout()
{
    Size aSize = GetSizePixel();
    const sal_Int32 nMargin = 8;
    const sal_Int32 nBtnHeight = 28;
    Point aPos(nMargin, nMargin);

    // ========================================
    // Status bar row: [Status] [Model] [Settings]
    // ========================================
    sal_Int32 statusWidth = (aSize.Width() - 3 * nMargin - 70) / 2;
    m_pStatusLabel->SetPosSizePixel(aPos, Size(statusWidth, 22));

    aPos.AdjustX(statusWidth + nMargin);
    m_pModelLabel->SetPosSizePixel(aPos, Size(statusWidth, 22));

    m_pSettingsButton->SetPosSizePixel(
        Point(aSize.Width() - nMargin - 60, nMargin),
        Size(60, 22)
    );
    aPos.setX(nMargin);
    aPos.AdjustY(28);

    // ========================================
    // Tab row: [Tabs...] [+]
    // ========================================
    sal_Int32 tabWidth = aSize.Width() - 3 * nMargin - 30;
    m_pTabControl->SetPosSizePixel(aPos, Size(tabWidth, 32));
    m_pNewTabButton->SetPosSizePixel(
        Point(aSize.Width() - nMargin - 28, aPos.Y()),
        Size(28, 28)
    );
    aPos.AdjustY(38);

    // ========================================
    // Calculate remaining height
    // ========================================
    sal_Int32 inputAreaHeight = nBtnHeight + nMargin + nBtnHeight + nMargin;

    // Check if edit approval bar should be shown
    AgenticSession* session = GetCurrentSession();
    bool bShowApprovalBar = false;
    if (session)
    {
        for (const auto& edit : session->pendingEdits)
        {
            if (!edit.approved && !edit.rejected)
            {
                bShowApprovalBar = true;
                break;
            }
        }
    }

    sal_Int32 approvalBarHeight = bShowApprovalBar ? 36 : 0;

    // Chat history (takes remaining space)
    sal_Int32 historyHeight = aSize.Height() - aPos.Y() - inputAreaHeight - approvalBarHeight - nMargin;
    m_pChatHistory->SetPosSizePixel(aPos, Size(aSize.Width() - 2 * nMargin, historyHeight));
    aPos.AdjustY(historyHeight + nMargin);

    // ========================================
    // Edit approval bar (if visible): [X pending edits] [Accept All] [Reject All] [Review]
    // ========================================
    if (bShowApprovalBar)
    {
        m_pEditApprovalBar->SetPosSizePixel(aPos, Size(aSize.Width() - 2 * nMargin, 32));
        m_pPendingEditsLabel->SetPosSizePixel(Point(aPos.X(), aPos.Y() + 6), Size(120, 20));
        m_pApproveAllButton->SetPosSizePixel(Point(aPos.X() + 125, aPos.Y() + 2), Size(80, nBtnHeight));
        m_pRejectAllButton->SetPosSizePixel(Point(aPos.X() + 210, aPos.Y() + 2), Size(75, nBtnHeight));
        m_pReviewButton->SetPosSizePixel(Point(aPos.X() + 290, aPos.Y() + 2), Size(60, nBtnHeight));

        m_pEditApprovalBar->Show();
        m_pPendingEditsLabel->Show();
        m_pApproveAllButton->Show();
        m_pRejectAllButton->Show();
        m_pReviewButton->Show();

        aPos.AdjustY(36);
    }
    else
    {
        m_pEditApprovalBar->Hide();
        m_pPendingEditsLabel->Hide();
        m_pApproveAllButton->Hide();
        m_pRejectAllButton->Hide();
        m_pReviewButton->Hide();
    }

    // ========================================
    // Input row: [Attach] [@SEL] [Input field............] [Send]
    // ========================================
    m_pAttachButton->SetPosSizePixel(aPos, Size(50, nBtnHeight));
    aPos.AdjustX(54);

    // Context button wider to fit "@SEL" when active
    m_pContextButton->SetPosSizePixel(aPos, Size(40, nBtnHeight));
    aPos.AdjustX(44);

    sal_Int32 inputFieldWidth = aSize.Width() - aPos.X() - nMargin - 50;
    m_pInputField->SetPosSizePixel(aPos, Size(inputFieldWidth, nBtnHeight));

    m_pSendButton->SetPosSizePixel(
        Point(aSize.Width() - nMargin - 45, aPos.Y()),
        Size(45, nBtnHeight)
    );
    aPos.setX(nMargin);
    aPos.AdjustY(nBtnHeight + nMargin);

    // ========================================
    // Bottom row: [Stop] [Reset]
    // ========================================
    m_pStopButton->SetPosSizePixel(aPos, Size(60, nBtnHeight));
    aPos.AdjustX(64);
    m_pResetButton->SetPosSizePixel(aPos, Size(60, nBtnHeight));
}

void AgenticPanel::Resize()
{
    DockingWindow::Resize();
    DoLayout();
}

void AgenticPanel::CheckConnection()
{
    if (m_pAgent)
    {
        m_bConnected = m_pAgent->checkConnection();
    }
    UpdateStatusBar();
}

void AgenticPanel::UpdateStatusBar()
{
    if (m_pStatusLabel)
    {
        if (m_bConnected)
        {
            m_pStatusLabel->SetText(OUString::fromUtf8("\xE2\x97\x8F Connected"));  // ● Connected
        }
        else
        {
            m_pStatusLabel->SetText(OUString::fromUtf8("\xE2\x97\x8B Disconnected"));  // ○ Disconnected
        }
    }

    if (m_pModelLabel)
    {
        m_pModelLabel->SetText("Model: GPT-4o");
    }

    if (m_pStopButton)
    {
        m_pStopButton->Enable(m_bAgentRunning);
    }
}

void AgenticPanel::UpdateEditApprovalBar()
{
    AgenticSession* session = GetCurrentSession();
    if (!session)
        return;

    // Count pending edits
    size_t pendingCount = 0;
    for (const auto& edit : session->pendingEdits)
    {
        if (!edit.approved && !edit.rejected)
            pendingCount++;
    }

    if (m_pPendingEditsLabel && pendingCount > 0)
    {
        OUString label = OUString::number(pendingCount) + " pending edit";
        if (pendingCount > 1)
            label += "s";
        m_pPendingEditsLabel->SetText(label);
    }

    // Relayout to show/hide the bar
    DoLayout();
}

// ========================================
// Session Management
// ========================================

OUString AgenticPanel::CreateNewSession(const OUString& name)
{
    AgenticSession session;
    session.id = GenerateId();
    session.name = name.isEmpty()
        ? "Chat " + OUString::number(m_nNextSessionNum++)
        : name;

    m_sessions[session.id] = session;
    m_currentSessionId = session.id;

    // Add tab
    if (m_pTabControl)
    {
        sal_uInt16 nPageId = m_pTabControl->GetPageCount() + 1;
        m_pTabControl->InsertPage(nPageId, session.name);
        m_pTabControl->SetCurPageId(nPageId);
    }

    RenderMessages();
    return session.id;
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

    RenderMessages();
}

AgenticSession* AgenticPanel::GetCurrentSession()
{
    auto it = m_sessions.find(m_currentSessionId);
    if (it != m_sessions.end())
        return &it->second;
    return nullptr;
}

// ========================================
// Message Handling
// ========================================

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
        if (m_pContextButton)
        {
            m_pContextButton->SetText("@");
            m_pContextButton->SetQuickHelpText("Include selected text in message (@selection)");
        }
    }

    // Add user message to UI
    if (!selection.isEmpty())
    {
        AppendMessage(message + u" (with selection)"_ustr, AgenticMessage::Role::USER);
    }
    else
    {
        AppendMessage(message, AgenticMessage::Role::USER);
    }

    // Clear input
    if (m_pInputField)
        m_pInputField->SetText("");

    // Mark agent as running
    m_bAgentRunning = true;
    UpdateStatusBar();

    // Get document context
    RefreshDocumentContext();
    OUString context = m_pDocController ? m_pDocController->getDocumentText() : OUString();

    // Send to backend
    AgentResponse response = m_pAgent->sendMessage(fullMessage, context);

    // Handle response
    if (!response.message.isEmpty())
    {
        AppendMessage(response.message, AgenticMessage::Role::ASSISTANT);

        // Handle patch if present
        if (response.hasPatch)
        {
            PendingEdit edit;
            edit.id = GenerateId();
            edit.editType = response.patchType;
            edit.newText = response.patchNewValue;
            edit.originalText = response.patchOldValue;
            edit.description = u"AI suggested edit"_ustr;
            AppendPendingEdit(edit);
        }
    }

    m_bAgentRunning = false;
    UpdateStatusBar();
}

void AgenticPanel::AppendMessage(const OUString& content, AgenticMessage::Role role)
{
    AgenticSession* session = GetCurrentSession();
    if (!session)
        return;

    AgenticMessage msg;
    msg.role = role;
    msg.content = content;
    msg.timestamp = GetTimestamp();

    session->messages.push_back(msg);
    RenderMessages();
}

void AgenticPanel::AppendToolCall(const ToolCallInfo& toolCall)
{
    AgenticSession* session = GetCurrentSession();
    if (!session || session->messages.empty())
        return;

    // Add to the last assistant message
    AgenticMessage& lastMsg = session->messages.back();
    if (lastMsg.role == AgenticMessage::Role::ASSISTANT)
    {
        lastMsg.toolCalls.push_back(toolCall);
        RenderMessages();
    }
}

void AgenticPanel::AppendPendingEdit(const PendingEdit& edit)
{
    AgenticSession* session = GetCurrentSession();
    if (!session)
        return;

    session->pendingEdits.push_back(edit);

    // Also add to last assistant message
    if (!session->messages.empty())
    {
        AgenticMessage& lastMsg = session->messages.back();
        if (lastMsg.role == AgenticMessage::Role::ASSISTANT)
        {
            lastMsg.pendingEdits.push_back(edit);
        }
    }

    UpdateEditApprovalBar();
    RenderMessages();
}

void AgenticPanel::RenderMessages()
{
    // The actual rendering happens in Paint()
    // Just trigger a repaint
    Invalidate();
}

// ========================================
// Document Integration
// ========================================

void AgenticPanel::SetDocument(const css::uno::Reference<css::text::XTextDocument>& xDoc)
{
    m_xDocument = xDoc;
    if (m_pDocController)
    {
        m_pDocController->setDocument(xDoc);
    }
}

OUString AgenticPanel::GetSelectedText()
{
    if (!m_pDocController)
        return OUString();

    return m_pDocController->getSelectedText();
}

void AgenticPanel::RefreshDocumentContext()
{
    // Refresh document controller state
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

// ========================================
// Edit Approval
// ========================================

void AgenticPanel::ApproveEdit(const OUString& editId)
{
    AgenticSession* session = GetCurrentSession();
    if (!session)
        return;

    for (auto& edit : session->pendingEdits)
    {
        if (edit.id == editId)
        {
            edit.approved = true;
            ApplyEdit(edit);
            break;
        }
    }

    UpdateEditApprovalBar();
    RenderMessages();
}

void AgenticPanel::RejectEdit(const OUString& editId)
{
    AgenticSession* session = GetCurrentSession();
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
    AgenticSession* session = GetCurrentSession();
    if (!session)
        return;

    for (auto& edit : session->pendingEdits)
    {
        if (!edit.approved && !edit.rejected)
        {
            edit.approved = true;
            ApplyEdit(edit);
        }
    }

    UpdateEditApprovalBar();
    RenderMessages();
}

void AgenticPanel::RejectAllEdits()
{
    AgenticSession* session = GetCurrentSession();
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

void AgenticPanel::ApplyEdit(const PendingEdit& edit)
{
    if (!m_pDocController || !m_pDocController->hasDocument())
        return;

    if (edit.editType == "INSERT")
    {
        OUString docText = m_pDocController->getDocumentText();
        m_pDocController->insertText(edit.newText, docText.getLength());
    }
    else if (edit.editType == "REPLACE")
    {
        m_pDocController->replaceSelection(edit.newText);
    }
}

// ========================================
// Agent Control
// ========================================

void AgenticPanel::StopAgent()
{
    if (!m_bAgentRunning)
        return;

    // TODO: Send cancel request to backend via m_currentExecutionId

    m_bAgentRunning = false;
    UpdateStatusBar();

    AppendMessage(u"[Stopped by user]"_ustr, AgenticMessage::Role::ASSISTANT);
}

void AgenticPanel::ResetSession()
{
    AgenticSession* session = GetCurrentSession();
    if (!session)
        return;

    // Stop any running agent
    if (m_bAgentRunning)
        StopAgent();

    // Clear messages and pending edits
    session->messages.clear();
    session->pendingEdits.clear();

    // Refresh UI
    RenderMessages();
    UpdateEditApprovalBar();

    // Add welcome message
    AppendMessage(u"Session reset. How can I help you with your document?"_ustr,
                  AgenticMessage::Role::ASSISTANT);
}

// ========================================
// Event Handlers
// ========================================

IMPL_LINK_NOARG(AgenticPanel, SendClickHdl, Button*, void)
{
    if (m_pInputField)
    {
        OUString message = m_pInputField->GetText();
        SendMessage(message);
    }
}

IMPL_LINK_NOARG(AgenticPanel, StopClickHdl, Button*, void)
{
    StopAgent();
}

IMPL_LINK_NOARG(AgenticPanel, ResetClickHdl, Button*, void)
{
    ResetSession();
}

IMPL_LINK_NOARG(AgenticPanel, ContextClickHdl, Button*, void)
{
    // Toggle selection reference mode
    m_bIncludeSelection = !m_bIncludeSelection;

    if (m_pContextButton)
    {
        if (m_bIncludeSelection)
        {
            // Make button stand out when selection mode is active
            m_pContextButton->SetText(u"@SEL"_ustr);
            m_pContextButton->SetQuickHelpText("Selection will be included (click to disable)");

            // Get selection preview for input field placeholder
            OUString selection = GetSelectedText();
            if (!selection.isEmpty() && m_pInputField)
            {
                OUString preview = selection.getLength() > 25 ?
                    selection.copy(0, 25) + u"..."_ustr : selection;
                // Note: VCL Edit doesn't have SetPlaceholderText, but we can update tooltip
                m_pInputField->SetQuickHelpText(u"Selection: \""_ustr + preview + u"\""_ustr);
            }
            else if (m_pInputField)
            {
                m_pInputField->SetQuickHelpText(u"No text selected - select text in document first"_ustr);
            }
        }
        else
        {
            m_pContextButton->SetText("@");
            m_pContextButton->SetQuickHelpText("Include selected text in message (@selection)");
            if (m_pInputField)
            {
                m_pInputField->SetQuickHelpText(u"Type your message here"_ustr);
            }
        }
    }
}

IMPL_LINK_NOARG(AgenticPanel, AttachClickHdl, Button*, void)
{
    // TODO: Open file picker for attachments
}

IMPL_LINK_NOARG(AgenticPanel, NewTabClickHdl, Button*, void)
{
    CreateNewSession();
}

IMPL_LINK_NOARG(AgenticPanel, SettingsClickHdl, Button*, void)
{
    // TODO: Open settings dialog
}

IMPL_LINK_NOARG(AgenticPanel, TabChangedHdl, TabControl*, void)
{
    if (!m_pTabControl)
        return;

    sal_uInt16 nPageId = m_pTabControl->GetCurPageId();

    // Find session by tab index
    sal_uInt16 idx = 0;
    for (const auto& [id, session] : m_sessions)
    {
        if (++idx == nPageId)
        {
            SwitchToSession(id);
            break;
        }
    }
}

IMPL_LINK_NOARG(AgenticPanel, ApproveAllClickHdl, Button*, void)
{
    ApproveAllEdits();
}

IMPL_LINK_NOARG(AgenticPanel, RejectAllClickHdl, Button*, void)
{
    RejectAllEdits();
}

IMPL_LINK_NOARG(AgenticPanel, ReviewClickHdl, Button*, void)
{
    // TODO: Open review dialog with diff view
}

// ========================================
// Streaming Event Handlers
// ========================================

void AgenticPanel::OnStreamEvent(const OUString& eventType, const OUString& eventData)
{
    if (eventType == "text")
    {
        // Append text to current assistant message (streaming)
    }
    else if (eventType == "tool_start")
    {
        ToolCallInfo tc;
        tc.id = GenerateId();
        tc.status = u"running"_ustr;
        // Parse eventData for tool name and arguments
        AppendToolCall(tc);
    }
    else if (eventType == "tool_complete")
    {
        // Update tool call status to success/error
    }
    else if (eventType == "pending_edit")
    {
        PendingEdit edit;
        edit.id = GenerateId();
        // Parse eventData for edit details
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

    AppendMessage(u"Error: "_ustr + error, AgenticMessage::Role::ASSISTANT);
}

// ========================================
// Lifecycle Methods
// ========================================

void AgenticPanel::StateChanged(StateChangedType nType)
{
    DockingWindow::StateChanged(nType);
}

void AgenticPanel::DataChanged(const DataChangedEvent& rDCEvt)
{
    DockingWindow::DataChanged(rDCEvt);
}

void AgenticPanel::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    DockingWindow::Paint(rRenderContext, rRect);

    // Draw messages in chat history area
    if (!m_pChatHistory)
        return;

    AgenticSession* session = GetCurrentSession();
    if (!session)
        return;

    Point chatPos = m_pChatHistory->GetPosPixel();
    Size chatSize = m_pChatHistory->GetSizePixel();

    // ========================================
    // Main chat background - dark theme for better contrast
    // ========================================
    rRenderContext.SetFillColor(Color(0x1E, 0x1E, 0x2E));  // Dark background like VS Code
    rRenderContext.SetLineColor();
    rRenderContext.DrawRect(tools::Rectangle(chatPos, chatSize));

    // Draw messages
    Point aPos(chatPos.X() + 12, chatPos.Y() + 12);
    const sal_Int32 lineHeight = 20;
    const sal_Int32 bubblePadding = 10;
    const sal_Int32 bubbleMargin = 8;
    const sal_Int32 maxBubbleWidth = chatSize.Width() - 40;

    for (const auto& msg : session->messages)
    {
        // Calculate message height for bubble
        sal_Int32 msgLines = 1;
        OUString content = msg.content;
        sal_Int32 tempIdx = 0;
        while (tempIdx < content.getLength())
        {
            sal_Int32 nextBreak = content.indexOf('\n', tempIdx);
            if (nextBreak >= 0)
            {
                msgLines++;
                tempIdx = nextBreak + 1;
            }
            else
            {
                tempIdx = content.getLength();
            }
        }
        sal_Int32 bubbleHeight = lineHeight * (msgLines + 1) + bubblePadding * 2;

        // ========================================
        // USER MESSAGE - Right-aligned blue bubble
        // ========================================
        if (msg.role == AgenticMessage::Role::USER)
        {
            // Blue bubble background
            sal_Int32 bubbleWidth = std::min(maxBubbleWidth, sal_Int32(content.getLength() * 8 + bubblePadding * 2));
            Point bubblePos(chatPos.X() + chatSize.Width() - bubbleWidth - 20, aPos.Y());

            rRenderContext.SetFillColor(Color(0x00, 0x7A, 0xCC));  // Blue for user
            rRenderContext.SetLineColor(Color(0x00, 0x5A, 0xAA));
            tools::Rectangle bubbleRect(bubblePos, Size(bubbleWidth, bubbleHeight));
            rRenderContext.DrawRect(bubbleRect);

            // Header: "You" with timestamp
            rRenderContext.SetTextColor(Color(0xFF, 0xFF, 0xFF));
            rRenderContext.DrawText(Point(bubblePos.X() + bubblePadding, aPos.Y() + bubblePadding),
                u"You"_ustr + "  " + msg.timestamp);

            // Content
            Point textPos(bubblePos.X() + bubblePadding, aPos.Y() + bubblePadding + lineHeight);
            sal_Int32 startIdx = 0;
            while (startIdx < content.getLength())
            {
                sal_Int32 endIdx = std::min(startIdx + 45, content.getLength());
                sal_Int32 lineBreak = content.indexOf('\n', startIdx);
                if (lineBreak >= 0 && lineBreak < endIdx)
                    endIdx = lineBreak;

                OUString line = content.copy(startIdx, endIdx - startIdx);
                rRenderContext.DrawText(textPos, line);
                textPos.AdjustY(lineHeight);

                startIdx = endIdx;
                if (lineBreak >= 0 && lineBreak == endIdx)
                    startIdx++;
            }

            aPos.AdjustY(bubbleHeight + bubbleMargin);
        }
        // ========================================
        // ASSISTANT MESSAGE - Left-aligned dark bubble
        // ========================================
        else if (msg.role == AgenticMessage::Role::ASSISTANT)
        {
            // Calculate bubble height including tool calls and edits
            sal_Int32 extraHeight = msg.toolCalls.size() * (lineHeight + 4);
            extraHeight += msg.pendingEdits.size() * (lineHeight * 2 + 8);
            bubbleHeight += extraHeight;

            // Dark gray bubble background
            Point bubblePos(chatPos.X() + 12, aPos.Y());
            sal_Int32 bubbleWidth = maxBubbleWidth;

            rRenderContext.SetFillColor(Color(0x2D, 0x2D, 0x3D));  // Dark gray for AI
            rRenderContext.SetLineColor(Color(0x3D, 0x3D, 0x4D));
            tools::Rectangle bubbleRect(bubblePos, Size(bubbleWidth, bubbleHeight));
            rRenderContext.DrawRect(bubbleRect);

            // Header: AI icon with timestamp
            rRenderContext.SetTextColor(Color(0x4E, 0xC9, 0xB0));  // Teal/cyan for AI
            rRenderContext.DrawText(Point(bubblePos.X() + bubblePadding, aPos.Y() + bubblePadding),
                OUString::fromUtf8("\xF0\x9F\xA4\x96 AI Agent") + "  " + msg.timestamp);  // 🤖

            // Content
            rRenderContext.SetTextColor(Color(0xE0, 0xE0, 0xE0));  // Light gray text
            Point textPos(bubblePos.X() + bubblePadding, aPos.Y() + bubblePadding + lineHeight);
            sal_Int32 startIdx = 0;
            while (startIdx < content.getLength())
            {
                sal_Int32 endIdx = std::min(startIdx + 50, content.getLength());
                sal_Int32 lineBreak = content.indexOf('\n', startIdx);
                if (lineBreak >= 0 && lineBreak < endIdx)
                    endIdx = lineBreak;

                OUString line = content.copy(startIdx, endIdx - startIdx);
                rRenderContext.DrawText(textPos, line);
                textPos.AdjustY(lineHeight);

                startIdx = endIdx;
                if (lineBreak >= 0 && lineBreak == endIdx)
                    startIdx++;
            }

            // ========================================
            // TOOL CALLS - Styled boxes
            // ========================================
            for (const auto& tc : msg.toolCalls)
            {
                textPos.AdjustY(4);

                // Tool call background
                rRenderContext.SetFillColor(Color(0x3D, 0x3D, 0x5C));
                tools::Rectangle toolRect(Point(textPos.X(), textPos.Y()),
                                          Size(bubbleWidth - bubblePadding * 2, lineHeight + 4));
                rRenderContext.DrawRect(toolRect);

                // Status indicator
                OUString statusIcon;
                Color statusColor;
                if (tc.status == "success")
                {
                    statusIcon = OUString::fromUtf8("\xE2\x9C\x93");  // ✓
                    statusColor = Color(0x4E, 0xC9, 0x4E);  // Green
                }
                else if (tc.status == "running")
                {
                    statusIcon = OUString::fromUtf8("\xE2\x8F\xB3");  // ⏳
                    statusColor = Color(0xFF, 0xC1, 0x07);  // Yellow
                }
                else
                {
                    statusIcon = OUString::fromUtf8("\xE2\x9C\x97");  // ✗
                    statusColor = Color(0xF4, 0x43, 0x36);  // Red
                }

                rRenderContext.SetTextColor(statusColor);
                rRenderContext.DrawText(Point(textPos.X() + 4, textPos.Y() + 2), statusIcon);

                rRenderContext.SetTextColor(Color(0xCE, 0x91, 0x78));  // Orange for tool name
                OUString toolText = " " + tc.name;
                if (!tc.result.isEmpty())
                {
                    OUString resultPreview = tc.result.getLength() > 35 ?
                        tc.result.copy(0, 35) + u"..."_ustr : tc.result;
                    rRenderContext.SetTextColor(Color(0x9C, 0xDC, 0xFE));  // Light blue for result
                    toolText += u" -> "_ustr + resultPreview;
                }
                rRenderContext.DrawText(Point(textPos.X() + 20, textPos.Y() + 2), toolText);

                textPos.AdjustY(lineHeight + 4);
            }

            // ========================================
            // PENDING EDITS - Styled boxes with diff preview
            // ========================================
            for (const auto& edit : msg.pendingEdits)
            {
                textPos.AdjustY(4);

                Color editBgColor;
                Color editTextColor;
                OUString statusIcon;
                OUString statusText;

                if (edit.approved)
                {
                    editBgColor = Color(0x1B, 0x4B, 0x1B);  // Dark green
                    editTextColor = Color(0x4E, 0xC9, 0x4E);  // Bright green
                    statusIcon = OUString::fromUtf8("\xE2\x9C\x93");  // ✓
                    statusText = u" Applied: "_ustr + edit.editType;
                }
                else if (edit.rejected)
                {
                    editBgColor = Color(0x4B, 0x1B, 0x1B);  // Dark red
                    editTextColor = Color(0xF4, 0x43, 0x36);  // Bright red
                    statusIcon = OUString::fromUtf8("\xE2\x9C\x97");  // ✗
                    statusText = u" Rejected: "_ustr + edit.editType;
                }
                else
                {
                    editBgColor = Color(0x4B, 0x4B, 0x1B);  // Dark yellow
                    editTextColor = Color(0xFF, 0xC1, 0x07);  // Bright yellow
                    statusIcon = OUString::fromUtf8("\xE2\x9A\xA0");  // ⚠
                    statusText = u" PENDING: "_ustr + edit.editType;
                }

                // Edit background
                rRenderContext.SetFillColor(editBgColor);
                sal_Int32 editBoxHeight = edit.approved || edit.rejected ? lineHeight + 4 : lineHeight * 2 + 8;
                tools::Rectangle editRect(Point(textPos.X(), textPos.Y()),
                                          Size(bubbleWidth - bubblePadding * 2, editBoxHeight));
                rRenderContext.DrawRect(editRect);

                // Status line
                rRenderContext.SetTextColor(editTextColor);
                rRenderContext.DrawText(Point(textPos.X() + 4, textPos.Y() + 2), statusIcon + statusText);

                // Show preview for pending edits
                if (!edit.approved && !edit.rejected && !edit.newText.isEmpty())
                {
                    textPos.AdjustY(lineHeight + 2);
                    OUString preview = edit.newText.getLength() > 45 ?
                        edit.newText.copy(0, 45) + u"..."_ustr : edit.newText;
                    rRenderContext.SetTextColor(Color(0x4E, 0xC9, 0x4E));  // Green for additions
                    rRenderContext.DrawText(Point(textPos.X() + 12, textPos.Y() + 2),
                        OUString::fromUtf8("+ ") + preview);
                }

                textPos.AdjustY(lineHeight + 4);
            }

            aPos.AdjustY(bubbleHeight + bubbleMargin);
        }
        // ========================================
        // SYSTEM MESSAGE - Centered gray text
        // ========================================
        else
        {
            rRenderContext.SetTextColor(Color(0x88, 0x88, 0x88));
            Point centerPos(chatPos.X() + chatSize.Width() / 2 - content.getLength() * 3, aPos.Y());
            rRenderContext.DrawText(centerPos, OUString::fromUtf8("\xE2\x84\xB9 ") + content);  // ℹ
            aPos.AdjustY(lineHeight + bubbleMargin);
        }

        // Check bounds
        if (aPos.Y() > chatPos.Y() + chatSize.Height() - lineHeight)
            break;
    }

    // ========================================
    // Empty state message
    // ========================================
    if (session->messages.empty())
    {
        rRenderContext.SetTextColor(Color(0x88, 0x88, 0x88));
        Point centerPos(chatPos.X() + chatSize.Width() / 2 - 100, chatPos.Y() + chatSize.Height() / 2);
        rRenderContext.DrawText(centerPos, u"Start a conversation..."_ustr);
        rRenderContext.DrawText(Point(centerPos.X() - 30, centerPos.Y() + lineHeight * 2),
            u"Type a message or use @ for selection"_ustr);
    }
}

} // namespace officelabs

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
