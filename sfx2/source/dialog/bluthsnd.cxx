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
#include <sfx2/app.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>
#include <vcl/DesktopType.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/weld/MessageDialog.hxx>

#if defined(LINUX) || defined(MACOSX)
#include <sys/wait.h>
#endif

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

    static constexpr int CommandNotFound = 127;
#elif defined(MACOSX)
    //macOS
    static constexpr OUString MacToolName = u"Bluetooth File Exchange"_ustr;
#endif

    SendMailResult eResult = SEND_MAIL_OK;
    OUString aFilePath = maAttachedDocuments.back();
    OUString aSystemPath;

    osl::File::getSystemPathFromFileURL(aFilePath, aSystemPath);

    OUString aSendCommand;
    OUString aToolName;
#ifdef LINUX
    OUString aPackageName;
    const DesktopType eDesktopEnvironment = Application::GetDesktopEnvironment();

    if (eDesktopEnvironment == DesktopType::Plasma5 || eDesktopEnvironment == DesktopType::Plasma6)
    {
        aPackageName = KdePackageName;
        aToolName =  KdeToolName;
        aSendCommand = aToolName + " -f " + aSystemPath;
    }
    else if (eDesktopEnvironment == DesktopType::GNOME)
    {
        aPackageName = GnomePackageName;
        aToolName = GnomeToolName;
        aSendCommand = aToolName + " " + aSystemPath;
    }
    else if (eDesktopEnvironment == DesktopType::Xfce || eDesktopEnvironment == DesktopType::LXQt)
    {
        aPackageName = Xfce_Lxqt_PackageName;
        aToolName = Xfce_Lxqt_ToolName;
        aSendCommand = aToolName + " " + aSystemPath;
    }
    else if (eDesktopEnvironment == DesktopType::MATE)
    {
        aPackageName = MatePackageName;
        aToolName = MateToolName;
        aSendCommand = aToolName + " " + aSystemPath;
    }
#elif defined(MACOSX)
    aToolName = MacToolName;
    aSendCommand = "open -a \"" + aToolName + "\" " + aSystemPath;
#endif

    OString aRawCommand = OUStringToOString( aSendCommand, RTL_TEXTENCODING_UTF8);

    int status = system(aRawCommand.getStr());
    if (status == -1 ||
        !WIFEXITED(status) ||
        WEXITSTATUS(status) != 0)
    {
        eResult = SEND_MAIL_ERROR;
        OUString errMsg;

#ifdef LINUX
        if(WIFEXITED(status) && WEXITSTATUS(status) == CommandNotFound)
        {
            errMsg = SfxResId(STR_BLUETOOTH_PACKAGE_ERROR_LINUX)
                                .replaceAll("%TOOL", aToolName)
                                .replaceAll("%PKG", aPackageName);
        }
#elif defined(MACOSX)
        errMsg = SfxResId(STR_BLUETOOTH_ERROR_MACOS).replaceAll("%TOOL", aToolName);
#endif

        if(!errMsg.isEmpty())
        {
            weld::Window* pWin = SfxGetpApp()->GetTopWindow();
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
                pWin, VclMessageType::Info, VclButtonsType::Ok, errMsg));
            xBox->run();
        }
    }
    return eResult;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
