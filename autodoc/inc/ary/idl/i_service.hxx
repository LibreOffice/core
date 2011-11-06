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



#ifndef ARY_IDL_I_SERVICE_HXX
#define ARY_IDL_I_SERVICE_HXX

// BASE CLASSES
#include <ary/idl/i_ce.hxx>

// USED SERVICES
#include <ary/idl/i_comrela.hxx>
#include <ary/stdconstiter.hxx>
#include <ary/idl/ik_service.hxx>




namespace ary
{
namespace info
{
    class CodeInformation;
}
namespace idl
{
namespace ifc_service
{
    struct attr;
}


/** Represents an IDL service.
*/
class Service : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2003 };     // See reposy.cxx

    // LIFECYCLE
                        Service(
                            const String &      i_sName,
                            Ce_id               i_nOwner );
                        ~Service();

    // INQUIRY
    void                Get_SupportedInterfaces(
                            Dyn_StdConstIterator<CommentedRelation> &
                                                o_rResult ) const;
    void                Get_IncludedServices(
                            Dyn_StdConstIterator<CommentedRelation> &
                                                o_rResult ) const;

    // ACCESS
    void                Add_Property(
                            Ce_id               i_nProperty );
    void                AddRef_IncludedService(
                            Type_id             i_nService,
                            DYN doc::OldIdlDocu *
                                                pass_dpDocu );
    void                AddRef_SupportedInterface(
                            Type_id             i_nInterface,
                            DYN doc::OldIdlDocu *
                                                pass_dpDocu );
  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // Interface ary::Object:
    virtual ClassId     get_AryClass() const;

    // Interface CodeEntity
    virtual const String &  inq_LocalName() const;
    virtual Ce_id           inq_NameRoom() const;
    virtual Ce_id           inq_Owner() const;
    virtual E_SightLevel    inq_SightLevel() const;

    // Locals
    typedef std::vector< CommentedRelation >    RelationList;
    typedef std::vector<Ce_id>                  PropertyList;
    friend struct ifc_service::attr;

    // DATA
    String              sName;
    Ce_id               nOwner;

    RelationList        aIncludedServices;
    RelationList        aSupportedInterfaces;
    PropertyList        aProperties;
};




// IMPLEMENTATION
inline void
Service::Add_Property( Ce_id i_nProperty )
    { aProperties.push_back(i_nProperty); }

inline void
Service::AddRef_IncludedService( Type_id                    i_nService,
                                 DYN doc::OldIdlDocu  *     pass_dpDocu )
    { aIncludedServices.push_back( CommentedRelation(i_nService, pass_dpDocu) ); }

inline void
Service::AddRef_SupportedInterface( Type_id                 i_nInterface,
                                    DYN doc::OldIdlDocu *   pass_dpDocu )
    { aSupportedInterfaces.push_back( CommentedRelation(i_nInterface, pass_dpDocu) ); }





}   // namespace idl
}   // namespace ary
#endif
