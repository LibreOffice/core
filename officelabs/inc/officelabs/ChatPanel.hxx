#ifndef INCLUDED_OFFICELABS_CHATPANEL_HXX
#define INCLUDED_OFFICELABS_CHATPANEL_HXX

#include <sfx2/sidebar/PanelLayout.hxx>
#include <sfx2/dllapi.h>
#include <vcl/weld.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <rtl/ustring.hxx>
#include <memory>
#include <vector>

namespace officelabs {

class AgentConnection;
class DocumentController;

struct ChatMessage {
    enum class Role { USER, ASSISTANT };
    Role role;
    OUString content;
    OUString timestamp;
};

class SFX2_DLLPUBLIC ChatPanel : public sfx2::sidebar::PanelLayout {
private:
    // UI Components (minimal stub)
    std::unique_ptr<weld::Label> m_xStatusLabel;
    std::unique_ptr<weld::TextView> m_xChatHistory;
    std::unique_ptr<weld::Entry> m_xInputField;
    std::unique_ptr<weld::Button> m_xSendButton;
    
    // Backend (minimal stub)
    std::unique_ptr<AgentConnection> m_pAgent;
    std::unique_ptr<DocumentController> m_pDocController;
    
    // State
    std::vector<ChatMessage> m_messages;
    bool m_connected;
    
    // Event handlers
    DECL_LINK(SendClickHdl, weld::Button&, void);

public:
    ChatPanel(weld::Widget& rParent);
    virtual ~ChatPanel();
    
    // Minimal interface
    void SendMessage(const OUString& message);
    void SetDocument(const css::uno::Reference<css::text::XTextDocument>& xDoc);
    
    // Factory
    static std::unique_ptr<PanelLayout> Create(weld::Widget& rParent);
};

} // namespace officelabs

#endif