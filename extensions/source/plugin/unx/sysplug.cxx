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

#include <config_folders.h>

#ifdef AIX
#define _LINUX_SOURCE_COMPAT
#include <sys/timer.h>
#undef _LINUX_SOURCE_COMPAT
#endif

#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <rtl/bootstrap.hxx>

#include <plugin/impl.hxx>


::boost::shared_ptr<SysPlugData> CreateSysPlugData()
{
    return ::boost::shared_ptr<SysPlugData>();
}

UnxPluginComm::UnxPluginComm(
                             const String& /*mimetype*/,
                             const String& library,
                             XLIB_Window aParent,
                             int nDescriptor1,
                             int nDescriptor2
                             ) :
        PluginComm( OUStringToOString( library, osl_getThreadTextEncoding() ), false ),
        PluginConnector( nDescriptor2 ),
        m_nCommPID( 0 )
{
    OString path;
    if (!getPluginappPath(&path))
    {
        SAL_WARN("extensions.plugin", "cannot construct path to pluginapp.bin");
        return;
    }

    char pDesc[32];
    char pWindow[32];
    sprintf( pWindow, "%d", (int)aParent );
    sprintf( pDesc, "%d", nDescriptor1 );
    OString aLib(OUStringToOString(library, osl_getThreadTextEncoding()));

    char const* pArgs[5];
    pArgs[0] = path.getStr();
    pArgs[1] = pDesc;
    pArgs[2] = aLib.getStr();
    pArgs[3] = pWindow;
    pArgs[4] = NULL;

    SAL_INFO(
        "extensions.plugin",
        "try to launch: " << pArgs[0] << " " << pArgs[1] << " " << pArgs[2]
            << " " << pArgs[3] << ", descriptors are " << nDescriptor1 << ", "
            << nDescriptor2);

    pid_t pid = fork();
    if( pid == 0 )
    {
        execvp( pArgs[0], const_cast< char ** >(pArgs) );
        SAL_WARN("extensions.plugin", "could not exec " << pArgs[0]);
        _exit(255);
    }

    if( pid == -1 )
    {
        SAL_WARN("extensions.plugin", "fork failed");
        return;
    }

    m_nCommPID = pid;
    // wait for pluginapp.bin to start up
    if( ! WaitForMessage( 5000 ) )
    {
        SAL_WARN(
            "extensions.plugin",
            "timeout on command: " << pArgs[0] << " " << pArgs[1] << " "
                << pArgs[2] << " " << pArgs[3]);
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

UnxPluginComm::~UnxPluginComm()
{
    NPP_Shutdown();
    if( m_nCommPID != 0 )
    {
        int status = 16777216;
        pid_t nExit = waitpid( m_nCommPID, &status, WUNTRACED );
        SAL_INFO(
            "extensions.plugin",
            "child " << nExit << " (plugin app child " << m_nCommPID
                << ") exited with status " << WEXITSTATUS(status));
    }
}

bool UnxPluginComm::getPluginappPath(OString * path) {
    OSL_ASSERT(path != NULL);
    OUString p("$BRAND_BASE_DIR/" LIBO_LIBEXEC_FOLDER "/pluginapp.bin");
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
