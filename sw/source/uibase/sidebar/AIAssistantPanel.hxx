/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs AI Assistant Panel - Fully Agentic
 *
 * AI assistant with agentic document control:
 * - Quick AI actions (clean, review, summarize, adapt, improve, translate, expand, condense)
 * - @ reference system for document elements
 * - Automatic document editing via AI tools
 * - Document insert/replace operations
 */

#ifndef INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_AIASSISTANTPANEL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_AIASSISTANTPANEL_HXX

#include <sfx2/sidebar/PanelLayout.hxx>
#include <vcl/weld/weld.hxx>
#include <vcl/weld/TextView.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <officelabs/AgentConnection.hxx>
#include <chrono>
#include <vector>

class SfxBindings;
class SwWrtShell;

namespace sw::sidebar {

// Edit history entry for revert system
struct EditHistoryEntry {
    OUString description;
    sal_uInt16 undoCount;  // Number of undo steps for this edit
    bool reverted = false;
    bool accepted = false;
    std::chrono::system_clock::time_point timestamp;
};

class AIAssistantPanel : public PanelLayout
{
private:
    // Status
    std::unique_ptr<weld::Label> m_xStatusLabel;

    // Quick action buttons row 1
    std::unique_ptr<weld::Button> m_xCleanButton;
    std::unique_ptr<weld::Button> m_xReviewButton;
    std::unique_ptr<weld::Button> m_xSummarizeButton;
    std::unique_ptr<weld::Button> m_xAdaptButton;

    // Quick action buttons row 2
    std::unique_ptr<weld::Button> m_xImproveButton;
    std::unique_ptr<weld::Button> m_xTranslateButton;
    std::unique_ptr<weld::Button> m_xExpandButton;
    std::unique_ptr<weld::Button> m_xCondenseButton;

    // Chat area
    std::unique_ptr<weld::TextView> m_xChatHistory;

    // Selection context (Cursor-like UI)
    std::unique_ptr<weld::Box> m_xSelectionContextBox;
    std::unique_ptr<weld::Label> m_xSelectionContextLabel;
    std::unique_ptr<weld::Button> m_xClearSelectionButton;
    OUString m_sCurrentSelection;  // Store current selection text

    // Input row
    std::unique_ptr<weld::Button> m_xAtRefButton;
    std::unique_ptr<weld::Entry> m_xInputField;
    std::unique_ptr<weld::Button> m_xSendButton;

    // Action buttons
    std::unique_ptr<weld::Button> m_xInsertButton;
    std::unique_ptr<weld::Button> m_xReplaceButton;
    std::unique_ptr<weld::Button> m_xResetButton;

    // Revert buttons (Phase 2.5)
    std::unique_ptr<weld::Button> m_xRevertAllButton;
    std::unique_ptr<weld::Button> m_xAcceptAllButton;

    SfxBindings* m_pBindings;
    SwWrtShell* m_pWrtShell;

    // Backend connection
    std::unique_ptr<officelabs::AgentConnection> m_pAgentConnection;

    // State tracking
    bool m_bProcessing;
    OUString m_sLastAIResponse;
    OUString m_sChatLog;

    // Edit history for revert system (Phase 2.5)
    std::vector<EditHistoryEntry> m_aEditHistory;

    // Quick action handlers
    DECL_LINK(CleanClickHdl, weld::Button&, void);
    DECL_LINK(ReviewClickHdl, weld::Button&, void);
    DECL_LINK(SummarizeClickHdl, weld::Button&, void);
    DECL_LINK(AdaptClickHdl, weld::Button&, void);
    DECL_LINK(ImproveClickHdl, weld::Button&, void);
    DECL_LINK(TranslateClickHdl, weld::Button&, void);
    DECL_LINK(ExpandClickHdl, weld::Button&, void);
    DECL_LINK(CondenseClickHdl, weld::Button&, void);

    // Chat handlers
    DECL_LINK(SendClickHdl, weld::Button&, void);
    DECL_LINK(InputActivateHdl, weld::Entry&, bool);

    // Action handlers
    DECL_LINK(InsertClickHdl, weld::Button&, void);
    DECL_LINK(ReplaceClickHdl, weld::Button&, void);
    DECL_LINK(ResetClickHdl, weld::Button&, void);

    // @ Reference button handler
    DECL_LINK(AtRefClickHdl, weld::Button&, void);

    // Revert handlers (Phase 2.5)
    DECL_LINK(RevertAllClickHdl, weld::Button&, void);
    DECL_LINK(AcceptAllClickHdl, weld::Button&, void);

    // Focus handler for selection refresh
    DECL_LINK(InputFocusInHdl, weld::Widget&, void);

    // Clear selection context handler
    DECL_LINK(ClearSelectionClickHdl, weld::Button&, void);

    void SendMessage(const OUString& message);
    void SendQuickAction(const OUString& action, const OUString& customPrompt = u""_ustr);
    void UpdateStatus(const OUString& status);
    void AppendToChat(const OUString& sender, const OUString& message);
    OUString GetSelectedText();
    OUString GetDocumentText();
    OUString GetDocumentContext();
    void ProcessResponse(const officelabs::AgentResponse& response);
    void ExecuteAutoEdits(const std::vector<officelabs::AutoEditCommand>& edits);
    void InsertFormattedText(const OUString& text);
    void InsertAtRefText(const OUString& ref);
    void RefreshSelectionContext();

    // Edit history management (Phase 2.5)
    void RecordEditStart(const OUString& description);
    void RecordEditEnd();
    void RevertLastEdit();
    void RevertAllEdits();
    void AcceptAllEdits();
    void UpdateRevertButtonState();

public:
    static std::unique_ptr<PanelLayout> Create(
        weld::Widget* pParent,
        SfxBindings* pBindings);

    AIAssistantPanel(
        weld::Widget* pParent,
        SfxBindings* pBindings);

    virtual ~AIAssistantPanel() override;
};

} // end of namespace sw::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
