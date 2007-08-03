/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: localizationmgr.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 10:00:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
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

#ifndef _LOCALIZATIONMGR_HXX
#define _LOCALIZATIONMGR_HXX

#ifndef _COM_SUN_STAR_RESOURCE_XSTRINGRESOURCEMANAGER_HPP_
#include <com/sun/star/resource/XStringResourceManager.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

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
