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
#include <algorithm>
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
#include <editeng/fhgtitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <svl/itemset.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/sfxsids.hrc>
#include <tools/color.hxx>
#include <tools/fontenum.hxx>
#include <numrule.hxx>
#include <fmtcol.hxx>
#include <i18nutil/searchopt.hxx>
#include <i18nutil/transliteration.hxx>
#include <cshtyp.hxx>
#include <tblsel.hxx>
#include <itabenum.hxx>

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
    , m_bAwaitingClarification(false)
    , m_nCurrentQuestionIndex(0)
    , m_aResponseChecker("AIAssistantPanel AsyncResponseChecker")
    , m_bAsyncRequestPending(false)
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

    // Selection context UI (Cursor-like)
    m_xSelectionContextBox = m_xBuilder->weld_box(u"selection_context_box"_ustr);
    m_xSelectionContextLabel = m_xBuilder->weld_label(u"selection_context_label"_ustr);
    m_xClearSelectionButton = m_xBuilder->weld_button(u"clear_selection_button"_ustr);

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
    {
        m_xInputField->connect_activate(LINK(this, AIAssistantPanel, InputActivateHdl));
        m_xInputField->connect_focus_in(LINK(this, AIAssistantPanel, InputFocusInHdl));
    }

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

    // Connect clear selection button handler
    if (m_xClearSelectionButton)
        m_xClearSelectionButton->connect_clicked(LINK(this, AIAssistantPanel, ClearSelectionClickHdl));

    // Revert buttons (Phase 2.5)
    m_xRevertAllButton = m_xBuilder->weld_button(u"revert_all_button"_ustr);
    m_xAcceptAllButton = m_xBuilder->weld_button(u"accept_all_button"_ustr);

    if (m_xRevertAllButton)
        m_xRevertAllButton->connect_clicked(LINK(this, AIAssistantPanel, RevertAllClickHdl));
    if (m_xAcceptAllButton)
        m_xAcceptAllButton->connect_clicked(LINK(this, AIAssistantPanel, AcceptAllClickHdl));

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

    // Check for selection to enable inline edit mode (Cursor-like feature)
    if (m_pWrtShell && m_pWrtShell->HasSelection())
    {
        OUString sSelection;
        m_pWrtShell->GetSelectedText(sSelection, ParaBreakType::ToBlank);
        if (!sSelection.isEmpty())
        {
            // Store the full selection
            m_sCurrentSelection = sSelection;

            // Truncate selection for display if too long
            OUString sDisplayText = sSelection;
            if (sDisplayText.getLength() > 50)
                sDisplayText = sDisplayText.copy(0, 50) + u"..."_ustr;

            // Show selection context in the badge UI
            if (m_xSelectionContextLabel)
                m_xSelectionContextLabel->set_label(u"\""_ustr + sDisplayText + u"\""_ustr);
            if (m_xSelectionContextBox)
                m_xSelectionContextBox->set_visible(true);

            UpdateStatus(u"Selection ready - Type your instruction"_ustr);

            // Focus the input field for quick typing
            if (m_xInputField)
                m_xInputField->grab_focus();
        }
    }

    // Set up async response checker (Idle handler for non-blocking HTTP)
    m_aResponseChecker.SetInvokeHandler(LINK(this, AIAssistantPanel, CheckAsyncResponseHdl));
    m_aResponseChecker.SetPriority(TaskPriority::DEFAULT_IDLE);
}

AIAssistantPanel::~AIAssistantPanel()
{
    // Stop async response checker if running
    m_aResponseChecker.Stop();
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

void AIAssistantPanel::AppendUserMessage(const OUString& message)
{
    if (!m_xChatHistory)
        return;

    OUString currentText = m_xChatHistory->get_text();
    OUString newText = currentText;
    if (!currentText.isEmpty())
        newText += u"\n"_ustr;

    // Modern user message bubble (Cursor/Copilot style)
    newText += u"┌─────────────────────────────────┐\n"_ustr;
    newText += u"│ 👤 You                          │\n"_ustr;
    newText += u"├─────────────────────────────────┤\n"_ustr;

    // Word wrap the message with indentation
    OUString wrappedMsg = message;
    // Remove excess whitespace
    wrappedMsg = wrappedMsg.trim();

    // Add message with padding
    newText += u"│ "_ustr + wrappedMsg + u"\n"_ustr;
    newText += u"└─────────────────────────────────┘\n"_ustr;

    m_xChatHistory->set_text(newText);
    m_sChatLog = newText;
    m_xChatHistory->select_region(-1, -1);
}

OUString AIAssistantPanel::CleanAIResponse(const OUString& response)
{
    OUString cleaned = response;

    // Remove JSON code blocks
    sal_Int32 jsonStart = cleaned.indexOf(u"```json");
    while (jsonStart >= 0)
    {
        sal_Int32 jsonEnd = cleaned.indexOf(u"```", jsonStart + 7);
        if (jsonEnd > jsonStart)
        {
            cleaned = cleaned.copy(0, jsonStart) + cleaned.copy(jsonEnd + 3);
        }
        else
        {
            break;
        }
        jsonStart = cleaned.indexOf(u"```json");
    }

    // Remove any remaining code blocks
    jsonStart = cleaned.indexOf(u"```");
    while (jsonStart >= 0)
    {
        sal_Int32 jsonEnd = cleaned.indexOf(u"```", jsonStart + 3);
        if (jsonEnd > jsonStart)
        {
            cleaned = cleaned.copy(0, jsonStart) + cleaned.copy(jsonEnd + 3);
        }
        else
        {
            break;
        }
        jsonStart = cleaned.indexOf(u"```");
    }

    // Remove raw JSON objects/arrays that might be in the response
    // Look for {"auto_edits": or {"action":
    sal_Int32 braceStart = cleaned.indexOf(u"{\"auto_edits\"");
    if (braceStart < 0)
        braceStart = cleaned.indexOf(u"{\"action\"");
    if (braceStart >= 0)
    {
        // Find matching closing brace
        int depth = 0;
        sal_Int32 braceEnd = braceStart;
        for (sal_Int32 i = braceStart; i < cleaned.getLength(); ++i)
        {
            if (cleaned[i] == '{') depth++;
            else if (cleaned[i] == '}') {
                depth--;
                if (depth == 0) {
                    braceEnd = i;
                    break;
                }
            }
        }
        if (braceEnd > braceStart)
        {
            cleaned = cleaned.copy(0, braceStart) + cleaned.copy(braceEnd + 1);
        }
    }

    // Trim whitespace
    cleaned = cleaned.trim();

    // If empty after cleaning, return empty (we'll show action summary instead)
    return cleaned;
}

OUString AIAssistantPanel::FormatActionSummary(const std::vector<officelabs::AutoEditCommand>& edits)
{
    if (edits.empty())
        return u""_ustr;

    OUStringBuffer summary;
    int displayCount = 0;
    const int maxDisplayed = 5;  // Claude for Excel style: show first 5, then collapse

    for (const auto& edit : edits)
    {
        OUString icon;
        OUString actionDesc;
        OUString location;  // Location reference like Claude for Excel

        // Build location reference from findText (like "Para: Introduction")
        if (!edit.findText.isEmpty())
        {
            OUString locText = edit.findText;
            if (locText.getLength() > 20)
                locText = locText.copy(0, 20) + u"..."_ustr;
            location = u"\"" + locText + u"\""_ustr;
        }
        else if (!edit.newText.isEmpty())
        {
            OUString locText = edit.newText;
            if (locText.getLength() > 20)
                locText = locText.copy(0, 20) + u"..."_ustr;
            location = u"\"" + locText + u"\""_ustr;
        }

        if (edit.action == u"format"_ustr || edit.action == u"apply_style"_ustr)
        {
            icon = u"🎨"_ustr;
            OUStringBuffer formatTypes;
            if (!edit.styleName.isEmpty())
            {
                formatTypes.append(edit.styleName);
            }
            else
            {
                if (edit.bold) formatTypes.append(u"Bold "_ustr);
                if (edit.italic) formatTypes.append(u"Italic "_ustr);
                if (edit.underline) formatTypes.append(u"Underline "_ustr);
                if (edit.headingLevel > 0)
                {
                    formatTypes.append(u"Heading "_ustr);
                    formatTypes.append(OUString::number(edit.headingLevel));
                }
            }
            actionDesc = formatTypes.makeStringAndClear().trim();
            if (actionDesc.isEmpty())
                actionDesc = u"Styled"_ustr;
        }
        else if (edit.action == u"insert"_ustr)
        {
            icon = u"➕"_ustr;
            actionDesc = u"Inserted"_ustr;
        }
        else if (edit.action == u"replace"_ustr || edit.action == u"replace_all"_ustr)
        {
            icon = u"✏️"_ustr;
            actionDesc = u"Replaced"_ustr;
        }
        else if (edit.action == u"delete"_ustr)
        {
            icon = u"🗑️"_ustr;
            actionDesc = u"Removed"_ustr;
        }
        else if (edit.action == u"create_list"_ustr)
        {
            icon = u"📝"_ustr;
            actionDesc = edit.listType + u" list"_ustr;
        }
        else if (edit.action == u"create_table"_ustr)
        {
            icon = u"📊"_ustr;
            actionDesc = OUString::number(edit.tableRows) + u"×"_ustr + OUString::number(edit.tableColumns) + u" table"_ustr;
        }
        else if (edit.action == u"paragraph_format"_ustr)
        {
            icon = u"¶"_ustr;
            OUStringBuffer paraFormat;
            if (!edit.alignment.isEmpty()) paraFormat.append(edit.alignment + u" "_ustr);
            if (edit.lineSpacing > 0) paraFormat.append(OUString::number(edit.lineSpacing) + u"× spacing"_ustr);
            actionDesc = paraFormat.makeStringAndClear().trim();
            if (actionDesc.isEmpty()) actionDesc = u"Formatted"_ustr;
        }
        else if (edit.action == u"clear_and_write"_ustr)
        {
            icon = u"📄"_ustr;
            actionDesc = u"Document replaced"_ustr;
        }
        else if (edit.action == u"insert_page_break"_ustr)
        {
            icon = u"📃"_ustr;
            actionDesc = u"Page break"_ustr;
        }
        else
        {
            icon = u"⚡"_ustr;
            actionDesc = edit.action;
        }

        displayCount++;
        if (displayCount <= maxDisplayed)
        {
            if (!summary.isEmpty())
                summary.append(u"\n"_ustr);
            // Format like Claude for Excel: icon action → location
            if (!location.isEmpty())
                summary.append(u"  "_ustr + icon + u" "_ustr + actionDesc + u" → "_ustr + location);
            else
                summary.append(u"  "_ustr + icon + u" "_ustr + actionDesc);
        }
    }

    // Add collapse indicator if more items
    if (displayCount > maxDisplayed)
    {
        int remaining = displayCount - maxDisplayed;
        summary.append(u"\n  ⋯ and "_ustr + OUString::number(remaining) + u" more actions"_ustr);
    }

    return summary.makeStringAndClear();
}

void AIAssistantPanel::AppendAIMessage(const OUString& message, const std::vector<officelabs::AutoEditCommand>& edits)
{
    if (!m_xChatHistory)
        return;

    OUString currentText = m_xChatHistory->get_text();
    OUString newText = currentText;
    if (!currentText.isEmpty())
        newText += u"\n"_ustr;

    // Claude for Excel style: Compact header with tool call count
    if (!edits.empty())
    {
        // Show tool call count like Claude for Excel: "▶ 12 tool calls completed"
        newText += u"┌─────────────────────────────────────┐\n"_ustr;
        newText += u"│ 🤖 AI Copilot • "_ustr + OUString::number(static_cast<int>(edits.size())) + u" actions completed  │\n"_ustr;
        newText += u"├─────────────────────────────────────┤\n"_ustr;
    }
    else
    {
        newText += u"┌─────────────────────────────────────┐\n"_ustr;
        newText += u"│ 🤖 AI Copilot                       │\n"_ustr;
        newText += u"├─────────────────────────────────────┤\n"_ustr;
    }

    // Clean and add message (remove JSON)
    OUString cleanedMsg = CleanAIResponse(message);
    if (!cleanedMsg.isEmpty())
    {
        // Split message into lines and add box padding
        sal_Int32 nIndex = 0;
        while (nIndex >= 0 && nIndex < cleanedMsg.getLength())
        {
            sal_Int32 nLineEnd = cleanedMsg.indexOf('\n', nIndex);
            OUString sLine;
            if (nLineEnd >= 0)
            {
                sLine = cleanedMsg.copy(nIndex, nLineEnd - nIndex);
                nIndex = nLineEnd + 1;
            }
            else
            {
                sLine = cleanedMsg.copy(nIndex);
                nIndex = -1;
            }
            newText += u"│ "_ustr + sLine + u"\n"_ustr;
        }
    }
    else if (!edits.empty())
    {
        newText += u"│ ✅ Done! Document updated.\n"_ustr;
    }

    // Add collapsible action summary if there are edits
    if (!edits.empty())
    {
        newText += u"├─────────────────────────────────────┤\n"_ustr;
        newText += u"│ ▼ Actions:\n"_ustr;  // ▼ indicates expanded, ▶ would be collapsed
        OUString actionSummary = FormatActionSummary(edits);
        // Add box padding to action summary
        sal_Int32 nIdx = 0;
        while (nIdx >= 0 && nIdx < actionSummary.getLength())
        {
            sal_Int32 nEnd = actionSummary.indexOf('\n', nIdx);
            OUString sLine;
            if (nEnd >= 0)
            {
                sLine = actionSummary.copy(nIdx, nEnd - nIdx);
                nIdx = nEnd + 1;
            }
            else
            {
                sLine = actionSummary.copy(nIdx);
                nIdx = -1;
            }
            newText += u"│"_ustr + sLine + u"\n"_ustr;
        }
    }

    newText += u"└─────────────────────────────────────┘\n"_ustr;

    m_xChatHistory->set_text(newText);
    m_sChatLog = newText;
    m_xChatHistory->select_region(-1, -1);
}

void AIAssistantPanel::AppendSystemMessage(const OUString& message, bool isSuccess)
{
    if (!m_xChatHistory)
        return;

    OUString currentText = m_xChatHistory->get_text();
    OUString newText = currentText;
    if (!currentText.isEmpty())
        newText += u"\n"_ustr;

    // System message with visual indicator
    if (isSuccess)
    {
        newText += u"  ✅ "_ustr + message + u"\n"_ustr;
    }
    else
    {
        newText += u"  ⚠️ "_ustr + message + u"\n"_ustr;
    }

    m_xChatHistory->set_text(newText);
    m_sChatLog = newText;
    m_xChatHistory->select_region(-1, -1);
}

void AIAssistantPanel::AppendClarificationHeader()
{
    if (!m_xChatHistory)
        return;

    OUString currentText = m_xChatHistory->get_text();
    OUString newText = currentText;
    if (!currentText.isEmpty())
        newText += u"\n"_ustr;

    // Modern clarification card (thinking mode)
    newText += u"╔═══════════════════════════════════╗\n"_ustr;
    newText += u"║ 💭 AI Thinking...                 ║\n"_ustr;
    newText += u"╠═══════════════════════════════════╣\n"_ustr;
    newText += u"║ To create the best content, I need║\n"_ustr;
    newText += u"║ to understand your requirements.  ║\n"_ustr;
    newText += u"║                                   ║\n"_ustr;
    newText += u"║ 📋 Please answer these questions: ║\n"_ustr;
    newText += u"╚═══════════════════════════════════╝\n"_ustr;

    m_xChatHistory->set_text(newText);
    m_sChatLog = newText;
    m_xChatHistory->select_region(-1, -1);
}

void AIAssistantPanel::DisplayClarificationQuestions()
{
    if (!m_xChatHistory || m_aPendingQuestions.empty())
        return;

    OUString currentText = m_xChatHistory->get_text();
    OUString newText = currentText;

    int questionNum = 1;
    for (const auto& question : m_aPendingQuestions)
    {
        // Question card with number
        newText += u"\n┌───────────────────────────────────\n"_ustr;
        newText += u"│ Q"_ustr + OUString::number(questionNum) + u": "_ustr + question.question + u"\n"_ustr;
        newText += u"├───────────────────────────────────\n"_ustr;

        // Display options if available
        if (!question.options.empty())
        {
            int optionNum = 1;
            for (const auto& option : question.options)
            {
                // Check if this is a default option
                bool isDefault = (!question.defaultValue.isEmpty() && option == question.defaultValue);
                OUString marker = isDefault ? u"◉"_ustr : u"○"_ustr;
                newText += u"│   "_ustr + marker + u" ["_ustr + OUString::number(optionNum) + u"] "_ustr + option;
                if (isDefault)
                    newText += u" ⭐"_ustr;
                newText += u"\n"_ustr;
                optionNum++;
            }
        }
        else
        {
            // Free text input expected
            newText += u"│   ✍️ (Type your answer)\n"_ustr;
        }
        newText += u"└───────────────────────────────────\n"_ustr;

        questionNum++;
    }

    // Instructions box
    newText += u"\n💡 How to answer:\n"_ustr;
    newText += u"   • Type option numbers: 1, 2, 1, Custom text\n"_ustr;
    newText += u"   • Or: 1:2, 2:Professional, 3:your answer\n"_ustr;
    newText += u"   • Press Enter when done\n"_ustr;

    m_xChatHistory->set_text(newText);
    m_sChatLog = newText;
    m_xChatHistory->select_region(-1, -1);

    // Set placeholder in input field
    if (m_xInputField)
    {
        m_xInputField->set_placeholder_text(u"Type your answers (e.g., 1, 2, 1, my focus)..."_ustr);
        m_xInputField->grab_focus();
    }
}

void AIAssistantPanel::ProcessClarificationAnswers(const OUString& input)
{
    if (!m_bAwaitingClarification || m_aPendingQuestions.empty())
        return;

    // Parse the user input to extract answers
    // Supported formats:
    // - "1:answer1, 2:answer2, 3:answer3"
    // - "answer1, answer2, answer3" (positional)
    // - "1, Professional, 2" (option numbers or text)

    std::vector<OUString> answers;

    // Split by comma
    sal_Int32 nIndex = 0;
    while (nIndex >= 0)
    {
        sal_Int32 nCommaPos = input.indexOf(',', nIndex);
        OUString part;
        if (nCommaPos >= 0)
        {
            part = input.copy(nIndex, nCommaPos - nIndex).trim();
            nIndex = nCommaPos + 1;
        }
        else
        {
            part = input.copy(nIndex).trim();
            nIndex = -1;
        }

        if (!part.isEmpty())
        {
            // Check if it's in "question#:answer" format
            sal_Int32 colonPos = part.indexOf(':');
            if (colonPos > 0)
            {
                OUString questionNumStr = part.copy(0, colonPos).trim();
                OUString answerStr = part.copy(colonPos + 1).trim();

                // Get question number (1-based)
                sal_Int32 questionNum = questionNumStr.toInt32();
                if (questionNum > 0 && questionNum <= static_cast<sal_Int32>(m_aPendingQuestions.size()))
                {
                    const auto& question = m_aPendingQuestions[questionNum - 1];

                    // Check if answer is an option number
                    sal_Int32 optionNum = answerStr.toInt32();
                    if (optionNum > 0 && optionNum <= static_cast<sal_Int32>(question.options.size()))
                    {
                        m_aClarificationAnswers[question.id] = question.options[optionNum - 1];
                    }
                    else
                    {
                        // It's a text answer
                        m_aClarificationAnswers[question.id] = answerStr;
                    }
                }
            }
            else
            {
                // Positional answer - assign to next unanswered question
                if (answers.size() < m_aPendingQuestions.size())
                {
                    answers.push_back(part);
                }
            }
        }
    }

    // Process positional answers
    for (size_t i = 0; i < answers.size() && i < m_aPendingQuestions.size(); ++i)
    {
        const auto& question = m_aPendingQuestions[i];
        if (m_aClarificationAnswers.find(question.id) == m_aClarificationAnswers.end())
        {
            OUString answer = answers[i];

            // Check if answer is an option number
            sal_Int32 optionNum = answer.toInt32();
            if (optionNum > 0 && optionNum <= static_cast<sal_Int32>(question.options.size()))
            {
                m_aClarificationAnswers[question.id] = question.options[optionNum - 1];
            }
            else
            {
                m_aClarificationAnswers[question.id] = answer;
            }
        }
    }

    // Fill in defaults for unanswered questions
    for (const auto& question : m_aPendingQuestions)
    {
        if (m_aClarificationAnswers.find(question.id) == m_aClarificationAnswers.end())
        {
            if (!question.defaultValue.isEmpty())
            {
                m_aClarificationAnswers[question.id] = question.defaultValue;
            }
            else if (!question.options.empty())
            {
                m_aClarificationAnswers[question.id] = question.options[0];
            }
        }
    }

    // Show what we understood
    AppendUserMessage(input);

    OUStringBuffer summary;
    summary.append(u"◀ AI: Got it! Here's what I understood:\n"_ustr);
    for (const auto& question : m_aPendingQuestions)
    {
        auto it = m_aClarificationAnswers.find(question.id);
        if (it != m_aClarificationAnswers.end())
        {
            summary.append(u"   • "_ustr + question.id + u": "_ustr + it->second + u"\n"_ustr);
        }
    }
    summary.append(u"\n   Now generating your content...\n"_ustr);

    if (m_xChatHistory)
    {
        OUString currentText = m_xChatHistory->get_text();
        m_xChatHistory->set_text(currentText + u"\n"_ustr + summary.makeStringAndClear());
        m_xChatHistory->select_region(-1, -1);
    }

    // Clear clarification state
    m_bAwaitingClarification = false;

    // Re-send the original message with clarification answers
    SendMessageWithClarification();
}

void AIAssistantPanel::SendMessageWithClarification()
{
    if (m_sPendingMessage.isEmpty())
        return;

    // Show processing state
    m_bProcessing = true;
    UpdateStatus(u"AI is generating content..."_ustr);
    if (m_xSendButton)
        m_xSendButton->set_sensitive(false);

    AppendTypingIndicator();

    // Get document content and selection
    OUString sDocumentContent = GetDocumentText();
    OUString sSelection = GetSelectedText();

    // Send message with clarification answers
    officelabs::AgentResponse response = m_pAgentConnection->sendMessageWithClarification(
        m_sPendingMessage, sDocumentContent, sSelection, m_aClarificationAnswers);

    // Clear pending state
    m_sPendingMessage.clear();
    m_aPendingQuestions.clear();
    m_aClarificationAnswers.clear();

    // Remove typing indicator and process response
    RemoveTypingIndicator();
    ProcessResponse(response);
}

void AIAssistantPanel::AppendTypingIndicator()
{
    if (!m_xChatHistory)
        return;

    OUString currentText = m_xChatHistory->get_text();
    OUString newText = currentText;
    if (!currentText.isEmpty())
        newText += u"\n"_ustr;

    // Modern typing indicator (Copilot style)
    newText += u"╔═══════════════════════════════════╗\n"_ustr;
    newText += u"║ 🤖 AI is working...               ║\n"_ustr;
    newText += u"║     ⏳ Processing your request... ║\n"_ustr;
    newText += u"╚═══════════════════════════════════╝\n"_ustr;

    m_xChatHistory->set_text(newText);
    m_xChatHistory->select_region(-1, -1);
}

void AIAssistantPanel::RemoveTypingIndicator()
{
    if (!m_xChatHistory)
        return;

    OUString text = m_xChatHistory->get_text();

    // Find and remove the typing indicator card (new format)
    sal_Int32 indicatorPos = text.indexOf(u"🤖 AI is working...");
    if (indicatorPos >= 0)
    {
        // Find the start of the card (look backwards for ╔)
        sal_Int32 cardStart = indicatorPos;
        while (cardStart > 0 && text[cardStart] != u'╔')
            cardStart--;

        // Find the end of the card (look for ╚...╝)
        sal_Int32 cardEnd = text.indexOf(u"╝", indicatorPos);
        if (cardEnd >= 0)
        {
            // Find newline after the card
            sal_Int32 lineEnd = text.indexOf(u"\n", cardEnd);
            if (lineEnd >= 0)
            {
                text = text.copy(0, cardStart) + text.copy(lineEnd + 1);
            }
            else
            {
                text = text.copy(0, cardStart);
            }
        }
    }

    // Also handle old format for backward compatibility
    sal_Int32 oldIndicatorPos = text.indexOf(u"◀ AI is typing...");
    if (oldIndicatorPos >= 0)
    {
        sal_Int32 dotsEnd = text.indexOf(u"● ○ ○", oldIndicatorPos);
        if (dotsEnd >= 0)
        {
            sal_Int32 lineEnd = text.indexOf(u"\n", dotsEnd);
            if (lineEnd >= 0)
            {
                text = text.copy(0, oldIndicatorPos) + text.copy(lineEnd + 1);
            }
            else
            {
                text = text.copy(0, oldIndicatorPos);
            }
        }
    }

    // Clean up any extra newlines at the end
    while (text.endsWith(u"\n\n"))
    {
        text = text.copy(0, text.getLength() - 1);
    }

    m_xChatHistory->set_text(text);
    m_sChatLog = text;
}

void AIAssistantPanel::HighlightChangedText(const OUString& text, bool isAddition)
{
    // This would highlight text in the document after AI edits
    // For now, we use character background color to show changes
    // Green for additions, yellow for modifications

    if (!m_pWrtShell)
    {
        SwView* pView = GetActiveView();
        if (pView)
            m_pWrtShell = &pView->GetWrtShell();
    }

    if (!m_pWrtShell || text.isEmpty())
        return;

    // Search for the text and apply temporary highlight
    // (This is a placeholder - full implementation would use find & mark)
    (void)isAddition;  // Suppress unused parameter warning for now
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

    // Re-enable buttons
    if (m_xSendButton)
        m_xSendButton->set_sensitive(true);

    // Check if AI needs clarification before proceeding
    if (response.needsClarification && !response.clarificationQuestions.empty())
    {
        // Store the pending state for clarification flow
        m_bAwaitingClarification = true;
        m_aPendingQuestions = response.clarificationQuestions;
        m_aClarificationAnswers.clear();
        m_nCurrentQuestionIndex = 0;

        // Display clarification header
        AppendClarificationHeader();

        // Display all questions with options
        DisplayClarificationQuestions();

        UpdateStatus(u"Please answer the questions above"_ustr);
        return;
    }

    // Display response with clean formatting
    if (!response.message.isEmpty() || !response.autoEdits.empty())
    {
        // Use new formatted AI message display
        AppendAIMessage(response.message, response.autoEdits);
        m_sLastAIResponse = response.message;
        UpdateStatus(u"Ready"_ustr);
    }
    else
    {
        AppendSystemMessage(u"No response from AI"_ustr, false);
        UpdateStatus(u"Error occurred"_ustr);
    }

    // Execute any automatic edits from the AI
    if (!response.autoEdits.empty())
    {
        ExecuteAutoEdits(response.autoEdits);
    }
}

void AIAssistantPanel::ExecuteAutoEdits(const std::vector<officelabs::AutoEditCommand>& edits)
{
    FILE* execDbg = fopen("C:\\temp\\execute_debug.log", "a");
    if (execDbg) { fprintf(execDbg, "ExecuteAutoEdits: starting with %d edits\n", (int)edits.size()); fflush(execDbg); }

    // Get fresh shell reference
    SwView* pView = GetActiveView();
    if (!pView)
    {
        if (execDbg) { fprintf(execDbg, "  ERROR: No active view!\n"); fclose(execDbg); }
        AppendToChat(u"System"_ustr, u"Cannot edit: No active document"_ustr);
        return;
    }

    if (execDbg) { fprintf(execDbg, "  Got active view\n"); fflush(execDbg); }

    SwWrtShell& rSh = pView->GetWrtShell();
    int editCount = 0;

    if (execDbg) { fprintf(execDbg, "  Got SwWrtShell, calling StartAllAction\n"); fflush(execDbg); }

    // Record edit start for revert system (Phase 2.5)
    OUString sEditDescription = OUString::number(edits.size()) + u" AI edit(s)"_ustr;
    RecordEditStart(sEditDescription);

    rSh.StartAllAction();
    if (execDbg) { fprintf(execDbg, "  StartAllAction done\n"); fflush(execDbg); }

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
            if (execDbg) { fprintf(execDbg, "  INSERT action starting\n"); fflush(execDbg); }

            if (cmd.position == u"start"_ustr)
            {
                if (execDbg) { fprintf(execDbg, "    Moving to start\n"); fflush(execDbg); }
                rSh.StartOfSection();
            }
            else if (cmd.position == u"end"_ustr)
            {
                if (execDbg) { fprintf(execDbg, "    Moving to end\n"); fflush(execDbg); }
                rSh.EndOfSection();
            }
            // Default "cursor" - just insert at current position

            // Apply heading style BEFORE inserting text (paragraph style)
            if (cmd.headingLevel > 0 && cmd.headingLevel <= 6)
            {
                if (execDbg) { fprintf(execDbg, "    Applying heading level %d\n", cmd.headingLevel); fflush(execDbg); }
                sal_uInt16 nPoolId = RES_POOLCOLL_HEADLINE1 + (cmd.headingLevel - 1);
                SwDoc* pDoc = rSh.GetDoc();
                if (pDoc)
                {
                    SwTextFormatColl* pColl = pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(nPoolId);
                    if (pColl)
                        rSh.SetTextFormatColl(pColl);
                }
            }

            // Remember start position for formatting
            if (execDbg) { fprintf(execDbg, "    Getting cursor position\n"); fflush(execDbg); }
            sal_Int32 nStartPos = rSh.GetCursor()->GetPoint()->GetContentIndex();

            // Insert the text
            if (execDbg) { OString txt = cmd.newText.toUtf8(); fprintf(execDbg, "    Inserting text: '%s'\n", txt.getStr()); fflush(execDbg); }
            rSh.Insert(cmd.newText);
            if (execDbg) { fprintf(execDbg, "    Insert done\n"); fflush(execDbg); }

            // Select the inserted text to apply character formatting
            sal_Int32 nEndPos = rSh.GetCursor()->GetPoint()->GetContentIndex();
            if (nEndPos > nStartPos && (cmd.bold || cmd.italic || cmd.underline || !cmd.fontColor.isEmpty() || cmd.fontSize > 0))
            {
                // Select the text we just inserted
                rSh.Left(SwCursorSkipMode::Chars, true, nEndPos - nStartPos, false);

                // Apply character formatting
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
                if (!cmd.fontColor.isEmpty())
                {
                    OUString colorStr = cmd.fontColor;
                    if (colorStr.startsWith("#"))
                        colorStr = colorStr.copy(1);
                    if (colorStr.getLength() == 6)
                    {
                        sal_uInt32 nColor = colorStr.toUInt32(16);
                        Color aColor(
                            static_cast<sal_uInt8>((nColor >> 16) & 0xFF),
                            static_cast<sal_uInt8>((nColor >> 8) & 0xFF),
                            static_cast<sal_uInt8>(nColor & 0xFF)
                        );
                        rSh.SetAttrItem(SvxColorItem(aColor, RES_CHRATR_COLOR));
                    }
                }
                if (cmd.fontSize > 0)
                {
                    sal_uInt32 nTwips = static_cast<sal_uInt32>(cmd.fontSize * 20);
                    SvxFontHeightItem aFontHeight(nTwips, 100, RES_CHRATR_FONTSIZE);
                    rSh.SetAttrItem(aFontHeight);
                }
                // New formatting features - disabled for debugging in insert action
                /*
                if (cmd.strikethrough)
                {
                    rSh.SetAttrItem(SvxCrossedOutItem(STRIKEOUT_SINGLE, RES_CHRATR_CROSSEDOUT));
                }
                if (cmd.superscript)
                {
                    rSh.SetAttrItem(SvxEscapementItem(SvxEscapement::Superscript, RES_CHRATR_ESCAPEMENT));
                }
                if (cmd.subscript)
                {
                    rSh.SetAttrItem(SvxEscapementItem(SvxEscapement::Subscript, RES_CHRATR_ESCAPEMENT));
                }
                if (!cmd.highlightColor.isEmpty())
                {
                    OUString colorStr = cmd.highlightColor;
                    if (colorStr.startsWith("#"))
                        colorStr = colorStr.copy(1);
                    if (colorStr.getLength() == 6)
                    {
                        sal_uInt32 nColor = colorStr.toUInt32(16);
                        Color aColor(
                            static_cast<sal_uInt8>((nColor >> 16) & 0xFF),
                            static_cast<sal_uInt8>((nColor >> 8) & 0xFF),
                            static_cast<sal_uInt8>(nColor & 0xFF)
                        );
                        rSh.SetAttrItem(SvxBrushItem(aColor, RES_CHRATR_BACKGROUND));
                    }
                }
                if (!cmd.fontName.isEmpty())
                {
                    SvxFontItem aFont(FAMILY_DONTKNOW, cmd.fontName, OUString(), PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, RES_CHRATR_FONT);
                    rSh.SetAttrItem(aFont);
                }
                */

                // Deselect (move cursor to end of inserted text)
                rSh.Right(SwCursorSkipMode::Chars, false, 0, false);
            }

            // If as_paragraph is true, add a paragraph break after the inserted text
            if (cmd.asParagraph)
            {
                if (execDbg) { fprintf(execDbg, "    SplitNode for paragraph\n"); fflush(execDbg); }
                rSh.SplitNode();
            }
            if (execDbg) { fprintf(execDbg, "  INSERT action complete\n"); fflush(execDbg); }
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
                    FILE* fmtDbg = fopen("C:\\temp\\format_debug.log", "a");
                    if (fmtDbg) { fprintf(fmtDbg, "Text found, applying formatting\n"); fflush(fmtDbg); }

                    // Text found and selected, apply formatting
                    if (cmd.bold)
                    {
                        if (fmtDbg) { fprintf(fmtDbg, "  Applying bold\n"); fflush(fmtDbg); }
                        rSh.SetAttrItem(SvxWeightItem(WEIGHT_BOLD, RES_CHRATR_WEIGHT));
                    }
                    if (cmd.italic)
                    {
                        if (fmtDbg) { fprintf(fmtDbg, "  Applying italic\n"); fflush(fmtDbg); }
                        rSh.SetAttrItem(SvxPostureItem(ITALIC_NORMAL, RES_CHRATR_POSTURE));
                    }
                    if (cmd.underline)
                    {
                        if (fmtDbg) { fprintf(fmtDbg, "  Applying underline\n"); fflush(fmtDbg); }
                        rSh.SetAttrItem(SvxUnderlineItem(LINESTYLE_SINGLE, RES_CHRATR_UNDERLINE));
                    }
                    if (cmd.headingLevel > 0 && cmd.headingLevel <= 6)
                    {
                        if (fmtDbg) { fprintf(fmtDbg, "  Applying heading %d\n", cmd.headingLevel); fflush(fmtDbg); }
                        sal_uInt16 nPoolId = RES_POOLCOLL_HEADLINE1 + (cmd.headingLevel - 1);
                        SwDoc* pDoc = rSh.GetDoc();
                        if (pDoc)
                        {
                            SwTextFormatColl* pColl = pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(nPoolId);
                            if (pColl)
                                rSh.SetTextFormatColl(pColl);
                        }
                    }
                    if (!cmd.fontColor.isEmpty())
                    {
                        if (fmtDbg) { OString cs = cmd.fontColor.toUtf8(); fprintf(fmtDbg, "  Applying color %s\n", cs.getStr()); fflush(fmtDbg); }
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
                    if (cmd.fontSize > 0)
                    {
                        if (fmtDbg) { fprintf(fmtDbg, "  Applying fontSize %.1f\n", cmd.fontSize); fflush(fmtDbg); }
                        // Font size is in points, convert to twips (1 point = 20 twips)
                        sal_uInt32 nTwips = static_cast<sal_uInt32>(cmd.fontSize * 20);
                        SvxFontHeightItem aFontHeight(nTwips, 100, RES_CHRATR_FONTSIZE);
                        rSh.SetAttrItem(aFontHeight);
                    }
                    // New formatting features - disabled for debugging
                    /*
                    if (cmd.strikethrough)
                    {
                        if (fmtDbg) { fprintf(fmtDbg, "  Applying strikethrough\n"); fflush(fmtDbg); }
                        rSh.SetAttrItem(SvxCrossedOutItem(STRIKEOUT_SINGLE, RES_CHRATR_CROSSEDOUT));
                    }
                    if (cmd.superscript)
                    {
                        if (fmtDbg) { fprintf(fmtDbg, "  Applying superscript\n"); fflush(fmtDbg); }
                        rSh.SetAttrItem(SvxEscapementItem(SvxEscapement::Superscript, RES_CHRATR_ESCAPEMENT));
                    }
                    if (cmd.subscript)
                    {
                        if (fmtDbg) { fprintf(fmtDbg, "  Applying subscript\n"); fflush(fmtDbg); }
                        rSh.SetAttrItem(SvxEscapementItem(SvxEscapement::Subscript, RES_CHRATR_ESCAPEMENT));
                    }
                    if (!cmd.highlightColor.isEmpty())
                    {
                        if (fmtDbg) { fprintf(fmtDbg, "  Applying highlight\n"); fflush(fmtDbg); }
                        OUString colorStr = cmd.highlightColor;
                        if (colorStr.startsWith("#"))
                            colorStr = colorStr.copy(1);
                        if (colorStr.getLength() == 6)
                        {
                            sal_uInt32 nColor = colorStr.toUInt32(16);
                            Color aColor(
                                static_cast<sal_uInt8>((nColor >> 16) & 0xFF),
                                static_cast<sal_uInt8>((nColor >> 8) & 0xFF),
                                static_cast<sal_uInt8>(nColor & 0xFF)
                            );
                            rSh.SetAttrItem(SvxBrushItem(aColor, RES_CHRATR_BACKGROUND));
                        }
                    }
                    if (!cmd.fontName.isEmpty())
                    {
                        if (fmtDbg) { fprintf(fmtDbg, "  Applying fontName\n"); fflush(fmtDbg); }
                        SvxFontItem aFont(FAMILY_DONTKNOW, cmd.fontName, OUString(), PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, RES_CHRATR_FONT);
                        rSh.SetAttrItem(aFont);
                    }
                    */
                    if (fmtDbg) { fprintf(fmtDbg, "  Format complete\n"); fclose(fmtDbg); }
                    editCount++;
                }
            }
        }
        else if (cmd.action == u"paragraph_format"_ustr)
        {
            // Find text and apply paragraph formatting
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
                    // Apply paragraph alignment
                    if (!cmd.alignment.isEmpty())
                    {
                        SvxAdjust eAdjust = SvxAdjust::Left;
                        if (cmd.alignment == u"center"_ustr)
                            eAdjust = SvxAdjust::Center;
                        else if (cmd.alignment == u"right"_ustr)
                            eAdjust = SvxAdjust::Right;
                        else if (cmd.alignment == u"justify"_ustr)
                            eAdjust = SvxAdjust::Block;
                        rSh.SetAttrItem(SvxAdjustItem(eAdjust, RES_PARATR_ADJUST));
                    }
                    // Apply line spacing
                    if (cmd.lineSpacing > 0)
                    {
                        SvxLineSpacingItem aSpacing(0, RES_PARATR_LINESPACING);
                        if (cmd.lineSpacing == 1.0)
                            aSpacing.SetLineSpaceRule(SvxLineSpaceRule::Auto);
                        else if (cmd.lineSpacing == 1.5)
                        {
                            aSpacing.SetPropLineSpace(150);
                        }
                        else if (cmd.lineSpacing == 2.0)
                        {
                            aSpacing.SetPropLineSpace(200);
                        }
                        else
                        {
                            aSpacing.SetPropLineSpace(static_cast<sal_uInt16>(cmd.lineSpacing * 100));
                        }
                        rSh.SetAttrItem(aSpacing);
                    }
                    // Apply space before/after paragraph (in twips: 1 pt = 20 twips)
                    if (cmd.spaceBefore > 0 || cmd.spaceAfter > 0)
                    {
                        SvxULSpaceItem aULSpace(RES_UL_SPACE);
                        if (cmd.spaceBefore > 0)
                            aULSpace.SetUpper(static_cast<sal_uInt16>(cmd.spaceBefore * 20));
                        if (cmd.spaceAfter > 0)
                            aULSpace.SetLower(static_cast<sal_uInt16>(cmd.spaceAfter * 20));
                        rSh.SetAttrItem(aULSpace);
                    }
                    // Apply indent settings (in twips: 1 cm = 567 twips)
                    if (cmd.indentLeft > 0 || cmd.indentRight > 0 || cmd.indentFirstLine > 0)
                    {
                        SvxLRSpaceItem aLRSpace(RES_LR_SPACE);
                        if (cmd.indentLeft > 0)
                            aLRSpace.SetLeft(SvxIndentValue::twips(cmd.indentLeft * 567));
                        if (cmd.indentRight > 0)
                            aLRSpace.SetRight(SvxIndentValue::twips(cmd.indentRight * 567));
                        if (cmd.indentFirstLine > 0)
                            aLRSpace.SetTextFirstLineOffset(SvxIndentValue::twips(cmd.indentFirstLine * 567));
                        rSh.SetAttrItem(aLRSpace);
                    }
                    editCount++;
                }
            }
        }
        else if (cmd.action == u"create_list"_ustr)
        {
            FILE* listDbg = fopen("C:\\temp\\list_debug.log", "a");
            if (listDbg) { fprintf(listDbg, "create_list: starting, position=%s\n", cmd.position.toUtf8().getStr()); fflush(listDbg); }

            // Move to position if specified
            if (cmd.position == u"start"_ustr)
            {
                if (listDbg) { fprintf(listDbg, "  Moving to start\n"); fflush(listDbg); }
                rSh.StartOfSection();
            }
            else if (cmd.position == u"end"_ustr)
            {
                if (listDbg) { fprintf(listDbg, "  Moving to end\n"); fflush(listDbg); }
                rSh.EndOfSection();
                // Add paragraph break before list if there's existing content
                if (listDbg) { fprintf(listDbg, "  SplitNode\n"); fflush(listDbg); }
                rSh.SplitNode();
                // Only turn off bullet/numbering if we're actually in a list
                if (rSh.GetNumRuleAtCurrCursorPos())
                {
                    if (listDbg) { fprintf(listDbg, "  NumOrBulletOff (in list context)\n"); fflush(listDbg); }
                    rSh.NumOrBulletOff();
                }
            }

            // Create list items
            if (!cmd.listItems.empty())
            {
                if (listDbg) { fprintf(listDbg, "  Inserting %d items\n", (int)cmd.listItems.size()); fflush(listDbg); }

                // Insert first item text
                if (listDbg) { fprintf(listDbg, "  Insert first item\n"); fflush(listDbg); }
                rSh.Insert(cmd.listItems[0]);

                // Apply bullet or numbering to make this a list
                if (cmd.listType == u"numbered"_ustr)
                {
                    if (listDbg) { fprintf(listDbg, "  NumOn() for numbered list\n"); fflush(listDbg); }
                    rSh.NumOn();  // Apply numbering format
                }
                else
                {
                    // Default to bullet list
                    if (listDbg) { fprintf(listDbg, "  BulletOn() for bullet list\n"); fflush(listDbg); }
                    rSh.BulletOn();  // Apply bullet format
                }

                // Insert remaining items
                for (size_t i = 1; i < cmd.listItems.size(); ++i)
                {
                    if (listDbg) { fprintf(listDbg, "  SplitNode + insert item %d\n", (int)i); fflush(listDbg); }
                    rSh.SplitNode();  // New paragraph continues the list
                    rSh.Insert(cmd.listItems[i]);
                }
                if (listDbg) { fprintf(listDbg, "  List complete\n"); fclose(listDbg); }
            }
            else
            {
                if (listDbg) { fprintf(listDbg, "  No list items!\n"); fclose(listDbg); }
            }
            editCount++;
        }
        else if (cmd.action == u"create_table"_ustr)
        {
            FILE* tblDbg2 = fopen("C:\\temp\\table_debug.log", "a");
            if (tblDbg2) { fprintf(tblDbg2, "create_table: starting, position=%s\n", cmd.position.toUtf8().getStr()); fflush(tblDbg2); }

            // Move to position if specified
            if (cmd.position == u"start"_ustr)
            {
                if (tblDbg2) { fprintf(tblDbg2, "  Moving to start\n"); fflush(tblDbg2); }
                rSh.StartOfSection();
            }
            else if (cmd.position == u"end"_ustr)
            {
                if (tblDbg2) { fprintf(tblDbg2, "  Moving to end\n"); fflush(tblDbg2); }
                rSh.EndOfSection();
                // Add paragraph break before table if there's existing content
                if (tblDbg2) { fprintf(tblDbg2, "  SplitNode\n"); fflush(tblDbg2); }
                rSh.SplitNode();
                // Only turn off bullet/numbering if we're actually in a list
                if (rSh.GetNumRuleAtCurrCursorPos())
                {
                    if (tblDbg2) { fprintf(tblDbg2, "  NumOrBulletOff (in list context)\n"); fflush(tblDbg2); }
                    rSh.NumOrBulletOff();
                }
            }
            if (tblDbg2) { fclose(tblDbg2); tblDbg2 = nullptr; }

            // Create a table - validate dimensions
            if (cmd.tableRows > 0 && cmd.tableRows <= 100 && cmd.tableColumns > 0 && cmd.tableColumns <= 50)
            {
                FILE* tblDbg = fopen("C:\\temp\\table_debug.log", "a");
                if (tblDbg) { fprintf(tblDbg, "Creating table %dx%d\n", cmd.tableRows, cmd.tableColumns); fflush(tblDbg); }

                // InsertTable creates a table with specified rows and columns
                rSh.InsertTable(SwInsertTableOptions(SwInsertTableFlags::DefaultBorder, 0),
                    cmd.tableRows, cmd.tableColumns);

                if (tblDbg) { fprintf(tblDbg, "InsertTable done, IsCursorInTable=%d\n", rSh.IsCursorInTable() ? 1 : 0); fflush(tblDbg); }

                // Navigate to start of table (first cell) - InsertTable may leave cursor after table
                bool bMoved = rSh.MoveTable(GotoPrevTable, fnTableStart);

                if (tblDbg) { fprintf(tblDbg, "MoveTable returned %d, IsCursorInTable=%d\n", bMoved ? 1 : 0, rSh.IsCursorInTable() ? 1 : 0); fflush(tblDbg); }

                // Insert table data if provided
                if (!cmd.tableData.empty())
                {
                    if (tblDbg) { fprintf(tblDbg, "Inserting %d rows of data\n", (int)cmd.tableData.size()); fflush(tblDbg); }

                    // Fill in the data row by row, cell by cell
                    for (size_t row = 0; row < cmd.tableData.size() && row < static_cast<size_t>(cmd.tableRows); ++row)
                    {
                        const auto& rowData = cmd.tableData[row];
                        for (size_t col = 0; col < rowData.size() && col < static_cast<size_t>(cmd.tableColumns); ++col)
                        {
                            // Insert the cell content
                            rSh.Insert(rowData[col]);

                            // Move to next cell (unless this is the last cell)
                            if (col < static_cast<size_t>(cmd.tableColumns) - 1 ||
                                row < cmd.tableData.size() - 1)
                            {
                                rSh.GoNextCell(false);  // false = don't create new row if at end
                            }
                        }
                        // If row has fewer columns than table, skip remaining cells in this row
                        for (size_t col = rowData.size(); col < static_cast<size_t>(cmd.tableColumns); ++col)
                        {
                            if (col < static_cast<size_t>(cmd.tableColumns) - 1 ||
                                row < cmd.tableData.size() - 1)
                            {
                                rSh.GoNextCell(false);
                            }
                        }
                    }
                    if (tblDbg) { fprintf(tblDbg, "Data insertion complete\n"); fclose(tblDbg); tblDbg = nullptr; }
                }
                if (tblDbg) fclose(tblDbg);
                editCount++;
            }
        }
        else if (cmd.action == u"replace_all"_ustr)
        {
            // Find and replace all occurrences
            if (!cmd.findText.isEmpty())
            {
                FILE* replDbg = fopen("C:\\temp\\replace_debug.log", "a");
                if (replDbg) { fprintf(replDbg, "replace_all: find='%s' replace='%s'\n", cmd.findText.toUtf8().getStr(), cmd.newText.toUtf8().getStr()); fflush(replDbg); }

                i18nutil::SearchOptions2 aSearchOpt;
                aSearchOpt.searchString = cmd.findText;
                aSearchOpt.replaceString = cmd.newText;
                aSearchOpt.Locale = css::lang::Locale();
                aSearchOpt.AlgorithmType2 = css::util::SearchAlgorithms2::ABSOLUTE;

                // Set case sensitivity and whole words
                if (!cmd.caseSensitive)
                    aSearchOpt.transliterateFlags = TransliterationFlags::IGNORE_CASE;
                if (cmd.wholeWords)
                    aSearchOpt.searchFlag |= css::util::SearchFlags::NORM_WORD_ONLY;

                // Replace all occurrences - use ReplaceAll pattern
                sal_Int32 nFound = rSh.SearchPattern(aSearchOpt,
                    false,  // bSearchInNotes
                    SwDocPositions::Start,
                    SwDocPositions::End,
                    FindRanges::InBody,
                    true);  // bReplace

                if (replDbg) { fprintf(replDbg, "  First SearchPattern returned: %d\n", (int)nFound); fflush(replDbg); }

                // Keep replacing until no more found - with safety limit
                int safetyCount = 0;
                const int MAX_REPLACEMENTS = 1000;
                while (nFound > 0 && nFound != SAL_MAX_INT32 && safetyCount < MAX_REPLACEMENTS)
                {
                    nFound = rSh.SearchPattern(aSearchOpt,
                        false,
                        SwDocPositions::Curr,
                        SwDocPositions::End,
                        FindRanges::InBody,
                        true);
                    safetyCount++;
                    if (replDbg && safetyCount % 10 == 0) { fprintf(replDbg, "  Replace iteration %d, nFound=%d\n", safetyCount, (int)nFound); fflush(replDbg); }
                }
                if (replDbg) { fprintf(replDbg, "  Replace complete, iterations=%d\n", safetyCount); fclose(replDbg); }
                editCount++;
            }
        }
        else if (cmd.action == u"insert_page_break"_ustr)
        {
            // Move to position if specified
            if (cmd.position == u"start"_ustr)
            {
                rSh.StartOfSection();
            }
            else if (cmd.position == u"end"_ustr)
            {
                rSh.EndOfSection();
            }
            // Insert page break
            rSh.InsertPageBreak();
            editCount++;
        }
        else if (cmd.action == u"apply_style"_ustr)
        {
            // Find text and apply named style
            if (!cmd.findText.isEmpty() && !cmd.styleName.isEmpty())
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
                    // Map style name to pool ID
                    sal_uInt16 nPoolId = RES_POOLCOLL_STANDARD;
                    if (cmd.styleName == u"Heading 1"_ustr || cmd.styleName == u"Title"_ustr)
                        nPoolId = RES_POOLCOLL_HEADLINE1;
                    else if (cmd.styleName == u"Heading 2"_ustr || cmd.styleName == u"Subtitle"_ustr)
                        nPoolId = RES_POOLCOLL_HEADLINE2;
                    else if (cmd.styleName == u"Heading 3"_ustr)
                        nPoolId = RES_POOLCOLL_HEADLINE3;
                    else if (cmd.styleName == u"Heading 4"_ustr)
                        nPoolId = RES_POOLCOLL_HEADLINE4;
                    else if (cmd.styleName == u"Heading 5"_ustr)
                        nPoolId = RES_POOLCOLL_HEADLINE5;
                    else if (cmd.styleName == u"Heading 6"_ustr)
                        nPoolId = RES_POOLCOLL_HEADLINE6;
                    else if (cmd.styleName == u"Text Body"_ustr)
                        nPoolId = RES_POOLCOLL_TEXT;
                    else if (cmd.styleName == u"Quotations"_ustr)
                        nPoolId = RES_POOLCOLL_HTML_BLOCKQUOTE;
                    else if (cmd.styleName == u"Preformatted Text"_ustr)
                        nPoolId = RES_POOLCOLL_HTML_PRE;

                    SwDoc* pDoc = rSh.GetDoc();
                    if (pDoc)
                    {
                        SwTextFormatColl* pColl = pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(nPoolId);
                        if (pColl)
                            rSh.SetTextFormatColl(pColl);
                    }
                    editCount++;
                }
            }
        }
        else if (cmd.action == u"undo"_ustr)
        {
            // Undo operations - limit to reasonable range
            int steps = std::min(std::max(cmd.undoSteps, 1), 100);
            for (int i = 0; i < steps; ++i)
            {
                if (SfxViewShell* pViewShell = SfxViewShell::Current())
                {
                    SfxDispatcher* pDispatcher = pViewShell->GetDispatcher();
                    if (pDispatcher)
                        pDispatcher->Execute(SID_UNDO, SfxCallMode::SYNCHRON);
                }
            }
            editCount++;
        }
        else if (cmd.action == u"redo"_ustr)
        {
            // Redo operations - limit to reasonable range
            int steps = std::min(std::max(cmd.undoSteps, 1), 100);
            for (int i = 0; i < steps; ++i)
            {
                if (SfxViewShell* pViewShell = SfxViewShell::Current())
                {
                    SfxDispatcher* pDispatcher = pViewShell->GetDispatcher();
                    if (pDispatcher)
                        pDispatcher->Execute(SID_REDO, SfxCallMode::SYNCHRON);
                }
            }
            editCount++;
        }
    }

    if (execDbg) { fprintf(execDbg, "All actions processed, calling EndAllAction\n"); fflush(execDbg); }
    rSh.EndAllAction();
    if (execDbg) { fprintf(execDbg, "EndAllAction done, editCount=%d\n", editCount); fflush(execDbg); }

    if (editCount > 0)
    {
        // Record edit completion for revert system (Phase 2.5)
        RecordEditEnd();

        OUString msg = OUString::number(editCount) + u" edit(s) applied"_ustr;
        AppendSystemMessage(msg, true);
        UpdateStatus(u"Ready"_ustr);
    }
    if (execDbg) { fprintf(execDbg, "ExecuteAutoEdits complete\n"); fclose(execDbg); }
}

void AIAssistantPanel::SendMessage(const OUString& message)
{
    // Delegate to async version for non-blocking UI
    SendMessageAsync(message);
}

void AIAssistantPanel::SendQuickAction(const OUString& action, const OUString& customPrompt)
{
    OUString selection = GetSelectedText();
    OUString textToProcess = selection.isEmpty() ? GetDocumentText() : selection;

    if (textToProcess.isEmpty())
    {
        AppendSystemMessage(u"No text to process. Select text or add content."_ustr, false);
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

        // Get document and style pool - with null checks
        SwDoc* pDoc = m_pWrtShell->GetDoc();
        if (!pDoc)
        {
            m_pWrtShell->Insert(sLine);
            continue;
        }

        // Check for heading (## or #)
        if (sLine.startsWith(u"## "))
        {
            OUString sHeadingText = sLine.copy(3);
            SwTextFormatColl* pColl = pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_HEADLINE2);
            if (pColl)
                m_pWrtShell->SetTextFormatColl(pColl);
            m_pWrtShell->Insert(sHeadingText);
        }
        else if (sLine.startsWith(u"# "))
        {
            OUString sHeadingText = sLine.copy(2);
            SwTextFormatColl* pColl = pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_HEADLINE1);
            if (pColl)
                m_pWrtShell->SetTextFormatColl(pColl);
            m_pWrtShell->Insert(sHeadingText);
        }
        else if (sLine.startsWith(u"- ") || sLine.startsWith(u"* "))
        {
            OUString sBulletText = sLine.copy(2);
            SwTextFormatColl* pColl = pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD);
            if (pColl)
                m_pWrtShell->SetTextFormatColl(pColl);
            m_pWrtShell->Insert(u"\u2022 "_ustr + sBulletText);
        }
        else if (sLine.startsWith(u"**") && sLine.endsWith(u"**") && sLine.getLength() > 4)
        {
            OUString sBoldText = sLine.copy(2, sLine.getLength() - 4);
            SwTextFormatColl* pColl = pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD);
            if (pColl)
                m_pWrtShell->SetTextFormatColl(pColl);
            SfxItemSet aSet(m_pWrtShell->GetAttrPool(), svl::Items<RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT>);
            aSet.Put(SvxWeightItem(WEIGHT_BOLD, RES_CHRATR_WEIGHT));
            m_pWrtShell->Insert(sBoldText);
        }
        else
        {
            SwTextFormatColl* pColl = pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD);
            if (pColl)
                m_pWrtShell->SetTextFormatColl(pColl);
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

// ==================== SELECTION CONTEXT (PHASE 2.5) ====================

IMPL_LINK_NOARG(AIAssistantPanel, InputFocusInHdl, weld::Widget&, void)
{
    // Refresh selection context when input field gets focus
    RefreshSelectionContext();
}

IMPL_LINK_NOARG(AIAssistantPanel, ClearSelectionClickHdl, weld::Button&, void)
{
    // Clear the selection context
    m_sCurrentSelection.clear();
    if (m_xSelectionContextBox)
        m_xSelectionContextBox->set_visible(false);
    UpdateStatus(u"Selection cleared"_ustr);
}

void AIAssistantPanel::RefreshSelectionContext()
{
    // Re-check for selection each time the input field gets focus
    // This allows the user to select text, then click on the input field
    if (!m_pWrtShell)
    {
        // Try to get the shell again (it might have become available)
        SwView* pView = GetActiveView();
        if (pView)
            m_pWrtShell = &pView->GetWrtShell();
    }

    if (m_pWrtShell && m_pWrtShell->HasSelection())
    {
        OUString sSelection;
        m_pWrtShell->GetSelectedText(sSelection, ParaBreakType::ToBlank);
        if (!sSelection.isEmpty())
        {
            // Store the full selection for later use
            m_sCurrentSelection = sSelection;

            // Count words in selection
            sal_Int32 nWordCount = 0;
            sal_Int32 nCharCount = sSelection.getLength();
            bool bInWord = false;
            for (sal_Int32 i = 0; i < sSelection.getLength(); ++i)
            {
                sal_Unicode c = sSelection[i];
                bool bIsWordChar = (c != ' ' && c != '\t' && c != '\n' && c != '\r');
                if (bIsWordChar && !bInWord)
                {
                    nWordCount++;
                    bInWord = true;
                }
                else if (!bIsWordChar)
                {
                    bInWord = false;
                }
            }

            // Build display text with word count (like Cursor)
            OUStringBuffer displayBuf;
            displayBuf.append(OUString::number(nWordCount));
            displayBuf.append(u" word");
            if (nWordCount != 1)
                displayBuf.append(u"s");
            displayBuf.append(u" ("_ustr);
            displayBuf.append(OUString::number(nCharCount));
            displayBuf.append(u" chars)"_ustr);

            // Add truncated preview
            OUString sPreview = sSelection;
            if (sPreview.getLength() > 30)
                sPreview = sPreview.copy(0, 30) + u"..."_ustr;
            displayBuf.append(u" \""_ustr);
            displayBuf.append(sPreview);
            displayBuf.append(u"\""_ustr);

            // Show selection context in the badge UI (Cursor-like)
            if (m_xSelectionContextLabel)
                m_xSelectionContextLabel->set_label(displayBuf.makeStringAndClear());
            if (m_xSelectionContextBox)
                m_xSelectionContextBox->set_visible(true);

            UpdateStatus(u"Selection ready - Type your instruction"_ustr);
        }
    }
    else
    {
        // No selection - hide the context box if there's no stored selection
        if (m_sCurrentSelection.isEmpty() && m_xSelectionContextBox)
            m_xSelectionContextBox->set_visible(false);
    }
}

// ==================== EDIT HISTORY (PHASE 2.5) ====================

void AIAssistantPanel::RecordEditStart(const OUString& description)
{
    // Create a new edit history entry
    EditHistoryEntry entry;
    entry.description = description;
    entry.undoCount = 0;  // Will be set in RecordEditEnd
    entry.reverted = false;
    entry.accepted = false;
    entry.timestamp = std::chrono::system_clock::now();

    m_aEditHistory.push_back(entry);
    UpdateRevertButtonState();
}

void AIAssistantPanel::RecordEditEnd()
{
    // The edit has been completed - the undo count is 1 for single operations
    if (!m_aEditHistory.empty())
    {
        m_aEditHistory.back().undoCount = 1;
    }
    UpdateRevertButtonState();
}

void AIAssistantPanel::RevertLastEdit()
{
    if (m_aEditHistory.empty())
        return;

    // Get the last non-reverted edit
    for (auto it = m_aEditHistory.rbegin(); it != m_aEditHistory.rend(); ++it)
    {
        if (!it->reverted)
        {
            if (m_pWrtShell)
            {
                // Undo the edit
                for (sal_uInt16 i = 0; i < it->undoCount && i < 10; ++i)
                {
                    m_pWrtShell->Undo();
                }
                it->reverted = true;
                AppendToChat(u"System"_ustr, u"[Reverted: "_ustr + it->description + u"]"_ustr);
                UpdateStatus(u"Reverted: "_ustr + it->description);
            }
            break;
        }
    }
    UpdateRevertButtonState();
}

void AIAssistantPanel::RevertAllEdits()
{
    if (m_aEditHistory.empty())
        return;

    // Count total undos needed
    sal_uInt16 nTotalUndos = 0;
    for (const auto& entry : m_aEditHistory)
    {
        if (!entry.reverted)
        {
            nTotalUndos += entry.undoCount;
        }
    }

    if (m_pWrtShell && nTotalUndos > 0)
    {
        // Undo all edits
        for (sal_uInt16 i = 0; i < nTotalUndos && i < 50; ++i)
        {
            m_pWrtShell->Undo();
        }

        // Mark all as reverted
        for (auto& entry : m_aEditHistory)
        {
            if (!entry.reverted)
            {
                entry.reverted = true;
            }
        }

        AppendToChat(u"System"_ustr, u"[All AI edits reverted]"_ustr);
        UpdateStatus(u"All edits reverted"_ustr);
    }
    UpdateRevertButtonState();
}

void AIAssistantPanel::AcceptAllEdits()
{
    // Mark all edits as accepted and clear history
    for (auto& entry : m_aEditHistory)
    {
        entry.accepted = true;
    }
    m_aEditHistory.clear();

    AppendToChat(u"System"_ustr, u"[All AI edits accepted]"_ustr);
    UpdateStatus(u"All edits accepted"_ustr);
    UpdateRevertButtonState();
}

void AIAssistantPanel::UpdateRevertButtonState()
{
    bool bHasUnrevertedEdits = false;
    for (const auto& entry : m_aEditHistory)
    {
        if (!entry.reverted && !entry.accepted)
        {
            bHasUnrevertedEdits = true;
            break;
        }
    }

    if (m_xRevertAllButton)
        m_xRevertAllButton->set_sensitive(bHasUnrevertedEdits);
    if (m_xAcceptAllButton)
        m_xAcceptAllButton->set_sensitive(bHasUnrevertedEdits);
}

IMPL_LINK_NOARG(AIAssistantPanel, RevertAllClickHdl, weld::Button&, void)
{
    RevertAllEdits();
}

IMPL_LINK_NOARG(AIAssistantPanel, AcceptAllClickHdl, weld::Button&, void)
{
    AcceptAllEdits();
}

// ==================== ASYNC HTTP SUPPORT ====================

void AIAssistantPanel::SendMessageAsync(const OUString& message)
{
    if (message.isEmpty())
        return;

    if (m_bProcessing || m_bAsyncRequestPending)
    {
        UpdateStatus(u"Please wait for current request..."_ustr);
        return;
    }

    // Check if we're awaiting clarification answers
    if (m_bAwaitingClarification)
    {
        // Process the input as clarification answers
        ProcessClarificationAnswers(message);

        // Clear input
        if (m_xInputField)
        {
            m_xInputField->set_text(u""_ustr);
            m_xInputField->set_placeholder_text(u""_ustr);
        }
        return;
    }

    // Store the message in case we need clarification
    m_sPendingMessage = message;

    // Add user message to chat with new formatting
    AppendUserMessage(message);

    // Clear input immediately
    if (m_xInputField)
        m_xInputField->set_text(u""_ustr);

    // Check connection
    if (!m_pAgentConnection->isConnected())
    {
        if (!m_pAgentConnection->checkConnection())
        {
            AppendSystemMessage(u"Backend not available at localhost:8765"_ustr, false);
            UpdateStatus(u"Backend offline"_ustr);
            return;
        }
    }

    // Update UI for processing state
    m_bProcessing = true;
    m_bAsyncRequestPending = true;
    UpdateStatus(u"AI is thinking..."_ustr);
    if (m_xSendButton)
        m_xSendButton->set_sensitive(false);

    // Show typing indicator in chat
    AppendTypingIndicator();

    // Get document content and selection for the AI - store for async use
    m_sAsyncDocContent = GetDocumentText();
    m_sAsyncSelection = GetSelectedText();

    // Launch HTTP call in background thread
    // Note: We need to capture the connection pointer, not 'this', to avoid thread safety issues
    auto* pConnection = m_pAgentConnection.get();
    OUString sMsg = message;
    OUString sDoc = m_sAsyncDocContent;
    OUString sSel = m_sAsyncSelection;

    m_aAsyncResponse = std::async(std::launch::async, [pConnection, sMsg, sDoc, sSel]() {
        return pConnection->sendMessage(sMsg, sDoc, sSel);
    });

    // Start polling for completion using Idle (runs on main thread)
    m_aResponseChecker.Start();
}

IMPL_LINK_NOARG(AIAssistantPanel, CheckAsyncResponseHdl, Timer*, void)
{
    if (!m_bAsyncRequestPending)
    {
        m_aResponseChecker.Stop();
        return;
    }

    // Check if the async operation has completed
    if (m_aAsyncResponse.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
    {
        // Response arrived - stop polling
        m_aResponseChecker.Stop();
        m_bAsyncRequestPending = false;

        try
        {
            // Get the result and process on main thread
            officelabs::AgentResponse response = m_aAsyncResponse.get();

            // Remove typing indicator and show response
            RemoveTypingIndicator();
            ProcessResponse(response);
        }
        catch (const std::exception& e)
        {
            // Handle any async errors
            RemoveTypingIndicator();
            m_bProcessing = false;
            if (m_xSendButton)
                m_xSendButton->set_sensitive(true);

            OUString sError = u"Async error: "_ustr + OUString::createFromAscii(e.what());
            AppendSystemMessage(sError, false);
            UpdateStatus(u"Request failed"_ustr);
        }
    }
    else
    {
        // Keep checking - restart the idle timer
        m_aResponseChecker.Start();
    }
}

} // end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
