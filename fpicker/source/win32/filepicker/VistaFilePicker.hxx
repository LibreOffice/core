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

#ifndef FPICKER_WIN32_VISTA_FILEPICKER_HXX
#define FPICKER_WIN32_VISTA_FILEPICKER_HXX

#include "asyncrequests.hxx"
#include "VistaFilePickerImpl.hxx"
#include "VistaFilePickerEventHandler.hxx"

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/XFilterGroupManager.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#include <com/sun/star/util/XCancellable.hpp>

#include <cppuhelper/compbase10.hxx>
#include <cppuhelper/basemutex.hxx>
#include <rtl/ustring.hxx>

//-----------------------------------------------------------------------------
// namespace
//-----------------------------------------------------------------------------

namespace css = ::com::sun::star;

namespace fpicker{
namespace win32{
namespace vista{

//-----------------------------------------------------------------------------
// types
//-----------------------------------------------------------------------------

typedef ::cppu::WeakComponentImplHelper10<
                css::ui::dialogs::XFilePicker2,
                css::ui::dialogs::XFilterManager,
                css::ui::dialogs::XFilterGroupManager,
                css::ui::dialogs::XFilePickerControlAccess,
                css::ui::dialogs::XFilePickerNotifier,
                css::ui::dialogs::XFilePreview,
                css::lang::XInitialization,
                css::util::XCancellable,
                css::lang::XEventListener,
                css::lang::XServiceInfo >   TVistaFilePickerBase;

//-----------------------------------------------------------------------------
/** Implements the XFilePicker & friends interface(s)
    for Windos Vista and upcoming versions.

    Note: This will be an UNO wrapper for the real file picker
    implementation oly. The real implementation is done in class
    VistaFilePickerImpl.
 */
class VistaFilePicker : public ::cppu::BaseMutex
                      , public TVistaFilePickerBase
{
public:

    //------------------------------------------------------------------------------------
    // ctor/dtor
    //------------------------------------------------------------------------------------

             VistaFilePicker( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR );
    virtual ~VistaFilePicker();

    //------------------------------------------------------------------------------------
    // XFilePickerNotifier
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL addFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener )
        throw( css::uno::RuntimeException );

    virtual void SAL_CALL removeFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener )
        throw( css::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XExecutableDialog functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL setTitle( const ::rtl::OUString& sTitle )
        throw( css::uno::RuntimeException );

    virtual sal_Int16 SAL_CALL execute(  )
        throw( css::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XFilePicker functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL setMultiSelectionMode( sal_Bool bMode )
        throw( css::uno::RuntimeException );

    virtual void SAL_CALL setDefaultName( const ::rtl::OUString& sName )
        throw( css::uno::RuntimeException );

    virtual void SAL_CALL setDisplayDirectory( const ::rtl::OUString& sDirectory )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException         );

    virtual ::rtl::OUString SAL_CALL getDisplayDirectory(  )
        throw( css::uno::RuntimeException );

    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getFiles(  )
        throw( css::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XFilePicker2 functions
    //------------------------------------------------------------------------------------

    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSelectedFiles(  )
        throw( css::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XFilterManager functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL appendFilter( const ::rtl::OUString& sTitle  ,
                                        const ::rtl::OUString& sFilter )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException         );

    virtual void SAL_CALL setCurrentFilter( const ::rtl::OUString& sTitle )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException         );

    virtual ::rtl::OUString SAL_CALL getCurrentFilter(  )
        throw( css::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XFilterGroupManager functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL appendFilterGroup( const ::rtl::OUString&                              sGroupTitle,
                                             const css::uno::Sequence< css::beans::StringPair >& lFilters   )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException         );

    //------------------------------------------------------------------------------------
    // XFilePickerControlAccess functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL setValue(       sal_Int16      nControlId    ,
                                          sal_Int16      nControlAction,
                                    const css::uno::Any& aValue        )
        throw (css::uno::RuntimeException);

    virtual css::uno::Any SAL_CALL getValue( sal_Int16 nControlId     ,
                                             sal_Int16 nControlAction )
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL enableControl( sal_Int16 nControlId,
                                         sal_Bool  bEnable   )
        throw(css::uno::RuntimeException );

    virtual void SAL_CALL setLabel(       sal_Int16        nControlId,
                                    const ::rtl::OUString& sLabel    )
        throw (css::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getLabel( sal_Int16 nControlId )
        throw (css::uno::RuntimeException);

    //------------------------------------------------
    // XFilePreview
    //------------------------------------------------

    virtual css::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats(  )
        throw (css::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getTargetColorDepth(  )
        throw (css::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getAvailableWidth(  )
        throw (css::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getAvailableHeight(  )
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setImage(       sal_Int16      nImageFormat,
                                    const css::uno::Any& aImage      )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL setShowState( sal_Bool bShowState )
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL getShowState(  )
        throw (css::uno::RuntimeException);

    //------------------------------------------------
    // XInitialization
    //------------------------------------------------

    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& lArguments )
        throw(css::uno::Exception       ,
              css::uno::RuntimeException);

    //------------------------------------------------
    // XCancellable
    //------------------------------------------------

    virtual void SAL_CALL cancel( )
        throw(css::uno::RuntimeException);

    //------------------------------------------------
    // XEventListener
    //------------------------------------------------

    virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent )
        throw(css::uno::RuntimeException);

    //------------------------------------------------
    // XServiceInfo
    //------------------------------------------------

    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw(css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw(css::uno::RuntimeException);

    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw(css::uno::RuntimeException);

    /*
    //------------------------------------------------------------------------------------
    // FilePicker Event functions
    //------------------------------------------------------------------------------------

    void SAL_CALL fileSelectionChanged(const css::ui::dialogs::FilePickerEvent& aEvent );
    void SAL_CALL directoryChanged(const css::ui::dialogs::FilePickerEvent& aEvent );
    ::rtl::OUString SAL_CALL helpRequested(const css::ui::dialogs::FilePickerEvent& aEvent ) const;
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

    private:

        //---------------------------------------------------------------------
        /// service manager to create own used uno services
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        //---------------------------------------------------------------------
        css::uno::Sequence< ::rtl::OUString > m_lLastFiles;

        //---------------------------------------------------------------------
        /** execute the COM dialog within a STA thread
         *  Must be used on the heap ... because it's implemented as OSL thread .-)
         */
        RequestHandlerRef m_rDialog;
        AsyncRequests m_aAsyncExecute;

        //---------------------------------------------------------------------
        oslThreadIdentifier m_nFilePickerThreadId;

        bool m_bInitialized;
};

} // namespace vista
} // namespace win32
} // namespace fpicker

#endif  // FPICKER_WIN32_VISTA_FILEPICKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
