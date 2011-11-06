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



#ifndef ARY_IDL_I_SINGLETON_HXX
#define ARY_IDL_I_SINGLETON_HXX

// BASE CLASSES
#include <ary/idl/i_ce.hxx>




namespace ary
{
namespace idl
{
namespace ifc_singleton
{
    struct attr;
}


/** Represents an IDL singleton.
*/
class Singleton : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2013 };

    // LIFECYCLE
                        Singleton(
                            const String &      i_sName,
                            Ce_id               i_nOwner );
                        ~Singleton();
    // INQUIRY
    Type_id             AssociatedService() const
                                                { return nService; }

    // ACCESS
    void                Set_Service(
                            Type_id             i_nService );
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
    friend struct ifc_singleton::attr;

    // DATA
    String              sName;
    Ce_id               nOwner;

    Type_id             nService;
};





// IMPLEMENTATION
inline void
Singleton::Set_Service( Type_id i_nService )
{
    nService = i_nService;
}




}   // namespace idl
}   // namespace ary
#endif
