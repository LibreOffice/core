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

#ifndef _UNXNOTIFYTHREAD_HXX_
#define _UNXNOTIFYTHREAD_HXX_

#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <osl/conditn.hxx>
#include <osl/mutex.hxx>
#include <osl/thread.hxx>

class UnxFilePicker;

class UnxFilePickerNotifyThread : public ::osl::Thread
{
protected:
    enum NotifyType
    {
        Nothing = 0,
        FileSelectionChanged
    // TODO More to come...
    };

    UnxFilePicker              *m_pUnxFilePicker;

    ::osl::Mutex                m_aMutex;

    ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener > m_xListener;

    sal_Bool                    m_bExit;
    ::osl::Condition            m_aExitCondition;

    NotifyType                  m_eNotifyType;
    ::osl::Condition            m_aNotifyCondition;
    sal_Int16                   m_nControlId;

public:
    UnxFilePickerNotifyThread( UnxFilePicker *pUnxFilePicker );

    virtual void SAL_CALL addFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener )
        throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener )
        throw( ::com::sun::star::uno::RuntimeException );

    void SAL_CALL               exit();

    void SAL_CALL               fileSelectionChanged();
    /* TODO
    void SAL_CALL directoryChanged( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent );
    rtl::OUString SAL_CALL helpRequested( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent ) const;
    void SAL_CALL controlStateChanged( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent );
    void SAL_CALL dialogSizeChanged( );
     */

protected:
    virtual void SAL_CALL       run();
};

#endif // _UNXNOTIFYTHREAD_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
