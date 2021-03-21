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
#include <comphelper/processfactory.hxx>
#include <rtl/ustring.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxresid.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/weld.hxx>
#include <openuriexternally.hxx>
#include <comphelper/lok.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <sfx2/viewsh.hxx>
#include <sfx2/strings.hrc>

namespace {

class URITools
{
private:
    Timer aOpenURITimer;
    OUString msURI;
    bool mbHandleSystemShellExecuteException;
    DECL_LINK(onOpenURI, Timer*, void);

public:
    URITools()
        : mbHandleSystemShellExecuteException(false)
    {
    }
    void openURI(const OUString& sURI, bool bHandleSystemShellExecuteException);
};

}

void URITools::openURI(const OUString& sURI, bool bHandleSystemShellExecuteException)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        if (SfxViewShell* pViewShell = SfxViewShell::Current())
        {
            pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_HYPERLINK_CLICKED,
                                                   sURI.toUtf8().getStr());
        }
        delete this;
        return;
    }

    mbHandleSystemShellExecuteException = bHandleSystemShellExecuteException;
    msURI = sURI;

    // tdf#116305 Workaround: Use timer to bring browsers to the front
    aOpenURITimer.SetInvokeHandler(LINK(this, URITools, onOpenURI));
#ifdef _WIN32
    // 200ms seems to be the best compromise between responsiveness and success rate
    aOpenURITimer.SetTimeout(200);
#else
    aOpenURITimer.SetTimeout(0);
#endif
    aOpenURITimer.SetDebugName("sfx2::openUriExternallyTimer");
    aOpenURITimer.Start();
}

IMPL_LINK_NOARG(URITools, onOpenURI, Timer*, void)
{
    css::uno::Reference< css::system::XSystemShellExecute > exec(
        css::system::SystemShellExecute::create(comphelper::getProcessComponentContext()));
    try {
        exec->execute(
            msURI, OUString(),
            css::system::SystemShellExecuteFlags::URIS_ONLY);
        return;
    } catch (css::lang::IllegalArgumentException & e) {
        if (e.ArgumentPosition != 0) {
            throw css::uno::RuntimeException(
                "unexpected IllegalArgumentException: " + e.Message);
        }
        SolarMutexGuard g;
        weld::Window *pWindow = SfxGetpApp()->GetTopWindow();
        std::unique_ptr<weld::MessageDialog> eb(Application::CreateMessageDialog(pWindow,
                                                                 VclMessageType::Warning, VclButtonsType::Ok,
                                                                 SfxResId(STR_NO_ABS_URI_REF)));
        eb->set_primary_text(eb->get_primary_text().replaceFirst("$(ARG1)", msURI));
        eb->run();
    } catch (css::system::SystemShellExecuteException & e) {
        if (!mbHandleSystemShellExecuteException) {
            throw;
        }
        SolarMutexGuard g;
        weld::Window *pWindow = SfxGetpApp()->GetTopWindow();
        std::unique_ptr<weld::MessageDialog> eb(Application::CreateMessageDialog(pWindow,
                                                                 VclMessageType::Warning, VclButtonsType::Ok,
                                                                 SfxResId(STR_NO_WEBBROWSER_FOUND)));
        eb->set_primary_text(
            eb->get_primary_text().replaceFirst("$(ARG1)", msURI)
            .replaceFirst("$(ARG2)", OUString::number(e.PosixError))
            .replaceFirst("$(ARG3)", e.Message));
            //TODO: avoid subsequent replaceFirst acting on previous replacement
        eb->run();
    }
    delete this;
}

void sfx2::openUriExternally(const OUString& sURI, bool bHandleSystemShellExecuteException)
{
    URITools* uriTools = new URITools;
    uriTools->openURI(sURI, bHandleSystemShellExecuteException);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
