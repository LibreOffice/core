#include <officelabs/DocumentController.hxx>
#include <com/sun/star/text/XText.hpp>

using namespace css;

namespace officelabs {

DocumentController::DocumentController() {
}

DocumentController::~DocumentController() {
}

void DocumentController::setDocument(const uno::Reference<text::XTextDocument>& xDoc) {
    m_xDocument = xDoc;
}

OUString DocumentController::getDocumentText() {
    if (!m_xDocument.is()) {
        return OUString();
    }
    
    try {
        uno::Reference<text::XText> xText = m_xDocument->getText();
        if (xText.is()) {
            return xText->getString();
        }
    } catch (...) {
        // Ignore errors for minimal stub
    }
    
    return OUString();
}

} // namespace officelabs