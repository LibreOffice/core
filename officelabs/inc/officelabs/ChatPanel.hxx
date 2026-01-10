/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs AI Assistant - Chat Panel Docking Window
 */

#ifndef INCLUDED_OFFICELABS_CHATPANEL_HXX
#define INCLUDED_OFFICELABS_CHATPANEL_HXX

#include <sfx2/dockwin.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <memory>
#include <vector>
#include <optional>

namespace officelabs {

class AgentConnection;
class DocumentController;

struct ChatMessage {
    enum class Role { USER, ASSISTANT };
    Role role;
    OUString content;
    OUString timestamp;
};

struct Patch {
    enum class Type { REPLACE, INSERT, DELETE };
    Type type;
    OUString target;
    OUString oldValue;
    OUString newValue;
    OUString diff;
};

class ChatPanel final : public SfxDockingWindow {
private:
    // UI Components
    VclPtr<FixedText> m_pStatusLabel;
    VclPtr<FixedText> m_pContextLabel;
    VclPtr<Control> m_pChatHistory;
    VclPtr<Edit> m_pInputField;
    VclPtr<PushButton> m_pSendButton;
    VclPtr<PushButton> m_pClearButton;

    // Patch preview
    VclPtr<Control> m_pPatchPreview;
    VclPtr<PushButton> m_pAcceptButton;
    VclPtr<PushButton> m_pRejectButton;
    VclPtr<PushButton> m_pRegenerateButton;

    // Backend & Document
    std::unique_ptr<AgentConnection> m_pAgent;
    std::unique_ptr<DocumentController> m_pDocController;
    css::uno::Reference<css::text::XTextDocument> m_xDocument;

    // State
    std::vector<ChatMessage> m_messages;
    std::optional<Patch> m_currentPatch;
    bool m_connected;
    bool m_processing;

    // Layout
    void DoLayout();

    // Event handlers
    DECL_LINK(SendClickHdl, Button*, void);
    DECL_LINK(ClearClickHdl, Button*, void);
    DECL_LINK(AcceptClickHdl, Button*, void);
    DECL_LINK(RejectClickHdl, Button*, void);
    DECL_LINK(RegenerateClickHdl, Button*, void);

    // Backend communication
    void SendToAgent(const OUString& message);
    void CheckConnection();

    // Document operations
    void ApplyPatch(const Patch& patch);

public:
    ChatPanel(SfxBindings* pBindings, SfxChildWindow* pChildWin, vcl::Window* pParent);
    virtual ~ChatPanel() override;
    virtual void dispose() override;

    // Lifecycle
    virtual void StateChanged(StateChangedType nType) override;
    virtual void DataChanged(const DataChangedEvent& rDCEvt) override;
    virtual void Resize() override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

    // Chat operations
    void SendMessage(const OUString& message);
    void ShowPatchPreview(const Patch& patch);
    void ClearChat();

    // Status
    void SetProcessing(bool processing);

    // Document
    void SetDocument(const css::uno::Reference<css::text::XTextDocument>& xDoc);
};

} // namespace officelabs

#endif // INCLUDED_OFFICELABS_CHATPANEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
