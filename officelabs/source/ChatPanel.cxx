/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs AI Assistant - Chat Panel Docking Window Implementation
 */

#include <officelabs/ChatPanel.hxx>
#include <officelabs/AgentConnection.hxx>
#include <officelabs/DocumentController.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/bindings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <tools/datetime.hxx>

namespace officelabs {

ChatPanel::ChatPanel(SfxBindings* pBindings, SfxChildWindow* pChildWin, vcl::Window* pParent)
    : SfxDockingWindow(pBindings, pChildWin, pParent, WB_STDDOCKWIN | WB_CLOSEABLE | WB_SIZEABLE)
    , m_connected(false)
    , m_processing(false)
{
    SetText("OfficeLabs AI Chat");
    SetSizePixel(Size(400, 600));
    
    // Create agent connection
    m_pAgent = std::make_unique<AgentConnection>();
    m_pDocController = std::make_unique<DocumentController>();
    
    // Create UI components
    m_pStatusLabel = VclPtr<FixedText>::Create(this, WB_LEFT);
    m_pContextLabel = VclPtr<FixedText>::Create(this, WB_LEFT);
    m_pChatHistory = VclPtr<Control>::Create(this, WB_BORDER);
    m_pInputField = VclPtr<Edit>::Create(this, WB_BORDER);
    m_pSendButton = VclPtr<PushButton>::Create(this);
    m_pClearButton = VclPtr<PushButton>::Create(this);
    
    // Patch preview (initially hidden)
    m_pPatchPreview = VclPtr<Control>::Create(this, WB_BORDER);
    m_pAcceptButton = VclPtr<PushButton>::Create(this);
    m_pRejectButton = VclPtr<PushButton>::Create(this);
    m_pRegenerateButton = VclPtr<PushButton>::Create(this);
    
    // Set button labels
    m_pSendButton->SetText("Send");
    m_pClearButton->SetText("Clear");
    m_pAcceptButton->SetText("Accept");
    m_pRejectButton->SetText("Reject");
    m_pRegenerateButton->SetText("Regenerate");
    
    // Connect handlers
    m_pSendButton->SetClickHdl(LINK(this, ChatPanel, SendClickHdl));
    m_pClearButton->SetClickHdl(LINK(this, ChatPanel, ClearClickHdl));
    m_pAcceptButton->SetClickHdl(LINK(this, ChatPanel, AcceptClickHdl));
    m_pRejectButton->SetClickHdl(LINK(this, ChatPanel, RejectClickHdl));
    m_pRegenerateButton->SetClickHdl(LINK(this, ChatPanel, RegenerateClickHdl));
    
    // Show all except patch preview
    m_pStatusLabel->Show();
    m_pContextLabel->Show();
    m_pChatHistory->Show();
    m_pInputField->Show();
    m_pSendButton->Show();
    m_pClearButton->Show();
    
    m_pPatchPreview->Hide();
    m_pAcceptButton->Hide();
    m_pRejectButton->Hide();
    m_pRegenerateButton->Hide();
    
    // Check connection
    CheckConnection();
    
    DoLayout();
}

ChatPanel::~ChatPanel() {
    disposeOnce();
}

void ChatPanel::dispose() {
    m_pStatusLabel.disposeAndClear();
    m_pContextLabel.disposeAndClear();
    m_pChatHistory.disposeAndClear();
    m_pInputField.disposeAndClear();
    m_pSendButton.disposeAndClear();
    m_pClearButton.disposeAndClear();
    m_pPatchPreview.disposeAndClear();
    m_pAcceptButton.disposeAndClear();
    m_pRejectButton.disposeAndClear();
    m_pRegenerateButton.disposeAndClear();
    SfxDockingWindow::dispose();
}

void ChatPanel::CheckConnection() {
    if (m_pAgent) {
        m_connected = m_pAgent->checkConnection();
        if (m_connected) {
            m_pStatusLabel->SetText("● Connected to OfficeLabs Backend");
        } else {
            m_pStatusLabel->SetText("○ Backend not available");
        }
    }
}

void ChatPanel::SetDocument(const css::uno::Reference<css::text::XTextDocument>& xDoc) {
    m_xDocument = xDoc;
    if (m_pDocController) {
        m_pDocController->setDocument(xDoc);
    }
    
    if (xDoc.is()) {
        m_pContextLabel->SetText("Document: Active");
    } else {
        m_pContextLabel->SetText("Document: None");
    }
}

void ChatPanel::DoLayout() {
    Size aSize = GetSizePixel();
    Point aPos(12, 12);
    
    // Status label
    m_pStatusLabel->SetPosSizePixel(aPos, Size(aSize.Width() - 24, 25));
    aPos.AdjustY(30);
    
    // Context label
    m_pContextLabel->SetPosSizePixel(aPos, Size(aSize.Width() - 24, 25));
    aPos.AdjustY(30);
    
    // Chat history
    sal_Int32 inputHeight = 80;
    sal_Int32 historyHeight = aSize.Height() - aPos.Y() - inputHeight - 24;
    m_pChatHistory->SetPosSizePixel(aPos, Size(aSize.Width() - 24, historyHeight));
    aPos.AdjustY(historyHeight + 8);
    
    // Input field
    m_pInputField->SetPosSizePixel(aPos, Size(aSize.Width() - 100, 30));
    
    // Send button
    m_pSendButton->SetPosSizePixel(
        Point(aPos.X() + aSize.Width() - 90, aPos.Y()),
        Size(70, 30)
    );
    aPos.AdjustY(35);
    
    // Clear button
    m_pClearButton->SetPosSizePixel(aPos, Size(100, 25));
}

void ChatPanel::Resize() {
    DockingWindow::Resize();
    DoLayout();
}

IMPL_LINK_NOARG(ChatPanel, SendClickHdl, Button*, void) {
    OUString message = m_pInputField->GetText();
    if (message.isEmpty()) return;
    
    SendMessage(message);
    m_pInputField->SetText("");
}

IMPL_LINK_NOARG(ChatPanel, ClearClickHdl, Button*, void) {
    ClearChat();
}

IMPL_LINK_NOARG(ChatPanel, AcceptClickHdl, Button*, void) {
    if (m_currentPatch.has_value()) {
        ApplyPatch(m_currentPatch.value());
        m_currentPatch.reset();
        
        m_pPatchPreview->Hide();
        m_pAcceptButton->Hide();
        m_pRejectButton->Hide();
        m_pRegenerateButton->Hide();
    }
}

IMPL_LINK_NOARG(ChatPanel, RejectClickHdl, Button*, void) {
    m_currentPatch.reset();
    m_pPatchPreview->Hide();
    m_pAcceptButton->Hide();
    m_pRejectButton->Hide();
    m_pRegenerateButton->Hide();
}

IMPL_LINK_NOARG(ChatPanel, RegenerateClickHdl, Button*, void) {
    // TODO: Request regeneration
}

void ChatPanel::SendMessage(const OUString& message) {
    // Add user message
    ChatMessage userMsg;
    userMsg.role = ChatMessage::Role::USER;
    userMsg.content = message;
    userMsg.timestamp = u""_ustr;
    m_messages.push_back(userMsg);
    
    Invalidate();
    
    // Send to backend
    SendToAgent(message);
}

void ChatPanel::SendToAgent(const OUString& message) {
    if (!m_pAgent || !m_pDocController) return;
    
    SetProcessing(true);
    
    OUString context = m_pDocController->getDocumentText();
    AgentResponse response = m_pAgent->sendMessage(message, context);
    
    // Display response
    ChatMessage aiMsg;
    aiMsg.role = ChatMessage::Role::ASSISTANT;
    aiMsg.content = response.message;
    aiMsg.timestamp = u""_ustr;
    m_messages.push_back(aiMsg);
    
    // Show patch if available
    if (response.hasPatch) {
        Patch patch;
        patch.type = Patch::Type::REPLACE;
        patch.diff = response.patchDiff;
        ShowPatchPreview(patch);
    }
    
    SetProcessing(false);
    Invalidate();
}

void ChatPanel::ShowPatchPreview(const Patch& patch) {
    m_currentPatch = patch;
    m_pPatchPreview->Show();
    m_pAcceptButton->Show();
    m_pRejectButton->Show();
    m_pRegenerateButton->Show();
}

void ChatPanel::ApplyPatch(const Patch& patch) {
    if (!m_pDocController || !m_pDocController->hasDocument()) return;
    
    // Simple implementation: replace all text
    // TODO: Implement proper patch application
    OUString currentText = m_pDocController->getDocumentText();
    m_pDocController->replaceText(0, currentText.getLength(), patch.newValue);
}

void ChatPanel::ClearChat() {
    m_messages.clear();
    Invalidate();
}

void ChatPanel::SetProcessing(bool processing) {
    m_processing = processing;
    m_pSendButton->Enable(!processing);
}

void ChatPanel::StateChanged(StateChangedType nType) {
    DockingWindow::StateChanged(nType);
}

void ChatPanel::DataChanged(const DataChangedEvent& rDCEvt) {
    DockingWindow::DataChanged(rDCEvt);
}

void ChatPanel::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) {
    DockingWindow::Paint(rRenderContext, rRect);
    
    // Simple message rendering
    Point aPos(12, 70);
    for (const auto& msg : m_messages) {
        OUString prefix = (msg.role == ChatMessage::Role::USER) ? "You: " : "AI: ";
        rRenderContext.DrawText(aPos, prefix + msg.content);
        aPos.AdjustY(20);
    }
}

} // namespace officelabs

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
