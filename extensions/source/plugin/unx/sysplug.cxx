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
#include "precompiled_extensions.hxx"

#ifdef AIX
#define _LINUX_SOURCE_COMPAT
#include <sys/timer.h>
#undef _LINUX_SOURCE_COMPAT
#endif

#include <cstdarg>

#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <rtl/bootstrap.hxx>

#include <plugin/impl.hxx>

int UnxPluginComm::nConnCounter = 0;

UnxPluginComm::UnxPluginComm(
                             const String& /*mimetype*/,
                             const String& library,
                             XLIB_Window aParent,
                             int nDescriptor1,
                             int nDescriptor2
                             ) :
        PluginComm( ::rtl::OUStringToOString( library, osl_getThreadTextEncoding() ), false ),
        PluginConnector( nDescriptor2 )
{
    char pDesc[32];
    char pWindow[32];
    sprintf( pWindow, "%d", (int)aParent );
    sprintf( pDesc, "%d", nDescriptor1 );
    ByteString aLib( library, osl_getThreadTextEncoding() );
    rtl::OString path;
    if (!getPluginappPath(&path)) {
        fprintf( stderr, "cannot construct path to pluginapp.bin\n" );
        m_nCommPID = -1;
        return;
    }

    char const* pArgs[5];
    pArgs[0] = path.getStr();
    pArgs[1] = pDesc;
    pArgs[2] = aLib.GetBuffer();
    pArgs[3] = pWindow;
    pArgs[4] = NULL;

#if OSL_DEBUG_LEVEL > 1
    m_nCommPID = 10;
    fprintf( stderr, "Try to launch: %s %s %s %s, descriptors are %d, %d\n", pArgs[0], pArgs[1], pArgs[2], pArgs[3], nDescriptor1, nDescriptor2 );
#endif

    if( ! ( m_nCommPID = fork() ) )
      {
         execvp( pArgs[0], const_cast< char ** >(pArgs) );
          fprintf( stderr, "Error: could not exec %s\n", pArgs[0] );
          _exit(255);
      }

    if( m_nCommPID != -1 )
    {
        // wait for pluginapp.bin to start up
        if( ! WaitForMessage( 5000 ) )
        {
            fprintf( stderr, "Timeout on command: %s %s %s %s\n", pArgs[0], pArgs[1], pArgs[2], pArgs[3] );
            invalidate();
        }
        else
        {
            MediatorMessage* pMessage = GetNextMessage( sal_True );
            Respond( pMessage->m_nID,
                     const_cast<char*>("init ack"),8,
                     NULL );
            delete pMessage;
            NPP_Initialize();
        }
    }
}

UnxPluginComm::~UnxPluginComm()
{
    NPP_Shutdown();
    if( m_nCommPID != -1 && m_nCommPID != 0 )
    {
        int status = 16777216;
        pid_t nExit = waitpid( m_nCommPID, &status, WUNTRACED );
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "child %d (plugin app child %d) exited with status %d\n", (int)nExit, (int)m_nCommPID, (int)WEXITSTATUS(status) );
#else
        (void)nExit;
#endif
    }
}

bool UnxPluginComm::getPluginappPath(rtl::OString * path) {
    OSL_ASSERT(path != NULL);
    rtl::OUString p(
        RTL_CONSTASCII_USTRINGPARAM("$OOO_BASE_DIR/program/pluginapp.bin"));
    rtl::Bootstrap::expandMacros(p);
    return
        (osl::FileBase::getSystemPathFromFileURL(p, p) ==
         osl::FileBase::E_None) &&
        p.convertToString(
            path, osl_getThreadTextEncoding(),
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
             RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
