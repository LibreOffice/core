/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <framework/desktop.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>

#include <vcl/svapp.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/DialogController.hxx>
#include <vcl/weld/weld.hxx>
#include <sal/main.h>

#include <iostream>

namespace
{
class TipOfTheDayDialog : public weld::GenericDialogController
{
public:
    TipOfTheDayDialog(weld::Window* pParent = nullptr);
    DECL_LINK(OnNextClick, weld::Button&, void);

private:
    std::unique_ptr<weld::Label> m_pTextLabel;
    std::unique_ptr<weld::Button> m_pNextButton;
    sal_Int32 m_nCounter = 0;
};

IMPL_LINK_NOARG(TipOfTheDayDialog, OnNextClick, weld::Button&, void)
{
    ++m_nCounter;
    m_pTextLabel->set_label(u"Here you will see tip of the day #"_ustr
                            + OUString::number(m_nCounter) + ".");
}

TipOfTheDayDialog::TipOfTheDayDialog(weld::Window* pParent)
    : weld::GenericDialogController(pParent, u"cui/ui/tipofthedaydialog.ui"_ustr,
                                    u"TipOfTheDayDialog"_ustr)
    , m_pTextLabel(m_xBuilder->weld_label(u"lbText"_ustr))
    , m_pNextButton(m_xBuilder->weld_button(u"btnNext"_ustr))
{
    m_pNextButton->connect_clicked(LINK(this, TipOfTheDayDialog, OnNextClick));
}

class TheApplication : public Application
{
public:
    virtual int Main();
};

int TheApplication::Main()
{
    TipOfTheDayDialog dialog;
    dialog.run();
    return 0;
}
}

SAL_IMPLEMENT_MAIN()
{
    try
    {
        TheApplication aApp;

        auto xContext = cppu::defaultBootstrap_InitialComponentContext();
        css::uno::Reference<css::lang::XMultiServiceFactory> xServiceManager(
            xContext->getServiceManager(), css::uno::UNO_QUERY);
        comphelper::setProcessServiceFactory(xServiceManager);
        LanguageTag::setConfiguredSystemLanguage(MsLangId::getSystemLanguage());
        InitVCL();

        aApp.Main();

        framework::getDesktop(::comphelper::getProcessComponentContext())->terminate();
        DeInitVCL();
        comphelper::setProcessServiceFactory(nullptr);
    }
    catch (...)
    {
        std::cerr << "An exception has occurred\n";
        return 1;
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
