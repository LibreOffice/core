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

#ifndef INCLUDED_BASCTL_SOURCE_INC_LOCALIZATIONMGR_HXX
#define INCLUDED_BASCTL_SOURCE_INC_LOCALIZATIONMGR_HXX

#include "scriptdocument.hxx"

#include <com/sun/star/resource/XStringResourceManager.hpp>

namespace basctl
{

class Shell;
class DlgEditor;

class LocalizationMgr
{
    ::com::sun::star::uno::Reference
        < ::com::sun::star::resource::XStringResourceManager >  m_xStringResourceManager;

    Shell*                                                      m_pShell;

    ScriptDocument                                              m_aDocument;
    OUString                                                    m_aLibName;

    ::com::sun::star::lang::Locale                              m_aLocaleBeforeBasicStart;

    enum HandleResourceMode
    {
        SET_IDS,
        RESET_IDS,
        RENAME_DIALOG_IDS,
        RENAME_CONTROL_IDS,
        REMOVE_IDS_FROM_RESOURCE,
        MOVE_RESOURCES,
        COPY_RESOURCES
    };
    static sal_Int32 implHandleControlResourceProperties(const css::uno::Any& rControlAny,
            const OUString& aDialogName, const OUString& aCtrlName,
            ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceManager >
            xStringResourceManager, ::com::sun::star::uno::Reference< ::com::sun::star::resource::
            XStringResourceResolver > xSourceStringResolver, HandleResourceMode eMode );

    void enableResourceForAllLibraryDialogs()
    {
        implEnableDisableResourceForAllLibraryDialogs( SET_IDS );
    }
    void disableResourceForAllLibraryDialogs()
    {
        implEnableDisableResourceForAllLibraryDialogs( RESET_IDS );
    }
    void implEnableDisableResourceForAllLibraryDialogs( HandleResourceMode eMode );

public:
    LocalizationMgr(Shell*, ScriptDocument const&, OUString const& aLibName,
        const ::com::sun::star::uno::Reference
            < ::com::sun::star::resource::XStringResourceManager >& xStringResourceManager );
    ::com::sun::star::uno::Reference
        < ::com::sun::star::resource::XStringResourceManager >getStringResourceManager()
    {
        return m_xStringResourceManager;
    }

    bool isLibraryLocalized();

    void handleTranslationbar();

    void handleAddLocales( const ::com::sun::star::uno::Sequence
        < ::com::sun::star::lang::Locale >& aLocaleSeq );

    void handleRemoveLocales( const ::com::sun::star::uno::Sequence
        < ::com::sun::star::lang::Locale >& aLocaleSeq );

    void handleSetDefaultLocale(const css::lang::Locale& rLocale);

    void handleSetCurrentLocale(const css::lang::Locale& rLocale);

    void handleBasicStarted();

    void handleBasicStopped();

    static void setControlResourceIDsForNewEditorObject(DlgEditor* pEditor,
        const css::uno::Any& rControlAny, const OUString& aCtrlName);

    static void renameControlResourceIDsForEditorObject(DlgEditor* pEditor,
        const css::uno::Any& rControlAny, const OUString& aNewCtrlName);

    static void deleteControlResourceIDsForDeletedEditorObject(DlgEditor* pEditor,
        const css::uno::Any& rControlAny, const OUString& aCtrlName);

    static void setStringResourceAtDialog( const ScriptDocument& rDocument, const OUString& aLibName, const OUString& aDlgName,
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > xDialogModel );

    static void renameStringResourceIDs( const ScriptDocument& rDocument, const OUString& aLibName, const OUString& aDlgName,
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > xDialogModel );

    static void removeResourceForDialog( const ScriptDocument& rDocument, const OUString& aLibName, const OUString& aDlgName,
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > xDialogModel );

    static ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceManager >
        getStringResourceFromDialogLibrary( ::com::sun::star::uno::Reference
            < ::com::sun::star::container::XNameContainer > xDialogLib );

    // Clipboard / Drag & Drop
    static void resetResourceForDialog(
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > xDialogModel,
        ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceManager > xStringResourceManager );

    static void setResourceIDsForDialog(
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > xDialogModel,
        ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceManager > xStringResourceManager );

    static void copyResourcesForPastedEditorObject( DlgEditor* pEditor,
        const css::uno::Any& rControlAny, const OUString& aCtrlName,
        ::com::sun::star::uno::Reference< ::com::sun::star::resource::
        XStringResourceResolver > xSourceStringResolver );

    static void copyResourceForDroppedDialog(
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > xDialogModel,
        const OUString& aDialogName,
        ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceManager > xStringResourceManager,
        ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceResolver > xSourceStringResolver );

    static void copyResourceForDialog(
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xDialogModel,
        const ::com::sun::star::uno::Reference< ::com::sun::star::resource::
            XStringResourceResolver >& xSourceStringResolver,
        const ::com::sun::star::uno::Reference< ::com::sun::star::resource::
            XStringResourceManager >& xTargetStringResourceManager );
};

}

#endif // INCLUDED_BASCTL_SOURCE_INC_LOCALIZATIONMGR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
