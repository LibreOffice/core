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
#include <tools/link.hxx>
#include <vcl/weld.hxx>

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
class UpdateDialog: public weld::GenericDialogController {
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
        weld::Window * parent,
        const std::vector< css::uno::Reference<
        css::deployment::XPackage > > & vExtensionList,
        std::vector< dp_gui::UpdateData > * updateData);

    virtual ~UpdateDialog() override;

    virtual short run() override;

    void notifyMenubar( bool bPrepareOnly, bool bRecheckOnly );
    static void createNotifyJob( bool bPrepareOnly,
        css::uno::Sequence< css::uno::Sequence< OUString > > const &rItemList );

private:
    UpdateDialog(UpdateDialog const &) = delete;
    UpdateDialog& operator =(UpdateDialog const &) = delete;

    struct DisabledUpdate;
    struct SpecificError;
    struct IgnoredUpdate;
    struct Index;
    friend struct Index;
    class Thread;
    friend class Thread;

    friend class CheckListBox;

    sal_uInt16 insertItem(UpdateDialog::Index *pIndex, bool bEnableCheckBox);
    void addAdditional(UpdateDialog::Index *pIndex, bool bEnableCheckBox);
    bool isIgnoredUpdate( UpdateDialog::Index *pIndex );

    void addEnabledUpdate( OUString const & name, dp_gui::UpdateData const & data );
    void addDisabledUpdate( UpdateDialog::DisabledUpdate const & data );
    void addSpecificError( UpdateDialog::SpecificError const & data );

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

    DECL_LINK(selectionHandler, weld::TreeView&, void);
    DECL_LINK(allHandler, weld::ToggleButton&, void);
    DECL_LINK(okHandler, weld::Button&, void);
    DECL_LINK(closeHandler, weld::Button&, void);
    typedef std::pair<int, int> row_col;
    DECL_LINK(entryToggled, const row_col&, void);

    css::uno::Reference< css::uno::XComponentContext >  m_context;
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
    std::vector< std::unique_ptr<UpdateDialog::IgnoredUpdate> > m_ignoredUpdates;
    std::vector< std::unique_ptr<Index> > m_ListboxEntries;
    std::vector< dp_gui::UpdateData > & m_updateData;
    rtl::Reference< UpdateDialog::Thread > m_thread;
    css::uno::Reference< css::deployment::XExtensionManager > m_xExtensionManager;

    bool    m_bModified;

    std::unique_ptr<weld::Label> m_xChecking;
    std::unique_ptr<weld::Spinner> m_xThrobber;
    std::unique_ptr<weld::Label> m_xUpdate;
    std::unique_ptr<weld::TreeView> m_xUpdates;
    std::unique_ptr<weld::CheckButton> m_xAll;
    std::unique_ptr<weld::Label> m_xDescription;
    std::unique_ptr<weld::Label> m_xPublisherLabel;
    std::unique_ptr<weld::LinkButton> m_xPublisherLink;
    std::unique_ptr<weld::Label> m_xReleaseNotesLabel;
    std::unique_ptr<weld::LinkButton> m_xReleaseNotesLink;
    std::unique_ptr<weld::TextView> m_xDescriptions;
    std::unique_ptr<weld::Button> m_xOk;
    std::unique_ptr<weld::Button> m_xClose;
    std::unique_ptr<weld::Button> m_xHelp;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
