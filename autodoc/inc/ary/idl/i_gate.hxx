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



#ifndef ARY_IDL_I_GATE_HXX
#define ARY_IDL_I_GATE_HXX

// USED SERVICES

namespace autodoc
{
    class Options;
}
namespace ary
{
namespace idl
{
    class CePilot;
    class TypePilot;
}
}




namespace ary
{
namespace idl
{


/** Main entry to access the IDL parts of the repository.
*/
class Gate
{
  public:
    // LIFECYCLE
    virtual             ~Gate() {}

    // OPERATIONS
    virtual void        Calculate_AllSecondaryInformation(
                            const String &      i_devman_reffilepath ) = 0;
//                            const ::autodoc::Options &
//                                                i_options ) = 0;
    // INQUIRY
    virtual const CePilot &
                        Ces() const = 0;
    virtual const TypePilot &
                        Types() const = 0;
    // ACCESS
    virtual CePilot &   Ces() = 0;
    virtual TypePilot & Types() = 0;
};




}   // namespace idl
}   // namespace ary
#endif
