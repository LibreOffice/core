/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SignatureLineDialog.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/xmltools.hxx>
#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>
#include <vcl/weld.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>

using namespace css;
using namespace css::uno;
using namespace css::beans;
using namespace css::frame;
using namespace css::io;
using namespace css::lang;
using namespace css::frame;
using namespace css::text;
using namespace css::drawing;
using namespace css::graphic;

SignatureLineDialog::SignatureLineDialog(weld::Widget* pParent, Reference<XModel> xModel,
                                         bool bEditExisting)
    : SignatureLineDialogBase(pParent, xModel, "cui/ui/signatureline.ui", "SignatureLineDialog")
    , m_xEditName(m_xBuilder->weld_entry("edit_name"))
    , m_xEditTitle(m_xBuilder->weld_entry("edit_title"))
    , m_xEditEmail(m_xBuilder->weld_entry("edit_email"))
    , m_xEditInstructions(m_xBuilder->weld_text_view("edit_instructions"))
    , m_xCheckboxCanAddComments(m_xBuilder->weld_check_button("checkbox_can_add_comments"))
    , m_xCheckboxShowSignDate(m_xBuilder->weld_check_button("checkbox_show_sign_date"))
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
    xProps->getPropertyValue("SignatureLineId") >>= m_aSignatureLineId;
    OUString aSuggestedSignerName;
    xProps->getPropertyValue("SignatureLineSuggestedSignerName") >>= aSuggestedSignerName;
    m_xEditName->set_text(aSuggestedSignerName);
    OUString aSuggestedSignerTitle;
    xProps->getPropertyValue("SignatureLineSuggestedSignerTitle") >>= aSuggestedSignerTitle;
    m_xEditTitle->set_text(aSuggestedSignerTitle);
    OUString aSuggestedSignerEmail;
    xProps->getPropertyValue("SignatureLineSuggestedSignerEmail") >>= aSuggestedSignerEmail;
    m_xEditEmail->set_text(aSuggestedSignerEmail);
    OUString aSigningInstructions;
    xProps->getPropertyValue("SignatureLineSigningInstructions") >>= aSigningInstructions;
    m_xEditInstructions->set_text(aSigningInstructions);
    bool bCanAddComments = false;
    xProps->getPropertyValue("SignatureLineCanAddComment") >>= bCanAddComments;
    m_xCheckboxCanAddComments->set_active(bCanAddComments);
    bool bShowSignDate = false;
    xProps->getPropertyValue("SignatureLineShowSignDate") >>= bShowSignDate;
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
    OUString aSvgImage(getSignatureImage());
    aSvgImage = aSvgImage.replaceAll("[SIGNER_NAME]", getCDataString(aSignerName));
    aSvgImage = aSvgImage.replaceAll("[SIGNER_TITLE]", getCDataString(aSignerTitle));

    // These are only filled if the signature line is signed.
    aSvgImage = aSvgImage.replaceAll("[SIGNATURE]", "");
    aSvgImage = aSvgImage.replaceAll("[SIGNED_BY]", "");
    aSvgImage = aSvgImage.replaceAll("[INVALID_SIGNATURE]", "");
    aSvgImage = aSvgImage.replaceAll("[DATE]", "");

    // Insert/Update graphic
    SvMemoryStream aSvgStream(4096, 4096);
    aSvgStream.WriteOString(OUStringToOString(aSvgImage, RTL_TEXTENCODING_UTF8));
    Reference<XInputStream> xInputStream(new utl::OSeekableInputStreamWrapper(aSvgStream));
    Reference<XComponentContext> xContext(comphelper::getProcessComponentContext());
    Reference<XGraphicProvider> xProvider = css::graphic::GraphicProvider::create(xContext);

    Sequence<PropertyValue> aMediaProperties(1);
    aMediaProperties[0].Name = "InputStream";
    aMediaProperties[0].Value <<= xInputStream;
    Reference<XGraphic> xGraphic(xProvider->queryGraphic(aMediaProperties));

    bool bIsExistingSignatureLine = m_xExistingShapeProperties.is();
    Reference<XPropertySet> xShapeProps;
    if (bIsExistingSignatureLine)
        xShapeProps = m_xExistingShapeProperties;
    else
        xShapeProps.set(Reference<lang::XMultiServiceFactory>(m_xModel, UNO_QUERY)
                            ->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        UNO_QUERY);

    xShapeProps->setPropertyValue("Graphic", Any(xGraphic));
    xShapeProps->setPropertyValue("SignatureLineUnsignedImage", Any(xGraphic));

    // Set signature line properties
    xShapeProps->setPropertyValue("IsSignatureLine", Any(true));
    xShapeProps->setPropertyValue("SignatureLineId", Any(m_aSignatureLineId));
    if (!aSignerName.isEmpty())
        xShapeProps->setPropertyValue("SignatureLineSuggestedSignerName", Any(aSignerName));
    if (!aSignerTitle.isEmpty())
        xShapeProps->setPropertyValue("SignatureLineSuggestedSignerTitle", Any(aSignerTitle));
    if (!aSignerEmail.isEmpty())
        xShapeProps->setPropertyValue("SignatureLineSuggestedSignerEmail", Any(aSignerEmail));
    if (!aSigningInstructions.isEmpty())
        xShapeProps->setPropertyValue("SignatureLineSigningInstructions",
                                      Any(aSigningInstructions));
    xShapeProps->setPropertyValue("SignatureLineShowSignDate", Any(bShowSignDate));
    xShapeProps->setPropertyValue("SignatureLineCanAddComment", Any(bCanAddComments));

    if (!bIsExistingSignatureLine)
    {
        // Default size
        Reference<XShape> xShape(xShapeProps, UNO_QUERY);
        awt::Size aShapeSize;
        aShapeSize.Height = 3000;
        aShapeSize.Width = 6000;
        xShape->setSize(aShapeSize);

        // Default anchoring
        xShapeProps->setPropertyValue("AnchorType", Any(TextContentAnchorType_AT_PARAGRAPH));

        // Insert into document
        const Reference<XTextDocument> xTextDocument(m_xModel, UNO_QUERY);
        Reference<XTextContent> xTextContent(xShape, UNO_QUERY_THROW);
        Reference<XTextViewCursorSupplier> xViewCursorSupplier(m_xModel->getCurrentController(),
                                                               UNO_QUERY_THROW);
        Reference<XTextViewCursor> xCursor = xViewCursorSupplier->getViewCursor();
        // use cursor's XText - it might be in table cell, frame, ...
        Reference<XText> const xText(xCursor->getText());
        assert(xText.is());
        xText->insertTextContent(xCursor, xTextContent, true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
