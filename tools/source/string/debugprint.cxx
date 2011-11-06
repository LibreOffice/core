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

// =======================================================================

#include <tools/string.hxx>

const sal_Char *dbg_dump(const ByteString &rStr)
{
    static ByteString aStr;
    aStr = rStr;
    aStr.Append(static_cast<char>(0));
    return aStr.GetBuffer();
}

const sal_Char *dbg_dump(const UniString &rStr)
{
    return dbg_dump(ByteString(rStr, RTL_TEXTENCODING_UTF8));
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
