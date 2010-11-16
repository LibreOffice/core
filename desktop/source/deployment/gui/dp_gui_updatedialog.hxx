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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_UPDATEDIALOG_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_UPDATEDIALOG_HXX

#include "sal/config.h"

#include <memory>
#include <vector>
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "svtools/svlbitm.hxx"
#include "svx/checklbx.hxx"
#include "tools/link.hxx"
#include "tools/solar.h"
#include "vcl/button.hxx"
#include "vcl/dialog.hxx"
#include "vcl/fixed.hxx"
#include <svtools/fixedhyper.hxx>

#include "descedit.hxx"
#include "dp_gui_updatedata.hxx"

/// @HTML

class Image;
class KeyEvent;
class MouseEvent;
class ResId;
class Window;

namespace com { namespace sun { namespace star {
    namespace awt { class XThrobber; }
    namespace deployment { class XExtensionManager;
                           class XPackage; }
    namespace uno { class XComponentContext; }
} } }

namespace dp_gui {
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

       @param vExtensionList
       check for updates for the contained extensions. There must only be one extension with
       a particular identifier. If one extension is installed in several repositories, then the
       one with the highest version must be used, because it contains the latest known update
       information.
    */
    UpdateDialog(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > const & context,
        Window * parent,
        const std::vector< com::sun::star::uno::Reference<
        com::sun::star::deployment::XPackage > > & vExtensionList,
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
            Image const & normalStaticImage);

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
    void addSpecificError(UpdateDialog::SpecificError const & data);

    void checkingDone();

    void enableOk();

    void initDescription();
    void clearDescription();
    bool showDescription(::com::sun::star::uno::Reference<
                         ::com::sun::star::deployment::XPackage > const & aExtension);
    bool showDescription(std::pair< rtl::OUString, rtl::OUString > const & pairPublisher,
                         rtl::OUString const & sReleaseNotes);
    bool showDescription( ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::dom::XNode > const & aUpdateInfo);
    bool showDescription( const String& rDescription, bool bWithPublisher );
    bool isReadOnly( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage ) const;

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
    rtl::OUString m_noDependencyCurVer;
    rtl::OUString m_browserbased;
    rtl::OUString m_version;
    std::vector< dp_gui::UpdateData > m_enabledUpdates;
    std::vector< UpdateDialog::DisabledUpdate > m_disabledUpdates;
    std::vector< rtl::OUString > m_generalErrors;
    std::vector< UpdateDialog::SpecificError > m_specificErrors;
    std::vector< dp_gui::UpdateData > & m_updateData;
    rtl::Reference< UpdateDialog::Thread > m_thread;
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XExtensionManager > m_xExtensionManager;

    Point m_aFirstLinePos;
    Size m_aFirstLineSize;
    long m_nFirstLineDelta;
    long m_nOneLineMissing;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
