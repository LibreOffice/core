/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
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

#ifndef _UNXCOMMANDTHREAD_HXX_
#define _UNXCOMMANDTHREAD_HXX_

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <osl/conditn.hxx>
#include <osl/mutex.hxx>
#include <osl/thread.hxx>
#include <rtl/ustring.hxx>

#include <vcl/svapp.hxx>

#include <list>

class UnxFilePickerNotifyThread;

/** Synchronization for the 'thread-less' version of the fpicker.

    Something like osl::Condition, but calls Application::Yield() while in
    wait().
*/
class YieldingCondition
{
    ::osl::Mutex m_aMutex;
    bool m_bValue;

    bool get()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_bValue;
    }

public:
    YieldingCondition() { reset(); }

    void reset()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_bValue = false;
    }

    void set()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_bValue = true;
    }

    void wait()
    {
        while ( !get() )
            Application::Yield();
    }
};

class UnxFilePickerCommandThread : public ::osl::Thread
{
protected:
    UnxFilePickerNotifyThread  *m_pNotifyThread;
    int                         m_nReadFD;

    ::osl::Mutex                m_aMutex;

    YieldingCondition           m_aExecCondition;
    sal_Bool                    m_aResult;

    ::osl::Condition            m_aGetCurrentFilterCondition;
    ::rtl::OUString             m_aGetCurrentFilter;

    ::osl::Condition            m_aGetDirectoryCondition;
    ::rtl::OUString             m_aGetDirectory;

    ::osl::Condition            m_aGetFilesCondition;
    ::std::list< ::rtl::OUString > m_aGetFiles;

    ::osl::Condition            m_aGetValueCondition;
    ::com::sun::star::uno::Any  m_aGetValue;

public:
    UnxFilePickerCommandThread( UnxFilePickerNotifyThread *pNotifyThread, int nReadFD );
    ~UnxFilePickerCommandThread();

    YieldingCondition& SAL_CALL execCondition() { return m_aExecCondition; }
    sal_Bool SAL_CALL           result();

    ::osl::Condition& SAL_CALL  getCurrentFilterCondition() { return m_aGetCurrentFilterCondition; }
    ::rtl::OUString SAL_CALL    getCurrentFilter();

    ::osl::Condition& SAL_CALL  getDirectoryCondition() { return m_aGetDirectoryCondition; }
    ::rtl::OUString SAL_CALL    getDirectory();

    ::osl::Condition& SAL_CALL  getFilesCondition() { return m_aGetFilesCondition; }
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getFiles();

    ::osl::Condition& SAL_CALL  getValueCondition() { return m_aGetValueCondition; }
    ::com::sun::star::uno::Any SAL_CALL getValue();

protected:
    virtual void SAL_CALL       run();

    virtual void SAL_CALL       handleCommand( const ::rtl::OUString &rCommand/*, sal_Bool &rQuit*/ );
    ::std::list< ::rtl::OUString > SAL_CALL tokenize( const ::rtl::OUString &rCommand );
};

#endif // _UNXCOMMANDTHREAD_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
