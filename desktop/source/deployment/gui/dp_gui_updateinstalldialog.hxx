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

#pragma once

#include <sal/config.h>
#include <vcl/weld.hxx>
#include <rtl/ref.hxx>

#include <string_view>
#include <vector>

/// @HTML
namespace com::sun::star::deployment {
    class XExtensionManager;
}
namespace com::sun::star::uno {
    class XComponentContext;
}

namespace vcl { class Window; }

namespace dp_gui {

    struct UpdateData;
    class UpdateCommandEnv;


/**
   The modal &ldquo;Download and Installation&rdquo; dialog.
*/
class UpdateInstallDialog : public weld::GenericDialogController
{
public:
    /**
       Create an instance.

       @param parent
       the parent window, may be null
    */
    UpdateInstallDialog(weld::Window* parent, std::vector<UpdateData> & aVecUpdateData,
        css::uno::Reference< css::uno::XComponentContext > const & xCtx);

    virtual ~UpdateInstallDialog() override;

    virtual short run() override;

private:
    UpdateInstallDialog(UpdateInstallDialog const &) = delete;
    UpdateInstallDialog& operator =(UpdateInstallDialog const &) = delete;

    class Thread;
    friend class Thread;
    friend class UpdateCommandEnv;

    DECL_LINK(cancelHandler, weld::Button&, void);

    //signals in the dialog that we have finished.
    void updateDone();
    //Writes a particular error into the info listbox.
    enum INSTALL_ERROR
    {
        ERROR_DOWNLOAD,
        ERROR_INSTALLATION,
        ERROR_LICENSE_DECLINED
    };
    void setError(INSTALL_ERROR err, std::u16string_view sExtension, std::u16string_view exceptionMessage);
    void setError(std::u16string_view exceptionMessage);
    const css::uno::Reference< css::deployment::XExtensionManager >& getExtensionManager() const
            { return m_xExtensionManager; }

    rtl::Reference< Thread > m_thread;
    css::uno::Reference< css::deployment::XExtensionManager > m_xExtensionManager;
    //Signals that an error occurred during download and installation
    bool m_bError;
    bool m_bNoEntry;

    OUString m_sInstalling;
    OUString m_sFinished;
    OUString m_sNoErrors;
    OUString m_sErrorDownload;
    OUString m_sErrorInstallation;
    OUString m_sErrorLicenseDeclined;
    OUString m_sNoInstall;
    OUString m_sThisErrorOccurred;

    std::unique_ptr<weld::Label> m_xFt_action;
    std::unique_ptr<weld::ProgressBar>  m_xStatusbar;
    std::unique_ptr<weld::Label> m_xFt_extension_name;
    std::unique_ptr<weld::TextView> m_xMle_info;
    std::unique_ptr<weld::Button> m_xHelp;
    std::unique_ptr<weld::Button> m_xOk;
    std::unique_ptr<weld::Button> m_xCancel;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
