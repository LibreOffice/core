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

#include "questiondialog.hxx"

#define DIALOG_WIDTH 240
#define DIALOG_HEIGHT 80
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

class YesNoActionListener : public ::cppu::WeakImplHelper<css::awt::XActionListener>
{
public:
    explicit YesNoActionListener(UnoDialog& rDialog)
        : mrDialog(rDialog)
    {
    }

    virtual void SAL_CALL actionPerformed(const css::awt::ActionEvent& rEvent) override
    {
        if (rEvent.ActionCommand == "buttonok")
        {
            mrDialog.endExecute(true);
        }
        else if (rEvent.ActionCommand == "buttoncancel")
        {
            mrDialog.endExecute(false);
        }
    }
    virtual void SAL_CALL disposing(const css::lang::EventObject&) override {}

private:
    UnoDialog& mrDialog;
};

void QuestionDialog::InitDialog(const OUString& rTitle, const OUString& rText)
{
    sal_Int32 nDialogHeight = DIALOG_HEIGHT;

    // setting the dialog properties
    OUString pNames[]
        = { OUString("Closeable"), OUString("Height"), OUString("Moveable"), OUString("PositionX"),
            OUString("PositionY"), OUString("Title"),  OUString("Width") };

    Any pValues[] = { Any(true),
                      Any(nDialogHeight),
                      Any(true),
                      Any(sal_Int32(245)),
                      Any(sal_Int32(115)),
                      Any(rTitle),
                      Any(sal_Int32(DIALOG_WIDTH)) };

    sal_Int32 nCount = SAL_N_ELEMENTS(pNames);

    Sequence<OUString> aNames(pNames, nCount);
    Sequence<Any> aValues(pValues, nCount);

    mxDialogModelMultiPropertySet->setPropertyValues(aNames, aValues);

    InsertImage(*this, "aboutimage", "private:standardimage/query", 5, 5, 25, 25, false);
    InsertFixedText(*this, "fixedtext", rText, PAGE_POS_X, 6, PAGE_WIDTH, 24, true, 0);

    InsertButton(*this, "buttonok", mxActionListener, DIALOG_WIDTH - 108, nDialogHeight - 20, 50, 1,
                 "Ok");
    InsertButton(*this, "buttoncancel", mxActionListener, DIALOG_WIDTH - 55, nDialogHeight - 20, 50,
                 2, "Cancel");
}

QuestionDialog::QuestionDialog(const Reference<XComponentContext>& rxContext,
                               Reference<XFrame> const& rxFrame, const OUString& rTitle,
                               const OUString& rText)
    : UnoDialog(rxContext, rxFrame)
    , mxActionListener(new YesNoActionListener(*this))
{
    /*    Reference< XFrame > xFrame( mxController->getFrame() );
    Reference< XWindow > xContainerWindow( xFrame->getContainerWindow() );
    Reference< XWindowPeer > xWindowPeer( xContainerWindow, UNO_QUERY_THROW );
    createWindowPeer( xWindowPeer );*/

    InitDialog(rTitle, rText);
}

void QuestionDialog::execute() { UnoDialog::execute(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
