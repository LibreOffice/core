#ifndef INCLUDED_OFFICELABS_DOCUMENTCONTROLLER_HXX
#define INCLUDED_OFFICELABS_DOCUMENTCONTROLLER_HXX

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <rtl/ustring.hxx>

namespace officelabs {

class DocumentController {
private:
    css::uno::Reference<css::text::XTextDocument> m_xDocument;
    css::uno::Reference<css::text::XText> m_xText;

public:
    DocumentController();
    ~DocumentController();
    
    void setDocument(const css::uno::Reference<css::text::XTextDocument>& xDoc);
    
    OUString getDocumentText();
    OUString getSelection();
    void insertText(const OUString& text, sal_Int32 position);
    void replaceText(sal_Int32 start, sal_Int32 end, const OUString& newText);
    void deleteText(sal_Int32 start, sal_Int32 end);
    
    bool hasDocument() const { return m_xDocument.is(); }
};

} // namespace officelabs

#endif
