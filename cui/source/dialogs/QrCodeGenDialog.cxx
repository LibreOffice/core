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
    : QrCodeGenDialogBase(pParent, std::move(xModel), "cui/ui/qrcodegen.ui", "QrCodeGenDialog")
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
        m_aECCSelect = 0;
        m_xRadioLow->set_active(true);
        return;
    }

    Reference<container::XIndexAccess> xIndexAccess(m_xModel->getCurrentSelection(),
                                                    UNO_QUERY_THROW);
    Reference<XPropertySet> xProps(xIndexAccess->getByIndex(0), UNO_QUERY_THROW);

    // Read properties from selected qr code
    xProps->getPropertyValue("QrCodeId") >>= m_aQrCodeId;

    OUString aQrText;
    xProps->getPropertyValue("QrCodeText") >>= aQrText;
    m_xEdittext->set_text(aQrText);

    /* Work with numbering and synchronising radio Buttons {87-94 some hack required}*/
    int aQrCodeECC;
    xProps->getPropertyValue("QrCodeECC") >>= aQrCodeECC;

    Link<weld::ToggleButton&, void> aLink = LINK(this, QrCodeGenDialog, SelectRadio_Impl);
    m_xRadioLow->connect_toggled(aLink);
    m_xRadioMedium->connect_toggled(aLink);
    m_xRadioQuartile->connect_toggled(aLink);
    m_xRadioHigh->connect_toggled(aLink);

    int aQrCodeBorder;
    xProps->getPropertyValue("QrCodeBorder") >>= aQrCodeBorder;
    m_xSpinBorder->set_value(aQrCodeBorder);

    // Mark this as existing shape
    m_xExistingShapeProperties = xProps;
}

void QrCodeGenDialog::Apply()
{
    if (m_aQrCodeId.isEmpty())
        m_aQrCodeId
            = OStringToOUString(comphelper::xml::generateGUIDString(), RTL_TEXTENCODING_ASCII_US);

    OUString aQrText(m_xEdittext->get_text());
    int aQrECC(m_aECCSelect);
    int aQrBorder(m_xSpinBorder->get_value());

    // Read svg and replace placeholder texts
    OUString aSvgImage = GenerateQrCode(aQrText, aQrECC, aQrBorder);

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
    xShapeProps->setPropertyValue("rImage", Any(xGraphic));

    // Set qrcode properties
    xShapeProps->setPropertyValue("IsQrCode", Any(true));
    xShapeProps->setPropertyValue("QrCodeId", Any(m_aQrCodeId));
    if (!aQrText.isEmpty())
        xShapeProps->setPropertyValue("QrCodeText", Any(aQrText));
    // ECC Type, there was a if argument to pass, Currently removing the condition
    xShapeProps->setPropertyValue("QrCodeECC", Any(aQrECC));
    if (aQrBorder > 0)
        xShapeProps->setPropertyValue("QrCodeBorder", Any(aQrBorder));

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

    SelectECCType(rButton);
}

void QrCodeGenDialog::SelectECCType(weld::ToggleButton& rButton)
{
    /* comment for me - The fuctions here are from cuitabarea.hxx.
    Its use was to group the Radio Buttons together and get int output. */
    sal_Int32 nPos = maBox.GetButtonPos(&rButton);
    if (nPos != -1 && nPos != maBox.GetCurrentButtonPos())
    {
        maBox.SelectButton(&rButton);
        m_aECCSelect = static_cast<int>(maBox.GetCurrentButtonPos());
    }
}

OUString QrCodeGenDialog::GenerateQrCode(OUString aQrText, int aQrECC, int aQrBorder)
{
    //Select ECC:: valur from aQrECC
    QrCode::Ecc bqrEcc;

    switch (aQrECC)
    {
        case 0:
        {
            bqrEcc = QrCode::Ecc::LOW;
            break;
        }
        case 1:
        {
            bqrEcc = QrCode::Ecc::MEDIUM;
            break;
        }
        case 2:
        {
            bqrEcc = QrCode::Ecc::QUARTILE;
            break;
        }
        case 4:
        {
            bqrEcc = QrCode::Ecc::HIGH;
            break;
        }
    }

    //OuString to char* qrtext
    OString o = OUStringToOString(aQrText, RTL_TEXTENCODING_ASCII_US);
    const char* qrtext = o.pData->buffer;

    //From Qr Code library.
    QrCode qr0 = QrCode::encodeText(qrtext, bqrEcc);
    std::string svg = qr0.toSvgString(aQrBorder);
    //cstring to OUString
    char* cstr = &svg[0];
    return OUString::createFromAscii(cstr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
