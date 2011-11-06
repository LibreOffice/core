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



#ifndef ARY_ARY_HXX
#define ARY_ARY_HXX


// USED SERVICES
    // BASE CLASSES
    // OTHER

namespace ary
{
namespace idl
{
    class Gate;
}

namespace cpp
{
    class Gate;
}
}



namespace ary
{

/** Starting point for all work with the
    Autodoc Sourcecode Repository.

    Create and destroy the repository and
    give access to the "Gates" for different tasks.

    @collab ::ary::cpp::Gate
    @collab ::ary::idl::Gate
*/

class Repository
{
  public:
    //  LIFECYCLE
    virtual             ~Repository() {}
    static DYN Repository &
                        Create_();
    // INQUIRY
    virtual const String &
                        Title() const = 0;
    virtual const ::ary::cpp::Gate &
                        Gate_Cpp() const = 0;
    virtual const ::ary::idl::Gate &
                        Gate_Idl() const = 0;
    // ACCESS
    virtual ::ary::cpp::Gate &
                        Gate_Cpp() = 0;
    virtual ::ary::idl::Gate &
                        Gate_Idl() = 0;
    virtual void        Set_Title(
                            const String &      i_sName ) = 0;
};



} // namespace ary
#endif
