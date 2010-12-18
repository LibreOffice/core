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
#include "precompiled_tools.hxx"

#define _TOOLS_TOOLSIN_CXX

#include <string.h>
#include <tools/shl.hxx>
#include <tools/debug.hxx>
#include <toolsin.hxx>

#if defined WNT || defined OS2
#include <dll.hxx>
#endif

void ImplDeleteCharTabData();

// =======================================================================

TOOLSINDATA* ImplGetToolsInData()
{
    TOOLSINDATA** ppData = (TOOLSINDATA**)GetAppData( SHL_TOOLS );
    if ( !(*ppData) )
    {
        TOOLSINDATA* pData = new TOOLSINDATA;
        memset( pData, 0, sizeof( TOOLSINDATA ) );
        *ppData = pData;
    }

    return *ppData;
}

// =======================================================================

void InitTools()
{
    DBG_DEBUGSTART();
}

// -----------------------------------------------------------------------

void DeInitTools()
{
    TOOLSINDATA**   ppData = (TOOLSINDATA**)GetAppData( SHL_TOOLS );
    TOOLSINDATA*    pData = *ppData;

    if ( pData )
    {
        ImplDeleteCharTabData();
        delete pData;
        *ppData = NULL;
    }

    DBG_DEBUGEND();
}

// -----------------------------------------------------------------------

void GlobalDeInitTools()
{
    DBG_GLOBALDEBUGEND();

#if defined WNT
    ImpDeInitWinTools();
#endif
#ifdef OS2
    ImpDeInitOS2Tools();
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
