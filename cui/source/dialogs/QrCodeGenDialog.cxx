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
#include <tools/stream.hxx>
#include <dialmgr.hxx>
#include <strings.hrc>
#include <unotools/streamwrap.hxx>
#include <utility>
#include <vcl/svapp.hxx>

#if ENABLE_QRCODEGEN
#if defined(SYSTEM_QRCODEGEN)
#include <qrcodegen/QrCode.hpp>
#else
#include <QrCode.hpp>
#endif
#endif

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
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <config_fuzzers.h>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

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
#if ENABLE_QRCODEGEN
using namespace qrcodegen;
#endif

QrCodeGenDialog::QrCodeGenDialog(weld::Widget* pParent, Reference<XModel> xModel,
                                 bool bEditExisting)
    : GenericDialogController(pParent, "cui/ui/qrcodegen.ui", "QrCodeGenDialog")
    , m_xModel(std::move(xModel))
    , m_xEdittext(m_xBuilder->weld_entry("edit_text"))
    , m_xECC{ m_xBuilder->weld_radio_button("button_low"),
              m_xBuilder->weld_radio_button("button_medium"),
              m_xBuilder->weld_radio_button("button_quartile"),
              m_xBuilder->weld_radio_button("button_high") }
    , m_xSpinBorder(m_xBuilder->weld_spin_button("edit_border"))
#if ENABLE_QRCODEGEN
    , mpParent(pParent)
#endif
{
    if (!bEditExisting)
    {
        // TODO: This only works in Writer doc. Should also work in shapes
        Reference<XIndexAccess> xSelections(m_xModel->getCurrentSelection(), UNO_QUERY);
        if (xSelections.is())
        {
            Reference<XTextRange> xSelection(xSelections->getByIndex(0), UNO_QUERY);
            if (xSelection.is())
                m_xEdittext->set_text(xSelection->getString());
        }
        return;
    }

    Reference<container::XIndexAccess> xIndexAccess(m_xModel->getCurrentSelection(),
                                                    UNO_QUERY_THROW);
    Reference<XPropertySet> xProps(xIndexAccess->getByIndex(0), UNO_QUERY_THROW);

    // Read properties from selected QR Code
    css::drawing::QRCode aQRCode;
    xProps->getPropertyValue("QRCodeProperties") >>= aQRCode;

    m_xEdittext->set_text(aQRCode.Payload);

    //Get Error Correction Constant from selected QR Code
    GetErrorCorrection(aQRCode.ErrorCorrection);

    m_xSpinBorder->set_value(aQRCode.Border);

    // Mark this as existing shape
    m_xExistingShapeProperties = xProps;
}

short QrCodeGenDialog::run()
{
#if ENABLE_QRCODEGEN
    short nRet;
    while (true)
    {
        nRet = GenericDialogController::run();
        if (nRet == RET_OK)
        {
            try
            {
                Apply();
                break;
            }
            catch (const qrcodegen::data_too_long&)
            {
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
                    mpParent, VclMessageType::Warning, VclButtonsType::Ok,
                    CuiResId(RID_SVXSTR_QRCODEDATALONG)));
                xBox->run();
            }
        }
        else
            break;
    }
    return nRet;
#else
    return RET_CANCEL;
#endif
}

void QrCodeGenDialog::Apply()
{
#if ENABLE_QRCODEGEN
    css::drawing::QRCode aQRCode;
    aQRCode.Payload = m_xEdittext->get_text();

    bool bLowECCActive(m_xECC[0]->get_active());
    bool bMediumECCActive(m_xECC[1]->get_active());
    bool bQuartileECCActive(m_xECC[2]->get_active());

    if (bLowECCActive)
    {
        aQRCode.ErrorCorrection = css::drawing::QRCodeErrorCorrection::LOW;
    }
    else if (bMediumECCActive)
    {
        aQRCode.ErrorCorrection = css::drawing::QRCodeErrorCorrection::MEDIUM;
    }
    else if (bQuartileECCActive)
    {
        aQRCode.ErrorCorrection = css::drawing::QRCodeErrorCorrection::QUARTILE;
    }
    else
    {
        aQRCode.ErrorCorrection = css::drawing::QRCodeErrorCorrection::HIGH;
    }

    aQRCode.Border = m_xSpinBorder->get_value();

    // Read svg and replace placeholder texts
    OUString aSvgImage = GenerateQRCode(aQRCode.Payload, aQRCode.ErrorCorrection, aQRCode.Border);

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

    bool bIsExistingQRCode = m_xExistingShapeProperties.is();
    Reference<XPropertySet> xShapeProps;
    if (bIsExistingQRCode)
        xShapeProps = m_xExistingShapeProperties;
    else
        xShapeProps.set(Reference<lang::XMultiServiceFactory>(m_xModel, UNO_QUERY_THROW)
                            ->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        UNO_QUERY);

    xShapeProps->setPropertyValue("Graphic", Any(xGraphic));

    // Set QRCode properties
    xShapeProps->setPropertyValue("QRCodeProperties", Any(aQRCode));

    if (bIsExistingQRCode)
        return;

    // Default size
    Reference<XShape> xShape(xShapeProps, UNO_QUERY);
    awt::Size aShapeSize;
    aShapeSize.Height = 4000;
    aShapeSize.Width = 4000;
    xShape->setSize(aShapeSize);

    // Default anchoring
    xShapeProps->setPropertyValue("AnchorType", Any(TextContentAnchorType_AT_PARAGRAPH));

    const Reference<XServiceInfo> xServiceInfo(m_xModel, UNO_QUERY_THROW);

    // Writer
    if (xServiceInfo->supportsService("com.sun.star.text.TextDocument"))
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
    else if (xServiceInfo->supportsService("com.sun.star.sheet.SpreadsheetDocument"))
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

    //Impress and Draw
    else if (xServiceInfo->supportsService("com.sun.star.presentation.PresentationDocument")
             || xServiceInfo->supportsService("com.sun.star.drawing.DrawingDocument"))
    {
        Reference<XDrawView> xView(m_xModel->getCurrentController(), UNO_QUERY_THROW);
        Reference<XDrawPage> xPage(xView->getCurrentPage(), UNO_SET_THROW);
        Reference<XShapes> xShapes(xPage, UNO_QUERY_THROW);

        xShapes->add(xShape);
        return;
    }

    else
    {
        //Not implemented for math,base and other apps.
        throw uno::RuntimeException("Not implemented");
    }
#endif
}

OUString QrCodeGenDialog::GenerateQRCode(OUString aQRText, long aQRECC, int aQRBorder)
{
#if ENABLE_QRCODEGEN
    //Select ECC:: value from aQrECC
    qrcodegen::QrCode::Ecc bqrEcc = qrcodegen::QrCode::Ecc::LOW;

    switch (aQRECC)
    {
        case 1:
        {
            bqrEcc = qrcodegen::QrCode::Ecc::LOW;
            break;
        }
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
    }

    //OuString to char* qrtext
    OString o = OUStringToOString(aQRText, RTL_TEXTENCODING_UTF8);
    const char* qrtext = o.pData->buffer;

    // From QR Code library
    qrcodegen::QrCode qr0 = qrcodegen::QrCode::encodeText(qrtext, bqrEcc);
    std::string svg = qr0.toSvgString(aQRBorder);
    //cstring to OUString
    return OUString::createFromAscii(svg.c_str());
#else
    (void)aQRText;
    (void)aQRECC;
    (void)aQRBorder;
    return OUString();
#endif
}

void QrCodeGenDialog::GetErrorCorrection(long ErrorCorrection)
{
    switch (ErrorCorrection)
    {
        case css::drawing::QRCodeErrorCorrection::LOW:
        {
            m_xECC[0]->set_active(true);
            break;
        }
        case css::drawing::QRCodeErrorCorrection::MEDIUM:
        {
            m_xECC[1]->set_active(true);
            break;
        }
        case css::drawing::QRCodeErrorCorrection::QUARTILE:
        {
            m_xECC[2]->set_active(true);
            break;
        }
        case css::drawing::QRCodeErrorCorrection::HIGH:
        {
            m_xECC[3]->set_active(true);
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
