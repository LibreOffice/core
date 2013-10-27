/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#define _TOOLS_TOOLSIN_CXX

#include <string.h>
#include <tools/shl.hxx>
#include <tools/debug.hxx>
#include <toolsin.hxx>

#if defined WNT
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
#ifdef OS2_YD_REMOVEME
    ImpDeInitOS2Tools();
#endif
}
