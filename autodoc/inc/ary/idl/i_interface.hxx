/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_interface.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:09:29 $
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

#ifndef ARY_IDL_I_INTERFACE_HXX
#define ARY_IDL_I_INTERFACE_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/idl/i_ce.hxx>
    // COMPONENTS
#include <ary/idl/i_comrela.hxx>
    // PARAMETERS
#include <ary/stdconstiter.hxx>


namespace ary
{
namespace idl
{
namespace ifc_interface
{
    struct attr;
}

class Interface_2s;

/** @resp
    Represents an IDL interface.
*/
class Interface : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2001 };

    // LIFECYCLE
                        Interface(
                            const String &      i_sName,
                            Ce_id               i_nOwner );
                        ~Interface();
    // INQUIRY
    bool                HasBase() const;

    // ACCESS
    void                Add_Function(
                            Ce_id               i_nId );
    void                Add_Attribute(
                            Ce_id               i_nId );
    void                Add_Base(
                            Type_id             i_nInterface,
                            DYN info::CodeInformation *
                                                pass_dpDocu );

  private:
    // Interface ary::RepositoryEntity:
    virtual RCid        inq_ClassId() const;

    // Interface CodeEntity:
    virtual void            do_Visit_CeHost(CeHost & o_rHost) const;
    virtual const String &  inq_LocalName() const;
    virtual Ce_id           inq_NameRoom() const;
    virtual Ce_id           inq_Owner() const;
    virtual E_SightLevel    inq_SightLevel() const;

    // Local
    typedef std::vector< CommentedRelation >    RelationList;
    typedef std::vector<Ce_id>                  MemberList;
    friend struct ifc_interface::attr;

    // DATA
    String              sName;
    Ce_id               nOwner;
    RelationList        aBases;
    MemberList          aFunctions;
    MemberList          aAttributes;
    Dyn<Interface_2s>   p2s;
};



// IMPLEMENTATION

inline bool
Interface::HasBase() const
    { return aBases.size() > 0; }
inline void
Interface::Add_Function( Ce_id i_nId )
    { aFunctions.push_back(i_nId); }
inline void
Interface::Add_Attribute( Ce_id i_nId )
    { aAttributes.push_back(i_nId); }
inline void
Interface::Add_Base( Type_id                     i_nInterface,
                     DYN info::CodeInformation * pass_dpDocu )
    { aBases.push_back( CommentedRelation(i_nInterface, pass_dpDocu) ); }

}   // namespace idl
}   // namespace ary


#endif
