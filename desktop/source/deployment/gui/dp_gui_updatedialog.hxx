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
#if 1

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
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/fixedhyper.hxx>
#include <vcl/throbber.hxx>

#include "descedit.hxx"
#include "dp_gui_updatedata.hxx"

/// @HTML

class Image;
class KeyEvent;
class MouseEvent;
class ResId;
class Window;

namespace com { namespace sun { namespace star {
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

    virtual sal_Bool Close();

    virtual short Execute();

    void notifyMenubar( bool bPrepareOnly, bool bRecheckOnly );
    static void createNotifyJob( bool bPrepareOnly,
        com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< rtl::OUString > > &rItemList );

private:
    UpdateDialog(UpdateDialog &); // not defined
    void operator =(UpdateDialog &); // not defined

    struct DisabledUpdate;
    struct SpecificError;
    struct IgnoredUpdate;
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

        sal_uInt16 getItemCount() const;

    private:
        CheckListBox(UpdateDialog::CheckListBox &); // not defined
        void operator =(UpdateDialog::CheckListBox &); // not defined

        virtual void MouseButtonDown(MouseEvent const & event);
        virtual void MouseButtonUp(MouseEvent const & event);
        virtual void KeyInput(KeyEvent const & event);

        void handlePopupMenu( const Point &rPos );

        rtl::OUString m_ignoreUpdate;
        rtl::OUString m_ignoreAllUpdates;
        rtl::OUString m_enableUpdate;
        UpdateDialog & m_dialog;
    };


    friend class CheckListBox;

    sal_uInt16 insertItem( UpdateDialog::Index *pIndex, SvLBoxButtonKind kind );
    void addAdditional( UpdateDialog::Index *pIndex, SvLBoxButtonKind kind );
    bool isIgnoredUpdate( UpdateDialog::Index *pIndex );
    void setIgnoredUpdate( UpdateDialog::Index *pIndex, bool bIgnore, bool bIgnoreAll );

    void addEnabledUpdate( rtl::OUString const & name, dp_gui::UpdateData & data );
    void addDisabledUpdate( UpdateDialog::DisabledUpdate & data );
    void addSpecificError( UpdateDialog::SpecificError & data );

    void checkingDone();

    void enableOk();

    void getIgnoredUpdates();
    void storeIgnoredUpdates();

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
    DECL_LINK(closeHandler, void *);
    DECL_LINK(hyperlink_clicked, FixedHyperlink *);

    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
        m_context;
    FixedText m_checking;
    Throbber m_throbber;
    FixedText m_update;
    UpdateDialog::CheckListBox m_updates;
    CheckBox m_all;
    FixedLine m_description;
    FixedText m_PublisherLabel;
    FixedHyperlink m_PublisherLink;
    FixedText m_ReleaseNotesLabel;
    FixedHyperlink m_ReleaseNotesLink;
    dp_gui::DescriptionEdit m_descriptions;
    FixedLine m_line;
    HelpButton m_help;
    PushButton m_ok;
    PushButton m_close;
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
    rtl::OUString m_ignoredUpdate;
    std::vector< dp_gui::UpdateData > m_enabledUpdates;
    std::vector< UpdateDialog::DisabledUpdate > m_disabledUpdates;
    std::vector< UpdateDialog::SpecificError > m_specificErrors;
    std::vector< UpdateDialog::IgnoredUpdate* > m_ignoredUpdates;
    std::vector< Index* > m_ListboxEntries;
    std::vector< dp_gui::UpdateData > & m_updateData;
    rtl::Reference< UpdateDialog::Thread > m_thread;
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XExtensionManager > m_xExtensionManager;

    Point   m_aFirstLinePos;
    Size    m_aFirstLineSize;
    long    m_nFirstLineDelta;
    long    m_nOneLineMissing;
    sal_uInt16  m_nLastID;
    bool    m_bModified;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
