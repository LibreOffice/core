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

#ifndef ARY_IDL_I_CONSTANT_HXX
#define ARY_IDL_I_CONSTANT_HXX

// BASE CLASSES
#include <ary/idl/i_ce.hxx>




namespace ary
{
namespace idl
{
namespace ifc_constant
{
    struct attr;
}


/** Represents an IDL constant.
*/
class Constant : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2012 };

    // LIFECYCLE
                        Constant(
                            const String &      i_sName,
                            Ce_id               i_nOwner,
                            Ce_id               i_nNameRoom,
                            Type_id             i_nType,
                            const String &      i_sInitValue );
                        ~Constant();
    // INQUIRY
    Type_id             Type() const;
    const String &      Value() const;


  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // Interface ary::Object
    virtual ClassId     get_AryClass() const;

    // Interface CodeEntity
    virtual const String &  inq_LocalName() const;
    virtual Ce_id           inq_NameRoom() const;
    virtual Ce_id           inq_Owner() const;
    virtual E_SightLevel    inq_SightLevel() const;

    friend struct ifc_constant::attr;

    // DATA
    String              sName;
    Ce_id               nNameRoom;
    Ce_id               nOwner;

    Type_id             nType;
    String              sInitValue;
};




// IMPLEMENTATION
inline Type_id
Constant::Type() const
{
    return nType;
}

inline const String &
Constant::Value() const
{
    return sInitValue;
}




}   // namespace idl
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
