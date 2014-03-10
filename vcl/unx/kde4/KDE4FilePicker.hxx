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

#include <cppuhelper/compbase5.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>
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

class ResMgr;

typedef ::cppu::WeakComponentImplHelper5
<   ::com::sun::star::ui::dialogs::XFilePicker2
,   ::com::sun::star::ui::dialogs::XFilePicker3
,   ::com::sun::star::ui::dialogs::XFilePickerControlAccess
// TODO ::com::sun::star::ui::dialogs::XFilePreview
,   ::com::sun::star::lang::XInitialization
,   ::com::sun::star::lang::XServiceInfo
> KDE4FilePicker_Base;

class KDE4FilePicker
    : public QObject
    , public KDE4FilePicker_Base
{
    Q_OBJECT
protected:

    ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener > m_xListener;

    ResMgr *_resMgr;

    //the dialog to display
    KFileDialog* _dialog;

    osl::Mutex _helperMutex;

    //running filter string to add to dialog
    QString _filter;

    //mapping of SAL control ID's to created custom controls
    QHash<sal_Int16, QWidget*> _customWidgets;

    //widget to contain extra custom controls
    QWidget* _extraControls;

    //layout for extra custom controls
    QLayout* _layout;

    bool allowRemoteUrls;

public:
    KDE4FilePicker( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& );
    virtual ~KDE4FilePicker();

    // XFilePickerNotifier
    virtual void SAL_CALL addFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException );

    // XExecutableDialog functions
    virtual void SAL_CALL setTitle( const OUString &rTitle ) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Int16 SAL_CALL execute() throw( ::com::sun::star::uno::RuntimeException );

    // XFilePicker functions
    virtual void SAL_CALL setMultiSelectionMode( sal_Bool bMode ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setDefaultName( const OUString &rName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setDisplayDirectory( const OUString &rDirectory ) throw( ::com::sun::star::uno::RuntimeException );
    virtual OUString SAL_CALL getDisplayDirectory() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getFiles() throw( ::com::sun::star::uno::RuntimeException );

    // XFilterManager functions
    virtual void SAL_CALL appendFilter( const OUString &rTitle, const OUString &rFilter ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setCurrentFilter( const OUString &rTitle ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual OUString SAL_CALL getCurrentFilter() throw( ::com::sun::star::uno::RuntimeException );

    // XFilterGroupManager functions
    virtual void SAL_CALL appendFilterGroup( const OUString &rGroupTitle, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > &rFilters ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // XFilePickerControlAccess functions
    virtual void SAL_CALL setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const ::com::sun::star::uno::Any &rValue ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getValue( sal_Int16 nControlId, sal_Int16 nControlAction ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL enableControl( sal_Int16 nControlId, sal_Bool bEnable ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setLabel( sal_Int16 nControlId, const OUString &rLabel ) throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getLabel( sal_Int16 nControlId ) throw (::com::sun::star::uno::RuntimeException);

    /* TODO XFilePreview

    virtual ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL  getTargetColorDepth(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL  getAvailableWidth(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL  getAvailableHeight(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL       setImage( sal_Int16 aImageFormat, const ::com::sun::star::uno::Any &rImage ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL   setShowState( sal_Bool bShowState ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL   getShowState(  ) throw (::com::sun::star::uno::RuntimeException);
    */

    // XFilePicker2 functions
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSelectedFiles()
            throw (::com::sun::star::uno::RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > &rArguments ) throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException );

    // XCancellable
    virtual void SAL_CALL cancel( ) throw( ::com::sun::star::uno::RuntimeException );

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject &rEvent ) throw( ::com::sun::star::uno::RuntimeException );
    using cppu::WeakComponentImplHelperBase::disposing;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const OUString &rServiceName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

private Q_SLOTS:
    // XExecutableDialog functions
    void setTitleSlot( const OUString &rTitle ) throw( ::com::sun::star::uno::RuntimeException ) { return setTitle( rTitle ); }
    sal_Int16 executeSlot() throw( ::com::sun::star::uno::RuntimeException ) { return execute(); }

    // XFilePicker functions
    void setMultiSelectionModeSlot( sal_Bool bMode ) throw( ::com::sun::star::uno::RuntimeException ) { return setMultiSelectionMode( bMode ); }
    void setDefaultNameSlot( const OUString &rName ) throw( ::com::sun::star::uno::RuntimeException ) { return setDefaultName( rName ); }
    void setDisplayDirectorySlot( const OUString &rDirectory ) throw( ::com::sun::star::uno::RuntimeException ) { return setDisplayDirectory( rDirectory ); }
    OUString getDisplayDirectorySlot() throw( ::com::sun::star::uno::RuntimeException ) { return getDisplayDirectory(); }
    ::com::sun::star::uno::Sequence< OUString > getFilesSlot() throw( ::com::sun::star::uno::RuntimeException ) { return getFiles(); }

    // XFilterManager functions
    void appendFilterSlot( const OUString &rTitle, const OUString &rFilter ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException ) { return appendFilter( rTitle, rFilter ); }
    void setCurrentFilterSlot( const OUString &rTitle ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException ) { return setCurrentFilter( rTitle ); }
    OUString getCurrentFilterSlot() throw( ::com::sun::star::uno::RuntimeException ) { return getCurrentFilter(); }

    // XFilterGroupManager functions
    void appendFilterGroupSlot( const OUString &rGroupTitle, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > &rFilters ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException) { return appendFilterGroup( rGroupTitle, rFilters ); }

    // XFilePickerControlAccess functions
    void setValueSlot( sal_Int16 nControlId, sal_Int16 nControlAction, const ::com::sun::star::uno::Any &rValue ) throw (::com::sun::star::uno::RuntimeException) { return setValue( nControlId, nControlAction, rValue ); }
    ::com::sun::star::uno::Any getValueSlot( sal_Int16 nControlId, sal_Int16 nControlAction ) throw (::com::sun::star::uno::RuntimeException) { return getValue( nControlId, nControlAction ); }
    void enableControlSlot( sal_Int16 nControlId, sal_Bool bEnable ) throw( ::com::sun::star::uno::RuntimeException ) { return enableControl( nControlId, bEnable ); }
    void setLabelSlot( sal_Int16 nControlId, const OUString &rLabel ) throw (::com::sun::star::uno::RuntimeException) { return setLabel( nControlId, rLabel ); }
    OUString getLabelSlot( sal_Int16 nControlId ) throw (::com::sun::star::uno::RuntimeException) { return getLabel( nControlId ); }

    // XFilePicker2 functions
    ::com::sun::star::uno::Sequence< OUString > getSelectedFilesSlot() throw (::com::sun::star::uno::RuntimeException) { return getSelectedFiles(); }

    // XInitialization
    void initializeSlot( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > &rArguments ) throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException ) { return initialize( rArguments ); }

Q_SIGNALS:
    // XExecutableDialog functions
    void setTitleSignal( const OUString &rTitle );
    sal_Int16 executeSignal();

    // XFilePicker functions
    void setMultiSelectionModeSignal( sal_Bool bMode );
    void setDefaultNameSignal( const OUString &rName );
    void setDisplayDirectorySignal( const OUString &rDirectory );
    OUString getDisplayDirectorySignal();
    ::com::sun::star::uno::Sequence< OUString > getFilesSignal();

    // XFilterManager functions
    void appendFilterSignal( const OUString &rTitle, const OUString &rFilter );
    void setCurrentFilterSignal( const OUString &rTitle );
    OUString getCurrentFilterSignal();

    // XFilterGroupManager functions
    void appendFilterGroupSignal( const OUString &rGroupTitle, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > &rFilters );

    // XFilePickerControlAccess functions
    void setValueSignal( sal_Int16 nControlId, sal_Int16 nControlAction, const ::com::sun::star::uno::Any &rValue );
    ::com::sun::star::uno::Any getValueSignal( sal_Int16 nControlId, sal_Int16 nControlAction );
    void enableControlSignal( sal_Int16 nControlId, sal_Bool bEnable );
    void setLabelSignal( sal_Int16 nControlId, const OUString &rLabel );
    OUString getLabelSignal( sal_Int16 nControlId );

    // XFilePicker2 functions
    ::com::sun::star::uno::Sequence< OUString > getSelectedFilesSignal() ;

    // XInitialization
    void initializeSignal( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > &rArguments );

    // Destructor proxy
    void cleanupProxySignal();

    // KDE protocol lookup
    void checkProtocolSignal();

private:
    // prevent copy and assignment
    KDE4FilePicker( const KDE4FilePicker& );
    KDE4FilePicker& operator=( const KDE4FilePicker& );

    //add a custom control widget to the file dialog
    void addCustomControl(sal_Int16 controlId);

private Q_SLOTS:
    void cleanupProxy();
    void checkProtocol();

    // emit XFilePickerListener controlStateChanged event
    void filterChanged(const QString &filter);
    // emit XFilePickerListener fileSelectionChanged event
    void selectionChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
