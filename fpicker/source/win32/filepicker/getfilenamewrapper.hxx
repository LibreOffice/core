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



#ifndef _GETFILENAMEWRAPPER_HXX_
#define _GETFILENAMEWRAPPER_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <sal/types.h>

#define WIN32_LEAN_AND_MEAN
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#include <commdlg.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

/*
    A simple wrapper around the GetOpenFileName/GetSaveFileName API.
    Because currently the Win32 API functions GetOpenFileName/GetSaveFileName
    work only properly in an Single Threaded Appartment.
*/

class CGetFileNameWrapper
{
public:
    CGetFileNameWrapper();

    bool getOpenFileName(LPOPENFILENAME lpofn);
    bool getSaveFileName(LPOPENFILENAME lpofn);
    int  commDlgExtendedError();

private:
    int m_ExtendedDialogError;
};

#endif
