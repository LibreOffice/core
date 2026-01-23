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

class SfxBindings;
class SwWrtShell;

namespace sw::sidebar {

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

    // Input row
    std::unique_ptr<weld::Button> m_xAtRefButton;
    std::unique_ptr<weld::Entry> m_xInputField;
    std::unique_ptr<weld::Button> m_xSendButton;

    // Action buttons
    std::unique_ptr<weld::Button> m_xInsertButton;
    std::unique_ptr<weld::Button> m_xReplaceButton;
    std::unique_ptr<weld::Button> m_xResetButton;

    SfxBindings* m_pBindings;
    SwWrtShell* m_pWrtShell;

    // Backend connection
    std::unique_ptr<officelabs::AgentConnection> m_pAgentConnection;

    // State tracking
    bool m_bProcessing;
    OUString m_sLastAIResponse;
    OUString m_sChatLog;

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
