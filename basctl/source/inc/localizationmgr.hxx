/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: localizationmgr.hxx,v $
 * $Revision: 1.9 $
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

#ifndef _LOCALIZATIONMGR_HXX
#define _LOCALIZATIONMGR_HXX

#include <com/sun/star/resource/XStringResourceManager.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

#include "scriptdocument.hxx"

class BasicIDEShell;
class DlgEditor;

class LocalizationMgr
{
    ::com::sun::star::uno::Reference
        < ::com::sun::star::resource::XStringResourceManager >  m_xStringResourceManager;

    BasicIDEShell*                                              m_pIDEShell;

    ScriptDocument                                              m_aDocument;
    String                                                      m_aLibName;

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
    static sal_Int32 implHandleControlResourceProperties( ::com::sun::star::uno::Any aControlAny,
            const ::rtl::OUString& aDialogName, const ::rtl::OUString& aCtrlName,
            ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceManager >
            xStringResourceManager, ::com::sun::star::uno::Reference< ::com::sun::star::resource::
            XStringResourceResolver > xSourceStringResolver, HandleResourceMode eMode );

    void enableResourceForAllLibraryDialogs( void )
    {
        implEnableDisableResourceForAllLibraryDialogs( SET_IDS );
    }
    void disableResourceForAllLibraryDialogs( void )
    {
        implEnableDisableResourceForAllLibraryDialogs( RESET_IDS );
    }
    void implEnableDisableResourceForAllLibraryDialogs( HandleResourceMode eMode );

public:
    LocalizationMgr( BasicIDEShell* pIDEShell, const ScriptDocument& rDocument, String aLibName,
        const ::com::sun::star::uno::Reference
            < ::com::sun::star::resource::XStringResourceManager >& xStringResourceManager );
    ::com::sun::star::uno::Reference
        < ::com::sun::star::resource::XStringResourceManager >getStringResourceManager( void )
    {
        return m_xStringResourceManager;
    }

    bool isLibraryLocalized( void );

    void handleTranslationbar( void );

    void handleAddLocales( ::com::sun::star::uno::Sequence
        < ::com::sun::star::lang::Locale > aLocaleSeq );

    void handleRemoveLocales( ::com::sun::star::uno::Sequence
        < ::com::sun::star::lang::Locale > aLocaleSeq );

    void handleSetDefaultLocale( ::com::sun::star::lang::Locale aLocale );

    void handleSetCurrentLocale( ::com::sun::star::lang::Locale aLocale );

    void handleBasicStarted( void );

    void handleBasicStopped( void );

    static void setControlResourceIDsForNewEditorObject( DlgEditor* pEditor,
        ::com::sun::star::uno::Any aControlAny, const ::rtl::OUString& aCtrlName );

    static void renameControlResourceIDsForEditorObject( DlgEditor* pEditor,
        ::com::sun::star::uno::Any aControlAny, const ::rtl::OUString& aNewCtrlName );

    static void deleteControlResourceIDsForDeletedEditorObject( DlgEditor* pEditor,
        ::com::sun::star::uno::Any aControlAny, const ::rtl::OUString& aCtrlName );

    static void setStringResourceAtDialog( const ScriptDocument& rDocument, const String& aLibName, const String& aDlgName,
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > xDialogModel );

    static void renameStringResourceIDs( const ScriptDocument& rDocument, const String& aLibName, const String& aDlgName,
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > xDialogModel );

    static void removeResourceForDialog( const ScriptDocument& rDocument, const String& aLibName, const String& aDlgName,
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
        ::com::sun::star::uno::Any aControlAny, const ::rtl::OUString& aCtrlName,
        ::com::sun::star::uno::Reference< ::com::sun::star::resource::
        XStringResourceResolver > xSourceStringResolver );

    static void copyResourceForDroppedDialog(
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > xDialogModel,
        const ::rtl::OUString& aDialogName,
        ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceManager > xStringResourceManager,
        ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceResolver > xSourceStringResolver );

    static void copyResourceForDialog(
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xDialogModel,
        const ::com::sun::star::uno::Reference< ::com::sun::star::resource::
            XStringResourceResolver >& xSourceStringResolver,
        const ::com::sun::star::uno::Reference< ::com::sun::star::resource::
            XStringResourceManager >& xTargetStringResourceManager );
};

#endif
