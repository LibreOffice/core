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



#ifndef _XMLOFF_FUNCTIONAL_HXX
#define _XMLOFF_FUNCTIONAL_HXX

#include <rtl/ustring.hxx>

/* THIS HEADER IS DEPRECATED. USE comphelper/stl_types.hxx INSTEAD!!! */

/** @#file
 *
 * re-implement STL functors as needed
 *
 * The standard comparison operators from the STL cause warnings with
 * several compilers about our sal_Bool (=unsigned char) being
 * converted to bool (C++ bool). We wish to avoid that.
 */

struct less_functor
{
    bool operator()(const ::rtl::OUString& x,
                    const ::rtl::OUString& y) const
    {
        return 0 != (x<y);
    }
};


#endif
