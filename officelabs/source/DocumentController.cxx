/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of OfficeLabs.
 *
 * DocumentController implementation - UNO API bridge
 */

#include <officelabs/DocumentController.hxx>

#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/util/XSearchable.hpp>
#include <com/sun/star/util/XSearchDescriptor.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>

#include <comphelper/processfactory.hxx>
#include <tools/color.hxx>

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
    if (m_xDocument.is())
    {
        m_xText = m_xDocument->getText();

        // Get controller for selection access
        uno::Reference<frame::XModel> xModel(m_xDocument, uno::UNO_QUERY);
        if (xModel.is())
        {
            m_xController = xModel->getCurrentController();
        }
    }
}

// Document info

OUString DocumentController::getDocumentTitle()
{
    if (!m_xDocument.is())
        return OUString();

    uno::Reference<frame::XModel> xModel(m_xDocument, uno::UNO_QUERY);
    if (xModel.is())
    {
        return xModel->getTitle();
    }
    return OUString();
}

OUString DocumentController::getApplicationType()
{
    // For now, this controller is Writer-specific
    // TODO: Detect actual application type when supporting Calc/Impress
    return u"writer"_ustr;
}

OUString DocumentController::getDocumentURL()
{
    if (!m_xDocument.is())
        return OUString();

    uno::Reference<frame::XModel> xModel(m_xDocument, uno::UNO_QUERY);
    if (xModel.is())
    {
        return xModel->getURL();
    }
    return OUString();
}

// Content access

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
    uno::Reference<text::XTextRange> xRange;
    aSelection >>= xRange;

    if (xRange.is())
    {
        return xRange->getString();
    }
    return OUString();
}

sal_Int32 DocumentController::getCursorPosition()
{
    if (!m_xController.is())
        return 0;

    uno::Reference<text::XTextViewCursorSupplier> xCursorSupplier(m_xController, uno::UNO_QUERY);
    if (!xCursorSupplier.is())
        return 0;

    uno::Reference<text::XTextViewCursor> xViewCursor = xCursorSupplier->getViewCursor();
    if (!xViewCursor.is())
        return 0;

    // Calculate position from start of document
    uno::Reference<text::XTextRange> xRange(xViewCursor, uno::UNO_QUERY);
    if (!xRange.is() || !m_xText.is())
        return 0;

    // Get text before cursor
    uno::Reference<text::XTextCursor> xCursor = m_xText->createTextCursorByRange(xRange->getStart());
    if (!xCursor.is())
        return 0;

    xCursor->gotoStart(true);
    return xCursor->getString().getLength();
}

// Text manipulation

void DocumentController::insertText(const OUString& text, sal_Int32 position)
{
    if (!m_xText.is())
        return;

    uno::Reference<text::XTextCursor> xCursor = m_xText->createTextCursor();
    if (!xCursor.is())
        return;

    xCursor->gotoStart(false);
    xCursor->goRight(position, false);
    m_xText->insertString(xCursor, text, false);
}

void DocumentController::replaceText(sal_Int32 start, sal_Int32 end, const OUString& newText)
{
    if (!m_xText.is())
        return;

    uno::Reference<text::XTextCursor> xCursor = m_xText->createTextCursor();
    if (!xCursor.is())
        return;

    xCursor->gotoStart(false);
    xCursor->goRight(start, false);
    xCursor->goRight(end - start, true);
    xCursor->setString(newText);
}

void DocumentController::replaceSelection(const OUString& text)
{
    if (!m_xController.is())
        return;

    uno::Reference<view::XSelectionSupplier> xSelSupplier(m_xController, uno::UNO_QUERY);
    if (!xSelSupplier.is())
        return;

    uno::Any aSelection = xSelSupplier->getSelection();
    uno::Reference<text::XTextRange> xRange;
    aSelection >>= xRange;

    if (xRange.is())
    {
        xRange->setString(text);
    }
}

void DocumentController::deleteText(sal_Int32 start, sal_Int32 end)
{
    replaceText(start, end, OUString());
}

// Formatting - character styles

uno::Reference<beans::XPropertySet> DocumentController::getSelectionPropertySet()
{
    if (!m_xController.is())
        return uno::Reference<beans::XPropertySet>();

    uno::Reference<view::XSelectionSupplier> xSelSupplier(m_xController, uno::UNO_QUERY);
    if (!xSelSupplier.is())
        return uno::Reference<beans::XPropertySet>();

    uno::Any aSelection = xSelSupplier->getSelection();
    uno::Reference<beans::XPropertySet> xPropSet;
    aSelection >>= xPropSet;

    return xPropSet;
}

void DocumentController::applyBold()
{
    uno::Reference<beans::XPropertySet> xPropSet = getSelectionPropertySet();
    if (!xPropSet.is())
        return;

    try
    {
        // Toggle bold
        float fWeight = 0;
        xPropSet->getPropertyValue(u"CharWeight"_ustr) >>= fWeight;

        if (fWeight >= awt::FontWeight::BOLD)
            xPropSet->setPropertyValue(u"CharWeight"_ustr, uno::Any(awt::FontWeight::NORMAL));
        else
            xPropSet->setPropertyValue(u"CharWeight"_ustr, uno::Any(awt::FontWeight::BOLD));
    }
    catch (...)
    {
        // Property not available
    }
}

void DocumentController::applyItalic()
{
    uno::Reference<beans::XPropertySet> xPropSet = getSelectionPropertySet();
    if (!xPropSet.is())
        return;

    try
    {
        // Toggle italic
        awt::FontSlant eSlant;
        xPropSet->getPropertyValue(u"CharPosture"_ustr) >>= eSlant;

        if (eSlant == awt::FontSlant_ITALIC)
            xPropSet->setPropertyValue(u"CharPosture"_ustr, uno::Any(awt::FontSlant_NONE));
        else
            xPropSet->setPropertyValue(u"CharPosture"_ustr, uno::Any(awt::FontSlant_ITALIC));
    }
    catch (...)
    {
        // Property not available
    }
}

void DocumentController::applyUnderline()
{
    uno::Reference<beans::XPropertySet> xPropSet = getSelectionPropertySet();
    if (!xPropSet.is())
        return;

    try
    {
        // Toggle underline
        sal_Int16 nUnderline = 0;
        xPropSet->getPropertyValue(u"CharUnderline"_ustr) >>= nUnderline;

        if (nUnderline != awt::FontUnderline::NONE)
            xPropSet->setPropertyValue(u"CharUnderline"_ustr, uno::Any(static_cast<sal_Int16>(awt::FontUnderline::NONE)));
        else
            xPropSet->setPropertyValue(u"CharUnderline"_ustr, uno::Any(static_cast<sal_Int16>(awt::FontUnderline::SINGLE)));
    }
    catch (...)
    {
        // Property not available
    }
}

void DocumentController::setFontSize(float size)
{
    uno::Reference<beans::XPropertySet> xPropSet = getSelectionPropertySet();
    if (!xPropSet.is())
        return;

    try
    {
        xPropSet->setPropertyValue(u"CharHeight"_ustr, uno::Any(size));
    }
    catch (...)
    {
        // Property not available
    }
}

void DocumentController::setFontName(const OUString& fontName)
{
    uno::Reference<beans::XPropertySet> xPropSet = getSelectionPropertySet();
    if (!xPropSet.is())
        return;

    try
    {
        xPropSet->setPropertyValue(u"CharFontName"_ustr, uno::Any(fontName));
    }
    catch (...)
    {
        // Property not available
    }
}

void DocumentController::setTextColor(const OUString& colorHex)
{
    uno::Reference<beans::XPropertySet> xPropSet = getSelectionPropertySet();
    if (!xPropSet.is())
        return;

    try
    {
        // Parse hex color (e.g., "#FF0000" or "FF0000")
        OUString hex = colorHex;
        if (hex.startsWith("#"))
            hex = hex.copy(1);

        sal_Int32 nColor = hex.toInt32(16);
        xPropSet->setPropertyValue(u"CharColor"_ustr, uno::Any(nColor));
    }
    catch (...)
    {
        // Property not available or parse error
    }
}

// Formatting - paragraph styles

void DocumentController::applyHeading(sal_Int32 level)
{
    uno::Reference<beans::XPropertySet> xPropSet = getSelectionPropertySet();
    if (!xPropSet.is())
        return;

    try
    {
        OUString styleName;
        if (level == 0)
            styleName = u"Standard"_ustr;
        else if (level >= 1 && level <= 6)
            styleName = u"Heading "_ustr + OUString::number(level);
        else
            return;

        xPropSet->setPropertyValue(u"ParaStyleName"_ustr, uno::Any(styleName));
    }
    catch (...)
    {
        // Style not available
    }
}

void DocumentController::applyBulletList()
{
    uno::Reference<beans::XPropertySet> xPropSet = getSelectionPropertySet();
    if (!xPropSet.is())
        return;

    try
    {
        // Apply bullet list style
        xPropSet->setPropertyValue(u"NumberingStyleName"_ustr, uno::Any(u"List 1"_ustr));
    }
    catch (...)
    {
        // Style not available
    }
}

void DocumentController::applyNumberedList()
{
    uno::Reference<beans::XPropertySet> xPropSet = getSelectionPropertySet();
    if (!xPropSet.is())
        return;

    try
    {
        // Apply numbered list style
        xPropSet->setPropertyValue(u"NumberingStyleName"_ustr, uno::Any(u"Numbering 123"_ustr));
    }
    catch (...)
    {
        // Style not available
    }
}

void DocumentController::setAlignment(const OUString& alignment)
{
    uno::Reference<beans::XPropertySet> xPropSet = getSelectionPropertySet();
    if (!xPropSet.is())
        return;

    try
    {
        style::ParagraphAdjust eAdjust = style::ParagraphAdjust_LEFT;

        if (alignment == "center")
            eAdjust = style::ParagraphAdjust_CENTER;
        else if (alignment == "right")
            eAdjust = style::ParagraphAdjust_RIGHT;
        else if (alignment == "justify")
            eAdjust = style::ParagraphAdjust_BLOCK;

        xPropSet->setPropertyValue(u"ParaAdjust"_ustr, uno::Any(static_cast<sal_Int16>(eAdjust)));
    }
    catch (...)
    {
        // Property not available
    }
}

// Search

OUString DocumentController::searchText(const OUString& pattern)
{
    if (!m_xDocument.is())
        return OUString();

    uno::Reference<util::XSearchable> xSearchable(m_xDocument, uno::UNO_QUERY);
    if (!xSearchable.is())
        return OUString();

    uno::Reference<util::XSearchDescriptor> xDesc = xSearchable->createSearchDescriptor();
    xDesc->setSearchString(pattern);

    uno::Reference<text::XTextRange> xFound(xSearchable->findFirst(xDesc), uno::UNO_QUERY);
    if (xFound.is())
    {
        return xFound->getString();
    }
    return OUString();
}

std::vector<sal_Int32> DocumentController::findAllOccurrences(const OUString& text)
{
    std::vector<sal_Int32> positions;

    if (!m_xDocument.is())
        return positions;

    uno::Reference<util::XSearchable> xSearchable(m_xDocument, uno::UNO_QUERY);
    if (!xSearchable.is())
        return positions;

    uno::Reference<util::XSearchDescriptor> xDesc = xSearchable->createSearchDescriptor();
    xDesc->setSearchString(text);

    uno::Reference<container::XIndexAccess> xResults(xSearchable->findAll(xDesc), uno::UNO_QUERY);
    if (xResults.is())
    {
        sal_Int32 nCount = xResults->getCount();
        for (sal_Int32 i = 0; i < nCount; ++i)
        {
            uno::Reference<text::XTextRange> xRange;
            xResults->getByIndex(i) >>= xRange;
            if (xRange.is() && m_xText.is())
            {
                // Get position by creating a cursor and measuring
                uno::Reference<text::XTextCursor> xCursor =
                    m_xText->createTextCursorByRange(xRange->getStart());
                if (xCursor.is())
                {
                    xCursor->gotoStart(true);
                    positions.push_back(xCursor->getString().getLength());
                }
            }
        }
    }

    return positions;
}

void DocumentController::refreshCursor()
{
    // Refresh the cursor from the current view
    if (!m_xController.is())
        return;

    uno::Reference<text::XTextViewCursorSupplier> xCursorSupplier(m_xController, uno::UNO_QUERY);
    if (xCursorSupplier.is())
    {
        uno::Reference<text::XTextViewCursor> xViewCursor = xCursorSupplier->getViewCursor();
        if (xViewCursor.is() && m_xText.is())
        {
            m_xCursor = m_xText->createTextCursorByRange(xViewCursor);
        }
    }
}

} // namespace officelabs

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
