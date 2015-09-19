/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>

#include <comphelper/anytostring.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <tools/date.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>

#include <officecfg/Office/Common.hxx>

#include "ApplicationStartupDialog.hxx"

using namespace css::system;
using namespace css::uno;
using namespace comphelper;

SfxApplicationStartupDialog::SfxApplicationStartupDialog(Window* pParent) :
    SfxModalDialog(pParent, "ApplicationStartupDialog", "sfx/ui/applicationstartupdialog.ui")
{
    get(mpDontShowAgainCB, "checkbutton1");
    get(mpOkButton, "ok");
    get(mpLinkButton, "link");
    get(mpLinkButton1, "linkbutton1");
    get(mpLinkButton2, "linkbutton2");
    mpOkButton->SetClickHdl(LINK(this, SfxApplicationStartupDialog, OkButtonClick));
    mpLinkButton->SetClickHdl(LINK(this, SfxApplicationStartupDialog, LinkClick));
    mpLinkButton1->SetClickHdl(LINK(this, SfxApplicationStartupDialog, LinkButton1Click));
    mpLinkButton2->SetClickHdl(LINK(this, SfxApplicationStartupDialog, LinkButton2Click));
}

SfxApplicationStartupDialog::~SfxApplicationStartupDialog()
{
    disposeOnce();
}

void SfxApplicationStartupDialog::dispose()
{
    mpDontShowAgainCB.clear();
    mpOkButton.clear();
    mpLinkButton.clear();
    mpLinkButton1.clear();
    mpLinkButton2.clear();
    SfxModalDialog::dispose();
}

IMPL_LINK_NOARG(SfxApplicationStartupDialog, OkButtonClick)
{
    if( mpDontShowAgainCB->IsChecked() )
    {
        std::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create() );
        officecfg::Office::Common::Startup::StartupMessage::set(false, batch);
        officecfg::Office::Common::Startup::StartupMessageLastShown::set(::Date(::Date::SYSTEM).GetDate(), batch);
        batch->commit();
    }

    EndDialog( RET_OK );

    return 0;
}

IMPL_LINK_NOARG(SfxApplicationStartupDialog, LinkClick)
{
    try
    {
        Reference<XSystemShellExecute> xSystemShellExecute(SystemShellExecute::create(comphelper::getProcessComponentContext()));
        xSystemShellExecute->execute(mpLinkButton->GetURL(), OUString(), SystemShellExecuteFlags::URIS_ONLY );
    }
    catch (const Exception&)
    {
        Any aException(cppu::getCaughtException());
        OUString aMessage(comphelper::anyToString(aException));
        const SolarMutexGuard guard;
        ErrorBox aErrorBox( NULL, WB_OK, aMessage);
        aErrorBox.SetText(GetText());
        aErrorBox.Execute();
    }
    return 0;
}

IMPL_LINK_NOARG(SfxApplicationStartupDialog, LinkButton1Click)
{
    try
    {
        Reference<XSystemShellExecute> xSystemShellExecute(SystemShellExecute::create(comphelper::getProcessComponentContext()));
        xSystemShellExecute->execute(mpLinkButton1->GetURL(), OUString(), SystemShellExecuteFlags::URIS_ONLY );
    }
    catch (const Exception&)
    {
        Any aException(cppu::getCaughtException());
        OUString aMessage(comphelper::anyToString(aException));
        const SolarMutexGuard guard;
        ErrorBox aErrorBox( NULL, WB_OK, aMessage);
        aErrorBox.SetText(GetText());
        aErrorBox.Execute();
    }
    return 0;
}

IMPL_LINK_NOARG(SfxApplicationStartupDialog, LinkButton2Click)
{
    try
    {
        Reference<XSystemShellExecute> xSystemShellExecute(SystemShellExecute::create(comphelper::getProcessComponentContext()));
        xSystemShellExecute->execute(mpLinkButton2->GetURL(), OUString(), SystemShellExecuteFlags::URIS_ONLY );
    }
    catch (const Exception&)
    {
        Any aException(cppu::getCaughtException());
        OUString aMessage(comphelper::anyToString(aException));
        const SolarMutexGuard guard;
        ErrorBox aErrorBox( NULL, WB_OK, aMessage);
        aErrorBox.SetText(GetText());
        aErrorBox.Execute();
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
