/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/system/SystemShellExecute.hpp"
#include "com/sun/star/system/SystemShellExecuteException.hpp"
#include "com/sun/star/system/SystemShellExecuteFlags.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "comphelper/processfactory.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sfx2/app.hxx"
#include "sfx2/sfxresid.hxx"
#include "tools/string.hxx"
#include "vcl/msgbox.hxx"
#include "vcl/svapp.hxx"

#include "openuriexternally.hxx"

#include "app.hrc"

bool sfx2::openUriExternally(
    OUString const & uri, bool handleSystemShellExecuteException)
{
    css::uno::Reference< css::system::XSystemShellExecute > exec(
        css::system::SystemShellExecute::create(comphelper::getProcessComponentContext()));
    try {
        exec->execute(
            uri, OUString(),
            css::system::SystemShellExecuteFlags::URIS_ONLY);
        return true;
    } catch (css::lang::IllegalArgumentException & e) {
        if (e.ArgumentPosition != 0) {
            throw css::uno::RuntimeException(
                (OUString(
                        "unexpected IllegalArgumentException: ")
                 + e.Message),
                css::uno::Reference< css::uno::XInterface >());
        }
        SolarMutexGuard g;
        ErrorBox eb(
            SfxGetpApp()->GetTopWindow(), SfxResId(MSG_ERR_NO_ABS_URI_REF));
        OUString msg(eb.GetMessText());
        msg = msg.replaceFirst("$(ARG1)", uri);
        eb.SetMessText(msg);
        eb.Execute();
    } catch (css::system::SystemShellExecuteException &) {
        if (!handleSystemShellExecuteException) {
            throw;
        }
        SolarMutexGuard g;
        ErrorBox(
            SfxGetpApp()->GetTopWindow(),
            SfxResId(MSG_ERR_NO_WEBBROWSER_FOUND)).
            Execute();
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
