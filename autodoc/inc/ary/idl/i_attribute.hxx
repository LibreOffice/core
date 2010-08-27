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
