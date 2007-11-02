/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: toolsin.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 13:02:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#define _TOOLS_TOOLSIN_CXX

#include <string.h>

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _TOOLSIN_HXX
#include <toolsin.hxx>
#endif

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
