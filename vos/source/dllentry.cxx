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


#include <windows.h>
#include <winsock.h>


#ifdef SHARED

sal_Int32 CALLBACK LibMain(HINSTANCE hinst, WORD wDataSeg, WORD cbHeap, LPSTR lpszCmdLine )
{
    HGLOBAL    hgblClassStruct;
    LPWNDCLASS lpClassStruct;
    static HINSTANCE hinstLib = NULL;

    /* Has the library been initialized yet? */
    if (hinstLib == NULL)
    {
        WORD    wVersionRequested;
        WSADATA wsaData;
        sal_Int32       error;


        wVersionRequested = 0x0101;
        error = WSAStartup( wVersionRequested, &wsaData );
        if (error)
        {
        }
        else
            hinstLib = hinst;
    }

    return (hinstLib ? TRUE : FALSE);  /* return TRUE = success; FALSE = fail */
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
