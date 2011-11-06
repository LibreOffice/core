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


#ifndef _SVT_LOGHELPER_HXX
#define _SVT_LOGHELPER_HXX

namespace css = ::com::sun::star;

class LogHelper
{
public:
    LogHelper();
    ~LogHelper();

    static void logIt(const css::uno::Exception&){}
};

inline void logIt(const css::uno::Exception& ex)
{
    ::rtl::OUStringBuffer sMsg(256);
    sMsg.appendAscii("Unexpected exception catched. Original message was:\n\""      );
    sMsg.append(ex.Message);
    sMsg.appendAscii("\"");
    OSL_ENSURE(sal_False, ::rtl::OUStringToOString(sMsg.makeStringAndClear(), RTL_TEXTENCODING_UTF8).getStr());
}

#endif

