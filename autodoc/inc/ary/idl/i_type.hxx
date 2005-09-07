/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_type.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:12:58 $
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

#ifndef ARY_IDL_I_TYPE_HXX
#define ARY_IDL_I_TYPE_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <ary/re.hxx>
    // PARAMETERS
#include <ary/idl/i_language.hxx>


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
                            RCid                i_nCeId );
};


/** @resp Base of all IDL types.

    @->Type represents the occurence of a type as base,
    parameter, return type or element type in UNO IDL code.
    Some of them relate to a @->CodeEntity, but
    the @->Type "MyInterface" is something different than
    the @->CodeEntity "MyInterface".

    This is a storage base class, where more special
    classes are derived from.
*/
class Type : public n22::RepositoryEntity
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
