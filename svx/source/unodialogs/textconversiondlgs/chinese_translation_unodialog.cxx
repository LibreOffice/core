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


#include <svx/chinese_translation_unodialog.hxx>
#include "chinese_translationdialog.hxx"
#include <vcl/svapp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

namespace textconversiondlgs
{
using namespace ::com::sun::star;

ChineseTranslation_UnoDialog::ChineseTranslation_UnoDialog(const css::uno::Reference< css::awt::XWindow >& xParentWindow)
{
    m_xParentWindow = xParentWindow;
}

ChineseTranslation_UnoDialog::~ChineseTranslation_UnoDialog()
{
    SolarMutexGuard aSolarGuard;
    impl_DeleteDialog();
}

void ChineseTranslation_UnoDialog::impl_DeleteDialog()
{
    if (m_xDialog)
    {
        m_xDialog->response(RET_CANCEL);
        m_xDialog.reset();
    }
}

sal_Int16 ChineseTranslation_UnoDialog::execute()
{
    sal_Int16 nRet = ui::dialogs::ExecutableDialogResults::CANCEL;
    {
        SolarMutexGuard aSolarGuard;
        if (!m_xDialog)
            m_xDialog.reset(new ChineseTranslationDialog(Application::GetFrameWeld(m_xParentWindow)));
        nRet = m_xDialog->run();
        if (nRet == RET_OK)
           nRet=ui::dialogs::ExecutableDialogResults::OK;
    }
    return nRet;
}

bool ChineseTranslation_UnoDialog::getIsDirectionToSimplified()
{
    bool bDirectionToSimplified = true;
    bool bTranslateCommonTerms = false;

    {
        SolarMutexGuard aSolarGuard;
        m_xDialog->getSettings(bDirectionToSimplified, bTranslateCommonTerms);
    }
    return bDirectionToSimplified;
}

bool ChineseTranslation_UnoDialog::getIsTranslateCommonTerms()
{
    bool bDirectionToSimplified = true;
    bool bTranslateCommonTerms = false;

    {
        SolarMutexGuard aSolarGuard;
        m_xDialog->getSettings(bDirectionToSimplified, bTranslateCommonTerms);
    }
    return bTranslateCommonTerms;
}

} //namespace textconversiondlgs

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
