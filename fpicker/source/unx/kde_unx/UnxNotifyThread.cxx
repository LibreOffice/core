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

#include <UnxNotifyThread.hxx>
#include <UnxFilePicker.hxx>

using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////
// UnxFilePickerNotifyThread
//////////////////////////////////////////////////////////////////////////

UnxFilePickerNotifyThread::UnxFilePickerNotifyThread( UnxFilePicker *pUnxFilePicker )
    : m_pUnxFilePicker( pUnxFilePicker ),
      m_bExit( sal_False ),
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

    m_bExit = sal_True;

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
