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

#ifndef _UNXFILEPICKER_HXX_
#define _UNXFILEPICKER_HXX_

#include <cppuhelper/compbase5.hxx>
#include <osl/conditn.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <list>

class UnxFilePickerCommandThread;
class UnxFilePickerNotifyThread;
class ResMgr;

class UnxFilePickerDummy
{
protected:
    osl::Mutex                  m_aMutex;
    osl::Mutex                  m_rbHelperMtx;
};

typedef ::cppu::WeakComponentImplHelper5<
        ::com::sun::star::ui::dialogs::XFilePicker2,
        ::com::sun::star::ui::dialogs::XFilePicker3,
        ::com::sun::star::ui::dialogs::XFilePickerControlAccess,
// TODO ::com::sun::star::ui::dialogs::XFilePreview,
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::lang::XServiceInfo >  UnxFilePicker_Base;

class UnxFilePicker :
    public UnxFilePickerDummy,
    public UnxFilePicker_Base
{
protected:

    pid_t                       m_nFilePickerPid;
    int                         m_nFilePickerWrite; // (kde|...)filepicker gets it as stdin
    int                         m_nFilePickerRead;  // (kde|...)filepicker gets it as stdout

    UnxFilePickerNotifyThread  *m_pNotifyThread;
    UnxFilePickerCommandThread *m_pCommandThread;

    ResMgr                     *m_pResMgr;

public:
    UnxFilePicker( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& );
    virtual ~UnxFilePicker();

    // XComponent

    using cppu::WeakComponentImplHelperBase::disposing;

    // XFilePickerNotifier

    virtual void SAL_CALL       addFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL       removeFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException );

    // XExecutableDialog functions

    virtual void SAL_CALL       setTitle( const ::rtl::OUString &rTitle ) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Int16 SAL_CALL  execute() throw( ::com::sun::star::uno::RuntimeException );

    // XFilePicker functions

    virtual void SAL_CALL       setMultiSelectionMode( sal_Bool bMode ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL       setDefaultName( const ::rtl::OUString &rName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL       setDisplayDirectory( const ::rtl::OUString &rDirectory ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getDisplayDirectory() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getFiles() throw( ::com::sun::star::uno::RuntimeException );

    // XFilterManager functions

    virtual void SAL_CALL       appendFilter( const ::rtl::OUString &rTitle, const ::rtl::OUString &rFilter ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL       setCurrentFilter( const ::rtl::OUString &rTitle ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getCurrentFilter() throw( ::com::sun::star::uno::RuntimeException );

    // XFilterGroupManager functions

    virtual void SAL_CALL       appendFilterGroup( const ::rtl::OUString &rGroupTitle, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > &rFilters ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // XFilePickerControlAccess functions

    virtual void SAL_CALL       setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const ::com::sun::star::uno::Any &rValue ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getValue( sal_Int16 nControlId, sal_Int16 nControlAction ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL       enableControl( sal_Int16 nControlId, sal_Bool bEnable ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL       setLabel( sal_Int16 nControlId, const ::rtl::OUString &rLabel ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLabel( sal_Int16 nControlId ) throw (::com::sun::star::uno::RuntimeException);

    /* TODO XFilePreview

    virtual ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL  getTargetColorDepth(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL  getAvailableWidth(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL  getAvailableHeight(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL       setImage( sal_Int16 aImageFormat, const ::com::sun::star::uno::Any &rImage ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL   setShowState( sal_Bool bShowState ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL   getShowState(  ) throw (::com::sun::star::uno::RuntimeException);
    */

    // XFilePicker2

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSelectedFiles()
            throw (::com::sun::star::uno::RuntimeException);

    // XInitialization

    virtual void SAL_CALL       initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > &rArguments ) throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException );

    // XCancellable

    virtual void SAL_CALL       cancel( ) throw( ::com::sun::star::uno::RuntimeException );

    // XEventListener

    virtual void SAL_CALL       disposing( const ::com::sun::star::lang::EventObject &rEvent ) throw( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo

    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL   supportsService( const ::rtl::OUString &rServiceName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

private:
    // prevent copy and assignment
    UnxFilePicker( const UnxFilePicker& );
    UnxFilePicker& operator=( const UnxFilePicker& );

protected:
    void                        initFilePicker();
    void                        checkFilePicker() throw( ::com::sun::star::uno::RuntimeException );

    // Async sendCommand
    void                        sendCommand( const ::rtl::OUString &rCommand );
    // Synchronized sendCommand
    void                        sendCommand( const ::rtl::OUString &rCommand, ::osl::Condition &rCondition );
    void                        appendEscaped( ::rtl::OUStringBuffer &rBuffer, const ::rtl::OUString &rString );

private:
    sal_Bool                    controlIdInfo( sal_Int16 nControlId, ::rtl::OUString &rType, sal_Int32 &rTitleId );
    sal_Bool                    controlActionInfo( sal_Int16 nControlId, ::rtl::OUString &rType );
    void                        sendAppendControlCommand( sal_Int16 nControlId );
};

#endif // _UNXFILEPICKER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
