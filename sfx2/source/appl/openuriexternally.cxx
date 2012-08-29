/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2012 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
 *   (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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

namespace {

namespace css = com::sun::star;

}

bool sfx2::openUriExternally(
    rtl::OUString const & uri, bool handleSystemShellExecuteException)
{
    css::uno::Reference< css::system::XSystemShellExecute > exec(
        css::system::SystemShellExecute::create(comphelper::getProcessComponentContext()));
    try {
        exec->execute(
            uri, rtl::OUString(),
            css::system::SystemShellExecuteFlags::URIS_ONLY);
        return true;
    } catch (css::lang::IllegalArgumentException & e) {
        if (e.ArgumentPosition != 0) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "unexpected IllegalArgumentException: "))
                 + e.Message),
                css::uno::Reference< css::uno::XInterface >());
        }
        SolarMutexGuard g;
        ErrorBox eb(
            SfxGetpApp()->GetTopWindow(), SfxResId(MSG_ERR_NO_ABS_URI_REF));
        String msg(eb.GetMessText());
        msg.SearchAndReplaceAscii("$(ARG1)", uri);
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
