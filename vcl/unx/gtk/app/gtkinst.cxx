/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gtkinst.cxx,v $
 * $Revision: 1.19 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <osl/module.h>
#include <plugins/gtk/gtkdata.hxx>
#include <plugins/gtk/gtkinst.hxx>
#include <salframe.h>
#include <salobj.h>
#include <plugins/gtk/gtkframe.hxx>
#include <plugins/gtk/gtkobject.hxx>
#include <plugins/gtk/atkbridge.hxx>

#include <rtl/strbuf.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>

GtkHookedYieldMutex::GtkHookedYieldMutex()
{
}

/*
 * These methods always occur in pairs
 * A ThreadsEnter is followed by a ThreadsLeave
 * We need to queue up the recursive lock count
 * for each pair, so we can accurately restore
 * it later.
 */
void GtkHookedYieldMutex::ThreadsEnter()
{
    acquire();
    if( !aYieldStack.empty() )
    { /* Previously called ThreadsLeave() */
        ULONG nCount = aYieldStack.front();
        aYieldStack.pop_front();
        while( nCount-- > 1 )
            acquire();
    }
}

void GtkHookedYieldMutex::ThreadsLeave()
{
    aYieldStack.push_front( mnCount );

#if OSL_DEBUG_LEVEL > 1
    if( mnThreadId &&
        mnThreadId != NAMESPACE_VOS(OThread)::getCurrentIdentifier())
        fprintf( stderr, "\n\n--- A different thread owns the mutex ...---\n\n\n");
#endif

    while( mnCount > 1 )
        release();
    release();
}

void GtkHookedYieldMutex::acquire()
{
    SalYieldMutex::acquire();
}

void GtkHookedYieldMutex::release()
{
    SalYieldMutex::release();
}

extern "C"
{
    #define GET_YIELD_MUTEX() static_cast<GtkHookedYieldMutex*>(GetSalData()->m_pInstance->GetYieldMutex())
    static void GdkThreadsEnter( void )
    {
        GtkHookedYieldMutex *pYieldMutex = GET_YIELD_MUTEX();
        pYieldMutex->ThreadsEnter();
    }
    static void GdkThreadsLeave( void )
    {
        GtkHookedYieldMutex *pYieldMutex = GET_YIELD_MUTEX();
        pYieldMutex->ThreadsLeave();
    }
    static bool hookLocks( oslModule pModule )
    {
        typedef void (*GdkLockFn) (GCallback enter_fn, GCallback leave_fn);
        rtl::OUString aSymbolName( RTL_CONSTASCII_USTRINGPARAM( "gdk_threads_set_lock_functions") );

        GdkLockFn gdk_threads_set_lock_functions =
                (GdkLockFn) osl_getFunctionSymbol( pModule, aSymbolName.pData );
        if ( !gdk_threads_set_lock_functions )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "Failed to hook gdk threads locks\n" );
#endif
            return false;
        }

        gdk_threads_set_lock_functions (GdkThreadsEnter, GdkThreadsLeave);
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "Hooked gdk threads locks\n" );
#endif
        return true;
    }

    VCL_DLLPUBLIC SalInstance* create_SalInstance( oslModule pModule )
    {
        #if OSL_DEBUG_LEVEL > 1
        int nFd = open( "/home/pl93762/log.txt", O_CREAT | O_TRUNC | O_WRONLY, 0755 );
        dup2( nFd, STDERR_FILENO );
        #endif

        const gchar* pVersion = gtk_check_version( 2, 2, 0 );
        if( pVersion )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "gtk version conflict: %s\n", pVersion );
#endif
            return NULL;
        }

        GtkYieldMutex *pYieldMutex;

        // init gdk thread protection
        if ( !g_thread_supported() )
            g_thread_init( NULL );

        if ( hookLocks( pModule ) )
            pYieldMutex = new GtkHookedYieldMutex();
        else
            pYieldMutex = new GtkYieldMutex();

        gdk_threads_init();

        GtkInstance* pInstance = new GtkInstance( pYieldMutex );
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "creating GtkSalInstance 0x%p\n", pInstance );
#endif

        // initialize SalData
        GtkData *pSalData = new GtkData();
        SetSalData( pSalData );
        pSalData->m_pInstance = pInstance;
        pSalData->Init();
        pSalData->initNWF();

        const char* pGtkModules = getenv( "GTK_MODULES" );
        if( pGtkModules )
        {
            rtl::OString aModules( pGtkModules );
            sal_Int32 nIndex = 0;
            while( nIndex >= 0 )
            {
                rtl::OString aToken = aModules.getToken( 0, ':', nIndex );
                if( aToken.equals( "gail" ) || aToken.equals( "atk-bridge" ) )
                {
                    InitAtkBridge();
                    break;
                }
            }
        }

        return pInstance;
    }
}

GtkInstance::~GtkInstance()
{
    DeInitAtkBridge();
}

SalFrame* GtkInstance::CreateFrame( SalFrame* pParent, ULONG nStyle )
{
    return new GtkSalFrame( pParent, nStyle );
}

SalFrame* GtkInstance::CreateChildFrame( SystemParentData* pParentData, ULONG )
{
    SalFrame* pFrame = new GtkSalFrame( pParentData );

    return pFrame;
}

SalObject* GtkInstance::CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, BOOL bShow )
{
    // there is no method to set a visual for a GtkWidget
    // so we need the X11SalObject in that case
    if( pWindowData )
        return X11SalObject::CreateObject( pParent, pWindowData, bShow );

    return new GtkSalObject( static_cast<GtkSalFrame*>(pParent), bShow );
}

GtkYieldMutex::GtkYieldMutex()
{
}

void GtkYieldMutex::acquire()
{
    vos::OThread::TThreadIdentifier aCurrentThread = vos::OThread::getCurrentIdentifier();
    // protect member manipulation
    OMutex::acquire();
    if( mnCount > 0 && mnThreadId == aCurrentThread )
    {
        mnCount++;
        OMutex::release();
        return;
    }
    OMutex::release();

    // obtain gdk mutex
    gdk_threads_enter();

    // obtained gdk mutex, now lock count is one by definition
    OMutex::acquire();
    mnCount = 1;
    mnThreadId = aCurrentThread;
    OMutex::release();
}

void GtkYieldMutex::release()
{
    vos::OThread::TThreadIdentifier aCurrentThread = vos::OThread::getCurrentIdentifier();
    // protect member manipulation
    OMutex::acquire();
    // strange things happen, do nothing if we don't own the mutex
    if( mnThreadId == aCurrentThread )
    {
        mnCount--;
        if( mnCount == 0 )
        {
            gdk_threads_leave();
            mnThreadId = 0;
        }
    }
    OMutex::release();
}

sal_Bool GtkYieldMutex::tryToAcquire()
{
    vos::OThread::TThreadIdentifier aCurrentThread = vos::OThread::getCurrentIdentifier();
    // protect member manipulation
    OMutex::acquire();
    if( mnCount > 0 )
    {
        if( mnThreadId == aCurrentThread )
        {
            mnCount++;
            OMutex::release();
            return sal_True;
        }
        else
        {
            OMutex::release();
            return sal_False;
        }
    }
    OMutex::release();

    // HACK: gdk_threads_mutex is private, we shouldn't use it.
    // how to we do a try_lock without having a gdk_threads_try_enter ?
    if( ! g_mutex_trylock( gdk_threads_mutex ) )
        return sal_False;

    // obtained gdk mutex, now lock count is one by definition
    OMutex::acquire();
    mnCount = 1;
    mnThreadId = aCurrentThread;
    OMutex::release();

    return sal_True;
}

int GtkYieldMutex::Grab()
{
    // this MUST only be called by gdk/gtk callbacks:
    // they are entered with gdk mutex locked; the mutex
    // was unlocked by GtkYieldMutex befor yielding which
    // is now locked again by gtk implicitly

    // obtained gdk mutex, now lock count is one by definition
    OMutex::acquire();
    int nRet = mnCount;
    if( mnCount == 0 ) // recursive else
        mnThreadId = vos::OThread::getCurrentIdentifier();
#if OSL_DEBUG_LEVEL > 1
    else if( mnThreadId != vos::OThread::getCurrentIdentifier() )
    {
        fprintf( stderr, "Yield mutex grabbed in different thread !\n" );
        abort();
    }
#endif
    mnCount = 1;
    OMutex::release();
    return nRet;
}

void GtkYieldMutex::Ungrab( int nGrabs )
{
    // this MUST only be called when leaving the callback
    // that locked the mutex with Grab()
    OMutex::acquire();
    mnCount = nGrabs;
    if( mnCount == 0 )
        mnThreadId = 0;
    OMutex::release();
}
