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



#include "stringconverter.hxx"

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>

/** Convert a Unicode string to an ANSI string based on CP_ACP
*/
std::string UnicodeToAnsiString(const std::wstring& UniString)
{
    int len = WideCharToMultiByte(
        CP_ACP, 0, UniString.c_str(), -1, 0, 0, 0, 0);

    char* buff = reinterpret_cast<char*>(_alloca(len));

    WideCharToMultiByte(
        CP_ACP, 0, UniString.c_str(), -1, buff, len, 0, 0);

    return std::string(buff);
}

/** Convert an ANSI string to unicode based on CP_ACP
*/
std::wstring AnsiToUnicodeString(const std::string& AnsiString)
{
    int len = MultiByteToWideChar(
        CP_ACP, 0, AnsiString.c_str(), -1, 0, 0);

    wchar_t* buff = reinterpret_cast<wchar_t*>(_alloca(len * sizeof(wchar_t)));

    MultiByteToWideChar(
        CP_ACP, 0, AnsiString.c_str(), -1, buff, len);

    return std::wstring(buff);
}


