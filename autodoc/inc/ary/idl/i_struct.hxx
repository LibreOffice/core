/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_struct.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:12:18 $
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

#ifndef ARY_IDL_I_STRUCT_HXX
#define ARY_IDL_I_STRUCT_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/idl/i_ce.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace idl
{

namespace ifc_struct
{
    struct attr;
}


/** @resp
    Represents an IDL struct.
*/
class Struct : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2008 };

    // LIFECYCLE
                        Struct(
                            const String &      i_sName,
                            Ce_id               i_nOwner,
                            Type_id             i_nBase,
                            const String &      i_sTemplateParameter, // = String::Null_() // First let the compiler find old calls to this function
                            Type_id             i_nTemplateParameterType
                                                );
                        ~Struct();
    // INQUIRY
    Type_id             Base() const;
    String              TemplateParameter() const;
    Type_id             TemplateParameterType() const;

    // ACCESS
    void                Add_Member(
                            Ce_id               i_nMember );
  private:
    // Interface ary::RepositoryEntity
    virtual RCid        inq_ClassId() const;

    // Interface CodeEntity
    virtual void            do_Visit_CeHost(CeHost & o_rHost) const;
    virtual const String &  inq_LocalName() const;
    virtual Ce_id           inq_NameRoom() const;
    virtual Ce_id           inq_Owner() const;
    virtual E_SightLevel    inq_SightLevel() const;

    // Locals
    typedef std::vector<Ce_id>     ElementList;
    friend struct ifc_struct::attr;

    // DATA
    String              sName;
    Ce_id               nOwner;

    Type_id             nBase;
    String              sTemplateParameter;
    Type_id             nTemplateParameterType;
    ElementList         aElements;
};



// IMPLEMENTATION

inline Type_id
Struct::Base() const
{
    return nBase;
}

inline String
Struct::TemplateParameter() const
{
    return sTemplateParameter;
}

inline Type_id
Struct::TemplateParameterType() const
{
    return nTemplateParameterType;
}

inline void
Struct::Add_Member( Ce_id i_nMember )
{
    aElements.push_back(i_nMember);
}


}   // namespace idl
}   // namespace ary


#endif
