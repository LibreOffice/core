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



#ifndef _NEONHEADREQUEST_HXX_
#define _NEONHEADREQUEST_HXX_

#include <vector>
#include "NeonTypes.hxx"
#include "DAVResource.hxx"

namespace webdav_ucp
{

class NeonHeadRequest
{
public:
        // named / allprop
        NeonHeadRequest( HttpSession* inSession,
                         const rtl::OUString & inPath,
                         const std::vector< ::rtl::OUString > & inHeaderNames,
                         DAVResource & ioResource,
                         int & nError );
        ~NeonHeadRequest();
};

} // namespace webdav_ucp

#endif // _NEONHEADREQUEST_HXX_

