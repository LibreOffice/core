/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of OfficeLabs.
 */

#include <officelabs/DocumentController.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/sheet/XSheetCellCursor.hpp>
#include <com/sun/star/sheet/XUsedAreaCursor.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

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

void DocumentController::setCalcDocument(const uno::Reference<sheet::XSpreadsheetDocument>& xDoc)
{
    m_xCalcDoc = xDoc;
    m_xDocument = uno::Reference<text::XTextDocument>(); // clear Writer ref
    m_xText = uno::Reference<text::XText>();
    m_xImpressDoc = uno::Reference<drawing::XDrawPagesSupplier>(); // clear Impress ref

    if (!m_xCalcDoc.is())
        return;

    uno::Reference<frame::XModel> xModel(m_xCalcDoc, uno::UNO_QUERY);
    if (xModel.is())
        m_xController = xModel->getCurrentController();
}

void DocumentController::setImpressDocument(const uno::Reference<drawing::XDrawPagesSupplier>& xDoc)
{
    m_xImpressDoc = xDoc;
    m_xDocument = uno::Reference<text::XTextDocument>(); // clear Writer ref
    m_xText = uno::Reference<text::XText>();
    m_xCalcDoc = uno::Reference<sheet::XSpreadsheetDocument>(); // clear Calc ref

    if (!m_xImpressDoc.is())
        return;

    uno::Reference<frame::XModel> xModel(m_xImpressDoc, uno::UNO_QUERY);
    if (xModel.is())
        m_xController = xModel->getCurrentController();
}

void DocumentController::setModel(const uno::Reference<frame::XModel>& xModel)
{
    m_xModel = xModel;
    if (xModel.is())
        m_xController = xModel->getCurrentController();
}

OUString DocumentController::getApplicationType()
{
    if (!m_sAppType.isEmpty())
        return m_sAppType;
    return u"writer"_ustr;
}

OUString DocumentController::getDocumentText()
{
    // Writer path
    if (m_xText.is())
        return m_xText->getString();

    // Calc path -- read active sheet cells as tab-separated text
    if (m_xCalcDoc.is())
    {
        try
        {
            uno::Reference<sheet::XSpreadsheets> xSheets = m_xCalcDoc->getSheets();
            uno::Reference<container::XIndexAccess> xSheetsIdx(xSheets, uno::UNO_QUERY);
            if (!xSheetsIdx.is() || xSheetsIdx->getCount() == 0)
                return OUString();

            // Read first (active) sheet
            uno::Reference<sheet::XSpreadsheet> xSheet;
            xSheetsIdx->getByIndex(0) >>= xSheet;
            if (!xSheet.is())
                return OUString();

            // Find used range via cursor
            uno::Reference<sheet::XSheetCellCursor> xCursor = xSheet->createCursor();
            uno::Reference<sheet::XUsedAreaCursor> xUsed(xCursor, uno::UNO_QUERY);
            if (!xUsed.is())
                return OUString();

            xUsed->gotoStartOfUsedArea(false);
            xUsed->gotoEndOfUsedArea(true);

            uno::Reference<sheet::XCellRangeAddressable> xAddr(xCursor, uno::UNO_QUERY);
            if (!xAddr.is())
                return OUString();

            auto addr = xAddr->getRangeAddress();
            sal_Int32 maxRows = std::min(
                static_cast<sal_Int32>(addr.EndRow - addr.StartRow + 1),
                sal_Int32(500));
            sal_Int32 nCols = addr.EndColumn - addr.StartColumn + 1;

            OUStringBuffer aBuf;
            for (sal_Int32 r = 0; r < maxRows; ++r)
            {
                for (sal_Int32 c = 0; c < nCols; ++c)
                {
                    if (c > 0)
                        aBuf.append('\t');

                    uno::Reference<table::XCell> xCell = xSheet->getCellByPosition(
                        addr.StartColumn + c, addr.StartRow + r);
                    if (xCell.is())
                    {
                        // Get string which preserves text and formulas
                        uno::Reference<text::XText> xCellText(xCell, uno::UNO_QUERY);
                        if (xCellText.is())
                            aBuf.append(xCellText->getString());
                    }
                }
                aBuf.append('\n');
            }
            return aBuf.makeStringAndClear();
        }
        catch (const uno::Exception&)
        {
            SAL_WARN("officelabs.cef", "getDocumentText: Calc read failed");
            return OUString();
        }
    }

    // Impress path -- read all slide text
    if (m_xImpressDoc.is())
    {
        try
        {
            uno::Reference<drawing::XDrawPages> xPages = m_xImpressDoc->getDrawPages();
            if (!xPages.is())
                return OUString();

            sal_Int32 nPages = xPages->getCount();
            OUStringBuffer aBuf;

            for (sal_Int32 p = 0; p < nPages; ++p)
            {
                uno::Reference<drawing::XDrawPage> xPage;
                xPages->getByIndex(p) >>= xPage;
                if (!xPage.is())
                    continue;

                aBuf.append("--- Slide ");
                aBuf.append(p + 1);
                aBuf.append(" ---\n");

                sal_Int32 nShapes = xPage->getCount();
                for (sal_Int32 s = 0; s < nShapes; ++s)
                {
                    uno::Reference<drawing::XShape> xShape;
                    xPage->getByIndex(s) >>= xShape;
                    if (!xShape.is())
                        continue;

                    uno::Reference<text::XText> xShapeText(xShape, uno::UNO_QUERY);
                    if (xShapeText.is())
                    {
                        OUString sText = xShapeText->getString();
                        if (!sText.isEmpty())
                        {
                            aBuf.append(sText);
                            aBuf.append('\n');
                        }
                    }
                }
            }
            return aBuf.makeStringAndClear();
        }
        catch (const uno::Exception&)
        {
            SAL_WARN("officelabs.cef", "getDocumentText: Impress read failed");
            return OUString();
        }
    }

    return OUString();
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
    if (xRanges.is())
    {
        const sal_Int32 nCount = xRanges->getCount();

        if (nCount == 1)
        {
            xRanges->getByIndex(0) >>= xRange;
            if (xRange.is())
                return xRange->getString();
        }
        else if (nCount > 1)
        {
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

            if (aBuf.getLength() > 0)
                return aBuf.makeStringAndClear();
        }
    }

    // Calc: selection may be a cell range
    if (m_xCalcDoc.is())
    {
        uno::Reference<table::XCellRange> xCellRange;
        aSelection >>= xCellRange;
        if (xCellRange.is())
        {
            try
            {
                uno::Reference<sheet::XCellRangeAddressable> xAddr(xCellRange, uno::UNO_QUERY);
                if (xAddr.is())
                {
                    auto addr = xAddr->getRangeAddress();
                    sal_Int32 nRows = addr.EndRow - addr.StartRow + 1;
                    sal_Int32 nCalcCols = addr.EndColumn - addr.StartColumn + 1;
                    sal_Int32 maxRows = std::min(nRows, sal_Int32(200));

                    OUStringBuffer aCalcBuf;
                    for (sal_Int32 r = 0; r < maxRows; ++r)
                    {
                        for (sal_Int32 c = 0; c < nCalcCols; ++c)
                        {
                            if (c > 0) aCalcBuf.append('\t');
                            uno::Reference<table::XCell> xCell = xCellRange->getCellByPosition(c, r);
                            if (xCell.is())
                            {
                                uno::Reference<text::XText> xCellText(xCell, uno::UNO_QUERY);
                                if (xCellText.is())
                                    aCalcBuf.append(xCellText->getString());
                            }
                        }
                        aCalcBuf.append('\n');
                    }
                    return aCalcBuf.makeStringAndClear();
                }
            }
            catch (const uno::Exception&)
            {
                SAL_WARN("officelabs.cef", "getSelectedText: Calc selection read failed");
            }
        }
    }

    // Impress: selection may be shape(s)
    if (m_xImpressDoc.is())
    {
        // Try single shape
        uno::Reference<drawing::XShape> xShape;
        aSelection >>= xShape;
        if (xShape.is())
        {
            uno::Reference<text::XText> xShapeText(xShape, uno::UNO_QUERY);
            if (xShapeText.is())
                return xShapeText->getString();
        }

        // Try collection of shapes
        uno::Reference<container::XIndexAccess> xShapes;
        aSelection >>= xShapes;
        if (xShapes.is())
        {
            OUStringBuffer aShapeBuf;
            for (sal_Int32 i = 0; i < xShapes->getCount(); ++i)
            {
                uno::Reference<drawing::XShape> xCurShape;
                xShapes->getByIndex(i) >>= xCurShape;
                if (!xCurShape.is()) continue;

                uno::Reference<text::XText> xShapeText(xCurShape, uno::UNO_QUERY);
                if (xShapeText.is())
                {
                    OUString sText = xShapeText->getString();
                    if (!sText.isEmpty())
                    {
                        if (aShapeBuf.getLength() > 0) aShapeBuf.append('\n');
                        aShapeBuf.append(sText);
                    }
                }
            }
            return aShapeBuf.makeStringAndClear();
        }
    }

    return OUString();
}

} // namespace officelabs

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
