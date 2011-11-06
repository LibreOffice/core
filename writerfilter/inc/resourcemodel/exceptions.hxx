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



#ifndef _DOCTOK_EXCEPTIONS_HXX
#define _DOCTOK_EXCEPTIONS_HXX

#include <string>
#include <WriterFilterDllApi.hxx>

namespace writerfilter {
using namespace ::std;

class WRITERFILTER_DLLPUBLIC Exception
{
    string mText;

public:
    Exception(string text) : mText(text) {}

    const string & getText() const { return mText; }
};

class WRITERFILTER_DLLPUBLIC ExceptionNotFound : public Exception
{
public:
    ExceptionNotFound(string text) : Exception(text) {}
};

class WRITERFILTER_DLLPUBLIC ExceptionOutOfBounds : public Exception
{
public:
    ExceptionOutOfBounds(string text) : Exception(text) {}
};
}

#endif // _DOCTOK_EXCEPTIONS_HXX
