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
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <rtl/ustring.hxx>

namespace officelabs {

class DocumentController
{
private:
    css::uno::Reference<css::text::XTextDocument> m_xDocument;
    css::uno::Reference<css::text::XText> m_xText;
    css::uno::Reference<css::frame::XController> m_xController;
    css::uno::Reference<css::frame::XModel> m_xModel;
    css::uno::Reference<css::sheet::XSpreadsheetDocument> m_xCalcDoc;
    css::uno::Reference<css::drawing::XDrawPagesSupplier> m_xImpressDoc;
    OUString m_sAppType;  // "writer", "calc", "impress", or "draw"

public:
    DocumentController();
    ~DocumentController();

    void setDocument(const css::uno::Reference<css::text::XTextDocument>& xDoc);
    void setCalcDocument(const css::uno::Reference<css::sheet::XSpreadsheetDocument>& xDoc);
    void setImpressDocument(const css::uno::Reference<css::drawing::XDrawPagesSupplier>& xDoc);
    void setModel(const css::uno::Reference<css::frame::XModel>& xModel);
    void setAppType(const OUString& sAppType) { m_sAppType = sAppType; }
    bool hasDocument() const { return m_xDocument.is() || m_xCalcDoc.is() || m_xImpressDoc.is() || !m_sAppType.isEmpty(); }

    OUString getApplicationType();
    OUString getDocumentText();
    OUString getSelectedText();
};

} // namespace officelabs

#endif // INCLUDED_OFFICELABS_DOCUMENTCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
