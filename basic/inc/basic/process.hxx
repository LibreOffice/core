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

#ifndef _PROCESS_HXX
#define _PROCESS_HXX

#include <tools/string.hxx>
#include <osl/process.h>

#include <map>

typedef std::map< String, String > Environment;
typedef Environment::value_type EnvironmentVariable;

class Process
{
    // Internal members and methods
    sal_uInt32                  m_nArgumentCount;
    rtl_uString               **m_pArgumentList;
    sal_uInt32                  m_nEnvCount;
    rtl_uString               **m_pEnvList;
    rtl::OUString               m_aProcessName;
    oslProcess                  m_pProcess;
    sal_Bool ImplIsRunning();
    long ImplGetExitCode();
    sal_Bool bWasGPF;
    sal_Bool bHasBeenStarted;

public:
    Process();
    ~Process();
    // Methoden
    void SetImage( const String &aAppPath, const String &aAppParams, const Environment *pEnv = NULL );
    sal_Bool Start();
    sal_uIntPtr GetExitCode();
    sal_Bool IsRunning();
    sal_Bool WasGPF();

    sal_Bool Terminate();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
