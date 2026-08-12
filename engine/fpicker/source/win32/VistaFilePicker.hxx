/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include "requests.hxx"
#include "VistaFilePickerImpl.hxx"
#include "VistaFilePickerEventHandler.hxx"

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>

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
                css::ui::dialogs::XFolderPicker2,
                css::lang::XInitialization,
                css::lang::XServiceInfo >   TVistaFilePickerBase;


/** Implements the XFilePicker & friends interface(s)
    for Windows Vista and upcoming versions.

    Note: This will be a UNO wrapper for the real file picker
    implementation only. The real implementation is done in class
    VistaFilePickerImpl.
 */
class VistaFilePicker : public ::cppu::BaseMutex
                      , public TVistaFilePickerBase
{
public:


    // ctor/dtor


    explicit VistaFilePicker( bool bFolderPicker );
    virtual ~VistaFilePicker() override;


    // XFilePickerNotifier


    virtual void addFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) override;

    virtual void removeFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) override;


    // XExecutableDialog functions


    virtual void setTitle( const OUString& sTitle ) override;

    virtual sal_Int16 execute(  ) override;


    // XFilePicker functions


    virtual void setMultiSelectionMode( bool bMode ) override;

    virtual void setDefaultName( const OUString& sName ) override;

    virtual void setDisplayDirectory( const OUString& sDirectory ) override;

    virtual OUString getDisplayDirectory(  ) override;

    virtual cpo::uno::Sequence< OUString > getFiles(  ) override;

    // XFilePicker2 functions
    virtual cpo::uno::Sequence< OUString > getSelectedFiles(  ) override;


    // XFilterManager functions


    virtual void appendFilter( const OUString& sTitle  ,
                                        const OUString& sFilter ) override;

    virtual void setCurrentFilter( const OUString& sTitle ) override;

    virtual OUString getCurrentFilter(  ) override;


    // XFilterGroupManager functions


    virtual void appendFilterGroup( const OUString&                              sGroupTitle,
                                             const cpo::uno::Sequence< css::beans::StringPair >& lFilters   ) override;


    // XFilePickerControlAccess functions


    virtual void setValue(       sal_Int16      nControlId    ,
                                          sal_Int16      nControlAction,
                                    const cpo::uno::Any& aValue        ) override;

    virtual cpo::uno::Any getValue( sal_Int16 nControlId     ,
                                             sal_Int16 nControlAction ) override;

    virtual void enableControl( sal_Int16 nControlId,
                                         bool  bEnable   ) override;

    virtual void setLabel(       sal_Int16        nControlId,
                                    const OUString& sLabel    ) override;

    virtual OUString getLabel( sal_Int16 nControlId ) override;


    // XFilePreview


    virtual cpo::uno::Sequence< sal_Int16 > getSupportedImageFormats(  ) override;

    virtual sal_Int32 getTargetColorDepth(  ) override;

    virtual sal_Int32 getAvailableWidth(  ) override;

    virtual sal_Int32 getAvailableHeight(  ) override;

    virtual void setImage(       sal_Int16      nImageFormat,
                                    const cpo::uno::Any& aImage      ) override;

    virtual bool setShowState( bool bShowState ) override;

    virtual bool getShowState(  ) override;


    // XInitialization


    virtual void initialize( const cpo::uno::Sequence< cpo::uno::Any >& lArguments ) override;


    // XCancellable


    virtual void cancel( ) override;


    // XEventListener

    /// @throws css::uno::RuntimeException
    virtual void disposing( const css::lang::EventObject& aEvent );


    // XServiceInfo


    virtual OUString getImplementationName(  ) override;

    virtual bool supportsService( const OUString& ServiceName ) override;

    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;


    // XFolderPicker functions


    virtual OUString getDirectory( ) override;

    virtual void setDescription( const OUString& aDescription ) override;


    private:

        // prevent copy and assignment
        VistaFilePicker( const VistaFilePicker& );
        VistaFilePicker& operator=( const VistaFilePicker& );

        using WeakComponentImplHelperBase::disposing;

        void ensureInit();

    private:

        cpo::uno::Sequence< OUString > m_lLastFiles;

        VistaFilePickerImpl m_rDialog;

        bool m_bInitialized;
        const bool m_bFolderPicker;
};

} // namespace vista
} // namespace win32
} // namespace fpicker

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
