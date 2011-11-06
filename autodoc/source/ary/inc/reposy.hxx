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



#ifndef ARY_REPOSY_HXX
#define ARY_REPOSY_HXX

// BASE CLASSES
#include <ary/ary.hxx>
// USED SERVICES
#include <cosv/ploc_dir.hxx>

namespace ary
{
namespace cpp
{
    class InternalGate;
}
namespace idl
{
    class InternalGate;
}
}   // namespace ary




namespace ary
{


/** Implements ::ary::Repository.

    @see Repository
*/

class RepositoryCenter : public ::ary::Repository
{
  public:
    //  LIFECYCLE
                        RepositoryCenter();
    virtual             ~RepositoryCenter();

    // INHERITED
        // Interface Repository:
    virtual const cpp::Gate &   Gate_Cpp() const;
    virtual const idl::Gate &   Gate_Idl() const;
    virtual const String &      Title() const;
    virtual cpp::Gate &         Gate_Cpp();
    virtual idl::Gate &         Gate_Idl();
    virtual void                Set_Title(const String & i_sName );

  private:
    // DATA
    String              sDisplayedName;     /// Name to be displayed for human users.
    csv::ploc::Directory
                        aLocation;
    Dyn<cpp::InternalGate>
                        pCppPartition;
    Dyn<idl::InternalGate>
                        pIdlPartition;
};




}   // namespace ary
#endif
