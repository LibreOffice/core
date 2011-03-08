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

#ifndef _OSL_SOCKETIMPL_H_
#define _OSL_SOCKETIMPL_H_

#include <osl/pipe.h>
#include <osl/socket.h>
#include <osl/interlck.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*oslCloseCallback) (void*);

struct oslSocketImpl {
    int                 m_Socket;
    int                 m_nLastError;
    oslCloseCallback    m_CloseCallback;
    void*               m_CallbackArg;
    oslInterlockedCount m_nRefCount;
#if defined(LINUX)
    sal_Bool            m_bIsAccepting;
    sal_Bool            m_bIsInShutdown;
#endif
};

struct oslSocketAddrImpl
{
    sal_Int32 m_nRefCount;
    struct sockaddr m_sockaddr;
};

/*
struct oslPipeImpl {
    int  m_Socket;
    sal_Char m_Name[PATH_MAX + 1];
    oslInterlockedCount m_nRefCount;
    sal_Bool m_bClosed;
#if defined(LINUX)
    sal_Bool m_bIsAccepting;
    sal_Bool m_bIsInShutdown;
#endif
};
*/

oslSocket __osl_createSocketImpl(int Socket);
void __osl_destroySocketImpl(oslSocket pImpl);

#ifdef __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
