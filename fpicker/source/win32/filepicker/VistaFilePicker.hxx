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

#ifndef INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_VISTAFILEPICKER_HXX
#define INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_VISTAFILEPICKER_HXX

#include "asyncrequests.hxx"
#include "VistaFilePickerImpl.hxx"
#include "VistaFilePickerEventHandler.hxx"

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <rtl/ustring.hxx>

namespace fpicker{
namespace win32{
namespace vista{


// types


typedef ::cppu::WeakComponentImplHelper<
                css::ui::dialogs::XFilePicker3,
                css::ui::dialogs::XFilePickerControlAccess,
                css::ui::dialogs::XFilePreview,
                css::lang::XInitialization,
                css::lang::XServiceInfo >   TVistaFilePickerBase;


/** Implements the XFilePicker & friends interface(s)
    for Windows Vista and upcoming versions.

    Note: This will be an UNO wrapper for the real file picker
    implementation only. The real implementation is done in class
    VistaFilePickerImpl.
 */
class VistaFilePicker : public ::cppu::BaseMutex
                      , public TVistaFilePickerBase
{
public:


    // ctor/dtor


    explicit VistaFilePicker( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR );
    virtual ~VistaFilePicker() override;


    // XFilePickerNotifier


    virtual void SAL_CALL addFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) override;

    virtual void SAL_CALL removeFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) override;


    // XExecutableDialog functions


    virtual void SAL_CALL setTitle( const OUString& sTitle ) override;

    virtual sal_Int16 SAL_CALL execute(  ) override;


    // XFilePicker functions


    virtual void SAL_CALL setMultiSelectionMode( sal_Bool bMode ) override;

    virtual void SAL_CALL setDefaultName( const OUString& sName ) override;

    virtual void SAL_CALL setDisplayDirectory( const OUString& sDirectory ) override;

    virtual OUString SAL_CALL getDisplayDirectory(  ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getFiles(  ) override;

    // XFilePicker2 functions
    virtual css::uno::Sequence< OUString > SAL_CALL getSelectedFiles(  ) override;


    // XFilterManager functions


    virtual void SAL_CALL appendFilter( const OUString& sTitle  ,
                                        const OUString& sFilter ) override;

    virtual void SAL_CALL setCurrentFilter( const OUString& sTitle ) override;

    virtual OUString SAL_CALL getCurrentFilter(  ) override;


    // XFilterGroupManager functions


    virtual void SAL_CALL appendFilterGroup( const OUString&                              sGroupTitle,
                                             const css::uno::Sequence< css::beans::StringPair >& lFilters   ) override;


    // XFilePickerControlAccess functions


    virtual void SAL_CALL setValue(       sal_Int16      nControlId    ,
                                          sal_Int16      nControlAction,
                                    const css::uno::Any& aValue        ) override;

    virtual css::uno::Any SAL_CALL getValue( sal_Int16 nControlId     ,
                                             sal_Int16 nControlAction ) override;

    virtual void SAL_CALL enableControl( sal_Int16 nControlId,
                                         sal_Bool  bEnable   ) override;

    virtual void SAL_CALL setLabel(       sal_Int16        nControlId,
                                    const OUString& sLabel    ) override;

    virtual OUString SAL_CALL getLabel( sal_Int16 nControlId ) override;


    // XFilePreview


    virtual css::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats(  ) override;

    virtual sal_Int32 SAL_CALL getTargetColorDepth(  ) override;

    virtual sal_Int32 SAL_CALL getAvailableWidth(  ) override;

    virtual sal_Int32 SAL_CALL getAvailableHeight(  ) override;

    virtual void SAL_CALL setImage(       sal_Int16      nImageFormat,
                                    const css::uno::Any& aImage      ) override;

    virtual sal_Bool SAL_CALL setShowState( sal_Bool bShowState ) override;

    virtual sal_Bool SAL_CALL getShowState(  ) override;


    // XInitialization


    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& lArguments ) override;


    // XCancellable


    virtual void SAL_CALL cancel( ) override;


    // XEventListener

    /// @throws css::uno::RuntimeException
    virtual void disposing( const css::lang::EventObject& aEvent );


    // XServiceInfo


    virtual OUString SAL_CALL getImplementationName(  ) override;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    /*

    // FilePicker Event functions


    void SAL_CALL fileSelectionChanged(const css::ui::dialogs::FilePickerEvent& aEvent );
    void SAL_CALL directoryChanged(const css::ui::dialogs::FilePickerEvent& aEvent );
    OUString SAL_CALL helpRequested(const css::ui::dialogs::FilePickerEvent& aEvent ) const;
    void SAL_CALL controlStateChanged(const css::ui::dialogs::FilePickerEvent& aEvent );
    void SAL_CALL dialogSizeChanged( );

    bool startupEventNotification(bool bStartupSuspended);
    void shutdownEventNotification();
    void suspendEventNotification();
    void resumeEventNotification();
    */

    private:

        // prevent copy and assignment
        VistaFilePicker( const VistaFilePicker& );
        VistaFilePicker& operator=( const VistaFilePicker& );

        using WeakComponentImplHelperBase::disposing;

        void ensureInit();

    private:


        /// service manager to create own used uno services
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;


        css::uno::Sequence< OUString > m_lLastFiles;


        /** execute the COM dialog within a STA thread
         *  Must be used on the heap ... because it's implemented as OSL thread .-)
         */
        RequestHandlerRef m_rDialog;
        AsyncRequests m_aAsyncExecute;


        oslThreadIdentifier m_nFilePickerThreadId;

        bool m_bInitialized;
};

} // namespace vista
} // namespace win32
} // namespace fpicker

#endif // INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_VISTAFILEPICKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
