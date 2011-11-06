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



#ifndef SD_SLIDESORTER_CACHE_REQUEST_PRIORITY_CLASS_HXX
#define SD_SLIDESORTER_CACHE_REQUEST_PRIORITY_CLASS_HXX

namespace sd { namespace slidesorter { namespace cache {


/** Each request for a preview creation has a priority.  This enum defines
    the available priorities.  The special values MIN__CLASS and MAX__CLASS
    are/can be used for validation and have to be kept up-to-date.
*/
enum RequestPriorityClass
{
    MIN__CLASS = 0,

    // The slide is visible.  A preview does not yet exist.
    VISIBLE_NO_PREVIEW = MIN__CLASS,
    // The slide is visible.  A preview exists but is not up-to-date anymore.
    VISIBLE_OUTDATED_PREVIEW,
    // The slide is not visible.
    NOT_VISIBLE,

    MAX__CLASS = NOT_VISIBLE
};


} } } // end of namespace ::sd::slidesorter::cache

#endif
