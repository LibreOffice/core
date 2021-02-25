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

#ifndef INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_VISTAFILEPICKERIMPL_HXX
#define INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_VISTAFILEPICKERIMPL_HXX

#include "platform_vista.h"

#include <shobjidl.h>

#include "asyncrequests.hxx"
#include "comptr.hxx"
#include "vistatypes.h"
#include "FilterContainer.hxx"
#include "VistaFilePickerEventHandler.hxx"
#include "IVistaFilePickerInternalNotify.hxx"
#include "resourceprovider.hxx"

#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/basemutex.hxx>
#include <osl/thread.hxx>
#include <osl/conditn.hxx>
#include <o3tl/safeCoInitUninit.hxx>
#include <rtl/ustring.hxx>

namespace fpicker{
namespace win32{
namespace vista{


// types, const etcpp


const ::sal_Int32 FEATURE_AUTOEXTENSION  =    1;
const ::sal_Int32 FEATURE_PASSWORD       =    2;
const ::sal_Int32 FEATURE_FILTEROPTIONS  =    4;
const ::sal_Int32 FEATURE_SELECTION      =    8;
const ::sal_Int32 FEATURE_TEMPLATE       =   16;
const ::sal_Int32 FEATURE_LINK           =   32;
const ::sal_Int32 FEATURE_PREVIEW        =   64;
const ::sal_Int32 FEATURE_IMAGETEMPLATE  =  128;
const ::sal_Int32 FEATURE_PLAY           =  256;
const ::sal_Int32 FEATURE_READONLY       =  512;
const ::sal_Int32 FEATURE_VERSION        = 1024;
const ::sal_Int32 FEATURE_GPGPASSWORD    = 2048;
const ::sal_Int32 FEATURE_IMAGEANCHOR    = 4096;

const OUString PROP_PICKER_LISTENER("picker_listener"   ); // [XFilePickerListenert]
const OUString PROP_DIALOG_SHOW_RESULT("dialog_show_result" ); // [sal_Bool] true=OK, false=CANCEL
const OUString PROP_SELECTED_FILES("selected_files"     ); // [seq< OUString >] contains all user selected files (can be empty!)
const OUString PROP_MULTISELECTION_MODE("multiselection_mode"); // [sal_Bool] true=ON, false=OFF
const OUString PROP_TITLE("title"              ); // [OUString]
const OUString PROP_FILENAME("filename"          ); // [OUString]
const OUString PROP_DIRECTORY("directory"          ); // [OUString]
const OUString PROP_FEATURES("features"           ); // [sal_Int32]
const OUString PROP_TEMPLATE_DESCR("templatedescription"); // [sal_Int32]
const OUString PROP_FILTER_TITLE("filter_title"       ); // [OUString]
const OUString PROP_FILTER_VALUE("filter_value"       ); // [OUString]
const OUString PROP_FILTER_GROUP("filter-group"       ); // [seq< css:beans::StringPair >] contains a group of filters

const OUString PROP_CONTROL_ID("control_id"         ); // [sal_Int16]
const OUString PROP_CONTROL_ACTION("control_action"     ); // [sal_Int16]
const OUString PROP_CONTROL_VALUE("control_value"      ); // [Any]
const OUString PROP_CONTROL_LABEL("control_label"      ); // [OUString]
const OUString PROP_CONTROL_ENABLE("control_enable"     ); // [sal_Bool] true=ON, false=OFF
const OUString PROP_PARENT_WINDOW("ParentWindow"); //[css::awt::XWindow] preferred parent window
const OUString STRING_SEPARATOR("------------------------------------------" );

class TDialogImplBase;

/** native implementation of the file picker on Vista and upcoming windows versions.
 *  This dialog uses COM internally. Further it marshall every request so it will
 *  be executed within its own STA thread !
 */

class VistaFilePickerImpl : private ::cppu::BaseMutex
                          , public  RequestHandler
                          , public  IVistaFilePickerInternalNotify
{
    public:

        // Workaround made to get input in Template Listbox
        css::uno::Sequence< OUString > m_lItems;
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
            E_CREATE_FOLDER_PICKER,
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


        // ctor/dtor - nothing special

                 VistaFilePickerImpl();
        virtual ~VistaFilePickerImpl() override;


        // RequestHandler


        virtual void before() override;
        virtual void doRequest(const RequestRef& rRequest) override;
        virtual void after() override;


        // IVistaFilePickerInternalNotify

        virtual void onAutoExtensionChanged (bool bChecked) override;
        virtual bool onFileTypeChanged( UINT nTypeIndex ) override;
        virtual void onDirectoryChanged() override;

    private:
        OUString GetDirectory();

        /// implementation of request E_ADD_FILEPICKER_LISTENER
        void impl_sta_addFilePickerListener(const RequestRef& rRequest);


        /// implementation of request E_REMOVE_FILEPICKER_LISTENER
        void impl_sta_removeFilePickerListener(const RequestRef& rRequest);


        /// implementation of request E_APPEND_FILTER
        void impl_sta_appendFilter(const RequestRef& rRequest);


        /// implementation of request E_APPEND_FILTERGROUP
        void impl_sta_appendFilterGroup(const RequestRef& rRequest);


        /// implementation of request E_SET_CURRENT_FILTER
        void impl_sta_setCurrentFilter(const RequestRef& rRequest);


        /// implementation of request E_GET_CURRENT_FILTER
        void impl_sta_getCurrentFilter(const RequestRef& rRequest);


        /// implementation of request E_CREATE_OPEN_DIALOG
        void impl_sta_CreateOpenDialog(const RequestRef& rRequest);


        /// implementation of request E_CREATE_SAVE_DIALOG
        void impl_sta_CreateSaveDialog(const RequestRef& rRequest);


        /// implementation of request E_CREATE_FOLDER_PICKER
        void impl_sta_CreateFolderPicker(const RequestRef& rRequest);


        /// implementation of request E_SET_MULTISELECTION_MODE
        void impl_sta_SetMultiSelectionMode(const RequestRef& rRequest);


        /// implementation of request E_SET_TITLE
        void impl_sta_SetTitle(const RequestRef& rRequest);


        /// implementation of request E_SET_FILENAME
        void impl_sta_SetFileName(const RequestRef& rRequest);


        /// implementation of request E_SET_DIRECTORY
        void impl_sta_SetDirectory(const RequestRef& rRequest);


        /// implementation of request E_GET_DIRECTORY
        void impl_sta_GetDirectory(const RequestRef& rRequest);


        /// implementation of request E_SET_DEFAULT_NAME
        void impl_sta_SetDefaultName(const RequestRef& rRequest);


        /// implementation of request E_GET_SELECTED_FILES
        void impl_sta_getSelectedFiles(const RequestRef& rRequest);


        /// implementation of request E_SHOW_DIALOG_MODAL
        void impl_sta_ShowDialogModal(const RequestRef& rRequest);


        /// implementation of request E_SET_CONTROL_VALUE
        void impl_sta_SetControlValue(const RequestRef& rRequest);


        /// implementation of request E_GET_CONTROL_VALUE
        void impl_sta_GetControlValue(const RequestRef& rRequest);


        /// implementation of request E_SET_CONTROL_LABEL
        void impl_sta_SetControlLabel(const RequestRef& rRequest);


        /// implementation of request E_GET_CONTROL_LABEL
        static void impl_sta_GetControlLabel(const RequestRef& rRequest);


        /// implementation of request E_ENABLE_CONTROL
        void impl_sta_EnableControl(const RequestRef& rRequest);

        /** create all needed (optional!) UI controls addressed by the field nFeatures.
         *  The given number nFeatures is used as a flag field. Use const values FEATURE_XXX
         *  to address it.
         *
         *  Internal new controls will be added to the dialog. Every control can be accessed
         *  by its own control id. Those control ID must be one of the const set
         *  css::ui::dialogs::ExtendedFilePickerElementIds.
         *
         *  @see setControlValue()
         *  @see getControlValue()
         *  @see setControlLabel()
         *  @see getControlLabel()
         *  @see enableControl()
         *
         *  @param  nFeatures
         *          flag field(!) knows all features which must be enabled.
         */
        void impl_sta_enableFeatures(::sal_Int32 nFeatures, ::sal_Int32 nTemplate);


        /** returns an interface, which can be used to customize the internally used
         *  COM dialog.
         *
         *  Because we use two member (open/save dialog) internally, this method
         *  ask the current active one for its customization interface.
         *
         *  @return the customization interface for the current used dialog.
         *          Must not be null.
         */
        TFileDialogCustomize impl_getCustomizeInterface();
        TFileDialog impl_getBaseDialogInterface();


        /// fill filter list of internal used dialog.
        void impl_sta_setFiltersOnDialog();

        void impl_SetDefaultExtension( const OUString& currentFilter );

   private:
        template <class TDialogImplClass> void impl_sta_CreateDialog();
        void impl_sta_InitDialog(const RequestRef& rRequest, DWORD nOrFlags);


        /// object representing a file dialog
        std::shared_ptr<TDialogImplBase> m_pDialog;


        /// @todo document me
        CFilterContainer m_lFilters;


        /** help us to handle dialog events and provide them to interested office
         *  listener.
         */
        TFileDialogEvents m_iEventHandler;


        /// @todo document me
        bool m_bInExecute;

        bool m_bWasExecuted;

        // handle to parent window
        HWND m_hParentWindow;


        OUString m_sDirectory;


        OUString m_sFilename;

        // to put back all the inits with COINIT_MULTITHREADED if needed
        int mnNbCallCoInitializeExForReinit;
};

} // namespace vista
} // namespace win32
} // namespace fpicker

#endif // INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_VISTAFILEPICKERIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
