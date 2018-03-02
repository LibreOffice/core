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
#include "precompiled_canvas.hxx"

#include "cairo_cairo.hxx"

#ifdef WNT
# include <tools/prewin.h>
# include <windows.h>
# include <tools/postwin.h>
#endif

#ifdef OS2
# include <svpm.h>
#endif

#include <vcl/sysdata.hxx>
#include <vcl/syschild.hxx>

namespace cairo
{
/****************************************************************************************
 * Platform independent part of surface backends for OpenOffice.org Cairo Canvas        *
 * For the rest of the functions (and the platform-specific derived                     *
 *  Surface classes), see platform specific cairo_<platform>_cairo.cxx                  *
 ****************************************************************************************/

    const SystemEnvData* GetSysData(const Window *pOutputWindow)
    {
        const SystemEnvData* pSysData = NULL;
        // check whether we're a SysChild: have to fetch system data
        // directly from SystemChildWindow, because the GetSystemData
        // method is unfortunately not virtual
        const SystemChildWindow* pSysChild = dynamic_cast< const SystemChildWindow* >( pOutputWindow );
        if( pSysChild )
            pSysData = pSysChild->GetSystemData();
        else
            pSysData = pOutputWindow->GetSystemData();
        return pSysData;
    }
}
