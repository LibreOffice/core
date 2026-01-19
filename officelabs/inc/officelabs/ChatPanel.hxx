/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs AI Assistant - Full Featured Chat Panel
 *
 * FEATURES:
 * =========
 * 1. Multi-Session Chat with Tabs
 *    - Create new chat sessions
 *    - Switch between sessions
 *    - Reset/clear sessions
 *
 * 2. @ Reference System
 *    - @selection - Current selected text
 *    - @document - Full document content
 *    - @table1, @table2 - Reference tables
 *    - @image1, @image2 - Reference images
 *    - @chapter1, @section2 - Reference sections
 *    - @page3 - Reference specific page
 *    - @style:Heading1 - Reference by style
 *
 * 3. Quick AI Actions
 *    - Clean Document - Remove formatting issues, fix spacing
 *    - Review Errors - Grammar, spelling, punctuation
 *    - Summarize - Create summary of selection/document
 *    - Adapt Text - Change tone, formality, length
 *    - Improve - Make text clearer and better
 *    - Translate - Translate to/from languages
 *
 * 4. Direct Formatting
 *    - Bold, Italic, Underline
 *    - Headings (H1-H6)
 *    - Lists (bullet, numbered)
 *    - Alignment
 *
 * 5. Document Operations
 *    - Insert AI response at cursor
 *    - Replace selection with AI response
 *    - Select All, Refresh document connection
 *
 * 6. Pending Edits Workflow
 *    - Preview changes before applying
 *    - Accept/Reject individual edits
 *    - Accept/Reject all edits
 */

#ifndef INCLUDED_OFFICELABS_CHATPANEL_HXX
#define INCLUDED_OFFICELABS_CHATPANEL_HXX

#define VCL_INTERNALS

#include <officelabs/officelabsdllapi.h>
#include <sfx2/dockwin.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/toolkit/fixed.hxx>
#include <vcl/toolkit/edit.hxx>
#include <vcl/toolkit/button.hxx>
#include <vcl/toolkit/tabctrl.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <memory>
#include <vector>
#include <map>
#include <optional>

namespace officelabs {

class AgentConnection;
class DocumentController;

// Message in chat
struct ChatMessage {
    enum class Role { USER, ASSISTANT, SYSTEM, TOOL };
    Role role;
    OUString content;
    OUString timestamp;
    OUString toolName;      // For TOOL messages
    OUString toolResult;    // For TOOL messages
};

// Pending document edit
struct PendingEdit {
    OUString id;
    OUString editType;      // INSERT, REPLACE, DELETE, FORMAT
    OUString targetRef;     // @selection, @table1, etc.
    OUString originalText;
    OUString newText;
    OUString description;
    bool approved = false;
    bool rejected = false;
};

// Chat session
struct ChatSession {
    OUString id;
    OUString name;
    std::vector<ChatMessage> messages;
    std::vector<PendingEdit> pendingEdits;
};

// Document element reference
struct DocElement {
    OUString type;          // table, image, chapter, page, style
    OUString id;            // table1, image2, etc.
    OUString displayName;   // "Table 1: Sales Data"
    sal_Int32 position;     // Position in document
};

class ChatPanel final : public SfxDockingWindow {
private:
    // ========== UI Components ==========

    // Header
    VclPtr<FixedText> m_pStatusLabel;
    VclPtr<FixedText> m_pModelLabel;
    VclPtr<PushButton> m_pSettingsButton;

    // Session Tabs
    VclPtr<TabControl> m_pTabControl;
    VclPtr<PushButton> m_pNewSessionButton;

    // Quick Actions Row 1 - AI Tools
    VclPtr<PushButton> m_pCleanDocButton;
    VclPtr<PushButton> m_pReviewErrorsButton;
    VclPtr<PushButton> m_pSummarizeButton;
    VclPtr<PushButton> m_pAdaptTextButton;

    // Quick Actions Row 2 - More AI Tools
    VclPtr<PushButton> m_pImproveButton;
    VclPtr<PushButton> m_pTranslateButton;
    VclPtr<PushButton> m_pExpandButton;
    VclPtr<PushButton> m_pCondenseButton;

    // Formatting Row
    VclPtr<PushButton> m_pBoldButton;
    VclPtr<PushButton> m_pItalicButton;
    VclPtr<PushButton> m_pUnderlineButton;
    VclPtr<PushButton> m_pHeadingButton;
    VclPtr<PushButton> m_pBulletButton;
    VclPtr<PushButton> m_pNumberButton;

    // Chat History
    VclPtr<Control> m_pChatHistory;

    // Pending Edits Bar
    VclPtr<Control> m_pEditsBar;
    VclPtr<FixedText> m_pEditsLabel;
    VclPtr<PushButton> m_pAcceptAllButton;
    VclPtr<PushButton> m_pRejectAllButton;
    VclPtr<PushButton> m_pReviewButton;

    // Input Area
    VclPtr<PushButton> m_pAtRefButton;      // @ reference menu
    VclPtr<PushButton> m_pAttachButton;     // Attach file
    VclPtr<Edit> m_pInputField;
    VclPtr<PushButton> m_pSendButton;

    // Bottom Row
    VclPtr<PushButton> m_pInsertButton;
    VclPtr<PushButton> m_pReplaceButton;
    VclPtr<PushButton> m_pResetButton;
    VclPtr<PushButton> m_pStopButton;

    // ========== Backend & State ==========
    std::unique_ptr<AgentConnection> m_pAgent;
    std::unique_ptr<DocumentController> m_pDocController;
    css::uno::Reference<css::text::XTextDocument> m_xDocument;

    // Sessions
    std::map<OUString, ChatSession> m_sessions;
    OUString m_currentSessionId;
    sal_Int32 m_nextSessionNum;

    // Document elements cache
    std::vector<DocElement> m_docElements;

    // State
    bool m_connected;
    bool m_processing;
    bool m_includeSelection;
    OUString m_pendingReference;  // Current @ reference

    // ========== Layout ==========
    void DoLayout();
    void InitializeDocument();
    void RefreshDocumentElements();

    // ========== Session Management ==========
    OUString CreateNewSession(const OUString& name = OUString());
    void SwitchToSession(const OUString& sessionId);
    void ResetCurrentSession();
    ChatSession* GetCurrentSession();

    // ========== Event Handlers - Sessions ==========
    DECL_LINK(NewSessionClickHdl, Button*, void);
    DECL_LINK(TabChangedHdl, TabControl*, void);
    DECL_LINK(SettingsClickHdl, Button*, void);

    // ========== Event Handlers - Quick Actions ==========
    DECL_LINK(CleanDocClickHdl, Button*, void);
    DECL_LINK(ReviewErrorsClickHdl, Button*, void);
    DECL_LINK(SummarizeClickHdl, Button*, void);
    DECL_LINK(AdaptTextClickHdl, Button*, void);
    DECL_LINK(ImproveClickHdl, Button*, void);
    DECL_LINK(TranslateClickHdl, Button*, void);
    DECL_LINK(ExpandClickHdl, Button*, void);
    DECL_LINK(CondenseClickHdl, Button*, void);

    // ========== Event Handlers - Formatting ==========
    DECL_LINK(BoldClickHdl, Button*, void);
    DECL_LINK(ItalicClickHdl, Button*, void);
    DECL_LINK(UnderlineClickHdl, Button*, void);
    DECL_LINK(HeadingClickHdl, Button*, void);
    DECL_LINK(BulletClickHdl, Button*, void);
    DECL_LINK(NumberClickHdl, Button*, void);

    // ========== Event Handlers - Chat ==========
    DECL_LINK(SendClickHdl, Button*, void);
    DECL_LINK(AtRefClickHdl, Button*, void);
    DECL_LINK(AttachClickHdl, Button*, void);

    // ========== Event Handlers - Actions ==========
    DECL_LINK(InsertClickHdl, Button*, void);
    DECL_LINK(ReplaceClickHdl, Button*, void);
    DECL_LINK(ResetClickHdl, Button*, void);
    DECL_LINK(StopClickHdl, Button*, void);

    // ========== Event Handlers - Edits ==========
    DECL_LINK(AcceptAllClickHdl, Button*, void);
    DECL_LINK(RejectAllClickHdl, Button*, void);
    DECL_LINK(ReviewClickHdl, Button*, void);

    // ========== Core Operations ==========
    void SendMessage(const OUString& message);
    void SendToAgent(const OUString& message);
    void AddMessage(ChatMessage::Role role, const OUString& content);
    void AddToolMessage(const OUString& toolName, const OUString& result);
    void CheckConnection();

    // ========== AI Quick Actions ==========
    void DoCleanDocument();
    void DoReviewErrors();
    void DoSummarize();
    void DoAdaptText();
    void DoImprove();
    void DoTranslate();
    void DoExpand();
    void DoCondense();

    // ========== @ Reference System ==========
    void ShowAtReferenceMenu();
    OUString GetReferenceContent(const OUString& ref);
    OUString BuildMessageWithReferences(const OUString& message);

    // ========== Document Operations ==========
    void InsertAtCursor(const OUString& text);
    void ReplaceSelection(const OUString& text);
    void ApplyFormatting(const OUString& formatType);
    OUString GetSelectedText();
    OUString GetDocumentText();

    // ========== Pending Edits ==========
    void AddPendingEdit(const PendingEdit& edit);
    void ApproveEdit(const OUString& editId);
    void RejectEdit(const OUString& editId);
    void ApproveAllEdits();
    void RejectAllEdits();
    void ApplyEdit(const PendingEdit& edit);
    void UpdateEditsBar();

    // ========== Rendering ==========
    void RenderMessages();

public:
    ChatPanel(SfxBindings* pBindings, SfxChildWindow* pChildWin, vcl::Window* pParent);
    virtual ~ChatPanel() override;
    virtual void dispose() override;

    // Lifecycle
    virtual void StateChanged(StateChangedType nType) override;
    virtual void DataChanged(const DataChangedEvent& rDCEvt) override;
    virtual void Resize() override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

    // Document
    void SetDocument(const css::uno::Reference<css::text::XTextDocument>& xDoc);
    void RefreshDocument();

    // Status
    void SetProcessing(bool processing);
};

} // namespace officelabs

#endif // INCLUDED_OFFICELABS_CHATPANEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
