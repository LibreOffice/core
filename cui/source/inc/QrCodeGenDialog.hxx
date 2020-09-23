/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <config_qrcodegen.h>

#include <vcl/weld.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

class QrCodeGenDialog : public weld::GenericDialogController
{
public:
    QrCodeGenDialog(weld::Widget* pParent, css::uno::Reference<css::frame::XModel> xModel,
                    bool bEditExisting);

    virtual short run() override;

protected:
    css::uno::Reference<css::frame::XModel> m_xModel;
    void Apply();

private:
    std::unique_ptr<weld::Entry> m_xEdittext;
    std::unique_ptr<weld::RadioButton> m_xECC[4];
    std::unique_ptr<weld::SpinButton> m_xSpinBorder;
#if ENABLE_QRCODEGEN
    weld::Widget* mpParent;
#endif

    css::uno::Reference<css::beans::XPropertySet> m_xExistingShapeProperties;

    void GetErrorCorrection(long);
    //Function contains QR Code Generating Library Calls
    static OUString GenerateQRCode(OUString aQrText, long aQrECC, int aQrBorder);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
