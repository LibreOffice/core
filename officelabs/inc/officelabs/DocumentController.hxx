/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of OfficeLabs.
 *
 * DocumentController - Bridge to LibreOffice UNO API for document manipulation
 */

#ifndef INCLUDED_OFFICELABS_DOCUMENTCONTROLLER_HXX
#define INCLUDED_OFFICELABS_DOCUMENTCONTROLLER_HXX

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <rtl/ustring.hxx>
#include <vector>

namespace officelabs {

/**
 * DocumentController - Provides document manipulation via UNO API
 *
 * This class wraps LibreOffice's UNO API to provide:
 * - Document content access (read/write)
 * - Selection manipulation
 * - Text formatting (bold, italic, headings, etc.)
 * - Cursor position tracking
 */
class DocumentController {
private:
    css::uno::Reference<css::text::XTextDocument> m_xDocument;
    css::uno::Reference<css::text::XText> m_xText;
    css::uno::Reference<css::text::XTextCursor> m_xCursor;
    css::uno::Reference<css::frame::XController> m_xController;

public:
    DocumentController();
    ~DocumentController();

    // Document binding
    void setDocument(const css::uno::Reference<css::text::XTextDocument>& xDoc);
    bool hasDocument() const { return m_xDocument.is(); }

    // Document info
    OUString getDocumentTitle();
    OUString getApplicationType();  // "writer", "calc", "impress"
    OUString getDocumentURL();

    // Content access
    OUString getDocumentText();
    OUString getSelectedText();
    sal_Int32 getCursorPosition();

    // Text manipulation
    void insertText(const OUString& text, sal_Int32 position);
    void replaceText(sal_Int32 start, sal_Int32 end, const OUString& newText);
    void replaceSelection(const OUString& text);
    void deleteText(sal_Int32 start, sal_Int32 end);

    // Formatting - character styles
    void applyBold();
    void applyItalic();
    void applyUnderline();
    void setFontSize(float size);
    void setFontName(const OUString& fontName);
    void setTextColor(const OUString& colorHex);

    // Formatting - paragraph styles
    void applyHeading(sal_Int32 level);  // 1-6, 0 = normal
    void applyBulletList();
    void applyNumberedList();
    void setAlignment(const OUString& alignment);  // "left", "center", "right", "justify"

    // Search
    OUString searchText(const OUString& pattern);
    std::vector<sal_Int32> findAllOccurrences(const OUString& text);

    // Phase 3: Collaboration & Export
    /**
     * Export the current document to PDF format.
     *
     * @param outputPath File path for the PDF output
     * @param options Optional export settings (page range, quality, etc.)
     * @return true if export succeeded, false otherwise
     */
    bool exportToPDF(const OUString& outputPath, const OUString& options = "");

    /**
     * Create a snapshot of the current document state for version history.
     *
     * Captures the full document content and metadata for later restoration.
     *
     * @param label Optional user-provided label for the snapshot
     * @return Snapshot ID if successful, empty string otherwise
     */
    OUString createSnapshot(const OUString& label = "");

    /**
     * Restore the document to a previous snapshot state.
     *
     * Replaces current document content with the saved snapshot content.
     * Creates an automatic backup snapshot of the current state before restoring.
     *
     * @param snapshotId The ID of the snapshot to restore
     * @return true if restoration succeeded, false otherwise
     */
    bool restoreSnapshot(const OUString& snapshotId);

    // Phase 4: Advanced AI Features
    /**
     * Get text context around the cursor for inline completions.
     *
     * Extracts a window of text before and after the cursor position
     * to provide context for AI-powered inline completions (Copilot-style).
     *
     * TODO Phase 4 Checkpoint 2:
     * - Extract configurable window size (default 100 chars before/after)
     * - Include cursor position information
     * - Handle edge cases (start/end of document)
     *
     * @param charsBefore Number of characters to extract before cursor (default: 100)
     * @param charsAfter Number of characters to extract after cursor (default: 100)
     * @return JSON string with context: {"before": "...", "after": "...", "position": 123}
     */
    OUString getCursorContext(sal_Int32 charsBefore = 100, sal_Int32 charsAfter = 100);

    /**
     * Insert ghost text completion at cursor position.
     *
     * Inserts AI-generated completion text at the current cursor position.
     * Used for accepting inline completions (Tab key acceptance).
     *
     * TODO Phase 4 Checkpoint 2:
     * - Insert text at cursor without moving cursor initially
     * - Support undo/redo for completion insertion
     * - Optionally format the inserted text to match surrounding style
     *
     * @param completionText The completion text to insert
     * @param moveCursor Whether to move cursor to end of inserted text (default: true)
     * @return true if insertion succeeded, false otherwise
     */
    bool insertCompletion(const OUString& completionText, bool moveCursor = true);

private:
    css::uno::Reference<css::beans::XPropertySet> getSelectionPropertySet();
    void refreshCursor();
};

} // namespace officelabs

#endif // INCLUDED_OFFICELABS_DOCUMENTCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
