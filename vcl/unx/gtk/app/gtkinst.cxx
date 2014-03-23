/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <osl/module.h>
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <unx/salobj.h>
#include <unx/gtk/gtkframe.hxx>
#include <unx/gtk/gtkobject.hxx>
#include <unx/gtk/atkbridge.hxx>

#include <rtl/strbuf.hxx>

#include <rtl/uri.hxx>

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
        sal_uLong nCount = aYieldStack.front();
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
        mnThreadId != vos::OThread::getCurrentIdentifier())
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

        GdkLockFn gdk_threads_set_lock_functions =
                (GdkLockFn) osl_getAsciiFunctionSymbol( pModule, "gdk_threads_set_lock_functions" );
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

    VCLPLUG_GTK_PUBLIC SalInstance* create_SalInstance( oslModule pModule )
    {
        /* #i92121# workaround deadlocks in the X11 implementation
        */
        static const char* pNoXInitThreads = getenv( "SAL_NO_XINITTHREADS" );
        /* #i90094#
           from now on we know that an X connection will be
           established, so protect X against itself
        */
        if( ! ( pNoXInitThreads && *pNoXInitThreads ) )
            XInitThreads();

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

        InitAtkBridge();

        return pInstance;
    }
}

GtkInstance::~GtkInstance()
{
    DeInitAtkBridge();
}

SalFrame* GtkInstance::CreateFrame( SalFrame* pParent, sal_uLong nStyle )
{
    return new GtkSalFrame( pParent, nStyle );
}

SalFrame* GtkInstance::CreateChildFrame( SystemParentData* pParentData, sal_uLong )
{
    SalFrame* pFrame = new GtkSalFrame( pParentData );

    return pFrame;
}

SalObject* GtkInstance::CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, sal_Bool bShow )
{
    // there is no method to set a visual for a GtkWidget
    // so we need the X11SalObject in that case
    if( pWindowData )
        return X11SalObject::CreateObject( pParent, pWindowData, bShow );

    return new GtkSalObject( static_cast<GtkSalFrame*>(pParent), bShow );
}

extern "C"
{
    typedef void*(* getDefaultFnc)();
    typedef void(* addItemFnc)(void *, const char *);
}

void GtkInstance::AddToRecentDocumentList(const rtl::OUString& rFileUrl, const rtl::OUString& rMimeType)
{
    rtl::OString sGtkURL;
    rtl_TextEncoding aSystemEnc = osl_getThreadTextEncoding();
    if ((aSystemEnc == RTL_TEXTENCODING_UTF8) || (rFileUrl.compareToAscii( "file://", 7 ) !=  0))
        sGtkURL = rtl::OUStringToOString(rFileUrl, RTL_TEXTENCODING_UTF8);
    else
    {
        //Non-utf8 locales are a bad idea if trying to work with non-ascii filenames
        //Decode %XX components
        rtl::OUString sDecodedUri = Uri::decode(rFileUrl.copy(7), rtl_UriDecodeToIuri, RTL_TEXTENCODING_UTF8);
        //Convert back to system locale encoding
        rtl::OString sSystemUrl = rtl::OUStringToOString(sDecodedUri, aSystemEnc);
        //Encode to an escaped ASCII-encoded URI
        gchar *g_uri = g_filename_to_uri(sSystemUrl.getStr(), NULL, NULL);
        sGtkURL = rtl::OString(g_uri);
        g_free(g_uri);
    }
#if GTK_CHECK_VERSION(2,10,0)
    GtkRecentManager *manager = gtk_recent_manager_get_default ();
    gtk_recent_manager_add_item( manager, sGtkURL.getStr());
    (void)rMimeType;
#else
    static getDefaultFnc sym_gtk_recent_manager_get_default =
        (getDefaultFnc)osl_getAsciiFunctionSymbol( GetSalData()->m_pPlugin, "gtk_recent_manager_get_default" );

    static addItemFnc sym_gtk_recent_manager_add_item =
        (addItemFnc)osl_getAsciiFunctionSymbol( GetSalData()->m_pPlugin, "gtk_recent_manager_add_item");
    if (sym_gtk_recent_manager_get_default && sym_gtk_recent_manager_add_item)
        sym_gtk_recent_manager_add_item(sym_gtk_recent_manager_get_default(), sGtkURL);
    else
        X11SalInstance::AddToRecentDocumentList(rFileUrl, rMimeType);
#endif
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
