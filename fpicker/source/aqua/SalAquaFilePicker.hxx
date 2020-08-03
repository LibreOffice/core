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

#pragma once

#include <cppuhelper/compbase.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/beans/StringPair.hpp>

#include "SalAquaPicker.hxx"

#include <rtl/ustring.hxx>
#include "FilterHelper.hxx"
#include "AquaFilePickerDelegate.hxx"

// Implementation class for the XFilePicker Interface

typedef ::cppu::WeakComponentImplHelper <
            css::ui::dialogs::XFilePicker3,
            css::ui::dialogs::XFilePickerControlAccess,
            css::lang::XInitialization,
            css::lang::XServiceInfo >  SalAquaFilePicker_Base;

class SalAquaFilePicker :
    public SalAquaPicker,
    public SalAquaFilePicker_Base
{
public:

    // constructor
    SalAquaFilePicker();

    // XFilePickerNotifier

    virtual void SAL_CALL addFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) override;
    virtual void SAL_CALL removeFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) override;

    // XExecutableDialog functions

    virtual void SAL_CALL setTitle( const OUString& aTitle ) override;

    virtual sal_Int16 SAL_CALL execute(  ) override;

    // XFilePicker functions

    virtual void SAL_CALL setMultiSelectionMode( sal_Bool bMode ) override;

    virtual void SAL_CALL setDefaultName( const OUString& aName ) override;

    virtual void SAL_CALL setDisplayDirectory( const OUString& aDirectory ) override;

    virtual OUString SAL_CALL getDisplayDirectory(  ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getFiles(  ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSelectedFiles(  ) override;

    // XFilterManager functions

    virtual void SAL_CALL appendFilter( const OUString& aTitle, const OUString& aFilter ) override;

    virtual void SAL_CALL setCurrentFilter( const OUString& aTitle ) override;

    virtual OUString SAL_CALL getCurrentFilter(  ) override;

    // XFilterGroupManager functions

    virtual void SAL_CALL appendFilterGroup( const OUString& sGroupTitle, const css::uno::Sequence< css::beans::StringPair >& aFilters ) override;

    // XFilePickerControlAccess functions

    virtual void SAL_CALL setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const css::uno::Any& aValue ) override;

    virtual css::uno::Any SAL_CALL getValue( sal_Int16 aControlId, sal_Int16 aControlAction ) override;

    virtual void SAL_CALL enableControl( sal_Int16 nControlId, sal_Bool bEnable ) override;

    virtual void SAL_CALL setLabel( sal_Int16 nControlId, const OUString& aLabel ) override;

    virtual OUString SAL_CALL getLabel( sal_Int16 nControlId ) override;

    // XInitialization

    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XCancellable

    virtual void SAL_CALL cancel( ) override;

    // XEventListener

    using cppu::WeakComponentImplHelperBase::disposing;
    /// @throws css::uno::RuntimeException
    virtual void disposing( const css::lang::EventObject& aEvent );

    // XServiceInfo

    virtual OUString SAL_CALL getImplementationName(  ) override;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // FilePicker Event functions

   void fileSelectionChanged( css::ui::dialogs::FilePickerEvent aEvent );
   void directoryChanged( css::ui::dialogs::FilePickerEvent aEvent );
   // OUString SAL_CALL helpRequested( css::ui::dialogs::FilePickerEvent aEvent ) const;
   void controlStateChanged( css::ui::dialogs::FilePickerEvent aEvent );
   void dialogSizeChanged( );

   AquaFilePickerDelegate * getDelegate() {
       return m_pDelegate;
   }

   OUString const & getSaveFileName() {
       return m_sSaveFileName;
   }

private:
    SalAquaFilePicker( const SalAquaFilePicker& ) = delete;
    SalAquaFilePicker& operator=( const SalAquaFilePicker& ) = delete;

    virtual void ensureFilterHelper();

    css::uno::Reference< css::ui::dialogs::XFilePickerListener >  m_xListener;
    FilterHelper *m_pFilterHelper;
    OUString m_sSaveFileName;
    AquaFilePickerDelegate *m_pDelegate;

    void updateFilterUI();
    void updateSaveFileNameExtension();

public:

    virtual ~SalAquaFilePicker() override;

    void filterControlChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
