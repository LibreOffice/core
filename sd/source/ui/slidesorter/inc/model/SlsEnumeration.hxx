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



#ifndef SD_SLIDESORTER_ENUMERATION_HXX
#define SD_SLIDESORTER_ENUMERATION_HXX

#include <memory>

namespace sd { namespace slidesorter { namespace model {


/** Interface to generic enumerations.  Designed to operate on shared
    pointers.  Therefore GetNextElement() returns T and not T&.
*/
template <class T>
class Enumeration
{
public:
    virtual bool HasMoreElements (void) const = 0;
    /** Returns T instead of T& so that it can handle shared pointers.
    */
    virtual T GetNextElement (void) = 0;
    virtual void Rewind (void) = 0;
    virtual ::std::auto_ptr<Enumeration<T> > Clone (void) = 0;
};

} } } // end of namespace ::sd::slidesorter::model

#endif
