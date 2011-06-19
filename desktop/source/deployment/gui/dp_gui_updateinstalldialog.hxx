/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_INSTALLDIALOG_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_INSTALLDIALOG_HXX

#include "sal/config.h"
#include "vcl/button.hxx"
#include "vcl/fixed.hxx"
#include "vcl/dialog.hxx"
#include "svtools/prgsbar.hxx"
#include "rtl/ref.hxx"
#include <vector>

#include "dp_gui_autoscrolledit.hxx"
/// @HTML

namespace com { namespace sun { namespace star { namespace deployment {
    class XExtensionManager;
}}}}
namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
}}}}
namespace com { namespace sun { namespace star { namespace xml { namespace dom {
    class XNode;
}}}}}
namespace com { namespace sun { namespace star { namespace xml { namespace xpath {
    class XXPathAPI;
}}}}}

class Window;
namespace osl {
    class Condition;
}

namespace dp_gui {

    struct UpdateData;
    class UpdateCommandEnv;


/**
   The modal &ldquo;Download and Installation&rdquo; dialog.
*/
class UpdateInstallDialog: public ModalDialog {
public:
    /**
       Create an instance.

       @param parent
       the parent window, may be null
    */
    UpdateInstallDialog(Window * parent, std::vector<UpdateData> & aVecUpdateData,
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xCtx);

    ~UpdateInstallDialog();

    sal_Bool Close();
    virtual short Execute();

private:
    UpdateInstallDialog(UpdateInstallDialog &); // not defined
    void operator =(UpdateInstallDialog &); // not defined

    class Thread;
    friend class Thread;
    friend class UpdateCommandEnv;

    DECL_LINK(cancelHandler, void *);

    //signals in the dialog that we have finished.
    void updateDone();
    //Writes a particular error into the info listbox.
    enum INSTALL_ERROR
    {
        ERROR_DOWNLOAD,
        ERROR_INSTALLATION,
        ERROR_LICENSE_DECLINED
    };
    void setError(INSTALL_ERROR err, ::rtl::OUString const & sExtension, ::rtl::OUString const & exceptionMessage);
    void setError(::rtl::OUString const & exceptionMessage);
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XExtensionManager > getExtensionManager() const
            { return m_xExtensionManager; }

    rtl::Reference< Thread > m_thread;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xComponentContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XExtensionManager > m_xExtensionManager;
    //Signals that an error occurred during download and installation
    bool m_bError;
    bool m_bNoEntry;
    bool m_bActivated;

    ::rtl::OUString m_sInstalling;
    ::rtl::OUString m_sFinished;
    ::rtl::OUString m_sNoErrors;
    ::rtl::OUString m_sErrorDownload;
    ::rtl::OUString m_sErrorInstallation;
    ::rtl::OUString m_sErrorLicenseDeclined;
    ::rtl::OUString m_sNoInstall;
    ::rtl::OUString m_sThisErrorOccurred;

    FixedText m_ft_action;
    ProgressBar m_statusbar;
    FixedText m_ft_extension_name;
    FixedText m_ft_results;
    AutoScrollEdit m_mle_info;
    FixedLine m_line;
    HelpButton m_help;
    OKButton m_ok;
    CancelButton m_cancel;
};




}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
