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



#include <precomp.h>
#include <cosv/persist.hxx>

// NOT FULLY DECLARED SERVICES
#include <cosv/streamstr.hxx>
#include <cosv/ploc.hxx>


#ifdef WNT
#include <io.h>

namespace csv
{
namespace ploc
{

bool
Persistent::Exists() const
{
    return access( StrPath(), 00) == 0;
}

} // namespace ploc
} // namespace csv


#elif defined(UNX)
#include <unistd.h>

/*
#ifndef __SUNPRO_CC
#include <unistd.h>
#else
#define F_OK    0   // Test for existence of File
extern int access(const char *, int);
#endif
*/

namespace csv
{
namespace ploc
{

bool
Persistent::Exists() const
{
    return access( StrPath(), F_OK ) == 0;
}


} // namespace ploc
} // namespace csv

#else
#error  For using csv::ploc there has to be defined: WNT or UNX.
#endif

namespace csv
{
namespace ploc
{

const char *
Persistent::StrPath() const
{
    if (sPath.empty() )
    {
#ifndef CSV_NO_MUTABLE
        StreamStr & rsPath = sPath;
#else
        StreamStr & rsPath = const_cast< StreamStr& >(sPath);
#endif
        rsPath.seekp(0);
        rsPath << MyPath();
        if (MyPath().IsDirectory())
            rsPath.pop_back(1);    // Remove closing delimiter.
    }
    return sPath.c_str();
}

} // namespace ploc
} // namespace csv




