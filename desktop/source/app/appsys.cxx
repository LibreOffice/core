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
#include "precompiled_desktop.hxx"

#include "appsys.hxx"

#ifdef WNT

#include <rtl/ustring.hxx>
#include <tools/solar.h>
#include <tools/prewin.h>
#include <winreg.h>
#include <tools/postwin.h>
#include <tools/urlobj.hxx>


#define U2S(STRING)     ::rtl::OUStringToOString(STRING, RTL_TEXTENCODING_UTF8)

namespace desktop
{

void FATToVFat_Impl( String& aName )
{
    INetURLObject aObj( aName );
    if ( aObj.GetProtocol() == INET_PROT_FILE )
    {
        WIN32_FIND_DATA aData;
        HANDLE h = FindFirstFile( U2S(aName).getStr(), &aData );
        if ( h )
        {
            // FAT-Kurzname in VFAT-Langname wandeln
            aObj.removeSegment();
            aObj.insertName( String::CreateFromAscii( aData.cFileName ) );
            aName = aObj.PathToFileName();
            FindClose( h );
        }
    }
}

}

#endif // WNT

