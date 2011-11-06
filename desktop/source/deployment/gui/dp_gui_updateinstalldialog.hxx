/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_INSTALLDIALOG_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_INSTALLDIALOG_HXX

#include "sal/config.h"
#ifndef _SV_BUTTON_HXX
#include "vcl/button.hxx"
#endif
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
