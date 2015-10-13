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

#ifndef INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_FILEPICKER_HXX
#define INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_FILEPICKER_HXX

#include <cppuhelper/compbase.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#include "asynceventnotifier.hxx"
#include "eventnotification.hxx"

#include <memory>

// Implementation class for the XFilePicker Interface

class CWinFileOpenImpl;

class CFilePickerDummy
{
protected:
    osl::Mutex  m_aMutex;
    osl::Mutex  m_rbHelperMtx;
};

typedef ::cppu::WeakComponentImplHelper <
            css::ui::dialogs::XFilePicker3,
            css::ui::dialogs::XFilePickerControlAccess,
            css::ui::dialogs::XFilePreview,
            css::lang::XInitialization,
            css::lang::XServiceInfo >   CFilePicker_Base;

class CFilePicker :
    public CFilePickerDummy,
    public CFilePicker_Base
{
public:

    // ctor
    CFilePicker( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceMgr );

    // XFilePickerNotifier

    virtual void SAL_CALL addFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener )
        throw( css::uno::RuntimeException );
    virtual void SAL_CALL removeFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener )
        throw( css::uno::RuntimeException );

    // XExecutableDialog functions

    virtual void SAL_CALL setTitle( const OUString& aTitle )
        throw( css::uno::RuntimeException );

    virtual sal_Int16 SAL_CALL execute(  )
        throw( css::uno::RuntimeException );

    // XFilePicker functions

    virtual void SAL_CALL setMultiSelectionMode( sal_Bool bMode )
        throw( css::uno::RuntimeException );

    virtual void SAL_CALL setDefaultName( const OUString& aName )
        throw( css::uno::RuntimeException );

    virtual void SAL_CALL setDisplayDirectory( const OUString& aDirectory )
        throw( css::lang::IllegalArgumentException, css::uno::RuntimeException );

    virtual OUString SAL_CALL getDisplayDirectory(  )
        throw( css::uno::RuntimeException );

    virtual css::uno::Sequence< OUString > SAL_CALL getFiles(  )
        throw( css::uno::RuntimeException );

    // XFilePicker2 functions

    virtual css::uno::Sequence< OUString > SAL_CALL getSelectedFiles(  )
        throw (css::uno::RuntimeException, std::exception);

    // XFilterManager functions

    virtual void SAL_CALL appendFilter( const OUString& aTitle, const OUString& aFilter )
        throw( css::lang::IllegalArgumentException, css::uno::RuntimeException );

    virtual void SAL_CALL setCurrentFilter( const OUString& aTitle )
        throw( css::lang::IllegalArgumentException, css::uno::RuntimeException );

    virtual OUString SAL_CALL getCurrentFilter(  )
        throw( css::uno::RuntimeException );

    // XFilterGroupManager functions

    virtual void SAL_CALL appendFilterGroup( const OUString& sGroupTitle, const css::uno::Sequence< css::beans::StringPair >& aFilters )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);

    // XFilePickerControlAccess functions

    virtual void SAL_CALL setValue( sal_Int16 aControlId, sal_Int16 aControlAction, const css::uno::Any& aValue )
        throw (css::uno::RuntimeException);

    virtual css::uno::Any SAL_CALL getValue( sal_Int16 aControlId, sal_Int16 aControlAction )
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL enableControl( sal_Int16 aControlId, sal_Bool bEnable )
        throw(css::uno::RuntimeException );

    virtual void SAL_CALL setLabel( sal_Int16 aControlId, const OUString& aLabel )
        throw (css::uno::RuntimeException);

    virtual OUString SAL_CALL getLabel( sal_Int16 aControlId )
        throw (css::uno::RuntimeException);

    // XFilePreview

    virtual css::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats(  ) throw (css::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getTargetColorDepth(  ) throw (css::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getAvailableWidth(  ) throw (css::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getAvailableHeight(  ) throw (css::uno::RuntimeException);

    virtual void SAL_CALL setImage( sal_Int16 aImageFormat, const css::uno::Any& aImage ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL setShowState( sal_Bool bShowState ) throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL getShowState(  ) throw (css::uno::RuntimeException);

    // XInitialization

    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw(css::uno::Exception, css::uno::RuntimeException);

    // XCancellable

    virtual void SAL_CALL cancel( )
        throw(css::uno::RuntimeException);

    // XEventListener

    virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent )
        throw(css::uno::RuntimeException);

    // XServiceInfo

    virtual OUString SAL_CALL getImplementationName(  )
        throw(css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw(css::uno::RuntimeException);

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw(css::uno::RuntimeException);

    // FilePicker Event functions

    void SAL_CALL fileSelectionChanged( css::ui::dialogs::FilePickerEvent aEvent );
    void SAL_CALL directoryChanged( css::ui::dialogs::FilePickerEvent aEvent );
    OUString SAL_CALL helpRequested( css::ui::dialogs::FilePickerEvent aEvent ) const;
    void SAL_CALL controlStateChanged( css::ui::dialogs::FilePickerEvent aEvent );
    void SAL_CALL dialogSizeChanged( );

    bool startupEventNotification(bool bStartupSuspended);
    void shutdownEventNotification();
    void suspendEventNotification();
    void resumeEventNotification();

private:
    // prevent copy and assignment
    CFilePicker( const CFilePicker& );
    CFilePicker& operator=( const CFilePicker& );

    using WeakComponentImplHelperBase::disposing;

private:
    css::uno::Reference< css::lang::XMultiServiceFactory > m_xServiceMgr;   // to instanciate own services
    CAsyncEventNotifier                                    m_aAsyncEventNotifier;
    std::unique_ptr<CWinFileOpenImpl>                      m_pImpl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
