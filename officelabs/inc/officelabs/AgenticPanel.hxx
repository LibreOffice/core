/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of OfficeLabs.
 *
 * AgenticPanel - Full agentic AI assistant panel with:
 * - Multi-session tab management
 * - Tool call visualization with approval flow
 * - Streaming response display
 * - Agent interruption capability
 */

#ifndef INCLUDED_OFFICELABS_AGENTICPANEL_HXX
#define INCLUDED_OFFICELABS_AGENTICPANEL_HXX

#include <sfx2/sidebar/PanelLayout.hxx>
#include <vcl/weld.hxx>
#include <memory>
#include <vector>
#include <map>

namespace officelabs
{

// Forward declarations
class AgentConnection;
class DocumentController;

/**
 * Represents a pending edit that needs user approval
 */
struct PendingEdit
{
    OUString id;
    OUString sectionId;
    OUString originalText;
    OUString newText;
    OUString editType;
    OUString description;
    bool approved = false;
    bool rejected = false;
};

/**
 * Represents a tool call made by the agent
 */
struct ToolCallInfo
{
    OUString id;
    OUString name;
    OUString status;  // "running", "success", "error"
    OUString result;
    OUString error;
};

/**
 * Represents a single message in the chat
 */
struct ChatMessage
{
    enum class Role { USER, ASSISTANT };
    Role role;
    OUString content;
    OUString timestamp;
    std::vector<ToolCallInfo> toolCalls;
    std::vector<PendingEdit> pendingEdits;
};

/**
 * Represents a chat session
 */
struct ChatSession
{
    OUString id;
    OUString name;
    std::vector<ChatMessage> messages;
    std::vector<PendingEdit> pendingEdits;
    bool isActive = true;
};

/**
 * Document context sent to AI
 */
struct DocumentContextInfo
{
    OUString title;
    OUString app;  // "writer", "calc", "impress"
    OUString content;
    OUString selection;
    sal_Int32 cursorPosition = 0;
    OUString url;
};

/**
 * AgenticPanel - Main panel for agentic AI assistant
 *
 * Features:
 * - Tab bar for multiple chat sessions
 * - Message display with tool call visualization
 * - Pending edit approval with diff preview
 * - Stop button for agent interruption
 * - Settings access
 */
class AgenticPanel : public sfx2::sidebar::PanelLayout
{
public:
    AgenticPanel(weld::Widget* pParent);
    virtual ~AgenticPanel() override;

    // Session management
    void CreateNewSession(const OUString& name = OUString());
    void SwitchToSession(const OUString& sessionId);
    void CloseSession(const OUString& sessionId);
    void RenameSession(const OUString& sessionId, const OUString& newName);

    // Message handling
    void SendMessage(const OUString& message);
    void AppendUserMessage(const OUString& message);
    void AppendAssistantMessage(const OUString& message);
    void AppendToolCall(const ToolCallInfo& toolCall);
    void AppendPendingEdit(const PendingEdit& edit);

    // Edit approval
    void ApproveEdit(const OUString& editId);
    void RejectEdit(const OUString& editId);
    void ApproveAllEdits();
    void RejectAllEdits();

    // Agent control
    void StopAgent();
    void ResetSession();  // Clear current session and start fresh
    bool IsAgentRunning() const { return m_bAgentRunning; }

    // Document context
    void RefreshDocumentContext();
    DocumentContextInfo GetCurrentDocumentContext();
    OUString GetSelectedText();
    void InsertAtCursor(const OUString& text);
    void ReplaceSelection(const OUString& text);
    void ApplyFormatting(const OUString& formatType, const OUString& value);

    // Connection
    void SetConnected(bool connected);
    bool IsConnected() const { return m_bConnected; }

private:
    // UI Components - Tab bar
    std::unique_ptr<weld::Notebook> m_xTabBar;
    std::unique_ptr<weld::Button> m_xNewTabButton;

    // UI Components - Status bar
    std::unique_ptr<weld::Box> m_xStatusBar;
    std::unique_ptr<weld::Label> m_xConnectionStatus;
    std::unique_ptr<weld::Label> m_xModelLabel;
    std::unique_ptr<weld::Button> m_xSettingsButton;

    // UI Components - Chat area
    std::unique_ptr<weld::ScrolledWindow> m_xChatScroll;
    std::unique_ptr<weld::Box> m_xMessagesContainer;
    std::unique_ptr<weld::TextView> m_xChatHistory;  // Simplified for now

    // UI Components - Input area
    std::unique_ptr<weld::Box> m_xInputArea;
    std::unique_ptr<weld::Button> m_xAttachButton;
    std::unique_ptr<weld::Button> m_xContextButton;  // "@" button for selection reference
    std::unique_ptr<weld::Entry> m_xInputField;
    std::unique_ptr<weld::Button> m_xStopButton;
    std::unique_ptr<weld::Button> m_xResetButton;    // Reset session button
    std::unique_ptr<weld::Button> m_xSendButton;

    // UI Components - Edit approval (shown when edits pending)
    std::unique_ptr<weld::Box> m_xEditApprovalBar;
    std::unique_ptr<weld::Label> m_xPendingEditsLabel;
    std::unique_ptr<weld::Button> m_xApproveAllButton;
    std::unique_ptr<weld::Button> m_xRejectAllButton;
    std::unique_ptr<weld::Button> m_xReviewButton;

    // Backend connection
    std::unique_ptr<AgentConnection> m_pAgentConnection;
    std::unique_ptr<DocumentController> m_pDocController;

    // State
    std::map<OUString, ChatSession> m_sessions;
    OUString m_currentSessionId;
    OUString m_currentExecutionId;
    DocumentContextInfo m_currentContext;  // Current document context
    bool m_bConnected = false;
    bool m_bAgentRunning = false;
    bool m_bIncludeSelection = false;  // Whether to include selection in next message

    // Event handlers
    DECL_LINK(SendClickHdl, weld::Button&, void);
    DECL_LINK(StopClickHdl, weld::Button&, void);
    DECL_LINK(ResetClickHdl, weld::Button&, void);
    DECL_LINK(NewTabClickHdl, weld::Button&, void);
    DECL_LINK(TabChangedHdl, weld::Notebook&, void);
    DECL_LINK(SettingsClickHdl, weld::Button&, void);
    DECL_LINK(AttachClickHdl, weld::Button&, void);
    DECL_LINK(ContextClickHdl, weld::Button&, void);
    DECL_LINK(ApproveAllClickHdl, weld::Button&, void);
    DECL_LINK(RejectAllClickHdl, weld::Button&, void);
    DECL_LINK(ReviewClickHdl, weld::Button&, void);
    DECL_LINK(InputKeyPressHdl, const KeyEvent&, bool);

    // Helper methods
    void InitializeUI();
    void UpdateUI();
    void UpdateTabBar();
    void UpdateStatusBar();
    void UpdateEditApprovalBar();
    void RenderMessages();
    void RenderToolCall(const ToolCallInfo& toolCall);
    void RenderPendingEdit(const PendingEdit& edit);
    void ScrollToBottom();
    OUString GetDocumentContext();
    ChatSession* GetCurrentSession();

    // Streaming response handling
    void OnStreamEvent(const OUString& eventType, const OUString& eventData);
    void OnStreamComplete();
    void OnStreamError(const OUString& error);
};

} // namespace officelabs

#endif // INCLUDED_OFFICELABS_AGENTICPANEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
