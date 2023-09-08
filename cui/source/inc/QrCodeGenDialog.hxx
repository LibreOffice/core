/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <config_zxing.h>

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
    static bool runAsync(const std::shared_ptr<QrCodeGenDialog>& rController,
                         const std::function<void(sal_Int32)>& rFunc);

    void Apply();
    weld::Widget* GetParent() { return mpParent; }

private:
    css::uno::Reference<css::frame::XModel> m_xModel;
    std::unique_ptr<weld::TextView> m_xEdittext;
    std::unique_ptr<weld::RadioButton> m_xECC[4];
    std::unique_ptr<weld::SpinButton> m_xSpinBorder;
    std::unique_ptr<weld::ComboBox> m_xComboType;

#if ENABLE_ZXING
    weld::Widget* mpParent;
#endif

    css::uno::Reference<css::beans::XPropertySet> m_xExistingShapeProperties;

    void GetErrorCorrection(tools::Long);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
