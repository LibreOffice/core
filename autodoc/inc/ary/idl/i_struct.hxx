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



#ifndef ARY_IDL_I_STRUCT_HXX
#define ARY_IDL_I_STRUCT_HXX

// BASE CLASSES
#include <ary/idl/i_ce.hxx>




namespace ary
{
namespace idl
{
namespace ifc_struct
{
    struct attr;
}


/** Represents an IDL struct.
*/
class Struct : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2008 };

    // LIFECYCLE
                        Struct(
                            const String &      i_sName,
                            Ce_id               i_nOwner,
                            Type_id             i_nBase,
                            const String &      i_sTemplateParameter,
                            Type_id             i_nTemplateParameterType );
                        ~Struct();
    // INQUIRY
    Type_id             Base() const;
    String              TemplateParameter() const;
    Type_id             TemplateParameterType() const;

    // ACCESS
    void                Add_Member(
                            Ce_id               i_nMember );
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
    typedef std::vector<Ce_id>     ElementList;
    friend struct ifc_struct::attr;

    // DATA
    String              sName;
    Ce_id               nOwner;

    Type_id             nBase;
    String              sTemplateParameter;
    Type_id             nTemplateParameterType;
    ElementList         aElements;
};




// IMPLEMENTATION
inline Type_id
Struct::Base() const
{
    return nBase;
}

inline String
Struct::TemplateParameter() const
{
    return sTemplateParameter;
}

inline Type_id
Struct::TemplateParameterType() const
{
    return nTemplateParameterType;
}

inline void
Struct::Add_Member( Ce_id i_nMember )
{
    aElements.push_back(i_nMember);
}




}   // namespace idl
}   // namespace ary
#endif
