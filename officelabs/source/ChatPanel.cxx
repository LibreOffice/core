/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs AI Assistant - Full Featured Chat Panel Implementation
 *
 * Complete AI assistant with:
 * - Multi-session tabs
 * - @ reference system for document elements
 * - Quick AI actions (clean, review, summarize, adapt, improve, translate)
 * - Direct formatting
 * - Pending edits workflow
 */

#include <officelabs/ChatPanel.hxx>
#include <officelabs/AgentConnection.hxx>
#include <officelabs/DocumentController.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/menu.hxx>
#include <tools/datetime.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/random.hxx>
#include <sstream>
#include <iomanip>
#include <ctime>

using namespace css;

namespace officelabs {

// ==================== THEME COLORS ====================
namespace Colors {
    const Color BG_DARK(0x1A, 0x1A, 0x2E);
    const Color BG_PANEL(0x16, 0x21, 0x3E);
    const Color BG_INPUT(0x0F, 0x3D, 0x3E);
    const Color BG_BUTTON(0x53, 0x35, 0x4A);
    const Color BG_BUTTON_HOVER(0x6B, 0x4D, 0x62);
    const Color BG_USER_MSG(0x00, 0x7A, 0xCC);
    const Color BG_AI_MSG(0x2D, 0x2D, 0x3D);
    const Color BG_TOOL_MSG(0x3D, 0x3D, 0x5C);
    const Color BG_PENDING(0x4A, 0x4A, 0x1A);
    const Color BG_APPROVED(0x1A, 0x4A, 0x1A);
    const Color BG_REJECTED(0x4A, 0x1A, 0x1A);
    const Color TEXT_PRIMARY(0xFF, 0xFF, 0xFF);
    const Color TEXT_SECONDARY(0xA0, 0xA0, 0xA0);
    const Color TEXT_USER(0xFF, 0xFF, 0xFF);
    const Color TEXT_AI(0x4E, 0xC9, 0xB0);
    const Color TEXT_SYSTEM(0xFF, 0xB0, 0x50);
    const Color TEXT_TOOL(0xCE, 0x91, 0x78);
    const Color ACCENT_BLUE(0x00, 0x7A, 0xCC);
    const Color ACCENT_GREEN(0x4E, 0xC9, 0x4E);
    const Color ACCENT_YELLOW(0xFF, 0xC1, 0x07);
    const Color ACCENT_RED(0xF4, 0x43, 0x36);
}

// ==================== HELPERS ====================
namespace {
    OUString GenerateId() {
        std::stringstream ss;
        for (int i = 0; i < 8; ++i)
            ss << std::hex << comphelper::rng::uniform_uint_distribution(0, 15);
        return OUString::fromUtf8(ss.str().c_str());
    }

    OUString GetTimestamp() {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M");
        return OUString::fromUtf8(oss.str().c_str());
    }
}

// ==================== CONSTRUCTOR ====================
ChatPanel::ChatPanel(SfxBindings* pBindings, SfxChildWindow* pChildWin, vcl::Window* pParent)
    : SfxDockingWindow(pBindings, pChildWin, pParent, WB_STDDOCKWIN | WB_CLOSEABLE | WB_SIZEABLE)
    , m_nextSessionNum(1)
    , m_connected(false)
    , m_processing(false)
    , m_includeSelection(false)
{
    SetText("OfficeLabs AI Assistant");
    SetSizePixel(Size(450, 800));
    SetBackground(Wallpaper(Colors::BG_DARK));

    // Backend
    m_pAgent = std::make_unique<AgentConnection>();
    m_pDocController = std::make_unique<DocumentController>();

    // ========== HEADER ==========
    m_pStatusLabel = VclPtr<FixedText>::Create(this, WB_LEFT);
    m_pModelLabel = VclPtr<FixedText>::Create(this, WB_LEFT);
    m_pSettingsButton = VclPtr<PushButton>::Create(this);
    m_pSettingsButton->SetText("Settings");

    // ========== SESSION TABS ==========
    m_pTabControl = VclPtr<TabControl>::Create(this);
    m_pNewSessionButton = VclPtr<PushButton>::Create(this);
    m_pNewSessionButton->SetText("+");
    m_pNewSessionButton->SetQuickHelpText("Create new chat session");

    // ========== QUICK ACTIONS ROW 1 ==========
    m_pCleanDocButton = VclPtr<PushButton>::Create(this);
    m_pReviewErrorsButton = VclPtr<PushButton>::Create(this);
    m_pSummarizeButton = VclPtr<PushButton>::Create(this);
    m_pAdaptTextButton = VclPtr<PushButton>::Create(this);

    m_pCleanDocButton->SetText("Clean");
    m_pReviewErrorsButton->SetText("Review");
    m_pSummarizeButton->SetText("Summary");
    m_pAdaptTextButton->SetText("Adapt");

    m_pCleanDocButton->SetQuickHelpText("Clean document: fix spacing, formatting issues");
    m_pReviewErrorsButton->SetQuickHelpText("Review: grammar, spelling, punctuation errors");
    m_pSummarizeButton->SetQuickHelpText("Summarize selected text or entire document");
    m_pAdaptTextButton->SetQuickHelpText("Adapt text: change tone, formality, style");

    // ========== QUICK ACTIONS ROW 2 ==========
    m_pImproveButton = VclPtr<PushButton>::Create(this);
    m_pTranslateButton = VclPtr<PushButton>::Create(this);
    m_pExpandButton = VclPtr<PushButton>::Create(this);
    m_pCondenseButton = VclPtr<PushButton>::Create(this);

    m_pImproveButton->SetText("Improve");
    m_pTranslateButton->SetText("Translate");
    m_pExpandButton->SetText("Expand");
    m_pCondenseButton->SetText("Condense");

    m_pImproveButton->SetQuickHelpText("Improve text clarity and quality");
    m_pTranslateButton->SetQuickHelpText("Translate selected text");
    m_pExpandButton->SetQuickHelpText("Expand text with more details");
    m_pCondenseButton->SetQuickHelpText("Make text shorter and more concise");

    // ========== FORMATTING ROW ==========
    m_pBoldButton = VclPtr<PushButton>::Create(this);
    m_pItalicButton = VclPtr<PushButton>::Create(this);
    m_pUnderlineButton = VclPtr<PushButton>::Create(this);
    m_pHeadingButton = VclPtr<PushButton>::Create(this);
    m_pBulletButton = VclPtr<PushButton>::Create(this);
    m_pNumberButton = VclPtr<PushButton>::Create(this);

    m_pBoldButton->SetText("B");
    m_pItalicButton->SetText("I");
    m_pUnderlineButton->SetText("U");
    m_pHeadingButton->SetText("H1");
    m_pBulletButton->SetText("List");
    m_pNumberButton->SetText("1.2.");

    // ========== CHAT HISTORY ==========
    m_pChatHistory = VclPtr<Control>::Create(this, WB_BORDER);

    // ========== PENDING EDITS BAR ==========
    m_pEditsBar = VclPtr<Control>::Create(this, WB_BORDER);
    m_pEditsLabel = VclPtr<FixedText>::Create(this, WB_LEFT);
    m_pAcceptAllButton = VclPtr<PushButton>::Create(this);
    m_pRejectAllButton = VclPtr<PushButton>::Create(this);
    m_pReviewButton = VclPtr<PushButton>::Create(this);

    m_pAcceptAllButton->SetText("Accept All");
    m_pRejectAllButton->SetText("Reject All");
    m_pReviewButton->SetText("Review");

    // ========== INPUT AREA ==========
    m_pAtRefButton = VclPtr<PushButton>::Create(this);
    m_pAttachButton = VclPtr<PushButton>::Create(this);
    m_pInputField = VclPtr<Edit>::Create(this, WB_BORDER);
    m_pSendButton = VclPtr<PushButton>::Create(this);

    m_pAtRefButton->SetText("@");
    m_pAttachButton->SetText("Attach");
    m_pSendButton->SetText("Send");

    m_pAtRefButton->SetQuickHelpText("Reference document elements: @selection, @table1, @image1, @chapter1");
    m_pAttachButton->SetQuickHelpText("Attach a file to the conversation");
    m_pInputField->SetPlaceholderText(u"Ask me anything... Use @ to reference document elements"_ustr);

    // ========== BOTTOM ROW ==========
    m_pInsertButton = VclPtr<PushButton>::Create(this);
    m_pReplaceButton = VclPtr<PushButton>::Create(this);
    m_pResetButton = VclPtr<PushButton>::Create(this);
    m_pStopButton = VclPtr<PushButton>::Create(this);

    m_pInsertButton->SetText("Insert");
    m_pReplaceButton->SetText("Replace");
    m_pResetButton->SetText("Reset");
    m_pStopButton->SetText("Stop");

    m_pInsertButton->SetQuickHelpText("Insert AI response at cursor position");
    m_pReplaceButton->SetQuickHelpText("Replace selection with AI response");
    m_pResetButton->SetQuickHelpText("Clear current session and start fresh");
    m_pStopButton->SetQuickHelpText("Stop AI processing");

    // ========== CONNECT HANDLERS ==========
    // Session handlers
    m_pNewSessionButton->SetClickHdl(LINK(this, ChatPanel, NewSessionClickHdl));
    m_pTabControl->SetActivatePageHdl(LINK(this, ChatPanel, TabChangedHdl));
    m_pSettingsButton->SetClickHdl(LINK(this, ChatPanel, SettingsClickHdl));

    // Quick action handlers
    m_pCleanDocButton->SetClickHdl(LINK(this, ChatPanel, CleanDocClickHdl));
    m_pReviewErrorsButton->SetClickHdl(LINK(this, ChatPanel, ReviewErrorsClickHdl));
    m_pSummarizeButton->SetClickHdl(LINK(this, ChatPanel, SummarizeClickHdl));
    m_pAdaptTextButton->SetClickHdl(LINK(this, ChatPanel, AdaptTextClickHdl));
    m_pImproveButton->SetClickHdl(LINK(this, ChatPanel, ImproveClickHdl));
    m_pTranslateButton->SetClickHdl(LINK(this, ChatPanel, TranslateClickHdl));
    m_pExpandButton->SetClickHdl(LINK(this, ChatPanel, ExpandClickHdl));
    m_pCondenseButton->SetClickHdl(LINK(this, ChatPanel, CondenseClickHdl));

    // Formatting handlers
    m_pBoldButton->SetClickHdl(LINK(this, ChatPanel, BoldClickHdl));
    m_pItalicButton->SetClickHdl(LINK(this, ChatPanel, ItalicClickHdl));
    m_pUnderlineButton->SetClickHdl(LINK(this, ChatPanel, UnderlineClickHdl));
    m_pHeadingButton->SetClickHdl(LINK(this, ChatPanel, HeadingClickHdl));
    m_pBulletButton->SetClickHdl(LINK(this, ChatPanel, BulletClickHdl));
    m_pNumberButton->SetClickHdl(LINK(this, ChatPanel, NumberClickHdl));

    // Chat handlers
    m_pSendButton->SetClickHdl(LINK(this, ChatPanel, SendClickHdl));
    m_pAtRefButton->SetClickHdl(LINK(this, ChatPanel, AtRefClickHdl));
    m_pAttachButton->SetClickHdl(LINK(this, ChatPanel, AttachClickHdl));

    // Action handlers
    m_pInsertButton->SetClickHdl(LINK(this, ChatPanel, InsertClickHdl));
    m_pReplaceButton->SetClickHdl(LINK(this, ChatPanel, ReplaceClickHdl));
    m_pResetButton->SetClickHdl(LINK(this, ChatPanel, ResetClickHdl));
    m_pStopButton->SetClickHdl(LINK(this, ChatPanel, StopClickHdl));

    // Edit handlers
    m_pAcceptAllButton->SetClickHdl(LINK(this, ChatPanel, AcceptAllClickHdl));
    m_pRejectAllButton->SetClickHdl(LINK(this, ChatPanel, RejectAllClickHdl));
    m_pReviewButton->SetClickHdl(LINK(this, ChatPanel, ReviewClickHdl));

    // ========== SHOW COMPONENTS ==========
    m_pStatusLabel->Show();
    m_pModelLabel->Show();
    m_pSettingsButton->Show();
    m_pTabControl->Show();
    m_pNewSessionButton->Show();

    m_pCleanDocButton->Show();
    m_pReviewErrorsButton->Show();
    m_pSummarizeButton->Show();
    m_pAdaptTextButton->Show();
    m_pImproveButton->Show();
    m_pTranslateButton->Show();
    m_pExpandButton->Show();
    m_pCondenseButton->Show();

    m_pBoldButton->Show();
    m_pItalicButton->Show();
    m_pUnderlineButton->Show();
    m_pHeadingButton->Show();
    m_pBulletButton->Show();
    m_pNumberButton->Show();

    m_pChatHistory->Show();

    m_pEditsBar->Hide();
    m_pEditsLabel->Hide();
    m_pAcceptAllButton->Hide();
    m_pRejectAllButton->Hide();
    m_pReviewButton->Hide();

    m_pAtRefButton->Show();
    m_pAttachButton->Show();
    m_pInputField->Show();
    m_pSendButton->Show();

    m_pInsertButton->Show();
    m_pReplaceButton->Show();
    m_pResetButton->Show();
    m_pStopButton->Show();
    m_pStopButton->Enable(false);

    // ========== INITIALIZE ==========
    InitializeDocument();
    CheckConnection();
    CreateNewSession(u"Chat 1"_ustr);
    DoLayout();
}

ChatPanel::~ChatPanel() {
    disposeOnce();
}

void ChatPanel::dispose() {
    m_pStatusLabel.disposeAndClear();
    m_pModelLabel.disposeAndClear();
    m_pSettingsButton.disposeAndClear();
    m_pTabControl.disposeAndClear();
    m_pNewSessionButton.disposeAndClear();

    m_pCleanDocButton.disposeAndClear();
    m_pReviewErrorsButton.disposeAndClear();
    m_pSummarizeButton.disposeAndClear();
    m_pAdaptTextButton.disposeAndClear();
    m_pImproveButton.disposeAndClear();
    m_pTranslateButton.disposeAndClear();
    m_pExpandButton.disposeAndClear();
    m_pCondenseButton.disposeAndClear();

    m_pBoldButton.disposeAndClear();
    m_pItalicButton.disposeAndClear();
    m_pUnderlineButton.disposeAndClear();
    m_pHeadingButton.disposeAndClear();
    m_pBulletButton.disposeAndClear();
    m_pNumberButton.disposeAndClear();

    m_pChatHistory.disposeAndClear();

    m_pEditsBar.disposeAndClear();
    m_pEditsLabel.disposeAndClear();
    m_pAcceptAllButton.disposeAndClear();
    m_pRejectAllButton.disposeAndClear();
    m_pReviewButton.disposeAndClear();

    m_pAtRefButton.disposeAndClear();
    m_pAttachButton.disposeAndClear();
    m_pInputField.disposeAndClear();
    m_pSendButton.disposeAndClear();

    m_pInsertButton.disposeAndClear();
    m_pReplaceButton.disposeAndClear();
    m_pResetButton.disposeAndClear();
    m_pStopButton.disposeAndClear();

    SfxDockingWindow::dispose();
}

// ==================== LAYOUT ====================
void ChatPanel::DoLayout() {
    Size aSize = GetSizePixel();
    const sal_Int32 M = 6;  // Margin
    const sal_Int32 BH = 24; // Button height
    const sal_Int32 SBH = 22; // Small button height
    Point pos(M, M);

    // Header row
    sal_Int32 headerW = (aSize.Width() - M * 3 - 60) / 2;
    m_pStatusLabel->SetPosSizePixel(pos, Size(headerW, 18));
    m_pModelLabel->SetPosSizePixel(Point(pos.X() + headerW + M, pos.Y()), Size(headerW, 18));
    m_pSettingsButton->SetPosSizePixel(Point(aSize.Width() - M - 55, pos.Y()), Size(55, SBH));
    pos.AdjustY(24);

    // Tab row
    sal_Int32 tabW = aSize.Width() - M * 3 - 28;
    m_pTabControl->SetPosSizePixel(pos, Size(tabW, 28));
    m_pNewSessionButton->SetPosSizePixel(Point(aSize.Width() - M - 26, pos.Y()), Size(26, 26));
    pos.AdjustY(32);

    // Quick actions row 1
    sal_Int32 qw = (aSize.Width() - M * 2 - 9) / 4;
    m_pCleanDocButton->SetPosSizePixel(pos, Size(qw, BH));
    m_pReviewErrorsButton->SetPosSizePixel(Point(pos.X() + qw + 3, pos.Y()), Size(qw, BH));
    m_pSummarizeButton->SetPosSizePixel(Point(pos.X() + (qw + 3) * 2, pos.Y()), Size(qw, BH));
    m_pAdaptTextButton->SetPosSizePixel(Point(pos.X() + (qw + 3) * 3, pos.Y()), Size(qw, BH));
    pos.AdjustY(BH + 3);

    // Quick actions row 2
    m_pImproveButton->SetPosSizePixel(pos, Size(qw, BH));
    m_pTranslateButton->SetPosSizePixel(Point(pos.X() + qw + 3, pos.Y()), Size(qw, BH));
    m_pExpandButton->SetPosSizePixel(Point(pos.X() + (qw + 3) * 2, pos.Y()), Size(qw, BH));
    m_pCondenseButton->SetPosSizePixel(Point(pos.X() + (qw + 3) * 3, pos.Y()), Size(qw, BH));
    pos.AdjustY(BH + 3);

    // Formatting row
    sal_Int32 fw = 28;
    m_pBoldButton->SetPosSizePixel(pos, Size(fw, SBH));
    m_pItalicButton->SetPosSizePixel(Point(pos.X() + fw + 2, pos.Y()), Size(fw, SBH));
    m_pUnderlineButton->SetPosSizePixel(Point(pos.X() + (fw + 2) * 2, pos.Y()), Size(fw, SBH));
    m_pHeadingButton->SetPosSizePixel(Point(pos.X() + (fw + 2) * 3, pos.Y()), Size(32, SBH));
    m_pBulletButton->SetPosSizePixel(Point(pos.X() + (fw + 2) * 3 + 35, pos.Y()), Size(36, SBH));
    m_pNumberButton->SetPosSizePixel(Point(pos.X() + (fw + 2) * 3 + 74, pos.Y()), Size(36, SBH));
    pos.AdjustY(SBH + 4);

    // Calculate remaining space
    sal_Int32 inputAreaH = BH + 4 + BH + M;
    sal_Int32 editsBarH = 0;

    ChatSession* session = GetCurrentSession();
    if (session) {
        for (const auto& edit : session->pendingEdits) {
            if (!edit.approved && !edit.rejected) {
                editsBarH = 32;
                break;
            }
        }
    }

    // Chat history
    sal_Int32 historyH = aSize.Height() - pos.Y() - inputAreaH - editsBarH - M;
    m_pChatHistory->SetPosSizePixel(pos, Size(aSize.Width() - M * 2, historyH));
    pos.AdjustY(historyH + 4);

    // Edits bar (if visible)
    if (editsBarH > 0) {
        m_pEditsBar->SetPosSizePixel(pos, Size(aSize.Width() - M * 2, 28));
        m_pEditsLabel->SetPosSizePixel(Point(pos.X() + 4, pos.Y() + 4), Size(100, 20));
        m_pAcceptAllButton->SetPosSizePixel(Point(pos.X() + 110, pos.Y() + 2), Size(70, SBH));
        m_pRejectAllButton->SetPosSizePixel(Point(pos.X() + 185, pos.Y() + 2), Size(65, SBH));
        m_pReviewButton->SetPosSizePixel(Point(pos.X() + 255, pos.Y() + 2), Size(55, SBH));
        m_pEditsBar->Show();
        m_pEditsLabel->Show();
        m_pAcceptAllButton->Show();
        m_pRejectAllButton->Show();
        m_pReviewButton->Show();
        pos.AdjustY(32);
    } else {
        m_pEditsBar->Hide();
        m_pEditsLabel->Hide();
        m_pAcceptAllButton->Hide();
        m_pRejectAllButton->Hide();
        m_pReviewButton->Hide();
    }

    // Input row: [@] [Attach] [Input...] [Send]
    m_pAtRefButton->SetPosSizePixel(pos, Size(28, BH));
    m_pAttachButton->SetPosSizePixel(Point(pos.X() + 31, pos.Y()), Size(50, BH));
    sal_Int32 inputW = aSize.Width() - M * 2 - 28 - 50 - 50 - 12;
    m_pInputField->SetPosSizePixel(Point(pos.X() + 84, pos.Y()), Size(inputW, BH));
    m_pSendButton->SetPosSizePixel(Point(aSize.Width() - M - 48, pos.Y()), Size(48, BH));
    pos.AdjustY(BH + 4);

    // Bottom row: [Insert] [Replace] [Reset] [Stop]
    sal_Int32 bw = (aSize.Width() - M * 2 - 9) / 4;
    m_pInsertButton->SetPosSizePixel(pos, Size(bw, BH));
    m_pReplaceButton->SetPosSizePixel(Point(pos.X() + bw + 3, pos.Y()), Size(bw, BH));
    m_pResetButton->SetPosSizePixel(Point(pos.X() + (bw + 3) * 2, pos.Y()), Size(bw, BH));
    m_pStopButton->SetPosSizePixel(Point(pos.X() + (bw + 3) * 3, pos.Y()), Size(bw, BH));
}

void ChatPanel::Resize() {
    DockingWindow::Resize();
    DoLayout();
}

// ==================== INITIALIZATION ====================
void ChatPanel::InitializeDocument() {
    try {
        uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
        uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(xContext);
        uno::Reference<frame::XFrame> xFrame = xDesktop->getCurrentFrame();
        if (xFrame.is()) {
            uno::Reference<frame::XController> xController = xFrame->getController();
            if (xController.is()) {
                uno::Reference<frame::XModel> xModel = xController->getModel();
                uno::Reference<text::XTextDocument> xTextDoc(xModel, uno::UNO_QUERY);
                if (xTextDoc.is()) {
                    SetDocument(xTextDoc);
                    RefreshDocumentElements();
                    return;
                }
            }
        }
    } catch (...) {}
    if (m_pStatusLabel)
        m_pStatusLabel->SetText("No document");
}

void ChatPanel::RefreshDocument() {
    InitializeDocument();
    AddMessage(ChatMessage::Role::SYSTEM, u"Document connection refreshed."_ustr);
}

void ChatPanel::SetDocument(const css::uno::Reference<css::text::XTextDocument>& xDoc) {
    m_xDocument = xDoc;
    if (m_pDocController)
        m_pDocController->setDocument(xDoc);
}

void ChatPanel::RefreshDocumentElements() {
    m_docElements.clear();

    if (!m_xDocument.is()) return;

    try {
        // Get tables
        uno::Reference<text::XTextTablesSupplier> xTablesSupp(m_xDocument, uno::UNO_QUERY);
        if (xTablesSupp.is()) {
            uno::Reference<container::XNameAccess> xTables = xTablesSupp->getTextTables();
            if (xTables.is()) {
                const uno::Sequence<OUString> aNames = xTables->getElementNames();
                sal_Int32 idx = 1;
                for (const auto& name : aNames) {
                    DocElement elem;
                    elem.type = "table";
                    elem.id = "table" + OUString::number(idx++);
                    elem.displayName = name;
                    m_docElements.push_back(elem);
                }
            }
        }

        // Get images
        uno::Reference<text::XTextGraphicObjectsSupplier> xGraphicsSupp(m_xDocument, uno::UNO_QUERY);
        if (xGraphicsSupp.is()) {
            uno::Reference<container::XNameAccess> xGraphics = xGraphicsSupp->getGraphicObjects();
            if (xGraphics.is()) {
                const uno::Sequence<OUString> aNames = xGraphics->getElementNames();
                sal_Int32 idx = 1;
                for (const auto& name : aNames) {
                    DocElement elem;
                    elem.type = "image";
                    elem.id = "image" + OUString::number(idx++);
                    elem.displayName = name;
                    m_docElements.push_back(elem);
                }
            }
        }

        // Get sections
        uno::Reference<text::XTextSectionsSupplier> xSectionsSupp(m_xDocument, uno::UNO_QUERY);
        if (xSectionsSupp.is()) {
            uno::Reference<container::XNameAccess> xSections = xSectionsSupp->getTextSections();
            if (xSections.is()) {
                const uno::Sequence<OUString> aNames = xSections->getElementNames();
                sal_Int32 idx = 1;
                for (const auto& name : aNames) {
                    DocElement elem;
                    elem.type = "section";
                    elem.id = "section" + OUString::number(idx++);
                    elem.displayName = name;
                    m_docElements.push_back(elem);
                }
            }
        }
    } catch (...) {}
}

void ChatPanel::CheckConnection() {
    if (m_pAgent) {
        m_connected = m_pAgent->checkConnection();
    }
    if (m_pStatusLabel) {
        m_pStatusLabel->SetText(m_connected ? u"Connected"_ustr : u"Offline - start localhost:8765"_ustr);
    }
    if (m_pModelLabel) {
        m_pModelLabel->SetText(u"Model: GPT-4"_ustr);
    }
}

// ==================== SESSION MANAGEMENT ====================
OUString ChatPanel::CreateNewSession(const OUString& name) {
    ChatSession session;
    session.id = GenerateId();
    session.name = name.isEmpty() ? u"Chat "_ustr + OUString::number(m_nextSessionNum++) : name;

    m_sessions[session.id] = session;
    m_currentSessionId = session.id;

    if (m_pTabControl) {
        sal_uInt16 nPageId = m_pTabControl->GetPageCount() + 1;
        m_pTabControl->InsertPage(nPageId, session.name);
        m_pTabControl->SetCurPageId(nPageId);
    }

    AddMessage(ChatMessage::Role::SYSTEM,
        u"Welcome! I can help you with your document.\n\n"
        "Quick actions: Clean, Review, Summarize, Adapt, Improve, Translate\n\n"
        "Use @ to reference: @selection, @document, @table1, @image1, @section1\n\n"
        "Type a message or click an action button to get started."_ustr);

    RenderMessages();
    return session.id;
}

void ChatPanel::SwitchToSession(const OUString& sessionId) {
    if (m_sessions.find(sessionId) == m_sessions.end()) return;
    m_currentSessionId = sessionId;
    RenderMessages();
    UpdateEditsBar();
}

void ChatPanel::ResetCurrentSession() {
    ChatSession* session = GetCurrentSession();
    if (!session) return;

    session->messages.clear();
    session->pendingEdits.clear();

    AddMessage(ChatMessage::Role::SYSTEM, u"Session reset. How can I help you?"_ustr);
    UpdateEditsBar();
}

ChatSession* ChatPanel::GetCurrentSession() {
    auto it = m_sessions.find(m_currentSessionId);
    return (it != m_sessions.end()) ? &it->second : nullptr;
}

// ==================== EVENT HANDLERS - SESSION ====================
IMPL_LINK_NOARG(ChatPanel, NewSessionClickHdl, Button*, void) {
    CreateNewSession();
}

IMPL_LINK_NOARG(ChatPanel, TabChangedHdl, TabControl*, void) {
    if (!m_pTabControl) return;
    sal_uInt16 nPageId = m_pTabControl->GetCurPageId();
    sal_uInt16 idx = 0;
    for (const auto& [id, session] : m_sessions) {
        if (++idx == nPageId) {
            SwitchToSession(id);
            break;
        }
    }
}

IMPL_LINK_NOARG(ChatPanel, SettingsClickHdl, Button*, void) {
    AddMessage(ChatMessage::Role::SYSTEM, u"Settings dialog coming soon. Configure AI model, API keys, and preferences."_ustr);
}

// ==================== EVENT HANDLERS - QUICK ACTIONS ====================
IMPL_LINK_NOARG(ChatPanel, CleanDocClickHdl, Button*, void) { DoCleanDocument(); }
IMPL_LINK_NOARG(ChatPanel, ReviewErrorsClickHdl, Button*, void) { DoReviewErrors(); }
IMPL_LINK_NOARG(ChatPanel, SummarizeClickHdl, Button*, void) { DoSummarize(); }
IMPL_LINK_NOARG(ChatPanel, AdaptTextClickHdl, Button*, void) { DoAdaptText(); }
IMPL_LINK_NOARG(ChatPanel, ImproveClickHdl, Button*, void) { DoImprove(); }
IMPL_LINK_NOARG(ChatPanel, TranslateClickHdl, Button*, void) { DoTranslate(); }
IMPL_LINK_NOARG(ChatPanel, ExpandClickHdl, Button*, void) { DoExpand(); }
IMPL_LINK_NOARG(ChatPanel, CondenseClickHdl, Button*, void) { DoCondense(); }

// ==================== EVENT HANDLERS - FORMATTING ====================
IMPL_LINK_NOARG(ChatPanel, BoldClickHdl, Button*, void) { ApplyFormatting(u"bold"_ustr); }
IMPL_LINK_NOARG(ChatPanel, ItalicClickHdl, Button*, void) { ApplyFormatting(u"italic"_ustr); }
IMPL_LINK_NOARG(ChatPanel, UnderlineClickHdl, Button*, void) { ApplyFormatting(u"underline"_ustr); }
IMPL_LINK_NOARG(ChatPanel, HeadingClickHdl, Button*, void) { ApplyFormatting(u"heading"_ustr); }
IMPL_LINK_NOARG(ChatPanel, BulletClickHdl, Button*, void) { ApplyFormatting(u"bullet"_ustr); }
IMPL_LINK_NOARG(ChatPanel, NumberClickHdl, Button*, void) { ApplyFormatting(u"number"_ustr); }

// ==================== EVENT HANDLERS - CHAT ====================
IMPL_LINK_NOARG(ChatPanel, SendClickHdl, Button*, void) {
    if (m_pInputField) {
        OUString msg = m_pInputField->GetText();
        if (!msg.isEmpty()) {
            SendMessage(msg);
            m_pInputField->SetText("");
        }
    }
}

IMPL_LINK_NOARG(ChatPanel, AtRefClickHdl, Button*, void) {
    ShowAtReferenceMenu();
}

IMPL_LINK_NOARG(ChatPanel, AttachClickHdl, Button*, void) {
    AddMessage(ChatMessage::Role::SYSTEM, u"File attachment coming soon. You'll be able to attach images and documents."_ustr);
}

// ==================== EVENT HANDLERS - ACTIONS ====================
IMPL_LINK_NOARG(ChatPanel, InsertClickHdl, Button*, void) {
    ChatSession* session = GetCurrentSession();
    if (!session) return;

    for (auto it = session->messages.rbegin(); it != session->messages.rend(); ++it) {
        if (it->role == ChatMessage::Role::ASSISTANT) {
            InsertAtCursor(it->content);
            AddMessage(ChatMessage::Role::SYSTEM, u"Text inserted at cursor."_ustr);
            return;
        }
    }
    AddMessage(ChatMessage::Role::SYSTEM, u"No AI response to insert."_ustr);
}

IMPL_LINK_NOARG(ChatPanel, ReplaceClickHdl, Button*, void) {
    ChatSession* session = GetCurrentSession();
    if (!session) return;

    OUString selected = GetSelectedText();
    if (selected.isEmpty()) {
        AddMessage(ChatMessage::Role::SYSTEM, u"Select text in the document first."_ustr);
        return;
    }

    for (auto it = session->messages.rbegin(); it != session->messages.rend(); ++it) {
        if (it->role == ChatMessage::Role::ASSISTANT) {
            ReplaceSelection(it->content);
            AddMessage(ChatMessage::Role::SYSTEM, u"Selection replaced."_ustr);
            return;
        }
    }
    AddMessage(ChatMessage::Role::SYSTEM, u"No AI response to use."_ustr);
}

IMPL_LINK_NOARG(ChatPanel, ResetClickHdl, Button*, void) {
    ResetCurrentSession();
}

IMPL_LINK_NOARG(ChatPanel, StopClickHdl, Button*, void) {
    m_processing = false;
    SetProcessing(false);
    AddMessage(ChatMessage::Role::SYSTEM, u"Processing stopped."_ustr);
}

// ==================== EVENT HANDLERS - EDITS ====================
IMPL_LINK_NOARG(ChatPanel, AcceptAllClickHdl, Button*, void) { ApproveAllEdits(); }
IMPL_LINK_NOARG(ChatPanel, RejectAllClickHdl, Button*, void) { RejectAllEdits(); }
IMPL_LINK_NOARG(ChatPanel, ReviewClickHdl, Button*, void) {
    AddMessage(ChatMessage::Role::SYSTEM, u"Review dialog coming soon. Preview all pending changes before applying."_ustr);
}

// ==================== CORE OPERATIONS ====================
void ChatPanel::SendMessage(const OUString& message) {
    OUString fullMessage = BuildMessageWithReferences(message);
    AddMessage(ChatMessage::Role::USER, message);
    SendToAgent(fullMessage);
}

void ChatPanel::SendToAgent(const OUString& message) {
    if (!m_pAgent) return;

    SetProcessing(true);

    OUString context = GetDocumentText();
    AgentResponse response = m_pAgent->sendMessage(message, context);

    AddMessage(ChatMessage::Role::ASSISTANT, response.message);

    if (response.hasPatch) {
        PendingEdit edit;
        edit.id = GenerateId();
        edit.editType = response.patchType;
        edit.newText = response.message;
        edit.description = u"AI suggested edit"_ustr;
        AddPendingEdit(edit);
    }

    SetProcessing(false);
}

void ChatPanel::AddMessage(ChatMessage::Role role, const OUString& content) {
    ChatSession* session = GetCurrentSession();
    if (!session) return;

    ChatMessage msg;
    msg.role = role;
    msg.content = content;
    msg.timestamp = GetTimestamp();
    session->messages.push_back(msg);

    RenderMessages();
}

void ChatPanel::AddToolMessage(const OUString& toolName, const OUString& result) {
    ChatSession* session = GetCurrentSession();
    if (!session) return;

    ChatMessage msg;
    msg.role = ChatMessage::Role::TOOL;
    msg.toolName = toolName;
    msg.toolResult = result;
    msg.timestamp = GetTimestamp();
    session->messages.push_back(msg);

    RenderMessages();
}

void ChatPanel::SetProcessing(bool processing) {
    m_processing = processing;
    if (m_pSendButton) m_pSendButton->Enable(!processing);
    if (m_pStopButton) m_pStopButton->Enable(processing);
    if (m_pStatusLabel) m_pStatusLabel->SetText(processing ? u"Processing..."_ustr : (m_connected ? u"Ready"_ustr : u"Offline"_ustr));
}

// ==================== AI QUICK ACTIONS ====================
void ChatPanel::DoCleanDocument() {
    OUString text = GetSelectedText();
    if (text.isEmpty()) text = GetDocumentText();
    if (text.isEmpty()) {
        AddMessage(ChatMessage::Role::SYSTEM, u"No text to clean."_ustr);
        return;
    }
    OUString prompt = u"Clean up the following text: fix spacing issues, remove extra whitespace, "
        "fix paragraph breaks, standardize formatting. Return only the cleaned text:\n\n"_ustr + text;
    AddMessage(ChatMessage::Role::USER, u"[Clean Document]"_ustr);
    SendToAgent(prompt);
}

void ChatPanel::DoReviewErrors() {
    OUString text = GetSelectedText();
    if (text.isEmpty()) text = GetDocumentText();
    if (text.isEmpty()) {
        AddMessage(ChatMessage::Role::SYSTEM, u"No text to review."_ustr);
        return;
    }
    OUString prompt = u"Review the following text for grammar, spelling, and punctuation errors. "
        "List each error found and provide the corrected version:\n\n"_ustr + text;
    AddMessage(ChatMessage::Role::USER, u"[Review Errors]"_ustr);
    SendToAgent(prompt);
}

void ChatPanel::DoSummarize() {
    OUString text = GetSelectedText();
    if (text.isEmpty()) text = GetDocumentText();
    if (text.isEmpty()) {
        AddMessage(ChatMessage::Role::SYSTEM, u"No text to summarize."_ustr);
        return;
    }
    OUString prompt = u"Provide a concise summary of the following text. "
        "Include key points and main ideas:\n\n"_ustr + text;
    AddMessage(ChatMessage::Role::USER, u"[Summarize]"_ustr);
    SendToAgent(prompt);
}

void ChatPanel::DoAdaptText() {
    OUString text = GetSelectedText();
    if (text.isEmpty()) {
        AddMessage(ChatMessage::Role::SYSTEM, u"Select text to adapt."_ustr);
        return;
    }
    OUString prompt = u"Adapt the following text to be more professional and formal. "
        "Maintain the core meaning but improve the tone:\n\n"_ustr + text;
    AddMessage(ChatMessage::Role::USER, u"[Adapt Text]"_ustr);
    SendToAgent(prompt);
}

void ChatPanel::DoImprove() {
    OUString text = GetSelectedText();
    if (text.isEmpty()) {
        AddMessage(ChatMessage::Role::SYSTEM, u"Select text to improve."_ustr);
        return;
    }
    OUString prompt = u"Improve the following text to make it clearer, more engaging, and better written. "
        "Keep the same meaning but enhance quality:\n\n"_ustr + text;
    AddMessage(ChatMessage::Role::USER, u"[Improve]"_ustr);
    SendToAgent(prompt);
}

void ChatPanel::DoTranslate() {
    OUString text = GetSelectedText();
    if (text.isEmpty()) {
        AddMessage(ChatMessage::Role::SYSTEM, u"Select text to translate."_ustr);
        return;
    }
    OUString prompt = u"Translate the following text to English. If already in English, translate to French:\n\n"_ustr + text;
    AddMessage(ChatMessage::Role::USER, u"[Translate]"_ustr);
    SendToAgent(prompt);
}

void ChatPanel::DoExpand() {
    OUString text = GetSelectedText();
    if (text.isEmpty()) {
        AddMessage(ChatMessage::Role::SYSTEM, u"Select text to expand."_ustr);
        return;
    }
    OUString prompt = u"Expand the following text with more details, examples, and explanations "
        "while maintaining the original tone and style:\n\n"_ustr + text;
    AddMessage(ChatMessage::Role::USER, u"[Expand]"_ustr);
    SendToAgent(prompt);
}

void ChatPanel::DoCondense() {
    OUString text = GetSelectedText();
    if (text.isEmpty()) {
        AddMessage(ChatMessage::Role::SYSTEM, u"Select text to condense."_ustr);
        return;
    }
    OUString prompt = u"Condense the following text to be shorter and more concise. "
        "Remove unnecessary words while keeping all important information:\n\n"_ustr + text;
    AddMessage(ChatMessage::Role::USER, u"[Condense]"_ustr);
    SendToAgent(prompt);
}

// ==================== @ REFERENCE SYSTEM ====================
void ChatPanel::ShowAtReferenceMenu() {
    RefreshDocumentElements();

    VclPtr<PopupMenu> pMenu = VclPtr<PopupMenu>::Create();
    sal_uInt16 nId = 1;

    pMenu->InsertItem(nId++, "@selection - Selected text");
    pMenu->InsertItem(nId++, "@document - Full document");
    pMenu->InsertSeparator();

    for (const auto& elem : m_docElements) {
        OUString label = "@" + elem.id + " - " + elem.displayName;
        pMenu->InsertItem(nId++, label);
    }

    if (m_docElements.empty()) {
        pMenu->InsertItem(nId, "(No tables, images, or sections found)");
        pMenu->EnableItem(nId, false);
    }

    Point aPos = m_pAtRefButton->GetPosPixel();
    aPos.AdjustY(m_pAtRefButton->GetSizePixel().Height());

    sal_uInt16 nSelected = pMenu->Execute(this, aPos);

    if (nSelected == 1) {
        m_pendingReference = "@selection";
        m_pInputField->SetText(m_pInputField->GetText() + " @selection ");
    } else if (nSelected == 2) {
        m_pendingReference = "@document";
        m_pInputField->SetText(m_pInputField->GetText() + " @document ");
    } else if (nSelected > 2 && nSelected - 3 < m_docElements.size()) {
        OUString ref = "@" + m_docElements[nSelected - 3].id;
        m_pInputField->SetText(m_pInputField->GetText() + " " + ref + " ");
    }

    m_pInputField->GrabFocus();
}

OUString ChatPanel::GetReferenceContent(const OUString& ref) {
    if (ref == "@selection") {
        return GetSelectedText();
    } else if (ref == "@document") {
        return GetDocumentText();
    }
    // TODO: Implement table/image/section content extraction
    return u"[Content of "_ustr + ref + u"]"_ustr;
}

OUString ChatPanel::BuildMessageWithReferences(const OUString& message) {
    OUString result = message;

    // Replace @selection
    if (result.indexOf(u"@selection"_ustr) >= 0) {
        OUString sel = GetSelectedText();
        if (!sel.isEmpty()) {
            OUString replacement = u"\n[Selection: \""_ustr + sel + u"\"]\n"_ustr;
            result = result.replaceAll(u"@selection"_ustr, replacement);
        }
    }

    // Replace @document
    if (result.indexOf(u"@document"_ustr) >= 0) {
        OUString doc = GetDocumentText();
        if (!doc.isEmpty()) {
            // Truncate if too long
            if (doc.getLength() > 2000) {
                doc = doc.copy(0, 2000) + u"... [truncated]"_ustr;
            }
            OUString replacement = u"\n[Document:\n"_ustr + doc + u"\n]\n"_ustr;
            result = result.replaceAll(u"@document"_ustr, replacement);
        }
    }

    // Replace other references
    for (const auto& elem : m_docElements) {
        OUString ref = u"@"_ustr + elem.id;
        if (result.indexOf(ref) >= 0) {
            OUString content = GetReferenceContent(ref);
            OUString replacement = u"\n["_ustr + elem.type + u" "_ustr + elem.id + u": "_ustr + content + u"]\n"_ustr;
            result = result.replaceAll(ref, replacement);
        }
    }

    return result;
}

// ==================== DOCUMENT OPERATIONS ====================
OUString ChatPanel::GetSelectedText() {
    if (m_pDocController) return m_pDocController->getSelectedText();
    return OUString();
}

OUString ChatPanel::GetDocumentText() {
    if (m_pDocController) return m_pDocController->getDocumentText();
    return OUString();
}

void ChatPanel::InsertAtCursor(const OUString& text) {
    if (!m_pDocController || text.isEmpty()) return;

    try {
        uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
        uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(xContext);
        uno::Reference<frame::XFrame> xFrame = xDesktop->getCurrentFrame();
        if (xFrame.is()) {
            uno::Reference<frame::XController> xController = xFrame->getController();
            uno::Reference<text::XTextViewCursorSupplier> xCursorSupplier(xController, uno::UNO_QUERY);
            if (xCursorSupplier.is()) {
                uno::Reference<text::XTextViewCursor> xViewCursor = xCursorSupplier->getViewCursor();
                if (xViewCursor.is()) {
                    xViewCursor->setString(text);
                    return;
                }
            }
        }
        m_pDocController->insertText(text, m_pDocController->getCursorPosition());
    } catch (...) {}
}

void ChatPanel::ReplaceSelection(const OUString& text) {
    if (m_pDocController) m_pDocController->replaceSelection(text);
}

void ChatPanel::ApplyFormatting(const OUString& formatType) {
    InitializeDocument();
    if (!m_pDocController || !m_pDocController->hasDocument()) {
        AddMessage(ChatMessage::Role::SYSTEM, u"No document available."_ustr);
        return;
    }

    if (formatType == u"bold"_ustr) m_pDocController->applyBold();
    else if (formatType == u"italic"_ustr) m_pDocController->applyItalic();
    else if (formatType == u"underline"_ustr) m_pDocController->applyUnderline();
    else if (formatType == u"heading"_ustr) m_pDocController->applyHeading(1);
    else if (formatType == u"bullet"_ustr) m_pDocController->applyBulletList();
    else if (formatType == u"number"_ustr) m_pDocController->applyNumberedList();
}

// ==================== PENDING EDITS ====================
void ChatPanel::AddPendingEdit(const PendingEdit& edit) {
    ChatSession* session = GetCurrentSession();
    if (!session) return;
    session->pendingEdits.push_back(edit);
    UpdateEditsBar();
}

void ChatPanel::ApproveEdit(const OUString& editId) {
    ChatSession* session = GetCurrentSession();
    if (!session) return;

    for (auto& edit : session->pendingEdits) {
        if (edit.id == editId) {
            edit.approved = true;
            ApplyEdit(edit);
            break;
        }
    }
    UpdateEditsBar();
}

void ChatPanel::RejectEdit(const OUString& editId) {
    ChatSession* session = GetCurrentSession();
    if (!session) return;

    for (auto& edit : session->pendingEdits) {
        if (edit.id == editId) {
            edit.rejected = true;
            break;
        }
    }
    UpdateEditsBar();
}

void ChatPanel::ApproveAllEdits() {
    ChatSession* session = GetCurrentSession();
    if (!session) return;

    for (auto& edit : session->pendingEdits) {
        if (!edit.approved && !edit.rejected) {
            edit.approved = true;
            ApplyEdit(edit);
        }
    }
    UpdateEditsBar();
    AddMessage(ChatMessage::Role::SYSTEM, u"All edits applied."_ustr);
}

void ChatPanel::RejectAllEdits() {
    ChatSession* session = GetCurrentSession();
    if (!session) return;

    for (auto& edit : session->pendingEdits) {
        if (!edit.approved && !edit.rejected) {
            edit.rejected = true;
        }
    }
    UpdateEditsBar();
    AddMessage(ChatMessage::Role::SYSTEM, u"All edits rejected."_ustr);
}

void ChatPanel::ApplyEdit(const PendingEdit& edit) {
    if (!m_pDocController) return;

    if (edit.editType == "INSERT") {
        InsertAtCursor(edit.newText);
    } else if (edit.editType == "REPLACE") {
        ReplaceSelection(edit.newText);
    }
}

void ChatPanel::UpdateEditsBar() {
    ChatSession* session = GetCurrentSession();
    sal_Int32 count = 0;

    if (session) {
        for (const auto& edit : session->pendingEdits) {
            if (!edit.approved && !edit.rejected) count++;
        }
    }

    if (m_pEditsLabel) {
        OUString label = OUString::number(count) + " pending";
        m_pEditsLabel->SetText(label);
    }

    DoLayout();
    RenderMessages();
}

void ChatPanel::RenderMessages() {
    Invalidate();
}

// ==================== LIFECYCLE ====================
void ChatPanel::StateChanged(StateChangedType nType) {
    DockingWindow::StateChanged(nType);
    if (nType == StateChangedType::InitShow) {
        InitializeDocument();
    }
}

void ChatPanel::DataChanged(const DataChangedEvent& rDCEvt) {
    DockingWindow::DataChanged(rDCEvt);
}

// ==================== PAINT ====================
void ChatPanel::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) {
    DockingWindow::Paint(rRenderContext, rRect);

    if (!m_pChatHistory) return;

    ChatSession* session = GetCurrentSession();
    if (!session) return;

    Point chatPos = m_pChatHistory->GetPosPixel();
    Size chatSize = m_pChatHistory->GetSizePixel();

    // Background
    rRenderContext.SetFillColor(Colors::BG_DARK);
    rRenderContext.SetLineColor(Colors::BG_PANEL);
    rRenderContext.DrawRect(tools::Rectangle(chatPos, chatSize));

    // Messages
    Point pos(chatPos.X() + 8, chatPos.Y() + 8);
    const sal_Int32 lineH = 16;
    const sal_Int32 msgPad = 8;
    const sal_Int32 maxW = chatSize.Width() - 30;

    for (const auto& msg : session->messages) {
        Color bgColor, textColor;
        OUString prefix;

        switch (msg.role) {
            case ChatMessage::Role::USER:
                bgColor = Colors::BG_USER_MSG;
                textColor = Colors::TEXT_USER;
                prefix = u"You: "_ustr;
                break;
            case ChatMessage::Role::ASSISTANT:
                bgColor = Colors::BG_AI_MSG;
                textColor = Colors::TEXT_AI;
                prefix = u"AI: "_ustr;
                break;
            case ChatMessage::Role::TOOL:
                bgColor = Colors::BG_TOOL_MSG;
                textColor = Colors::TEXT_TOOL;
                prefix = u"Tool: "_ustr + msg.toolName + u" "_ustr;
                break;
            default:
                bgColor = Colors::BG_DARK;
                textColor = Colors::TEXT_SYSTEM;
                prefix = u""_ustr;
        }

        // Calculate height
        sal_Int32 lines = 1;
        OUString content = msg.content;
        sal_Int32 idx = 0;
        while (idx < content.getLength()) {
            sal_Int32 nl = content.indexOf('\n', idx);
            if (nl >= 0) { lines++; idx = nl + 1; }
            else idx = content.getLength();
        }
        sal_Int32 bubbleH = lineH * std::min(lines + 1, sal_Int32(8)) + msgPad * 2;

        // Draw bubble
        rRenderContext.SetFillColor(bgColor);
        rRenderContext.SetLineColor();
        tools::Rectangle bubble(pos, Size(maxW, bubbleH));
        rRenderContext.DrawRect(bubble);

        // Header
        rRenderContext.SetTextColor(textColor);
        rRenderContext.DrawText(Point(pos.X() + msgPad, pos.Y() + msgPad), prefix + msg.timestamp);

        // Content
        rRenderContext.SetTextColor(Colors::TEXT_PRIMARY);
        Point textPos(pos.X() + msgPad, pos.Y() + msgPad + lineH);
        idx = 0;
        sal_Int32 lineCount = 0;
        while (idx < content.getLength() && lineCount < 6) {
            sal_Int32 endIdx = std::min(idx + 55, content.getLength());
            sal_Int32 nl = content.indexOf('\n', idx);
            if (nl >= 0 && nl < endIdx) endIdx = nl;

            OUString line = content.copy(idx, endIdx - idx);
            rRenderContext.DrawText(textPos, line);
            textPos.AdjustY(lineH);
            lineCount++;

            idx = endIdx;
            if (nl >= 0 && nl == endIdx) idx++;
        }

        if (lineCount >= 6 && idx < content.getLength()) {
            rRenderContext.SetTextColor(Colors::TEXT_SECONDARY);
            rRenderContext.DrawText(textPos, u"... [more]"_ustr);
        }

        pos.AdjustY(bubbleH + 6);

        if (pos.Y() > chatPos.Y() + chatSize.Height() - lineH * 2)
            break;
    }

    // Empty state
    if (session->messages.empty()) {
        rRenderContext.SetTextColor(Colors::TEXT_SECONDARY);
        Point center(chatPos.X() + 20, chatPos.Y() + 30);
        rRenderContext.DrawText(center, u"Welcome to OfficeLabs AI Assistant!"_ustr);
    }
}

} // namespace officelabs

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
