/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteException.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <comphelper/processfactory.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxresid.hxx>
#include <vcl/layout.hxx>
#include <vcl/svapp.hxx>

#include "openuriexternally.hxx"

#include "sfx2/strings.hrc"

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
                "unexpected IllegalArgumentException: " + e.Message);
        }
        SolarMutexGuard g;
        ScopedVclPtrInstance<MessageDialog> eb(
            SfxGetpApp()->GetTopWindow(), SfxResId(STR_NO_ABS_URI_REF));
        eb->set_primary_text(eb->get_primary_text().replaceFirst("$(ARG1)", uri));
        eb->Execute();
    } catch (css::system::SystemShellExecuteException & e) {
        if (!handleSystemShellExecuteException) {
            throw;
        }
        SolarMutexGuard g;
        ScopedVclPtrInstance<MessageDialog> eb(
            SfxGetpApp()->GetTopWindow(),
            SfxResId(STR_NO_WEBBROWSER_FOUND));
        eb->set_primary_text(
            eb->get_primary_text().replaceFirst("$(ARG1)", uri)
            .replaceFirst("$(ARG2)", OUString::number(e.PosixError))
            .replaceFirst("$(ARG3)", e.Message));
            //TODO: avoid subsequent replaceFirst acting on previous replacement
        eb->Execute();
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
