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

#ifndef ARY_IDL_I_INTERFACE_HXX
#define ARY_IDL_I_INTERFACE_HXX

// BASE CLASSES
#include <ary/idl/i_ce.hxx>

// USED SERVICES
#include <ary/idl/i_comrela.hxx>
#include <ary/stdconstiter.hxx>




namespace ary
{
namespace idl
{
namespace ifc_interface
{
    struct attr;
}
    class Interface_2s;


/** Represents an IDL interface.
*/
class Interface : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2001 };

    // LIFECYCLE
                        Interface(
                            const String &      i_sName,
                            Ce_id               i_nOwner );
                        ~Interface();
    // INQUIRY
    bool                HasBase() const;

    // ACCESS
    void                Add_Function(
                            Ce_id               i_nId );
    void                Add_Attribute(
                            Ce_id               i_nId );
    void                Add_Base(
                            Type_id             i_nInterface,
                            DYN doc::OldIdlDocu *
                                                pass_dpDocu );

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
    typedef std::vector< CommentedRelation >    RelationList;
    typedef std::vector<Ce_id>                  MemberList;
    friend struct ifc_interface::attr;

    // DATA
    String              sName;
    Ce_id               nOwner;
    RelationList        aBases;
    MemberList          aFunctions;
    MemberList          aAttributes;
    Dyn<Interface_2s>   p2s;
};




// IMPLEMENTATION
inline bool
Interface::HasBase() const
    { return aBases.size() > 0; }
inline void
Interface::Add_Function( Ce_id i_nId )
    { aFunctions.push_back(i_nId); }
inline void
Interface::Add_Attribute( Ce_id i_nId )
    { aAttributes.push_back(i_nId); }
inline void
Interface::Add_Base( Type_id                i_nInterface,
                     DYN doc::OldIdlDocu *  pass_dpDocu )
    { aBases.push_back( CommentedRelation(i_nInterface, pass_dpDocu) ); }




}   // namespace idl
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
