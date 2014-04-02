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

#include <UnxNotifyThread.hxx>
#include <UnxFilePicker.hxx>

using namespace ::com::sun::star;

// UnxFilePickerNotifyThread

UnxFilePickerNotifyThread::UnxFilePickerNotifyThread( UnxFilePicker *pUnxFilePicker )
    : m_pUnxFilePicker( pUnxFilePicker ),
      m_bExit( false ),
      m_eNotifyType( Nothing ),
      m_nControlId( 0 )
{
}

void SAL_CALL UnxFilePickerNotifyThread::addFilePickerListener( const uno::Reference< ui::dialogs::XFilePickerListener >& xListener )
    throw( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    m_xListener = xListener;
}

void SAL_CALL UnxFilePickerNotifyThread::removeFilePickerListener( const uno::Reference< ui::dialogs::XFilePickerListener >& /*xListener*/ )
    throw( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    m_xListener.clear();
}

void SAL_CALL UnxFilePickerNotifyThread::exit()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    m_bExit = true;

    m_aExitCondition.reset();
    m_aNotifyCondition.set();

    m_aExitCondition.wait();
}

void SAL_CALL UnxFilePickerNotifyThread::fileSelectionChanged()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    m_eNotifyType = FileSelectionChanged;
    m_nControlId = 0;

    m_aNotifyCondition.set();
}

void SAL_CALL UnxFilePickerNotifyThread::run()
{
    do {
        m_aNotifyCondition.reset();
        m_aNotifyCondition.wait();

        if ( m_xListener.is() && m_pUnxFilePicker )
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            ui::dialogs::FilePickerEvent aEvent( *m_pUnxFilePicker, m_nControlId );

            switch ( m_eNotifyType )
            {
                case FileSelectionChanged:
                    m_xListener->fileSelectionChanged( aEvent );
                    break;

                // TODO More to come...

                default:
                    // nothing
                    break;
            }
        }
    } while ( !m_bExit );

    m_aExitCondition.set();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
