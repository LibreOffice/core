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

#include <QObject>
#include <QString>
#include <QHash>

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
    virtual ~KDE4FilePicker();

    // XFilePickerNotifier
    virtual void SAL_CALL addFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) throw( css::uno::RuntimeException, std::exception ) override;

    // XExecutableDialog functions
    virtual void SAL_CALL setTitle( const OUString &rTitle ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int16 SAL_CALL execute() throw( css::uno::RuntimeException, std::exception ) override;

    // XFilePicker functions
    virtual void SAL_CALL setMultiSelectionMode( sal_Bool bMode ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setDefaultName( const OUString &rName ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setDisplayDirectory( const OUString &rDirectory ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getDisplayDirectory() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getFiles() throw( css::uno::RuntimeException, std::exception ) override;

    // XFilterManager functions
    virtual void SAL_CALL appendFilter( const OUString &rTitle, const OUString &rFilter ) throw( css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setCurrentFilter( const OUString &rTitle ) throw( css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getCurrentFilter() throw( css::uno::RuntimeException, std::exception ) override;

    // XFilterGroupManager functions
    virtual void SAL_CALL appendFilterGroup( const OUString &rGroupTitle, const css::uno::Sequence< css::beans::StringPair > &rFilters ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    // XFilePickerControlAccess functions
    virtual void SAL_CALL setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const css::uno::Any &rValue ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getValue( sal_Int16 nControlId, sal_Int16 nControlAction ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL enableControl( sal_Int16 nControlId, sal_Bool bEnable ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setLabel( sal_Int16 nControlId, const OUString &rLabel ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getLabel( sal_Int16 nControlId ) throw (css::uno::RuntimeException, std::exception) override;

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
    virtual css::uno::Sequence< OUString > SAL_CALL getSelectedFiles()
            throw (css::uno::RuntimeException, std::exception) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any > &rArguments ) throw( css::uno::Exception, css::uno::RuntimeException, std::exception ) override;

    // XCancellable
    virtual void SAL_CALL cancel( ) throw( css::uno::RuntimeException, std::exception ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject &rEvent ) throw( css::uno::RuntimeException );
    using cppu::WeakComponentImplHelperBase::disposing;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString &rServiceName ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

private Q_SLOTS:
    // XExecutableDialog functions
    void setTitleSlot( const OUString &rTitle ) throw( css::uno::RuntimeException ) { return setTitle( rTitle ); }
    sal_Int16 executeSlot() throw( css::uno::RuntimeException ) { return execute(); }

    // XFilePicker functions
    void setMultiSelectionModeSlot( bool bMode ) throw( css::uno::RuntimeException ) { return setMultiSelectionMode( bMode ); }
    void setDefaultNameSlot( const OUString &rName ) throw( css::uno::RuntimeException ) { return setDefaultName( rName ); }
    void setDisplayDirectorySlot( const OUString &rDirectory ) throw( css::uno::RuntimeException ) { return setDisplayDirectory( rDirectory ); }
    OUString getDisplayDirectorySlot() throw( css::uno::RuntimeException ) { return getDisplayDirectory(); }
    css::uno::Sequence< OUString > getFilesSlot() throw( css::uno::RuntimeException ) { return getFiles(); }

    // XFilterManager functions
    void appendFilterSlot( const OUString &rTitle, const OUString &rFilter ) throw( css::lang::IllegalArgumentException, css::uno::RuntimeException ) { return appendFilter( rTitle, rFilter ); }
    void setCurrentFilterSlot( const OUString &rTitle ) throw( css::lang::IllegalArgumentException, css::uno::RuntimeException ) { return setCurrentFilter( rTitle ); }
    OUString getCurrentFilterSlot() throw( css::uno::RuntimeException ) { return getCurrentFilter(); }

    // XFilterGroupManager functions
    void appendFilterGroupSlot( const OUString &rGroupTitle, const css::uno::Sequence< css::beans::StringPair > &rFilters ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException) { return appendFilterGroup( rGroupTitle, rFilters ); }

    // XFilePickerControlAccess functions
    void setValueSlot( sal_Int16 nControlId, sal_Int16 nControlAction, const css::uno::Any &rValue ) throw (css::uno::RuntimeException) { return setValue( nControlId, nControlAction, rValue ); }
    css::uno::Any getValueSlot( sal_Int16 nControlId, sal_Int16 nControlAction ) throw (css::uno::RuntimeException) { return getValue( nControlId, nControlAction ); }
    void enableControlSlot( sal_Int16 nControlId, bool bEnable ) throw( css::uno::RuntimeException ) { return enableControl( nControlId, bEnable ); }
    void setLabelSlot( sal_Int16 nControlId, const OUString &rLabel ) throw (css::uno::RuntimeException) { return setLabel( nControlId, rLabel ); }
    OUString getLabelSlot( sal_Int16 nControlId ) throw (css::uno::RuntimeException) { return getLabel( nControlId ); }

    // XFilePicker2 functions
    css::uno::Sequence< OUString > getSelectedFilesSlot() throw (css::uno::RuntimeException) { return getSelectedFiles(); }

    // XInitialization
    void initializeSlot( const css::uno::Sequence< css::uno::Any > &rArguments ) throw( css::uno::Exception, css::uno::RuntimeException ) { return initialize( rArguments ); }

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

    static QString getResString( sal_Int16 aRedId );

private Q_SLOTS:
    void cleanupProxy();
    void checkProtocol();

    // emit XFilePickerListener controlStateChanged event
    void filterChanged(const QString &filter);
    // emit XFilePickerListener fileSelectionChanged event
    void selectionChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
