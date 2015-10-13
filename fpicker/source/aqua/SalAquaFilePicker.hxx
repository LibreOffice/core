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

#ifndef INCLUDED_FPICKER_SOURCE_AQUA_SALAQUAFILEPICKER_HXX
#define INCLUDED_FPICKER_SOURCE_AQUA_SALAQUAFILEPICKER_HXX

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

    virtual void SAL_CALL addFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener )
        throw( css::uno::RuntimeException ) override;
    virtual void SAL_CALL removeFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener )
        throw( css::uno::RuntimeException ) override;

    // XExecutableDialog functions

    virtual void SAL_CALL setTitle( const OUString& aTitle )
        throw( css::uno::RuntimeException ) override;

    virtual sal_Int16 SAL_CALL execute(  )
        throw( css::uno::RuntimeException ) override;

    // XFilePicker functions

    virtual void SAL_CALL setMultiSelectionMode( sal_Bool bMode )
        throw( css::uno::RuntimeException ) override;

    virtual void SAL_CALL setDefaultName( const OUString& aName )
        throw( css::uno::RuntimeException ) override;

    virtual void SAL_CALL setDisplayDirectory( const OUString& aDirectory )
        throw( css::lang::IllegalArgumentException,
               css::uno::RuntimeException ) override;

    virtual OUString SAL_CALL getDisplayDirectory(  )
        throw( css::uno::RuntimeException ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getFiles(  )
        throw( css::uno::RuntimeException ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSelectedFiles(  )
        throw( css::uno::RuntimeException, std::exception ) override;

    // XFilterManager functions

    virtual void SAL_CALL appendFilter( const OUString& aTitle, const OUString& aFilter )
        throw( css::lang::IllegalArgumentException, css::uno::RuntimeException ) override;

    virtual void SAL_CALL setCurrentFilter( const OUString& aTitle )
        throw( css::lang::IllegalArgumentException, css::uno::RuntimeException ) override;

    virtual OUString SAL_CALL getCurrentFilter(  )
        throw( css::uno::RuntimeException ) override;

    // XFilterGroupManager functions

    virtual void SAL_CALL appendFilterGroup( const OUString& sGroupTitle, const css::uno::Sequence< css::beans::StringPair >& aFilters )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException) override;

    // XFilePickerControlAccess functions

    virtual void SAL_CALL setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const css::uno::Any& aValue )
        throw (css::uno::RuntimeException) override;

    virtual css::uno::Any SAL_CALL getValue( sal_Int16 aControlId, sal_Int16 aControlAction )
        throw (css::uno::RuntimeException) override;

    virtual void SAL_CALL enableControl( sal_Int16 nControlId, sal_Bool bEnable )
        throw(css::uno::RuntimeException ) override;

    virtual void SAL_CALL setLabel( sal_Int16 nControlId, const OUString& aLabel )
        throw (css::uno::RuntimeException) override;

    virtual OUString SAL_CALL getLabel( sal_Int16 nControlId )
        throw (css::uno::RuntimeException) override;

    // XInitialization

    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw(css::uno::Exception, css::uno::RuntimeException) override;

    // XCancellable

    virtual void SAL_CALL cancel( )
        throw( css::uno::RuntimeException ) override;

    // XEventListener

    using cppu::WeakComponentImplHelperBase::disposing;
    virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent )
        throw(css::uno::RuntimeException);

    // XServiceInfo

    virtual OUString SAL_CALL getImplementationName(  )
        throw(css::uno::RuntimeException) override;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw(css::uno::RuntimeException) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw(css::uno::RuntimeException) override;

    // FilePicker Event functions

   void SAL_CALL fileSelectionChanged( css::ui::dialogs::FilePickerEvent aEvent );
   void SAL_CALL directoryChanged( css::ui::dialogs::FilePickerEvent aEvent );
   // OUString SAL_CALL helpRequested( css::ui::dialogs::FilePickerEvent aEvent ) const;
   void SAL_CALL controlStateChanged( css::ui::dialogs::FilePickerEvent aEvent );
   void SAL_CALL dialogSizeChanged( );

   inline AquaFilePickerDelegate * getDelegate() {
       return m_pDelegate;
   }

   inline OUString getSaveFileName() {
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

    virtual ~SalAquaFilePicker();

    void filterControlChanged();

    void implInitialize();
};

#endif // INCLUDED_FPICKER_SOURCE_AQUA_SALAQUAFILEPICKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
