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



#ifndef INCLUDED_TEST_OUSTRINGOSTREAMINSERTER_HXX
#define INCLUDED_TEST_OUSTRINGOSTREAMINSERTER_HXX

#include "sal/config.h"

#include <ostream>

#include "osl/thread.h"
#include "rtl/ustring.hxx"

// Include this header to support rtl::OUString in CPPUNIT_ASSERT macros.

template< typename charT, typename traits > std::basic_ostream<charT, traits> &
operator <<(
    std::basic_ostream<charT, traits> & stream, rtl::OUString const & string)
{
    return stream <<
        rtl::OUStringToOString(string, osl_getThreadTextEncoding()).getStr();
        // best effort; potentially loses data due to conversion failures and
        // embedded null characters
}

#endif
