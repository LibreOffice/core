#ifndef INCLUDED_OFFICELABS_DOCUMENTCONTROLLER_HXX
#define INCLUDED_OFFICELABS_DOCUMENTCONTROLLER_HXX

#include <com/sun/star/text/XTextDocument.hpp>
#include <rtl/ustring.hxx>
#include <sfx2/dllapi.h>

namespace officelabs {

class SFX2_DLLPUBLIC DocumentController {
private:
    css::uno::Reference<css::text::XTextDocument> m_xDocument;

public:
    DocumentController();
    ~DocumentController();
    
    void setDocument(const css::uno::Reference<css::text::XTextDocument>& xDoc);
    OUString getDocumentText();
    bool hasDocument() const { return m_xDocument.is(); }
};

} // namespace officelabs

#endif