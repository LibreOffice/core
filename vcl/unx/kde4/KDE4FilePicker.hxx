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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <osl/conditn.hxx>
#include <osl/mutex.hxx>

#include <rtl/ustrbuf.hxx>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QHash>

class KFileDialog;
class QWidget;
class QLayout;

typedef ::cppu::WeakComponentImplHelper
<   css::ui::dialogs::XFilePicker3
,   css::ui::dialogs::XFilePickerControlAccess
// TODO css::ui::dialogs::XFilePreview
,   css::lang::XInitialization
,   css::lang::XServiceInfo
> KDE4FilePicker_Base;

class KDE4FilePicker
    : public QObject
    , public KDE4FilePicker_Base
{
    Q_OBJECT
protected:

    css::uno::Reference< css::ui::dialogs::XFilePickerListener > m_xListener;

    //the dialog to display
    KFileDialog* _dialog;

    osl::Mutex _helperMutex;

    //running filter string to add to dialog
    QString _filter;
    // string to set the current filter
    QString _currentFilter;

    //mapping of SAL control ID's to created custom controls
    QHash<sal_Int16, QWidget*> _customWidgets;

    //widget to contain extra custom controls
    QWidget* _extraControls;

    //layout for extra custom controls
    QLayout* _layout;

    bool allowRemoteUrls;

public:
    explicit KDE4FilePicker( const css::uno::Reference< css::uno::XComponentContext >& );
    virtual ~KDE4FilePicker() override;

    // XFilePickerNotifier
    virtual void SAL_CALL addFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) override;
    virtual void SAL_CALL removeFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) override;

    // XExecutableDialog functions
    virtual void SAL_CALL setTitle( const OUString &rTitle ) override;
    virtual sal_Int16 SAL_CALL execute() override;

    // XFilePicker functions
    virtual void SAL_CALL setMultiSelectionMode( sal_Bool bMode ) override;
    virtual void SAL_CALL setDefaultName( const OUString &rName ) override;
    virtual void SAL_CALL setDisplayDirectory( const OUString &rDirectory ) override;
    virtual OUString SAL_CALL getDisplayDirectory() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getFiles() override;

    // XFilterManager functions
    virtual void SAL_CALL appendFilter( const OUString &rTitle, const OUString &rFilter ) override;
    virtual void SAL_CALL setCurrentFilter( const OUString &rTitle ) override;
    virtual OUString SAL_CALL getCurrentFilter() override;

    // XFilterGroupManager functions
    virtual void SAL_CALL appendFilterGroup( const OUString &rGroupTitle, const css::uno::Sequence< css::beans::StringPair > &rFilters ) override;

    // XFilePickerControlAccess functions
    virtual void SAL_CALL setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const css::uno::Any &rValue ) override;
    virtual css::uno::Any SAL_CALL getValue( sal_Int16 nControlId, sal_Int16 nControlAction ) override;
    virtual void SAL_CALL enableControl( sal_Int16 nControlId, sal_Bool bEnable ) override;
    virtual void SAL_CALL setLabel( sal_Int16 nControlId, const OUString &rLabel ) override;
    virtual OUString SAL_CALL getLabel( sal_Int16 nControlId ) override;

    /* TODO XFilePreview

    virtual css::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats(  ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL  getTargetColorDepth(  ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL  getAvailableWidth(  ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL  getAvailableHeight(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL       setImage( sal_Int16 aImageFormat, const css::uno::Any &rImage ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL   setShowState( sal_Bool bShowState ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL   getShowState(  ) throw (css::uno::RuntimeException);
    */

    // XFilePicker2 functions
    virtual css::uno::Sequence< OUString > SAL_CALL getSelectedFiles() override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any > &rArguments ) override;

    // XCancellable
    virtual void SAL_CALL cancel( ) override;

    // XEventListener
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL disposing( const css::lang::EventObject &rEvent );
    using cppu::WeakComponentImplHelperBase::disposing;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString &rServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

private Q_SLOTS:
    // XExecutableDialog functions
    /// @throws css::uno::RuntimeException
    void setTitleSlot( const OUString &rTitle ) { return setTitle( rTitle ); }
    /// @throws css::uno::RuntimeException
    sal_Int16 executeSlot() { return execute(); }

    // XFilePicker functions
    /// @throws css::uno::RuntimeException
    void setMultiSelectionModeSlot( bool bMode ) { return setMultiSelectionMode( bMode ); }
    /// @throws css::uno::RuntimeException
    void setDefaultNameSlot( const OUString &rName ) { return setDefaultName( rName ); }
    /// @throws css::uno::RuntimeException
    void setDisplayDirectorySlot( const OUString &rDirectory ) { return setDisplayDirectory( rDirectory ); }
    /// @throws css::uno::RuntimeException
    OUString getDisplayDirectorySlot() { return getDisplayDirectory(); }
    /// @throws css::uno::RuntimeException
    css::uno::Sequence< OUString > getFilesSlot() { return getFiles(); }

    // XFilterManager functions
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    void appendFilterSlot( const OUString &rTitle, const OUString &rFilter ) { return appendFilter( rTitle, rFilter ); }
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    void setCurrentFilterSlot( const OUString &rTitle ) { return setCurrentFilter( rTitle ); }
    /// @throws css::uno::RuntimeException
    OUString getCurrentFilterSlot() { return getCurrentFilter(); }

    // XFilterGroupManager functions
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    void appendFilterGroupSlot( const OUString &rGroupTitle, const css::uno::Sequence< css::beans::StringPair > &rFilters ) { return appendFilterGroup( rGroupTitle, rFilters ); }

    // XFilePickerControlAccess functions
    /// @throws css::uno::RuntimeException
    void setValueSlot( sal_Int16 nControlId, sal_Int16 nControlAction, const css::uno::Any &rValue ) { return setValue( nControlId, nControlAction, rValue ); }
    /// @throws css::uno::RuntimeException
    css::uno::Any getValueSlot( sal_Int16 nControlId, sal_Int16 nControlAction ) { return getValue( nControlId, nControlAction ); }
    /// @throws css::uno::RuntimeException
    void enableControlSlot( sal_Int16 nControlId, bool bEnable ) { return enableControl( nControlId, bEnable ); }
    /// @throws css::uno::RuntimeException
    void setLabelSlot( sal_Int16 nControlId, const OUString &rLabel ) { return setLabel( nControlId, rLabel ); }
    /// @throws css::uno::RuntimeException
    OUString getLabelSlot( sal_Int16 nControlId ) { return getLabel( nControlId ); }

    // XFilePicker2 functions
    /// @throws css::uno::RuntimeException
    css::uno::Sequence< OUString > getSelectedFilesSlot() { return getSelectedFiles(); }

    // XInitialization
    /// @throws css::uno::Exception
    /// @throws css::uno::RuntimeException
    void initializeSlot( const css::uno::Sequence< css::uno::Any > &rArguments ) { return initialize( rArguments ); }

Q_SIGNALS:
    // XExecutableDialog functions
    void setTitleSignal( const OUString &rTitle );
    sal_Int16 executeSignal();

    // XFilePicker functions
    void setMultiSelectionModeSignal( bool bMode );
    void setDefaultNameSignal( const OUString &rName );
    void setDisplayDirectorySignal( const OUString &rDirectory );
    OUString getDisplayDirectorySignal();
    css::uno::Sequence< OUString > getFilesSignal();

    // XFilterManager functions
    void appendFilterSignal( const OUString &rTitle, const OUString &rFilter );
    void setCurrentFilterSignal( const OUString &rTitle );
    OUString getCurrentFilterSignal();

    // XFilterGroupManager functions
    void appendFilterGroupSignal( const OUString &rGroupTitle, const css::uno::Sequence< css::beans::StringPair > &rFilters );

    // XFilePickerControlAccess functions
    void setValueSignal( sal_Int16 nControlId, sal_Int16 nControlAction, const css::uno::Any &rValue );
    css::uno::Any getValueSignal( sal_Int16 nControlId, sal_Int16 nControlAction );
    void enableControlSignal( sal_Int16 nControlId, bool bEnable );
    void setLabelSignal( sal_Int16 nControlId, const OUString &rLabel );
    OUString getLabelSignal( sal_Int16 nControlId );

    // XFilePicker2 functions
    css::uno::Sequence< OUString > getSelectedFilesSignal() ;

    // XInitialization
    void initializeSignal( const css::uno::Sequence< css::uno::Any > &rArguments );

    // Destructor proxy
    void cleanupProxySignal();

    // KDE protocol lookup
    void checkProtocolSignal();

private:
    KDE4FilePicker( const KDE4FilePicker& ) = delete;
    KDE4FilePicker& operator=( const KDE4FilePicker& ) = delete;

    //add a custom control widget to the file dialog
    void addCustomControl(sal_Int16 controlId);

    static QString getResString(const char* pRedId);

private Q_SLOTS:
    void cleanupProxy();
    void checkProtocol();

    // emit XFilePickerListener controlStateChanged event
    void filterChanged(const QString &filter);
    // emit XFilePickerListener fileSelectionChanged event
    void selectionChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
