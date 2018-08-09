/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "errordialog.hxx"
#include "informationdialog.hxx"
#include "optimizationstats.hxx"
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <rtl/ustrbuf.hxx>
#include <sal/macros.h>

#define DIALOG_WIDTH 240
#define DIALOG_HEIGHT 60
#define PAGE_POS_X 35
#define PAGE_WIDTH (DIALOG_WIDTH - PAGE_POS_X) - 6

using namespace ::com::sun::star;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

void ErrorDialog::InitDialog()
{
    // setting the dialog properties
    OUString pNames[]
        = { OUString("Closeable"), OUString("Height"), OUString("Moveable"), OUString("PositionX"),
            OUString("PositionY"), OUString("Title"),  OUString("Width") };

    Any pValues[] = { Any(true),
                      Any(sal_Int32(DIALOG_HEIGHT)),
                      Any(true),
                      Any(sal_Int32(245)),
                      Any(sal_Int32(115)),
                      Any(getString(STR_SUN_OPTIMIZATION_WIZARD2)),
                      Any(sal_Int32(DIALOG_WIDTH)) };

    sal_Int32 nCount = SAL_N_ELEMENTS(pNames);

    Sequence<OUString> aNames(pNames, nCount);
    Sequence<Any> aValues(pValues, nCount);

    mxDialogModelMultiPropertySet->setPropertyValues(aNames, aValues);

    css::uno::Reference<css::awt::XItemListener> xItemListener;
    InsertImage(*this, "aboutimage", "private:standardimage/error", 5, 5, 25, 25, false);
    InsertFixedText(*this, "fixedtext", maText, PAGE_POS_X, 6, PAGE_WIDTH, 24, true, 0);
    InsertButton(*this, "button", mxActionListener, DIALOG_WIDTH / 2 - 25, DIALOG_HEIGHT - 20, 50,
                 2, getString(STR_OK));
}

ErrorDialog::ErrorDialog(const Reference<XComponentContext>& rxContext,
                         Reference<XFrame> const& rxFrame, const OUString& rText)
    : UnoDialog(rxContext, rxFrame)
    , ConfigurationAccess(rxContext)
    , mxActionListener(new OKActionListener(*this))
    , maText(rText)
{
    Reference<XFrame> xFrame(mxController->getFrame());
    Reference<XWindow> xContainerWindow(xFrame->getContainerWindow());
    Reference<XWindowPeer> xWindowPeer(xContainerWindow, UNO_QUERY_THROW);
    createWindowPeer(xWindowPeer);

    InitDialog();
}

ErrorDialog::~ErrorDialog() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
