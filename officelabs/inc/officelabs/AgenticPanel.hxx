/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of OfficeLabs.
 *
 * AgenticPanel - Full agentic AI assistant panel using VCL toolkit
 *
 * This provides a Cursor-like AI experience for LibreOffice with:
 * - Multi-session chat tabs
 * - Document context and selection reference (@selection)
 * - Tool call visualization (read_document, edit_section, etc.)
 * - Pending edit approval workflow
 * - Real-time agent status
 */

#ifndef INCLUDED_OFFICELABS_AGENTICPANEL_HXX
#define INCLUDED_OFFICELABS_AGENTICPANEL_HXX

// Allow deprecated VCL toolkit headers for docking window
#define VCL_INTERNALS

#include <officelabs/officelabsdllapi.h>
#include <sfx2/dockwin.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/toolkit/fixed.hxx>
#include <vcl/toolkit/edit.hxx>
#include <vcl/toolkit/button.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <memory>
#include <vector>
#include <map>

namespace officelabs
{

// Forward declarations
class AgentConnection;
class DocumentController;

/**
 * Tool call information - shows what tools the AI is using
 */
struct ToolCallInfo
{
    OUString id;
    OUString name;           // e.g., "read_document", "edit_section"
    OUString arguments;      // JSON string of arguments
    OUString status;         // "running", "success", "error"
    OUString result;         // Result or error message
};

/**
 * Pending edit - an edit waiting for user approval
 */
struct PendingEdit
{
    OUString id;
    OUString editType;       // "INSERT", "REPLACE", "DELETE"
    OUString targetSection;  // Section/location being edited
    OUString originalText;   // Text before edit (for REPLACE/DELETE)
    OUString newText;        // New text (for INSERT/REPLACE)
    OUString description;    // Human-readable description
    bool approved = false;
    bool rejected = false;
};

/**
 * Chat message structure with tool calls and pending edits
 */
struct AgenticMessage
{
    enum class Role { USER, ASSISTANT, SYSTEM };
    Role role;
    OUString content;
    OUString timestamp;
    std::vector<ToolCallInfo> toolCalls;
    std::vector<PendingEdit> pendingEdits;
};

/**
 * Chat session structure with pending edits
 */
struct AgenticSession
{
    OUString id;
    OUString name;
    std::vector<AgenticMessage> messages;
    std::vector<PendingEdit> pendingEdits;  // All pending edits in session
    bool isActive = true;
};

/**
 * AgenticPanel - Full agentic AI panel with Cursor-like features
 *
 * Features:
 * - Multi-session tabs with create/switch
 * - Selection context (@selection button)
 * - Tool call visualization (shows AI actions)
 * - Pending edit approval (Accept/Reject edits)
 * - Stop agent button
 * - Reset session button
 * - Document manipulation via DocumentController
 */
class OFFICELABS_DLLPUBLIC AgenticPanel final : public SfxDockingWindow
{
private:
    // UI Components - Header
    VclPtr<FixedText> m_pStatusLabel;
    VclPtr<FixedText> m_pModelLabel;
    VclPtr<PushButton> m_pSettingsButton;

    // UI Components - Tabs
    VclPtr<TabControl> m_pTabControl;
    VclPtr<PushButton> m_pNewTabButton;

    // UI Components - Chat area
    VclPtr<Control> m_pChatHistory;

    // UI Components - Edit approval bar (shown when edits pending)
    VclPtr<Control> m_pEditApprovalBar;
    VclPtr<FixedText> m_pPendingEditsLabel;
    VclPtr<PushButton> m_pApproveAllButton;
    VclPtr<PushButton> m_pRejectAllButton;
    VclPtr<PushButton> m_pReviewButton;

    // UI Components - Input area
    VclPtr<PushButton> m_pAttachButton;
    VclPtr<PushButton> m_pContextButton;  // @ button for selection
    VclPtr<Edit> m_pInputField;
    VclPtr<PushButton> m_pStopButton;
    VclPtr<PushButton> m_pResetButton;
    VclPtr<PushButton> m_pSendButton;

    // Backend & Document
    std::unique_ptr<AgentConnection> m_pAgent;
    std::unique_ptr<DocumentController> m_pDocController;
    css::uno::Reference<css::text::XTextDocument> m_xDocument;

    // State
    std::map<OUString, AgenticSession> m_sessions;
    OUString m_currentSessionId;
    OUString m_currentExecutionId;
    bool m_bConnected;
    bool m_bAgentRunning;
    bool m_bIncludeSelection;
    sal_Int32 m_nNextSessionNum;

    // Layout
    void DoLayout();

    // Session management
    OUString CreateNewSession(const OUString& name = OUString());
    void SwitchToSession(const OUString& sessionId);
    void CloseSession(const OUString& sessionId);
    AgenticSession* GetCurrentSession();

    // Event handlers
    DECL_LINK(SendClickHdl, Button*, void);
    DECL_LINK(StopClickHdl, Button*, void);
    DECL_LINK(ResetClickHdl, Button*, void);
    DECL_LINK(ContextClickHdl, Button*, void);
    DECL_LINK(AttachClickHdl, Button*, void);
    DECL_LINK(NewTabClickHdl, Button*, void);
    DECL_LINK(SettingsClickHdl, Button*, void);
    DECL_LINK(TabChangedHdl, TabControl*, void);
    DECL_LINK(ApproveAllClickHdl, Button*, void);
    DECL_LINK(RejectAllClickHdl, Button*, void);
    DECL_LINK(ReviewClickHdl, Button*, void);

    // UI helpers
    void UpdateStatusBar();
    void UpdateEditApprovalBar();
    void RenderMessages();
    void AppendMessage(const OUString& content, AgenticMessage::Role role);
    void AppendToolCall(const ToolCallInfo& toolCall);
    void AppendPendingEdit(const PendingEdit& edit);

    // Backend
    void SendMessage(const OUString& message);
    void CheckConnection();

    // Edit approval
    void ApproveEdit(const OUString& editId);
    void RejectEdit(const OUString& editId);
    void ApproveAllEdits();
    void RejectAllEdits();
    void ApplyEdit(const PendingEdit& edit);

public:
    AgenticPanel(SfxBindings* pBindings, SfxChildWindow* pChildWin, vcl::Window* pParent);
    virtual ~AgenticPanel() override;
    virtual void dispose() override;

    // Lifecycle
    virtual void StateChanged(StateChangedType nType) override;
    virtual void DataChanged(const DataChangedEvent& rDCEvt) override;
    virtual void Resize() override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

    // Document
    void SetDocument(const css::uno::Reference<css::text::XTextDocument>& xDoc);

    // Selection context
    OUString GetSelectedText();
    void RefreshDocumentContext();

    // Document manipulation
    void InsertAtCursor(const OUString& text);
    void ReplaceSelection(const OUString& text);
    void ApplyFormatting(const OUString& formatType, const OUString& value);

    // Agent control
    void StopAgent();
    void ResetSession();
    bool IsAgentRunning() const { return m_bAgentRunning; }

    // Streaming event handlers (for backend SSE integration)
    void OnStreamEvent(const OUString& eventType, const OUString& eventData);
    void OnStreamComplete();
    void OnStreamError(const OUString& error);
};

} // namespace officelabs

#endif // INCLUDED_OFFICELABS_AGENTICPANEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
