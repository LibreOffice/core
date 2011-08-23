/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 * Copyright 2010 Novell, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _UNXFILEPICKER_HXX_
#define _UNXFILEPICKER_HXX_

#include <cppuhelper/compbase8.hxx>
#include <osl/conditn.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/XFilterGroupManager.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#include <com/sun/star/util/XCancellable.hpp>

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

class UnxFilePicker :
    public UnxFilePickerDummy,
    public cppu::WeakComponentImplHelper8<
        ::com::sun::star::ui::dialogs::XFilterManager,
        ::com::sun::star::ui::dialogs::XFilterGroupManager,
        ::com::sun::star::ui::dialogs::XFilePickerControlAccess,
        ::com::sun::star::ui::dialogs::XFilePickerNotifier,
// TODO ::com::sun::star::ui::dialogs::XFilePreview,
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::util::XCancellable,
        ::com::sun::star::lang::XEventListener,
        ::com::sun::star::lang::XServiceInfo >
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceMgr;   // to instanciate own services

    pid_t                       m_nFilePickerPid;
    int                         m_nFilePickerWrite; // (kde|...)filepicker gets it as stdin
    int                         m_nFilePickerRead;  // (kde|...)filepicker gets it as stdout

    UnxFilePickerNotifyThread  *m_pNotifyThread;
    UnxFilePickerCommandThread *m_pCommandThread;

    ResMgr                     *m_pResMgr;

public:
    UnxFilePicker( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceMgr );
    virtual ~UnxFilePicker();

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
