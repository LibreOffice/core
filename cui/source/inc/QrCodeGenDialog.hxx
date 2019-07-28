/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_CUI_INC_QRCODEGENDIALOG_HXX
#define INCLUDED_CUI_INC_QRCODEGENDIALOG_HXX

#include <vcl/weld.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

// cuitabarea included to use Button Box class for group of Radio Buttons
#include "cuitabarea.hxx"

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
    std::unique_ptr<weld::RadioButton> m_xRadioLow;
    std::unique_ptr<weld::RadioButton> m_xRadioMedium;
    std::unique_ptr<weld::RadioButton> m_xRadioQuartile;
    std::unique_ptr<weld::RadioButton> m_xRadioHigh;
    std::unique_ptr<weld::SpinButton> m_xSpinBorder;

    css::uno::Reference<css::beans::XPropertySet> m_xExistingShapeProperties;

    /* maBox - holds radioButton, helped in writing code. */
    ButtonBox maBox;
    /* Stores which error correction is selected. */
    long m_aECCSelect;
    void SelectErrorCorrection(weld::ToggleButton&);
    void GetErrorCorrection(long);
    static OUString GenerateQrCode(OUString aQrText, int aQrECC, int aQrBorder);

    DECL_LINK(SelectRadio_Impl, weld::ToggleButton&, void);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
