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



#ifndef ARY_IDL_I_SISERVICE_HXX
#define ARY_IDL_I_SISERVICE_HXX

// BASE CLASSES
#include <ary/idl/i_ce.hxx>




namespace ary
{
namespace idl
{
namespace ifc_sglifcservice
{
    struct attr;
}


/** Represents an IDL single interface service.
*/
class SglIfcService : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2015 };     // See reposy.cxx

    // LIFECYCLE
                        SglIfcService(
                            const String &      i_sName,
                            Ce_id               i_nOwner,
                            Type_id             i_nBaseInterface );
                        ~SglIfcService();
    // ACCESS
    void                Add_Constructor(
                            Ce_id               i_nId );

    // INFO
    Type_id             BaseInterface() const   { return nBaseInterface; }

  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // Interface ary::Object:
    virtual ClassId     get_AryClass() const;

    // Interface CodeEntity:
    virtual const String &  inq_LocalName() const;
    virtual Ce_id           inq_NameRoom() const;
    virtual Ce_id           inq_Owner() const;
    virtual E_SightLevel    inq_SightLevel() const;

    // Local
    typedef std::vector<Ce_id>          CtorList;
    friend struct ifc_sglifcservice::attr;

    // DATA
    String              sName;
    Ce_id               nOwner;
    Type_id             nBaseInterface;

    CtorList            aConstructors;
};




// IMPLEMENTATION
inline void
SglIfcService::Add_Constructor( Ce_id i_nId )
{
    aConstructors.push_back(i_nId);
}




}   // namespace idl
}   // namespace ary
#endif
