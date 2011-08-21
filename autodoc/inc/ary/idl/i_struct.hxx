/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
