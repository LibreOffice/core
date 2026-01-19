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

private:
    css::uno::Reference<css::beans::XPropertySet> getSelectionPropertySet();
    void refreshCursor();
};

} // namespace officelabs

#endif // INCLUDED_OFFICELABS_DOCUMENTCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
