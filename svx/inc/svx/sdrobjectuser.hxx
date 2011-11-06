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



#ifndef _SDR_OBJECTUSER_HXX
#define _SDR_OBJECTUSER_HXX

#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predeclarations
class SdrObject;

////////////////////////////////////////////////////////////////////////////////////////////////////
// To make things more safe, allow users of the page to register at it. The users need to be derived
// from sdr::PageUser to get a call. The users do not need to call RemovePageUser() at the page
// when they get called from PageInDestruction().

namespace sdr
{
    class ObjectUser
    {
    public:
        // this method is called form the destructor of the referenced page.
        // do all necessary action to forget the page. It is not necessary to call
        // RemovePageUser(), that is done form the destructor.
        virtual void ObjectInDestruction(const SdrObject& rObject) = 0;
    };

    // typedef for ObjectUserVector
    typedef ::std::vector< ObjectUser* > ObjectUserVector;
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OBJECTUSER_HXX
// eof
