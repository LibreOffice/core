/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/frame/XFrame.hpp>

#include <bluthsndapi.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <sfx2/app.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

SfxBluetoothModel::SendMailResult SfxBluetoothModel::SaveAndSend( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    SaveResult      eSaveResult;
    SendMailResult  eResult = SEND_MAIL_ERROR;
    OUString   aFileName;

    eSaveResult  = SaveDocumentAsFormat( OUString(), xFrame, OUString(), aFileName );
    if( eSaveResult == SAVE_SUCCESSFUL )
    {
        maAttachedDocuments.push_back( aFileName );
        return Send();
    }
    else if( eSaveResult == SAVE_CANCELLED )
        eResult = SEND_MAIL_CANCELLED;

    return eResult;
}

SfxBluetoothModel::SendMailResult SfxBluetoothModel::Send()
{
#if !defined(LINUX) && !defined(MACOSX)
    (void) this; // avoid loplugin:staticmethods
    return SEND_MAIL_ERROR;
#else
    SendMailResult eResult = SEND_MAIL_OK;
    OUString aFilePath = maAttachedDocuments.back();
    OUString aSystemPath;

    osl::File::getSystemPathFromFileURL(aFilePath, aSystemPath);

    OUString aExecutable;
    std::vector<OUString> aArgs;
#ifdef LINUX
    //KDE
    static constexpr OUString KdePackageName = u"bluedevil"_ustr;
    static constexpr OUString KdeToolName = u"bluedevil-sendfile"_ustr;

    //Gnome
    static constexpr OUString GnomePackageName = u"gnome-bluetooth"_ustr;
    static constexpr OUString GnomeToolName = u"bluetooth-sendto"_ustr;

    //XFCE and LXQT
    static constexpr OUString Xfce_Lxqt_PackageName = u"blueman"_ustr;
    static constexpr OUString Xfce_Lxqt_ToolName = u"blueman-sendto"_ustr;

    //Mate
    static constexpr OUString MatePackageName = u"blueberry"_ustr;
    static constexpr OUString MateToolName = u"blueberry-sendto"_ustr;

    OUString aPackageName;
    const OUString& aDesktopEnvironment = Application::GetDesktopEnvironment();

    if(aDesktopEnvironment == u"PLASMA5"_ustr || aDesktopEnvironment == u"PLASMA6"_ustr)
    {
        aPackageName = KdePackageName;
        aExecutable = KdeToolName;
        aArgs.emplace_back("-f");
        aArgs.emplace_back(aSystemPath);
    }
    else if(aDesktopEnvironment == u"GNOME"_ustr)
    {
        aPackageName = GnomePackageName;
        aExecutable = GnomeToolName;
        aArgs.emplace_back(aSystemPath);
    }
    else if(aDesktopEnvironment == u"XFCE"_ustr || aDesktopEnvironment == u"LXQT"_ustr)
    {
        aPackageName = Xfce_Lxqt_PackageName;
        aExecutable = Xfce_Lxqt_ToolName;
        aArgs.emplace_back(aSystemPath);
    }
    else if(aDesktopEnvironment == u"MATE"_ustr)
    {
        aPackageName = MatePackageName;
        aExecutable = MateToolName;
        aArgs.emplace_back(aSystemPath);
    }
#elif defined(MACOSX)
    //macOS
    static constexpr OUString MacToolName = u"Bluetooth File Exchange"_ustr;

    aExecutable = u"open"_ustr;
    aArgs.emplace_back("-a");
    aArgs.emplace_back(MacToolName);
    aArgs.emplace_back(aSystemPath);
#endif

    if (aExecutable.isEmpty())
        return SEND_MAIL_ERROR;

    std::vector<rtl_uString*> aArgsPtrs;
    aArgsPtrs.reserve(aArgs.size());
    for (const auto& aArg : aArgs)
        aArgsPtrs.emplace_back(aArg.pData);

    oslProcess aProcess = nullptr;
    oslProcessError nError = osl_executeProcess(aExecutable.pData, aArgsPtrs.data(),
                                aArgsPtrs.size(), osl_Process_SEARCHPATH, nullptr,
                                nullptr, nullptr, 0, &aProcess);

    if (aProcess)
        osl_freeProcessHandle(aProcess);

    if (nError != osl_Process_E_None)
    {
        eResult = SEND_MAIL_ERROR;
        OUString errMsg;

#ifdef LINUX
        if(nError == osl_Process_E_NotFound)
        {
            errMsg = SfxResId(STR_BLUETOOTH_PACKAGE_ERROR_LINUX)
                                .replaceAll("%TOOL", aExecutable)
                                .replaceAll("%PKG", aPackageName);
        }
        else
        {
            errMsg = SfxResId(STR_BLUETOOTH_ERROR_GENERAL).replaceAll("%TOOL", aExecutable);
        }
#elif defined(MACOSX)
        errMsg = SfxResId(STR_BLUETOOTH_ERROR_GENERAL).replaceAll("%TOOL", MacToolName);
#endif

        weld::Window* pWin = SfxGetpApp()->GetTopWindow();
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
            pWin, VclMessageType::Info, VclButtonsType::Ok, errMsg));
        xBox->run();
    }
    return eResult;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
