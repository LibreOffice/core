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
#include <cosv/std_outp.hxx>

// NOT FULLY DECLARED SERVICES
#include <stdlib.h>


namespace csv
{

#ifdef CSV_NO_IOSTREAMS
redirect_out *  redirect_out::pStdOut_ = 0;
redirect_out *  redirect_out::pStdErr_ = 0;
#endif // defined(CSV_NO_IOSTREAMS)


void
Endl( ostream & io_rStream, bool /*bUseRedirect*/, int* )
{
#ifndef CSV_NO_IOSTREAMS
//    if (NOT bUseRedirect)
        io_rStream << std::endl;
//    else
#endif
//        static_cast< redirect_out& >(io_rStream).re_endl();
}

void
Flush( ostream & io_rStream, bool /*bUseRedirect*/, int* )
{
#ifndef CSV_NO_IOSTREAMS
//    if (NOT bUseRedirect)
        io_rStream << std::flush;
//    else
#endif
//        static_cast< redirect_out& >(io_rStream).re_flush();
}

void
PerformAssertion(const char * condition, const char * file, unsigned line)
{
    Cout() << "assertion failed: "
         << condition
         << " in file: "
         << file
         << " at line: "
         << line
         << Endl;

    exit(3);
}

}   // namespace csv

