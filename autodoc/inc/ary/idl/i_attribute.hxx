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

#ifndef ARY_IDL_I_ATTRIBUTE_HXX
#define ARY_IDL_I_ATTRIBUTE_HXX

// BASE CLASSES
#include <ary/idl/i_ce.hxx>




namespace ary
{
namespace idl
{
namespace ifc_attribute
{
    struct attr;
}




/** @resp
    Represents an IDL property.
*/
class Attribute : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2014 };

    // LIFECYCLE
                        Attribute(
                            const String &      i_sName,
                            Ce_id               i_nInterface,
                            Ce_id               i_nModule,
                            Type_id             i_nType,
                            bool                i_bReadonly,
                            bool                i_bBound );
                        ~Attribute();
    // OPERATIONS
    void                Add_GetException(
                            Type_id             i_nException );
    void                Add_SetException(
                            Type_id             i_nException );

    // INQUIRY
    Type_id             Type() const;
    bool                IsReadonly() const;
    bool                IsBound() const;

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

    // Local
    typedef std::vector< Type_id >      ExceptionList;
    friend struct ifc_attribute::attr;

    // DATA
    String              sName;
    Ce_id               nOwner;
    Ce_id               nNameRoom;

    Type_id             nType;
    ExceptionList       aGetExceptions;
    ExceptionList       aSetExceptions;
    bool                bReadonly;
    bool                bBound;
};




// IMPLEMENTATION
inline void
Attribute::Add_GetException( Type_id i_nException )
    { aGetExceptions.push_back(i_nException); }

inline void
Attribute::Add_SetException( Type_id i_nException )
    { aSetExceptions.push_back(i_nException); }

inline Type_id
Attribute::Type() const
    { return nType; }

inline bool
Attribute::IsReadonly() const
    { return bReadonly; }

inline bool
Attribute::IsBound() const
    { return bBound; }




}   // namespace idl
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
