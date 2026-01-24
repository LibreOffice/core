/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs AI Assistant Panel - Full Featured Implementation
 *
 * Complete AI assistant with:
 * - Quick AI actions (clean, review, summarize, adapt, improve, translate, expand, condense)
 * - @ reference system for document elements
 * - Direct formatting buttons
 * - Document insert/replace operations
 */

#include "AIAssistantPanel.hxx"
#include <sal/log.hxx>
#include <cstdio>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/svapp.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <poolfmt.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/colritem.hxx>
#include <svl/itemset.hxx>
#include <tools/color.hxx>
#include <tools/fontenum.hxx>
#include <numrule.hxx>
#include <fmtcol.hxx>
#include <i18nutil/searchopt.hxx>
#include <cshtyp.hxx>

namespace sw::sidebar {

std::unique_ptr<PanelLayout> AIAssistantPanel::Create(
    weld::Widget* pParent,
    SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException(
            u"no parent Window given to AIAssistantPanel::Create"_ustr, nullptr, 0);
    if (pBindings == nullptr)
        throw css::lang::IllegalArgumentException(
            u"no SfxBindings given to AIAssistantPanel::Create"_ustr, nullptr, 0);

    return std::make_unique<AIAssistantPanel>(pParent, pBindings);
}

AIAssistantPanel::AIAssistantPanel(
    weld::Widget* pParent,
    SfxBindings* pBindings)
    : PanelLayout(pParent, u"ChatPanel"_ustr, u"modules/swriter/ui/chatpanel.ui"_ustr)
    , m_pBindings(pBindings)
    , m_pWrtShell(nullptr)
    , m_bProcessing(false)
{
    // Get UI components - Status
    m_xStatusLabel = m_xBuilder->weld_label(u"status_label"_ustr);

    // Quick action buttons row 1
    m_xCleanButton = m_xBuilder->weld_button(u"clean_button"_ustr);
    m_xReviewButton = m_xBuilder->weld_button(u"review_button"_ustr);
    m_xSummarizeButton = m_xBuilder->weld_button(u"summarize_button"_ustr);
    m_xAdaptButton = m_xBuilder->weld_button(u"adapt_button"_ustr);

    // Quick action buttons row 2
    m_xImproveButton = m_xBuilder->weld_button(u"improve_button"_ustr);
    m_xTranslateButton = m_xBuilder->weld_button(u"translate_button"_ustr);
    m_xExpandButton = m_xBuilder->weld_button(u"expand_button"_ustr);
    m_xCondenseButton = m_xBuilder->weld_button(u"condense_button"_ustr);

    // Chat area
    m_xChatHistory = m_xBuilder->weld_text_view(u"chat_history"_ustr);

    // Input row
    m_xAtRefButton = m_xBuilder->weld_button(u"at_ref_button"_ustr);
    m_xInputField = m_xBuilder->weld_entry(u"input_field"_ustr);
    m_xSendButton = m_xBuilder->weld_button(u"send_button"_ustr);

    // Action buttons
    m_xInsertButton = m_xBuilder->weld_button(u"insert_button"_ustr);
    m_xReplaceButton = m_xBuilder->weld_button(u"replace_button"_ustr);
    m_xResetButton = m_xBuilder->weld_button(u"reset_button"_ustr);

    // Connect quick action handlers
    if (m_xCleanButton)
        m_xCleanButton->connect_clicked(LINK(this, AIAssistantPanel, CleanClickHdl));
    if (m_xReviewButton)
        m_xReviewButton->connect_clicked(LINK(this, AIAssistantPanel, ReviewClickHdl));
    if (m_xSummarizeButton)
        m_xSummarizeButton->connect_clicked(LINK(this, AIAssistantPanel, SummarizeClickHdl));
    if (m_xAdaptButton)
        m_xAdaptButton->connect_clicked(LINK(this, AIAssistantPanel, AdaptClickHdl));
    if (m_xImproveButton)
        m_xImproveButton->connect_clicked(LINK(this, AIAssistantPanel, ImproveClickHdl));
    if (m_xTranslateButton)
        m_xTranslateButton->connect_clicked(LINK(this, AIAssistantPanel, TranslateClickHdl));
    if (m_xExpandButton)
        m_xExpandButton->connect_clicked(LINK(this, AIAssistantPanel, ExpandClickHdl));
    if (m_xCondenseButton)
        m_xCondenseButton->connect_clicked(LINK(this, AIAssistantPanel, CondenseClickHdl));

    // Connect chat handlers
    if (m_xSendButton)
        m_xSendButton->connect_clicked(LINK(this, AIAssistantPanel, SendClickHdl));
    if (m_xInputField)
        m_xInputField->connect_activate(LINK(this, AIAssistantPanel, InputActivateHdl));

    // Connect action handlers
    if (m_xInsertButton)
        m_xInsertButton->connect_clicked(LINK(this, AIAssistantPanel, InsertClickHdl));
    if (m_xReplaceButton)
        m_xReplaceButton->connect_clicked(LINK(this, AIAssistantPanel, ReplaceClickHdl));
    if (m_xResetButton)
        m_xResetButton->connect_clicked(LINK(this, AIAssistantPanel, ResetClickHdl));

    // Connect @ reference button handler
    if (m_xAtRefButton)
        m_xAtRefButton->connect_clicked(LINK(this, AIAssistantPanel, AtRefClickHdl));

    // Configure chat history
    if (m_xChatHistory)
        m_xChatHistory->set_editable(false);

    // Initialize backend connection
    m_pAgentConnection = std::make_unique<officelabs::AgentConnection>();

    // Get Writer shell for document access
    SwView* pView = GetActiveView();
    if (pView)
    {
        m_pWrtShell = &pView->GetWrtShell();
    }

    // Set initial status based on connection
    if (m_pAgentConnection->isConnected())
    {
        UpdateStatus(u"Connected to AI Backend"_ustr);
    }
    else
    {
        UpdateStatus(u"Backend offline - Start server at localhost:8765"_ustr);
    }
}

AIAssistantPanel::~AIAssistantPanel()
{
}

void AIAssistantPanel::UpdateStatus(const OUString& status)
{
    if (m_xStatusLabel)
        m_xStatusLabel->set_label(status);
}

void AIAssistantPanel::AppendToChat(const OUString& sender, const OUString& message)
{
    if (!m_xChatHistory)
        return;

    OUString currentText = m_xChatHistory->get_text();
    OUString newText = currentText;
    if (!currentText.isEmpty())
        newText += u"\n"_ustr;

    newText += sender + u": "_ustr + message + u"\n"_ustr;
    m_xChatHistory->set_text(newText);
    m_sChatLog = newText;

    // Scroll to bottom
    m_xChatHistory->select_region(-1, -1);
}

OUString AIAssistantPanel::GetSelectedText()
{
    if (!m_pWrtShell)
    {
        SwView* pView = GetActiveView();
        if (pView)
            m_pWrtShell = &pView->GetWrtShell();
    }

    if (!m_pWrtShell)
        return u""_ustr;

    OUString sSelection;
    if (m_pWrtShell->HasSelection())
    {
        m_pWrtShell->GetSelectedText(sSelection, ParaBreakType::ToBlank);
    }
    return sSelection;
}

OUString AIAssistantPanel::GetDocumentText()
{
    // Debug: write to file
    FILE* dbg = fopen("C:\\temp\\ai_debug.log", "a");
    if (dbg) { fprintf(dbg, "GetDocumentText() called\n"); fflush(dbg); }

    // Always get fresh view/shell - don't use cached pointer
    SwView* pView = GetActiveView();
    if (dbg) { fprintf(dbg, "GetActiveView() returned: %s\n", pView ? "valid" : "nullptr"); fflush(dbg); }

    if (!pView)
    {
        if (dbg) { fprintf(dbg, "No active view - returning empty\n"); fclose(dbg); }
        return u""_ustr;
    }

    SwWrtShell& rSh = pView->GetWrtShell();
    if (dbg) { fprintf(dbg, "Got fresh SwWrtShell reference\n"); fflush(dbg); }

    // Lock view and save cursor position (pattern from view2.cxx)
    rSh.LockView(true);
    rSh.Push();

    // Select all using the pattern from LibreOffice source
    rSh.SelAll();
    if (dbg) { fprintf(dbg, "After SelAll()\n"); fflush(dbg); }

    rSh.ExtendedSelectAll();
    if (dbg) { fprintf(dbg, "After ExtendedSelectAll()\n"); fflush(dbg); }

    OUString sDocText;
    rSh.GetSelectedText(sDocText, ParaBreakType::ToBlank);

    if (dbg) { fprintf(dbg, "GetSelectedText returned: %d chars\n", (int)sDocText.getLength()); fflush(dbg); }

    // Restore cursor position and unlock view
    rSh.Pop(SwCursorShell::PopMode::DeleteCurrent);
    rSh.LockView(false);

    if (dbg) { fprintf(dbg, "Final result: %d chars\n", (int)sDocText.getLength()); fclose(dbg); }

    // Limit to 10000 chars
    if (sDocText.getLength() > 10000)
        sDocText = sDocText.copy(0, 10000) + u"... [truncated]"_ustr;

    return sDocText;
}

OUString AIAssistantPanel::GetDocumentContext()
{
    OUString sSelection = GetSelectedText();

    OUStringBuffer context;
    if (!sSelection.isEmpty())
    {
        context.append(u"Selected text: "_ustr);
        if (sSelection.getLength() > 500)
            context.append(sSelection.subView(0, 500) + u"..."_ustr);
        else
            context.append(sSelection);
    }
    else
    {
        context.append(u"(No selection)"_ustr);
    }

    return context.makeStringAndClear();
}

void AIAssistantPanel::ProcessResponse(const officelabs::AgentResponse& response)
{
    m_bProcessing = false;

    // Debug: log response details
    FILE* dbg = fopen("C:\\temp\\process_response_debug.log", "a");
    if (dbg) {
        fprintf(dbg, "ProcessResponse called\n");
        fprintf(dbg, "  message length: %d\n", (int)response.message.getLength());
        fprintf(dbg, "  autoEdits count: %d\n", (int)response.autoEdits.size());
        fprintf(dbg, "  hasPatch: %s\n", response.hasPatch ? "true" : "false");
        fclose(dbg);
    }

    // Re-enable buttons
    if (m_xSendButton)
        m_xSendButton->set_sensitive(true);

    // Display response
    if (!response.message.isEmpty())
    {
        AppendToChat(u"AI"_ustr, response.message);
        m_sLastAIResponse = response.message;
        UpdateStatus(u"Ready"_ustr);
    }
    else
    {
        AppendToChat(u"System"_ustr, u"No response from AI"_ustr);
        UpdateStatus(u"Error occurred"_ustr);
    }

    // Execute any automatic edits from the AI
    if (!response.autoEdits.empty())
    {
        ExecuteAutoEdits(response.autoEdits);
    }
    else
    {
        FILE* dbg2 = fopen("C:\\temp\\process_response_debug.log", "a");
        if (dbg2) {
            fprintf(dbg2, "  autoEdits is EMPTY - not calling ExecuteAutoEdits\n");
            fclose(dbg2);
        }
    }
}

void AIAssistantPanel::ExecuteAutoEdits(const std::vector<officelabs::AutoEditCommand>& edits)
{
    // Get fresh shell reference
    SwView* pView = GetActiveView();
    if (!pView)
    {
        AppendToChat(u"System"_ustr, u"Cannot edit: No active document"_ustr);
        return;
    }

    SwWrtShell& rSh = pView->GetWrtShell();
    int editCount = 0;

    rSh.StartAllAction();

    for (const auto& cmd : edits)
    {
        FILE* dbg = fopen("C:\\temp\\auto_edit_debug.log", "a");
        if (dbg) {
            OString action = cmd.action.toUtf8();
            OString findText = cmd.findText.toUtf8();
            OString newText = cmd.newText.toUtf8();
            OString position = cmd.position.toUtf8();
            fprintf(dbg, "AutoEdit: action=%s find='%s' new='%s' pos=%s\n",
                    action.getStr(), findText.getStr(), newText.getStr(), position.getStr());
            fclose(dbg);
        }

        if (cmd.action == u"clear_and_write"_ustr)
        {
            // Clear entire document and write new content
            rSh.LockView(true);
            rSh.Push();
            rSh.SelAll();
            rSh.ExtendedSelectAll();
            rSh.DelRight();
            rSh.Pop(SwCursorShell::PopMode::DeleteCurrent);
            rSh.LockView(false);

            // Insert new text
            InsertFormattedText(cmd.newText);
            editCount++;
        }
        else if (cmd.action == u"insert"_ustr)
        {
            if (cmd.position == u"start"_ustr)
            {
                rSh.StartOfSection();
            }
            else if (cmd.position == u"end"_ustr)
            {
                rSh.EndOfSection();
            }
            // Default "cursor" - just insert at current position

            InsertFormattedText(cmd.newText);
            editCount++;
        }
        else if (cmd.action == u"replace"_ustr)
        {
            if (!cmd.findText.isEmpty())
            {
                // Use find & replace via SearchPattern
                i18nutil::SearchOptions2 aSearchOpt;
                aSearchOpt.searchString = cmd.findText;
                aSearchOpt.replaceString = cmd.newText;
                aSearchOpt.Locale = css::lang::Locale();
                aSearchOpt.AlgorithmType2 = css::util::SearchAlgorithms2::ABSOLUTE;

                // SearchPattern returns count of replacements made
                sal_Int32 nFound = rSh.SearchPattern(aSearchOpt,
                    false,  // bSearchInNotes
                    SwDocPositions::Start,
                    SwDocPositions::End,
                    FindRanges::InBody,
                    true);  // bReplace

                if (nFound > 0 && nFound != SAL_MAX_INT32)
                    editCount++;
            }
            else if (rSh.HasSelection())
            {
                // Replace current selection
                rSh.DelRight();
                InsertFormattedText(cmd.newText);
                editCount++;
            }
        }
        else if (cmd.action == u"delete"_ustr)
        {
            if (!cmd.findText.isEmpty())
            {
                // Find and delete text (replace with empty string)
                i18nutil::SearchOptions2 aSearchOpt;
                aSearchOpt.searchString = cmd.findText;
                aSearchOpt.replaceString = u""_ustr;
                aSearchOpt.Locale = css::lang::Locale();
                aSearchOpt.AlgorithmType2 = css::util::SearchAlgorithms2::ABSOLUTE;

                sal_Int32 nFound = rSh.SearchPattern(aSearchOpt,
                    false,
                    SwDocPositions::Start,
                    SwDocPositions::End,
                    FindRanges::InBody,
                    true);

                if (nFound > 0 && nFound != SAL_MAX_INT32)
                    editCount++;
            }
        }
        else if (cmd.action == u"format"_ustr)
        {
            // Find text and apply formatting
            if (!cmd.findText.isEmpty())
            {
                // First search to select the text
                i18nutil::SearchOptions2 aSearchOpt;
                aSearchOpt.searchString = cmd.findText;
                aSearchOpt.Locale = css::lang::Locale();
                aSearchOpt.AlgorithmType2 = css::util::SearchAlgorithms2::ABSOLUTE;

                sal_Int32 nFound = rSh.SearchPattern(aSearchOpt,
                    false,
                    SwDocPositions::Start,
                    SwDocPositions::End,
                    FindRanges::InBody,
                    false);  // Don't replace, just find

                if (nFound > 0 && nFound != SAL_MAX_INT32)
                {
                    // Text found and selected, apply formatting
                    if (cmd.bold)
                    {
                        rSh.SetAttrItem(SvxWeightItem(WEIGHT_BOLD, RES_CHRATR_WEIGHT));
                    }
                    if (cmd.italic)
                    {
                        rSh.SetAttrItem(SvxPostureItem(ITALIC_NORMAL, RES_CHRATR_POSTURE));
                    }
                    if (cmd.underline)
                    {
                        rSh.SetAttrItem(SvxUnderlineItem(LINESTYLE_SINGLE, RES_CHRATR_UNDERLINE));
                    }
                    if (cmd.headingLevel > 0 && cmd.headingLevel <= 6)
                    {
                        sal_uInt16 nPoolId = RES_POOLCOLL_HEADLINE1 + (cmd.headingLevel - 1);
                        rSh.SetTextFormatColl(rSh.GetDoc()->getIDocumentStylePoolAccess().GetTextCollFromPool(nPoolId));
                    }
                    if (!cmd.fontColor.isEmpty())
                    {
                        // Parse hex color like "#FF0000" or "FF0000"
                        OUString colorStr = cmd.fontColor;
                        if (colorStr.startsWith("#"))
                            colorStr = colorStr.copy(1);
                        if (colorStr.getLength() == 6)
                        {
                            sal_uInt32 nColor = colorStr.toUInt32(16);
                            Color aColor(
                                static_cast<sal_uInt8>((nColor >> 16) & 0xFF),  // R
                                static_cast<sal_uInt8>((nColor >> 8) & 0xFF),   // G
                                static_cast<sal_uInt8>(nColor & 0xFF)           // B
                            );
                            rSh.SetAttrItem(SvxColorItem(aColor, RES_CHRATR_COLOR));
                        }
                    }
                    editCount++;
                }
            }
        }
    }

    rSh.EndAllAction();

    if (editCount > 0)
    {
        OUString msg = OUString::number(editCount) + u" edit(s) applied to document"_ustr;
        AppendToChat(u"System"_ustr, msg);
        UpdateStatus(msg);
    }
}

void AIAssistantPanel::SendMessage(const OUString& message)
{
    if (message.isEmpty())
        return;

    if (m_bProcessing)
    {
        UpdateStatus(u"Please wait for current request..."_ustr);
        return;
    }

    // Add user message to chat
    AppendToChat(u"You"_ustr, message);

    // Clear input immediately
    if (m_xInputField)
        m_xInputField->set_text(u""_ustr);

    // Check connection
    if (!m_pAgentConnection->isConnected())
    {
        if (!m_pAgentConnection->checkConnection())
        {
            AppendToChat(u"System"_ustr, u"Error: Backend not available at localhost:8765"_ustr);
            UpdateStatus(u"Backend offline"_ustr);
            return;
        }
    }

    // Update UI for processing state
    m_bProcessing = true;
    UpdateStatus(u"Thinking..."_ustr);
    if (m_xSendButton)
        m_xSendButton->set_sensitive(false);

    // Get document content and selection for the AI
    FILE* dbg = fopen("C:/temp/sendmsg_debug.log", "a");
    if (dbg) { fprintf(dbg, "SendMessage: About to call GetDocumentText()\n"); fflush(dbg); }

    OUString sDocumentContent = GetDocumentText();
    OUString sSelection = GetSelectedText();

    if (dbg) {
        fprintf(dbg, "SendMessage: GetDocumentText returned %d chars\n", (int)sDocumentContent.getLength());
        fprintf(dbg, "SendMessage: GetSelectedText returned %d chars\n", (int)sSelection.getLength());
        fclose(dbg);
    }

    // Make HTTP call to backend with document content and selection
    officelabs::AgentResponse response = m_pAgentConnection->sendMessage(message, sDocumentContent, sSelection);
    ProcessResponse(response);
}

void AIAssistantPanel::SendQuickAction(const OUString& action, const OUString& customPrompt)
{
    OUString selection = GetSelectedText();
    OUString textToProcess = selection.isEmpty() ? GetDocumentText() : selection;

    if (textToProcess.isEmpty())
    {
        AppendToChat(u"System"_ustr, u"No text to process. Select text or add content to document."_ustr);
        return;
    }

    OUString prompt;
    if (!customPrompt.isEmpty())
    {
        prompt = customPrompt;
    }
    else if (action == u"clean"_ustr)
    {
        prompt = u"Clean up this text. Fix spacing, punctuation, and formatting issues. Return only the cleaned text:\n\n"_ustr + textToProcess;
    }
    else if (action == u"review"_ustr)
    {
        prompt = u"Review this text for grammar, spelling, and punctuation errors. List each error found and suggest corrections:\n\n"_ustr + textToProcess;
    }
    else if (action == u"summarize"_ustr)
    {
        prompt = u"Summarize the following text in a concise manner. Keep the key points:\n\n"_ustr + textToProcess;
    }
    else if (action == u"adapt"_ustr)
    {
        prompt = u"Adapt this text to be more professional and formal while keeping the same meaning:\n\n"_ustr + textToProcess;
    }
    else if (action == u"improve"_ustr)
    {
        prompt = u"Improve this text to be clearer, more engaging, and better written. Return the improved version:\n\n"_ustr + textToProcess;
    }
    else if (action == u"translate"_ustr)
    {
        prompt = u"Translate this text to English (if not English) or to the user's language. Return only the translation:\n\n"_ustr + textToProcess;
    }
    else if (action == u"expand"_ustr)
    {
        prompt = u"Expand this text with more details, examples, and explanations while maintaining the original tone:\n\n"_ustr + textToProcess;
    }
    else if (action == u"condense"_ustr)
    {
        prompt = u"Condense this text to be shorter and more concise while keeping the essential meaning:\n\n"_ustr + textToProcess;
    }
    else
    {
        prompt = action + u":\n\n"_ustr + textToProcess;
    }

    SendMessage(prompt);
}

void AIAssistantPanel::InsertAtRefText(const OUString& ref)
{
    if (!m_xInputField)
        return;

    OUString current = m_xInputField->get_text();
    m_xInputField->set_text(current + ref + u" "_ustr);
    m_xInputField->grab_focus();
}

// ==================== QUICK ACTION HANDLERS ====================

IMPL_LINK_NOARG(AIAssistantPanel, CleanClickHdl, weld::Button&, void)
{
    SendQuickAction(u"clean"_ustr);
}

IMPL_LINK_NOARG(AIAssistantPanel, ReviewClickHdl, weld::Button&, void)
{
    SendQuickAction(u"review"_ustr);
}

IMPL_LINK_NOARG(AIAssistantPanel, SummarizeClickHdl, weld::Button&, void)
{
    SendQuickAction(u"summarize"_ustr);
}

IMPL_LINK_NOARG(AIAssistantPanel, AdaptClickHdl, weld::Button&, void)
{
    SendQuickAction(u"adapt"_ustr);
}

IMPL_LINK_NOARG(AIAssistantPanel, ImproveClickHdl, weld::Button&, void)
{
    SendQuickAction(u"improve"_ustr);
}

IMPL_LINK_NOARG(AIAssistantPanel, TranslateClickHdl, weld::Button&, void)
{
    SendQuickAction(u"translate"_ustr);
}

IMPL_LINK_NOARG(AIAssistantPanel, ExpandClickHdl, weld::Button&, void)
{
    SendQuickAction(u"expand"_ustr);
}

IMPL_LINK_NOARG(AIAssistantPanel, CondenseClickHdl, weld::Button&, void)
{
    SendQuickAction(u"condense"_ustr);
}

// ==================== CHAT HANDLERS ====================

IMPL_LINK_NOARG(AIAssistantPanel, SendClickHdl, weld::Button&, void)
{
    if (m_xInputField)
    {
        OUString message = m_xInputField->get_text();
        if (!message.isEmpty())
            SendMessage(message);
    }
}

IMPL_LINK_NOARG(AIAssistantPanel, InputActivateHdl, weld::Entry&, bool)
{
    // Enter key pressed
    if (m_xSendButton)
        SendClickHdl(*m_xSendButton);
    return true;
}

// ==================== ACTION HANDLERS ====================

void AIAssistantPanel::InsertFormattedText(const OUString& text)
{
    if (!m_pWrtShell)
    {
        SwView* pView = GetActiveView();
        if (pView)
            m_pWrtShell = &pView->GetWrtShell();
    }

    if (!m_pWrtShell)
        return;

    // Parse markdown-like formatting and insert with Writer styles
    sal_Int32 nIndex = 0;
    bool bFirstParagraph = true;

    while (nIndex < text.getLength())
    {
        // Find end of line
        sal_Int32 nLineEnd = text.indexOf('\n', nIndex);
        if (nLineEnd < 0)
            nLineEnd = text.getLength();

        OUString sLine = text.copy(nIndex, nLineEnd - nIndex).trim();
        nIndex = nLineEnd + 1;

        // Skip empty lines but add paragraph break
        if (sLine.isEmpty())
        {
            if (!bFirstParagraph)
                m_pWrtShell->SplitNode();
            continue;
        }

        // Add paragraph break between paragraphs
        if (!bFirstParagraph)
            m_pWrtShell->SplitNode();
        bFirstParagraph = false;

        // Check for heading (## or #)
        if (sLine.startsWith(u"## "))
        {
            OUString sHeadingText = sLine.copy(3);
            m_pWrtShell->SetTextFormatColl(m_pWrtShell->GetDoc()->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_HEADLINE2));
            m_pWrtShell->Insert(sHeadingText);
        }
        else if (sLine.startsWith(u"# "))
        {
            OUString sHeadingText = sLine.copy(2);
            m_pWrtShell->SetTextFormatColl(m_pWrtShell->GetDoc()->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_HEADLINE1));
            m_pWrtShell->Insert(sHeadingText);
        }
        else if (sLine.startsWith(u"- ") || sLine.startsWith(u"* "))
        {
            OUString sBulletText = sLine.copy(2);
            m_pWrtShell->SetTextFormatColl(m_pWrtShell->GetDoc()->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD));
            m_pWrtShell->Insert(u"\u2022 "_ustr + sBulletText);
        }
        else if (sLine.startsWith(u"**") && sLine.endsWith(u"**") && sLine.getLength() > 4)
        {
            OUString sBoldText = sLine.copy(2, sLine.getLength() - 4);
            m_pWrtShell->SetTextFormatColl(m_pWrtShell->GetDoc()->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD));
            SfxItemSet aSet(m_pWrtShell->GetAttrPool(), svl::Items<RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT>);
            aSet.Put(SvxWeightItem(WEIGHT_BOLD, RES_CHRATR_WEIGHT));
            m_pWrtShell->Insert(sBoldText);
        }
        else
        {
            m_pWrtShell->SetTextFormatColl(m_pWrtShell->GetDoc()->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD));
            OUString sCleanLine = sLine.replaceAll(u"\\n", u" ");
            m_pWrtShell->Insert(sCleanLine);
        }
    }
}

IMPL_LINK_NOARG(AIAssistantPanel, InsertClickHdl, weld::Button&, void)
{
    if (m_sLastAIResponse.isEmpty())
    {
        UpdateStatus(u"No AI response to insert"_ustr);
        return;
    }

    if (!m_pWrtShell)
    {
        SwView* pView = GetActiveView();
        if (pView)
            m_pWrtShell = &pView->GetWrtShell();
    }

    if (!m_pWrtShell)
    {
        UpdateStatus(u"No document available"_ustr);
        return;
    }

    // Insert formatted text at cursor position
    m_pWrtShell->StartAllAction();
    InsertFormattedText(m_sLastAIResponse);
    m_pWrtShell->EndAllAction();

    AppendToChat(u"System"_ustr, u"Text inserted into document"_ustr);
    UpdateStatus(u"Text inserted"_ustr);
}

IMPL_LINK_NOARG(AIAssistantPanel, ReplaceClickHdl, weld::Button&, void)
{
    if (m_sLastAIResponse.isEmpty())
    {
        UpdateStatus(u"No AI response to replace with"_ustr);
        return;
    }

    if (!m_pWrtShell)
    {
        SwView* pView = GetActiveView();
        if (pView)
            m_pWrtShell = &pView->GetWrtShell();
    }

    if (!m_pWrtShell)
    {
        UpdateStatus(u"No document available"_ustr);
        return;
    }

    if (!m_pWrtShell->HasSelection())
    {
        UpdateStatus(u"Please select text to replace"_ustr);
        return;
    }

    // Delete selection and insert AI response
    m_pWrtShell->StartAllAction();
    m_pWrtShell->DelRight();
    InsertFormattedText(m_sLastAIResponse);
    m_pWrtShell->EndAllAction();

    AppendToChat(u"System"_ustr, u"Selection replaced with AI response"_ustr);
    UpdateStatus(u"Text replaced"_ustr);
}

IMPL_LINK_NOARG(AIAssistantPanel, ResetClickHdl, weld::Button&, void)
{
    // Clear chat history
    if (m_xChatHistory)
        m_xChatHistory->set_text(u""_ustr);

    m_sChatLog = u""_ustr;
    m_sLastAIResponse = u""_ustr;

    // Clear input
    if (m_xInputField)
        m_xInputField->set_text(u""_ustr);

    UpdateStatus(u"Chat reset - Ready"_ustr);
}

// ==================== @ REFERENCE HANDLER ====================

IMPL_LINK_NOARG(AIAssistantPanel, AtRefClickHdl, weld::Button&, void)
{
    // Insert @ symbol and show help text
    InsertAtRefText(u"@"_ustr);
    // Show quick help in status
    UpdateStatus(u"Type: selection, document, table1, image1, section1"_ustr);
}

} // end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
