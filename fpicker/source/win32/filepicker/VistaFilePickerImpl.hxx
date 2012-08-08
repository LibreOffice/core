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

#ifndef FPICKER_WIN32_VISTA_FILEPICKERIMPL_HXX
#define FPICKER_WIN32_VISTA_FILEPICKERIMPL_HXX

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning( disable : 4917 )
#endif

#include "platform_vista.h"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <shobjidl.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

// Without IFileDialogCustomize we can't do this
#ifdef __IFileDialogCustomize_INTERFACE_DEFINED__

#include "asyncrequests.hxx"
#include "comptr.hxx"
#include "vistatypes.h"
#include "FilterContainer.hxx"
#include "VistaFilePickerEventHandler.hxx"
#include "IVistaFilePickerInternalNotify.hxx"
#include "../misc/resourceprovider.hxx"

#include <com/sun/star/uno/Sequence.hxx>

#include <comphelper/sequenceashashmap.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/basemutex.hxx>
#include <osl/thread.hxx>
#include <osl/conditn.hxx>
#include <rtl/ustring.hxx>

namespace css = ::com::sun::star;

namespace fpicker{
namespace win32{
namespace vista{

//-----------------------------------------------------------------------------
// types, const etcpp
//-----------------------------------------------------------------------------

static const ::sal_Int32 FEATURE_AUTOEXTENSION  =    1;
static const ::sal_Int32 FEATURE_PASSWORD       =    2;
static const ::sal_Int32 FEATURE_FILTEROPTIONS  =    4;
static const ::sal_Int32 FEATURE_SELECTION      =    8;
static const ::sal_Int32 FEATURE_TEMPLATE       =   16;
static const ::sal_Int32 FEATURE_LINK           =   32;
static const ::sal_Int32 FEATURE_PREVIEW        =   64;
static const ::sal_Int32 FEATURE_IMAGETEMPLATE  =  128;
static const ::sal_Int32 FEATURE_PLAY           =  256;
static const ::sal_Int32 FEATURE_READONLY       =  512;
static const ::sal_Int32 FEATURE_VERSION        = 1024;

static const ::rtl::OUString PROP_PICKER_LISTENER(RTL_CONSTASCII_USTRINGPARAM("picker_listener"   ) ); // [XFilePickerListenert]
static const ::rtl::OUString PROP_DIALOG_SHOW_RESULT(RTL_CONSTASCII_USTRINGPARAM("dialog_show_result" )); // [sal_Bool] true=OK, false=CANCEL
static const ::rtl::OUString PROP_SELECTED_FILES(RTL_CONSTASCII_USTRINGPARAM("selected_files"     )); // [seq< OUString >] contains all user selected files (can be empty!)
static const ::rtl::OUString PROP_MULTISELECTION_MODE(RTL_CONSTASCII_USTRINGPARAM("multiselection_mode")); // [sal_Bool] true=ON, false=OFF
static const ::rtl::OUString PROP_TITLE(RTL_CONSTASCII_USTRINGPARAM("title"              )); // [OUString]
static const ::rtl::OUString PROP_FILENAME(RTL_CONSTASCII_USTRINGPARAM("filename"          ) ); // [OUString]
static const ::rtl::OUString PROP_DIRECTORY(RTL_CONSTASCII_USTRINGPARAM("directory"          )); // [OUString]
static const ::rtl::OUString PROP_FEATURES(RTL_CONSTASCII_USTRINGPARAM("features"           )); // [sal_Int32]
static const ::rtl::OUString PROP_TEMPLATE_DESCR(RTL_CONSTASCII_USTRINGPARAM("templatedescription")); // [sal_Int32]
static const ::rtl::OUString PROP_FILTER_TITLE(RTL_CONSTASCII_USTRINGPARAM("filter_title"       )); // [OUString]
static const ::rtl::OUString PROP_FILTER_VALUE(RTL_CONSTASCII_USTRINGPARAM("filter_value"       )); // [OUString]
static const ::rtl::OUString PROP_FORCE(RTL_CONSTASCII_USTRINGPARAM("force"              )); // [sal_Bool]
static const ::rtl::OUString PROP_FILTER_GROUP(RTL_CONSTASCII_USTRINGPARAM("filter-group"       )); // [seq< css:beans::StringPair >] contains a group of filters

static const ::rtl::OUString PROP_CONTROL_ID(RTL_CONSTASCII_USTRINGPARAM("control_id"         )); // [sal_Int16]
static const ::rtl::OUString PROP_CONTROL_ACTION(RTL_CONSTASCII_USTRINGPARAM("control_action"     )); // [sal_Int16]
static const ::rtl::OUString PROP_CONTROL_VALUE(RTL_CONSTASCII_USTRINGPARAM("control_value"      )); // [Any]
static const ::rtl::OUString PROP_CONTROL_LABEL(RTL_CONSTASCII_USTRINGPARAM("control_label"      )); // [OUString]
static const ::rtl::OUString PROP_CONTROL_ENABLE(RTL_CONSTASCII_USTRINGPARAM("control_enable"     )); // [sal_Bool] true=ON, false=OFF
static const ::rtl::OUString STRING_SEPARATOR(RTL_CONSTASCII_USTRINGPARAM("------------------------------------------" ));

//-----------------------------------------------------------------------------
/** native implementation of the file picker on Vista and upcoming windows versions.
 *  This dialog uses COM internaly. Further it marshall every request so it will
 *  be executed within it's own STA thread !
 */
//-----------------------------------------------------------------------------
class VistaFilePickerImpl : private ::cppu::BaseMutex
                          , public  RequestHandler
                          , public  IVistaFilePickerInternalNotify
{
    public:

        //---------------------------------------------------------------------
        /** used for marshalling requests.
         *  Will be used to map requests to the right implementations.
         */
        enum ERequest
        {
            E_NO_REQUEST,
            E_ADD_PICKER_LISTENER,
            E_REMOVE_PICKER_LISTENER,
            E_APPEND_FILTER,
            E_SET_CURRENT_FILTER,
            E_GET_CURRENT_FILTER,
            E_CREATE_OPEN_DIALOG,
            E_CREATE_SAVE_DIALOG,
            E_SET_MULTISELECTION_MODE,
            E_SET_TITLE,
            E_SET_FILENAME,
            E_GET_DIRECTORY,
            E_SET_DIRECTORY,
            E_SET_DEFAULT_NAME,
            E_GET_SELECTED_FILES,
            E_SHOW_DIALOG_MODAL,
            E_SET_CONTROL_VALUE,
            E_GET_CONTROL_VALUE,
            E_SET_CONTROL_LABEL,
            E_GET_CONTROL_LABEL,
            E_ENABLE_CONTROL,
            E_APPEND_FILTERGROUP
        };

    public:

        //---------------------------------------------------------------------
        // ctor/dtor - nothing special
        //---------------------------------------------------------------------
                 VistaFilePickerImpl();
        virtual ~VistaFilePickerImpl();

        //---------------------------------------------------------------------
        // RequestHandler
        //---------------------------------------------------------------------

        virtual void before();
        virtual void doRequest(const RequestRef& rRequest);
        virtual void after();

        //---------------------------------------------------------------------
        // IVistaFilePickerInternalNotify
        //---------------------------------------------------------------------
        virtual void onAutoExtensionChanged (bool bChecked);
        virtual bool onFileTypeChanged( UINT nTypeIndex );

    private:

        //---------------------------------------------------------------------
        /// implementation of request E_ADD_FILEPICKER_LISTENER
        void impl_sta_addFilePickerListener(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_REMOVE_FILEPICKER_LISTENER
        void impl_sta_removeFilePickerListener(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_APPEND_FILTER
        void impl_sta_appendFilter(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_APPEND_FILTERGROUP
        void impl_sta_appendFilterGroup(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_SET_CURRENT_FILTER
        void impl_sta_setCurrentFilter(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_GET_CURRENT_FILTER
        void impl_sta_getCurrentFilter(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_CREATE_OPEN_DIALOG
        void impl_sta_CreateOpenDialog(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_CREATE_SAVE_DIALOG
        void impl_sta_CreateSaveDialog(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_SET_MULTISELECTION_MODE
        void impl_sta_SetMultiSelectionMode(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_SET_TITLE
        void impl_sta_SetTitle(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_SET_FILENAME
        void impl_sta_SetFileName(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_SET_DIRECTORY
        void impl_sta_SetDirectory(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_GET_DIRECTORY
        void impl_sta_GetDirectory(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_SET_DEFAULT_NAME
        void impl_sta_SetDefaultName(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_GET_SELECTED_FILES
        void impl_sta_getSelectedFiles(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_SHOW_DIALOG_MODAL
        void impl_sta_ShowDialogModal(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_SET_CONTROL_VALUE
        void impl_sta_SetControlValue(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_GET_CONTROL_VALUE
        void impl_sta_GetControlValue(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_SET_CONTROL_LABEL
        void impl_sta_SetControlLabel(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_GET_CONTROL_LABEL
        void impl_sta_GetControlLabel(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /// implementation of request E_ENABLE_CONTROL
        void impl_sta_EnableControl(const RequestRef& rRequest);

        //---------------------------------------------------------------------
        /** create all needed (optional!) UI controls addressed by the field nFeatures.
         *  The given number nFeatures is used as a flag field. Use const values FEATURE_XXX
         *  to address it.
         *
         *  Internal new controls will be added to the dialog. Every control can be accessed
         *  by it's own control id. Those control ID must be one of the const set
         *  css::ui::dialogs::ExtendedFilePickerElementIds.
         *
         *  @see setControlValue()
         *  @see getControlValue()
         *  @see setControlLabel()
         *  @see getControlLabel()
         *  @see enableControl()
         *
         *  @param  nFeatures
         *          flag field(!) knows all features wich must be enabled.
         */
        void impl_sta_enableFeatures(::sal_Int32 nFeatures, ::sal_Int32 nTemplate);

        //---------------------------------------------------------------------
        /** returns an interface, which can be used to customize the internaly used
         *  COM dialog.
         *
         *  Because we use two member (open/save dialog) internaly, this method
         *  ask the current active one for it's customization interface.
         *
         *  @return the customization interface for the current used dialog.
         *          Must not be null.
         */
        TFileDialogCustomize impl_getCustomizeInterface();
        TFileDialog impl_getBaseDialogInterface();

        //---------------------------------------------------------------------
        /// fill filter list of internal used dialog.
        void impl_sta_setFiltersOnDialog();

        void impl_SetDefaultExtension( const rtl::OUString& currentFilter );

   private:

        //---------------------------------------------------------------------
        /// COM object representing a file open dialog
        TFileOpenDialog m_iDialogOpen;

        //---------------------------------------------------------------------
        /// COM object representing a file save dialog
        TFileSaveDialog m_iDialogSave;

        //---------------------------------------------------------------------
        /// knows the return state of the last COM call
        HRESULT m_hLastResult;

        //---------------------------------------------------------------------
        /// @todo document me
        CFilterContainer m_lFilters;

        //---------------------------------------------------------------------
        /** cache last selected list of files
         *  Because those list must be retrieved directly after closing the dialog
         *  (and only in case it was finished successfully) we cache it internaly.
         *  Because the outside provided UNO API decouple showing the dialog
         *  and asking for results .-)
         */
        css::uno::Sequence< ::rtl::OUString > m_lLastFiles;

        //---------------------------------------------------------------------
        /** help us to handle dialog events and provide them to interested office
         *  listener.
         */
        TFileDialogEvents m_iEventHandler;

        //---------------------------------------------------------------------
        /// @todo document me
        ::sal_Bool m_bInExecute;

        ::sal_Bool m_bWasExecuted;

        // handle to parent window
        HWND m_hParentWindow;

        //
        ::rtl::OUString m_sDirectory;

        //
        ::rtl::OUString m_sFilename;

        // Resource provider
        CResourceProvider m_ResProvider;
};

} // namespace vista
} // namespace win32
} // namespace fpicker

#endif // __IFileDialogCustomize_INTERFACE_DEFINED__

#endif // FPICKER_WIN32_VISTA_FILEPICKERIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
