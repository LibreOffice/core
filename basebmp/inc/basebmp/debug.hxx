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



#ifndef INCLUDED_BASEBMP_DEBUG_HXX
#define INCLUDED_BASEBMP_DEBUG_HXX

#include <iostream>
#include <boost/shared_ptr.hpp>

namespace basebmp
{
    class BitmapDevice;

    /** Dump content of BitmapDevice to given output stream.

        @param rDevice
        Device whose content should be dumped.

        @param rOutputStream
        Stream to write output to.
    */
    void debugDump( const boost::shared_ptr< BitmapDevice >& rDevice,
                    ::std::ostream&                          rOutputStream );
}

#endif /* INCLUDED_BASEBMP_DEBUG_HXX */
