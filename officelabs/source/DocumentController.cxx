/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of OfficeLabs.
 */

#include <officelabs/DocumentController.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <rtl/ustrbuf.hxx>

using namespace css;

namespace officelabs {

DocumentController::DocumentController()
{
}

DocumentController::~DocumentController()
{
}

void DocumentController::setDocument(const uno::Reference<text::XTextDocument>& xDoc)
{
    m_xDocument = xDoc;
    m_xText = uno::Reference<text::XText>();
    m_xController = uno::Reference<frame::XController>();

    if (!m_xDocument.is())
        return;

    m_xText = m_xDocument->getText();

    uno::Reference<frame::XModel> xModel(m_xDocument, uno::UNO_QUERY);
    if (xModel.is())
        m_xController = xModel->getCurrentController();
}

OUString DocumentController::getApplicationType()
{
    // CEF sidebar is currently wired in Writer.
    return u"writer"_ustr;
}

OUString DocumentController::getDocumentText()
{
    if (!m_xText.is())
        return OUString();

    return m_xText->getString();
}

OUString DocumentController::getSelectedText()
{
    if (!m_xController.is())
        return OUString();

    uno::Reference<view::XSelectionSupplier> xSelSupplier(m_xController, uno::UNO_QUERY);
    if (!xSelSupplier.is())
        return OUString();

    uno::Any aSelection = xSelSupplier->getSelection();

    // Simple case: a direct text range.
    uno::Reference<text::XTextRange> xRange;
    aSelection >>= xRange;
    if (xRange.is())
        return xRange->getString();

    // Writer often returns a collection of text ranges.
    uno::Reference<container::XIndexAccess> xRanges;
    aSelection >>= xRanges;
    if (!xRanges.is())
        return OUString();

    const sal_Int32 nCount = xRanges->getCount();
    if (nCount <= 0)
        return OUString();

    if (nCount == 1)
    {
        xRanges->getByIndex(0) >>= xRange;
        if (xRange.is())
            return xRange->getString();
        return OUString();
    }

    OUStringBuffer aBuf;
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        uno::Reference<text::XTextRange> xCurrentRange;
        xRanges->getByIndex(i) >>= xCurrentRange;
        if (!xCurrentRange.is())
            continue;

        if (aBuf.getLength() > 0)
            aBuf.append('\n');
        aBuf.append(xCurrentRange->getString());
    }

    return aBuf.makeStringAndClear();
}

} // namespace officelabs

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
