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



{
    HKEY    hkeyLogon;
    HKEY    hkeyWorkgroup;
    DWORD   dwResult = 0;


    if ( ERROR_SUCCESS  == RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        TEXT("Network\\Logon"),
        0, KEY_READ, &hkeyLogon ) )
    {
        DWORD   dwLogon = 0;
        DWORD   dwLogonSize = sizeof(dwLogon);
        RegQueryValueEx( hkeyLogon, TEXT("LMLogon"), 0, NULL, (LPBYTE)&dwLogon, &dwLogonSize );
        RegCloseKey( hkeyLogon );

        if ( dwLogon )
        {
            HKEY    hkeyNetworkProvider;

            if ( ERROR_SUCCESS  == RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                TEXT("SYSTEM\\CurrentControlSet\\Services\\MSNP32\\NetworkProvider"),
                0, KEY_READ, &hkeyNetworkProvider ) )
            {
                DWORD   dwBufferSize = nSize;
                LONG    lResult = RegQueryValueEx( hkeyNetworkProvider, TEXT("AuthenticatingAgent"), 0, NULL, (LPBYTE)lpBuffer, &dwBufferSize );

                if ( ERROR_SUCCESS == lResult || ERROR_MORE_DATA == lResult )
                    dwResult = dwBufferSize / sizeof(TCHAR);

                RegCloseKey( hkeyNetworkProvider );
            }
        }
    }
    else if ( ERROR_SUCCESS == RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        TEXT("SYSTEM\\CurrentControlSet\\Services\\VxD\\VNETSUP"),
        0, KEY_READ, &hkeyWorkgroup ) )
    {
        DWORD   dwBufferSize = nSize;
        LONG    lResult = RegQueryValueEx( hkeyWorkgroup, TEXT("Workgroup"), 0, NULL, (LPBYTE)lpBuffer, &dwBufferSize );

        if ( ERROR_SUCCESS == lResult || ERROR_MORE_DATA == lResult )
            dwResult = dwBufferSize / sizeof(TCHAR);

        RegCloseKey( hkeyWorkgroup );
    }


    return dwResult;
}
