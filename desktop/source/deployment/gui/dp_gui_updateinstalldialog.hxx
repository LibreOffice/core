/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_updateinstalldialog.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-22 15:03:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2006 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_INSTALLDIALOG_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_INSTALLDIALOG_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif
#ifndef _SV_BUTTON_HXX
#include "vcl/button.hxx"
#endif
#ifndef _SV_FIXED_HXX
#include "vcl/fixed.hxx"
#endif
#ifndef _SV_DIALOG_HXX
#include "vcl/dialog.hxx"
#endif
#ifndef _PRGSBAR_HXX
#include "svtools/prgsbar.hxx"
#endif
#ifndef _RTL_REF_HXX_
#include "rtl/ref.hxx"
#endif
#include <vector>

#include "dp_gui_autoscrolledit.hxx"
/// @HTML

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
class ::osl::Condition;

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

    BOOL Close();
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

    rtl::Reference< Thread > m_thread;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xComponentContext;
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
