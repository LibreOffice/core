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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_UPDATEDIALOG_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_UPDATEDIALOG_HXX

#include <sal/config.h>

#include <vector>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <svtools/svlbitm.hxx>
#include <svx/checklbx.hxx>
#include <tools/link.hxx>
#include <vcl/layout.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/fixedhyper.hxx>
#include <vcl/throbber.hxx>

#include "dp_gui_updatedata.hxx"

/// @HTML

class Image;
class KeyEvent;
class MouseEvent;
class ResId;
namespace vcl { class Window; }

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
        css::uno::Reference< css::uno::XComponentContext > const & context,
        vcl::Window * parent,
        const std::vector< css::uno::Reference<
        css::deployment::XPackage > > & vExtensionList,
        std::vector< dp_gui::UpdateData > * updateData);

    virtual ~UpdateDialog();
    virtual void dispose() SAL_OVERRIDE;

    virtual bool Close() SAL_OVERRIDE;

    virtual short Execute() SAL_OVERRIDE;

    void notifyMenubar( bool bPrepareOnly, bool bRecheckOnly );
    static void createNotifyJob( bool bPrepareOnly,
        css::uno::Sequence< css::uno::Sequence< OUString > > &rItemList );

private:
    UpdateDialog(UpdateDialog &) = delete;
    void operator =(UpdateDialog &) = delete;

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
            vcl::Window* pParent, UpdateDialog & dialog);

        sal_uInt16 getItemCount() const;

    private:
        explicit CheckListBox(UpdateDialog::CheckListBox &) = delete;
        void operator =(UpdateDialog::CheckListBox &) = delete;

        virtual void MouseButtonDown(MouseEvent const & event) SAL_OVERRIDE;
        virtual void MouseButtonUp(MouseEvent const & event) SAL_OVERRIDE;
        virtual void KeyInput(KeyEvent const & event) SAL_OVERRIDE;

        void handlePopupMenu( const Point &rPos );

        OUString m_ignoreUpdate;
        OUString m_ignoreAllUpdates;
        OUString m_enableUpdate;
        UpdateDialog & m_dialog;
    };


    friend class CheckListBox;

    sal_uInt16 insertItem( UpdateDialog::Index *pIndex, SvLBoxButtonKind kind );
    void addAdditional( UpdateDialog::Index *pIndex, SvLBoxButtonKind kind );
    bool isIgnoredUpdate( UpdateDialog::Index *pIndex );
    void setIgnoredUpdate( UpdateDialog::Index *pIndex, bool bIgnore, bool bIgnoreAll );

    void addEnabledUpdate( OUString const & name, dp_gui::UpdateData & data );
    void addDisabledUpdate( UpdateDialog::DisabledUpdate & data );
    void addSpecificError( UpdateDialog::SpecificError & data );

    void checkingDone();

    void enableOk();

    void getIgnoredUpdates();
    void storeIgnoredUpdates();

    void initDescription();
    void clearDescription();
    bool showDescription(css::uno::Reference<
                         css::deployment::XPackage > const & aExtension);
    bool showDescription(std::pair< OUString, OUString > const & pairPublisher,
                         OUString const & sReleaseNotes);
    bool showDescription( css::uno::Reference<
        css::xml::dom::XNode > const & aUpdateInfo);
    bool showDescription( const OUString& rDescription);

    DECL_LINK_TYPED(selectionHandler, SvTreeListBox*, void);
    DECL_LINK_TYPED(allHandler, CheckBox&, void);
    DECL_LINK_TYPED(okHandler, Button*, void);
    DECL_LINK_TYPED(closeHandler, Button*, void);
    DECL_LINK(hyperlink_clicked, FixedHyperlink *);

    css::uno::Reference< css::uno::XComponentContext >  m_context;
    VclPtr<FixedText> m_pchecking;
    VclPtr<Throbber> m_pthrobber;
    VclPtr<FixedText> m_pUpdate;
    VclPtr<VclViewport> m_pContainer;
    VclPtr<UpdateDialog::CheckListBox> m_pUpdates;
    VclPtr<CheckBox> m_pAll;
    VclPtr<FixedText> m_pDescription;
    VclPtr<FixedText> m_pPublisherLabel;
    VclPtr<FixedHyperlink> m_pPublisherLink;
    VclPtr<FixedText> m_pReleaseNotesLabel;
    VclPtr<FixedHyperlink> m_pReleaseNotesLink;
    VclPtr<VclMultiLineEdit> m_pDescriptions;
    VclPtr<HelpButton> m_pHelp;
    VclPtr<PushButton> m_pOk;
    VclPtr<PushButton> m_pClose;
    OUString m_none;
    OUString m_noInstallable;
    OUString m_failure;
    OUString m_unknownError;
    OUString m_noDescription;
    OUString m_noInstall;
    OUString m_noDependency;
    OUString m_noDependencyCurVer;
    OUString m_browserbased;
    OUString m_version;
    OUString m_ignoredUpdate;
    std::vector< dp_gui::UpdateData > m_enabledUpdates;
    std::vector< UpdateDialog::DisabledUpdate > m_disabledUpdates;
    std::vector< UpdateDialog::SpecificError > m_specificErrors;
    std::vector< UpdateDialog::IgnoredUpdate* > m_ignoredUpdates;
    std::vector< Index* > m_ListboxEntries;
    std::vector< dp_gui::UpdateData > & m_updateData;
    rtl::Reference< UpdateDialog::Thread > m_thread;
    css::uno::Reference< css::deployment::XExtensionManager > m_xExtensionManager;

    sal_uInt16  m_nLastID;
    bool    m_bModified;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
