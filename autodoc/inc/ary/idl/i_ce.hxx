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
