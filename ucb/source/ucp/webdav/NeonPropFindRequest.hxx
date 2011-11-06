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



#ifndef _NEONPROPFINDREQUEST_HXX_
#define _NEONPROPFINDREQUEST_HXX_

#include <vector>
#include <rtl/ustring.hxx>
#include "NeonTypes.hxx"
#include "DAVTypes.hxx"
#include "DAVResource.hxx"

namespace webdav_ucp
{

class NeonPropFindRequest
{
public:
    // named / allprop
    NeonPropFindRequest( HttpSession* inSession,
                         const char*  inPath,
                         const Depth  inDepth,
                         const std::vector< ::rtl::OUString > & inPropNames,
                         std::vector< DAVResource > & ioResources,
                         int & nError );
    // propnames
    NeonPropFindRequest( HttpSession* inSession,
                         const char*  inPath,
                         const Depth  inDepth,
                         std::vector< DAVResourceInfo > & ioResInfo,
                         int & nError );

    ~NeonPropFindRequest();
};

} // namespace webdav_ucp

#endif // _NEONPROPFINDREQUEST_HXX_
