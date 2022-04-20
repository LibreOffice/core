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
#include <comphelper/propertyvalue.hxx>
#include <tools/stream.hxx>
#include <dialmgr.hxx>
#include <strings.hrc>
#include <unotools/streamwrap.hxx>
#include <utility>
#include <vcl/svapp.hxx>

#if ENABLE_ZXING
#include <rtl/strbuf.hxx>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif

#include <BarcodeFormat.h>
#include <BitArray.h>
#include <BitMatrix.h>
#include <MultiFormatWriter.h>
#include <TextUtfEncoding.h>

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif // ENABLE_ZXING

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/drawing/BarCode.hpp>
#include <com/sun/star/drawing/BarCodeErrorCorrection.hpp>
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
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

using namespace css;
using namespace css::uno;
using namespace css::beans;
using namespace css::container;
using namespace css::frame;
using namespace css::io;
using namespace css::lang;
using namespace css::sheet;
using namespace css::text;
using namespace css::drawing;
using namespace css::graphic;

namespace
{
#if ENABLE_ZXING
// Implementation adapted from the answer: https://stackoverflow.com/questions/10789059/create-qr-code-in-vector-image/60638350#60638350
OString ConvertToSVGFormat(const ZXing::BitMatrix& bitmatrix)
{
    OStringBuffer sb;
    const int width = bitmatrix.width();
    const int height = bitmatrix.height();
    ZXing::BitArray row(width);
    sb.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 "
              + OString::number(width) + " " + OString::number(height)
              + "\" stroke=\"none\">\n"
                "<path d=\"");
    for (int i = 0; i < height; ++i)
    {
        bitmatrix.getRow(i, row);
        for (int j = 0; j < width; ++j)
        {
            if (row.get(j))
            {
                sb.append("M" + OString::number(j) + "," + OString::number(i) + "h1v1h-1z");
            }
        }
    }
    sb.append("\"/>\n</svg>");
    return sb.toString();
}

std::string GetBarCodeType(int type)
{
    switch (type)
    {
        case 1:
            return "Code128";
        default:
            return "QRCode";
    }
}

OString GenerateQRCode(std::u16string_view aQRText, tools::Long aQRECC, int aQRBorder, int aQRType)
{
    // Associated ZXing error correction levels (0-8) to our constants arbitrarily.
    int bqrEcc = 1;

    switch (aQRECC)
    {
        case css::drawing::BarCodeErrorCorrection::LOW:
        {
            bqrEcc = 1;
            break;
        }
        case css::drawing::BarCodeErrorCorrection::MEDIUM:
        {
            bqrEcc = 3;
            break;
        }
        case css::drawing::BarCodeErrorCorrection::QUARTILE:
        {
            bqrEcc = 5;
            break;
        }
        case css::drawing::BarCodeErrorCorrection::HIGH:
        {
            bqrEcc = 7;
            break;
        }
    }

    OString o = OUStringToOString(aQRText, RTL_TEXTENCODING_UTF8);
    std::string QRText(o.getStr(), o.getLength());
    ZXing::BarcodeFormat format = ZXing::BarcodeFormatFromString(GetBarCodeType(aQRType));
    auto writer = ZXing::MultiFormatWriter(format).setMargin(aQRBorder).setEccLevel(bqrEcc);
    writer.setEncoding(ZXing::CharacterSet::UTF8);
    ZXing::BitMatrix bitmatrix = writer.encode(ZXing::TextUtfEncoding::FromUtf8(QRText), 0, 0);
    return ConvertToSVGFormat(bitmatrix);
}
#endif

} // anonymous namespace

QrCodeGenDialog::QrCodeGenDialog(weld::Widget* pParent, Reference<XModel> xModel,
                                 bool bEditExisting)
    : GenericDialogController(pParent, "cui/ui/qrcodegen.ui", "QrCodeGenDialog")
    , m_xModel(std::move(xModel))
    , m_xEdittext(m_xBuilder->weld_entry("edit_text"))
    , m_xECC{ m_xBuilder->weld_radio_button("button_low"),
              m_xBuilder->weld_radio_button("button_medium"),
              m_xBuilder->weld_radio_button("button_quartile"),
              m_xBuilder->weld_radio_button("button_high") }
    , m_xSpinBorder(m_xBuilder->weld_spin_button("edit_margin"))
    , m_xComboType(m_xBuilder->weld_combo_box("choose_type"))
#if ENABLE_ZXING
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
    css::drawing::BarCode aBarCode;
    xProps->getPropertyValue("BarCodeProperties") >>= aBarCode;

    m_xEdittext->set_text(aBarCode.Payload);

    //Get Error Correction Constant from selected QR Code
    GetErrorCorrection(aBarCode.ErrorCorrection);

    m_xSpinBorder->set_value(aBarCode.Border);

    m_xComboType->set_active(aBarCode.Type);

    // Mark this as existing shape
    m_xExistingShapeProperties = xProps;
}

short QrCodeGenDialog::run()
{
#if ENABLE_ZXING
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
            catch (const std::exception&)
            {
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
                    mpParent, VclMessageType::Warning, VclButtonsType::Ok,
                    CuiResId(RID_CUISTR_QRCODEDATALONG)));
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
#if ENABLE_ZXING
    css::drawing::BarCode aBarCode;
    aBarCode.Payload = m_xEdittext->get_text();
    aBarCode.Type = m_xComboType->get_active();

    bool bLowECCActive(m_xECC[0]->get_active());
    bool bMediumECCActive(m_xECC[1]->get_active());
    bool bQuartileECCActive(m_xECC[2]->get_active());

    if (bLowECCActive)
    {
        aBarCode.ErrorCorrection = css::drawing::BarCodeErrorCorrection::LOW;
    }
    else if (bMediumECCActive)
    {
        aBarCode.ErrorCorrection = css::drawing::BarCodeErrorCorrection::MEDIUM;
    }
    else if (bQuartileECCActive)
    {
        aBarCode.ErrorCorrection = css::drawing::BarCodeErrorCorrection::QUARTILE;
    }
    else
    {
        aBarCode.ErrorCorrection = css::drawing::BarCodeErrorCorrection::HIGH;
    }

    aBarCode.Border = m_xSpinBorder->get_value();

    // Read svg and replace placeholder texts
    OString aSvgImage = GenerateQRCode(aBarCode.Payload, aBarCode.ErrorCorrection, aBarCode.Border,
                                       aBarCode.Type);

    // Insert/Update graphic
    SvMemoryStream aSvgStream(4096, 4096);
    aSvgStream.WriteOString(aSvgImage);
    Reference<XInputStream> xInputStream(new utl::OSeekableInputStreamWrapper(aSvgStream));
    Reference<XComponentContext> xContext(comphelper::getProcessComponentContext());
    Reference<XGraphicProvider> xProvider = css::graphic::GraphicProvider::create(xContext);

    Sequence<PropertyValue> aMediaProperties{ comphelper::makePropertyValue("InputStream",
                                                                            xInputStream) };
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
    xShapeProps->setPropertyValue("BarCodeProperties", Any(aBarCode));

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

void QrCodeGenDialog::GetErrorCorrection(tools::Long ErrorCorrection)
{
    switch (ErrorCorrection)
    {
        case css::drawing::BarCodeErrorCorrection::LOW:
        {
            m_xECC[0]->set_active(true);
            break;
        }
        case css::drawing::BarCodeErrorCorrection::MEDIUM:
        {
            m_xECC[1]->set_active(true);
            break;
        }
        case css::drawing::BarCodeErrorCorrection::QUARTILE:
        {
            m_xECC[2]->set_active(true);
            break;
        }
        case css::drawing::BarCodeErrorCorrection::HIGH:
        {
            m_xECC[3]->set_active(true);
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
