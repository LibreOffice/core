/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: i_type.hxx,v $
 * $Revision: 1.6 $
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

#ifndef ARY_IDL_I_TYPE_HXX
#define ARY_IDL_I_TYPE_HXX

// USED SERVICES
#include <ary/entity.hxx>
#include <ary/idl/i_types4idl.hxx>




namespace ary
{
namespace idl
{
    class Gate;


/** Abstract base for all secondary productions of types
*/
class Type_2s
{
  public:
    virtual             ~Type_2s() {}

    static DYN Type_2s *
                        Create_(
                            ClassId             i_nCeId );
};


/** Base of all IDL types.

    Type represents the occurence of a type as base,
    parameter, return type or element type in UNO IDL code.
    Some of them relate to a ->CodeEntity, but
    the ->Type "MyInterface" is something different than
    the ->CodeEntity "MyInterface".

    This is a storage base class, where more special
    classes are derived from.
*/
class Type : public ary::Entity
{
  public:
    typedef Type_2s secondary_productions;

    // LIFECYCLE
    virtual             ~Type() {}

    // INQUIRY
    Type_id             TypeId() const          { return Type_id(Id()); }

    /// @descr Does NOT clear the output-parameters.
    void                Get_Text(
                            StringVector &      o_module,
                            String &            o_name,
                            Ce_id &             o_nRelatedCe,
                            int &               o_nSequenceCount,
                            const Gate &        i_rGate ) const;
    Type_id             TemplateParameterType() const;

  private:
    virtual void        inq_Get_Text(
                            StringVector &      o_module,
                            String &            o_name,
                            Ce_id &             o_nRelatedCe,
                            int &               o_nSequemceCount,
                            const Gate &        i_rGate ) const = 0;
    virtual Type_id     inq_TemplateParameterType() const;
};




// IMPLEMENTATION
inline void
Type::Get_Text( StringVector &      o_module,
                String &            o_name,
                Ce_id &             o_nRelatedCe,
                int &               o_nSequenceCount,
                const Gate &        i_rGate ) const
{
    inq_Get_Text(o_module,o_name,o_nRelatedCe,o_nSequenceCount,i_rGate);
}

inline Type_id
Type::TemplateParameterType() const
{
    return inq_TemplateParameterType();
}




}   // namespace idl
}   // namespace ary
#endif
