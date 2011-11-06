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

