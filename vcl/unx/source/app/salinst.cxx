/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <osl/module.hxx>

#include "salunx.h"

#include "saldata.hxx"
#include "saldisp.hxx"
#include "salinst.h"
#include "salframe.h"
#include "dtint.hxx"
#include "salprn.h"
#include "sm.hxx"

#include "vcl/salwtype.hxx"
#include "vcl/apptypes.hxx"
#include "vcl/helper.hxx"
#include <tools/solarmutex.hxx>
#include "osl/mutex.hxx"
#include <sal/macros.h>

// -------------------------------------------------------------------------
//
// SalYieldMutex
//
// -------------------------------------------------------------------------

SalYieldMutex::SalYieldMutex()
{
    mnCount     = 0;
    mnThreadId  = 0;
    ::tools::SolarMutex::SetSolarMutex( this );
}

void SalYieldMutex::acquire()
{
    SolarMutexObject::acquire();
    mnThreadId = osl::Thread::getCurrentIdentifier();
    mnCount++;
}

void SalYieldMutex::release()
{
    if ( mnThreadId == osl::Thread::getCurrentIdentifier() )
    {
        if ( mnCount == 1 )
            mnThreadId = 0;
        mnCount--;
    }
    SolarMutexObject::release();
}

sal_Bool SalYieldMutex::tryToAcquire()
{
    if ( SolarMutexObject::tryToAcquire() )
    {
        mnThreadId = osl::Thread::getCurrentIdentifier();
        mnCount++;
        return True;
    }
    else
        return False;
}

//----------------------------------------------------------------------------

// -=-= SalInstance =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// plugin factory function
extern "C"
{
    VCL_DLLPUBLIC SalInstance* create_SalInstance()
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

        X11SalInstance* pInstance = new X11SalInstance( new SalYieldMutex() );

        // initialize SalData
        X11SalData *pSalData = new X11SalData;
        SetSalData( pSalData );
        pSalData->m_pInstance = pInstance;
        pSalData->Init();

        return pInstance;
    }
}

X11SalInstance::~X11SalInstance()
{
    // close session management
    SessionManagerClient::close();

    // dispose SalDisplay list from SalData
    // would be done in a static destructor else which is
    // a little late

    X11SalData *pSalData = GetX11SalData();
    pSalData->deInitNWF();
    delete pSalData;
    SetSalData( NULL );

    ::tools::SolarMutex::SetSolarMutex( 0 );
      delete mpSalYieldMutex;
}


// --------------------------------------------------------
// AnyInput from sv/mow/source/app/svapp.cxx

struct PredicateReturn
{
    sal_uInt16  nType;
    sal_Bool    bRet;
};

extern "C" {
Bool ImplPredicateEvent( Display *, XEvent *pEvent, char *pData )
{
    PredicateReturn *pPre = (PredicateReturn *)pData;

    if ( pPre->bRet )
        return False;

    sal_uInt16 nType;

    switch( pEvent->type )
    {
        case ButtonPress:
        case ButtonRelease:
        case MotionNotify:
        case EnterNotify:
        case LeaveNotify:
            nType = INPUT_MOUSE;
            break;

        case XLIB_KeyPress:
        //case KeyRelease:
            nType = INPUT_KEYBOARD;
            break;
        case Expose:
        case GraphicsExpose:
        case NoExpose:
            nType = INPUT_PAINT;
            break;
        default:
            nType = 0;
    }

    if ( (nType & pPre->nType) || ( ! nType && (pPre->nType & INPUT_OTHER) ) )
        pPre->bRet = sal_True;

    return False;
}
}

bool X11SalInstance::AnyInput(sal_uInt16 nType)
{
    X11SalData *pSalData = GetX11SalData();
    Display *pDisplay  = pSalData->GetDisplay()->GetDisplay();
    sal_Bool bRet = sal_False;

    if( (nType & INPUT_TIMER) &&
        pSalData->GetDisplay()->GetXLib()->CheckTimeout( false ) )
    {
        bRet = sal_True;
    }
    else if (XPending(pDisplay) )
    {
        PredicateReturn aInput;
        XEvent          aEvent;

        aInput.bRet     = sal_False;
        aInput.nType    = nType;

        XCheckIfEvent(pDisplay, &aEvent, ImplPredicateEvent,
                      (char *)&aInput );

        bRet = aInput.bRet;
    }
    return bRet;
}

osl::SolarMutex* X11SalInstance::GetYieldMutex()
{
    return mpSalYieldMutex;
}

// -----------------------------------------------------------------------

sal_uLong X11SalInstance::ReleaseYieldMutex()
{
    SalYieldMutex* pYieldMutex = mpSalYieldMutex;
    if ( pYieldMutex->GetThreadId() ==
         osl::Thread::getCurrentIdentifier() )
    {
        sal_uLong nCount = pYieldMutex->GetAcquireCount();
        sal_uLong n = nCount;
        while ( n )
        {
            pYieldMutex->release();
            n--;
        }

        return nCount;
    }
    else
        return 0;
}

// -----------------------------------------------------------------------

void X11SalInstance::AcquireYieldMutex( sal_uLong nCount )
{
    SalYieldMutex* pYieldMutex = mpSalYieldMutex;
    while ( nCount )
    {
        pYieldMutex->acquire();
        nCount--;
    }
}

// -----------------------------------------------------------------------

bool X11SalInstance::CheckYieldMutex()
{
    bool bRet = true;

    SalYieldMutex* pYieldMutex = mpSalYieldMutex;
    if ( pYieldMutex->GetThreadId() != osl::Thread::getCurrentIdentifier() )
    {
        bRet = false;
    }

    return bRet;
}

// -----------------------------------------------------------------------

void X11SalInstance::Yield( bool bWait, bool bHandleAllCurrentEvents )
{ GetX11SalData()->GetLib()->Yield( bWait, bHandleAllCurrentEvents ); }

void* X11SalInstance::GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes )
{
    static const char* pDisplay = getenv( "DISPLAY" );
    rReturnedType   = AsciiCString;
    rReturnedBytes  = pDisplay ? strlen( pDisplay )+1 : 1;
    return pDisplay ? (void*)pDisplay : (void*)"";
}

SalFrame *X11SalInstance::CreateFrame( SalFrame *pParent, sal_uLong nSalFrameStyle )
{
    SalFrame *pFrame = new X11SalFrame( pParent, nSalFrameStyle );

    return pFrame;
}

SalFrame* X11SalInstance::CreateChildFrame( SystemParentData* pParentData, sal_uLong nStyle )
{
    SalFrame* pFrame = new X11SalFrame( NULL, nStyle, pParentData );

    return pFrame;
}

void X11SalInstance::DestroyFrame( SalFrame* pFrame )
{
    delete pFrame;
}

static void getServerDirectories( std::list< rtl::OString >& o_rFontPaths )
{
#ifdef LINUX
    /*
     *  chkfontpath exists on some (RH derived) Linux distributions
     */
    static const char* pCommands[] = {
        "/usr/sbin/chkfontpath 2>/dev/null", "chkfontpath 2>/dev/null"
    };
    ::std::list< ByteString > aLines;

    for( unsigned int i = 0; i < SAL_N_ELEMENTS(pCommands); i++ )
    {
        FILE* pPipe = popen( pCommands[i], "r" );
        aLines.clear();
        if( pPipe )
        {
            char line[1024];
            char* pSearch;
            while( fgets( line, sizeof(line), pPipe ) )
            {
                int nLen = strlen( line );
                if( line[nLen-1] == '\n' )
                    line[nLen-1] = 0;
                pSearch = strstr( line, ": " );
                if( pSearch )
                    aLines.push_back( pSearch+2 );
            }
            if( ! pclose( pPipe ) )
                break;
        }
    }

    for( ::std::list< ByteString >::iterator it = aLines.begin(); it != aLines.end(); ++it )
    {
        if( ! access( it->GetBuffer(), F_OK ) )
        {
            o_rFontPaths.push_back( *it );
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "adding fs dir %s\n", it->GetBuffer() );
#endif
        }
    }
#else
    (void)o_rFontPaths;
#endif
}



void X11SalInstance::FillFontPathList( std::list< rtl::OString >& o_rFontPaths )
{
    Display *pDisplay = GetX11SalData()->GetDisplay()->GetDisplay();

    DBG_ASSERT( pDisplay, "No Display !" );
    if( pDisplay )
    {
        // get font paths to look for fonts
        int nPaths = 0, i;
        char** pPaths = XGetFontPath( pDisplay, &nPaths );

        bool bServerDirs = false;
        for( i = 0; i < nPaths; i++ )
        {
            OString aPath( pPaths[i] );
            sal_Int32 nPos = 0;
            if( ! bServerDirs
                && ( nPos = aPath.indexOf( ':' ) ) > 0
                && ( !aPath.copy(nPos).equals( ":unscaled" ) ) )
            {
                bServerDirs = true;
                getServerDirectories( o_rFontPaths );
            }
            else
            {
                psp::normPath( aPath );
                o_rFontPaths.push_back( aPath );
            }
        }

        if( nPaths )
            XFreeFontPath( pPaths );
    }

    // insert some standard directories
    o_rFontPaths.push_back( "/usr/openwin/lib/X11/fonts/TrueType" );
    o_rFontPaths.push_back( "/usr/openwin/lib/X11/fonts/Type1" );
    o_rFontPaths.push_back( "/usr/openwin/lib/X11/fonts/Type1/sun" );
    o_rFontPaths.push_back( "/usr/X11R6/lib/X11/fonts/truetype" );
    o_rFontPaths.push_back( "/usr/X11R6/lib/X11/fonts/Type1" );

    #ifdef SOLARIS
    /* cde specials, from /usr/dt/bin/Xsession: here are the good fonts,
    the OWfontpath file may contain as well multiple lines as a comma
    separated list of fonts in each line. to make it even more weird
    environment variables are allowed as well */

    const char* lang = getenv("LANG");
    if ( lang != NULL )
    {
        String aOpenWinDir( String::CreateFromAscii( "/usr/openwin/lib/locale/" ) );
        aOpenWinDir.AppendAscii( lang );
        aOpenWinDir.AppendAscii( "/OWfontpath" );

        SvFileStream aStream( aOpenWinDir, STREAM_READ );

        // TODO: replace environment variables
        while( aStream.IsOpen() && ! aStream.IsEof() )
        {
            ByteString aLine;
            aStream.ReadLine( aLine );
            // need an OString for normpath
            OString aNLine( aLine );
            psp::normPath( aNLine );
            aLine = aNLine;
            // try to avoid bad fonts in some cases
            static bool bAvoid = (strncasecmp( lang, "ar", 2 ) == 0) || (strncasecmp( lang, "he", 2 ) == 0) || strncasecmp( lang, "iw", 2 ) == 0 || (strncasecmp( lang, "hi", 2 ) == 0);
            if( bAvoid && aLine.Search( "iso_8859" ) != STRING_NOTFOUND )
                continue;
            o_rFontPaths.push_back( aLine );
        }
    }
    #endif /* SOLARIS */
}

extern "C" { static void SAL_CALL thisModule() {} }

void X11SalInstance::AddToRecentDocumentList(const rtl::OUString& rFileUrl, const rtl::OUString& rMimeType)
{
    const rtl::OUString SYM_ADD_TO_RECENTLY_USED_FILE_LIST(RTL_CONSTASCII_USTRINGPARAM("add_to_recently_used_file_list"));
    const rtl::OUString LIB_RECENT_FILE(RTL_CONSTASCII_USTRINGPARAM("librecentfile.so"));
    typedef void (*PFUNC_ADD_TO_RECENTLY_USED_LIST)(const rtl::OUString&, const rtl::OUString&);

    PFUNC_ADD_TO_RECENTLY_USED_LIST add_to_recently_used_file_list = 0;

    osl::Module module;
    module.loadRelative( &thisModule, LIB_RECENT_FILE );
    if (module.is())
        add_to_recently_used_file_list = (PFUNC_ADD_TO_RECENTLY_USED_LIST)module.getFunctionSymbol(SYM_ADD_TO_RECENTLY_USED_FILE_LIST);
    if (add_to_recently_used_file_list)
        add_to_recently_used_file_list(rFileUrl, rMimeType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
