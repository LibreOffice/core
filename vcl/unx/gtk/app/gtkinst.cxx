/*************************************************************************
 *
 *  $RCSfile: gtkinst.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-02 14:53:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <osl/module.h>
#include <plugins/gtk/gtkdata.hxx>
#include <plugins/gtk/gtkinst.hxx>
#include <salframe.h>
#include <plugins/gtk/gtkframe.hxx>
#include <plugins/gtk/gtkobject.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <cstdio>
#endif

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
    #define GET_YIELD_MUTEX() static_cast<GtkHookedYieldMutex*>(GetSalData()->pInstance_->GetYieldMutex())
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
                (GdkLockFn) osl_getSymbol( pModule, aSymbolName.pData );
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

    SalInstance* create_SalInstance( oslModule pModule )
    {
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
        SalData *pSalData = new GtkData();
        SetSalData( pSalData );
        pSalData->pInstance_ = pInstance;
        pSalData->Init();
        pSalData->initNWF();

        return pInstance;
    }
}

GtkInstance::~GtkInstance()
{
}

SalFrame* GtkInstance::CreateFrame( SalFrame* pParent, ULONG nStyle )
{
    return new GtkSalFrame( pParent, nStyle );
}

SalObject* GtkInstance::CreateObject( SalFrame* pParent )
{
    return new GtkSalObject( static_cast<GtkSalFrame*>(pParent) );
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
