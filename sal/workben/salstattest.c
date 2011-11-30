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



#include <stdio.h>
#include <systools/win32/shell9x.h>
#include <systools/win32/kernel9x.h>

//#include <windows.h>
//#include <shlobj.h>

int _cdecl main( int argc, char * argv[] )
{
    BROWSEINFOW bi;
    wchar_t     buff[260];
    DWORD       dwRet;

    bi.hwndOwner = NULL;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = buff;
    bi.lpszTitle = L"Test";
    bi.ulFlags = 0;
    bi.lpfn = NULL;
    bi.lParam = 0;

    SHBrowseForFolderW( &bi );

    dwRet = GetLongPathNameW( L"c:\\", buff, 260 );

    return(0);
}


