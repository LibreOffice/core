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
#include "precompiled_sal.hxx"

#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>

const sal_Char *dbg_dump(const rtl::OString &rStr)
{
    static rtl::OStringBuffer aStr;

    aStr = rtl::OStringBuffer(rStr);
    aStr.append(static_cast<char>(0));
    return aStr.getStr();
}

const sal_Char *dbg_dump(const rtl::OUString &rStr)
{
    return dbg_dump(rtl::OUStringToOString(rStr, RTL_TEXTENCODING_UTF8));
}

const sal_Char *dbg_dump(rtl_String *pStr)
{
    return dbg_dump(rtl::OString(pStr));
}

const sal_Char *dbg_dump(rtl_uString *pStr)
{
    return dbg_dump(rtl::OUString(pStr));
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
