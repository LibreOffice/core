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


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "osl/module.hxx"

#include "unx/salunx.h"
#include "unx/saldata.hxx"
#include "unx/saldisp.hxx"
#include "generic/geninst.h"
#include "generic/genpspgraphics.h"
#include "unx/salframe.h"
#include "generic/genprn.h"
#include "unx/sm.hxx"

#include "vcl/apptypes.hxx"
#include "vcl/helper.hxx"

#include "salwtype.hxx"
#include <sal/macros.h>

// plugin factory function
extern "C"
{
    VCLPLUG_GEN_PUBLIC SalInstance* create_SalInstance()
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
        X11SalData *pSalData = new X11SalData( SAL_DATA_UNX, pInstance );

        pSalData->Init();
        pInstance->SetLib( pSalData->GetLib() );

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
    GetGenericData()->Dispose();
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
            nType = VCL_INPUT_MOUSE;
            break;

        case XLIB_KeyPress:
        //case KeyRelease:
            nType = VCL_INPUT_KEYBOARD;
            break;
        case Expose:
        case GraphicsExpose:
        case NoExpose:
            nType = VCL_INPUT_PAINT;
            break;
        default:
            nType = 0;
    }

    if ( (nType & pPre->nType) || ( ! nType && (pPre->nType & VCL_INPUT_OTHER) ) )
        pPre->bRet = sal_True;

    return False;
}
}

bool X11SalInstance::AnyInput(sal_uInt16 nType)
{
    SalGenericData *pData = GetGenericData();
    Display *pDisplay  = pData->GetSalDisplay()->GetDisplay();
    sal_Bool bRet = sal_False;

    if( (nType & VCL_INPUT_TIMER) && (mpXLib && mpXLib->CheckTimeout(false)) )
        bRet = sal_True;
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
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "AnyInput 0x%x = %s\n", nType, bRet ? "true" : "false" );
#endif
    return bRet;
}

void X11SalInstance::Yield( bool bWait, bool bHandleAllCurrentEvents )
{
    mpXLib->Yield( bWait, bHandleAllCurrentEvents );
}

void* X11SalInstance::GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType,
                                               int& rReturnedBytes )
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

static void getServerDirectories( std::list< OString >& o_rFontPaths )
{
#ifdef LINUX
    /*
     *  chkfontpath exists on some (RH derived) Linux distributions
     */
    static const char* pCommands[] = {
        "/usr/sbin/chkfontpath 2>/dev/null", "chkfontpath 2>/dev/null"
    };
    ::std::list< OString > aLines;

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

    for( ::std::list< OString >::iterator it = aLines.begin(); it != aLines.end(); ++it )
    {
        if( ! access( it->getStr(), F_OK ) )
        {
            o_rFontPaths.push_back( *it );
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "adding fs dir %s\n", it->getStr() );
#endif
        }
    }
#else
    (void)o_rFontPaths;
#endif
}



void X11SalInstance::FillFontPathList( std::list< OString >& o_rFontPaths )
{
    Display *pDisplay = GetGenericData()->GetSalDisplay()->GetDisplay();

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
        OUString aOpenWinDir( "/usr/openwin/lib/locale/" );
        aOpenWinDir += OUString::createFromAscii( lang );
        aOpenWinDir += "/OWfontpath";

        SvFileStream aStream( aOpenWinDir, STREAM_READ );

        // TODO: replace environment variables
        while( aStream.IsOpen() && ! aStream.IsEof() )
        {
            OString aLine;
            aStream.ReadLine( aLine );
            psp::normPath( aLine );
            // try to avoid bad fonts in some cases
            static bool bAvoid = (strncasecmp( lang, "ar", 2 ) == 0) || (strncasecmp( lang, "he", 2 ) == 0) || strncasecmp( lang, "iw", 2 ) == 0 || (strncasecmp( lang, "hi", 2 ) == 0);
            if( bAvoid && aLine.indexOfL(RTL_CONSTASCII_STRINGPARAM("iso_8859")) != -1 )
                continue;
            o_rFontPaths.push_back( aLine );
        }
    }
    #endif /* SOLARIS */
}

extern "C" { static void SAL_CALL thisModule() {} }

void X11SalInstance::AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService)
{
    const OUString SYM_ADD_TO_RECENTLY_USED_FILE_LIST("add_to_recently_used_file_list");
    const OUString LIB_RECENT_FILE("librecentfile.so");
    typedef void (*PFUNC_ADD_TO_RECENTLY_USED_LIST)(const OUString&, const OUString&, const OUString&);

    PFUNC_ADD_TO_RECENTLY_USED_LIST add_to_recently_used_file_list = 0;

    osl::Module module;
    module.loadRelative( &thisModule, LIB_RECENT_FILE );
    if (module.is())
        add_to_recently_used_file_list = (PFUNC_ADD_TO_RECENTLY_USED_LIST)module.getFunctionSymbol(SYM_ADD_TO_RECENTLY_USED_FILE_LIST);
    if (add_to_recently_used_file_list)
        add_to_recently_used_file_list(rFileUrl, rMimeType, rDocumentService);
}

void X11SalInstance::PostPrintersChanged()
{
    SalDisplay* pDisp = GetGenericData()->GetSalDisplay();
    const std::list< SalFrame* >& rList = pDisp->getFrames();
    for( std::list< SalFrame* >::const_iterator it = rList.begin();
         it != rList.end(); ++it )
        pDisp->SendInternalEvent( *it, NULL, SALEVENT_PRINTERCHANGED );
}

GenPspGraphics *X11SalInstance::CreatePrintGraphics()
{
    return new GenPspGraphics();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
