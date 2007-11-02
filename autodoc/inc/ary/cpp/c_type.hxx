/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_type.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:52:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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



/** Base of all C++ types
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
                            const Gate &        i_rGate ) const;

    /** It is guaranteed, that the output is correct, also,
        if all three output-streams are the same instance.
    */
    void                Get_Text(
                            StreamStr &         o_rPreName,
                            StreamStr &         o_rName,
                            StreamStr &         o_rPostName,
                            const Gate &        i_rGate ) const;

  private:
    virtual Rid         inq_RelatedCe() const;  // Defaulted to 0. Implemented in c_builtintype.cxx.
    virtual bool        inq_IsConst() const = 0;
    virtual void        inq_Get_Text(
                            StreamStr &         o_rPreName,
                            StreamStr &         o_rName,
                            StreamStr &         o_rPostName,
                            const Gate &        i_rGate ) const = 0;
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
                const Gate &        i_rGate ) const
{
    inq_Get_Text( o_rOut, o_rOut, o_rOut, i_rGate );
}

inline void
Type::Get_Text( StreamStr &         o_rPreName,
                StreamStr &         o_rName,
                StreamStr &         o_rPostName,
                const Gate &        i_rGate ) const
{
    inq_Get_Text( o_rPreName,o_rName,o_rPostName, i_rGate );
}





}   // namespace  cpp
}   // namespace  ary
#endif
