#include <officelabs/DocumentController.hxx>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextRange.hpp>

using namespace css;

namespace officelabs {

DocumentController::DocumentController() {
}

DocumentController::~DocumentController() {
}

void DocumentController::setDocument(const uno::Reference<text::XTextDocument>& xDoc) {
    m_xDocument = xDoc;
    if (m_xDocument.is()) {
        m_xText = m_xDocument->getText();
    }
}

OUString DocumentController::getDocumentText() {
    if (!m_xText.is()) {
        return OUString();
    }
    return m_xText->getString();
}

OUString DocumentController::getSelection() {
    // TODO: Get actual selection from view
    return OUString();
}

void DocumentController::insertText(const OUString& text, sal_Int32 position) {
    if (!m_xText.is()) return;
    
    uno::Reference<text::XTextCursor> xCursor = m_xText->createTextCursor();
    if (!xCursor.is()) return;
    
    xCursor->gotoStart(false);
    xCursor->goRight(position, false);
    m_xText->insertString(xCursor, text, false);
}

void DocumentController::replaceText(sal_Int32 start, sal_Int32 end, const OUString& newText) {
    if (!m_xText.is()) return;
    
    uno::Reference<text::XTextCursor> xCursor = m_xText->createTextCursor();
    if (!xCursor.is()) return;
    
    xCursor->gotoStart(false);
    xCursor->goRight(start, false);
    xCursor->goRight(end - start, true);
    xCursor->setString(newText);
}

void DocumentController::deleteText(sal_Int32 start, sal_Int32 end) {
    replaceText(start, end, OUString());
}

} // namespace officelabs
