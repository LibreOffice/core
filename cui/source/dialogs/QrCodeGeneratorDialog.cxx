/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QrCodeGeneratorDialog.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/xmltools.hxx>
#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>
#include <utility>
#include <vcl/weld.hxx>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include "BitBuffer.hpp"
#include "QrCode.hpp"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
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
using namespace css::io;
using namespace css::lang;
using namespace css::frame;
using namespace css::sheet;
using namespace css::text;
using namespace css::drawing;
using namespace css::graphic;

using std::uint8_t;
using qrcodegen::QrCode;
using qrcodegen::QrSegment;

QrCodeGeneratorDialog::QrCodeGeneratorDialog(weld::Widget* pParent, Reference<XModel> xModel,
                                         bool bEditExisting)
    : QrCodeGeneratorDialogBase(pParent, std::move(xModel), "cui/ui/qrcodegenerator.ui",
                              "QrCodeGeneratorDialog")
    , m_xEditText(m_xBuilder->weld_entry("edit_text"))
    , m_xCheckboxLow(m_xBuilder->weld_check_button("check_low"))
    , m_xCheckboxMedium(m_xBuilder->weld_check_button("check_medium"))
    , m_xCheckboxQuartile(m_xBuilder->weld_check_button("checkbox_quartile"))
    , m_xCheckboxHigh(m_xBuilder->weld_check_button("checkbox_high"))
    /*, m_xSpinbtnBorder(m_xBuilder->weld_spin_button("edit_border"))*/
{
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
    xProps->getPropertyValue("SignatureLineId") >>= m_aQrCodeGeneratorId;
    OUString aURLText;
    xProps->getPropertyValue("SignatureLineSuggestedSignerName") >>= aURLText;
    m_xEditText->set_text(aURLText);
    
    bool bECCLow = false;
    bool bECCMedium = false;
    bool bECCQuartile = false;
    bool bECCHigh = false;
    //Checking ECC checkboxes exclusive
    xProps->getPropertyValue("ErrorCorrection") >>= bECCLow;
    if(bECCLow) {
        m_xCheckboxLow->set_active(true);
        m_xCheckboxMedium->set_active(false);
        m_xCheckboxQuartile->set_active(false);
        m_xCheckboxHigh->set_active(false);
    }
    else {
        m_xCheckboxLow->set_active(false);
    }

    xProps->getPropertyValue("ErrorCorrection") >>= bECCMedium;
    if(bECCMedium) {
        m_xCheckboxLow->set_active(false);
        m_xCheckboxMedium->set_active(true);
        m_xCheckboxQuartile->set_active(false);
        m_xCheckboxHigh->set_active(false);
    }
    else {
        m_xCheckboxMedium->set_active(false);
    }

    xProps->getPropertyValue("ErrorCorrection") >>= bECCQuartile;
    if(bECCLow) {
        m_xCheckboxLow->set_active(false);
        m_xCheckboxMedium->set_active(false);
        m_xCheckboxQuartile->set_active(true);
        m_xCheckboxHigh->set_active(false);
    }
    else {
        m_xCheckboxQuartile->set_active(false);
    }

    xProps->getPropertyValue("ErrorCorrection") >>= bECCHigh;
    if(bECCLow) {
        m_xCheckboxLow->set_active(false);
        m_xCheckboxMedium->set_active(false);
        m_xCheckboxQuartile->set_active(false);
        m_xCheckboxHigh->set_active(true);
    }
    else {
        m_xCheckboxHigh->set_active(false);
    }
    
    /*Link<weld::SpinButton&,void> aSpinLink = LINK(this, QrCodeGeneratorDialog, SpinHdl);
    m_xColumnsEdit->connect_value_changed(aSpinLink);
    */

    // Mark this as existing shape
    m_xExistingShapeProperties = xProps;
}

/*IMPL_LINK_NOARG(QrCodeGeneratorDialog, SpinHdl, weld::SpinButton&, void)
{
    if (!m_xColumnsBtn->get_active())
        return;

    if (m_xColumnsEdit->get_value() % 2 == 0)
    {
        m_xBookModeChk->set_sensitive(true);
    }
    else
    {
        m_xBookModeChk->set_active(false);
        m_xBookModeChk->set_sensitive(false);
    }

    m_bModified = true;
}
*/

OUString QrCodeGeneratorDialog::QRGen() {
    
    if (m_aQrCodeGeneratorId.isEmpty())
        m_aQrCodeGeneratorId
            = OStringToOUString(comphelper::xml::generateGUIDString(), RTL_TEXTENCODING_ASCII_US);
    
    OUString aURLText(m_xEditText->get_text());

    //convert to std::string
    //Question: Whether it supports UTF.
    //Not taken care whether or not url uses utf.

    OString o = OUStringToOString( aURLText, RTL_TEXTENCODING_ASCII_US );
    char *urltext = o.pData->buffer;        //Noticed from https://forum.openoffice.org/en/forum/viewtopic.php?f=44&t=54978

    QrCode::Ecc errCorLvl;
    
    if (m_xCheckboxHigh->get_active()) {
        errCorLvl = QrCode::Ecc::HIGH;
    } else if (m_xCheckboxMedium->get_active()) {
        errCorLvl = QrCode::Ecc::MEDIUM;
    } else if (m_xCheckboxQuartile->get_active()) {
        errCorLvl = QrCode::Ecc::QUARTILE;
    } else {
        errCorLvl = QrCode::Ecc::LOW;
    }

    const QrCode qr = QrCode::encodeText(text, errCorLvl);

    const char *con =  qr.toSvgString(4).c_str();   //Add Border feature must se. Now -4, string.cpp in c++ fol.

    return OUString::createFromAscii(con);
}   

void QrCodeGeneratorDialog::Apply()
{

    // Read svg and replace placeholder texts
    /* Here do the qr code generaton thing */
    OUString aSvgImage(QRGen());

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

        const Reference<XServiceInfo> xServiceInfo(m_xModel, UNO_QUERY);

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
        if (xSpreadsheetDocument.is())
        {
            Reference<XPropertySet> xSheetCell(m_xModel->getCurrentSelection(), UNO_QUERY_THROW);
            awt::Point aCellPosition;
            xSheetCell->getPropertyValue("Position") >>= aCellPosition;
            xShape->setPosition(aCellPosition);

            Reference<XSpreadsheetView> xView(m_xModel->getCurrentController(), UNO_QUERY_THROW);
            Reference<XSpreadsheet> xSheet(xView->getActiveSheet(), UNO_QUERY_THROW);
            Reference<XDrawPageSupplier> xDrawPageSupplier(xSheet, UNO_QUERY_THROW);
            Reference<XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_QUERY_THROW);
            Reference<XShapes> xShapes(xDrawPage, UNO_QUERY_THROW);

            xShapes->add(xShape);
            return;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
