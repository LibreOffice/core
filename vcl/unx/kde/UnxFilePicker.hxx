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

#ifndef INCLUDED_VCL_UNX_KDE_UNXFILEPICKER_HXX
#define INCLUDED_VCL_UNX_KDE_UNXFILEPICKER_HXX

#include <cppuhelper/compbase.hxx>
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

typedef ::cppu::WeakComponentImplHelper<
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
    explicit UnxFilePicker( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& );
    virtual ~UnxFilePicker();

    // XComponent

    using cppu::WeakComponentImplHelperBase::disposing;

    // XFilePickerNotifier

    virtual void SAL_CALL       addFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL       removeFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XExecutableDialog functions

    virtual void SAL_CALL       setTitle( const OUString &rTitle ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL  execute() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XFilePicker functions

    virtual void SAL_CALL       setMultiSelectionMode( sal_Bool bMode ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL       setDefaultName( const OUString &rName ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL       setDisplayDirectory( const OUString &rDirectory ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual OUString SAL_CALL getDisplayDirectory() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getFiles() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XFilterManager functions

    virtual void SAL_CALL       appendFilter( const OUString &rTitle, const OUString &rFilter ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL       setCurrentFilter( const OUString &rTitle ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual OUString SAL_CALL getCurrentFilter() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XFilterGroupManager functions

    virtual void SAL_CALL       appendFilterGroup( const OUString &rGroupTitle, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > &rFilters ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XFilePickerControlAccess functions

    virtual void SAL_CALL       setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const ::com::sun::star::uno::Any &rValue ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getValue( sal_Int16 nControlId, sal_Int16 nControlAction ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL       enableControl( sal_Int16 nControlId, sal_Bool bEnable ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL       setLabel( sal_Int16 nControlId, const OUString &rLabel ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getLabel( sal_Int16 nControlId ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

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

    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSelectedFiles()
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XInitialization

    virtual void SAL_CALL       initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > &rArguments ) throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XCancellable

    virtual void SAL_CALL       cancel( ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XEventListener

    virtual void SAL_CALL       disposing( const ::com::sun::star::lang::EventObject &rEvent ) throw( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo

    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL   supportsService( const OUString &rServiceName ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

private:
    UnxFilePicker( const UnxFilePicker& ) SAL_DELETED_FUNCTION;
    UnxFilePicker& operator=( const UnxFilePicker& ) SAL_DELETED_FUNCTION;

protected:
    void                        initFilePicker();
    void                        checkFilePicker() throw( ::com::sun::star::uno::RuntimeException );

    // Async sendCommand
    void                        sendCommand( const OUString &rCommand );
    // Synchronized sendCommand
    void                        sendCommand( const OUString &rCommand, ::osl::Condition &rCondition );

private:
    void                        sendAppendControlCommand( sal_Int16 nControlId );
};

#endif // INCLUDED_VCL_UNX_KDE_UNXFILEPICKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
