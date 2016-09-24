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
        css::ui::dialogs::XFilePicker3,
        css::ui::dialogs::XFilePickerControlAccess,
// TODO css::ui::dialogs::XFilePreview,
        css::lang::XInitialization,
        css::lang::XServiceInfo >  UnxFilePicker_Base;

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
    explicit UnxFilePicker( const css::uno::Reference< css::uno::XComponentContext >& );
    virtual ~UnxFilePicker();

    // XComponent

    using cppu::WeakComponentImplHelperBase::disposing;

    // XFilePickerNotifier

    virtual void SAL_CALL       addFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL       removeFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) throw( css::uno::RuntimeException, std::exception ) override;

    // XExecutableDialog functions

    virtual void SAL_CALL       setTitle( const OUString &rTitle ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int16 SAL_CALL  execute() throw( css::uno::RuntimeException, std::exception ) override;

    // XFilePicker functions

    virtual void SAL_CALL       setMultiSelectionMode( sal_Bool bMode ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL       setDefaultName( const OUString &rName ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL       setDisplayDirectory( const OUString &rDirectory ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getDisplayDirectory() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getFiles() throw( css::uno::RuntimeException, std::exception ) override;

    // XFilterManager functions

    virtual void SAL_CALL       appendFilter( const OUString &rTitle, const OUString &rFilter ) throw( css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL       setCurrentFilter( const OUString &rTitle ) throw( css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getCurrentFilter() throw( css::uno::RuntimeException, std::exception ) override;

    // XFilterGroupManager functions

    virtual void SAL_CALL       appendFilterGroup( const OUString &rGroupTitle, const css::uno::Sequence< css::beans::StringPair > &rFilters ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    // XFilePickerControlAccess functions

    virtual void SAL_CALL       setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const css::uno::Any &rValue ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getValue( sal_Int16 nControlId, sal_Int16 nControlAction ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL       enableControl( sal_Int16 nControlId, sal_Bool bEnable ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL       setLabel( sal_Int16 nControlId, const OUString &rLabel ) throw (css::uno::RuntimeException, std::exception) override;
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

    // XFilePicker2

    virtual css::uno::Sequence< OUString > SAL_CALL getSelectedFiles()
            throw (css::uno::RuntimeException, std::exception) override;

    // XInitialization

    virtual void SAL_CALL       initialize( const css::uno::Sequence< css::uno::Any > &rArguments ) throw( css::uno::Exception, css::uno::RuntimeException, std::exception ) override;

    // XCancellable

    virtual void SAL_CALL       cancel( ) throw( css::uno::RuntimeException, std::exception ) override;

    // XEventListener

    virtual void SAL_CALL       disposing( const css::lang::EventObject &rEvent ) throw( css::uno::RuntimeException );

    // XServiceInfo

    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL   supportsService( const OUString &rServiceName ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

private:
    UnxFilePicker( const UnxFilePicker& ) = delete;
    UnxFilePicker& operator=( const UnxFilePicker& ) = delete;

protected:
    void                        initFilePicker();
    void                        checkFilePicker() throw( css::uno::RuntimeException );

    // Async sendCommand
    void                        sendCommand( const OUString &rCommand );
    // Synchronized sendCommand
    void                        sendCommand( const OUString &rCommand, ::osl::Condition &rCondition );

private:
    void                        sendAppendControlCommand( sal_Int16 nControlId );
};

#endif // INCLUDED_VCL_UNX_KDE_UNXFILEPICKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
