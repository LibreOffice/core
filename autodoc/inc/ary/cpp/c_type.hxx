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

#ifndef ARY_CPP_C_TYPE_HXX
#define ARY_CPP_C_TYPE_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/entity.hxx>
    // OTHER
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_traits.hxx>


namespace ary
{
namespace cpp
{
    class Gate;
}
}





namespace ary
{
namespace cpp
{



/**	Base of all C++ types
*/
class Type : public ::ary::Entity
{
  public:
    typedef Type_Traits       traits_t;

    virtual             ~Type() {}

    Type_id             TypeId() const          { return Type_id(Id()); }
    Ce_id               RelatedCe() const;
    bool                IsConst() const;
    void                Get_Text(
                            StreamStr &         o_rOut,
                            const Gate &	    i_rGate ) const;

    /** It is guaranteed, that the output is correct, also,
        if all three output-streams are the same instance.
    */
    void                Get_Text(
                            StreamStr &         o_rPreName,
                            StreamStr &         o_rName,
                            StreamStr &         o_rPostName,
                            const Gate &	    i_rGate ) const;

  private:
    virtual Rid         inq_RelatedCe() const;  // Defaulted to 0. Implemented in c_builtintype.cxx.
    virtual bool        inq_IsConst() const = 0;
    virtual void        inq_Get_Text(
                            StreamStr &         o_rPreName,
                            StreamStr &         o_rName,
                            StreamStr &         o_rPostName,
                            const Gate &	    i_rGate ) const = 0;
};




// IMPLEMENTATION
inline Ce_id
Type::RelatedCe() const
{
    return Ce_id(inq_RelatedCe());
}

inline bool
Type::IsConst() const
{
    return inq_IsConst();
}

inline void
Type::Get_Text( StreamStr &         o_rOut,
                const Gate &	    i_rGate ) const
{
    inq_Get_Text( o_rOut, o_rOut, o_rOut, i_rGate );
}

inline void
Type::Get_Text( StreamStr &         o_rPreName,
                StreamStr &         o_rName,
                StreamStr &         o_rPostName,
                const Gate &	    i_rGate ) const
{
    inq_Get_Text( o_rPreName,o_rName,o_rPostName, i_rGate );
}





}   // namespace  cpp
}   // namespace  ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
