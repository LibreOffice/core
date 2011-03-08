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

#ifndef ARY_IDL_I_CE_HXX
#define ARY_IDL_I_CE_HXX

// BASE CLASSES
#include <ary/entity.hxx>
// USED SERVICES
#include <ary/doc/d_docu.hxx>
#include <ary/idl/i_ce2s.hxx>
#include <ary/idl/i_types4idl.hxx>






namespace ary
{
namespace idl
{


/** @resp Base class for all IDL code entities.

    A @->CodeEntity is a namespace, type, data or function, which occures in
    the parsed UNO IDL code and is described and/or commented within the
    Autodoc repository.

    This is a storage base class, where more special classes are
    derived from.
*/
class CodeEntity : public ary::Entity
{
  public:
    // LIFECYCLE
    virtual             ~CodeEntity();

    // OPERATION

    // INQUIRY
    Ce_id               CeId() const            { return Ce_id(Id()); }
    const String &      LocalName() const;
    Ce_id               NameRoom() const;
    Ce_id               Owner() const;
    E_SightLevel        SightLevel() const;

    const ary::doc::Documentation &
                        Docu() const;
    const Ce_2s &       Secondaries() const;

    static const CodeEntity &
                        Null_();
    // ACCESS
    void                Set_Docu(
                            DYN ary::doc::Node &
                                                pass_data );
    Ce_2s &             Secondaries();

  protected:
                        CodeEntity();
  private:
    // Locals
    virtual const String &  inq_LocalName() const = 0;
    virtual Ce_id           inq_NameRoom() const = 0;
    virtual Ce_id           inq_Owner() const = 0;
    virtual E_SightLevel    inq_SightLevel() const = 0;

    // DATA
    ary::doc::Documentation
                        aDocu;
    Dyn<Ce_2s>          p2s;
};




// IMPLEMENTATION
inline const String &
CodeEntity::LocalName() const
    { return inq_LocalName(); }

inline Ce_id
CodeEntity::NameRoom() const
    { return inq_NameRoom(); }

inline Ce_id
CodeEntity::Owner() const
    { return inq_Owner(); }

inline E_SightLevel
CodeEntity::SightLevel() const
    { return inq_SightLevel(); }

inline const ary::doc::Documentation &
CodeEntity::Docu() const
    { return aDocu; }

inline void
CodeEntity::Set_Docu(DYN ary::doc::Node & pass_data)
{
    aDocu.Set_Data(pass_data);
}




}   // namespace idl
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
