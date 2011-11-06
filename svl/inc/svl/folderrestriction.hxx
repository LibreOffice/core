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



#ifndef SVTOOLS_FOLDER_RESTRICTION_HXX
#define SVTOOLS_FOLDER_RESTRICTION_HXX

#include "svl/svldllapi.h"
#include <tools/string.hxx>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

//........................................................................
namespace svt
{
//........................................................................

    /** retrieves a list of folders which's access is not restricted.

        <p>Note that this is not meant as security feature, but only as
        method to restrict some UI presentation, such as browsing
        in the file open dialog.</p>
    */
    SVL_DLLPUBLIC void getUnrestrictedFolders( ::std::vector< String >& _rFolders );

//........................................................................
}   // namespace svt
//........................................................................

#endif // SVTOOLS_FOLDER_RESTRICTION_HXX
