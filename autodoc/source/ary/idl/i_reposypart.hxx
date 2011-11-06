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



#ifndef ARY_IDL_I_REPOSYPART_HXX
#define ARY_IDL_I_REPOSYPART_HXX



// USED SERVICES
    // BASE CLASSES
#include <idl_internalgate.hxx>
    // OTHER


namespace ary
{
namespace idl
{
    class CeAdmin;
    class TypeAdmin;
    class NameLookup;
}
}





namespace ary
{
namespace idl
{



/** The idl part of the Autodoc repository.
*/
class RepositoryPartition : public InternalGate
{
  public:
    // LIFECYCLE
                        RepositoryPartition(
                            RepositoryCenter &  i_repository );
                        ~RepositoryPartition();
    // INHERITED
        // Interface Gate:
    virtual void        Calculate_AllSecondaryInformation(
                            const String &      i_devman_reffilepath );
//                            const ::autodoc::Options &
//                                                i_options );
    virtual const CePilot &
                        Ces() const;
    virtual const TypePilot &
                        Types() const;
    virtual CePilot &   Ces();
    virtual TypePilot & Types();

  private:
    // DATA
    RepositoryCenter *  pCenter;

    Dyn<CeAdmin>        pCes;
    Dyn<TypeAdmin>      pTypes;
    Dyn<NameLookup>     pNamesDictionary;
};




}   //  namespace idl
}   //  namespace ary
#endif
