/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_updatedialog.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 11:04:07 $
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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_UPDATEDIALOG_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_UPDATEDIALOG_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#include <memory>
#include <vector>

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include "com/sun/star/uno/Sequence.hxx"
#endif
#ifndef _RTL_REF_HXX_
#include "rtl/ref.hxx"
#endif
#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif
#ifndef _SVLBOXITM_HXX
#include "svtools/svlbitm.hxx"
#endif
#ifndef _SVX_CHECKLBX_HXX
#include "svx/checklbx.hxx"
#endif
#ifndef _LINK_HXX
#include "tools/link.hxx"
#endif
#ifndef _SOLAR_H
#include "tools/solar.h"
#endif
#ifndef _SV_BUTTON_HXX
#include "vcl/button.hxx"
#endif
#ifndef _SV_DIALOG_HXX
#include "vcl/dialog.hxx"
#endif
#ifndef _SV_FIXED_HXX
#include "vcl/fixed.hxx"
#endif
#ifndef SVTOOLS_FIXEDHYPER_HXX
#include <svtools/fixedhyper.hxx>
#endif

#include "descedit.hxx"

/// @HTML

class Image;
class KeyEvent;
class MouseEvent;
class ResId;
class Window;
namespace com { namespace sun { namespace star {
    namespace awt { class XThrobber; }
    namespace deployment { class XPackageManager; }
    namespace uno { class XComponentContext; }
} } }
namespace dp_gui {
    class SelectedPackageIterator;
    struct UpdateData;
}

namespace dp_gui {

    struct DialogImpl;

/**
   The modal &ldquo;Check for Updates&rdquo; dialog.
*/
class UpdateDialog: public ModalDialog {
public:
    /**
       Create an instance.

       <p>Exactly one of <code>selectedPackages</code> and
       <code>packageManagers</code> must be non-null.</p>

       @param context
       a non-null component context

       @param parent
       the parent window, may be null

       @param selectedPackages
       if non-null, only check for updates for the selected packages

       @param packageManagers
       if non-null, check for updates for all managed packages
    */
    UpdateDialog(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
            const & context,
        Window * parent,
        rtl::Reference<DialogImpl> const & extensionManagerDialog,
        rtl::Reference< dp_gui::SelectedPackageIterator > const &
            selectedPackages,
        com::sun::star::uno::Sequence< com::sun::star::uno::Reference<
            com::sun::star::deployment::XPackageManager > > const &
            packageManagers,
        std::vector< dp_gui::UpdateData > * updateData);

    ~UpdateDialog();

    virtual BOOL Close();

    virtual short Execute();

    void notifyMenubar( bool bPrepareOnly, bool bRecheckOnly );
    static void createNotifyJob( bool bPrepareOnly,
        com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< rtl::OUString > > &rItemList );

private:
    UpdateDialog(UpdateDialog &); // not defined
    void operator =(UpdateDialog &); // not defined

    struct DisabledUpdate;
    struct SpecificError;
    union IndexUnion;
    friend union IndexUnion;
    struct Index;
    friend struct Index;
    class Thread;
    friend class Thread;

    class CheckListBox: public SvxCheckListBox {
    public:
        CheckListBox(
            UpdateDialog & dialog, ResId const & resource,
            Image const & normalStaticImage,
            Image const & highContrastStaticImage);

        virtual ~CheckListBox();

        USHORT getItemCount() const;

    private:
        CheckListBox(UpdateDialog::CheckListBox &); // not defined
        void operator =(UpdateDialog::CheckListBox &); // not defined

        virtual void MouseButtonDown(MouseEvent const & event);

        virtual void MouseButtonUp(MouseEvent const & event);

        virtual void KeyInput(KeyEvent const & event);

        UpdateDialog & m_dialog;
    };

    friend class CheckListBox;

    void insertItem(
        rtl::OUString const & name, USHORT position,
        std::auto_ptr< UpdateDialog::Index const > index,
        SvLBoxButtonKind kind);

    void addAdditional(
        rtl::OUString const & name, USHORT position,
        std::auto_ptr< UpdateDialog::Index const > index,
        SvLBoxButtonKind kind);

    void addEnabledUpdate(
        rtl::OUString const & name, dp_gui::UpdateData const & data);

    void addDisabledUpdate(UpdateDialog::DisabledUpdate const & data);

    void addGeneralError(rtl::OUString const & message);

    void addSpecificError(UpdateDialog::SpecificError const & data);

    void checkingDone();

    void enableOk();

    void initDescription();
    void clearDescription();
    bool showDescription( ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::dom::XNode > const & aUpdateInfo);
    bool showDescription( const String& rDescription, bool bWithPublisher );

    DECL_LINK(selectionHandler, void *);
    DECL_LINK(allHandler, void *);
    DECL_LINK(okHandler, void *);
    DECL_LINK(cancelHandler, void *);
    DECL_LINK(hyperlink_clicked, svt::FixedHyperlink *);

    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
        m_context;
    FixedText m_checking;
    com::sun::star::uno::Reference< com::sun::star::awt::XThrobber > m_throbber;
    FixedText m_update;
    UpdateDialog::CheckListBox m_updates;
    CheckBox m_all;
    FixedLine m_description;
    FixedText m_PublisherLabel;
    svt::FixedHyperlink m_PublisherLink;
    FixedText m_ReleaseNotesLabel;
    svt::FixedHyperlink m_ReleaseNotesLink;
    dp_gui::DescriptionEdit m_descriptions;
    FixedLine m_line;
    HelpButton m_help;
    PushButton m_ok;
    CancelButton m_cancel;
    rtl::OUString m_error;
    rtl::OUString m_none;
    rtl::OUString m_noInstallable;
    rtl::OUString m_failure;
    rtl::OUString m_unknownError;
    rtl::OUString m_noDescription;
    rtl::OUString m_noInstall;
    rtl::OUString m_noDependency;
    rtl::OUString m_noPermission;
    rtl::OUString m_noPermissionVista;
    rtl::OUString m_browserbased;
    rtl::OUString m_version;
    std::vector< dp_gui::UpdateData > m_enabledUpdates;
    std::vector< UpdateDialog::DisabledUpdate > m_disabledUpdates;
    std::vector< rtl::OUString > m_generalErrors;
    std::vector< UpdateDialog::SpecificError > m_specificErrors;
    std::vector< dp_gui::UpdateData > & m_updateData;
    rtl::Reference< UpdateDialog::Thread > m_thread;

    Point m_aFirstLinePos;
    Size m_aFirstLineSize;
    long m_nFirstLineDelta;
    long m_nOneLineMissing;
    // The dialog only knows if we already showed the warning about
    //updating a shared extension during this session.
    const ::rtl::Reference<DialogImpl> m_extensionManagerDialog;
};

}

#endif
