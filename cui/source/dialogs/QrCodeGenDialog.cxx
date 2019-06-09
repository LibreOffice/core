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


QrCodeGenDialog::QrCodeGenDialog(weld::Widget* pParent, Reference<XModel> xModel,
                                         bool bEditExisting)
    : QrCodeGenDialogBase(pParent, std::move(xModel), "cui/ui/qrcodegen.ui",
                              "QrCodeGenDialog")
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
        m_aECCSelect = ECC::Low;
        m_xRadioLow->set_active(true);
        return;
    }

    Reference<container::XIndexAccess> xIndexAccess(m_xModel->getCurrentSelection(),
                                                    UNO_QUERY_THROW);
    Reference<XPropertySet> xProps(xIndexAccess->getByIndex(0), UNO_QUERY_THROW);

    // Read properties from selected qr code
    xProps->getPropertyValue("QrCodeId") >>= m_aQrCodeId;

    OUString aURLText;
    xProps->getPropertyValue("QrCodeText") >>= aURLText;
    m_xEdittext->set_text(aURLText);

    /* Work with numbering and synchronising radio Buttons {87-94 some hack required}*/
    ECC aECCValue;
    xProps->getPropertyValue("QrCodeECC") >>= aECCValue;

    Link<weld::Button&,void> aLink( LINK( this, QrCodeGenDialog, SelectRadio_Impl ) );
    m_xRadioLow->connect_clicked( aLink );
    m_xRadioMedium->connect_clicked( aLink );
    m_xRadioQuartile->connect_clicked( aLink );
    m_xRadioHigh->connect_clicked( aLink );

    OUString aBorderValue;
    xProps->getPropertyValue("QrCodeBorder") >>= aBorderValue;
    m_xSpinBorder->set_value(aBorderValue);

    // Mark this as existing shape
    m_xExistingShapeProperties = xProps;
}

void QrCodeGenDialog::Apply()
{
    if (m_aQrCodeId.isEmpty())
        m_aQrCodeId
            = OStringToOUString(comphelper::xml::generateGUIDString(), RTL_TEXTENCODING_ASCII_US);

    OUString aQrText(m_xEdittext->get_text());
    ECC aQrECC(m_aECCSelect);
    int aQrBorder(m_xSpinBorder->get_value());

    // Read svg and replace placeholder texts
    OUString aSvgImage("");

    //Use qrCode Generating Library. and generate svg to return in aSvgImage.

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

    // Set signature line properties
    xShapeProps->setPropertyValue("IsQrCode", Any(true));
    xShapeProps->setPropertyValue("QrCodeId", Any(m_aQrCodeId));
    if (!aURLText.isEmpty())
        xShapeProps->setPropertyValue("QrCodeText", Any(aURLText));
    if (!aECCValue.isEmpty())
        xShapeProps->setPropertyValue("QrCodeECC", Any(aECCValue));
    if (!aBorderValue.isEmpty())
        xShapeProps->setPropertyValue("QrCodeBorder", Any(aBorderValue));

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
    if(!rButton.get_active())
        rButton.set_active(true);

    SelectECCType(rButton);

    /*
    switch(m_aECCSelect)
    {
        default:
        case ECC::Low:
        {
            m_xRadioLow->set_active(false);
            break;
        }
        case ECC::Medium:
        {
            m_xRadioMedium->set_active(false);
            break;
        }
        case ECC::Quartile:
        {
            m_xRadioQuartile->set_active(false);
            break;
        }
        case ECC::High:
        {
            m_xRadioHigh->set_active(false);
            break;
        }
    }
    */
}

void QrCodeGenDialog::SelectECCType(weld::ToggleButton& rButton)
{
    sal_Int32 nPos = maBox.GetButtonPos(&rButton);
    if (nPos != -1 && nPos != maBox.GetCurrentButtonPos())
    {
        maBox.SelectButton(&rButton);
        m_aECCSelect = static_cast<ECC>(maBox.GetCurrentButtonPos());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
