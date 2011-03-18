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

#ifndef _SV_DBGGUI_HXX
#define _SV_DBGGUI_HXX

#include <vcl/sv.h>

// ------------
// - DBG_UITL -
// ------------

#ifdef DBG_UTIL

class Window;
class XubString;

void DbgGUIInit();
void DbgGUIDeInit();
void DbgGUIStart();
void DbgDialogTest( Window* pWindow );

/** registers a named user-defined channel for emitting the diagnostic messages
    @return
        a unique number for this channel, which can be used for ->DbgData::nErrorOut,
        ->DbgData::nWarningOut and ->DbgData::nTraceOut
    @see DBG_OUT_USER_CHANNEL_0
*/
sal_uInt16 DbgRegisterNamedUserChannel( const XubString& _rChannelUIName, DbgPrintLine pProc );

#define DBGGUI_INIT()           DbgGUIInit()
#define DBGGUI_DEINIT()         DbgGUIDeInit()
#define DBGGUI_START()          DbgGUIStart()

#define DBG_DIALOGTEST( pWindow )                   \
    if ( DbgIsDialog() )                            \
        DbgDialogTest( pWindow );

#else


#define DBGGUI_INIT()
#define DBGGUI_DEINIT()
#define DBGGUI_START()

#define DBG_DIALOGTEST( pWindow )

#endif

#endif  // _SV_DBGGUI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
