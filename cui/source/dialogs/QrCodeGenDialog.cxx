/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QrCodeGenDialog.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/xmltools.hxx>
#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>
#include <utility>
#include <vcl/weld.hxx>

#include <QrCode.hpp>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/drawing/QRCode.hpp>
#include <com/sun/star/drawing/QRCodeErrorCorrection.hpp>
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
using namespace qrcodegen;

QrCodeGenDialog::QrCodeGenDialog(weld::Widget* pParent, Reference<XModel> xModel,
                                 bool bEditExisting)
    : GenericDialogController(pParent, "cui/ui/qrcodegen.ui", "QrCodeGenDialog")
    , m_xModel(std::move(xModel))
    , m_xEdittext(m_xBuilder->weld_entry("edit_text"))
    , m_xRadioLow(m_xBuilder->weld_radio_button("button_low"))
    , m_xRadioMedium(m_xBuilder->weld_radio_button("button_medium"))
    , m_xRadioQuartile(m_xBuilder->weld_radio_button("button_quartile"))
    , m_xRadioHigh(m_xBuilder->weld_radio_button("button_high"))
    , m_xSpinBorder(m_xBuilder->weld_spin_button("edit_border"))
{
    maBox.AddButton(m_xRadioLow.get());
    maBox.AddButton(m_xRadioMedium.get());
    maBox.AddButton(m_xRadioQuartile.get());
    maBox.AddButton(m_xRadioHigh.get());

    // Set ECC to Low by Default.
    if (!bEditExisting)
    {
        // ECC::Low     0
        m_aECCSelect = css::drawing::QRCodeErrorCorrection::LOW;
        m_xRadioLow->set_active(true);
        return;
    }

    Reference<container::XIndexAccess> xIndexAccess(m_xModel->getCurrentSelection(),
                                                    UNO_QUERY_THROW);
    Reference<XPropertySet> xProps(xIndexAccess->getByIndex(0), UNO_QUERY_THROW);

    // Read properties from selected qr code
    css::drawing::QRCode aQrCode;
    xProps->getPropertyValue("QRCodeProperties") >>= aQrCode;

    m_xEdittext->set_text(aQrCode.Payload);
    GetErrorCorrection(aQrCode.ErrorCorrection);

    Link<weld::ToggleButton&, void> aLink = LINK(this, QrCodeGenDialog, SelectRadio_Impl);
    m_xRadioLow->connect_toggled(aLink);
    m_xRadioMedium->connect_toggled(aLink);
    m_xRadioQuartile->connect_toggled(aLink);
    m_xRadioHigh->connect_toggled(aLink);

    m_xSpinBorder->set_value(aQrCode.Border);

    // Mark this as existing shape
    m_xExistingShapeProperties = xProps;
}

short QrCodeGenDialog::run()
{
    short nRet = GenericDialogController::run();
    if (nRet == RET_OK)
        Apply();
    return nRet;
}

void QrCodeGenDialog::Apply()
{
    css::drawing::QRCode aQrCode;
    aQrCode.Payload = m_xEdittext->get_text();
    aQrCode.ErrorCorrection = m_aECCSelect;
    aQrCode.Border = m_xSpinBorder->get_value();

    // Read svg and replace placeholder texts
    OUString aSvgImage = GenerateQrCode(aQrCode.Payload, aQrCode.ErrorCorrection, aQrCode.Border);

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

    bool bIsExistingQrCode = m_xExistingShapeProperties.is();
    Reference<XPropertySet> xShapeProps;
    if (bIsExistingQrCode)
        xShapeProps = m_xExistingShapeProperties;
    else
        xShapeProps.set(Reference<lang::XMultiServiceFactory>(m_xModel, UNO_QUERY_THROW)
                            ->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        UNO_QUERY);

    xShapeProps->setPropertyValue("Graphic", Any(xGraphic));

    // Set qrcode properties
    xShapeProps->setPropertyValue("QRCodeProperties", Any(aQrCode));

    if (!bIsExistingQrCode)
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
            Reference<XSpreadsheet> xSheet(xView->getActiveSheet(), UNO_SET_THROW);
            Reference<XDrawPageSupplier> xDrawPageSupplier(xSheet, UNO_QUERY_THROW);
            Reference<XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
            Reference<XShapes> xShapes(xDrawPage, UNO_QUERY_THROW);

            xShapes->add(xShape);
            return;
        }
    }
}

IMPL_LINK(QrCodeGenDialog, SelectRadio_Impl, weld::ToggleButton&, rButton, void)
{
    // If the button is already active do not toggle it back.
    if (!rButton.get_active())
        rButton.set_active(true);

    SelectErrorCorrection(rButton);
}

void QrCodeGenDialog::SelectErrorCorrection(weld::ToggleButton& rButton)
{
    sal_Int32 nPos = maBox.GetButtonPos(&rButton);
    if (nPos != -1 && nPos != maBox.GetCurrentButtonPos())
    {
        maBox.SelectButton(&rButton);
        m_aECCSelect = static_cast<int>(maBox.GetCurrentButtonPos()) + 1;
    }
}

OUString QrCodeGenDialog::GenerateQrCode(OUString aQrText, int aQrECC, int aQrBorder)
{
    //Select ECC:: value from aQrECC
    qrcodegen::QrCode::Ecc bqrEcc = qrcodegen::QrCode::Ecc::LOW;

    switch (aQrECC)
    {
        case 2:
        {
            bqrEcc = qrcodegen::QrCode::Ecc::MEDIUM;
            break;
        }
        case 3:
        {
            bqrEcc = qrcodegen::QrCode::Ecc::QUARTILE;
            break;
        }
        case 4:
        {
            bqrEcc = qrcodegen::QrCode::Ecc::HIGH;
            break;
        }
        default:
        {
            bqrEcc = qrcodegen::QrCode::Ecc::LOW;
            break;
        }
    }

    //OuString to char* qrtext
    OString o = OUStringToOString(aQrText, RTL_TEXTENCODING_ASCII_US);
    const char* qrtext = o.pData->buffer;

    //From Qr Code library.
    qrcodegen::QrCode qr0 = qrcodegen::QrCode::encodeText(qrtext, bqrEcc);
    std::string svg = qr0.toSvgString(aQrBorder);
    //cstring to OUString
    char* cstr = &svg[0];
    return OUString::createFromAscii(cstr);
}

void QrCodeGenDialog::GetErrorCorrection(long ErrorCorrection)
{
    switch (ErrorCorrection)
    {
        case css::drawing::QRCodeErrorCorrection::MEDIUM:
        {
            m_xRadioMedium->set_active(true);
            m_aECCSelect = ErrorCorrection;
            break;
        }
        case css::drawing::QRCodeErrorCorrection::QUARTILE:
        {
            m_xRadioQuartile->set_active(true);
            m_aECCSelect = ErrorCorrection;
            break;
        }
        case css::drawing::QRCodeErrorCorrection::HIGH:
        {
            m_xRadioHigh->set_active(true);
            m_aECCSelect = ErrorCorrection;
            break;
        }
        default:
        {
            m_xRadioLow->set_active(true);
            m_aECCSelect = css::drawing::QRCodeErrorCorrection::LOW;
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
