/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <DocumentConverterDialog.hxx>

#include <comphelper/processfactory.hxx>
#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>
#include <utility>
#include <vcl/weld.hxx>
#include <sal/log.hxx>

#if defined(SYSTEM_QRCODEGEN)
#include <qrcodegen/QrCode.hpp>
#else
#include <QrCode.hpp>
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
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <config_fuzzers.h>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
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
using namespace qrcodegen;

DocumentConverterDialog::DocumentConverterDialog(weld::Widget* pParent)
    : GenericDialogController(pParent, "cui/ui/documentconverterdialog.ui",
                              "DocumentConverterDialog")
{
}

short DocumentConverterDialog::run()
{
    short nRet = GenericDialogController::run();
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
