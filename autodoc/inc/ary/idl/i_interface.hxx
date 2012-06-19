/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
