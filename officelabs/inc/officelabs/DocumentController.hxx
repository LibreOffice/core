/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of OfficeLabs.
 *
 * Minimal DocumentController used by the CEF sidebar fast path.
 * Keeps direct UNO reads for document text and selection to avoid
 * additional network/agent hops in the UI loop.
 */

#ifndef INCLUDED_OFFICELABS_DOCUMENTCONTROLLER_HXX
#define INCLUDED_OFFICELABS_DOCUMENTCONTROLLER_HXX

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <rtl/ustring.hxx>

namespace officelabs {

class DocumentController
{
private:
    css::uno::Reference<css::text::XTextDocument> m_xDocument;
    css::uno::Reference<css::text::XText> m_xText;
    css::uno::Reference<css::frame::XController> m_xController;
    OUString m_sAppType;  // "writer", "calc", "impress", or "draw"

public:
    DocumentController();
    ~DocumentController();

    void setDocument(const css::uno::Reference<css::text::XTextDocument>& xDoc);
    void setAppType(const OUString& sAppType) { m_sAppType = sAppType; }
    bool hasDocument() const { return m_xDocument.is() || !m_sAppType.isEmpty(); }

    OUString getApplicationType();
    OUString getDocumentText();
    OUString getSelectedText();
};

} // namespace officelabs

#endif // INCLUDED_OFFICELABS_DOCUMENTCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
