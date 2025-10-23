#include <officelabs/ChatPanel.hxx>
#include <officelabs/AgentConnection.hxx>
#include <officelabs/DocumentController.hxx>
#include <vcl/svapp.hxx>
#include <tools/datetime.hxx>

namespace officelabs {

ChatPanel::ChatPanel(weld::Widget& rParent)
    : PanelLayout(&rParent, "ChatPanel", "modules/swriter/ui/chatpanel.ui")
    , m_connected(false)
{
    // Minimal stub - just create basic UI elements
    try {
        m_xStatusLabel = m_xBuilder->weld_label("status_label");
        m_xChatHistory = m_xBuilder->weld_text_view("chat_history");
        m_xInputField = m_xBuilder->weld_entry("input_field");
        m_xSendButton = m_xBuilder->weld_button("send_button");
        
        if (m_xSendButton) {
            m_xSendButton->connect_clicked(LINK(this, ChatPanel, SendClickHdl));
        }
        
        if (m_xStatusLabel) {
            m_xStatusLabel->set_label("OfficeLabs AI - Ready");
        }
        
        // Create backend connections (minimal)
        m_pAgent = std::make_unique<AgentConnection>();
        m_pDocController = std::make_unique<DocumentController>();
        
    } catch (...) {
        // Fallback if UI file not found - just continue
    }
}

ChatPanel::~ChatPanel() {
}

std::unique_ptr<PanelLayout> ChatPanel::Create(weld::Widget& rParent) {
    return std::make_unique<ChatPanel>(rParent);
}

void ChatPanel::SendMessage(const OUString& message) {
    if (message.isEmpty()) return;
    
    // Add to chat history (minimal implementation)
    ChatMessage userMsg;
    userMsg.role = ChatMessage::Role::USER;
    userMsg.content = message;
    userMsg.timestamp = "now";
    m_messages.push_back(userMsg);
    
    // Simple response
    ChatMessage aiMsg;
    aiMsg.role = ChatMessage::Role::ASSISTANT;
    aiMsg.content = "AI response: " + message;
    aiMsg.timestamp = "now";
    m_messages.push_back(aiMsg);
    
    // Update display
    if (m_xChatHistory) {
        OUString display;
        for (const auto& msg : m_messages) {
            display += (msg.role == ChatMessage::Role::USER ? "You: " : "AI: ");
            display += msg.content + "\n";
        }
        m_xChatHistory->set_text(display);
    }
    
    // Clear input
    if (m_xInputField) {
        m_xInputField->set_text("");
    }
}

void ChatPanel::SetDocument(const css::uno::Reference<css::text::XTextDocument>& xDoc) {
    if (m_pDocController) {
        m_pDocController->setDocument(xDoc);
    }
}

IMPL_LINK_NOARG(ChatPanel, SendClickHdl, weld::Button&, void) {
    if (m_xInputField) {
        OUString message = m_xInputField->get_text();
        if (!message.isEmpty()) {
            SendMessage(message);
        }
    }
}

} // namespace officelabs