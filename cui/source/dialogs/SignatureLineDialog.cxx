/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SignatureLineDialog.hxx>

#include <comphelper/xmltools.hxx>
#include <utility>
#include <vcl/weld.hxx>
#include <svx/signaturelinehelper.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>

using namespace css;
using namespace css::uno;
using namespace css::beans;
using namespace css::container;
using namespace css::frame;
using namespace css::lang;
using namespace css::frame;
using namespace css::sheet;
using namespace css::text;
using namespace css::drawing;
using namespace css::graphic;

SignatureLineDialog::SignatureLineDialog(weld::Widget* pParent, Reference<XModel> xModel,
                                         bool bEditExisting)
    : SignatureLineDialogBase(pParent, std::move(xModel), u"cui/ui/signatureline.ui"_ustr,
                              u"SignatureLineDialog"_ustr)
    , m_xEditName(m_xBuilder->weld_entry(u"edit_name"_ustr))
    , m_xEditTitle(m_xBuilder->weld_entry(u"edit_title"_ustr))
    , m_xEditEmail(m_xBuilder->weld_entry(u"edit_email"_ustr))
    , m_xEditInstructions(m_xBuilder->weld_text_view(u"edit_instructions"_ustr))
    , m_xCheckboxCanAddComments(m_xBuilder->weld_check_button(u"checkbox_can_add_comments"_ustr))
    , m_xCheckboxShowSignDate(m_xBuilder->weld_check_button(u"checkbox_show_sign_date"_ustr))
{
    m_xEditInstructions->set_size_request(m_xEditInstructions->get_approximate_digit_width() * 48,
                                          m_xEditInstructions->get_text_height() * 5);

    // No signature line selected - start with empty dialog and set some default values
    if (!bEditExisting)
    {
        m_xCheckboxCanAddComments->set_active(true);
        m_xCheckboxShowSignDate->set_active(true);
        return;
    }

    Reference<container::XIndexAccess> xIndexAccess(m_xModel->getCurrentSelection(),
                                                    UNO_QUERY_THROW);
    Reference<XPropertySet> xProps(xIndexAccess->getByIndex(0), UNO_QUERY_THROW);

    // Read properties from selected signature line
    xProps->getPropertyValue(u"SignatureLineId"_ustr) >>= m_aSignatureLineId;
    OUString aSuggestedSignerName;
    xProps->getPropertyValue(u"SignatureLineSuggestedSignerName"_ustr) >>= aSuggestedSignerName;
    m_xEditName->set_text(aSuggestedSignerName);
    OUString aSuggestedSignerTitle;
    xProps->getPropertyValue(u"SignatureLineSuggestedSignerTitle"_ustr) >>= aSuggestedSignerTitle;
    m_xEditTitle->set_text(aSuggestedSignerTitle);
    OUString aSuggestedSignerEmail;
    xProps->getPropertyValue(u"SignatureLineSuggestedSignerEmail"_ustr) >>= aSuggestedSignerEmail;
    m_xEditEmail->set_text(aSuggestedSignerEmail);
    OUString aSigningInstructions;
    xProps->getPropertyValue(u"SignatureLineSigningInstructions"_ustr) >>= aSigningInstructions;
    m_xEditInstructions->set_text(aSigningInstructions);
    bool bCanAddComments = false;
    xProps->getPropertyValue(u"SignatureLineCanAddComment"_ustr) >>= bCanAddComments;
    m_xCheckboxCanAddComments->set_active(bCanAddComments);
    bool bShowSignDate = false;
    xProps->getPropertyValue(u"SignatureLineShowSignDate"_ustr) >>= bShowSignDate;
    m_xCheckboxShowSignDate->set_active(bShowSignDate);

    // Mark this as existing shape
    m_xExistingShapeProperties = xProps;
}

void SignatureLineDialog::Apply()
{
    if (m_aSignatureLineId.isEmpty())
        m_aSignatureLineId
            = OStringToOUString(comphelper::xml::generateGUIDString(), RTL_TEXTENCODING_ASCII_US);
    OUString aSignerName(m_xEditName->get_text());
    OUString aSignerTitle(m_xEditTitle->get_text());
    OUString aSignerEmail(m_xEditEmail->get_text());
    OUString aSigningInstructions(m_xEditInstructions->get_text());
    bool bCanAddComments(m_xCheckboxCanAddComments->get_active());
    bool bShowSignDate(m_xCheckboxShowSignDate->get_active());

    // Read svg and replace placeholder texts
    OUString aSvgImage(svx::SignatureLineHelper::getSignatureImage());
    aSvgImage = aSvgImage.replaceAll("[SIGNER_NAME]", getCDataString(aSignerName));
    aSvgImage = aSvgImage.replaceAll("[SIGNER_TITLE]", getCDataString(aSignerTitle));

    // These are only filled if the signature line is signed.
    aSvgImage = aSvgImage.replaceAll("[SIGNATURE]", "");
    aSvgImage = aSvgImage.replaceAll("[SIGNED_BY]", "");
    aSvgImage = aSvgImage.replaceAll("[INVALID_SIGNATURE]", "");
    aSvgImage = aSvgImage.replaceAll("[DATE]", "");

    // Insert/Update graphic
    Reference<XGraphic> xGraphic = svx::SignatureLineHelper::importSVG(aSvgImage);

    bool bIsExistingSignatureLine = m_xExistingShapeProperties.is();
    Reference<XPropertySet> xShapeProps;
    if (bIsExistingSignatureLine)
        xShapeProps = m_xExistingShapeProperties;
    else
        xShapeProps.set(Reference<lang::XMultiServiceFactory>(m_xModel, UNO_QUERY_THROW)
                            ->createInstance(u"com.sun.star.drawing.GraphicObjectShape"_ustr),
                        UNO_QUERY);

    xShapeProps->setPropertyValue(u"Graphic"_ustr, Any(xGraphic));
    xShapeProps->setPropertyValue(u"SignatureLineUnsignedImage"_ustr, Any(xGraphic));

    // Set signature line properties
    xShapeProps->setPropertyValue(u"IsSignatureLine"_ustr, Any(true));
    xShapeProps->setPropertyValue(u"SignatureLineId"_ustr, Any(m_aSignatureLineId));
    if (!aSignerName.isEmpty())
        xShapeProps->setPropertyValue(u"SignatureLineSuggestedSignerName"_ustr, Any(aSignerName));
    if (!aSignerTitle.isEmpty())
        xShapeProps->setPropertyValue(u"SignatureLineSuggestedSignerTitle"_ustr, Any(aSignerTitle));
    if (!aSignerEmail.isEmpty())
        xShapeProps->setPropertyValue(u"SignatureLineSuggestedSignerEmail"_ustr, Any(aSignerEmail));
    if (!aSigningInstructions.isEmpty())
        xShapeProps->setPropertyValue(u"SignatureLineSigningInstructions"_ustr,
                                      Any(aSigningInstructions));
    xShapeProps->setPropertyValue(u"SignatureLineShowSignDate"_ustr, Any(bShowSignDate));
    xShapeProps->setPropertyValue(u"SignatureLineCanAddComment"_ustr, Any(bCanAddComments));

    if (bIsExistingSignatureLine)
        return;

    // Default size
    Reference<XShape> xShape(xShapeProps, UNO_QUERY);
    awt::Size aShapeSize;
    aShapeSize.Height = 3000;
    aShapeSize.Width = 6000;
    xShape->setSize(aShapeSize);

    // Default anchoring
    xShapeProps->setPropertyValue(u"AnchorType"_ustr, Any(TextContentAnchorType_AT_PARAGRAPH));

    // Writer
    const Reference<XTextDocument> xTextDocument(m_xModel, UNO_QUERY);
    if (xTextDocument.is())
    {
        Reference<XTextContent> xTextContent(xShape, UNO_QUERY_THROW);
        Reference<XTextViewCursorSupplier> xViewCursorSupplier(m_xModel->getCurrentController(),
                                                               UNO_QUERY_THROW);
        Reference<XTextViewCursor> xCursor = xViewCursorSupplier->getViewCursor();
        // use cursor's XText - it might be in table cell, frame, ...
        Reference<XText> const xText(xCursor->getText());
        assert(xText.is());
        xText->insertTextContent(xCursor, xTextContent, true);
        return;
    }

    // Calc
    const Reference<XSpreadsheetDocument> xSpreadsheetDocument(m_xModel, UNO_QUERY);
    if (!xSpreadsheetDocument.is())
        return;

    Reference<XPropertySet> xSheetCell(m_xModel->getCurrentSelection(), UNO_QUERY_THROW);
    awt::Point aCellPosition;
    xSheetCell->getPropertyValue(u"Position"_ustr) >>= aCellPosition;
    xShape->setPosition(aCellPosition);

    Reference<XSpreadsheetView> xView(m_xModel->getCurrentController(), UNO_QUERY_THROW);
    Reference<XSpreadsheet> xSheet(xView->getActiveSheet(), UNO_SET_THROW);
    Reference<XDrawPageSupplier> xDrawPageSupplier(xSheet, UNO_QUERY_THROW);
    Reference<XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    Reference<XShapes> xShapes(xDrawPage, UNO_QUERY_THROW);

    xShapes->add(xShape);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
