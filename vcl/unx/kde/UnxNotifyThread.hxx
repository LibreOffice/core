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
#if 1

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
